/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "vir/transform/gc_vsc_vir_cpp.h"

typedef struct VSC_CPP_USAGE
{
    VIR_Instruction *inst;
    VIR_Operand     *opnd;
} VSC_CPP_Usage;

static gctUINT _VSC_CPP_Usage_HFUNC(const void *ptr)
{
    return (gctUINT)(gctUINTPTR_T)((VSC_CPP_Usage*)ptr)->inst >> 2;
}

static gctBOOL _VSC_CPP_Usage_HKCMP(const void *pHashKey1, const void *pHashKey2)
{
    return (((VSC_CPP_Usage*)pHashKey1)->inst == ((VSC_CPP_Usage*)pHashKey2)->inst)
        && (((VSC_CPP_Usage*)pHashKey1)->opnd == ((VSC_CPP_Usage*)pHashKey2)->opnd);
}

static VSC_CPP_Usage* _VSC_CPP_NewUsage(
    IN OUT VSC_CPP_CopyPropagation  *cpp,
    IN VIR_USAGE                    *usage
    )
{
    VSC_CPP_Usage   *pUsage = (VSC_CPP_Usage*)vscMM_Alloc(VSC_CPP_GetMM(cpp), sizeof(VSC_CPP_Usage));
    if(!pUsage)
    {
        return pUsage;
    }
    pUsage->inst = usage->usageKey.pUsageInst;
    pUsage->opnd = usage->usageKey.pOperand;
    return pUsage;
}

static void VSC_CPP_Init(
    IN OUT VSC_CPP_CopyPropagation  *cpp,
    IN gcePATCH_ID                  patchId,
    IN VIR_Shader                   *shader,
    IN VIR_DEF_USAGE_INFO           *du_info,
    IN VSC_OPTN_CPPOptions          *options,
    IN VSC_CPP_PASS_DATA            *passData,
    IN VIR_Dumper                   *dumper,
    IN VSC_MM                       *pMM,
    IN VSC_HW_CONFIG                *pHwCfg
    )
{
    VSC_CPP_SetAppNameId(cpp, patchId);
    VSC_CPP_SetShader(cpp, shader);
    VSC_CPP_SetCurrBB(cpp, gcvNULL);
    VSC_CPP_SetDUInfo(cpp, du_info);
    VSC_CPP_SetOptions(cpp, options);
    VSC_CPP_SetPassData(cpp, passData);
    VSC_CPP_SetDumper(cpp, dumper);
    VSC_CPP_SetFWOptCount(cpp, 0);
    VSC_CPP_SetBWOptCount(cpp, 0);
    VSC_CPP_SetMM(cpp, pMM);
    VSC_CPP_SetHWCFG(cpp, pHwCfg);
    VSC_CPP_SetInvalidCfg(cpp, gcvFALSE);

    memset(&cpp->checkRedefinedResInfo, 0, sizeof(VSC_CHECK_REDEFINED_RES));
    cpp->checkRedefinedResInfo.pMM = pMM;
}

static void VSC_CPP_Final(
    IN OUT VSC_CPP_CopyPropagation  *cpp
    )
{
    VSC_CHECK_REDEFINED_RES checkRedefinedResInfo = cpp->checkRedefinedResInfo;

    VSC_CPP_SetShader(cpp, gcvNULL);
    VSC_CPP_SetOptions(cpp, gcvNULL);
    VSC_CPP_SetDumper(cpp, gcvNULL);

    /* Free the resource for the redefined instruction. */
    if (checkRedefinedResInfo.pInstHashTable != gcvNULL)
    {
        vscHTBL_Destroy(checkRedefinedResInfo.pInstHashTable);
    }

    if (checkRedefinedResInfo.pBBHashTable != gcvNULL)
    {
        vscHTBL_Destroy(checkRedefinedResInfo.pBBHashTable);
    }

    if (checkRedefinedResInfo.pBBFlowMask != gcvNULL)
    {
        vscBV_Destroy(checkRedefinedResInfo.pBBFlowMask);
    }

    if (checkRedefinedResInfo.pBBCheckStatusMask != gcvNULL)
    {
        vscBV_Destroy(checkRedefinedResInfo.pBBCheckStatusMask);
    }

    if (checkRedefinedResInfo.pBBCheckValueMask != gcvNULL)
    {
        vscBV_Destroy(checkRedefinedResInfo.pBBCheckValueMask);
    }
}

/*
    If there is a call is between endInst and startInst, return gcvTURE.

    travese backward from the endInst (to all its predecessors):
    if meet startInst, return gcvFALSE;
    if meet the entry, return gcvFALSE;
    if meet call, return gcvTRUE;
    if the currBB is already visited, return gcvFALSE;
*/

static gctBOOL _VSC_CPP_CallInstInBetween(
    IN VIR_Instruction      *startInst,
    IN VIR_Instruction      *endInst,
    IN OUT VSC_HASH_TABLE   *visitSet
    )
{
    VIR_BASIC_BLOCK     *currBB = VIR_Inst_GetBasicBlock(endInst);
    VIR_Instruction     *currInst;

    if (vscHTBL_DirectTestAndGet(visitSet, (void*) currBB, gcvNULL))
    {
        return gcvFALSE;
    }

    vscHTBL_DirectSet(visitSet, (void*) currBB, gcvNULL);

    currInst = endInst;
    while (currInst)
    {
        /* if there is a call instruction in between, we should treat it as black box */
        if (VIR_Inst_GetOpcode(currInst) == VIR_OP_CALL)
        {
            return gcvTRUE;
        }

        if (currInst == startInst)
        {
            return gcvFALSE;
        }

        currInst = VIR_Inst_GetPrev(currInst);
    }

    /* go through all its predecessors */
    {
        VIR_BASIC_BLOCK             *pPredBasicBlk;
        VSC_ADJACENT_LIST_ITERATOR  predEdgeIter;
        VIR_CFG_EDGE                *pPredEdge;

        if (DGND_GET_IN_DEGREE(&currBB->dgNode) == 0)
        {
            return gcvFALSE;
        }

        VSC_ADJACENT_LIST_ITERATOR_INIT(&predEdgeIter, &currBB->dgNode.predList);
        pPredEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&predEdgeIter);
        for (; pPredEdge != gcvNULL;
            pPredEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&predEdgeIter))
        {
            pPredBasicBlk = CFG_EDGE_GET_TO_BB(pPredEdge);

            /* entry block or empty block */
            if (pPredBasicBlk->flowType == VIR_FLOW_TYPE_ENTRY ||
                BB_GET_END_INST(pPredBasicBlk) == gcvNULL)
            {
                continue;
            }

            if (_VSC_CPP_CallInstInBetween(startInst, BB_GET_END_INST(pPredBasicBlk), visitSet))
            {
                return gcvTRUE;
            }
        }
    }

    return gcvFALSE;
}

static VSC_ErrCode _VSC_CPP_RemoveDefInst(
    IN OUT VSC_CPP_CopyPropagation  *cpp,
    IN     VIR_Instruction          *defInst)
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    VIR_Function    *func = VIR_Inst_GetFunction(defInst);
    VSC_OPTN_CPPOptions *options = VSC_CPP_GetOptions(cpp);

    VIR_DEF         *pDef;
    VIR_DEF_KEY     defKey;
    gctUINT         tempDefIdx;
    VIR_OperandInfo dstInfo, movSrcInfo;

    VIR_Operand     *movDst = VIR_Inst_GetDest(defInst);
    VIR_Operand     *movSrc = VIR_Inst_GetSource(defInst, 0);

    VIR_Swizzle movSrcSwizzle = VIR_Operand_GetSwizzle(movSrc);
    VIR_Enable  movEnable = VIR_Operand_GetEnable(movDst);

    VIR_Operand_GetOperandInfo(defInst, movDst, &dstInfo);
    VIR_Operand_GetOperandInfo(defInst, movSrc, &movSrcInfo);

    defKey.pDefInst = defInst;
    defKey.regNo = dstInfo.u1.virRegInfo.virReg;
    defKey.channel = VIR_CHANNEL_ANY;
    tempDefIdx = vscBT_HashSearch(&VSC_CPP_GetDUInfo(cpp)->defTable, &defKey);

    while (VIR_INVALID_DEF_INDEX != tempDefIdx)
    {
        pDef = GET_DEF_BY_IDX(&VSC_CPP_GetDUInfo(cpp)->defTable, tempDefIdx);

        if (pDef->defKey.pDefInst == defInst &&
            !DU_CHAIN_CHECK_EMPTY(&pDef->duChain))
        {
            return errCode;
        }

        tempDefIdx = pDef->nextDefIdxOfSameRegNo;
    }

    /* remove the def */
    vscVIR_DeleteDef(
        VSC_CPP_GetDUInfo(cpp),
        defInst,
        dstInfo.u1.virRegInfo.virReg,
        1,
        movEnable,
        VIR_HALF_CHANNEL_MASK_FULL,
        gcvNULL);

    if (movSrcInfo.isVreg)
    {
        vscVIR_DeleteUsage(VSC_CPP_GetDUInfo(cpp),
            VIR_ANY_DEF_INST,
            defInst,
            movSrc,
            gcvFALSE,
            movSrcInfo.u1.virRegInfo.virReg,
            1,
            VIR_Swizzle_2_Enable(movSrcSwizzle),
            VIR_HALF_CHANNEL_MASK_FULL,
            gcvNULL);
    }

    if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
        VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
    {
        VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
        VIR_LOG(dumper, "[FW] ==> removed instruction\n");
        VIR_LOG_FLUSH(dumper);
        VIR_Inst_Dump(dumper, defInst);
        VIR_LOG_FLUSH(dumper);
    }

    /* remove MOV */
    errCode = vscVIR_DeleteInstructionWithDu(gcvNULL, func, defInst, &VSC_CPP_GetInvalidCfg(cpp));

    return errCode;
}

static VSC_ErrCode _VSC_CPP_ReplaceSource(
    IN OUT VSC_CPP_CopyPropagation  *cpp,
    IN     VIR_Instruction          *inst,
    IN     VIR_Operand              *parentSrcOpnd,
    IN     gctUINT                  srcNum,
    IN     VIR_Operand              *newSrc
    )
{
    VSC_ErrCode     errCode  = VSC_ERR_NONE;
    VIR_Function    *function = VIR_Inst_GetFunction(inst);

    if (parentSrcOpnd != gcvNULL)
    {
        VIR_ParmPassing *parm = VIR_Operand_GetParameters(parentSrcOpnd);

        gcmASSERT(VIR_Operand_isParameters(parentSrcOpnd) && srcNum < parm->argNum);

        VIR_Function_FreeOperand(function, parm->args[srcNum]);

        parm->args[srcNum] = newSrc;
    }
    else
    {
        VIR_Inst_FreeSource(inst, srcNum);
        VIR_Inst_SetSource(inst, srcNum, newSrc);
    }

    return errCode;
}

static gctBOOL _VSC_CPP_CopySrcTypeFromMov(
    IN OUT VSC_CPP_CopyPropagation* pCpp,
    IN     VIR_Instruction*         pInst,
    IN     VIR_Instruction*         pMovInst,
    IN     VIR_Operand*             pNewSrc
    )
{
    gctBOOL         bCopySrcTypeFromMov = gcvFALSE;
    VIR_Operand*    pSrc0 = VIR_Inst_GetSource(pInst, 0);

    if (VSC_CPP_GetFlag(pCpp) & VSC_CPP_USE_SRC_TYPE_FROM_MOVE)
    {
        VIR_OpCode  opCode = VIR_Inst_GetOpcode(pInst);

        /* Now we only copy the src type from MOV for imageFetch(sampler2DMS). */
        if (opCode == VIR_OP_INTRINSIC &&
            VIR_Intrinsics_isImageFetch(VIR_Operand_GetIntrinsicKind(pSrc0)) &&
            VIR_TypeId_isSamplerMS(VIR_Operand_GetTypeId(pNewSrc)))
        {
            bCopySrcTypeFromMov = gcvTRUE;
        }
    }

    return bCopySrcTypeFromMov;
}

static gctBOOL _VSC_CPP_AnyOtherUsageCanNotBeOptimize(
    IN VIR_DEF_USAGE_INFO*          pDuInfo,
    IN VIR_Instruction*             pCurrentUsageInst,
    IN VIR_Instruction*             pDefInst,
    IN VIR_Enable                   currentEnable,
    IN gctUINT                      defRegNo
    )
{
    gctUINT8                        channel;
    VIR_GENERAL_DU_ITERATOR         du_iter;
    VIR_USAGE*                      pUsage = gcvNULL;
    gctBOOL                         bResult = gcvFALSE;

    for (channel = 0; channel < VIR_CHANNEL_COUNT; ++channel)
    {
        if (!(currentEnable & (1 << channel)))
        {
            continue;
        }
        vscVIR_InitGeneralDuIterator(&du_iter,
                                     pDuInfo,
                                     pDefInst,
                                     defRegNo,
                                     channel,
                                     gcvFALSE);

        for (pUsage = vscVIR_GeneralDuIterator_First(&du_iter);
             pUsage != gcvNULL;
             pUsage = vscVIR_GeneralDuIterator_Next(&du_iter))
        {
            VIR_Instruction* pUsageInst = pUsage->usageKey.pUsageInst;

            if (VIR_IS_OUTPUT_USAGE_INST(pUsageInst))
            {
                bResult = gcvTRUE;
                return bResult;
            }

            /* A rough check here, only check the previous instructions. */
            if (pCurrentUsageInst != gcvNULL &&
                VIR_Inst_GetId(pUsageInst) < VIR_Inst_GetId(pCurrentUsageInst))
            {
                continue;
            }

            if (!vscVIR_IsUniqueDefInstOfUsageInst(pDuInfo,
                                                   pUsageInst,
                                                   pUsage->usageKey.pOperand,
                                                   pUsage->usageKey.bIsIndexingRegUsage,
                                                   pDefInst,
                                                   gcvNULL))
            {
                bResult = gcvTRUE;
                return bResult;
            }
        }
    }

    return bResult;
}

static gctBOOL _VSC_CPP_NeedToFindNearestDefInst(
    IN OUT VSC_CPP_CopyPropagation  *cpp,
    IN     VIR_Instruction          *inst,
    IN     VIR_Operand              *srcOpnd,
    IN     VIR_Operand              *parentSrcOpnd,
    IN     gctUINT                  srcNum
    )
{
    VIR_OpCode                      opCode = VIR_Inst_GetOpcode(inst);
    VIR_Operand*                    pSrc0Opnd = VIR_Inst_GetSource(inst, 0);
    VIR_OperandInfo                 src0OpndInfo;
    VSC_HW_CONFIG*                  pHwCfg = VSC_CPP_GetHWCFG(cpp);

    /* So far we only need to check if the src0 of an image-related instruction is not a uniform. */
    if (pHwCfg->hwFeatureFlags.canSrc0OfImgLdStBeTemp || !VIR_OPCODE_isImgRelated(opCode))
    {
        return gcvFALSE;
    }
    if (srcNum != 0 || parentSrcOpnd != gcvNULL)
    {
        return gcvFALSE;
    }
    VIR_Operand_GetOperandInfo(inst, pSrc0Opnd, &src0OpndInfo);
    if (!src0OpndInfo.isVreg)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctBOOL _VSC_CPP_IsSrcAbsOrNegOnly(
    IN     VIR_Operand              *srcOpnd,
    IN OUT gctBOOL                  *isAbs,
    IN OUT gctBOOL                  *isNeg
    )
{
    gctBOOL                         bHasOtherModifier = gcvFALSE;
    gctBOOL                         bIsAbs = gcvFALSE, bIsNeg = gcvFALSE;

    if (VIR_Operand_GetModifier(srcOpnd) & (~(VIR_MOD_NEG | VIR_MOD_ABS)))
    {
        bHasOtherModifier = gcvTRUE;
    }
    if (VIR_Operand_GetModifier(srcOpnd) & VIR_MOD_NEG)
    {
        bIsNeg = gcvTRUE;
    }
    if (VIR_Operand_GetModifier(srcOpnd) & VIR_MOD_ABS)
    {
        bIsAbs = gcvTRUE;
    }

    if (isNeg)
    {
        *isNeg = bIsNeg;
    }
    if (isAbs)
    {
        *isAbs = bIsAbs;
    }

    return bHasOtherModifier;
}

static VSC_ErrCode _VSC_CPP_CopyFromMOVOnOperand(
    IN OUT VSC_CPP_CopyPropagation  *cpp,
    IN     VIR_Instruction          *inst,
    IN OUT VSC_HASH_TABLE           *visitSet,
    IN     VIR_Operand              *srcOpnd,
    IN     VIR_Operand              *parentSrcOpnd, /* For texldParm or parameter only. */
    IN     gctUINT                  srcNum
    )
{
    VSC_ErrCode                     errCode  = VSC_ERR_NONE;
    VIR_Shader                      *shader = VSC_CPP_GetShader(cpp);
    VIR_Function                    *func = VIR_Shader_GetCurrentFunction(shader);
    VSC_OPTN_CPPOptions             *options = VSC_CPP_GetOptions(cpp);
    VIR_OpCode                      instOpcode = VIR_Inst_GetOpcode(inst);
    VIR_Operand                     *instDst = VIR_Inst_GetDest(inst);
    VIR_Instruction                 *defInst = gcvNULL;
    gctBOOL                         srcOpndIsRelIndexing = (VIR_Operand_GetRelAddrMode(srcOpnd) != VIR_INDEXED_NONE);
    gctBOOL                         bCopyFromOutputParam = (VSC_CPP_GetFlag(cpp) & VSC_CPP_COPY_FROM_OUTPUT_PARAM);
    gctBOOL                         bHasUniqueDefInst = gcvFALSE;
    gctBOOL                         bUseUniqueNearestDef = gcvFALSE;
    gctBOOL                         bHandleModifier = (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetOPTS(options), VSC_OPTN_CPPOptions_HANDLE_MODIFIER));
    gctBOOL                         bIsUsageSrcNeg = gcvFALSE, bIsUsageSrcAbs = gcvFALSE;
    gctBOOL                         bMovSrcNegAbsOnly = gcvFALSE, bIsMovSrcNeg = gcvFALSE, bIsMovSrcAbs = gcvFALSE;
    gctBOOL                         bSetNeg = gcvFALSE, bSetAbs = gcvFALSE;

    do
    {
        VIR_Enable                  srcEnable = VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(srcOpnd));
        VIR_OperandInfo             srcInfo;

        VIR_Operand_GetOperandInfo(inst, srcOpnd, &srcInfo);
        if (srcInfo.isImmVal || srcInfo.isVecConst)
        {
            continue;
        }

        if (srcInfo.isOutputParm && VIR_Shader_GetLevel(cpp->shader) < VIR_SHLEVEL_Pre_Low)
        {
            /* do not copy output parameter to its use before Lowlevel, so the inliner
             * can find the out parameter and rename it */
            continue;
        }

        /* Skip any rounding mode right now. */
        if (VIR_Operand_GetRoundMode(srcOpnd))
        {
             continue;
        }

        /* Check if we neend to handle the modifier. */
        if (bHandleModifier)
        {
            /* Get the modifier information. */
            _VSC_CPP_IsSrcAbsOrNegOnly(srcOpnd, &bIsUsageSrcAbs, &bIsUsageSrcNeg);
        }
        else if (VIR_Operand_GetModifier(srcOpnd) != VIR_MOD_NONE)
        {
            continue;
        }

        /* Do not copy the temp256 pair in SCPP, otherwise it will break the assumption that the register pair wiil be contiguous. */
        if (VIR_Operand_isTemp256High(srcOpnd) || VIR_Operand_isTemp256Low(srcOpnd))
        {
             continue;
        }

        /* selectadd add an extra src4 to fake the partial write, do not copy
           propagate it away */
        if (instOpcode == VIR_OP_VX_SELECTADD &&
            srcNum == 4)
        {
            continue;
        }
        if (instOpcode == VIR_OP_SUBSAT && srcInfo.isUniform &&
            srcNum == 1)
        {
            continue;
        }

        /* this inst's srcOpnd only has one MOV def */
        bHasUniqueDefInst = vscVIR_IsUniqueDefInstOfUsageInst(VSC_CPP_GetDUInfo(cpp),
                                                              inst,
                                                              srcOpnd,
                                                              gcvFALSE,
                                                              gcvNULL,
                                                              &defInst);

        if (!bHasUniqueDefInst)
        {
            /* If there is no need to check the nearest defined instruction, just bail out. */
            if (!(VSC_CPP_GetFlag(cpp) & VSC_CPP_FIND_NEAREST_DEF_INST) ||
                defInst == gcvNULL ||
                !_VSC_CPP_NeedToFindNearestDefInst(cpp, inst, srcOpnd, parentSrcOpnd, srcNum))
            {
                continue;
            }

            /* Try to find the nearest defined instruction. */
            if (!vscVIR_FindUniqueNearestDefInst(VSC_CPP_GetDUInfo(cpp),
                                                 inst,
                                                 srcOpnd,
                                                 VIR_Inst_GetPrev(inst),
                                                 gcvNULL,
                                                 &defInst))
            {
                continue;
            }

            gcmASSERT(defInst != gcvNULL);
            bUseUniqueNearestDef = gcvTRUE;
        }

        if (!VIR_IS_IMPLICIT_DEF_INST(defInst) &&
            defInst != VIR_UNDEF_INST &&
            (VIR_Inst_GetOpcode(defInst) == VIR_OP_MOV || VIR_Inst_GetOpcode(defInst) == VIR_OP_COPY)
            )
        {
            VIR_Operand     *movDst = VIR_Inst_GetDest(defInst);
            VIR_Operand     *movSrc = VIR_Inst_GetSource(defInst, 0);
            VIR_OperandInfo  movSrcInfo;
            VIR_Operand      *newSrc;
            /*VIR_Symbol       *dstSym, *srcSym;*/

            VIR_Swizzle movSrcSwizzle = VIR_Operand_GetSwizzle(movSrc);
            VIR_Enable  movEnable = VIR_Operand_GetEnable(movDst);
            VIR_Swizzle instSrcSwizzle = VIR_Operand_GetSwizzle(srcOpnd);
            VIR_Enable  instEnable = instDst ? VIR_Operand_GetEnable(instDst) : VIR_ENABLE_XYZW;
            VIR_Swizzle newSwizzle = instSrcSwizzle;
            VIR_Swizzle channelMapping = VIR_SWIZZLE_X;

            VIR_Swizzle lastSwizzle = VIR_SWIZZLE_X;
            gctINT      lastChannel = -1;
            gctUINT8    channel;

            if (!VIR_Inst_isComponentwise(inst))
            {
                if ((instOpcode == VIR_OP_DP2) || (instOpcode == VIR_OP_NORM_DP2))
                {
                    instEnable = VIR_ENABLE_XY;
                }
                else if ((instOpcode == VIR_OP_DP3) || (instOpcode == VIR_OP_NORM_DP3))
                {
                    instEnable = VIR_ENABLE_XYZ;
                }
                else if ((instOpcode == VIR_OP_DP4) || (instOpcode == VIR_OP_NORM_DP4) || (instOpcode == VIR_OP_JMPC) || VIR_OPCODE_isAtomCmpxChg(instOpcode))
                {
                    instEnable = VIR_ENABLE_XYZW;
                }
                else if (instOpcode == VIR_OP_CROSS)
                {
                    instEnable = VIR_ENABLE_XYZ;
                }
                else
                {
                    instEnable = VIR_ENABLE_XYZW;
                }
            }

            gcmASSERT(instEnable != VIR_ENABLE_NONE);

            if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
            {
                VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                VIR_LOG(dumper, "\n[FW] instruction:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, inst);
                VIR_LOG_FLUSH(dumper);

                VIR_LOG(dumper, "[FW] def Instruction:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, defInst);
                VIR_LOG_FLUSH(dumper);
            }

            VIR_Operand_GetOperandInfo(defInst, movSrc, &movSrcInfo);

            /* Get the ABS/NEG status. */
            bMovSrcNegAbsOnly = _VSC_CPP_IsSrcAbsOrNegOnly(movSrc, &bIsMovSrcAbs, &bIsMovSrcNeg);

            /* Usage source ABS: we can ignore the modifier of the MOV source, just use the modifier of usage source. */
            if (bIsUsageSrcAbs)
            {
                bSetAbs = gcvTRUE;
                bSetNeg = bIsUsageSrcNeg;
            }
            /* Usage source Neg: Use the ABS of the MOV source, and inverse the NEG of the MOV source. */
            else if (bIsUsageSrcNeg)
            {
                bSetAbs = bIsMovSrcAbs;
                bSetNeg = !bIsMovSrcNeg;
            }
            /* Usage none: just use the modifier of the MOV source. */
            else
            {
                bSetAbs = bIsMovSrcAbs;
                bSetNeg = bIsMovSrcNeg;
            }

            if (movSrcInfo.isOutputParm && VIR_Shader_GetLevel(cpp->shader) < VIR_SHLEVEL_Pre_Low)
            {
                /* do not copy output parameter to its use before Lowlevel, so the inliner
                 * can find the out parameter and rename it */
                if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                {
                    VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                    VIR_LOG(dumper, "[FW] ==> bail out: output parameter not copied before lowlevel");
                    VIR_LOG_FLUSH(dumper);
                }
                break;
            }

            if (VIR_Operand_GetRelAddrMode(movSrc) != VIR_INDEXED_NONE)
            {
                VIR_Instruction* pUsageInst = gcvNULL;
                VIR_Operand *pUsageOper = gcvNULL;
                gctBOOL bIsIndexingRegUsage;
                /* if movSrc and srcOpnd are indexing access, skip this case */
                if (srcOpndIsRelIndexing)
                {
                    /* do not copy src with relAddr to its use */
                    if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                    {
                        VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                        VIR_LOG(dumper, "[FW] ==> bail out: movSrc and being replaced src are relindexing access");
                        VIR_LOG_FLUSH(dumper);
                    }
                    break;
                }

                /* if movDst has 1+ usage, no benefit to copy propagation since more mova/ldarr are generated */
                if (!vscVIR_DoesDefInstHaveUniqueUsageInst(VSC_CPP_GetDUInfo(cpp),
                                                           defInst, gcvTRUE,
                                                           &pUsageInst, &pUsageOper, &bIsIndexingRegUsage))
                {
                    if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                    {
                        VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                        VIR_LOG(dumper, "[FW] ==> bail out: movSrc is indexing access and dest has 1+ usage no good for propagation");
                        VIR_LOG_FLUSH(dumper);
                    }
                    break;
                }
            }
            /* mov src is constant. do very limited constant propagation */
            if (movSrcInfo.isImmVal || movSrcInfo.isVecConst)
            {
                if (VIR_OPCODE_isVX(instOpcode))
                {
                    gctINT evisSrcNo = VIR_OPCODE_EVISModifier_SrcNo(instOpcode);
                    gcmASSERT(evisSrcNo >= 0 && (gctUINT)evisSrcNo < VIR_Inst_GetSrcNum(inst));

                    /* VX instruction's operand can not be immediate number */
                    if (evisSrcNo != (gctINT)srcNum)
                    {
                        /* define and user must in same function */
                        if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                        {
                            VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                            VIR_LOG(dumper, "[FW] ==> bail out: definition and user are not in same function");
                            VIR_LOG_FLUSH(dumper);
                        }
                        break;
                    }
                }

                /* update the DU, remove the usage of srcOpnd */
                vscVIR_DeleteUsage(VSC_CPP_GetDUInfo(cpp),
                                   defInst,
                                   inst,
                                   srcOpnd,
                                   gcvFALSE,
                                   srcInfo.u1.virRegInfo.virReg,
                                   1,
                                   srcEnable,
                                   VIR_HALF_CHANNEL_MASK_FULL,
                                   gcvNULL);

                /* duplicate movSrc */
                VIR_Function_DupOperand(func, movSrc, &newSrc);

                /* Use the component type for a immediate. */
                if (VIR_Operand_isImm(newSrc))
                {
                    VIR_Operand_SetTypeId(newSrc, VIR_GetTypeComponentType(VIR_Operand_GetTypeId(srcOpnd)));
                }
                else
                {
                    VIR_Operand_SetTypeId(newSrc, VIR_Operand_GetTypeId(srcOpnd));
                }

                VIR_Operand_SetLShift(newSrc, VIR_Operand_GetLShift(srcOpnd));
                VIR_Operand_SetModifier(newSrc, VIR_Operand_GetModifier(srcOpnd));

                /* Update the modifier. */
                if (bSetAbs)
                {
                    VIR_Operand_SetOneModifier(newSrc, VIR_MOD_ABS);
                }
                else
                {
                    VIR_Operand_ClrOneModifier(newSrc, VIR_MOD_ABS);
                }

                VIR_Operand_ClrOneModifier(newSrc, VIR_MOD_NEG);
                if (bSetNeg)
                {
                    VIR_Operand_NegateOperand(shader, newSrc);
                }

                /* we need to map swizzle for a vector constant. */
                if (movSrcInfo.isVecConst)
                {
                    for (channel = 0; channel < VIR_CHANNEL_NUM; channel++ )
                    {
                        if (movEnable & (1 << channel))
                        {
                            VIR_Swizzle_SetChannel(channelMapping, channel,
                                VIR_Swizzle_GetChannel(movSrcSwizzle, channel));
                        }
                    }
                    /*
                    ** We must also update the used components because after Peephole,
                    ** the enable count of dest may change.
                    */
                    for (channel = 0; channel < VIR_CHANNEL_NUM; channel++ )
                    {
                        if (instEnable & (1 << channel))
                        {
                            lastSwizzle = VIR_Swizzle_GetChannel(channelMapping,
                                    VIR_Swizzle_GetChannel(instSrcSwizzle, channel));

                            VIR_Swizzle_SetChannel(newSwizzle, channel, lastSwizzle);

                            /* It is the first enable component. Set the prev-unenable components. */
                            if (lastChannel == -1)
                            {
                                gctUINT8 i;
                                for (i = 0; i < channel; i++)
                                {
                                    VIR_Swizzle_SetChannel(newSwizzle, i, lastSwizzle);
                                }
                            }
                            lastChannel = (gctINT8)channel;
                        }
                        /* Use the swizzle of last enable component. */
                        else if (lastChannel != -1)
                        {
                            VIR_Swizzle_SetChannel(newSwizzle, channel, lastSwizzle);
                        }
                    }
                    VIR_Operand_SetSwizzle(newSrc, newSwizzle);

                    /* If this is a simple swizzle, we can just change it to a immediate operand. */
                    if (VIR_Swizzle_Channel_Count(newSwizzle) == 1)
                    {
                        VIR_Const*  pConstValue;
                        pConstValue = (VIR_Const*)VIR_GetSymFromId(&shader->constTable, VIR_Operand_GetConstId(newSrc));

                        VIR_Operand_SetOpKind(newSrc, VIR_OPND_IMMEDIATE);
                        VIR_Operand_SetImmUint(newSrc, pConstValue->value.vecVal.u32Value[VIR_Swizzle_GetChannel(newSwizzle, 0)]);
                        VIR_Operand_SetTypeId(newSrc, VIR_GetTypeComponentType(VIR_Operand_GetTypeId(newSrc)));
                    }
                }
                else
                {
                    gcmASSERT(movSrcInfo.isImmVal);
                    VIR_Operand_SetSwizzle(newSrc, VIR_SWIZZLE_XXXX);
                    VIR_Operand_SetTypeId(newSrc, VIR_GetTypeComponentType(VIR_Operand_GetTypeId(newSrc)));
                }

                /* Replace the source. */
                _VSC_CPP_ReplaceSource(cpp, inst, parentSrcOpnd, srcNum, newSrc);

                /* change the immediate to EvisModifier for EVIS inst if it is modifier operand */
                if (VIR_OPCODE_isVX(instOpcode))
                {
                    gctINT evisSrcNo = VIR_OPCODE_EVISModifier_SrcNo(instOpcode);
                    gcmASSERT(evisSrcNo >= 0 && (gctUINT)evisSrcNo < VIR_Inst_GetSrcNum(inst));

                    if (evisSrcNo == (gctINT)srcNum)
                    {
                        /* set newSrc to EVISModifier operand */
                        VIR_Operand_SetOpKind(newSrc, VIR_OPND_EVIS_MODIFIER);
                    }
                }

                _VSC_CPP_RemoveDefInst(cpp, defInst);
            }
            else
            {
                /* In the IR, there exists implict type conversion, thus we need to bail out if the types mismatch */
                VIR_TypeId ty0 = VIR_Operand_GetTypeId(movDst);
                VIR_TypeId ty1 = VIR_Operand_GetTypeId(srcOpnd);

                if(!VIR_TypeId_isPrimitive(ty0)) {
                    ty0 = VIR_Type_GetBaseTypeId(VIR_Shader_GetTypeFromId(shader, ty0));
                }
                if(!VIR_TypeId_isPrimitive(ty1)) {
                    ty1 = VIR_Type_GetBaseTypeId(VIR_Shader_GetTypeFromId(shader, ty1));
                }
                gcmASSERT(ty0 < VIR_TYPE_PRIMITIVETYPE_COUNT &&
                    ty1 < VIR_TYPE_PRIMITIVETYPE_COUNT);

                if (VIR_Inst_GetFunction(inst) != VIR_Inst_GetFunction(defInst))
                {
                    /* define and user must in same function */
                    if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                    {
                        VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                        VIR_LOG(dumper, "[FW] ==> bail out: definition and user are not in same function");
                        VIR_LOG_FLUSH(dumper);
                    }
                    break;
                }

                if (VIR_Shader_isDual16Mode(shader) &&
                    VIR_Operand_GetPrecision(movDst) != VIR_Operand_GetPrecision(movSrc))
                {
                    if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                    {
                        VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                        VIR_LOG(dumper, "[FW] ==> bail out: because of different precision in dual16 mode");
                        VIR_LOG_FLUSH(dumper);
                    }
                    break;
                }

                if (VIR_Inst_GetOpcode(defInst) == VIR_OP_COPY)
                {
                    if (!VIR_Symbol_isImage(VIR_Operand_GetSymbol(defInst->src[0])) &&
                        !VIR_Symbol_isImageT(VIR_Operand_GetSymbol(defInst->src[0])) &&
                        !VIR_Symbol_isUniform(VIR_Operand_GetSymbol(defInst->src[0])) &&
                        !VIR_Symbol_isSampler(VIR_Operand_GetSymbol(defInst->src[0])))
                    {
                        /* mov's dest is per patch output */
                        if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                        {
                            VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                            VIR_LOG(dumper, "[FW] ==> bail out: copy's src is not uniform");
                            VIR_LOG_FLUSH(dumper);
                        }
                        break;
                    }
                }

                if (VIR_Operand_IsPerPatch(defInst->dest))
                {
                    /* mov's dest is per patch output */
                    if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                    {
                        VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                        VIR_LOG(dumper, "[FW] ==> bail out: mov's dest is per patch output");
                        VIR_LOG_FLUSH(dumper);
                    }
                    break;
                }

                if (!(
                      ((VIR_GetTypeFlag(ty0) & VIR_TYFLAG_ISFLOAT) && (VIR_GetTypeFlag(ty1) & VIR_TYFLAG_ISFLOAT))
                      ||
                      (((VIR_GetTypeFlag(ty0) & VIR_TYFLAG_ISINTEGER) || (VIR_GetTypeTypeKind(ty0) == VIR_TY_IMAGE))
                       &&
                       ((VIR_GetTypeFlag(ty1) & VIR_TYFLAG_ISINTEGER) || (VIR_GetTypeTypeKind(ty1) == VIR_TY_IMAGE)))
                      ||
                      (VIR_GetTypeTypeKind(ty1) == VIR_TY_SAMPLER && (VIR_GetTypeFlag(ty0) & VIR_TYFLAG_ISINTEGER))
                      ||
                      ((ty0 == ty1) && (VIR_GetTypeFlag(ty0) & VIR_TYFLAG_IS_SAMPLER))
                     ))
                {
                    if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                    {
                        VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                        VIR_LOG(dumper, "[FW] ==> bail out because of not same type");
                        VIR_LOG_FLUSH(dumper);
                    }
                    break;
                }

                /* if UsageSrc or movSrc has modifier, check the type and opcode
                 */
                if (bHandleModifier)
                {
                    /* Check the source's modifier, so far we can only support NEG and ABS. */
                    if (bMovSrcNegAbsOnly)
                    {
                        if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                        {
                            VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                            VIR_LOG(dumper, "[FW] ==> bail out because of Neg/Abs Modifier Only");
                            VIR_LOG_FLUSH(dumper);
                        }
                        break;
                    }

                    if (bIsUsageSrcNeg || bIsUsageSrcAbs || bIsMovSrcAbs || bIsMovSrcNeg)
                    {
                        if (ty0 != ty1)
                        {
                            if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                            {
                                VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                                VIR_LOG(dumper, "[FW] ==> bail out because of movSrc and UsageSrc not same type");
                                VIR_LOG_FLUSH(dumper);
                            }
                            break;
                        }
                    }
                }

                if (VIR_Operand_GetOpKind(movDst) != VIR_OPND_SYMBOL || VIR_Operand_GetOpKind(movSrc) != VIR_OPND_SYMBOL)
                {
                    if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                    {
                        VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                        VIR_LOG(dumper, "[FW] ==> bail out because of not symbol");
                        VIR_LOG_FLUSH(dumper);
                    }
                    break;
                }

                /* If COPY_FROM_OUTPUT_PARAM is disabled, we need to check if this source is a output parameter. */
                if (!bCopyFromOutputParam && movSrcInfo.isVreg && VIR_Operand_isSymbol(movSrc))
                {
                    VIR_Symbol*     pSrcSym = VIR_Operand_GetSymbol(movSrc);

                    if (VIR_Symbol_isVreg(pSrcSym))
                    {
                        if (VIR_Symbol_isOutParamVirReg(pSrcSym) || VIR_Symbol_isOutParam(VIR_Symbol_GetVregVariable(pSrcSym)))
                        {
                            break;
                        }
                    }
                    else if (VIR_Symbol_isOutParam(pSrcSym))
                    {
                        break;
                    }
                }

                if (VIR_Operand_GetModifier(movDst) || VIR_Operand_GetRoundMode(movDst) ||
                    VIR_Operand_GetRoundMode(movSrc))
                {
                    if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                    {
                        VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                        VIR_LOG(dumper, "[FW] ==> bail out because of modifier");
                        VIR_LOG_FLUSH(dumper);
                    }
                    break;
                }

                /* Check if we neend to handle the modifier. */
                if (!bHandleModifier && VIR_Operand_GetModifier(movSrc) != VIR_MOD_NONE)
                {
                    continue;
                }

                /*
                ** If any usage instruction of this MOV has more than one DEF instruction, which means we can't remove this MOV,
                ** we don't generate this new copy because it may increase the live range of SOURCE0.
                */
                if (!bUseUniqueNearestDef &&
                    _VSC_CPP_AnyOtherUsageCanNotBeOptimize(VSC_CPP_GetDUInfo(cpp),
                                                           inst,
                                                           defInst,
                                                           movEnable,
                                                           srcInfo.u1.virRegInfo.virReg))
                {
                    if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                    {
                        VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                        VIR_LOG(dumper, "[FW] ==> bail out because one use could not be replaced\n");
                        VIR_LOG_FLUSH(dumper);
                    }
                    break;
                }

                /* copy to LDARR with attribute is not benefial at all. For example,
                    MOV t1.x, att1.x
                    LDARR t2, base, t1.x
                    ADD  t3, t2, t4
                    ==> (after CPP)
                    LDARR t2, base, att1.x
                    ADD t3, t2, t4
                    ==> (after converter)
                    MOV t5.x, att1.x
                    LDARR t2, base, t5.x
                    ADD t3, t2, t4

                    There is no gain in the above case. While, if more components are used in
                    one MOV and shared by more LDARRs, there will be performance loss.

                    MOV t1.xy, att1.xy
                    LDARR t2, base, t1.x
                    ADD  t3, t2, t4
                    LDARR t5, base, t1.y
                    ADD  t6, t5, t4
                    ==> (after CPP)
                    LDARR t2, base, att1.x
                    ADD  t3, t2, t4
                    LDARR t5, base, att1.y
                    ADD  t6, t5, t4
                    ==> (after converter)
                    MOV  t7.x, att1.x
                    LDARR t2, base, t7.x
                    ADD  t3, t2, t4
                    MOV t8.x, att1.y
                    LDARR t5, base, t8.x
                    ADD  t6, t5, t4

                    Thus disable such case.
                */
                if (movSrcInfo.isInput && VIR_Inst_GetOpcode(inst) == VIR_OP_LDARR)
                {
                    if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                    {
                        VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                        VIR_LOG(dumper, "[FW] ==> bail out because movSrc is input");
                        VIR_LOG_FLUSH(dumper);
                    }
                    break;
                }

                /* old CG, in base[index], index has to be temp */
                if (VIR_Inst_GetOpcode(inst) == VIR_OP_LDARR &&
                    srcNum == 1 &&
                    !VIR_Symbol_isVreg(VIR_Operand_GetSymbol(movSrc))
                    )
                {
                    if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                    {
                        VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                        VIR_LOG(dumper, "[FW] ==> bail out because LDARR index has to be temp ");
                        VIR_LOG_FLUSH(dumper);
                    }
                    break;
                }

                /* inst and its def in the same bb */
                if (VIR_Inst_GetBasicBlock(inst) == VIR_Inst_GetBasicBlock(defInst))
                {
                    /* no redefine of movSrc between defInst and inst */
                    VIR_Instruction* next = VIR_Inst_GetNext(defInst);
                    gctBOOL         invalidCase = gcvFALSE;
                    while (next != inst)
                    {
                        if (VIR_Operand_SameLocation(defInst, movSrc, next, VIR_Inst_GetDest(next)) ||
                            VIR_Inst_GetOpcode(next) == VIR_OP_CALL)
                        {
                            if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                            {
                                VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                                VIR_LOG(dumper, "[FW] ==> bail out because of redefine or call\n");
                                VIR_LOG_FLUSH(dumper);
                                VIR_Inst_Dump(dumper, next);
                                VIR_LOG_FLUSH(dumper);
                            }
                            invalidCase = gcvTRUE;
                            break;
                        }
                        next = VIR_Inst_GetNext(next);
                    }

                    if (invalidCase)
                    {
                        break;

                    }

                }
                else
                {
                    if (!VSC_CPP_isGlobalCPP(cpp) &&
                        !(VIR_Symbol_isUniform(VIR_Operand_GetSymbol(movSrc)) ||
                          VIR_Symbol_isSampler(VIR_Operand_GetSymbol(movSrc)) ||
                          VIR_Symbol_isImage(VIR_Operand_GetSymbol(movSrc)) ||
                          VIR_Symbol_isImageT(VIR_Operand_GetSymbol(movSrc))))
                    {
                        if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                        {
                            VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                            VIR_LOG(dumper, "[FW] ==> bail out because of not same BB\n");
                            VIR_LOG_FLUSH(dumper);
                        }
                        break;
                    }

                    else
                    {
                        /* no call between defInst and inst */
                        {
                            if (!(movSrcInfo.isInput ||
                                  VIR_Symbol_isUniform(VIR_Operand_GetSymbol(movSrc)) ||
                                  VIR_Symbol_isSampler(VIR_Operand_GetSymbol(movSrc)) ||
                                  VIR_Symbol_isImage(VIR_Operand_GetSymbol(movSrc)) ||
                                  VIR_Symbol_isImageT(VIR_Operand_GetSymbol(movSrc)))
                                 &&
                                _VSC_CPP_CallInstInBetween(defInst, inst, visitSet))
                            {
                                if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                                {
                                    VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                                    VIR_LOG(dumper, "[FW] ==> bail out because of call\n");
                                    VIR_LOG_FLUSH(dumper);
                                }
                                vscHTBL_Reset(visitSet);
                                break;
                            }
                            vscHTBL_Reset(visitSet);
                        }
                        {
                            /* no redefine of movSrc between defInst and inst */
                            VIR_Instruction *redefInst = gcvNULL;
                            if (!(VIR_Symbol_isUniform(VIR_Operand_GetSymbol(movSrc)) ||
                                  VIR_Symbol_isSampler(VIR_Operand_GetSymbol(movSrc)) ||
                                  VIR_Symbol_isImage(VIR_Operand_GetSymbol(movSrc)) ||
                                  VIR_Symbol_isImageT(VIR_Operand_GetSymbol(movSrc)))
                                &&
                                vscVIR_RedefineBetweenInsts(&cpp->checkRedefinedResInfo, VSC_CPP_GetDUInfo(cpp),
                                    defInst, inst, movSrc, &redefInst))
                            {
                                if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                                {
                                    VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                                    VIR_LOG(dumper, "[FW] ==> bail out because of redefine\n");
                                    VIR_LOG_FLUSH(dumper);
                                    VIR_Inst_Dump(dumper, redefInst);
                                    VIR_LOG_FLUSH(dumper);
                                }
                                break;
                            }
                        }
                    }
                }

                for (channel = 0; channel < VIR_CHANNEL_NUM; channel++ )
                {
                    if (movEnable & (1 << channel))
                    {
                        VIR_Swizzle_SetChannel(channelMapping, channel, VIR_Swizzle_GetChannel(movSrcSwizzle, channel));

                    }
                }

                /*
                ** We must also update the used components because after Peephole,
                ** the enable count of dest may change.
                */
                for (channel = 0; channel < VIR_CHANNEL_NUM; channel++ )
                {
                    if (instEnable & (1 << channel))
                    {
                        lastSwizzle = VIR_Swizzle_GetChannel(channelMapping, VIR_Swizzle_GetChannel(instSrcSwizzle, channel));
 
                        VIR_Swizzle_SetChannel(newSwizzle, channel, lastSwizzle);

                        /* It is the first enable component. Set the prev-unenable components. */
                        if (lastChannel == -1)
                        {
                            gctUINT8 i;

                            for (i = 0; i < channel; i++)
                            {
                                VIR_Swizzle_SetChannel(newSwizzle, i, lastSwizzle);
                            }
                        }
                        lastChannel = (gctINT8)channel;
                    }

                    /* Use the swizzle of last enable component. */
                    else if (lastChannel != -1)
                    {
                        VIR_Swizzle_SetChannel(newSwizzle, channel, lastSwizzle);
                    }
                }

                if (VIR_OPCODE_isVX(VIR_Inst_GetOpcode(inst)) &&
                    !VIR_OPCODE_isImgRelated(VIR_Inst_GetOpcode(inst)) &&
                    newSwizzle != VIR_SWIZZLE_XXXX &&
                    newSwizzle != VIR_SWIZZLE_XYYY &&
                    newSwizzle != VIR_SWIZZLE_XYZZ &&
                    newSwizzle != VIR_SWIZZLE_XYZW    )
                {
                    /* the EVIS inst src0's swizzle bits are used for EVIS info like startBin,
                        * make sure the src0 operand does NOT use them
                        * VX_IMG_xxx instruction has no this restriction
                        */
                    if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                    {
                        VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                        VIR_LOG(dumper, "[FW] ==> bail out because of VX instruction cannot swizzle source\n");
                        VIR_LOG_FLUSH(dumper);
                    }
                    break;
                }

                if (VIR_Inst_GetThreadMode(defInst) == VIR_THREAD_D16_DUAL_32 &&
                    VIR_Operand_GetPrecision(movSrc) == VIR_PRECISION_HIGH)
                {
                    VIR_Inst_SetThreadMode(inst, VIR_THREAD_D16_DUAL_32);
                }

                vscVIR_DeleteUsage(VSC_CPP_GetDUInfo(cpp),
                                   VIR_ANY_DEF_INST,
                                   inst,
                                   srcOpnd,
                                   gcvFALSE,
                                   srcInfo.u1.virRegInfo.virReg,
                                   1,
                                   srcEnable,
                                   VIR_HALF_CHANNEL_MASK_FULL,
                                   gcvNULL);

                /* duplicate movSrc */
                {
                    VIR_TypeId ty = VIR_Operand_GetTypeId(srcOpnd);

                    VIR_Function_DupOperand(func, movSrc, &newSrc);
                    VIR_Operand_SetSwizzle(newSrc, newSwizzle);
                    VIR_Operand_SetLShift(newSrc, VIR_Operand_GetLShift(srcOpnd));

                    /* Set the modifier if needed. */
                    if (bHandleModifier)
                    {
                        VIR_Operand_SetModifier(newSrc, VIR_Operand_GetModifier(srcOpnd));
                        if (bSetAbs)
                        {
                            VIR_Operand_SetOneModifier(newSrc, VIR_MOD_ABS);
                        }
                        else
                        {
                            VIR_Operand_ClrOneModifier(newSrc, VIR_MOD_ABS);
                        }

                        VIR_Operand_ClrOneModifier(newSrc, VIR_MOD_NEG);
                        if (bSetNeg)
                        {
                            VIR_Operand_NegateOperand(shader, newSrc);
                        }
                    }

                    /* Set the type id. */
                    if (!_VSC_CPP_CopySrcTypeFromMov(cpp, inst, defInst, newSrc))
                    {
                        VIR_Operand_SetTypeId(newSrc, ty);
                    }

                    /* copy reladdr info to newSrc if needed */
                    if (VIR_Operand_GetRelAddrMode(srcOpnd))
                    {
                        VIR_Operand_SetRelAddrMode(newSrc, VIR_Operand_GetRelAddrMode(srcOpnd));
                        VIR_Operand_SetRelIndexing(newSrc, VIR_Operand_GetRelIndexing(srcOpnd));
                    }
                    else if (VIR_Operand_GetRelAddrMode(movSrc))
                    {
                        VIR_Operand_SetRelAddrMode(newSrc, VIR_Operand_GetRelAddrMode(movSrc));
                        VIR_Operand_SetRelIndexing(newSrc, VIR_Operand_GetRelIndexing(movSrc));
                    }
                    /* Replace the source. */
                    _VSC_CPP_ReplaceSource(cpp, inst, parentSrcOpnd, srcNum, newSrc);

                    srcOpnd = gcvNULL;    /* reset srcOpnd to avoid misuse */
                }

                /* update the du info */
                {
                    VIR_GENERAL_UD_ITERATOR udIter;
                    VIR_DEF* pDef;
                    vscVIR_InitGeneralUdIterator(&udIter,
                        VSC_CPP_GetDUInfo(cpp), defInst, movSrc, gcvFALSE, gcvFALSE);

                    for(pDef = vscVIR_GeneralUdIterator_First(&udIter);
                        pDef != gcvNULL;
                        pDef = vscVIR_GeneralUdIterator_Next(&udIter))
                    {
                        if (VIR_Swizzle_2_Enable(newSwizzle) & (1 << pDef->defKey.channel))
                        {
                            vscVIR_AddNewUsageToDef(VSC_CPP_GetDUInfo(cpp),
                                                    pDef->defKey.pDefInst,
                                                    inst,
                                                    newSrc,
                                                    gcvFALSE,
                                                    movSrcInfo.u1.virRegInfo.virReg,
                                                    1,
                                                    (1 << pDef->defKey.channel),
                                                    VIR_HALF_CHANNEL_MASK_FULL,
                                                    gcvNULL);
                        }
                    }
                }

                _VSC_CPP_RemoveDefInst(cpp, defInst);
            }

            if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
            {
                VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                VIR_LOG(dumper, "[FW] ==> change to:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, inst);
                VIR_LOG_FLUSH(dumper);
            }

            VSC_CPP_SetFWOptCount(cpp, VSC_CPP_GetFWOptCount(cpp) + 1);

            /* since defInst is the unique def of srcOpnd, thus break out*/
            break;
        }
    } while (gcvFALSE);

    return errCode;
}

/* Forward propagation
       MOV t1.x t2.y
       ADD dst.x, t1.x, t3.x
       ==>
       ADD dst.x, t2.y, t3.x
*/
static VSC_ErrCode _VSC_CPP_CopyFromMOV(
    IN OUT VSC_CPP_CopyPropagation  *cpp,
    IN     VIR_Instruction          *inst,
    IN OUT VSC_HASH_TABLE           *visitSet
    )
{
    VSC_ErrCode     errCode  = VSC_ERR_NONE;
    VIR_Operand     *instDst = VIR_Inst_GetDest(inst);
    VIR_OpCode      opCode = VIR_Inst_GetOpcode(inst);
    gctUINT         srcNum;

    if (instDst == gcvNULL)
    {
        return errCode;
    }

    srcNum = VIR_Inst_GetSrcNum(inst);
    while (srcNum-- && (errCode ==  VSC_ERR_NONE))
    {
        VIR_Operand         *srcOpnd = VIR_Inst_GetSource(inst, srcNum);

        /* Enable for INTRINSIC only now. */
        if (VIR_Operand_isParameters(srcOpnd) && opCode == VIR_OP_INTRINSIC)
        {
            VIR_ParmPassing *parm = VIR_Operand_GetParameters(srcOpnd);
            gctUINT         i;

            for (i = 0; i < parm->argNum; i++)
            {
                if (parm->args[i])
                {
                    errCode = _VSC_CPP_CopyFromMOVOnOperand(cpp, inst, visitSet, parm->args[i], srcOpnd, i);
                    ON_ERROR(errCode, "copy from MOV for a single operand");
                }
            }
        }
        else
        {
            errCode = _VSC_CPP_CopyFromMOVOnOperand(cpp, inst, visitSet, srcOpnd, gcvNULL, srcNum);
            ON_ERROR(errCode, "copy from MOV for a single operand");
        }
    }

OnError:
    return errCode;
}

/* Backward Propagation
   ADD t1.x  t2.x t3.x
   MOV t4.x t1.x

   ==>
   ADD t4.x t2.x t3.x
*/

static VSC_ErrCode _VSC_CPP_CopyToMOV(
    IN OUT VSC_CPP_CopyPropagation  *cpp,
    IN     VIR_Instruction          *inst,
    IN     VSC_HASH_TABLE           *inst_def_set,
    IN     VSC_HASH_TABLE           *inst_usage_set
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Shader          *shader = VSC_CPP_GetShader(cpp);
    VIR_Function        *func   = VIR_Shader_GetCurrentFunction(shader);
    VSC_OPTN_CPPOptions *options = VSC_CPP_GetOptions(cpp);

    VIR_Operand     *inst_dest          = gcvNULL;
    VIR_Enable      inst_dest_enable;
    VIR_Operand     *inst_src0          = gcvNULL;
    VIR_Swizzle     inst_src0_swizzle, movInst_remapSwizzle;
    VIR_Enable      inst_src0_enable    = VIR_ENABLE_NONE;
    VIR_OperandInfo inst_dest_info, inst_src0_info;

    gctBOOL          invalid_case       = gcvFALSE;
    gctBOOL          hasUse             = gcvFALSE;

    VIR_GENERAL_UD_ITERATOR ud_iter;
    VIR_GENERAL_DU_ITERATOR du_iter;

    VIR_DEF         *def                = gcvNULL;
    VIR_Instruction *def_inst           = gcvNULL;
    VIR_Operand     *def_inst_dest      = gcvNULL;
    VIR_Enable       def_inst_enable    = VIR_ENABLE_NONE;
    gctUINT8         channel            = 0;
    VIR_TypeId       ty0, ty1;
    VIR_NATIVE_DEF_FLAGS      nativeDefFlags;

    gctBOOL covered_channels[VIR_CHANNEL_NUM] = {gcvFALSE, gcvFALSE, gcvFALSE, gcvFALSE};
    gctBOOL movInstRemappable, needRemapChannel = gcvFALSE, def2UsageRemappable;

    gcmASSERT(VIR_Inst_GetOpcode(inst) == VIR_OP_MOV || VIR_Inst_GetOpcode(inst) == VIR_OP_SAT);

    inst_src0 = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_GetOperandInfo(inst, inst_src0, &inst_src0_info);

    inst_dest = VIR_Inst_GetDest(inst);
    VIR_Operand_GetOperandInfo(inst, inst_dest, &inst_dest_info);

    if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                      VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
    {
        VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
        VIR_LOG(dumper, "\n[BW] instruction:\n");
        VIR_LOG_FLUSH(dumper);
        VIR_Inst_Dump(dumper, inst);
        VIR_LOG_FLUSH(dumper);
    }

    /* inst_dest should not have modifier or round */
    if(VIR_Operand_GetModifier(inst_dest) || VIR_Operand_GetRoundMode(inst_dest))
    {
        if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                          VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
        {
            VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
            VIR_LOG(dumper, "[BW] ==> bail out because its dest has modifier or round.\n");
            VIR_LOG_FLUSH(dumper);
        }
        return errCode;
    }

    /* inst_src0 should not have modifier or round */
    if(VIR_Operand_GetModifier(inst_src0) || VIR_Operand_GetRoundMode(inst_src0))
    {
        if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                          VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
        {
            VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
            VIR_LOG(dumper, "[BW] ==> bail out because its src0 has modifier or round.\n");
            VIR_LOG_FLUSH(dumper);
        }
        return errCode;
    }

    /* disable cpp for per patch dest */
    if(VIR_Operand_IsPerPatch(inst->dest))
    {
        if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                          VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
        {
            VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
            VIR_LOG(dumper, "[BW] ==> bail out because its dest is per patch output.\n");
            VIR_LOG_FLUSH(dumper);
        }
        return errCode;
    }

    if(inst_src0_info.isImmVal || inst_src0_info.isVecConst)
    {
        if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                          VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
        {
            VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
            VIR_LOG(dumper, "[BW] ==> bail out because its src0 is a constant.\n");
            VIR_LOG_FLUSH(dumper);
        }
        return errCode;
    }

    /* inst_src0 should not be an input or output. */
    if(inst_src0_info.isInput || inst_src0_info.isOutput)
    {
        if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                          VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
        {
            VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
            VIR_LOG(dumper, "[BW] ==> bail out because its src0 is an input or output.\n");
            VIR_LOG_FLUSH(dumper);
        }
        return errCode;
    }

    ty0 = VIR_Operand_GetTypeId(inst_src0);
    ty1 = VIR_Operand_GetTypeId(inst_dest);

    /* must be primitive type */
    if (!VIR_Type_isPrimitive(VIR_Shader_GetTypeFromId(shader, ty0)) ||
        !VIR_Type_isPrimitive(VIR_Shader_GetTypeFromId(shader, ty1)))
    {
        if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                          VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
        {
            VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
            VIR_LOG(dumper, "[BW] ==> bail out because of non-primitive types.\n");
            VIR_LOG_FLUSH(dumper);
        }
        return errCode;
    }

    gcmASSERT(ty0 < VIR_TYPE_PRIMITIVETYPE_COUNT &&
        ty1 < VIR_TYPE_PRIMITIVETYPE_COUNT);

    /* Their types should be same. This is for avoiding some bugs from other place. */
    if(!(((VIR_GetTypeFlag(ty0) & VIR_TYFLAG_ISFLOAT) &&
        (VIR_GetTypeFlag(ty1) & VIR_TYFLAG_ISFLOAT)) ||
        ((VIR_GetTypeFlag(ty0) & VIR_TYFLAG_ISINTEGER) &&
        (VIR_GetTypeFlag(ty1) & VIR_TYFLAG_ISINTEGER)) ||
        ((ty0 == ty1) && (VIR_GetTypeFlag(ty0) & VIR_TYFLAG_IS_SAMPLER))))
    {
        if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                          VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
        {
            VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
            VIR_LOG(dumper, "[BW] ==> bail out because they have different types.\n");
            VIR_LOG_FLUSH(dumper);
        }
        return errCode;
    }

    inst_src0_swizzle = VIR_Operand_GetSwizzle(inst_src0);
    inst_src0_enable = VIR_Swizzle_2_Enable(inst_src0_swizzle);

    inst_dest_enable = VIR_Operand_GetEnable(inst_dest);

    for(channel = 0; channel < VIR_CHANNEL_COUNT; ++channel)
    {
        if(inst_dest_enable & (1 << channel))
        {
            vscVIR_InitGeneralDuIterator(&du_iter, VSC_CPP_GetDUInfo(cpp), inst, inst_dest_info.u1.virRegInfo.virReg, channel, gcvFALSE);
            if (vscVIR_GeneralDuIterator_First(&du_iter) != gcvNULL)
            {
                hasUse = gcvTRUE;
                break;
            }
        }
    }

    if (!hasUse)
    {
        if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                          VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
        {
            VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
            VIR_LOG(dumper, "[BW] ==> bail out because its dst0 dose not have any use.\n");
            VIR_LOG_FLUSH(dumper);
        }
        return errCode;
    }

    vscVIR_InitGeneralUdIterator(&ud_iter, VSC_CPP_GetDUInfo(cpp), inst, inst_src0, gcvFALSE, gcvFALSE);
    if((def = vscVIR_GeneralUdIterator_First(&ud_iter)) == gcvNULL)
    {
        if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                          VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
        {
            VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
            VIR_LOG(dumper, "[BW] ==> bail out because its src0 dose not have any def.\n");
            VIR_LOG_FLUSH(dumper);
        }
        return errCode;
    }
    /* mov Inst channel is one-one mapping s*/
    movInstRemappable = VIR_Swizzle_GetMappingSwizzle2Enable(inst_src0_swizzle, inst_dest_enable, &movInst_remapSwizzle);

    for(; def != gcvNULL && (!invalid_case);  def = vscVIR_GeneralUdIterator_Next(&ud_iter))
    {
        VIR_OpCode opcode;
        gctBOOL    remappableChannel = gcvFALSE;

        def_inst        = def->defKey.pDefInst;

        if (vscHTBL_DirectTestAndGet(inst_def_set, (void*) def_inst, gcvNULL))
        {
            continue;
        }

        vscHTBL_DirectSet(inst_def_set, (void*) def_inst, gcvNULL);

        def_inst_dest   = VIR_Inst_GetDest(def_inst);
        def_inst_enable = VIR_Operand_GetEnable(def_inst_dest);
        opcode          = VIR_Inst_GetOpcode(def_inst);

        switch(opcode)
        {
        case VIR_OP_TEXLD:
        case VIR_OP_TEXLD_U:
        case VIR_OP_TEXLD_U_F_L:
        case VIR_OP_TEXLD_U_F_B:
        case VIR_OP_TEXLD_U_S_L:
        case VIR_OP_TEXLD_U_U_L:
        case VIR_OP_TEXLDPROJ:
        case VIR_OP_TEXLDPCF:
        case VIR_OP_TEXLDPCFPROJ:
        case VIR_OP_TEXLD_BIAS:
        case VIR_OP_TEXLD_BIAS_PCF:
        case VIR_OP_TEXLD_PLAIN:
        case VIR_OP_TEXLD_PCF:
        case VIR_OP_TEXLDB:
        case VIR_OP_TEXLDD:
        case VIR_OP_TEXLD_G:
        case VIR_OP_TEXLDL:
        case VIR_OP_TEXLDP:
        case VIR_OP_TEXLD_LOD:
        case VIR_OP_TEXLD_LOD_PCF:
        case VIR_OP_TEXLD_G_PCF:
        case VIR_OP_TEXLD_U_PLAIN:
        case VIR_OP_TEXLD_U_LOD:
        case VIR_OP_TEXLD_U_BIAS:
        case VIR_OP_TEXLD_GATHER:
        case VIR_OP_TEXLD_GATHER_PCF:
        case VIR_OP_TEXLD_FETCH_MS:
        case VIR_OP_SURLD:
        case VIR_OP_SURSTORE:
        case VIR_OP_SURRED:
            /* Should NOT do backward CPP for TEXLD.
               For example,
               TEXLD t1.xyzw, sampler2D t3, t4.x,
               MOV t2.x t1.x  ==> only use one component

               We should NOT change it to
               TEXLD t1.x, sampler2D t3, t4.x,
               Since TEXLD generates a vec4 */

            invalid_case = gcvTRUE;
            break;
        default:
            if (VIR_OPCODE_isVX(opcode))
            {
                /* VX instruction has specific (packed) type dest, we don't want the
                 * backward copy to change its type */
                invalid_case = gcvTRUE;
            }
            break;
        }

        /* Don't do CPP for a CSELECT if it is a special pattern to generate CMPs. */
        if (opcode == VIR_OP_CSELECT &&
            (VIR_Inst_GetConditionOp(def_inst) == VIR_COP_NOT || VIR_Inst_GetConditionOp(def_inst) == VIR_COP_NOT_ZERO))
        {
            VIR_Operand*    pSrc1Opnd = VIR_Inst_GetSource(def_inst, 1);
            VIR_OperandInfo dstOpnd, src1Opnd;

            VIR_Operand_GetOperandInfo(def_inst, def_inst_dest, &dstOpnd);
            VIR_Operand_GetOperandInfo(def_inst, pSrc1Opnd, &src1Opnd);

            if (dstOpnd.isVreg && src1Opnd.isVreg && dstOpnd.u1.virRegInfo.virReg == src1Opnd.u1.virRegInfo.virReg)
            {
                continue;
            }
        }

        if(invalid_case)
        {
            if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                          VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
            {
                VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                VIR_LOG(dumper, "[BW] ==> bail out because of its def instruction [not supported opcode]:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, def_inst);
                VIR_LOG_FLUSH(dumper);
            }
            break;
        }

        /* now the remap case would be following 2 cases, which could use one time remap
         * opcode t1.xy =
         * MOV    t2.xy = t1.yx
         * or case 2:
         * opcode t1.xy =
         * MOV    t2.zw = t1.xy
         * case 3:
         * opcode t1.xy =
         * MOV    t2.yz = t1.yx
         */
        def2UsageRemappable = (VIR_Swizzle_2_Enable(inst_src0_swizzle) == def_inst_enable);
        remappableChannel =  (VIR_OPCODE_isComponentwise(opcode) &&
                              movInstRemappable &&
                              (VIR_Enable_Channel_Count(inst_dest_enable) == VIR_Enable_Channel_Count(def_inst_enable)) &&
                              (def2UsageRemappable || inst_dest_enable == def_inst_enable));

        for(channel = 0; channel < VIR_CHANNEL_COUNT; ++channel)
        {
            if(inst_dest_enable & (1 << channel))
            {
                if((VIR_Swizzle)channel != VIR_Swizzle_GetChannel(inst_src0_swizzle, channel))
                {
                    if (!remappableChannel)
                    {
                        invalid_case = gcvTRUE;
                        break;
                    }
                    else
                    {
                        needRemapChannel |= gcvTRUE;
                    }
                }
            }
        }

        if(invalid_case)
        {
            /* special handling for cmad/cmul */
            if (opcode != VIR_OP_CMAD &&
                opcode != VIR_OP_CMUL &&
                opcode != VIR_OP_CADD &&
                opcode != VIR_OP_CMADCJ &&
                opcode != VIR_OP_CMULCJ &&
                opcode != VIR_OP_CADDCJ &&
                opcode != VIR_OP_CSUBCJ )
            {
                if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                              VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
                {
                    VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                    VIR_LOG(dumper, "[BW] ==> bail out because inst_dest's enable dose not match with inst_src0's swizzle.\n");
                    VIR_LOG_FLUSH(dumper);
                }
                vscHTBL_Reset(inst_def_set);
                return errCode;
            }
            else
            {
                invalid_case = 0;
            }
        }

        ty0 = VIR_Operand_GetTypeId(def_inst_dest);
        ty1 = VIR_Operand_GetTypeId(inst_dest);

        gcmASSERT(ty0 < VIR_TYPE_PRIMITIVETYPE_COUNT &&
            ty1 < VIR_TYPE_PRIMITIVETYPE_COUNT);

        /* Their types should be same. This is for avoiding some bugs from other place. */
        if(!(((VIR_GetTypeFlag(ty0) & VIR_TYFLAG_ISFLOAT) &&
            (VIR_GetTypeFlag(ty1) & VIR_TYFLAG_ISFLOAT)) ||
            ((VIR_GetTypeFlag(ty0) & VIR_TYFLAG_ISINTEGER) &&
            (VIR_GetTypeFlag(ty1) & VIR_TYFLAG_ISINTEGER))))
        {
            if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                              VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
            {
                VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                VIR_LOG(dumper, "[BW] ==> bail out because of its def instruction [different types]:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, def_inst);
                VIR_LOG_FLUSH(dumper);
            }
            invalid_case = gcvTRUE;
            break;
        }

        /* if def_inst doesn't support sat_0_to_1, skip the change
         * Only support sat for FLOAT32 or LOAD/STORE/IMG_STORE/I2I/CONV
         */
        if (VIR_Inst_GetOpcode(inst) == VIR_OP_SAT &&
            (!(VIR_GetTypeFlag(ty0) & VIR_TYFLAG_ISFLOAT) || (VIR_OPCODE_NotSupportSat0to1(opcode))))
        {
            if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                              VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
            {
                VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                VIR_LOG(dumper, "[BW] ==> bail out because of its def instruction [doesn't support .sat]:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, def_inst);
                VIR_LOG_FLUSH(dumper);
            }
            invalid_case = gcvTRUE;
            break;
        }
        /* inst_enable should cover def_dest_enable */
        if (!VIR_Enable_Covers(inst_dest_enable, def_inst_enable) &&
            opcode != VIR_OP_CMAD   &&
            opcode != VIR_OP_CMUL   &&
            opcode != VIR_OP_CADD   &&
            opcode != VIR_OP_CMADCJ &&
            opcode != VIR_OP_CMULCJ &&
            opcode != VIR_OP_CADDCJ &&
            opcode != VIR_OP_CSUBCJ &&
            (!remappableChannel))
        {
            if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                              VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
            {
                VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                VIR_LOG(dumper, "[BW] ==> bail out because of its def instruction [enable not covered]:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, def_inst);
                VIR_LOG_FLUSH(dumper);
            }
            invalid_case = gcvTRUE;
            break;
        }
        else
        {
            for(channel = 0; channel < VIR_CHANNEL_NUM; channel++)
            {
                if(!(def_inst_enable & (1 << channel)))
                {
                    continue;
                }

                /* already covered in another def */
                if (covered_channels[channel])
                {
                    if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                              VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
                    {
                        VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                        VIR_LOG(dumper, "[BW] ==> bail out because of its multiple def instructions overlapping\n");
                        VIR_LOG_FLUSH(dumper);
                        VIR_Inst_Dump(dumper, def_inst);
                        VIR_LOG_FLUSH(dumper);
                    }
                    invalid_case = gcvTRUE;
                    break;
                }
                else
                {
                    covered_channels[channel] = gcvTRUE;
                }
            }
        }

        if(VIR_Operand_GetModifier(def_inst_dest) || VIR_Operand_GetRoundMode(def_inst_dest))
        {
            if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                              VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
            {
                VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                VIR_LOG(dumper, "[BW] ==> bail out because its def [has modifier/round]:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, def_inst);
                VIR_LOG_FLUSH(dumper);
            }
            invalid_case = gcvTRUE;
            break;
        }

        if(VIR_Inst_GetBasicBlock(inst) != VIR_Inst_GetBasicBlock(def_inst))
        {
            if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                              VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
            {
                VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                VIR_LOG(dumper, "[BW] ==> bail outd because of the following def and inst are not in the same bb:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, def_inst);
                VIR_LOG_FLUSH(dumper);
            }
            invalid_case = gcvTRUE;
            break;
        }

        /* inst should be the only use of def_inst */
        {
            VIR_Instruction* breaking_use;
            if(!vscVIR_IsUniqueUsageInstOfDefInst(VSC_CPP_GetDUInfo(cpp), def_inst, inst, gcvNULL, gcvFALSE, &breaking_use, gcvNULL, gcvNULL))
            {
                if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                                  VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
                {
                    VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                    VIR_LOG(dumper, "[BW] ==> bail out because of another use instruction:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, breaking_use);
                    VIR_LOG_FLUSH(dumper);
                }
                invalid_case = gcvTRUE;
                break;
            }
        }

        /*
        Check in BB: no redefine of inst_dst between def_inst and inst

        add t0, t1, t2
        ... ...
        mul t3, t4, t5      <== redefine of t3, invalid case
        ... ...
        mov t3, t0
        ... ...
        some use of t3

        if change to the following, wrong code is generated

        add t3, t1, t2
        ... ...
        mul t3, t4, t5
        ... ...
        some use of t3
        */
        /* there should be no re-def of inst_dest between def_inst and inst */
        {
            VIR_Instruction* next = VIR_Inst_GetNext(def_inst);
            while(next && next != inst)
            {
                if(VIR_Operand_SameLocation(inst, inst_dest, next, VIR_Inst_GetDest(next)))
                {
                    if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                                      VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
                    {
                        VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                        VIR_LOG(dumper, "[BW] ==> bail out because between def_inst and inst:\n");
                        VIR_LOG_FLUSH(dumper);
                        VIR_Inst_Dump(dumper, def_inst);
                        VIR_LOG_FLUSH(dumper);
                        VIR_LOG(dumper, "[BW] this intruction re-def inst_dest:\n");
                        VIR_LOG_FLUSH(dumper);
                        VIR_Inst_Dump(dumper, next);
                        VIR_LOG_FLUSH(dumper);
                    }
                    invalid_case = gcvTRUE;
                }

                if (VIR_Inst_GetOpcode(next) == VIR_OP_EMIT0 &&
                    inst_dest_info.isOutput)
                {
                    if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                                      VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
                    {
                        VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                        VIR_LOG(dumper, "[BW] ==> bail out because EMIT between def_inst and inst:\n");
                        VIR_LOG_FLUSH(dumper);
                    }
                    invalid_case = gcvTRUE;
                }

                if(invalid_case)
                {
                    break;
                }
                next = VIR_Inst_GetNext(next);
            }

            if (VIR_Inst_GetOpcode(def_inst) == VIR_OP_NORM_MUL && VIR_Shader_isDual16Mode(shader) &&
                VIR_Operand_GetPrecision(VIR_Inst_GetDest(inst)) != VIR_Operand_GetPrecision(VIR_Inst_GetSource(inst, 0)))
            {
                /* do not copy the MOV back to NORM_MUL, it cannot mix high and medium precision
                 * operands/dest due to HW restriction */
                invalid_case = gcvTRUE;
            }

        }

        /*
            Check in BB: no use of inst_dst between def_inst and inst
            1. mov t3, u1
            ...
            2. add t0, t1, t2
            ... ...
            3. mul t4, t3, t5      <== use of t3 (whose definition is 1)
            ... ...
            4. mov t3, t0

            if it is changed to the following, wrong code is generated
            1. mov t3, u1
            ...
            2. add t3, t1, t2
            ... ...
            3. mul t4, t3, t5   <= wrong t3 is used (whose definition is 2)
            ... ...
            some use of t3

            But we should allow this case:
            1. mov t3.x, u1
            ...
            2. add t0.zw, t1, t2    (<== add t3.zw, t1, t2 after copy )
            ... ...
            3. mul t4, t3.x, t5      <== use of different channels of t3
            ... ...
            4. mov t3.zw, t0.zw

        */
        /* there should be no use of inst_dest between def_inst and inst
         * the final enable is enable of inst_dest
         */
        {
            VIR_Instruction* next = VIR_Inst_GetNext(def_inst);
            VIR_Enable defInstEnable = VIR_Inst_GetEnable(inst);
            gctSIZE_T i = 0;

            while(next && next != inst)
            {
                for(i = 0; i < VIR_Inst_GetSrcNum(next); ++i)
                {
                    if(VIR_Operand_SameLocationByEnable(inst, inst_dest, defInstEnable, next, VIR_Inst_GetSource(next, i)))
                    {
                        if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                                          VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
                        {
                            VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                            VIR_LOG(dumper, "[BW] ==> bail out because between def_inst and inst:\n");
                            VIR_LOG_FLUSH(dumper);
                            VIR_Inst_Dump(dumper, def_inst);
                            VIR_LOG_FLUSH(dumper);
                            VIR_LOG(dumper, "[BW] this intruction use of inst_dest:\n");
                            VIR_LOG_FLUSH(dumper);
                            VIR_Inst_Dump(dumper, next);
                            VIR_LOG_FLUSH(dumper);
                        }
                        invalid_case = gcvTRUE;
                        break;
                    }
                }
                if(invalid_case)
                {
                    break;
                }
                next = VIR_Inst_GetNext(next);
            }
        }

        if(invalid_case)
        {
            break;
        }
    }

    /* check the covered channel */
    for(channel = 0; channel < VIR_CHANNEL_NUM; channel++)
    {
        if(!(inst_dest_enable & (1 << channel)))
        {
            continue;
        }

        if (!covered_channels[channel] &&
            VIR_Inst_GetOpcode(def_inst) != VIR_OP_CMAD &&
            VIR_Inst_GetOpcode(def_inst) != VIR_OP_CMUL &&
            VIR_Inst_GetOpcode(def_inst) != VIR_OP_CADD &&
            VIR_Inst_GetOpcode(def_inst) != VIR_OP_CMADCJ &&
            VIR_Inst_GetOpcode(def_inst) != VIR_OP_CMULCJ &&
            VIR_Inst_GetOpcode(def_inst) != VIR_OP_CADDCJ &&
            VIR_Inst_GetOpcode(def_inst) != VIR_OP_CSUBCJ &&
            (!needRemapChannel))
        {
            if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                              VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
            {
                VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                VIR_LOG(dumper, "[BW] ==> bail out because of def_enable not fully covered\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, def_inst);
                VIR_LOG_FLUSH(dumper);
            }
            invalid_case = gcvTRUE;
            break;
        }
    }

    if(invalid_case)
    {
        vscHTBL_Reset(inst_def_set);
        return errCode;
    }

    /* do transformation */
    /* collect the usage of the dest of the input inst, change the sym and map the usage channel */
    {
        VIR_GENERAL_DU_ITERATOR  inst_du_iter;
        VIR_USAGE               *inst_usage         = gcvNULL;
        gctUINT                 *channelMask        = gcvNULL;

        for(channel = 0; channel < VIR_CHANNEL_NUM; channel++)
        {
            if(!(inst_dest_enable & (1 << channel)))
            {
                continue;
            }

            vscVIR_InitGeneralDuIterator(
                &inst_du_iter,
                VSC_CPP_GetDUInfo(cpp),
                inst,
                inst_dest_info.u1.virRegInfo.virReg,
                channel,
                gcvFALSE);

            for(inst_usage = vscVIR_GeneralDuIterator_First(&inst_du_iter);
                inst_usage != gcvNULL;
                inst_usage = vscVIR_GeneralDuIterator_Next(&inst_du_iter))
            {
                if(vscHTBL_DirectTestAndGet(inst_usage_set, (void*)&(inst_usage->usageKey), (void**)&channelMask))
                {
                    *channelMask = (*channelMask) | (1<<channel);
                    vscHTBL_DirectSet(inst_usage_set, (void*)&(inst_usage->usageKey), channelMask);
                }
                else
                {
                    VSC_CPP_Usage* pHashKey = _VSC_CPP_NewUsage(cpp, inst_usage);
                    if(!pHashKey)
                        ON_ERROR(VSC_ERR_OUT_OF_MEMORY, "Fail to allocate CPP new usage");
                    channelMask = (gctUINT*)vscMM_Alloc(VSC_CPP_GetMM(cpp), sizeof(gctUINT));
                    if(!channelMask)
                        ON_ERROR(VSC_ERR_OUT_OF_MEMORY, "Fail to allocate channelMask.");
                    *channelMask = (1<<channel);
                    vscHTBL_DirectSet(inst_usage_set, (void*)pHashKey, channelMask);
                }
            }
        }

        {
            VSC_HASH_ITERATOR inst_def_set_iter;
            VSC_DIRECT_HNODE_PAIR inst_def_set_pair;
            vscHTBLIterator_Init(&inst_def_set_iter, inst_def_set);
            for(inst_def_set_pair = vscHTBLIterator_DirectFirst(&inst_def_set_iter);
                IS_VALID_DIRECT_HNODE_PAIR(&inst_def_set_pair); inst_def_set_pair = vscHTBLIterator_DirectNext(&inst_def_set_iter))
            {
                VIR_TypeId tyId;

                def_inst = (VIR_Instruction*)VSC_DIRECT_HNODE_PAIR_FIRST(&inst_def_set_pair);

                if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                                  VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
                {
                    VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                    VIR_LOG(dumper, "[BW] ==> changed def instrucion from:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, def_inst);
                    VIR_LOG_FLUSH(dumper);
                }

                /* this statement is the modification */
                def_inst_dest   = VIR_Inst_GetDest(def_inst);
                def_inst_enable = VIR_Operand_GetEnable(def_inst_dest);
                vscVIR_DeleteDef(
                    VSC_CPP_GetDUInfo(cpp),
                    def_inst,
                    inst_src0_info.u1.virRegInfo.virReg,
                    1,
                    def_inst_enable,
                    VIR_HALF_CHANNEL_MASK_FULL,
                    gcvNULL);

                if ((VIR_Inst_GetOpcode(def_inst) == VIR_OP_CMAD ||
                     VIR_Inst_GetOpcode(def_inst) == VIR_OP_CMUL ||
                     VIR_Inst_GetOpcode(def_inst) == VIR_OP_CADD ||
                     VIR_Inst_GetOpcode(def_inst) == VIR_OP_CMADCJ ||
                     VIR_Inst_GetOpcode(def_inst) == VIR_OP_CMULCJ ||
                     VIR_Inst_GetOpcode(def_inst) == VIR_OP_CADDCJ ||
                     VIR_Inst_GetOpcode(def_inst) == VIR_OP_CSUBCJ ) &&
                    VIR_Operand_GetEnable(inst_dest) == VIR_ENABLE_ZW)
                {
                    def_inst_enable = VIR_ENABLE_ZW;
                }
                else if (needRemapChannel)
                {
                    /*keep original MOV dest enable mask */
                    gctUINT idx;
                    gcmASSERT(VIR_OPCODE_isComponentwise(VIR_Inst_GetOpcode(def_inst)));
                    /* remap the swizzle of all sources of the def-instruction
                     * def_inst: def_inst_enable
                     * mov:   inst_dest_enable, inst_src0_swizzle
                     */
                    for (idx = 0; idx < VIR_Inst_GetSrcNum(def_inst); idx++) {
                        VIR_Operand *src = VIR_Inst_GetSource(def_inst, idx);
                        VIR_Swizzle old_swizzle, new_swizzle = (VIR_Swizzle)0;
                        if (VIR_Operand_isImm(src)) continue;
                        old_swizzle = VIR_Operand_GetSwizzle(src);
                        if (inst_dest_enable == def_inst_enable)
                        {
                            /*
                             *  t1.xy = t3.yz    => t2.xy = t3.zy
                             *  t2.xy = t1.yx
                             */
                            VIR_Swizzle mappingSwizzle = VIR_Enable_GetMappingSwizzle(def_inst_enable, inst_src0_swizzle);
                            new_swizzle = VIR_Swizzle_ApplySwizzlingSwizzle(old_swizzle, mappingSwizzle);
                        }
                        else
                        {
                            /* t1.zw = t3.zzzw  => t2.xy = t3.zw
                             * t2.xy = t1.zw
                             */
                            gctUINT i = 0;
                            VIR_Swizzle swizzle0;
                            VIR_Swizzle swizzle1;
                            new_swizzle = old_swizzle;
                            for (i = 0; i < VIR_CHANNEL_COUNT; i++)
                            {
                                if(inst_dest_enable & (1 << i))
                                {
                                    swizzle0 = VIR_Swizzle_GetChannel(inst_src0_swizzle, i);
                                    swizzle1 = VIR_Swizzle_GetChannel(old_swizzle, swizzle0);
                                    VIR_Swizzle_SetChannel(new_swizzle, i, swizzle1);
                                }
                            }
                        }
                        VIR_Operand_SetSwizzle(src, new_swizzle);
                    }
                    /*set def_inst_enable with inst_dest_enalbe */
                    def_inst_enable = inst_dest_enable;
                }
                /* keep the def_inst_dest's type:
                 *   004: RSHIFT   uchar_P2  temp(276).x, uchar_P2 temp(6).x, int 7
                 *   005: MOV      short  temp(278).x, short temp(276).x
                 *    ==>
                 *   004: RSHIFT   uchar_P2  temp(278).x, uchar_P2 temp(6).x, int 7
                 */
                tyId = VIR_Operand_GetTypeId(def_inst_dest);
                VIR_Operand_ReplaceDefOperandWithDef(
                    def_inst_dest,
                    inst_dest,
                    def_inst_enable,
                    gcvFALSE);
                VIR_Operand_SetTypeId(def_inst_dest, tyId);

                memset(&nativeDefFlags, 0, sizeof(nativeDefFlags));
                nativeDefFlags.bIsInput = inst_dest_info.isInput;
                nativeDefFlags.bIsOutput = inst_dest_info.isOutput;
                vscVIR_AddNewDef(
                    VSC_CPP_GetDUInfo(cpp),
                    def_inst,
                    inst_dest_info.u1.virRegInfo.virReg,
                    1,
                    def_inst_enable,
                    VIR_HALF_CHANNEL_MASK_FULL,
                    &nativeDefFlags,
                    gcvNULL);

                VIR_Operand_SetModifier(def_inst_dest, VIR_Operand_GetModifier(inst_src0));
                if (VIR_Inst_GetOpcode(inst) == VIR_OP_SAT)
                {
                    gcmASSERT(VIR_Operand_GetModifier(def_inst_dest) == VIR_MOD_NONE);
                    VIR_Operand_SetModifier(def_inst_dest, VIR_MOD_SAT_0_TO_1);
                }

                if (VIR_Inst_GetThreadMode(inst) == VIR_THREAD_D16_DUAL_32 &&
                    VIR_Operand_GetPrecision(inst_dest) == VIR_PRECISION_HIGH)
                {
                    VIR_Inst_SetThreadMode(def_inst, VIR_THREAD_D16_DUAL_32);
                }

                /* add the usage info between the def of work_inst and the src of inst_usage_inst */
                {
                    VSC_HASH_ITERATOR inst_usage_set_iter;
                    VSC_DIRECT_HNODE_PAIR inst_usage_set_pair;
                    vscHTBLIterator_Init(&inst_usage_set_iter, inst_usage_set);
                    for(inst_usage_set_pair = vscHTBLIterator_DirectFirst(&inst_usage_set_iter);
                        IS_VALID_DIRECT_HNODE_PAIR(&inst_usage_set_pair);
                        inst_usage_set_pair = vscHTBLIterator_DirectNext(&inst_usage_set_iter))
                    {
                        VSC_CPP_Usage   *inst_usage = (VSC_CPP_Usage*)VSC_DIRECT_HNODE_PAIR_FIRST(&inst_usage_set_pair);
                        gctUINT         *inst_channel = (gctUINT*) VSC_DIRECT_HNODE_PAIR_SECOND(&inst_usage_set_pair);
                        VIR_Instruction* inst_usage_inst = inst_usage->inst;
                        VIR_Operand* inst_usage_opnd;
                        VIR_Enable enable;

                        if (VIR_IS_OUTPUT_USAGE_INST(inst_usage_inst))
                        {
                            /* Operand of output usage must be reg number of def */
                            inst_usage_opnd = (VIR_Operand*)(gctUINTPTR_T)(inst_dest_info.u1.virRegInfo.virReg);
                        }
                        else
                        {
                            inst_usage_opnd = inst_usage->opnd;
                        }
                        enable = *inst_channel;

                        if ((VIR_Enable)(enable & def_inst_enable) != VIR_ENABLE_NONE)
                        {
                            vscVIR_DeleteUsage(
                                VSC_CPP_GetDUInfo(cpp),
                                inst,
                                inst_usage_inst,
                                inst_usage_opnd,
                                gcvFALSE,
                                inst_dest_info.u1.virRegInfo.virReg,
                                1,
                                (VIR_Enable)(enable & def_inst_enable),
                                VIR_HALF_CHANNEL_MASK_FULL,
                                gcvNULL);

                            vscVIR_AddNewUsageToDef(
                                VSC_CPP_GetDUInfo(cpp),
                                def_inst,
                                inst_usage_inst,
                                inst_usage_opnd,
                                gcvFALSE,
                                inst_dest_info.u1.virRegInfo.virReg,
                                1,
                                (VIR_Enable)(enable & def_inst_enable),
                                VIR_HALF_CHANNEL_MASK_FULL,
                                gcvNULL);
                        }
                    }
                }
                if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                                  VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
                {
                    VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                    VIR_LOG(dumper, "[BW] ==> to:\n");
                    VIR_LOG_FLUSH(dumper);
                    VIR_Inst_Dump(dumper, def_inst);
                    VIR_LOG_FLUSH(dumper);
                }
            }
        }

        /* delete the usage info of the src of input inst */
        vscVIR_DeleteUsage(
            VSC_CPP_GetDUInfo(cpp),
            VIR_ANY_DEF_INST,
            inst,
            inst_src0,
            gcvFALSE,
            inst_src0_info.u1.virRegInfo.virReg,
            1,
            inst_src0_enable,
            VIR_HALF_CHANNEL_MASK_FULL,
            gcvNULL);

        /* delete the def info of the dest of input inst */
        vscVIR_DeleteDef(
            VSC_CPP_GetDUInfo(cpp),
            inst,
            inst_dest_info.u1.virRegInfo.virReg,
            1,
            inst_dest_enable,
            VIR_HALF_CHANNEL_MASK_FULL,
            gcvNULL);

        VSC_CPP_SetBWOptCount(cpp, VSC_CPP_GetBWOptCount(cpp) + 1);

        if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                          VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
        {
            VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
            VIR_LOG(dumper, "[BW] ==> removed instruction\n");
            VIR_LOG_FLUSH(dumper);
            VIR_Inst_Dump(dumper, inst);
            VIR_LOG_FLUSH(dumper);
        }
        vscVIR_DeleteInstructionWithDu(gcvNULL, func, inst, &VSC_CPP_GetInvalidCfg(cpp));

        vscHTBL_Reset(inst_usage_set);
    }

    vscHTBL_Reset(inst_def_set);

    OnError:
    return errCode;
}

static VSC_ErrCode _VSC_CPP_CopyPropagationForBB(
    IN OUT VSC_CPP_CopyPropagation  *cpp,
    IN     VSC_HASH_TABLE           *inst_def_set,
    IN     VSC_HASH_TABLE           *inst_usage_set,
    IN     VSC_HASH_TABLE           *visitSet
    )
{
    VSC_ErrCode         errCode  = VSC_ERR_NONE;
    VIR_BB              *bb = VSC_CPP_GetCurrBB(cpp);
    VSC_OPTN_CPPOptions *options = VSC_CPP_GetOptions(cpp);

    VIR_Instruction     *inst;
    VIR_Instruction     *next_inst;

    inst = BB_GET_START_INST(bb);
    while(inst != VIR_Inst_GetNext(BB_GET_END_INST(bb)))
    {
        /* save the next instruction, since inst could be deleted by _VSC_CPP_CopyToMOV */
        next_inst = VIR_Inst_GetNext(inst);

        if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetOPTS(options),
                          VSC_OPTN_CPPOptions_FORWARD_OPT))
        {
            _VSC_CPP_CopyFromMOV(cpp, inst, visitSet);
        }

        /* treat SAT as MOV and dest_inst has some specialty */
        if (VIR_Inst_GetOpcode(inst) == VIR_OP_MOV || VIR_Inst_GetOpcode(inst) == VIR_OP_SAT)
        {
            if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetOPTS(options),
                VSC_OPTN_CPPOptions_BACKWARD_OPT))
            {
                 _VSC_CPP_CopyToMOV(cpp, inst, inst_def_set, inst_usage_set);
            }
        }

        inst = next_inst;
    }

    return errCode;
}

static VSC_ErrCode VSC_CPP_PerformOnFunction(
    IN OUT VSC_CPP_CopyPropagation *cpp
    )
{
    VSC_ErrCode     errCode  = VSC_ERR_NONE;
    VIR_Shader      *shader = VSC_CPP_GetShader(cpp);
    VIR_Function    *func = VIR_Shader_GetCurrentFunction(shader);
    VSC_OPTN_CPPOptions *options = VSC_CPP_GetOptions(cpp);

    VIR_CONTROL_FLOW_GRAPH* cfg;
    CFG_ITERATOR cfg_iter;
    VIR_BASIC_BLOCK* bb;
    /* hash table create once here for using _VSC_CPP_CopyToMOV */
    VSC_HASH_TABLE          *inst_def_set     = gcvNULL;
    VSC_HASH_TABLE          *inst_usage_set   = gcvNULL;
    VSC_HASH_TABLE          *visitSet         = gcvNULL;

    cfg = VIR_Function_GetCFG(func);

    /* dump input cfg */
    if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_INPUT))
    {
        VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
        VIR_LOG(dumper, "%s\nCopy Propagation: input cfg of function %s\n%s\n",
            VSC_TRACE_STAR_LINE, VIR_Shader_GetSymNameString(shader, VIR_Function_GetSymbol(func)), VSC_TRACE_STAR_LINE);
        VIR_LOG_FLUSH(dumper);
        VIR_CFG_Dump(dumper, cfg, gcvTRUE);
    }

    if (VIR_Inst_Count(&func->instList) > 1)
    {
        CFG_ITERATOR_INIT(&cfg_iter, cfg);
        /* create hash tables here */
        inst_def_set = vscHTBL_Create(VSC_CPP_GetMM(cpp), vscHFUNC_Default, vscHKCMP_Default, 512);
        inst_usage_set = vscHTBL_Create(VSC_CPP_GetMM(cpp), _VSC_CPP_Usage_HFUNC, _VSC_CPP_Usage_HKCMP, 512);
        visitSet = vscHTBL_Create(VSC_CPP_GetMM(cpp), vscHFUNC_Default, vscHKCMP_Default, 512);
        for(bb = CFG_ITERATOR_FIRST(&cfg_iter); bb != gcvNULL; bb = CFG_ITERATOR_NEXT(&cfg_iter))
        {
            if(BB_GET_LENGTH(bb) != 0)
            {
                VSC_CPP_SetCurrBB(cpp, bb);
                errCode = _VSC_CPP_CopyPropagationForBB(cpp, inst_def_set, inst_usage_set, visitSet);
            }

            if(errCode)
            {
                return errCode;
            }
        }

        func->instList.pHead = func->instList.pHead->parent.BB->pStartInst;
        func->instList.pTail = func->instList.pTail->parent.BB->pEndInst;

        /*destroy hash tables */
        vscHTBL_Destroy(inst_def_set);
        vscHTBL_Destroy(inst_usage_set);
        vscHTBL_Destroy(visitSet);
    }

    /* dump output cfg */
    if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT) ||
       VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
    {
        VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
        VIR_LOG(dumper, "%s\n ====== %d copy Propagation performed ====== \n%s\n",
            VSC_TRACE_STAR_LINE, VSC_CPP_GetFWOptCount(cpp) + VSC_CPP_GetBWOptCount(cpp), VSC_TRACE_STAR_LINE);
        VIR_LOG_FLUSH(dumper);
    }

    if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options), VSC_OPTN_CPPOptions_TRACE_OUTPUT))
    {
        VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
        VIR_LOG(dumper, "%s\nCopy Propagation: output cfg of function %s\n%s\n",
            VSC_TRACE_STAR_LINE,
            VIR_Shader_GetSymNameString(shader, VIR_Function_GetSymbol(func)),
            VSC_TRACE_STAR_LINE);
        VIR_LOG_FLUSH(dumper);
        VIR_CFG_Dump(dumper, cfg, gcvTRUE);
    }

    return errCode;
}

DEF_QUERY_PASS_PROP(VSC_CPP_PerformOnShader)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_ML |
                                 VSC_PASS_LEVEL_LL |
                                 VSC_PASS_LEVEL_MC;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_CPP;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;

    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
}

DEF_SH_NECESSITY_CHECK(VSC_CPP_PerformOnShader)
{
    return gcvTRUE;
}

VSC_ErrCode VSC_CPP_PerformOnShader(
    IN VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode         errcode  = VSC_ERR_NONE;
    VIR_Shader          *shader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_FuncIterator    func_iter;
    VIR_FunctionNode    *func_node;
    VSC_CPP_CopyPropagation cpp;
    VSC_OPTN_CPPOptions* cpp_options = (VSC_OPTN_CPPOptions*)pPassWorker->basePassWorker.pBaseOption;
    VSC_CPP_PASS_DATA   cppPassData = { VSC_CPP_NONE, gcvTRUE };

    if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(cpp_options),
        VSC_OPTN_CPPOptions_TRACE_INPUT))
    {
        VIR_Shader_Dump(gcvNULL, "Shader before Copy Propagation", shader, gcvTRUE);
    }

    if (pPassWorker->basePassWorker.pPassSpecificData)
    {
        cppPassData = *(VSC_CPP_PASS_DATA*)pPassWorker->basePassWorker.pPassSpecificData;
    }

    VSC_CPP_Init(&cpp, pPassWorker->pCompilerParam->cfg.ctx.appNameId, shader, pPassWorker->pDuInfo, cpp_options, &cppPassData,
                 pPassWorker->basePassWorker.pDumper, pPassWorker->basePassWorker.pMM, &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg);

    /* don't perform global CPP when the cfg has too many nodes*/
    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(shader));
    for(func_node = VIR_FuncIterator_First(&func_iter);
        func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function    *func = func_node->function;
        gctUINT         maxInstCount = VSC_CPP_MAX_INST_COUNT_GENERAL;

        if (VSC_CPP_GetAppNameId(&cpp) == gcvPATCH_DEQP)
        {
            maxInstCount = VSC_CPP_MAX_INST_COUNT_DEQP;
        }

        if (VIR_Function_GetCFG(func)->dgGraph.nodeList.info.count > 1000 ||
            VIR_Function_GetInstCount(func) > maxInstCount)
        {
            VSC_CPP_SetGlobalCPP(&cpp, gcvFALSE);
            break;
        }
    }

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(shader));
    for(func_node = VIR_FuncIterator_First(&func_iter);
        func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function    *func = func_node->function;

        VIR_Shader_SetCurrentFunction(shader, func);
        errcode = VSC_CPP_PerformOnFunction(&cpp);
        if(errcode)
        {
            break;
        }
    }

    if (VSC_CPP_GetInvalidCfg(&cpp))
    {
        pPassWorker->pResDestroyReq->s.bInvalidateCfg = gcvTRUE;
    }

    if ((VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(VSC_CPP_GetOptions(&cpp)),
         VSC_OPTN_CPPOptions_TRACE_INPUT)) ||
        VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(shader), VIR_Shader_GetId(shader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        if (VSC_CPP_isGlobalCPP(&cpp))
        {
            VIR_Shader_Dump(gcvNULL, "After Global Copy Propagation.", shader, gcvTRUE);
        }
        else
        {
            VIR_Shader_Dump(gcvNULL, "After Local Copy Propagation.", shader, gcvTRUE);
        }
    }

    VSC_CPP_Final(&cpp);

    return errcode;
}

/* simple local copy propagation */
typedef struct VIR_SCPP
{
    VIR_Shader              *shader;
    VIR_DEF_USAGE_INFO      *du_info;
    VSC_OPTN_SCPPOptions    *options;
    VIR_Dumper              *dumper;
    VSC_MM                  *mm;
    gctBOOL                 bChanged;
} VIR_SCPP;

#define VIR_SCPP_GetShader(scpp)                ((scpp)->shader)
#define VIR_SCPP_SetShader(scpp, s)             ((scpp)->shader = (s))
#define VIR_SCPP_GetDUInfo(scpp)                ((scpp)->du_info)
#define VIR_SCPP_SetDUInfo(scpp, d)             ((scpp)->du_info = (d))
#define VIR_SCPP_GetOptions(scpp)               ((scpp)->options)
#define VIR_SCPP_SetOptions(scpp, o)            ((scpp)->options = (o))
#define VIR_SCPP_GetDumper(scpp)                ((scpp)->dumper)
#define VIR_SCPP_SetDumper(scpp, d)             ((scpp)->dumper = (d))
#define VIR_SCPP_GetIsChanged(scpp)             ((scpp)->bChanged)
#define VIR_SCPP_SetIsChanged(scpp, b)          ((scpp)->bChanged = (b))
#define VIR_SCPP_GetMM(scpp)                    ((scpp)->mm)
#define VIR_SCPP_SetMM(scpp, m)                 ((scpp)->mm = (m))

void
VIR_SCPP_Init(
    VIR_SCPP* scpp,
    VIR_DEF_USAGE_INFO* du_info,
    VIR_Shader* shader,
    VSC_OPTN_SCPPOptions* options,
    VIR_Dumper* dumper,
    VSC_MM* mm
    )
{
    VIR_SCPP_SetShader(scpp, shader);
    VIR_SCPP_SetDUInfo(scpp, du_info);
    VIR_SCPP_SetOptions(scpp, options);
    VIR_SCPP_SetDumper(scpp, dumper);
    VIR_SCPP_SetIsChanged(scpp, gcvFALSE);
    VIR_SCPP_SetMM(scpp, mm);
}

void
VIR_SCPP_Final(
    VIR_SCPP* scpp
    )
{

}

typedef struct VIR_SCPP_COPY
{
    VIR_SymId rhsSymId[VIR_CHANNEL_NUM];
    VIR_Swizzle mappingSwizzle;
    VIR_Instruction*    pMovInst;
    VIR_Modifier destModifier;
    VIR_Modifier src0Modifier;
} VIR_SCPP_Copy;

#define VIR_SCPP_Copy_GetRhsSymId(d, i)                  ((d)->rhsSymId[i])
#define VIR_SCPP_Copy_SetRhsSymId(d, i, r)               ((d)->rhsSymId[i] = (r))
#define VIR_SCPP_Copy_GetMappingSwizzle(d)               ((d)->mappingSwizzle)
#define VIR_SCPP_Copy_SetMappingSwizzle(d, m)            ((d)->mappingSwizzle = (VIR_Swizzle)(m))
#define VIR_SCPP_Copy_GetSingleMappingSwizzle(d, i)      VIR_Swizzle_GetChannel((d)->mappingSwizzle, i)
#define VIR_SCPP_Copy_SetSingleMappingSwizzle(d, i, m)   VIR_Swizzle_SetChannel((d)->mappingSwizzle, i, (m))
#define VIR_SCPP_Copy_GetMovInst(d)                      ((d)->pMovInst)
#define VIR_SCPP_Copy_SetMovInst(d, m)                   ((d)->pMovInst = (m))
#define VIR_SCPP_Copy_GetDestModifier(d)                 ((d)->destModifier)
#define VIR_SCPP_Copy_SetDestModifier(d, m)              ((d)->destModifier = (m))
#define VIR_SCPP_Copy_GetSrc0Modifier(d)                 ((d)->src0Modifier)
#define VIR_SCPP_Copy_SetSrc0Modifier(d, m)              ((d)->src0Modifier = (m))

void
_VIR_SCPP_Copy_Init(
    VIR_SCPP_Copy* copy,
    VIR_Instruction* pMovInst
    )
{
    gctUINT i;

    for(i = 0; i < VIR_CHANNEL_NUM; i++)
    {
        VIR_SCPP_Copy_SetRhsSymId(copy, i, VIR_INVALID_ID);
    }
    VIR_SCPP_Copy_SetMappingSwizzle(copy, VIR_SWIZZLE_XYZW);
    VIR_SCPP_Copy_SetMovInst(copy, pMovInst);

    VIR_SCPP_Copy_SetDestModifier(copy, VIR_Operand_GetModifier(VIR_Inst_GetDest(pMovInst)));
    VIR_SCPP_Copy_SetSrc0Modifier(copy, VIR_Operand_GetModifier(VIR_Inst_GetSource(pMovInst, 0)));
}

static VIR_SymId
_VIR_SCPP_Copy_GetRhs(
    VIR_SCPP_Copy* copy,
    VIR_Swizzle swizzle
    )
{
    VIR_SymId firstRhsSymId = VIR_INVALID_ID;
    VIR_Enable enable = VIR_Swizzle_2_Enable(swizzle);
    gctUINT i;

    for(i = 0; i < VIR_CHANNEL_NUM; i++)
    {
        if(enable & (1 << i))
        {
            if(VIR_SCPP_Copy_GetRhsSymId(copy, i) != VIR_INVALID_ID)
            {
                if(firstRhsSymId == VIR_INVALID_ID)
                {
                    firstRhsSymId = VIR_SCPP_Copy_GetRhsSymId(copy, i);
                }
                else if(firstRhsSymId != VIR_SCPP_Copy_GetRhsSymId(copy, i))
                {
                    return VIR_INVALID_ID;
                }
            }
            else
            {
                return VIR_INVALID_ID;
            }
        }
    }

    return firstRhsSymId;
}

static void
_VIR_SCPP_Copy_UpdateChannel(
    VIR_SCPP_Copy* copy,
    gctUINT i,
    VIR_SymId lhsSymId,
    VIR_SymId rhsSymId,
    VIR_Swizzle swizzle
    )
{
    if(lhsSymId == rhsSymId)
    {
        VIR_SCPP_Copy_SetRhsSymId(copy, i, VIR_INVALID_ID);
    }
    else
    {
        VIR_SCPP_Copy_SetRhsSymId(copy, i, rhsSymId);
        VIR_SCPP_Copy_SetSingleMappingSwizzle(copy, i, swizzle);
    }
}

static void
_VIR_SCPP_Copy_Dump(
    VIR_SCPP_Copy* copy,
    VIR_Dumper* dumper
    )
{
    gctUINT i;

    for(i = 0; i < VIR_CHANNEL_NUM; i++)
    {
        if(VIR_SCPP_Copy_GetRhsSymId(copy, i) != VIR_INVALID_ID)
        {
            VIR_LOG(dumper, "channel%d: symbol %d, swizzle %d\n", i, VIR_SCPP_Copy_GetRhsSymId(copy, i), VIR_SCPP_Copy_GetSingleMappingSwizzle(copy, i));
        }
    }
    VIR_LOG_FLUSH(dumper);
}

static gctUINT
_VIR_SCPP_SymbolHashFunc(const void* ptr)
{
    VIR_Symbol* sym = (VIR_Symbol*)ptr;
    return (gctUINT)(VIR_Symbol_GetIndex(sym));
}

static gctBOOL
_VIR_SCPP_SymbolCmpFunc(const void* pHashKey1, const void* pHashKey2)
{
    VIR_Symbol* sym1 = (VIR_Symbol*)pHashKey1;
    VIR_Symbol* sym2 = (VIR_Symbol*)pHashKey2;
    return VIR_Symbol_GetIndex(sym1) == VIR_Symbol_GetIndex(sym2);
}

static gctBOOL
_VIR_SCPP_NeedToUpdateCopy(
    VIR_DEF_USAGE_INFO*     pDuInfo,
    VIR_Shader*             pShader,
    VIR_Instruction*        pInst
    )
{
    gctBOOL                 bNeed = gcvFALSE;
    VIR_Operand*            pDestOpnd = VIR_Inst_GetDest(pInst);
    VIR_OperandInfo         destOpndInfo;
    VIR_GENERAL_DU_ITERATOR instDuIter;
    VIR_Enable              enable = VIR_Operand_GetEnable(pDestOpnd);
    gctUINT8                channel = 0;

    VIR_Operand_GetOperandInfo(pInst, pDestOpnd, &destOpndInfo);

    /* If this instruction has no any usage, no need to to propagation. */
    for (channel = 0; channel < VIR_CHANNEL_NUM; channel++)
    {
        if (!(enable & (1 << channel)))
        {
            continue;
        }

        vscVIR_InitGeneralDuIterator(&instDuIter,
                                     pDuInfo,
                                     pInst,
                                     destOpndInfo.u1.virRegInfo.virReg,
                                     (VIR_Enable)channel,
                                     gcvFALSE);

        if (vscVIR_GeneralDuIterator_First(&instDuIter) != gcvNULL)
        {
            bNeed = gcvTRUE;
            break;
        }
    }

    /* if dest is output, skip propagete output to src's def in conversative way
     * failed case: temp(379) is output
     * 127: ADD                hp temp(274).hp.x,
     * 128: MOV                hp temp(379).hp.x, hp  temp(274).hp.x
     * 133: EMIT0
     * 134: MOV                hp temp(379).hp.x, hp  temp(274).hp.x  <- temp(274) is replace with temp(379)
     * ==> without this check, after scpp, usage of temp(379) will cross the EMIT0 and
     *     EMIT0 will kill all def of output variables in DU analysis and
     *     make temp(379) in 134 has no def pos
     * 128: ADD                hp temp(379).hp.x,
     * 133: EMIT0
     * 134: MOV                hp temp(379).hp.x, hp  temp(379).hp.x
     * VSC_CPP_PerformOnShade pass has detailed check for output variable and could do this prop in that pass
     */
    if (destOpndInfo.isOutput)
    {
        bNeed = gcvFALSE;
    }
    return bNeed;
}

static gctBOOL
_VIR_SCPP_AbleToDoNeighborPropagation(
    VIR_Shader* shader,
    VIR_Instruction* inst
    )
{
    gctBOOL result = gcvTRUE;
    VIR_Type* prevInstDestType;
    VIR_Type* curInstDestType;
    VIR_Instruction* prevInst = VIR_Inst_GetPrev(inst);
    VIR_OpCode prevOp;
    VIR_Operand* prevInstDest;
    VIR_Symbol* prevInstDestSym;
    VIR_Operand* src;
    gctUINT i;

    gcmASSERT(VIR_Inst_GetOpcode(inst) == VIR_OP_MOV);
    src = VIR_Inst_GetSource(inst, 0);
    gcmASSERT(VIR_Operand_isSymbol(src));

    if(prevInst == gcvNULL ||
       VIR_Inst_GetBasicBlock(prevInst) != VIR_Inst_GetBasicBlock(inst))
    {
        return gcvFALSE;
    }

    prevOp = VIR_Inst_GetOpcode(prevInst);
    if(prevOp != VIR_OP_ADD &&
       prevOp != VIR_OP_SUB &&
       prevOp != VIR_OP_MUL &&
       prevOp != VIR_OP_DIV &&
       prevOp != VIR_OP_RSHIFT &&
       prevOp != VIR_OP_LSHIFT)
    {
        return gcvFALSE;
    }

    prevInstDest = VIR_Inst_GetDest(prevInst);
    prevInstDestSym = VIR_Operand_GetSymbol(prevInstDest);

    if(prevInstDestSym != VIR_Operand_GetSymbol(src) ||
       VIR_Operand_GetSwizzle(src) != VIR_SWIZZLE_XXXX ||
       VIR_Inst_GetEnable(prevInst) != VIR_ENABLE_X)
    {
        return gcvFALSE;
    }

    prevInstDestType = VIR_Shader_GetTypeFromId(shader, VIR_Operand_GetTypeId(prevInstDest));
    curInstDestType = VIR_Shader_GetTypeFromId(shader, VIR_Operand_GetTypeId(VIR_Inst_GetDest(inst)));

    if (VIR_GetTypeComponentType(VIR_Type_GetBaseTypeId(prevInstDestType)) !=
        VIR_GetTypeComponentType(VIR_Type_GetBaseTypeId(curInstDestType)))
    {
        return gcvFALSE;
    }

    for(i = 0; i < VIR_Inst_GetSrcNum(prevInst); i++)
    {
        VIR_Operand* prevInstSrc = VIR_Inst_GetSource(prevInst, i);

        if(VIR_Operand_isSymbol(prevInstSrc) && VIR_Operand_GetSymbol(VIR_Inst_GetDest(prevInst)) == VIR_Operand_GetSymbol(prevInstSrc))
        {
            result = gcvFALSE;
            break;
        }
    }

    return result;
}

VSC_ErrCode VIR_SCPP_PerformOnBB(
    VIR_SCPP* scpp,
    VIR_Function* func,
    VIR_BB* bb
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    VIR_Shader* shader = VIR_SCPP_GetShader(scpp);
    VSC_HASH_TABLE* defsTable = vscHTBL_Create(VIR_SCPP_GetMM(scpp), _VIR_SCPP_SymbolHashFunc, _VIR_SCPP_SymbolCmpFunc, 256);
    VIR_SCPP_Copy* copy;
    VIR_Instruction* instIter = BB_GET_START_INST(bb);
    VSC_OPTN_SCPPOptions* option = VIR_SCPP_GetOptions(scpp);
    VIR_DEF_USAGE_INFO* pDuInfo = VIR_SCPP_GetDUInfo(scpp);
    gctBOOL bChanged = gcvFALSE;
    gctBOOL bHandleModifier = gcvFALSE;

    if(VSC_UTILS_MASK(VSC_OPTN_SCPPOptions_GetTrace(option), VSC_OPTN_SCPPOptions_TRACE_INPUT_BB))
    {
        VIR_LOG(VIR_SCPP_GetDumper(scpp), "bb before scpp from mov:\n");
        VIR_BasicBlock_Dump(VIR_SCPP_GetDumper(scpp), bb, gcvFALSE);
    }

    /* walk over bb instructions forwardly */
    while(gcvTRUE)
    {
        VIR_OpCode opcode = VIR_Inst_GetOpcode(instIter);
        gctUINT i;

        /* propagate over source operands if possiable */
        if(opcode != VIR_OP_LDARR)  /* if we propagate a uniform to LDARR's src1, it might be wrong */
        {
            for(i = 0; i < VIR_Inst_GetSrcNum(instIter); i++)
            {
                VIR_Operand* src = VIR_Inst_GetSource(instIter, i);
                VIR_Symbol* srcSym = gcvNULL;
                VIR_Swizzle srcSwizzle = VIR_Operand_GetSwizzle(src);
                VIR_OperandInfo srcOpndInfo;

                /* Do not copy the temp256 pair in SCPP, otherwise it will break the assumption that the register pair wiil be contiguous. */
                if (VIR_Operand_isTemp256High(src) || VIR_Operand_isTemp256Low(src))
                {
                     continue;
                }

                if(VIR_Operand_isSymbol(src))
                {
                    srcSym = VIR_Operand_GetSymbol(src);

                    if(vscHTBL_DirectTestAndGet(defsTable, (void*)srcSym, (void**)&copy))
                    {
                        VIR_SymId defSymId = _VIR_SCPP_Copy_GetRhs(copy, srcSwizzle);
                        VIR_Symbol* defSym;

                        if(defSymId != VIR_INVALID_ID)
                        {
                            defSym = VIR_Function_GetSymFromId(func, defSymId);
                            srcSwizzle = VIR_Swizzle_ApplyMappingSwizzle(srcSwizzle, VIR_SCPP_Copy_GetMappingSwizzle(copy));

                            if (VIR_OPCODE_isVX(VIR_Inst_GetOpcode(instIter)) &&
                                srcSwizzle!= VIR_SWIZZLE_XXXX &&
                                srcSwizzle != VIR_SWIZZLE_XYYY &&
                                srcSwizzle != VIR_SWIZZLE_XYZZ &&
                                srcSwizzle != VIR_SWIZZLE_XYZW    )
                            {
                                /* the EVIS inst src0's swizzle bits are used for EVIS info like startBin,
                                 * make sure the src0 operand does NOT use them
                                 */
                                continue;
                            }

                            if (VIR_SCPP_Copy_GetSrc0Modifier(copy) != VIR_MOD_NONE)
                            {
                                if (!bHandleModifier)
                                {
                                    continue;
                                }
                            }

                            if(VSC_UTILS_MASK(VSC_OPTN_SCPPOptions_GetTrace(option), VSC_OPTN_SCPPOptions_TRACE_DETAIL))
                            {
                                VIR_LOG(VIR_SCPP_GetDumper(scpp), "transform instruction:\n");
                                VIR_Inst_Dump(VIR_SCPP_GetDumper(scpp), instIter);
                            }

                            VIR_Operand_GetOperandInfo(instIter, src, &srcOpndInfo);

                            /* Delete the old usage. */
                            if (srcOpndInfo.isVreg)
                            {
                                vscVIR_DeleteUsage(pDuInfo,
                                                   VIR_SCPP_Copy_GetMovInst(copy),
                                                   instIter,
                                                   src,
                                                   gcvFALSE,
                                                   srcOpndInfo.u1.virRegInfo.virReg,
                                                   1,
                                                   VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(src)),
                                                   VIR_HALF_CHANNEL_MASK_FULL,
                                                   gcvNULL);
                            }

                            VIR_Operand_SetOpKind(src, VIR_OPND_SYMBOL);
                            VIR_Operand_SetSym(src, defSym);
                            VIR_Operand_SetPrecision(src, VIR_Symbol_GetPrecision(defSym));
                            VIR_Operand_SetSwizzle(src, srcSwizzle);

                            /* Add the new usage. */
                            VIR_Operand_GetOperandInfo(instIter, src, &srcOpndInfo);
                            if (srcOpndInfo.isVreg)
                            {
                                vscVIR_AddNewUsageToDef(pDuInfo,
                                                        VIR_ANY_DEF_INST,
                                                        instIter,
                                                        src,
                                                        gcvFALSE,
                                                        srcOpndInfo.u1.virRegInfo.virReg,
                                                        1,
                                                        VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(src)),
                                                        VIR_HALF_CHANNEL_MASK_FULL,
                                                        gcvNULL);
                            }

                            bChanged = gcvTRUE;

                            if(VSC_UTILS_MASK(VSC_OPTN_SCPPOptions_GetTrace(option), VSC_OPTN_SCPPOptions_TRACE_DETAIL))
                            {
                                VIR_LOG(VIR_SCPP_GetDumper(scpp), "to:\n");
                                VIR_Inst_Dump(VIR_SCPP_GetDumper(scpp), instIter);
                                VIR_LOG_FLUSH(VIR_SCPP_GetDumper(scpp));
                            }
                        }
                    }
                }
            }
        }

        /* update if copy is redefined */
        if(VIR_OPCODE_hasDest(opcode) && VIR_Operand_isSymbol(VIR_Inst_GetDest(instIter)))
        {
            VIR_Operand* dest = VIR_Inst_GetDest(instIter);
            VIR_Symbol* destSym = VIR_Operand_GetSymbol(dest);
            VIR_SymId destSymId = VIR_Symbol_GetIndex(destSym);
            VIR_Enable enable = VIR_Operand_GetEnable(dest);
            VSC_HASH_ITERATOR iter;
            VSC_DIRECT_HNODE_PAIR pair;

            if (!VIR_Symbol_IsInArray(destSym))
            {
                vscHTBLIterator_Init(&iter, defsTable);
                for(pair = vscHTBLIterator_DirectFirst(&iter);
                    IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
                {
                    VIR_Symbol* keySym = (VIR_Symbol*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
                    gctUINT i;

                    copy = (VIR_SCPP_Copy*)VSC_DIRECT_HNODE_PAIR_SECOND(&pair);

                    if(keySym == destSym)
                    {
                        for(i = 0; i < VIR_CHANNEL_NUM; i++)
                        {
                            if(enable & (1 << i))
                            {
                                if(VSC_UTILS_MASK(VSC_OPTN_SCPPOptions_GetTrace(option), VSC_OPTN_SCPPOptions_TRACE_DETAIL))
                                {
                                    VIR_LOG(VIR_SCPP_GetDumper(scpp), "according to instruction:\n");
                                    VIR_Inst_Dump(VIR_SCPP_GetDumper(scpp), instIter);
                                    VIR_LOG(VIR_SCPP_GetDumper(scpp), "reset symbol(%d)'s channel%d copy.\n", destSymId, i);
                                    _VIR_SCPP_Copy_Dump(copy, VIR_SCPP_GetDumper(scpp));
                                    VIR_LOG_FLUSH(VIR_SCPP_GetDumper(scpp));
                                }
                                VIR_SCPP_Copy_SetRhsSymId(copy, i, VIR_INVALID_ID);
                            }
                        }
                    }

                    for(i = 0; i < VIR_CHANNEL_NUM; i++)
                    {
                        if(VIR_SCPP_Copy_GetRhsSymId(copy, i) == destSymId &&
                            enable & (1 << VIR_SCPP_Copy_GetSingleMappingSwizzle(copy, i)))
                        {
                            if(VSC_UTILS_MASK(VSC_OPTN_SCPPOptions_GetTrace(option), VSC_OPTN_SCPPOptions_TRACE_DETAIL))
                            {
                                VIR_LOG(VIR_SCPP_GetDumper(scpp), "according to instruction:\n");
                                VIR_Inst_Dump(VIR_SCPP_GetDumper(scpp), instIter);
                                VIR_LOG(VIR_SCPP_GetDumper(scpp), "reset symbol(%d)'s channel%d copy.\n", destSymId, i);
                                _VIR_SCPP_Copy_Dump(copy, VIR_SCPP_GetDumper(scpp));
                                VIR_LOG_FLUSH(VIR_SCPP_GetDumper(scpp));
                            }
                            VIR_SCPP_Copy_SetRhsSymId(copy, i, VIR_INVALID_ID);
                        }
                    }
                }
            }
        }

        if(opcode == VIR_OP_MOV)
        {
            VIR_Operand* dest = VIR_Inst_GetDest(instIter);
            VIR_Symbol* destSym = VIR_Operand_GetSymbol(dest);
            VIR_Operand* src = VIR_Inst_GetSource(instIter, 0);
            gctBOOL      destIndexing = gcvFALSE;
            gctBOOL      srcIndexing  = gcvFALSE;
            /* the simple copy cannot support indexing access, so we need to
             * invalidate any MOV which has indexing access in either dest or src
             */
            if (VIR_Operand_GetIsConstIndexing(dest) ||
                VIR_Operand_GetMatrixConstIndex(dest) ||
                VIR_Operand_GetRelAddrMode(dest) != VIR_INDEXED_NONE)
            {
                destIndexing = gcvTRUE;
            }
            if (VIR_Operand_GetIsConstIndexing(src) ||
                VIR_Operand_GetMatrixConstIndex(src) ||
                VIR_Operand_GetRelAddrMode(src) != VIR_INDEXED_NONE)
            {
                srcIndexing = gcvTRUE;
            }
            if(!VIR_Symbol_IsInArray(destSym) &&
               VIR_Operand_isSymbol(src) &&
               !VIR_Symbol_IsInArray(VIR_Operand_GetSymbol(src)) &&
               (VIR_Operand_GetRelIndexing(src) == 0) &&
               _VIR_SCPP_NeedToUpdateCopy(pDuInfo, shader, instIter))
            {
                if(!destIndexing && !srcIndexing &&
                   (VIR_Operand_GetModifier(src) == VIR_MOD_NONE  || bHandleModifier)
                   &&
                   _VIR_SCPP_AbleToDoNeighborPropagation(shader, instIter))
                {
                    /* transform
                        028: ADD                int hp  temp(273).hp.x, int hp  temp(262).hp.x, int 1
                        029: MOV                int hp  temp(262).hp.x, int hp  temp(273).hp.x
                        to
                        036: ADD                int hp  temp(262).hp.x, int hp  temp(262).hp.x, int 1
                        to ease loop optimization */
                    VIR_Instruction*    pPrevInst = VIR_Inst_GetPrev(instIter);
                    VIR_Operand*        pNewOpnd = gcvNULL;
                    VIR_OperandInfo     operandInfo;
                    gctUINT             i;

                    VIR_Operand_GetOperandInfo(instIter, src, &operandInfo);

                    /* Delete the original usage. */
                    if (operandInfo.isVreg)
                    {
                        vscVIR_DeleteUsage(pDuInfo,
                                           pPrevInst,
                                           instIter,
                                           src,
                                           gcvFALSE,
                                           operandInfo.u1.virRegInfo.virReg,
                                           1,
                                           VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(src)),
                                           VIR_HALF_CHANNEL_MASK_FULL,
                                           gcvNULL);
                    }
                    VIR_Function_FreeOperand(func, src);

                    /* Copy the opcode and sources from the previous instruction. */
                    VIR_Inst_SetOpcode(instIter, VIR_Inst_GetOpcode(pPrevInst));
                    VIR_Inst_SetSrcNum(instIter, VIR_Inst_GetSrcNum(pPrevInst));

                    /* Add the new usage. */
                    for (i = 0; i < VIR_Inst_GetSrcNum(instIter); i++)
                    {
                        VIR_Function_DupOperand(func, VIR_Inst_GetSource(pPrevInst, i), &pNewOpnd);
                        VIR_Inst_SetSource(instIter, i, pNewOpnd);

                        VIR_Operand_GetOperandInfo(instIter, pNewOpnd, &operandInfo);

                        if (operandInfo.isVreg)
                        {
                            vscVIR_AddNewUsageToDef(pDuInfo,
                                                    VIR_ANY_DEF_INST,
                                                    instIter,
                                                    pNewOpnd,
                                                    gcvFALSE,
                                                    operandInfo.u1.virRegInfo.virReg,
                                                    1,
                                                    VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(pNewOpnd)),
                                                    VIR_HALF_CHANNEL_MASK_FULL,
                                                    gcvNULL);
                        }
                    }

                    if(gcvTRUE)
                    {
                        VIR_Operand* dest = VIR_Inst_GetDest(instIter);
                        VIR_Operand* prevDest = VIR_Inst_GetDest(pPrevInst);
                        VIR_Symbol* lhsSym = VIR_Operand_GetSymbol(prevDest);
                        VIR_Symbol* rhsSym = VIR_Operand_GetSymbol(dest);

                        if(!vscHTBL_DirectTestAndGet(defsTable, (void*)lhsSym, (void**)&copy))
                        {
                            copy = (VIR_SCPP_Copy*)vscMM_Alloc(VIR_SCPP_GetMM(scpp), sizeof(VIR_SCPP_Copy));
                            if(!copy)
                                ON_ERROR(VSC_ERR_OUT_OF_MEMORY, "Fail to allocate VIR_SCPP_Copy.");
                            _VIR_SCPP_Copy_Init(copy, instIter);
                            vscHTBL_DirectSet(defsTable, (void*)lhsSym, (void*)copy);
                        }

                        _VIR_SCPP_Copy_UpdateChannel(copy, VIR_SWIZZLE_X, VIR_Symbol_GetIndex(lhsSym), VIR_Symbol_GetIndex(rhsSym), VIR_SWIZZLE_X);

                        if(VSC_UTILS_MASK(VSC_OPTN_SCPPOptions_GetTrace(option), VSC_OPTN_SCPPOptions_TRACE_DETAIL))
                        {
                            VIR_LOG(VIR_SCPP_GetDumper(scpp), "and its previous instruction:\n");
                            VIR_Inst_Dump(VIR_SCPP_GetDumper(scpp), pPrevInst);
                            VIR_LOG(VIR_SCPP_GetDumper(scpp), "update symbol(%d)'s copy to:\n", VIR_Symbol_GetIndex(lhsSym));
                            _VIR_SCPP_Copy_Dump(copy, VIR_SCPP_GetDumper(scpp));
                            VIR_LOG_FLUSH(VIR_SCPP_GetDumper(scpp));
                        }
                    }

                    bChanged = gcvTRUE;
                }
                else
                {
                    /* generate new copy */

                    VIR_Operand* dest = VIR_Inst_GetDest(instIter);
                    VIR_Enable destEnable = VIR_Operand_GetEnable(dest);
                    VIR_OperandInfo destInfo;
                    VIR_Symbol* lhsSym = VIR_Operand_GetSymbol(dest);
                    VIR_Symbol* rhsSym = VIR_Operand_GetSymbol(src);
                    VIR_Enable enable = VIR_Operand_GetEnable(dest);
                    VIR_Swizzle swizzle = VIR_Operand_GetSwizzle(src);
                    VIR_GENERAL_DU_ITERATOR du_iter;
                    VIR_USAGE* pUsage = gcvNULL;
                    gctUINT i;
                    gctUINT8 channel;
                    gctBOOL bInvalidCase = gcvFALSE;

                    VIR_Operand_GetOperandInfo(instIter, dest, &destInfo);

                    /*
                    ** If any usage instruction of this MOV has more than one DEF instruction, which means we can't remove this MOV,
                    ** we don't generate this new copy because it may increase the live range of SOURCE0.
                    */
                    for (channel = 0; channel < VIR_CHANNEL_COUNT; ++channel)
                    {
                        if (!(destEnable & (1 << channel)))
                        {
                            continue;
                        }
                        vscVIR_InitGeneralDuIterator(&du_iter,
                                                     pDuInfo,
                                                     instIter,
                                                     destInfo.u1.virRegInfo.virReg,
                                                     channel,
                                                     gcvFALSE);

                        for (pUsage = vscVIR_GeneralDuIterator_First(&du_iter);
                             pUsage != gcvNULL;
                             pUsage = vscVIR_GeneralDuIterator_Next(&du_iter))
                        {
                            VIR_Instruction* pUsageInst = pUsage->usageKey.pUsageInst;

                            if (VIR_IS_OUTPUT_USAGE_INST(pUsageInst))
                            {
                                bInvalidCase = gcvTRUE;
                                break;
                            }

                            if (!vscVIR_IsUniqueDefInstOfUsageInst(VIR_SCPP_GetDUInfo(scpp),
                                                                   pUsageInst,
                                                                   pUsage->usageKey.pOperand,
                                                                   pUsage->usageKey.bIsIndexingRegUsage,
                                                                   instIter,
                                                                   gcvNULL))
                            {
                                bInvalidCase = gcvTRUE;
                                break;
                            }
                        }

                        if (bInvalidCase)
                        {
                            break;
                        }
                    }

                    if (!bInvalidCase)
                    {
                        if (!vscHTBL_DirectTestAndGet(defsTable, (void*)lhsSym, (void**)&copy))
                        {
                            copy = (VIR_SCPP_Copy*)vscMM_Alloc(VIR_SCPP_GetMM(scpp), sizeof(VIR_SCPP_Copy));
                            if(!copy)
                                ON_ERROR(VSC_ERR_OUT_OF_MEMORY, "Fail to allocate VIR_SCPP_Copy.");
                            _VIR_SCPP_Copy_Init(copy, instIter);
                            vscHTBL_DirectSet(defsTable, (void*)lhsSym, (void*)copy);
                        }

                        for (i = 0; i < VIR_CHANNEL_NUM; i++)
                        {
                            if (enable & 1 << i)
                            {
                                if (!destIndexing && !srcIndexing)
                                {
                                    _VIR_SCPP_Copy_UpdateChannel(copy, i, VIR_Symbol_GetIndex(lhsSym), VIR_Symbol_GetIndex(rhsSym), VIR_Swizzle_GetChannel(swizzle, i));
                                }
                                else
                                {
                                    _VIR_SCPP_Copy_UpdateChannel(copy, i, VIR_Symbol_GetIndex(lhsSym), VIR_INVALID_ID, VIR_Swizzle_GetChannel(swizzle, i));
                                }
                            }
                        }

                        if (VSC_UTILS_MASK(VSC_OPTN_SCPPOptions_GetTrace(option), VSC_OPTN_SCPPOptions_TRACE_DETAIL))
                        {
                            VIR_LOG(VIR_SCPP_GetDumper(scpp), "according to mov instruction:\n");
                            VIR_Inst_Dump(VIR_SCPP_GetDumper(scpp), instIter);
                            VIR_LOG(VIR_SCPP_GetDumper(scpp), "update symbol(%d)'s copy to:\n", VIR_Symbol_GetIndex(lhsSym));
                            _VIR_SCPP_Copy_Dump(copy, VIR_SCPP_GetDumper(scpp));
                            VIR_LOG_FLUSH(VIR_SCPP_GetDumper(scpp));
                        }
                    }
                }
            }
        }

        if (instIter == BB_GET_END_INST(bb))
        {
            break;
        }
        else
        {
            instIter = VIR_Inst_GetNext(instIter);
        }
    }

    /* destroy the hash table and free memory */
    {
        VSC_HASH_ITERATOR iter;
        VSC_DIRECT_HNODE_PAIR pair;

        vscHTBLIterator_Init(&iter, defsTable);
        for(pair = vscHTBLIterator_DirectFirst(&iter);
            IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
        {
            copy = (VIR_SCPP_Copy*)VSC_DIRECT_HNODE_PAIR_SECOND(&pair);
            vscMM_Free(VIR_SCPP_GetMM(scpp), copy);
        }
        vscHTBL_Destroy(defsTable);
    }

    if (VSC_UTILS_MASK(VSC_OPTN_SCPPOptions_GetTrace(option), VSC_OPTN_SCPPOptions_TRACE_OUTPUT_BB))
    {
        VIR_LOG(VIR_SCPP_GetDumper(scpp), "bb after scpp from mov:\n");
        VIR_BasicBlock_Dump(VIR_SCPP_GetDumper(scpp), bb, gcvFALSE);
    }

    if (bChanged)
    {
        VIR_SCPP_SetIsChanged(scpp, gcvTRUE);
    }

    OnError:
    return errCode;
}

VSC_ErrCode VIR_SCPP_PerformOnFunction(
    VIR_SCPP* scpp,
    VIR_Function* func
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    VIR_CONTROL_FLOW_GRAPH* cfg;
    CFG_ITERATOR cfgIter;
    VIR_BB* bb;
    VSC_OPTN_SCPPOptions* option = VIR_SCPP_GetOptions(scpp);

    if (VSC_UTILS_MASK(VSC_OPTN_SCPPOptions_GetTrace(option), VSC_OPTN_SCPPOptions_TRACE_INPUT_FUNC))
    {
        VIR_LOG(VIR_SCPP_GetDumper(scpp), "function before scpp:\n");
        VIR_Function_Dump(VIR_SCPP_GetDumper(scpp), func);
    }

    cfg = VIR_Function_GetCFG(func);
    CFG_ITERATOR_INIT(&cfgIter, cfg);
    for(bb = CFG_ITERATOR_FIRST(&cfgIter); bb != gcvNULL; bb = CFG_ITERATOR_NEXT(&cfgIter))
    {
        if(BB_GET_LENGTH(bb) != 0)
        {
            errCode = VIR_SCPP_PerformOnBB(scpp, func, bb);
        }

        if(errCode)
        {
            return errCode;
        }
    }

    if(VSC_UTILS_MASK(VSC_OPTN_SCPPOptions_GetTrace(option), VSC_OPTN_SCPPOptions_TRACE_OUTPUT_FUNC))
    {
        VIR_LOG(VIR_SCPP_GetDumper(scpp), "function after scpp:\n");
        VIR_Function_Dump(VIR_SCPP_GetDumper(scpp), func);
    }

    return errCode;
}

VSC_ErrCode VIR_SCPP_PerformOnShader(
    VIR_SCPP* scpp
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    VIR_FuncIterator funcIter;
    VIR_FunctionNode* funcNode;

    VIR_FuncIterator_Init(&funcIter, VIR_Shader_GetFunctions(VIR_SCPP_GetShader(scpp)));
    for(funcNode = VIR_FuncIterator_First(&funcIter);
        funcNode != gcvNULL; funcNode = VIR_FuncIterator_Next(&funcIter))
    {
        VIR_Function* func = funcNode->function;

        errCode = VIR_SCPP_PerformOnFunction(scpp, func);
        if(errCode)
        {
            break;
        }
    }

    return errCode;
}

DEF_QUERY_PASS_PROP(VSC_SCPP_PerformOnShader)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_LL;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_SCPP;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;

    pPassProp->passFlag.resCreationReq.s.bNeedCfg = gcvTRUE;

    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
}

DEF_SH_NECESSITY_CHECK(VSC_SCPP_PerformOnShader)
{
    return gcvTRUE;
}

VSC_ErrCode VSC_SCPP_PerformOnShader(
    VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    VIR_SCPP scpp;
    VIR_Shader* shader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VSC_OPTN_SCPPOptions* scppOptions = (VSC_OPTN_SCPPOptions*)pPassWorker->basePassWorker.pBaseOption;

    if(!VSC_OPTN_InRange(VIR_Shader_GetId(shader), VSC_OPTN_SCPPOptions_GetBeforeShader(scppOptions), VSC_OPTN_SCPPOptions_GetAfterShader(scppOptions)))
    {
        if(VSC_OPTN_SCPPOptions_GetTrace(scppOptions))
        {
            VIR_Dumper* dumper = pPassWorker->basePassWorker.pDumper;
            VIR_LOG(dumper, "Simple Copy Propagation skip shader(%d)\n", VIR_Shader_GetId(shader));
            VIR_LOG_FLUSH(dumper);
        }
        return errCode;
    }
    else
    {
        if(VSC_OPTN_SCPPOptions_GetTrace(scppOptions))
        {
            VIR_Dumper* dumper = pPassWorker->basePassWorker.pDumper;
            VIR_LOG(dumper, "Simple Copy Propagation start for shader(%d)\n", VIR_Shader_GetId(shader));
            VIR_LOG_FLUSH(dumper);
        }
    }

    if (VSC_UTILS_MASK(VSC_OPTN_SCPPOptions_GetTrace(scppOptions), VSC_OPTN_SCPPOptions_TRACE_INPUT_SHADER))
    {
        VIR_Shader_Dump(gcvNULL, "Before Simple Copy Propagation.", shader, gcvTRUE);
    }

    VIR_SCPP_Init(&scpp, pPassWorker->pDuInfo, shader, scppOptions, pPassWorker->basePassWorker.pDumper, pPassWorker->basePassWorker.pMM);
    errCode = VIR_SCPP_PerformOnShader(&scpp);
    VIR_SCPP_Final(&scpp);

    /* invalid data flow info only when code changed */
    pPassWorker->pResDestroyReq->s.bInvalidateCfg = VIR_SCPP_GetIsChanged(&scpp);

    if (VIR_SCPP_GetIsChanged(&scpp) &&
        (VSC_UTILS_MASK(VSC_OPTN_SCPPOptions_GetTrace(scppOptions), VSC_OPTN_SCPPOptions_TRACE_OUTPUT_SHADER) ||
         VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(shader), VIR_Shader_GetId(shader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE)))
    {
        VIR_Shader_Dump(gcvNULL, "After Simple Copy Propagation.", shader, gcvTRUE);
    }

    return errCode;
}

