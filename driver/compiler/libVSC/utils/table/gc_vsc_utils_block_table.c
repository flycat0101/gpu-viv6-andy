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


#include "gc_vsc.h"

VSC_BLOCK_TABLE* vscBT_Create(
    VSC_MM*                 pMM,
    gctUINT                 flag,
    gctUINT                 entrySize,
    gctUINT                 blockSize,
    gctUINT                 initBlockCount,
    PFN_VSC_HASH_FUNC       pfnHashFunc,
    PFN_VSC_KEY_CMP         pfnKeyCmp,
    gctINT                  hashTableSize)
{
    VSC_BLOCK_TABLE*     pBT = gcvNULL;

    pBT = (VSC_BLOCK_TABLE*)vscMM_Alloc(pMM, sizeof(VSC_BLOCK_TABLE));

    vscBT_Initialize(pBT, pMM, flag, entrySize, blockSize,
                     initBlockCount, pfnHashFunc, pfnKeyCmp, hashTableSize);

    return pBT;
}

void vscBT_Initialize(
    VSC_BLOCK_TABLE*        pBT,
    VSC_MM*                 pMM,
    gctUINT                 flag,
    gctUINT                 entrySize,
    gctUINT                 blockSize,
    gctUINT                 initBlockCount,
    PFN_VSC_HASH_FUNC       pfnHashFunc,
    PFN_VSC_KEY_CMP         pfnKeyCmp,
    gctINT                  hashTableSize)
{
    gcmASSERT(entrySize > 0);
    gcmASSERT(!(flag & VSC_BLOCK_TABLE_FLAG_FREE_ENTRY_LIST) || (entrySize >= sizeof(VSC_BT_FREE_ENTRY)));

    /* Force to safe block size */
    if (blockSize < entrySize)
    {
        blockSize = entrySize;
    }

    pBT->flag = flag;
    pBT->pHashTable = gcvNULL;
    pBT->entrySize = entrySize;
    pBT->blockSize = vscAlignToPow2(blockSize, MAX_EFFECT_POW2_EXPONENT);
    pBT->entryCountPerBlock = pBT->blockSize / pBT->entrySize;
    gcmASSERT(pBT->entrySize <= pBT->blockSize);

    gcmASSERT(initBlockCount > 0);
    pBT->blockCount = initBlockCount;
    pBT->ppBlockArray = (VSC_BT_BLOCK_PTR*)vscMM_Alloc(pMM, initBlockCount*sizeof(VSC_BT_BLOCK_PTR));
    memset(pBT->ppBlockArray, 0, initBlockCount*sizeof(VSC_BT_BLOCK_PTR));

    pBT->curBlockIdx = 0;
    pBT->nextOffsetInCurBlock = 0;
    pBT->pFreeEntryList = gcvNULL;
    pBT->pMM = pMM;

    if (flag & VSC_BLOCK_TABLE_FLAG_HASH_ENTRIES)
    {
        pBT->pHashTable = vscHTBL_Create(pMM, pfnHashFunc, pfnKeyCmp, hashTableSize);
    }
}

void vscBT_Finalize(VSC_BLOCK_TABLE* pBT)
{
    gctUINT i;

    if (pBT->pHashTable)
    {
        gcmASSERT(BT_HAS_HASHTABLE(pBT));
        vscHTBL_Destroy(pBT->pHashTable);
    }
    pBT->pHashTable = gcvNULL;

    pBT->entrySize = 0;
    pBT->blockSize = 0;
    pBT->blockCount = 0;

    pBT->pFreeEntryList = gcvNULL;

    for (i = 0; i < pBT->blockCount; i ++)
    {
        if (pBT->ppBlockArray[i])
        {
            vscMM_Free(pBT->pMM, pBT->ppBlockArray[i]);
        }
    }

    vscMM_Free(pBT->pMM, pBT->ppBlockArray);
    pBT->ppBlockArray = gcvNULL;

    pBT->curBlockIdx = 0;
    pBT->nextOffsetInCurBlock = 0;
    pBT->flag = (VSC_BLOCK_TABLE_FLAG)0;
}

void vscBT_Destroy(VSC_BLOCK_TABLE* pBT)
{
    if (pBT)
    {
        vscBT_Finalize(pBT);
        vscMM_Free(pBT->pMM, pBT);
        pBT = gcvNULL;
    }
}

static gctUINT _AllocContinuousEntries(VSC_BLOCK_TABLE* pBT, void* pData, gctUINT entryCount)
{
    gctUINT   totalReqSize, firstEntryId = INVALID_BT_ENTRY_ID;
    gctUINT8* pFirstEntry;

    totalReqSize = pBT->entrySize * entryCount;

    if (totalReqSize >= pBT->blockSize)
    {
        /* fatal error, the table is not big enough for requested entries */
        gcmASSERT(gcvFALSE);
        return INVALID_BT_ENTRY_ID;
    }

    /* If current block can not accomodate requested size, then move to next block */
    if (pBT->blockSize- pBT->nextOffsetInCurBlock < totalReqSize)
    {
        pBT->curBlockIdx ++;
        pBT->nextOffsetInCurBlock = 0;
    }

    /* Oops, the requested size is too big, let's increase block count to allocate more blocks */
    if (pBT->curBlockIdx == pBT->blockCount)
    {
        /* Increase the block array size by double */
        pBT->blockCount *= 2;
        pBT->ppBlockArray = (VSC_BT_BLOCK_PTR*) vscMM_Realloc(pBT->pMM,
                                                              pBT->ppBlockArray,
                                                              pBT->blockCount*sizeof(VSC_BT_BLOCK_PTR));

        gcmASSERT(pBT->ppBlockArray != gcvNULL);

        memset(pBT->ppBlockArray + pBT->curBlockIdx, 0,
               (pBT->blockCount - pBT->curBlockIdx)*sizeof(VSC_BT_BLOCK_PTR));
    }

    /* If the block has not been allocated, now allocate it */
    if (pBT->ppBlockArray[pBT->curBlockIdx] == gcvNULL)
    {
        pBT->ppBlockArray[pBT->curBlockIdx] = (VSC_BT_BLOCK_PTR)vscMM_Alloc(pBT->pMM, pBT->blockSize);
    }

    pFirstEntry = pBT->ppBlockArray[pBT->curBlockIdx] + pBT->nextOffsetInCurBlock;
    pBT->nextOffsetInCurBlock += totalReqSize;

    if (pData != gcvNULL)
        memcpy(pFirstEntry, pData, totalReqSize);
    else
        memset(pFirstEntry, 0, totalReqSize);

    firstEntryId = BT_MAKE_ENTRY_ID(pBT, pBT->curBlockIdx,
                                    (gctINT)((gctUINT8*)pFirstEntry - pBT->ppBlockArray[pBT->curBlockIdx]));
    if (BT_AUTO_HASH(pBT))
    {
        /* add the newly added entry to hash table */
        vscBT_AddToHash(pBT, firstEntryId, pFirstEntry);
    }
    return firstEntryId;
}

gctUINT vscBT_NewEntry(VSC_BLOCK_TABLE* pBT)
{
    /* cannot use NewEntry if the table has auto hash attribute, you need to
     * use AddEntry with non-empty data  */
    gcmASSERT(!BT_AUTO_HASH(pBT));
    return vscBT_AddEntry(pBT, gcvNULL);
}

gctUINT vscBT_AddEntry(VSC_BLOCK_TABLE* pBT, void* pData)
{
    gctUINT            i;
    VSC_BT_FREE_ENTRY* pEntry = gcvNULL;

    /* Firstly, try to allocate from free list, always get the header */
    if (pBT->pFreeEntryList)
    {
        gcmASSERT(BT_HAS_FREELIST(pBT));

        if (pBT->pFreeEntryList)
        {
            gctUINT id;
            pEntry = pBT->pFreeEntryList;
            pBT->pFreeEntryList = pEntry->pNextEntry;

            if (pData != gcvNULL)
            {
                memcpy(pEntry, pData, pBT->entrySize);
            }
            else
            {
                memset(pEntry, 0, pBT->entrySize);
            }

            /* To find which block own this entry */
            for (i = 0; i <= pBT->curBlockIdx; i ++)
            {
                gcmASSERT(pBT->ppBlockArray[i]);

                if ((gctUINT8*)pEntry >= pBT->ppBlockArray[i] &&
                    (gctUINT8*)pEntry < pBT->ppBlockArray[i] + pBT->blockSize)
                {
                    break;
                }
            }

            id = BT_MAKE_ENTRY_ID(pBT, i, (gctINT)((gctUINT8*)pEntry - pBT->ppBlockArray[i]));

            if (BT_AUTO_HASH(pBT))
            {
                /* add to hash table */
                vscBT_AddToHash(pBT, id, pData);
            }

            return id;
        }
    }

    /* Secondly, allocate the entry in normal block */
    return _AllocContinuousEntries(pBT, pData, 1);
}

gctUINT vscBT_AddContinuousEntries(VSC_BLOCK_TABLE* pBT, void* pData, gctUINT entryCount)
{
    return _AllocContinuousEntries(pBT, pData, entryCount);
}

void* vscBT_RemoveEntry(VSC_BLOCK_TABLE* pBT, gctUINT entryId)
{
    void*   pRemovedData = BT_GET_ENTRY_DATA(pBT, entryId);

    /* Try to add it to free list */
    if (BT_HAS_FREELIST(pBT))
    {
        ((VSC_BT_FREE_ENTRY*)pRemovedData)->pNextEntry = pBT->pFreeEntryList;
        pBT->pFreeEntryList = (VSC_BT_FREE_ENTRY*)pRemovedData;
    }

    return pRemovedData;
}

void vscBT_AddToHash(VSC_BLOCK_TABLE* pBT, gctUINT entryId, void* pHashKey)
{
    gcmASSERT(BT_HAS_HASHTABLE(pBT));
    gcmASSERT(pBT->pHashTable);

    vscHTBL_DirectSet(pBT->pHashTable, pHashKey, (void*)(gctUINTPTR_T)entryId);
}

gctUINT vscBT_HashSearch(VSC_BLOCK_TABLE* pBT, void* pHashKey)
{
    gctUINT* pHashedEntryId = (gctUINT*)INVALID_BT_ENTRY_ID;

    gcmASSERT(BT_HAS_HASHTABLE(pBT));
    gcmASSERT(pBT->pHashTable);

    if (vscHTBL_DirectTestAndGet(pBT->pHashTable, pHashKey, (void **)&pHashedEntryId))
    {
        return (gctUINT)(gctUINTPTR_T)pHashedEntryId;
    }
    else
    {
        return INVALID_BT_ENTRY_ID;
    }
}

gctUINT vscBT_Find(VSC_BLOCK_TABLE* pBT, void* pHashKey)
{
    gctUINT entryId = vscBT_HashSearch(pBT, pHashKey);

    if (entryId == INVALID_BT_ENTRY_ID)
    {
        entryId =  vscBT_AddEntry(pBT, pHashKey);
        if (!BT_AUTO_HASH(pBT))
        {
            /* add to hash table if it is not automatically added */
            vscBT_AddToHash(pBT, entryId, pHashKey);
        }
    }

    return entryId;
}

gctUINT vscBT_RemoveFromHash(VSC_BLOCK_TABLE* pBT, void* pHashKey)
{
    gcmASSERT(BT_HAS_HASHTABLE(pBT));
    gcmASSERT(pBT->pHashTable);

    return (gctUINT)(gctUINTPTR_T)vscHTBL_DirectRemove(pBT->pHashTable, pHashKey);
}

