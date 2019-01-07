/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
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
    return (gctUINT)(gctUINTPTR_T)((VSC_CPP_Usage*)ptr)->inst & 0xff;
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
    pUsage->inst = usage->usageKey.pUsageInst;
    pUsage->opnd = usage->usageKey.pOperand;
    return pUsage;
}

static void VSC_CPP_Init(
    IN OUT VSC_CPP_CopyPropagation  *cpp,
    IN VIR_Shader                   *shader,
    IN VIR_DEF_USAGE_INFO           *du_info,
    IN VSC_OPTN_CPPOptions          *options,
    IN VSC_CPP_PASS_DATA            *passData,
    IN VIR_Dumper                   *dumper,
    IN VSC_MM                       *pMM
    )
{
    VSC_CPP_SetShader(cpp, shader);
    VSC_CPP_SetCurrBB(cpp, gcvNULL);
    VSC_CPP_SetDUInfo(cpp, du_info);
    VSC_CPP_SetOptions(cpp, options);
    VSC_CPP_SetPassData(cpp, passData);
    VSC_CPP_SetDumper(cpp, dumper);
    VSC_CPP_SetFWOptCount(cpp, 0);
    VSC_CPP_SetBWOptCount(cpp, 0);

    cpp->pMM = pMM;
}

static void VSC_CPP_Final(
    IN OUT VSC_CPP_CopyPropagation  *cpp
    )
{
    VSC_CPP_SetShader(cpp, gcvNULL);
    VSC_CPP_SetOptions(cpp, gcvNULL);
    VSC_CPP_SetDumper(cpp, gcvNULL);
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
    errCode = VIR_Function_DeleteInstruction(func, defInst);

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

static VSC_ErrCode _VSC_CPP_CopyFromMOVOnOperand(
    IN OUT VSC_CPP_CopyPropagation  *cpp,
    IN     VIR_Instruction          *inst,
    IN     VIR_Operand              *srcOpnd,
    IN     VIR_Operand              *parentSrcOpnd, /* For texldParm or parameter only. */
    IN     gctUINT                  srcNum
    )
{
    VSC_ErrCode     errCode  = VSC_ERR_NONE;
    VIR_Shader      *shader = VSC_CPP_GetShader(cpp);
    VIR_Function    *func = VIR_Shader_GetCurrentFunction(shader);

    VSC_OPTN_CPPOptions *options = VSC_CPP_GetOptions(cpp);

    VIR_OpCode      instOpcode = VIR_Inst_GetOpcode(inst);
    VIR_Operand     *instDst = VIR_Inst_GetDest(inst);

    VIR_GENERAL_UD_ITERATOR udIter;
    VIR_DEF         *pDef;
    gctBOOL         bCopyFromOutputParam = (VSC_CPP_GetFlag(cpp) & VSC_CPP_COPY_FROM_OUTPUT_PARAM);

    do
    {
        VIR_Enable          srcEnable = VIR_Swizzle_2_Enable(
                                            VIR_Operand_GetSwizzle(srcOpnd));
        VIR_OperandInfo     srcInfo;

        VIR_Operand_GetOperandInfo(inst, srcOpnd, &srcInfo);
        if (srcInfo.isImmVal || srcInfo.isVecConst)
        {
            continue;
        }

        if (VIR_Operand_GetModifier(srcOpnd) ||
            VIR_Operand_GetRoundMode(srcOpnd))
        {
             continue;
        }

        /* cannot copy to temp256 pair, otherwise it will break the assumption that
         * register pair will be contiguous*/
        if (VIR_Operand_isTemp256High(srcOpnd) ||
            VIR_Operand_isTemp256Low(srcOpnd))
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

        vscVIR_InitGeneralUdIterator(&udIter, VSC_CPP_GetDUInfo(cpp), inst, srcOpnd, gcvFALSE, gcvFALSE);
        for (pDef = vscVIR_GeneralUdIterator_First(&udIter); pDef != gcvNULL;
             pDef = vscVIR_GeneralUdIterator_Next(&udIter))
        {
            VIR_Instruction     *defInst = pDef->defKey.pDefInst;
            gctUINT8             channel;

            if (defInst &&
                (!VIR_IS_IMPLICIT_DEF_INST(defInst) &&
                 defInst != VIR_UNDEF_INST) &&
                 (VIR_Inst_GetOpcode(defInst) == VIR_OP_MOV ||
                  VIR_Inst_GetOpcode(defInst) == VIR_OP_COPY) &&
                 /* this inst's srcOpnd only has one MOV def */
                 vscVIR_IsUniqueDefInstOfUsageInst(
                    VSC_CPP_GetDUInfo(cpp),
                    inst,
                    srcOpnd,
                    gcvFALSE,
                    defInst,
                    gcvNULL)
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

                if(!VIR_Inst_isComponentwise(inst))
                {
                    if ((instOpcode == VIR_OP_DP2) ||
                        (instOpcode == VIR_OP_NORM_DP2))
                    {
                        instEnable = VIR_ENABLE_XY;
                    }
                    else if ((instOpcode == VIR_OP_DP3) ||
                        (instOpcode == VIR_OP_NORM_DP3))
                    {
                        instEnable = VIR_ENABLE_XYZ;
                    }
                    else if ((instOpcode == VIR_OP_DP4) ||
                        (instOpcode == VIR_OP_NORM_DP4) ||
                        (instOpcode == VIR_OP_JMPC) ||
                        (instOpcode == VIR_OP_ATOMCMPXCHG))
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

                if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                                  VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
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

                /* mov src is constant. do very limited constant propagation */
                if (movSrcInfo.isImmVal || movSrcInfo.isVecConst)
                {
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
                    VIR_Operand_SetLShift(newSrc, VIR_Operand_GetLShift(srcOpnd));
                    VIR_Operand_SetTypeId(newSrc, VIR_Operand_GetTypeId(srcOpnd));
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
                    }
                    else
                    {
                        gcmASSERT(movSrcInfo.isImmVal);
                        VIR_Operand_SetSwizzle(newSrc, VIR_SWIZZLE_XXXX);
                    }

                    /* Replace the source. */
                    _VSC_CPP_ReplaceSource(cpp, inst, parentSrcOpnd, srcNum, newSrc);

                    /* change the immediate to EvisModifier for EVIS inst if it is modifier operand */
                    if (VIR_OPCODE_isVXOnly(instOpcode))
                    {
                        int evisSrcNo = VIR_OPCODE_EVISModifier_SrcNo(instOpcode);
                        gcmASSERT(evisSrcNo >= 0 && (gctUINT)evisSrcNo < VIR_Inst_GetSrcNum(inst));

                        if (evisSrcNo == (int)srcNum)
                        {
                            /* set newSrc to EVISModifier operand */
                            VIR_Operand_SetOpKind(newSrc, VIR_OPND_EVIS_MODIFIER);
                        }
                    }

                    _VSC_CPP_RemoveDefInst(cpp, defInst);
                }
                else
                {
                    /* In the IR, there exists implict type conversion, thus we need
                       to bail out if the types mismatch */
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

                    if  (VIR_Inst_GetFunction(inst) != VIR_Inst_GetFunction(defInst))
                    {
                     /* define and user must in same function */
                        if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                                          VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
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
                        if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                                          VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                        {
                            VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                            VIR_LOG(dumper, "[FW] ==> bail out: because of different precision in dual16 mode");
                            VIR_LOG_FLUSH(dumper);
                        }
                        break;
                    }

                    if  (VIR_Inst_GetOpcode(defInst) == VIR_OP_COPY)
                    {
                        if (!VIR_Symbol_isImage(VIR_Operand_GetSymbol(defInst->src[0])) &&
                            !VIR_Symbol_isUniform(VIR_Operand_GetSymbol(defInst->src[0])) &&
                            !VIR_Symbol_isSampler(VIR_Operand_GetSymbol(defInst->src[0])))
                        {
                            /* mov's dest is per patch output */
                            if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                                              VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                            {
                                VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                                VIR_LOG(dumper, "[FW] ==> bail out: copy's src is not uniform");
                                VIR_LOG_FLUSH(dumper);
                            }
                            break;
                        }
                    }

                    if  (VIR_Operand_IsPerPatch(defInst->dest))
                    {
                        /* mov's dest is per patch output */
                        if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                                          VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                        {
                            VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                            VIR_LOG(dumper, "[FW] ==> bail out: mov's dest is per patch output");
                            VIR_LOG_FLUSH(dumper);
                        }
                        break;
                    }

                    if(!(((VIR_GetTypeFlag(ty0) & VIR_TYFLAG_ISFLOAT) &&
                        (VIR_GetTypeFlag(ty1) & VIR_TYFLAG_ISFLOAT)) ||
                        (((VIR_GetTypeFlag(ty0) & VIR_TYFLAG_ISINTEGER) ||
                          (VIR_GetTypeTypeKind(ty0) == VIR_TY_IMAGE)) &&
                         ((VIR_GetTypeFlag(ty1) & VIR_TYFLAG_ISINTEGER) ||
                          (VIR_GetTypeTypeKind(ty1) == VIR_TY_IMAGE))) ||
                        (VIR_GetTypeTypeKind(ty1) == VIR_TY_SAMPLER &&
                         (VIR_GetTypeFlag(ty0) & VIR_TYFLAG_ISINTEGER)) ||
                        ((ty0 == ty1) && (VIR_GetTypeFlag(ty0) & VIR_TYFLAG_IS_SAMPLER))))
                    {
                        if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                                          VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                        {
                            VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                            VIR_LOG(dumper, "[FW] ==> bail out because of not same type");
                            VIR_LOG_FLUSH(dumper);
                        }
                        break;
                    }

                    if (VIR_Operand_GetOpKind(movDst) != VIR_OPND_SYMBOL ||
                        VIR_Operand_GetOpKind(movSrc) != VIR_OPND_SYMBOL)
                    {
                        if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                                          VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
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

                    /*  if there is a use that has def other than mov, this mov
                        could not be removed. This copy could increase the live range of movSrc.
                    */
                    {
                        VIR_USAGE               *pUsage;
                        VSC_DU_ITERATOR         duIter;
                        VIR_DU_CHAIN_USAGE_NODE *pUsageNode;
                        VIR_Instruction         *pUseInst;
                        gctBOOL                 invalidCase = gcvFALSE;

                        /* go through all the uses */
                        VSC_DU_ITERATOR_INIT(&duIter, &pDef->duChain);
                        pUsageNode = VSC_DU_ITERATOR_FIRST(&duIter);
                        for (; pUsageNode != gcvNULL; pUsageNode = VSC_DU_ITERATOR_NEXT(&duIter))
                        {
                            pUsage = GET_USAGE_BY_IDX(&VSC_CPP_GetDUInfo(cpp)->usageTable, pUsageNode->usageIdx);
                            pUseInst = pUsage->usageKey.pUsageInst;

                            if (pUseInst == VIR_OUTPUT_USAGE_INST ||
                                VIR_Inst_GetOpcode(pUseInst) == VIR_OP_EMIT ||
                                VIR_Inst_GetOpcode(pUseInst) == VIR_OP_EMIT0) /* skip copy if usage is emit */
                            {
                                invalidCase = gcvTRUE;
                                break;
                            }
                            if(!vscVIR_IsUniqueDefInstOfUsageInst(
                                        VSC_CPP_GetDUInfo(cpp),
                                        pUseInst,
                                        pUsage->usageKey.pOperand,
                                        pUsage->usageKey.bIsIndexingRegUsage,
                                        defInst,
                                        gcvNULL))
                            {
                                invalidCase = gcvTRUE;
                                break;
                            }
                        }
                        if (invalidCase)
                        {
                            if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                                              VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                            {
                                VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                                VIR_LOG(dumper, "[FW] ==> bail out because one use could not be replaced\n");
                                VIR_LOG_FLUSH(dumper);
                            }
                            break;
                        }
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
                    if (movSrcInfo.isInput &&
                        VIR_Inst_GetOpcode(inst) == VIR_OP_LDARR)
                    {
                        if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                                          VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
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
                        if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                                          VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                        {
                            VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                            VIR_LOG(dumper, "[FW] ==> bail out because LDARR index has to be temp ");
                            VIR_LOG_FLUSH(dumper);
                        }
                        break;

                    }

                    /* we could move between different precison  */

                     /* instruction has modifier
                        TODO: relax
                     */
                    if (VIR_Operand_GetModifier(movDst) ||
                        VIR_Operand_GetRoundMode(movDst) ||
                        VIR_Operand_GetModifier(movSrc) ||
                        VIR_Operand_GetRoundMode(movSrc))
                    {
                        if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                                          VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                        {
                            VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                            VIR_LOG(dumper, "[FW] ==> bail out because of modifier");
                            VIR_LOG_FLUSH(dumper);
                        }
                        break;
                    }

                    /* inst and its def in the same bb */
                    if(VIR_Inst_GetBasicBlock(inst) == VIR_Inst_GetBasicBlock(defInst))
                    {
                        /* no redefine of movSrc between defInst and inst */
                        VIR_Instruction* next = VIR_Inst_GetNext(defInst);
                        gctBOOL         invalidCase = gcvFALSE;
                        while(next != inst)
                        {
                            if(VIR_Operand_SameLocation(defInst, movSrc, next, VIR_Inst_GetDest(next)) ||
                               VIR_Inst_GetOpcode(next) == VIR_OP_CALL)
                            {
                                if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                                              VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
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
                             VIR_Symbol_isImage(VIR_Operand_GetSymbol(movSrc))))
                        {
                            if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                                                  VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
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
                                VSC_HASH_TABLE  *visitSet = gcvNULL;
                                visitSet = vscHTBL_Create(VSC_CPP_GetMM(cpp),
                                    vscHFUNC_Default, vscHKCMP_Default, 512);
                                if (!(movSrcInfo.isInput ||
                                      VIR_Symbol_isUniform(VIR_Operand_GetSymbol(movSrc)) ||
                                      VIR_Symbol_isSampler(VIR_Operand_GetSymbol(movSrc)) ||
                                      VIR_Symbol_isImage(VIR_Operand_GetSymbol(movSrc))     ) &&
                                    _VSC_CPP_CallInstInBetween(defInst, inst, visitSet))
                                {
                                    if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                                                      VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                                    {
                                        VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                                        VIR_LOG(dumper, "[FW] ==> bail out because of call\n");
                                        VIR_LOG_FLUSH(dumper);
                                    }
                                    vscHTBL_Destroy(visitSet);
                                    break;
                                }
                                vscHTBL_Destroy(visitSet);
                            }

                            {
                                /* no redefine of movSrc between defInst and inst */
                                VIR_Instruction *redefInst = gcvNULL;
                                if (!(VIR_Symbol_isUniform(VIR_Operand_GetSymbol(movSrc)) ||
                                      VIR_Symbol_isSampler(VIR_Operand_GetSymbol(movSrc)) ||
                                      VIR_Symbol_isImage(VIR_Operand_GetSymbol(movSrc))) &&
                                    vscVIR_RedefineBetweenInsts(VSC_CPP_GetMM(cpp), VSC_CPP_GetDUInfo(cpp),
                                        defInst, inst, movSrc, &redefInst))
                                {
                                    if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                                                      VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
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

                        if (!_VSC_CPP_CopySrcTypeFromMov(cpp, inst, defInst, newSrc))
                        {
                            VIR_Operand_SetTypeId(newSrc, ty);
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

                if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                                  VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
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
    IN     VIR_Instruction          *inst
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
                    errCode = _VSC_CPP_CopyFromMOVOnOperand(cpp, inst, parm->args[i], srcOpnd, i);
                    ON_ERROR(errCode, "copy from MOV for a single operand");
                }
            }
        }
        else
        {
            errCode = _VSC_CPP_CopyFromMOVOnOperand(cpp, inst, srcOpnd, gcvNULL, srcNum);
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
    VIR_Swizzle     inst_src0_swizzle;
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

    gcmASSERT(VIR_Inst_GetOpcode(inst) == VIR_OP_MOV);

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

    /* TO-Do: relax here */
    for(channel = 0; channel < VIR_CHANNEL_COUNT; ++channel)
    {
        if(inst_dest_enable & (1 << channel))
        {
            if((VIR_Swizzle)channel != VIR_Swizzle_GetChannel(inst_src0_swizzle, channel))
            {
                invalid_case = gcvTRUE;
                break;
            }
        }
    }

    if(invalid_case)
    {
        if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                          VSC_OPTN_CPPOptions_TRACE_BACKWARD_OPT))
        {
            VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
            VIR_LOG(dumper, "[BW] ==> bail out because inst_dest's enable dose not match with inst_src0's swizzle.\n");
            VIR_LOG_FLUSH(dumper);
        }
        return errCode;
    }

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

    for(; def != gcvNULL && (!invalid_case);  def = vscVIR_GeneralUdIterator_Next(&ud_iter))
    {
        VIR_OpCode opcode;

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
        case VIR_OP_MOV:
        case VIR_OP_ADD:
        case VIR_OP_SUB:
        case VIR_OP_MUL:
        case VIR_OP_DIV:
        case VIR_OP_MOD:
        case VIR_OP_REM:
        case VIR_OP_MAD:
        case VIR_OP_CSELECT:
        case VIR_OP_STEP:
        case VIR_OP_AND_BITWISE:
        case VIR_OP_OR_BITWISE:
        case VIR_OP_XOR_BITWISE:
        case VIR_OP_NOT_BITWISE:
        case VIR_OP_LSHIFT:
        case VIR_OP_RSHIFT:
        case VIR_OP_ROTATE:
        case VIR_OP_LDARR:
        case VIR_OP_VX_ICASTP:
        case VIR_OP_VX_ICASTD:
        case VIR_OP_DP4:
            break;

        default:
            /* Should NOT do backward CPP for TEXLD.
               For example,
               TEXLD t1.xyzw, sampler2D t3, t4.x,
               MOV t2.x t1.x  ==> only use one component

               We should NOT change it to
               TEXLD t1.x, sampler2D t3, t4.x,
               Since TEXLD generates a vec4 */

            invalid_case = gcvTRUE;
            break;
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

        /* inst_enable should cover def_dest_enable */
        if(!VIR_Enable_Covers(inst_dest_enable, def_inst_enable))
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

        /* inst and its def should be in the same bb */
        /* TO-Do: relax here */
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
        */
        /* there should be no use of inst_dest between def_inst and inst */
        {
            VIR_Instruction* next = VIR_Inst_GetNext(def_inst);
            gctSIZE_T i = 0;

            while(next && next != inst)
            {
                for(i = 0; i < VIR_Inst_GetSrcNum(next); ++i)
                {
                    if(VIR_Operand_SameLocation(inst, inst_dest, next, VIR_Inst_GetSource(next, i)))
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

        if (!covered_channels[channel])
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
                    channelMask = (gctUINT*)vscMM_Alloc(VSC_CPP_GetMM(cpp), sizeof(gctUINT));
                    *channelMask = (1<<channel);
                    vscHTBL_DirectSet(inst_usage_set, (void*)_VSC_CPP_NewUsage(cpp, inst_usage), channelMask);
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
                    def_inst_enable);
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
        VIR_Function_DeleteInstruction(func, inst);

        vscHTBL_Reset(inst_usage_set);
    }

    vscHTBL_Reset(inst_def_set);

    return errCode;
}

static VSC_ErrCode _VSC_CPP_CopyPropagationForBB(
    IN OUT VSC_CPP_CopyPropagation  *cpp,
    IN     VSC_HASH_TABLE           *inst_def_set,
    IN     VSC_HASH_TABLE           *inst_usage_set
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
            _VSC_CPP_CopyFromMOV(cpp, inst);
        }

        if (VIR_Inst_GetOpcode(inst) == VIR_OP_MOV)
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
    VSC_HASH_TABLE          *inst_usage_set     = gcvNULL;

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
        /* create two hash tables here */
        inst_def_set = vscHTBL_Create(VSC_CPP_GetMM(cpp), vscHFUNC_Default, vscHKCMP_Default, 512);
        inst_usage_set = vscHTBL_Create(VSC_CPP_GetMM(cpp), _VSC_CPP_Usage_HFUNC, _VSC_CPP_Usage_HKCMP, 512);

        for(bb = CFG_ITERATOR_FIRST(&cfg_iter); bb != gcvNULL; bb = CFG_ITERATOR_NEXT(&cfg_iter))
        {
            if(BB_GET_LENGTH(bb) != 0)
            {
                VSC_CPP_SetCurrBB(cpp, bb);
                errCode = _VSC_CPP_CopyPropagationForBB(cpp, inst_def_set, inst_usage_set);
            }

            if(errCode)
            {
                return errCode;
            }
        }

        func->instList.pHead = func->instList.pHead->parent.BB->pStartInst;
        func->instList.pTail = func->instList.pTail->parent.BB->pEndInst;

        /*destroy two hash tables */
        vscHTBL_Destroy(inst_def_set);
        vscHTBL_Destroy(inst_usage_set);
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

VSC_ErrCode _ConvEvisInstForShader(
    IN VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode         errCode  = VSC_ERR_NONE;
    VIR_Shader         *pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;

    VIR_FuncIterator func_iter;
    VIR_FunctionNode* func_node;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for(func_node = VIR_FuncIterator_First(&func_iter);
        func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function     *func = func_node->function;
        VIR_InstIterator  inst_iter;
        VIR_Instruction  *pInst;
        VIR_Instruction  *pMovConstBorderInst = gcvNULL;
        VIR_VirRegId      constBorderColorRegId = VIR_INVALID_ID;  /* constant border color temp reg id */

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
        for (pInst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             pInst != gcvNULL; pInst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            VIR_OpCode          opCode = VIR_Inst_GetOpcode(pInst);
            VIR_Operand        *pSrc2Opnd = VIR_Inst_GetSource(pInst, 2);
            VIR_DEF_USAGE_INFO *pDuInfo = pPassWorker->pDuInfo;
            VIR_Instruction    *pMovInst = gcvNULL;
            VIR_VirRegId        regId;                     /* newly created temp reg id */
            VIR_SymId           regSymId;
            VIR_TypeId          regTypeId;
            /*
            ** For some EVIS instructions, HW doesn't decode src2's swizzle if src2 it is not a IMMEDIATE,
            ** so we insert a MOV instruction whose enable is XYZW to replace the src2.
            */
            if (pSrc2Opnd                       &&
                VIR_OPCODE_isVX(opCode)         &&
                opCode != VIR_OP_VX_IACCSQ      &&
                opCode != VIR_OP_VX_LERP        &&
                opCode != VIR_OP_VX_MULSHIFT    &&
                opCode != VIR_OP_VX_BILINEAR    &&
                opCode != VIR_OP_VX_ATOMICADD
                )
            {
                VIR_TypeId       src2TypeId = VIR_Operand_GetTypeId(pSrc2Opnd);
                VIR_Swizzle      src2Swizzle = VIR_Operand_GetSwizzle(pSrc2Opnd);

                if ((VIR_Operand_isVirReg(pSrc2Opnd) || VIR_Operand_isSymbol(pSrc2Opnd) || VIR_Operand_isConst(pSrc2Opnd))
                    &&
                    src2Swizzle != VIR_SWIZZLE_XYZW)
                {
                    VIR_Operand             * pOpnd = gcvNULL;
                    VIR_OperandInfo           srcInfo;
                    VIR_GENERAL_UD_ITERATOR   udIter;
                    VIR_DEF*                  pDef;
                    VIR_Symbol              * src2Sym = VIR_Operand_isSymbol(pSrc2Opnd) ?
                                                            VIR_Operand_GetSymbol(pSrc2Opnd) : gcvNULL;
                    gctBOOL                   useConstBorderColor = gcvFALSE;

                    /* special optimization for constant border color */
                    if (opCode == VIR_OP_VX_CLAMP && src2Sym && VIR_Symbol_isUniform(src2Sym) &&
                        VIR_Symbol_GetUniformKind(src2Sym) == VIR_UNIFORM_CONST_BORDER_VALUE)
                    {
                        useConstBorderColor = gcvTRUE;
                        if (pMovConstBorderInst != gcvNULL)
                        {
                            /* already created mov constant border color Inst, use it */
                            /* Update the SRC2 of EVIS instruction. */
                            VIR_Operand_Copy(pSrc2Opnd, VIR_Inst_GetDest(pMovConstBorderInst));
                            VIR_Operand_SetLvalue(pSrc2Opnd, gcvFALSE);
                            VIR_Operand_SetSwizzle(pSrc2Opnd, VIR_SWIZZLE_XYZW);

                            /* update the usage of new temp */
                            vscVIR_AddNewUsageToDef(pDuInfo, pMovConstBorderInst, pInst,
                                                    pSrc2Opnd,
                                                    gcvFALSE, constBorderColorRegId, 1, VIR_ENABLE_XYZW,
                                                    VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
                            continue;
                        }
                    }
                    /* Create a temp to hold the src2. */
                    regTypeId = src2TypeId;
                    regId = VIR_Shader_NewVirRegId(pShader, 1);
                    errCode = VIR_Shader_AddSymbol(pShader,
                                                   VIR_SYM_VIRREG,
                                                   regId,
                                                   VIR_Shader_GetTypeFromId(pShader, regTypeId),
                                                   VIR_STORAGE_UNKNOWN,
                                                   &regSymId);
                    ON_ERROR(errCode, "Add symbol failed");

                    /* Insert a MOV. */
                    errCode = VIR_Function_AddInstructionBefore(
                                    func,
                                    VIR_OP_MOV,
                                    regTypeId,
                                    useConstBorderColor ? VIR_Function_GetInstStart(func) : pInst,
                                    gcvTRUE,
                                    &pMovInst);
                    ON_ERROR(errCode, "Insert instruction failed");

                    if (useConstBorderColor)
                    {
                        constBorderColorRegId = regId;
                        pMovConstBorderInst = pMovInst;
                    }

                    /* Set DEST. */
                    pOpnd = VIR_Inst_GetDest(pMovInst);
                    VIR_Operand_SetTempRegister(pOpnd,
                                                func,
                                                regSymId,
                                                regTypeId);
                    VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_XYZW);

                    /* Set SRC0. */
                    pOpnd = VIR_Inst_GetSource(pMovInst, 0);
                    VIR_Operand_Copy(pOpnd, pSrc2Opnd);

                    /* update du info for new MOV instruction */
                    vscVIR_AddNewDef(pDuInfo,
                                        pMovInst,
                                        regId,
                                        1,
                                        VIR_ENABLE_XYZW,
                                        VIR_HALF_CHANNEL_MASK_FULL,
                                        gcvNULL,
                                        gcvNULL);

                    /* find the def of pOpnd and update it usage */
                    VIR_Operand_GetOperandInfo(pInst, pSrc2Opnd, &srcInfo);

                    vscVIR_InitGeneralUdIterator(&udIter, pDuInfo, pInst, pSrc2Opnd, gcvFALSE, gcvFALSE);

                    for (pDef = vscVIR_GeneralUdIterator_First(&udIter);
                            pDef != gcvNULL;
                            pDef = vscVIR_GeneralUdIterator_Next(&udIter))
                    {
                        vscVIR_AddNewUsageToDef(pDuInfo,
                                                pDef->defKey.pDefInst,
                                                pMovInst,
                                                pOpnd,
                                                gcvFALSE,
                                                srcInfo.u1.virRegInfo.virReg,
                                                1,
                                                1 << pDef->defKey.channel,
                                                VIR_HALF_CHANNEL_MASK_FULL,
                                                gcvNULL);
                    }

                    /* Update the SRC2 of EVIS instruction. */
                    VIR_Operand_Copy(pSrc2Opnd, VIR_Inst_GetDest(pMovInst));
                    VIR_Operand_SetLvalue(pSrc2Opnd, gcvFALSE);
                    VIR_Operand_SetSwizzle(pSrc2Opnd, VIR_SWIZZLE_XYZW);

                    /* update the usage of new temp */
                    vscVIR_AddNewUsageToDef(pDuInfo, pMovInst, pInst,
                                            pSrc2Opnd,
                                            gcvFALSE, regId, 1, VIR_ENABLE_XYZW,
                                            VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
                }
            }
        }
    }
OnError:
    return errCode;
}

DEF_QUERY_PASS_PROP(VSC_CPP_PerformOnShader)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_LL |
                                 VSC_PASS_LEVEL_MC;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_CPP;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;

    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
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

    if (pPassWorker->basePassWorker.pPrvData)
    {
        cppPassData = *(VSC_CPP_PASS_DATA*)pPassWorker->basePassWorker.pPrvData;
    }

    VSC_CPP_Init(&cpp, shader, pPassWorker->pDuInfo, cpp_options, &cppPassData,
                 pPassWorker->basePassWorker.pDumper, pPassWorker->basePassWorker.pMM);

    /* don't perform global CPP when the cfg has too many nodes*/
    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(shader));
    for(func_node = VIR_FuncIterator_First(&func_iter);
        func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function    *func = func_node->function;

        if (VIR_Function_GetCFG(func)->dgGraph.nodeList.info.count > 1000 ||
            VIR_Function_GetInstCount(func) > 3400)
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

    /*
    ** For some EVIS instructions, HW doesn't decode src2's swizzle if src2 it is not a IMMEDIATE,
    ** so we insert a MOV instruction whose enable is XYZW to replace the src2.
    **
    ** We have to insert the MOV after copy propagation, otherwise it will be reversed by CPP
    ** put the phase here is to make sure every time CPP is called, ConvEvisInstForShader() is
    ** called again.
    */
    errcode = _ConvEvisInstForShader(pPassWorker);
    ON_ERROR(errcode, "Convert evis instruction");


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

OnError:
    return errcode;
}


/* simple local copy propagation */

typedef struct VIR_SCPP
{
    VIR_Shader              *shader;
    VSC_OPTN_SCPPOptions    *options;
    VIR_Dumper              *dumper;
    VSC_MM                  *mm;
} VIR_SCPP;

#define VIR_SCPP_GetShader(scpp)                ((scpp)->shader)
#define VIR_SCPP_SetShader(scpp, s)             ((scpp)->shader = (s))
#define VIR_SCPP_GetOptions(scpp)               ((scpp)->options)
#define VIR_SCPP_SetOptions(scpp, o)            ((scpp)->options = (o))
#define VIR_SCPP_GetDumper(scpp)                ((scpp)->dumper)
#define VIR_SCPP_SetDumper(scpp, d)             ((scpp)->dumper = (d))
#define VIR_SCPP_GetMM(scpp)                    ((scpp)->mm)
#define VIR_SCPP_SetMM(scpp, m)                 ((scpp)->mm = (m))

void
VIR_SCPP_Init(
    VIR_SCPP* scpp,
    VIR_Shader* shader,
    VSC_OPTN_SCPPOptions* options,
    VIR_Dumper* dumper,
    VSC_MM* mm
    )
{
    VIR_SCPP_SetShader(scpp, shader);
    VIR_SCPP_SetOptions(scpp, options);
    VIR_SCPP_SetDumper(scpp, dumper);
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
} VIR_SCPP_Copy;

#define VIR_SCPP_Copy_GetRhsSymId(d, i)                  ((d)->rhsSymId[i])
#define VIR_SCPP_Copy_SetRhsSymId(d, i, r)               ((d)->rhsSymId[i] = (r))
#define VIR_SCPP_Copy_GetMappingSwizzle(d)               ((d)->mappingSwizzle)
#define VIR_SCPP_Copy_SetMappingSwizzle(d, m)            ((d)->mappingSwizzle = (VIR_Swizzle)(m))
#define VIR_SCPP_Copy_GetSingleMappingSwizzle(d, i)      VIR_Swizzle_GetChannel((d)->mappingSwizzle, i)
#define VIR_SCPP_Copy_SetSingleMappingSwizzle(d, i, m)   VIR_Swizzle_SetChannel((d)->mappingSwizzle, i, (m))

void
_VIR_SCPP_Copy_Init(
    VIR_SCPP_Copy* copy
    )
{
    gctUINT i;

    for(i = 0; i < VIR_CHANNEL_NUM; i++)
    {
        VIR_SCPP_Copy_SetRhsSymId(copy, i, VIR_INVALID_ID);
    }
    VIR_SCPP_Copy_SetMappingSwizzle(copy, VIR_SWIZZLE_XYZW);
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
    return (gctUINT)(VIR_Symbol_GetIndex(sym) & 0xff);
}

static gctBOOL
_VIR_SCPP_SymbolCmpFunc(const void* pHashKey1, const void* pHashKey2)
{
    VIR_Symbol* sym1 = (VIR_Symbol*)pHashKey1;
    VIR_Symbol* sym2 = (VIR_Symbol*)pHashKey2;
    return VIR_Symbol_GetIndex(sym1) == VIR_Symbol_GetIndex(sym2);
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

                if(VIR_Operand_isSymbol(src))
                {
                    srcSym = VIR_Operand_GetSymbol(src);

                    if(vscHTBL_DirectTestAndGet(defsTable, (void*)srcSym, (void**)&copy))
                    {
                        VIR_SymId defSymId = _VIR_SCPP_Copy_GetRhs(copy, srcSwizzle);
                        VIR_Symbol* defSym;

                        if(defSymId != VIR_INVALID_ID)
                        {
                            if(VSC_UTILS_MASK(VSC_OPTN_SCPPOptions_GetTrace(option), VSC_OPTN_SCPPOptions_TRACE_DETAIL))
                            {
                                VIR_LOG(VIR_SCPP_GetDumper(scpp), "transform instruction:\n");
                                VIR_Inst_Dump(VIR_SCPP_GetDumper(scpp), instIter);
                            }

                            defSym = VIR_Function_GetSymFromId(func, defSymId);
                            srcSwizzle = VIR_Swizzle_ApplyMappingSwizzle(srcSwizzle, VIR_SCPP_Copy_GetMappingSwizzle(copy));

                            VIR_Operand_SetOpKind(src, VIR_OPND_SYMBOL);
                            VIR_Operand_SetSym(src, defSym);
                            VIR_Operand_SetPrecision(src, VIR_Symbol_GetPrecision(defSym));
                            VIR_Operand_SetSwizzle(src, srcSwizzle);

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

            if(!VIR_Symbol_IsInArray(destSym) &&
               VIR_Operand_isSymbol(src) &&
               !VIR_Symbol_IsInArray(VIR_Operand_GetSymbol(src)))
            {
                if(_VIR_SCPP_AbleToDoNeighborPropagation(shader, instIter))
                {
                    /* transform
                        028: ADD                int hp  temp(273).hp.x, int hp  temp(262).hp.x, int 1
                        029: MOV                int hp  temp(262).hp.x, int hp  temp(273).hp.x
                        to
                        036: ADD                int hp  temp(262).hp.x, int hp  temp(262).hp.x, int 1
                        to ease loop optimization */
                    VIR_Instruction* prevInst = VIR_Inst_GetPrev(instIter);
                    VIR_Instruction* newInst;

                    VIR_Function_AddCopiedInstructionAfter(func, prevInst, instIter, gcvTRUE, &newInst);
                    if(VSC_UTILS_MASK(VSC_OPTN_SCPPOptions_GetTrace(option), VSC_OPTN_SCPPOptions_TRACE_DETAIL))
                    {
                        VIR_LOG(VIR_SCPP_GetDumper(scpp), "propagation instruction:\n");
                        VIR_Inst_Dump(VIR_SCPP_GetDumper(scpp), prevInst);
                        VIR_LOG(VIR_SCPP_GetDumper(scpp), "and transform instruction:\n");
                        VIR_Inst_Dump(VIR_SCPP_GetDumper(scpp), instIter);
                    }
                    VIR_Operand_Copy(VIR_Inst_GetDest(newInst), VIR_Inst_GetDest(instIter));
                    VIR_Function_RemoveInstruction(func, instIter);
                    if(VSC_UTILS_MASK(VSC_OPTN_SCPPOptions_GetTrace(option), VSC_OPTN_SCPPOptions_TRACE_DETAIL))
                    {
                        VIR_LOG(VIR_SCPP_GetDumper(scpp), "to:\n");
                        VIR_Inst_Dump(VIR_SCPP_GetDumper(scpp), newInst);
                    }
                    instIter = newInst;

                    if(gcvTRUE)
                    {
                        VIR_Operand* dest = VIR_Inst_GetDest(instIter);
                        VIR_Operand* prevDest = VIR_Inst_GetDest(prevInst);
                        VIR_Symbol* lhsSym = VIR_Operand_GetSymbol(prevDest);
                        VIR_Symbol* rhsSym = VIR_Operand_GetSymbol(dest);

                        if(!vscHTBL_DirectTestAndGet(defsTable, (void*)lhsSym, (void**)&copy))
                        {
                            copy = (VIR_SCPP_Copy*)vscMM_Alloc(VIR_SCPP_GetMM(scpp), sizeof(VIR_SCPP_Copy));
                            _VIR_SCPP_Copy_Init(copy);
                            vscHTBL_DirectSet(defsTable, (void*)lhsSym, (void*)copy);
                        }

                        _VIR_SCPP_Copy_UpdateChannel(copy, VIR_SWIZZLE_X, VIR_Symbol_GetIndex(lhsSym), VIR_Symbol_GetIndex(rhsSym), VIR_SWIZZLE_X);

                        if(VSC_UTILS_MASK(VSC_OPTN_SCPPOptions_GetTrace(option), VSC_OPTN_SCPPOptions_TRACE_DETAIL))
                        {
                            VIR_LOG(VIR_SCPP_GetDumper(scpp), "according to new instruction:\n");
                            VIR_Inst_Dump(VIR_SCPP_GetDumper(scpp), newInst);
                            VIR_LOG(VIR_SCPP_GetDumper(scpp), "and its previous instruction:\n");
                            VIR_Inst_Dump(VIR_SCPP_GetDumper(scpp), prevInst);
                            VIR_LOG(VIR_SCPP_GetDumper(scpp), "update symbol(%d)'s copy to:\n", VIR_Symbol_GetIndex(lhsSym));
                            _VIR_SCPP_Copy_Dump(copy, VIR_SCPP_GetDumper(scpp));
                            VIR_LOG_FLUSH(VIR_SCPP_GetDumper(scpp));
                        }
                    }
                }
                else
                {
                    /* generate new copy */

                    VIR_Operand* dest = VIR_Inst_GetDest(instIter);
                    VIR_Symbol* lhsSym = VIR_Operand_GetSymbol(dest);
                    VIR_Symbol* rhsSym = VIR_Operand_GetSymbol(src);
                    VIR_Enable enable = VIR_Operand_GetEnable(dest);
                    VIR_Swizzle swizzle = VIR_Operand_GetSwizzle(src);
                    gctUINT i;

                    if(!vscHTBL_DirectTestAndGet(defsTable, (void*)lhsSym, (void**)&copy))
                    {
                        copy = (VIR_SCPP_Copy*)vscMM_Alloc(VIR_SCPP_GetMM(scpp), sizeof(VIR_SCPP_Copy));
                        _VIR_SCPP_Copy_Init(copy);
                        vscHTBL_DirectSet(defsTable, (void*)lhsSym, (void*)copy);
                    }

                    for(i = 0; i < VIR_CHANNEL_NUM; i++)
                    {
                        if(enable & 1 << i)
                        {
                            _VIR_SCPP_Copy_UpdateChannel(copy, i, VIR_Symbol_GetIndex(lhsSym), VIR_Symbol_GetIndex(rhsSym), VIR_Swizzle_GetChannel(swizzle, i));
                        }
                    }

                    if(VSC_UTILS_MASK(VSC_OPTN_SCPPOptions_GetTrace(option), VSC_OPTN_SCPPOptions_TRACE_DETAIL))
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

        if(instIter == BB_GET_END_INST(bb))
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

    if(VSC_UTILS_MASK(VSC_OPTN_SCPPOptions_GetTrace(option), VSC_OPTN_SCPPOptions_TRACE_OUTPUT_BB))
    {
        VIR_LOG(VIR_SCPP_GetDumper(scpp), "bb after scpp from mov:\n");
        VIR_BasicBlock_Dump(VIR_SCPP_GetDumper(scpp), bb, gcvFALSE);
    }

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

    if(VSC_UTILS_MASK(VSC_OPTN_SCPPOptions_GetTrace(option), VSC_OPTN_SCPPOptions_TRACE_INPUT_FUNC))
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

    VIR_SCPP_Init(&scpp, shader, scppOptions, pPassWorker->basePassWorker.pDumper, pPassWorker->basePassWorker.pMM);
    errCode = VIR_SCPP_PerformOnShader(&scpp);
    VIR_SCPP_Final(&scpp);

    if (VSC_UTILS_MASK(VSC_OPTN_SCPPOptions_GetTrace(scppOptions), VSC_OPTN_SCPPOptions_TRACE_OUTPUT_SHADER) ||
        VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(shader), VIR_Shader_GetId(shader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After Simple Copy Propagation.", shader, gcvTRUE);
    }

    return errCode;
}

