/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


/* This file is triggered by CompileShader API call of any client. A SEP may be generated
   if cFlags tells it should generate that. */

#include "gc_vsc.h"
#include "vir/lower/gc_vsc_vir_ml_2_ll.h"
#include "vir/lower/gc_vsc_vir_ll_2_mc.h"
#include "vir/transform/gc_vsc_vir_scalarization.h"
#include "vir/transform/gc_vsc_vir_peephole.h"
#include "vir/transform/gc_vsc_vir_simplification.h"
#include "vir/transform/gc_vsc_vir_misc_opts.h"
#include "vir/transform/gc_vsc_vir_cpp.h"
#include "vir/transform/gc_vsc_vir_dce.h"
#include "vir/transform/gc_vsc_vir_cpf.h"
#include "vir/transform/gc_vsc_vir_cse.h"
#include "vir/transform/gc_vsc_vir_inline.h"
#include "vir/transform/gc_vsc_vir_fcp.h"
#include "vir/transform/gc_vsc_vir_uniform.h"
#include "vir/transform/gc_vsc_vir_static_patch.h"
#include "vir/transform/gc_vsc_vir_vectorization.h"
#include "vir/codegen/gc_vsc_vir_inst_scheduler.h"
#include "vir/codegen/gc_vsc_vir_reg_alloc.h"
#include "vir/codegen/gc_vsc_vir_mc_gen.h"
#include "vir/codegen/gc_vsc_vir_ep_gen.h"
#include "vir/codegen/gc_vsc_vir_ep_back_patch.h"

gceSTATUS vscInitializeSEP(SHADER_EXECUTABLE_PROFILE* pSEP)
{
    gctUINT i;

    gcoOS_ZeroMemory(pSEP, sizeof(SHADER_EXECUTABLE_PROFILE));

    for (i = 0; i < SHADER_IO_USAGE_TOTAL_COUNT; i ++)
    {
        pSEP->inputMapping.ioVtxPxl.usage2IO[i].mainFirstValidIoChannel =
        pSEP->outputMapping.ioVtxPxl.usage2IO[i].mainFirstValidIoChannel =
        pSEP->inputMapping.ioVtxPxl.usage2IO[i].mainIoIndex =
        pSEP->outputMapping.ioVtxPxl.usage2IO[i].mainIoIndex = NOT_ASSIGNED;

        pSEP->inputMapping.ioPrim.usage2IO[i].mainFirstValidIoChannel =
        pSEP->outputMapping.ioPrim.usage2IO[i].mainFirstValidIoChannel =
        pSEP->inputMapping.ioPrim.usage2IO[i].mainIoIndex =
        pSEP->outputMapping.ioPrim.usage2IO[i].mainIoIndex = NOT_ASSIGNED;
    }

    pSEP->inputMapping.ioVtxPxl.ioCategory = SHADER_IO_CATEGORY_PER_VTX_PXL;
    pSEP->inputMapping.ioPrim.ioCategory = SHADER_IO_CATEGORY_PER_PRIM;

    pSEP->outputMapping.ioVtxPxl.ioCategory = SHADER_IO_CATEGORY_PER_VTX_PXL;
    pSEP->outputMapping.ioPrim.ioCategory = SHADER_IO_CATEGORY_PER_PRIM;

    for (i = 0; i < SHADER_CONSTANT_USAGE_TOTAL_COUNT; i ++)
    {
        pSEP->constantMapping.usage2ArrayIndex[i] = NOT_ASSIGNED;
    }

    return gcvSTATUS_OK;
}

gceSTATUS vscFinalizeSEP(SHADER_EXECUTABLE_PROFILE* pSEP)
{
    gctUINT i;

    if (pSEP->pMachineCode)
    {
        gcoOS_Free(gcvNULL, pSEP->pMachineCode);
        pSEP->pMachineCode = gcvNULL;
    }

    if (pSEP->inputMapping.ioVtxPxl.pIoRegMapping)
    {
        gcoOS_Free(gcvNULL, pSEP->inputMapping.ioVtxPxl.pIoRegMapping);
        pSEP->inputMapping.ioVtxPxl.pIoRegMapping = gcvNULL;
    }

    if (pSEP->inputMapping.ioPrim.pIoRegMapping)
    {
        gcoOS_Free(gcvNULL, pSEP->inputMapping.ioPrim.pIoRegMapping);
        pSEP->inputMapping.ioPrim.pIoRegMapping = gcvNULL;
    }

    if (pSEP->outputMapping.ioVtxPxl.pIoRegMapping)
    {
        gcoOS_Free(gcvNULL, pSEP->outputMapping.ioVtxPxl.pIoRegMapping);
        pSEP->outputMapping.ioVtxPxl.pIoRegMapping = gcvNULL;
    }

    if (pSEP->outputMapping.ioPrim.pIoRegMapping)
    {
        gcoOS_Free(gcvNULL, pSEP->outputMapping.ioPrim.pIoRegMapping);
        pSEP->outputMapping.ioPrim.pIoRegMapping = gcvNULL;
    }

    if (pSEP->constantMapping.pConstantArrayMapping)
    {
        if (pSEP->constantMapping.pConstantArrayMapping->pSubConstantArrays)
        {
            gcoOS_Free(gcvNULL, pSEP->constantMapping.pConstantArrayMapping->pSubConstantArrays);
            pSEP->constantMapping.pConstantArrayMapping->pSubConstantArrays = gcvNULL;
        }

        gcoOS_Free(gcvNULL, pSEP->constantMapping.pConstantArrayMapping);
        pSEP->constantMapping.pConstantArrayMapping = gcvNULL;
    }

    if (pSEP->constantMapping.pCompileTimeConstant)
    {
        gcoOS_Free(gcvNULL, pSEP->constantMapping.pCompileTimeConstant);
        pSEP->constantMapping.pCompileTimeConstant = gcvNULL;
    }

    for (i = 0; i < pSEP->staticPatchConstants.countOfEntries; i ++)
    {
        if (pSEP->staticPatchConstants.pPatchConstantEntries[i].pPrivateData)
        {
            gcoOS_Free(gcvNULL, pSEP->staticPatchConstants.pPatchConstantEntries[i].pPrivateData);
            pSEP->staticPatchConstants.pPatchConstantEntries[i].pPrivateData = gcvNULL;
        }
    }

    if (pSEP->staticPatchConstants.pPatchConstantEntries)
    {
        gcoOS_Free(gcvNULL, pSEP->staticPatchConstants.pPatchConstantEntries);
        pSEP->staticPatchConstants.pPatchConstantEntries = gcvNULL;
        pSEP->staticPatchConstants.countOfEntries = 0;
    }

    for (i = 0; i < pSEP->staticPatchExtraMems.countOfEntries; i ++)
    {
        if (pSEP->staticPatchExtraMems.pPatchCommonEntries[i].pPrivateData)
        {
            gcoOS_Free(gcvNULL, pSEP->staticPatchExtraMems.pPatchCommonEntries[i].pPrivateData);
            pSEP->staticPatchExtraMems.pPatchCommonEntries[i].pPrivateData = gcvNULL;
        }
    }

    if (pSEP->staticPatchExtraMems.pPatchCommonEntries)
    {
        gcoOS_Free(gcvNULL, pSEP->staticPatchExtraMems.pPatchCommonEntries);
        pSEP->staticPatchExtraMems.pPatchCommonEntries = gcvNULL;
        pSEP->staticPatchExtraMems.countOfEntries = 0;
    }

    vscInitializeSEP(pSEP);

    return gcvSTATUS_OK;
}

gctBOOL vscIsValidSEP(SHADER_EXECUTABLE_PROFILE* pSEP)
{
    return (pSEP->pMachineCode != gcvNULL);
}

gceSTATUS vscInitializeIoRegMapping(SHADER_IO_REG_MAPPING* pIoRegMapping)
{
    gctUINT i;

    gcoOS_ZeroMemory(pIoRegMapping, sizeof(SHADER_IO_REG_MAPPING));

    pIoRegMapping->ioIndex =
    pIoRegMapping->firstValidIoChannel = NOT_ASSIGNED;

    for (i = 0; i < CHANNEL_NUM; i ++)
    {
        pIoRegMapping->ioChannelMapping[i].ioUsage = SHADER_IO_USAGE_GENERAL;
        pIoRegMapping->ioChannelMapping[i].hwLoc.cmnHwLoc.u.hwRegNo = NOT_ASSIGNED;
        pIoRegMapping->ioChannelMapping[i].hwLoc.t1HwLoc.hwRegNo = NOT_ASSIGNED;
    }

    return gcvSTATUS_OK;
}

gceSTATUS vscFinalizeIoRegMapping(SHADER_IO_REG_MAPPING* pIoRegMapping)
{
    return vscInitializeIoRegMapping(pIoRegMapping);
}

gceSTATUS vscInitializeCTC(SHADER_COMPILE_TIME_CONSTANT* pCompileTimeConstant)
{
    gcoOS_ZeroMemory(pCompileTimeConstant, sizeof(SHADER_COMPILE_TIME_CONSTANT));

    pCompileTimeConstant->hwConstantLocation.hwAccessMode = SHADER_HW_ACCESS_MODE_REGISTER;
    pCompileTimeConstant->hwConstantLocation.hwLoc.hwRegNo = NOT_ASSIGNED;

    return gcvSTATUS_OK;
}

gceSTATUS vscFinalizeCTC(SHADER_COMPILE_TIME_CONSTANT* pCompileTimeConstant)
{
    return vscInitializeCTC(pCompileTimeConstant);
}

static VSC_ErrCode _PreprocessShader(VSC_PASS_MANAGER* pPassMnger)
{
    VIR_Shader*         pShader = pPassMnger->pCompilerParam->pShader;

    return vscVIR_RemoveNop(pShader);
}

static VSC_ErrCode _PostprocessShader(VSC_PASS_MANAGER*          pPassMnger,
                                      SHADER_EXECUTABLE_PROFILE* pOutSEP)
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_Shader*             pShader = pPassMnger->pCompilerParam->pShader;
    VSC_OPTN_SEPGenOptions* sepgen_options = VSC_OPTN_Options_GetSEPGenOptions(pPassMnger->options);

    if (pOutSEP && vscIsValidSEP(pOutSEP))
    {
        errCode = vscVIR_PerformSEPBackPatch(pShader,
                                             pPassMnger->pCompilerParam->cfg.pHwCfg,
                                             pOutSEP,
                                             VSC_OPTN_SEPGenOptions_GetTrace(sepgen_options));
        ON_ERROR(errCode, "Perform SEP back patch");
    }

OnError:
    return VSC_ERR_NONE;
}

static VSC_ErrCode _CompileShaderAtHighLevel(VSC_PASS_MANAGER* pPassMnger)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Shader*         pShader = pPassMnger->pCompilerParam->pShader;

    gcmASSERT(VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Pre_High);

    /* We are at end of HL of VIR, so set this correct level */
    VIR_Shader_SetLevel(pShader, VIR_SHLEVEL_Post_High);

/*OnError:*/
    return errCode;
}

static VSC_ErrCode _CompileShaderAtMedLevel(VSC_PASS_MANAGER* pPassMnger)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Shader*         pShader = pPassMnger->pCompilerParam->pShader;

    gcmASSERT(VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Pre_Medium ||
              VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Post_High);

    if (VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Post_High)
    {
        /* It should be moved into hl2ml lower if the lower is implemented later */
        VIR_Shader_SetLevel(pShader, VIR_SHLEVEL_Pre_Medium);
    }

    /* We are at end of ML of VIR, so set this correct level */
    VIR_Shader_SetLevel(pShader, VIR_SHLEVEL_Post_Medium);

/*OnError:*/
    return errCode;
}

static VSC_ErrCode _CompileShaderAtLowLevel(VSC_PASS_MANAGER* pPassMnger)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Shader*         pShader = pPassMnger->pCompilerParam->pShader;
    VIR_Dumper*         dumper = pShader->dumper;

    gcmASSERT(VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Pre_Low ||
              VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Post_Medium);

    /* if RA enable set the shader to be VIR_Shader_isRAEnabled,
       this flag is used in the lowering */
    {
        VSC_OPTN_RAOptions* ra_options = VSC_OPTN_Options_GetRAOptions(pPassMnger->options);
        if (VSC_OPTN_RAOptions_GetSwitchOn(ra_options))
        {
            VIR_Shader_SetRAEnabled(pShader, gcvTRUE);
        }
    }

    if (VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Post_Medium)
    {
        /* Do lowering from mid level to low level */
        {
            VSC_OPTN_LowerM2LOptions* lowerM2L_options = VSC_OPTN_Options_GetLowerM2LOptions(pPassMnger->options);
            if (VSC_OPTN_LowerM2LOptions_GetSwitchOn(lowerM2L_options))
            {
                errCode = VIR_Lower_MiddleLevel_To_LowLevel(pShader, pPassMnger->pCompilerParam->cfg.pHwCfg);
            }
        }
    }

    /* Create Call Graph */
    vscVIR_BuildCallGraph(pShader, &pPassMnger->callGraph);

    /* Create SSA form */

    /* Do Inline */
    {
        VSC_OPTN_ILOptions* inliner_options = VSC_OPTN_Options_GetInlinerOptions(pPassMnger->options);
        if (VSC_OPTN_ILOptions_GetSwitchOn(inliner_options))
        {
            VIR_Inliner inliner;
            VSC_IL_Init(&inliner, pShader, pPassMnger->pCompilerParam->cfg.pHwCfg,
                inliner_options, dumper, &pPassMnger->callGraph);
            errCode = VSC_IL_PerformOnShader(&inliner);
            VSC_IL_Final(&inliner);
        }
    }

    /* Create CFG */
    vscVIR_BuildCFG(pShader);

    /* Do constant propagation and folding before du is built */
    {
        VSC_OPTN_CPFOptions* cpf_options = VSC_OPTN_Options_GetCPFOptions(pPassMnger->options);
        if (VSC_OPTN_CPFOptions_GetSwitchOn(cpf_options))
        {
            errCode = VSC_CPF_PerformOnShader(pShader, cpf_options, dumper);
        }
    }

    /* Do simplification */
    {
        VSC_OPTN_SIMPOptions* simp_options = VSC_OPTN_Options_GetSIMPOptions(pPassMnger->options);
        if (VSC_OPTN_SIMPOptions_GetSwitchOn(simp_options))
        {
            errCode = VSC_SIMP_Simplification_PerformOnShader(pShader, simp_options, dumper);
        }
    }

    /* Do local common subexpression elimination before du is built */
    {
        VSC_OPTN_LCSEOptions* cse_options = VSC_OPTN_Options_GetLCSEOptions(pPassMnger->options);
        if (VSC_OPTN_LCSEOptions_GetSwitchOn(cse_options))
        {
            errCode = VSC_LCSE_PerformOnShader(pShader, cse_options, dumper);
        }
    }

    /* Do D-U analysis */
    vscVIR_BuildDefUsageInfo(&pPassMnger->callGraph, &pPassMnger->duInfo, gcvFALSE);

    /* Do copy propagation opt*/
    {
        VSC_OPTN_CPPOptions* cpp_options = VSC_OPTN_Options_GetCPPOptions(pPassMnger->options);
        if (VSC_OPTN_CPPOptions_GetSwitchOn(cpp_options))
        {
            VSC_CPP_CopyPropagation cpp;
            VSC_CPP_Init(&cpp, pShader, &pPassMnger->duInfo, cpp_options, dumper);
            errCode = VSC_CPP_PerformOnShader(&cpp);
            VSC_CPP_Final(&cpp);
        }
    }

    /* Do scalarization */
    {
        VSC_OPTN_SCLOptions* scl_options = VSC_OPTN_Options_GetSCLOptions(pPassMnger->options);
        if (VSC_OPTN_SCLOptions_GetSwitchOn(scl_options))
        {
            VSC_SCL_Scalarization scl;
            VSC_SCL_Scalarization_Init(&scl, pShader, scl_options, dumper);
            errCode = VSC_SCL_Scalarization_PerformOnShader(&scl);
            VSC_SCL_Scalarization_Final(&scl);
        }
    }

    /* Do peephole opt*/
    {
        VSC_OPTN_PHOptions* ph_options = VSC_OPTN_Options_GetPHOptions(pPassMnger->options);
        if (VSC_OPTN_PHOptions_GetSwitchOn(ph_options))
        {
            VSC_PH_Peephole ph;
            VSC_PH_Peephole_Init(&ph, pShader, &pPassMnger->duInfo, ph_options, dumper);
            errCode = VSC_PH_Peephole_PerformOnShader(&ph);
            if (VSC_PH_Peephole_GetCfgChanged(&ph))
            {
                /* PH changes CFG in removing the unnecessary JMPs, so we need redo CFG/DU */

                /* Destroy previous CFG/DU */
                vscVIR_DestroyDefUsageInfo(&pPassMnger->duInfo);
                vscVIR_DestroyCFG(pShader);

                /* Rebuild CFG/DU */
                vscVIR_BuildCFG(pShader);
                vscVIR_BuildDefUsageInfo(&pPassMnger->callGraph, &pPassMnger->duInfo, gcvFALSE);
            }
            VSC_PH_Peephole_Final(&ph);
        }
    }

    if (ENABLE_FULL_NEW_LINKER)
    {
        errCode = vscVIR_DoLocalVectorization(pShader, &pPassMnger->duInfo);
    }

    /* Do dead code elimination */
    {
        VSC_OPTN_DCEOptions* dce_options = VSC_OPTN_Options_GetDCEOptions(pPassMnger->options);
        gctBOOL              rebuildCFG = gcvFALSE;
        if (VSC_OPTN_DCEOptions_GetSwitchOn(dce_options))
        {
            errCode = VSC_DCE_Perform(pShader, &pPassMnger->duInfo, dce_options, dumper, &rebuildCFG);
            if (rebuildCFG)
            {
                /* Destroy previous CFG/DU */
                vscVIR_DestroyDefUsageInfo(&pPassMnger->duInfo);
                vscVIR_DestroyCFG(pShader);

                /* Rebuild CFG/DU */
                vscVIR_BuildCFG(pShader);
                vscVIR_BuildDefUsageInfo(&pPassMnger->callGraph, &pPassMnger->duInfo, gcvFALSE);
            }
        }
    }

    /* adjust the precision for dual16 purpose */
    {
        errCode = vscVIR_AdjustPrecision(pShader, &pPassMnger->duInfo, pPassMnger->pCompilerParam->cfg.pHwCfg);
    }

    if (ENABLE_FULL_NEW_LINKER)
    {
        /* Destroy D-U analysis */
        vscVIR_DestroyDefUsageInfo(&pPassMnger->duInfo);

        /* Destroy CG/CFG */
        vscVIR_DestroyCFG(pPassMnger->pCompilerParam->pShader);
        vscVIR_DestroyCallGraph(&pPassMnger->callGraph);
    }

    /* We are at the end of LL of VIR, so set this correct level */
    VIR_Shader_SetLevel(pShader, VIR_SHLEVEL_Post_Low);

    return errCode;
}

static VSC_ErrCode _CompileShaderAtMCLevel(VSC_PASS_MANAGER*          pPassMnger)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Shader*         pShader = pPassMnger->pCompilerParam->pShader;

    gcmASSERT(VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Pre_Machine ||
              VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Post_Low);

    if (VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Post_Low)
    {
        if (ENABLE_FULL_NEW_LINKER)
        {
            errCode = VIR_Lower_LowLevel_To_MachineCodeLevel(pShader, pPassMnger->pCompilerParam->cfg.pHwCfg);
            ON_ERROR(errCode, "ll2mc lower");
        }
        else
        {
            /* It should be moved into ll2mc lower if the lower is implemented later */
            VIR_Shader_SetLevel(pShader, VIR_SHLEVEL_Pre_Machine);
        }
    }

    if (ENABLE_FULL_NEW_LINKER)
    {
        errCode = vscVIR_PerformSpecialHwPatches(pShader);
        ON_ERROR(errCode, "Perform special HW patch");

        /* Check if the shader can run on dual16 mode, should before vscVIR_PutImmValueToUniform,
           since vscVIR_PutImmValueToUniform will use dual16 flag */
        if (VIR_Shader_IsDual16able(pShader, pPassMnger->pCompilerParam->cfg.pHwCfg))
        {
            VIR_Shader_SetDual16Mode(pShader, gcvTRUE);
        }
        else
        {
            VIR_Shader_SetDual16Mode(pShader, gcvFALSE);
        }

        errCode = vscVIR_PutImmValueToUniform(pShader, pPassMnger->pCompilerParam->cfg.pHwCfg);
        ON_ERROR(errCode, "Put imm value to uniform");
    }

    /* We are at end of MC level of VIR, so set this correct level */
    VIR_Shader_SetLevel(pShader, VIR_SHLEVEL_Post_Machine);

OnError:
    return errCode;
}

static VSC_ErrCode _PerformCodegen(VSC_PASS_MANAGER*          pPassMnger,
                                   SHADER_EXECUTABLE_PROFILE* pOutSEP)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Shader*         pShader = pPassMnger->pCompilerParam->pShader;
    VIR_Dumper*         dumper = pShader->dumper;
    gctBOOL             bRAed = gcvFALSE, bMCGened = gcvFALSE;

    /* Uniform allocation */
    {
        VSC_OPTN_RAOptions* ra_options = VSC_OPTN_Options_GetRAOptions(pPassMnger->options);
        if (VSC_OPTN_RAOptions_GetSwitchOn(ra_options))
        {
            errCode = VIR_RA_LS_PerformUniformAlloc(pShader, pPassMnger->pCompilerParam->cfg.pHwCfg,
                                                    ra_options, dumper);
            ON_ERROR(errCode, "Uniform RA");

            errCode = vscVIR_CheckCstRegFileReadPortLimitation(pShader, pPassMnger->pCompilerParam->cfg.pHwCfg);
            ON_ERROR(errCode, "Check constant register file read port limitation");
        }
    }

    /* A temp solution to check whether we need build CG/CFG/DU.
       TOTO: We need implement correct pass manager to manage all passes */
    if (pPassMnger->callGraph.pOwnerShader == gcvNULL)
    {
        /* Create Call Graph */
        vscVIR_BuildCallGraph(pShader, &pPassMnger->callGraph);

        /* Create CFG */
        vscVIR_BuildCFG(pShader);

        /* Do D-U analysis */
        vscVIR_BuildDefUsageInfo(&pPassMnger->callGraph, &pPassMnger->duInfo, gcvFALSE);
    }

    /* patch dual16 shader */
    {
        errCode = vscVIR_PatchDual16Shader(pShader, &pPassMnger->duInfo);
        ON_ERROR(errCode, "Patch dual16 shader");
    }

    /* Try to build web if it is not built yet */
    errCode = vscVIR_BuildWebs(&pPassMnger->callGraph, &pPassMnger->duInfo, gcvFALSE);
    ON_ERROR(errCode, "Build webs");

    /* Liveness analysis must be before IS & RA */
    errCode = vscVIR_BuildLivenessInfo(&pPassMnger->callGraph,
                                       &pPassMnger->lvInfo,
                                       &pPassMnger->duInfo);
    ON_ERROR(errCode, "Build LV");

    /* Do instruction scheduling before temp-reg RA*/
    {
        VSC_OPTN_ISOptions* is_options = VSC_OPTN_Options_GetPreRAISOptions(pPassMnger->options);
        if (VSC_OPTN_ISOptions_GetSwitchOn(is_options))
        {
            errCode = VSC_IS_InstSched_PerformOnShader(pShader, pPassMnger->pCompilerParam->cfg.pHwCfg,
                                                       &pPassMnger->duInfo, &pPassMnger->lvInfo,
                                                       is_options, dumper);
            ON_ERROR(errCode, "Pre-pass inst sked");
        }
    }

    /* Do temp-register allocation */
    {
        VSC_OPTN_RAOptions* ra_options = VSC_OPTN_Options_GetRAOptions(pPassMnger->options);
        if (VSC_OPTN_RAOptions_GetSwitchOn(ra_options))
        {
            errCode = VIR_RA_LS_PerformTempRegAlloc(pShader, pPassMnger->pCompilerParam->cfg.pHwCfg,
                                                    &pPassMnger->lvInfo, ra_options, dumper,
                                                    &pPassMnger->callGraph);
            ON_ERROR(errCode, "Temp-reg RA");

            bRAed = gcvTRUE;
        }
    }

    /* Do instruction scheduling after temp-reg RA*/
    {
        VSC_OPTN_ISOptions* is_options = VSC_OPTN_Options_GetPostRAISOptions(pPassMnger->options);
        if (VSC_OPTN_ISOptions_GetSwitchOn(is_options))
        {
            errCode = VSC_IS_InstSched_PerformOnShader(pShader, pPassMnger->pCompilerParam->cfg.pHwCfg,
                                                       &pPassMnger->duInfo, &pPassMnger->lvInfo,
                                                       is_options, dumper);

            ON_ERROR(errCode, "Post-pass inst sked");
        }
    }

    /* Do final cleanup phase */
    {
        /* this phase is to replace/remove ldarr. when RA is enabled, we don't need this phase */
        VSC_OPTN_FCPOptions* fcp_options = VSC_OPTN_Options_GetFCPOptions(pPassMnger->options);
        if (VSC_OPTN_FCPOptions_GetSwitchOn(fcp_options))
        {
            errCode = VIR_FCP_PerformOnShader(pShader, &pPassMnger->duInfo, fcp_options, dumper);
            ON_ERROR(errCode, "Final Cleanup Phase");
        }
    }

    /* Do machine code generation */
    {
        VSC_OPTN_MCGenOptions* mcgen_options = VSC_OPTN_Options_GetMCGenOptions(pPassMnger->options);
        if (VSC_OPTN_MCGenOptions_GetSwitchOn(mcgen_options))
        {
            errCode = VSC_MC_GEN_MachineCodeGen(pShader, pPassMnger->pCompilerParam->cfg.pHwCfg,
                                                mcgen_options, dumper);
            ON_ERROR(errCode, "MC codegen");

            bMCGened = gcvTRUE;
        }
    }

    if (pOutSEP)
    {
        if (bRAed && bMCGened)
        {
            errCode = vscVIR_GenerateSEP(pShader, pPassMnger->pCompilerParam->cfg.pHwCfg, gcvFALSE, pOutSEP);
        }
        else
        {
            /* Oops, why you ask me to generate SEP but you did not set mc-gen and RA?? */
            gcmASSERT(gcvFALSE);
            errCode = VSC_ERR_INVALID_ARGUMENT;
        }

        ON_ERROR(errCode, "SEP generation");
    }

OnError:
    return errCode;
}

static VIR_ShLevel _GetExpectedLastLevel(VSC_SHADER_COMPILER_PARAM* pCompilerParam)
{
    if (pCompilerParam->cfg.cFlags & VSC_COMPILER_FLAG_COMPILE_TO_MC)
    {
        return VIR_SHLEVEL_Post_Machine;
    }
    else if (pCompilerParam->cfg.cFlags & VSC_COMPILER_FLAG_COMPILE_TO_LL)
    {
        return VIR_SHLEVEL_Post_Low;
    }
    else if (pCompilerParam->cfg.cFlags & VSC_COMPILER_FLAG_COMPILE_TO_ML)
    {
        return VIR_SHLEVEL_Post_Medium;
    }
    else if (pCompilerParam->cfg.cFlags & VSC_COMPILER_FLAG_COMPILE_TO_HL)
    {
        return VIR_SHLEVEL_Post_High;
    }
    else
    {
        return VIR_SHLEVEL_Unknown;
    }
}

#define NEED_HL_TRANSFORMS(pShader, expectedLastLevel)                             \
    ((expectedLastLevel) >= VIR_SHLEVEL_Post_High &&                               \
     VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Pre_High)

#define NEED_ML_TRANSFORMS(pShader, expectedLastLevel)                             \
    ((expectedLastLevel) >= VIR_SHLEVEL_Post_Medium &&                             \
     (VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Pre_Medium ||                  \
      VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Post_High))

#define NEED_LL_TRANSFORMS(pShader, expectedLastLevel)                             \
    ((expectedLastLevel) >= VIR_SHLEVEL_Post_Low &&                                \
     (VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Pre_Low ||                     \
      VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Post_Medium))

#define NEED_MC_TRANSFORMS(pShader, expectedLastLevel)                             \
    ((expectedLastLevel) >= VIR_SHLEVEL_Post_Machine &&                            \
     (VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Pre_Machine ||                 \
      VIR_Shader_GetLevel((pShader)) == VIR_SHLEVEL_Post_Low))

#define CAN_PERFORM_CODE_GEN(pCompilerParam)                                       \
    (VIR_Shader_GetLevel((pCompilerParam)->pShader) == VIR_SHLEVEL_Post_Machine && \
     ((pCompilerParam)->cfg.cFlags & VSC_COMPILER_FLAG_COMPILE_CODE_GEN))

VSC_ErrCode _CompileShaderInternal(VSC_PASS_MANAGER*          pPassMnger,
                                   SHADER_EXECUTABLE_PROFILE* pOutSEP)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Shader*         pShader = pPassMnger->pCompilerParam->pShader;
    VIR_ShLevel         expectedLastLevel = _GetExpectedLastLevel(pPassMnger->pCompilerParam);

    errCode = _PreprocessShader(pPassMnger);
    ON_ERROR(errCode, "Preprocess shader");

    /* HL compiling */
    if (NEED_HL_TRANSFORMS(pShader, expectedLastLevel))
    {
        errCode = _CompileShaderAtHighLevel(pPassMnger);
        ON_ERROR(errCode, "HL compiling");
    }

    /* ML compiling */
    if (NEED_ML_TRANSFORMS(pShader, expectedLastLevel))
    {
        errCode = _CompileShaderAtMedLevel(pPassMnger);
        ON_ERROR(errCode, "ML compiling");
    }

    /* LL compiling */
    if (NEED_LL_TRANSFORMS(pShader, expectedLastLevel))
    {
        errCode = _CompileShaderAtLowLevel(pPassMnger);
        ON_ERROR(errCode, "LL compiling");
    }

    /* MC compiling */
    if (NEED_MC_TRANSFORMS(pShader, expectedLastLevel))
    {
        errCode = _CompileShaderAtMCLevel(pPassMnger);
        ON_ERROR(errCode, "MC compiling");
    }

    /* Perform codegen */
    if (CAN_PERFORM_CODE_GEN(pPassMnger->pCompilerParam))
    {
        errCode = _PerformCodegen(pPassMnger, pOutSEP);
        ON_ERROR(errCode, "Perform codegen");
    }

    errCode = _PostprocessShader(pPassMnger, pOutSEP);
    ON_ERROR(errCode, "Post-process shader");

OnError:
    return errCode;
}

gceSTATUS vscCompileShader(VSC_SHADER_COMPILER_PARAM* pCompilerParam,
                           SHADER_EXECUTABLE_PROFILE* pOutSEP)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VSC_PASS_MANAGER    passMnger;

    /* Initialize our pass manager who will take over all passes
       whether to trigger or not */
    vscInitializePassManager(&passMnger, pCompilerParam);

    /* Do real compiling jobs */
    errCode = _CompileShaderInternal(&passMnger, pOutSEP);
    ON_ERROR(errCode, "Compiler internal");

OnError:
    vscFinalizePassManager(&passMnger);

    return vscERR_CastErrCode2GcStatus(errCode);
}

gceSTATUS vscExtractKernel(VIR_Shader*     pKernelProgram,
                           gctCONST_STRING pKernelName,
                           VIR_Shader*     pOutKernel)
{
    return gcvSTATUS_OK;
}


