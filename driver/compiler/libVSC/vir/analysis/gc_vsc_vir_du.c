/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_vsc.h"

#define DEF_USAGE_HASH_TABLE_SIZE   32
#define INVALID_REG_NO              VIR_INVALID_ID

#define ARE_INSTS_IN_SAME_BASIC_BLOCK(pInst1, pInst2)                    \
    (((pInst1) < VIR_OUTPUT_USAGE_INST) && ((pInst1) != gcvNULL) &&      \
     ((pInst2) < VIR_OUTPUT_USAGE_INST) && ((pInst2) != gcvNULL) &&      \
     (VIR_Inst_GetBasicBlock(pInst1) == VIR_Inst_GetBasicBlock(pInst2)))

#define IS_WEB_CAN_BE_REMOVED(pWeb)                                      \
    (((pWeb)->numOfDef == 0) &&                                          \
     ((pWeb)->firstUsageIdx == VIR_INVALID_USAGE_INDEX) &&               \
     ((pWeb)->firstDefIdx == VIR_INVALID_DEF_INDEX) &&                   \
     ((pWeb)->channelMask == 0))

#define IS_USAGE_CAN_BE_REMOVED(pUsage)                                  \
    (((pUsage)->webIdx == VIR_INVALID_WEB_INDEX) &&                      \
     ((pUsage)->nextWebUsageIdx == VIR_INVALID_USAGE_INDEX) &&           \
     (UD_CHAIN_CHECK_EMPTY(&((pUsage)->udChain))))

static gctUINT _HFUNC_DefPassThroughRegNo(const void* pKey)
{
    VIR_DEF_KEY*     pDefKey = (VIR_DEF_KEY*)pKey;

    gcmASSERT(pDefKey->regNo != INVALID_REG_NO);

    return pDefKey->regNo;
}

static gctBOOL _HKCMP_DefKeyEqual(const void* pHashKey1, const void* pHashKey2)
{
    VIR_DEF_KEY*     pDefKey1 = (VIR_DEF_KEY*)pHashKey1;
    VIR_DEF_KEY*     pDefKey2 = (VIR_DEF_KEY*)pHashKey2;

    gcmASSERT(pDefKey1->regNo != INVALID_REG_NO &&
              pDefKey2->regNo != INVALID_REG_NO);

    if ((pDefKey1->pDefInst == pDefKey2->pDefInst || /* Inst compare */
         pDefKey1->pDefInst == VIR_ANY_DEF_INST   ||
         pDefKey2->pDefInst == VIR_ANY_DEF_INST)
         &&
        (pDefKey1->channel == pDefKey2->channel   || /* Channel compare */
         pDefKey1->channel == VIR_CHANNEL_ANY     ||
         pDefKey2->channel == VIR_CHANNEL_ANY))
    {
        return (pDefKey1->regNo == pDefKey2->regNo);
    }
    else
    {
        return gcvFALSE;
    }
}

static void _InitializeDef(VIR_DEF* pDef,
                           VIR_Instruction* pDefInst,
                           gctUINT regNo,
                           VIR_Enable enableMask,
                           gctUINT8 halfChannelMask,
                           gctUINT8 channel,
                           VIR_DEF_FLAGS defFlags)
{
    pDef->defKey.pDefInst = pDefInst;
    pDef->flags.bIsInput = defFlags.bIsInput;
    pDef->flags.bIsOutput = defFlags.bIsOutput;
    pDef->flags.bNoUsageCrossRoutine = gcvTRUE;
    pDef->flags.bIsPerPrim = defFlags.bIsPerPrim;
    pDef->flags.bIsPerVtxCp = defFlags.bIsPerVtxCp;
    pDef->flags.bHwSpecialInput = defFlags.bHwSpecialInput;
    pDef->flags.bDynIndexed = gcvFALSE;

    pDef->defKey.regNo = regNo;
    pDef->OrgEnableMask = enableMask;
    pDef->halfChannelMask = halfChannelMask;
    pDef->defKey.channel = channel;
    DU_CHAIN_INITIALIZE(&pDef->duChain);

    pDef->nextDefIdxOfSameRegNo = VIR_INVALID_DEF_INDEX;
    pDef->nextDefInWebIdx = VIR_INVALID_DEF_INDEX;
    pDef->webIdx = VIR_INVALID_WEB_INDEX;
}

static void _FinalizeDef(VIR_DEF* pDef)
{
    pDef->defKey.pDefInst = gcvNULL;
    pDef->flags.bIsInput = gcvFALSE;
    pDef->flags.bIsOutput = gcvFALSE;
    pDef->flags.bNoUsageCrossRoutine = gcvTRUE;
    pDef->flags.bIsPerPrim = gcvFALSE;
    pDef->flags.bIsPerVtxCp = gcvFALSE;
    pDef->flags.bHwSpecialInput = gcvFALSE;
    pDef->flags.bDynIndexed = gcvFALSE;
    pDef->defKey.regNo = VIR_INVALID_ID;
    pDef->OrgEnableMask = VIR_ENABLE_NONE;
    pDef->halfChannelMask = VIR_HALF_CHANNEL_MASK_NONE;
    pDef->defKey.channel = VIR_CHANNEL_ANY;
    DU_CHAIN_FINALIZE(&pDef->duChain);

    pDef->nextDefIdxOfSameRegNo = VIR_INVALID_DEF_INDEX;
    pDef->nextDefInWebIdx = VIR_INVALID_DEF_INDEX;
    pDef->webIdx = VIR_INVALID_WEB_INDEX;
}

static gctBOOL _AddNewDefToTable(VIR_DEF_USAGE_INFO* pDuInfo,
                                 VSC_BLOCK_TABLE* pDefTable,
                                 gctUINT firstRegNo,
                                 gctUINT regNoRange,
                                 VIR_Enable defEnableMask,
                                 gctUINT8 halfChannelMask,
                                 VIR_Instruction* pDefInst,
                                 VIR_DEF_FLAGS defFlags,
                                 gctBOOL bCheckRedundant,
                                 gctBOOL bPartialUpdate,
                                 gctUINT* pRetDefIdxArray)
{
    VIR_DEF*               pNewDef;
    VIR_DEF*               pOldDef;
    gctUINT                regNo, newDefIdx, oldDefIdx, firstDefIdxOfSameRegNo;
    VIR_DEF_KEY            defKey;
    gctUINT8               channel;
    VIR_Enable             totalEnableMask = defEnableMask;
    VIR_Enable             needNewAddEnableMask = VIR_ENABLE_NONE;
    gctBOOL                bNewDefAdded = gcvFALSE;

    if (defEnableMask == VIR_ENABLE_NONE || halfChannelMask == VIR_HALF_CHANNEL_MASK_NONE)
    {
        /* So weired, Uha??? */
        gcmASSERT(gcvFALSE);
        return bNewDefAdded;
    }

    for (regNo = firstRegNo; regNo < firstRegNo + regNoRange; regNo ++)
    {
        if (pDuInfo->maxVirRegNo < regNo)
        {
            pDuInfo->maxVirRegNo = regNo;
        }

        if (bCheckRedundant)
        {
            for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
            {
                /* Create a key to search */
                defKey.pDefInst = pDefInst;
                defKey.regNo = regNo;
                defKey.channel = channel;

                oldDefIdx = vscBT_HashSearch(pDefTable, &defKey);
                if (VIR_INVALID_DEF_INDEX != oldDefIdx)
                {
                    /* If we can find it in table, just OR previous enable mask with current one */

                    pOldDef = GET_DEF_BY_IDX(pDefTable, oldDefIdx);
                    gcmASSERT(pOldDef->defKey.channel == channel);
                    gcmASSERT(pOldDef->halfChannelMask == halfChannelMask);

                    VSC_UTILS_SET(pOldDef->OrgEnableMask, defEnableMask);
                    VSC_UTILS_SET(totalEnableMask, pOldDef->OrgEnableMask);

                    if (pRetDefIdxArray && VSC_UTILS_TST_BIT(defEnableMask, channel))
                    {
                        pRetDefIdxArray[(regNo-firstRegNo)*VIR_CHANNEL_NUM+channel] = oldDefIdx;
                    }
                }
                else
                {
                    if (VSC_UTILS_TST_BIT(defEnableMask, channel))
                    {
                        VSC_UTILS_SET_BIT(needNewAddEnableMask, channel);
                    }
                }
            }
        }
        else
        {
            needNewAddEnableMask = defEnableMask;
        }

        /* No need go on if there is no def to be added */
        if (needNewAddEnableMask == 0)
        {
            continue;
        }

        /* For any channel that needs to be newly added into table, add them now */
        for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
        {
            if (!VSC_UTILS_TST_BIT(needNewAddEnableMask, channel))
            {
                continue;
            }

            /* Just get an empty def entry to fill */
            newDefIdx = vscBT_NewEntry(pDefTable);
            pNewDef = GET_DEF_BY_IDX(pDefTable, newDefIdx);
            gcmASSERT(pNewDef);

            /* Initialize def */
            _InitializeDef(pNewDef, pDefInst, regNo, totalEnableMask, halfChannelMask, channel, defFlags);

            if (pRetDefIdxArray)
            {
                pRetDefIdxArray[(regNo-firstRegNo)*VIR_CHANNEL_NUM+channel] = newDefIdx;
            }

            /* Now make all defs with same regNo linked together */
            if (bPartialUpdate && 0)
            {
                /* ???????????? BUG: HOW TO MAKE CORRECT SEQUENCE ??????????? */
            }
            else
            {
                defKey.pDefInst = VIR_ANY_DEF_INST;
                defKey.regNo = regNo;
                defKey.channel = VIR_CHANNEL_ANY;
                firstDefIdxOfSameRegNo = vscBT_HashSearch(pDefTable, &defKey);
                if (VIR_INVALID_DEF_INDEX != firstDefIdxOfSameRegNo)
                {
                    pNewDef->nextDefIdxOfSameRegNo = firstDefIdxOfSameRegNo;
                }
            }

            if (pNewDef->defKey.pDefInst < VIR_INPUT_DEF_INST)
            {
                /* if the def is in STARR set bDynIndexed */
                if (VIR_Inst_GetOpcode(pNewDef->defKey.pDefInst) == VIR_OP_STARR)
                {
                    pNewDef->flags.bDynIndexed = gcvTRUE;
                }
            }

            /* Lastly, add defIdx to hash */
            vscBT_AddToHash(pDefTable, newDefIdx, &pNewDef->defKey);

            bNewDefAdded = gcvTRUE;
        }
    }

    return bNewDefAdded;
}

static gctBOOL _DeleteDefFromTable(VIR_DEF_USAGE_INFO* pDuInfo,
                                   VSC_BLOCK_TABLE* pDefTable,
                                   gctUINT firstRegNo,
                                   gctUINT regNoRange,
                                   VIR_Enable defEnableMask,
                                   gctUINT8 halfChannelMask,
                                   VIR_Instruction* pDefInst,
                                   gctUINT* pRetDefIdxArray)
{
    VSC_BLOCK_TABLE*         pUsageTable = &pDuInfo->usageTable;
    VSC_BLOCK_TABLE*         pWebTable = &pDuInfo->webTable;
    VIR_DEF*                 pDefToDel;
    VIR_DEF*                 pTmpDef;
    VIR_DEF*                 pTmpPrevDef;
    VIR_USAGE*               pUsage;
    VIR_WEB*                 pWeb;
    gctUINT                  regNo, defIdxToDel, tmpDefIdx;
    VIR_DEF_KEY              defKey;
    gctUINT8                 channel;
    VSC_DU_ITERATOR          duIter;
    VIR_DU_CHAIN_USAGE_NODE* pUsageNode;
    VIR_DU_CHAIN_USAGE_NODE* pTmpUsageNode;
    gctBOOL                  bDefDeleted = gcvFALSE;

    if (defEnableMask == VIR_ENABLE_NONE || halfChannelMask == VIR_HALF_CHANNEL_MASK_NONE)
    {
        /* So weired, Uha??? */
        gcmASSERT(gcvFALSE);
        return bDefDeleted;
    }

    for (regNo = firstRegNo; regNo < firstRegNo + regNoRange; regNo ++)
    {
        /* For any channel that needs to be deleted from table, delete them now */
        for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
        {
            if (!VSC_UTILS_TST_BIT(defEnableMask, channel))
            {
                continue;
            }

            defKey.pDefInst = pDefInst;
            defKey.regNo = regNo;
            defKey.channel = channel;
            defIdxToDel = vscBT_HashSearch(pDefTable, &defKey);
            if (VIR_INVALID_DEF_INDEX != defIdxToDel)
            {
                pDefToDel = GET_DEF_BY_IDX(pDefTable, defIdxToDel);
                gcmASSERT(pDefToDel);

                gcmASSERT(pDefToDel->halfChannelMask == halfChannelMask);

                if (pRetDefIdxArray)
                {
                    pRetDefIdxArray[(regNo-firstRegNo)*VIR_CHANNEL_NUM+channel] = defIdxToDel;
                }

                /* Update DU/UD for deleting def */
                VSC_DU_ITERATOR_INIT(&duIter, &pDefToDel->duChain);
                pUsageNode = VSC_DU_ITERATOR_FIRST(&duIter);
                for (; pUsageNode != gcvNULL; )
                {
                    pUsage = GET_USAGE_BY_IDX(pUsageTable, pUsageNode->usageIdx);
                    gcmASSERT(pUsage);

                    /* Remove def from this usage */
                    UD_CHAIN_REMOVE_DEF(&pUsage->udChain, defIdxToDel);

                    /* Must get next because we will delete current one */
                    pTmpUsageNode = VSC_DU_ITERATOR_NEXT(&duIter);

                    /* Remove this usage from du chain of this def */
                    DU_CHAIN_REMOVE_USAGE(&pDefToDel->duChain, pUsageNode);

                    /* Usage node is no longer used any more */
                    vscMM_Free(&pDuInfo->pmp.mmWrapper, pUsageNode);

                    pUsageNode = pTmpUsageNode;
                }

                /* Now update web */
                if (pDefToDel->webIdx != VIR_INVALID_WEB_INDEX)
                {
                    pWeb = GET_WEB_BY_IDX(pWebTable, pDefToDel->webIdx);
                    gcmASSERT(pWeb);

                    /* Channel mask must be re-calc'ed */
                    pWeb->channelMask = 0;

                    /* Remove def from web */
                    tmpDefIdx = pWeb->firstDefIdx;
                    pTmpPrevDef = gcvNULL;
                    while (tmpDefIdx != VIR_INVALID_DEF_INDEX)
                    {
                        pTmpDef = GET_DEF_BY_IDX(pDefTable, tmpDefIdx);
                        gcmASSERT(pTmpDef);

                        if (tmpDefIdx == defIdxToDel)
                        {
                            if (pTmpPrevDef == gcvNULL)
                            {
                                pWeb->firstDefIdx = pTmpDef->nextDefInWebIdx;
                            }
                            else
                            {
                                pTmpPrevDef->nextDefInWebIdx = pTmpDef->nextDefInWebIdx;
                            }

                            pWeb->numOfDef --;
                        }
                        else
                        {
                            pWeb->channelMask |= (1 << pTmpDef->defKey.channel);
                        }

                        pTmpPrevDef = pTmpDef;
                        tmpDefIdx = pTmpDef->nextDefInWebIdx;
                    }

                    /* If web can be removed, remove it now */
                    if (IS_WEB_CAN_BE_REMOVED(pWeb))
                    {
                        vscBT_RemoveEntry(pWebTable, pDefToDel->webIdx);
                    }
                }

                /* Update def link for same reg */
                defKey.pDefInst = VIR_ANY_DEF_INST;
                defKey.regNo = regNo;
                defKey.channel = VIR_CHANNEL_ANY;
                tmpDefIdx = vscBT_HashSearch(pDefTable, &defKey);
                pTmpPrevDef = gcvNULL;
                while (VIR_INVALID_DEF_INDEX != tmpDefIdx)
                {
                    pTmpDef = GET_DEF_BY_IDX(&pDuInfo->defTable, tmpDefIdx);
                    gcmASSERT(pTmpDef);

                    if (tmpDefIdx == defIdxToDel)
                    {
                        if (pTmpPrevDef != gcvNULL)
                        {
                            pTmpPrevDef->nextDefIdxOfSameRegNo = pTmpDef->nextDefIdxOfSameRegNo;
                        }

                        break;
                    }

                    pTmpPrevDef = pTmpDef;

                    /* Get next def with same regNo */
                    tmpDefIdx = pTmpDef->nextDefIdxOfSameRegNo;
                }

                /* Remove it from def-table */
                vscBT_RemoveFromHash(pDefTable, &pDefToDel->defKey);
                _FinalizeDef(pDefToDel);
                vscBT_RemoveEntry(pDefTable, defIdxToDel);

                bDefDeleted = gcvTRUE;
            }
        }
    }

    return bDefDeleted;
}

gctBOOL vscVIR_QueryRealWriteVirRegInfo(VIR_Shader* pShader,
                                        VIR_Instruction* pInst,
                                        VIR_Enable *pDefEnableMask,
                                        gctUINT8 *pHalfChannelMask,
                                        gctUINT* pFirstRegNo,
                                        gctUINT* pRegNoRange,
                                        VIR_DEF_FLAGS* pDefFlags,
                                        gctBOOL* pIsIndexing)
{
    VIR_OperandInfo        operandInfo, operandInfo0;

    if (pInst->dest == gcvNULL)
    {
        return gcvFALSE;
    }

    /* Get dst operand info */
    VIR_Operand_GetOperandInfo(pInst,
                               pInst->dest,
                               &operandInfo);

    if (!VIR_OpndInfo_Is_Virtual_Reg(&operandInfo))
    {
        return gcvFALSE;
    }

    *pDefEnableMask = VIR_Operand_GetEnable(pInst->dest);

    if (pHalfChannelMask)
    {
        *pHalfChannelMask = (gctUINT8)operandInfo.halfChannelMask;
    }

    if (pDefFlags)
    {
        pDefFlags->bIsInput = gcvFALSE;
        pDefFlags->bHwSpecialInput = operandInfo.needHwSpecialDef;
        pDefFlags->bIsOutput = operandInfo.isOutput;
        pDefFlags->bIsPerPrim = VIR_Operand_IsPerPatch(pInst->dest);
        pDefFlags->bIsPerVtxCp = VIR_Operand_IsArrayedPerVertex(pInst->dest);
    }

    if (pIsIndexing)
    {
        *pIsIndexing = gcvFALSE;
    }

    /* A starr inst may potentially write all elements in array */
    if (VIR_Inst_GetOpcode(pInst) == VIR_OP_STARR)
    {
        VIR_Operand_GetOperandInfo(pInst,
                                   pInst->src[VIR_Operand_Src0],
                                   &operandInfo0);

        if (operandInfo0.isImmVal)
        {
            *pFirstRegNo = operandInfo.u1.virRegInfo.virReg + operandInfo0.u1.immValue.iValue;
            *pRegNoRange = 1;
        }
        else
        {
            *pFirstRegNo = operandInfo.u1.virRegInfo.startVirReg;
            *pRegNoRange = operandInfo.u1.virRegInfo.virRegCount;

            if (pIsIndexing)
            {
                *pIsIndexing = gcvTRUE;
            }
        }
    }
    /* Normal def */
    else if (VIR_OPCODE_isWritten2Dest(VIR_Inst_GetOpcode(pInst)))
    {
        /* Then add each def for each reg no */
        *pFirstRegNo = operandInfo.u1.virRegInfo.virReg;
        *pRegNoRange = 1;
    }

    return gcvTRUE;
}

static void _AddRealWriteDefs(VIR_Shader* pShader, VIR_DEF_USAGE_INFO* pDuInfo,
                              VSC_BLOCK_TABLE* pDefTable, VIR_Instruction* pInst)
{
    VIR_Enable             defEnableMask;
    gctUINT                firstRegNo, regNoRange;
    gctUINT8               halfChannelMask;
    VIR_DEF_FLAGS          defFlags;

    if (vscVIR_QueryRealWriteVirRegInfo(pShader,
                                        pInst,
                                        &defEnableMask,
                                        &halfChannelMask,
                                        &firstRegNo,
                                        &regNoRange,
                                        &defFlags,
                                        gcvNULL))
    {
        _AddNewDefToTable(pDuInfo,
                          pDefTable,
                          firstRegNo,
                          regNoRange,
                          defEnableMask,
                          halfChannelMask,
                          pInst,
                          defFlags,
                          gcvFALSE,
                          gcvFALSE,
                          gcvNULL);
    }
}

static void _AddInputDefs(VIR_Shader* pShader, VIR_DEF_USAGE_INFO* pDuInfo,
                          VSC_BLOCK_TABLE* pDefTable, VIR_Instruction* pInst)
{
    VIR_OperandInfo         operandInfo, operandInfo1;
    VIR_Enable              defEnableMask;
    gctUINT                 firstRegNo, regNoRange;
    VIR_DEF_FLAGS           defFlags;
    VIR_SrcOperand_Iterator srcOpndIter;
    VIR_Operand*            pOpnd;

    defFlags.bIsInput = gcvTRUE;
    defFlags.bIsOutput = gcvFALSE;
    defFlags.bHwSpecialInput = gcvFALSE;

    /* A ldarr inst to attribute array may potentially read all elements in array */
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
            if (operandInfo1.isInput && VIR_OpndInfo_Is_Virtual_Reg(&operandInfo1))
            {
                defFlags.bIsPerPrim = VIR_Operand_IsPerPatch(pInst->src[VIR_Operand_Src1]);
                defFlags.bIsPerVtxCp = VIR_Operand_IsArrayedPerVertex(pInst->src[VIR_Operand_Src1]);

                _AddNewDefToTable(pDuInfo,
                                  pDefTable,
                                  operandInfo1.u1.virRegInfo.virReg,
                                  1,
                                  VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(pInst->src[VIR_Operand_Src1])),
                                  (gctUINT8)operandInfo1.halfChannelMask,
                                  VIR_INPUT_DEF_INST,
                                  defFlags,
                                  gcvTRUE,
                                  gcvFALSE,
                                  gcvNULL);
            }

            firstRegNo = operandInfo.u1.virRegInfo.startVirReg;
            regNoRange = operandInfo.u1.virRegInfo.virRegCount;
        }

        if (operandInfo.isInput && VIR_OpndInfo_Is_Virtual_Reg(&operandInfo))
        {
            defEnableMask = VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(pInst->src[VIR_Operand_Src0]));
            defFlags.bIsPerPrim = VIR_Operand_IsPerPatch(pInst->src[VIR_Operand_Src0]);
            defFlags.bIsPerVtxCp = VIR_Operand_IsArrayedPerVertex(pInst->src[VIR_Operand_Src0]);

            _AddNewDefToTable(pDuInfo,
                              pDefTable,
                              firstRegNo,
                              regNoRange,
                              defEnableMask,
                              (gctUINT8)operandInfo.halfChannelMask,
                              VIR_INPUT_DEF_INST,
                              defFlags,
                              gcvTRUE,
                              gcvFALSE,
                              gcvNULL);
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

            if (operandInfo.isInput && VIR_OpndInfo_Is_Virtual_Reg(&operandInfo))
            {
                firstRegNo = operandInfo.u1.virRegInfo.virReg;
                regNoRange = 1;
                defEnableMask = VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(pOpnd));
                defFlags.bIsPerPrim = VIR_Operand_IsPerPatch(pOpnd);
                defFlags.bIsPerVtxCp = VIR_Operand_IsArrayedPerVertex(pOpnd);

                _AddNewDefToTable(pDuInfo,
                                  pDefTable,
                                  firstRegNo,
                                  regNoRange,
                                  defEnableMask,
                                  (gctUINT8)operandInfo.halfChannelMask,
                                  VIR_INPUT_DEF_INST,
                                  defFlags,
                                  gcvTRUE,
                                  gcvFALSE,
                                  gcvNULL);
            }
        }
    }
}

static void _AddHwSpecificDefs(VIR_Shader* pShader, VIR_DEF_USAGE_INFO* pDuInfo,
                               VSC_BLOCK_TABLE* pDefTable, VIR_Instruction* pInst)
{
    VIR_OperandInfo         operandInfo;
    VIR_Enable              defEnableMask;
    gctUINT                 firstRegNo, regNoRange;
    VIR_DEF_FLAGS           defFlags;
    VIR_SrcOperand_Iterator srcOpndIter;
    VIR_Operand*            pOpnd;

    defFlags.bIsInput = gcvFALSE;
    defFlags.bIsOutput = gcvFALSE;
    defFlags.bIsPerPrim = gcvFALSE;
    defFlags.bIsPerVtxCp = gcvFALSE;
    defFlags.bHwSpecialInput = gcvTRUE;

    VIR_SrcOperand_Iterator_Init(pInst, &srcOpndIter);
    pOpnd = VIR_SrcOperand_Iterator_First(&srcOpndIter);

    for (; pOpnd != gcvNULL; pOpnd = VIR_SrcOperand_Iterator_Next(&srcOpndIter))
    {
        VIR_Operand_GetOperandInfo(pInst,
                                   pOpnd,
                                   &operandInfo);

        if (operandInfo.needHwSpecialDef && VIR_OpndInfo_Is_Virtual_Reg(&operandInfo))
        {
            firstRegNo = operandInfo.u1.virRegInfo.virReg;
            regNoRange = 1;
            defEnableMask = VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(pOpnd));

            _AddNewDefToTable(pDuInfo,
                              pDefTable,
                              firstRegNo,
                              regNoRange,
                              defEnableMask,
                              (gctUINT8)operandInfo.halfChannelMask,
                              VIR_HW_SPECIAL_DEF_INST,
                              defFlags,
                              gcvTRUE,
                              gcvFALSE,
                              gcvNULL);
        }
    }
}

static VSC_ErrCode _BuildDefTable(VIR_CALL_GRAPH* pCg, VIR_DEF_USAGE_INFO* pDuInfo)
{
    VSC_ErrCode            errCode = VSC_ERR_NONE;
    VIR_Shader*            pShader = pCg->pOwnerShader;
    CG_ITERATOR            funcBlkIter;
    VIR_FUNC_BLOCK*        pFuncBlk;
    VIR_Function*          pFunc;
    VIR_InstIterator       instIter;
    VIR_Instruction*       pInst;

    /* Initialize def table with hash enabled due to we want to get def index with def content */
    vscBT_Initialize(&pDuInfo->defTable,
                     &pDuInfo->pmp.mmWrapper,
                     VSC_BLOCK_TABLE_FLAG_HASH_ENTRIES,
                     sizeof(VIR_DEF),
                     40*sizeof(VIR_DEF),
                     1,
                     _HFUNC_DefPassThroughRegNo,
                     _HKCMP_DefKeyEqual,
                     DEF_USAGE_HASH_TABLE_SIZE);

    pDuInfo->maxVirRegNo = 0;

    /* Go through whole shader func by func */
    CG_ITERATOR_INIT(&funcBlkIter, pCg);
    pFuncBlk = (VIR_FUNC_BLOCK *)CG_ITERATOR_FIRST(&funcBlkIter);
    for (; pFuncBlk != gcvNULL; pFuncBlk = (VIR_FUNC_BLOCK *)CG_ITERATOR_NEXT(&funcBlkIter))
    {
        pFunc = pFuncBlk->pVIRFunc;

        /* Check each instruction whether it has a virtual reg write */
        VIR_InstIterator_Init(&instIter, &pFunc->instList);
        pInst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);
        for (; pInst != gcvNULL; pInst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter))
        {
            /* Insts who has real writes into vir regs */
            _AddRealWriteDefs(pShader, pDuInfo, &pDuInfo->defTable, pInst);

            /* Input (attribute) reg is not explicitly defined by shader, but they do are
               defined by pre-shader stage. So we need add them to our def table */
            _AddInputDefs(pShader, pDuInfo, &pDuInfo->defTable, pInst);

            /* For MC-level shader, there might be registers that are specially assigned by
               HW to certain HW registers, and these special HW registers will be used like
               input, such as out-sample-mask, so we need add this kind of def to table */
            _AddHwSpecificDefs(pShader, pDuInfo, &pDuInfo->defTable, pInst);
        }
    }

    return errCode;
}

static void _Update_ReachDef_Local_Kill_All_Output_Defs(VIR_DEF_USAGE_INFO* pDuInfo,
                                                        VSC_BLOCK_TABLE* pDefTable,
                                                        VSC_BIT_VECTOR* pGenFlow,
                                                        VSC_BIT_VECTOR* pKillFlow)
{
    gctUINT                  defCount = pDuInfo->baseTsDFA.baseDFA.flowSize;
    VIR_DEF*                 pDef;
    VIR_DEF*                 pThisDef;
    gctUINT                  thisDefIdx, defIdx;
    VIR_DEF_KEY              defKey;
    VSC_BIT_VECTOR           tmpMask;

    vscBV_Initialize(&tmpMask, pDuInfo->baseTsDFA.baseDFA.pMM, pDuInfo->baseTsDFA.baseDFA.flowSize);

    for (thisDefIdx = 0; thisDefIdx < defCount; thisDefIdx ++)
    {
        /* Processed before? */
        if (vscBV_TestBit(&tmpMask, thisDefIdx))
        {
            continue;
        }

        pThisDef = GET_DEF_BY_IDX(pDefTable, thisDefIdx);

        if (pThisDef->flags.bIsOutput)
        {
            defKey.pDefInst = VIR_ANY_DEF_INST;
            defKey.regNo = pThisDef->defKey.regNo;
            defKey.channel = VIR_CHANNEL_ANY;
            defIdx = vscBT_HashSearch(pDefTable, &defKey);

            while (VIR_INVALID_DEF_INDEX != defIdx)
            {
                pDef = GET_DEF_BY_IDX(pDefTable, defIdx);
                gcmASSERT(pDef);

                /* If this def is a real output, kill it */
                if (pDef->flags.bIsOutput)
                {
                    if (pKillFlow)
                    {
                        vscBV_SetBit(pKillFlow, defIdx);
                    }

                    vscBV_ClearBit(pGenFlow, defIdx);
                }

                vscBV_SetBit(&tmpMask, defIdx);

                /* Get next def with same regNo */
                defIdx = pDef->nextDefIdxOfSameRegNo;
            }
        }
    }

    vscBV_Finalize(&tmpMask);
}

static void _Update_ReachDef_Local_GenKill(VSC_BLOCK_TABLE* pDefTable,
                                           VSC_BIT_VECTOR* pGenFlow,
                                           VSC_BIT_VECTOR* pKillFlow,
                                           VSC_STATE_VECTOR* pLocalHalfChannelKillFlow,
                                           VIR_Instruction* pInst,
                                           gctUINT firstRegNo,
                                           gctUINT regNoRange,
                                           VIR_Enable defEnableMask,
                                           gctUINT8 halfChannelMask,
                                           gctBOOL bCertainWrite) /* Means regs in range are 100% written by inst */
{
    VIR_DEF_KEY            defKey;
    gctUINT                regNo, defIdx;
    VIR_DEF*               pDef;
    gctUINT8               channel;
    gctUINT8               killedHalfChannelMask;

    if (defEnableMask == VIR_ENABLE_NONE || halfChannelMask == VIR_HALF_CHANNEL_MASK_NONE)
    {
        /* So weired, Uha??? */
        gcmASSERT(gcvFALSE);
        return;
    }

    /* A def generates this def, but kill all others of same regNo on same channels */
    for (regNo = firstRegNo; regNo < firstRegNo + regNoRange; regNo ++)
    {
        defKey.pDefInst = VIR_ANY_DEF_INST;
        defKey.regNo = regNo;
        defKey.channel = VIR_CHANNEL_ANY;
        defIdx = vscBT_HashSearch(pDefTable, &defKey);

        /* Check all defs with the same channels on same regNo */
        while (VIR_INVALID_DEF_INDEX != defIdx)
        {
            pDef = GET_DEF_BY_IDX(pDefTable, defIdx);
            gcmASSERT(pDef->defKey.regNo == regNo);

            for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
            {
                if (!VSC_UTILS_TST_BIT(defEnableMask, channel))
                {
                    continue;
                }

                /* ONLY consider def with same channel */
                if (channel == pDef->defKey.channel)
                {
                    /* Gen */
                    if (pDef->defKey.pDefInst == pInst)
                    {
                        gcmASSERT(pDef->halfChannelMask == halfChannelMask);

                        vscBV_SetBit(pGenFlow, defIdx);
                    }
                    /* Kill */
                    else if (bCertainWrite)
                    {
                        killedHalfChannelMask = (gctUINT8)vscSV_Get(pLocalHalfChannelKillFlow, defIdx);
                        killedHalfChannelMask |= halfChannelMask;

                        if (VSC_UTILS_TST(killedHalfChannelMask, pDef->halfChannelMask) == pDef->halfChannelMask)
                        {
                            if (pKillFlow)
                            {
                                vscBV_SetBit(pKillFlow, defIdx);
                            }

                            vscBV_ClearBit(pGenFlow, defIdx);

                            vscSV_Set(pLocalHalfChannelKillFlow, defIdx, VIR_HALF_CHANNEL_MASK_NONE);
                        }
                        else
                        {
                            vscSV_Set(pLocalHalfChannelKillFlow, defIdx, killedHalfChannelMask);
                        }
                    }
                }
            }

            /* Get next def with same regNo */
            defIdx = pDef->nextDefIdxOfSameRegNo;
        }
    }
}

static void _ReachDef_Local_GenKill_Resolver(VIR_BASE_TS_DFA* pBaseTsDFA, VIR_TS_BLOCK_FLOW* pTsBlockFlow)
{
    VIR_BASIC_BLOCK*       pBasicBlock = pTsBlockFlow->pOwnerBB;
    VIR_Shader*            pShader = pBasicBlock->pOwnerCFG->pOwnerFuncBlk->pOwnerCG->pOwnerShader;
    VSC_BLOCK_TABLE*       pDefTable = &((VIR_DEF_USAGE_INFO*)pBaseTsDFA)->defTable;
    VSC_BIT_VECTOR*        pGenFlow = &pTsBlockFlow->genFlow;
    VSC_BIT_VECTOR*        pKillFlow = &pTsBlockFlow->killFlow;
    VIR_Instruction*       pInst = BB_GET_START_INST(pBasicBlock);
    VIR_Instruction*       pEndInst = BB_GET_END_INST(pBasicBlock);
    VIR_Enable             defEnableMask;
    gctUINT                firstRegNo, regNoRange;
    gctUINT8               halfChannelMask;
    gctBOOL                bIndexing, bCertainWrite;
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
        if (vscVIR_QueryRealWriteVirRegInfo(pShader,
                                            pInst,
                                            &defEnableMask,
                                            &halfChannelMask,
                                            &firstRegNo,
                                            &regNoRange,
                                            gcvNULL,
                                            &bIndexing))
        {
            bCertainWrite = (!bIndexing &&
                             !VIR_OPCODE_CONDITIONAL_WRITE(VIR_Inst_GetOpcode(pInst)) &&
                             !VIR_OPCODE_DestOnlyUseEnable(VIR_Inst_GetOpcode(pInst)));

            _Update_ReachDef_Local_GenKill(pDefTable,
                                           pGenFlow,
                                           pKillFlow,
                                           &localHalfChannelKillFlow,
                                           pInst,
                                           firstRegNo,
                                           regNoRange,
                                           defEnableMask,
                                           halfChannelMask,
                                           bCertainWrite);
        }

        /* Emit will implicitly kill all output's defs */
        if (VIR_Inst_GetOpcode(pInst) == VIR_OP_EMIT ||
            VIR_Inst_GetOpcode(pInst) == VIR_OP_AQ_EMIT)
        {
            _Update_ReachDef_Local_Kill_All_Output_Defs((VIR_DEF_USAGE_INFO*)pBaseTsDFA,
                                                        pDefTable,
                                                        pGenFlow,
                                                        pKillFlow);
        }

        /* If current inst is the last inst of block, just bail out */
        if (pInst == pEndInst)
        {
            break;
        }

        /* Move to next inst */
        pInst = VIR_Inst_GetNext(pInst);
    }

    /* A full def kill can not be across boundary of basic-block. So for a def, either it is
       fully killed in basic-block, or it is not fully killed. */
    gcmASSERT(vscSV_All(&localHalfChannelKillFlow, VIR_HALF_CHANNEL_MASK_NONE));

    vscSV_Finalize(&localHalfChannelKillFlow);
}

static void _ReachDef_Init_Resolver(VIR_BASE_TS_DFA* pBaseTsDFA, VIR_TS_BLOCK_FLOW* pTsBlockFlow)
{
#if SUPPORT_IPA_DFA
    VIR_CALL_GRAPH*        pCG = pTsBlockFlow->pOwnerBB->pOwnerCFG->pOwnerFuncBlk->pOwnerCG;
#endif
    VSC_BLOCK_TABLE*       pDefTable = &((VIR_DEF_USAGE_INFO*)pBaseTsDFA)->defTable;
    VSC_BIT_VECTOR*        pInFlow = &pTsBlockFlow->inFlow;
    gctUINT                defIdx;
    VIR_DEF*               pDef;

    /* All attributes and hw special inputs are regarded as live at entry block */
    if (
#if SUPPORT_IPA_DFA
        (CG_GET_MAIN_FUNC(pCG) == pTsBlockFlow->pOwnerBB->pOwnerCFG->pOwnerFuncBlk->pVIRFunc) &&
#endif
        (BB_GET_FLOWTYPE(pTsBlockFlow->pOwnerBB) == VIR_FLOW_TYPE_ENTRY)
        )
    {
        for (defIdx = 0; defIdx < (gctUINT)pBaseTsDFA->baseDFA.flowSize; defIdx ++)
        {
            pDef = GET_DEF_BY_IDX(pDefTable, defIdx);

            /* Inputs */
            if (pDef->flags.bIsInput)
            {
                gcmASSERT(pDef->defKey.pDefInst == VIR_INPUT_DEF_INST);

                vscBV_SetBit(pInFlow, defIdx);
            }

            /* HW special inputs */
            if (pDef->flags.bHwSpecialInput && !pDef->flags.bIsOutput)
            {
                gcmASSERT(pDef->defKey.pDefInst == VIR_HW_SPECIAL_DEF_INST);

                vscBV_SetBit(pInFlow, defIdx);
            }
        }
    }
}

static gctBOOL _ReachDef_Iterate_Resolver(VIR_BASE_TS_DFA* pBaseTsDFA, VIR_TS_BLOCK_FLOW* pTsBlockFlow)
{
    VSC_BIT_VECTOR*        pInFlow = &pTsBlockFlow->inFlow;
    VSC_BIT_VECTOR*        pOutFlow = &pTsBlockFlow->outFlow;
    VSC_BIT_VECTOR*        pGenFlow = &pTsBlockFlow->genFlow;
    VSC_BIT_VECTOR*        pKillFlow = &pTsBlockFlow->killFlow;
    VSC_BIT_VECTOR         tmpFlow;
    gctBOOL                bChanged = gcvFALSE;

    vscBV_Initialize(&tmpFlow, pBaseTsDFA->baseDFA.pMM, pBaseTsDFA->baseDFA.flowSize);

    /* Out = Gen U (In - Kill) */
    vscBV_Minus2(&tmpFlow, pInFlow, pKillFlow);
    vscBV_Or1(&tmpFlow, pGenFlow);

    bChanged = !vscBV_Equal(&tmpFlow, pOutFlow);
    if (bChanged)
    {
        vscBV_Copy(pOutFlow, &tmpFlow);
    }

    vscBV_Finalize(&tmpFlow);

    return bChanged;
}

static gctBOOL _ReachDef_Combine_Resolver(VIR_BASE_TS_DFA* pBaseTsDFA, VIR_TS_BLOCK_FLOW* pTsBlockFlow)
{
    VIR_BASIC_BLOCK*             pPredBasicBlk;
    VSC_ADJACENT_LIST_ITERATOR   predEdgeIter;
    VIR_CFG_EDGE*                pPredEdge;
    VIR_BASIC_BLOCK*             pBasicBlock = pTsBlockFlow->pOwnerBB;
    VSC_BIT_VECTOR*              pInFlow = &pTsBlockFlow->inFlow;
    VSC_BIT_VECTOR               tmpFlow;
    gctBOOL                      bChanged = gcvFALSE;

    /* If there is no predecessors, then just reture FALSE */
    if (DGND_GET_IN_DEGREE(&pBasicBlock->dgNode) == 0)
    {
        return gcvFALSE;
    }

    vscBV_Initialize(&tmpFlow, pBaseTsDFA->baseDFA.pMM, pBaseTsDFA->baseDFA.flowSize);

    /* In = U all-pred-Outs */
    VSC_ADJACENT_LIST_ITERATOR_INIT(&predEdgeIter, &pBasicBlock->dgNode.predList);
    pPredEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&predEdgeIter);
    for (; pPredEdge != gcvNULL; pPredEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&predEdgeIter))
    {
        pPredBasicBlk = CFG_EDGE_GET_TO_BB(pPredEdge);
        vscBV_Or1(&tmpFlow, &pPredBasicBlk->pTsWorkDataFlow->outFlow);
    }

    bChanged = !vscBV_Equal(&tmpFlow, pInFlow);
    if (bChanged)
    {
        vscBV_Copy(pInFlow, &tmpFlow);
    }

    vscBV_Finalize(&tmpFlow);

    return bChanged;
}

#if SUPPORT_IPA_DFA
static gctBOOL _ReachDef_Block_Flow_Combine_From_Callee_Resolver(VIR_BASE_TS_DFA* pBaseTsDFA, VIR_TS_BLOCK_FLOW* pCallerTsBlockFlow)
{
    VIR_BASIC_BLOCK*       pBasicBlock = pCallerTsBlockFlow->pOwnerBB;
    VSC_BIT_VECTOR*        pOutFlow = &pCallerTsBlockFlow->outFlow;
    VSC_BIT_VECTOR*        pInFlow = &pCallerTsBlockFlow->inFlow;
    VIR_FUNC_BLOCK*        pCallee = VIR_Inst_GetCallee(pBasicBlock->pStartInst)->pFuncBlock;
    VIR_TS_FUNC_FLOW*      pCalleeFuncFlow = (VIR_TS_FUNC_FLOW*)vscSRARR_GetElement(&pBaseTsDFA->tsFuncFlowArray, pCallee->dgNode.id);
    VSC_BIT_VECTOR         tmpFlow;
    gctBOOL                bChanged = gcvFALSE;

    gcmASSERT(pBasicBlock->flowType == VIR_FLOW_TYPE_CALL);

    vscBV_Initialize(&tmpFlow, pBaseTsDFA->baseDFA.pMM, pBaseTsDFA->baseDFA.flowSize);

    /* U flow that not flows into callee and out flow of callee */
    vscBV_And2(&tmpFlow, &pCalleeFuncFlow->inFlow, pInFlow);
    vscBV_Minus2(&tmpFlow, pInFlow, &tmpFlow);
    vscBV_Or1(&tmpFlow, &pCalleeFuncFlow->outFlow);

    bChanged = !vscBV_Equal(pOutFlow, &tmpFlow);
    if (bChanged)
    {
        vscBV_Copy(pOutFlow, &tmpFlow);
    }

    vscBV_Finalize(&tmpFlow);

    return bChanged;
}

static void _And_Def_BVs_On_Same_Reg_No(VSC_MM* pMM, VSC_BLOCK_TABLE* pDefTable,
                                        VSC_BIT_VECTOR* pDefBV1, VSC_BIT_VECTOR* pDefBV2,
                                        gctUINT maxVirRegNo)
{
    VSC_BIT_VECTOR               regNoBV1, regNoBV2, andResBV;
    gctUINT                      regNoBVSize = (maxVirRegNo + 1) * VIR_CHANNEL_NUM;
    gctUINT                      defIdx, startBitOrdinal;
    VIR_DEF*                     pDef;

    vscBV_Initialize(&regNoBV1, pMM, regNoBVSize);
    vscBV_Initialize(&regNoBV2, pMM, regNoBVSize);
    vscBV_Initialize(&andResBV, pMM, regNoBVSize);

    /* Convert BV on def to BV on regNo */
    startBitOrdinal = 0;
    while ((defIdx = vscBV_FindSetBitForward(pDefBV1, startBitOrdinal)) != (gctUINT)INVALID_BIT_LOC)
    {
        pDef = GET_DEF_BY_IDX(pDefTable, defIdx);
        gcmASSERT(pDef);

        vscBV_SetBit(&regNoBV1, pDef->defKey.regNo*VIR_CHANNEL_NUM+pDef->defKey.channel);
        startBitOrdinal = defIdx + 1;
    }

    startBitOrdinal = 0;
    while ((defIdx = vscBV_FindSetBitForward(pDefBV2, startBitOrdinal)) != (gctUINT)INVALID_BIT_LOC)
    {
        pDef = GET_DEF_BY_IDX(pDefTable, defIdx);
        gcmASSERT(pDef);

        vscBV_SetBit(&regNoBV2, pDef->defKey.regNo*VIR_CHANNEL_NUM+pDef->defKey.channel);
        startBitOrdinal = defIdx + 1;
    }

    /* And regNo BVs */
    vscBV_And2(&andResBV, &regNoBV1, &regNoBV2);

    /* Change def BVs based on and'ed result of regNo BVs */
    startBitOrdinal = 0;
    while ((defIdx = vscBV_FindSetBitForward(pDefBV1, startBitOrdinal)) != (gctUINT)INVALID_BIT_LOC)
    {
        pDef = GET_DEF_BY_IDX(pDefTable, defIdx);
        gcmASSERT(pDef);

        if (!vscBV_TestBit(&andResBV, pDef->defKey.regNo*VIR_CHANNEL_NUM+pDef->defKey.channel))
        {
            vscBV_ClearBit(pDefBV1, defIdx);
        }

        startBitOrdinal = defIdx + 1;
    }

    startBitOrdinal = 0;
    while ((defIdx = vscBV_FindSetBitForward(pDefBV2, startBitOrdinal)) != (gctUINT)INVALID_BIT_LOC)
    {
        pDef = GET_DEF_BY_IDX(pDefTable, defIdx);
        gcmASSERT(pDef);

        if (!vscBV_TestBit(&andResBV, pDef->defKey.regNo*VIR_CHANNEL_NUM+pDef->defKey.channel))
        {
            vscBV_ClearBit(pDefBV2, defIdx);
        }

        startBitOrdinal = defIdx + 1;
    }

    vscBV_Finalize(&regNoBV1);
    vscBV_Finalize(&regNoBV2);
    vscBV_Finalize(&andResBV);
}

static gctBOOL _ReachDef_Func_Flow_Combine_From_Callers_Resolver(VIR_BASE_TS_DFA* pBaseTsDFA, VIR_TS_FUNC_FLOW* pCalleeTsFuncFlow)
{
    gctUINT                      callerIdx;
    VIR_BASIC_BLOCK*             pCallerBasicBlk;
    VIR_Instruction*             pCallSiteInst;
    VIR_FUNC_BLOCK*              pCalleeFuncBlock = pCalleeTsFuncFlow->pOwnerFB;
    VSC_BIT_VECTOR*              pInFlow = &pCalleeTsFuncFlow->inFlow;
    VSC_ADJACENT_LIST_ITERATOR   callerIter;
    VIR_CG_EDGE*                 pCallerEdge;
    VSC_BIT_VECTOR               tmpFlow, callerInFlow;
    gctBOOL                      bChanged = gcvFALSE, bFirstCallSite = gcvTRUE;
    gctUINT                      maxVirRegNo = ((VIR_DEF_USAGE_INFO*)pBaseTsDFA)->maxVirRegNo;
    VSC_BLOCK_TABLE*             pDefTable = &((VIR_DEF_USAGE_INFO*)pBaseTsDFA)->defTable;

    vscBV_Initialize(&tmpFlow, pBaseTsDFA->baseDFA.pMM, pBaseTsDFA->baseDFA.flowSize);
    vscBV_Initialize(&callerInFlow, pBaseTsDFA->baseDFA.pMM, pBaseTsDFA->baseDFA.flowSize);

    /* U all in flow of caller at every call site */
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

            if (pCallerBasicBlk == gcvNULL)
            {
                gcmASSERT(gcvFALSE);
                continue;
            }

            vscBV_Copy(&callerInFlow, &pCallerBasicBlk->pTsWorkDataFlow->inFlow);

            if (bFirstCallSite)
            {
                bFirstCallSite = gcvFALSE;
            }
            else
            {
                _And_Def_BVs_On_Same_Reg_No(pBaseTsDFA->baseDFA.pMM, pDefTable, &tmpFlow, &callerInFlow, maxVirRegNo);
            }

            vscBV_Or1(&tmpFlow, &callerInFlow);
        }
    }

    bChanged = !vscBV_Equal(&tmpFlow, pInFlow);
    if (bChanged)
    {
        vscBV_Copy(pInFlow, &tmpFlow);
    }

    vscBV_Finalize(&tmpFlow);
    vscBV_Finalize(&callerInFlow);

    return bChanged;
}
#endif

static VSC_ErrCode _DoReachDefAnalysis(VIR_CALL_GRAPH* pCg, VIR_DEF_USAGE_INFO* pDuInfo)
{
    VSC_ErrCode            errCode = VSC_ERR_NONE;
    gctUINT                flowSize = BT_GET_MAX_VALID_ID(&pDuInfo->defTable);
    VIR_TS_DFA_RESOLVERS   tsDfaResolvers = {
                                             _ReachDef_Local_GenKill_Resolver,
                                             _ReachDef_Init_Resolver,
                                             _ReachDef_Iterate_Resolver,
                                             _ReachDef_Combine_Resolver,
#if SUPPORT_IPA_DFA
                                             _ReachDef_Block_Flow_Combine_From_Callee_Resolver,
                                             _ReachDef_Func_Flow_Combine_From_Callers_Resolver
#endif
                                            };

    vscVIR_InitializeBaseTsDFA(&pDuInfo->baseTsDFA,
                               pCg,
                               VIR_DFA_TYPE_REACH_DEF,
                               flowSize,
                               &pDuInfo->pmp.mmWrapper,
                               &tsDfaResolvers);

    /* Do analysis! */
    vscVIR_DoForwardIterativeTsDFA(pCg, &pDuInfo->baseTsDFA);

    return errCode;
}

gctBOOL DEF_INDEX_CMP(void* pNode1, void* pNode2)
{
    gctUINT* pDefIndex1 = (gctUINT*)pNode1;
    gctUINT* pDefIndex2 = (gctUINT*)pNode2;

    return (*pDefIndex1 == *pDefIndex2);
}

static gctUINT _HFUNC_UsageInstLSB8(const void* pKey)
{
    VIR_USAGE_KEY*     pUsageKey = (VIR_USAGE_KEY*)pKey;

    return ((gctUINT)(gctUINTPTR_T)pUsageKey->pUsageInst & 0xFF);
}

static gctBOOL _HKCMP_UsageKeyEqual(const void* pHashKey1, const void* pHashKey2)
{
    VIR_USAGE_KEY*     pUsageKey1 = (VIR_USAGE_KEY*)pHashKey1;
    VIR_USAGE_KEY*     pUsageKey2 = (VIR_USAGE_KEY*)pHashKey2;

    if (pUsageKey1->pOperand == pUsageKey2->pOperand  ||
        pUsageKey1->pOperand == VIR_ANY_USAGE_OPERAND ||
        pUsageKey2->pOperand == VIR_ANY_USAGE_OPERAND)
    {
        return (pUsageKey1->pUsageInst == pUsageKey2->pUsageInst);
    }
    else
    {
        return gcvFALSE;
    }
}

static void _InitializeUsage(VIR_USAGE* pUsage,
                             VSC_MM* pMM,
                             VIR_Instruction* pUsageInst,
                             VIR_Operand* pOperand,
                             gctUINT8 realChannelMask,
                             gctUINT8 halfChannelMask)
{
    pUsage->usageKey.pUsageInst = pUsageInst;
    pUsage->usageKey.pOperand = pOperand;

    pUsage->realChannelMask = realChannelMask;
    pUsage->halfChannelMask = halfChannelMask;

    pUsage->webIdx = VIR_INVALID_WEB_INDEX;
    pUsage->nextWebUsageIdx = VIR_INVALID_USAGE_INDEX;

    UD_CHAIN_INITIALIZE(&pUsage->udChain);
}

static void _FinalizeUsage(VIR_USAGE* pUsage)
{
    pUsage->usageKey.pUsageInst = gcvNULL;
    pUsage->usageKey.pOperand = VIR_INVALID_USAGE_OPERAND;

    pUsage->realChannelMask = 0;

    pUsage->webIdx = VIR_INVALID_WEB_INDEX;
    pUsage->nextWebUsageIdx = VIR_INVALID_USAGE_INDEX;

    UD_CHAIN_FINALIZE(&pUsage->udChain);
}

void vscUSGN_Initialize(VIR_DU_CHAIN_USAGE_NODE* pUsageNode, gctUINT usageIndex)
{
    vscULN_Initialize(&pUsageNode->uniLstNode);
    pUsageNode->usageIdx = usageIndex;
}

void vscUSGN_Finalize(VIR_DU_CHAIN_USAGE_NODE* pUsageNode)
{
    vscULN_Finalize(&pUsageNode->uniLstNode);
}

static gctBOOL _AddNewUsageToTable(VIR_DEF_USAGE_INFO* pDuInfo,
                                   VSC_BIT_VECTOR *pWorkingDefFlow,
                                   VIR_Instruction* pUsageInst,
                                   VIR_Operand* pOperand,
                                   gctUINT firstRegNo,
                                   gctUINT regNoRange,
                                   VIR_Enable defEnableMask,
                                   gctUINT8 halfChannelMask,
                                   gctBOOL bPartialUpdate,
                                   gctUINT* pRetUsageIdx)
{
    VIR_USAGE*               pNewUsage = gcvNULL;
    VIR_USAGE*               pTmpUsage;
    VIR_DEF*                 pDef;
    VIR_DEF_KEY              defKey;
    VIR_USAGE_KEY            usageKey;
    VSC_BLOCK_TABLE*         pDefTable = &pDuInfo->defTable;
    VSC_BLOCK_TABLE*         pUsageTable = &pDuInfo->usageTable;
    gctUINT                  regNo, defIdx;
    gctUINT                  newUsageIdx = VIR_INVALID_USAGE_INDEX;
    gctUINT8                 channel;
    gctBOOL                  bDefFound = gcvFALSE, bNewUsageAdded = gcvFALSE;
    VIR_DU_CHAIN_USAGE_NODE* pUsageNode;
    VSC_DU_ITERATOR          duIter;

    if (defEnableMask == VIR_ENABLE_NONE || halfChannelMask == VIR_HALF_CHANNEL_MASK_NONE)
    {
        /* So weired, Uha??? */
        gcmASSERT(gcvFALSE);
        return bNewUsageAdded;
    }

    if (bPartialUpdate)
    {
        usageKey.pOperand = pOperand;
        usageKey.pUsageInst = pUsageInst;
        newUsageIdx = vscBT_HashSearch(&pDuInfo->usageTable, &usageKey);
        if (VIR_INVALID_USAGE_INDEX != newUsageIdx)
        {
            pNewUsage = GET_USAGE_BY_IDX(pUsageTable, newUsageIdx);
            gcmASSERT(pNewUsage);

            gcmASSERT(pNewUsage->halfChannelMask == halfChannelMask);

            pNewUsage->realChannelMask |= defEnableMask;
        }
    }

    if (newUsageIdx == VIR_INVALID_USAGE_INDEX)
    {
        /* Just get an empty usage entry to fill */
        newUsageIdx = vscBT_NewEntry(pUsageTable);
        pNewUsage = GET_USAGE_BY_IDX(pUsageTable, newUsageIdx);
        gcmASSERT(pNewUsage);

        /* Initialize usage */
        _InitializeUsage(pNewUsage, &pDuInfo->pmp.mmWrapper, pUsageInst, pOperand, defEnableMask, halfChannelMask);

        /* Add usageIdx to hash */
        vscBT_AddToHash(pUsageTable, newUsageIdx, &pNewUsage->usageKey);

        bNewUsageAdded = gcvTRUE;
    }

    if (pRetUsageIdx)
    {
        *pRetUsageIdx = newUsageIdx;
    }

    for (regNo = firstRegNo; regNo < firstRegNo + regNoRange; regNo ++)
    {
        /* For any channel that is used, try to find its def */
        for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
        {
            if (!VSC_UTILS_TST_BIT(defEnableMask, channel))
            {
                continue;
            }

            defKey.pDefInst = VIR_ANY_DEF_INST;
            defKey.regNo = regNo;
            defKey.channel = VIR_CHANNEL_ANY;
            defIdx = vscBT_HashSearch(pDefTable, &defKey);

            bDefFound = gcvFALSE;

            while (VIR_INVALID_DEF_INDEX != defIdx)
            {
                pDef = GET_DEF_BY_IDX(pDefTable, defIdx);
                gcmASSERT(pDef);

                /* If this def on same channel flows with intersection of half-channle mask at current inst,
                   yes, we find a corresponding def */
                if (vscBV_TestBit(pWorkingDefFlow, defIdx) &&
                    (pDef->defKey.channel == channel) &&
                    VSC_UTILS_TST(pDef->halfChannelMask, halfChannelMask))
                {
                    /* If usage is already there, we need check whether def and usage have made a connection before */
                    pUsageNode = gcvNULL;
                    if (bPartialUpdate && !bNewUsageAdded)
                    {
                        VSC_DU_ITERATOR_INIT(&duIter, &pDef->duChain);
                        pUsageNode = VSC_DU_ITERATOR_FIRST(&duIter);
                        for (; pUsageNode != gcvNULL; pUsageNode = VSC_DU_ITERATOR_NEXT(&duIter))
                        {
                            if (pUsageNode->usageIdx == newUsageIdx)
                            {
                                pTmpUsage = GET_USAGE_BY_IDX(pUsageTable, pUsageNode->usageIdx);

                                if (pTmpUsage)
                                {
                                    gcmASSERT(UD_CHAIN_CHECK_DEF(&pTmpUsage->udChain, defIdx));
                                }
                                else
                                {
                                    gcmASSERT(gcvFALSE);
                                }

                                break;
                            }
                        }
                    }

                    if (pUsageNode == gcvNULL)
                    {
                        /* DU chain */
                        pUsageNode = (VIR_DU_CHAIN_USAGE_NODE*)vscMM_Alloc(&pDuInfo->pmp.mmWrapper,
                                                                           sizeof(VIR_DU_CHAIN_USAGE_NODE));
                        vscUSGN_Initialize(pUsageNode, newUsageIdx);
                        DU_CHAIN_ADD_USAGE(&pDef->duChain, pUsageNode);

                        /* If this usage locates function other that def's, clear bNoUsageCrossRoutine */
                        if (pDef->defKey.pDefInst < VIR_INPUT_DEF_INST && pUsageInst < VIR_OUTPUT_USAGE_INST)
                        {
                            pDef->flags.bNoUsageCrossRoutine &=
                                (VIR_Inst_GetFunction(pUsageInst) == VIR_Inst_GetFunction(pDef->defKey.pDefInst));
                        }

                        if (pUsageInst < VIR_OUTPUT_USAGE_INST)
                        {
                            /* if the use is in LDARR set bDynIndexed */
                            if (VIR_Inst_GetOpcode(pUsageInst) == VIR_OP_LDARR &&
                                VIR_Inst_GetSourceIndex(pUsageInst, pOperand) == 0)
                            {
                                pDef->flags.bDynIndexed = gcvTRUE;
                            }
                        }

                        /* UD-chain */
                        UD_CHAIN_ADD_DEF(&pNewUsage->udChain, defIdx);
                    }
                    else if (bPartialUpdate)
                    {
                        vscBV_ClearBit(pWorkingDefFlow, defIdx);
                    }

                    bDefFound = gcvTRUE;
                }

                /* Get next def with same regNo */
                defIdx = pDef->nextDefIdxOfSameRegNo;
            }

            /* Note that even if we say we find a def here, but we can not deduce all pathes have def for
               this usage!!!!! To exactly check undefined usages, must do LV analysis. */
            if (!bDefFound)
            {
                /* Oops, this usage has no def!!!!!! */

                /* As current indexing range determination in IR is very conservative, so this range may
                   include undefined regs or undefined channels which are meaningful for application because
                   application DOES really not use them. So before we resolve accurate indexing range in IR,
                   ingore following check for range */
                if (regNoRange == 1)
                {
                    gcmASSERT(gcvFALSE);
                }
            }
        }
    }

    return bNewUsageAdded;
}

static gctBOOL _DeleteUsageFromTable(VIR_DEF_USAGE_INFO* pDuInfo,
                                     VIR_Instruction* pDefInst,
                                     VIR_Instruction* pUsageInst,
                                     VIR_Operand* pOperand,
                                     gctUINT firstUsageRegNo,
                                     gctUINT usageRegNoRange,
                                     VIR_Enable defEnableMask,
                                     gctUINT8 halfChannelMask,
                                     gctUINT* pRetUsageIdx)
{
    VSC_BLOCK_TABLE*         pDefTable = &pDuInfo->defTable;
    VSC_BLOCK_TABLE*         pUsageTable = &pDuInfo->usageTable;
    VSC_BLOCK_TABLE*         pWebTable = &pDuInfo->webTable;
    VIR_USAGE*               pUsageToDel;
    VIR_USAGE*               pTmpUsage;
    VIR_USAGE*               pTmpPreUsage;
    VIR_DEF*                 pDef;
    VIR_WEB*                 pWeb;
    VIR_DEF_KEY              defKey;
    VIR_USAGE_KEY            usageKey;
    gctBOOL                  bUsageDeleted = gcvFALSE;
    gctUINT                  usageIdxToDel, regNo, defIdx, tmpUsageIdx;
    gctUINT8                 channel;
    VIR_DU_CHAIN_USAGE_NODE* pUsageNode;
    VSC_DU_ITERATOR          duIter;

    if (defEnableMask == VIR_ENABLE_NONE || halfChannelMask == VIR_HALF_CHANNEL_MASK_NONE)
    {
        /* So weired, Uha??? */
        gcmASSERT(gcvFALSE);
        return bUsageDeleted;
    }

    usageKey.pOperand = pOperand;
    usageKey.pUsageInst = pUsageInst;
    usageIdxToDel = vscBT_HashSearch(&pDuInfo->usageTable, &usageKey);

    if (VIR_INVALID_USAGE_INDEX == usageIdxToDel)
    {
        return bUsageDeleted;
    }

    pUsageToDel = GET_USAGE_BY_IDX(pUsageTable, usageIdxToDel);
    gcmASSERT(pUsageToDel);

    gcmASSERT(pUsageToDel->halfChannelMask == halfChannelMask);

    if (pRetUsageIdx)
    {
        *pRetUsageIdx = usageIdxToDel;
    }

    /* Update DU/UD chain for requested defs */
    for (regNo = firstUsageRegNo; regNo < firstUsageRegNo + usageRegNoRange; regNo ++)
    {
        /* For any channel that needs to be checked */
        for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
        {
            if (!VSC_UTILS_TST_BIT(defEnableMask, channel))
            {
                continue;
            }

            defKey.pDefInst = pDefInst;
            defKey.regNo = regNo;
            defKey.channel = (pDefInst == VIR_ANY_DEF_INST) ? VIR_CHANNEL_ANY : channel;
            defIdx = vscBT_HashSearch(pDefTable, &defKey);
            while (VIR_INVALID_DEF_INDEX != defIdx)
            {
                pDef = GET_DEF_BY_IDX(pDefTable, defIdx);
                gcmASSERT(pDef);

                if (pDef->defKey.channel == channel)
                {
                    VSC_DU_ITERATOR_INIT(&duIter, &pDef->duChain);
                    pUsageNode = VSC_DU_ITERATOR_FIRST(&duIter);
                    for (; pUsageNode != gcvNULL; pUsageNode = VSC_DU_ITERATOR_NEXT(&duIter))
                    {
                        if (pUsageNode->usageIdx == usageIdxToDel)
                        {
                            pTmpUsage = GET_USAGE_BY_IDX(pUsageTable, pUsageNode->usageIdx);
                            gcmASSERT(pTmpUsage);

                            /* Remove this usage from du chain of this def */
                            DU_CHAIN_REMOVE_USAGE(&pDef->duChain, pUsageNode);

                            /* Remove def from this usage */
                            UD_CHAIN_REMOVE_DEF(&pTmpUsage->udChain, defIdx);

                            break;
                        }
                    }
                }

                /* If not for any inst, just bail out */
                if (pDefInst != VIR_ANY_DEF_INST)
                {
                    break;
                }

                /* Get next def with same regNo */
                defIdx = pDef->nextDefIdxOfSameRegNo;
            }
        }
    }

    /* Update web */
    if (pUsageToDel->webIdx != VIR_INVALID_WEB_INDEX &&
        UD_CHAIN_CHECK_EMPTY(&pUsageToDel->udChain))
    {
        pWeb = GET_WEB_BY_IDX(pWebTable, pUsageToDel->webIdx);
        gcmASSERT(pWeb);

        /* Remove usage from web */
        tmpUsageIdx = pWeb->firstUsageIdx;
        pTmpPreUsage = gcvNULL;
        while (tmpUsageIdx != VIR_INVALID_USAGE_INDEX)
        {
            pTmpUsage = GET_USAGE_BY_IDX(pUsageTable, tmpUsageIdx);
            gcmASSERT(pTmpUsage);

            if (tmpUsageIdx == usageIdxToDel)
            {
                if (pTmpPreUsage == gcvNULL)
                {
                    pWeb->firstUsageIdx = pTmpUsage->nextWebUsageIdx;
                }
                else
                {
                    pTmpPreUsage->nextWebUsageIdx = pTmpUsage->nextWebUsageIdx;
                }

                break;
            }

            pTmpPreUsage = pTmpUsage;
            tmpUsageIdx = pTmpUsage->nextWebUsageIdx;
        }

        /* If web can be removed, remove it now */
        if (IS_WEB_CAN_BE_REMOVED(pWeb))
        {
            vscBT_RemoveEntry(pWebTable, pUsageToDel->webIdx);
        }

        pUsageToDel->webIdx = VIR_INVALID_WEB_INDEX;
        pUsageToDel->nextWebUsageIdx = VIR_INVALID_USAGE_INDEX;
    }

    /* If this usage dose not attach to anything, just delete it now */
    if (IS_USAGE_CAN_BE_REMOVED(pUsageToDel))
    {
        vscBT_RemoveFromHash(pUsageTable, &pUsageToDel->usageKey);
        _FinalizeUsage(pUsageToDel);
        vscBT_RemoveEntry(pUsageTable, usageIdxToDel);

        bUsageDeleted = gcvTRUE;
    }

    return bUsageDeleted;
}

static void _CheckFalseOutput(VIR_DEF_USAGE_INFO* pDuInfo, VIR_DEF* pOutputDef)
{
    VSC_BLOCK_TABLE*         pUsageTable = &pDuInfo->usageTable;
    VIR_USAGE*               pUsage;
    VIR_DU_CHAIN_USAGE_NODE* pUsageNode;
    VSC_DU_ITERATOR          duIter;
    gctBOOL                  bTrueOutput = gcvFALSE;

    VSC_DU_ITERATOR_INIT(&duIter, &pOutputDef->duChain);
    pUsageNode = VSC_DU_ITERATOR_FIRST(&duIter);
    for (; pUsageNode != gcvNULL; pUsageNode = VSC_DU_ITERATOR_NEXT(&duIter))
    {
        pUsage = GET_USAGE_BY_IDX(pUsageTable, pUsageNode->usageIdx);
        gcmASSERT(pUsage);

        if (IS_OUTPUT_USAGE(pUsage))
        {
            bTrueOutput = gcvTRUE;
            break;
        }
    }

    if (!bTrueOutput)
    {
        pOutputDef->flags.bIsOutput = gcvFALSE;
    }
    else
    {
        gcmASSERT(pOutputDef->flags.bIsOutput);
    }
}

static gctBOOL _CanAddUsageToOutputDef(VIR_DEF_USAGE_INFO* pDuInfo,
                                       VSC_BIT_VECTOR* pWorkingDefFlow,
                                       gctUINT outputDefIdx,
                                       VIR_Instruction* pOutputUsageInst)
{
    VSC_BLOCK_TABLE*         pDefTable = &pDuInfo->defTable;
    VIR_DEF*                 pOutputDef = GET_DEF_BY_IDX(pDefTable, outputDefIdx);

    gcmASSERT(pOutputDef->flags.bIsOutput);

    if (pOutputUsageInst == VIR_OUTPUT_USAGE_INST)
    {
        /* Calling here must be at the end of main routine to determine implicit
           output usages for FFU. */

        /* Check if it is a false output */
        if (!vscBV_TestBit(pWorkingDefFlow, outputDefIdx))
        {
            _CheckFalseOutput(pDuInfo, pOutputDef);
        }

        /* For a true outupt that flows to the end of main routine */
        if (pOutputDef->flags.bIsOutput &&
            vscBV_TestBit(pWorkingDefFlow, outputDefIdx))
        {
            return gcvTRUE;
        }
    }
    else
    {
        /* Calling here is because we need determine implicits output usages for EMIT */

        gcmASSERT(VIR_Inst_GetOpcode(pOutputUsageInst) == VIR_OP_EMIT ||
                  VIR_Inst_GetOpcode(pOutputUsageInst) == VIR_OP_AQ_EMIT);

        if (vscBV_TestBit(pWorkingDefFlow, outputDefIdx))
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

static void _AddOutputUsages(VIR_DEF_USAGE_INFO* pDuInfo,
                             VSC_BIT_VECTOR* pWorkingDefFlow,
                             VIR_Instruction* pOutputUsageInst)
{
    VSC_BLOCK_TABLE*         pDefTable = &pDuInfo->defTable;
    VSC_BLOCK_TABLE*         pUsageTable = &pDuInfo->usageTable;
    gctUINT                  defCount = pDuInfo->baseTsDFA.baseDFA.flowSize;
    VIR_DEF*                 pDef;
    VIR_DEF*                 pThisDef;
    gctUINT                  newUsageIdx, thisDefIdx, defIdx;
    VIR_USAGE*               pNewUsage;
    VIR_DU_CHAIN_USAGE_NODE* pUsageNode;
    VIR_DEF_KEY              defKey;
    VSC_BIT_VECTOR           tmpMask;

    vscBV_Initialize(&tmpMask, pDuInfo->baseTsDFA.baseDFA.pMM, pDuInfo->baseTsDFA.baseDFA.flowSize);

    for (thisDefIdx = 0; thisDefIdx < defCount; thisDefIdx ++)
    {
        /* Processed before? */
        if (vscBV_TestBit(&tmpMask, thisDefIdx))
        {
            continue;
        }

        pThisDef = GET_DEF_BY_IDX(pDefTable, thisDefIdx);

        /* If this def is an output */
        if (pThisDef->flags.bIsOutput)
        {
            if (!_CanAddUsageToOutputDef(pDuInfo, pWorkingDefFlow, thisDefIdx, pOutputUsageInst))
            {
                continue;
            }

            /* Just get an empty usage entry to fill */
            newUsageIdx = vscBT_NewEntry(pUsageTable);
            pNewUsage = GET_USAGE_BY_IDX(pUsageTable, newUsageIdx);
            gcmASSERT(pNewUsage);

            /* Initialize usage */
            _InitializeUsage(pNewUsage, &pDuInfo->pmp.mmWrapper,
                             pOutputUsageInst,
                             (VIR_Operand*)(gctUINTPTR_T)pThisDef->defKey.regNo,
                             0x0,
                             pThisDef->halfChannelMask);

            /* Add usageIdx to hash */
            vscBT_AddToHash(pUsageTable, newUsageIdx, &pNewUsage->usageKey);

            defKey.pDefInst = VIR_ANY_DEF_INST;
            defKey.regNo = pThisDef->defKey.regNo;
            defKey.channel = VIR_CHANNEL_ANY;
            defIdx = vscBT_HashSearch(pDefTable, &defKey);

            while (VIR_INVALID_DEF_INDEX != defIdx)
            {
                pDef = GET_DEF_BY_IDX(pDefTable, defIdx);
                gcmASSERT(pDef);

                /* If this def is an output */
                if (pDef->flags.bIsOutput)
                {
                    if (_CanAddUsageToOutputDef(pDuInfo, pWorkingDefFlow, defIdx, pOutputUsageInst))
                    {
                        /* DU chain */
                        pUsageNode = (VIR_DU_CHAIN_USAGE_NODE*)vscMM_Alloc(&pDuInfo->pmp.mmWrapper,
                                                                           sizeof(VIR_DU_CHAIN_USAGE_NODE));
                        vscUSGN_Initialize(pUsageNode, newUsageIdx);
                        DU_CHAIN_ADD_USAGE(&pDef->duChain, pUsageNode);

                        /* UD-chain */
                        UD_CHAIN_ADD_DEF(&pNewUsage->udChain, defIdx);

                        /* Mark real used channel mask */
                        pNewUsage->realChannelMask |= (1 << pDef->defKey.channel);
                    }
                }

                vscBV_SetBit(&tmpMask, defIdx);

                /* Get next def with same regNo */
                defIdx = pDef->nextDefIdxOfSameRegNo;
            }
        }
    }

    vscBV_Finalize(&tmpMask);
}

static void _AddUsages(VIR_Shader* pShader,
                       VSC_BIT_VECTOR *pWorkingDefFlow,
                       VIR_Instruction* pInst,
                       VIR_DEF_USAGE_INFO* pDuInfo)
{
    gctUINT                 firstRegNo, regNoRange;
    VIR_Enable              defEnableMask;
    VIR_OperandInfo         operandInfo, operandInfo1;
    VIR_SrcOperand_Iterator srcOpndIter;
    VIR_Operand*            pOpnd;

    /* A ldarr inst to attribute array may potentially read all elements in array */
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
                _AddNewUsageToTable(pDuInfo,
                                    pWorkingDefFlow,
                                    pInst,
                                    pInst->src[VIR_Operand_Src1],
                                    operandInfo1.u1.virRegInfo.virReg,
                                    1,
                                    VIR_Operand_GetRealUsedChannels(pInst->src[VIR_Operand_Src1], pInst, gcvNULL),
                                    (gctUINT8)operandInfo1.halfChannelMask,
                                    gcvFALSE,
                                    gcvNULL);
            }

            firstRegNo = operandInfo.u1.virRegInfo.startVirReg;
            regNoRange = operandInfo.u1.virRegInfo.virRegCount;
        }

        if (VIR_OpndInfo_Is_Virtual_Reg(&operandInfo))
        {
            defEnableMask = VIR_Operand_GetRealUsedChannels(pInst->src[VIR_Operand_Src0], pInst, gcvNULL);

            _AddNewUsageToTable(pDuInfo,
                                pWorkingDefFlow,
                                pInst,
                                pInst->src[VIR_Operand_Src0],
                                firstRegNo,
                                regNoRange,
                                defEnableMask,
                                (gctUINT8)operandInfo.halfChannelMask,
                                gcvFALSE,
                                gcvNULL);
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
                firstRegNo = operandInfo.u1.virRegInfo.virReg;
                regNoRange = 1;
                defEnableMask = VIR_Operand_GetRealUsedChannels(pOpnd, pInst, gcvNULL);

                _AddNewUsageToTable(pDuInfo,
                                    pWorkingDefFlow,
                                    pInst,
                                    pOpnd,
                                    firstRegNo,
                                    regNoRange,
                                    defEnableMask,
                                    (gctUINT8)operandInfo.halfChannelMask,
                                    gcvFALSE,
                                    gcvNULL);
            }
        }
    }

    /* Emit will implicitly use all outputs */
    if (VIR_Inst_GetOpcode(pInst) == VIR_OP_EMIT ||
        VIR_Inst_GetOpcode(pInst) == VIR_OP_AQ_EMIT)
    {
        _AddOutputUsages(pDuInfo, pWorkingDefFlow, pInst);
    }
}

static void _BuildDUUDChainPerBB(VIR_BASIC_BLOCK* pBasicBlk, VIR_DEF_USAGE_INFO* pDuInfo)
{
    VIR_Shader*            pShader = pBasicBlk->pOwnerCFG->pOwnerFuncBlk->pOwnerCG->pOwnerShader;
    VSC_BLOCK_TABLE*       pDefTable = &pDuInfo->defTable;
    VSC_BIT_VECTOR         workingDefFlow;
    VIR_Instruction*       pInst = BB_GET_START_INST(pBasicBlk);
    VIR_Instruction*       pEndInst = BB_GET_END_INST(pBasicBlk);
    VIR_Enable             defEnableMask;
    gctUINT                firstRegNo, regNoRange;
    gctUINT8               halfChannelMask;
    gctBOOL                bIndexing, bCertainWrite;
    VSC_STATE_VECTOR       localHalfChannelKillFlow;

    vscBV_Initialize(&workingDefFlow, &pDuInfo->pmp.mmWrapper, pDuInfo->baseTsDFA.baseDFA.flowSize);

    /* Initialize working flow as input flow */
    vscBV_Copy(&workingDefFlow, &pBasicBlk->pTsWorkDataFlow->inFlow);

    /* 4 states we are using:
       VIR_HALF_CHANNEL_MASK_NONE (0),
       VIR_HALF_CHANNEL_MASK_LOW  (1),
       VIR_HALF_CHANNEL_MASK_HIGH (2),
       VIR_HALF_CHANNEL_MASK_FULL (3)
    */
    vscSV_Initialize(&localHalfChannelKillFlow, pDuInfo->baseTsDFA.baseDFA.pMM,
                     pDuInfo->baseTsDFA.baseDFA.flowSize, 4);

    /* Go through all instructions of basic block to create each usage and make connection between
       this usage and its def */
    while (pInst)
    {
        /* To each vir reg read, find its def, if can not be found, it is an undef'ed usage */
        _AddUsages(pShader, &workingDefFlow, pInst, pDuInfo);

        /* Update working flow */
        if (vscVIR_QueryRealWriteVirRegInfo(pShader,
                                            pInst,
                                            &defEnableMask,
                                            &halfChannelMask,
                                            &firstRegNo,
                                            &regNoRange,
                                            gcvNULL,
                                            &bIndexing))
        {
            bCertainWrite = (!bIndexing &&
                             !VIR_OPCODE_CONDITIONAL_WRITE(VIR_Inst_GetOpcode(pInst)) &&
                             !VIR_OPCODE_DestOnlyUseEnable(VIR_Inst_GetOpcode(pInst)));

            _Update_ReachDef_Local_GenKill(pDefTable,
                                           &workingDefFlow,
                                           gcvNULL,
                                           &localHalfChannelKillFlow,
                                           pInst,
                                           firstRegNo,
                                           regNoRange,
                                           defEnableMask,
                                           halfChannelMask,
                                           bCertainWrite);
        }

        /* Emit will implicitly kill all output's defs */
        if (VIR_Inst_GetOpcode(pInst) == VIR_OP_EMIT ||
            VIR_Inst_GetOpcode(pInst) == VIR_OP_AQ_EMIT)
        {
            _Update_ReachDef_Local_Kill_All_Output_Defs(pDuInfo,
                                                        pDefTable,
                                                        &workingDefFlow,
                                                        gcvNULL);
        }

        /* If current inst is the last inst of block, just bail out */
        if (pInst == pEndInst)
        {
            break;
        }

        /* Move to next inst */
        pInst = VIR_Inst_GetNext(pInst);
    }

    gcmASSERT(vscBV_Equal(&workingDefFlow, &pBasicBlk->pTsWorkDataFlow->outFlow) ||
              pBasicBlk->flowType == VIR_FLOW_TYPE_CALL);

    /* A full def kill can not be across boundary of basic-block. So for a def, either it is
       fully killed in basic-block, or it is not fully killed. */
    gcmASSERT(vscSV_All(&localHalfChannelKillFlow, VIR_HALF_CHANNEL_MASK_NONE));

    vscSV_Finalize(&localHalfChannelKillFlow);
    vscBV_Finalize(&workingDefFlow);
}

static VSC_ErrCode _BuildDUUDChain(VIR_CALL_GRAPH* pCg, VIR_DEF_USAGE_INFO* pDuInfo)
{
    VSC_ErrCode            errCode = VSC_ERR_NONE;
    gctUINT                defCount = pDuInfo->baseTsDFA.baseDFA.flowSize;
    CG_ITERATOR            funcBlkIter;
    VIR_FUNC_BLOCK*        pFuncBlk;
    CFG_ITERATOR           basicBlkIter;
    VIR_BASIC_BLOCK*       pThisBlock;
    VIR_FUNC_BLOCK*        pMainFuncBlock = CG_GET_MAIN_FUNC(pCg)->pFuncBlock;
    VIR_TS_FUNC_FLOW*      pMainFuncFlow;
    VSC_BIT_VECTOR*        pMainFlowOut;

    vscBT_Initialize(&pDuInfo->usageTable,
                     &pDuInfo->pmp.mmWrapper,
                     VSC_BLOCK_TABLE_FLAG_HASH_ENTRIES,
                     sizeof(VIR_USAGE),
                     (gctUINT)((defCount*1.5)*sizeof(VIR_USAGE)),
                     1,
                     _HFUNC_UsageInstLSB8,
                     _HKCMP_UsageKeyEqual,
                     DEF_USAGE_HASH_TABLE_SIZE);

    /* Go through whole shader func by func */
    CG_ITERATOR_INIT(&funcBlkIter, pCg);
    pFuncBlk = (VIR_FUNC_BLOCK *)CG_ITERATOR_FIRST(&funcBlkIter);
    for (; pFuncBlk != gcvNULL; pFuncBlk = (VIR_FUNC_BLOCK *)CG_ITERATOR_NEXT(&funcBlkIter))
    {
        /* Go through all basic blocks of each func */
        CFG_ITERATOR_INIT(&basicBlkIter, &pFuncBlk->cfg);
        pThisBlock = (VIR_BASIC_BLOCK *)CFG_ITERATOR_FIRST(&basicBlkIter);
        for (; pThisBlock != gcvNULL; pThisBlock = (VIR_BASIC_BLOCK *)CFG_ITERATOR_NEXT(&basicBlkIter))
        {
            /* No usage exists in entry and exit basic block */
            if (BB_GET_FLOWTYPE(pThisBlock) == VIR_FLOW_TYPE_ENTRY ||
                BB_GET_FLOWTYPE(pThisBlock) == VIR_FLOW_TYPE_EXIT)
            {
                continue;
            }

            /* For each basic block, build du/ud chain */
            _BuildDUUDChainPerBB(pThisBlock, pDuInfo);
        }
    }

    pMainFuncFlow = (VIR_TS_FUNC_FLOW*)vscSRARR_GetElement(&pDuInfo->baseTsDFA.tsFuncFlowArray,
                                                           pMainFuncBlock->dgNode.id);
    pMainFlowOut = &pMainFuncFlow->outFlow;

    /* Outputs have special implicit usages from next stage. All false outputs will be
       also corrected by setting bIsOutput as FALSE */
    _AddOutputUsages(pDuInfo, pMainFlowOut, VIR_OUTPUT_USAGE_INST);

    return errCode;
}

static void _InitializeWeb(VIR_WEB* pWeb, VIR_WEB_TYPE webType)
{
    pWeb->webType = webType;
    pWeb->numOfDef = 0;
    pWeb->firstUsageIdx = VIR_INVALID_USAGE_INDEX;
    pWeb->firstDefIdx = VIR_INVALID_DEF_INDEX;
    pWeb->channelMask = 0;
}

static void _MergeTwoWebs(VIR_DEF_USAGE_INFO* pDuInfo,
                          gctUINT dstWebIdx,
                          gctUINT srcWebIdx)
{
    VSC_BLOCK_TABLE*         pDefTable = &pDuInfo->defTable;
    VSC_BLOCK_TABLE*         pUsageTable = &pDuInfo->usageTable;
    VSC_BLOCK_TABLE*         pWebTable = &pDuInfo->webTable;
    VIR_WEB*                 pDstWeb;
    VIR_WEB*                 pSrcWeb;
    VIR_DEF*                 pDef;
    VIR_USAGE*               pUsage;
    gctUINT                  defIdx, usageIdx;

    gcmASSERT(dstWebIdx != VIR_INVALID_WEB_INDEX &&
              srcWebIdx != VIR_INVALID_WEB_INDEX);
    gcmASSERT(dstWebIdx != srcWebIdx);

    pDstWeb = GET_WEB_BY_IDX(pWebTable, dstWebIdx);
    pSrcWeb = GET_WEB_BY_IDX(pWebTable, srcWebIdx);
    gcmASSERT(pDstWeb);
    gcmASSERT(pSrcWeb);

    /* Move all defs from src web to dst web */
    while (pSrcWeb->firstDefIdx != VIR_INVALID_DEF_INDEX)
    {
        defIdx = pSrcWeb->firstDefIdx;
        pDef = GET_DEF_BY_IDX(pDefTable, defIdx);
        gcmASSERT(pDef);

        /* Remove from src */
        pSrcWeb->firstDefIdx = pDef->nextDefInWebIdx;
        pSrcWeb->numOfDef --;

        /* Add it to dst */
        pDef->webIdx = dstWebIdx;
        pDef->nextDefInWebIdx = pDstWeb->firstDefIdx;
        pDstWeb->firstDefIdx = defIdx;
        pDstWeb->numOfDef ++;
        pDstWeb->channelMask |= (1 << pDef->defKey.channel);
    }
    pSrcWeb->channelMask = 0;

    /* Move all usages from src web to dst web */
    while (pSrcWeb->firstUsageIdx != VIR_INVALID_DEF_INDEX)
    {
        usageIdx = pSrcWeb->firstUsageIdx;
        pUsage = GET_USAGE_BY_IDX(pUsageTable, usageIdx);
        gcmASSERT(pUsage);

        /* Remove from src */
        pSrcWeb->firstUsageIdx = pUsage->nextWebUsageIdx;

        /* Add it to dst */
        pUsage->webIdx = dstWebIdx;
        pUsage->nextWebUsageIdx = pDstWeb->firstUsageIdx;
        pDstWeb->firstUsageIdx = usageIdx;
    }

    /* Remove src web */
    gcmASSERT(IS_WEB_CAN_BE_REMOVED(pSrcWeb));
    vscBT_RemoveEntry(pWebTable, srcWebIdx);
}

static void _PostProcessNewWeb(VIR_DEF_USAGE_INFO* pDuInfo, gctUINT newWebIdx)
{
    PVSC_HW_CONFIG           pHwCfg = pDuInfo->baseTsDFA.baseDFA.pOwnerCG->pOwnerShader->passMnger->pCompilerParam->cfg.pHwCfg;
    VSC_BLOCK_TABLE*         pDefTable = &pDuInfo->defTable;
    VSC_BLOCK_TABLE*         pWebTable = &pDuInfo->webTable;
    VIR_WEB*                 pNewWeb = GET_WEB_BY_IDX(pWebTable, newWebIdx);
    VIR_DEF_KEY              defKey;
    VIR_DEF*                 pDef;
    VIR_DEF*                 pTempDef;
    gctUINT                  defIdx;

    gcmASSERT(pNewWeb);
    gcmASSERT(pNewWeb->firstDefIdx != VIR_INVALID_DEF_INDEX);

    pDef = GET_DEF_BY_IDX(pDefTable, pNewWeb->firstDefIdx);
    gcmASSERT(pDef->webIdx != VIR_INVALID_WEB_INDEX);

    /* 1. Do output web merge for emit (if GS supports EMIT) */
    if (pHwCfg->hwFeatureFlags.gsSupportEmit)
    {
        if (pDef->flags.bIsOutput)
        {
            defKey.pDefInst = VIR_ANY_DEF_INST;
            defKey.regNo = pDef->defKey.regNo;
            defKey.channel = VIR_CHANNEL_ANY;
            defIdx = vscBT_HashSearch(pDefTable, &defKey);

            while (VIR_INVALID_DEF_INDEX != defIdx)
            {
                pTempDef = GET_DEF_BY_IDX(pDefTable, defIdx);

                if (pTempDef->webIdx != VIR_INVALID_WEB_INDEX && pTempDef->webIdx != newWebIdx)
                {
                    if (pTempDef->flags.bIsOutput)
                    {
                        _MergeTwoWebs(pDuInfo, pTempDef->webIdx, newWebIdx);
                        break;
                    }
                }

                /* Get next def with same regNo */
                defIdx = pTempDef->nextDefIdxOfSameRegNo;
            }
        }
    }

    /* 2. Do HW dependent web merge */
    if (pDef->flags.bHwSpecialInput || pDef->defKey.pDefInst == VIR_HW_SPECIAL_DEF_INST)
    {
        defKey.pDefInst = VIR_ANY_DEF_INST;
        defKey.regNo = pDef->defKey.regNo;
        defKey.channel = VIR_CHANNEL_ANY;
        defIdx = vscBT_HashSearch(pDefTable, &defKey);

        while (VIR_INVALID_DEF_INDEX != defIdx)
        {
            pTempDef = GET_DEF_BY_IDX(pDefTable, defIdx);

            if (pTempDef->webIdx != VIR_INVALID_WEB_INDEX && pTempDef->webIdx != newWebIdx)
            {
                if (pTempDef->flags.bHwSpecialInput || pTempDef->defKey.pDefInst == VIR_HW_SPECIAL_DEF_INST)
                {
                    _MergeTwoWebs(pDuInfo, pTempDef->webIdx, newWebIdx);
                    break;
                }
            }

            /* Get next def with same regNo */
            defIdx = pTempDef->nextDefIdxOfSameRegNo;
        }
    }

    /* 3. Do half-channel web merge */
    if (pDef->defKey.pDefInst < VIR_OUTPUT_USAGE_INST &&
        VIR_Inst_GetDual16ExpandSeq(pDef->defKey.pDefInst) != NOT_ASSIGNED)
    {
        defKey.pDefInst = VIR_ANY_DEF_INST;
        defKey.regNo = pDef->defKey.regNo;
        defKey.channel = VIR_CHANNEL_ANY;
        defIdx = vscBT_HashSearch(pDefTable, &defKey);

        while (VIR_INVALID_DEF_INDEX != defIdx)
        {
            pTempDef = GET_DEF_BY_IDX(pDefTable, defIdx);

            if (pTempDef->webIdx != VIR_INVALID_WEB_INDEX && pTempDef->webIdx != newWebIdx)
            {
                if (VIR_Inst_GetDual16ExpandSeq(pTempDef->defKey.pDefInst) ==
                    VIR_Inst_GetDual16ExpandSeq(pDef->defKey.pDefInst))
                {
                    _MergeTwoWebs(pDuInfo, pTempDef->webIdx, newWebIdx);
                    break;
                }
            }

            /* Get next def with same regNo */
            defIdx = pTempDef->nextDefIdxOfSameRegNo;
        }
    }
}

static gctBOOL _BuildNewWeb(VIR_DEF_USAGE_INFO* pDuInfo,
                            VSC_BIT_VECTOR* pGlobalWorkingFlow,
                            VSC_BIT_VECTOR* pLocalWorkingFlow,
                            gctUINT* pGlobalSearchStartDefIdx,
                            gctBOOL bPartialUpdate,
                            gctUINT* pRetWebeIdx)
{
    VSC_BLOCK_TABLE*         pDefTable = &pDuInfo->defTable;
    VSC_BLOCK_TABLE*         pUsageTable = &pDuInfo->usageTable;
    VSC_BLOCK_TABLE*         pWebTable = &pDuInfo->webTable;
    VIR_Shader*              pShader = pDuInfo->baseTsDFA.baseDFA.pOwnerCG->pOwnerShader;
    VIR_DEF*                 pDef;
    VIR_DEF*                 pThisDef;
    VIR_USAGE*               pUsage;
    VIR_WEB*                 pNewWeb = gcvNULL;
    gctUINT                  defIdx, thisDefIdx;
    gctUINT                  newWebIdx = VIR_INVALID_WEB_INDEX;
    gctUINT                  i;
    VSC_DU_ITERATOR          duIter;
    VIR_DU_CHAIN_USAGE_NODE* pUsageNode;
    VIR_DEF_KEY              defKey;
    gctUINT8                 channel;
    gctBOOL                  bNewWebAdded = gcvFALSE;
    VSC_BIT_VECTOR           extendedDefFlow;
    VIR_Enable               defEnableMask;
    gctUINT                  firstRegNo, regNoRange, regNo;

    if (!vscBV_Any(pLocalWorkingFlow))
    {
        return bNewWebAdded;
    }

    /* Check whether there is existed web for current requested defs, if yes, just
       use it. We may need merge webs if original defs are located into different
       webs */
    if (bPartialUpdate)
    {
        vscBV_Initialize(&extendedDefFlow,
                         pDuInfo->baseTsDFA.baseDFA.pMM,
                         BT_GET_MAX_VALID_ID(&pDuInfo->defTable));

        /* Defs who have true usages for same instruction must be in the same web. So
           we need firstly find out these extended defs as def candidates for web */
        thisDefIdx = 0;
        while ((thisDefIdx = vscBV_FindSetBitForward(pLocalWorkingFlow, thisDefIdx)) != (gctUINT)INVALID_BIT_LOC)
        {
            gcmASSERT(VIR_INVALID_DEF_INDEX != thisDefIdx);

            pThisDef = GET_DEF_BY_IDX(pDefTable, thisDefIdx);
            gcmASSERT(pThisDef);

            if (!VIR_IS_IMPLICIT_DEF_INST(pThisDef->defKey.pDefInst))
            {
                gcmVERIFY(vscVIR_QueryRealWriteVirRegInfo(pShader,
                                                          pThisDef->defKey.pDefInst,
                                                          &defEnableMask,
                                                          gcvNULL,
                                                          &firstRegNo,
                                                          &regNoRange,
                                                          gcvNULL,
                                                          gcvNULL));
            }
            else
            {
                firstRegNo = pThisDef->defKey.regNo;
                regNoRange = 1;
            }

            for (regNo = firstRegNo; regNo < firstRegNo + regNoRange; regNo ++)
            {
                for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
                {
                    /* Skip self */
                    if (channel == pThisDef->defKey.channel &&
                        regNo == pThisDef->defKey.regNo)
                    {
                        continue;
                    }

                    defKey.pDefInst = pThisDef->defKey.pDefInst;
                    defKey.regNo = regNo;
                    defKey.channel = channel;
                    defIdx = vscBT_HashSearch(pDefTable, &defKey);
                    if (VIR_INVALID_DEF_INDEX != defIdx)
                    {
                        gcmASSERT(thisDefIdx != defIdx);

                        pDef = GET_DEF_BY_IDX(pDefTable, defIdx);
                        gcmASSERT(pDef);

#if MAKE_DEAD_DEF_IN_SEPERATED_WEB
                        if (DU_CHAIN_GET_USAGE_COUNT(&pDef->duChain) == 0)
                        {
                            gcmASSERT(pDef->webIdx != VIR_INVALID_WEB_INDEX);
                            continue;
                        }
#endif

                        vscBV_SetBit(&extendedDefFlow, defIdx);
                    }
                }
            }

            thisDefIdx ++;
        }

        /* Add extended defs to local working flow */
        vscBV_Or1(pLocalWorkingFlow, &extendedDefFlow);
        vscBV_Finalize(&extendedDefFlow);

        /* Determine which web index we will use as long as there are defs who originally was added
           into a web */
        thisDefIdx = 0;
        while ((thisDefIdx = vscBV_FindSetBitForward(pLocalWorkingFlow, thisDefIdx)) != (gctUINT)INVALID_BIT_LOC)
        {
            gcmASSERT(VIR_INVALID_DEF_INDEX != thisDefIdx);

            pThisDef = GET_DEF_BY_IDX(pDefTable, thisDefIdx);
            gcmASSERT(pThisDef);

            if (pThisDef->webIdx != VIR_INVALID_WEB_INDEX)
            {
                if (newWebIdx == VIR_INVALID_WEB_INDEX)
                {
                    newWebIdx = pThisDef->webIdx;
                }
                else if (newWebIdx != pThisDef->webIdx)
                {
                    _MergeTwoWebs(pDuInfo, newWebIdx, pThisDef->webIdx);
                }
            }

            thisDefIdx ++;
        }

        if (newWebIdx != VIR_INVALID_WEB_INDEX)
        {
            pNewWeb = GET_WEB_BY_IDX(pWebTable, newWebIdx);
            gcmASSERT(pNewWeb);
        }
    }

    if (newWebIdx == VIR_INVALID_WEB_INDEX)
    {
        /* New a web and init it */
        newWebIdx = vscBT_NewEntry(pWebTable);
        pNewWeb = GET_WEB_BY_IDX(pWebTable, newWebIdx);
        gcmASSERT(pNewWeb);
        _InitializeWeb(pNewWeb, VIR_WEB_TYPE_UNKNOWN);

        bNewWebAdded = gcvTRUE;
    }

    if (pRetWebeIdx)
    {
        *pRetWebeIdx = newWebIdx;
    }

    while ((thisDefIdx = vscBV_FindSetBitForward(pLocalWorkingFlow, 0)) != (gctUINT)INVALID_BIT_LOC)
    {
        gcmASSERT(VIR_INVALID_DEF_INDEX != thisDefIdx);

        vscBV_ClearBit(pLocalWorkingFlow, thisDefIdx);

        pThisDef = GET_DEF_BY_IDX(pDefTable, thisDefIdx);
        gcmASSERT(pThisDef);

        /* Add def into web */
        if (pThisDef->webIdx == VIR_INVALID_WEB_INDEX)
        {
            pThisDef->webIdx = newWebIdx;
            pThisDef->nextDefInWebIdx = pNewWeb->firstDefIdx;
            pNewWeb->firstDefIdx = thisDefIdx;
            pNewWeb->numOfDef ++;
            pNewWeb->channelMask |= (1 << pThisDef->defKey.channel);
        }
        else if (!bPartialUpdate)
        {
            /* Only partial update, go on */
            continue;
        }

        /* Defs who have true usages for same instruction must be in the same web */
        if (!bPartialUpdate)
        {
#if MAKE_DEAD_DEF_IN_SEPERATED_WEB
            if (DU_CHAIN_GET_USAGE_COUNT(&pThisDef->duChain) != 0)
#endif
            {
                if (!VIR_IS_IMPLICIT_DEF_INST(pThisDef->defKey.pDefInst))
                {
                    gcmVERIFY(vscVIR_QueryRealWriteVirRegInfo(pShader,
                                                              pThisDef->defKey.pDefInst,
                                                              &defEnableMask,
                                                              gcvNULL,
                                                              &firstRegNo,
                                                              &regNoRange,
                                                              gcvNULL,
                                                              gcvNULL));
                }
                else
                {
                    firstRegNo = pThisDef->defKey.regNo;
                    regNoRange = 1;
                }

                for (regNo = firstRegNo; regNo < firstRegNo + regNoRange; regNo ++)
                {
                    for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
                    {
                        /* Skip self */
                        if (channel == pThisDef->defKey.channel &&
                            regNo == pThisDef->defKey.regNo)
                        {
                            continue;
                        }

                        defKey.pDefInst = pThisDef->defKey.pDefInst;
                        defKey.regNo = regNo;
                        defKey.channel = channel;
                        defIdx = vscBT_HashSearch(pDefTable, &defKey);
                        if (VIR_INVALID_DEF_INDEX != defIdx)
                        {
                            gcmASSERT(thisDefIdx != defIdx);

                            pDef = GET_DEF_BY_IDX(pDefTable, defIdx);
                            gcmASSERT(pDef);

                            if (
#if MAKE_DEAD_DEF_IN_SEPERATED_WEB
                                (DU_CHAIN_GET_USAGE_COUNT(&pDef->duChain) == 0) ||
#endif
                                (pDef->webIdx != VIR_INVALID_WEB_INDEX))
                            {
                                continue;
                            }

                            vscBV_SetBit(pLocalWorkingFlow, defIdx);
                        }
                    }
                }
            }
        }

        /* For each usage in this du chain */
        VSC_DU_ITERATOR_INIT(&duIter, &pThisDef->duChain);
        pUsageNode = VSC_DU_ITERATOR_FIRST(&duIter);
        for (; pUsageNode != gcvNULL; pUsageNode = VSC_DU_ITERATOR_NEXT(&duIter))
        {
            pUsage = GET_USAGE_BY_IDX(pUsageTable, pUsageNode->usageIdx);
            gcmASSERT(pUsage);

            if (pUsage->webIdx != VIR_INVALID_WEB_INDEX)
            {
                continue;
            }

            /* Add usage into web */
            pUsage->webIdx = newWebIdx;
            pUsage->nextWebUsageIdx = pNewWeb->firstUsageIdx;
            pNewWeb->firstUsageIdx = pUsageNode->usageIdx;

            if (!bPartialUpdate)
            {
                /* Check ud chain to add all un-webized def to local working list */
                for (i = 0; i < UD_CHAIN_GET_DEF_COUNT(&pUsage->udChain); i ++)
                {
                    defIdx = UD_CHAIN_GET_DEF(&pUsage->udChain, i);
                    gcmASSERT(VIR_INVALID_DEF_INDEX != defIdx);

                    pDef = GET_DEF_BY_IDX(pDefTable, defIdx);
                    gcmASSERT(pDef);

                    if (pDef->webIdx != VIR_INVALID_WEB_INDEX)
                    {
                        continue;
                    }

                    vscBV_SetBit(pLocalWorkingFlow, defIdx);
                }
            }
        }

        /* Clear this def for global working list */
        if (pGlobalWorkingFlow)
        {
            vscBV_ClearBit(pGlobalWorkingFlow, thisDefIdx);
        }

        /* Set proper next global search index */
        if (pGlobalSearchStartDefIdx)
        {
            if (*pGlobalSearchStartDefIdx == thisDefIdx)
            {
                (*pGlobalSearchStartDefIdx) ++;
            }
        }
    };

    /* For a new created web, we might need do post-process, such as webs merge */
    if (bNewWebAdded)
    {
        _PostProcessNewWeb(pDuInfo, newWebIdx);
    }

    return bNewWebAdded;
}

static VSC_ErrCode _BuildWebs(VIR_CALL_GRAPH* pCg, VIR_DEF_USAGE_INFO* pDuInfo)
{
    VSC_ErrCode            errCode = VSC_ERR_NONE;
    gctUINT                defCount = BT_GET_MAX_VALID_ID(&pDuInfo->defTable);
    VSC_BLOCK_TABLE*       pDefTable = &pDuInfo->defTable;
    VSC_BIT_VECTOR         globalWorkingFlow, localWorkingFlow;
    gctUINT                defIdx, globalsearchStartDefIdx = 0;
    VIR_DEF*               pDef;
    VIR_DEF_KEY            defKey;

    vscBT_Initialize(&pDuInfo->webTable,
                     &pDuInfo->pmp.mmWrapper,
                     0,
                     sizeof(VIR_WEB),
                     (defCount)*sizeof(VIR_WEB),
                     1,
                     gcvNULL,
                     gcvNULL,
                     0);

    /* Mark web has been built */
    pDuInfo->bWebTableBuilt = gcvTRUE;

    /* Let web table initialized before we make this check */
    if (defCount == 0)
    {
        return errCode;
    }

    vscBV_Initialize(&globalWorkingFlow, &pDuInfo->pmp.mmWrapper, defCount);
    vscBV_Initialize(&localWorkingFlow, &pDuInfo->pmp.mmWrapper, defCount);

    /* Mark all defs not processed */
    vscBV_SetAll(&globalWorkingFlow);

    /* As long as there still are defs not processed, go on ... */
    while ((defIdx = vscBV_FindSetBitForward(&globalWorkingFlow, globalsearchStartDefIdx ++)) != (gctUINT)INVALID_BIT_LOC)
    {
        gcmASSERT(VIR_INVALID_DEF_INDEX != defIdx);
        pDef = GET_DEF_BY_IDX(pDefTable, defIdx);
        gcmASSERT(pDef);

        if (!IS_VALID_DEF(pDef))
        {
            vscBV_ClearBit(&globalWorkingFlow, defIdx);
            continue;
        }

        defKey.pDefInst = VIR_ANY_DEF_INST;
        defKey.regNo = pDef->defKey.regNo;
        defKey.channel = VIR_CHANNEL_ANY;
        defIdx = vscBT_HashSearch(pDefTable, &defKey);

        /* For all defs with same regNo, try to build webs */
        while (VIR_INVALID_DEF_INDEX != defIdx)
        {
            pDef = GET_DEF_BY_IDX(pDefTable, defIdx);
            gcmASSERT(pDef);

            /* If this def is still in global working list, do it */
            if (vscBV_TestBit(&globalWorkingFlow, defIdx))
            {
                vscBV_ClearAll(&localWorkingFlow);
                vscBV_SetBit(&localWorkingFlow, defIdx);

                _BuildNewWeb(pDuInfo,
                             &globalWorkingFlow,
                             &localWorkingFlow,
                             &globalsearchStartDefIdx,
                             gcvFALSE,
                             gcvNULL);
            }

            /* Get next def with same regNo */
            defIdx = pDef->nextDefIdxOfSameRegNo;
        }
    }

    vscBV_Finalize(&globalWorkingFlow);
    vscBV_Finalize(&localWorkingFlow);

    return errCode;
}

VSC_ErrCode _DestoryWebs(VIR_DEF_USAGE_INFO* pDuInfo, gctBOOL bOnlyFinalizeWebTable)
{
    VSC_ErrCode            errCode = VSC_ERR_NONE;
    gctUINT                defCount, usageCount, defIdx, usageIdx;
    VSC_BLOCK_TABLE*       pDefTable = &pDuInfo->defTable;
    VSC_BLOCK_TABLE*       pUsageTable = &pDuInfo->usageTable;
    VIR_DEF*               pDef;
    VIR_USAGE*             pUsage;

    if (pDuInfo->bWebTableBuilt)
    {
        /* We need remove any web info for each def and usage if it is requested */
        if (!bOnlyFinalizeWebTable)
        {
            /* For def */
            defCount = BT_GET_MAX_VALID_ID(&pDuInfo->defTable);
            for (defIdx = 0; defIdx < defCount; defIdx ++)
            {
                pDef = GET_DEF_BY_IDX(pDefTable, defIdx);
                if (IS_VALID_DEF(pDef))
                {
                    pDef->nextDefInWebIdx = VIR_INVALID_DEF_INDEX;
                    pDef->webIdx = VIR_INVALID_WEB_INDEX;
                }
            }

            /* For usage */
            usageCount = BT_GET_MAX_VALID_ID(&pDuInfo->usageTable);
            for (usageIdx = 0; usageIdx < usageCount; usageIdx ++)
            {
                pUsage = GET_USAGE_BY_IDX(pUsageTable, usageIdx);
                if (IS_VALID_USAGE(pUsage))
                {
                    pUsage->webIdx = VIR_INVALID_WEB_INDEX;
                    pUsage->nextWebUsageIdx = VIR_INVALID_USAGE_INDEX;
                }
            }
        }

        vscBT_Finalize(&pDuInfo->webTable);

        /* Mark web has been destoryed */
        pDuInfo->bWebTableBuilt = gcvFALSE;
    }

    return errCode;
}

VSC_ErrCode vscVIR_BuildDefUsageInfo(VIR_CALL_GRAPH* pCg,
                                     VIR_DEF_USAGE_INFO* pDuInfo,
                                     gctBOOL bBuildWeb)
{
    VSC_ErrCode            errCode = VSC_ERR_NONE;

    /* Initialize web not built */
    pDuInfo->bWebTableBuilt = gcvFALSE;

    /* Initialize our pmp, you can also use other MM if you want */
    vscPMP_Intialize(&pDuInfo->pmp,
                     gcvNULL,
                     (40*sizeof(VIR_DEF)+80*sizeof(VIR_USAGE) + 20*sizeof(VIR_WEB))*5,
                     sizeof(void*),
                     gcvTRUE);

    /* Build def table */
    errCode = _BuildDefTable(pCg, pDuInfo);
    CHECK_ERROR(errCode, "Build def table");

    /* Do global reach-def analysis */
    errCode = _DoReachDefAnalysis(pCg, pDuInfo);
    CHECK_ERROR(errCode, "Do reach-def analysis");

    /* Build DU chain, as well as UD chain */
    errCode = _BuildDUUDChain(pCg, pDuInfo);
    CHECK_ERROR(errCode, "Build du/ud chain");

    /* Build Web */
    if (bBuildWeb)
    {
        errCode = _BuildWebs(pCg, pDuInfo);
        CHECK_ERROR(errCode, "Build web");
    }

    /* Mark we have successfully built the DU info */
    vscVIR_SetDFAValidity(&pDuInfo->baseTsDFA.baseDFA, gcvTRUE);

    return errCode;
}

VSC_ErrCode vscVIR_BuildWebs(VIR_CALL_GRAPH* pCg,
                             VIR_DEF_USAGE_INFO* pDuInfo,
                             gctBOOL bForceToBuild)
{
    gcmASSERT(vscVIR_GetDFAValidity(&pDuInfo->baseTsDFA.baseDFA));

    if (pDuInfo->bWebTableBuilt)
    {
        if (!bForceToBuild)
        {
            return VSC_ERR_NONE;
        }

        /* If web is forced to built, we need destory previous one firstly */
        _DestoryWebs(pDuInfo, gcvFALSE);
    }

    return _BuildWebs(pCg, pDuInfo);
}

VSC_ErrCode vscVIR_DestroyDefUsageInfo(VIR_DEF_USAGE_INFO* pDuInfo)
{
    VSC_ErrCode            errCode = VSC_ERR_NONE;

    if (vscVIR_GetDFAValidity(&pDuInfo->baseTsDFA.baseDFA))
    {
        vscBT_Finalize(&pDuInfo->defTable);
        vscBT_Finalize(&pDuInfo->usageTable);

        _DestoryWebs(pDuInfo, gcvTRUE);

        vscVIR_FinalizeBaseTsDFA(&pDuInfo->baseTsDFA);
        vscPMP_Finalize(&pDuInfo->pmp);

        /* Mark DU info has been invalid */
        vscVIR_SetDFAValidity(&pDuInfo->baseTsDFA.baseDFA, gcvFALSE);
    }

    return errCode;
}

VSC_ErrCode vscVIR_DestoryWebs(VIR_DEF_USAGE_INFO* pDuInfo)
{
    return _DestoryWebs(pDuInfo, gcvFALSE);
}

void vscVIR_AddNewDef(VIR_DEF_USAGE_INFO* pDuInfo,
                      VIR_Instruction* pDefInst,
                      gctUINT firstDefRegNo,
                      gctUINT defRegNoRange,
                      VIR_Enable defEnableMask,
                      gctUINT8 halfChannelMask,
                      gctBOOL bIsInputDef,
                      gctBOOL bIsOutputDef,
                      gctUINT* pRetDefIdxArray)
{
    VSC_BLOCK_TABLE*       pDefTable = &pDuInfo->defTable;
    VIR_DEF_FLAGS          defFlags = {0};
    gctUINT                i;

    defFlags.bIsInput = bIsInputDef;
    defFlags.bIsOutput = bIsOutputDef;

    if (pRetDefIdxArray)
    {
        for (i = 0; i < VIR_CHANNEL_NUM*defRegNoRange; i ++)
        {
            pRetDefIdxArray[i] = VIR_INVALID_DEF_INDEX;
        }
    }

    if (_AddNewDefToTable(pDuInfo,
                          pDefTable,
                          firstDefRegNo,
                          defRegNoRange,
                          defEnableMask,
                          halfChannelMask,
                          pDefInst,
                          defFlags,
                          gcvTRUE,
                          gcvTRUE,
                          pRetDefIdxArray))
    {
        /* Mark DU info partially be updated */
        pDuInfo->baseTsDFA.baseDFA.cmnDfaFlags.bFlowUpdated = gcvTRUE;
    }
}

void vscVIR_DeleteDef(VIR_DEF_USAGE_INFO* pDuInfo,
                      VIR_Instruction* pDefInst,
                      gctUINT firstDefRegNo,
                      gctUINT defRegNoRange,
                      VIR_Enable defEnableMask,
                      gctUINT8 halfChannelMask,
                      gctUINT pRetDefIdxArray[])
{
    VSC_BLOCK_TABLE*       pDefTable = &pDuInfo->defTable;
    gctUINT                i;

    if (pRetDefIdxArray)
    {
        for (i = 0; i < VIR_CHANNEL_NUM*defRegNoRange; i ++)
        {
            pRetDefIdxArray[i] = VIR_INVALID_DEF_INDEX;
        }
    }

    if (_DeleteDefFromTable(pDuInfo,
                            pDefTable,
                            firstDefRegNo,
                            defRegNoRange,
                            defEnableMask,
                            halfChannelMask,
                            pDefInst,
                            pRetDefIdxArray))
    {
        /* Mark DU info partially be updated */
        pDuInfo->baseTsDFA.baseDFA.cmnDfaFlags.bFlowUpdated = gcvTRUE;
    }
}

void vscVIR_AddNewUsageToDef(VIR_DEF_USAGE_INFO* pDuInfo,
                             VIR_Instruction* pDefInst,
                             VIR_Instruction* pUsageInst,
                             VIR_Operand* pOperand,
                             gctUINT firstUsageRegNo,
                             gctUINT usageRegNoRange,
                             VIR_Enable defEnableMask,
                             gctUINT8 halfChannelMask,
                             gctUINT* pRetUsageIdx)
{
    VSC_BLOCK_TABLE*         pDefTable = &pDuInfo->defTable;
    VSC_BIT_VECTOR           tmpWorkingDefFlow;
    gctUINT                  regNo, defIdx;
    gctUINT8                 channel;
    VIR_DEF_KEY              defKey;
    VIR_OperandInfo          operandInfo;

    if (VIR_IS_OUTPUT_USAGE_INST(pUsageInst))
    {
        gcmASSERT((gctUINT)(gctUINTPTR_T)pOperand == firstUsageRegNo);
    }
    else
    {
        gcmASSERT(VIR_Operand_IsOwnerInst(pOperand, pUsageInst));

        VIR_Operand_GetOperandInfo(pUsageInst,
            pOperand,
            &operandInfo);

        /* Only vir reg operand can be added as usage */
        if (!VIR_OpndInfo_Is_Virtual_Reg(&operandInfo))
        {
            return;
        }
    }

    if (pRetUsageIdx)
    {
        *pRetUsageIdx = VIR_INVALID_DEF_INDEX;
    }

    vscBV_Initialize(&tmpWorkingDefFlow,
                     pDuInfo->baseTsDFA.baseDFA.pMM,
                     BT_GET_MAX_VALID_ID(&pDuInfo->defTable));

    /* Check which target defs are for this new usage */
    for (regNo = firstUsageRegNo; regNo < firstUsageRegNo + usageRegNoRange; regNo ++)
    {
        /* For any channel that is used, try to find its def */
        for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
        {
            if (!VSC_UTILS_TST_BIT(defEnableMask, channel))
            {
                continue;
            }

            defKey.pDefInst = pDefInst;
            defKey.regNo = regNo;
            defKey.channel = channel;
            defIdx = vscBT_HashSearch(pDefTable, &defKey);
            if (VIR_INVALID_DEF_INDEX != defIdx)
            {
                vscBV_SetBit(&tmpWorkingDefFlow, defIdx);
            }
        }
    }

    /* Add new usage based on target defs */
    _AddNewUsageToTable(pDuInfo,
                        &tmpWorkingDefFlow,
                        pUsageInst,
                        pOperand,
                        firstUsageRegNo,
                        usageRegNoRange,
                        defEnableMask,
                        halfChannelMask,
                        gcvTRUE,
                        pRetUsageIdx);

    /* Then add new usage to web that defs have already holded */
    if (pDuInfo->bWebTableBuilt)
    {
        _BuildNewWeb(pDuInfo,
                     gcvNULL,
                     &tmpWorkingDefFlow,
                     gcvNULL,
                     gcvTRUE,
                     gcvNULL);
    }

    vscBV_Finalize(&tmpWorkingDefFlow);
}

void vscVIR_DeleteUsage(VIR_DEF_USAGE_INFO* pDuInfo,
                        VIR_Instruction* pDefInst,
                        VIR_Instruction* pUsageInst,
                        VIR_Operand* pOperand,
                        gctUINT firstUsageRegNo,
                        gctUINT usageRegNoRange,
                        VIR_Enable defEnableMask,
                        gctUINT8 halfChannelMask,
                        gctUINT* pRetUsageIdx)
{
    if (VIR_IS_OUTPUT_USAGE_INST(pUsageInst))
    {
        gcmASSERT((gctUINT)(gctUINTPTR_T)pOperand == firstUsageRegNo);
    }
    else
    {
        gcmASSERT(VIR_Operand_IsOwnerInst(pOperand, pUsageInst));
    }

    if (pRetUsageIdx)
    {
        *pRetUsageIdx = VIR_INVALID_DEF_INDEX;
    }

    _DeleteUsageFromTable(pDuInfo,
                          pDefInst,
                          pUsageInst,
                          pOperand,
                          firstUsageRegNo,
                          usageRegNoRange,
                          defEnableMask,
                          halfChannelMask,
                          pRetUsageIdx);
}

void vscVIR_MergeTwoWebs(VIR_DEF_USAGE_INFO* pDuInfo,
                         gctUINT dstWebIdx,
                         gctUINT srcWebIdx)
{
    if (!pDuInfo->bWebTableBuilt)
    {
        return;
    }

    _MergeTwoWebs(pDuInfo, dstWebIdx, srcWebIdx);
}

VIR_DEF* vscVIR_GetDef(VIR_DEF_USAGE_INFO*      pDuInfo,
                       VIR_Instruction*         pDefInst,
                       gctUINT                  defRegNo,
                       gctUINT8                 defChannel,
                       VIR_WEB**                ppWeb)
{
    VIR_DEF_KEY            defKey;
    gctUINT                defIdx;
    VIR_DEF*               pDef = gcvNULL;

    if (ppWeb)
    {
        *ppWeb = gcvNULL;
    }

    defKey.pDefInst = pDefInst;
    defKey.regNo = defRegNo;
    defKey.channel = defChannel;

    defIdx = vscBT_HashSearch(&pDuInfo->defTable, &defKey);
    if (VIR_INVALID_DEF_INDEX != defIdx)
    {
        pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, defIdx);
        gcmASSERT(pDef);

        if (ppWeb && pDuInfo->bWebTableBuilt)
        {
            *ppWeb = GET_WEB_BY_IDX(&pDuInfo->webTable, pDef->webIdx);
        }
    }

    return pDef;
}

VIR_USAGE* vscVIR_GetUsage(VIR_DEF_USAGE_INFO*    pDuInfo,
                           VIR_Instruction*       pUsageInst,
                           VIR_Operand*           pUsageOperand,
                           VIR_WEB**              ppWeb)
{
    VIR_USAGE_KEY            usageKey;
    gctUINT                  usageIdx;
    VIR_USAGE*               pUsage = gcvNULL;

    if (!VIR_IS_OUTPUT_USAGE_INST(pUsageInst))
    {
        gcmASSERT(VIR_Operand_IsOwnerInst(pUsageOperand, pUsageInst));
    }

    if (ppWeb)
    {
        *ppWeb = gcvNULL;
    }

    usageKey.pUsageInst = pUsageInst;
    usageKey.pOperand = pUsageOperand;

    usageIdx = vscBT_HashSearch(&pDuInfo->usageTable, &usageKey);
    if (VIR_INVALID_USAGE_INDEX != usageIdx)
    {
        pUsage = GET_USAGE_BY_IDX(&pDuInfo->usageTable, usageIdx);
        gcmASSERT(pUsage);

        if (ppWeb && pDuInfo->bWebTableBuilt)
        {
            *ppWeb = GET_WEB_BY_IDX(&pDuInfo->webTable, pUsage->webIdx);
        }
    }

    return pUsage;
}

void vscVIR_InitGeneralDuIterator(VIR_GENERAL_DU_ITERATOR* pIter,
                                  VIR_DEF_USAGE_INFO*      pDuInfo,
                                  VIR_Instruction*         pDefInst,
                                  gctUINT                  defRegNo,
                                  gctUINT8                 defChannel,
                                  gctBOOL                  bSameBBOnly)
{
    gctUINT      defIdx;
    VIR_DEF*     pDef;

    gcmASSERT(pDefInst != VIR_ANY_DEF_INST);

    pIter->bSameBBOnly = bSameBBOnly;

    pIter->defKey.pDefInst = pDefInst;
    pIter->defKey.regNo = defRegNo;
    pIter->defKey.channel = defChannel;
    pIter->pDuInfo = pDuInfo;

    defIdx = vscBT_HashSearch(&pDuInfo->defTable, &pIter->defKey);
    gcmASSERT(VIR_INVALID_DEF_INDEX != defIdx);

    pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, defIdx);
    gcmASSERT(pDef);

    VSC_DU_ITERATOR_INIT(&pIter->duIter, &pDef->duChain);
}

VIR_USAGE* vscVIR_GeneralDuIterator_First(VIR_GENERAL_DU_ITERATOR* pIter)
{
    VIR_DU_CHAIN_USAGE_NODE*    pUsageNode;
    VIR_USAGE*                  pUsage;

    pUsageNode = VSC_DU_ITERATOR_FIRST(&pIter->duIter);

    while (pUsageNode)
    {
        pUsage = GET_USAGE_BY_IDX(&pIter->pDuInfo->usageTable, pUsageNode->usageIdx);
        gcmASSERT(pUsage);

        if (!pIter->bSameBBOnly ||
            ARE_INSTS_IN_SAME_BASIC_BLOCK(pUsage->usageKey.pUsageInst, pIter->defKey.pDefInst))
        {
            return pUsage;
        }

        pUsageNode = VSC_DU_ITERATOR_NEXT(&pIter->duIter);
    }

    return gcvNULL;
}

VIR_USAGE* vscVIR_GeneralDuIterator_Next(VIR_GENERAL_DU_ITERATOR* pIter)
{
    VIR_DU_CHAIN_USAGE_NODE*    pUsageNode;
    VIR_USAGE*                  pUsage;

    pUsageNode = VSC_DU_ITERATOR_NEXT(&pIter->duIter);

    while (pUsageNode)
    {
        pUsage = GET_USAGE_BY_IDX(&pIter->pDuInfo->usageTable, pUsageNode->usageIdx);
        gcmASSERT(pUsage);

        if (!pIter->bSameBBOnly ||
            ARE_INSTS_IN_SAME_BASIC_BLOCK(pUsage->usageKey.pUsageInst, pIter->defKey.pDefInst))
        {
            return pUsage;
        }

        pUsageNode = VSC_DU_ITERATOR_NEXT(&pIter->duIter);
    }

    return gcvNULL;
}

void vscVIR_InitGeneralUdIterator(VIR_GENERAL_UD_ITERATOR*  pIter,
                                  VIR_DEF_USAGE_INFO*       pDuInfo,
                                  VIR_Instruction*          pUsageInst,
                                  VIR_Operand*              pUsageOperand,
                                  gctBOOL                   bSameBBOnly)
{
    gctUINT          usageIdx;
    VIR_USAGE*       pUsage;
    VIR_OperandInfo  operandInfo;

    gcmASSERT(pUsageOperand);

    pIter->bSameBBOnly = bSameBBOnly;
    pIter->usageKey.pUsageInst = pUsageInst;
    pIter->usageKey.pOperand = pUsageOperand;
    pIter->pDuInfo = pDuInfo;
    pIter->curIdx = 0;

    VIR_Operand_GetOperandInfo(pUsageInst,
                               pUsageOperand,
                               &operandInfo);

    if (VIR_OpndInfo_Is_Virtual_Reg(&operandInfo))
    {
        usageIdx = vscBT_HashSearch(&pDuInfo->usageTable, &pIter->usageKey);

        if (VIR_INVALID_USAGE_INDEX != usageIdx)
        {
            /* Normal path, that's good */
            pUsage = GET_USAGE_BY_IDX(&pDuInfo->usageTable, usageIdx);
            gcmASSERT(pUsage);

            pIter->pUdChain = &pUsage->udChain;
        }
        else
        {
            if (VIR_OUTPUT_USAGE_INST != pUsageInst &&
                VIR_Operand_IsOwnerInst(pUsageOperand, pUsageInst))
            {
                /* DU info has bug, please check it */
                gcmASSERT(gcvFALSE);
            }
            else
            {
                /* Runs into here because user try to get UD for the pUsageOperand
                   which does not belong to pUsageInst */
                pIter->pUdChain = gcvNULL;
            }
        }
    }
    else
    {
        /* Runs into here generally because user passes into illegal operand that
           uses non temp register, such as imm/const/sampler/texture/... */
        pIter->pUdChain = gcvNULL;
    }
}

VIR_DEF* vscVIR_GeneralUdIterator_First(VIR_GENERAL_UD_ITERATOR* pIter)
{
    gctUINT                   defIdx;
    VIR_DEF*                  pDef;

    if (!pIter->pUdChain)
    {
        return gcvNULL;
    }

    defIdx = UD_CHAIN_GET_DEF(pIter->pUdChain, pIter->curIdx);
    pIter->curIdx ++;
    pDef = (VIR_INVALID_DEF_INDEX == defIdx) ? gcvNULL : GET_DEF_BY_IDX(&pIter->pDuInfo->defTable, defIdx);

    while (pDef)
    {
        if (!pIter->bSameBBOnly ||
             ARE_INSTS_IN_SAME_BASIC_BLOCK(pDef->defKey.pDefInst, pIter->usageKey.pUsageInst))
        {
            return pDef;
        }

        defIdx = UD_CHAIN_GET_DEF(pIter->pUdChain, pIter->curIdx);
        pIter->curIdx ++;
        pDef = (VIR_INVALID_DEF_INDEX == defIdx) ? gcvNULL : GET_DEF_BY_IDX(&pIter->pDuInfo->defTable, defIdx);
    }

    return gcvNULL;
}

VIR_DEF* vscVIR_GeneralUdIterator_Next(VIR_GENERAL_UD_ITERATOR* pIter)
{
    return vscVIR_GeneralUdIterator_First(pIter);
}

VIR_DEF* vscVIR_GetNextHomonymyDef(VIR_DEF_USAGE_INFO*      pDuInfo,
                                   VIR_Instruction*         pDefInst,
                                   gctUINT                  defRegNo,
                                   gctUINT8                 defChannel,
                                   gctBOOL                  bSameBBOnly)
{
    VIR_DEF_KEY            defKey;
    gctUINT                defIdx;
    VIR_DEF*               pDef;
    VIR_DEF*               pPrevDef = gcvNULL;

    gcmASSERT(pDefInst != VIR_ANY_DEF_INST);

    defKey.pDefInst = VIR_ANY_DEF_INST;
    defKey.regNo = defRegNo;
    defKey.channel = VIR_CHANNEL_ANY;
    defIdx = vscBT_HashSearch(&pDuInfo->defTable, &defKey);

    /* Note that order is reversed */
    while (VIR_INVALID_DEF_INDEX != defIdx)
    {
        pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, defIdx);
        gcmASSERT(pDef->defKey.regNo == defRegNo);

        if (defChannel == pDef->defKey.channel &&
            (!bSameBBOnly ||
             ARE_INSTS_IN_SAME_BASIC_BLOCK(pDef->defKey.pDefInst, pDefInst)))
        {
            if (pDefInst == pDef->defKey.pDefInst)
            {
                return pPrevDef;
            }
            else
            {
                pPrevDef = pDef;
            }
        }

        /* Get next def with same regNo */
        defIdx = pDef->nextDefIdxOfSameRegNo;
    }

    return gcvNULL;
}

VIR_DEF* vscVIR_GetPrevHomonymyDef(VIR_DEF_USAGE_INFO*      pDuInfo,
                                   VIR_Instruction*         pDefInst,
                                   gctUINT                  defRegNo,
                                   gctUINT8                 defChannel,
                                   gctBOOL                  bSameBBOnly)
{
    VIR_DEF_KEY            defKey;
    gctUINT                defIdx;
    VIR_DEF*               pDef;

    gcmASSERT(pDefInst != VIR_ANY_DEF_INST);

    defKey.pDefInst = pDefInst;
    defKey.regNo = defRegNo;
    defKey.channel = defChannel;
    defIdx = vscBT_HashSearch(&pDuInfo->defTable, &defKey);

    /* Note that order is reversed */
    while (VIR_INVALID_DEF_INDEX != defIdx)
    {
        pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, defIdx);
        gcmASSERT(pDef->defKey.regNo == defRegNo);

        if (pDefInst != pDef->defKey.pDefInst &&
            defChannel == pDef->defKey.channel &&
            (!bSameBBOnly ||
             ARE_INSTS_IN_SAME_BASIC_BLOCK(pDef->defKey.pDefInst, pDefInst)))
        {
            return pDef;
        }

        /* Get next def with same regNo */
        defIdx = pDef->nextDefIdxOfSameRegNo;
    }

    return gcvNULL;
}

void vscVIR_InitHomonymyDefIterator(VIR_HOMONYMY_DEF_ITERATOR* pIter,
                                    VIR_DEF_USAGE_INFO*        pDuInfo,
                                    VIR_Instruction*           pDefInst,
                                    gctUINT                    defRegNo,
                                    gctUINT8                   defChannel,
                                    gctBOOL                    bSameBBOnly,
                                    gctBOOL                    bBackward)
{
    gcmASSERT(pDefInst != VIR_ANY_DEF_INST);

    pIter->bSameBBOnly = bSameBBOnly;
    pIter->bBackward = bBackward;

    pIter->defKey.pDefInst = pDefInst;
    pIter->defKey.regNo = defRegNo;
    pIter->defKey.channel = defChannel;
    pIter->pDuInfo = pDuInfo;
}

VIR_DEF* vscVIR_HomonymyDefIterator_First(VIR_HOMONYMY_DEF_ITERATOR* pIter)
{
    VIR_DEF* pDef;

    if (pIter->bBackward)
    {
        pDef = vscVIR_GetPrevHomonymyDef(pIter->pDuInfo,
                                         pIter->defKey.pDefInst,
                                         pIter->defKey.regNo,
                                         pIter->defKey.channel,
                                         pIter->bSameBBOnly);
    }
    else
    {
        pDef = vscVIR_GetNextHomonymyDef(pIter->pDuInfo,
                                         pIter->defKey.pDefInst,
                                         pIter->defKey.regNo,
                                         pIter->defKey.channel,
                                         pIter->bSameBBOnly);
    }

    if (pDef != gcvNULL)
    {
        memcpy(&pIter->defKey.pDefInst, &pDef->defKey, sizeof(VIR_DEF_KEY));
    }

    return pDef;
}

VIR_DEF* vscVIR_HomonymyDefIterator_Next(VIR_HOMONYMY_DEF_ITERATOR* pIter)
{
    return vscVIR_HomonymyDefIterator_First(pIter);
}

gctBOOL vscVIR_IsUniqueUsageInstOfDefInst(VIR_DEF_USAGE_INFO*     pDuInfo,
                                          VIR_Instruction*        pDefInst,
                                          VIR_Instruction*        pExpectedUniqueUsageInst,
                                          VIR_Instruction**       ppFirstOtherUsageInst)
{
    VIR_Enable              defEnableMask;
    gctUINT                 regNo, firstRegNo, regNoRange;
    gctUINT8                channel;
    VIR_GENERAL_DU_ITERATOR duIter;
    VIR_USAGE*              pUsage;

    /* We only consider normal instructions */
    gcmASSERT(pDefInst < VIR_OUTPUT_USAGE_INST);

    if (vscVIR_QueryRealWriteVirRegInfo(pDuInfo->baseTsDFA.baseDFA.pOwnerCG->pOwnerShader,
                                        pDefInst,
                                        &defEnableMask,
                                        gcvNULL,
                                        &firstRegNo,
                                        &regNoRange,
                                        gcvNULL,
                                        gcvNULL))
    {
        for (regNo = firstRegNo; regNo < firstRegNo + regNoRange; regNo ++)
        {
            for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
            {
                if (!VSC_UTILS_TST_BIT(defEnableMask, channel))
                {
                    continue;
                }

                vscVIR_InitGeneralDuIterator(&duIter,
                                             pDuInfo,
                                             pDefInst,
                                             regNo,
                                             channel,
                                             gcvFALSE);

                for (pUsage = vscVIR_GeneralDuIterator_First(&duIter); pUsage != gcvNULL;
                     pUsage = vscVIR_GeneralDuIterator_Next(&duIter))
                {
                    if (pUsage->usageKey.pUsageInst != pExpectedUniqueUsageInst)
                    {
                        if (ppFirstOtherUsageInst)
                        {
                            *ppFirstOtherUsageInst = pUsage->usageKey.pUsageInst;
                        }

                        return gcvFALSE;
                    }
                }
            }
        }

        return gcvTRUE;
    }

    return gcvFALSE;
}

gctBOOL vscVIR_IsUniqueUsageInstOfDefsInItsUDChain(VIR_DEF_USAGE_INFO* pDuInfo,
                                                   VIR_Instruction*    pUsageInst,
                                                   VIR_Operand*        pUsageOperand,
                                                   VIR_Instruction**   ppFirstMultiUsageDefInst,
                                                   VIR_Instruction**   ppFirstOtherUsageInst)
{
    VIR_GENERAL_UD_ITERATOR udIter;
    VIR_DEF*                pDef;
    gctBOOL                 bHasDef = gcvFALSE;

    /* We only consider normal instructions */
    gcmASSERT(pUsageInst < VIR_OUTPUT_USAGE_INST);

    vscVIR_InitGeneralUdIterator(&udIter, pDuInfo, pUsageInst, pUsageOperand, gcvFALSE);
    for (pDef = vscVIR_GeneralUdIterator_First(&udIter); pDef != gcvNULL;
         pDef = vscVIR_GeneralUdIterator_Next(&udIter))
    {
        if (VIR_IS_IMPLICIT_DEF_INST(pDef->defKey.pDefInst))
        {
            continue;
        }

        if (!vscVIR_IsUniqueUsageInstOfDefInst(pDuInfo, pDef->defKey.pDefInst, pUsageInst, ppFirstOtherUsageInst))
        {
            if (ppFirstMultiUsageDefInst)
            {
                *ppFirstMultiUsageDefInst = pDef->defKey.pDefInst;
            }

            return gcvFALSE;
        }

        bHasDef = gcvTRUE;
    }

    return (bHasDef ? gcvTRUE : gcvFALSE);
}

gctBOOL vscVIR_IsUniqueDefInstOfUsageInst(VIR_DEF_USAGE_INFO*     pDuInfo,
                                          VIR_Instruction*        pUsageInst,
                                          VIR_Operand*            pUsageOperand,
                                          VIR_Instruction*        pExpectedUniqueDefInst,
                                          VIR_Instruction**       ppFirstOtherDefInst)
{
    VIR_GENERAL_UD_ITERATOR udIter;
    VIR_DEF*                pDef;
    gctBOOL                 bHasDef = gcvFALSE;

    /* We only consider normal instructions */
    gcmASSERT(pUsageInst < VIR_OUTPUT_USAGE_INST);

    vscVIR_InitGeneralUdIterator(&udIter, pDuInfo, pUsageInst, pUsageOperand, gcvFALSE);
    for (pDef = vscVIR_GeneralUdIterator_First(&udIter); pDef != gcvNULL;
         pDef = vscVIR_GeneralUdIterator_Next(&udIter))
    {
        if (pDef->defKey.pDefInst != pExpectedUniqueDefInst)
        {
            if (ppFirstOtherDefInst)
            {
                *ppFirstOtherDefInst = pDef->defKey.pDefInst;
            }

            return gcvFALSE;
        }

        bHasDef = gcvTRUE;
    }

    return (bHasDef ? gcvTRUE : gcvFALSE);
}

gctBOOL vscVIR_IsUniqueDefInstOfUsagesInItsDUChain(VIR_DEF_USAGE_INFO* pDuInfo,
                                                   VIR_Instruction*    pDefInst,
                                                   VIR_Instruction**   ppFirstOtherDefInst,
                                                   VIR_Instruction**   ppFirstMultiDefUsageInst)
{
    VIR_Enable              defEnableMask;
    gctUINT                 regNo, firstRegNo, regNoRange;
    gctUINT8                channel;
    VIR_GENERAL_DU_ITERATOR duIter;
    VIR_USAGE*              pUsage;

    /* We only consider normal instructions */
    gcmASSERT(pDefInst < VIR_OUTPUT_USAGE_INST);

    if (vscVIR_QueryRealWriteVirRegInfo(pDuInfo->baseTsDFA.baseDFA.pOwnerCG->pOwnerShader,
                                        pDefInst,
                                        &defEnableMask,
                                        gcvNULL,
                                        &firstRegNo,
                                        &regNoRange,
                                        gcvNULL,
                                        gcvNULL))
    {
        for (regNo = firstRegNo; regNo < firstRegNo + regNoRange; regNo ++)
        {
            for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
            {
                if (!VSC_UTILS_TST_BIT(defEnableMask, channel))
                {
                    continue;
                }

                vscVIR_InitGeneralDuIterator(&duIter,
                                             pDuInfo,
                                             pDefInst,
                                             regNo,
                                             channel,
                                             gcvFALSE);

                for (pUsage = vscVIR_GeneralDuIterator_First(&duIter); pUsage != gcvNULL;
                     pUsage = vscVIR_GeneralDuIterator_Next(&duIter))
                {
                    if (VIR_IS_OUTPUT_USAGE_INST(pUsage->usageKey.pUsageInst))
                    {
                        continue;
                    }

                    if (!vscVIR_IsUniqueDefInstOfUsageInst(pDuInfo,
                                                           pUsage->usageKey.pUsageInst,
                                                           pUsage->usageKey.pOperand,
                                                           pDefInst,
                                                           ppFirstOtherDefInst))
                    {
                        if (ppFirstMultiDefUsageInst)
                        {
                            *ppFirstMultiDefUsageInst = pUsage->usageKey.pUsageInst;
                        }

                        return gcvFALSE;
                    }
                }
            }
        }

        return gcvTRUE;
    }

    return gcvFALSE;
}

