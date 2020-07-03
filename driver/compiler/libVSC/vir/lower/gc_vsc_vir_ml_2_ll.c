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


#include "gc_vsc.h"
#include "vir/lower/gc_vsc_vir_ml_2_ll.h"
#include "vir/lower/gc_vsc_vir_lower_common_func.h"
#include "vir/linker/gc_vsc_vir_linker.h"

/* lowering middle level vir to lower level vir (Machine code level) */

DEF_QUERY_PASS_PROP(VIR_Lower_MiddleLevel_To_LowLevel_Pre)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_ML;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_M2LLOWER;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;

    /* Lower must invalidate all analyzed resources */
    pPassProp->passFlag.resDestroyReq.s.bInvalidateCg = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateCfg = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateRdFlow = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateDu = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateWeb = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateLvFlow = gcvTRUE;
}

DEF_SH_NECESSITY_CHECK(VIR_Lower_MiddleLevel_To_LowLevel_Pre)
{
    VIR_Shader *            shader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;

    if (VIR_Shader_GetLevel(shader) != VIR_SHLEVEL_Post_Medium)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

VSC_ErrCode
VIR_Lower_MiddleLevel_To_LowLevel_Pre(
    IN  VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode             errCode  = VSC_ERR_NONE;
    VIR_PatternLowerContext context;
    VIR_Shader *            shader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VSC_HW_CONFIG*          hwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    gctBOOL                 bRAEnabled = *(gctBOOL*)pPassWorker->basePassWorker.pPassSpecificData;

    VIR_Shader_SetRAEnabled(shader, bRAEnabled);

    VIR_Lower_Initialize(shader, &context, hwCfg, pPassWorker->basePassWorker.pMM);

    errCode = VIR_Lower_ArraryIndexing_To_LDARR_STARR(shader, hwCfg, gcvNULL);
    CHECK_ERROR(errCode, "VIR_Lower_ArraryIndexing_To_LDARR_STARR failed.");

    errCode = VIR_Lower_MiddleLevel_To_LowLevel_Expand_Pre(shader, &pPassWorker->pCompilerParam->cfg.ctx, &context);
    CHECK_ERROR(errCode, "ML to LL expand pre failed.");

    errCode = VIR_Lower_MiddleLevel_To_LowLevel_Scalar(shader, &pPassWorker->pCompilerParam->cfg.ctx, &context);
    CHECK_ERROR(errCode, "VIR_Lower_MiddleLevel_To_LowLevel_Scalar failed.");

    if (gcUseFullNewLinker(hwCfg->hwFeatureFlags.hasHalti2))
    {
        errCode = VIR_Lower_MiddleLevel_To_LowLevel_Machine_Pre(shader, &pPassWorker->pCompilerParam->cfg.ctx, &context);
        CHECK_ERROR(errCode, "VIR_Lower_MiddleLevel_To_LowLevel_Machine_Pre failed.");
    }

    VIR_Shader_SetLevel(shader, VIR_SHLEVEL_Pre_Low);

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(shader), VIR_Shader_GetId(shader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After Lowered to LowLevel pre.", shader, gcvTRUE);
    }

    return errCode;
}

DEF_QUERY_PASS_PROP(VIR_Lower_MiddleLevel_To_LowLevel_Post)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_LL;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_M2LLOWER;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;

    /* Lower must invalidate all analyzed resources */
    pPassProp->passFlag.resDestroyReq.s.bInvalidateCg = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateCfg = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateRdFlow = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateDu = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateWeb = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateLvFlow = gcvTRUE;
}

DEF_SH_NECESSITY_CHECK(VIR_Lower_MiddleLevel_To_LowLevel_Post)
{
    VIR_Shader *            shader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;

    if (VIR_Shader_GetLevel(shader) != VIR_SHLEVEL_Pre_Low)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

VSC_ErrCode
VIR_Lower_MiddleLevel_To_LowLevel_Post(
    IN  VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode             errCode  = VSC_ERR_NONE;
    VIR_PatternLowerContext context;
    VIR_Shader *            shader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VSC_HW_CONFIG*          hwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    gctBOOL                 bRAEnabled = *(gctBOOL*)pPassWorker->basePassWorker.pPassSpecificData;

    VIR_Shader_SetRAEnabled(shader, bRAEnabled);

    VIR_Lower_Initialize(shader, &context, hwCfg, pPassWorker->basePassWorker.pMM);

    errCode = VIR_Lower_MiddleLevel_To_LowLevel_Expand_Post(shader, &pPassWorker->pCompilerParam->cfg.ctx, &context);
    CHECK_ERROR(errCode, "ML to LL expand post failed.");

    if (gcUseFullNewLinker(hwCfg->hwFeatureFlags.hasHalti2))
    {
        errCode = VIR_Lower_MiddleLevel_To_LowLevel_Machine_Post(shader, &pPassWorker->pCompilerParam->cfg.ctx, &context);
        CHECK_ERROR(errCode, "VIR_Lower_MiddleLevel_To_LowLevel_Machine_Post failed.");
    }

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(shader), VIR_Shader_GetId(shader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After Lowered to LowLevel post.", shader, gcvTRUE);
    }

    return errCode;
}

