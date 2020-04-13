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


#include "gc_vsc.h"

/* This macro is not used anymore, we use estimateDUHashTableSize to estimate the table size. */
#define DEF_USAGE_HASH_TABLE_SIZE   2048

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

static gctUINT _HFUNC_DefPassThroughRegNoInst(const void* pKey)
{
    VIR_DEF_KEY*     pDefKey = (VIR_DEF_KEY*)pKey;

    gcmASSERT(pDefKey->regNo != VIR_INVALID_REG_NO  &&
              pDefKey->pDefInst != VIR_ANY_DEF_INST);

    return pDefKey->regNo ^ (((gctUINT)(gctPTRDIFF_T)pDefKey->pDefInst) << 6);
}

static gctUINT _HFUNC_DefPassThroughRegNo(const void* pKey)
{
    VIR_DEF_KEY*     pDefKey = (VIR_DEF_KEY*)pKey;

    gcmASSERT(pDefKey->regNo != VIR_INVALID_REG_NO);

    return pDefKey->regNo;
}

static gctBOOL _HKCMP_DefKeyInstEqual(const void* pHashKey1, const void* pHashKey2)
{
    VIR_DEF_KEY*     pDefKey1 = (VIR_DEF_KEY*)pHashKey1;
    VIR_DEF_KEY*     pDefKey2 = (VIR_DEF_KEY*)pHashKey2;

    gcmASSERT(pDefKey1->regNo != VIR_INVALID_REG_NO &&
              pDefKey2->regNo != VIR_INVALID_REG_NO &&
              pDefKey1->pDefInst != VIR_ANY_DEF_INST &&
              pDefKey2->pDefInst != VIR_ANY_DEF_INST);

    if ((pDefKey1->regNo == pDefKey2->regNo) &&
        (pDefKey1->pDefInst == pDefKey2->pDefInst)
         &&
        (pDefKey1->channel == pDefKey2->channel   || /* Channel compare */
         pDefKey1->channel == VIR_CHANNEL_ANY     ||
         pDefKey2->channel == VIR_CHANNEL_ANY))
    {
        return gcvTRUE;
    }
    else
    {
        return gcvFALSE;
    }
}

static gctBOOL _HKCMP_DefKeyEqual(const void* pHashKey1, const void* pHashKey2)
{
    VIR_DEF_KEY*     pDefKey1 = (VIR_DEF_KEY*)pHashKey1;
    VIR_DEF_KEY*     pDefKey2 = (VIR_DEF_KEY*)pHashKey2;

    gcmASSERT(pDefKey1->regNo != VIR_INVALID_REG_NO &&
              pDefKey2->regNo != VIR_INVALID_REG_NO);

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
static gctUINT _HFUNC_FirstDefRegNo(const void* pKey)
{
    gctUINT     regNo = (gctUINT)(gctPTRDIFF_T)pKey;

    gcmASSERT(regNo != VIR_INVALID_REG_NO);

    return regNo;
}
static gctBOOL _HKCMP_FirstDefKeyEqual(const void* pHashKey1, const void* pHashKey2)
{
    gctUINT     regNo1 = (gctUINT)(gctPTRDIFF_T)pHashKey1;
    gctUINT     regNo2 = (gctUINT)(gctPTRDIFF_T)pHashKey2;

    gcmASSERT(regNo1 != VIR_INVALID_REG_NO &&
              regNo2 != VIR_INVALID_REG_NO);

    return regNo1 == regNo2;
}

VIR_1st_DEF_INFO* vscVIR_FindFirstDefInfo(VIR_DEF_USAGE_INFO* pDuInfo,
                                           gctUINT FirstDefRegNo)
{
    VIR_1st_DEF_INFO * ptr;

    ptr = (VIR_1st_DEF_INFO *)vscHTBL_DirectGet(pDuInfo->pFirstDefTable, (void*)(gctPTRDIFF_T)FirstDefRegNo);
    return ptr;
}

gctUINT vscVIR_FindFirstDefIndex(VIR_DEF_USAGE_INFO* pDuInfo,
                                 gctUINT FirstDefRegNo)
{
    gctUINT fisrtDefIndex;
    if (pDuInfo->bHashRegNoInst)
    {
        VIR_1st_DEF_INFO * ptr;

        ptr = (VIR_1st_DEF_INFO *)vscHTBL_DirectGet(pDuInfo->pFirstDefTable, (void*)(gctPTRDIFF_T)FirstDefRegNo);
        fisrtDefIndex =  (ptr == gcvNULL) ? VIR_INVALID_DEF_INDEX : ptr->firstDefIndex;
    }
    else
    {
        VIR_DEF_KEY            defKey;
        defKey.pDefInst = VIR_ANY_DEF_INST;
        defKey.regNo = FirstDefRegNo;
        defKey.channel = VIR_CHANNEL_ANY;
        fisrtDefIndex = vscBT_HashSearch(&pDuInfo->defTable, &defKey);
    }
    return fisrtDefIndex;
}

gctUINT vscVIR_FindFirstDefIndexWithChannel(VIR_DEF_USAGE_INFO* pDuInfo,
                                           gctUINT             FirstDefRegNo,
                                           gctUINT8            Channel)
{
    gctUINT fisrtDefIndex = VIR_INVALID_DEF_INDEX;
    if (pDuInfo->bHashRegNoInst)
    {
        VIR_1st_DEF_INFO * ptr;

        ptr = (VIR_1st_DEF_INFO *)vscHTBL_DirectGet(pDuInfo->pFirstDefTable, (void*)(gctPTRDIFF_T)FirstDefRegNo);
        if (gcvNULL != ptr)
        {
            VIR_DEF*                 pDef;
            gctUINT                  defIdx;

            defIdx = ptr->firstDefIndex;
            if (defIdx != VIR_INVALID_DEF_INDEX)
            {
                pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, defIdx);
                if (pDef->defKey.channel == Channel ||
                    Channel == VIR_CHANNEL_ANY)
                {
                    fisrtDefIndex = defIdx;
                }
            }
        }
    }
    else
    {
        VIR_DEF_KEY            defKey;
        defKey.pDefInst = VIR_ANY_DEF_INST;
        defKey.regNo = FirstDefRegNo;
        defKey.channel = Channel;
        fisrtDefIndex = vscBT_HashSearch(&pDuInfo->defTable, &defKey);
    }
    return fisrtDefIndex;
}

static void _AddFirstDefIndex(VIR_DEF_USAGE_INFO* pDuInfo,
                                            gctUINT FirstDefRegNo,
                                            gctUINT NewDefIndex)
{
    VIR_DEF *    pDef;
    if (pDuInfo->bHashRegNoInst)
    {
        VIR_1st_DEF_INFO * firstDefInfo;
        firstDefInfo = vscVIR_FindFirstDefInfo(pDuInfo, FirstDefRegNo);
        if (gcvNULL == firstDefInfo)
        {
            /* first time, create a new entry */
            firstDefInfo = (VIR_1st_DEF_INFO *)vscMM_Alloc(&pDuInfo->pmp.mmWrapper,
                                                           sizeof(VIR_1st_DEF_INFO));
            firstDefInfo->firstDefIndex = firstDefInfo->lastDefIndex = NewDefIndex;
            vscHTBL_DirectSet(pDuInfo->pFirstDefTable,
                              (void*)(gctPTRDIFF_T)FirstDefRegNo,
                              (void*)firstDefInfo);
        }
        else
        {
            /* update the lastDefIndex */
            if (firstDefInfo->firstDefIndex == VIR_INVALID_DEF_INDEX)
            {
                /* the firstDef is entered then removed from the table */
                gcmASSERT(firstDefInfo->lastDefIndex == VIR_INVALID_DEF_INDEX);
                firstDefInfo->firstDefIndex = firstDefInfo->lastDefIndex = NewDefIndex;
            }
            else
            {
                gcmASSERT(firstDefInfo->lastDefIndex != VIR_INVALID_DEF_INDEX);
                pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, NewDefIndex);
                pDef->nextDefIdxOfSameRegNo = firstDefInfo->firstDefIndex;
                firstDefInfo->firstDefIndex = NewDefIndex;
            }
        }
    }
    else
    {
        gctUINT firstDefIdxOfSameRegNo = vscVIR_FindFirstDefIndex(pDuInfo, FirstDefRegNo);
        if (VIR_INVALID_DEF_INDEX != firstDefIdxOfSameRegNo)
        {
            pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, NewDefIndex);
            pDef->nextDefIdxOfSameRegNo = firstDefIdxOfSameRegNo;
        }
    }
    return ;
}

/* update the first def table when deleting a defIndex,
 * DelDefIndex is the defIndex to delete,
 * PrevDefIndex is the previous defIndex in the def chain */
static void _UpdateFirstDefInfo(VIR_DEF_USAGE_INFO* pDuInfo,
                                gctUINT RegNo,
                                gctUINT DelDefIndex,
                                gctUINT PrevDefIndex)
{
    if (pDuInfo->bHashRegNoInst)
    {
        VIR_1st_DEF_INFO * firstDefInfo;
        firstDefInfo = vscVIR_FindFirstDefInfo(pDuInfo, RegNo);
        if (gcvNULL != firstDefInfo)
        {
            VIR_DEF *    pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, DelDefIndex);
            if (firstDefInfo->firstDefIndex == DelDefIndex)
            {
                firstDefInfo->firstDefIndex = pDef->nextDefIdxOfSameRegNo;
                gcmASSERT(PrevDefIndex == VIR_INVALID_DEF_INDEX);
            }
            if (firstDefInfo->lastDefIndex == DelDefIndex)
            {
                firstDefInfo->lastDefIndex = PrevDefIndex;
                gcmASSERT(pDef->nextDefIdxOfSameRegNo == VIR_INVALID_DEF_INDEX);
            }
        }
    }
}

static void _InitializeDef(VIR_DEF* pDef,
                           VIR_Instruction* pDefInst,
                           gctUINT regNo,
                           VIR_Enable enableMask,
                           gctUINT8 halfChannelMask,
                           gctUINT8 channel,
                           VIR_NATIVE_DEF_FLAGS nativeDefFlags)
{
    pDef->defKey.pDefInst = pDefInst;
    pDef->flags.nativeDefFlags.bIsInput = nativeDefFlags.bIsInput;
    pDef->flags.nativeDefFlags.bIsOutput = nativeDefFlags.bIsOutput;
    pDef->flags.nativeDefFlags.bIsPerPrim = nativeDefFlags.bIsPerPrim;
    pDef->flags.nativeDefFlags.bIsPerVtxCp = nativeDefFlags.bIsPerVtxCp;
    pDef->flags.nativeDefFlags.bHwSpecialInput = nativeDefFlags.bHwSpecialInput;
    pDef->flags.deducedDefFlags.bNoUsageCrossRoutine = gcvTRUE;
    pDef->flags.deducedDefFlags.bDynIndexed = gcvFALSE;
    pDef->flags.deducedDefFlags.bIndexingReg = gcvFALSE;
    pDef->flags.deducedDefFlags.bHasUsageOnNoSwizzleInst = gcvFALSE;
    pDef->flags.deducedDefFlags.bHasUsageOnFalseDepInst = gcvFALSE;

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
    pDef->flags.nativeDefFlags.bIsInput = gcvFALSE;
    pDef->flags.nativeDefFlags.bIsOutput = gcvFALSE;
    pDef->flags.nativeDefFlags.bIsPerPrim = gcvFALSE;
    pDef->flags.nativeDefFlags.bIsPerVtxCp = gcvFALSE;
    pDef->flags.nativeDefFlags.bHwSpecialInput = gcvFALSE;
    pDef->flags.deducedDefFlags.bNoUsageCrossRoutine = gcvTRUE;
    pDef->flags.deducedDefFlags.bDynIndexed = gcvFALSE;
    pDef->flags.deducedDefFlags.bIndexingReg = gcvFALSE;
    pDef->flags.deducedDefFlags.bHasUsageOnNoSwizzleInst = gcvFALSE;
    pDef->flags.deducedDefFlags.bHasUsageOnFalseDepInst = gcvFALSE;
    pDef->defKey.regNo = VIR_INVALID_REG_NO;
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
                                 VIR_NATIVE_DEF_FLAGS nativeDefFlags,
                                 gctBOOL bCheckRedundant,
                                 gctBOOL bPartialUpdate,
                                 gctUINT* pRetDefIdxArray,
                                 gctUINT* pUpdatedDefIdxArray,
                                 gctBOOL* pIsUpdateDefHomonymyArray)
{
    VIR_DEF*               pNewDef;
    VIR_DEF*               pOldDef;
    gctUINT                regNo, newDefIdx, oldDefIdx;
    VIR_DEF_KEY            defKey;
    gctUINT8               channel;
    VIR_Enable             totalEnableMask = defEnableMask;
    VIR_Enable             needNewAddEnableMask = VIR_ENABLE_NONE;
    gctBOOL                bNewDefAdded = gcvFALSE;
    VIR_OperandInfo        operandInfo, operandInfo0;

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
            _InitializeDef(pNewDef, pDefInst, regNo, totalEnableMask, halfChannelMask, channel, nativeDefFlags);

            if (pRetDefIdxArray)
            {
                pRetDefIdxArray[(regNo-firstRegNo)*VIR_CHANNEL_NUM+channel] = newDefIdx;
            }

            if (pUpdatedDefIdxArray)
            {
                pUpdatedDefIdxArray[(regNo-firstRegNo)*VIR_CHANNEL_NUM+channel] = newDefIdx;
            }

            if (pIsUpdateDefHomonymyArray)
            {
                pIsUpdateDefHomonymyArray[(regNo-firstRegNo)*VIR_CHANNEL_NUM+channel] =
                                    vscVIR_HasHomonymyDefs(pDuInfo, pNewDef->defKey.pDefInst, pNewDef->defKey.regNo,
                                                        pNewDef->defKey.channel, gcvFALSE);
            }

            /* Now make all defs with same regNo linked together */
            if (bPartialUpdate && 0)
            {
                /* ???????????? BUG: HOW TO MAKE CORRECT SEQUENCE ??????????? */
            }
            else
            {
                /* add newDefIdx to the first def table */
                _AddFirstDefIndex(pDuInfo, regNo, newDefIdx);
            }

            /* Take care of bDynIndexed */
            if (pNewDef->defKey.pDefInst < VIR_INPUT_DEF_INST)
            {
                VIR_Operand_GetOperandInfo(pNewDef->defKey.pDefInst,
                                           pNewDef->defKey.pDefInst->dest,
                                           &operandInfo);

                if (VIR_Inst_GetOpcode(pNewDef->defKey.pDefInst) == VIR_OP_STARR)
                {
                    /* For the case of STARR */
                    VIR_Operand_GetOperandInfo(pNewDef->defKey.pDefInst,
                                               pNewDef->defKey.pDefInst->src[VIR_Operand_Src0],
                                               &operandInfo0);

                    if (!operandInfo0.isImmVal)
                    {
                        pNewDef->flags.deducedDefFlags.bDynIndexed = gcvTRUE;
                    }
                }
                else if (operandInfo.indexingVirRegNo != VIR_INVALID_REG_NO)
                {
                    /* For the case of Rb[Ro.single_channel] access */
                    pNewDef->flags.deducedDefFlags.bDynIndexed = gcvTRUE;
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
                                   gctUINT* pRetDefIdxArray,
                                   gctUINT* pUpdatedDefIdxArray,
                                   gctBOOL* pIsUpdateDefHomonymyArray)
{
    VSC_BLOCK_TABLE*         pUsageTable = &pDuInfo->usageTable;
    VSC_BLOCK_TABLE*         pWebTable = &pDuInfo->webTable;
    VIR_DEF*                 pDefToDel;
    VIR_DEF*                 pTmpDef;
    VIR_DEF*                 pTmpPrevDef;
    gctUINT                  tmpPrevDefIndex;
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

                if (pUpdatedDefIdxArray)
                {
                    pUpdatedDefIdxArray[(regNo-firstRegNo)*VIR_CHANNEL_NUM+channel] = defIdxToDel;
                }

                if (pIsUpdateDefHomonymyArray)
                {
                    pIsUpdateDefHomonymyArray[(regNo-firstRegNo)*VIR_CHANNEL_NUM+channel] =
                                        vscVIR_HasHomonymyDefs(pDuInfo, pDefToDel->defKey.pDefInst, pDefToDel->defKey.regNo,
                                                            pDefToDel->defKey.channel, gcvFALSE);
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
                tmpDefIdx = vscVIR_FindFirstDefIndex(pDuInfo, regNo);
                pTmpPrevDef = gcvNULL;
                tmpPrevDefIndex = VIR_INVALID_DEF_INDEX;
                while (VIR_INVALID_DEF_INDEX != tmpDefIdx)
                {
                    pTmpDef = GET_DEF_BY_IDX(&pDuInfo->defTable, tmpDefIdx);
                    gcmASSERT(pTmpDef);

                    if (tmpDefIdx == defIdxToDel)
                    {
                        _UpdateFirstDefInfo(pDuInfo, regNo,
                                                   defIdxToDel,
                                                   tmpPrevDefIndex);
                        if (pTmpPrevDef != gcvNULL)
                        {
                            pTmpPrevDef->nextDefIdxOfSameRegNo = pTmpDef->nextDefIdxOfSameRegNo;
                        }
                        break;
                    }

                    pTmpPrevDef = pTmpDef;
                    tmpPrevDefIndex = tmpDefIdx;
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
                                        VIR_NATIVE_DEF_FLAGS* pNativeDefFlags,
                                        gctBOOL* pIsIndexing)
{
    VIR_OperandInfo         operandInfo, operandInfo0;
    VIR_Enable              defEnableMask;
    gctUINT8                halfChannelMask = 0;
    gctUINT                 firstRegNo = 0, regNoRange = 0;
    VIR_NATIVE_DEF_FLAGS    nativeDefFlags;
    gctBOOL                 bIsIndexing = gcvFALSE;

    if (pInst == gcvNULL || VIR_Inst_GetDest(pInst) == gcvNULL)
    {
        return gcvFALSE;
    }

    /* Get dst operand info */
    VIR_Operand_GetOperandInfo(pInst,
                               VIR_Inst_GetDest(pInst),
                               &operandInfo);

    if (!VIR_OpndInfo_Is_Virtual_Reg(&operandInfo))
    {
        return gcvFALSE;
    }

    defEnableMask = VIR_Operand_GetEnable(VIR_Inst_GetDest(pInst));

    halfChannelMask = (gctUINT8)operandInfo.halfChannelMask;

    nativeDefFlags.bIsInput = gcvFALSE;
    nativeDefFlags.bHwSpecialInput = operandInfo.needHwSpecialDef;
    nativeDefFlags.bIsOutput = operandInfo.isOutput;
    nativeDefFlags.bIsPerPrim = operandInfo.isPerPrim;
    nativeDefFlags.bIsPerVtxCp = operandInfo.isPerVtxCp;
    nativeDefFlags.reserved = 0;

    /* For the case of Rb[Ro.single_channel] access */
    if (operandInfo.indexingVirRegNo != VIR_INVALID_REG_NO)
    {
        firstRegNo = operandInfo.u1.virRegInfo.startVirReg;
        regNoRange = operandInfo.u1.virRegInfo.virRegCount;
        bIsIndexing = gcvTRUE;
    }
    /* A starr inst may potentially write all elements in array */
    else if (VIR_Inst_GetOpcode(pInst) == VIR_OP_STARR)
    {
        VIR_Operand_GetOperandInfo(pInst,
                                   pInst->src[VIR_Operand_Src0],
                                   &operandInfo0);

        if (operandInfo0.isImmVal)
        {
            firstRegNo = operandInfo.u1.virRegInfo.virReg + operandInfo0.u1.immValue.iValue;
            regNoRange = 1;
        }
        else
        {
            gcmASSERT(VIR_OpndInfo_Is_Virtual_Reg(&operandInfo));
            firstRegNo = operandInfo.u1.virRegInfo.startVirReg;
            regNoRange = operandInfo.u1.virRegInfo.virRegCount;
            bIsIndexing = gcvTRUE;
        }
    }
    /* Normal def */
    else if (VIR_OPCODE_isWritten2Dest(VIR_Inst_GetOpcode(pInst)))
    {
        /* Then add each def for each reg no */
        firstRegNo = operandInfo.u1.virRegInfo.virReg;
        if (VIR_Inst_GetOpcode(pInst) == VIR_OP_COPY)
        {
            regNoRange = operandInfo.u1.virRegInfo.virRegCount;
        }
        else
        {
            regNoRange = 1;
        }
    }

    /* Save the result. */
    if (pDefEnableMask)
    {
        *pDefEnableMask = defEnableMask;
    }

    if (pHalfChannelMask)
    {
        *pHalfChannelMask = halfChannelMask;
    }

    if (pFirstRegNo)
    {
        *pFirstRegNo = firstRegNo;
    }

    if (pRegNoRange)
    {
        *pRegNoRange = regNoRange;
    }

    if (pNativeDefFlags)
    {
        *pNativeDefFlags = nativeDefFlags;
    }

    if (pIsIndexing)
    {
        *pIsIndexing = bIsIndexing;
    }

    return gcvTRUE;
}

static void _AddRealWriteDefs(VIR_Shader* pShader, VIR_DEF_USAGE_INFO* pDuInfo,
                              VSC_BLOCK_TABLE* pDefTable, VIR_Instruction* pInst)
{
    VIR_Enable             defEnableMask;
    gctUINT                firstRegNo, regNoRange;
    gctUINT8               halfChannelMask;
    VIR_NATIVE_DEF_FLAGS   nativeDefFlags;

    if (vscVIR_QueryRealWriteVirRegInfo(pShader,
                                        pInst,
                                        &defEnableMask,
                                        &halfChannelMask,
                                        &firstRegNo,
                                        &regNoRange,
                                        &nativeDefFlags,
                                        gcvNULL))
    {
        _AddNewDefToTable(pDuInfo,
                          pDefTable,
                          firstRegNo,
                          regNoRange,
                          defEnableMask,
                          halfChannelMask,
                          pInst,
                          nativeDefFlags,
                          gcvFALSE,
                          gcvFALSE,
                          gcvNULL,
                          gcvNULL,
                          gcvNULL);
    }
}

static void _AddInputDefs(VIR_Shader* pShader, VIR_DEF_USAGE_INFO* pDuInfo,
                          VSC_BLOCK_TABLE* pDefTable, VIR_Instruction* pInst)
{
    VIR_OperandInfo         operandInfo, operandInfo1;
    VIR_Enable              defEnableMask;
    gctUINT                 firstRegNo, regNoRange;
    VIR_NATIVE_DEF_FLAGS    nativeDefFlags;
    VIR_SrcOperand_Iterator srcOpndIter;
    VIR_Operand*            pOpnd;

    nativeDefFlags.bIsInput = gcvTRUE;
    nativeDefFlags.bIsOutput = gcvFALSE;
    nativeDefFlags.bHwSpecialInput = gcvFALSE;

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
                nativeDefFlags.bIsPerPrim = operandInfo1.isPerPrim;
                nativeDefFlags.bIsPerVtxCp = operandInfo1.isPerVtxCp;

                _AddNewDefToTable(pDuInfo,
                                  pDefTable,
                                  operandInfo1.u1.virRegInfo.virReg,
                                  1,
                                  VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(pInst->src[VIR_Operand_Src1])),
                                  (gctUINT8)operandInfo1.halfChannelMask,
                                  VIR_INPUT_DEF_INST,
                                  nativeDefFlags,
                                  gcvTRUE,
                                  gcvFALSE,
                                  gcvNULL,
                                  gcvNULL,
                                  gcvNULL);
            }

            firstRegNo = operandInfo.u1.virRegInfo.startVirReg;
            regNoRange = operandInfo.u1.virRegInfo.virRegCount;
        }

        if (operandInfo.isInput && VIR_OpndInfo_Is_Virtual_Reg(&operandInfo))
        {
            defEnableMask = VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(pInst->src[VIR_Operand_Src0]));
            nativeDefFlags.bIsPerPrim = operandInfo.isPerPrim;
            nativeDefFlags.bIsPerVtxCp = operandInfo.isPerVtxCp;

            _AddNewDefToTable(pDuInfo,
                              pDefTable,
                              firstRegNo,
                              regNoRange,
                              defEnableMask,
                              (gctUINT8)operandInfo.halfChannelMask,
                              VIR_INPUT_DEF_INST,
                              nativeDefFlags,
                              gcvTRUE,
                              gcvFALSE,
                              gcvNULL,
                              gcvNULL,
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
                nativeDefFlags.bIsPerPrim = operandInfo.isPerPrim;
                nativeDefFlags.bIsPerVtxCp = operandInfo.isPerVtxCp;

                _AddNewDefToTable(pDuInfo,
                                  pDefTable,
                                  firstRegNo,
                                  regNoRange,
                                  defEnableMask,
                                  (gctUINT8)operandInfo.halfChannelMask,
                                  VIR_INPUT_DEF_INST,
                                  nativeDefFlags,
                                  gcvTRUE,
                                  gcvFALSE,
                                  gcvNULL,
                                  gcvNULL,
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
    VIR_NATIVE_DEF_FLAGS    nativeDefFlags;
    VIR_SrcOperand_Iterator srcOpndIter;
    VIR_Operand*            pOpnd;

    nativeDefFlags.bIsInput = gcvFALSE;
    nativeDefFlags.bIsOutput = gcvFALSE;
    nativeDefFlags.bIsPerPrim = gcvFALSE;
    nativeDefFlags.bIsPerVtxCp = gcvFALSE;
    nativeDefFlags.bHwSpecialInput = gcvTRUE;

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
                              nativeDefFlags,
                              gcvTRUE,
                              gcvFALSE,
                              gcvNULL,
                              gcvNULL,
                              gcvNULL);
        }
    }
}

static gctINT estimateDUHashTableSize(VIR_Shader*            pShader)
{
    gctINT instCount = BT_GET_MAX_VALID_ID(&pShader->instTable);
    if (instCount/2 < 32)
    {
        return 32;
    }
    if (instCount > 4000)
    {
        return 2048;
    }
    return instCount/2;
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
    gctINT                 duHTSize = estimateDUHashTableSize(pShader);

    pDuInfo->pFirstDefTable =
        vscHTBL_Create(&pDuInfo->pmp.mmWrapper,
                        _HFUNC_FirstDefRegNo,
                        _HKCMP_FirstDefKeyEqual,
                        duHTSize);

    /* Initialize def table with hash enabled due to we want to get def index with def content */
    vscBT_Initialize(&pDuInfo->defTable,
                     &pDuInfo->pmp.mmWrapper,
                     VSC_BLOCK_TABLE_FLAG_HASH_ENTRIES,
                     sizeof(VIR_DEF),
                     40*sizeof(VIR_DEF),
                     1,
                     gcvNULL,
                     pDuInfo->bHashRegNoInst ? _HFUNC_DefPassThroughRegNoInst : _HFUNC_DefPassThroughRegNo,
                     pDuInfo->bHashRegNoInst ? _HKCMP_DefKeyInstEqual : _HKCMP_DefKeyEqual,
                     duHTSize);

    pDuInfo->maxVirRegNo = 0;
    pDuInfo->bHashRegNoInst = gcvFALSE;

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

static void _Update_ReachDef_Local_Kill_Output_Defs_By_Emit(VIR_Shader* pShader,
                                                            VIR_DEF_USAGE_INFO* pDuInfo,
                                                            VSC_BLOCK_TABLE* pDefTable,
                                                            VSC_BIT_VECTOR* pGenFlow,
                                                            VSC_BIT_VECTOR* pKillFlow,
                                                            gctBOOL bCheckAllOutput,
                                                            gctINT streamNumber)
{
    gctUINT                  defCount = pDuInfo->baseTsDFA.baseDFA.flowSize;
    VIR_DEF*                 pDef;
    VIR_DEF*                 pThisDef;
    VIR_Symbol*              pTempSym;
    VIR_Symbol*              pOutputSym;
    gctUINT                  thisDefIdx, defIdx;
    VSC_BIT_VECTOR           tmpMask;

    vscBV_Initialize(&tmpMask, pDuInfo->baseTsDFA.baseDFA.pScratchMemPool, pDuInfo->baseTsDFA.baseDFA.flowSize);

    for (thisDefIdx = 0; thisDefIdx < defCount; thisDefIdx ++)
    {
        /* Processed before? */
        if (vscBV_TestBit(&tmpMask, thisDefIdx))
        {
            continue;
        }

        pThisDef = GET_DEF_BY_IDX(pDefTable, thisDefIdx);

        /* Skip non-output def. */
        if (!pThisDef->flags.nativeDefFlags.bIsOutput)
        {
            continue;
        }

        /* Find the specified output. */
        if (!bCheckAllOutput)
        {
            pTempSym = VIR_Shader_FindSymbolByTempIndex(pShader, pThisDef->defKey.regNo);
            gcmASSERT(pTempSym);

            pOutputSym = VIR_Symbol_GetVregVariable(pTempSym);
            gcmASSERT(pOutputSym);

            if (VIR_Symbol_GetStreamNumber(pOutputSym) != streamNumber)
            {
                continue;
            }
        }

        defIdx = vscVIR_FindFirstDefIndex(pDuInfo, pThisDef->defKey.regNo);

        while (VIR_INVALID_DEF_INDEX != defIdx)
        {
            pDef = GET_DEF_BY_IDX(pDefTable, defIdx);
            gcmASSERT(pDef);

            /* If this def is a real output, kill it */
            if (pDef->flags.nativeDefFlags.bIsOutput)
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

    vscBV_Finalize(&tmpMask);
}

static void _Update_ReachDef_Local_GenKill(VIR_DEF_USAGE_INFO* pDuInfo,
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
    VSC_BLOCK_TABLE*       pDefTable = &pDuInfo->defTable;
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
        defIdx = vscVIR_FindFirstDefIndex(pDuInfo, regNo);

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
    VIR_DEF_USAGE_INFO*    pDuInfo = (VIR_DEF_USAGE_INFO*)pBaseTsDFA; /* pBaseTsDFA is the first field of VIR_DEF_USAGE_INFO!!! */
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
    vscSV_Initialize(&localHalfChannelKillFlow, pBaseTsDFA->baseDFA.pScratchMemPool, pBaseTsDFA->baseDFA.flowSize, 4);

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
                             vscVIR_IsInstDefiniteWrite(pDuInfo, pInst, firstRegNo, gcvTRUE));

            _Update_ReachDef_Local_GenKill(pDuInfo,
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

            _Update_ReachDef_Local_Kill_Output_Defs_By_Emit(pShader,
                                                            (VIR_DEF_USAGE_INFO*)pBaseTsDFA,
                                                            pDefTable,
                                                            pGenFlow,
                                                            pKillFlow,
                                                            bCheckAllOutput,
                                                            streamNumber);
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
            if (pDef->flags.nativeDefFlags.bIsInput)
            {
                gcmASSERT(pDef->defKey.pDefInst == VIR_INPUT_DEF_INST);

                vscBV_SetBit(pInFlow, defIdx);
            }

            /* HW special inputs */
            if (pDef->flags.nativeDefFlags.bHwSpecialInput && !pDef->flags.nativeDefFlags.bIsOutput)
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

    vscBV_Initialize(&tmpFlow, pBaseTsDFA->baseDFA.pScratchMemPool, pBaseTsDFA->baseDFA.flowSize);

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

    vscBV_Initialize(&tmpFlow, pBaseTsDFA->baseDFA.pScratchMemPool, pBaseTsDFA->baseDFA.flowSize);

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
    VSC_BIT_VECTOR         tmpFlow, tmpFlow1, tmpFlow2;
    gctBOOL                bChanged = gcvFALSE;

    gcmASSERT(pBasicBlock->flowType == VIR_FLOW_TYPE_CALL);

    vscBV_Initialize(&tmpFlow, pBaseTsDFA->baseDFA.pScratchMemPool, pBaseTsDFA->baseDFA.flowSize);
    vscBV_Initialize(&tmpFlow1, pBaseTsDFA->baseDFA.pScratchMemPool, pBaseTsDFA->baseDFA.flowSize);
    vscBV_Initialize(&tmpFlow2, pBaseTsDFA->baseDFA.pScratchMemPool, pBaseTsDFA->baseDFA.flowSize);

    /* U flow that not flows into callee and out flow of callee excluding flows that are from other callers */
    vscBV_And2(&tmpFlow1, &pCalleeFuncFlow->inFlow, pInFlow);
    vscBV_Minus2(&tmpFlow, pInFlow, &tmpFlow1);
    vscBV_Minus2(&tmpFlow1, &pCalleeFuncFlow->inFlow, &tmpFlow1);
    vscBV_Minus2(&tmpFlow1, &pCalleeFuncFlow->outFlow, &tmpFlow1);
    vscBV_And2(&tmpFlow2, &pCalleeFuncFlow->inFlow, &pCalleeFuncFlow->outFlow);
    vscBV_Or1(&tmpFlow, &tmpFlow1);
    vscBV_Or1(&tmpFlow, &tmpFlow2);

    bChanged = !vscBV_Equal(pOutFlow, &tmpFlow);
    if (bChanged)
    {
        vscBV_Copy(pOutFlow, &tmpFlow);
    }

    vscBV_Finalize(&tmpFlow);
    vscBV_Finalize(&tmpFlow1);
    vscBV_Finalize(&tmpFlow2);

    return bChanged;
}

static void _And_Def_BVs_On_Same_Reg_No(VSC_MM* pScratchMemPool, VSC_BLOCK_TABLE* pDefTable,
                                        VSC_BIT_VECTOR* pDefBV1, VSC_BIT_VECTOR* pDefBV2,
                                        gctUINT maxVirRegNo)
{
    VSC_BIT_VECTOR               regNoBV1, regNoBV2, andResBV;
    gctUINT                      regNoBVSize = (maxVirRegNo + 1) * VIR_CHANNEL_NUM;
    gctUINT                      defIdx, startBitOrdinal;
    VIR_DEF*                     pDef;

    vscBV_Initialize(&regNoBV1, pScratchMemPool, regNoBVSize);
    vscBV_Initialize(&regNoBV2, pScratchMemPool, regNoBVSize);
    vscBV_Initialize(&andResBV, pScratchMemPool, regNoBVSize);

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

    vscBV_Initialize(&tmpFlow, pBaseTsDFA->baseDFA.pScratchMemPool, pBaseTsDFA->baseDFA.flowSize);
    vscBV_Initialize(&callerInFlow, pBaseTsDFA->baseDFA.pScratchMemPool, pBaseTsDFA->baseDFA.flowSize);

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
                _And_Def_BVs_On_Same_Reg_No(pBaseTsDFA->baseDFA.pScratchMemPool, pDefTable, &tmpFlow, &callerInFlow, maxVirRegNo);
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
#else
                                             gcvNULL,
                                             gcvNULL
#endif
                                            };

    vscVIR_InitializeBaseTsDFA(&pDuInfo->baseTsDFA,
                               pCg,
                               VIR_DFA_TYPE_REACH_DEF,
                               flowSize,
                               &pDuInfo->pmp.mmWrapper,
                               &tsDfaResolvers);

    /* Do analysis! */
    vscVIR_DoForwardIterativeTsDFA(pCg,
                                   &pDuInfo->baseTsDFA,
#if SUPPORT_IPA_DFA
                                   gcvTRUE
#else
                                   gcvFALSE
#endif
                                   );

    /* Mark we have successfully built the reach-def flow */
    vscVIR_SetDFAFlowBuilt(&pDuInfo->baseTsDFA.baseDFA, gcvTRUE);

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

    return ((gctUINT)(gctUINTPTR_T)pUsageKey->pUsageInst >> 2);
}

static gctBOOL _HKCMP_UsageKeyEqual(const void* pHashKey1, const void* pHashKey2)
{
    VIR_USAGE_KEY*     pUsageKey1 = (VIR_USAGE_KEY*)pHashKey1;
    VIR_USAGE_KEY*     pUsageKey2 = (VIR_USAGE_KEY*)pHashKey2;

    if (pUsageKey1->pOperand == pUsageKey2->pOperand  ||
        pUsageKey1->pOperand == VIR_ANY_USAGE_OPERAND ||
        pUsageKey2->pOperand == VIR_ANY_USAGE_OPERAND)
    {
        return (pUsageKey1->bIsIndexingRegUsage == pUsageKey2->bIsIndexingRegUsage &&
                pUsageKey1->pUsageInst == pUsageKey2->pUsageInst);
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
                             gctBOOL bIsIndexingRegUsage,
                             gctUINT8 realChannelMask,
                             gctUINT8 halfChannelMask)
{
    pUsage->usageKey.pUsageInst = pUsageInst;
    pUsage->usageKey.pOperand = pOperand;
    pUsage->usageKey.bIsIndexingRegUsage = bIsIndexingRegUsage;

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
    pUsage->usageKey.bIsIndexingRegUsage = gcvFALSE;

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
                                   gctBOOL bIsIndexingRegUsage,
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
    VIR_USAGE_KEY            usageKey;
    VSC_BLOCK_TABLE*         pDefTable = &pDuInfo->defTable;
    VSC_BLOCK_TABLE*         pUsageTable = &pDuInfo->usageTable;
    gctUINT                  regNo, defIdx;
    gctUINT                  newUsageIdx = VIR_INVALID_USAGE_INDEX;
    gctUINT8                 channel;
    gctBOOL                  bDefFound = gcvFALSE, bNewUsageAdded = gcvFALSE;
    VIR_DU_CHAIN_USAGE_NODE* pUsageNode;
    VSC_DU_ITERATOR          duIter;
    VIR_OperandInfo          operandInfo, operandInfo1;

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
        usageKey.bIsIndexingRegUsage = bIsIndexingRegUsage;
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
        _InitializeUsage(pNewUsage, &pDuInfo->pmp.mmWrapper, pUsageInst, pOperand,
                         bIsIndexingRegUsage, defEnableMask, halfChannelMask);

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

            defIdx = vscVIR_FindFirstDefIndex(pDuInfo, regNo);

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
                            pDef->flags.deducedDefFlags.bNoUsageCrossRoutine &=
                                ((VIR_Inst_GetFunction(pUsageInst) == VIR_Inst_GetFunction(pDef->defKey.pDefInst)) ? gcvTRUE : gcvFALSE);
                        }

                        if (!VIR_IS_OUTPUT_USAGE_INST(pUsageInst))
                        {
                            VIR_Operand_GetOperandInfo(pUsageInst, pOperand, &operandInfo);

                            if ((VIR_OPCODE_isVX(VIR_Inst_GetOpcode(pUsageInst)) &&
                                 VIR_Inst_GetSourceIndex(pUsageInst, pOperand) == 0) ||
                                VIR_Inst_GetOpcode(pUsageInst) == VIR_OP_SWIZZLE)
                            {
                                pDef->flags.deducedDefFlags.bHasUsageOnNoSwizzleInst = gcvTRUE;
                            }

                            /* On some HW, there is no per-component dependence support for LD/ST/TEXLD def.
                               For ST instruction, the def is same as src2 if it is temp. */
                            if (VIR_OPCODE_hasStoreOperation(VIR_Inst_GetOpcode(pUsageInst)) &&
                                ((VIR_OPCODE_useSrc3AsInstType(VIR_Inst_GetOpcode(pUsageInst)) && VIR_Inst_GetSourceIndex(pUsageInst, pOperand) == 3)
                                 ||
                                 (!VIR_OPCODE_useSrc3AsInstType(VIR_Inst_GetOpcode(pUsageInst)) && VIR_Inst_GetSourceIndex(pUsageInst, pOperand) == 2)))
                            {
                                pDef->flags.deducedDefFlags.bHasUsageOnFalseDepInst = gcvTRUE;
                            }

                            /* Take care of bDynIndexed */
                            else if (VIR_Inst_GetOpcode(pUsageInst) == VIR_OP_LDARR)
                            {
                                /* For the case of LDARR */
                                if (VIR_Inst_GetSourceIndex(pUsageInst, pOperand) == 0)
                                {
                                    VIR_Operand_GetOperandInfo(pUsageInst,
                                                               pUsageInst->src[VIR_Operand_Src1],
                                                               &operandInfo1);

                                    if (!operandInfo1.isImmVal)
                                    {
                                        pDef->flags.deducedDefFlags.bDynIndexed = gcvTRUE;
                                    }
                                }
                            }
                            else if (operandInfo.indexingVirRegNo != VIR_INVALID_REG_NO &&
                                     !bIsIndexingRegUsage)
                            {
                                /* For the case of Rb[Ro.single_channel] access */
                                pDef->flags.deducedDefFlags.bDynIndexed = gcvTRUE;
                            }
                        }

                        pDef->flags.deducedDefFlags.bIndexingReg = bIsIndexingRegUsage;

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
                                     gctBOOL bIsIndexingRegUsage,
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
    usageKey.bIsIndexingRegUsage = bIsIndexingRegUsage;
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

            if (pDefInst == VIR_ANY_DEF_INST)
            {
                defIdx = vscVIR_FindFirstDefIndex(pDuInfo, regNo);
            }
            else
            {
                defKey.pDefInst = pDefInst;
                defKey.regNo = regNo;
                defKey.channel = channel;
                defIdx = vscBT_HashSearch(pDefTable, &defKey);
            }
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

                            /* Usage node is no longer used any more */
                            vscMM_Free(&pDuInfo->pmp.mmWrapper, pUsageNode);

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

        if (VIR_IS_OUTPUT_USAGE_INST(pUsage->usageKey.pUsageInst))
        {
            bTrueOutput = gcvTRUE;
            break;
        }
    }

    if (!bTrueOutput)
    {
        pOutputDef->flags.nativeDefFlags.bIsOutput = gcvFALSE;
    }
    else
    {
        gcmASSERT(pOutputDef->flags.nativeDefFlags.bIsOutput);
    }
}

static gctBOOL _CanAddUsageToOutputDef(VIR_DEF_USAGE_INFO* pDuInfo,
                                       VSC_BIT_VECTOR* pWorkingDefFlow,
                                       gctUINT outputDefIdx,
                                       VIR_Instruction* pOutputUsageInst)
{
    VSC_BLOCK_TABLE*         pDefTable = &pDuInfo->defTable;
    VIR_DEF*                 pOutputDef = GET_DEF_BY_IDX(pDefTable, outputDefIdx);

    gcmASSERT(pOutputDef->flags.nativeDefFlags.bIsOutput);

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
        if (pOutputDef->flags.nativeDefFlags.bIsOutput &&
            vscBV_TestBit(pWorkingDefFlow, outputDefIdx))
        {
            return gcvTRUE;
        }
    }
    else
    {
        /* Calling here is because we need determine implicits output usages for EMIT */

        gcmASSERT(VIR_Inst_GetOpcode(pOutputUsageInst) == VIR_OP_EMIT0  ||
                  VIR_Inst_GetOpcode(pOutputUsageInst) == VIR_OP_EMIT   ||
                  VIR_Inst_GetOpcode(pOutputUsageInst) == VIR_OP_EMIT_STREAM);

        if (vscBV_TestBit(pWorkingDefFlow, outputDefIdx))
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

static void _AddOutputUsages(VIR_Shader* pShader,
                             VIR_DEF_USAGE_INFO* pDuInfo,
                             VSC_BIT_VECTOR* pWorkingDefFlow,
                             VIR_Instruction* pOutputUsageInst,
                             gctBOOL bCheckAllOutput,
                             gctINT streamNumber)
{
    VSC_BLOCK_TABLE*         pDefTable = &pDuInfo->defTable;
    VSC_BLOCK_TABLE*         pUsageTable = &pDuInfo->usageTable;
    gctUINT                  defCount = pDuInfo->baseTsDFA.baseDFA.flowSize;
    VIR_DEF*                 pDef;
    VIR_DEF*                 pThisDef;
    VIR_Symbol*              pTempSym;
    VIR_Symbol*              pOutputSym;
    gctUINT                  newUsageIdx, thisDefIdx, defIdx;
    VIR_USAGE*               pNewUsage;
    VIR_DU_CHAIN_USAGE_NODE* pUsageNode;
    VSC_BIT_VECTOR           tmpMask;

    vscBV_Initialize(&tmpMask, pDuInfo->baseTsDFA.baseDFA.pScratchMemPool, pDuInfo->baseTsDFA.baseDFA.flowSize);

    for (thisDefIdx = 0; thisDefIdx < defCount; thisDefIdx ++)
    {
        /* Processed before? */
        if (vscBV_TestBit(&tmpMask, thisDefIdx))
        {
            continue;
        }

        pThisDef = GET_DEF_BY_IDX(pDefTable, thisDefIdx);

        /* Skip non-output def. */
        if (!pThisDef->flags.nativeDefFlags.bIsOutput)
        {
            continue;
        }

        /* Find the specified output. */
        if (!bCheckAllOutput)
        {
            pTempSym = VIR_Shader_FindSymbolByTempIndex(pShader, pThisDef->defKey.regNo);
            gcmASSERT(pTempSym);

            pOutputSym = VIR_Symbol_GetVregVariable(pTempSym);
            gcmASSERT(pOutputSym);

            if (VIR_Symbol_GetStreamNumber(pOutputSym) != streamNumber)
            {
                continue;
            }
        }

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
                         gcvFALSE,
                         0x0,
                         pThisDef->halfChannelMask);

        /* Add usageIdx to hash */
        vscBT_AddToHash(pUsageTable, newUsageIdx, &pNewUsage->usageKey);

        defIdx = vscVIR_FindFirstDefIndex(pDuInfo, pThisDef->defKey.regNo);

        while (VIR_INVALID_DEF_INDEX != defIdx)
        {
            pDef = GET_DEF_BY_IDX(pDefTable, defIdx);
            gcmASSERT(pDef);

            /* If this def is an output */
            if (pDef->flags.nativeDefFlags.bIsOutput)
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

            _AddNewUsageToTable(pDuInfo,
                                pWorkingDefFlow,
                                pInst,
                                pInst->dest,
                                gcvTRUE,
                                firstRegNo,
                                regNoRange,
                                defEnableMask,
                                (gctUINT8)operandInfo.halfChannelMaskOfIndexingVirRegNo,
                                gcvFALSE,
                                gcvNULL);
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
                _AddNewUsageToTable(pDuInfo,
                                    pWorkingDefFlow,
                                    pInst,
                                    pInst->src[VIR_Operand_Src1],
                                    gcvFALSE,
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
                                gcvFALSE,
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

                _AddNewUsageToTable(pDuInfo,
                                    pWorkingDefFlow,
                                    pInst,
                                    pOpnd,
                                    gcvFALSE,
                                    firstRegNo,
                                    regNoRange,
                                    defEnableMask,
                                    (gctUINT8)operandInfo.halfChannelMask,
                                    gcvFALSE,
                                    gcvNULL);
            }

            /* For the case of Rb[Ro.single_channel] access, we need consider Ro usage */
            if (operandInfo.indexingVirRegNo != VIR_INVALID_REG_NO)
            {
                firstRegNo = operandInfo.indexingVirRegNo;
                regNoRange = 1;
                defEnableMask = (1 << operandInfo.componentOfIndexingVirRegNo);

                _AddNewUsageToTable(pDuInfo,
                                    pWorkingDefFlow,
                                    pInst,
                                    pOpnd,
                                    gcvTRUE,
                                    firstRegNo,
                                    regNoRange,
                                    defEnableMask,
                                    (gctUINT8)operandInfo.halfChannelMaskOfIndexingVirRegNo,
                                    gcvFALSE,
                                    gcvNULL);
            }
        }
    }

    /* Emit will implicitly use all outputs */
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

        _AddOutputUsages(pShader, pDuInfo, pWorkingDefFlow, pInst, bCheckAllOutput, streamNumber);
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

    vscBV_Initialize(&workingDefFlow, pDuInfo->baseTsDFA.baseDFA.pScratchMemPool, pDuInfo->baseTsDFA.baseDFA.flowSize);

    /* Initialize working flow as input flow */
    vscBV_Copy(&workingDefFlow, &pBasicBlk->pTsWorkDataFlow->inFlow);

    /* 4 states we are using:
       VIR_HALF_CHANNEL_MASK_NONE (0),
       VIR_HALF_CHANNEL_MASK_LOW  (1),
       VIR_HALF_CHANNEL_MASK_HIGH (2),
       VIR_HALF_CHANNEL_MASK_FULL (3)
    */
    vscSV_Initialize(&localHalfChannelKillFlow, pDuInfo->baseTsDFA.baseDFA.pScratchMemPool,
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
                             vscVIR_IsInstDefiniteWrite(pDuInfo, pInst, firstRegNo, gcvTRUE));

            _Update_ReachDef_Local_GenKill(pDuInfo,
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

            _Update_ReachDef_Local_Kill_Output_Defs_By_Emit(pShader,
                                                            pDuInfo,
                                                            pDefTable,
                                                            &workingDefFlow,
                                                            gcvNULL,
                                                            bCheckAllOutput,
                                                            streamNumber);
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
                     gcvNULL,
                     _HFUNC_UsageInstLSB8,
                     _HKCMP_UsageKeyEqual,
                     estimateDUHashTableSize(pCg->pOwnerShader));

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
            /* For each basic block, build du/ud chain */
            _BuildDUUDChainPerBB(pThisBlock, pDuInfo);
        }
    }

    pMainFuncFlow = (VIR_TS_FUNC_FLOW*)vscSRARR_GetElement(&pDuInfo->baseTsDFA.tsFuncFlowArray,
                                                           pMainFuncBlock->dgNode.id);
    pMainFlowOut = &pMainFuncFlow->outFlow;

    /* Outputs have special implicit usages from next stage. All false outputs will be
       also corrected by setting bIsOutput as FALSE */
    _AddOutputUsages(pCg->pOwnerShader, pDuInfo, pMainFlowOut, VIR_OUTPUT_USAGE_INST, gcvTRUE, 0);

    pDuInfo->bDUUDChainBuilt = gcvTRUE;

    return errCode;
}

VSC_ErrCode _DestoryDUUDChain(VIR_DEF_USAGE_INFO* pDuInfo, gctBOOL bOnlyFinalizeUsageTable)
{
    VSC_ErrCode              errCode = VSC_ERR_NONE;
    gctUINT                  defCount, defIdx;
    VSC_BLOCK_TABLE*         pDefTable = &pDuInfo->defTable;
    VIR_DEF*                 pDef;
    VIR_DU_CHAIN_USAGE_NODE* pUsageNode;
    VSC_DU_ITERATOR          duIter;

    if (pDuInfo->bDUUDChainBuilt)
    {
        /* We need remove any du chain info for each def if it is requested */
        if (!bOnlyFinalizeUsageTable)
        {
            /* For def */
            defCount = BT_GET_MAX_VALID_ID(&pDuInfo->defTable);
            for (defIdx = 0; defIdx < defCount; defIdx ++)
            {
                pDef = GET_DEF_BY_IDX(pDefTable, defIdx);
                if (IS_VALID_DEF(pDef))
                {
                    VSC_DU_ITERATOR_INIT(&duIter, &pDef->duChain);
                    pUsageNode = VSC_DU_ITERATOR_FIRST(&duIter);
                    for (; pUsageNode != gcvNULL; pUsageNode = VSC_DU_ITERATOR_NEXT(&duIter))
                    {
                        /* Remove this usage from du chain of this def */
                        DU_CHAIN_REMOVE_USAGE(&pDef->duChain, pUsageNode);

                        /* Usage node is no longer used any more */
                        vscMM_Free(&pDuInfo->pmp.mmWrapper, pUsageNode);
                    }

                    DU_CHAIN_INITIALIZE(&pDef->duChain);
                }
            }
        }

        vscBT_Finalize(&pDuInfo->usageTable);

        /* Mark DUUD chain has been destoryed */
        pDuInfo->bDUUDChainBuilt = gcvFALSE;
    }

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
    PVSC_HW_CONFIG           pHwCfg = &pDuInfo->baseTsDFA.baseDFA.pOwnerCG->pOwnerShader->pCompilerCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg;
    VSC_BLOCK_TABLE*         pDefTable = &pDuInfo->defTable;
    VSC_BLOCK_TABLE*         pWebTable = &pDuInfo->webTable;
    VIR_WEB*                 pNewWeb = GET_WEB_BY_IDX(pWebTable, newWebIdx);
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
        if (pDef->flags.nativeDefFlags.bIsOutput)
        {
            defIdx = vscVIR_FindFirstDefIndex(pDuInfo, pDef->defKey.regNo);

            while (VIR_INVALID_DEF_INDEX != defIdx)
            {
                pTempDef = GET_DEF_BY_IDX(pDefTable, defIdx);

                if (pTempDef->webIdx != VIR_INVALID_WEB_INDEX && pTempDef->webIdx != newWebIdx)
                {
                    if (pTempDef->flags.nativeDefFlags.bIsOutput)
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
    if (pDef->flags.nativeDefFlags.bHwSpecialInput || pDef->defKey.pDefInst == VIR_HW_SPECIAL_DEF_INST)
    {
        defIdx = vscVIR_FindFirstDefIndex(pDuInfo, pDef->defKey.regNo);

        while (VIR_INVALID_DEF_INDEX != defIdx)
        {
            pTempDef = GET_DEF_BY_IDX(pDefTable, defIdx);

            if (pTempDef->webIdx != VIR_INVALID_WEB_INDEX && pTempDef->webIdx != newWebIdx)
            {
                if (pTempDef->flags.nativeDefFlags.bHwSpecialInput || pTempDef->defKey.pDefInst == VIR_HW_SPECIAL_DEF_INST)
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
        defIdx = vscVIR_FindFirstDefIndex(pDuInfo, pDef->defKey.regNo);

        while (VIR_INVALID_DEF_INDEX != defIdx)
        {
            pTempDef = GET_DEF_BY_IDX(pDefTable, defIdx);

            if (pTempDef->webIdx != VIR_INVALID_WEB_INDEX && pTempDef->webIdx != newWebIdx &&
                !VIR_IS_IMPLICIT_DEF_INST(pTempDef->defKey.pDefInst))
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

    /* 4. In debug mode, merge all "varibles" with same regno into one web */
    if (gcmOPT_EnableDebug() || gcmOPT_DisableOPTforDebugger())
    {
        VIR_Symbol  *sym = gcvNULL;
        sym = VIR_Shader_FindSymbolByTempIndex(pDuInfo->baseTsDFA.baseDFA.pOwnerCG->pOwnerShader,
                                               pDef->defKey.regNo);

        /* this temp has underlying variable */
        if (sym && VIR_Symbol_GetVregVariable(sym) != gcvNULL)
        {
            defIdx = vscVIR_FindFirstDefIndex(pDuInfo, pDef->defKey.regNo);

            while (VIR_INVALID_DEF_INDEX != defIdx)
            {
                pTempDef = GET_DEF_BY_IDX(pDefTable, defIdx);

                if (pTempDef->webIdx != VIR_INVALID_WEB_INDEX && pTempDef->webIdx != newWebIdx)
                {
                    _MergeTwoWebs(pDuInfo, pTempDef->webIdx, newWebIdx);
                    break;
                }

                /* Get next def with same regNo */
                defIdx = pTempDef->nextDefIdxOfSameRegNo;
            }
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
                         pDuInfo->baseTsDFA.baseDFA.pScratchMemPool,
                         BT_GET_MAX_VALID_ID(&pDuInfo->defTable));

        /* Un-indexing-reg defs who have true usages for same instruction must be in the
           same web. So we need firstly find out these extended defs as def candidates
           for web */
        thisDefIdx = 0;
        while ((thisDefIdx = vscBV_FindSetBitForward(pLocalWorkingFlow, thisDefIdx)) != (gctUINT)INVALID_BIT_LOC)
        {
            gcmASSERT(VIR_INVALID_DEF_INDEX != thisDefIdx);

            pThisDef = GET_DEF_BY_IDX(pDefTable, thisDefIdx);
            gcmASSERT(pThisDef);

            if (!pThisDef->flags.deducedDefFlags.bIndexingReg)
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

        /* Un-indexing-reg defs who have true usages for same instruction must be in the same web */
        if (!bPartialUpdate)
        {
#if MAKE_DEAD_DEF_IN_SEPERATED_WEB
            if (DU_CHAIN_GET_USAGE_COUNT(&pThisDef->duChain) != 0)
#endif
            {
                if (!pThisDef->flags.deducedDefFlags.bIndexingReg)
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

    vscBT_Initialize(&pDuInfo->webTable,
                     &pDuInfo->pmp.mmWrapper,
                     0,
                     sizeof(VIR_WEB),
                     (defCount)*sizeof(VIR_WEB),
                     1,
                     gcvNULL,
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

    vscBV_Initialize(&globalWorkingFlow, pCg->pScratchMemPool, defCount);
    vscBV_Initialize(&localWorkingFlow, pCg->pScratchMemPool, defCount);

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

        defIdx = vscVIR_FindFirstDefIndex(pDuInfo, pDef->defKey.regNo);

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
                                     gctBOOL bBuildDUUDChain,
                                     gctBOOL bBuildWeb)
{
    VSC_ErrCode            errCode = VSC_ERR_NONE;

    /* Initialize duud-chain/web not built */
    pDuInfo->bDUUDChainBuilt = gcvFALSE;
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
    if (bBuildDUUDChain)
    {
        errCode = _BuildDUUDChain(pCg, pDuInfo);
        CHECK_ERROR(errCode, "Build du/ud chain");
    }

    /* Build Web */
    if (bBuildWeb)
    {
        errCode = _BuildWebs(pCg, pDuInfo);
        CHECK_ERROR(errCode, "Build web");
    }

    return errCode;
}

VSC_ErrCode vscVIR_BuildDUUDChain(VIR_CALL_GRAPH* pCg,
                                  VIR_DEF_USAGE_INFO* pDuInfo,
                                  gctBOOL bForceToBuild
                                  )
{
    gcmASSERT(vscVIR_CheckDFAFlowBuilt(&pDuInfo->baseTsDFA.baseDFA));

    if (pDuInfo->bDUUDChainBuilt)
    {
        if (!bForceToBuild)
        {
            return VSC_ERR_NONE;
        }

        /* If DUUD chain is forced to built, we need destory previous one firstly */
        _DestoryDUUDChain(pDuInfo, gcvFALSE);
    }

    return _BuildDUUDChain(pCg, pDuInfo);
}

VSC_ErrCode vscVIR_BuildWebs(VIR_CALL_GRAPH* pCg,
                             VIR_DEF_USAGE_INFO* pDuInfo,
                             gctBOOL bForceToBuild)
{
    gcmASSERT(pDuInfo->bDUUDChainBuilt);

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

    if (vscVIR_CheckDFAFlowBuilt(&pDuInfo->baseTsDFA.baseDFA))
    {
        vscBT_Finalize(&pDuInfo->defTable);

        _DestoryDUUDChain(pDuInfo, gcvTRUE);

        _DestoryWebs(pDuInfo, gcvTRUE);

        vscVIR_FinalizeBaseTsDFA(&pDuInfo->baseTsDFA);
        vscPMP_Finalize(&pDuInfo->pmp);

        /* Mark DU info has been invalid */
        vscVIR_SetDFAFlowBuilt(&pDuInfo->baseTsDFA.baseDFA, gcvFALSE);
    }

    return errCode;
}

VSC_ErrCode vscVIR_DestoryDUUDChain(VIR_DEF_USAGE_INFO* pDuInfo)
{
    return _DestoryDUUDChain(pDuInfo, gcvFALSE);
}

VSC_ErrCode vscVIR_DestoryWebs(VIR_DEF_USAGE_INFO* pDuInfo)
{
    return _DestoryWebs(pDuInfo, gcvFALSE);
}

static gctBOOL _UpdateReachDefFlow(VIR_DEF_USAGE_INFO* pDuInfo,
                                   VIR_BASIC_BLOCK* pDefBasicBlock,
                                   gctUINT* pUpdatedDefIdxArray,
                                   gctBOOL* pIsUpdateDefHomonymyArray,
                                   gctUINT  udiArraySize,
                                   gctBOOL bAddNewDef)
{
    gctBOOL             bSuccUpdated = gcvTRUE;
    VIR_CALL_GRAPH*     pCg = pDuInfo->baseTsDFA.baseDFA.pOwnerCG;
    VIR_BASIC_BLOCK*    pToBasicBlock;
    VIR_TS_FUNC_FLOW*   pDefFuncFlow;
    VIR_TS_FUNC_FLOW*   pToFuncFlow;
    VIR_TS_BLOCK_FLOW*  pDefTsBlockFlow;
    VIR_TS_BLOCK_FLOW*  pToTsBlockFlow;
    gctUINT             orgFlowSize = pDuInfo->baseTsDFA.baseDFA.flowSize;
    VSC_BIT_VECTOR      workingDefFlow;
    VSC_BIT_VECTOR*     pGlobalFwdReachOutBBSet = &pDefBasicBlock->globalReachSet.fwdReachOutBBSet;
    gctUINT             i, defIdx, globalToBbIdx;

    /* We need firstly update flow size */
    vscVIR_UpdateBaseTsDFAFlowSize(&pDuInfo->baseTsDFA, BT_GET_MAX_VALID_ID(&pDuInfo->defTable));

    vscBV_Initialize(&workingDefFlow, pCg->pScratchMemPool, pDuInfo->baseTsDFA.baseDFA.flowSize);

    /* Generate working def flow based on updated def-index-array */
    for (i = 0; i < udiArraySize; i ++)
    {
        defIdx = pUpdatedDefIdxArray[i];

        if (defIdx == VIR_INVALID_DEF_INDEX)
        {
            continue;
        }

        if (pIsUpdateDefHomonymyArray[i])
        {
            bSuccUpdated = gcvFALSE;
            break;
        }

        /* When adding new def, don't consider old def */
        if (bAddNewDef && defIdx < orgFlowSize)
        {
            continue;
        }

        vscBV_SetBit(&workingDefFlow, defIdx);
    }

    if (bSuccUpdated)
    {
        pDefFuncFlow = (VIR_TS_FUNC_FLOW*)vscSRARR_GetElement(&pDuInfo->baseTsDFA.tsFuncFlowArray,
                                                      pDefBasicBlock->pOwnerCFG->pOwnerFuncBlk->dgNode.id);
        pDefTsBlockFlow = (VIR_TS_BLOCK_FLOW*)vscSRARR_GetElement(&pDefFuncFlow->tsBlkFlowArray,
                                                                  pDefBasicBlock->dgNode.id);

        /* A def in BB can always reach to def BB's bottom */
        vscVIR_UpdateTsFlow(&pDefTsBlockFlow->outFlow, &workingDefFlow, !bAddNewDef);

        /* If def bb is the exit, also update out-flow of def func */
        if (pDefBasicBlock->flowType == VIR_FLOW_TYPE_EXIT)
        {
            vscVIR_UpdateTsFlow(&pDefFuncFlow->outFlow, &workingDefFlow, !bAddNewDef);
        }

        /* Update in/out flow of other BBs based on BB's reach-relation */
        globalToBbIdx = 0;
        while ((globalToBbIdx = vscBV_FindSetBitForward(pGlobalFwdReachOutBBSet, globalToBbIdx)) != (gctUINT)INVALID_BIT_LOC)
        {
            pToBasicBlock = CG_GET_BB_BY_GLOBAL_ID(pCg, globalToBbIdx);

            pToFuncFlow = (VIR_TS_FUNC_FLOW*)vscSRARR_GetElement(&pDuInfo->baseTsDFA.tsFuncFlowArray,
                                                          pToBasicBlock->pOwnerCFG->pOwnerFuncBlk->dgNode.id);
            pToTsBlockFlow = (VIR_TS_BLOCK_FLOW*)vscSRARR_GetElement(&pToFuncFlow->tsBlkFlowArray,
                                                                     pToBasicBlock->dgNode.id);

            /* Update in-flow of reached bb */
            vscVIR_UpdateTsFlow(&pToTsBlockFlow->inFlow, &workingDefFlow, !bAddNewDef);

            /* If this reached bb is the entry, also update in-flow of reached-func */
            if (pToBasicBlock->flowType == VIR_FLOW_TYPE_ENTRY)
            {
                vscVIR_UpdateTsFlow(&pToFuncFlow->inFlow, &workingDefFlow, !bAddNewDef);
            }

            /* Def bb has been consider'ed before */
            if (pToBasicBlock != pDefBasicBlock)
            {
                /* Update out-flow of reached bb */
                vscVIR_UpdateTsFlow(&pToTsBlockFlow->outFlow, &workingDefFlow, !bAddNewDef);

                /* If this reached bb is the exit, also update out-flow of reached-func */
                if (pToBasicBlock->flowType == VIR_FLOW_TYPE_EXIT)
                {
                    vscVIR_UpdateTsFlow(&pToFuncFlow->outFlow, &workingDefFlow, !bAddNewDef);
                }
            }

            globalToBbIdx ++;
        }
    }

    vscBV_Finalize(&workingDefFlow);

    return bSuccUpdated;
}

void vscVIR_AddNewDef(VIR_DEF_USAGE_INFO* pDuInfo,
                      VIR_Instruction* pDefInst,
                      gctUINT firstDefRegNo,
                      gctUINT defRegNoRange,
                      VIR_Enable defEnableMask,
                      gctUINT8 halfChannelMask,
                      VIR_NATIVE_DEF_FLAGS* pNativeDefFlags,
                      gctUINT* pRetDefIdxArray)
{
    VSC_BLOCK_TABLE*       pDefTable = &pDuInfo->defTable;
    gctUINT                i;
    gctUINT*               pUpdatedDefIdxArray;
    gctBOOL*               pIsUpdateDefHomonymyArray;
    VIR_NATIVE_DEF_FLAGS   nativeDefFlags;

    pUpdatedDefIdxArray = (gctUINT*)vscMM_Alloc(&pDuInfo->pmp.mmWrapper,
                                                sizeof(gctUINT)*VIR_CHANNEL_NUM*defRegNoRange);
    pIsUpdateDefHomonymyArray = (gctBOOL*)vscMM_Alloc(&pDuInfo->pmp.mmWrapper,
                                                sizeof(gctBOOL)*VIR_CHANNEL_NUM*defRegNoRange);

    for (i = 0; i < VIR_CHANNEL_NUM*defRegNoRange; i ++)
    {
        if (pRetDefIdxArray)
        {
            pRetDefIdxArray[i] = VIR_INVALID_DEF_INDEX;
        }

        pUpdatedDefIdxArray[i] = VIR_INVALID_DEF_INDEX;
        pIsUpdateDefHomonymyArray[i] = gcvFALSE;
    }

    if (pNativeDefFlags)
    {
        nativeDefFlags = *pNativeDefFlags;
    }
    else
    {
        memset(&nativeDefFlags, 0, sizeof(VIR_NATIVE_DEF_FLAGS));
    }

    if (_AddNewDefToTable(pDuInfo,
                          pDefTable,
                          firstDefRegNo,
                          defRegNoRange,
                          defEnableMask,
                          halfChannelMask,
                          pDefInst,
                          nativeDefFlags,
                          gcvTRUE,
                          gcvTRUE,
                          pRetDefIdxArray,
                          pUpdatedDefIdxArray,
                          pIsUpdateDefHomonymyArray))
    {
        /* If flow has been invalidated, no need to update it anymore */
        if (!pDuInfo->baseTsDFA.baseDFA.cmnDfaFlags.bFlowInvalidated &&
            pDefInst != VIR_INPUT_DEF_INST)
        {
            if (!_UpdateReachDefFlow(pDuInfo,
                                     VIR_Inst_GetBasicBlock(pDefInst),
                                     pUpdatedDefIdxArray,
                                     pIsUpdateDefHomonymyArray,
                                     VIR_CHANNEL_NUM*defRegNoRange,
                                     gcvTRUE))
            {
                /* If flow can not be updated, then mark DFA's flow is invalid now */
                pDuInfo->baseTsDFA.baseDFA.cmnDfaFlags.bFlowInvalidated = gcvTRUE;
            }
        }
    }

    vscMM_Free(&pDuInfo->pmp.mmWrapper, pUpdatedDefIdxArray);
    vscMM_Free(&pDuInfo->pmp.mmWrapper, pIsUpdateDefHomonymyArray);
}

void vscVIR_DeleteDef(VIR_DEF_USAGE_INFO* pDuInfo,
                      VIR_Instruction* pDefInst,
                      gctUINT firstDefRegNo,
                      gctUINT defRegNoRange,
                      VIR_Enable defEnableMask,
                      gctUINT8 halfChannelMask,
                      gctUINT* pRetDefIdxArray)
{
    VSC_BLOCK_TABLE*       pDefTable = &pDuInfo->defTable;
    gctUINT                i;
    gctUINT*               pUpdatedDefIdxArray;
    gctBOOL*               pIsUpdateDefHomonymyArray;

    pUpdatedDefIdxArray = (gctUINT*)vscMM_Alloc(&pDuInfo->pmp.mmWrapper,
                                                sizeof(gctUINT)*VIR_CHANNEL_NUM*defRegNoRange);
    pIsUpdateDefHomonymyArray = (gctBOOL*)vscMM_Alloc(&pDuInfo->pmp.mmWrapper,
                                                sizeof(gctBOOL)*VIR_CHANNEL_NUM*defRegNoRange);

    for (i = 0; i < VIR_CHANNEL_NUM*defRegNoRange; i ++)
    {
        if (pRetDefIdxArray)
        {
            pRetDefIdxArray[i] = VIR_INVALID_DEF_INDEX;
        }

        pUpdatedDefIdxArray[i] = VIR_INVALID_DEF_INDEX;
        pIsUpdateDefHomonymyArray[i] = gcvFALSE;
    }

    if (_DeleteDefFromTable(pDuInfo,
                            pDefTable,
                            firstDefRegNo,
                            defRegNoRange,
                            defEnableMask,
                            halfChannelMask,
                            pDefInst,
                            pRetDefIdxArray,
                            pUpdatedDefIdxArray,
                            pIsUpdateDefHomonymyArray))
    {
        /* If flow has been invalidated, no need to update it anymore */
        if (!pDuInfo->baseTsDFA.baseDFA.cmnDfaFlags.bFlowInvalidated)
        {
            if (!_UpdateReachDefFlow(pDuInfo,
                                     VIR_Inst_GetBasicBlock(pDefInst),
                                     pUpdatedDefIdxArray,
                                     pIsUpdateDefHomonymyArray,
                                     VIR_CHANNEL_NUM*defRegNoRange,
                                     gcvFALSE))
            {
                /* If flow can not be updated, then mark DFA's flow is invalid now */
                pDuInfo->baseTsDFA.baseDFA.cmnDfaFlags.bFlowInvalidated = gcvTRUE;
            }
        }
    }

    vscMM_Free(&pDuInfo->pmp.mmWrapper, pUpdatedDefIdxArray);
    vscMM_Free(&pDuInfo->pmp.mmWrapper, pIsUpdateDefHomonymyArray);
}

void vscVIR_AddNewUsageToDef(VIR_DEF_USAGE_INFO* pDuInfo,
                             VIR_Instruction* pDefInst,
                             VIR_Instruction* pUsageInst,
                             VIR_Operand* pOperand,
                             gctBOOL bIsIndexingRegUsage,
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
    VIR_DEF*                 pDef;

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
                     pDuInfo->baseTsDFA.baseDFA.pScratchMemPool,
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

            /* Find all def inst. */
            if (pDefInst == VIR_ANY_DEF_INST)
            {
                defIdx = vscVIR_FindFirstDefIndex(pDuInfo, regNo);

                while (VIR_INVALID_DEF_INDEX != defIdx)
                {
                    vscBV_SetBit(&tmpWorkingDefFlow, defIdx);

                    /* Get next def with same regNo */
                    pDef = GET_DEF_BY_IDX(pDefTable, defIdx);
                    gcmASSERT(pDef);
                    defIdx = pDef->nextDefIdxOfSameRegNo;
                }
            }
            /* Find the specific def inst. */
            else
            {
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
    }

    /* Add new usage based on target defs */
    _AddNewUsageToTable(pDuInfo,
                        &tmpWorkingDefFlow,
                        pUsageInst,
                        pOperand,
                        bIsIndexingRegUsage,
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
                        gctBOOL bIsIndexingRegUsage,
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
                          bIsIndexingRegUsage,
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
                           gctBOOL                bIsIndexingRegUsage,
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
    usageKey.bIsIndexingRegUsage = bIsIndexingRegUsage;

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
                                  gctBOOL                   bIsIndexingRegUsage,
                                  gctBOOL                   bSameBBOnly)
{
    gctUINT          usageIdx;
    VIR_USAGE*       pUsage;
    VIR_OperandInfo  operandInfo;

    gcmASSERT(pUsageOperand);

    pIter->bSameBBOnly = bSameBBOnly;
    pIter->usageKey.pUsageInst = pUsageInst;
    pIter->usageKey.pOperand = pUsageOperand;
    pIter->usageKey.bIsIndexingRegUsage = bIsIndexingRegUsage;
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

gctBOOL vscVIR_HasHomonymyDefs(VIR_DEF_USAGE_INFO*          pDuInfo,
                               VIR_Instruction*             pDefInst,
                               gctUINT                      defRegNo,
                               gctUINT8                     defChannel,
                               gctBOOL                      bSameBBOnly)
{
    gctUINT                defIdx;
    VIR_DEF*               pDef;

    gcmASSERT(pDefInst != VIR_ANY_DEF_INST);

    defIdx = vscVIR_FindFirstDefIndex(pDuInfo, defRegNo);

    /* Note that order is reversed */
    while (VIR_INVALID_DEF_INDEX != defIdx)
    {
        pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, defIdx);
        gcmASSERT(pDef->defKey.regNo == defRegNo);

        if (defChannel == pDef->defKey.channel &&
            pDef->defKey.pDefInst != pDefInst &&
            (!bSameBBOnly ||
             ARE_INSTS_IN_SAME_BASIC_BLOCK(pDef->defKey.pDefInst, pDefInst)))
        {
            return gcvTRUE;
        }

        /* Get next def with same regNo */
        defIdx = pDef->nextDefIdxOfSameRegNo;
    }

    return gcvFALSE;
}

VIR_DEF* vscVIR_GetNextHomonymyDef(VIR_DEF_USAGE_INFO*      pDuInfo,
                                   VIR_Instruction*         pDefInst,
                                   gctUINT                  defRegNo,
                                   gctUINT8                 defChannel,
                                   gctBOOL                  bSameBBOnly)
{
    gctUINT                defIdx;
    VIR_DEF*               pDef;
    VIR_DEF*               pPrevDef = gcvNULL;

    gcmASSERT(pDefInst != VIR_ANY_DEF_INST);

    defIdx = vscVIR_FindFirstDefIndex(pDuInfo, defRegNo);

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

gctBOOL vscVIR_IsUniqueUsageInstOfDefInst(VIR_DEF_USAGE_INFO* pDuInfo,
                                          VIR_Instruction*    pDefInst,
                                          VIR_Instruction*    pExpectedUniqueUsageInst,
                                          VIR_Operand*        pExpectedUniqueUsageOperand,
                                          gctBOOL             bIsIdxingRegForExpectedUniqueUsage,
                                          VIR_Instruction**   ppFirstOtherUsageInst,
                                          VIR_Operand**       ppFirstOtherUsageOperand,
                                          gctBOOL*            pIsIdxingRegForFirstOtherUsage)
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
                    if ((pUsage->usageKey.pUsageInst != pExpectedUniqueUsageInst) ||
                        (pExpectedUniqueUsageOperand && ((pUsage->usageKey.pOperand != pExpectedUniqueUsageOperand) ||
                                                         (pUsage->usageKey.bIsIndexingRegUsage != bIsIdxingRegForExpectedUniqueUsage))))
                    {
                        if (ppFirstOtherUsageInst)
                        {
                            *ppFirstOtherUsageInst = pUsage->usageKey.pUsageInst;
                        }

                        if (ppFirstOtherUsageOperand)
                        {
                            *ppFirstOtherUsageOperand = pUsage->usageKey.pOperand;
                        }

                        if (pIsIdxingRegForFirstOtherUsage)
                        {
                            *pIsIdxingRegForFirstOtherUsage = pUsage->usageKey.bIsIndexingRegUsage;
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

gctBOOL vscVIR_IsUniqueDefInstOfUsageInst(VIR_DEF_USAGE_INFO*     pDuInfo,
                                          VIR_Instruction*        pUsageInst,
                                          VIR_Operand*            pUsageOperand,
                                          gctBOOL                 bIsIndexingRegUsage,
                                          VIR_Instruction*        pExpectedUniqueDefInst,
                                          VIR_Instruction**       ppFirstDefInstOrFirstOtherDefInst)
{
    VIR_GENERAL_UD_ITERATOR udIter;
    VIR_DEF*                pDef;
    gctBOOL                 bHasDef = gcvFALSE;
    gctBOOL                 bCheckGivenDefInst = (pExpectedUniqueDefInst != gcvNULL);
    VIR_Instruction*        pFirstDefInst = gcvNULL;

    /* We only consider normal instructions */
    gcmASSERT(pUsageInst < VIR_OUTPUT_USAGE_INST);

    vscVIR_InitGeneralUdIterator(&udIter, pDuInfo, pUsageInst, pUsageOperand, bIsIndexingRegUsage, gcvFALSE);
    for (pDef = vscVIR_GeneralUdIterator_First(&udIter); pDef != gcvNULL;
         pDef = vscVIR_GeneralUdIterator_Next(&udIter))
    {
        /* Compare all defined instructions with the given defined instruction. */
        if (bCheckGivenDefInst)
        {
            if (pDef->defKey.pDefInst != pExpectedUniqueDefInst)
            {
                if (ppFirstDefInstOrFirstOtherDefInst)
                {
                    *ppFirstDefInstOrFirstOtherDefInst = pDef->defKey.pDefInst;
                }

                return gcvFALSE;
            }
        }
        /* Compare all defined instructions. */
        else
        {
            if (pFirstDefInst == gcvNULL)
            {
                pFirstDefInst = pDef->defKey.pDefInst;
            }
            else if (pFirstDefInst != pDef->defKey.pDefInst)
            {
                return gcvFALSE;
            }

            if (ppFirstDefInstOrFirstOtherDefInst)
            {
                *ppFirstDefInstOrFirstOtherDefInst = pFirstDefInst;
            }
        }

        bHasDef = gcvTRUE;
    }

    return (bHasDef ? gcvTRUE : gcvFALSE);
}

/* Given a register no, check whether it has a unique def instruction, if yes, return TRUE and the def instruction. */
gctBOOL vscVIR_IsRegNoHasUniqueDefInst(VIR_DEF_USAGE_INFO*      pDuInfo,
                                       VIR_VirRegId             regNo,
                                       VIR_Instruction**        ppUniqueDefInst)
{
    VIR_Instruction*    pDefInst = gcvNULL;
    VIR_DEF*            pDef;
    VIR_DEF_KEY         defKey;
    gctUINT             tempDefIdx;

    /* Find all DEFs. */
    defKey.pDefInst = VIR_ANY_DEF_INST;
    defKey.regNo = regNo;
    defKey.channel = VIR_CHANNEL_ANY;
    tempDefIdx = vscBT_HashSearch(&pDuInfo->defTable, &defKey);

    while (VIR_INVALID_DEF_INDEX != tempDefIdx)
    {
        pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, tempDefIdx);

        if (pDefInst == gcvNULL)
        {
            pDefInst = pDef->defKey.pDefInst;
        }
        else if (pDefInst != pDef->defKey.pDefInst)
        {
            return gcvFALSE;
        }

        tempDefIdx = pDef->nextDefIdxOfSameRegNo;
    }

    if (ppUniqueDefInst)
    {
        *ppUniqueDefInst = pDefInst;
    }

    return gcvTRUE;
}

/* Given a register no, if one of its DEF is conditional write, check whether all its def instructions have the same writeMask. */
gctBOOL vscVIR_IsRegAllDefHaveSameWriteMask(VIR_DEF_USAGE_INFO*      pDuInfo,
                                            VIR_VirRegId             regNo)
{
    gctBOOL             bMatched = gcvTRUE;
    VIR_Instruction*    pDefInst = gcvNULL;
    VIR_OpCode          defInstOpCode;
    VIR_DEF*            pDef;
    VIR_DEF_KEY         defKey;
    gctUINT             tempDefIdx;
    VIR_TypeId          instTypeId = VIR_INVALID_ID;
    gctUINT             startBin = 0xFFFF, endBin = 0xFFFF;
    gctUINT             i;

    /* Find all DEFs. */
    defKey.pDefInst = VIR_ANY_DEF_INST;
    defKey.regNo = regNo;
    defKey.channel = VIR_CHANNEL_ANY;
    tempDefIdx = vscBT_HashSearch(&pDuInfo->defTable, &defKey);

    /*
    ** So far we only check for VX instruction:
    **      if all its def instructions are VX instruction and have the same startBin/endBin, they have the same writeMask.
    */
    while (VIR_INVALID_DEF_INDEX != tempDefIdx)
    {
        VIR_Operand*    typeOpnd = gcvNULL;
        VIR_TypeId      tyId;

        pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, tempDefIdx);
        pDefInst = pDef->defKey.pDefInst;

        /* Implicit DEF or non-VX DEF, just return. */
        if (VIR_IS_IMPLICIT_DEF_INST(pDefInst) || !VIR_OPCODE_isVX(VIR_Inst_GetOpcode(pDefInst)))
        {
            bMatched = gcvFALSE;
            break;
        }

        /* For VX ImgLoad, HW may disable some channels(when is out of border) to be written, so the writeMask is not fixed. */
        if (VIR_OPCODE_isVXImgLoad(VIR_Inst_GetOpcode(pDefInst)))
        {
            bMatched = gcvFALSE;
            break;
        }

        defInstOpCode = VIR_Inst_GetOpcode(pDefInst);
        if (VIR_OPCODE_hasDest(defInstOpCode))
        {
            typeOpnd = VIR_Inst_GetDest(pDefInst);
        }
        else
        {
            if (VIR_OPCODE_useSrc2AsInstType(defInstOpCode))
            {
                typeOpnd = VIR_Inst_GetSource(pDefInst, 2);
            }
            else if (VIR_OPCODE_useSrc3AsInstType(defInstOpCode))
            {
                typeOpnd = VIR_Inst_GetSource(pDefInst, 3);
            }
            else
            {
                gcmASSERT(VIR_OPCODE_useSrc0AsInstType(defInstOpCode));
                typeOpnd = VIR_Inst_GetSource(pDefInst, 0);
            }
        }

        tyId = VIR_Operand_GetTypeId(typeOpnd);
        gcmASSERT(tyId < VIR_TYPE_PRIMITIVETYPE_COUNT);

        for (i = 0; i < VIR_Inst_GetSrcNum(pDefInst); i++)
        {
            VIR_Operand *pOpnd = VIR_Inst_GetSource(pDefInst, i);
            VIR_EVIS_Modifier evisModifier;

            if (pOpnd && VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_EVIS_MODIFIER)
            {
                evisModifier.u1 = VIR_Operand_GetEvisModifier(pOpnd);

                if (startBin == 0xFFFF)
                {
                    startBin = evisModifier.u0.startBin;
                    endBin = evisModifier.u0.endBin;
                    instTypeId = tyId;
                }
                else if (startBin != evisModifier.u0.startBin   ||
                         endBin != evisModifier.u0.endBin       ||
                         instTypeId != tyId)
                {
                    bMatched = gcvFALSE;
                    break;
                }
            }
        }

        if (!bMatched)
        {
            break;
        }

        /* Get the next DEF. */
        tempDefIdx = pDef->nextDefIdxOfSameRegNo;
    }

    return bMatched;
}

/*
** Check whether this instruction is a definite write.
**      1) non-conditional write.
**      2) this DEF is the unique DEF.
**      3) conditional write, but use the fixed writeMask.
*/
gctBOOL vscVIR_IsInstDefiniteWrite(VIR_DEF_USAGE_INFO*   pDuInfo,
                                   VIR_Instruction*      pInst,
                                   VIR_VirRegId          regNo,
                                   gctBOOL               bCheckDef)
{
    gctBOOL                 bDefiniteWrite = gcvFALSE;
    VIR_OpCode              opCode = VIR_Inst_GetOpcode(pInst);
    VIR_Instruction*        pDefInst = gcvNULL;
    gctBOOL                 bUseConstForEvisState = gcvTRUE;

    if (VIR_OPCODE_DestOnlyUseEnable(opCode))
    {
        return gcvFALSE;
    }

    if (!VIR_Inst_ConditionalWrite(pInst))
    {
        bDefiniteWrite = gcvTRUE;
    }

    if (!bDefiniteWrite && bCheckDef)
    {
        /*
        ** Although this instruction is a conditional write, if it is the unique DEF instruction for this reg,
        ** we can still local kill this instruction.
        */
        if (vscVIR_IsRegNoHasUniqueDefInst(pDuInfo, regNo, &pDefInst) &&
            pDefInst == pInst)
        {
            bDefiniteWrite = gcvTRUE;
        }
        /* For a VX instruction, if all its def instructions have the same writeMask, we can treat it as a definite write. */
        else if (VIR_OPCODE_isVX(opCode) && bUseConstForEvisState)
        {
            bDefiniteWrite = vscVIR_IsRegAllDefHaveSameWriteMask(pDuInfo, regNo);
        }
    }

    return bDefiniteWrite;
}

#define GotoResult(Value)  do { (result) = (Value); goto OnError; } while (0)

static gctBOOL _CheckTwoBasicBlockSameBranch(VIR_DEF_USAGE_INFO* pDuInfo,
                                             VIR_BB*             pFromBB,
                                             VIR_BB*             pToBB,
                                             VSC_BIT_VECTOR*     pBBMask)
{
    VSC_ADJACENT_LIST*      pList = &pFromBB->dgNode.succList;
    VIR_CFG_EDGE*           pEdge;
    VSC_ADJACENT_LIST_ITERATOR   edgeIter;
    gctBOOL                 result = gcvFALSE;

    /* Detect a recursion, just return FALSE. */
    if (vscBV_TestBit(pBBMask, pFromBB->globalBbId))
    {
        GotoResult(gcvFALSE);
    }
    vscBV_SetBit(pBBMask, pFromBB->globalBbId);

    /* Same BB, return TRUE.*/
    if (pFromBB == pToBB)
    {
        GotoResult(gcvTRUE);
    }

    /* Check the edges. */
    VSC_ADJACENT_LIST_ITERATOR_INIT(&edgeIter, pList);
    pEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edgeIter);

    /* No edge. */
    if (pEdge == gcvNULL)
    {
        GotoResult(gcvFALSE);
    }
    else
    {
        /* We only have two leaves for a success branch, TRUE of FALSE. */
        gctBOOL leafResult[2] = { gcvFALSE, gcvFALSE };
        gctUINT leafIdx = 0;

        for (; pEdge != gcvNULL; pEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edgeIter))
        {
            VIR_BB* pNextBB = CFG_EDGE_GET_TO_BB(pEdge);

            /*
            ** Three situations:
            ** 1) This BB is a conditional branch, check if TRUE and FALSE branch can both reach the dest BB.
            ** 2) This BB is a non-conditional branch:
            **      a) The target BB is the dest BB, match case.
            **      b) The target BB is not the dest BB, check this target BB.
            **
            */
            if (CFG_EDGE_GET_TYPE(pEdge) != VIR_CFG_EDGE_TYPE_ALWAYS)
            {
                gcmASSERT(leafIdx < 2);

                leafResult[leafIdx] = _CheckTwoBasicBlockSameBranch(pDuInfo, pNextBB, pToBB, pBBMask);

                if (leafIdx == 0)
                {
                    if (!leafResult[0])
                    {
                        GotoResult(gcvFALSE);
                    }
                }
                else if (leafIdx == 1)
                {
                    if (leafResult[0] && leafResult[1])
                    {
                        GotoResult(gcvTRUE);
                    }
                    else
                    {
                        GotoResult(gcvFALSE);
                    }
                }

                leafIdx++;
            }
            else if (pNextBB == pToBB)
            {
                GotoResult(gcvTRUE);
            }
            else
            {
                result = _CheckTwoBasicBlockSameBranch(pDuInfo, pNextBB, pToBB, pBBMask);
                GotoResult(result);
            }
        }
    }

OnError:
    vscBV_ClearBit(pBBMask, pFromBB->globalBbId);
    return result;
}

gctBOOL vscVIR_IsDefInstAndUsageInstSameBranch(VIR_DEF_USAGE_INFO* pDuInfo,
                                               VIR_Instruction*    pUsageInst,
                                               VIR_Instruction*    pDefInst)
{
    gctBOOL                 bSameBranch = gcvFALSE;
    VIR_BB*                 pUsageBB = VIR_Inst_GetBasicBlock(pUsageInst);
    VIR_BB*                 pDefBB = VIR_Inst_GetBasicBlock(pDefInst);
    VIR_Function*           pUsageFunc = VIR_Inst_GetFunction(pUsageInst);
    VIR_Function*           pDefFunc = VIR_Inst_GetFunction(pDefInst);
    gctUINT                 bbCount;
    VSC_BIT_VECTOR          bbMask;

    if (pUsageFunc != pDefFunc)
    {
        return bSameBranch;
    }

    bbCount = CG_GET_HIST_GLOBAL_BB_COUNT(VIR_Function_GetFuncBlock(pUsageFunc)->pOwnerCG);
    vscBV_Initialize(&bbMask, pDuInfo->baseTsDFA.baseDFA.pScratchMemPool, bbCount);

    bSameBranch = _CheckTwoBasicBlockSameBranch(pDuInfo, pDefBB, pUsageBB, &bbMask);

    vscBV_Finalize(&bbMask);
    return bSameBranch;
}

gctBOOL vscVIR_DoesDefInstHaveUniqueUsageInst(VIR_DEF_USAGE_INFO* pDuInfo,
                                              VIR_Instruction*    pDefInst,
                                              gctBOOL             bUniqueOperand,
                                              VIR_Instruction**   ppUniqueUsageInst,
                                              VIR_Operand**       ppUniqueUsageOperand,
                                              gctBOOL*            pIsIdxingRegForUniqueUsage)
{
    VIR_Enable              defEnableMask;
    gctUINT                 regNo, firstRegNo, regNoRange;
    gctUINT8                channel;
    VIR_GENERAL_DU_ITERATOR duIter;
    VIR_USAGE*              pUsage;
    VIR_Instruction*        pFirstUsageInst = gcvNULL;
    VIR_Operand*            pFirstUsageOperand = gcvNULL;
    gctBOOL                 bIsIndexingRegUsageForFirstUsage = gcvFALSE;

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
                    if (pUsage->usageKey.pUsageInst == VIR_OUTPUT_USAGE_INST)
                    {
                        return gcvFALSE;
                    }

                    if (pFirstUsageInst == gcvNULL && pFirstUsageOperand == gcvNULL)
                    {
                        pFirstUsageInst = pUsage->usageKey.pUsageInst;
                        pFirstUsageOperand = pUsage->usageKey.pOperand;
                        bIsIndexingRegUsageForFirstUsage = pUsage->usageKey.bIsIndexingRegUsage;
                    }
                    else if ((pUsage->usageKey.pUsageInst != pFirstUsageInst) ||
                             (bUniqueOperand && ((pUsage->usageKey.pOperand != pFirstUsageOperand) ||
                                                 (pUsage->usageKey.bIsIndexingRegUsage != bIsIndexingRegUsageForFirstUsage))))
                    {
                        return gcvFALSE;
                    }
                }
            }
        }

        /* No usage found. */
        if (pFirstUsageInst == gcvNULL)
        {
            return gcvFALSE;
        }

        if (ppUniqueUsageInst)
        {
            *ppUniqueUsageInst = pFirstUsageInst;
        }

        if (ppUniqueUsageOperand)
        {
            *ppUniqueUsageOperand = pFirstUsageOperand;
        }

        if (pIsIdxingRegForUniqueUsage)
        {
            *pIsIdxingRegForUniqueUsage = bIsIndexingRegUsageForFirstUsage;
        }

        return gcvTRUE;
    }

    return gcvFALSE;
}

gctBOOL vscVIR_DoesUsageInstHaveUniqueDefInst(VIR_DEF_USAGE_INFO* pDuInfo,
                                              VIR_Instruction*    pUsageInst,
                                              VIR_Operand*        pUsageOperand,
                                              gctBOOL             bIsIndexingRegUsage,
                                              VIR_Instruction**   ppUniqueDefInst)
{
    VIR_GENERAL_UD_ITERATOR udIter;
    VIR_DEF*                pDef;
    gctBOOL                 bHasDef = gcvFALSE;
    VIR_Instruction*        pFirstDefInst = gcvNULL;

    /* We only consider normal instructions */
    gcmASSERT(pUsageInst < VIR_OUTPUT_USAGE_INST);

    vscVIR_InitGeneralUdIterator(&udIter, pDuInfo, pUsageInst, pUsageOperand, bIsIndexingRegUsage, gcvFALSE);
    for (pDef = vscVIR_GeneralUdIterator_First(&udIter); pDef != gcvNULL;
         pDef = vscVIR_GeneralUdIterator_Next(&udIter))
    {
        if (VIR_IS_IMPLICIT_DEF_INST(pDef->defKey.pDefInst))
        {
            return gcvFALSE;
        }

        if (pFirstDefInst == gcvNULL)
        {
            pFirstDefInst = pDef->defKey.pDefInst;
        }
        else if (pDef->defKey.pDefInst != pFirstDefInst)
        {
            return gcvFALSE;
        }

        bHasDef = gcvTRUE;
    }

    if (bHasDef)
    {
        if (ppUniqueDefInst)
        {
            *ppUniqueDefInst = pFirstDefInst;
        }

        return gcvTRUE;
    }
    else
    {
        return gcvFALSE;
    }
}

gctBOOL vscVIR_IsUniqueUsageInstOfDefsInItsUDChain(VIR_DEF_USAGE_INFO* pDuInfo,
                                                   VIR_Instruction*    pUsageInst,
                                                   VIR_Operand*        pUsageOperand,
                                                   gctBOOL             bIsIndexingRegUsage,
                                                   VIR_Instruction**   ppFirstMultiUsageDefInst,
                                                   VIR_Instruction**   ppFirstOtherUsageInst)
{
    VIR_GENERAL_UD_ITERATOR udIter;
    VIR_DEF*                pDef;
    gctBOOL                 bHasDef = gcvFALSE;

    /* We only consider normal instructions */
    gcmASSERT(pUsageInst < VIR_OUTPUT_USAGE_INST);

    vscVIR_InitGeneralUdIterator(&udIter, pDuInfo, pUsageInst, pUsageOperand, bIsIndexingRegUsage, gcvFALSE);
    for (pDef = vscVIR_GeneralUdIterator_First(&udIter); pDef != gcvNULL;
         pDef = vscVIR_GeneralUdIterator_Next(&udIter))
    {
        if (VIR_IS_IMPLICIT_DEF_INST(pDef->defKey.pDefInst))
        {
            continue;
        }

        if (!vscVIR_IsUniqueUsageInstOfDefInst(pDuInfo, pDef->defKey.pDefInst, pUsageInst,
                                               gcvNULL, gcvFALSE, ppFirstOtherUsageInst, gcvNULL, gcvNULL))
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
                    else if (pUsage->usageKey.pUsageInst->_opcode < 0 ||
                             pUsage->usageKey.pUsageInst->_opcode >= VIR_OP_MAXOPCODE)
                    {
                        return gcvFALSE;
                    }
                    if (!vscVIR_IsUniqueDefInstOfUsageInst(pDuInfo,
                                                           pUsage->usageKey.pUsageInst,
                                                           pUsage->usageKey.pOperand,
                                                           pUsage->usageKey.bIsIndexingRegUsage,
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

/*
    If def is between endInst and startInst, return gcvTURE.

    travese backward from the defInst (to all its predecessors):
    if meet endInst, return gcvFALSE;
    if meet startInst, return gcvTRUE;
    if meet the entry, return gcvFALSE;
    if the currBB is already visited, return gcvFALSE;
*/

static gctBOOL _vscVIR_DefInstInBetween(
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

            if (_vscVIR_DefInstInBetween(startInst, endInst, BB_GET_END_INST(pPredBasicBlk), visitSet))
            {
                return gcvTRUE;
            }
        }
    }

    return gcvFALSE;
}

static gctBOOL _vscVIR_DefBBInBetween(
    IN VIR_BB               *pStartBB,
    IN VIR_BB               *pEndBB,
    IN VIR_BB               *pReDefBB,
    IN VSC_BIT_VECTOR       *pFlowMask,
    IN VSC_BIT_VECTOR       *pCheckStatusMask,
    IN VSC_BIT_VECTOR       *pCheckValueMask,
    INOUT gctBOOL           *pMeetReDefBB
    )
{
    VSC_ADJACENT_LIST*          pList = &pStartBB->dgNode.succList;
    VIR_CFG_EDGE*               pEdge;
    VSC_ADJACENT_LIST_ITERATOR  edgeIter;
    /* There are only three edge, ALWYAS, TRUE and FALSE. */
    gctBOOL                     result[VIR_CFG_EDGE_TYPE_COUNT] = { gcvFALSE, gcvFALSE, gcvFALSE };
    gctUINT                     idx = 0;

    /* In any flow, when we meet EndBB, if we have already met ReDefBB, this ReDefBB is between StartBB and EndBB. */
    gcmASSERT(pMeetReDefBB);

    /* Check StartBB first. */
    if (pStartBB == pReDefBB)
    {
        *pMeetReDefBB = gcvTRUE;
    }
    else if (pStartBB == pEndBB)
    {
        if (*pMeetReDefBB)
        {
            return gcvTRUE;
        }
    }

    /* Detect a loop flow, just return FALSE. */
    if (vscBV_TestBit(pFlowMask, pStartBB->globalBbId))
    {
        return gcvFALSE;
    }
    vscBV_SetBit(pFlowMask, pStartBB->globalBbId);

    /* Check if this bb is checked before, if so, just return the value. */
    if (vscBV_TestBit(pCheckStatusMask, pStartBB->globalBbId))
    {
        return vscBV_TestBit(pCheckValueMask, pStartBB->globalBbId);
    }

    /* Check all edges. */
    VSC_ADJACENT_LIST_ITERATOR_INIT(&edgeIter, pList);
    for (pEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edgeIter);
         pEdge != gcvNULL;
         pEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edgeIter), idx++)
    {
        VIR_BB* pNextBB = CFG_EDGE_GET_TO_BB(pEdge);
        gctBOOL meetReDefBB = gcvFALSE;

        /* Meet EndBB. */
        if (pNextBB == pEndBB)
        {
            if (*pMeetReDefBB)
            {
                result[idx] = gcvTRUE;
            }
        }
        else
        {
            meetReDefBB = *pMeetReDefBB;
            if (pNextBB == pReDefBB)
            {
                meetReDefBB = gcvTRUE;
            }
            result[idx] = _vscVIR_DefBBInBetween(pNextBB,
                                                 pEndBB,
                                                 pReDefBB,
                                                 pFlowMask,
                                                 pCheckStatusMask,
                                                 pCheckValueMask,
                                                 &meetReDefBB);
        }

        if (result[idx])
        {
            break;
        }
    }

    /* Clear the flow mask. */
    vscBV_ClearBit(pFlowMask, pStartBB->globalBbId);

    /* Set the check status. */
    vscBV_SetBit(pCheckStatusMask, pStartBB->globalBbId);

    if (result[0] || result[1] || result[2])
    {
        vscBV_SetBit(pCheckValueMask, pStartBB->globalBbId);
        return gcvTRUE;
    }
    else
    {
        return gcvFALSE;
    }
}

static gctBOOL
_IsRedefineBetweenInsts(
    IN VSC_CHECK_REDEFINED_RES  *pResInfo,
    IN VIR_DEF_USAGE_INFO       *duInfo,
    IN VIR_Instruction          *startInst,
    IN VIR_Instruction          *endInst,
    IN VIR_Operand              *srcOpndOfStartInst,
    IN gctBOOL                  bCheckSameBBOnly,
    IN gctBOOL                  bCheckDifferentBBOnly,
    OUT VIR_Instruction         **redefInst
    )
{
    VSC_MM          *pMM = pResInfo->pMM;
    gctBOOL         retValue = gcvFALSE;
    VIR_Instruction *pDefInst = gcvNULL;
    VSC_HASH_TABLE  *pInstHashTable = pResInfo->pInstHashTable;
    VSC_HASH_TABLE  *pBBHashTable = pResInfo->pBBHashTable;

    VIR_OperandInfo srcInfo;
    VIR_Enable      enableMask = VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(srcOpndOfStartInst));

    gctUINT         firstRegNo, regNoRange, regNo, defIdx;
    gctUINT8        channel;
    VIR_DEF         *pDef;
    gctBOOL         bIsStartEndInSameBB = gcvFALSE;

    if (VIR_Inst_GetBasicBlock(startInst) == VIR_Inst_GetBasicBlock(endInst))
    {
        bIsStartEndInSameBB = gcvTRUE;
    }

    VIR_Operand_GetOperandInfo(startInst, srcOpndOfStartInst, &srcInfo);
    firstRegNo = srcInfo.u1.virRegInfo.virReg;
    regNoRange = srcInfo.u1.virRegInfo.virRegCount;

    /* Get instruction hash table. */
    if (pInstHashTable == gcvNULL)
    {
        pInstHashTable = vscHTBL_Create(pMM, vscHFUNC_Default, vscHKCMP_Default, 512);
        pResInfo->pInstHashTable = pInstHashTable;
    }

    /* Get the BB hash table. */
    if (pBBHashTable == gcvNULL)
    {
        pBBHashTable = vscHTBL_Create(pMM, vscHFUNC_Default, vscHKCMP_Default, 512);
        pResInfo->pBBHashTable = pBBHashTable;
    }

    /* find all defs of the same firstRegNo*/
    for (regNo = firstRegNo; regNo < firstRegNo + regNoRange; regNo ++)
    {
        for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
        {
            if (!VSC_UTILS_TST_BIT(enableMask, channel))
            {
                continue;
            }

            defIdx = vscVIR_FindFirstDefIndex(duInfo, regNo);
            while (VIR_INVALID_DEF_INDEX != defIdx)
            {
                pDef = GET_DEF_BY_IDX(&duInfo->defTable, defIdx);
                gcmASSERT(pDef);
                pDefInst = pDef->defKey.pDefInst;

                /* Skip unmatch def. */
                if (pDef->defKey.channel != channel || VIR_IS_SPECIAL_INST(pDefInst))
                {
                    /* Get the next def. */
                    defIdx = pDef->nextDefIdxOfSameRegNo;
                    continue;
                }

                if (bIsStartEndInSameBB)
                {
                    if (bCheckSameBBOnly &&
                        (VIR_Inst_GetBasicBlock(pDefInst) !=VIR_Inst_GetBasicBlock(startInst)))
                    {
                        defIdx = pDef->nextDefIdxOfSameRegNo;
                        continue;
                    }
                    else if (bCheckDifferentBBOnly &&
                        (VIR_Inst_GetBasicBlock(pDefInst) ==VIR_Inst_GetBasicBlock(startInst)))
                    {
                        defIdx = pDef->nextDefIdxOfSameRegNo;
                        continue;
                    }
                }

                /* If this DEF instruction is checked before, just skip it. */
                if (vscHTBL_DirectTestAndGet(pInstHashTable, (void*)pDefInst, gcvNULL))
                {
                    defIdx = pDef->nextDefIdxOfSameRegNo;
                    continue;
                }

                vscHTBL_DirectSet(pInstHashTable, (void*)pDefInst, gcvNULL);

                /* Get a refined match. */
                if (pDefInst == endInst || pDefInst == startInst)
                {
                    retValue = gcvTRUE;
                    *redefInst = pDefInst;
                    break;
                }

                /* Start to check this def. */
                /*
                ** If START/END/DEF instruction are in the same BB, the check would be more easier because we can treat the entire BB as
                ** a atomic operation and we don't need to check the back-jmp situation, we only need to check if this DEF instruction is
                ** within this instruction fragment:
                ** 1) DEF is between START and END, just return TRUE.
                ** 2) DEF is after END or before start, just continue to check the rest DEF instruction.
                */
                if (bIsStartEndInSameBB && VIR_Inst_GetBasicBlock(pDefInst) == VIR_Inst_GetBasicBlock(startInst))
                {
                    VIR_BB*                 pBB = VIR_Inst_GetBasicBlock(pDefInst);
                    VIR_Instruction*        pIndexInst = gcvNULL;
                    gctBOOL                 bMeetStart = gcvFALSE;

                    pIndexInst = BB_GET_START_INST(pBB);

                    while (pIndexInst && pIndexInst != VIR_Inst_GetNext(BB_GET_END_INST(pBB)))
                    {
                        pIndexInst = VIR_Inst_GetNext(pIndexInst);

                        if (pIndexInst == pDefInst)
                        {
                            /* DEF is between START and END, just return TRUE. */
                            if (bMeetStart)
                            {
                                retValue = gcvTRUE;
                                *redefInst = pDefInst;
                            }
                            break;
                        }
                        else if (pIndexInst == startInst)
                        {
                            bMeetStart = gcvTRUE;
                        }
                        else if (pIndexInst == endInst)
                        {
                            break;
                        }
                    }

                    if (retValue)
                    {
                        break;
                    }
                    else
                    {
                        defIdx = pDef->nextDefIdxOfSameRegNo;
                        continue;
                    }
                }

                vscHTBL_Reset(pBBHashTable);
                /* If any of the instructions in the workSet, that is between
                    endInst and startInst, return gcvTURE.*/
                if (_vscVIR_DefInstInBetween(startInst, endInst, pDef->defKey.pDefInst, pBBHashTable))
                {
                    retValue = gcvTRUE;
                    *redefInst = pDefInst;
                    break;
                }
                else
                {
                    if (VIR_Inst_GetFunction(startInst) == VIR_Inst_GetFunction(endInst)
                        &&
                        VIR_Inst_GetFunction(startInst) == VIR_Inst_GetFunction(pDefInst))
                    {
                        VIR_BB* pStartBB = VIR_Inst_GetBasicBlock(startInst);
                        VIR_BB* pEndBB = VIR_Inst_GetBasicBlock(endInst);
                        VIR_BB* pReDefBB = VIR_Inst_GetBasicBlock(pDefInst);
                        gctBOOL meetReDefBB = gcvFALSE;
                        gctUINT bbCount;
                        VSC_BIT_VECTOR* pBBFlowMask = pResInfo->pBBFlowMask;
                        VSC_BIT_VECTOR* pBBCheckStatusMask = pResInfo->pBBCheckStatusMask;
                        VSC_BIT_VECTOR* pBBCheckValueMask = pResInfo->pBBCheckValueMask;

                        bbCount = CG_GET_HIST_GLOBAL_BB_COUNT(VIR_Function_GetFuncBlock(VIR_Inst_GetFunction(startInst))->pOwnerCG);

                        /* Create or resize the BB flow mask. */
                        if (pBBFlowMask == gcvNULL)
                        {
                            pBBFlowMask = vscBV_Create(pMM, bbCount);
                            pResInfo->pBBFlowMask = pBBFlowMask;
                        }
                        else
                        {
                            vscBV_Resize(pBBFlowMask, bbCount, gcvFALSE);
                        }

                        /* Create or resize the BB check status mask. */
                        if (pBBCheckStatusMask == gcvNULL)
                        {
                            pBBCheckStatusMask = vscBV_Create(pMM, bbCount);
                            pResInfo->pBBCheckStatusMask = pBBCheckStatusMask;
                        }
                        else
                        {
                            vscBV_Resize(pBBCheckStatusMask, bbCount, gcvFALSE);
                        }

                        /* Create or resize the BB check value mask. */
                        if (pBBCheckValueMask == gcvNULL)
                        {
                            pBBCheckValueMask = vscBV_Create(pMM, bbCount);
                            pResInfo->pBBCheckValueMask = pBBCheckValueMask;
                        }
                        else
                        {
                            vscBV_Resize(pBBCheckValueMask, bbCount, gcvFALSE);
                        }

                        if (_vscVIR_DefBBInBetween(pStartBB,
                                                   pEndBB,
                                                   pReDefBB,
                                                   pBBFlowMask,
                                                   pBBCheckStatusMask,
                                                   pBBCheckValueMask,
                                                   &meetReDefBB))
                        {
                            retValue = gcvTRUE;
                            *redefInst = pDefInst;
                            break;
                        }
                    }
                }

                /* Get the next def. */
                defIdx = pDef->nextDefIdxOfSameRegNo;
            }

            if (retValue)
            {
                break;
            }
        }
    }

    vscHTBL_Reset(pInstHashTable);
    vscHTBL_Reset(pBBHashTable);

    return retValue;
}

/*
    If there is define of srcOpnd that is between startInst and endInst,
    return gcvTRUE.

    For example:
    MOV t1.x, t2.y        (startInst)
    SUB t2.y, t5.x, t6.x  (def is between startInst and endInst, return gcvTRUE)
    ADD t3.x, t1.x, t4.x  (endInst)

    Make sure the loop is correctly handled.

    Above check is not enough, it can't handle if redefine is after the endInst and there is a back JMP to the startInst.
    For example, check the below code segment, the redfine Inst "r2 += r2" is after the endInst "r3 = r1".
    To solve this, use CFG to check if there is not refineInst BB between startBB and endBB.

    gctBOOL check = gcvFALSE;
    for (; ;)
    {
        if (cond_true)
        {
            if (!check)
            {
                r1 = r2;
                check = gcvTRUE;
            }
        }
        else if (check)
        {
            r3 = r1;
        }

        r2 += r2;

        if (...)
        {
            ...
            break;
        }
    }

*/
gctBOOL vscVIR_RedefineBetweenInsts(
    IN VSC_CHECK_REDEFINED_RES  *pResInfo,
    IN VIR_DEF_USAGE_INFO       *duInfo,
    IN VIR_Instruction          *startInst,
    IN VIR_Instruction          *endInst,
    IN VIR_Operand              *srcOpndOfStartInst,
    OUT VIR_Instruction         **redefInst
    )
{
    gctBOOL         retValue = gcvFALSE;
    gctBOOL         bIsStartEndInSameBB = gcvFALSE;

    if (VIR_Inst_GetBasicBlock(startInst) == VIR_Inst_GetBasicBlock(endInst))
    {
        bIsStartEndInSameBB = gcvTRUE;
    }

    /* If there is no other instruction between start and end, just return. */
    if (bIsStartEndInSameBB && VIR_Inst_GetNext(startInst) == endInst)
    {
        return retValue;
    }

    /*
    ** If START/END are in the same basic block, we only need to check if there is any DEFs between START and END,
    ** if no, we can just return FALSE.
    */
    if (bIsStartEndInSameBB)
    {
        retValue = _IsRedefineBetweenInsts(pResInfo, duInfo, startInst, endInst, srcOpndOfStartInst, gcvTRUE, gcvFALSE, redefInst);

        if (retValue)
        {
            return gcvTRUE;
        }
        else
        {
            return gcvFALSE;
        }
    }
    else
    {
        retValue = _IsRedefineBetweenInsts(pResInfo, duInfo, startInst, endInst, srcOpndOfStartInst, gcvFALSE, gcvFALSE, redefInst);
        return retValue;
    }
}

/* Find the unique nearest defined instruction. */
gctBOOL vscVIR_FindUniqueNearestDefInst(
    IN VIR_DEF_USAGE_INFO*          pDuInfo,
    IN VIR_Instruction*             pUsageInst,
    IN VIR_Operand*                 pUsageOpnd,
    IN VIR_Instruction*             pStartSearchInst,
    IN PFN_VSC_DEF_CMP              pDefCmpFunc,
    INOUT VIR_Instruction**         ppNearestDefInst
    )
{
    gctBOOL                         bFound = gcvFALSE;
    VIR_Instruction*                pWorkingInst = pStartSearchInst;
    VIR_BB*                         pWorkingBB = VIR_Inst_GetBasicBlock(pWorkingInst);
    VSC_ADJACENT_LIST_ITERATOR      prevEdgeIter;
    VIR_CFG_EDGE*                   pPrevEdge = gcvNULL;
    VIR_BB*                         pPrevBB = gcvNULL;
    VIR_Instruction*                pNearestDefInst = gcvNULL;
    VIR_Instruction*                pPrevDefInst = gcvNULL;
    VIR_OperandInfo                 usageOpndInfo, destOpndInfo;
    VIR_Operand*                    pDestOpnd = gcvNULL;
    VIR_Enable                      usageEnable2Swizzle = VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(pUsageOpnd));

    VIR_Operand_GetOperandInfo(pUsageInst, pUsageOpnd, &usageOpndInfo);

    /* Search the current BB. */
    while(gcvTRUE)
    {
        pDestOpnd = VIR_Inst_GetDest(pWorkingInst);

        if (pDestOpnd)
        {
            VIR_Operand_GetOperandInfo(pWorkingInst, pDestOpnd, &destOpndInfo);

            if (destOpndInfo.isVreg
                &&
                destOpndInfo.u1.virRegInfo.virReg == usageOpndInfo.u1.virRegInfo.virReg
                &&
                VIR_Enable_Covers(VIR_Operand_GetEnable(pDestOpnd), usageEnable2Swizzle))
            {
                bFound = gcvTRUE;
            }
        }

        if (bFound)
        {
            pNearestDefInst = pWorkingInst;
            break;
        }

        /* Get the previous instruction. */
        if (pWorkingInst == BB_GET_START_INST(pWorkingBB))
        {
            break;
        }
        else
        {
            pWorkingInst = VIR_Inst_GetPrev(pWorkingInst);
        }
    }

    /* Find a nearest def. */
    if (bFound)
    {
        if (pDefCmpFunc)
        {
            bFound = pDefCmpFunc(pNearestDefInst);
        }

        /* Get the result. */
        if (ppNearestDefInst && bFound)
        {
            *ppNearestDefInst = pNearestDefInst;
        }

        return bFound;
    }

    /* Not find, search all the previous edges. */
    VSC_ADJACENT_LIST_ITERATOR_INIT(&prevEdgeIter, &pWorkingBB->dgNode.predList);
    pPrevEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&prevEdgeIter);
    for (; pPrevEdge != gcvNULL; pPrevEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&prevEdgeIter))
    {
        pPrevBB = CFG_EDGE_GET_TO_BB(pPrevEdge);

        if (pPrevBB == gcvNULL || BB_GET_LENGTH(pPrevBB) == 0)
        {
            continue;
        }
        bFound = vscVIR_FindUniqueNearestDefInst(pDuInfo,
                                                 pUsageInst,
                                                 pUsageOpnd,
                                                 BB_GET_END_INST(pPrevBB),
                                                 pDefCmpFunc,
                                                 &pNearestDefInst);

        /* If there are multiple definition, they must be the same. */
        if (bFound)
        {
            if (pPrevDefInst == gcvNULL)
            {
                pPrevDefInst = pNearestDefInst;
            }
            else if (pPrevDefInst != pNearestDefInst)
            {
                bFound = gcvFALSE;
                break;
            }
        }
    }

    /* Get the result. */
    if (ppNearestDefInst && bFound)
    {
        *ppNearestDefInst = pNearestDefInst;
    }

    return bFound;
}

