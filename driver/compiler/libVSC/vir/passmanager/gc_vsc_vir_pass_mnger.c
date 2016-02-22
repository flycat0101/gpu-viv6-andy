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

static void _PrepareUserOptions(VSC_PASS_MANAGER* pPassMnger, gctUINT64 optFlags)
{
    VIR_Dumper*         dumper = pPassMnger->passWorker.pCompilerParam->pShader->dumper;

    pPassMnger->passWorker.pOptions = VSC_OPTN_Get_Options();

    if (VSC_OPTN_Options_GetOptionsUsage(pPassMnger->passWorker.pOptions))
    {
        VSC_OPTN_Options_Usage(dumper);
    }

    if (VSC_OPTN_Options_GetDumpOptions(pPassMnger->passWorker.pOptions))
    {
        VSC_OPTN_Options_Dump(pPassMnger->passWorker.pOptions, dumper);
    }
}

void vscInitializePassManager(VSC_PASS_MANAGER* pPassMnger,
                              VSC_SHADER_COMPILER_PARAM* pCompilerParam)
{
    gcoOS_ZeroMemory(pPassMnger, sizeof(VSC_PASS_MANAGER));

    pPassMnger->passWorker.pCompilerParam = pCompilerParam;
    pCompilerParam->pShader->passMnger = pPassMnger;
    _PrepareUserOptions(pPassMnger, pCompilerParam->cfg.optFlags);
}

void vscFinalizePassManager(VSC_PASS_MANAGER* pPassMnger)
{
    if (pPassMnger->passWorker.pCompilerParam)
    {
        /* Destroy liveness information */
        vscVIR_DestroyLivenessInfo(&pPassMnger->lvInfo);

        /* Destroy D-U analysis */
        vscVIR_DestroyDefUsageInfo(&pPassMnger->duInfo);

        /* Destroy CG/CFG */
        vscVIR_DestroyCFG(pPassMnger->passWorker.pCompilerParam->pShader);
        vscVIR_DestroyCallGraph(&pPassMnger->callGraph);
    }
}

