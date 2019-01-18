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


#ifndef __gc_vsc_vir_param_opts_h_
#define __gc_vsc_vir_param_opts_h_

#include "gc_vsc.h"

BEGIN_EXTERN_C()


typedef struct VIR_PARAM_OPTIMIZATION
{
    VIR_Shader* shader;
    VSC_SIMPLE_RESIZABLE_ARRAY *candidateFuncs;
    VSC_SIMPLE_RESIZABLE_ARRAY *longSizeArguments;
    VIR_Operand *argMmPtr;
    VIR_Dumper *dumper;
    gctUINT paramLengthThreshold;
    VSC_OPTN_ParamOptOptions *options;
    VIR_DEF_USAGE_INFO *duInfo;
} VSC_PARAM_optimization;

typedef struct _CANDIDATE_FUNCTION
{
    VIR_FUNC_BLOCK* functionBlock;
    VSC_SIMPLE_RESIZABLE_ARRAY* longSizeParams;
}CANDIDATE_FUNCTION;

typedef struct _LONG_SIZE_PARAMETER
{
    VIR_Function* ownerFunc;
    gctUINT regStartIndex;
    gctUINT paramArraySize;
    gctUINT paramTypeByteSize;
    VIR_Operand *paramPtr;
}LONG_SIZE_PARAMETER;

typedef struct _LONG_SIZE_ARGUMENT
{
    VIR_Function *holderFunction;

    gctUINT regStartIndex;
    /*Length of argument array.*/
    gctUINT argArraySize;
    /*The length of one item in array.*/
    gctUINT8 argTypeByteSize;
    /*Offset to the start of first argument.*/
    gctUINT offset;
    /*Is this argument defined in caller function.*/
    gctUINT isDefinedInCaller;
}LONG_SIZE_ARGUMENT;

#define VSC_PARAM_optimization_GetShader(po)             ((po)->shader)
#define VSC_PARAM_optimization_SetShader(po, s)          ((po)->shader = (s))
#define VSC_PARAM_optimization_GetOptions(po)            ((po)->options)
#define VSC_PARAM_optimization_SetOptions(po, o)         ((po)->options = (o))
#define VSC_PARAM_optimization_GetCandidateFuncs(po)     ((po)->candidateFuncs)
#define VSC_PARAM_optimization_SetCandidateFuncs(po, c)  ((po)->candidateFuncs = (c))
#define VSC_PARAM_optimization_GetlongSizeArguments(po)     ((po)->longSizeArguments)
#define VSC_PARAM_optimization_SetlongSizeArguments(po, l)  ((po)->longSizeArguments = (l))
#define VSC_PARAM_optimization_GetArgumentPtr(po)             ((po)->argMmPtr)
#define VSC_PARAM_optimization_SetArgumentPtr(po, a)          ((po)->argMmPtr = (a))
#define VSC_PARAM_optimization_GetThreshold(po)             ((po)->paramLengthThreshold)
#define VSC_PARAM_optimization_SetThreshold(po, t)          ((po)->paramLengthThreshold = (t))
#define VSC_PARAM_optimization_GetDumper(po)             ((po)->dumper)
#define VSC_PARAM_optimization_SetDumper(po, d)          ((po)->dumper = (d))
#define VSC_PARAM_optimization_GetDuInfo(po)             ((po)->duInfo)
#define VSC_PARAM_optimization_SetDuInfo(po, du)          ((po)->duInfo = (du))


extern void VSC_PARAM_optimization_Init(
    IN OUT VSC_PARAM_optimization *po,
    IN VIR_Shader *shader,
    IN VSC_SIMPLE_RESIZABLE_ARRAY *candidateFuncs,
    IN VSC_SIMPLE_RESIZABLE_ARRAY *longSizeArguments,
    IN VIR_Operand *argMmPtr,
    IN VIR_Dumper *dumper,
    IN VSC_OPTN_ParamOptOptions *options,
    IN VIR_DEF_USAGE_INFO *duInfo
    );

extern void VSC_PARAM_optimization_Final(
    IN OUT VSC_PARAM_optimization* po
    );

extern VSC_ErrCode VSC_PARAM_Optimization_PerformOnShader(
    VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(VSC_PARAM_Optimization_PerformOnShader);
DECLARE_SH_NECESSITY_CHECK(VSC_PARAM_Optimization_PerformOnShader);

END_EXTERN_C()

#endif

