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


#ifndef __GC_VX_PROFILER_H__
#define __GC_VX_PROFILER_H__

#include "shared/gc_hal_types.h"

/* Profile information. */
typedef struct _vx_profiler
{
    gctBOOL         enable;
    /* If true, the counter is per-graph-process,
    ** else is per-node-process(default).
    */
    gctBOOL         perGraphProcess;

    gctUINT32       frameNumber;
    gctUINT64       frameStartTimeusec;
    gctUINT64       frameEndTimeusec;
}
vx_profiler_s;

gctINT vxoProfiler_Initialize(vx_context context);
void vxoProfiler_Destroy(vx_context context);
gctINT vxoProfiler_Begin(vx_reference refernece);
gctINT vxoProfiler_End(vx_reference refernece);

#endif

