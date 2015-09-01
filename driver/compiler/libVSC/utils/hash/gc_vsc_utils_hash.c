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

void vscHND_Initialize(VSC_HASH_NODE* pThisNode, void* pHashKey)
{
    vscULN_Initialize(&pThisNode->uniListNode);
    pThisNode->pHashKey = pHashKey;
}

void vscHND_Finalize(VSC_HASH_NODE* pThisNode)
{
    vscULN_Finalize(&pThisNode->uniListNode);
}

void* vscHND_GetHashKey(VSC_HASH_NODE* pThisNode)
{
    return pThisNode->pHashKey;
}

void vscHNDEXT_Initialize(VSC_HASH_NODE_EXT* pThisExtNode, void* pHashKey, void* pUserData)
{
    vscBSNODE_Initialize(&pThisExtNode->baseNode, pUserData);
    vscHND_Initialize(&pThisExtNode->hashNode, pHashKey);
}

void vscHNDEXT_Finalize(VSC_HASH_NODE_EXT* pThisExtNode)
{
    vscHND_Finalize(&pThisExtNode->hashNode);
}

void* vscHNDEXT_GetHashKey(VSC_HASH_NODE_EXT* pThisExtNode)
{
    return vscHND_GetHashKey(&pThisExtNode->hashNode);
}

void* vscHNDEXT_GetContainedUserData(VSC_HASH_NODE_EXT* pThisExtNode)
{
    return vscBSNODE_GetContainedUserData(&pThisExtNode->baseNode);
}

void vscHNDEXT_SetUserData(VSC_HASH_NODE_EXT* pThisExtNode, void* pUserData)
{
    vscBSNODE_Initialize(&pThisExtNode->baseNode, pUserData);
}

void
vscHTBL_Initialize(
    VSC_HASH_TABLE*   pHT,
    VSC_MM*           pMM,
    PFN_VSC_HASH_FUNC pfnHashFunc,
    PFN_VSC_KEY_CMP   pfnKeyCmp,
    gctINT            tableSize)
{
    gctINT            i;

    if (tableSize <= 0)
    {
        return;
    }

    pHT->pfnHashFunc = pfnHashFunc;
    pHT->pfnKeyCmp   = pfnKeyCmp ? pfnKeyCmp : vscHKCMP_Default;
    pHT->tableSize   = tableSize;
    pHT->pMM         = pMM;
    pHT->pTable = (VSC_HASH_NODE_LIST*)vscMM_Alloc(pMM, tableSize*sizeof(VSC_HASH_NODE_LIST));
    for (i = 0; i < pHT->tableSize; i ++)
    {
        HNLST_INITIALIZE(pHT->pTable + i);
    }

    pHT->itemCount   = 0;
}

VSC_HASH_TABLE *vscHTBL_Create(
    VSC_MM*           pMM,
    PFN_VSC_HASH_FUNC pfnHashFunc,
    PFN_VSC_KEY_CMP   pfnKeyCmp,
    gctINT            tableSize)
{
    VSC_HASH_TABLE*   pHT = gcvNULL;

    if (tableSize <= 0)
    {
        return pHT;
    }

    pHT = (VSC_HASH_TABLE*)vscMM_Alloc(pMM, sizeof(VSC_HASH_TABLE));
    vscHTBL_Initialize(pHT, pMM, pfnHashFunc, pfnKeyCmp, tableSize);

    return pHT;
}

void vscHTBL_Finalize(VSC_HASH_TABLE* pHT)
{
    gctINT i;

    vscHTBL_Reset(pHT);
    pHT->pfnHashFunc = gcvNULL;

    for (i = 0; i < pHT->tableSize; i ++)
    {
        HNLST_FINALIZE(pHT->pTable + i);
    }

    /* Free table list */
    vscMM_Free(pHT->pMM, pHT->pTable);
    pHT->pTable = gcvNULL;
    pHT->tableSize = 0;
    pHT->itemCount = 0;
}

void vscHTBL_Destroy(VSC_HASH_TABLE* pHT)
{
    if (pHT)
    {
        vscHTBL_Finalize(pHT);

        /* Free table itself */
        vscMM_Free(pHT->pMM, pHT);
        pHT = gcvNULL;
    }
}

void vscHTBL_Reset(VSC_HASH_TABLE *pHT)
{
    gctINT             i;
    VSC_HASH_NODE*     pHashNode;

    if (pHT->pMM->mmType == VSC_MM_TYPE_PMP || pHT->pMM->mmType == VSC_MM_TYPE_AMS)
    {
        for (i = 0; i < pHT->tableSize; i ++)
        {
            HNLST_EMPTY(pHT->pTable + i);
        }
    }
    else
    {
        for (i = 0; i < pHT->tableSize; i ++)
        {
            for (pHashNode = HNLST_GET_FIRST_HASH_NODE(pHT->pTable + i);
                 pHashNode != gcvNULL;
                 pHashNode = HNLST_GET_FIRST_HASH_NODE(pHT->pTable + i))
            {
                HNLST_REMOVE_HASH_NODE(pHT->pTable + i, pHashNode);
            }
        }
    }

    pHT->itemCount = 0;
}

static gctINT _CalcHashValue(VSC_HASH_TABLE* pHT, void* pHashKey)
{
    gctUINT            hashVal;

    /* Get hash value by hash func */
    hashVal = pHT->pfnHashFunc(pHashKey);

    /* A safe last-chance to hash cross table entries if user's hash func has not assure
       hash value in range [0, pHT->tableSize) */
    hashVal %= pHT->tableSize;

    return hashVal;
}

gctBOOL vscHTBL_TestAndGet(VSC_HASH_TABLE* pHT, void* pHashKey, VSC_HASH_NODE** ppNode)
{
    gctINT              hashVal;
    VSC_HASH_NODE_LIST* pList;
    VSC_HASH_NODE*      pHashNode;

    /* Get hash value */
    hashVal = _CalcHashValue(pHT, pHashKey);

    /* Find in corresponding list */
    pList = &(pHT->pTable[hashVal]);
    for (pHashNode = HNLST_GET_FIRST_HASH_NODE(pList);
         pHashNode != NULL;
         pHashNode = HND_GET_NEXT_HASH_NODE(pHashNode))
    {
        if (pHT->pfnKeyCmp(vscHND_GetHashKey(pHashNode), pHashKey))
        {
            /* Yes, found it */
            if (ppNode)
            {
                *ppNode = pHashNode;
            }

            return gcvTRUE;
        }
    }

    /* Not found */
    if (ppNode)
    {
        *ppNode = NULL;
    }

    return gcvFALSE;
}

VSC_HASH_NODE* vscHTBL_Get(VSC_HASH_TABLE* pHT, void* pHashKey)
{
    VSC_HASH_NODE* pHashNode;

    vscHTBL_TestAndGet(pHT, pHashKey, &pHashNode);

    return pHashNode;
}

VSC_HASH_NODE* vscHTBL_Set(VSC_HASH_TABLE* pHT, void* pHashKey, VSC_HASH_NODE* pNode)
{
    gctINT              hashVal;
    VSC_HASH_NODE_LIST* pList;
    VSC_HASH_NODE*      pHashNode = gcvNULL;

    gcmASSERT(pHashKey == vscHND_GetHashKey(pNode));

    /* Try to remove hashkey-duplicated one */
    pHashNode = vscHTBL_Remove(pHT, pHashKey);

    /* Get hash value */
    hashVal = _CalcHashValue(pHT, pHashKey);

    /* Find in corresponding list */
    pList = &(pHT->pTable[hashVal]);

    /* Prepend new one */
    HNLST_ADD_HASH_NODE(pList, pNode);

    pHT->itemCount ++;
    return pHashNode;
}

VSC_HASH_NODE* vscHTBL_Remove(VSC_HASH_TABLE* pHT, void* pHashKey)
{
    gctINT              hashVal;
    VSC_HASH_NODE_LIST* pList;
    VSC_HASH_NODE*      pHashNode;

    /* Get hash value */
    hashVal = _CalcHashValue(pHT, pHashKey);

    /* Find in corresponding list */
    pList = &(pHT->pTable[hashVal]);
    for (pHashNode = HNLST_GET_FIRST_HASH_NODE(pList);
         pHashNode != NULL;
         pHashNode = HND_GET_NEXT_HASH_NODE(pHashNode))
    {
        if (pHT->pfnKeyCmp(vscHND_GetHashKey(pHashNode), pHashKey))
        {
            /* Yes, found it, so remove it from list */
            HNLST_REMOVE_HASH_NODE(pList, pHashNode);
            pHT->itemCount --;
            return pHashNode;
        }
    }

    return gcvNULL;
}

gctBOOL vscHTBL_DirectTestAndGet(VSC_HASH_TABLE* pHT, void* pHashKey, void** ppVal)
{
    VSC_HASH_NODE_EXT*  pExtHashNode = gcvNULL;

    if (vscHTBL_TestAndGet(pHT, pHashKey, (VSC_HASH_NODE**)&pExtHashNode))
    {
        if (ppVal)
        {
            *ppVal = vscHNDEXT_GetContainedUserData(pExtHashNode);
        }
        return gcvTRUE;
    }

    return gcvFALSE;
}

void* vscHTBL_DirectGet(VSC_HASH_TABLE* pHT, void* pHashKey)
{
    VSC_HASH_NODE_EXT*  pExtHashNode = CAST_HND_2_HEXTND(vscHTBL_Get(pHT, pHashKey));

    if (pExtHashNode)
    {
        return vscHNDEXT_GetContainedUserData(pExtHashNode);
    }

    return gcvNULL;
}

void vscHTBL_DirectSet(VSC_HASH_TABLE* pHT, void* pHashKey, void* pVal)
{
    VSC_HASH_NODE_EXT*     pExtHashNode = gcvNULL;
    gctBOOL                bHit;

    bHit = vscHTBL_TestAndGet(pHT, pHashKey, (VSC_HASH_NODE**)&pExtHashNode);
    if (!bHit)
    {
        /* Allocate a new hash ext-node */
        pExtHashNode = (VSC_HASH_NODE_EXT*)vscMM_Alloc(pHT->pMM, sizeof(VSC_HASH_NODE_EXT));

        /* Initialize member of new hash ext-node */
        vscHNDEXT_Initialize(pExtHashNode, pHashKey, pVal);

        /* Add this new hash ext-node to hash entry */
        vscHTBL_Set(pHT, pHashKey, CAST_HEXTND_2_HND(pExtHashNode));
    }
    else
    {
        /* Set new user data of old hash ext-node */
        vscHNDEXT_SetUserData(pExtHashNode, pVal);
    }
}

void* vscHTBL_DirectRemove(VSC_HASH_TABLE* pHT, void* pHashKey)
{
    VSC_HASH_NODE_EXT*  pExtHashNode = CAST_HND_2_HEXTND(vscHTBL_Remove(pHT, pHashKey));
    void*               pRetVal;

    if (pExtHashNode)
    {
        pRetVal = vscHNDEXT_GetContainedUserData(pExtHashNode);

        vscHNDEXT_Finalize(pExtHashNode);
        vscMM_Free(pHT->pMM, pExtHashNode);

        return pRetVal;
    }

    return gcvNULL;
}

void vscHTBL_DirectDuplicate(VSC_HASH_TABLE* pDstHT, VSC_HASH_TABLE* pSrcHT)
{
    gcmASSERT(pSrcHT && pDstHT);

    if (pDstHT->tableSize > 0)
    {
        vscHTBL_Reset(pDstHT);
    }

    if (pSrcHT->tableSize > 0)
    {
        VSC_HASH_NODE* pSrcHashNode;
        VSC_HASH_ITERATOR iter;
        vscHTBLIterator_Init(&iter, pSrcHT);
        for (pSrcHashNode = vscHTBLIterator_First(&iter); pSrcHashNode != gcvNULL; pSrcHashNode = vscHTBLIterator_Next(&iter))
        {
            void* pVal = vscHTBL_DirectGet(pSrcHT, pSrcHashNode->pHashKey);
            vscHTBL_DirectSet(pDstHT, pSrcHashNode->pHashKey, pVal);
        }
    }
}

void vscHTBLIterator_Init(VSC_HASH_ITERATOR* pIter, VSC_HASH_TABLE* pHtbl)
{
    gcmASSERT(pIter && pHtbl);
    gcmASSERT(pHtbl->tableSize > 0);

    pIter->pHashTable = pHtbl;
    vscULIterator_Init(&pIter->htblEntryIterator, &pHtbl->pTable[0]);
    pIter->curEntryIdx = 0;
}

VSC_HASH_NODE *vscHTBLIterator_First(VSC_HASH_ITERATOR* pIter)
{
    VSC_HASH_NODE* pRetHashNode = gcvNULL;

    while (gcvTRUE)
    {
        pRetHashNode = CAST_ULN_2_HND(vscULIterator_First(&pIter->htblEntryIterator));

        if (pRetHashNode)
        {
            return pRetHashNode;
        }
        else
        {
            if (pIter->curEntryIdx == pIter->pHashTable->tableSize - 1)
            {
                return gcvNULL;
            }
            else
            {
                vscULIterator_Init(&pIter->htblEntryIterator, &pIter->pHashTable->pTable[++ pIter->curEntryIdx]);
            }
        }
    };

    return gcvNULL;
}

VSC_HASH_NODE *vscHTBLIterator_Next(VSC_HASH_ITERATOR* pIter)
{
    VSC_HASH_NODE* pRetHashNode = gcvNULL;

    pRetHashNode = CAST_ULN_2_HND(vscULIterator_Next(&pIter->htblEntryIterator));

    if (pRetHashNode)
    {
        return pRetHashNode;
    }
    else
    {
        if (pIter->curEntryIdx == pIter->pHashTable->tableSize - 1)
        {
            return gcvNULL;
        }
        else
        {
            vscULIterator_Init(&pIter->htblEntryIterator, &pIter->pHashTable->pTable[++ pIter->curEntryIdx]);
            return vscHTBLIterator_First(pIter);
        }
    }
}

VSC_HASH_NODE *vscHTBLIterator_Last(VSC_HASH_ITERATOR* pIter)
{
    gctINT         i;
    VSC_HASH_NODE* pRetHashNode = gcvNULL;

    for (i = pIter->pHashTable->tableSize - 1; i >= 0; i --)
    {
        vscULIterator_Init(&pIter->htblEntryIterator, &pIter->pHashTable->pTable[i]);
        pRetHashNode = CAST_ULN_2_HND(vscULIterator_Last(&pIter->htblEntryIterator));
        if (pRetHashNode)
        {
            return pRetHashNode;
        }
    }

    return gcvNULL;
}

VSC_DIRECT_HNODE_PAIR vscHTBLIterator_DirectFirst(VSC_HASH_ITERATOR * pIter)
{
    VSC_HASH_NODE_EXT*    pExtHashNode = CAST_HND_2_HEXTND(vscHTBLIterator_First(pIter));
    VSC_DIRECT_HNODE_PAIR dhnPair = {gcvNULL, gcvNULL};

    if (pExtHashNode)
    {
        dhnPair.pHashKey = pExtHashNode->hashNode.pHashKey;
        dhnPair.pValue   = vscHNDEXT_GetContainedUserData(pExtHashNode);
    }

    return dhnPair;
}

VSC_DIRECT_HNODE_PAIR vscHTBLIterator_DirectNext(VSC_HASH_ITERATOR * pIter)
{
    VSC_HASH_NODE_EXT*    pExtHashNode = CAST_HND_2_HEXTND(vscHTBLIterator_Next(pIter));
    VSC_DIRECT_HNODE_PAIR dhnPair = {gcvNULL, gcvNULL};

    if (pExtHashNode)
    {
        dhnPair.pHashKey = pExtHashNode->hashNode.pHashKey;
        dhnPair.pValue   = vscHNDEXT_GetContainedUserData(pExtHashNode);
    }

    return dhnPair;
}

VSC_DIRECT_HNODE_PAIR vscHTBLIterator_DirectLast(VSC_HASH_ITERATOR * pIter)
{
    VSC_HASH_NODE_EXT*    pExtHashNode = CAST_HND_2_HEXTND(vscHTBLIterator_Last(pIter));
    VSC_DIRECT_HNODE_PAIR dhnPair = {gcvNULL, gcvNULL};

    if (pExtHashNode)
    {
        dhnPair.pHashKey = pExtHashNode->hashNode.pHashKey;
        dhnPair.pValue   = vscHNDEXT_GetContainedUserData(pExtHashNode);
    }

    return dhnPair;
}

/* JS Hash */
gctUINT
vscHFUNC_Default(const void* ptr)
{
    return (gctUINT)(gctUINTPTR_T)ptr & 0xff;
}

gctUINT
vscHFUNC_String(const void *Str)
{
    const char *str = (const char *)Str;
    gctUINT32 hashVal = 1315423911;
    while (*str)
    {
        hashVal ^= ((hashVal << 5) + (*str++) + (hashVal >> 2));
    }
    return (hashVal & 0x7FFFFFFF);
}

gctBOOL vscHKCMP_Default(const void* pHashKey1, const void* pHashKey2)
{
    return (pHashKey1 == pHashKey2);
}

gctBOOL vcsHKCMP_String(const void* Str1, const void* Str2)
{
    return (strcmp((const char*)Str1, (const char*)Str2) == 0);
}


