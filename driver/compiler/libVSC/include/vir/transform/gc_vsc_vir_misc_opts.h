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


#ifndef __gc_vsc_vir_misc_opts_h_
#define __gc_vsc_vir_misc_opts_h_

#include "gc_vsc.h"

BEGIN_EXTERN_C()

VSC_ErrCode vscVIR_RemoveNop(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_RemoveNop);

VSC_ErrCode vscVIR_VX_ReplaceDest(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_VX_ReplaceDest);

VSC_ErrCode vscVIR_PutScalarConstToImm(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_PutScalarConstToImm);

VSC_ErrCode vscVIR_PutImmValueToUniform(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_PutImmValueToUniform);

VSC_ErrCode vscVIR_CheckCstRegFileReadPortLimitation(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_CheckCstRegFileReadPortLimitation);

VSC_ErrCode vscVIR_CheckEvisInstSwizzleRestriction(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_CheckEvisInstSwizzleRestriction);

VSC_ErrCode vscVIR_CheckPosAndDepthConflict(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_CheckPosAndDepthConflict);

VSC_ErrCode VIR_Shader_CheckDual16able(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(VIR_Shader_CheckDual16able);

VSC_ErrCode vscVIR_AddOutOfBoundCheckSupport(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_AddOutOfBoundCheckSupport);

VSC_ErrCode vscVIR_AdjustPrecision(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_AdjustPrecision);

VSC_ErrCode vscVIR_ConvertVirtualInstructions(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_ConvertVirtualInstructions);

VSC_ErrCode vscVIR_PreprocessLLShader(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_PreprocessLLShader);

VSC_ErrCode vscVIR_CheckMustInlineFuncForML(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_CheckMustInlineFuncForML);

VSC_ErrCode vscVIR_PostprocessMLPostShader(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_PostprocessMLPostShader);

VSC_ErrCode vscVIR_CheckVariableUsage(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_CheckVariableUsage);

VSC_ErrCode vscVIR_FixTexldOffset(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_FixTexldOffset);

VSC_ErrCode vscVIR_InitializeVariables(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_InitializeVariables);

VSC_ErrCode vscVIR_FixDynamicIdxDep(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_FixDynamicIdxDep);

VSC_ErrCode vscVIR_GenCombinedSampler(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_GenCombinedSampler);

VSC_ErrCode vscVIR_GenExternalAtomicCall(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_GenExternalAtomicCall);

VSC_ErrCode vscVIR_GenRobustBoundCheck(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_GenRobustBoundCheck);

END_EXTERN_C()

#endif /* __gc_vsc_vir_misc_opts_h_ */

