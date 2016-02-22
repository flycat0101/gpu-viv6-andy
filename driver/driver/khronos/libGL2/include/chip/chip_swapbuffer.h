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


#ifndef __chip_swapbuffer_h_
#define __chip_swapbuffer_h_

#include "gc_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

struct glsBackBuffer
{
    gctPOINTER                  context;
    gcoSURF                     surface;
    gctUINT32                   offset;
    gcsPOINT                    origin;
    gcsSIZE                     size;
} ;

struct glsWorkerInfo
{
    gctSIGNAL                  signal;
    gctSIGNAL                  targetSignal;

    /* Owner of this surface. */
    GLvoid *                   draw;
    GLvoid *                   bits;

    struct glsBackBuffer       backBuffer;

    /* Used by eglDisplay worker list. */
    struct glsWorkerInfo *    prev;
    struct glsWorkerInfo *    next;
} ;

typedef struct glsWorkerInfo    *glsWORKINFO_PTR;
typedef struct glsBackBuffer    *glsBACKBUFFER_PTR;

#define WORKER_COUNT        4

#ifdef __cplusplus
}
#endif

#endif /* __chip_swapbuffer_h_ */

