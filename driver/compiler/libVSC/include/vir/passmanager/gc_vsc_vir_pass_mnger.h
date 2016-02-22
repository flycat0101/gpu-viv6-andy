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


#ifndef __gc_vsc_vir_pass_mnger_h_
#define __gc_vsc_vir_pass_mnger_h_

BEGIN_EXTERN_C()

typedef enum VSC_PASS_MEMPOOL_SEL
{
    VSC_PASS_MEMPOOL_SEL_AUTO = 0,
    VSC_PASS_MEMPOOL_SEL_PMP,
    VSC_PASS_MEMPOOL_SEL_BMS,
    VSC_PASS_MEMPOOL_SEL_AMS
}VSC_PASS_MEMPOOL_SEL;

typedef struct _VSC_PASS_WORKER
{
    VSC_SHADER_COMPILER_PARAM* pCompilerParam;

    /* User options controlling each pass */
    VSC_OPTN_Options*          pOptions;

    /* References from pass-manager for current pass. If pass does not need it,
       set it to NULL */
    VIR_CALL_GRAPH*            pCallGraph;
    VIR_DEF_USAGE_INFO*        pDuInfo;
    VIR_LIVENESS_INFO*         pLvInfo;

    /* Real used mem pool based on VSC_PASS_MEMPOOL_SEL */
    VSC_MM*                    pMM;
}VSC_PASS_WORKER;

typedef struct _VSC_PASS_FLAG
{
    gctUINT                    bNeedCfg   : 1;
    gctUINT                    bNeedDu    : 1;
    gctUINT                    bNeedLv    : 1;
    gctUINT                    memPoolSel : 2;

    gctUINT                    reserved   : 27;
}VSC_PASS_FLAG;

/* A pass-manager is used to take over scheduling of all passes to get best opt result
   with minimum CPU time/memory-footprint.

   Note that, normally, a PM */
typedef struct _VSC_PASS_MANAGER
{
    /* A pass worker that is used by each pass. Every pass can not see content of pass
       manager */
    VSC_PASS_WORKER            passWorker;

    /* Control related analysis info */
    VIR_CALL_GRAPH             callGraph;

    /* Global data related analysis info */
    VIR_DEF_USAGE_INFO         duInfo;
    VIR_LIVENESS_INFO          lvInfo;

    /* Memory pools that pass can use to store pass internal data */
    VSC_PRIMARY_MEM_POOL       pmp;
    VSC_BUDDY_MEM_SYS          bms;
    VSC_ARENA_MEM_SYS          ams;
}VSC_PASS_MANAGER;

/* Pass prototype */
typedef VSC_ErrCode (*PFN_PASS_ROUTINE)(VSC_PASS_WORKER* pPassWorker);

/* Functions */
void vscInitializePassManager(VSC_PASS_MANAGER* pPassMnger,
                              VSC_SHADER_COMPILER_PARAM* pCompilerParam);

void vscFinalizePassManager(VSC_PASS_MANAGER* pPassMnger);

VSC_ErrCode vscBeginPass(VSC_PASS_MANAGER* pPassMnger, VSC_PASS_FLAG passFlag, VSC_PASS_WORKER** ppPassWorker);
VSC_ErrCode vscEndPass(VSC_PASS_MANAGER* pPassMnger, VSC_PASS_WORKER* pPassWorker);

#define VSC_BEGIN_PASS(passFlag) errCode = vscBeginPass(pPassMnger, passFlag, &pPassWorker); \
                                 if (pPassWorker == gcvNULL && errCode == VSC_ERR_NONE) return VSC_ERR_NONE;
#define VSC_END_PASS(pPassWorker) vscEndPass(pPassMnger, pPassWorker)

END_EXTERN_C()

#endif /* __gc_vsc_vir_pass_mnger_h_ */

