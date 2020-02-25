/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_vsc.h"

static void _Update_Liveness_Local_Kill(VIR_DEF_USAGE_INFO* pDuInfo,
                                        VSC_BIT_VECTOR* pGenFlow,
                                        VSC_BIT_VECTOR* pKillFlow,
                                        VSC_STATE_VECTOR* pLocalHalfChannelKillFlow,
                                        VIR_Instruction* pInst,
                                        gctUINT firstRegNo,
                                        gctUINT regNoRange,
                                        VIR_Enable defEnableMask,
                                        gctUINT8 halfChannelMask)
{
    VSC_BLOCK_TABLE*         pDefTable = &pDuInfo->defTable;
    VIR_DEF*                 pDef;
    gctUINT                  regNo, defIdx;
    gctUINT8                 channel;
    gctUINT8                 killedHalfChannelMask;
#if ENABLE_AGGRESSIVE_IPA_LIVENESS
    gctUINT                  i, otherDefIdx;
    VIR_DEF*                 pOtherDef;
    VSC_BLOCK_TABLE*         pUsageTable = &pDuInfo->usageTable;
    VSC_DU_ITERATOR          duIter;
    VIR_DU_CHAIN_USAGE_NODE* pUsageNode;
    VIR_USAGE*               pUsage;
    VIR_FUNC_BLOCK*          pUsageFuncBlk;
    VIR_FUNC_FLOW*           pUsageRDFuncFlow;
#endif

    for (regNo = firstRegNo; regNo < firstRegNo + regNoRange; regNo ++)
    {
        for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
        {
            if (!VSC_UTILS_TST_BIT(defEnableMask, channel))
            {
                continue;
            }

            /* Find the def with current channel */
            defIdx = vscVIR_FindFirstDefIndex(pDuInfo, regNo);
            while (VIR_INVALID_DEF_INDEX != defIdx)
            {
                pDef = GET_DEF_BY_IDX(pDefTable, defIdx);
                gcmASSERT(pDef);

                if (pDef->defKey.channel == channel)
                {
                    killedHalfChannelMask = (gctUINT8)vscSV_Get(pLocalHalfChannelKillFlow, defIdx);
                    killedHalfChannelMask |= halfChannelMask;

                    if (VSC_UTILS_TST(killedHalfChannelMask, pDef->halfChannelMask) == pDef->halfChannelMask)
                    {
                        vscBV_SetBit(pKillFlow, defIdx);
                        vscBV_ClearBit(pGenFlow, defIdx);

                        vscSV_Set(pLocalHalfChannelKillFlow, defIdx, VIR_HALF_CHANNEL_MASK_NONE);
                    }
                    else
                    {
                        vscSV_Set(pLocalHalfChannelKillFlow, defIdx, killedHalfChannelMask);
                    }
                }

#if ENABLE_AGGRESSIVE_IPA_LIVENESS

                /* If usage of this def is at other function, and it is not alive of
                   reach-def at end of that function, then we should also kill other
                   defs of that usage with same channel. */

                if (vscDG_GetNodeCount(&pDuInfo->baseDFA.pOwnerCG->dgGraph) == 1)
                {
                    continue;
                }

                if (pDef->flags.bNoUsageCrossRoutine)
                {
                    continue;
                }

                /* For each usage in this du chain */
                VSC_DU_ITERATOR_INIT(&duIter, &pDef->duChain);
                pUsageNode = VSC_DU_ITERATOR_FIRST(&duIter);
                for (; pUsageNode != gcvNULL; pUsageNode = VSC_DU_ITERATOR_NEXT(&duIter))
                {
                    pUsage = GET_USAGE_BY_IDX(pUsageTable, pUsageNode->usageIdx);
                    gcmASSERT(pUsage);

                    if (VIR_Inst_GetFunction(pUsage->usageKey.pUsageInst) !=
                        VIR_Inst_GetFunction(pDef->defKey.pDefInst))
                    {
                        pUsageFuncBlk = VIR_Inst_GetFunction(pUsage->usageKey.pUsageInst)->pFuncBlock;
                        pUsageRDFuncFlow = (VIR_FUNC_FLOW*)vscSRARR_GetElement(&pDuInfo->baseDFA.funcFlowArray,
                                                                             pUsageFuncBlk->dgNode.id);

                        if (vscBV_TestBit(&pUsageRDFuncFlow->outFlow, defIdx))
                        {
                            continue;
                        }

                        /* Check ud chain to add kill */
                        for (i = 0; i < UD_CHAIN_GET_DEF_COUNT(&pUsage->udChain); i ++)
                        {
                            otherDefIdx = UD_CHAIN_GET_DEF(&pUsage->udChain, i);
                            gcmASSERT(VIR_INVALID_DEF_INDEX != otherDefIdx);

                            if (otherDefIdx == defIdx)
                            {
                                continue;
                            }

                            pOtherDef = GET_DEF_BY_IDX(pDefTable, otherDefIdx);
                            gcmASSERT(pOtherDef);

                            if (pOtherDef->defKey.channel == channel &&
                                !vscBV_TestBit(&pUsageRDFuncFlow->outFlow, otherDefIdx))
                            {
                                vscBV_SetBit(pKillFlow, otherDefIdx);
                                vscBV_ClearBit(pGenFlow, otherDefIdx);
                            }
                        }
                    }
                }
#endif

                /* Get next def with same regNo */
                defIdx = pDef->nextDefIdxOfSameRegNo;
            }
        }
    }
}

static void _Update_Liveness_Local_Gen_Outputs_By_Emit(VIR_Shader* pShader,
                                                       VIR_DEF_USAGE_INFO* pDuInfo,
                                                       VSC_BIT_VECTOR* pGenFlow,
                                                       VSC_BIT_VECTOR* pKillFlow,
                                                       VIR_Instruction* pInst,
                                                       gctBOOL bCheckAllOutput,
                                                       gctINT streamNumber)
{
    VSC_BLOCK_TABLE*       pDefTable = &pDuInfo->defTable;
    VSC_BLOCK_TABLE*       pUsageTable = &pDuInfo->usageTable;
    gctUINT                usageCount, usageIdx, i, defIdx;
    VIR_USAGE*             pUsage = gcvNULL;
    VIR_DEF*               pDef;
    VIR_Symbol*            pTempSym;
    VIR_Symbol*            pOutputSym;

    usageCount = BT_GET_MAX_VALID_ID(&pDuInfo->usageTable);
    for (usageIdx = 0; usageIdx < usageCount; usageIdx ++)
    {
        pUsage = GET_USAGE_BY_IDX(pUsageTable, usageIdx);
        if (IS_VALID_USAGE(pUsage))
        {
            if (pUsage->usageKey.pUsageInst == pInst)
            {
                /* Each def of this usage are live now */
                for (i = 0; i < UD_CHAIN_GET_DEF_COUNT(&pUsage->udChain); i ++)
                {
                    defIdx = UD_CHAIN_GET_DEF(&pUsage->udChain, i);
                    pDef = GET_DEF_BY_IDX(pDefTable, defIdx);
                    gcmASSERT(pDef);

                    /* Its def must be ouput def */
                    if (!pDef->flags.nativeDefFlags.bIsOutput)
                    {
                        gcmASSERT(gcvFALSE);
                    }

                    /* Find the specified output. */
                    if (!bCheckAllOutput)
                    {
                        pTempSym = VIR_Shader_FindSymbolByTempIndex(pShader, pDef->defKey.regNo);
                        gcmASSERT(pTempSym);

                        pOutputSym = VIR_Symbol_GetVregVariable(pTempSym);
                        gcmASSERT(pOutputSym);

                        if (VIR_Symbol_GetStreamNumber(pOutputSym) != streamNumber)
                        {
                            continue;
                        }
                    }

                    vscBV_SetBit(pGenFlow, defIdx);
                }
            }
        }
    }
}

static void _Update_Liveness_Local_Gen(VIR_DEF_USAGE_INFO* pDuInfo,
                                       VSC_BIT_VECTOR* pGenFlow,
                                       VSC_BIT_VECTOR* pKillFlow,
                                       VSC_STATE_VECTOR* pLocalHalfChannelKillFlow,
                                       VIR_Instruction* pInst,
                                       VIR_Operand* pOperand,
                                       gctBOOL bForIndexingReg,
                                       gctUINT firstRegNo,
                                       gctUINT regNoRange,
                                       VIR_Enable defEnableMask,
                                       gctUINT8 halfChannelMask)
{
    gctUINT                usageIdx, i, defIdx;
    VIR_USAGE_KEY          usageKey;
    VIR_USAGE*             pUsage = gcvNULL;

    if (defEnableMask == VIR_ENABLE_NONE)
    {
        return;
    }

    /* Find the usage */
    usageKey.pUsageInst = pInst;
    usageKey.pOperand = pOperand;
    usageKey.bIsIndexingRegUsage = bForIndexingReg;
    usageIdx = vscBT_HashSearch(&pDuInfo->usageTable, &usageKey);
    if (VIR_INVALID_USAGE_INDEX != usageIdx)
    {
        pUsage = GET_USAGE_BY_IDX(&pDuInfo->usageTable, usageIdx);
        gcmASSERT(pUsage);

        gcmASSERT(pUsage->halfChannelMask == halfChannelMask);

        /* Each def of this usage are live now */
        for (i = 0; i < UD_CHAIN_GET_DEF_COUNT(&pUsage->udChain); i ++)
        {
            defIdx = UD_CHAIN_GET_DEF(&pUsage->udChain, i);
            vscBV_SetBit(pGenFlow, defIdx);
        }
    }

    if (pUsage)
    {
        VIR_DEF*         pDef = gcvNULL;
        gctUINT          firstRegNo1, regNoRange1;
        VIR_Enable       defEnableMask1;
        VIR_OperandInfo  operandInfo, operandInfo1;

        defIdx = UD_CHAIN_GET_DEF(&pUsage->udChain, 0);
        gcmASSERT(VIR_INVALID_DEF_INDEX != defIdx);
        pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, defIdx);

        if (pDef->defKey.pDefInst < VIR_INPUT_DEF_INST)
        {
            if (vscVIR_IsUniqueDefInstOfUsageInst(pDuInfo, pInst, pOperand, bForIndexingReg,
                                                  pDef->defKey.pDefInst, gcvNULL) &&
                VIR_Inst_GetOpcode(pDef->defKey.pDefInst) == VIR_OP_LDARR)
            {
                VIR_Operand_GetOperandInfo(pDef->defKey.pDefInst,
                                           pDef->defKey.pDefInst->src[VIR_Operand_Src0],
                                           &operandInfo);

                VIR_Operand_GetOperandInfo(pDef->defKey.pDefInst,
                                           pDef->defKey.pDefInst->src[VIR_Operand_Src1],
                                           &operandInfo1);

                if (operandInfo1.isImmVal)
                {
                    firstRegNo1 = operandInfo.u1.virRegInfo.virReg + operandInfo1.u1.immValue.iValue;
                    regNoRange1 = 1;
                }
                else
                {
                    firstRegNo1 = operandInfo.u1.virRegInfo.startVirReg;
                    regNoRange1 = operandInfo.u1.virRegInfo.virRegCount;
                }

                if (VIR_OpndInfo_Is_Virtual_Reg(&operandInfo))
                {
                    defEnableMask1 = VIR_Operand_GetRealUsedChannels(pDef->defKey.pDefInst->src[VIR_Operand_Src0],
                                                                     pDef->defKey.pDefInst,
                                                                     gcvNULL);

                    _Update_Liveness_Local_Gen(pDuInfo,
                                               pGenFlow,
                                               pKillFlow,
                                               pLocalHalfChannelKillFlow,
                                               pDef->defKey.pDefInst,
                                               pDef->defKey.pDefInst->src[VIR_Operand_Src0],
                                               gcvFALSE,
                                               firstRegNo1,
                                               regNoRange1,
                                               defEnableMask1,
                                               (gctUINT8)operandInfo.halfChannelMask);
                }
            }
        }
    }
}

static void _Update_Liveness_Local_Gens(VIR_Shader* pShader,
                                        VIR_DEF_USAGE_INFO* pDuInfo,
                                        VSC_BIT_VECTOR* pGenFlow,
                                        VSC_BIT_VECTOR* pKillFlow,
                                        VSC_STATE_VECTOR* pLocalHalfChannelKillFlow,
                                        VIR_Instruction* pInst)
{
    gctUINT                 firstRegNo, regNoRange;
    VIR_Enable              defEnableMask;
    VIR_OperandInfo         operandInfo, operandInfo1;
    VIR_SrcOperand_Iterator srcOpndIter;
    VIR_Operand*            pOpnd;

    /* If dest is accessed by Rb[Ro.single_channel], we need consider Ro usage */
    if (pInst->dest != gcvNULL)
    {
        VIR_Operand_GetOperandInfo(pInst,
                                   pInst->dest,
                                   &operandInfo);

        if (operandInfo.indexingVirRegNo != VIR_INVALID_REG_NO)
        {
            firstRegNo = operandInfo.indexingVirRegNo;
            regNoRange = 1;
            defEnableMask = (1 << operandInfo.componentOfIndexingVirRegNo);

            _Update_Liveness_Local_Gen(pDuInfo,
                                       pGenFlow,
                                       pKillFlow,
                                       pLocalHalfChannelKillFlow,
                                       pInst,
                                       pInst->dest,
                                       gcvTRUE,
                                       firstRegNo,
                                       regNoRange,
                                       defEnableMask,
                                       (gctUINT8)operandInfo.halfChannelMaskOfIndexingVirRegNo);
        }
    }

    /* A ldarr inst to array may potentially read all elements in array */
    if (VIR_Inst_GetOpcode(pInst) == VIR_OP_LDARR)
    {
        VIR_Operand_GetOperandInfo(pInst,
                                   pInst->src[VIR_Operand_Src0],
                                   &operandInfo);

        VIR_Operand_GetOperandInfo(pInst,
                                   pInst->src[VIR_Operand_Src1],
                                   &operandInfo1);

        if (operandInfo1.isImmVal)
        {
            firstRegNo = operandInfo.u1.virRegInfo.virReg + operandInfo1.u1.immValue.iValue;
            regNoRange = 1;
        }
        else
        {
            if (VIR_OpndInfo_Is_Virtual_Reg(&operandInfo1))
            {
                _Update_Liveness_Local_Gen(pDuInfo,
                                           pGenFlow,
                                           pKillFlow,
                                           pLocalHalfChannelKillFlow,
                                           pInst,
                                           pInst->src[VIR_Operand_Src1],
                                           gcvFALSE,
                                           operandInfo1.u1.virRegInfo.virReg,
                                           1,
                                           VIR_Operand_GetRealUsedChannels(pInst->src[VIR_Operand_Src1], pInst, gcvNULL),
                                           (gctUINT8)operandInfo1.halfChannelMask);
            }

            firstRegNo = operandInfo.u1.virRegInfo.startVirReg;
            regNoRange = operandInfo.u1.virRegInfo.virRegCount;
        }

        if (VIR_OpndInfo_Is_Virtual_Reg(&operandInfo))
        {
            defEnableMask = VIR_Operand_GetRealUsedChannels(pInst->src[VIR_Operand_Src0], pInst, gcvNULL);

            _Update_Liveness_Local_Gen(pDuInfo,
                                       pGenFlow,
                                       pKillFlow,
                                       pLocalHalfChannelKillFlow,
                                       pInst,
                                       pInst->src[VIR_Operand_Src0],
                                       gcvFALSE,
                                       firstRegNo,
                                       regNoRange,
                                       defEnableMask,
                                       (gctUINT8)operandInfo.halfChannelMask);
        }
    }
    else
    {
        VIR_SrcOperand_Iterator_Init(pInst, &srcOpndIter);
        pOpnd = VIR_SrcOperand_Iterator_First(&srcOpndIter);

        for (; pOpnd != gcvNULL; pOpnd = VIR_SrcOperand_Iterator_Next(&srcOpndIter))
        {
            VIR_Operand_GetOperandInfo(pInst,
                                       pOpnd,
                                       &operandInfo);

            if (VIR_OpndInfo_Is_Virtual_Reg(&operandInfo))
            {
                if (operandInfo.indexingVirRegNo != VIR_INVALID_REG_NO)
                {
                    /* For the case of Rb[Ro.single_channel] access */

                    firstRegNo = operandInfo.u1.virRegInfo.startVirReg;
                    regNoRange = operandInfo.u1.virRegInfo.virRegCount;
                }
                else
                {
                    firstRegNo = operandInfo.u1.virRegInfo.virReg;
                    regNoRange = 1;
                }
                defEnableMask = VIR_Operand_GetRealUsedChannels(pOpnd, pInst, gcvNULL);

                _Update_Liveness_Local_Gen(pDuInfo,
                                           pGenFlow,
                                           pKillFlow,
                                           pLocalHalfChannelKillFlow,
                                           pInst,
                                           pOpnd,
                                           gcvFALSE,
                                           firstRegNo,
                                           regNoRange,
                                           defEnableMask,
                                           (gctUINT8)operandInfo.halfChannelMask);
            }

            /* For the case of Rb[Ro.single_channel] access, we need consider Ro usage */
            if (operandInfo.indexingVirRegNo != VIR_INVALID_REG_NO)
            {
                firstRegNo = operandInfo.indexingVirRegNo;
                regNoRange = 1;
                defEnableMask = (1 << operandInfo.componentOfIndexingVirRegNo);

                _Update_Liveness_Local_Gen(pDuInfo,
                                           pGenFlow,
                                           pKillFlow,
                                           pLocalHalfChannelKillFlow,
                                           pInst,
                                           pOpnd,
                                           gcvTRUE,
                                           firstRegNo,
                                           regNoRange,
                                           defEnableMask,
                                           (gctUINT8)operandInfo.halfChannelMaskOfIndexingVirRegNo);
            }
        }
    }
}

static void _Liveness_Local_GenKill_Resolver(VIR_BASE_TS_DFA* pBaseTsDFA, VIR_TS_BLOCK_FLOW* pTsBlockFlow)
{
    VIR_BASIC_BLOCK*       pBasicBlock = pTsBlockFlow->pOwnerBB;
    VIR_Shader*            pShader = pBasicBlock->pOwnerCFG->pOwnerFuncBlk->pOwnerCG->pOwnerShader;
    VIR_DEF_USAGE_INFO*    pDuInfo = ((VIR_LIVENESS_INFO*)pBaseTsDFA)->pDuInfo;
    VSC_BIT_VECTOR*        pGenFlow = &pTsBlockFlow->genFlow;
    VSC_BIT_VECTOR*        pKillFlow = &pTsBlockFlow->killFlow;
    VIR_Instruction*       pStartInst = BB_GET_START_INST(pBasicBlock);
    VIR_Instruction*       pInst = BB_GET_END_INST(pBasicBlock);
    VIR_Enable             defEnableMask;
    gctUINT                firstRegNo, regNoRange;
    gctUINT8               halfChannelMask;
    gctBOOL                bIndexing;
    VSC_STATE_VECTOR       localHalfChannelKillFlow;

    /* 4 states we are using:
       VIR_HALF_CHANNEL_MASK_NONE (0),
       VIR_HALF_CHANNEL_MASK_LOW  (1),
       VIR_HALF_CHANNEL_MASK_HIGH (2),
       VIR_HALF_CHANNEL_MASK_FULL (3)
    */
    vscSV_Initialize(&localHalfChannelKillFlow, pBaseTsDFA->baseDFA.pMM, pBaseTsDFA->baseDFA.flowSize, 4);

    /* Go through all instructions of basic block to analyze local gen and kill set */
    while (pInst)
    {
        /* Kill at real def */
        if (vscVIR_QueryRealWriteVirRegInfo(pShader,
                                            pInst,
                                            &defEnableMask,
                                            &halfChannelMask,
                                            &firstRegNo,
                                            &regNoRange,
                                            gcvNULL,
                                            &bIndexing))
        {
            if (
                /* For dynamic indexing, add all to killSet to avoid the usage in the inflow set of Function */
                /*!bIndexing &&*/
                vscVIR_IsInstDefiniteWrite(pDuInfo, pInst, firstRegNo, gcvTRUE))
            {
                _Update_Liveness_Local_Kill(pDuInfo,
                                            pGenFlow,
                                            pKillFlow,
                                            &localHalfChannelKillFlow,
                                            pInst,
                                            firstRegNo,
                                            regNoRange,
                                            defEnableMask,
                                            halfChannelMask);
            }
        }

        /* Gen at usages */
        _Update_Liveness_Local_Gens(pShader,
                                    pDuInfo,
                                    pGenFlow,
                                    pKillFlow,
                                    &localHalfChannelKillFlow,
                                    pInst);

         /* Emit has implicit usage for all outputs, so we also gen these */
        if (VIR_Inst_GetOpcode(pInst) == VIR_OP_EMIT0   ||
            VIR_Inst_GetOpcode(pInst) == VIR_OP_EMIT    ||
            VIR_Inst_GetOpcode(pInst) == VIR_OP_EMIT_STREAM)
        {
            gctBOOL     bCheckAllOutput = gcvTRUE;
            gctINT      streamNumber = 0;

            if (VIR_Inst_GetOpcode(pInst) == VIR_OP_EMIT_STREAM)
            {
                bCheckAllOutput = gcvFALSE;
                gcmASSERT(VIR_Operand_isImm(VIR_Inst_GetSource(pInst, 0)));
                streamNumber = VIR_Operand_GetImmediateInt(VIR_Inst_GetSource(pInst, 0));
            }

            _Update_Liveness_Local_Gen_Outputs_By_Emit(pShader,
                                                       pDuInfo,
                                                       pGenFlow,
                                                       pKillFlow,
                                                       pInst,
                                                       bCheckAllOutput,
                                                       streamNumber);
        }

        /* If current inst is the start inst of block, just bail out */
        if (pInst == pStartInst)
        {
            break;
        }

        /* Move to previous inst */
        pInst = VIR_Inst_GetPrev(pInst);
    }

    /* A full LV kill can not be across boundary of basic-block. So for a LV, either it is
       fully killed in basic-block, or it is not fully killed. */
    gcmASSERT(vscSV_All(&localHalfChannelKillFlow, VIR_HALF_CHANNEL_MASK_NONE));

    vscSV_Finalize(&localHalfChannelKillFlow);
}

static void _Liveness_Init_Resolver(VIR_BASE_TS_DFA* pBaseTsDFA, VIR_TS_BLOCK_FLOW* pTsBlockFlow)
{
#if SUPPORT_IPA_DFA
    VIR_CALL_GRAPH*          pCG = pTsBlockFlow->pOwnerBB->pOwnerCFG->pOwnerFuncBlk->pOwnerCG;
#endif
    VSC_BLOCK_TABLE*         pDefTable = &((VIR_LIVENESS_INFO*)pBaseTsDFA)->pDuInfo->defTable;
    VSC_BLOCK_TABLE*         pUsageTable = &((VIR_LIVENESS_INFO*)pBaseTsDFA)->pDuInfo->usageTable;
    VSC_BIT_VECTOR*          pOutFlow = &pTsBlockFlow->outFlow;
    gctUINT                  defIdx;
    VIR_DEF*                 pDef;
    VIR_USAGE*               pUsage;
    VIR_DU_CHAIN_USAGE_NODE* pUsageNode;
    VSC_DU_ITERATOR          duIter;

    /* All outputs are regarded as live at exit block */
    if (
#if SUPPORT_IPA_DFA
        (CG_GET_MAIN_FUNC(pCG) == pTsBlockFlow->pOwnerBB->pOwnerCFG->pOwnerFuncBlk->pVIRFunc) &&
#endif
        (BB_GET_FLOWTYPE(pTsBlockFlow->pOwnerBB) == VIR_FLOW_TYPE_EXIT)
        )
    {
        /* Implicit outputs to FFU are considered */
        for (defIdx = 0; defIdx < (gctUINT)pBaseTsDFA->baseDFA.flowSize; defIdx ++)
        {
            pDef = GET_DEF_BY_IDX(pDefTable, defIdx);
            if (pDef->flags.nativeDefFlags.bIsOutput)
            {
                VSC_DU_ITERATOR_INIT(&duIter, &pDef->duChain);
                pUsageNode = VSC_DU_ITERATOR_FIRST(&duIter);
                for (; pUsageNode != gcvNULL; pUsageNode = VSC_DU_ITERATOR_NEXT(&duIter))
                {
                    pUsage = GET_USAGE_BY_IDX(pUsageTable, pUsageNode->usageIdx);
                    gcmASSERT(pUsage);

                    if (pUsage->usageKey.pUsageInst == VIR_OUTPUT_USAGE_INST)
                    {
                        vscBV_SetBit(pOutFlow, defIdx);
                        break;
                    }
                }
            }
        }
    }
}

static gctBOOL _Liveness_Iterate_Resolver(VIR_BASE_TS_DFA* pBaseTsDFA, VIR_TS_BLOCK_FLOW* pTsBlockFlow)
{
    VSC_BIT_VECTOR*        pInFlow = &pTsBlockFlow->inFlow;
    VSC_BIT_VECTOR*        pOutFlow = &pTsBlockFlow->outFlow;
    VSC_BIT_VECTOR*        pGenFlow = &pTsBlockFlow->genFlow;
    VSC_BIT_VECTOR*        pKillFlow = &pTsBlockFlow->killFlow;
    VSC_BIT_VECTOR         tmpFlow;
    gctBOOL                bChanged = gcvFALSE;

    vscBV_Initialize(&tmpFlow, pBaseTsDFA->baseDFA.pScratchMemPool, pBaseTsDFA->baseDFA.flowSize);

    /* In = Gen U (Out - Kill) */
    vscBV_Minus2(&tmpFlow, pOutFlow, pKillFlow);
    vscBV_Or1(&tmpFlow, pGenFlow);

    bChanged = !vscBV_Equal(&tmpFlow, pInFlow);
    if (bChanged)
    {
        vscBV_Copy(pInFlow, &tmpFlow);
    }

    vscBV_Finalize(&tmpFlow);

    return bChanged;
}

static gctBOOL _Liveness_Combine_Resolver(VIR_BASE_TS_DFA* pBaseTsDFA, VIR_TS_BLOCK_FLOW* pTsBlockFlow)
{
    VIR_BASIC_BLOCK*             pSuccBasicBlk;
    VSC_ADJACENT_LIST_ITERATOR   succEdgeIter;
    VIR_CFG_EDGE*                pSuccEdge;
    VIR_BASIC_BLOCK*             pBasicBlock = pTsBlockFlow->pOwnerBB;
    VSC_BIT_VECTOR*              pOutFlow = &pTsBlockFlow->outFlow;
    VSC_BIT_VECTOR               tmpFlow;
    gctBOOL                      bChanged = gcvFALSE;

    /* If there is no successors, then just reture FALSE */
    if (DGND_GET_OUT_DEGREE(&pBasicBlock->dgNode) == 0)
    {
        return gcvFALSE;
    }

    vscBV_Initialize(&tmpFlow, pBaseTsDFA->baseDFA.pScratchMemPool, pBaseTsDFA->baseDFA.flowSize);

    /* Out = U all-succ-Ins */
    VSC_ADJACENT_LIST_ITERATOR_INIT(&succEdgeIter, &pBasicBlock->dgNode.succList);
    pSuccEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&succEdgeIter);
    for (; pSuccEdge != gcvNULL; pSuccEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&succEdgeIter))
    {
        pSuccBasicBlk = CFG_EDGE_GET_TO_BB(pSuccEdge);
        vscBV_Or1(&tmpFlow, &pSuccBasicBlk->pTsWorkDataFlow->inFlow);
    }

    bChanged = !vscBV_Equal(&tmpFlow, pOutFlow);
    if (bChanged)
    {
        vscBV_Copy(pOutFlow, &tmpFlow);
    }

    vscBV_Finalize(&tmpFlow);

    return bChanged;
}

#if SUPPORT_IPA_DFA
static gctBOOL _Liveness_Block_Flow_Combine_From_Callee_Resolver(VIR_BASE_TS_DFA* pBaseTsDFA, VIR_TS_BLOCK_FLOW* pCallerTsBlockFlow)
{
    VIR_BASIC_BLOCK*       pBasicBlock = pCallerTsBlockFlow->pOwnerBB;
    VSC_BIT_VECTOR*        pInFlow = &pCallerTsBlockFlow->inFlow;
#if ENABLE_AGGRESSIVE_IPA_LIVENESS
    VSC_BIT_VECTOR*        pOutFlow = &pCallerTsBlockFlow->outFlow;
#endif
    VIR_FUNC_BLOCK*        pCallee = VIR_Inst_GetCallee(pBasicBlock->pStartInst)->pFuncBlock;
    VIR_TS_FUNC_FLOW*      pCalleeFuncFlow = (VIR_TS_FUNC_FLOW*)vscSRARR_GetElement(&pBaseTsDFA->tsFuncFlowArray, pCallee->dgNode.id);
    VSC_BIT_VECTOR         tmpFlow;
    gctBOOL                bChanged = gcvFALSE;

    gcmASSERT(pBasicBlock->flowType == VIR_FLOW_TYPE_CALL);

    vscBV_Initialize(&tmpFlow, pBaseTsDFA->baseDFA.pScratchMemPool, pBaseTsDFA->baseDFA.flowSize);

#if ENABLE_AGGRESSIVE_IPA_LIVENESS
    /* U flow that not flows into callee and in flow of callee */
    vscBV_Minus2(&tmpFlow, pOutFlow, &pCalleeFuncFlow->outFlow);
    vscBV_Or1(&tmpFlow, &pCalleeFuncFlow->inFlow);
#else
    vscBV_Copy(&tmpFlow, &pCalleeFuncFlow->inFlow);
#endif

    bChanged = !vscBV_Equal(pInFlow, &tmpFlow);
    if (bChanged)
    {
        vscBV_Copy(pInFlow, &tmpFlow);
    }

    vscBV_Finalize(&tmpFlow);

    return bChanged;
}

static gctBOOL _Liveness_Func_Flow_Combine_From_Callers_Resolver(VIR_BASE_TS_DFA* pBaseTsDFA, VIR_TS_FUNC_FLOW* pCalleeTsFuncFlow)
{
    gctUINT                      callerIdx;
    VIR_BASIC_BLOCK*             pCallerBasicBlk;
    VIR_Instruction*             pCallSiteInst;
    VIR_FUNC_BLOCK*              pCalleeFuncBlock = pCalleeTsFuncFlow->pOwnerFB;
    VSC_BIT_VECTOR*              pOutFlow = &pCalleeTsFuncFlow->outFlow;
    VSC_ADJACENT_LIST_ITERATOR   callerIter;
    VIR_CG_EDGE*                 pCallerEdge;
    VSC_BIT_VECTOR               tmpFlow;
    gctBOOL                      bChanged = gcvFALSE;

    vscBV_Initialize(&tmpFlow, pBaseTsDFA->baseDFA.pScratchMemPool, pBaseTsDFA->baseDFA.flowSize);

#if ENABLE_AGGRESSIVE_IPA_LIVENESS
    vscBV_SetAll(&tmpFlow);
#endif

    /* n (all out flow of caller at every call site) */
    VSC_ADJACENT_LIST_ITERATOR_INIT(&callerIter, &pCalleeFuncBlock->dgNode.predList);
    pCallerEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&callerIter);
    for (; pCallerEdge != gcvNULL; pCallerEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&callerIter))
    {
        /* Call site info is only stored at successor edge */
        pCallerEdge = CG_PRED_EDGE_TO_SUCC_EDGE(pCallerEdge);

        for (callerIdx = 0; callerIdx < vscSRARR_GetElementCount(&pCallerEdge->callSiteArray); callerIdx ++)
        {
            pCallSiteInst = *(VIR_Instruction**)vscSRARR_GetElement(&pCallerEdge->callSiteArray, callerIdx);
            pCallerBasicBlk = VIR_Inst_GetBasicBlock(pCallSiteInst);

#if ENABLE_AGGRESSIVE_IPA_LIVENESS
            vscBV_And1(&tmpFlow, &pCallerBasicBlk->pWorkDataFlow->outFlow);
#else
            vscBV_Or1(&tmpFlow, &pCallerBasicBlk->pTsWorkDataFlow->outFlow);
#endif
        }
    }

    bChanged = !vscBV_Equal(&tmpFlow, pOutFlow);
    if (bChanged)
    {
        vscBV_Copy(pOutFlow, &tmpFlow);
    }

    vscBV_Finalize(&tmpFlow);

    return bChanged;
}
#endif

static VSC_ErrCode _DoLivenessAnalysis(VIR_CALL_GRAPH* pCg, VIR_LIVENESS_INFO* pLvInfo)
{
    VSC_ErrCode            errCode = VSC_ERR_NONE;
    VSC_BLOCK_TABLE*       pDefTable = &pLvInfo->pDuInfo->defTable;
    gctUINT                flowSize = BT_GET_MAX_VALID_ID(&pLvInfo->pDuInfo->defTable);
    VIR_TS_FUNC_FLOW*      pInFlowOfMainFunc;
    gctUINT                defIdx, startBitOrdinal = 0;
    VIR_DEF*               pDef;
    VIR_TS_DFA_RESOLVERS   tsDfaResolvers = {
                                             _Liveness_Local_GenKill_Resolver,
                                             _Liveness_Init_Resolver,
                                             _Liveness_Iterate_Resolver,
                                             _Liveness_Combine_Resolver,
#if SUPPORT_IPA_DFA
                                             _Liveness_Block_Flow_Combine_From_Callee_Resolver,
                                             _Liveness_Func_Flow_Combine_From_Callers_Resolver
#else
                                             gcvNULL,
                                             gcvNULL
#endif
                                            };

    vscVIR_InitializeBaseTsDFA(&pLvInfo->baseTsDFA,
                               pCg,
                               VIR_DFA_TYPE_LIVE_VAR,
                               flowSize,
                               &pLvInfo->pmp.mmWrapper,
                               &tsDfaResolvers);

    /* Do analysis! */
    vscVIR_DoBackwardIterativeTsDFA(pCg,
                                    &pLvInfo->baseTsDFA,
#if SUPPORT_IPA_DFA
                                    gcvTRUE
#else
                                    gcvFALSE
#endif
                                    );

    /* More strict undefined variables check than DU as this check is based on MOP */
    pInFlowOfMainFunc = (VIR_TS_FUNC_FLOW*)vscSRARR_GetElement(&pLvInfo->baseTsDFA.tsFuncFlowArray,
                                                               CG_GET_MAIN_FUNC(pCg)->pFuncBlock->dgNode.id);
    while ((defIdx = vscBV_FindSetBitForward(&pInFlowOfMainFunc->inFlow, startBitOrdinal)) != (gctUINT)INVALID_BIT_LOC)
    {
        pDef = GET_DEF_BY_IDX(pDefTable, defIdx);
        gcmASSERT(pDef);

        if (!pDef->flags.nativeDefFlags.bIsInput && !pDef->flags.nativeDefFlags.bHwSpecialInput)
        {
            /* Oops, there are still LVs other than inputs at entry of main func!!!
               That means corresponding regs are not def'ed at least on one path. */
#if ENABLE_AGGRESSIVE_IPA_LIVENESS
            gcmASSERT(gcvFALSE);
#endif

            break;
        }

        startBitOrdinal = defIdx + 1;
    }

    /* Mark we have successfully built the LV flow */
    vscVIR_SetDFAFlowBuilt(&pLvInfo->baseTsDFA.baseDFA, gcvTRUE);

    return errCode;
}

VSC_ErrCode vscVIR_BuildLivenessInfo(VIR_CALL_GRAPH* pCg,
                                     VIR_LIVENESS_INFO* pLvInfo,
                                     VIR_DEF_USAGE_INFO* pDuInfo)
{
    VSC_ErrCode            errCode = VSC_ERR_NONE;

    /* Initialize our pmp, you can also use other MM if you want */
    vscPMP_Intialize(&pLvInfo->pmp,
                     gcvNULL,
                     BT_GET_MAX_VALID_ID(&pDuInfo->defTable)*sizeof(gctUINT),
                     sizeof(void*),
                     gcvTRUE);

    pLvInfo->pDuInfo = pDuInfo;

    /* Do global LV analysis */
    errCode = _DoLivenessAnalysis(pCg, pLvInfo);
    CHECK_ERROR(errCode, "Do liveness analysis");

    return errCode;
}

VSC_ErrCode vscVIR_DestroyLivenessInfo(VIR_LIVENESS_INFO* pLvInfo)
{
    VSC_ErrCode            errCode = VSC_ERR_NONE;

    if (vscVIR_CheckDFAFlowBuilt(&pLvInfo->baseTsDFA.baseDFA))
    {
        vscVIR_FinalizeBaseTsDFA(&pLvInfo->baseTsDFA);
        vscPMP_Finalize(&pLvInfo->pmp);

        /* Mark LV info has been invalid */
        vscVIR_SetDFAFlowBuilt(&pLvInfo->baseTsDFA.baseDFA, gcvFALSE);
    }

    return errCode;
}

