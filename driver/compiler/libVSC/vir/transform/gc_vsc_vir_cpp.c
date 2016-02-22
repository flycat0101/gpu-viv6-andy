/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
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

void VSC_CPP_Init(
    IN OUT VSC_CPP_CopyPropagation  *cpp,
    IN VIR_Shader                   *shader,
    IN VIR_DEF_USAGE_INFO           *du_info,
    IN VSC_OPTN_CPPOptions          *options,
    IN VIR_Dumper                   *dumper,
    IN gctBOOL                      globaCPP
    )
{
    VSC_CPP_SetShader(cpp, shader);
    VSC_CPP_SetCurrBB(cpp, gcvNULL);
    VSC_CPP_SetDUInfo(cpp, du_info);
    VSC_CPP_SetOptions(cpp, options);
    VSC_CPP_SetDumper(cpp, dumper);
    VSC_CPP_SetGlobalCPP(cpp, globaCPP);
    VSC_CPP_SetFWOptCount(cpp, 0);
    VSC_CPP_SetBWOptCount(cpp, 0);

    vscPMP_Intialize(VSC_CPP_GetPmp(cpp), gcvNULL, 1024,
                     sizeof(void*), gcvTRUE);
}

void VSC_CPP_Final(
    IN OUT VSC_CPP_CopyPropagation  *cpp
    )
{
    VSC_CPP_SetShader(cpp, gcvNULL);
    VSC_CPP_SetOptions(cpp, gcvNULL);
    VSC_CPP_SetDumper(cpp, gcvNULL);
    vscPMP_Finalize(VSC_CPP_GetPmp(cpp));
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

/*
    If def is between endInst and startInst, return gcvTURE.

    travese backward from the defInst (to all its predecessors):
    if meet endInst, return gcvFALSE;
    if meet startInst, return gcvTRUE;
    if meet the entry, return gcvFALSE;
    if the currBB is already visited, return gcvFALSE;
*/

static gctBOOL _VSC_CPP_DefInstInBetween(
    IN VIR_Instruction      *startInst,
    IN VIR_Instruction      *endInst,
    IN VIR_Instruction      *defInst,
    IN OUT VSC_HASH_TABLE   *visitSet
    )
{
    VIR_BASIC_BLOCK     *currBB = VIR_Inst_GetBasicBlock(defInst);
    VIR_Instruction     *currInst;

    if (vscHTBL_DirectTestAndGet(visitSet, (void*) currBB, gcvNULL))
    {
        return gcvFALSE;
    }

    vscHTBL_DirectSet(visitSet, (void*) currBB, gcvNULL);

    currInst = defInst;
    while (currInst)
    {
        if (currInst == endInst)
        {
            return gcvFALSE;
        }

        if (currInst == startInst)
        {
            return gcvTRUE;
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

            if (_VSC_CPP_DefInstInBetween(startInst, endInst, BB_GET_END_INST(pPredBasicBlk), visitSet))
            {
                return gcvTRUE;
            }
        }
    }

    return gcvFALSE;
}

/*
    If there is define of srcOpnd that is between startInst and endInst,
    return gcvTRUE.

    For example:
    MOV t1.x, t2.y        (startInst)
    SUB t2.y, t5.x, t6.x  (def is between startInst and endInst, return gcvTRUE)
    ADD t3.x, t1.x, t4.x  (endInst)

    Make sure the loop is correctly handled.

*/
static gctBOOL _VSC_CPP_RedefineBetweenInsts(
    IN VSC_CPP_CopyPropagation  *cpp,
    IN VIR_DEF_USAGE_INFO       *duInfo,
    IN VIR_Instruction          *startInst,
    IN VIR_Instruction          *endInst,
    IN VIR_Operand              *srcOpnd,
    OUT VIR_Instruction         **redefInst
    )
{
    gctBOOL         retValue = gcvFALSE;

    VSC_HASH_TABLE  *visitSet = gcvNULL;

    VIR_OperandInfo srcInfo;
    VIR_Enable      enableMask = VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(srcOpnd));

    gctUINT         firstRegNo, regNoRange, regNo, defIdx;
    gctUINT8        channel;
    VIR_DEF         *pDef;
    VIR_DEF_KEY     defKey;

    VIR_Operand_GetOperandInfo(startInst, srcOpnd, &srcInfo);
    firstRegNo = srcInfo.u1.virRegInfo.virReg;
    regNoRange = srcInfo.u1.virRegInfo.virRegCount;

    visitSet = vscHTBL_Create(VSC_CPP_GetMM(cpp), vscHFUNC_Default, vscHKCMP_Default, 512);

    /* find all defs of the same firstRegNo*/
    for (regNo = firstRegNo; regNo < firstRegNo + regNoRange; regNo ++)
    {
        for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
        {
            if (!VSC_UTILS_TST_BIT(enableMask, channel))
            {
                continue;
            }

            defKey.pDefInst = VIR_ANY_DEF_INST;
            defKey.regNo = regNo;
            defKey.channel = VIR_CHANNEL_ANY;
            defIdx = vscBT_HashSearch(&duInfo->defTable, &defKey);

            while (VIR_INVALID_DEF_INDEX != defIdx)
            {
                pDef = GET_DEF_BY_IDX(&duInfo->defTable, defIdx);
                gcmASSERT(pDef);

                if (pDef->defKey.channel == channel)
                {
                    if (pDef->defKey.pDefInst == endInst ||
                        pDef->defKey.pDefInst == startInst)
                    {
                        retValue = gcvTRUE;
                        *redefInst = pDef->defKey.pDefInst;
                        break;
                    }

                    if (pDef->defKey.pDefInst != VIR_INPUT_DEF_INST &&
                        pDef->defKey.pDefInst != VIR_UNDEF_INST)
                    {
                        vscHTBL_Reset(visitSet);
                        /* If any of the instructions in the workSet, that is between
                            endInst and startInst, return gcvTURE.*/
                        if (_VSC_CPP_DefInstInBetween(startInst, endInst,
                                        pDef->defKey.pDefInst, visitSet))
                        {
                            retValue = gcvTRUE;
                            *redefInst = pDef->defKey.pDefInst;
                            break;
                        }
                    }
                }
                defIdx = pDef->nextDefIdxOfSameRegNo;
            }

            if (retValue)
            {
                break;
            }
        }
    }

    vscHTBL_Destroy(visitSet);

    return retValue;
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
    VIR_Shader      *shader = VSC_CPP_GetShader(cpp);
    VIR_Function    *func = VIR_Shader_GetCurrentFunction(shader);

    VSC_OPTN_CPPOptions *options = VSC_CPP_GetOptions(cpp);

    VIR_OpCode      instOpcode = VIR_Inst_GetOpcode(inst);
    VIR_Operand     *instDst = VIR_Inst_GetDest(inst);

    VIR_GENERAL_UD_ITERATOR udIter;
    VIR_DEF         *pDef;

    gctUINT         srcNum;

    if (instDst == gcvNULL)
    {
        return errCode;
    }

    srcNum = VIR_Inst_GetSrcNum(inst);
    while (srcNum-- && (errCode ==  VSC_ERR_NONE))
    {
        VIR_Operand         *srcOpnd = VIR_Inst_GetSource(inst, srcNum);
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

        vscVIR_InitGeneralUdIterator(&udIter, VSC_CPP_GetDUInfo(cpp), inst, srcOpnd, gcvFALSE, gcvFALSE);
        for (pDef = vscVIR_GeneralUdIterator_First(&udIter); pDef != gcvNULL;
             pDef = vscVIR_GeneralUdIterator_Next(&udIter))
        {
            VIR_Instruction     *defInst = pDef->defKey.pDefInst;
            gctUINT8             channel;

            if (defInst &&
                (!VIR_IS_IMPLICIT_DEF_INST(defInst) &&
                 defInst != VIR_UNDEF_INST) &&
                 VIR_Inst_GetOpcode(defInst) == VIR_OP_MOV &&
                 /* this inst's srcOpnd only has one MOV def */
                 vscVIR_IsUniqueDefInstOfUsageInst(
                    VSC_CPP_GetDUInfo(cpp),
                    inst,
                    srcOpnd,
                    gcvFALSE,
                    defInst,
                    gcvNULL))
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
                    VIR_Operand_SetType(newSrc, VIR_Operand_GetType(movDst));
                    if (movSrcInfo.isVecConst)
                    {
                        VIR_Operand_SetSwizzle(newSrc, instSrcSwizzle);
                    }
                    VIR_Inst_SetSource(inst, srcNum, newSrc);
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
                }
                else
                {
                    /* In the IR, there exists implict type conversion, thus we need
                       to bail out if the types mismatch */
                    VIR_TypeId ty0 = VIR_Operand_GetType(movDst);
                    VIR_TypeId ty1 = VIR_Operand_GetType(srcOpnd);

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
                          (VIR_GetTypeTypeKind(ty1) == VIR_TY_IMAGE)))))
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

                            if (pUseInst == VIR_OUTPUT_USAGE_INST)
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
                    if ((movSrcInfo.isInput ||
                        VIR_Symbol_isUniform(VIR_Operand_GetSymbol(movSrc)) ||
                        VIR_Symbol_isSampler(VIR_Operand_GetSymbol(movSrc)) ||
                        VIR_Symbol_isImage(VIR_Operand_GetSymbol(movSrc))) &&
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
                        if (!VSC_CPP_isGlobalCPP(cpp))
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
                                if (_VSC_CPP_RedefineBetweenInsts(cpp, VSC_CPP_GetDUInfo(cpp),
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

                    if(!VIR_OPCODE_isComponentwise(instOpcode))
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

                    if (instEnable == VIR_ENABLE_NONE)
                    {
                        if(VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(options),
                                          VSC_OPTN_CPPOptions_TRACE_FORWARD_OPT))
                        {
                            VIR_Dumper* dumper = VSC_CPP_GetDumper(cpp);
                            VIR_LOG(dumper, "[FW] ==> bail out because of not inst enable\n");
                            VIR_LOG_FLUSH(dumper);
                        }
                        break;
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
                        VIR_TypeId ty = VIR_Operand_GetType(srcOpnd);

                        VIR_Function_DupOperand(func, movSrc, &newSrc);
                        VIR_Operand_SetSwizzle(newSrc, newSwizzle);
                        VIR_Operand_SetLShift(newSrc, VIR_Operand_GetLShift(srcOpnd));
                        VIR_Inst_FreeOperand(inst, srcNum);
                        VIR_Operand_SetType(newSrc, ty);
                        VIR_Inst_SetSource(inst, srcNum, newSrc);
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

                /* no need to remove MOV, since it will be removed by DCE */
            }
        }
    }

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
    IN     VIR_Instruction          *inst
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
    VSC_HASH_TABLE          *inst_def_set     = gcvNULL;
    inst_def_set = vscHTBL_Create(VSC_CPP_GetMM(cpp),
                            vscHFUNC_Default, vscHKCMP_Default, 512);

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

    ty0 = VIR_Operand_GetType(inst_src0);
    ty1 = VIR_Operand_GetType(inst_dest);

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
        case VIR_OP_MAD:
        case VIR_OP_SELECT:
        case VIR_OP_STEP:
        case VIR_OP_AND_BITWISE:
        case VIR_OP_OR_BITWISE:
        case VIR_OP_XOR_BITWISE:
        case VIR_OP_NOT_BITWISE:
        case VIR_OP_LSHIFT:
        case VIR_OP_RSHIFT:
        case VIR_OP_ROTATE:
        case VIR_OP_LDARR:
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

        ty0 = VIR_Operand_GetType(def_inst_dest);
        ty1 = VIR_Operand_GetType(inst_dest);

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
        return errCode;
    }

    /* do transformation */
    /* collect the usage of the dest of the input inst, change the sym and map the usage channel */
    {
        VSC_HASH_TABLE          *inst_usage_set     = gcvNULL;
        VIR_GENERAL_DU_ITERATOR  inst_du_iter;
        VIR_USAGE               *inst_usage         = gcvNULL;
        gctUINT                 *channelMask        = gcvNULL;

        inst_usage_set = vscHTBL_Create(
            VSC_CPP_GetMM(cpp),
            _VSC_CPP_Usage_HFUNC,
            _VSC_CPP_Usage_HKCMP,
            512);
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

                VIR_Operand_ReplaceDefOperandWithDef(
                    def_inst_dest,
                    inst_dest,
                    def_inst_enable);

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
        VIR_Function_RemoveInstruction(func, inst);

        vscHTBL_Destroy(inst_usage_set);
    }

    vscHTBL_Destroy(inst_def_set);

    return errCode;
}

static VSC_ErrCode _VSC_CPP_CopyPropagationForBB(
    IN OUT VSC_CPP_CopyPropagation  *cpp
    )
{
    VSC_ErrCode         errCode  = VSC_ERR_NONE;
    VIR_BB              *bb = VSC_CPP_GetCurrBB(cpp);
    VSC_OPTN_CPPOptions *options = VSC_CPP_GetOptions(cpp);

    VIR_Instruction     *inst;

    inst = BB_GET_START_INST(bb);
    while(inst != VIR_Inst_GetNext(BB_GET_END_INST(bb)))
    {
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
                _VSC_CPP_CopyToMOV(cpp, inst);
            }
        }

        inst = VIR_Inst_GetNext(inst);
    }

    return errCode;
}

VSC_ErrCode VSC_CPP_PerformOnFunction(
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
        for(bb = CFG_ITERATOR_FIRST(&cfg_iter); bb != gcvNULL; bb = CFG_ITERATOR_NEXT(&cfg_iter))
        {
            if(BB_GET_LENGTH(bb) != 0)
            {
                VSC_CPP_SetCurrBB(cpp, bb);
                errCode = _VSC_CPP_CopyPropagationForBB(cpp);
            }

            if(errCode)
            {
                return errCode;
            }
        }

        func->instList.pHead = func->instList.pHead->parent.BB->pStartInst;
        func->instList.pTail = func->instList.pTail->parent.BB->pEndInst;
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

VSC_ErrCode VSC_CPP_PerformOnShader(
    IN OUT VSC_CPP_CopyPropagation *cpp
    )
{
    VSC_ErrCode         errcode  = VSC_ERR_NONE;
    VIR_Shader          *shader = VSC_CPP_GetShader(cpp);
    VIR_FuncIterator    func_iter;
    VIR_FunctionNode    *func_node;

    if (VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(VSC_CPP_GetOptions(cpp)),
        VSC_OPTN_CPPOptions_TRACE_INPUT))
    {
        VIR_Shader_Dump(gcvNULL, "Shader before Copy Propagation", shader, gcvTRUE);
    }

    /* don't perform global CPP when the cfg has too many nodes*/
    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(shader));
    for(func_node = VIR_FuncIterator_First(&func_iter);
        func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function    *func = func_node->function;

        if (VIR_Function_GetCFG(func)->dgGraph.nodeList.info.count > 1000)
        {
            VSC_CPP_SetGlobalCPP(cpp, gcvFALSE);
            break;
        }
    }

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(shader));
    for(func_node = VIR_FuncIterator_First(&func_iter);
        func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function    *func = func_node->function;

        VIR_Shader_SetCurrentFunction(shader, func);
        errcode = VSC_CPP_PerformOnFunction(cpp);
        if(errcode)
        {
            break;
        }
    }
    if ((VSC_UTILS_MASK(VSC_OPTN_CPPOptions_GetTrace(VSC_CPP_GetOptions(cpp)),
         VSC_OPTN_CPPOptions_TRACE_INPUT)) ||
        gcSHADER_DumpCodeGenVerbose(shader))
    {
        if (VSC_CPP_isGlobalCPP(cpp))
        {
            VIR_Shader_Dump(gcvNULL, "After Global Copy Propagation.", shader, gcvTRUE);
        }
        else
        {
            VIR_Shader_Dump(gcvNULL, "After Local Copy Propagation.", shader, gcvTRUE);
        }
    }

    return errcode;
}

