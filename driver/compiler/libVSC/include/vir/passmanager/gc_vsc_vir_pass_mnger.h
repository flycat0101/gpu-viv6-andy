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

typedef enum _VSC_PASS_MEMPOOL_SEL
{
    VSC_PASS_MEMPOOL_SEL_NONE = 0, /* Pass uses its own maintained mem-pool, it is disencouraged! */
    VSC_PASS_MEMPOOL_SEL_AUTO,
    VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP,
    VSC_PASS_MEMPOOL_SEL_SHARED_PMP,
    VSC_PASS_MEMPOOL_SEL_BMS,
    VSC_PASS_MEMPOOL_SEL_AMS
}VSC_PASS_MEMPOOL_SEL;

typedef enum _VSC_PASS_LEVEL
{
    VSC_PASS_LEVEL_PRE  = 0x01,
    VSC_PASS_LEVEL_HL   = 0x02,
    VSC_PASS_LEVEL_ML   = 0x04,
    VSC_PASS_LEVEL_LL   = 0x08,
    VSC_PASS_LEVEL_MC   = 0x10,
    VSC_PASS_LEVEL_CG   = 0x20,
    VSC_PASS_LEVEL_PST  = 0x40
}VSC_PASS_LEVEL;

typedef union _VSC_PASS_RES_CREATION_REQ_FLAG
{
    struct
    {
        gctUINT                           bNeedCg           : 1;
        gctUINT                           bNeedCfg          : 1;
        gctUINT                           bNeedRdFlow       : 1;
        gctUINT                           bNeedDu           : 1;
        gctUINT                           bNeedWeb          : 1;
        gctUINT                           bNeedLvFlow       : 1;
        gctUINT                           bNeedSSAForm      : 1;

        gctUINT                           reserved          : 25;
    } s;

    gctUINT                               data;
}VSC_PASS_RES_CREATION_REQ_FLAG;

typedef union _VSC_PASS_RES_DESTROY_REQ_FLAG
{
    struct
    {
        gctUINT                           bInvalidateCg     : 1;
        gctUINT                           bInvalidateCfg    : 1;
        gctUINT                           bInvalidateRdFlow : 1;
        gctUINT                           bInvalidateDu     : 1;
        gctUINT                           bInvalidateWeb    : 1;
        gctUINT                           bInvalidateLvFlow : 1;

        gctUINT                           reserved          : 26;
    } s;

    gctUINT                               data;
}VSC_PASS_RES_DESTROY_REQ_FLAG;

typedef struct _VSC_PASS_FLAG
{
    VSC_PASS_RES_CREATION_REQ_FLAG    resCreationReq;
    VSC_PASS_RES_DESTROY_REQ_FLAG     resDestroyReq;
}VSC_PASS_FLAG;

typedef struct _VSC_PASS_PROPERTY
{
    VSC_PASS_FLAG                     passFlag;

    VSC_PASS_MEMPOOL_SEL              memPoolSel;
    VSC_PASS_LEVEL                    supportedLevels;
    VSC_PASS_OPTN_TYPE                passOptionType;
}VSC_PASS_PROPERTY;

typedef struct _VSC_SHADER_PASS_RES
{
    /* Control related analysis info */
    VIR_CALL_GRAPH                    callGraph;

    /* Global data related analysis info */
    VIR_DEF_USAGE_INFO                duInfo;
    VIR_LIVENESS_INFO                 lvInfo;
}VSC_SHADER_PASS_RES;

/* A pass worker that is used by each pass. Every pass can not see content of pass
   manager. When each pass begins, a pass worker will be initialized to that pass,
   so pass can use it */
typedef struct _VSC_BASE_PASS_WORKER
{
    /* Dumper */
    VIR_Dumper*                       pDumper;

    /* Point to the option that control this pass */
    VSC_OPTN_BASE*                    pBaseOption;

    /* Pass specific privated data */
    void*                             pPrvData;

    /* Real used mem pool based on VSC_PASS_MEMPOOL_SEL */
    VSC_MM*                           pMM;
}VSC_BASE_PASS_WORKER;

typedef struct _VSC_SH_PASS_WORKER
{
    /* It must be at first place */
    VSC_BASE_PASS_WORKER              basePassWorker;

    /* Compiler parameter from client */
    VSC_SHADER_COMPILER_PARAM*        pCompilerParam;

    /* References from pass-manager for current pass. If pass does not need it,
       set it to NULL */
    VIR_CALL_GRAPH*                   pCallGraph;
    VIR_DEF_USAGE_INFO*               pDuInfo;
    VIR_LIVENESS_INFO*                pLvInfo;

    /* Resource destroy requests might be dynamically determined inside of pass */
    VSC_PASS_RES_DESTROY_REQ_FLAG*    pResDestroyReq;
}VSC_SH_PASS_WORKER;

typedef struct _VSC_GPG_PASS_WORKER
{
    /* It must be at first place */
    VSC_BASE_PASS_WORKER              basePassWorker;

    /* Linker parameter from client */
    VSC_PROGRAM_LINKER_PARAM*         pPgmLinkerParam;

    /* References from pass-manager for current pass. */
    VSC_SHADER_PASS_RES*              pShPassResArray[VSC_MAX_SHADER_STAGE_COUNT];

    /* Resource destroy requests might be dynamically determined inside of pass */
    VSC_PASS_RES_DESTROY_REQ_FLAG     resDestroyReqArray[VSC_MAX_SHADER_STAGE_COUNT];
}VSC_GPG_PASS_WORKER;

/* Pass prototype.

   NOTE every pass author must provide these two functions. Especially, for declaration and
   definition of query-pass-property, must use DECLARE_QUERY_PASS_PROP and DEF_QUERY_PASS_PROP,
   meanwhile, call of that function must use QUERY_PASS_PROP_NAME!!! Please note that pass-manager
   has made sure that pPassProp has been initialized into zero. */
typedef VSC_ErrCode (*PFN_SH_PASS_ROUTINE)(VSC_SH_PASS_WORKER* pPassWorker);
typedef VSC_ErrCode (*PFN_GPG_PASS_ROUTINE)(VSC_GPG_PASS_WORKER* pPassWorker);
typedef void (*PFN_QUERY_PASS_PROP)(VSC_PASS_PROPERTY* pPassProp, void* pPrvData);

#define QUERY_PASS_PROP_NAME(nameOfPassRoutine) nameOfPassRoutine##_QueryPassProp
#define DECLARE_QUERY_PASS_PROP(nameOfPassRoutine) \
                     void QUERY_PASS_PROP_NAME(nameOfPassRoutine)(VSC_PASS_PROPERTY* pPassProp, void* pPrvData)
#define DEF_QUERY_PASS_PROP(nameOfPassRoutine) DECLARE_QUERY_PASS_PROP(nameOfPassRoutine)

/* Used by full-auto mode */
typedef struct _VSC_PASS_NODE
{
    VSC_PASS_PROPERTY                 passProp;

    PFN_SH_PASS_ROUTINE               pfnPassRoutine;
    PFN_QUERY_PASS_PROP               pfnQueryPassProp;
}VSC_PASS_NODE;

typedef enum _VSC_PM_MODE
{
    VSC_PM_MODE_SEMI_AUTO,
    VSC_PM_MODE_FULL_AUTO
}VSC_PM_MODE;

/* A pass-manager is used to take over scheduling of all passes to get best opt result
   with minimum CPU time/memory-footprint.

   NOTE that, normally, a pass-manager manages several levels of passes, from module to
   inst, however, our pass-manager is light-weigh one, and only does coarse management
   for each pass who will act on whole shader.

   Also NOTE that we provide two management modes, one is semi-auto which need pass author
   schedule his/her pass at each compiling level; the another one is full-auto which pass
   manager will fully help schedule all passes that has already registered in pass-manager.
   But for both modes, resources(cfg/du/..)/mempool that passes need will be still managemed
   by pass-manager. Currently, we only support semi-auto mode. */
typedef struct _VSC_BASE_PASS_MANAGER
{
    VSC_PM_MODE                       pmMode;

    VIR_Dumper*                       pDumper;

    /* User options controlling each pass */
    VSC_OPTN_Options*                 pOptions;

    /* Current pass level that pass-manager is managing, only passes that support this
       level can run */
    VSC_PASS_LEVEL                    curPassLevel;
}VSC_BASE_PASS_MANAGER;

typedef struct _VSC_PASS_MM_POOL
{
    VSC_PRIMARY_MEM_POOL              privatePMP;
    VSC_PRIMARY_MEM_POOL              sharedPMP;
    VSC_BUDDY_MEM_SYS                 BMS;
    VSC_ARENA_MEM_SYS                 AMS;
}VSC_PASS_MM_POOL;

typedef struct _VSC_SHADER_PASS_MANAGER
{
    /* It must be put at fist place */
    VSC_BASE_PASS_MANAGER             basePM;

    VSC_SHADER_COMPILER_PARAM*        pCompilerParam;

    VSC_SHADER_PASS_RES               passRes;

    /* NOT using internally builtin mm pool is because for program compiling,
       many shaders can share same mm pool maintained by VSC_BASE_PG_PASS_MANAGER */
    VSC_PASS_MM_POOL*                 pMmPool;
}VSC_SHADER_PASS_MANAGER;

typedef struct _VSC_BASE_PG_PASS_MANAGER
{
    /* It must be put at fist place */
    VSC_BASE_PASS_MANAGER             basePM;

    /* Memory pools for whole program */
    VSC_PASS_MM_POOL                  pgMmPool;

    /* Memory pools for each shader that progrom includes */
    VSC_PASS_MM_POOL                  shMmPool;
}VSC_BASE_PG_PASS_MANAGER;

typedef struct _VSC_GPG_PASS_MANAGER
{
    /* It must be put at fist place */
    VSC_BASE_PG_PASS_MANAGER          basePgmPM;

    VSC_PROGRAM_LINKER_PARAM*         pPgmLinkerParam;

    /* They should be references of passRes of VSC_SHADER_PASS_MANAGER */
    VSC_SHADER_PASS_RES*              pShPassResArray[VSC_MAX_SHADER_STAGE_COUNT];
}VSC_GPG_PASS_MANAGER;

typedef struct _VSC_KPG_PASS_MANAGER
{
    /* It must be put at fist place */
    VSC_BASE_PG_PASS_MANAGER          basePgmPM;

    VSC_KERNEL_PROGRAM_LINKER_PARAM*  pKrnlPgLinkParam;
}VSC_KPG_PASS_MANAGER;

/* Functions */
void vscInitializePassMMPool(VSC_PASS_MM_POOL* pMmPool);
void vscFinalizePassMMPool(VSC_PASS_MM_POOL* pMmPool);

void vscInitializeOptions(VSC_OPTN_Options* pOptions,
                          VSC_HW_CONFIG* pHwCfg,
                          gctUINT cFlags,
                          gctUINT64 optFlags);

void vscFinalizeOptions(VSC_OPTN_Options* pOptions);

void vscPM_SetCurPassLevel(VSC_BASE_PASS_MANAGER* pBasePassMnger, VSC_PASS_LEVEL passLevel);

void vscSPM_Initialize(VSC_SHADER_PASS_MANAGER* pShPassMnger,
                       VSC_SHADER_COMPILER_PARAM* pCompilerParam,
                       VSC_PASS_MM_POOL* pMmPool,
                       gctBOOL bInitSharedPMP, /* If shared PMP is not initialized, need initialize it */
                       VIR_Dumper* pDumper,
                       VSC_OPTN_Options* pOptions,
                       VSC_PM_MODE pmMode);
void vscSPM_Finalize(VSC_SHADER_PASS_MANAGER* pShPassMnger,
                     gctBOOL bFinalizeSharedPMP); /* Finalize shared PMP. Be careful to set it TRUE */

void vscGPPM_Initialize(VSC_GPG_PASS_MANAGER* pPgPassMnger,
                        VSC_PROGRAM_LINKER_PARAM* pPgLinkerParam,
                        VIR_Dumper* pDumper,
                        VSC_OPTN_Options* pOptions,
                        VSC_PM_MODE pmMode);
void vscGPPM_Finalize(VSC_GPG_PASS_MANAGER* pPgPassMnger);

void vscGPPM_SetPassRes(VSC_GPG_PASS_MANAGER* pPgPassMnger,
                        VSC_SHADER_PASS_RES** ppShPassResArray); /* It must be array sized with VSC_MAX_SHADER_STAGE_COUNT */

void vscKPPM_Initialize(VSC_KPG_PASS_MANAGER* pPgPassMnger,
                        VSC_KERNEL_PROGRAM_LINKER_PARAM* pKrnlPgLinkParam,
                        VIR_Dumper* pDumper,
                        VSC_OPTN_Options* pOptions,
                        VSC_PM_MODE pmMode);
void vscKPPM_Finalize(VSC_KPG_PASS_MANAGER* pPgPassMnger);

/* Semi-auto mode of SPM */
VSC_ErrCode vscSPM_CallPass(VSC_SHADER_PASS_MANAGER* pShPassMnger,
                            PFN_SH_PASS_ROUTINE pfnPassRoutine,
                            PFN_QUERY_PASS_PROP pfnQueryPassProp,
                            void* pPrvData);

/* Full-auto mode of SPM */
void vscSPM_RegisterPass(VSC_SHADER_PASS_MANAGER* pShPassMnger,
                         PFN_SH_PASS_ROUTINE pfnPassRoutine,
                         PFN_QUERY_PASS_PROP pfnQueryPassProp,
                         void* pPrvData);
VSC_ErrCode vscSPM_RunPasses(VSC_SHADER_PASS_MANAGER* pShPassMnger);

/* GPPM only supports semi-auto mode */
VSC_ErrCode vscGPPM_CallPass(VSC_GPG_PASS_MANAGER* pPgPassMnger,
                             PFN_GPG_PASS_ROUTINE pfnPassRoutine,
                             PFN_QUERY_PASS_PROP pfnQueryPassProp,
                             void* pPrvData);

#define CALL_SH_PASS(pfnPassRoutine, pPrvData) \
          errCode = vscSPM_CallPass(pShPassMnger, (pfnPassRoutine), QUERY_PASS_PROP_NAME(pfnPassRoutine), (pPrvData)); \
          ON_ERROR(errCode, #pfnPassRoutine);

#define REGISTER_SH_PASS(pfnPassRoutine, pPrvData) \
          vscSPM_RegisterPass(pShPassMnger, (pfnPassRoutine), QUERY_PASS_PROP_NAME(pfnPassRoutine), (pPrvData));

#define RUN_SH_PASSES() errCode = vscSPM_RunPasses(pShPassMnger); ON_ERROR(errCode, "run shader passes");

#define CALL_GPG_PASS(pfnPassRoutine, pPrvData) \
          errCode = vscGPPM_CallPass(pPgPassMnger, (pfnPassRoutine), QUERY_PASS_PROP_NAME(pfnPassRoutine), (pPrvData)); \
          ON_ERROR(errCode, #pfnPassRoutine);

END_EXTERN_C()

#endif /* __gc_vsc_vir_pass_mnger_h_ */

