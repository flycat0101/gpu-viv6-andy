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


#ifndef __gc_vsc_vir_loop_h_
#define __gc_vsc_vir_loop_h_

#include "gc_vsc.h"

BEGIN_EXTERN_C()

typedef struct VIR_LOOPINFOMGR VIR_LoopInfoMgr;

typedef struct VIR_LOOPOPTS
{
    VIR_Shader* shader;
    VIR_Function* func;
    VIR_LoopInfoMgr* loopInfoMgr;
    VSC_OPTN_LoopOptsOptions* options;
    VIR_Dumper* dumper;
    VSC_MM* mm;
    gctUINT allowedInstNumAfterUnroll;
    gctBOOL hwsupportPerCompDepForLS;
} VIR_LoopOpts;

#define VIR_LoopOpts_GetShader(lo)                        ((lo)->shader)
#define VIR_LoopOpts_SetShader(lo, s)                     ((lo)->shader = (s))
#define VIR_LoopOpts_GetFunc(lo)                          ((lo)->func)
#define VIR_LoopOpts_SetFunc(lo, f)                       ((lo)->func = (f))
#define VIR_LoopOpts_GetLoopInfoMgr(lo)                   ((lo)->loopInfoMgr)
#define VIR_LoopOpts_SetLoopInfoMgr(lo, l)                ((lo)->loopInfoMgr = (l))
#define VIR_LoopOpts_GetOptions(lo)                       ((lo)->options)
#define VIR_LoopOpts_SetOptions(lo, o)                    ((lo)->options = (o))
#define VIR_LoopOpts_GetDumper(lo)                        ((lo)->dumper)
#define VIR_LoopOpts_SetDumper(lo, d)                     ((lo)->dumper = (d))
#define VIR_LoopOpts_GetMM(lo)                            ((lo)->mm)
#define VIR_LoopOpts_SetMM(lo, m)                         ((lo)->mm = (m))
#define VIR_LoopOpts_GetAllowedInstNumAfterUnroll(lo)     ((lo)->allowedInstNumAfterUnroll)
#define VIR_LoopOpts_SetAllowedInstNumAfterUnroll(lo, n)  ((lo)->allowedInstNumAfterUnroll = (n))
#define VIR_LoopOpts_HWsupportPerCompDepForLS(lo)         ((lo)->hwsupportPerCompDepForLS)
#define VIR_LoopOpts_SetHWsupportPerCompDepForLS(lo, b)   ((lo)->hwsupportPerCompDepForLS = (b))

void
VIR_LoopOpts_Init(
    VIR_LoopOpts* loopOpts,
    VIR_Shader* shader,
    VIR_Function* func,
    VSC_OPTN_LoopOptsOptions* options,
    VIR_Dumper* dumper,
    VSC_MM* mm,
    gctBOOL hwSuppertCompDepForLS
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


