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


#include "gc_vsc.h"

extern VSC_OPTN_HWOptions* hw_options;

static void _PrepareUserOptions(VSC_PASS_MANAGER* pPassMnger, gctUINT64 optFlags)
{
    VIR_Dumper*         dumper = pPassMnger->pCompilerParam->pShader->dumper;

    pPassMnger->options = VSC_OPTN_Get_Options();
    if(VSC_OPTN_Options_GetOptionsUsage(pPassMnger->options))
    {
        VSC_OPTN_Options_Usage(dumper);
    }
    if(VSC_OPTN_Options_GetDumpOptions(pPassMnger->options))
    {
        VSC_OPTN_Options_Dump(pPassMnger->options, dumper);
    }
    hw_options = VSC_OPTN_Options_GetHWOptions(pPassMnger->options);
}

void vscInitializePassManager(VSC_PASS_MANAGER* pPassMnger,
                              VSC_SHADER_COMPILER_PARAM* pCompilerParam)
{
    gcoOS_ZeroMemory(pPassMnger, sizeof(VSC_PASS_MANAGER));

    pPassMnger->pCompilerParam = pCompilerParam;
    pCompilerParam->pShader->passMnger = pPassMnger;
    _PrepareUserOptions(pPassMnger, pCompilerParam->cfg.optFlags);
}

void vscFinalizePassManager(VSC_PASS_MANAGER* pPassMnger)
{
    if (pPassMnger->pCompilerParam)
    {
        /* Destroy liveness information */
        vscVIR_DestroyLivenessInfo(&pPassMnger->lvInfo);

        /* Destroy D-U analysis */
        vscVIR_DestroyDefUsageInfo(&pPassMnger->duInfo);

        /* Destroy CG/CFG */
        vscVIR_DestroyCFG(pPassMnger->pCompilerParam->pShader);
        vscVIR_DestroyCallGraph(&pPassMnger->callGraph);
    }
}

