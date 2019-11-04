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


#include "gc_cl_compiler.h"
#include "gc_hal_user.h"
#include "gc_cl_preprocessor.h"
#include "gc_cl_parser.h"
#include "gc_cl_emit_code.h"
#include "debug/gc_vsc_debug.h"

#define _cldFILENAME_MAX 1024
#define _TURN_OFF_OPTIMIZATION_LOOP_UNROLL   1

#define __ENABLE_VSC_MP_POOL__      (gcvTRUE)
#define __VSC_GENERAL_MP_SIZE__     (512 * 1024)
#define __VSC_PRIVATE_SIZE__        (32 * 1024)

/* cloCOMPILER object. */
struct _cloCOMPILER
{
    clsOBJECT        object;
    gctUINT32        langVersion;
    cleSHADER_TYPE   shaderType;
    gcSHADER         binary;
    gctSTRING        log;
    gctUINT          logBufSize;
#if __USE_VSC_MP__
    VSC_PRIMARY_MEM_POOL        generalPMP;
    VSC_PRIMARY_MEM_POOL        privatePMP;
    VSC_MM                     *currentMmWrapper;
#elif __USE_VSC_BMS__
    VSC_PRIMARY_MEM_POOL        generalPMP;
    VSC_PRIMARY_MEM_POOL        sharedPMP;
    VSC_BUDDY_MEM_SYS           scratchMemPool;
    VSC_MM                     *currentMmWrapper;
#else
    slsDLINK_LIST    memoryPool;
    slsDLINK_LIST    generalMemoryPool;
#endif
    gctCHAR          fileNameBuffer[_cldFILENAME_MAX + 1];
    struct {
        gctUINT16    errorCount;
        gctUINT16    warnCount;
        clsHASH_TABLE    stringPool;
        clsHASH_TABLE    generalStringPool;
        cltOPTIMIZATION_OPTIONS    optimizationOptions;
        cltEXTENSIONS    extensions;
        cltDUMP_OPTIONS  dumpOptions;
        cleSCANNER_STATE scannerState;
        slsSLINK_LIST    *parserState;
        slsSLINK_LIST    *switchScope;
        slsSLINK_LIST    *designationScope;
        slsSLINK_LIST    *builtinFuncReferenced;
        slsSLINK_LIST    *constantVariables;
        gctUINT          stringCount;
        gctSTRING        *strings;
        gctUINT          currentLineNo;
        gctUINT          currentStringNo;
        gctUINT          currentCharNo;
        gctSIZE_T        fileNameBufferSize;
        gctSTRING        currentFileName;
        slsDLINK_LIST    dataTypes;
        clsNAME_SPACE *  unnamedSpace;
        clsNAME_SPACE *  builtinSpace;
        clsNAME_SPACE *  generalBuiltinSpace;
        clsNAME_SPACE *  globalSpace;
        clsNAME_SPACE *  currentSpace;
        clsNAME          *unnamedVariables[cldNumBuiltinVariables];
#if !(__USE_VSC_MP__ || __USE_VSC_BMS__)
        slsSLINK_LIST    *namePools;
        clsNAME_POOL     *curNamePool;
        gctUINT          namePoolSize;
        gctUINT          nameCount;
        slsSLINK_LIST    *generalNamePools;
        clsNAME_POOL     *curGeneralNamePool;
        gctUINT          generalNamePoolSize;
        gctUINT          generalNameCount;
#endif
        cloIR_VARIABLE   paramChainVariables[cldMaxParamChains];
        cloIR_SET        rootSet;
        gctUINT          localTempRegCount;
        gctUINT          tempRegCount;
        gctUINT          labelCount;
        gctUINT          maxKernelFunctionArgs;
        gctSIZE_T        constantMemorySize;
        gctBOOL          needConstantMemory;
        gctSIZE_T        privateMemorySize;
        gctBOOL          needPrivateMemory;
        gctBOOL          needPrintfMemory;
        gctBOOL          inKernelFunctionEpilog;
        gctBOOL          needLocalMemory;
        gctBOOL          hasLocalMemoryKernelArg;
        gctBOOL          loadingGeneralBuiltIns;
        gctBOOL          loadingBuiltins;
        gctBOOL          mainDefined;
        gctBOOL          kernelFuncDefined;
        gctUINT          hasFloatOps;
        gctUINT          hasFloatOpsAux;
        gctUINT32        fpConfig;
        gctBOOL          allowExternSymbols;
        gctUINT          unnamedCount;
        gctBOOL          hasInt64;
        gctBOOL          hasImageQuery;
        gctUINT          imageArrayMaxLevel;
        gctBOOL          basicTypePacked;
        gctBOOL          gcslDriverImage;
        gctBOOL          longUlongPatch;
        VSC_DIContext *  debugInfo;
        gctBOOL          mainFile;
    } context;
    cloPREPROCESSOR      preprocessor;
    cloCODE_EMITTER      codeEmitter;
    cloCODE_GENERATOR    codeGenerator;
};

cloCOMPILER gcKernelCompiler[1] = {gcvNULL};

cloCOMPILER *
gcGetKernelCompiler(void)
{
    return &gcKernelCompiler[0];
}

typedef struct _clsPOOL_STRING_NODE
{
    slsDLINK_NODE    node;
    cltPOOL_STRING   string;
    gctUINT          crc32Value;
}
clsPOOL_STRING_NODE;

struct gcsATOM CompilerLockRef = gcmATOM_INITIALIZER;

gceSTATUS
cloCOMPILER_Load(void)
{
  gceSTATUS status = gcvSTATUS_OK;

  gcmVERIFY_OK(gcoOS_AtomIncrement(gcvNULL, &CompilerLockRef, gcvNULL));

  return status;
}

gceSTATUS
cloCOMPILER_Unload(void)
{
   gceSTATUS status = gcvSTATUS_OK;
   gctINT32 reference = 0;

   /* Decrement the reference counter */
   gcmVERIFY_OK(gcoOS_AtomDecrement(gcvNULL, &CompilerLockRef, &reference));

   if (reference == 1)
   {
     status = clCleanupKeywords();
     if(gcmIS_ERROR(status)) return status;
     status = clCleanupBuiltins();
     if(gcmIS_ERROR(status)) return status;
     gcKernelCompiler[0] = gcvNULL;
   }

   return status;
}

gceSTATUS
cloCOMPILER_Construct(
    INOUT cloCOMPILER Compiler
    )
{
    gceSTATUS status;
    cloCOMPILER compiler = Compiler;
    gcePATCH_ID patchId  = gcvPATCH_INVALID;

    gcmASSERT(Compiler);

    do {
#if (__USE_VSC_MP__ || __USE_VSC_BMS__)
        VSC_PRIMARY_MEM_POOL    generalPMP;
#else
        slsDLINK_LIST           generalMemoryPool;
        slsSLINK_LIST *         generalNamePools;
        clsNAME_POOL     *curGeneralNamePool;
        gctUINT          generalNamePoolSize;
        gctUINT          generalNameCount;
#endif
        clsHASH_TABLE           generalStringPool;
        clsNAME_SPACE *         generalBuiltinSpace;

        /* Copy the general parts into the temp variables. */
#if (__USE_VSC_MP__ || __USE_VSC_BMS__)
        gcoOS_MemCopy(&generalPMP, &compiler->generalPMP, gcmSIZEOF(VSC_PRIMARY_MEM_POOL));
#else
        gcoOS_MemCopy(&generalMemoryPool, &compiler->generalMemoryPool, gcmSIZEOF(slsDLINK_LIST));
        generalNamePools = compiler->context.generalNamePools;
        curGeneralNamePool = compiler->context.curGeneralNamePool;
        generalNamePoolSize = compiler->context.generalNamePoolSize;
        generalNameCount = compiler->context.generalNameCount;
#endif
        gcoOS_MemCopy(&generalStringPool, &compiler->context.generalStringPool, gcmSIZEOF(clsHASH_TABLE));
        generalBuiltinSpace = compiler->context.generalBuiltinSpace;

        gcoOS_ZeroMemory(compiler, gcmSIZEOF(struct _cloCOMPILER));

        /* Copy back the general parts. */
#if (__USE_VSC_MP__ || __USE_VSC_BMS__)
        gcoOS_MemCopy(&compiler->generalPMP, &generalPMP, gcmSIZEOF(VSC_PRIMARY_MEM_POOL));
#else
        gcoOS_MemCopy(&compiler->generalMemoryPool, &generalMemoryPool, gcmSIZEOF(slsDLINK_LIST));
        compiler->context.generalNamePools = generalNamePools;
        compiler->context.curGeneralNamePool = curGeneralNamePool;
        compiler->context.generalNameCount = generalNameCount;
        compiler->context.generalNamePoolSize = generalNamePoolSize;
#endif
        gcoOS_MemCopy(&compiler->context.generalStringPool, &generalStringPool, gcmSIZEOF(clsHASH_TABLE));
        compiler->context.generalBuiltinSpace = generalBuiltinSpace;

        /* Initialize members */
        compiler->object.type   = clvOBJ_COMPILER;
        compiler->shaderType    = clvSHADER_TYPE_CL;
        compiler->langVersion   = cloGetDefaultLanguageVersion();
        compiler->binary        = gcvNULL;
        compiler->log           = gcvNULL;
        compiler->logBufSize    = 0;
        compiler->fileNameBuffer[0] = '\0';

#if __USE_VSC_MP__
        vscPMP_Intialize(&compiler->privatePMP, gcvNULL, __VSC_PRIVATE_SIZE__, sizeof(void *), __ENABLE_VSC_MP_POOL__);
        compiler->currentMmWrapper = &compiler->privatePMP.mmWrapper;
#elif __USE_VSC_BMS__
        vscPMP_Intialize(&compiler->sharedPMP, gcvNULL, __VSC_PRIVATE_SIZE__, sizeof(void *), gcvTRUE);
        vscBMS_Initialize(&compiler->scratchMemPool, &compiler->sharedPMP);
        compiler->currentMmWrapper = &compiler->scratchMemPool.mmWrapper;
#else
        slsDLINK_LIST_Initialize(&compiler->memoryPool);
#endif

        compiler->context.currentFileName = compiler->fileNameBuffer;
        compiler->context.fileNameBufferSize = _cldFILENAME_MAX;
        compiler->context.errorCount    = 0;
        compiler->context.warnCount     = 0;

        compiler->context.tempRegCount  = 0;
/* set aside registers for private and local memory addressing if needed */
        compiler->context.localTempRegCount = cldNumMemoryAddressRegs;
        compiler->context.labelCount    = 0;
        compiler->context.needConstantMemory = gcvFALSE;
        compiler->context.needPrivateMemory = gcvFALSE;
        compiler->context.needPrintfMemory = gcvFALSE;
        compiler->context.needLocalMemory = gcvFALSE;
        compiler->context.hasLocalMemoryKernelArg = gcvFALSE;
        compiler->context.hasInt64 = gcvFALSE;
        compiler->context.hasImageQuery = gcvFALSE;
        compiler->context.basicTypePacked = gcmOPT_oclPackedBasicType();
        compiler->context.imageArrayMaxLevel = cldVxImageArrayMaxLevelDefault;
        compiler->context.constantMemorySize = 0;
        compiler->context.privateMemorySize = 0;
        compiler->context.maxKernelFunctionArgs = 0;

        compiler->context.hasFloatOps = 0;
        compiler->context.hasFloatOpsAux = 0;
        compiler->context.fpConfig = cldFpFAST_RELAXED_MATH;
        compiler->context.allowExternSymbols = gcvFALSE;

        patchId = *gcGetPatchId();

        if (vscDIConstructContext(gcvNULL,gcvNULL, &compiler->context.debugInfo) != gcvSTATUS_OK)
        {
            vscDIDestroyContext(compiler->context.debugInfo);
            compiler->context.debugInfo = gcvNULL;
        }

        if (patchId == gcvPATCH_OCLCTS)
        {
            compiler->context.fpConfig = 0;
        }

        if ((gcmOPT_oclFpCaps()) & VC_OPTION_OCLFPCAPS_NOFASTRELAXEDMATH)
        {
            compiler->context.fpConfig = 0;
        }

        compiler->context.loadingBuiltins   = gcvFALSE;
        compiler->context.mainDefined        = gcvFALSE;
        compiler->context.kernelFuncDefined = gcvFALSE;
        compiler->context.inKernelFunctionEpilog   = gcvFALSE;

        clsHASH_TABLE_Initialize(&compiler->context.stringPool);

        compiler->context.stringCount   = 0;
        compiler->context.strings       = gcvNULL;

        slsDLINK_LIST_Initialize(&compiler->context.dataTypes);

        slmSLINK_LIST_Initialize(compiler->context.builtinFuncReferenced);
        slmSLINK_LIST_Initialize(compiler->context.switchScope);
        slmSLINK_LIST_Initialize(compiler->context.designationScope);
        slmSLINK_LIST_Initialize(compiler->context.parserState);
        slmSLINK_LIST_Initialize(compiler->context.constantVariables);
#if !(__USE_VSC_MP__ || __USE_VSC_BMS__)
        slmSLINK_LIST_Initialize(compiler->context.namePools);
        compiler->context.namePoolSize = 0;
        compiler->context.curNamePool = gcvNULL;
#endif

        /* Create unnamed space */
        status = clsNAME_SPACE_Construct(compiler,
                         gcvNULL,
                         &compiler->context.unnamedSpace);
        if (gcmIS_ERROR(status)) {
            gcmASSERT(compiler->context.unnamedSpace == gcvNULL);
            break;
        }

        compiler->context.unnamedSpace->die = compiler->context.debugInfo ?
                                              VSC_DI_SPACE_SKIP_DIE :
                                              VSC_DI_INVALIDE_DIE;

        /* Create built-in name space */
        status = clsNAME_SPACE_Construct(compiler,
                         compiler->context.generalBuiltinSpace,
                         &compiler->context.builtinSpace);
        if (gcmIS_ERROR(status)) {
            gcmASSERT(compiler->context.builtinSpace == gcvNULL);
            break;
        }
        cloCOMPILER_AllocatePoolString(compiler,
                                       "$__namespace_builtin",
                                       &compiler->context.builtinSpace->symbol);

        compiler->context.builtinSpace->die = compiler->context.debugInfo ?
                                              compiler->context.debugInfo->cu :
                                              VSC_DI_INVALIDE_DIE;

        compiler->context.currentSpace = compiler->context.builtinSpace;

        /* Create global name space */
        status = clsNAME_SPACE_Construct(compiler,
                         compiler->context.builtinSpace,
                         &compiler->context.globalSpace);
        if (gcmIS_ERROR(status)) {
            gcmASSERT(compiler->context.globalSpace == gcvNULL);
            break;
        }

        cloCOMPILER_AllocatePoolString(compiler,
                                       "$__namespace_global",
                                       &compiler->context.globalSpace->symbol);

        compiler->context.globalSpace->die = compiler->context.debugInfo ?
                                             compiler->context.debugInfo->cu :
                                             VSC_DI_INVALIDE_DIE;

        /* Create IR root */
        status = cloIR_SET_Construct(compiler,
                         1,
                         0,
                         clvDECL_SET,
                         &compiler->context.rootSet);

        if (gcmIS_ERROR(status)) {
            gcmASSERT(compiler->context.rootSet == gcvNULL);
            break;
        }

#ifndef CL_SCAN_NO_PREPROCESSOR
        /* Create the preprocessor */
        status = cloPREPROCESSOR_Construct(compiler, &compiler->preprocessor);
        if (gcmIS_ERROR(status)) break;
#endif

        /* Create the code emitter */
        status = cloCODE_EMITTER_Construct(compiler, &compiler->codeEmitter);
        if (gcmIS_ERROR(status)) break;

        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    if (compiler != gcvNULL) cloCOMPILER_Destroy(compiler);
    return status;
}

gceSTATUS
cloCOMPILER_Construct_General(
    IN gctCONST_STRING Options,
    OUT cloCOMPILER * Compiler
    )
{
    gceSTATUS status;
    cloCOMPILER compiler = gcvNULL;

    gcmASSERT(Compiler);

    do {
        /* Allocate memory for cloCOMPILER object */
        status = gcoOS_Allocate(gcvNULL,
                    sizeof(struct _cloCOMPILER),
                    (gctPOINTER *)&compiler
                    );

        if (gcmIS_ERROR(status)) {
            compiler = gcvNULL;
            break;
        }

        gcoOS_ZeroMemory(compiler, gcmSIZEOF(struct _cloCOMPILER));

        /* Initialize members */
        compiler->object.type   = clvOBJ_COMPILER;
        compiler->shaderType    = clvSHADER_TYPE_CL;
        compiler->langVersion   = cloGetDefaultLanguageVersion();

        if (Options && gcoOS_StrStr(Options, "cl-viv-vx-extension", gcvNULL)) {
            status = cloCOMPILER_EnableExtension(compiler,
                                                 clvEXTENSION_VIV_VX,
                                                 gcvTRUE);
            if(gcmIS_ERROR(status)) return status;
        }

#if (__USE_VSC_MP__ ||__USE_VSC_BMS__)
        vscPMP_Intialize(&compiler->generalPMP, gcvNULL, __VSC_GENERAL_MP_SIZE__, sizeof(void *), __ENABLE_VSC_MP_POOL__);
        compiler->currentMmWrapper = &compiler->generalPMP.mmWrapper;
#else
        slsDLINK_LIST_Initialize(&compiler->generalMemoryPool);
        slmSLINK_LIST_Initialize(compiler->context.generalNamePools);
        compiler->context.generalNamePoolSize = 0;
        compiler->context.curGeneralNamePool = gcvNULL;
#endif
        clsHASH_TABLE_Initialize(&compiler->context.generalStringPool);

        compiler->context.loadingGeneralBuiltIns = gcvTRUE;

        /* Create built-in name space */
        status = clsNAME_SPACE_Construct(compiler,
                         gcvNULL,
                         &compiler->context.generalBuiltinSpace);
        if (gcmIS_ERROR(status)) {
            gcmASSERT(compiler->context.generalBuiltinSpace == gcvNULL);
            break;
        }
        cloCOMPILER_AllocatePoolString(compiler,
                                       "$__namespace_builtin_general",
                                       &compiler->context.generalBuiltinSpace->symbol);

        compiler->context.generalBuiltinSpace->die = compiler->context.debugInfo ?
                                              compiler->context.debugInfo->cu :
                                              VSC_DI_INVALIDE_DIE;

        compiler->context.currentSpace = compiler->context.generalBuiltinSpace;

        /* Load the built-ins */
        status = cloCOMPILER_LoadGeneralBuiltIns(compiler);
        if (gcmIS_ERROR(status)) return status;
#if (__USE_VSC_MP__ || __USE_VSC_BMS__)
        compiler->currentMmWrapper = gcvNULL;
#endif

        *Compiler = compiler;
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    if (compiler != gcvNULL) cloCOMPILER_Destroy_General(compiler);
    *Compiler = gcvNULL;
    return status;
}

gceSTATUS
cloCOMPILER_LoadGeneralBuiltIns(
    IN cloCOMPILER Compiler
    )
{
    gceSTATUS           status;
    clsNAME_SPACE *     currentSpace = Compiler->context.currentSpace;

    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    Compiler->context.currentSpace = Compiler->context.generalBuiltinSpace;
    Compiler->context.loadingGeneralBuiltIns = gcvTRUE;
    Compiler->context.loadingBuiltins = gcvTRUE;

    status = clLoadGeneralBuiltIns(Compiler, Compiler->shaderType);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    Compiler->context.currentSpace = currentSpace;
    Compiler->context.loadingGeneralBuiltIns = gcvFALSE;
    Compiler->context.loadingBuiltins = gcvFALSE;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_LoadBuiltins(
    IN cloCOMPILER Compiler
    )
{
    gceSTATUS    status;
    cleSHADER_TYPE    shaderType;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    gcmVERIFY_OK(cloCOMPILER_LoadingBuiltins(Compiler, gcvTRUE));

    /* Load all kind of built-ins */
    gcmVERIFY_OK(cloCOMPILER_GetShaderType(Compiler, &shaderType));

    status = clLoadBuiltins(Compiler, shaderType);
    if (gcmIS_ERROR(status)) return status;

    gcmVERIFY_OK(cloCOMPILER_LoadingBuiltins(Compiler, gcvFALSE));
    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_ConstructByLangVersion(
    IN  gctUINT32 LangVersion,
    OUT cloCOMPILER * Compiler
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    cloCOMPILER compiler = *Compiler;

    gcmASSERT(Compiler);
    switch(LangVersion)
    {
    case _cldCL1Dot1:
        break;

    case _cldCL1Dot2:
        status = cloCOMPILER_Construct(compiler);
        if (gcmIS_ERROR(status)) return status;
        compiler->context.allowExternSymbols = gcvTRUE;
        break;

    default:
        gcmASSERT(0);
        status = gcvSTATUS_INVALID_DATA;
        break;
    }

    *Compiler = compiler;
    return status;
}

gceSTATUS
cloCOMPILER_Destroy(
    IN cloCOMPILER Compiler
    )
{
    clsDATA_TYPE *    dataType;
    slsDLINK_LIST *    poolStringBucket;
    clsPOOL_STRING_NODE *    poolStringNode;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    if (Compiler->codeEmitter != gcvNULL) {
        gcmVERIFY_OK(cloCODE_EMITTER_Destroy(Compiler, Compiler->codeEmitter));
    }

#ifndef CL_SCAN_NO_PREPROCESSOR
    if (Compiler->preprocessor != gcvNULL) {
        gcmVERIFY_OK(cloPREPROCESSOR_Destroy(Compiler, Compiler->preprocessor));
    }
#endif
    if (Compiler->binary != gcvNULL) {
        gcmVERIFY_OK(gcSHADER_Destroy(Compiler->binary));
    }

    if (Compiler->log != gcvNULL) {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Compiler->log));
    }

    /* Destroy the whole IR tree */
    if (Compiler->context.rootSet != gcvNULL) {
        gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &Compiler->context.rootSet->base));
    }

    /* Destroy unnamed name space */
    if (Compiler->context.unnamedSpace != gcvNULL) {
        gcmVERIFY_OK(clsNAME_SPACE_Destroy(Compiler, Compiler->context.unnamedSpace));
    }

    /* Destroy built-in name space */
    if (Compiler->context.builtinSpace != gcvNULL)
    {
        clsNAME_SPACE * subSpace;

        FOR_EACH_DLINK_NODE_REVERSELY(&Compiler->context.generalBuiltinSpace->subSpaces, clsNAME_SPACE, subSpace)
        {
            if (subSpace == Compiler->context.builtinSpace)
            {
                slsDLINK_NODE_Detach((slsDLINK_NODE *)subSpace);
                break;
            }
        }
        gcmVERIFY_OK(clsNAME_SPACE_Destroy(Compiler, Compiler->context.builtinSpace));
        Compiler->context.builtinSpace = gcvNULL;
    }

    if (Compiler->context.debugInfo != gcvNULL) {
        vscDIDestroyContext(Compiler->context.debugInfo);
    }

    /* Destroy data types */
    while (!slsDLINK_LIST_IsEmpty(&Compiler->context.dataTypes)) {
        slsDLINK_LIST_DetachFirst(&Compiler->context.dataTypes, clsDATA_TYPE, &dataType);
        gcmVERIFY_OK(clsDATA_TYPE_Destroy(Compiler, dataType));
    }

    /* Destroy switch scope */
    while (!slmSLINK_LIST_IsEmpty(Compiler->context.switchScope)) {
       clsSWITCH_SCOPE *scope;
       slmSLINK_LIST_DetachFirst(Compiler->context.switchScope, clsSWITCH_SCOPE, &scope);
       gcmVERIFY_OK(cloCOMPILER_Free(Compiler, scope));
    }

    /* Destroy designation scope */
    while (!slmSLINK_LIST_IsEmpty(Compiler->context.designationScope)) {
       clsDESIGNATION_SCOPE *scope;
       slmSLINK_LIST_DetachFirst(Compiler->context.designationScope, clsDESIGNATION_SCOPE, &scope);
       gcmVERIFY_OK(cloCOMPILER_Free(Compiler, scope));
    }

    /* Destroy parser state */
    while (!slmSLINK_LIST_IsEmpty(Compiler->context.parserState)) {
       clsPARSER_STATE *parserState;
       slmSLINK_LIST_DetachFirst(Compiler->context.parserState, clsPARSER_STATE, &parserState);
       gcmVERIFY_OK(cloCOMPILER_Free(Compiler, parserState));
    }

    /* Destroy referenced builtin function list */
    while (!slmSLINK_LIST_IsEmpty(Compiler->context.builtinFuncReferenced)) {
       clsBUILTIN_FUNC_REFERENCED *builtin;
       slmSLINK_LIST_DetachFirst(Compiler->context.builtinFuncReferenced,
                     clsBUILTIN_FUNC_REFERENCED,
                     &builtin);
       gcmVERIFY_OK(cloCOMPILER_Free(Compiler, builtin));
    }

#if !(__USE_VSC_MP__ || __USE_VSC_BMS__)
    /* Destroy name pool list */
    while (!slmSLINK_LIST_IsEmpty(Compiler->context.namePools)) {
       clsNAME_POOL *namePool;
       slmSLINK_LIST_DetachFirst(Compiler->context.namePools,
                     clsNAME_POOL,
                     &namePool);
       gcmVERIFY_OK(cloCOMPILER_Free(Compiler, namePool->member));
       gcmVERIFY_OK(cloCOMPILER_Free(Compiler, namePool));
    }
#endif

    /* Destroy constant variable list */
    while (!slmSLINK_LIST_IsEmpty(Compiler->context.constantVariables)) {
       clsCONSTANT_VARIABLE *constantVariable;

       slmSLINK_LIST_DetachFirst(Compiler->context.constantVariables,
                     clsCONSTANT_VARIABLE,
                     &constantVariable);
       gcmVERIFY_OK(cloCOMPILER_Free(Compiler, constantVariable));
    }

    /* Destroy string pool */
    FOR_EACH_HASH_BUCKET(&Compiler->context.stringPool, poolStringBucket) {
        while (!slsDLINK_LIST_IsEmpty(poolStringBucket)) {
            slsDLINK_LIST_DetachFirst(poolStringBucket, clsPOOL_STRING_NODE, &poolStringNode);
            gcmVERIFY_OK(cloCOMPILER_Free(Compiler, poolStringNode));
        }
    }

#if __USE_VSC_MP__
    gcmASSERT(Compiler->currentMmWrapper == &Compiler->privatePMP.mmWrapper);
    vscPMP_Finalize(&Compiler->privatePMP);
    Compiler->currentMmWrapper = &Compiler->generalPMP.mmWrapper;
#elif __USE_VSC_BMS__
    gcmASSERT(Compiler->currentMmWrapper == &Compiler->scratchMemPool.mmWrapper);
    vscBMS_Finalize(&Compiler->scratchMemPool, gcvFALSE);
    vscPMP_Finalize(&Compiler->sharedPMP);
    Compiler->currentMmWrapper = &Compiler->generalPMP.mmWrapper;
#else
    /* Empty the memory pool */
    gcmVERIFY_OK(cloCOMPILER_EmptyMemoryPool(Compiler));
#endif

    /* Clean up some variables in general built-in name space. */
    if (Compiler->context.generalBuiltinSpace != gcvNULL)
    {
        clsNAME *name = gcvNULL;

        /* If a builtin function is used, we need to reset it. */
        FOR_EACH_DLINK_NODE_REVERSELY(&Compiler->context.generalBuiltinSpace->names, clsNAME, name)
        {
            if (name->type == clvFUNC_NAME && name->u.funcInfo.refCount > 0)
            {
                clsNAME *paramName = gcvNULL;
                clsNAME_Reset(Compiler, name);

                FOR_EACH_DLINK_NODE_REVERSELY(&name->u.funcInfo.localSpace->names, clsNAME, paramName)
                {
                    clsNAME_Reset(Compiler, paramName);
                }
            }
        }
    }

    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_Destroy_General(
    IN cloCOMPILER Compiler
    )
{
    slsDLINK_LIST *    poolStringBucket;
    clsPOOL_STRING_NODE *    poolStringNode;
#if !(__USE_VSC_MP__ || __USE_VSC_BMS__)
    slsDLINK_NODE *    node;
#endif

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

#if (__USE_VSC_MP__ || __USE_VSC_BMS__)
    Compiler->currentMmWrapper = &Compiler->generalPMP.mmWrapper;
#endif

    /* Destroy general built-in name space */
    if (Compiler->context.generalBuiltinSpace != gcvNULL) {
        gcmVERIFY_OK(clsNAME_SPACE_Destroy(Compiler, Compiler->context.generalBuiltinSpace));
    }

#if !(__USE_VSC_MP__ || __USE_VSC_BMS__)
    /* Destroy general name pool list */
    while (!slmSLINK_LIST_IsEmpty(Compiler->context.generalNamePools)) {
       clsNAME_POOL *namePool;

       slmSLINK_LIST_DetachFirst(Compiler->context.generalNamePools,
                     clsNAME_POOL,
                     &namePool);
       gcmVERIFY_OK(cloCOMPILER_Free(Compiler, namePool->member));
       gcmVERIFY_OK(cloCOMPILER_Free(Compiler, namePool));
    }
#endif

    /* Destroy string pool */
    FOR_EACH_HASH_BUCKET(&Compiler->context.generalStringPool, poolStringBucket) {
        while (!slsDLINK_LIST_IsEmpty(poolStringBucket)) {
            slsDLINK_LIST_DetachFirst(poolStringBucket, clsPOOL_STRING_NODE, &poolStringNode);
            gcmVERIFY_OK(cloCOMPILER_Free(Compiler, poolStringNode));
        }
    }

#if (__USE_VSC_MP__ || __USE_VSC_BMS__)
    vscPMP_Finalize(&Compiler->generalPMP);
#else
    /* Empty the memory pool */
    while (!slsDLINK_LIST_IsEmpty(&Compiler->generalMemoryPool)) {
        slsDLINK_LIST_DetachFirst(&Compiler->generalMemoryPool, slsDLINK_NODE, &node);
        gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)node));
    }
#endif

    /* Free compiler struct */
    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Compiler));
    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_GetShaderType(
    IN cloCOMPILER Compiler,
    OUT cleSHADER_TYPE * ShaderType
    )
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(ShaderType);

    *ShaderType = Compiler->shaderType;
    return gcvSTATUS_OK;
}

gctBOOL
cloCOMPILER_IsLoadingBuiltin(
    IN cloCOMPILER Compiler
    )
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    return Compiler->context.loadingBuiltins;
}

gceSTATUS
cloCOMPILER_GetBinary(
    IN cloCOMPILER Compiler,
    OUT gcSHADER * Binary
    )
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Binary);

    *Binary = Compiler->binary;
    return (Compiler->binary == gcvNULL)? gcvSTATUS_INVALID_DATA : gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_LoadingBuiltins(
    IN cloCOMPILER Compiler,
    IN gctBOOL LoadingBuiltins
    )
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    Compiler->context.loadingBuiltins = LoadingBuiltins;
    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_LoadingGeneralBuiltins(
    IN cloCOMPILER Compiler,
    IN gctBOOL LoadingBuiltins
    )
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    Compiler->context.loadingGeneralBuiltIns = LoadingBuiltins;
    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_InKernelFunctionEpilog(
    IN cloCOMPILER Compiler,
    IN gctBOOL InKernelFunctionEpilog
    )
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    Compiler->context.inKernelFunctionEpilog = InKernelFunctionEpilog;
    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_MainDefined(
    IN cloCOMPILER Compiler
    )
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    if (Compiler->context.mainDefined) return gcvSTATUS_INVALID_REQUEST;
    Compiler->context.mainDefined = gcvTRUE;
    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_KernelFuncDefined(
    IN cloCOMPILER Compiler
    )
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    if (Compiler->context.kernelFuncDefined) return gcvSTATUS_INVALID_REQUEST;
    Compiler->context.kernelFuncDefined = gcvTRUE;
    return gcvSTATUS_OK;
}

#define LOG_BUF_RESERVED_SIZE    1024

gceSTATUS
cloCOMPILER_AddLog(
    IN cloCOMPILER Compiler,
    IN gctCONST_STRING Log
    )
{
    gceSTATUS    status = gcvSTATUS_OK;
    gctUINT      requiredLogBufSize;
    gctSTRING    newLog;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(Log);

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_COMPILER, Log);

    requiredLogBufSize = (gctUINT) gcoOS_StrLen(Log, gcvNULL) + 1;

    if (Compiler->logBufSize > 0) {
        requiredLogBufSize += (gctUINT) gcoOS_StrLen(Compiler->log, gcvNULL);
    }

    if (requiredLogBufSize > Compiler->logBufSize) {
        requiredLogBufSize += LOG_BUF_RESERVED_SIZE;
        status = gcoOS_Allocate(gcvNULL,
                                (gctSIZE_T)requiredLogBufSize,
                                (gctPOINTER *) &newLog);
        if (gcmIS_ERROR(status)) return status;

        if (Compiler->logBufSize > 0) {
            gcmVERIFY_OK(gcoOS_StrCopySafe(newLog, requiredLogBufSize, Compiler->log));
            gcmVERIFY_OK(gcoOS_StrCatSafe(newLog, requiredLogBufSize, Log));
            gcmVERIFY_OK(gcoOS_Free(gcvNULL, Compiler->log));
        }
        else {
            gcmASSERT(Compiler->log == gcvNULL);
            gcmVERIFY_OK(gcoOS_StrCopySafe(newLog, requiredLogBufSize, Log));
        }

        Compiler->log        = newLog;
        Compiler->logBufSize    = requiredLogBufSize;
    }
    else {
        gcmVERIFY_OK(gcoOS_StrCatSafe(Compiler->log, Compiler->logBufSize, Log));
    }
    return gcvSTATUS_OK;
}

#define MAX_SINGLE_LOG_LENGTH        1024

gceSTATUS
cloCOMPILER_VOutputLog(
    IN cloCOMPILER Compiler,
    IN gctCONST_STRING Message,
    IN gctARGUMENTS Arguments
    )
{
    char buffer[MAX_SINGLE_LOG_LENGTH + 1];
    gctUINT offset = 0;

    gcmVERIFY_OK(gcoOS_PrintStrVSafe(buffer, gcmSIZEOF(buffer), &offset, Message, Arguments));

    buffer[MAX_SINGLE_LOG_LENGTH] = '\0';

    if (Compiler->context.debugInfo != gcvNULL)
        gcmPRINT("%s", buffer);

    return cloCOMPILER_AddLog(Compiler, buffer);
}

gceSTATUS
cloCOMPILER_OutputLog(
    IN cloCOMPILER Compiler,
    IN gctCONST_STRING Message,
    IN ...
    )
{
    gceSTATUS    status;
    gctARGUMENTS arguments;

    gcmARGUMENTS_START(arguments, Message);
    status = cloCOMPILER_VOutputLog(Compiler,
                    Message,
                    arguments);
    gcmARGUMENTS_END(arguments);
    return status;
}

gceSTATUS
cloCOMPILER_GenCode(
    IN cloCOMPILER Compiler
    )
{
    gceSTATUS    status;
    clsGEN_CODE_PARAMETERS    parameters;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    if (Compiler->context.rootSet == gcvNULL) return gcvSTATUS_INVALID_DATA;

    /* Check if 'main' function or kernel function defined */
#if !cldNoMain
    if (!Compiler->context.mainDefined) {
         if (!Compiler->context.kernelFuncDefined) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            0,
                            0,
                            clvREPORT_ERROR,
                            "'main' function undefined"));
        return gcvSTATUS_INVALID_DATA;
          }
          else {
        gctUINT topKernelCnt;
        clsNAME *topKernelFunc;
        clsNAME *mainFunc;

        topKernelCnt = cloCOMPILER_FindTopKernelFunc(Compiler, &topKernelFunc);
        if(topKernelCnt >1) {
             gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                             0,
                             0,
                             clvREPORT_ERROR,
                             "there are more than one top kernel function defined"));
                 return gcvSTATUS_INVALID_DATA;
        }
        else if(topKernelCnt == 0) {
          if(!topKernelFunc) {
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                0,
                                0,
                                clvREPORT_ERROR,
                                "no top kernel function been defined"));
             return gcvSTATUS_NOT_FOUND;
          }
          else {
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                0,
                                0,
                                clvREPORT_WARN,
                                "top kernel function \'%s\' is empty",
                             topKernelFunc->symbol));
          }
        }
        gcmASSERT(topKernelFunc);
        mainFunc = clParseMakeFakeMain(Compiler, topKernelFunc);
        if(!mainFunc) {
             gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                             0,
                             0,
                             clvREPORT_ERROR,
                             "cannot created main function"));
                 return gcvSTATUS_INVALID_DATA;
        }
         }
    }
#endif
    status = cloCODE_GENERATOR_Construct(Compiler, &Compiler->codeGenerator);
    if (gcmIS_ERROR(status)) return status;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_CODE_GENERATOR,
                      "<PROGRAM>"));

    clsGEN_CODE_PARAMETERS_Initialize(&parameters, gcvFALSE, gcvFALSE);

    if(Compiler->context.needPrivateMemory ||
           Compiler->context.needLocalMemory ||
           Compiler->context.hasLocalMemoryKernelArg ||
           Compiler->context.needConstantMemory) {
        SetShaderMaxLocalTempRegCount(Compiler->binary, cldMaxLocalTempRegs);
        gcShaderSetHasBaseMemoryAddr(Compiler->binary);
        /* pre-allocate cldMaxLocalTempRegs temp registers,
         * which are reserved for kernel epilog
         */
        gcSHADER_NewTempRegs(Compiler->binary,
                             cldMaxLocalTempRegs,
                             gcSHADER_FLOAT_X1);
    }
    else {
        SetShaderMaxLocalTempRegCount(Compiler->binary, 0);
    }

    status = cloIR_OBJECT_Accept(Compiler,
                     &Compiler->context.rootSet->base,
                     &Compiler->codeGenerator->visitor,
                     &parameters);

    clsGEN_CODE_PARAMETERS_Finalize(&parameters);

    gcmVERIFY_OK(cloCODE_GENERATOR_Destroy(Compiler, Compiler->codeGenerator));

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                clvDUMP_CODE_GENERATOR,
                "</PROGRAM>"));
    if (gcmIS_ERROR(status)) return status;
    return gcvSTATUS_OK;
}

gceSTATUS
cloCompiler_InitializeConstantMemory(
cloCOMPILER Compiler,
gctSIZE_T BufferSize,
gctCHAR *Buffer
)
{
   clsCONSTANT_VARIABLE *constantVariable;
   clsNAME *constantVarName;
   cloIR_CONSTANT constant;
   gctCHAR *endBufPtr;
   cltELEMENT_TYPE elementType;
   gctUINT i, j;
   gctUINT padding = 0;
   union {
    char * charPtr;
    unsigned char *ucharPtr;
    short * shortPtr;
    unsigned short * ushortPtr;
    int * intPtr;
    unsigned int * uintPtr;
    gctBOOL * boolPtr;
    float * floatPtr;
    gctINT64 * longPtr;
    gctUINT64 * ulongPtr;
   } bufPtr;

   gcmASSERT(BufferSize >= Compiler->context.constantMemorySize);

   endBufPtr = Buffer + BufferSize;
   while (!slmSLINK_LIST_IsEmpty(Compiler->context.constantVariables)) {

      slmSLINK_LIST_DetachFirst(Compiler->context.constantVariables,
                                clsCONSTANT_VARIABLE,
                                &constantVariable);
      constantVarName = constantVariable->member;
      gcmASSERT(clmNAME_VariableHasMemoryOffset(constantVarName));
      bufPtr.charPtr = Buffer + clmNAME_VariableMemoryOffset_NOCHECK_GET(constantVarName);

      if(constantVarName->u.variableInfo.u.constant == gcvNULL) {
         /* initialize to zero */
         gctSIZE_T memoryReqd;

         memoryReqd = clsDECL_GetByteSize(Compiler, &constantVarName->decl);
         if((bufPtr.charPtr + memoryReqd) > endBufPtr) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                            0,
                                            0,
                                            clvREPORT_FATAL_ERROR,
                                            "write beyond buffer boundary"));
            return gcvSTATUS_INVALID_DATA;
         }
         for(i = 0; i < memoryReqd; i++) {
           *(bufPtr.charPtr)++ = 0;
         }
         continue;
      }

      constant = constantVarName->u.variableInfo.u.constant;
      if(clmDECL_IsUnderlyingStructOrUnion(&constant->exprBase.decl) ||
         constant->buffer) {
         if((bufPtr.charPtr + constant->valueCount) > endBufPtr) {
             gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                             0,
                                             0,
                                             clvREPORT_FATAL_ERROR,
                                             "write beyond buffer boundary"));
             return gcvSTATUS_INVALID_DATA;
         }
         gcoOS_MemCopy(bufPtr.charPtr,
                       constant->buffer,
                       constant->valueCount);
         bufPtr.charPtr += constant->valueCount;
         continue;
      }
      gcmASSERT(endBufPtr > bufPtr.charPtr);
      elementType = constant->exprBase.decl.dataType->elementType;

      if (clmDATA_TYPE_vectorSize_NOCHECK_GET(constant->exprBase.decl.dataType) == 3) {
         padding = 3;
      }
      else {
         padding = 0;
      }

      j = padding;
      for(i=0; i < constant->valueCount; i++) {
         if(i != 0  && i == j) {
            j += padding;
            switch(elementType) {
            case clvTYPE_CHAR:
            case clvTYPE_CHAR_PACKED:
               bufPtr.charPtr++;
               break;

            case clvTYPE_UCHAR:
            case clvTYPE_UCHAR_PACKED:
               bufPtr.ucharPtr++;
               break;

            case clvTYPE_SHORT:
            case clvTYPE_SHORT_PACKED:
               bufPtr.shortPtr++;
               break;

            case clvTYPE_USHORT:
            case clvTYPE_USHORT_PACKED:
               bufPtr.ushortPtr++;
               break;

            case clvTYPE_BOOL:
               bufPtr.boolPtr++;
               break;

            case clvTYPE_BOOL_PACKED:
               bufPtr.charPtr++;
               break;

            case clvTYPE_INT:
               bufPtr.intPtr++;
               break;

            case clvTYPE_UINT:
               bufPtr.uintPtr++;
               break;

            case clvTYPE_FLOAT:
               bufPtr.floatPtr++;
               break;

            case clvTYPE_LONG:
               bufPtr.longPtr++;
               break;

            case clvTYPE_ULONG:
               bufPtr.ulongPtr++;
               break;

            default:
               gcmASSERT(0);
               break;
            }

            if(bufPtr.charPtr > endBufPtr) {
               gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                               0,
                                               0,
                                               clvREPORT_FATAL_ERROR,
                                               "write beyond buffer boundary"));
               return gcvSTATUS_INVALID_DATA;
            }
         }

         switch(elementType) {
         case clvTYPE_CHAR:
         case clvTYPE_CHAR_PACKED:
            *(bufPtr.charPtr)++ = (char)constant->values[i].intValue;
            break;

         case clvTYPE_UCHAR:
         case clvTYPE_UCHAR_PACKED:
            *(bufPtr.ucharPtr)++ = (unsigned char)constant->values[i].uintValue;
            break;

         case clvTYPE_SHORT:
         case clvTYPE_SHORT_PACKED:
            *(bufPtr.shortPtr)++ = (short)constant->values[i].intValue;
            break;

         case clvTYPE_USHORT:
         case clvTYPE_USHORT_PACKED:
            *(bufPtr.ushortPtr)++ = (unsigned short)constant->values[i].uintValue;
            break;

         case clvTYPE_BOOL:
            *(bufPtr.boolPtr)++ = constant->values[i].boolValue;
            break;

         case clvTYPE_BOOL_PACKED:
            *(bufPtr.charPtr)++ = (char)constant->values[i].boolValue;
            break;

         case clvTYPE_INT:
            *(bufPtr.intPtr)++ = constant->values[i].intValue;
            break;

         case clvTYPE_UINT:
            *(bufPtr.uintPtr)++ = constant->values[i].uintValue;
            break;

         case clvTYPE_FLOAT:
            *(bufPtr.floatPtr)++ = constant->values[i].floatValue;
            break;

         case clvTYPE_HALF:
         case clvTYPE_HALF_PACKED:
            *(bufPtr.ushortPtr)++ = (unsigned short)clmF2H(constant->values[i].floatValue);
            break;

         case clvTYPE_LONG:
            *(bufPtr.longPtr)++ = constant->values[i].longValue;
            break;

         case clvTYPE_ULONG:
            *(bufPtr.ulongPtr)++ = constant->values[i].ulongValue;
            break;

         default:
            gcmASSERT(0);
            break;
         }

         if(bufPtr.charPtr > endBufPtr) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                            0,
                                            0,
                                            clvREPORT_FATAL_ERROR,
                                            "write beyond buffer boundary"));
            return gcvSTATUS_INVALID_DATA;
         }
      }
   }

   return gcvSTATUS_OK;
}


gceSTATUS
cloCOMPILER_Compile(
    IN cloCOMPILER Compiler,
    IN cltOPTIMIZATION_OPTIONS OptimizationOptions,
    IN cltDUMP_OPTIONS DumpOptions,
    IN gctUINT StringCount,
    IN gctCONST_STRING Strings[],
    IN gctCONST_STRING Options,
    OUT gcSHADER * Binary,
    OUT gctSTRING * Log
    )
{
    gceSTATUS    status;
    clsPARSER_STATE *parserState;
    gctUINT32    compilerVersion[2];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    *Binary = gcvNULL;

    Compiler->context.optimizationOptions    = OptimizationOptions;
    Compiler->context.extensions    = clvEXTENSION_NONE;
    Compiler->context.dumpOptions    = DumpOptions;
    Compiler->context.scannerState    = clvSCANNER_NORMAL;

    status = cloCOMPILER_ZeroMemoryAllocate(Compiler,
                                            (gctSIZE_T)sizeof(clsPARSER_STATE),
                                            (gctPOINTER *) &parserState);
    if (gcmIS_ERROR(status)) return gcvSTATUS_OUT_OF_MEMORY;
    parserState->state = clvPARSER_NORMAL;
    slmSLINK_LIST_InsertFirst(Compiler->context.parserState, &parserState->node);

    do {
        cloCOMPILER_SetCollectDIE(Compiler, gcvTRUE);

        if (gcSHADER_DumpSource(0)) {
            gctCHAR buffer[512];
            gctCONST_STRING theSource;
            gctUINT offset;
            gctUINT n;
            gctUINT i;
            gctUINT length;

            gcoOS_Print("Compiler Options: %s", Options ? Options : "none");
            for (i = 0; i < StringCount; i++) {
                offset = 0;
                theSource = Strings[i];
                gcoOS_Print("===== Source string %d =====", i);

                length = (gctUINT) gcoOS_StrLen(theSource, gcvNULL);
                for (n = 0; n < length; ++n) {
                    if ((theSource[n] == '\n') ||
                        (offset == gcmSIZEOF(buffer) - 2)) {
                        if (theSource[n] != '\n') {
                            /* take care the last char */
                            buffer[offset++] = theSource[n];
                        }
                        buffer[offset] = '\0';
                        gcoOS_Print("%s", buffer);
                        offset = 0;
                    }
                    else {
                        /* don't print \r, which may be from a DOS format file */
                        if (theSource[n] != '\r')
                            buffer[offset++] = theSource[n];
                    }
                }
                if (offset != 0) {
                    buffer[offset] = '\0';
                    gcoOS_Print("%s", buffer);
                }
                gcoOS_Print("==================");
            }
        }
        /* Parse the source string */
        status = cloCOMPILER_Parse(Compiler,
                                   StringCount,
                                   Strings,
                                   Options);
        if(gcmIS_ERROR(status)) {
            break;
        }


        gcmONERROR(cloCOMPILER_DumpIR(Compiler));

        if (Compiler->context.errorCount > 0) {
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        cloCOMPILER_SetCollectDIE(Compiler, gcvFALSE);

        /* Construct the binary */
        gcmONERROR(gcSHADER_Construct(Compiler->shaderType,
                                      &Compiler->binary));

        cloCOMPILER_GetVersion(Compiler,
                               Compiler->shaderType,
                               compilerVersion);

        gcmONERROR(gcSHADER_SetCompilerVersion(Compiler->binary,
                                               compilerVersion));
#if cldSupportMultiKernelFunction
        gcmONERROR(gcSHADER_SetMaxKernelFunctionArgs(Compiler->binary,
                                                     Compiler->context.maxKernelFunctionArgs));
#endif

        /* turn off unrolling if VIR is used */
#if _TURN_OFF_OPTIMIZATION_LOOP_UNROLL
        if(gcGetHWCaps()->hwFeatureFlags.hasHalti2 &&
           gcmOPT_CLUseVIRCodeGen() &&
           (!Compiler->context.hasInt64 ||
            gcmOPT_oclInt64InVIR())) {
            Compiler->context.optimizationOptions &= ~clvOPTIMIZATION_UNROLL_ITERATION;
        }
#endif

        /* Generate the code */
        gcmONERROR(cloCOMPILER_GenCode(Compiler));

        if (Compiler->context.errorCount > 0) {
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        gcmONERROR(gcSHADER_SetPrivateMemorySize(Compiler->binary,
                                                 Compiler->context.privateMemorySize));

        if(Compiler->context.constantMemorySize) {
            gctCHAR *buffer;
            gctPOINTER pointer;

            status = cloCOMPILER_Allocate(Compiler,
                                          (gctSIZE_T)sizeof(gctCHAR) * Compiler->context.constantMemorySize + 4,
                                          &pointer);
            if (gcmIS_ERROR(status)) return gcvSTATUS_OUT_OF_MEMORY;

            buffer = pointer;
            status = cloCompiler_InitializeConstantMemory(Compiler,
                                                          Compiler->context.constantMemorySize,
                                                          buffer);
            if (gcmIS_ERROR(status)) {
                cloCOMPILER_Free(Compiler, buffer);
                break;
            }

            gcmONERROR(gcSHADER_SetConstantMemorySize(Compiler->binary,
                                                      Compiler->context.constantMemorySize,
                                                      buffer));
            cloCOMPILER_Free(Compiler, buffer);
        }

        gcShaderClrHasInt64(Compiler->binary);
        if(_SUPPORT_LONG_ULONG_DATA_TYPE &&
           Compiler->context.hasInt64) {
           gcShaderSetHasInt64(Compiler->binary);
        }

        gcShaderClrHasImageQuery(Compiler->binary);
        if(Compiler->context.hasImageQuery) {
           gcShaderSetHasImageQuery(Compiler->binary);
        }

        gcShaderClrHasVivVxExtension(Compiler->binary);
        if(cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)) {
           gcShaderSetHasVivVxExtension(Compiler->binary);
        }

        gcShaderClrHasVivGcslDriverImage(Compiler->binary);
        if(cloCOMPILER_IsGcslDriverImage(Compiler)) {
           gcShaderSetHasVivGcslDriverImage(Compiler->binary);
        }

        /* Pack the binary */
        gcmONERROR(gcSHADER_Pack(Compiler->binary));

        gcmONERROR(gcSHADER_AnalyzeFunctions(Compiler->binary, gcvTRUE));

        if (StringCount == 1)
        {
            gcSHADER shader = clTuneKernel(Compiler->binary, Strings[0], Options);
            if (shader != gcvNULL)
            {
                gcSHADER_Destroy(Compiler->binary);
                Compiler->binary = shader;
            }
        }
        cloCOMPILER_DumpDIETree(Compiler);

        gcSHADER_SetDebugInfo(Compiler->binary, Compiler->context.debugInfo);
        Compiler->context.debugInfo = gcvNULL;

        /* Return */
        *Binary    = Compiler->binary;
        Compiler->binary = gcvNULL;

        /* copy the source code to shader binary when debug option is on */
        if (StringCount == 1 && gcmOPT_EnableDebugDumpALL())
        {
            (*Binary)->sourceLength = (gctUINT)gcoOS_StrLen(Strings[0], gcvNULL) + 1;
            gcoOS_StrDup(gcvNULL, Strings[0], &(*Binary)->source);
        }

        if (Log != gcvNULL) {
            *Log    = Compiler->log;
            Compiler->log    = gcvNULL;
        }
        return gcvSTATUS_OK;
    } while (gcvFALSE);

OnError:
    *Binary = gcvNULL;


    if (Log != gcvNULL) {
        *Log    = Compiler->log;
        Compiler->log    = gcvNULL;
    }
    return status;
}

/* Dump IR data */
gceSTATUS
cloCOMPILER_DumpIR(
IN cloCOMPILER Compiler
)
{
    clsDATA_TYPE *dataType;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    if (!(Compiler->context.dumpOptions & clvDUMP_IR)) return gcvSTATUS_OK;
    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                    clvDUMP_IR,
                    "<IR>"));

    /* Dump all data types */
    FOR_EACH_DLINK_NODE(&Compiler->context.dataTypes, clsDATA_TYPE, dataType) {
        gcmVERIFY_OK(clsDATA_TYPE_Dump(Compiler, dataType));
    }

    /* Dump all name spaces and names */
    if (Compiler->context.globalSpace != gcvNULL) {
        gcmVERIFY_OK(clsNAME_SPACE_Dump(Compiler, Compiler->context.globalSpace));
    }

    /* Dump syntax tree */
    if (Compiler->context.rootSet != gcvNULL) {
        gcmVERIFY_OK(cloIR_OBJECT_Dump(Compiler, &Compiler->context.rootSet->base));
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                clvDUMP_IR,
                "</IR>"));
    return gcvSTATUS_OK;
}

gctUINT32
cloGetDefaultLanguageVersion()
{
    gctUINT chipModel = gcGetHWCaps()->chipModel;
    gctUINT chipRevision = gcGetHWCaps()->chipRevision;

    if(((chipModel == gcv1500 && chipRevision == 0x5246) ||
        (chipModel == gcv2000 && chipRevision == 0x5108) ||
        (chipModel == gcv3000 && chipRevision == 0x5513)))
    {
        return _cldCL1Dot1;
    }
    else
    {
        return _cldCL1Dot2;
    }
}

gceSTATUS
cloCOMPILER_SetScannerState(
IN cloCOMPILER Compiler,
IN cleSCANNER_STATE State
)
{
    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);
    Compiler->context.scannerState = State;
    return gcvSTATUS_OK;
}

cleSCANNER_STATE
cloCOMPILER_GetScannerState(
IN cloCOMPILER Compiler
)
{
    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);
    return Compiler->context.scannerState;
}

gceSTATUS
cloCOMPILER_SetParserState(
IN cloCOMPILER Compiler,
IN clePARSER_STATE State
)
{
    if( !slmSLINK_LIST_IsEmpty(Compiler->context.parserState) ) {
       clsPARSER_STATE *parserState;
       parserState =
           slmSLINK_LIST_First(Compiler->context.parserState, clsPARSER_STATE);
       parserState->state = State;
       return gcvSTATUS_OK;
    }
    else return cloCOMPILER_PushParserState(Compiler, State);
}

gceSTATUS
cloCOMPILER_PushParserState(
IN cloCOMPILER Compiler,
IN clePARSER_STATE State
)
{
    gceSTATUS status;
    clsPARSER_STATE *parserState;

    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);
    status = cloCOMPILER_ZeroMemoryAllocate(Compiler,
                                            (gctSIZE_T)sizeof(clsPARSER_STATE),
                                            (gctPOINTER *) &parserState);
    if (gcmIS_ERROR(status)) return gcvSTATUS_OUT_OF_MEMORY;
    parserState->state = State;
    slmSLINK_LIST_InsertFirst(Compiler->context.parserState, &parserState->node);
    return gcvSTATUS_OK;
}


gceSTATUS
cloCOMPILER_PopParserState(
IN cloCOMPILER Compiler
)
{
    if( !slmSLINK_LIST_IsEmpty(Compiler->context.parserState) ) {
       clsPARSER_STATE *oldState;
       slmSLINK_LIST_DetachFirst(Compiler->context.parserState, clsPARSER_STATE, &oldState);
       gcmVERIFY_OK(cloCOMPILER_Free(Compiler, oldState));
    }
    return gcvSTATUS_OK;
}

clsPARSER_STATE *
cloCOMPILER_GetParserStateHandle(
IN cloCOMPILER Compiler
)
{
    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);

    if(!slmSLINK_LIST_IsEmpty(Compiler->context.parserState) ) {
       return slmSLINK_LIST_First(Compiler->context.parserState, clsPARSER_STATE);
    }
    return gcvNULL;
}

clePARSER_STATE
cloCOMPILER_GetParserState(
IN cloCOMPILER Compiler
)
{
    clsPARSER_STATE *parserState;
    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);

    parserState = cloCOMPILER_GetParserStateHandle(Compiler);

    if(parserState) {
       return parserState->state;
    }
    else return clvPARSER_NORMAL;
}

gceSTATUS
cloCOMPILER_AddReferencedBuiltinFunc(
IN cloCOMPILER Compiler,
IN cloIR_POLYNARY_EXPR FuncCall
)
{
   gceSTATUS status = gcvSTATUS_OK;
   clsBUILTIN_FUNC_REFERENCED *builtin;
   gctPOINTER pointer;

   status = cloCOMPILER_Allocate(Compiler,
                                 (gctSIZE_T)sizeof(clsBUILTIN_FUNC_REFERENCED),
                 (gctPOINTER *) &pointer);
   if (gcmIS_ERROR(status)) return status;
   builtin = pointer;

   builtin->member = FuncCall;
   slmSLINK_LIST_InsertLast(Compiler->context.builtinFuncReferenced, &builtin->node);
   return status;
}

slsSLINK_LIST *
cloCOMPILER_GetReferencedBuiltinFuncList(
IN cloCOMPILER Compiler
)
{
   return Compiler->context.builtinFuncReferenced;
}

gceSTATUS
cloCOMPILER_AddConstantVariable(
IN cloCOMPILER Compiler,
IN clsNAME *ConstantVariable
)
{
   gceSTATUS status = gcvSTATUS_OK;
   clsCONSTANT_VARIABLE *constantVariable;
   gctPOINTER pointer;

   status = cloCOMPILER_Allocate(Compiler,
                                 (gctSIZE_T)sizeof(clsCONSTANT_VARIABLE),
                                 (gctPOINTER *) &pointer);
   if (gcmIS_ERROR(status)) return status;
   constantVariable = pointer;

   constantVariable->member = ConstantVariable;
   slmSLINK_LIST_InsertLast(Compiler->context.constantVariables, &constantVariable->node);
   return status;
}


gceSTATUS
cloCOMPILER_FindConstantVariable(
IN cloCOMPILER Compiler,
IN gctINT Offset,
IN OUT clsNAME **ConstantVariable
)
{
   clsCONSTANT_VARIABLE *prevConstVar;
   clsCONSTANT_VARIABLE *nextConstVar;

   FOR_EACH_SLINK_NODE(Compiler->context.constantVariables, clsCONSTANT_VARIABLE, prevConstVar, nextConstVar) {
      clsNAME *constantVariable;

      constantVariable = nextConstVar->member;
      gcmASSERT(clmNAME_VariableHasMemoryOffset(constantVariable));
      if(Offset == clmNAME_VariableMemoryOffset_NOCHECK_GET(constantVariable)) {
          gcmASSERT(constantVariable->u.variableInfo.u.constant);

          *ConstantVariable = constantVariable;
          return gcvSTATUS_OK;
      }
      else if(Offset < clmNAME_VariableMemoryOffset_NOCHECK_GET(constantVariable)) {
          if(prevConstVar) {
              gcmASSERT(prevConstVar->member->u.variableInfo.u.constant);
              *ConstantVariable = prevConstVar->member;
              return gcvSTATUS_OK;
          }
          else break;
      }
   }

   *ConstantVariable = gcvNULL;
   return gcvSTATUS_NOT_FOUND;
}

gceSTATUS
cloCOMPILER_PushSwitchScope(
IN cloCOMPILER Compiler,
IN cloIR_LABEL Cases
)
{
    gceSTATUS status;
    clsSWITCH_SCOPE *switchScope;
    gctPOINTER pointer;

    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);
    status = cloCOMPILER_Allocate(Compiler,
                                  (gctSIZE_T)sizeof(clsSWITCH_SCOPE),
                                  (gctPOINTER *) &pointer);
    if (gcmIS_ERROR(status)) return gcvSTATUS_OUT_OF_MEMORY;
    switchScope = pointer;
    switchScope->cases = Cases;
    slmSLINK_LIST_InsertFirst(Compiler->context.switchScope, &switchScope->node);
    return gcvSTATUS_OK;
}


gceSTATUS
cloCOMPILER_PopSwitchScope(
IN cloCOMPILER Compiler
)
{
    if( !slmSLINK_LIST_IsEmpty(Compiler->context.switchScope) ) {
       clsSWITCH_SCOPE *oldScope;
       slmSLINK_LIST_DetachFirst(Compiler->context.switchScope, clsSWITCH_SCOPE, &oldScope);
       gcmVERIFY_OK(cloCOMPILER_Free(Compiler, oldScope));
    }
    return gcvSTATUS_OK;
}

clsSWITCH_SCOPE *
cloCOMPILER_GetSwitchScope(
IN cloCOMPILER Compiler
)
{
    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);

    if( !slmSLINK_LIST_IsEmpty(Compiler->context.switchScope) ) {
       return slmSLINK_LIST_First(Compiler->context.switchScope, clsSWITCH_SCOPE);
    }
    return gcvNULL;
}

gceSTATUS
cloCOMPILER_SetSwitchScope(
IN cloCOMPILER Compiler,
IN cloIR_LABEL Cases
)
{
    if( !slmSLINK_LIST_IsEmpty(Compiler->context.switchScope) ) {
       clsSWITCH_SCOPE *switchScope;
       switchScope =
           slmSLINK_LIST_First(Compiler->context.switchScope, clsSWITCH_SCOPE);
       switchScope->cases = Cases;
       return gcvSTATUS_OK;
    }
    else {
           return cloCOMPILER_PushSwitchScope(Compiler, Cases);
    }
}

gceSTATUS
cloCOMPILER_PushDesignationScope(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Designation
)
{
    gceSTATUS status;
    clsDESIGNATION_SCOPE *designationScope;
    gctPOINTER pointer;

    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);
    status = cloCOMPILER_Allocate(Compiler,
                      (gctSIZE_T)sizeof(clsDESIGNATION_SCOPE),
                      (gctPOINTER *) &pointer);
    if (gcmIS_ERROR(status)) return gcvSTATUS_OUT_OF_MEMORY;
    designationScope = pointer;
    designationScope->designation = Designation;
    slmSLINK_LIST_InsertFirst(Compiler->context.designationScope, &designationScope->node);
    return gcvSTATUS_OK;
}


gceSTATUS
cloCOMPILER_PopDesignationScope(
IN cloCOMPILER Compiler
)
{
    if( !slmSLINK_LIST_IsEmpty(Compiler->context.designationScope) ) {
       clsDESIGNATION_SCOPE *oldScope;
       slmSLINK_LIST_DetachFirst(Compiler->context.designationScope, clsDESIGNATION_SCOPE, &oldScope);
       gcmVERIFY_OK(cloCOMPILER_Free(Compiler, oldScope));
    }
    return gcvSTATUS_OK;
}

clsDESIGNATION_SCOPE *
cloCOMPILER_GetDesignationScope(
IN cloCOMPILER Compiler
)
{
    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);

    if( !slmSLINK_LIST_IsEmpty(Compiler->context.designationScope) ) {
       return slmSLINK_LIST_First(Compiler->context.designationScope, clsDESIGNATION_SCOPE);
    }
    return gcvNULL;
}

gceSTATUS
cloCOMPILER_SetDesignationScope(
IN cloCOMPILER Compiler,
IN cloIR_EXPR Designation
)
{
    if( !slmSLINK_LIST_IsEmpty(Compiler->context.designationScope) ) {
       clsDESIGNATION_SCOPE *designationScope;
       designationScope =
           slmSLINK_LIST_First(Compiler->context.designationScope, clsDESIGNATION_SCOPE);
       designationScope->designation = Designation;
       return gcvSTATUS_OK;
    }
    else {
           return cloCOMPILER_PushDesignationScope(Compiler, Designation);
    }
}

cloPREPROCESSOR
cloCOMPILER_GetPreprocessor(
IN cloCOMPILER Compiler
)
{
    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);
    return Compiler->preprocessor;
}

cloCODE_EMITTER
cloCOMPILER_GetCodeEmitter(
IN cloCOMPILER Compiler
)
{
    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);
    return Compiler->codeEmitter;
}

cloCODE_GENERATOR
cloCOMPILER_GetCodeGenerator(
IN cloCOMPILER Compiler
)
{
    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);
    return Compiler->codeGenerator;
}

gctBOOL
cloCOMPILER_OptimizationEnabled(
IN cloCOMPILER Compiler,
IN cleOPTIMIZATION_OPTION OptimizationOption
)
{
    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);
    return (Compiler->context.optimizationOptions & OptimizationOption);
}

gctBOOL
cloCOMPILER_ExtensionEnabled(
IN cloCOMPILER Compiler,
IN cleEXTENSION Extension
)
{
    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);
    return (Compiler->context.extensions & Extension);
}

cleEXTENSION
cloCOMPILER_GetExtension(
IN cloCOMPILER Compiler
)
{
    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);
    return Compiler->context.extensions ;
}

/* Enable extension */
gceSTATUS
cloCOMPILER_EnableExtension(
IN cloCOMPILER Compiler,
IN cleEXTENSION Extension,
IN gctBOOL Enable
)
{
    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);

    if (Enable) {
        Compiler->context.extensions |= Extension;
    }
    else {
        Compiler->context.extensions &= ~Extension;
    }
    clScanInitLanguageVersion(Compiler->langVersion,
                              Compiler->context.extensions);
    return gcvSTATUS_OK;
}

gctBOOL
cloCOMPILER_IsBasicTypePacked(
IN cloCOMPILER Compiler
)
{
    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);
    return Compiler->context.basicTypePacked;
}

/* Set the packed attribute for basic type */
gceSTATUS
cloCOMPILER_SetBasicTypePacked(
IN cloCOMPILER Compiler
)
{
    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);

    Compiler->context.basicTypePacked = gcvTRUE;
    return gcvSTATUS_OK;
}

gctBOOL
cloCOMPILER_IsGcslDriverImage(
IN cloCOMPILER Compiler
)
{
    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);
    return Compiler->context.gcslDriverImage;
}

/* Set image in gcsl driver mode */
gceSTATUS
cloCOMPILER_SetGcslDriverImage(
IN cloCOMPILER Compiler
)
{
    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);

    Compiler->context.gcslDriverImage = gcvTRUE;
    return gcvSTATUS_OK;
}

gctBOOL
cloCOMPILER_IsLongUlongPatch(
IN cloCOMPILER Compiler
)
{
    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);
    return Compiler->context.longUlongPatch;
}

/* Set long ulong patch library */
gceSTATUS
cloCOMPILER_SetLongUlongPatch(
IN cloCOMPILER Compiler
)
{
    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);

    Compiler->context.longUlongPatch = gcvTRUE;
    return gcvSTATUS_OK;
}

#define MAX_ERROR    100

gceSTATUS
cloCOMPILER_VReport(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN cleREPORT_TYPE Type,
IN gctCONST_STRING Message,
IN gctARGUMENTS Arguments
)
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    switch (Type) {
    case clvREPORT_FATAL_ERROR:
    case clvREPORT_INTERNAL_ERROR:
    case clvREPORT_ERROR:
        if (Compiler->context.errorCount >= MAX_ERROR) return gcvSTATUS_OK;
        break;
    default: break;
    }

    if (LineNo > 0) {
        cloCOMPILER_OutputLog(Compiler, "(%d:%d) : ", LineNo, StringNo);
    }

    switch (Type) {
    case clvREPORT_FATAL_ERROR:
        Compiler->context.errorCount = MAX_ERROR;
        cloCOMPILER_OutputLog(Compiler, "fatal error : ");
        break;

    case clvREPORT_INTERNAL_ERROR:
        Compiler->context.errorCount++;
        cloCOMPILER_OutputLog(Compiler, "internal error : ");
        break;

    case clvREPORT_ERROR:
        Compiler->context.errorCount++;
        cloCOMPILER_OutputLog(Compiler, "error : ");
        break;

    case clvREPORT_WARN:
        Compiler->context.warnCount++;
        cloCOMPILER_OutputLog(Compiler, "warning : ");
        break;

    case clvREPORT_INFO:
        cloCOMPILER_OutputLog(Compiler, "info : ");
        break;

    default:
        gcmASSERT(0);
    }

    cloCOMPILER_VOutputLog(Compiler, Message, Arguments);
    cloCOMPILER_OutputLog(Compiler, "\n");
    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_Report(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN cleREPORT_TYPE Type,
IN gctCONST_STRING Message,
IN ...
)
{
    gceSTATUS    status;
    gctARGUMENTS arguments;

    gcmARGUMENTS_START(arguments, Message);
    status = cloCOMPILER_VReport(Compiler,
                     LineNo,
                     StringNo,
                     Type,
                     Message,
                     arguments);
    gcmARGUMENTS_END(arguments);

    return status;
}

gceSTATUS
cloCOMPILER_DumpDIE(
    IN cloCOMPILER Compiler,
    IN cleDUMP_OPTION DumpOption,
    IN gctUINT16 id
    )
{
    if (gcmOPT_EnableDebugDump())
        vscDIDumpDIE(Compiler->context.debugInfo, id, 0, 0xffffffff);

    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_Dump(
IN cloCOMPILER Compiler,
IN cleDUMP_OPTION DumpOption,
IN gctCONST_STRING Message,
IN ...
)
{
    gceSTATUS    status;
    gctARGUMENTS arguments;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    if (!(Compiler->context.dumpOptions & DumpOption)) return gcvSTATUS_OK;

    gcmARGUMENTS_START(arguments, Message);
    status = cloCOMPILER_VOutputLog(Compiler, Message, arguments);
    gcmARGUMENTS_END(arguments);

    return status;
}


gceSTATUS
cloCOMPILER_Allocate(
IN cloCOMPILER Compiler,
IN gctSIZE_T Bytes,
OUT gctPOINTER * Memory
)
{
    gceSTATUS    status = gcvSTATUS_OK;
#if !(__USE_VSC_MP__ || __USE_VSC_BMS__)
    gctSIZE_T    bytes;
    slsDLINK_NODE * node;
#else
    gctPOINTER      pointer = gcvNULL;
#endif

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

#if (__USE_VSC_MP__ || __USE_VSC_BMS__)
    pointer = vscMM_Alloc(Compiler->currentMmWrapper, (gctUINT)Bytes);
    if (!pointer) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        0,
                        0,
                        clvREPORT_FATAL_ERROR,
                        "not enough memory"));
        status = gcvSTATUS_OUT_OF_MEMORY;
        return status;
    }
#else
    bytes = Bytes + sizeof(slsDLINK_NODE);
    status = gcoOS_Allocate(gcvNULL,
                bytes,
                (gctPOINTER *) &node);

    if (gcmIS_ERROR(status)) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        0,
                        0,
                        clvREPORT_FATAL_ERROR,
                        "not enough memory"));
        return status;
    }
#endif

#if (__USE_VSC_MP__ || __USE_VSC_BMS__)
    if (Memory)
    {
        *Memory = pointer;
    }
#else
    /* Add node into the memory pool */
    if (Compiler->context.loadingGeneralBuiltIns)
    {
        slsDLINK_LIST_InsertLast(&Compiler->generalMemoryPool, node);
    }
    else
    {
        slsDLINK_LIST_InsertLast(&Compiler->memoryPool, node);
    }
    *Memory = (gctPOINTER)(node + 1);
#endif

    return status;
}

gceSTATUS
cloCOMPILER_ZeroMemoryAllocate(
IN cloCOMPILER Compiler,
IN gctSIZE_T Bytes,
OUT gctPOINTER * Memory
)
{
    gceSTATUS status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    status = cloCOMPILER_Allocate(Compiler,
                      Bytes,
                      Memory);
    if (gcmIS_ERROR(status)) return status;

    gcoOS_ZeroMemory((gctPOINTER)*Memory, Bytes);

    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_Free(
IN cloCOMPILER Compiler,
IN gctPOINTER Memory
)
{
#if !(__USE_VSC_MP__ || __USE_VSC_BMS__)
    slsDLINK_NODE * node;
#endif

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

#if (__USE_VSC_MP__ || __USE_VSC_BMS__)
    vscMM_Free(Compiler->currentMmWrapper, Memory);
    return gcvSTATUS_OK;
#else
    node = (slsDLINK_NODE *)Memory - 1;

    /* Detach node from the memory pool */
    slsDLINK_NODE_Detach(node);

    return gcoOS_Free(gcvNULL, (gctPOINTER)node);
#endif
}

#if !(__USE_VSC_MP__ || __USE_VSC_BMS__)
#define _cldNAME_POOL_SIZE 2048

/* Allocate name for the compiler */
gceSTATUS
cloCOMPILER_AllocateName(
    IN cloCOMPILER Compiler,
    OUT gctPOINTER * Memory
    )
{
    gceSTATUS status;
    gctSIZE_T bytes = (gctSIZE_T)sizeof(clsNAME) * _cldNAME_POOL_SIZE;
    gctPOINTER pointer = gcvNULL;
    clsNAME_POOL *namePool;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    if (Compiler->context.loadingGeneralBuiltIns)
    {
        if (Compiler->context.generalNamePoolSize == 0 ||
            Compiler->context.generalNameCount >= Compiler->context.generalNamePoolSize)
        {
            status = cloCOMPILER_Allocate(Compiler,
                                     (gctSIZE_T)sizeof(clsNAME_POOL),
                                     (gctPOINTER *) &pointer);
            if (gcmIS_ERROR(status)) return status;
            namePool = pointer;

            status = cloCOMPILER_Allocate(Compiler,
                                          bytes,
                                          &pointer);
            if (gcmIS_ERROR(status)) return status;
            gcoOS_ZeroMemory(pointer, bytes);

            namePool->member = pointer;
            slmSLINK_LIST_InsertFirst(Compiler->context.generalNamePools, &namePool->node);
            Compiler->context.curGeneralNamePool = namePool;
            Compiler->context.generalNamePoolSize = _cldNAME_POOL_SIZE;
            Compiler->context.generalNameCount = 0;
        }

        *Memory = Compiler->context.curGeneralNamePool->member + Compiler->context.generalNameCount;
        Compiler->context.generalNameCount++;

        return gcvSTATUS_OK;
    }

    if (Compiler->context.namePoolSize == 0 ||
        Compiler->context.nameCount >= Compiler->context.namePoolSize)
    {
        status = cloCOMPILER_Allocate(Compiler,
                                 (gctSIZE_T)sizeof(clsNAME_POOL),
                                 (gctPOINTER *) &pointer);
        if (gcmIS_ERROR(status)) return status;
        namePool = pointer;

        status = cloCOMPILER_Allocate(Compiler,
                                      bytes,
                                      &pointer);
        if (gcmIS_ERROR(status)) return status;
        gcoOS_ZeroMemory(pointer, bytes);

        namePool->member = pointer;
        slmSLINK_LIST_InsertFirst(Compiler->context.namePools, &namePool->node);
        Compiler->context.curNamePool = namePool;
        Compiler->context.namePoolSize = _cldNAME_POOL_SIZE;
        Compiler->context.nameCount = 0;
    }

    *Memory = Compiler->context.curNamePool->member + Compiler->context.nameCount;
    Compiler->context.nameCount++;
    return gcvSTATUS_OK;
}
#endif

gceSTATUS
cloCOMPILER_DetachFromMemoryPool(
IN cloCOMPILER Compiler,
IN gctPOINTER Memory
)
{
#if !(__USE_VSC_MP__ || __USE_VSC_BMS__)
    slsDLINK_NODE *    node;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    node = (slsDLINK_NODE *)Memory - 1;

    /* Detach node from the memory pool */
    slsDLINK_NODE_Detach(node);
#endif
    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_EmptyMemoryPool(
IN cloCOMPILER Compiler
)
{
#if !(__USE_VSC_MP__ || __USE_VSC_BMS__)
    slsDLINK_NODE *    node;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    /* Free all unfreed memory block in the pool */
    while (!slsDLINK_LIST_IsEmpty(&Compiler->memoryPool)) {
        slsDLINK_LIST_DetachFirst(&Compiler->memoryPool, slsDLINK_NODE, &node);
        gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)node));
    }
#endif

    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_FindGeneralPoolString(
IN cloCOMPILER Compiler,
IN gctCONST_STRING String,
OUT cltPOOL_STRING * PoolString
)
{
    slsDLINK_NODE *generalBucket = gcvNULL;
    clsPOOL_STRING_NODE *node;
    gctUINT      crc32Value = clEvaluateCRC32ForShaderString(String,
                                                             (gctUINT)gcoOS_StrLen(String, gcvNULL));

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    generalBucket = clsHASH_TABLE_Bucket(&Compiler->context.generalStringPool,
                    clmBUCKET_INDEX(clHashString(String)));

    FOR_EACH_DLINK_NODE(generalBucket, clsPOOL_STRING_NODE, node) {
        if (node->crc32Value == crc32Value &&
            gcmIS_SUCCESS(gcoOS_StrCmp(node->string, String))) {
                *PoolString = node->string;
                return gcvSTATUS_OK;
        }
    }
    return gcvSTATUS_NOT_FOUND;
}

gceSTATUS
cloCOMPILER_FindPrivatePoolString(
IN cloCOMPILER Compiler,
IN gctCONST_STRING String,
OUT cltPOOL_STRING * PoolString
)
{
    slsDLINK_NODE *privateBucket = gcvNULL;
    clsPOOL_STRING_NODE *node;
    gctUINT      crc32Value = clEvaluateCRC32ForShaderString(String,
                                                             (gctUINT)gcoOS_StrLen(String, gcvNULL));

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    privateBucket = clsHASH_TABLE_Bucket(&Compiler->context.stringPool,
                    clmBUCKET_INDEX(clHashString(String)));
    FOR_EACH_DLINK_NODE(privateBucket, clsPOOL_STRING_NODE, node) {
        if (node->crc32Value == crc32Value &&
            gcmIS_SUCCESS(gcoOS_StrCmp(node->string, String))) {
                *PoolString = node->string;
                return gcvSTATUS_OK;
        }
    }

    return gcvSTATUS_NOT_FOUND;
}

gceSTATUS
cloCOMPILER_FindPoolString(
IN cloCOMPILER Compiler,
IN gctCONST_STRING String,
OUT cltPOOL_STRING * PoolString
)
{
    gceSTATUS    status;
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    if (Compiler->context.loadingGeneralBuiltIns)
    {
        return cloCOMPILER_FindGeneralPoolString(Compiler,
                                                 String,
                                                 PoolString);
    }

    status = cloCOMPILER_FindPrivatePoolString(Compiler,
                                      String,
                                      PoolString);
    if (status == gcvSTATUS_NOT_FOUND)
        status = cloCOMPILER_FindGeneralPoolString(Compiler,
                                          String,
                                          PoolString);
    return status;
}

gceSTATUS
cloCOMPILER_AllocatePoolString(
IN cloCOMPILER Compiler,
IN gctCONST_STRING String,
OUT cltPOOL_STRING * PoolString
)
{
    gceSTATUS    status;
    slsDLINK_NODE *generalBucket = gcvNULL;
    slsDLINK_NODE *privateBucket = gcvNULL;
    slsDLINK_NODE *bucket;
    gctSIZE_T    length;
    clsPOOL_STRING_NODE *node;
    gctUINT      crc32Value = clEvaluateCRC32ForShaderString(String,
                                                             (gctUINT)gcoOS_StrLen(String, gcvNULL));

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    if (!Compiler->context.loadingGeneralBuiltIns)
    {
        privateBucket = clsHASH_TABLE_Bucket(&Compiler->context.stringPool,
                        clmBUCKET_INDEX(clHashString(String)));
        FOR_EACH_DLINK_NODE(privateBucket, clsPOOL_STRING_NODE, node) {
            if (node->crc32Value == crc32Value &&
                gcmIS_SUCCESS(gcoOS_StrCmp(node->string, String))) {
                    *PoolString = node->string;
                    return gcvSTATUS_OK;
            }
        }
    }
    generalBucket = clsHASH_TABLE_Bucket(&Compiler->context.generalStringPool,
                                  clmBUCKET_INDEX(clHashString(String)));
    FOR_EACH_DLINK_NODE(generalBucket, clsPOOL_STRING_NODE, node) {
        if (node->crc32Value == crc32Value &&
            gcmIS_SUCCESS(gcoOS_StrCmp(node->string, String))) {
                *PoolString = node->string;
                return gcvSTATUS_OK;
        }
    }

    if (Compiler->context.loadingGeneralBuiltIns)
    {
        bucket = generalBucket;
    }
    else
    {
        bucket = privateBucket;
    }

    length = gcoOS_StrLen(String, gcvNULL);
    status = cloCOMPILER_Allocate(Compiler,
                    sizeof(clsPOOL_STRING_NODE) + length + 1,
                    (gctPOINTER *) &node);
    if (gcmIS_ERROR(status)) return status;

    node->string = (cltPOOL_STRING)((gctINT8 *)node + sizeof(clsPOOL_STRING_NODE));
    node->crc32Value = crc32Value;
    gcmVERIFY_OK(gcoOS_StrCopySafe(node->string, length + 1, String));
    slsDLINK_LIST_InsertFirst(bucket, &node->node);
    *PoolString = node->string;
    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_GetChar(
IN cloCOMPILER Compiler,
OUT gctINT_PTR Char
)
{
    gcmASSERT(Compiler);
    gcmASSERT(Compiler->context.strings);

    if (Compiler->context.strings[Compiler->context.currentStringNo][Compiler->context.currentCharNo] != '\0') {
       *Char = Compiler->context.strings[Compiler->context.currentStringNo][Compiler->context.currentCharNo++];
    }
    else if (Compiler->context.currentStringNo == Compiler->context.stringCount) {
       *Char = T_EOF;
    }
    else {
       gcmASSERT(Compiler->context.currentStringNo < Compiler->context.stringCount);

       Compiler->context.currentStringNo++;
       Compiler->context.currentCharNo = 0;

       while (Compiler->context.currentStringNo < Compiler->context.stringCount) {
        if (Compiler->context.strings[Compiler->context.currentStringNo][0] != '\0') break;
            gcmVERIFY_OK(cloCOMPILER_SetCurrentStringNo(Compiler, ++(Compiler->context.currentStringNo)));
       }

       if (Compiler->context.currentStringNo == Compiler->context.stringCount) {
        *Char = T_EOF;
       }
       else {
        gcmASSERT(Compiler->context.currentStringNo < Compiler->context.stringCount);
        *Char = Compiler->context.strings[Compiler->context.currentStringNo][Compiler->context.currentCharNo++];
      }
    }

    if (*Char == '\n') {
        gcmVERIFY_OK(cloCOMPILER_SetCurrentLineNo( Compiler, ++(Compiler->context.currentLineNo)));
    }

    return gcvSTATUS_OK;
}

static cloCOMPILER CurrentCompiler    = gcvNULL;

static gceSTATUS
_InitFpCapsFromVcOption(
IN cloCOMPILER Compiler
)
{
    gctUINT32 flag = 0;
    gctUINT vcOptionOclFpCaps;

    vcOptionOclFpCaps = gcmOPT_oclFpCaps();
    if(vcOptionOclFpCaps & VC_OPTION_OCLFPCAPS_FASTRELAXEDMATH) {
        flag |= cldFpFAST_RELAXED_MATH;
    }
    if(vcOptionOclFpCaps & VC_OPTION_OCLFPCAPS_FINITEMATHONLY) {
        flag |= cldFpFINITE_MATH_ONLY;
    }
    if(vcOptionOclFpCaps & VC_OPTION_OCLFPCAPS_ROUNDTOEVEN) {
        flag |= cldFpROUND_TO_NEAREST;
    }
    if(vcOptionOclFpCaps & VC_OPTION_OCLFPCAPS_ROUNDTOZERO) {
        flag |= cldFpROUND_TO_ZERO;
    }
    return cloCOMPILER_SetFpConfig(Compiler,
                                   flag);
}

gceSTATUS
cloCOMPILER_MakeCurrent(
    IN cloCOMPILER Compiler,
    IN gctUINT StringCount,
    IN gctCONST_STRING Strings[],
    IN gctCONST_STRING Options
    )
{
    gceSTATUS status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    CurrentCompiler    = Compiler;
#ifndef CL_SCAN_NO_PREPROCESSOR

    status = cloPREPROCESSOR_SetSourceStrings(
                                        Compiler->preprocessor,
                                        StringCount,
                                        Strings,
                                        Options);
    if (gcmIS_ERROR(status))
    {
        return status;
    }

#else
    CurrentCompiler->context.stringCount    = StringCount;
    CurrentCompiler->context.strings    = Strings;
#endif
    gcmVERIFY_OK(cloCOMPILER_SetCurrentLineNo(CurrentCompiler, 1));
    gcmVERIFY_OK(cloCOMPILER_SetCurrentStringNo(CurrentCompiler, 0));
    CurrentCompiler->context.currentCharNo    = 0;

    /* Load the built-ins */
    status = cloCOMPILER_LoadBuiltins(Compiler);
    if (gcmIS_ERROR(status)) return status;

    /* Set the global scope as current */
    Compiler->context.currentSpace = Compiler->context.globalSpace;
    return _InitFpCapsFromVcOption(Compiler);
}

gctINT
clInput(
IN gctINT MaxSize,
OUT gctSTRING Buffer
)
{
    gceSTATUS   status;
    gctINT      actualSize;

    gcmASSERT(CurrentCompiler);

    status = cloPREPROCESSOR_Parse_New(
                                CurrentCompiler->preprocessor,
                                MaxSize,
                                gcvNULL,
                                Buffer,
                                &actualSize);

    if (gcmIS_ERROR(status))
    {
        return 0;
    }
    return actualSize;

}

gctPOINTER
clMalloc(
IN gctSIZE_T Bytes
)
{
    gceSTATUS status;
    gctSIZE_T_PTR memory;

    status = cloCOMPILER_Allocate(CurrentCompiler,
                      Bytes + gcmSIZEOF(gctSIZE_T_PTR),
                      (gctPOINTER *) &memory);

    if (gcmIS_ERROR(status)) return gcvNULL;

    memory[0] = Bytes;
    return &memory[1];
}

gctPOINTER
clRealloc(
IN gctPOINTER Memory,
IN gctSIZE_T NewBytes
)
{
    gceSTATUS status;
    gctSIZE_T_PTR memory = gcvNULL;

    do {
        gcmERR_BREAK(cloCOMPILER_Allocate(CurrentCompiler,
                          NewBytes + gcmSIZEOF(gctSIZE_T_PTR),
                          (gctPOINTER *) &memory));

        memory[0] = NewBytes;
        gcoOS_MemCopy(memory + 1,
                       Memory,
                       ((gctSIZE_T_PTR) Memory)[-1]);

        gcmERR_BREAK(cloCOMPILER_Free(CurrentCompiler,
                          &((gctSIZE_T_PTR) Memory)[-1]));

        return &memory[1];
    } while (gcvFALSE);

    if (memory != gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Free(CurrentCompiler, memory));
    }
    return gcvNULL;
}

void
clFree(
IN gctPOINTER Memory
)
{
    if (Memory != gcvNULL) {
        gcmVERIFY_OK(cloCOMPILER_Free(CurrentCompiler,
                          &((gctSIZE_T_PTR) Memory)[-1]));
    }
}

void
clReport(
IN gctUINT LineNo,
IN gctUINT StringNo,
IN cleREPORT_TYPE Type,
IN gctSTRING Message,
IN ...
)
{
    gctARGUMENTS arguments;
    gcmASSERT(CurrentCompiler);

    gcmARGUMENTS_START(arguments, Message);
    gcmVERIFY_OK(cloCOMPILER_VReport(CurrentCompiler,
                     LineNo,
                     StringNo,
                     Type,
                     Message,
                     arguments));
    gcmARGUMENTS_END(arguments);
}

gceSTATUS
cloCOMPILER_ClonePtrDscr(
IN cloCOMPILER Compiler,
IN slsSLINK_LIST *Source,
OUT slsSLINK_LIST **Destination
)
{
    gceSTATUS  status;
    clsTYPE_QUALIFIER *prevDscr;
    clsTYPE_QUALIFIER *nextDscr;
    clsTYPE_QUALIFIER *ptrDscrElem;
    slsSLINK_LIST *ptrDscr;

    slmSLINK_LIST_Initialize(ptrDscr);

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    FOR_EACH_SLINK_NODE(Source, clsTYPE_QUALIFIER, prevDscr, nextDscr) {
        status = cloCOMPILER_Allocate(Compiler,
                                      (gctSIZE_T)sizeof(clsTYPE_QUALIFIER),
                                      (gctPOINTER *) &ptrDscrElem);
        if (gcmIS_ERROR(status)) return status;
        *ptrDscrElem = *nextDscr;
        slmSLINK_LIST_InsertLast(ptrDscr, &ptrDscrElem->node);
    }
    *Destination = ptrDscr;
    return gcvSTATUS_OK;
}

/* Get or Contruct Data type */
static gceSTATUS
_clGetOrConstructDataType(
IN cloCOMPILER Compiler,
IN gctINT TokenType,
IN gctPOINTER Generic,
IN cltQUALIFIER AccessQualifier,
IN cltQUALIFIER AddrSpaceQualifier,
OUT clsDATA_TYPE ** DataType
)
{
    gceSTATUS       status;
    clsDATA_TYPE *  dataType;

    gcmHEADER_ARG("Compiler=0x%x TokenType=%d Generic=0x%x"
                  " AccessQualifier=%d AddrSpaceQualifier=%d" ,
                  Compiler, TokenType, Generic, AccessQualifier, AddrSpaceQualifier);

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmVERIFY_ARGUMENT(DataType);

    do {
        clsBUILTIN_DATATYPE_INFO *typeInfo;
        gctPOINTER pointer;

        typeInfo = clGetBuiltinDataTypeInfo(TokenType);
        if(typeInfo != gcvNULL) {
          dataType = typeInfo->typePtr[AccessQualifier][AddrSpaceQualifier];
        }
        else dataType = gcvNULL;

        if(dataType == gcvNULL) {
            if (typeInfo != gcvNULL) {
                status = gcoOS_Allocate(gcvNULL,
                                    (gctSIZE_T)sizeof(clsDATA_TYPE),
                                    &pointer);
                if (gcmIS_ERROR(status)) return status;
                dataType = pointer;

                dataType->type  = TokenType;
                dataType->accessQualifier  = AccessQualifier;
                dataType->addrSpaceQualifier  = AddrSpaceQualifier;
                dataType->u.generic = Generic;
                dataType->elementType = typeInfo->dataType.elementType;
                dataType->matrixSize = typeInfo->dataType.matrixSize;
                dataType->virPrimitiveType = typeInfo->virPrimitiveType;
                typeInfo->typePtr[AccessQualifier][AddrSpaceQualifier] = dataType;
            }
            else {
                gcmONERROR(cloCOMPILER_Allocate(Compiler,
                                                (gctSIZE_T)sizeof(clsDATA_TYPE),
                                                &pointer));
                dataType = pointer;

                dataType->type  = TokenType;
                dataType->accessQualifier  = AccessQualifier;
                dataType->addrSpaceQualifier  = AddrSpaceQualifier;
                dataType->u.generic = Generic;

                switch(TokenType) {
                case T_STRUCT:
                      dataType->elementType = clvTYPE_STRUCT;
                      break;

                case T_UNION:
                      dataType->elementType = clvTYPE_UNION;
                      break;

                case T_ENUM:
                      dataType->elementType = clvTYPE_INT;
                      dataType->virPrimitiveType = VIR_TYPE_INT32;
                      break;

                case T_FLOATNXM:
                      dataType->elementType = clvTYPE_FLOAT;
                      dataType->virPrimitiveType = VIR_TYPE_FLOAT32;
                      break;

                case T_TYPE_NAME:
                      dataType->elementType = clvTYPE_TYPEDEF;
                      break;

                default:
                      gcmASSERT(0);
                }
                dataType->u.generic  = Generic;
                clmDATA_TYPE_matrixSize_SET(dataType, 0, 0);
                slsDLINK_LIST_InsertLast(&Compiler->context.dataTypes, &dataType->node);
            }
        }

        *DataType = dataType;

        gcmFOOTER_ARG("*DataType=0x%x", *DataType);
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

OnError:
    *DataType = gcvNULL;

    gcmFOOTER();
    return status;
}

gceSTATUS
cloCOMPILER_CreateDataType(
IN cloCOMPILER Compiler,
IN gctINT TokenType,
IN gctPOINTER Generic,
IN cltQUALIFIER AccessQualifier,
IN cltQUALIFIER AddrSpaceQualifier,
OUT clsDATA_TYPE ** DataType
)
{
    gceSTATUS       status;
    clsDATA_TYPE *  dataType;

    gcmHEADER_ARG("Compiler=0x%x TokenType=%d Generic=0x%x"
                  " AccessQualifier=%d AddrSpaceQualifier=%d",
                  Compiler, TokenType, Generic, AccessQualifier, AddrSpaceQualifier);

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    status = _clGetOrConstructDataType(Compiler,
                                       TokenType,
                                       Generic,
                                       AccessQualifier,
                                       AddrSpaceQualifier,
                                       &dataType);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    *DataType = dataType;

    gcmFOOTER_ARG("*DataType=0x%x", *DataType);
    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_CreateDecl(
IN cloCOMPILER Compiler,
IN gctINT TokenType,
IN gctPOINTER Generic,
IN cltQUALIFIER AccessQualifier,
IN cltQUALIFIER AddrSpaceQualifier,
OUT clsDECL *Decl
)
{
    gceSTATUS    status;
    clsDATA_TYPE *dataType;

    gcmHEADER_ARG("Compiler=0x%x TokenType=%d Generic=0x%x"
                  " AccessQualifier=%d AddrSpaceQualifier=%d",
                  Compiler, TokenType, Generic, AccessQualifier, AddrSpaceQualifier);

    gcmASSERT(Decl);

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    status = _clGetOrConstructDataType(Compiler,
                                       TokenType,
                                       Generic,
                                       AccessQualifier,
                                       AddrSpaceQualifier,
                                       &dataType);
    if (gcmIS_ERROR(status)) {
        gcmFOOTER();
        return status;
    }

    clmDECL_Initialize(Decl, dataType, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);

    gcmFOOTER_ARG("Decl=0x%x", Decl);
    return gcvSTATUS_OK;
}


static clsARRAY *
_DecrementArrayDim(
IN clsARRAY *Array,
OUT clsARRAY *ResArray
)
{
   gctINT i;
   gcmASSERT(Array && Array->numDim);

   for(i = 0; i < Array->numDim - 1; i++) {
      ResArray->length[i] = Array->length[i+1];
   }
   ResArray->numDim = Array->numDim - 1;
   return ResArray;
}

gceSTATUS
cloCOMPILER_CreateArrayDecl(
IN cloCOMPILER Compiler,
IN clsDATA_TYPE * ElementDataType,
IN clsARRAY *Array,
IN slsSLINK_LIST *PtrDscr,
OUT clsDECL *Decl
)
{
    gcmHEADER_ARG("Compiler=0x%x ElementDataType=0x%x Array=0x%x",
                  Compiler, ElementDataType, Array);

    gcmASSERT(ElementDataType);
    gcmASSERT(Decl);
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    clmDECL_Initialize(Decl, ElementDataType, Array, PtrDscr, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);

    gcmFOOTER_ARG("Decl=0x%x", Decl);
    return gcvSTATUS_OK;
}

static gceSTATUS
_clGetOrConstructElement(
    IN cloCOMPILER Compiler,
    IN clsDECL *CompoundDecl,
    OUT clsDECL *Decl
    )
{
    gceSTATUS    status;
    clsDATA_TYPE *  dataType;

    gcmHEADER_ARG("Compiler=0x%x CompoundDecl=0x%x",
                  Compiler, CompoundDecl);

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmVERIFY_ARGUMENT(CompoundDecl);
    gcmVERIFY_ARGUMENT(Decl);

    do {
        clsBUILTIN_DATATYPE_INFO *typeInfo;
        gctINT componentType;
        clsARRAY *array;
        clsARRAY arrayBuffer[1];
        slsSLINK_LIST *ptrDscr;

        slmSLINK_LIST_Initialize(ptrDscr);
        array = &CompoundDecl->array;
        if (clmDECL_IsMat(CompoundDecl)) {
           gctUINT8 componentCount;
           componentCount = clmDATA_TYPE_matrixRowCount_GET(CompoundDecl->dataType);
           componentType = clGetVectorTerminalToken(CompoundDecl->dataType->elementType,
                                   componentCount);
           typeInfo = clGetBuiltinDataTypeInfo(componentType);
        }
        else {
           componentType = CompoundDecl->dataType->type;
           if(!slmSLINK_LIST_IsEmpty(CompoundDecl->ptrDscr)) {
              status = cloCOMPILER_ClonePtrDscr(Compiler,
                                                CompoundDecl->ptrDscr,
                                                &ptrDscr);
              if (gcmIS_ERROR(status)) break;
           }

           typeInfo = clGetBuiltinDataTypeInfo(componentType);
           if(typeInfo == gcvNULL) {
               if (clmDECL_IsUnderlyingArray(CompoundDecl)) {
                  array = _DecrementArrayDim(&CompoundDecl->array, arrayBuffer);
               }
               else if(ptrDscr) {
                  clParseRemoveIndirectionOneLevel(Compiler, &ptrDscr);
               }
               else {
                   gcmASSERT(0);
                   status = gcvSTATUS_INVALID_ARGUMENT;
                   break;
               }
           }
           else {
               if (clmDECL_IsUnderlyingArray(CompoundDecl)) {
                   array = _DecrementArrayDim(&CompoundDecl->array, arrayBuffer);
               }
               else if(ptrDscr) {
                   clParseRemoveIndirectionOneLevel(Compiler, &ptrDscr);
               }
               else {
                   if(typeInfo->type == typeInfo->componentType) {
                       status = gcvSTATUS_INVALID_ARGUMENT;
                       break;
                   }
                   componentType = typeInfo->componentType;
                   typeInfo = clGetBuiltinDataTypeInfo(componentType);
               }
           }
        }
        if(typeInfo != gcvNULL) {
           dataType = typeInfo->typePtr[CompoundDecl->dataType->accessQualifier]
                              [CompoundDecl->dataType->addrSpaceQualifier];
        }
        else dataType = gcvNULL;

        if(dataType == gcvNULL) {
            gctPOINTER pointer;


            if(typeInfo != gcvNULL) {
                status = gcoOS_Allocate(gcvNULL,
                                    (gctSIZE_T)sizeof(clsDATA_TYPE),
                                    &pointer);
                if (gcmIS_ERROR(status)) return status;
                dataType = pointer;

                dataType->type  = componentType;
                dataType->accessQualifier  = CompoundDecl->dataType->accessQualifier;
                dataType->addrSpaceQualifier  = CompoundDecl->dataType->addrSpaceQualifier;
                dataType->u.generic = CompoundDecl->dataType->u.generic;
                dataType->elementType = typeInfo->dataType.elementType;
                dataType->matrixSize = typeInfo->dataType.matrixSize;
                typeInfo->typePtr[CompoundDecl->dataType->accessQualifier]
                                 [CompoundDecl->dataType->addrSpaceQualifier] = dataType;
            }
            else {
                gcmONERROR(cloCOMPILER_Allocate(Compiler,
                                                (gctSIZE_T)sizeof(clsDATA_TYPE),
                                                &pointer));
                dataType = pointer;

                dataType->type  = componentType;
                dataType->accessQualifier  = CompoundDecl->dataType->accessQualifier;
                dataType->addrSpaceQualifier  = CompoundDecl->dataType->addrSpaceQualifier;
                dataType->u.generic = CompoundDecl->dataType->u.generic;
                dataType->elementType = CompoundDecl->dataType->elementType;
                dataType->matrixSize = CompoundDecl->dataType->matrixSize;
                slsDLINK_LIST_InsertLast(&Compiler->context.dataTypes, &dataType->node);
            }
        }

        clmDECL_Initialize(Decl, dataType, array, ptrDscr,
        array->numDim ? CompoundDecl->ptrDominant : gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
        gcmFOOTER_ARG("Decl=0x%x", Decl);
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

OnError:
    clmDECL_Initialize(Decl, gcvNULL, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);

    gcmFOOTER();
    return status;
}

gceSTATUS
cloCOMPILER_CreateElementDecl(
    IN cloCOMPILER Compiler,
    IN clsDECL * CompoundDecl,
    OUT clsDECL *Decl
    )
{
    gceSTATUS status;
    clsDECL decl;

    gcmHEADER_ARG("Compiler=0x%x CompoundDecl=0x%x",
                  Compiler, CompoundDecl);

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    status = _clGetOrConstructElement(Compiler,
                                      CompoundDecl,
                                      &decl);

    if (gcmIS_ERROR(status)) {
        gcmFOOTER();
        return status;
    }

    *Decl = decl;

    gcmFOOTER_ARG("Decl=0x%x", Decl);
    return gcvSTATUS_OK;
}

static gceSTATUS
_clGetOrCloneDataType(
IN cloCOMPILER Compiler,
IN cltQUALIFIER AccessQualifier,
IN cltQUALIFIER AddrSpaceQualifier,
IN clsDATA_TYPE *Source,
IN gctBOOL AllocateForBuiltin,
OUT clsDATA_TYPE ** DataType
)
{
    gceSTATUS       status;
    clsDATA_TYPE *  dataType;

    gcmHEADER_ARG("Compiler=0x%x AccessQualifier=%d AddrSpaceQualifier=%d Source=0x%x",
                  Compiler, AccessQualifier, AddrSpaceQualifier, Source);

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmVERIFY_ARGUMENT(DataType);

    do {
        clsBUILTIN_DATATYPE_INFO *typeInfo = gcvNULL;

        if (AllocateForBuiltin)
        {
            dataType = gcvNULL;
        }
        else
        {
            typeInfo = clGetBuiltinDataTypeInfo(Source->type);
            if(typeInfo != gcvNULL) {
              dataType = typeInfo->typePtr[AccessQualifier][AddrSpaceQualifier];
            }
            else dataType = gcvNULL;
        }

        if(dataType == gcvNULL) {
            gctPOINTER pointer;

            if(typeInfo != gcvNULL) {
                status = gcoOS_Allocate(gcvNULL,
                                        (gctSIZE_T)sizeof(clsDATA_TYPE),
                                        &pointer);
                dataType = pointer;

                dataType->type  = Source->type;
                dataType->accessQualifier  = AccessQualifier;
                dataType->addrSpaceQualifier  = AddrSpaceQualifier;
                dataType->u.generic = gcvNULL;
                dataType->elementType = typeInfo->dataType.elementType;
                dataType->matrixSize = typeInfo->dataType.matrixSize;
                dataType->virPrimitiveType = typeInfo->virPrimitiveType;
                typeInfo->typePtr[AccessQualifier][AddrSpaceQualifier] = dataType;
            }
            else {
                gcmONERROR(cloCOMPILER_Allocate(Compiler,
                                                (gctSIZE_T)sizeof(clsDATA_TYPE),
                                                &pointer));
                dataType = pointer;

                dataType->type  = Source->type;
                dataType->accessQualifier  = AccessQualifier;
                dataType->addrSpaceQualifier  = AddrSpaceQualifier;
                dataType->u.generic = gcvNULL;

                dataType->elementType = Source->elementType;
                dataType->u.generic  = Source->u.generic;
                dataType->matrixSize = Source->matrixSize;
                dataType->virPrimitiveType = Source->virPrimitiveType;
                slsDLINK_LIST_InsertLast(&Compiler->context.dataTypes, &dataType->node);
            }
        }
        *DataType = dataType;

        gcmFOOTER_ARG("*DataType=0x%x", *DataType);
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);
OnError:
    *DataType = gcvNULL;

    gcmFOOTER();
    return status;
}

gceSTATUS
cloCOMPILER_CloneDataType(
IN cloCOMPILER Compiler,
IN cltQUALIFIER AccessQualifier,
IN cltQUALIFIER AddrSpaceQualifier,
IN clsDATA_TYPE * Source,
OUT clsDATA_TYPE ** DataType
)
{
    gceSTATUS       status;
    clsDATA_TYPE *  dataType;

    gcmHEADER_ARG("Compiler=0x%x AccessQualifier=%d AddrSpaceQualifier=%d Source=0x%x",
                  Compiler, AccessQualifier, AddrSpaceQualifier, Source);

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    status = _clGetOrCloneDataType(Compiler,
                                   AccessQualifier,
                                   AddrSpaceQualifier,
                                   Source,
                                   gcvFALSE,
                                   &dataType);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    *DataType = dataType;

    gcmFOOTER_ARG("*DataType=0x%x", *DataType);
    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_CloneDataTypeExplicit(
IN cloCOMPILER Compiler,
IN cltQUALIFIER AccessQualifier,
IN cltQUALIFIER AddrSpaceQualifier,
IN clsDATA_TYPE * Source,
OUT clsDATA_TYPE ** DataType
)
{
    gceSTATUS       status;
    clsDATA_TYPE *  dataType;

    gcmHEADER_ARG("Compiler=0x%x AccessQualifier=%d AddrSpaceQualifier=%d Source=0x%x",
                  Compiler, AccessQualifier, AddrSpaceQualifier, Source);

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    status = _clGetOrCloneDataType(Compiler,
                                   AccessQualifier,
                                   AddrSpaceQualifier,
                                   Source,
                                   gcvTRUE,
                                   &dataType);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    *DataType = dataType;

    gcmFOOTER_ARG("*DataType=0x%x", *DataType);
    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_CloneDecl(
IN cloCOMPILER Compiler,
IN cltQUALIFIER AccessQualifier,
IN cltQUALIFIER AddrSpaceQualifier,
IN clsDECL *Source,
OUT clsDECL *Decl
)
{
    gceSTATUS status;
    clsDECL decl;

    gcmHEADER_ARG("Compiler=0x%x AccessQualifier=%d AddrSpaceQualifier=%d Source=0x%x",
                  Compiler, AccessQualifier, AddrSpaceQualifier, Source);

    gcmASSERT(Source);
    gcmASSERT(Decl);

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    status = _clGetOrCloneDataType(Compiler,
                                   AccessQualifier,
                                   AddrSpaceQualifier,
                                   Source->dataType,
                                   gcvFALSE,
                                   &decl.dataType);

    if (gcmIS_ERROR(status)) {
        gcmFOOTER();
        return status;
    }

    clmDECL_Initialize(&decl, decl.dataType, &Source->array, gcvNULL, Source->ptrDominant, Source->storageQualifier);
    if(!slmSLINK_LIST_IsEmpty(Source->ptrDscr)) {
       status = cloCOMPILER_ClonePtrDscr(Compiler,
                                         Source->ptrDscr,
                                         &decl.ptrDscr);
       if (gcmIS_ERROR(status)) {
          gcmFOOTER();
          return status;
       }
    }
    *Decl = decl;
    gcmFOOTER_ARG("Decl=0x%x", Decl);
    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_MakeConstantName(
IN cloCOMPILER Compiler,
IN gctSTRING Prefix,
cltPOOL_STRING *SymbolInPool
)
{
    gctUINT offset = 0;
    gctCHAR nameBuf[256];

    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcoOS_PrintStrSafe(nameBuf, 256, &offset, "#__%s_%d",
                       Prefix, Compiler->context.unnamedCount++);

    return cloCOMPILER_AllocatePoolString(Compiler,
                                          nameBuf,
                                          SymbolInPool);
}

gceSTATUS
cloCOMPILER_CreateName(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN cleNAME_TYPE Type,
IN clsDECL *Decl,
IN cltPOOL_STRING Symbol,
IN slsSLINK_LIST *PtrDscr,
IN cleEXTENSION Extension,
OUT clsNAME **Name
)
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    if (!Compiler->context.loadingBuiltins
        && gcmIS_SUCCESS(gcoOS_StrNCmp(Symbol, "gl_", 3))) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        LineNo,
                        StringNo,
                        clvREPORT_ERROR,
                        "The identifier: '%s' starting with 'gl_' is reserved",
                        Symbol));

        return gcvSTATUS_INVALID_ARGUMENT;
    }

    return clsNAME_SPACE_CreateName(Compiler,
                    Compiler->context.currentSpace,
                    LineNo,
                    StringNo,
                    Type,
                    Decl,
                    Symbol,
                    PtrDscr,
                    Type != clvPARAMETER_NAME ?
                    Compiler->context.loadingBuiltins : gcvFALSE,
                    Extension,
                    Name);
}

gceSTATUS
cloCOMPILER_SearchName(
IN cloCOMPILER Compiler,
IN cltPOOL_STRING Symbol,
IN gctBOOL Recursive,
OUT clsNAME ** Name
)
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    return clsNAME_SPACE_Search(Compiler,
                    Compiler->context.currentSpace,
                    Symbol,
                    Recursive,
                    Name);
}

gceSTATUS
cloCOMPILER_SetCurrentLineNo(
IN cloCOMPILER Compiler,
IN gctUINT LineNo
)
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
        Compiler->context.currentLineNo = LineNo;
        clScanSetCurrentLineNo(Compiler, LineNo);
    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_SetCurrentFileName(
IN cloCOMPILER Compiler,
IN gctSTRING Text
)
{
    gceSTATUS status;
    gctSIZE_T length;
    gctPOINTER pointer;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    length = gcoOS_StrLen(Text, gcvNULL);
    if(length > 3 && Text[length - 2] == 'h' && Text[length - 3] =='.')
        Compiler->context.mainFile = gcvFALSE;
    else
        Compiler->context.mainFile = gcvTRUE;
    if(Compiler->context.fileNameBufferSize < length) {
       if(Compiler->context.currentFileName &&
          Compiler->context.currentFileName != Compiler->fileNameBuffer) {
          gcmVERIFY_OK(cloCOMPILER_Free(Compiler, Compiler->context.currentFileName));
          Compiler->context.currentFileName = gcvNULL;
       }
           status = cloCOMPILER_Allocate(Compiler,
                                         sizeof(char) * length + 1,
                         (gctPOINTER *) &pointer);
          if (gcmIS_ERROR(status)) return gcvSTATUS_OUT_OF_MEMORY;
       Compiler->context.currentFileName = pointer;
       Compiler->context.fileNameBufferSize = length;
    }
        gcoOS_StrCopySafe(Compiler->context.currentFileName, length + 1, Text);
        clScanSetCurrentFileName(Compiler, Compiler->context.currentFileName);
    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_SetCurrentStringNo(
IN cloCOMPILER Compiler,
IN gctUINT StringNo
)
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
        Compiler->context.currentStringNo = StringNo;
        clScanSetCurrentStringNo(Compiler, StringNo);
    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_SetIsMainFile(
IN cloCOMPILER Compiler,
IN gctBOOL     isMainFile
)
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    Compiler->context.mainFile = isMainFile;
    return gcvSTATUS_OK;
}

gctUINT
cloCOMPILER_GetCurrentLineNo(
IN cloCOMPILER Compiler
)
{
    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);
    return Compiler->context.currentLineNo;
}

gctUINT
cloCOMPILER_GetCurrentStringNo(
IN cloCOMPILER Compiler
)
{
    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);
    return Compiler->context.currentStringNo;
}

gctSTRING
cloCOMPILER_GetCurrentFileName(
IN cloCOMPILER Compiler
)
{
    /* Verify the arguments. */
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);
    return Compiler->context.currentFileName;
}

gceSTATUS
cloCOMPILER_CheckNewFuncName(
IN cloCOMPILER Compiler,
IN clsNAME * FuncName,
OUT clsNAME ** FirstFuncName
)
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(FuncName);

    return clsNAME_SPACE_CheckNewFuncName(Compiler,
                          Compiler->context.globalSpace,
                          FuncName,
                          FirstFuncName);
}

gceSTATUS
cloCOMPILER_CreateNameSpace(
IN cloCOMPILER Compiler,
OUT clsNAME_SPACE ** NameSpace
)
{
    gceSTATUS    status;
    clsNAME_SPACE *    nameSpace;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(NameSpace);

    status = clsNAME_SPACE_Construct(Compiler,
                    Compiler->context.currentSpace,
                    &nameSpace);

    if (gcmIS_ERROR(status)) return status;

    Compiler->context.currentSpace = nameSpace;
    *NameSpace = nameSpace;
    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_PopCurrentNameSpace(
IN cloCOMPILER Compiler,
OUT clsNAME_SPACE ** PrevNameSpace
)
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    if (Compiler->context.currentSpace == gcvNULL ||
        Compiler->context.currentSpace->parent == gcvNULL) {
        return gcvSTATUS_INTERFACE_ERROR;
    }

    if (PrevNameSpace != gcvNULL) *PrevNameSpace = Compiler->context.currentSpace;
    Compiler->context.currentSpace = Compiler->context.currentSpace->parent;
    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_PushUnnamedSpace(
IN cloCOMPILER Compiler,
OUT clsNAME_SPACE ** UnnamedSpace
)
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    if (Compiler->context.unnamedSpace == gcvNULL) {
        return gcvSTATUS_INTERFACE_ERROR;
    }
    *UnnamedSpace = Compiler->context.unnamedSpace;
    Compiler->context.unnamedSpace->parent = Compiler->context.currentSpace;
    Compiler->context.currentSpace = Compiler->context.unnamedSpace;
    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_AtGlobalNameSpace(
    IN cloCOMPILER Compiler,
    OUT gctBOOL * AtGlobalNameSpace
    )
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(AtGlobalNameSpace);
    gcmASSERT(Compiler->context.currentSpace);
    gcmASSERT(Compiler->context.globalSpace);

    *AtGlobalNameSpace = (Compiler->context.globalSpace == Compiler->context.currentSpace);
    return gcvSTATUS_OK;
}

gceSTATUS
cloIR_SET_IsRoot(
IN cloCOMPILER Compiler,
IN cloIR_SET Set,
OUT gctBOOL * IsRoot
)
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(Set, clvIR_SET);
    gcmASSERT(IsRoot);

    *IsRoot = (Set == Compiler->context.rootSet);
    return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_AddExternalDecl(
IN cloCOMPILER Compiler,
IN cloIR_BASE ExternalDecl
)
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    return cloIR_SET_AddMember(Compiler,
                Compiler->context.rootSet,
                ExternalDecl);
}

gceSTATUS
cloCOMPILER_AddStatementPlaceHolder(
IN cloCOMPILER Compiler,
IN clsNAME *FuncName
)
{
    gceSTATUS status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    gcmASSERT(!FuncName->u.funcInfo.isFuncDef);
    gcmASSERT(!FuncName->u.funcInfo.funcBody);

    status = cloIR_SET_Construct(Compiler,
                     FuncName->lineNo,
                     FuncName->stringNo,
                     clvSTATEMENT_SET,
                     &FuncName->u.funcInfo.funcBody);
    if (gcmIS_ERROR(status)) return status;

    return cloIR_SET_AddMember(Compiler,
                                   Compiler->context.rootSet,
                                   &FuncName->u.funcInfo.funcBody->base);
}

gceSTATUS
cloCOMPILER_BindFuncCall(
IN cloCOMPILER Compiler,
IN OUT cloIR_POLYNARY_EXPR PolynaryExpr
)
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(PolynaryExpr->type == clvPOLYNARY_FUNC_CALL);

    if(Compiler->context.fpConfig & cldFpFAST_RELAXED_MATH) {
         cltPOOL_STRING fastFunc;

         fastFunc = clGetFastRelaxedMathFunction(Compiler,
                                                 PolynaryExpr->funcSymbol);
         if(fastFunc) {
            PolynaryExpr->funcSymbol = fastFunc;
            return clsNAME_SPACE_BindFuncName(Compiler,
                                              Compiler->context.generalBuiltinSpace,
                                              PolynaryExpr);
         }
    }
    /* Bind the function name */
    return clsNAME_SPACE_BindFuncName(Compiler,
                      Compiler->context.globalSpace,
                      PolynaryExpr);
}

gceSTATUS
cloCOMPILER_BindBuiltinFuncCall(
IN cloCOMPILER Compiler,
IN OUT cloIR_POLYNARY_EXPR PolynaryExpr
)
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(PolynaryExpr->type == clvPOLYNARY_FUNC_CALL);

    if(Compiler->context.fpConfig & cldFpFAST_RELAXED_MATH) {
           cltPOOL_STRING fastFunc;

           fastFunc = clGetFastRelaxedMathFunction(Compiler,
                                                   PolynaryExpr->funcSymbol);
           if(fastFunc) {
              PolynaryExpr->funcSymbol = fastFunc;
           }
    }
    /* Bind the builtin function name */
    return clsNAME_SPACE_BindFuncName(Compiler,
                      Compiler->context.generalBuiltinSpace,
                      PolynaryExpr);
}

gctREG_INDEX
clNewTempRegs(
IN cloCOMPILER Compiler,
IN gctUINT RegCount,
IN cltELEMENT_TYPE ElementType
)
{
    gctREG_INDEX    regIndex;
    gctUINT regCount = RegCount;

    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);
#if _GEN_IMAGE_OR_SAMPLER_PARAMETER_VARIABLES
    if(clmIsElementTypeImage(ElementType)) {
#else
    if((cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX) ||
        gcmOPT_oclUseImgIntrinsicQuery()) &&
        clmIsElementTypeImage(ElementType)) {
#endif
        regCount <<= 1;
    }
    if(Compiler->context.inKernelFunctionEpilog) {
       return clNewLocalTempRegs(Compiler, regCount);
    }
    if(!cldHandleHighPrecisionInFrontEnd &&
       clmIsElementTypeHighPrecision(ElementType)) { /* need to reserve double the number of registers */
        regCount <<= 1;
        Compiler->context.hasInt64 = gcvTRUE;
    }
    regIndex = (gctREG_INDEX) gcSHADER_NewTempRegs(Compiler->binary,
                                                   regCount,
                                                   gcSHADER_FLOAT_X1);


    return regIndex;
}

gctREG_INDEX
clNewLocalTempRegs(
IN cloCOMPILER Compiler,
IN gctUINT RegCount
)
{
    gctREG_INDEX    regIndex;

    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);
    regIndex = (gctREG_INDEX) Compiler->context.localTempRegCount;
    Compiler->context.localTempRegCount += RegCount;
    gcmASSERT(Compiler->context.localTempRegCount <= cldMaxLocalTempRegs);
    return regIndex;
}

void
clResetLocalTempRegs(
IN cloCOMPILER Compiler,
IN gctUINT TempIndex
)
{
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);
    if(TempIndex < cldNumMemoryAddressRegs) {
        Compiler->context.localTempRegCount = cldNumMemoryAddressRegs;  /* set aside registers for private and local memory addressing */
    }
    else {  /* Reuse temps after TempIndex */
        gctUINT tempIndex = TempIndex + 1;
        if(TempIndex < cldMaxLocalTempRegs) {
            Compiler->context.localTempRegCount = tempIndex;
        }
        else {
            gcmASSERT(0);
            Compiler->context.localTempRegCount = cldMaxLocalTempRegs - 1;
        }
    }
}

gceSTATUS
cloCOMPILER_SetImageArrayMaxLevel(
    IN cloCOMPILER Compiler,
    IN gctSTRING MaxLevel
)
{
   gceSTATUS status = gcvSTATUS_OK;
   gctINT level = 1;

   gcmHEADER_ARG("Compiler=0x%x MaxLevel=%s",
                 Compiler, MaxLevel);

   if (gcmIS_SUCCESS(gcoOS_StrToInt(MaxLevel, &level)) &&
       level > 0) {
      Compiler->context.imageArrayMaxLevel = level;
   }
   else {
      status = gcvSTATUS_INVALID_DATA;
   }
   gcmFOOTER();
   return status;
}

gctUINT32
cloCOMPILER_GetImageArrayMaxLevel(
    IN cloCOMPILER Compiler
)
{
  return Compiler ? Compiler->context.imageArrayMaxLevel : cldVxImageArrayMaxLevelDefault;
}

gceSTATUS
cloCOMPILER_SetLanguageVersion(
    IN cloCOMPILER Compiler,
    IN gctSTRING LangVersion
)
{
   gceSTATUS status = gcvSTATUS_OK;
   gctUINT32 defaultLanguageVersion = cloGetDefaultLanguageVersion();

   gcmHEADER_ARG("Compiler=0x%x LangVersion=%s",
                 Compiler, LangVersion);

   if (gcmIS_SUCCESS(gcoOS_StrCmp(LangVersion, "CL1.1"))) {
      if(_cldCL1Dot1 <= defaultLanguageVersion) {
          Compiler->langVersion = _cldCL1Dot1;
      }
      else {
          status = gcvSTATUS_INVALID_DATA;
      }
   }
   else if (gcmIS_SUCCESS(gcoOS_StrCmp(LangVersion, "CL1.2"))) {
      if(_cldCL1Dot2 <= defaultLanguageVersion) {
          Compiler->langVersion = _cldCL1Dot2;
      }
      else {
          status = gcvSTATUS_INVALID_DATA;
      }
   }
   else {
      Compiler->langVersion = defaultLanguageVersion;
      status = gcvSTATUS_INVALID_DATA;
   }

   clScanInitLanguageVersion(Compiler->langVersion,
                             Compiler->context.extensions);
   gcmFOOTER();
   return status;
}

gctUINT32
cloCOMPILER_GetLanguageVersion(
    IN cloCOMPILER Compiler
)
{
  return Compiler ? Compiler->langVersion : cloGetDefaultLanguageVersion();
}

void
cloCOMPILER_GetVersion(
    IN cloCOMPILER Compiler,
    IN cleSHADER_TYPE ShaderType,
    IN OUT gctUINT32 *CompilerVersion
)
{
  gctUINT32 version = cloGetDefaultLanguageVersion();
  gcmASSERT(CompilerVersion);
  if(Compiler) {
     version = Compiler->langVersion;
  }
  CompilerVersion[0] = _cldLanguageType | (ShaderType << 16);
  CompilerVersion[1] = version;
  return;
}

gctLABEL
clNewLabel(
IN cloCOMPILER Compiler
)
{
    gctLABEL  label;

    label =  ++Compiler->context.labelCount;
    return label;
}


#define clmSetFloatOpsUsed(BinaryOp) do { \
    if(Compiler->context.hasFloatOps & (BinaryOp)) { \
           if(!(Compiler->context.hasFloatOpsAux & (BinaryOp))) { \
              Compiler->context.hasFloatOpsAux |= (BinaryOp); \
              Compiler->context.hasFloatOps ^= (BinaryOp); \
           } \
    } \
    else Compiler->context.hasFloatOps |= (BinaryOp); \
   } while (gcvFALSE)

void
clSetFloatOpsUsed(
IN cloCOMPILER Compiler,
IN cleBINARY_EXPR_TYPE BinaryOp
)
{
    switch(BinaryOp) {
    case clvBINARY_ADD:
    case clvBINARY_SUB:
    case clvBINARY_MUL:
    case clvBINARY_DIV:
    case clvBINARY_MOD:
        clmSetFloatOpsUsed(BinaryOp);
        break;
    case clvBINARY_ADD_ASSIGN:
        clmSetFloatOpsUsed(clvBINARY_ADD);
        break;
    case clvBINARY_SUB_ASSIGN:
        clmSetFloatOpsUsed(clvBINARY_SUB);
        break;
    case clvBINARY_MUL_ASSIGN:
        clmSetFloatOpsUsed(clvBINARY_MUL);
        break;
    case clvBINARY_DIV_ASSIGN:
        clmSetFloatOpsUsed(clvBINARY_DIV);
        break;
    case clvBINARY_MOD_ASSIGN:
        clmSetFloatOpsUsed(clvBINARY_MOD);
        break;
    default:
        break;
    }
}

gctUINT
clGetFloatOpsUsed(
IN cloCOMPILER Compiler,
IN gctBOOL GtThanTwo
)
{
   if(GtThanTwo) { /*greater than two each */
      return Compiler->context.hasFloatOps & Compiler->context.hasFloatOpsAux;
   }
   else { /* each operator has count 2 or less */
      return Compiler->context.hasFloatOps ^ Compiler->context.hasFloatOpsAux;
   }
}

gceSTATUS
cloCOMPILER_SetFpConfig(
IN cloCOMPILER Compiler,
IN gctUINT32 Flag
)
{
   if(Flag & ~cldFpAllFlags) {
      gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                      0,
                                      0,
                                      clvREPORT_ERROR,
                                      "unrecognizable floating config flags 0x",
                                      Flag & ~cldFpAllFlags));
      return gcvSTATUS_INVALID_DATA;
   }
   else {
      Compiler->context.fpConfig |= Flag;
   }
   return gcvSTATUS_OK;
}

gctUINT32
cloCOMPILER_GetFpConfig(
IN cloCOMPILER Compiler
)
{
      return Compiler->context.fpConfig;

}

gctUINT
cloCOMPILER_FindTopKernelFunc(
IN cloCOMPILER Compiler,
OUT clsNAME **TopKernelFunc
)
{
    gctUINT topKernelCnt;
    clsNAME *name;
    clsNAME *emptyKernel;
    clmASSERT_OBJECT(Compiler, clvOBJ_COMPILER);

    topKernelCnt = 0;
    emptyKernel = gcvNULL;
    *TopKernelFunc = gcvNULL;
    FOR_EACH_DLINK_NODE(&Compiler->context.globalSpace->names, clsNAME, name) {
       if (name->type == clvKERNEL_FUNC_NAME)  {
          if(name->u.funcInfo.isFuncDef
             && !name->u.funcInfo.refCount) {
             if(cloIR_SET_IsEmpty(Compiler, name->u.funcInfo.funcBody)) {
                 emptyKernel = name;
                 continue;
             }

             topKernelCnt++;
             *TopKernelFunc = name;
          }
       }
    }

    if(topKernelCnt == 0) *TopKernelFunc = emptyKernel;
    return topKernelCnt;
}


gceSTATUS
cloCOMPILER_AllocateVariableMemory(
cloCOMPILER Compiler,
clsNAME *Variable
)
{
   gceSTATUS status = gcvSTATUS_OK;
   gctSIZE_T memoryReqd;
   gctSIZE_T memoryOffset = 0;
   cltQUALIFIER  addrSpaceQualifier;

   gcmASSERT(Variable);

   if(Variable->type == clvPARAMETER_NAME) return gcvSTATUS_OK;
   if(Variable->u.variableInfo.allocated) return gcvSTATUS_OK;

   memoryReqd = clsDECL_GetByteSize(Compiler, &Variable->decl);
   addrSpaceQualifier = Variable->decl.dataType->addrSpaceQualifier;
   if(Variable->decl.dataType->accessQualifier == clvQUALIFIER_CONST)
       addrSpaceQualifier = clvQUALIFIER_CONSTANT;
   switch(addrSpaceQualifier) {
   case clvQUALIFIER_GLOBAL:
      memoryOffset = clAlignMemory(Compiler,
                                   Variable,
                                   Compiler->context.privateMemorySize);
      Compiler->context.privateMemorySize = memoryOffset + memoryReqd;
      Compiler->context.needPrivateMemory = gcvTRUE;
      break;

   case clvQUALIFIER_LOCAL:
      {
         cloIR_SET funcBody;
         funcBody = Compiler->codeGenerator->currentFuncDefContext.funcBody;
         gcmASSERT(funcBody);
         memoryOffset = clAlignMemory(Compiler,
                                      Variable,
                                      funcBody->funcName->u.funcInfo.localMemorySize);
         funcBody->funcName->u.funcInfo.localMemorySize = memoryOffset + memoryReqd;
         Compiler->context.needLocalMemory = gcvTRUE;
      }
      break;

   case clvQUALIFIER_NONE:
   case clvQUALIFIER_PRIVATE:
      memoryOffset = clAlignMemory(Compiler,
                                   Variable,
                                   Compiler->context.privateMemorySize);
      Compiler->context.privateMemorySize = memoryOffset + memoryReqd;
      Compiler->context.needPrivateMemory = gcvTRUE;
      break;

   case clvQUALIFIER_CONSTANT:
      memoryOffset = clAlignMemory(Compiler,
                                   Variable,
                                   Compiler->context.constantMemorySize);
      Compiler->context.constantMemorySize = memoryOffset + memoryReqd;
      Compiler->context.needConstantMemory = gcvTRUE;
      status = cloCOMPILER_AddConstantVariable(Compiler,
                                               Variable);
      if (gcmIS_ERROR(status)) return status;
      break;

   default:
      gcmASSERT(0);
   }
   clmNAME_VariableMemoryOffset_SET(Variable, (gctINT)memoryOffset);
   Variable->u.variableInfo.allocated = gcvTRUE;
   return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_SetNeedConstantMemory(
cloCOMPILER Compiler
)
{
   Compiler->context.needConstantMemory = gcvTRUE;
   return gcvSTATUS_OK;
}

gctBOOL
cloCOMPILER_IsConstantMemoryNeeded(
cloCOMPILER Compiler
)
{
   return Compiler->context.constantMemorySize &&
          Compiler->context.needConstantMemory;
}


gctSIZE_T
cloCOMPILER_GetConstantMemoryNeeded(
cloCOMPILER Compiler
)
{
   return Compiler->context.constantMemorySize;
}


gceSTATUS
cloCOMPILER_SetNeedPrivateMemory(
cloCOMPILER Compiler
)
{
   Compiler->context.needPrivateMemory = gcvTRUE;
   return gcvSTATUS_OK;
}

gctBOOL
cloCOMPILER_IsPrivateMemoryNeeded(
cloCOMPILER Compiler
)
{
   return Compiler->context.privateMemorySize &&
          Compiler->context.needPrivateMemory;
}

gctSIZE_T
cloCOMPILER_GetPrivateMemoryNeeded(
cloCOMPILER Compiler
)
{
   return Compiler->context.privateMemorySize;
}

gceSTATUS
cloCOMPILER_SetHasImageQuery(
cloCOMPILER Compiler
)
{
   Compiler->context.hasImageQuery = gcvTRUE;
   return gcvSTATUS_OK;
}

gceSTATUS
cloCOMPILER_SetNeedPrintfMemory(
cloCOMPILER Compiler
)
{
   Compiler->context.needPrintfMemory = gcvTRUE;
   return gcvSTATUS_OK;
}

gctBOOL
cloCOMPILER_IsPrintfMemoryNeeded(
cloCOMPILER Compiler
)
{
   return Compiler->context.needPrintfMemory;
}

gceSTATUS
cloCOMPILER_SetNeedLocalMemory(
cloCOMPILER Compiler
)
{
   Compiler->context.needLocalMemory = gcvTRUE;
   return gcvSTATUS_OK;
}

gctBOOL
cloCOMPILER_IsLocalMemoryNeeded(
cloCOMPILER Compiler
)
{
   return Compiler->context.needLocalMemory;
}

gceSTATUS
cloCOMPILER_SetHasLocalMemoryKernelArg(
cloCOMPILER Compiler
)
{
   Compiler->context.hasLocalMemoryKernelArg = gcvTRUE;
   return gcvSTATUS_OK;
}

gctBOOL
cloCOMPILER_HasLocalMemoryKernelArg(
cloCOMPILER Compiler
)
{
   return Compiler->context.hasLocalMemoryKernelArg;
}

gceSTATUS
cloCOMPILER_SetMaxKernelFunctionArgs(
cloCOMPILER Compiler,
gctUINT numArgs
)
{
   if(numArgs > Compiler->context.maxKernelFunctionArgs) {
      Compiler->context.maxKernelFunctionArgs = numArgs;
   }
   return gcvSTATUS_OK;
}

gctBOOL
cloCOMPILER_IsExternSymbolsAllowed(
cloCOMPILER Compiler
)
{
   return Compiler->context.allowExternSymbols ||
          cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX);
}

gceSTATUS
cloCOMPILER_RegisterBuiltinVariable(
IN cloCOMPILER Compiler,
IN gctINT VariableNum,
IN clsNAME *Unnamed
)
{
   gcmASSERT(VariableNum < cldNumBuiltinVariables);
   Compiler->context.unnamedVariables[VariableNum] = Unnamed;
   return gcvSTATUS_OK;
}

clsNAME *
cloCOMPILER_GetBuiltinVariable(
IN cloCOMPILER Compiler,
IN gctINT VariableNum
)
{
   gcmASSERT(VariableNum < cldNumBuiltinVariables);
   return Compiler->context.unnamedVariables[VariableNum];
}

cloIR_VARIABLE
cloCOMPILER_GetParamChainVariable(
IN cloCOMPILER Compiler,
IN gctINT LineNo,
IN gctINT StringNo,
IN gctINT VariableNum
)
{
   gcmASSERT(VariableNum < cldMaxParamChains);
   if(Compiler->context.paramChainVariables[VariableNum] == gcvNULL) {
       gceSTATUS status;
       clsNAME *unnamedVariable;
       clsNAME_SPACE *nameSpace;
       cltPOOL_STRING symbolInPool;
       gctUINT offset = 0;
       gctCHAR nameBuf[256];
       clsDECL decl[1];

       status = cloCOMPILER_CreateDecl(Compiler,
                                       T_INT,
                                       gcvNULL,
                                       clvQUALIFIER_NONE,
                                       clvQUALIFIER_NONE,
                                       decl);
       if (gcmIS_ERROR(status)) return gcvNULL;

       status = cloCOMPILER_PushUnnamedSpace(Compiler, &nameSpace);
       if (gcmIS_ERROR(status)) return gcvNULL;

       gcoOS_PrintStrSafe(nameBuf, 256, &offset, "#__%VARIABLE_%d", VariableNum);
       cloCOMPILER_AllocatePoolString(Compiler,
                                      nameBuf,
                                      &symbolInPool);

       gcmONERROR(cloCOMPILER_CreateName(Compiler,
                                         LineNo,
                                         StringNo,
                                         clvVARIABLE_NAME,
                                         decl,
                                         symbolInPool,
                                         gcvNULL,
                                         clvEXTENSION_NONE,
                                         &unnamedVariable));

       gcmONERROR(cloIR_VARIABLE_Construct(Compiler,
                                           LineNo,
                                           StringNo,
                                           unnamedVariable,
                                           &Compiler->context.paramChainVariables[VariableNum]));
   OnError:
       cloCOMPILER_PopCurrentNameSpace(Compiler, &nameSpace);
   }

   return Compiler->context.paramChainVariables[VariableNum];
}

clsNAME_SPACE *
cloCOMPILER_GetBuiltinSpace(
IN cloCOMPILER Compiler
)
{
   return Compiler->context.builtinSpace;
}

clsNAME_SPACE *
cloCOMPILER_GetGeneralBuiltinSpace(
IN cloCOMPILER Compiler
)
{
   return Compiler->context.generalBuiltinSpace;
}

clsNAME_SPACE *
cloCOMPILER_GetCurrentSpace(
IN cloCOMPILER Compiler
)
{
   return Compiler->context.currentSpace;
}

void
cloCOMPILER_SetCurrentSpace(
IN cloCOMPILER Compiler,
IN clsNAME_SPACE *NameSpace
)
{
    Compiler->context.currentSpace = NameSpace;
}

clsNAME_SPACE *
cloCOMPILER_GetGlobalSpace(
IN cloCOMPILER Compiler
)
{
   return Compiler->context.globalSpace;
}

clsNAME_SPACE *
cloCOMPILER_GetUnnamedSpace(
IN cloCOMPILER Compiler
)
{
   return Compiler->context.unnamedSpace;
}

gctBOOL
cloCOMPILER_GenDebugInfo(
IN cloCOMPILER Compiler
)
{
    return (Compiler->context.debugInfo != gcvNULL);
}

void
cloCOMPILER_ChangeUniformDebugInfo(
    IN cloCOMPILER Compiler,
    gctUINT tmpStart,
    gctUINT tmpEnd,
    gctUINT uniformIdx
)
{
    if (Compiler->context.debugInfo != gcvNULL)
    {
        vscDIChangeUniformSWLoc(Compiler->context.debugInfo,
                                tmpStart,
                                tmpEnd,
                                uniformIdx);
    }
}
gctUINT16
cloCOMPILER_AddDIE(
IN cloCOMPILER Compiler,
IN VSC_DIE_TAG Tag,
IN gctUINT16 Parent,
IN gctCONST_STRING Name,
IN gctUINT FileNo,
IN gctUINT LineNo,
IN gctUINT EndLineNo,
IN gctUINT ColNo
)
{
    gctUINT16 die;

    if (Compiler->context.debugInfo && !Compiler->context.debugInfo->collect)
        return VSC_DI_INVALIDE_DIE;

    /* we need generate type for header file */
    if (!Compiler->context.mainFile && (Tag == !VSC_DI_TAG_TYPE))
        return VSC_DI_INVALIDE_DIE;

    if (!Compiler->context.mainFile && (Parent == VSC_DI_INVALIDE_DIE))
        return VSC_DI_INVALIDE_DIE;

    if (Parent == VSC_DI_SPACE_SKIP_DIE)
        return VSC_DI_INVALIDE_DIE;

    FileNo = Compiler->context.mainFile ? 0 : 1;
    die = vscDIAddDIE(Compiler->context.debugInfo, Tag, Parent, Name, FileNo, LineNo, EndLineNo, ColNo);

    if (gcmOPT_EnableDebugDumpALL())
        vscDIDumpDIE(Compiler->context.debugInfo,die, 0, 0xffffffff);

    return die;
}

void
cloCOMPILER_DumpDIETree(
IN cloCOMPILER Compiler
)
{
    if (gcmOPT_EnableDebugDumpALL() && Compiler->context.debugInfo)
        vscDIDumpDIETree(Compiler->context.debugInfo, Compiler->context.debugInfo->cu, 0xffffffff);
}

void
cloCOMPILER_SetDIESourceLoc(
IN cloCOMPILER Compiler,
IN gctUINT16 Id,
IN gctUINT FileNo,
IN gctUINT LineNo,
IN gctUINT EndLineNo,
IN gctUINT ColNo
)
{
    VSC_DIE * die;

    if (Compiler->context.debugInfo == gcvNULL) return;

    die = vscDIGetDIE(Compiler->context.debugInfo, Id);

    if (die != gcvNULL)
    {
        if (!Compiler->context.mainFile)
            FileNo = 1;

        die->fileNo = (gctUINT8)FileNo;
        die->lineNo = (gctUINT16)LineNo;
        die->endLineNo = (gctUINT16) EndLineNo;
        die->colNo = (gctUINT8)ColNo;
    }

    return;
}

void
cloCOMPILER_SetDIELogicalReg(
IN cloCOMPILER Compiler,
IN gctUINT16 Id,
IN gctUINT32 regIndex,
IN gctUINT num,
IN gctUINT mask
)
{
    VSC_DIE * die;

    if (Compiler->context.debugInfo == gcvNULL)  return;

    die = vscDIGetDIE(Compiler->context.debugInfo, Id);

    if (die &&
        ((die->tag == VSC_DI_TAG_VARIABE) ||
        (die->tag == VSC_DI_TAG_PARAMETER))
        )
    {
        VSC_DI_SW_LOC * swLoc, * prevSwLoc;
        gctUINT16 loc = VSC_DI_INVALID_SW_LOC;

        loc = vscDIAddSWLoc(Compiler->context.debugInfo);

        swLoc = vscDIGetSWLoc(Compiler->context.debugInfo, loc);

        if (swLoc == gcvNULL)
        {
            gcmPRINT("%s, invalid swLoc = %d!!!!!!!", __FUNCTION__,die->u.variable.swLoc);
            return;
        }

        swLoc->reg = gcvTRUE;
        swLoc->u.reg.type = VSC_DIE_REG_TYPE_TMP;
        swLoc->u.reg.start = (gctUINT16)regIndex;
        swLoc->u.reg.end = (gctUINT16)(regIndex + num -1);
        swLoc->u.reg.mask = (gctUINT16)mask;

        if (die->u.variable.swLoc == VSC_DI_INVALID_SW_LOC)
        {
            die->u.variable.swLoc = loc;
        }
        else
        {
            prevSwLoc = vscDIGetSWLoc(Compiler->context.debugInfo, die->u.variable.swLoc);

            while (prevSwLoc->next != VSC_DI_INVALID_SW_LOC)
            {
                prevSwLoc = vscDIGetSWLoc(Compiler->context.debugInfo, prevSwLoc->next);
            }
            prevSwLoc->next = loc;
        }

        if (gcmOPT_EnableDebugDumpALL())
        {
            gcmPRINT("set swLoc[%d] reg[%d,%d]", swLoc->id, regIndex, regIndex + num -1);
            vscDIDumpDIE(Compiler->context.debugInfo, die->id, 0, (1 << VSC_DI_TAG_VARIABE)|(1 << VSC_DI_TAG_PARAMETER));
        }
    }
}

void
cloCOMPILER_SetStructDIELogicalReg(
IN cloCOMPILER Compiler,
IN clsNAME * Variable,
IN gctUINT16 ParentId,
IN gctCONST_STRING Symbol,
IN gctUINT32 regIndex,
IN gctUINT num,
IN gctUINT mask
)
{
    VSC_DIE * die;
    gctINT i;

    if (Compiler->context.debugInfo == gcvNULL)  return;

    die = vscDIGetDIE(Compiler->context.debugInfo, vscDIAddDIE(Compiler->context.debugInfo, VSC_DI_TAG_VARIABE, ParentId, Symbol, 0, 0, 0, 0));

    cloCOMPILER_SetDIELogicalReg(Compiler, die->id, regIndex, num, mask);

    /* set the member of structure to primitive */
    if (die->tag == VSC_DI_TAG_VARIABE)
    {
        die->u.variable.type.isPrimitiveType = gcvTRUE;

        die->u.variable.type.type = Variable->decl.dataType->virPrimitiveType;
        die->u.variable.type.array.numDim = Variable->decl.array.numDim;

        for (i = 0 ; i < die->u.variable.type.array.numDim; i++)
        {
            die->u.variable.type.array.length[i] = Variable->decl.array.length[i];
        }
        if (clmDECL_IsPointerType(&(Variable->decl)))
            die->u.variable.type.isPointer = gcvTRUE;
    }
}

gctUINT16
cloCOMPILER_GetDIEType(
IN cloCOMPILER Compiler,
IN clsDATA_TYPE *DataType
)
{
    return vscDIGetDIEType(Compiler->context.debugInfo);
}

gctUINT16
_GetStructUnionType(
IN cloCOMPILER Compiler,
IN clsNAME * Variable
)
{
    struct _clsDECL * decl = &Variable->decl;
    clsNAME_SPACE * nameSpace;

    nameSpace = (clsNAME_SPACE *)(decl->dataType->u.generic);
    return nameSpace->die;
}

gctUINT16
cloCOMPILER_AddDIEWithName(
IN cloCOMPILER Compiler,
IN clsNAME * Variable
)
{
    gctUINT16 parent;
    gctUINT16  die = VSC_DI_INVALIDE_DIE;
    struct _clsDECL * decl;
    VSC_DIE_TAG tag = VSC_DI_TAG_INVALID;
    gctBOOL gen = gcvTRUE;
    struct _VSC_DI_TYPE type = {0};
    gctINT i;

    if ((Compiler->context.debugInfo == gcvNULL) ||
         !Compiler->context.debugInfo->collect)
    {
        return die;
    }

    if (Variable != gcvNULL)
    {
        if (Compiler->context.loadingBuiltins &&
            (Variable->type == clvFUNC_NAME ||
             Variable->type == clvPARAMETER_NAME)
            )
        {
            gen = gcvFALSE;
        }

        if (!Compiler->context.mainFile &&
            (Variable->type == clvFUNC_NAME ||
             Variable->type == clvPARAMETER_NAME)
           )
        {
            gen = gcvFALSE;
        }

        if (!Compiler->context.mainFile &&
            Variable->mySpace->die == VSC_DI_INVALIDE_DIE)
        {
            gen = gcvFALSE;
        }

        if (Variable->mySpace->die == VSC_DI_SPACE_SKIP_DIE)
        {
            gen = gcvFALSE;
        }
    }
    else
    {
        gen = gcvFALSE;
    }

    if (gen)
    {
        parent = Variable->mySpace->die;
        type.isPrimitiveType = gcvTRUE;
        type.type = VIR_TYPE_UNKNOWN;

        if (parent != VSC_DI_INVALIDE_DIE)
        {
            switch (Variable->type)
            {
            case clvVARIABLE_NAME:
            case clvPARAMETER_NAME:
                decl = &(Variable->decl);

                /* struct and enum will not fall into here, so, we don't need set tag = TYPE for these two */
                if (decl->dataType->accessQualifier == clvQUALIFIER_CONST
                    && parent == 0) /* const variables in the program scope */
                {
                    tag = VSC_DI_TAG_CONSTANT;
                }
                else if(cloCOMPILER_GetParserState(Compiler) == clvPARSER_IN_TYPEDEF)
                {
                    tag = VSC_DI_TAG_TYPE;
                }
                else if (Variable->type == clvPARAMETER_NAME)
                {
                    tag = VSC_DI_TAG_PARAMETER;
                }
                else
                {
                    tag = VSC_DI_TAG_VARIABE;
                }

                if (decl->dataType->type == T_STRUCT ||
                    decl->dataType->type == T_UNION)
                {
                    type.isPrimitiveType = gcvFALSE;
                    type.type = (gctINT)_GetStructUnionType(Compiler,Variable);
                }
                else
                {
                    /* As for primitive type, I don't add DIE for every one, so,
                    for array, that don't have die, we record full type info.
                    Struct/Union or some other compound type, they must have die type!!!!*/
                    type.isPrimitiveType = gcvTRUE;
                    type.type = Variable->decl.dataType->virPrimitiveType;
                    type.array.numDim = Variable->decl.array.numDim;

                    for (i = 0 ; i < type.array.numDim; i++)
                    {
                        type.array.length[i] = Variable->decl.array.length[i];
                    }
                }
                break;

            case clvFUNC_NAME:
            case clvKERNEL_FUNC_NAME:
                tag = VSC_DI_TAG_SUBPROGRAM;
                type.isPrimitiveType = gcvTRUE;
                type.type = Variable->decl.dataType->virPrimitiveType;
                break;

            case clvENUM_NAME:
                tag = VSC_DI_TAG_INVALID;
                break;

            case clvENUM_TAG_NAME:
            case clvSTRUCT_NAME:
            case clvUNION_NAME:
                tag = VSC_DI_TAG_INVALID;
                break;

            case clvFIELD_NAME:
                tag = VSC_DI_TAG_TYPE;
                break;

            case clvTYPE_NAME:
            default:
                gcmASSERT(0);
            }
        }
        else
        {
            gcmASSERT(0);
        }

        if (tag != VSC_DI_TAG_INVALID &&
            tag != VSC_DI_TAG_CONSTANT)
        {
            VSC_DIE * ptr;

            die = vscDIAddDIE(Compiler->context.debugInfo,
                              tag,
                              parent,
                              Variable->symbol,
                              (Compiler->context.mainFile ? 0 : 1),
                              Variable->lineNo,
                              Variable->lineNo,
                              Variable->stringNo
                              );

            ptr = vscDIGetDIE(Compiler->context.debugInfo, die);

            if (tag == VSC_DI_TAG_TYPE)
            {
                ptr->u.type = type;
            }
            else if (tag == VSC_DI_TAG_SUBPROGRAM)
            {
                ptr->u.func.retType = type;
            }
            else if (tag == VSC_DI_TAG_VARIABE ||
                     tag == VSC_DI_TAG_PARAMETER)
            {
                ptr->u.variable.type = type;
                if (clmDECL_IsPointerType(&(Variable->decl)))
                    ptr->u.variable.type.isPointer = gcvTRUE;
            }
            else if (tag == VSC_DI_TAG_CONSTANT)
            {
            }
            else
            {
                gcmASSERT(0);
            }
        }
    }

    if (gcmOPT_EnableDebugDumpALL())
        vscDIDumpDIE(Compiler->context.debugInfo,die, 0, 0xffffffff);

    return die;
}

void
cloCOMPILER_SetCollectDIE(
IN cloCOMPILER Compiler,
gctBOOL collect
)
{
    if (Compiler->context.debugInfo)
    {
        Compiler->context.debugInfo->collect = collect;
    }
}

void
cloCOMPILER_SetDIEType(
IN  cloCOMPILER Compiler,
IN  clsDECL     *Decl,
IN  gctUINT16   Id
)
{
    VSC_DIE     *Die;
    struct _VSC_DI_TYPE type = {0};
    if(Compiler->context.debugInfo)
    {
        type.type = Decl->dataType->u.fieldSpace->die;
        type.isPrimitiveType = gcvFALSE;
        Die = vscDIGetDIE(Compiler->context.debugInfo, Id);
        Die->u.type = type;
        Die->u.variable.type = type;
    }
}

gctBOOL
cloCOMPILER_InGlobalSpace(
IN cloCOMPILER Compiler
)
{
   return Compiler->context.currentSpace == Compiler->context.globalSpace;
}

gctBOOL
cloCOMPILER_IsNameSpaceGlobal(
IN cloCOMPILER Compiler,
clsNAME_SPACE *  NameSpace
)
{
   return NameSpace == Compiler->context.globalSpace;
}

gctBOOL
cloCOMPILER_IsNameSpaceUnnamed(
IN cloCOMPILER Compiler,
clsNAME_SPACE *  NameSpace
)
{
   return NameSpace == Compiler->context.unnamedSpace;
}

gctBOOL
cloCOMPILER_IsDumpOn(
IN cloCOMPILER Compiler,
IN cleDUMP_OPTION DumpOption
)
{
    if (Compiler->context.dumpOptions & DumpOption)
        return gcvTRUE;
    return gcvFALSE;
}
