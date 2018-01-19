/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_spirv_mempool.h"

#define FOR_EACH_LINK_NODE(mempool) \
    for (; mempool != gcvNULL; mempool = mempool->next)

#define SPV_MEMPOOL_Initialize(mempool) \
    { \
        (mempool)->ptr = gcvNULL; \
        (mempool)->poolSize = 0; \
        (mempool)->curPos = 0; \
        (mempool)->next = gcvNULL; \
    }

static gceSTATUS __FreeSpvMemPool(SpvMemPool * memPool)
{
    gceSTATUS status = gcvSTATUS_OK;
    SpvMemPool *oldPtr = gcvNULL;

    while(memPool != gcvNULL)
    {
        oldPtr = memPool;
        memPool = memPool->next;

        if (oldPtr->ptr != gcvNULL)
        {
            gcoOS_Free(gcvNULL, oldPtr->ptr);
            oldPtr->ptr = gcvNULL;
        }

        gcoOS_Free(gcvNULL, oldPtr);
    }

    return status;
}

gceSTATUS spvInitializeMemPool(IN gctUINT memSize, INOUT SpvMemPool **memPool)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctPOINTER pointer = gcvNULL;

    if (*memPool != gcvNULL)
    {
        status = __FreeSpvMemPool(*memPool);
    }
    if (gcmIS_ERROR(status)) return status;

    /* Allocate start pointer of memory pool */
    status = gcoOS_Allocate(gcvNULL, gcmSIZEOF(struct _SpvMemPool), &pointer);
    if (gcmIS_ERROR(status)) return status;

    *memPool = (SpvMemPool *)pointer;

    SPV_MEMPOOL_Initialize(*memPool);

    /* Allocate first pool */
    status = gcoOS_Allocate(gcvNULL, memSize, &pointer);
    if (gcmIS_ERROR(status)) return status;

    (*memPool)->ptr = pointer;
    (*memPool)->poolSize = memSize;
    (*memPool)->curPos = 0;
    (*memPool)->next = gcvNULL;

    return status;
}

gceSTATUS spvUninitializeMemPool(IN SpvMemPool *memPool)
{
    gceSTATUS status = gcvSTATUS_OK;

    if (memPool != gcvNULL)
    {
        status = __FreeSpvMemPool(memPool);
    }

    return status;
}


gceSTATUS
spvAllocate(
    IN SpvMemPool *memPool,
    IN gctSIZE_T Bytes,
    OUT gctPOINTER * Memory
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gctBOOL findPool = gcvFALSE;
    gctPOINTER pointer = gcvNULL;
    SpvMemPool * newMemPool = gcvNULL;
    SpvMemPool * lastNode = gcvNULL;

    gcmASSERT(memPool);

    Bytes = gcmALIGN(Bytes, 8);

    FOR_EACH_LINK_NODE(memPool)
    {
        if ((memPool->ptr != gcvNULL ) &&
            (memPool->curPos + Bytes < memPool->poolSize))
        {
            pointer = (gctCHAR *)memPool->ptr + memPool->curPos;
            memPool->curPos += (gctUINT)Bytes;
            findPool = gcvTRUE;
            break;
        }

        lastNode = memPool;
    }

    /* not find usable pool, create one */
    if (findPool == gcvFALSE)
    {
        spvInitializeMemPool((Bytes > SPV_MEMPOOL_PAGESIZE) ? (gctUINT)Bytes : SPV_MEMPOOL_PAGESIZE, &newMemPool);
        lastNode->next = newMemPool;

        pointer = newMemPool->ptr;
        newMemPool->curPos += (gctUINT)Bytes;
    }

    if (Memory != gcvNULL)
    {
        *Memory = pointer;
    }

    return status;
}

gceSTATUS
spvFree(
    IN SpvMemPool *memPool,
    IN gctPOINTER Memory
    )
{
    return gcvSTATUS_OK;
}
