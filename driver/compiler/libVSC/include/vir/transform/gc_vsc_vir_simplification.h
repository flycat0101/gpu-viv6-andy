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


#ifndef __gc_vsc_vir_simplification_h_
#define __gc_vsc_vir_simplification_h_

#include "gc_vsc.h"

BEGIN_EXTERN_C()

typedef struct VIR_SIMP_SIMPLIFICATION
{
    VIR_Shader* shader;
    VIR_Function* currFunc;
    VIR_BASIC_BLOCK* currBB;
    VSC_OPTN_SIMPOptions* options;
    VIR_Dumper* dumper;
} VSC_SIMP_Simplification;

#define VSC_SIMP_Simplification_GetShader(simp)             ((simp)->shader)
#define VSC_SIMP_Simplification_SetShader(simp, s)          ((simp)->shader = (s))
#define VSC_SIMP_Simplification_GetCurrFunc(simp)           ((simp)->currFunc)
#define VSC_SIMP_Simplification_SetCurrFunc(simp, f)        ((simp)->currFunc = (f))
#define VSC_SIMP_Simplification_GetCurrBB(simp)             ((simp)->currBB)
#define VSC_SIMP_Simplification_SetCurrBB(simp, b)          ((simp)->currBB = (b))
#define VSC_SIMP_Simplification_GetOptions(simp)            ((simp)->options)
#define VSC_SIMP_Simplification_SetOptions(simp, o)         ((simp)->options = (o))
#define VSC_SIMP_Simplification_GetDumper(simp)             ((simp)->dumper)
#define VSC_SIMP_Simplification_SetDumper(simp, d)          ((simp)->dumper = (d))

extern void VSC_SIMP_Simplification_Init(
    IN VSC_SIMP_Simplification* simp,
    IN VIR_Shader* shader,
    IN VIR_Function* currFunc,
    IN VIR_BASIC_BLOCK* currBB,
    IN VSC_OPTN_SIMPOptions* options,
    IN VIR_Dumper* dumper
    );

extern void VSC_SIMP_Simplification_Final(
    IN OUT VSC_SIMP_Simplification* simp
    );

extern VSC_ErrCode VSC_SIMP_Simplification_PerformOnInst(
    IN OUT VSC_SIMP_Simplification* simp,
    IN OUT VIR_Instruction* inst,
    OUT gctBOOL* change
    );

extern VSC_ErrCode VSC_SIMP_Simplification_PerformOnBB(
    IN OUT VSC_SIMP_Simplification* simp
    );

extern VSC_ErrCode VSC_SIMP_Simplification_PerformOnFunction(
    IN OUT VSC_SIMP_Simplification* simp
    );

extern VSC_ErrCode VSC_SIMP_Simplification_PerformOnShader(
    IN VSC_SH_PASS_WORKER* pPassWorker
    );
DECLARE_QUERY_PASS_PROP(VSC_SIMP_Simplification_PerformOnShader);

END_EXTERN_C()
#endif

