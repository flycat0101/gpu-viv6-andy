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


#ifndef __gc_vsc_utils_table_h_
#define __gc_vsc_utils_table_h_

#include "gc_vsc_utils_mm.h"

/*
   Tables defined here are all based on other basic data structures, so hash table
   is not put here. The biggest purpose of these tables are for symbol table. You
   can use these tables if you think they meet your requirements.
*/

BEGIN_EXTERN_C()

/* Block table tries to provide simply mapping between table entry and its ID within
   table. The table entry ID is the virtual offset from the begin of virtual block.
   The entries can be hashed in hash table if you want. Its layout is as below:
 *
 *  +-----------+                blcok[0] +---------------------------~~~ --+
 *  |         --+------------------------>+---------------------------~~~ --+
 *  |           |
 *  +-----------+                blcok[1] +---------------------------~~~ --+
 *  |           +------------------------>+---------------------------~~~ --+
 *  |           |
 *  +-----------+                blcok[2] +---------------------------~~~ --+
 *  |           +------------------------>+---------------------------~~~ --+
 *  +-----------+
 *  ~           ~
 *  +-----------+                blcok[n] +---------------------------~~~ --+
 *  |           +------------------------>+---------------------------~~~ --+
 *  +-----------+
 *
 */

 /* the first 2 MSB bits of id is scope info, other 30 bits are for id index */
#define INVALID_BT_ENTRY_ID     0x3FFFFFFF
#define VALID_BT_ENTRY_ID_MASK  0x3FFFFFFF
#define BT_ENTRY_ID_FUNC_SCOPE  0x40000000

#define BT_ENTRY_ID_SCOPE_MASK  0xC0000000
#define BT_ENTRY_VALID_BITS     30

#define BT_MAKE_ENTRY_ID(pBT, blockId, offset) ((blockId)*(pBT)->entryCountPerBlock + (offset)/(pBT)->entrySize)
#define BT_GET_BLOCK_INDEX(pBT, entryId)       ((entryId)/(pBT)->entryCountPerBlock)
#define BT_GET_BLOCK_OFFSET(pBT, entryId)      (((entryId)%(pBT)->entryCountPerBlock)*(pBT)->entrySize)
#define BT_GET_ENTRY_DATA(pBT, entryId)        ((pBT)->ppBlockArray[BT_GET_BLOCK_INDEX((pBT), (entryId))] + \
                                               BT_GET_BLOCK_OFFSET((pBT), (entryId)))
#define BT_GET_MAX_VALID_ID(pBT)               BT_MAKE_ENTRY_ID((pBT), (pBT)->curBlockIdx, (pBT)->nextOffsetInCurBlock)

#define BT_HAS_HASHTABLE(pBT)                  (((pBT)->flag & VSC_BLOCK_TABLE_FLAG_HASH_ENTRIES) != 0)
#define BT_AUTO_HASH(pBT)                      (((pBT)->flag & VSC_BLOCK_TABLE_FLAG_AUTO_HASH) != 0)
#define BT_HAS_FREELIST(pBT)                   (((pBT)->flag & VSC_BLOCK_TABLE_FLAG_FREE_ENTRY_LIST) != 0)
#define BT_IS_FUNCTION_SCOPE(pBT)              (((pBT)->flag & VSC_BLOCK_TABLE_FLAG_FUNCTION_SCOPE) != 0)
#define BT_FREELIST_USE_PTR(pBT)               (((pBT)->flag & VSC_BLOCK_TABLE_FLAG_FREE_ENTRY_USE_PTR) != 0)

typedef enum _VSC_BLOCK_TABLE_FLAG
{
    VSC_BLOCK_TABLE_FLAG_FREE_ENTRY_LIST     = 0x01,
    VSC_BLOCK_TABLE_FLAG_HASH_ENTRIES        = 0x02,
    VSC_BLOCK_TABLE_FLAG_AUTO_HASH           = 0x04,
    VSC_BLOCK_TABLE_FLAG_FUNCTION_SCOPE      = 0x08,
    VSC_BLOCK_TABLE_FLAG_FREE_ENTRY_USE_PTR  = 0x10,
} VSC_BLOCK_TABLE_FLAG;

typedef gctUINT8* VSC_BT_BLOCK_PTR;

typedef union _VSC_BT_FREE_ENTRY
{
    gctUINT            nextFreeEntryId;     /* Next free entry id */
    void *             nextFreeEntryPtr;    /* ptr to next free entry */
}VSC_BT_FREE_ENTRY;

/* Get free-entry from entry, it might be located anywhere inside of entry based on
   user's designation in this function. This callback is introduced because some of
   users might need re-access some of data in free'ed entry, and user hopes use this
   function to ask BT not to touch any data outside of VSC_BT_FREE_ENTRY area. But
   it is unelegant (BT-inside free list management is exposed to user) and unsafe (
   the erea that user hopes to be untouched can be still be modified by BT accidently
   or dynamically)!!!! We should remove this later!!! */
typedef VSC_BT_FREE_ENTRY* (*PFN_VSC_GET_FREE_ENTRY)(void* pEntry);

typedef struct _VSC_BLOCK_TABLE
{
    VSC_BLOCK_TABLE_FLAG      flag;

    /* To hash entries of table */
    VSC_HASH_TABLE*           pHashTable;

    /* The size of entry in the block, in bytes */
    gctUINT                   entrySize;

    /* The size of each block in power of 2 */
    gctUINT                   blockSize;

    /* It is equal to blockSize/entrySize */
    gctUINT                   entryCountPerBlock;

    /* Block array that this table maintains */
    gctUINT                   blockCount;
    VSC_BT_BLOCK_PTR*         ppBlockArray;

    /* Current block index that new entry can be allocated */
    gctUINT                   curBlockIdx;

    /* The available offset in the cur block */
    gctUINT                   nextOffsetInCurBlock;

    /* Re-usable freed entries, it has higher priority than current block when allocating new
       entry. Note that it is only used when VSC_BLOCK_TABLE_FLAG_FIXED_ENTRY_SIZE is set. Also
       note that only entry whose entrySize is GE pointer size (normally 4 bytes) can be put
       in free list */
    VSC_BT_FREE_ENTRY         firstFreeEntry;

    /* User specified get-free-entry callback */
    PFN_VSC_GET_FREE_ENTRY    pfnGetFreeEntry;

    /* What type of MM are this block table built on? */
    VSC_MM*                   pMM;
} VSC_BLOCK_TABLE;

/* Creation and destroy */
VSC_BLOCK_TABLE* vscBT_Create(
    VSC_MM*                 pMM,
    gctUINT                 flag,
    gctUINT                 entrySize,
    gctUINT                 blockSize,
    gctUINT                 initBlockCount,
    PFN_VSC_GET_FREE_ENTRY  pfnGetFreeEntry,
    PFN_VSC_HASH_FUNC       pfnHashFunc,
    PFN_VSC_KEY_CMP         pfnKeyCmp,
    gctINT                  hashTableSize);

void vscBT_Initialize(
    VSC_BLOCK_TABLE*        pBT,
    VSC_MM*                 pMM,
    gctUINT                 flag,
    gctUINT                 entrySize,
    gctUINT                 blockSize,
    gctUINT                 initBlockCount,
    PFN_VSC_GET_FREE_ENTRY  pfnGetFreeEntry,
    PFN_VSC_HASH_FUNC       pfnHashFunc,
    PFN_VSC_KEY_CMP         pfnKeyCmp,
    gctINT                  hashTableSize);

void vscBT_Finalize(VSC_BLOCK_TABLE* pBT);
void vscBT_Destroy(VSC_BLOCK_TABLE* pBT);

/* Operations */

/* Get a new empty entry (initialize whole enty to 0) in block table and return
   the id of the entry */
gctUINT vscBT_NewEntry(VSC_BLOCK_TABLE* pBT);
/* return a new entry pointer the table */
void * vscBT_NewEntryPtr(VSC_BLOCK_TABLE* pBT);

/* Add pData to the block table and return the id of the entry, if pData is empty,
   same as newEntry */
gctUINT vscBT_AddEntry(VSC_BLOCK_TABLE* pBT, void* pData);

gctUINT vscBT_AddContinuousEntries(VSC_BLOCK_TABLE* pBT, void* pData, gctUINT entryCount);
void* vscBT_RemoveEntry(VSC_BLOCK_TABLE* pBT, gctUINT entryId);
void vscBT_RemoveEntryPtr(VSC_BLOCK_TABLE* pBT, void * entryPtr);
void vscBT_AddToHash(VSC_BLOCK_TABLE* pBT, gctUINT entryId, void* pHashKey);
gctUINT vscBT_HashSearch(VSC_BLOCK_TABLE* pBT, void* pHashKey);
gctUINT vscBT_RemoveFromHash(VSC_BLOCK_TABLE* pBT, void* pHashKey);
VSC_ErrCode vscBT_ResizeBlockArray(VSC_BLOCK_TABLE* pBT, gctUINT newBlockCount, gctBOOL bPreAllocBlock);

/* Try to find an entry based on key, if found, just return, otherwise, add a new
   entry for the key */
gctUINT vscBT_Find(VSC_BLOCK_TABLE* pBT, void* pHashKey);

/* Clone a BT */
VSC_ErrCode vscBT_Copy(VSC_BLOCK_TABLE* pDstBT, VSC_BLOCK_TABLE* pSrcBT);

gctUINT vscBT_GetUsedSize(VSC_BLOCK_TABLE* pBT);

END_EXTERN_C()

#endif /* __gc_vsc_utils_table_h_ */

