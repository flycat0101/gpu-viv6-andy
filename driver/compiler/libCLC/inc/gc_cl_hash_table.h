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


#ifndef __gc_cl_hash_table_h_
#define __gc_cl_hash_table_h_

#include "gc_cl_link_list.h"

typedef enum _cleHASH_TABLE_BUCKET_SIZE
{
    clvHTBS_TINY        = 53,
    clvHTBS_SMALL       = 101,
    clvHTBS_NORMAL      = 211,
    clvHTBS_LARGE       = 401,
    clvHTBS_HUGE        = 1009
}
cleHASH_TABLE_BUCKET_SIZE;

typedef gctUINT        gctHASH_VALUE;

gctHASH_VALUE
clHashString(
    IN gctCONST_STRING String
    );

/* Hash table */
typedef struct _clsHASH_TABLE
{
    slsDLINK_NODE    buckets[clvHTBS_NORMAL];
}
clsHASH_TABLE;

#define clmBUCKET_INDEX(hashValue)        ((gctHASH_VALUE)(hashValue) % clvHTBS_NORMAL)

#define clsHASH_TABLE_Initialize(hashTable) \
    do \
    { \
        gctINT i; \
        for (i = 0; i < clvHTBS_NORMAL; i++) \
        { \
            slsDLINK_LIST_Initialize(&((hashTable)->buckets[i])); \
        } \
    } \
    while (gcvFALSE)

#define clsHASH_TABLE_Bucket(hashTable, index) \
    ((hashTable)->buckets + (index))

#define clsHASH_TABLE_Insert(hashTable, hashValue, node) \
    slsDLINK_LIST_InsertFirst( \
        &((hashTable)->buckets[slmBUCKET_INDEX(hashValue)]), \
        node)

#define clsHASH_TABLE_Detach(hashTable, node) \
    slsDLINK_NODE_Detach(node)

#define FOR_EACH_HASH_BUCKET(hashTable, iter) \
    for ((iter) = (hashTable)->buckets; \
        (iter) < (hashTable)->buckets + clvHTBS_NORMAL; \
        (iter)++)

#endif /* __gc_cl_hash_table_h_ */

