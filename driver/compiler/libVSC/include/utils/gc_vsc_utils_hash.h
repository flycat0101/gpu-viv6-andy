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


#ifndef __gc_vsc_utils_hash_h_
#define __gc_vsc_utils_hash_h_

BEGIN_EXTERN_C()

typedef gctUINT (*PFN_VSC_HASH_FUNC)(const void* pHashKey);
typedef gctBOOL (*PFN_VSC_KEY_CMP)(const void* pHashKey1, const void* pHashKey2);

/* Hash node */
typedef struct _VSC_HASH_NODE
{
    /* Next hash node at same hash point. It must be put at FIRST place!!!! */
    VSC_UNI_LIST_NODE         uniListNode;

    /* Hash key for this node */
    void*                     pHashKey;
}VSC_HASH_NODE;

void vscHND_Initialize(VSC_HASH_NODE* pThisNode, void* pHashKey);
void vscHND_Finalize(VSC_HASH_NODE* pThisNode);
void* vscHND_GetHashKey(VSC_HASH_NODE* pThisNode);
#define CAST_HND_2_ULN(pHnd)                  (VSC_UNI_LIST_NODE*)(pHnd)
#define CAST_ULN_2_HND(pUln)                  (VSC_HASH_NODE*)(pUln)
#define HND_GET_NEXT_HASH_NODE(pThisHnd)      CAST_ULN_2_HND(vscULN_GetNextNode(CAST_HND_2_ULN(pThisHnd)))

/* Hash ext-node */
typedef struct _VSC_HASH_NODE_EXT
{
    /* !!!!VSC_HASH_NODE must be put at FIRST place */
    VSC_HASH_NODE             hashNode;
    VSC_BASE_NODE             baseNode;
}VSC_HASH_NODE_EXT;

void vscHNDEXT_Initialize(VSC_HASH_NODE_EXT* pThisExtNode, void* pHashKey, void* pUserData);
void vscHNDEXT_Finalize(VSC_HASH_NODE_EXT* pThisExtNode);
void* vscHNDEXT_GetHashKey(VSC_HASH_NODE_EXT* pThisExtNode);
void* vscHNDEXT_GetContainedUserData(VSC_HASH_NODE_EXT* pThisExtNode);
void vscHNDEXT_SetUserData(VSC_HASH_NODE_EXT* pThisExtNode, void* pUserData);
#define CAST_HEXTND_2_HND(pHExtNd)    (VSC_HASH_NODE*)(pHExtNd)
#define CAST_HND_2_HEXTND(pHnd)       (VSC_HASH_NODE_EXT*)(pHnd)

/* Hash node list */
typedef VSC_UNI_LIST VSC_HASH_NODE_LIST;
#define HNLST_INITIALIZE(pHNList)              vscUNILST_Initialize((pHNList), gcvFALSE)
#define HNLST_FINALIZE(pHNList)                vscUNILST_Finalize((pHNList))
#define HNLST_EMPTY(pHNList)                   vscUNILST_Reset((pHNList))
#define HNLST_ADD_HASH_NODE(pHNList, pHnd)     vscUNILST_Prepend((pHNList), CAST_HND_2_ULN((pHnd)))
#define HNLST_GET_FIRST_HASH_NODE(pHNList)     CAST_ULN_2_HND(vscUNILST_GetHead((pHNList)))
#define HNLST_REMOVE_HASH_NODE(pHNList, pHnd)  vscUNILST_Remove((pHNList), CAST_HND_2_ULN((pHnd)))

/* Hash search */
typedef struct _SEARCH_TIME
{
    gctINT*                   searchTimesArray;
    gctINT                    searchTotal;
    gctINT                    searchSucceed;
    gctINT                    searchFailed;
    gctINT                    searchMostTimes;
    gctINT                    searchMostCount;
    gctINT                    maxSearchTimes;
}SEARCH_TIME;

/* Hash table */
typedef struct _VSC_HASH_TABLE
{
    PFN_VSC_HASH_FUNC         pfnHashFunc;
    PFN_VSC_KEY_CMP           pfnKeyCmp;
    VSC_HASH_NODE_LIST*       pTable;
    gctINT                    tableSize;
    gctUINT                   itemCount;
    SEARCH_TIME*              searchTime;

    /* What type of MM are this hash table built on? */
    VSC_MM*                   pMM;
}VSC_HASH_TABLE;

#define HTBL_MAX_SEARCH_TIMES(pHTAB)        (pHTAB->searchTime->maxSearchTimes)
#define HTBL_GET_TABLE_SIZE(pHTAB)          ((pHTAB) ? (pHTAB)->tableSize : 0)
#define HTBL_GET_ITEM_COUNT(pHTAB)          ((pHTAB) ? (pHTAB)->itemCount : 0)

/* Creation and destroy */
VSC_HASH_TABLE *vscHTBL_Create(VSC_MM* pMM, PFN_VSC_HASH_FUNC pfnHashFunc,
                               PFN_VSC_KEY_CMP pfnKeyCmp, gctINT tableSize);
VSC_ErrCode vscHTBL_Initialize(VSC_HASH_TABLE* pHT, VSC_MM* pMM, PFN_VSC_HASH_FUNC pfnHashFunc,
                        PFN_VSC_KEY_CMP pfnKeyCmp, gctINT tableSize);
void vscHTBL_Destroy(VSC_HASH_TABLE* pHT);
void vscHTBL_Finalize(VSC_HASH_TABLE* pHT);
VSC_ErrCode vscHTBL_CreateOrInitialize(
    IN     VSC_MM*           pMM,
    IN OUT VSC_HASH_TABLE ** ppHT,
    IN PFN_VSC_HASH_FUNC     pfnHashFunc,
    IN PFN_VSC_KEY_CMP       pfnKeyCmp,
    IN gctINT                tableSize
    );

/* Make table empty */
void vscHTBL_Reset(VSC_HASH_TABLE *pHT);

/* Hash node operations for common cases. Note that these functions also for case of container, but user
   needs maintain VSC_HASH_NODE_EXT and cast it to VSC_HASH_NODE before using following functions. */
gctBOOL vscHTBL_TestAndGet(VSC_HASH_TABLE* pHT, void* pHashKey, VSC_HASH_NODE** ppNode);
VSC_HASH_NODE* vscHTBL_Get(VSC_HASH_TABLE* pHT, void* pHashKey);
VSC_HASH_NODE* vscHTBL_Set(VSC_HASH_TABLE* pHT, void* pHashKey, VSC_HASH_NODE* pNode);
VSC_HASH_NODE* vscHTBL_Remove(VSC_HASH_TABLE* pHT, void* pHashKey);
gctINT vscHTBL_CountItems(VSC_HASH_TABLE* pHT);

/* Hash node operations exclusively for special case of container that user does not need maintain VSC_HASH_NODE_EXT */
gctBOOL vscHTBL_DirectTestAndGet(VSC_HASH_TABLE* pHT, void* pHashKey, void** ppVal);
void* vscHTBL_DirectGet(VSC_HASH_TABLE* pHT, void* pHashKey);
VSC_ErrCode vscHTBL_DirectSet(VSC_HASH_TABLE* pHT, void* pHashKey, void* pVal);
void* vscHTBL_DirectRemove(VSC_HASH_TABLE* pHT, void* pHashKey);
VSC_ErrCode vscHTBL_DirectDuplicate(VSC_HASH_TABLE* pDstHT, VSC_HASH_TABLE* pSrcHT);

/* Iterator to iterate all hash node */
typedef struct _VSC_HASH_ITERATOR
{
    VSC_HASH_TABLE*      pHashTable;
    VSC_UL_ITERATOR      htblEntryIterator;
    gctINT               curEntryIdx;
    gctINT               count;       /* the entries iterated */
} VSC_HASH_ITERATOR;

void vscHTBLIterator_Init(VSC_HASH_ITERATOR *, VSC_HASH_TABLE *);
VSC_HASH_NODE *vscHTBLIterator_First(VSC_HASH_ITERATOR *);
VSC_HASH_NODE *vscHTBLIterator_Next(VSC_HASH_ITERATOR *);
VSC_HASH_NODE *vscHTBLIterator_Last(VSC_HASH_ITERATOR *);

/* Direct-version of iterator */
typedef struct _VSC_DIRECT_HNODE_PAIR
{
    void*                     pHashKey;
    void*                     pValue;
}VSC_DIRECT_HNODE_PAIR;

VSC_DIRECT_HNODE_PAIR vscHTBLIterator_DirectFirst(VSC_HASH_ITERATOR *);
VSC_DIRECT_HNODE_PAIR vscHTBLIterator_DirectNext(VSC_HASH_ITERATOR *);
VSC_DIRECT_HNODE_PAIR vscHTBLIterator_DirectLast(VSC_HASH_ITERATOR *);

#define VSC_DIRECT_HNODE_PAIR_FIRST(pDhnPair)       ((pDhnPair)->pHashKey)
#define VSC_DIRECT_HNODE_PAIR_SECOND(pDhnPair)      ((pDhnPair)->pValue)
#define IS_VALID_DIRECT_HNODE_PAIR(pDhnPair)        ((pDhnPair)->pHashKey != gcvNULL)

/* Common hash functions are defined here */
gctUINT vscHFUNC_Default(const void *);
gctUINT vscHFUNC_DefaultPtr(const void *);
gctUINT vscHFUNC_String(const void *); /* For string */

/* Common hash key compare functions are defined here */
gctBOOL vscHKCMP_Default(const void* pHashKey1, const void* pHashKey2); /* Default key-comparator if user does not provide */
gctBOOL vcsHKCMP_String(const void *, const void *);


END_EXTERN_C()

#endif /* __gc_vsc_utils_hash_h_ */

