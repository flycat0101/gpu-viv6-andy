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


#include "gc_hal_types.h"
#include "gc_hal.h"

#define SPV_MEMPOOL_PAGESIZE    1024*16

typedef struct _SpvMemPool {

    gctPOINTER ptr;
    gctUINT poolSize;
    gctUINT curPos;
    struct _SpvMemPool *next;

} SpvMemPool;

gceSTATUS spvInitializeMemPool(IN gctUINT memSize, INOUT SpvMemPool **memPool);

gceSTATUS spvUninitializeMemPool(IN SpvMemPool *memPool);

gceSTATUS
spvAllocate(
    IN SpvMemPool *memPool,
    IN gctSIZE_T Bytes,
    OUT gctPOINTER * Memory
    );

gceSTATUS
spvFree(
    IN SpvMemPool *memPool,
    IN gctPOINTER Memory
    );
