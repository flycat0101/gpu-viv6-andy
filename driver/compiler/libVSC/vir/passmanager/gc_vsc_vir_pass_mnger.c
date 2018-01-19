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


#include "gc_vsc.h"

#define PG_PMP_LOW_LIMIT_OF_CHUNK_SIZE  (512)
#define SH_PMP_LOW_LIMIT_OF_CHUNK_SIZE  (1024)

void vscInitializeOptions(VSC_OPTN_Options* pOptions,
                          VSC_HW_CONFIG* pHwCfg,
                          gctUINT cFlags,
                          gctUINT64 optFlags)
{
    /* set default optimization level to 2 */
    gctUINT optLevel = 2;

    gcmASSERT(pOptions);

    /* Initialize options. */
    gcoOS_ZeroMemory(pOptions, gcmSIZEOF(VSC_OPTN_Options));

    /* Options Priority(low to high):
    ** 1) Set the default option value.
    ** 2) Set the options from compile flags.
    ** 3) Set the options from option flags.
    ** 4) Set some special options, e.g. SW/HW limitations.
    ** 5) Get the ENV options.
    */

    /* if optimization level is set in options, get it here */
    VSC_OPTN_Options_GetOptLevelFromEnv(&optLevel);

    /* Set the default options. */
    VSC_OPTN_Options_SetDefault(pOptions, optLevel);

    /* Set the options by compile flags. */
    VSC_OPTN_Options_SetOptionsByCompileFlags(pOptions, cFlags);

    /* Set the options by Optflags. */
    VSC_OPTN_Options_SetOptionsByOptFlags(pOptions, optFlags);

    /* Set some special options, e.g. SW/HW limitations. */
    VSC_OPTN_Options_SetSpecialOptions(pOptions, pHwCfg);

    /* Get the options from env if existed. */
    VSC_OPTN_Options_GetOptionFromEnv(pOptions);

    /* Merge VC_OPTION from env if existed. */
    VSC_OPTN_Options_MergeVCEnvOption(pOptions);
}

void vscFinalizeOptions(VSC_OPTN_Options* pOptions)
{
}

void vscInitializePassMMPool(VSC_PASS_MM_POOL* pMmPool)
{
    gcoOS_ZeroMemory(pMmPool, sizeof(VSC_PASS_MM_POOL));
}

void vscFinalizePassMMPool(VSC_PASS_MM_POOL* pMmPool)
{
    vscAMS_Finalize(&pMmPool->AMS);
    vscBMS_Finalize(&pMmPool->BMS, gcvFALSE);
    vscPMP_Finalize(&pMmPool->sharedPMP);
    vscPMP_Finalize(&pMmPool->privatePMP);
}

static void _InitializeBPM(VSC_BASE_PASS_MANAGER* pBasePassMnger,
                           VIR_Dumper* pDumper,
                           VSC_OPTN_Options* pOptions,
                           VSC_PM_MODE pmMode)
{
    gcoOS_ZeroMemory(pBasePassMnger, sizeof(VSC_BASE_PASS_MANAGER));

    pBasePassMnger->pOptions = pOptions;
    pBasePassMnger->pDumper = pDumper;
    pBasePassMnger->pmMode = pmMode;
}

static void _FinalizeBPM(VSC_BASE_PASS_MANAGER* pBasePassMnger)
{
    /* Nothing to do */
}

void vscPM_SetCurPassLevel(VSC_BASE_PASS_MANAGER* pBasePassMnger, VSC_PASS_LEVEL passLevel)
{
    pBasePassMnger->curPassLevel = passLevel;
}

void vscSPM_Initialize(VSC_SHADER_PASS_MANAGER* pShPassMnger,
                       VSC_SHADER_COMPILER_PARAM* pCompilerParam,
                       VSC_PASS_MM_POOL* pMmPool,
                       gctBOOL bInitSharedPMP,
                       VIR_Dumper* pDumper,
                       VSC_OPTN_Options* pOptions,
                       VSC_PM_MODE pmMode)
{
    VIR_Shader*  pShader = (VIR_Shader*)pCompilerParam->hShader;

    gcoOS_ZeroMemory(pShPassMnger, sizeof(VSC_SHADER_PASS_MANAGER));

    _InitializeBPM(&pShPassMnger->basePM, pDumper, pOptions, pmMode);

    pShPassMnger->pCompilerParam = pCompilerParam;
    pShPassMnger->pMmPool = pMmPool;

    if (bInitSharedPMP && !vscPMP_IsInitialized(&pShPassMnger->pMmPool->sharedPMP))
    {
        vscPMP_Intialize(&pShPassMnger->pMmPool->sharedPMP, gcvNULL,
                         SH_PMP_LOW_LIMIT_OF_CHUNK_SIZE, sizeof(void *), gcvTRUE);
    }

    /* TODO: Will reconsider followings later */
    pShader->pCompilerCfg = &pCompilerParam->cfg;
    VIR_Shader_SetDumpOptions(pShader, VSC_OPTN_Options_GetDumpOptions(pOptions));

    if (pCompilerParam->cfg.ctx.isPatchLib)
    {
        VIR_Shader_SetFlag(pShader, VIR_SHFLAG_PATCH_LIB);
    }
}

void vscSPM_Finalize(VSC_SHADER_PASS_MANAGER* pShPassMnger, gctBOOL bFinalizeSharedPMP)
{
    if (pShPassMnger->pCompilerParam)
    {
        /* Destroy liveness information */
        vscVIR_DestroyLivenessInfo(&pShPassMnger->passRes.lvInfo);

        /* Destroy D-U analysis */
        vscVIR_DestroyDefUsageInfo(&pShPassMnger->passRes.duInfo);

        /* Destroy CG/CFG */
        vscVIR_DestroyCFG((VIR_Shader*)pShPassMnger->pCompilerParam->hShader);
        vscVIR_DestroyCallGraph(&pShPassMnger->passRes.callGraph);
    }

    if (bFinalizeSharedPMP)
    {
        vscPMP_Finalize(&pShPassMnger->pMmPool->sharedPMP);
    }

    _FinalizeBPM(&pShPassMnger->basePM);
}

static void _InitializeBPPM(VSC_BASE_PG_PASS_MANAGER* pBasePPM,
                            VIR_Dumper* pDumper,
                            VSC_OPTN_Options* pOptions,
                            VSC_PM_MODE pmMode)
{
    _InitializeBPM(&pBasePPM->basePM, pDumper, pOptions, pmMode);
    vscInitializePassMMPool(&pBasePPM->pgMmPool);
    vscInitializePassMMPool(&pBasePPM->shMmPool);

    /* For program pass manager, intialize shared PMP directly */
    vscPMP_Intialize(&pBasePPM->pgMmPool.sharedPMP, gcvNULL,
                     PG_PMP_LOW_LIMIT_OF_CHUNK_SIZE, sizeof(void *), gcvTRUE);
}

static void _FinalizeBPPM(VSC_BASE_PG_PASS_MANAGER* pBasePPM)
{
    vscFinalizePassMMPool(&pBasePPM->pgMmPool);
    vscFinalizePassMMPool(&pBasePPM->shMmPool);
    _FinalizeBPM(&pBasePPM->basePM);
}

void vscGPPM_Initialize(VSC_GPG_PASS_MANAGER* pPgPassMnger,
                        VSC_PROGRAM_LINKER_PARAM* pPgLinkerParam,
                        VIR_Dumper* pDumper,
                        VSC_OPTN_Options* pOptions,
                        VSC_PM_MODE pmMode)
{
    gcoOS_ZeroMemory(pPgPassMnger, sizeof(VSC_GPG_PASS_MANAGER));

    pPgPassMnger->pPgmLinkerParam = pPgLinkerParam;

    _InitializeBPPM(&pPgPassMnger->basePgmPM, pDumper, pOptions, pmMode);
}

void vscGPPM_Finalize(VSC_GPG_PASS_MANAGER* pPgPassMnger)
{
    if (pPgPassMnger->pPgmLinkerParam)
    {
        _FinalizeBPPM(&pPgPassMnger->basePgmPM);
    }
}

void vscGPPM_SetPassRes(VSC_GPG_PASS_MANAGER* pPgPassMnger,
                        VSC_SHADER_PASS_RES** ppShPassResArray)
{
    memcpy(pPgPassMnger->pShPassResArray, ppShPassResArray,
           sizeof(VSC_SHADER_PASS_RES*)*VSC_MAX_SHADER_STAGE_COUNT);
}

void vscKPPM_Initialize(VSC_KPG_PASS_MANAGER* pPgPassMnger,
                        VSC_KERNEL_PROGRAM_LINKER_PARAM* pKrnlPgLinkParam,
                        VIR_Dumper* pDumper,
                        VSC_OPTN_Options* pOptions,
                        VSC_PM_MODE pmMode)
{
     pPgPassMnger->pKrnlPgLinkParam = pKrnlPgLinkParam;

     _InitializeBPPM(&pPgPassMnger->basePgmPM, pDumper, pOptions, pmMode);
}

void vscKPPM_Finalize(VSC_KPG_PASS_MANAGER* pPgPassMnger)
{
    if (pPgPassMnger->pKrnlPgLinkParam)
    {
        _FinalizeBPPM(&pPgPassMnger->basePgmPM);
    }
}

static gctBOOL _Gate(VSC_BASE_PASS_WORKER* pBasePassWorker)
{
    return (pBasePassWorker->pBaseOption == gcvNULL || /* Pass has no option provided, always assume gated */
            pBasePassWorker->pBaseOption->switch_on);  /* Pass is switched on */
}

static VSC_ErrCode _DestroyShaderPassResources(VSC_PASS_PROPERTY* pPassProp,
                                               VIR_Shader** ppShaderArray,
                                               VSC_SHADER_PASS_RES** ppShaderPassResArray,
                                               VSC_PASS_RES_DESTROY_REQ_FLAG* pPerShResDestroyReqArray,
                                               gctUINT shaderCount,
                                               VSC_PASS_RES_DESTROY_REQ_FLAG* pGlobalResDestroyReq)
{
    VSC_ErrCode                   errCode = VSC_ERR_NONE;
    gctUINT                       i;
    gctBOOL                       bNeedDestroyCg, bNeedDestroyCfg, bNeedDestroyRdFlow, bNeedDestroyDu;
    gctBOOL                       bNeedDestroyWeb, bNeedDestroyLvFlow;
    VSC_PASS_RES_DESTROY_REQ_FLAG resDestroyReq;

    for (i = 0; i < shaderCount; i ++)
    {
        if (ppShaderArray[i] == gcvNULL)
        {
            continue;
        }

        resDestroyReq.data = pGlobalResDestroyReq->data | pPerShResDestroyReqArray[i].data;

        bNeedDestroyCg = resDestroyReq.s.bInvalidateCg && !resDestroyReq.s.bCgUnmodified;

        bNeedDestroyCfg = ((resDestroyReq.s.bInvalidateCg && !resDestroyReq.s.bCgUnmodified) ||
                            (resDestroyReq.s.bInvalidateCfg && !resDestroyReq.s.bCfgUnmodified));

        bNeedDestroyRdFlow = ((resDestroyReq.s.bInvalidateCg && !resDestroyReq.s.bCgUnmodified) ||
                                (resDestroyReq.s.bInvalidateCfg && !resDestroyReq.s.bCfgUnmodified) ||
                                (resDestroyReq.s.bInvalidateRdFlow && !resDestroyReq.s.bRdFlowUnmodified));

        bNeedDestroyDu = ((resDestroyReq.s.bInvalidateCg && !resDestroyReq.s.bCfgUnmodified) ||
                            (resDestroyReq.s.bInvalidateCfg && !resDestroyReq.s.bCfgUnmodified) ||
                            (resDestroyReq.s.bInvalidateRdFlow && !resDestroyReq.s.bRdFlowUnmodified) ||
                            (resDestroyReq.s.bInvalidateDu && !resDestroyReq.s.bDuUnmodified));

        bNeedDestroyWeb = ((resDestroyReq.s.bInvalidateCg && !resDestroyReq.s.bCgUnmodified) ||
                            (resDestroyReq.s.bInvalidateCfg && !resDestroyReq.s.bCfgUnmodified) ||
                            (resDestroyReq.s.bInvalidateRdFlow && !resDestroyReq.s.bRdFlowUnmodified) ||
                            (resDestroyReq.s.bInvalidateDu && !resDestroyReq.s.bDuUnmodified) ||
                            (resDestroyReq.s.bInvalidateWeb && !resDestroyReq.s.bWebUnmodified));

        bNeedDestroyLvFlow = ((resDestroyReq.s.bInvalidateCg && !resDestroyReq.s.bCgUnmodified) ||
                            (resDestroyReq.s.bInvalidateCfg && !resDestroyReq.s.bCfgUnmodified) ||
                            (resDestroyReq.s.bInvalidateRdFlow && !resDestroyReq.s.bRdFlowUnmodified) ||
                            (resDestroyReq.s.bInvalidateDu && !resDestroyReq.s.bDuUnmodified) ||
                            (resDestroyReq.s.bInvalidateLvFlow && !resDestroyReq.s.bLvFlowUnmodified));

        if (bNeedDestroyLvFlow)
        {
            errCode = vscVIR_DestroyLivenessInfo(&ppShaderPassResArray[i]->lvInfo);
            ON_ERROR(errCode, "Destroy LV");
        }

        if (bNeedDestroyWeb)
        {
            errCode = vscVIR_DestoryWebs(&ppShaderPassResArray[i]->duInfo);
            ON_ERROR(errCode, "Destroy Web");
        }

        if (bNeedDestroyDu)
        {
            errCode = vscVIR_DestoryDUUDChain(&ppShaderPassResArray[i]->duInfo);
            ON_ERROR(errCode, "Destroy DU");
        }

        if (bNeedDestroyRdFlow)
        {
            errCode = vscVIR_DestroyDefUsageInfo(&ppShaderPassResArray[i]->duInfo);
            ON_ERROR(errCode, "Destroy RD flow");
        }

        if (bNeedDestroyCfg)
        {
            errCode = vscVIR_DestroyCFG(ppShaderArray[i]);
            ON_ERROR(errCode, "Destroy CFG");
        }

        if (bNeedDestroyCg)
        {
            errCode = vscVIR_DestroyCallGraph(&ppShaderPassResArray[i]->callGraph);
            ON_ERROR(errCode, "Destroy CG");
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _CreateShaderPassResources(VSC_PASS_PROPERTY* pPassProp,
                                              VIR_Shader** ppShaderArray,
                                              VSC_SHADER_PASS_RES** ppShaderPassResArray,
                                              gctUINT shaderCount)
{
    VSC_ErrCode                     errCode = VSC_ERR_NONE;
    gctUINT                         i;
    gctBOOL                         bNeedBuildCg, bNeedBuildCfg, bNeedSSAForm;
    gctBOOL                         bNeedBuildRdFlow, bNeedBuildDu, bNeedBuildWeb, bNeedBuildLvFlow;
    gctBOOL                         bNeedDestroyDFARes = gcvFALSE;
    VSC_PASS_RES_DESTROY_REQ_FLAG   resDestroyReq;

    bNeedBuildCg = (pPassProp->passFlag.resCreationReq.s.bNeedCg || pPassProp->passFlag.resCreationReq.s.bNeedCfg ||
                    pPassProp->passFlag.resCreationReq.s.bNeedRdFlow || pPassProp->passFlag.resCreationReq.s.bNeedDu ||
                    pPassProp->passFlag.resCreationReq.s.bNeedWeb || pPassProp->passFlag.resCreationReq.s.bNeedLvFlow);

    bNeedBuildCfg = (pPassProp->passFlag.resCreationReq.s.bNeedCfg || pPassProp->passFlag.resCreationReq.s.bNeedRdFlow ||
                     pPassProp->passFlag.resCreationReq.s.bNeedDu || pPassProp->passFlag.resCreationReq.s.bNeedWeb ||
                     pPassProp->passFlag.resCreationReq.s.bNeedLvFlow);

    bNeedBuildRdFlow = (pPassProp->passFlag.resCreationReq.s.bNeedRdFlow || pPassProp->passFlag.resCreationReq.s.bNeedDu ||
                        pPassProp->passFlag.resCreationReq.s.bNeedWeb || pPassProp->passFlag.resCreationReq.s.bNeedLvFlow);

    bNeedBuildDu = (pPassProp->passFlag.resCreationReq.s.bNeedDu || pPassProp->passFlag.resCreationReq.s.bNeedWeb ||
                    pPassProp->passFlag.resCreationReq.s.bNeedLvFlow);

    bNeedBuildWeb = pPassProp->passFlag.resCreationReq.s.bNeedWeb;

    bNeedBuildLvFlow = pPassProp->passFlag.resCreationReq.s.bNeedLvFlow;

    bNeedSSAForm = pPassProp->passFlag.resCreationReq.s.bNeedSSAForm;

    for (i = 0; i < shaderCount; i ++)
    {
        if (ppShaderArray[i] == gcvNULL)
        {
            continue;
        }

        if (bNeedSSAForm)
        {
            if (!VIR_Shader_BySSAForm(ppShaderArray[i]))
            {
                errCode = vscVIR_Transform2SSA(ppShaderArray[i]);
                ON_ERROR(errCode, "Tranform to SSA");

                /* Form changing needs dfa be invalidated */
                bNeedDestroyDFARes = gcvTRUE;
            }
        }
        else
        {
            if (VIR_Shader_BySSAForm(ppShaderArray[i]))
            {
                errCode = vscVIR_TransformFromSSA(ppShaderArray[i]);
                ON_ERROR(errCode, "Tranform from SSA");

                /* Form changing needs dfa be invalidated */
                bNeedDestroyDFARes = gcvTRUE;
            }
            else if (VIR_Shader_BySpvSSAForm(ppShaderArray[i]))
            {
                errCode = vscVIR_TransformFromSpvSSA(ppShaderArray[i]);
                ON_ERROR(errCode, "Tranform from Spirv SSA");

                /* Form changing needs dfa be invalidated */
                bNeedDestroyDFARes = gcvTRUE;
            }
        }

        if (bNeedDestroyDFARes)
        {
            resDestroyReq.data = 0;
            resDestroyReq.s.bInvalidateDu = gcvTRUE;
            resDestroyReq.s.bInvalidateLvFlow = gcvTRUE;
            resDestroyReq.s.bInvalidateRdFlow = gcvTRUE;
            resDestroyReq.s.bInvalidateWeb = gcvTRUE;

            errCode = _DestroyShaderPassResources(pPassProp, &ppShaderArray[i], &ppShaderPassResArray[i],
                                                  &resDestroyReq, 1, &resDestroyReq);
            ON_ERROR(errCode, "Destroy shader pass resources");
        }

        if (bNeedBuildCg && !vscVIR_IsCallGraphBuilt(&ppShaderPassResArray[i]->callGraph))
        {
            errCode = vscVIR_BuildCallGraph(ppShaderArray[i], &ppShaderPassResArray[i]->callGraph);
            ON_ERROR(errCode, "Build call graph");
        }

        if (bNeedBuildCfg && !vscVIR_IsCFGBuilt(ppShaderArray[i]))
        {
            errCode = vscVIR_BuildCFG(ppShaderArray[i]);
            ON_ERROR(errCode, "Build CFG");
        }

        if (bNeedBuildRdFlow && !vscVIR_CheckDFAFlowBuilt(&ppShaderPassResArray[i]->duInfo.baseTsDFA.baseDFA))
        {
            errCode = vscVIR_BuildDefUsageInfo(&ppShaderPassResArray[i]->callGraph, &ppShaderPassResArray[i]->duInfo, gcvFALSE, gcvFALSE);
            ON_ERROR(errCode, "Build RD flow");
        }

        if (bNeedBuildDu && !ppShaderPassResArray[i]->duInfo.bDUUDChainBuilt)
        {
            errCode = vscVIR_BuildDUUDChain(&ppShaderPassResArray[i]->callGraph, &ppShaderPassResArray[i]->duInfo, gcvFALSE);
            ON_ERROR(errCode, "Build DUUD chain");
        }

        if (bNeedBuildWeb && !ppShaderPassResArray[i]->duInfo.bWebTableBuilt)
        {
            errCode = vscVIR_BuildWebs(&ppShaderPassResArray[i]->callGraph, &ppShaderPassResArray[i]->duInfo, gcvFALSE);
            ON_ERROR(errCode, "Build webs");
        }

        if (bNeedBuildLvFlow && !vscVIR_CheckDFAFlowBuilt(&ppShaderPassResArray[i]->lvInfo.baseTsDFA.baseDFA))
        {
            errCode = vscVIR_BuildLivenessInfo(&ppShaderPassResArray[i]->callGraph,
                                               &ppShaderPassResArray[i]->lvInfo,
                                               &ppShaderPassResArray[i]->duInfo);
            ON_ERROR(errCode, "Build LV");
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _InitMemPool(VSC_PASS_PROPERTY* pPassProp,
                               VSC_PASS_MM_POOL* pMmPool)
{
    VSC_ErrCode          errCode = VSC_ERR_NONE;
    gctBOOL              bNeedSharedPMPInit, bNeedBMSInit, bNeedAMSInit;

    bNeedSharedPMPInit = (pPassProp->memPoolSel == VSC_PASS_MEMPOOL_SEL_AUTO ||
                          pPassProp->memPoolSel == VSC_PASS_MEMPOOL_SEL_AMS ||
                          pPassProp->memPoolSel == VSC_PASS_MEMPOOL_SEL_BMS ||
                          pPassProp->memPoolSel == VSC_PASS_MEMPOOL_SEL_SHARED_PMP);

    bNeedBMSInit = (pPassProp->memPoolSel == VSC_PASS_MEMPOOL_SEL_AUTO ||
                    pPassProp->memPoolSel == VSC_PASS_MEMPOOL_SEL_AMS ||
                    pPassProp->memPoolSel == VSC_PASS_MEMPOOL_SEL_BMS);

    bNeedAMSInit = (pPassProp->memPoolSel == VSC_PASS_MEMPOOL_SEL_AUTO ||
                    pPassProp->memPoolSel == VSC_PASS_MEMPOOL_SEL_AMS);

    if (bNeedSharedPMPInit)
    {
        if (!vscPMP_IsInitialized(&pMmPool->sharedPMP))
        {
            vscPMP_Intialize(&pMmPool->sharedPMP, gcvNULL,
                             SH_PMP_LOW_LIMIT_OF_CHUNK_SIZE, sizeof(void *), gcvTRUE);
        }
    }

    if (bNeedBMSInit)
    {
        if (!vscBMS_IsInitialized(&pMmPool->BMS))
        {
            vscBMS_Initialize(&pMmPool->BMS, &pMmPool->sharedPMP);
        }
    }

    if (bNeedAMSInit)
    {
        if (!vscAMS_IsInitialized(&pMmPool->AMS))
        {
            vscAMS_Initialize(&pMmPool->AMS, &pMmPool->BMS, 1024, sizeof(void *));
        }
        else
        {
            /* Reset it, so we can reuse the mem again */
            vscAMS_Reset(&pMmPool->AMS);
        }
    }

    return errCode;
}

static VSC_ErrCode _FinalizeMemPool(VSC_PASS_PROPERTY* pPassProp,
                                    VSC_PASS_MM_POOL* pMmPool)
{
    VSC_ErrCode          errCode = VSC_ERR_NONE;

    if (pPassProp->memPoolSel == VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP)
    {
        vscPMP_Finalize(&pMmPool->privatePMP);
    }

    return errCode;
}

static VSC_ErrCode _BeginShaderPass(VSC_SHADER_PASS_MANAGER* pShPassMnger,
                                    VSC_PASS_PROPERTY* pPassProp,
                                    gctUINT passId,
                                    void* pPrvData,
                                    VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode          errCode = VSC_ERR_NONE;
    VIR_Shader*          pShader = (VIR_Shader*)pShPassMnger->pCompilerParam->hShader;
    VSC_SHADER_PASS_RES* pPassRes = &pShPassMnger->passRes;

    /* Firstly check gate */
    pPassWorker->basePassWorker.pBaseOption = VSC_OPTN_Options_GetOption(pShPassMnger->basePM.pOptions, pPassProp->passOptionType, passId);
    if (!_Gate(&pPassWorker->basePassWorker))
    {
        return errCode;
    }

    /* Create resources */
    errCode = _CreateShaderPassResources(pPassProp, &pShader, &pPassRes, 1);
    ON_ERROR(errCode, "Create Shader pass resources");

    /* Get resources for current pass */
    if (pPassProp->passFlag.resCreationReq.s.bNeedCg || pPassProp->passFlag.resCreationReq.s.bNeedCfg)
    {
        pPassWorker->pCallGraph = &pShPassMnger->passRes.callGraph;
    }

    if (pPassProp->passFlag.resCreationReq.s.bNeedRdFlow ||
        pPassProp->passFlag.resCreationReq.s.bNeedDu ||
        pPassProp->passFlag.resCreationReq.s.bNeedWeb)
    {
        pPassWorker->pDuInfo = &pShPassMnger->passRes.duInfo;
    }

    if (pPassProp->passFlag.resCreationReq.s.bNeedLvFlow)
    {
        pPassWorker->pLvInfo = &pShPassMnger->passRes.lvInfo;
    }

    errCode = _InitMemPool(pPassProp, pShPassMnger->pMmPool);
    ON_ERROR(errCode, "Init mem pool");

    /* Select mem pool based on pass's request */
    if (pPassProp->memPoolSel == VSC_PASS_MEMPOOL_SEL_AUTO ||
        pPassProp->memPoolSel == VSC_PASS_MEMPOOL_SEL_AMS)
    {
        pPassWorker->basePassWorker.pMM = &pShPassMnger->pMmPool->AMS.mmWrapper;
    }
    else if (pPassProp->memPoolSel == VSC_PASS_MEMPOOL_SEL_BMS)
    {
        pPassWorker->basePassWorker.pMM = &pShPassMnger->pMmPool->BMS.mmWrapper;
    }
    else if (pPassProp->memPoolSel == VSC_PASS_MEMPOOL_SEL_SHARED_PMP)
    {
        pPassWorker->basePassWorker.pMM = &pShPassMnger->pMmPool->sharedPMP.mmWrapper;
    }
    else if (pPassProp->memPoolSel == VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP)
    {
        vscPMP_Intialize(&pShPassMnger->pMmPool->privatePMP, gcvNULL,
                         SH_PMP_LOW_LIMIT_OF_CHUNK_SIZE, sizeof(void *), gcvTRUE);
        pPassWorker->basePassWorker.pMM = &pShPassMnger->pMmPool->privatePMP.mmWrapper;
    }

    pPassWorker->pCompilerParam = pShPassMnger->pCompilerParam;
    pPassWorker->basePassWorker.pDumper = pShPassMnger->basePM.pDumper;
    pPassWorker->basePassWorker.pPrvData = pPrvData;
    pPassWorker->pResDestroyReq = &pPassProp->passFlag.resDestroyReq;

OnError:
    return errCode;
}

static VSC_ErrCode _BeginGpgPass(VSC_GPG_PASS_MANAGER* pPgPassMnger,
                                 VSC_PASS_PROPERTY* pPassProp,
                                 gctUINT passId,
                                 void* pPrvData,
                                 VSC_GPG_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode          errCode = VSC_ERR_NONE;
    gctUINT              i;

    /* Firstly check gate */
    pPassWorker->basePassWorker.pBaseOption = VSC_OPTN_Options_GetOption(pPgPassMnger->basePgmPM.basePM.pOptions,
                                                                         pPassProp->passOptionType, passId);
    if (!_Gate(&pPassWorker->basePassWorker))
    {
        return errCode;
    }

    /* Create resources */
    errCode = _CreateShaderPassResources(pPassProp, (VIR_Shader**)pPgPassMnger->pPgmLinkerParam->hShaderArray,
                                         pPgPassMnger->pShPassResArray, VSC_MAX_SHADER_STAGE_COUNT);
    ON_ERROR(errCode, "Create Shader pass resources");

    /* Get resources for current pass */
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i ++)
    {
        if (pPassProp->passFlag.resCreationReq.s.bNeedCg ||
            pPassProp->passFlag.resCreationReq.s.bNeedCfg ||
            pPassProp->passFlag.resCreationReq.s.bNeedRdFlow ||
            pPassProp->passFlag.resCreationReq.s.bNeedDu ||
            pPassProp->passFlag.resCreationReq.s.bNeedWeb ||
            pPassProp->passFlag.resCreationReq.s.bNeedLvFlow)
        {
            pPassWorker->pShPassResArray[i] = pPgPassMnger->pShPassResArray[i];
        }
    }

    errCode = _InitMemPool(pPassProp, &pPgPassMnger->basePgmPM.pgMmPool);
    ON_ERROR(errCode, "Init mem pool");

    /* Select mem pool based on pass's request */
    if (pPassProp->memPoolSel == VSC_PASS_MEMPOOL_SEL_AUTO ||
        pPassProp->memPoolSel == VSC_PASS_MEMPOOL_SEL_AMS)
    {
        pPassWorker->basePassWorker.pMM = &pPgPassMnger->basePgmPM.pgMmPool.AMS.mmWrapper;
    }
    else if (pPassProp->memPoolSel == VSC_PASS_MEMPOOL_SEL_BMS)
    {
        pPassWorker->basePassWorker.pMM = &pPgPassMnger->basePgmPM.pgMmPool.BMS.mmWrapper;
    }
    else if (pPassProp->memPoolSel == VSC_PASS_MEMPOOL_SEL_SHARED_PMP)
    {
        pPassWorker->basePassWorker.pMM = &pPgPassMnger->basePgmPM.pgMmPool.sharedPMP.mmWrapper;
    }
    else if (pPassProp->memPoolSel == VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP)
    {
        vscPMP_Intialize(&pPgPassMnger->basePgmPM.pgMmPool.privatePMP, gcvNULL,
                         PG_PMP_LOW_LIMIT_OF_CHUNK_SIZE, sizeof(void *), gcvTRUE);
        pPassWorker->basePassWorker.pMM = &pPgPassMnger->basePgmPM.pgMmPool.privatePMP.mmWrapper;
    }

    pPassWorker->pPgmLinkerParam = pPgPassMnger->pPgmLinkerParam;
    pPassWorker->basePassWorker.pDumper = pPgPassMnger->basePgmPM.basePM.pDumper;
    pPassWorker->basePassWorker.pPrvData = pPrvData;

OnError:
    return errCode;
}

static VSC_ErrCode _EndShaderPass(VSC_SHADER_PASS_MANAGER* pShPassMnger,
                                  VSC_PASS_PROPERTY* pPassProp,
                                  VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode          errCode = VSC_ERR_NONE;
    VIR_Shader*          pShader = (VIR_Shader*)pShPassMnger->pCompilerParam->hShader;
    VSC_SHADER_PASS_RES* pPassRes = &pShPassMnger->passRes;

    /* Pass is not gated, just bail out */
    if (!_Gate(&pPassWorker->basePassWorker))
    {
        return errCode;
    }

    /* Destroy resources */
    errCode = _DestroyShaderPassResources(pPassProp, &pShader, &pPassRes,
                                          &pPassProp->passFlag.resDestroyReq, 1,
                                          &pPassProp->passFlag.resDestroyReq);
    ON_ERROR(errCode, "Destroy shader pass resources");

    /* Finalize mem pool */
    errCode = _FinalizeMemPool(pPassProp, pShPassMnger->pMmPool);
    ON_ERROR(errCode, "Finalize mem pool");

OnError:
    return errCode;
}

static VSC_ErrCode _EndGpgPass(VSC_GPG_PASS_MANAGER* pPgPassMnger,
                               VSC_PASS_PROPERTY* pPassProp,
                               VSC_GPG_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode          errCode = VSC_ERR_NONE;

    /* Pass is not gated, just bail out */
    if (!_Gate(&pPassWorker->basePassWorker))
    {
        return errCode;
    }

    /* Destroy resources */
    errCode = _DestroyShaderPassResources(pPassProp, (VIR_Shader**)pPgPassMnger->pPgmLinkerParam->hShaderArray,
                                          pPgPassMnger->pShPassResArray, pPassWorker->resDestroyReqArray,
                                          VSC_MAX_SHADER_STAGE_COUNT, &pPassProp->passFlag.resDestroyReq);
    ON_ERROR(errCode, "Destroy shader pass resources");

    /* Finalize mem pool */
    errCode = _FinalizeMemPool(pPassProp, &pPgPassMnger->basePgmPM.pgMmPool);
    ON_ERROR(errCode, "Finalize mem pool");

OnError:
    return errCode;
}

VSC_ErrCode vscSPM_CallPass(VSC_SHADER_PASS_MANAGER* pShPassMnger,
                            PFN_SH_PASS_ROUTINE pfnPassRoutine,
                            PFN_QUERY_PASS_PROP pfnQueryPassProp,
                            gctUINT passId,
                            void* pPrvData)
{
    VSC_ErrCode        errCode = VSC_ERR_NONE;
    VSC_PASS_PROPERTY  passProp;
    VSC_SH_PASS_WORKER passWorker;

    gcmASSERT(pShPassMnger->basePM.pmMode == VSC_PM_MODE_SEMI_AUTO);

    /* Query the pass-property for this pass */
    memset(&passProp, 0, sizeof(VSC_PASS_PROPERTY));
    pfnQueryPassProp(&passProp, pPrvData);

    /* Only the pass that can support on the level at which pass-manager is managing can go on */
    if (!(passProp.supportedLevels & pShPassMnger->basePM.curPassLevel))
    {
        WARNING_REPORT(errCode, "A pass can not run on expected pass level");
    }

    /* Begin a pass to prepare resources to generate a pass-worker for the run of this pass */
    memset(&passWorker, 0, sizeof(VSC_SH_PASS_WORKER));
    errCode = _BeginShaderPass(pShPassMnger, &passProp, passId, pPrvData, &passWorker);
    ON_ERROR(errCode, "Begin shader pass");

    /* Now run this pass if pass is gated */
    if (_Gate(&passWorker.basePassWorker))
    {
        errCode = pfnPassRoutine(&passWorker);
        ON_ERROR(errCode, "Run shader pass routine");
    }

    /* End a pass to post process resources */
    errCode = _EndShaderPass(pShPassMnger, &passProp, &passWorker);
    ON_ERROR(errCode, "End shader pass");

OnError:
    return errCode;
}

VSC_ErrCode vscGPPM_CallPass(VSC_GPG_PASS_MANAGER* pPgPassMnger,
                             PFN_GPG_PASS_ROUTINE pfnPassRoutine,
                             PFN_QUERY_PASS_PROP pfnQueryPassProp,
                             gctUINT passId,
                             void* pPrvData)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VSC_PASS_PROPERTY   passProp;
    VSC_GPG_PASS_WORKER passWorker;

    gcmASSERT(pPgPassMnger->basePgmPM.basePM.pmMode == VSC_PM_MODE_SEMI_AUTO);

    /* Query the pass-property for this pass */
    memset(&passProp, 0, sizeof(VSC_PASS_PROPERTY));
    pfnQueryPassProp(&passProp, pPrvData);

    /* Only the pass that can support on the level at which pass-manager is managing can go on */
    if (!(passProp.supportedLevels & pPgPassMnger->basePgmPM.basePM.curPassLevel))
    {
        WARNING_REPORT(errCode, "A pass can not run on expected pass level");
    }

    /* Begin a pass to prepare resources to generate a pass-worker for the run of this pass */
    memset(&passWorker, 0, sizeof(VSC_GPG_PASS_WORKER));
    errCode = _BeginGpgPass(pPgPassMnger, &passProp, passId, pPrvData, &passWorker);
    ON_ERROR(errCode, "Begin GPG pass");

    /* Now run this pass if pass is gated */
    if (_Gate(&passWorker.basePassWorker))
    {
        errCode = pfnPassRoutine(&passWorker);
        ON_ERROR(errCode, "Run GPG pass routine");
    }

    /* End a pass to post process resources */
    errCode = _EndGpgPass(pPgPassMnger, &passProp, &passWorker);
    ON_ERROR(errCode, "End GPG pass");

OnError:
    return errCode;
}

void vscSPM_RegisterPass(VSC_SHADER_PASS_MANAGER* pShPassMnger,
                         PFN_SH_PASS_ROUTINE pfnPassRoutine,
                         PFN_QUERY_PASS_PROP pfnQueryPassProp,
                         void* pPrvData)
{
    gcmASSERT(pShPassMnger->basePM.pmMode == VSC_PM_MODE_FULL_AUTO);
}

VSC_ErrCode vscSPM_RunPasses(VSC_SHADER_PASS_MANAGER* pShPassMnger)
{
    gcmASSERT(pShPassMnger->basePM.pmMode == VSC_PM_MODE_FULL_AUTO);

    return VSC_ERR_NONE;
}


