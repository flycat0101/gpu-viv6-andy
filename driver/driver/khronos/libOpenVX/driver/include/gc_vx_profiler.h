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


#ifndef __GC_VX_PROFILER_H__
#define __GC_VX_PROFILER_H__

#include "gc_hal_types.h"

/* Profile information. */
typedef struct _vx_profiler
{
    gctBOOL         enable;
    gctBOOL         perVxfinish;

    gctUINT32       frameNumber;
    gctUINT32       frameMaxNum;
    gctUINT64       frameStartTimeusec;
    gctUINT64       frameEndTimeusec;
    gctUINT64       frameStartCPUTimeusec;
    gctUINT64       frameEndCPUTimeusec;
}
vx_profiler_s;

#define gcmWRITE_CONST(ConstValue) \
    do \
    { \
        gceSTATUS status; \
        gctINT32 value = ConstValue; \
        gcmERR_BREAK(gcoPROFILER_Write(context->phal, gcmSIZEOF(value), &value)); \
    } \
    while (gcvFALSE)

#define gcmWRITE_VALUE(IntData) \
    do \
    { \
        gceSTATUS status; \
        gctINT32 value = IntData; \
        gcmERR_BREAK(gcoPROFILER_Write(context->phal, gcmSIZEOF(value), &value)); \
    } \
    while (gcvFALSE)

#define gcmWRITE_COUNTER(Counter, Value) \
    gcmWRITE_CONST(Counter); \
    gcmWRITE_VALUE(Value)

/* Write a string value (char*). */
#define gcmWRITE_STRING(String) \
    do \
    { \
        gceSTATUS status; \
        gctINT32 length; \
        length = (gctINT32) gcoOS_StrLen((gctSTRING)String, gcvNULL); \
        gcmERR_BREAK(gcoPROFILER_Write(context->phal, gcmSIZEOF(length), &length)); \
        gcmERR_BREAK(gcoPROFILER_Write(context->phal, length, String)); \
    } \
    while (gcvFALSE)

gctINT vxoProfiler_Initialize(vx_context context);
void vxoProfiler_Destroy(vx_context context);
gctINT vxoProfiler_Begin(vx_reference refernece);
gctINT vxoProfiler_End(vx_reference refernece);

#endif

