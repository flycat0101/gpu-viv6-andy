/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_glsl_hash_table_h_
#define __gc_glsl_hash_table_h_

#include "gc_glsl_link_list.h"

typedef enum _sleHASH_TABLE_BUCKET_SIZE
{
    slvHTBS_TINY        = 53,
    slvHTBS_SMALL       = 101,
    slvHTBS_NORMAL      = 211,
    slvHTBS_LARGE       = 401,
    slvHTBS_HUGE        = 1009
}
sleHASH_TABLE_BUCKET_SIZE;

typedef gctUINT        gctHASH_VALUE;

gctHASH_VALUE
slHashString(
    IN gctCONST_STRING String
    );

/* Hash table */
typedef struct _slsHASH_TABLE
{
    slsDLINK_NODE    buckets[slvHTBS_NORMAL];
}
slsHASH_TABLE;

#define slmBUCKET_INDEX(hashValue)        ((gctHASH_VALUE)(hashValue) % slvHTBS_NORMAL)

#define slsHASH_TABLE_Initialize(hashTable) \
    do \
    { \
        gctINT i; \
        for (i = 0; i < slvHTBS_NORMAL; i++) \
        { \
            slsDLINK_LIST_Initialize(&((hashTable)->buckets[i])); \
        } \
    } \
    while (gcvFALSE)

#define slsHASH_TABLE_Bucket(hashTable, index) \
    ((hashTable)->buckets + (index))

#define slsHASH_TABLE_Insert(hashTable, hashValue, node) \
    slsDLINK_LIST_InsertFirst( \
        &((hashTable)->buckets[slmBUCKET_INDEX(hashValue)]), \
        node)

#define slsHASH_TABLE_Detach(hashTable, node) \
    slsDLINK_NODE_Detach(node)

#define FOR_EACH_HASH_BUCKET(hashTable, iter) \
    for ((iter) = (hashTable)->buckets; \
        (iter) < (hashTable)->buckets + slvHTBS_NORMAL; \
        (iter)++)

/* New Hash table implementation. */
/* Hash table function. */
typedef gctUINT (*slsHASH_FUNC)(const void* pHashKey);
typedef gctBOOL (*slsKEY_CMP_FUNC)(const void* pHashKey1, const void* pHashKey2);

/* Hash table buck list. */
typedef slsDLINK_NODE slsHASH_NODE_LIST;

typedef struct _slsHASH_NODE
{
    slsDLINK_NODE               node;
    void*                       hashKeyValue;
}slsHASH_NODE;

typedef struct _slsBASE_NODE
{
    /* User data that this base node contains when EXT_NODE acts as a container. Two concepts of this
       container (suppose A is EXT_NODE, and B is derived from A):
       1. If A is the first member of B, then user data can store any user side data or NULL because
          A and B can be simply casted each other.
       2. Otherwise, if A is NOT the first member of B, the user data must be used to store pointer of
          B for the purpose of that A can be easily converted to B, so it can not store other special
          user data any more */
    void*                       userData;
}slsBASE_NODE;

typedef struct _slsHASH_NODE_EXT
{
    slsHASH_NODE                hashNode;
    slsBASE_NODE                baseNode;
}slsHASH_NODE_EXT;

/* Hash table */
struct _slsHASH_TABLE_EX
{
    slsHASH_FUNC                slsHashFunc;
    slsKEY_CMP_FUNC             slsKeyCmpFunc;
    slsHASH_NODE_LIST*          slsHashTable;
    gctINT                      slsTableSize;
    gctINT                      slsItemCount;
};

typedef struct _slsHASH_TABLE_EX * slsHASH_TABLE_EX;

gceSTATUS slsHTBL_Initialize(
    IN OUT slsHASH_TABLE_EX HashTable,
    IN slsHASH_FUNC         HashFunc,
    IN slsKEY_CMP_FUNC      KeyCmpFunc,
    IN gctINT               TableSize
    );

gceSTATUS slsHTBL_Finalize(
    IN OUT slsHASH_TABLE_EX HashTable
    );

gceSTATUS slsHTBL_Create(
    IN slsHASH_FUNC         HashFunc,
    IN slsKEY_CMP_FUNC      KeyCmpFunc,
    IN gctINT               TableSize,
    IN OUT slsHASH_TABLE_EX*HashTable
    );

gceSTATUS slsHTBL_Destroy(
    IN OUT slsHASH_TABLE_EX HashTable
    );

gctBOOL slsHTBL_Get(
    IN OUT slsHASH_TABLE_EX HashTable,
    IN void*                HashKey,
    IN slsHASH_NODE**       HashNode
    );

gceSTATUS slsHTBL_Set(
    IN OUT slsHASH_TABLE_EX HashTable,
    IN void*                HashKey,
    IN void*                HashKeyValue
    );

#endif /* __gc_glsl_hash_table_h_ */

