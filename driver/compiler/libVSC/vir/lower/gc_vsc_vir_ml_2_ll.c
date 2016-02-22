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


#include "gc_vsc.h"
#include "vir/lower/gc_vsc_vir_ml_2_ll.h"
#include "vir/lower/gc_vsc_vir_lower_common_func.h"

/* lowering middle level vir to lower level vir (Machine code level) */
VSC_ErrCode
VIR_Lower_MiddleLevel_To_LowLevel_Preprocess(
    IN  VIR_Shader              *Shader,
    IN  VSC_HW_CONFIG           *HwCfg,
    IN  VIR_PatternLowerContext *Context
    );

VSC_ErrCode
VIR_Lower_MiddleLevel_To_LowLevel_Expand(
    IN  VIR_Shader              *Shader,
    IN  VSC_HW_CONFIG           *HwCfg,
    IN  VIR_PatternLowerContext *Context
    );

VSC_ErrCode
VIR_Lower_MiddleLevel_To_LowLevel_Scalar(
    IN  VIR_Shader              *Shader,
    IN  VSC_HW_CONFIG           *HwCfg,
    IN  VIR_PatternLowerContext *Context
    );

VSC_ErrCode
VIR_Lower_MiddleLevel_To_LowLevel_Machine(
    IN  VIR_Shader              *Shader,
    IN  VSC_HW_CONFIG           *HwCfg,
    IN  VIR_PatternLowerContext *Context
    );

static void
_Lower_Initialize(
    IN VIR_Shader               *Shader,
    IN VIR_PatternLowerContext  *Context,
    IN VSC_HW_CONFIG            *HwCfg
    )
{
    Context->hwCfg = HwCfg;

    Context->hasNEW_TEXLD = HwCfg->hwFeatureFlags.hasHalti2;

    if (HwCfg->hwFeatureFlags.hasSHEnhance2)
    {
        if (gcmOPT_NOIMMEDIATE())
            Context->generateImmediate  = gcvFALSE;
        else
            Context->generateImmediate  = gcvTRUE;
    }
    else
    {
        Context->generateImmediate  = gcvFALSE;
    }

    Context->isCL_X  = (gctBOOL)HwCfg->hwFeatureFlags.needCLXFixes;

    Context->hasCL   = Context->isCL_X || (gctBOOL)HwCfg->hwFeatureFlags.needCLXEFixes;

    Context->hasHalti1 = (gctBOOL)HwCfg->hwFeatureFlags.hasHalti1;
    Context->hasHalti2 = (gctBOOL)HwCfg->hwFeatureFlags.hasHalti2;
    Context->hasHalti3 = (gctBOOL)HwCfg->hwFeatureFlags.hasHalti3;
    Context->hasHalti4 = (gctBOOL)HwCfg->hwFeatureFlags.hasHalti4;
}

VSC_ErrCode
VIR_Lower_MiddleLevel_To_LowLevel(
    IN  VIR_Shader *    Shader,
    IN  VSC_HW_CONFIG*  HwCfg
    )
{
    VSC_ErrCode             errCode  = VSC_ERR_NONE;
    VIR_PatternLowerContext context;

    gcmASSERT(VIR_Shader_GetLevel(Shader) == VIR_SHLEVEL_Post_Medium);

    _Lower_Initialize(Shader, &context, HwCfg);

    errCode = VIR_Lower_MiddleLevel_To_LowLevel_Preprocess(Shader, HwCfg, &context);
    CHECK_ERROR(errCode, "VIR_Lower_MiddleLevel_To_LowLevel failed.");

    errCode = VIR_Lower_MiddleLevel_To_LowLevel_Expand(Shader, HwCfg, &context);
    CHECK_ERROR(errCode, "VIR_Lower_MiddleLevel_To_LowLevel failed.");

    errCode = VIR_Lower_MiddleLevel_To_LowLevel_Scalar(Shader, HwCfg, &context);
    CHECK_ERROR(errCode, "VIR_Lower_MiddleLevel_To_LowLevel failed.");

    if (ENABLE_FULL_NEW_LINKER)
    {
        errCode = VIR_Lower_MiddleLevel_To_LowLevel_Machine(Shader, HwCfg, &context);
        CHECK_ERROR(errCode, "VIR_Lower_MiddleLevel_To_LowLevel failed.");
    }

    VIR_Shader_SetLevel(Shader, VIR_SHLEVEL_Pre_Low);

    if (gcSHADER_DumpCodeGenVerbose(Shader))
    {
        VIR_Shader_Dump(gcvNULL, "After Lowered to LowLevel.", Shader, gcvTRUE);
    }

    return errCode;
}

