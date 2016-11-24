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


#ifndef __gc_vsc_vir_loop_h_
#define __gc_vsc_vir_loop_h_

#include "gc_vsc.h"

BEGIN_EXTERN_C()

typedef struct VIR_LOOPINFOMGR
{
    VIR_Shader*                     shader;
    VIR_Function*                   func;
    gctUINT                         nextLoopId;
    VSC_UNI_LIST                    loopInfos;
    VIR_Dumper*                     dumper;
    VSC_MM*                         mm;
} VIR_LoopInfoMgr;

#define VIR_LoopInfoMgr_GetShader(lim)          ((lim)->shader)
#define VIR_LoopInfoMgr_SetShader(lim, s)       ((lim)->shader = (s))
#define VIR_LoopInfoMgr_GetFunc(lim)            ((lim)->func)
#define VIR_LoopInfoMgr_SetFunc(lim, f)         ((lim)->func = (f))
#define VIR_LoopInfoMgr_GetNextLoopId(lim)      ((lim)->nextLoopId)
#define VIR_LoopInfoMgr_SetNextLoopId(lim, n)   ((lim)->nextLoopId = (n))
#define VIR_LoopInfoMgr_IncNextLoopId(lim)      ((lim)->nextLoopId++)
#define VIR_LoopInfoMgr_GetLoopInfos(lim)       (&(lim)->loopInfos)
#define VIR_LoopInfoMgr_GetLoopInfoCount(lim)   (vscUNILST_GetNodeCount(VIR_LoopInfoMgr_GetLoopInfos(lim)))
#define VIR_LoopInfoMgr_GetMM(lim)              ((lim)->mm)
#define VIR_LoopInfoMgr_SetMM(lim, m)           ((lim)->mm = (m))
#define VIR_LoopInfoMgr_GetDumper(lim)          ((lim)->dumper)
#define VIR_LoopInfoMgr_SetDumper(lim, d)       ((lim)->dumper = (d))

typedef struct VIR_LOOPOPTS
{
    VIR_Shader* shader;
    VIR_Function* func;
    VIR_LoopInfoMgr* loopInfoMgr;
    VSC_OPTN_LoopOptsOptions* options;
    VIR_Dumper* dumper;
    VSC_MM* mm;
} VIR_LoopOpts;

#define VIR_LoopOpts_GetShader(lo)              ((lo)->shader)
#define VIR_LoopOpts_SetShader(lo, s)           ((lo)->shader = (s))
#define VIR_LoopOpts_GetFunc(lo)                ((lo)->func)
#define VIR_LoopOpts_SetFunc(lo, f)             ((lo)->func = (f))
#define VIR_LoopOpts_GetLoopInfoMgr(lo)         ((lo)->loopInfoMgr)
#define VIR_LoopOpts_SetLoopInfoMgr(lo, l)      ((lo)->loopInfoMgr = (l))
#define VIR_LoopOpts_GetOptions(lo)             ((lo)->options)
#define VIR_LoopOpts_SetOptions(lo, o)          ((lo)->options = (o))
#define VIR_LoopOpts_GetDumper(lo)              ((lo)->dumper)
#define VIR_LoopOpts_SetDumper(lo, d)           ((lo)->dumper = (d))
#define VIR_LoopOpts_GetMM(lo)                  ((lo)->mm)
#define VIR_LoopOpts_SetMM(lo, m)               ((lo)->mm = (m))

void
VIR_LoopOpts_Init(
    VIR_LoopOpts* loopOpts,
    VIR_Shader* shader,
    VIR_Function* func,
    VSC_OPTN_LoopOptsOptions* options,
    VIR_Dumper* dumper,
    VSC_MM* mm
    );

void
VIR_LoopOpts_Final(
    VIR_LoopOpts* loopOpts
    );

VSC_ErrCode
VIR_LoopOpts_PerformLoopInversionOnShader(
    VIR_LoopOpts* loopOpts,
    gctBOOL* changed
    );

VSC_ErrCode
VIR_LoopOpts_PerformLoopUnrollingOnShader(
    VIR_LoopOpts* loopOpts,
    gctBOOL* changed
    );

VSC_ErrCode
VIR_LoopOpts_PerformOnShader(
    VSC_SH_PASS_WORKER* pPassWorker
    );
DECLARE_QUERY_PASS_PROP(VIR_LoopOpts_PerformOnShader);

END_EXTERN_C()

#endif /* __gc_vsc_vir_loop_h_ */


