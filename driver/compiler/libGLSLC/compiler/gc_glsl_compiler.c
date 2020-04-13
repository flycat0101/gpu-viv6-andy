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


#include "gc_glsl_compiler.h"
#include "gc_hal_user.h"
#include "gc_glsl_preprocessor.h"
#include "gc_glsl_parser.h"
#include "gc_glsl_ast_walk.h"
#include "gc_glsl_emit_code.h"

/*******************************Redeclared built-in variables*******************************/
typedef gctBOOL
(*sltREDECLARED_CHECK_FUNC_PTR)(
    IN sloCOMPILER,
    IN slsNAME*,
    IN slsDATA_TYPE*
    );

typedef gceSTATUS
(*sltREDECLARED_UPDATE_FUNC_PTR)(
    IN sloCOMPILER,
    IN slsNAME*,
    IN slsDATA_TYPE*
    );

/*******************************Redeclared built-in functions*******************************/
gctBOOL
_CheckRedeclaredForInterpolation(
    IN sloCOMPILER Compiler,
    IN slsNAME* Name,
    IN slsDATA_TYPE* NewDataType
    )
{
    /* Everything in data type must be the same expect for the ineterpolation qualifier. */
    if (!slsDATA_TYPE_IsEqual(Name->dataType, NewDataType))
    {
        return gcvFALSE;
    }

    if (Name->dataType->qualifiers.auxiliary != NewDataType->qualifiers.auxiliary
        ||
        Name->dataType->qualifiers.precision != NewDataType->qualifiers.precision
        ||
        Name->dataType->qualifiers.storage != NewDataType->qualifiers.storage
        ||
        Name->dataType->qualifiers.memoryAccess != NewDataType->qualifiers.memoryAccess
        ||
        Name->dataType->qualifiers.flags != NewDataType->qualifiers.flags)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

gceSTATUS
_UpdateRedeclaredForInterpolation(
    IN sloCOMPILER Compiler,
    IN slsNAME* Name,
    IN slsDATA_TYPE* NewDataType
    )
{
    Name->dataType->qualifiers.interpolation = NewDataType->qualifiers.interpolation;
    return gcvSTATUS_OK;
}

gctBOOL
_CheckRedeclaredForClipDistance(
    IN sloCOMPILER Compiler,
    IN slsNAME* Name,
    IN slsDATA_TYPE* NewDataType
    )
{
    slsDATA_TYPE* DataType1 = Name->dataType;

    /* Everything in data type must be the same expect for the array size. */
    if (!((DataType1->elementType == NewDataType->elementType)
         &&
         (slmDATA_TYPE_vectorSize_NOCHECK_GET(DataType1) == slmDATA_TYPE_vectorSize_NOCHECK_GET(NewDataType))
         &&
         (slmDATA_TYPE_matrixRowCount_GET(DataType1) == slmDATA_TYPE_matrixRowCount_GET(NewDataType))
         &&
         (slmDATA_TYPE_matrixColumnCount_GET(DataType1) == slmDATA_TYPE_matrixColumnCount_GET(NewDataType))
         &&
         (DataType1->arrayLengthCount == NewDataType->arrayLengthCount)
         &&
         (DataType1->orgFieldSpace == NewDataType->orgFieldSpace)))
    {
        return gcvFALSE;
    }

    if (DataType1->qualifiers.interpolation != NewDataType->qualifiers.interpolation
        ||
        DataType1->qualifiers.auxiliary != NewDataType->qualifiers.auxiliary
        ||
        DataType1->qualifiers.precision != NewDataType->qualifiers.precision
        ||
        DataType1->qualifiers.storage != NewDataType->qualifiers.storage
        ||
        DataType1->qualifiers.memoryAccess != NewDataType->qualifiers.memoryAccess
        ||
        DataType1->qualifiers.flags != NewDataType->qualifiers.flags)
    {
        return gcvFALSE;
    }

    /* The array length can be larger than the MaxClipDistance. */
    if (NewDataType->arrayLength > (gctINT)GetGLMaxClipDistances())
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

gceSTATUS
_UpdateRedeclaredForClipDistance(
    IN sloCOMPILER Compiler,
    IN slsNAME* Name,
    IN slsDATA_TYPE* NewDataType
    )
{
    Name->dataType->arrayLength = NewDataType->arrayLength;
    Name->dataType->arrayLengthList[0] = NewDataType->arrayLengthList[0];
    return gcvSTATUS_OK;
}

/* Reclared built-in Variables */
typedef struct _slsREDECLARED_VARIABLE
{
    sloEXTENSION                    extension;
    gctSTRING                       variableName;
    sltREDECLARED_CHECK_FUNC_PTR    checkFunc;
    sltREDECLARED_UPDATE_FUNC_PTR   updateFunc;
} slsREDECLARED_VARIABLE;

static slsREDECLARED_VARIABLE VSRedeclaredVariables[] =
{
    { {slvEXTENSION1_SUPPORT_OGL, slvEXTENSION2_NONE },         "gl_FrontColor",            _CheckRedeclaredForInterpolation,        _UpdateRedeclaredForInterpolation },
    { {slvEXTENSION1_SUPPORT_OGL, slvEXTENSION2_NONE },         "gl_BackColor",             _CheckRedeclaredForInterpolation,        _UpdateRedeclaredForInterpolation },
    { {slvEXTENSION1_SUPPORT_OGL, slvEXTENSION2_NONE },         "gl_FrontSecondaryColor",   _CheckRedeclaredForInterpolation,        _UpdateRedeclaredForInterpolation },
    { {slvEXTENSION1_SUPPORT_OGL, slvEXTENSION2_NONE },         "gl_BackSecondaryColor",    _CheckRedeclaredForInterpolation,        _UpdateRedeclaredForInterpolation },
    { {slvEXTENSION1_SUPPORT_OGL, slvEXTENSION2_NONE },         "gl_ClipDistance",          _CheckRedeclaredForClipDistance,         _UpdateRedeclaredForClipDistance },
};
static gctUINT VSRedeclaredVariableCount = sizeof(VSRedeclaredVariables) / sizeof(slsREDECLARED_VARIABLE);

static slsREDECLARED_VARIABLE GSRedeclaredVariables[] =
{
    { {slvEXTENSION1_SUPPORT_OGL, slvEXTENSION2_NONE },         "gl_FrontColor",            _CheckRedeclaredForInterpolation,        _UpdateRedeclaredForInterpolation },
    { {slvEXTENSION1_SUPPORT_OGL, slvEXTENSION2_NONE },         "gl_BackColor",             _CheckRedeclaredForInterpolation,        _UpdateRedeclaredForInterpolation },
    { {slvEXTENSION1_SUPPORT_OGL, slvEXTENSION2_NONE },         "gl_FrontSecondaryColor",   _CheckRedeclaredForInterpolation,        _UpdateRedeclaredForInterpolation },
    { {slvEXTENSION1_SUPPORT_OGL, slvEXTENSION2_NONE },         "gl_BackSecondaryColor",    _CheckRedeclaredForInterpolation,        _UpdateRedeclaredForInterpolation },
};
static gctUINT GSRedeclaredVariableCount = sizeof(GSRedeclaredVariables) / sizeof(slsREDECLARED_VARIABLE);

static slsREDECLARED_VARIABLE FSRedeclaredVariables[] =
{
    { {slvEXTENSION1_SUPPORT_OGL, slvEXTENSION2_NONE },         "gl_Color",                 _CheckRedeclaredForInterpolation,        _UpdateRedeclaredForInterpolation },
    { {slvEXTENSION1_SUPPORT_OGL, slvEXTENSION2_NONE },         "gl_SecondaryColor",        _CheckRedeclaredForInterpolation,        _UpdateRedeclaredForInterpolation },
    { {slvEXTENSION1_SUPPORT_OGL, slvEXTENSION2_NONE },         "gl_ClipDistance",          _CheckRedeclaredForClipDistance,         _UpdateRedeclaredForClipDistance },
};
static gctUINT FSRedeclaredVariableCount = sizeof(FSRedeclaredVariables) / sizeof(slsREDECLARED_VARIABLE);

sloCOMPILER gcCompiler[__GC_COMPILER_NUMBER__] = {gcvNULL, gcvNULL, gcvNULL, gcvNULL, gcvNULL, gcvNULL, gcvNULL};

sloCOMPILER *
gcGetCompiler(
    gctUINT Index
    )
{
    return &gcCompiler[Index];
}

static gctUINT32 _slCompilerVersion[2] = { _SHADER_GL_LANGUAGE_TYPE,
                                           _SHADER_ES11_VERSION };

typedef struct _slsPOOL_STRING_NODE
{
    slsDLINK_NODE       node;
    gctUINT             crc32Value;
    sltPOOL_STRING      string;
}
slsPOOL_STRING_NODE;

extern gctINT
_convertShaderType(
    IN gctINT ShaderType
    );

gceSTATUS
sloCOMPILER_Construct_General(
    IN sleSHADER_TYPE ShaderType,
    IN gceAPI ClientApiVersion,
    OUT sloCOMPILER * Compiler
    )
{
    gceSTATUS status;
    sloCOMPILER compiler = gcvNULL;
    gctUINT i;
    sltPOOL_STRING str = gcvNULL;

    gcmHEADER_ARG("Hal=0x%x ShaderType=%d", Hal, ShaderType);

    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(Compiler);

    do
    {
        gctPOINTER pointer = gcvNULL;

        /* Allocate memory for sloCOMPILER object */
        status = gcoOS_Allocate(gcvNULL,
                                sizeof(struct _sloCOMPILER),
                                &pointer
                                );

        if (gcmIS_ERROR(status))
        {
            compiler = gcvNULL;
            break;
        }

        compiler = pointer;

        gcoOS_ZeroMemory(compiler, sizeof(struct _sloCOMPILER));

        /* Initialize members */
        compiler->object.type               = slvOBJ_COMPILER;
        compiler->shaderType                = (sleSHADER_TYPE)_convertShaderType(ShaderType);
        compiler->clientApiVersion          = ClientApiVersion;

#if __USE_VSC_MP__
        vscPMP_Intialize(&compiler->generalPMP, gcvNULL, __VSC_GENERAL_MP_SIZE__, sizeof(void *), __ENABLE_VSC_MP_POOL__);
        compiler->currentPMP = &compiler->generalPMP;
#else
        slsDLINK_LIST_Initialize(&compiler->generalMemoryPool);
#endif
        compiler->context.loadingGeneralBuiltIns = gcvTRUE;

        slsHASH_TABLE_Initialize(&compiler->context.generalStringPool);
        slsSLINK_LIST_Initialize(&compiler->context.switchScope);
        slsSLINK_LIST_Initialize(&compiler->context.layoutOffset);
        slsSLINK_LIST_Initialize(&compiler->context.sharedVariables);
        slsDLINK_LIST_Initialize(&compiler->context.dataTypeNameList);
        slsDLINK_LIST_Initialize(&compiler->context.constantVariables);
        compiler->context.constantBufferSize = 0;

        for (i = 0; i < sldMAX_VECTOR_COMPONENT; i++)
        {
           slsDLINK_LIST_Initialize(&compiler->context.vecConstants.typeFloat[i]);
           slsDLINK_LIST_Initialize(&compiler->context.vecConstants.typeInt[i]);
           slsDLINK_LIST_Initialize(&compiler->context.vecConstants.typeUInt[i]);
           slsDLINK_LIST_Initialize(&compiler->context.vecConstants.typeBool[i]);
        }

        status = sloCOMPILER_AllocatePoolString(compiler,
                                                "General BuiltIn Space",
                                                &str);
        if (gcmIS_ERROR(status)) break;

        /* Create built-in name space */
        status = slsNAME_SPACE_Construct(compiler,
                                         str,
                                         gcvNULL,
                                         slvNAME_SPACE_TYPE_BUILTIN,
                                         &compiler->context.generalBuiltinSpace);
        if (gcmIS_ERROR(status)) break;
        compiler->context.currentSpace = compiler->context.generalBuiltinSpace;

        /* Load general builtins. */
        status = sloCOMPILER_LoadGeneralBuiltIns(compiler);
        if (gcmIS_ERROR(status)) break;

#if __USE_VSC_MP__
        compiler->currentPMP = gcvNULL;
#endif

        *Compiler = compiler;

        gcmFOOTER_ARG("*Compiler=0x%x", *Compiler);
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    if (compiler != gcvNULL) sloCOMPILER_Destroy_General(compiler);

    *Compiler = gcvNULL;

    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_Construct(
    IN sleSHADER_TYPE ShaderType,
    IN gceAPI ClientApiVersion,
    INOUT sloCOMPILER Compiler
    )
{
    gceSTATUS status;
    sloCOMPILER compiler = Compiler;
    gctUINT i;
    sltPOOL_STRING str = gcvNULL;

    gcmHEADER_ARG("Hal=0x%x ShaderType=%d", Hal, ShaderType);

    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(Compiler);

    do
    {
        sleSHADER_TYPE          shaderType;
#if __USE_VSC_MP__
        VSC_PRIMARY_MEM_POOL    generalPMP;
#else
        slsDLINK_LIST           generalMemoryPool;
#endif
        slsHASH_TABLE           generalStringPool;
        slsNAME_SPACE *         generalBuiltinSpace;

        /* Copy the general parts into the temp variables. */
        gcoOS_MemCopy(&shaderType, &compiler->shaderType, gcmSIZEOF(shaderType));
#if __USE_VSC_MP__
        gcoOS_MemCopy(&generalPMP, &compiler->generalPMP, gcmSIZEOF(VSC_PRIMARY_MEM_POOL));
#else
        gcoOS_MemCopy(&generalMemoryPool, &compiler->generalMemoryPool, gcmSIZEOF(slsDLINK_LIST));
#endif
        gcoOS_MemCopy(&generalStringPool, &compiler->context.generalStringPool, gcmSIZEOF(slsHASH_TABLE));
        generalBuiltinSpace = compiler->context.generalBuiltinSpace;

        /* Zero compiler. */
        gcoOS_ZeroMemory(compiler, sizeof(struct _sloCOMPILER));

        /* Copy back the general parts. */
        compiler->shaderType = shaderType;
#if __USE_VSC_MP__
        gcoOS_MemCopy(&compiler->generalPMP, &generalPMP, gcmSIZEOF(VSC_PRIMARY_MEM_POOL));
#else
        gcoOS_MemCopy(&compiler->generalMemoryPool, &generalMemoryPool, gcmSIZEOF(slsDLINK_LIST));
#endif
        gcoOS_MemCopy(&compiler->context.generalStringPool, &generalStringPool, gcmSIZEOF(slsHASH_TABLE));
        compiler->context.generalBuiltinSpace = generalBuiltinSpace;

        /* Initialize members */
        compiler->object.type               = slvOBJ_COMPILER;
        compiler->patchId                   = *GetPatchID();
        compiler->langVersion               = _SHADER_ES11_VERSION;

        gcmASSERT(compiler->shaderType == (sleSHADER_TYPE)_convertShaderType(ShaderType));

        if (ShaderType == slvSHADER_TYPE_VERTEX_DEFAULT_UBO ||
            ShaderType == slvSHADER_TYPE_FRAGMENT_DEFAULT_UBO)
        {
            compiler->createDefaultUBO      = gcvTRUE;
        }
        else
        {
            compiler->createDefaultUBO      = gcvFALSE;
        }

        compiler->clientApiVersion          = ClientApiVersion;

#if __USE_VSC_MP__
        vscPMP_Intialize(&compiler->privatePMP, gcvNULL, __VSC_PRIVATE_SIZE__, sizeof(void *), __ENABLE_VSC_MP_POOL__);
        compiler->currentPMP = &compiler->privatePMP;
#else
        slsDLINK_LIST_Initialize(&compiler->privateMemoryPool);
#endif

        /* Init context. */
        slsHASH_TABLE_Initialize(&compiler->context.privateStringPool);
        slsSLINK_LIST_Initialize(&compiler->context.switchScope);
        slsSLINK_LIST_Initialize(&compiler->context.layoutOffset);
        slsSLINK_LIST_Initialize(&compiler->context.sharedVariables);
        slsDLINK_LIST_Initialize(&compiler->context.dataTypeNameList);
        slsDLINK_LIST_Initialize(&compiler->context.constantVariables);
        compiler->context.constantBufferSize = 0;

        for(i = 0; i < sldMAX_VECTOR_COMPONENT; i++)
        {
           slsDLINK_LIST_Initialize(&compiler->context.vecConstants.typeFloat[i]);
           slsDLINK_LIST_Initialize(&compiler->context.vecConstants.typeInt[i]);
           slsDLINK_LIST_Initialize(&compiler->context.vecConstants.typeUInt[i]);
           slsDLINK_LIST_Initialize(&compiler->context.vecConstants.typeBool[i]);
        }

        /* Create unnamed space */
        status = sloCOMPILER_AllocatePoolString(compiler,
                                                "Unnamed Space",
                                                &str);
        if (gcmIS_ERROR(status)) break;

        status = slsNAME_SPACE_Construct(compiler,
                                         str,
                                         gcvNULL,
                                         slvNAME_SPACE_TYPE_STATE_SET,
                                         &compiler->context.unnamedSpace);
        if (gcmIS_ERROR(status)) break;

        /* Create built-in name space */
        status = sloCOMPILER_AllocatePoolString(compiler,
                                                "BuiltIn Space",
                                                &str);
        if (gcmIS_ERROR(status)) break;

        status = slsNAME_SPACE_Construct(compiler,
                                         str,
                                         compiler->context.generalBuiltinSpace,
                                         slvNAME_SPACE_TYPE_BUILTIN,
                                         &compiler->context.builtinSpace);
        if (gcmIS_ERROR(status)) break;

        compiler->context.currentSpace = compiler->context.builtinSpace;

        /* Create global name space */
        status = sloCOMPILER_AllocatePoolString(compiler,
                                                "Global Space",
                                                &str);
        if (gcmIS_ERROR(status)) break;

        status = slsNAME_SPACE_Construct(compiler,
                                         str,
                                         compiler->context.builtinSpace,
                                         slvNAME_SPACE_TYPE_DECL_SET,
                                         &compiler->context.globalSpace);
        if (gcmIS_ERROR(status)) break;

        compiler->context.auxGlobalSpace = gcvNULL;

        compiler->context.uniformDefaultLayout.id = slvLAYOUT_SHARED | slvLAYOUT_COLUMN_MAJOR;
        compiler->context.bufferDefaultLayout.id = slvLAYOUT_SHARED | slvLAYOUT_COLUMN_MAJOR;

        /* Init input layout. */
        compiler->context.inDefaultLayout.tesPrimitiveMode = slvTES_PRIMITIVE_MODE_NONE;
        compiler->context.inDefaultLayout.tesVertexSpacing = slvTES_VERTEX_SPACING_NONE;
        compiler->context.inDefaultLayout.tesOrdering = slvTES_ORDERING_NONE;
        compiler->context.inDefaultLayout.tesPointMode = slvTES_POINT_MODE_NONE;
        compiler->context.inDefaultLayout.gsPrimitive = slvGS_PRIMITIVE_NONE;
        compiler->context.inDefaultLayout.gsInvocationTime = -1;
        compiler->context.inDefaultLayout.maxVerticesNumber = (gctINT)GetGLMaxTessPatchVertices();
        gcoOS_MemFill(&compiler->context.applyInputLayout, 0, gcmSIZEOF(sloApplyLayout));
        compiler->context.applyInputLayout.storageQual = slvSTORAGE_QUALIFIER_IN;

        /* Init output layout. */
        compiler->context.outDefaultLayout.gsPrimitive = slvGS_PRIMITIVE_NONE;
        /* The initial default stream number is zero. */
        compiler->context.outDefaultLayout.streamNumber = 0;
        compiler->context.outDefaultLayout.maxVerticesNumber = (gctINT)GetGLMaxTessPatchVertices();
        compiler->context.outDefaultLayout.maxGSVerticesNumber = -1;
        compiler->context.outDefaultLayout.verticesNumber = -1;
        gcoOS_MemFill(&compiler->context.applyOutputLayout, 0, gcmSIZEOF(sloApplyLayout));
        compiler->context.applyOutputLayout.storageQual = slvSTORAGE_QUALIFIER_OUT;

        compiler->context.uniformLocationMaxLength = GetGLMaxUniformLocations();
        compiler->context.currentIterationCount = 1;

        if (vscDIConstructContext(gcvNULL,gcvNULL, &compiler->context.debugInfo) != gcvSTATUS_OK)
        {
            vscDIDestroyContext(compiler->context.debugInfo);
            compiler->context.debugInfo = gcvNULL;
        }

        /* Create IR root */
        status = sloIR_SET_Construct(compiler,
                                     1,
                                     0,
                                     slvDECL_SET,
                                     &compiler->context.rootSet);
        if (gcmIS_ERROR(status)) break;

        compiler->context.hasNotStagesRelatedLinkError = gcvSTATUS_OK;

#ifndef SL_SCAN_NO_PREPROCESSOR
        /* Create the preprocessor */
        status = sloPREPROCESSOR_Construct(
                                        compiler,
                                        &compiler->preprocessor);

        if (gcmIS_ERROR(status)) break;
#endif

        /* Create the code emitter */
        status = sloCODE_EMITTER_Construct(compiler,
                                           &compiler->codeEmitter);
        if (gcmIS_ERROR(status)) break;

        gcmFOOTER_ARG("*Compiler=0x%x", *Compiler);
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    if (compiler != gcvNULL) sloCOMPILER_Destroy(compiler);

    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_Destroy_General(
    IN sloCOMPILER Compiler
    )
{
    slsDLINK_LIST *         poolStringBucket;
    slsPOOL_STRING_NODE *   poolStringNode;
    gctINT i;

    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

#if __USE_VSC_MP__
    Compiler->currentPMP = &Compiler->generalPMP;
#endif

    if (Compiler->codeEmitter != gcvNULL)
    {
        gcmVERIFY_OK(sloCODE_EMITTER_Destroy(Compiler, Compiler->codeEmitter));
        Compiler->codeEmitter = gcvNULL;
    }

#ifndef SL_SCAN_NO_PREPROCESSOR
    if (Compiler->preprocessor != gcvNULL)
    {
        gcmVERIFY_OK(sloPREPROCESSOR_Destroy(Compiler, Compiler->preprocessor));
        Compiler->preprocessor = gcvNULL;
    }
#endif

    if (Compiler->binary != gcvNULL)
    {
        gcmVERIFY_OK(gcSHADER_Destroy(Compiler->binary));
        Compiler->binary = gcvNULL;
    }

    if (Compiler->log != gcvNULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Compiler->log));
        Compiler->log = gcvNULL;
    }

    /* Destory the whole IR tree */
    if (Compiler->context.rootSet != gcvNULL)
    {
        gcmVERIFY_OK(sloIR_OBJECT_Destroy(Compiler, &Compiler->context.rootSet->base));
        Compiler->context.rootSet = gcvNULL;
    }

    /* Destroy unnamed name space */
    if (Compiler->context.unnamedSpace != gcvNULL)
    {
        gcmVERIFY_OK(slsNAME_SPACE_Destory(Compiler, Compiler->context.unnamedSpace));
        Compiler->context.unnamedSpace = gcvNULL;
    }

    if (Compiler->context.debugInfo != gcvNULL)
    {
        vscDIDestroyContext(Compiler->context.debugInfo);
        Compiler->context.debugInfo = gcvNULL;
    }

    /* Destroy vec constant list */
    for (i = 0; i < sldMAX_VECTOR_COMPONENT; i++)
    {
        while (!slsDLINK_LIST_IsEmpty(&Compiler->context.vecConstants.typeFloat[i]))
        {
            slsNAME *constVar;

            slsDLINK_LIST_DetachFirst(&Compiler->context.vecConstants.typeFloat[i], slsNAME, &constVar);
            gcmASSERT(constVar->u.variableInfo.constant);
            gcmVERIFY_OK(sloIR_CONSTANT_Destroy(Compiler, &constVar->u.variableInfo.constant->exprBase.base));
            constVar->u.variableInfo.constant = gcvNULL;
            gcmVERIFY_OK(slsNAME_Destory(Compiler, constVar));
        }
        while (!slsDLINK_LIST_IsEmpty(&Compiler->context.vecConstants.typeInt[i]))
        {
            slsNAME *constVar;

            slsDLINK_LIST_DetachFirst(&Compiler->context.vecConstants.typeInt[i], slsNAME, &constVar);
            gcmASSERT(constVar->u.variableInfo.constant);
            gcmVERIFY_OK(sloIR_CONSTANT_Destroy(Compiler, &constVar->u.variableInfo.constant->exprBase.base));
            constVar->u.variableInfo.constant = gcvNULL;
            gcmVERIFY_OK(slsNAME_Destory(Compiler, constVar));
        }
        while (!slsDLINK_LIST_IsEmpty(&Compiler->context.vecConstants.typeUInt[i]))
        {
            slsNAME *constVar;

            slsDLINK_LIST_DetachFirst(&Compiler->context.vecConstants.typeUInt[i], slsNAME, &constVar);
            gcmASSERT(constVar->u.variableInfo.constant);
            gcmVERIFY_OK(sloIR_CONSTANT_Destroy(Compiler, &constVar->u.variableInfo.constant->exprBase.base));
            constVar->u.variableInfo.constant = gcvNULL;
            gcmVERIFY_OK(slsNAME_Destory(Compiler, constVar));
        }
        while (!slsDLINK_LIST_IsEmpty(&Compiler->context.vecConstants.typeBool[i]))
        {
            slsNAME *constVar;

            slsDLINK_LIST_DetachFirst(&Compiler->context.vecConstants.typeBool[i], slsNAME, &constVar);
            gcmASSERT(constVar->u.variableInfo.constant);
            gcmVERIFY_OK(sloIR_CONSTANT_Destroy(Compiler, &constVar->u.variableInfo.constant->exprBase.base));
            constVar->u.variableInfo.constant = gcvNULL;
            gcmVERIFY_OK(slsNAME_Destory(Compiler, constVar));
        }
    }

    /* Destroy general built-in name space */
    if (Compiler->context.generalBuiltinSpace != gcvNULL)
    {
        gcmVERIFY_OK(slsNAME_SPACE_Destory(Compiler, Compiler->context.generalBuiltinSpace));
        Compiler->context.generalBuiltinSpace = gcvNULL;
    }

    /* Destroy switch scope */
    while (!slsSLINK_LIST_IsEmpty(&Compiler->context.switchScope))
    {
        slsSWITCH_SCOPE *scope;
        slsSLINK_LIST_DetachFirst(&Compiler->context.switchScope, slsSWITCH_SCOPE, &scope);
        gcmVERIFY_OK(sloCOMPILER_Free(Compiler, scope));
    }

    /* Destroy layoutOffset */
    while (!slsSLINK_LIST_IsEmpty(&Compiler->context.layoutOffset))
    {
        slsLAYOUT_OFFSET *layoutOffset;
        slsSLINK_LIST_DetachFirst(&Compiler->context.layoutOffset, slsLAYOUT_OFFSET, &layoutOffset);

        while (!slsSLINK_LIST_IsEmpty(&layoutOffset->offset_list))
        {
            slsBINDING_OFFSET_LIST *layoutOffsetList;
            slsSLINK_LIST_DetachFirst(&layoutOffset->offset_list, slsBINDING_OFFSET_LIST, &layoutOffsetList);
            gcmVERIFY_OK(sloCOMPILER_Free(Compiler, layoutOffsetList));
        }

        gcmVERIFY_OK(sloCOMPILER_Free(Compiler, layoutOffset));
    }

    /* Destroy sharedVariables list */
    while (!slsSLINK_LIST_IsEmpty(&Compiler->context.sharedVariables))
    {
        slsSHARED_VARIABLE *sharedVariable;
        slsSLINK_LIST_DetachFirst(&Compiler->context.sharedVariables, slsSHARED_VARIABLE, &sharedVariable);
        gcmVERIFY_OK(sloCOMPILER_Free(Compiler, sharedVariable));
    }

    /* Destroy constantVariables list */
    gcmVERIFY_OK(sloCOMPILER_DestroyConstantVariableList(Compiler));

    /* Destory string pool */
    FOR_EACH_HASH_BUCKET(&Compiler->context.generalStringPool, poolStringBucket)
    {
        while (!slsDLINK_LIST_IsEmpty(poolStringBucket))
        {
            slsDLINK_LIST_DetachFirst(poolStringBucket, slsPOOL_STRING_NODE, &poolStringNode);
            gcmVERIFY_OK(sloCOMPILER_Free(Compiler, poolStringNode));
        }
    }

#if __USE_VSC_MP__
    vscPMP_Finalize(&Compiler->generalPMP);
#else
    gcmVERIFY_OK(sloCOMPILER_EmptyMemoryPool(Compiler, gcvTRUE));
#endif

    slCleanupKeywords();

    /* Free compiler struct */
    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Compiler));

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_Destroy(
    IN sloCOMPILER Compiler
    )
{
    slsDLINK_LIST *         poolStringBucket;
    slsPOOL_STRING_NODE *   poolStringNode;
    gctINT i;

    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    if (Compiler->codeEmitter != gcvNULL)
    {
        gcmVERIFY_OK(sloCODE_EMITTER_Destroy(Compiler, Compiler->codeEmitter));
        Compiler->codeEmitter = gcvNULL;
    }

#ifndef SL_SCAN_NO_PREPROCESSOR
    if (Compiler->preprocessor != gcvNULL)
    {
        gcmVERIFY_OK(sloPREPROCESSOR_Destroy(Compiler, Compiler->preprocessor));
        Compiler->preprocessor = gcvNULL;
    }
#endif

    if (Compiler->binary != gcvNULL)
    {
        gcmVERIFY_OK(gcSHADER_Destroy(Compiler->binary));
        Compiler->binary = gcvNULL;
    }

    if (Compiler->log != gcvNULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Compiler->log));
        Compiler->log = gcvNULL;
    }

    /* Destory the whole IR tree */
    if (Compiler->context.rootSet != gcvNULL)
    {
        gcmVERIFY_OK(sloIR_OBJECT_Destroy(Compiler, &Compiler->context.rootSet->base));
        Compiler->context.rootSet = gcvNULL;
    }

    /* Destroy unnamed name space */
    if (Compiler->context.unnamedSpace != gcvNULL)
    {
        gcmVERIFY_OK(slsNAME_SPACE_Destory(Compiler, Compiler->context.unnamedSpace));
        Compiler->context.unnamedSpace = gcvNULL;
    }

    if (Compiler->context.debugInfo != gcvNULL)
    {
        vscDIDestroyContext(Compiler->context.debugInfo);
        Compiler->context.debugInfo = gcvNULL;
    }

    /* Destroy vec constant list */
    for (i = 0; i < sldMAX_VECTOR_COMPONENT; i++)
    {
        while (!slsDLINK_LIST_IsEmpty(&Compiler->context.vecConstants.typeFloat[i]))
        {
            slsNAME *constVar;

            slsDLINK_LIST_DetachFirst(&Compiler->context.vecConstants.typeFloat[i], slsNAME, &constVar);
            gcmASSERT(constVar->u.variableInfo.constant);
            gcmVERIFY_OK(sloIR_CONSTANT_Destroy(Compiler, &constVar->u.variableInfo.constant->exprBase.base));
            constVar->u.variableInfo.constant = gcvNULL;
            gcmVERIFY_OK(slsNAME_Destory(Compiler, constVar));
        }
        while (!slsDLINK_LIST_IsEmpty(&Compiler->context.vecConstants.typeInt[i]))
        {
            slsNAME *constVar;

            slsDLINK_LIST_DetachFirst(&Compiler->context.vecConstants.typeInt[i], slsNAME, &constVar);
            gcmASSERT(constVar->u.variableInfo.constant);
            gcmVERIFY_OK(sloIR_CONSTANT_Destroy(Compiler, &constVar->u.variableInfo.constant->exprBase.base));
            constVar->u.variableInfo.constant = gcvNULL;
            gcmVERIFY_OK(slsNAME_Destory(Compiler, constVar));
        }
        while (!slsDLINK_LIST_IsEmpty(&Compiler->context.vecConstants.typeUInt[i]))
        {
            slsNAME *constVar;

            slsDLINK_LIST_DetachFirst(&Compiler->context.vecConstants.typeUInt[i], slsNAME, &constVar);
            gcmASSERT(constVar->u.variableInfo.constant);
            gcmVERIFY_OK(sloIR_CONSTANT_Destroy(Compiler, &constVar->u.variableInfo.constant->exprBase.base));
            constVar->u.variableInfo.constant = gcvNULL;
            gcmVERIFY_OK(slsNAME_Destory(Compiler, constVar));
        }
        while (!slsDLINK_LIST_IsEmpty(&Compiler->context.vecConstants.typeBool[i]))
        {
            slsNAME *constVar;

            slsDLINK_LIST_DetachFirst(&Compiler->context.vecConstants.typeBool[i], slsNAME, &constVar);
            gcmASSERT(constVar->u.variableInfo.constant);
            gcmVERIFY_OK(sloIR_CONSTANT_Destroy(Compiler, &constVar->u.variableInfo.constant->exprBase.base));
            constVar->u.variableInfo.constant = gcvNULL;
            gcmVERIFY_OK(slsNAME_Destory(Compiler, constVar));
        }
    }

    /* Clean up some variables in general built-in name space. */
    if (Compiler->context.generalBuiltinSpace != gcvNULL)
    {
        slsNAME *name = gcvNULL;

        /* If a builtin function is used, we need to initialize it. */
        FOR_EACH_DLINK_NODE_REVERSELY(&Compiler->context.generalBuiltinSpace->names, slsNAME, name)
        {
            if (name->type == slvFUNC_NAME && name->context.function != gcvNULL)
            {
                slsNAME *paramName = gcvNULL;

                slsNAME_Initialize(Compiler, name, gcvFALSE);

                FOR_EACH_DLINK_NODE_REVERSELY(&name->u.funcInfo.localSpace->names, slsNAME, paramName)
                {
                    slsNAME_Initialize(Compiler, paramName, gcvFALSE);
                }
            }
        }
    }

    /* Destroy built-in name space */
    if (Compiler->context.builtinSpace != gcvNULL)
    {
        slsNAME_SPACE * subSpace;

        FOR_EACH_DLINK_NODE_REVERSELY(&Compiler->context.generalBuiltinSpace->subSpaces, slsNAME_SPACE, subSpace)
        {
            if (subSpace == Compiler->context.builtinSpace)
            {
                slsDLINK_NODE_Detach((slsDLINK_NODE *)subSpace);
                break;
            }
        }

        gcmVERIFY_OK(slsNAME_SPACE_Destory(Compiler, Compiler->context.builtinSpace));
        Compiler->context.builtinSpace = gcvNULL;
    }


    /* Destroy switch scope */
    while (!slsSLINK_LIST_IsEmpty(&Compiler->context.switchScope))
    {
        slsSWITCH_SCOPE *scope;
        slsSLINK_LIST_DetachFirst(&Compiler->context.switchScope, slsSWITCH_SCOPE, &scope);
        gcmVERIFY_OK(sloCOMPILER_Free(Compiler, scope));
    }

    /* Destroy layoutOffset */
    while (!slsSLINK_LIST_IsEmpty(&Compiler->context.layoutOffset))
    {
        slsLAYOUT_OFFSET *layoutOffset;
        slsSLINK_LIST_DetachFirst(&Compiler->context.layoutOffset, slsLAYOUT_OFFSET, &layoutOffset);

        while (!slsSLINK_LIST_IsEmpty(&layoutOffset->offset_list))
        {
            slsBINDING_OFFSET_LIST *layoutOffsetList;
            slsSLINK_LIST_DetachFirst(&layoutOffset->offset_list, slsBINDING_OFFSET_LIST, &layoutOffsetList);
            gcmVERIFY_OK(sloCOMPILER_Free(Compiler, layoutOffsetList));
        }

        gcmVERIFY_OK(sloCOMPILER_Free(Compiler, layoutOffset));
    }

    /* Destroy sharedVariables list */
    while (!slsSLINK_LIST_IsEmpty(&Compiler->context.sharedVariables))
    {
        slsSHARED_VARIABLE *sharedVariable;
        slsSLINK_LIST_DetachFirst(&Compiler->context.sharedVariables, slsSHARED_VARIABLE, &sharedVariable);
        gcmVERIFY_OK(sloCOMPILER_Free(Compiler, sharedVariable));
    }

    /* Destroy constantVariables list */
    gcmVERIFY_OK(sloCOMPILER_DestroyConstantVariableList(Compiler));

    /* Destory string pool */
    FOR_EACH_HASH_BUCKET(&Compiler->context.privateStringPool, poolStringBucket)
    {
        while (!slsDLINK_LIST_IsEmpty(poolStringBucket))
        {
            slsDLINK_LIST_DetachFirst(poolStringBucket, slsPOOL_STRING_NODE, &poolStringNode);
            gcmVERIFY_OK(sloCOMPILER_Free(Compiler, poolStringNode));
        }
    }

#if __USE_VSC_MP__
    gcmASSERT(Compiler->currentPMP == &Compiler->privatePMP);
    vscPMP_Finalize(&Compiler->privatePMP);
#else
    gcmVERIFY_OK(sloCOMPILER_EmptyMemoryPool(Compiler, gcvFALSE));
#endif

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_GetShaderType(
    IN sloCOMPILER Compiler,
    OUT sleSHADER_TYPE * ShaderType
    )
{
    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(ShaderType);

    *ShaderType = Compiler->shaderType;

    gcmFOOTER_ARG("*ShaderType=%d", *ShaderType);
    return gcvSTATUS_OK;
}

/* If this shader use default UBO. */
gceSTATUS
sloCOMPILER_IsCreateDefaultUBO(
    IN sloCOMPILER Compiler,
    OUT gctBOOL * IsCreateDefaultUBO
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(IsCreateDefaultUBO);

    *IsCreateDefaultUBO = Compiler->createDefaultUBO;

    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_GetBinary(
    IN sloCOMPILER Compiler,
    OUT gcSHADER * Binary
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(Binary);

    *Binary = Compiler->binary;

    if (Compiler->binary == gcvNULL)
    {
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }
    else
    {
        gcmFOOTER_ARG("*Binary=0x%x", *Binary);
        return gcvSTATUS_OK;
    }
}

gceSTATUS
sloCOMPILER_LoadGeneralBuiltIns(
    IN sloCOMPILER Compiler
    )
{
    gceSTATUS           status;
    slsNAME_SPACE *     currentSpace = Compiler->context.currentSpace;

    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    Compiler->context.currentSpace = Compiler->context.generalBuiltinSpace;
    Compiler->context.loadingGeneralBuiltIns = gcvTRUE;
    Compiler->context.loadingBuiltIns = gcvTRUE;

    status = slLoadGeneralBuiltIns(Compiler, Compiler->shaderType);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    Compiler->context.currentSpace = currentSpace;
    Compiler->context.loadingGeneralBuiltIns = gcvFALSE;
    Compiler->context.loadingBuiltIns = gcvFALSE;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_LoadBuiltIns(
    IN sloCOMPILER Compiler
    )
{
    gceSTATUS           status;
    slsNAME_SPACE *     currentSpace = Compiler->context.currentSpace;

    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    Compiler->context.currentSpace = Compiler->context.builtinSpace;
    Compiler->context.loadingBuiltIns = gcvTRUE;

    status = slLoadBuiltIns(Compiler, Compiler->shaderType);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    Compiler->context.loadingBuiltIns = gcvFALSE;
    Compiler->context.currentSpace = currentSpace;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_BuiltinFuncEnabled(
    IN sloCOMPILER Compiler,
    IN gctSTRING Symbol)
{
    gceSTATUS status = gcvSTATUS_OK;
    sloEXTENSION extension = slGetBuiltinFunctionExtension(Symbol);

    if (!(extension.extension1 == slvEXTENSION1_NONE ||
        sloCOMPILER_ExtensionEnabled(Compiler, &extension)))
    {
        status = gcvSTATUS_NOT_SUPPORTED;
    }

    return status;
}

gceSTATUS
sloCOMPILER_MainDefined(
    IN sloCOMPILER Compiler
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    if (Compiler->context.mainDefined)
    {
        status = gcvSTATUS_INVALID_REQUEST;
        gcmFOOTER();
        return status;
    }

    Compiler->context.mainDefined = gcvTRUE;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

#define LOG_BUF_RESERVED_SIZE   1024

gceSTATUS
sloCOMPILER_AddLog(
    IN sloCOMPILER Compiler,
    IN gctCONST_STRING Log
    )
{
    gceSTATUS       status;
    gctUINT         length = 0;
    gctUINT         requiredLogBufSize;
    gctSTRING       newLog;

    gcmHEADER_ARG("Compiler=0x%x Log=0x%x",
                  Compiler, Log);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(Log);

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_COMPILER, Log);

    length = (gctUINT)gcoOS_StrLen(Log, gcvNULL);

    requiredLogBufSize = length + 1;

    if (Compiler->logBufSize > 0)
    {
        length = (gctUINT)gcoOS_StrLen(Compiler->log, gcvNULL);

        requiredLogBufSize += length;
    }

    if (requiredLogBufSize > Compiler->logBufSize)
    {
        gctPOINTER pointer = gcvNULL;

        requiredLogBufSize += LOG_BUF_RESERVED_SIZE;

        status = gcoOS_Allocate(
                                gcvNULL,
                                (gctSIZE_T)requiredLogBufSize,
                                &pointer);
        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }

        newLog = pointer;

        if (Compiler->logBufSize > 0)
        {
            gcmVERIFY_OK(gcoOS_StrCopySafe(newLog, requiredLogBufSize, Compiler->log));
            gcmVERIFY_OK(gcoOS_StrCatSafe(newLog, requiredLogBufSize, Log));

            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Compiler->log));
        }
        else
        {
            gcmASSERT(Compiler->log == gcvNULL);

            gcmVERIFY_OK(gcoOS_StrCopySafe(newLog, requiredLogBufSize, Log));
        }

        Compiler->log           = newLog;
        Compiler->logBufSize    = requiredLogBufSize;
    }
    else
    {
        gcmVERIFY_OK(gcoOS_StrCatSafe(Compiler->log, Compiler->logBufSize, Log));
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_VOutputLog(
    IN sloCOMPILER Compiler,
    IN gctCONST_STRING Message,
    IN gctARGUMENTS Arguments
    )
{
    char *buffer;
    gctUINT offset = 0;
    gceSTATUS status;
    gctSIZE_T bytes;

    gcmHEADER_ARG("Compiler=0x%x Message=0x%x Arguments=0x%x",
                  Compiler, Message, Arguments);

    bytes = (MAX_SINGLE_LOG_LENGTH + 1) * gcmSIZEOF(char);

    gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, (gctPOINTER*) &buffer));

    gcmVERIFY_OK(gcoOS_PrintStrVSafe(buffer, bytes,
                                     &offset,
                                     Message,
                                     Arguments));

    buffer[bytes - 1] = '\0';

    status = sloCOMPILER_AddLog(Compiler, buffer);

    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, buffer));

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_VOutputLogDump(
    IN sloCOMPILER Compiler,
    IN gctCONST_STRING Message,
    IN gctARGUMENTS Arguments
    )
{
    char *buffer;
    gctUINT offset = 0;
    gceSTATUS status;
    gctSIZE_T bytes;
    gctINT i;

    gcmHEADER_ARG("Compiler=0x%x Message=0x%x Arguments=0x%x",
                  Compiler, Message, Arguments);

    bytes = (MAX_SINGLE_LOG_LENGTH + 1) * gcmSIZEOF(char);

    gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, (gctPOINTER*) &buffer));

    buffer[0] = '\0';

    for (i = 0; i < Compiler->context.dumpOffset; i++)
    {
        gcmVERIFY_OK(gcoOS_StrCatSafe(buffer, bytes,"    "));
        offset += 4;
    }

    gcmVERIFY_OK(gcoOS_PrintStrVSafe(buffer, bytes,
                                     &offset,
                                     Message,
                                     Arguments));

    buffer[bytes - 1] = '\0';

    gcmPRINT("%s", buffer);

    /*status = sloCOMPILER_AddLog(Compiler, buffer);*/

    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, buffer));

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_OutputLog(
    IN sloCOMPILER Compiler,
    IN gctCONST_STRING Message,
    IN ...
    )
{
    gceSTATUS    status;
    gctARGUMENTS arguments;

    gcmHEADER_ARG("Compiler=0x%x Message=0x%x ...",
                  Compiler, Message);

    gcmARGUMENTS_START(arguments, Message);
    status = sloCOMPILER_VOutputLog(Compiler,
                                    Message,
                                    arguments);
    gcmARGUMENTS_END(arguments);

    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_GenCode(
    IN sloCOMPILER Compiler
    )
{
    gceSTATUS               status;
    sloCODE_GENERATOR       codeGenerator = gcvNULL;
    sloOBJECT_COUNTER       objectCounter = gcvNULL;
    slsGEN_CODE_PARAMETERS  parameters;
    gctINT                  i;

    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    if (Compiler->context.rootSet == gcvNULL)
    {
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    gcmONERROR(sloCODE_GENERATOR_Construct(Compiler, &codeGenerator));
    Compiler->codeGenerator = codeGenerator;

    /* Count the objects */
    gcmONERROR(sloOBJECT_COUNTER_Construct(Compiler, &objectCounter));

    /* save a pointer of CodeGenerator. */
    objectCounter->codeGenerator = codeGenerator;

    gcmONERROR(sloIR_OBJECT_Accept(Compiler,
                                   &Compiler->context.rootSet->base,
                                   &objectCounter->visitor,
                                   &parameters));

    codeGenerator->attributeCount= objectCounter->attributeCount;
    codeGenerator->uniformCount  = objectCounter->uniformCount;
    codeGenerator->variableCount = objectCounter->variableCount;
    codeGenerator->outputCount   = objectCounter->outputCount;
    codeGenerator->functionCount = objectCounter->functionCount;

    for (i = 0; i < slvOPCODE_MAXOPCODE; i++)
    {
        codeGenerator->opcodeCount[i] = objectCounter->opcodeCount[i];
    }

    gcmVERIFY_OK(sloOBJECT_COUNTER_Destroy(Compiler, objectCounter));
    objectCounter = gcvNULL;

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                  slvDUMP_CODE_GENERATOR,
                                  "Program "
                  "object count: attributes = %d "
                  "uniforms = %d variables = %d "
                  "outputs = %d functions = %d />",
                  codeGenerator->attributeCount,
                  codeGenerator->uniformCount,
                  codeGenerator->variableCount,
                  codeGenerator->outputCount,
                  codeGenerator->functionCount));

    gcmONERROR(sloIR_AllocObjectPointerArrays(Compiler,
                                              codeGenerator));

    slsGEN_CODE_PARAMETERS_Initialize(&parameters, gcvFALSE, gcvFALSE);

    gcmONERROR(sloIR_OBJECT_Accept(Compiler,
                                   &Compiler->context.rootSet->base,
                                   &codeGenerator->visitor,
                                   &parameters));

    /* Do some clean-up work after CG. */
    gcmONERROR(sloCOMPILER_CleanUp(Compiler, codeGenerator));

    slsGEN_CODE_PARAMETERS_Finalize(&parameters);

    gcmVERIFY_OK(sloCODE_GENERATOR_Destroy(Compiler, codeGenerator));
    codeGenerator = gcvNULL;

    gcmONERROR(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_CODE_GENERATOR,
                                "</PROGRAM>"));

    /* Check if 'main' function defined */
    if (!Compiler->context.mainDefined)
    {
        /* GL allows a shader without a main function in FE, we need to do more checks in BE.*/
        if (!sloCOMPILER_IsOGLVersion(Compiler))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            0,
                                            0,
                                            slvREPORT_ERROR,
                                            "'main' function undefined"));
            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }
    }
    else
    {
        gcShaderSetHasDefineMainFunc(Compiler->binary);
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    if (objectCounter != gcvNULL)
    {
        gcmVERIFY_OK(sloOBJECT_COUNTER_Destroy(Compiler, objectCounter));
        objectCounter = gcvNULL;
    }
    if (codeGenerator != gcvNULL)
    {
        gcmVERIFY_OK(sloCODE_GENERATOR_Destroy(Compiler, codeGenerator));
        codeGenerator = gcvNULL;
    }
    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_Compile(
    IN sloCOMPILER Compiler,
    IN sltOPTIMIZATION_OPTIONS OptimizationOptions,
    IN sltDUMP_OPTIONS DumpOptions,
    IN gctUINT StringCount,
    IN gctCONST_STRING Strings[],
    OUT gcSHADER * Binary,
    OUT gctSTRING * Log
    )
{
    gceSTATUS       status;
    gctBOOL         isCreateDefaultUBO;
    gcSHADER        binary = gcvNULL;

    gcmHEADER_ARG("Compiler=0x%x OptimizationOptions=%u DumpOptions=%u "
                  "StringCount=%u Strings=0x%x Binary=0x%x Log=0x%x",
                  Compiler, OptimizationOptions, DumpOptions,
                  StringCount, Strings, Binary, Log);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    gcmASSERT(Binary != gcvNULL);

    *Binary = gcvNULL;

    Compiler->context.optimizationOptions   = OptimizationOptions;
    Compiler->context.extensions.extension1 = slvEXTENSION1_NON_HALTI;
    Compiler->context.extensions.extension2 = slvEXTENSION2_NONE;
    Compiler->context.dumpOptions           = DumpOptions;
    Compiler->context.scannerState          = slvSCANNER_NORMAL;

    /* For OGL, set default language version as 110 first. */
    if (sloCOMPILER_GetClientApiVersion(Compiler) == gcvAPI_OPENGL)
    {
        sloCOMPILER_SetLanguageVersion(Compiler, 110, gcvTRUE);
    }
    /* Check if "GL_ARB_explicit_attrib_location" extention is enable. */
    if (gcoOS_StrStr(GetGLExtensionString(), "GL_ARB_explicit_attrib_location", gcvNULL))
    {
        sloEXTENSION extension = {0};
        extension.extension2 = slvEXTENSION2_GL_ARB_EXPLICIT_ATTRIB_LOCATION;
        sloCOMPILER_EnableExtension(Compiler, &extension, gcvTRUE);
    }
    /* Check if "GL_ARB_uniform_buffer_object" extention is enable. */
    if (gcoOS_StrStr(GetGLExtensionString(), "GL_ARB_uniform_buffer_object", gcvNULL))
    {
        sloEXTENSION extension = {0};
        extension.extension2 = slvEXTENSION2_GL_ARB_UNIFORM_BUFFER_OBJECT;
        sloCOMPILER_EnableExtension(Compiler, &extension, gcvTRUE);
    }

    /* Check if HW has HALTI5 and FMA support */
    if(GetHWHasHalti5() && GetHWHasFmaSupport())
    {
        sloEXTENSION extension = {0};
        extension.extension1 = slvEXTENSION1_HALTI5_WITH_FMA_SUPPORT;
        sloCOMPILER_EnableExtension(Compiler, &extension, gcvTRUE);
    }
    do
    {
        /* Set the global scope as current */
        Compiler->context.currentSpace = Compiler->context.globalSpace;

        /* Parse the source string */
        gcmERR_BREAK(sloCOMPILER_Parse(Compiler,
                                       StringCount,
                                       Strings));

        /* Dump IR. */
        gcmERR_BREAK(sloCOMPILER_DumpIR(Compiler));

        /* Check extension. */
        gcmERR_BREAK(sloCOMPILER_CheckExtensions(Compiler));

        if (Compiler->context.errorCount > 0)
        {
            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            break;
        }

        /* Construct the binary */
        gcmERR_BREAK(gcSHADER_Construct(Compiler->shaderType,
                                        &Compiler->binary));
        binary = Compiler->binary;

        /* Set enable default UBO or not. */
        gcmERR_BREAK(sloCOMPILER_IsCreateDefaultUBO(Compiler, &isCreateDefaultUBO));

        /* Set default UBO. */
        gcmERR_BREAK(gcSHADER_SetDefaultUBO(binary,
                                            (isCreateDefaultUBO && (gcmOPT_CreateDefaultUBO() != 0)) || gcmOPT_CreateDefaultUBO()));

        /* Set shader version. */
        gcmERR_BREAK(gcSHADER_SetCompilerVersion(binary,
                                                 sloCOMPILER_GetVersion(Compiler, Compiler->shaderType)));

        /* Set earlyFragTest. */
        gcmERR_BREAK(gcSHADER_SetEarlyFragTest(binary,
                                               slsCOMPILER_HasEarlyFragText(Compiler->context.compilerFlags)));

        /* Set the client api version for the shader binary. */
        gcmERR_BREAK(gcSHADER_SetClientApiVersion(binary, Compiler->clientApiVersion));

        /* Set patch for centroid varying. */
        SetShaderNeedPatchForCentroid(binary,
                                      slsCOMPILER_HasPatchForCentroidVarying(Compiler->context.compilerFlags));

        /* Set force all invariant. */
        if (sloCOMPILER_GetOutputInvariant(Compiler))
        {
            gcShaderSetAllOutputInvariant(binary);
        }

        /* Generate the code */
        gcmERR_BREAK(sloCOMPILER_GenCode(Compiler));

        if (Compiler->context.errorCount > 0)
        {
            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            break;
        }

        /* Set the layout. */
        gcmERR_BREAK(sloCOMPILER_SetLayout(Compiler));

        /* expand all arrays of arrays. */
        gcmERR_BREAK(gcSHADER_ExpandArraysOfArrays(binary));

        /* save NotStagesRelatedLinkError. */
        gcSHADER_SetNotStagesRelatedLinkError(binary,
                                              Compiler->context.hasNotStagesRelatedLinkError);

        /* Copy the constantVariableList into the constantBuffer. */
        if (Compiler->context.constantBufferSize > 0)
        {
            gctCHAR *buffer;
            gctPOINTER pointer;

            status = sloCOMPILER_Allocate(Compiler,
                                          gcmSIZEOF(gctCHAR) * Compiler->context.constantBufferSize,
                                          &pointer);
            if (gcmIS_ERROR(status)) return gcvSTATUS_OUT_OF_MEMORY;

            buffer = (gctCHAR*)pointer;

            /* Initialize the constant buffer with the constant variables*/
            gcmERR_BREAK(sloCOMPILER_InitializeConstantBuffer(Compiler, buffer));

            /* Copy the buffer into gcSHADER. */
            gcmERR_BREAK(gcSHADER_SetConstantMemorySize(Compiler->binary,
                                                        Compiler->context.constantBufferSize,
                                                        buffer));
            sloCOMPILER_Free(Compiler, pointer);
        }

        /* Pack the binary */
        gcmERR_BREAK(gcSHADER_Pack(binary));

        /* Analyze functions. */
        gcmERR_BREAK(gcSHADER_AnalyzeFunctions(binary, gcvTRUE));

        if (gcmIS_ERROR(status))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            0,
                                            0,
                                            slvREPORT_ERROR,
                                            "Static and dynamic recursion is not allowed."));
            break;
        }

        /* Return */
        *Binary             = binary;
        Compiler->binary    = gcvNULL;

        gcSHADER_SetDebugInfo(*Binary, Compiler->context.debugInfo);
        Compiler->context.debugInfo = gcvNULL;

        /* copy the source code to shader binary */
        if (StringCount == 1)
        {
            (*Binary)->sourceLength = (gctUINT)gcoOS_StrLen(Strings[0], gcvNULL) + 1;
            gcoOS_StrDup(gcvNULL, Strings[0], &(*Binary)->source);
        }

        if (Log != gcvNULL)
        {
            *Log                = Compiler->log;
            Compiler->log       = gcvNULL;
        }

        gcmFOOTER_ARG("*Binary=%lu *Log=0x%x",
                      *Binary, gcmOPT_POINTER(Log));
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    if (Log != gcvNULL)
    {
        *Log                = Compiler->log;
        Compiler->log       = gcvNULL;
    }

    gcmFOOTER();
    return status;
}

#define BUFFER_SIZE     1024

gceSTATUS
sloCOMPILER_Preprocess(
    IN sloCOMPILER Compiler,
    IN sltOPTIMIZATION_OPTIONS OptimizationOptions,
    IN sltDUMP_OPTIONS DumpOptions,
    IN gctUINT StringCount,
    IN gctCONST_STRING Strings[],
    OUT gctSTRING * Log
    )
{
    gceSTATUS   status;
#ifndef SL_SCAN_NO_PREPROCESSOR
    gctCHAR     buffer[BUFFER_SIZE];
    gctINT      actualSize;
#endif

    gcmHEADER_ARG("Compiler=0x%x OptimizationOptions=%u DumpOptions=%u "
                  "StringCount=%u Strings=0x%x Log=0x%x",
                  Compiler, OptimizationOptions, DumpOptions,
                  StringCount, Strings, Log);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    Compiler->context.optimizationOptions   = OptimizationOptions;
    Compiler->context.dumpOptions           = DumpOptions;

    do
    {
        status = sloCOMPILER_MakeCurrent(Compiler, StringCount, Strings);

        if (gcmIS_ERROR(status)) break;

 #ifndef SL_SCAN_NO_PREPROCESSOR
        /* Preprocess the source string */
        while (gcvTRUE)
        {
            status = sloPREPROCESSOR_Parse(
                                        Compiler->preprocessor,
                                        BUFFER_SIZE,
                                        buffer,
                                        &actualSize);

            if (gcmIS_ERROR(status)) break;

            if (actualSize == 0) break;

            gcmVERIFY_OK(sloCOMPILER_OutputLog(
                                            Compiler,
                                            "<PP_TOKEN line=\"%d\" string=\"%d\" text=\"%s\" />",
                                            sloCOMPILER_GetCurrentLineNo(Compiler),
                                            sloCOMPILER_GetCurrentStringNo(Compiler),
                                            buffer));
        }
#endif

        /* Return */
        if (Log != gcvNULL)
        {
            *Log                = Compiler->log;
            Compiler->log       = gcvNULL;
        }

        gcmFOOTER_ARG("*Log=0x%x", gcmOPT_POINTER(Log));
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    if (Log != gcvNULL)
    {
        *Log                = Compiler->log;
        Compiler->log       = gcvNULL;
    }


    gcmFOOTER();
    return status;
}

/* Dump IR data */
gceSTATUS
sloCOMPILER_DumpIR(
    IN sloCOMPILER Compiler
    )
{
    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    if (!(Compiler->context.dumpOptions & slvDUMP_IR))
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_IR,
                                "Dump IR"));

    /* Dump all name spaces and names */
    if (Compiler->context.globalSpace != gcvNULL)
    {
        gcmVERIFY_OK(slsNAME_SPACE_Dump(Compiler, Compiler->context.globalSpace));
    }

    /* Dump syntax tree */
    if (Compiler->context.rootSet != gcvNULL)
    {
        gcmVERIFY_OK(sloCOMPILER_Dump(
                                    Compiler,
                                    slvDUMP_IR,
                                    "--------Root tree------"));

        gcmVERIFY_OK(sloIR_OBJECT_Dump(Compiler, &Compiler->context.rootSet->base));

        gcmVERIFY_OK(sloCOMPILER_Dump(
                                    Compiler,
                                    slvDUMP_IR,
                                    "------Root tree end-----"));
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_CheckExtensions(
    IN sloCOMPILER Compiler
    )
{
    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    /* For TES/TCS/GS shader, if they are not natively supported in this lang version, the corresponding extended behavior must first be enabled. */
    {
        sloEXTENSION extensionTes = {0}, extensionGeo = {0};
        extensionTes.extension1 = slvEXTENSION1_TESSELLATION_SHADER;
        extensionGeo.extension1 = slvEXTENSION1_EXT_GEOMETRY_SHADER;
        switch (Compiler->shaderType)
        {
        case slvSHADER_TYPE_TCS:
        case slvSHADER_TYPE_TES:
            if (Compiler->langVersion < _SHADER_ES32_VERSION &&
                !sloCOMPILER_ExtensionEnabled(Compiler, &extensionTes))
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                0,
                                                0,
                                                slvREPORT_ERROR,
                                                "TESSELLATION extension is not enabled/required."));
            }
            break;

        case slvSHADER_TYPE_GS:
            if (Compiler->langVersion < _SHADER_ES32_VERSION &&
                !sloCOMPILER_ExtensionEnabled(Compiler, &extensionGeo))
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                0,
                                                0,
                                                slvREPORT_ERROR,
                                                "GEOMETRY extension is not enabled/required."));
            }
            break;

        default:
            break;
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_SetScannerState(
    IN sloCOMPILER Compiler,
    IN sleSCANNER_STATE State
    )
{
    gcmHEADER_ARG("Compiler=0x%x State=%d", Compiler, State);

    /* Verify the arguments. */
    slmASSERT_OBJECT(Compiler, slvOBJ_COMPILER);

    Compiler->context.scannerState = State;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

sleSCANNER_STATE
sloCOMPILER_GetScannerState(
    IN sloCOMPILER Compiler
    )
{
    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    /* Verify the arguments. */
    slmASSERT_OBJECT(Compiler, slvOBJ_COMPILER);

    gcmFOOTER_ARG("<return>=%d", Compiler->context.scannerState);
    return Compiler->context.scannerState;
}

gceSTATUS
sloCOMPILER_PushSwitchScope(
IN sloCOMPILER Compiler,
IN sloIR_LABEL Cases
)
{
    gceSTATUS status;
    slsSWITCH_SCOPE *switchScope;
    gctPOINTER pointer;

    gcmHEADER_ARG("Compiler=0x%x Cases=0x%x", Compiler, Cases);

    /* Verify the arguments. */
    slmASSERT_OBJECT(Compiler, slvOBJ_COMPILER);
    status = sloCOMPILER_Allocate(Compiler,
                                  (gctSIZE_T)sizeof(slsSWITCH_SCOPE),
                  (gctPOINTER *) &pointer);
    if (gcmIS_ERROR(status)) {
        gcmFOOTER_NO();
        return gcvSTATUS_OUT_OF_MEMORY;
    }

    switchScope = pointer;
    switchScope->cases = Cases;
    slsSLINK_LIST_InsertFirst(&Compiler->context.switchScope, &switchScope->node);
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}


gceSTATUS
sloCOMPILER_PopSwitchScope(
IN sloCOMPILER Compiler
)
{
    gcmHEADER_ARG("Compiler=0x%x", Compiler);
    if( !slsSLINK_LIST_IsEmpty(&Compiler->context.switchScope) ) {
         slsSWITCH_SCOPE *oldScope;
         slsSLINK_LIST_DetachFirst(&Compiler->context.switchScope, slsSWITCH_SCOPE, &oldScope);
         gcmVERIFY_OK(sloCOMPILER_Free(Compiler, oldScope));
    }
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

slsSWITCH_SCOPE *
sloCOMPILER_GetSwitchScope(
IN sloCOMPILER Compiler
)
{
    gcmHEADER_ARG("Compiler=0x%x", Compiler);
    /* Verify the arguments. */
    slmASSERT_OBJECT(Compiler, slvOBJ_COMPILER);

    if( !slsSLINK_LIST_IsEmpty(&Compiler->context.switchScope) ) {
         gcmFOOTER_ARG("<return>=0x%x", slsSLINK_LIST_First(&Compiler->context.switchScope, slsSWITCH_SCOPE));

         return  slsSLINK_LIST_First(&Compiler->context.switchScope, slsSWITCH_SCOPE);
    }

    gcmFOOTER_ARG("<return>=%s", "<nil>");
    return gcvNULL;
}

gceSTATUS
sloCOMPILER_SetSwitchScope(
IN sloCOMPILER Compiler,
IN sloIR_LABEL Cases
)
{
    gceSTATUS status;
    gcmHEADER_ARG("Compiler=0x%x Cases=0x%x", Compiler, Cases);

    if( !slsSLINK_LIST_IsEmpty(&Compiler->context.switchScope) ) {
        slsSWITCH_SCOPE *switchScope;
        switchScope = slsSLINK_LIST_First(&Compiler->context.switchScope, slsSWITCH_SCOPE);
        switchScope->cases = Cases;
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
    else {
        status =  sloCOMPILER_PushSwitchScope(Compiler, Cases);
        gcmFOOTER();
        return status;
    }
}

gceSTATUS
sloCOMPILER_SearchLayoutOffset(
    IN  sloCOMPILER        Compiler,
    IN  gctUINT            Binding,
    OUT slsLAYOUT_OFFSET **LayoutOffset
    )
{
    gceSTATUS         status       = gcvSTATUS_OK;
    slsLAYOUT_OFFSET *layoutOffset = gcvNULL;

    gcmHEADER_ARG("Compiler=0x%x Binding=0x%x LayoutOffset=0x%x",
                  Compiler, Binding, LayoutOffset);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    *LayoutOffset   = gcvNULL;

    FOR_EACH_SLINK_NODE(&Compiler->context.layoutOffset, slsLAYOUT_OFFSET, layoutOffset)
    {
        if (Binding == layoutOffset->binding)
        {
            *LayoutOffset      = layoutOffset;
            break;
        }
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_ConstructLayoutOffsetInBinding(
    IN  sloCOMPILER        Compiler,
    IN  gctUINT            Offset,
    IN  slsLAYOUT_OFFSET  *LayoutOffset
    )
{
    gceSTATUS               status;
    gctPOINTER              pointer = gcvNULL;
    slsBINDING_OFFSET_LIST *newNode = gcvNULL;

    status = sloCOMPILER_Allocate(Compiler, sizeof(slsBINDING_OFFSET_LIST), &pointer);

    gcoOS_ZeroMemory(pointer, sizeof(slsBINDING_OFFSET_LIST));

    newNode = (slsBINDING_OFFSET_LIST *)pointer;

    slsSLINK_LIST_InsertFirst(&LayoutOffset->offset_list, &newNode->node);

    newNode->offset     = Offset;

    return status;
}

gceSTATUS
sloCOMPILER_FindLayoutOffsetInBinding(
    IN  sloCOMPILER        Compiler,
    IN  slsLAYOUT_OFFSET  *LayoutOffset,
    IN  gctUINT            Offset,
    IN  gctUINT            DataTypeSize,
    IN  gctBOOL            HasIdentifier,
    OUT gctBOOL           *OverLaps
)
{
    gceSTATUS               status            = gcvSTATUS_OK;
    slsBINDING_OFFSET_LIST *bindingOffsetList = gcvNULL;

    gcmHEADER_ARG("Compiler=0x%x LayoutOffset=0x%x",
                  Compiler,  LayoutOffset);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    *OverLaps = gcvFALSE;
    FOR_EACH_SLINK_NODE(&LayoutOffset->offset_list, slsBINDING_OFFSET_LIST, bindingOffsetList)
    {
        if (((Offset + DataTypeSize * 4) > bindingOffsetList->offset &&
            Offset < bindingOffsetList->offset) ||
            Offset == bindingOffsetList->offset)
        {
            *OverLaps = gcvTRUE;
            break;
        }
    }

    if(!(*OverLaps) && HasIdentifier)
    {
        sloCOMPILER_ConstructLayoutOffsetInBinding(Compiler, Offset, LayoutOffset);
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_ConstructLayoutOffset(
    IN  sloCOMPILER        Compiler,
    IN  gctUINT            Binding,
    OUT slsLAYOUT_OFFSET **LayoutOffset
    )
{
    gceSTATUS         status;
    gctPOINTER        pointer = gcvNULL;
    slsLAYOUT_OFFSET *newNode = gcvNULL;

    gcmHEADER_ARG("Compiler=0x%x Binding=%u",
                  Compiler, Binding);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    status = sloCOMPILER_Allocate(Compiler, sizeof(slsLAYOUT_OFFSET), &pointer);

    gcoOS_ZeroMemory(pointer, sizeof(slsLAYOUT_OFFSET));

    newNode = (slsLAYOUT_OFFSET *)pointer;

    slsSLINK_LIST_InsertFirst(&Compiler->context.layoutOffset, &newNode->node);

    newNode->binding     = Binding;
    slsSLINK_LIST_Initialize(&newNode->offset_list);

    *LayoutOffset = newNode;

    gcmFOOTER();
    return status;
}

gctBOOL
sloCOMPILER_ExpandNorm(
    IN sloCOMPILER Compiler
    )
{
    return (Compiler->context.optimizationOptions & slvOPTIMIZATION_EXPAND_NORM);
}

gctBOOL
sloCOMPILER_Extension1Enabled(
    IN sloCOMPILER Compiler,
    IN sleEXTENSION1 Extension
    )
{
    gctBOOL result;
    gcmHEADER_ARG("Compiler=0x%x Extension=%d",
                  Compiler, Extension);

    /* Verify the arguments. */
    slmASSERT_OBJECT(Compiler, slvOBJ_COMPILER);

    result = (Compiler->context.extensions.extension1 & Extension);
    gcmFOOTER_ARG("<return>=%d", result);

    return result;
}

gctBOOL
sloCOMPILER_Extension2Enabled(
    IN sloCOMPILER Compiler,
    IN sleEXTENSION2 Extension
    )
{
    gctBOOL result;
    gcmHEADER_ARG("Compiler=0x%x Extension=%d",
                  Compiler, Extension);

    /* Verify the arguments. */
    slmASSERT_OBJECT(Compiler, slvOBJ_COMPILER);

    result = (Compiler->context.extensions.extension2 & Extension);
    gcmFOOTER_ARG("<return>=%d", result);

    return result;
}

gctBOOL
sloCOMPILER_ExtensionEnabled(
    IN sloCOMPILER Compiler,
    IN sloEXTENSION* Extension
    )
{
    gctBOOL result;
    gcmHEADER_ARG("Compiler=0x%x Extension=%d",
                  Compiler, Extension->extension1);

    /* Verify the arguments. */
    slmASSERT_OBJECT(Compiler, slvOBJ_COMPILER);

    if (Extension->extension2)
    {
        result = (Extension->extension2 == slvEXTENSION2_NONE) || (Compiler->context.extensions.extension2 & Extension->extension2);
    }
    else
    {
        result = (Extension->extension1 == slvEXTENSION1_NONE) || (Compiler->context.extensions.extension1 & Extension->extension1);
    }
    gcmFOOTER_ARG("<return>=%d", result);

    return result;
}

/* Enable extension */
gceSTATUS
sloCOMPILER_EnableExtension(
    IN sloCOMPILER Compiler,
    IN sloEXTENSION* extension,
    IN gctBOOL Enable
    )
{
    gcmHEADER_ARG("Compiler=0x%x Extension=%d Enable=%d",
        Compiler, extension, Enable);

    /* Verify the arguments. */
    slmASSERT_OBJECT(Compiler, slvOBJ_COMPILER);

    if (Enable)
    {
        if (extension->extension1)
        {
            Compiler->context.extensions.extension1 |= extension->extension1;
        }
        else
        {
            Compiler->context.extensions.extension2 |= extension->extension2;
        }
    }
    else
    {
        if (extension->extension1)
        {
            Compiler->context.extensions.extension1 &= ~extension->extension1;
        }
        else
        {
            Compiler->context.extensions.extension2 &= ~extension->extension2;
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

#define MAX_ERROR   100

gceSTATUS
sloCOMPILER_VReport(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleREPORT_TYPE Type,
    IN gctCONST_STRING Message,
    IN gctARGUMENTS Arguments
    )
{
    gcmHEADER_ARG("Compiler=0x%x LineNo=%d StringNo=%d "
                  "Type=%d Message=0x%x Arguments=0x%x",
                  Compiler, LineNo, StringNo,
                  Type, Message, Arguments);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    switch (Type)
    {
    case slvREPORT_FATAL_ERROR:
    case slvREPORT_INTERNAL_ERROR:
    case slvREPORT_ERROR:
        if (Compiler->context.errorCount >= MAX_ERROR)
        {
            gcmFOOTER_NO();
            return gcvSTATUS_OK;
        }
        break;
    default: break;
    }

    if (LineNo > 0)
    {
        sloCOMPILER_OutputLog(Compiler, "(%d:%d) : ", LineNo, StringNo);
    }

    switch (Type)
    {
    case slvREPORT_FATAL_ERROR:
        Compiler->context.errorCount = MAX_ERROR;
        sloCOMPILER_OutputLog(Compiler, "fatal error : ");
        break;

    case slvREPORT_INTERNAL_ERROR:
        Compiler->context.errorCount++;
        sloCOMPILER_OutputLog(Compiler, "internal error : ");
        break;

    case slvREPORT_ERROR:
        Compiler->context.errorCount++;
        sloCOMPILER_OutputLog(Compiler, "error : ");
        break;

    case slvREPORT_WARN:
        Compiler->context.warnCount++;
        sloCOMPILER_OutputLog(Compiler, "warning : ");
        break;

    default:
        gcmASSERT(0);
    }

    sloCOMPILER_VOutputLog(Compiler, Message, Arguments);

    sloCOMPILER_OutputLog(Compiler, "\n");

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_CheckErrorLog(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo
    )
{
    gceSTATUS status = gcvSTATUS_FALSE;

    gcmHEADER_ARG("Compiler=0x%x LineNo=%d StringNo=%d ",
                  Compiler, LineNo, StringNo);

    if (Compiler->context.errorCount)
        status = gcvSTATUS_TRUE;

    gcmFOOTER_NO();
    return status;
}

gceSTATUS
sloCOMPILER_Report(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleREPORT_TYPE Type,
    IN gctCONST_STRING Message,
    IN ...
    )
{
    gceSTATUS    status;
    gctARGUMENTS arguments;

    gcmHEADER_ARG("Compiler=0x%x LineNo=%d StringNo=%d "
                  "Type=%d Message=0x%x ...",
                  Compiler, LineNo, StringNo,
                  Type, Message);

    gcmARGUMENTS_START(arguments, Message);
    status = sloCOMPILER_VReport(Compiler,
                                 LineNo,
                                 StringNo,
                                 Type,
                                 Message,
                                 arguments);
    gcmARGUMENTS_END(arguments);

    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_IncrDumpOffset(
    IN sloCOMPILER Compiler
    )
{
    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_IR,
                                "{"));

    Compiler->context.dumpOffset++;
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_DecrDumpOffset(
    IN sloCOMPILER Compiler
    )
{
    Compiler->context.dumpOffset--;

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_IR,
                                "}"));
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_Dump(
    IN sloCOMPILER Compiler,
    IN sleDUMP_OPTION DumpOption,
    IN gctCONST_STRING Message,
    IN ...
    )
{
    gceSTATUS    status;
    gctARGUMENTS arguments;

    gcmHEADER_ARG("Compiler=0x%x DumpOption=%d Message=0x%x ...",
                  Compiler, DumpOption, Message);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    if (!(Compiler->context.dumpOptions & DumpOption))
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    gcmARGUMENTS_START(arguments, Message);
    status = sloCOMPILER_VOutputLogDump(Compiler, Message, arguments);
    gcmARGUMENTS_END(arguments);

    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_Allocate(
    IN sloCOMPILER Compiler,
    IN gctSIZE_T Bytes,
    OUT gctPOINTER * Memory
    )
{
    gceSTATUS       status = gcvSTATUS_OK;
    gctPOINTER      pointer = gcvNULL;
#if !__USE_VSC_MP__
    slsDLINK_NODE * node;
#endif

    gcmHEADER_ARG("Compiler=0x%x Bytes=%lu", Compiler, Bytes);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

#if __USE_VSC_MP__
    pointer = vscMM_Alloc(&Compiler->currentPMP->mmWrapper, (gctUINT)Bytes);
#else
    status = gcoOS_Allocate(gcvNULL,
                            Bytes + sizeof(slsDLINK_NODE),
                            &pointer);
#endif

    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        0,
                                        0,
                                        slvREPORT_FATAL_ERROR,
                                        "not enough memory"));

        gcmFOOTER();
        return status;
    }

#if __USE_VSC_MP__
    if (Memory)
    {
        *Memory = pointer;
    }
#else
    node = pointer;
    /* Add node into the memory pool */
    if (Compiler->context.loadingGeneralBuiltIns)
    {
        slsDLINK_LIST_InsertLast(&Compiler->generalMemoryPool, node);
    }
    else
    {
        slsDLINK_LIST_InsertLast(&Compiler->privateMemoryPool, node);
    }
    *Memory = (gctPOINTER)(node + 1);
#endif

    gcmFOOTER_ARG("status=%d *Memory=0x%x", status, *Memory);
    return status;
}

gceSTATUS
sloCOMPILER_Free(
    IN sloCOMPILER Compiler,
    IN gctPOINTER Memory
    )
{
#if !__USE_VSC_MP__
    slsDLINK_NODE * node;
#endif
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x Memory=0x%x", Compiler, Memory);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

#if __USE_VSC_MP__
    vscMM_Free(&Compiler->currentPMP->mmWrapper, Memory);
#else
    node = (slsDLINK_NODE *)Memory - 1;
    /* Detach node from the memory pool */
    slsDLINK_NODE_Detach(node);
    status = gcmOS_SAFE_FREE(gcvNULL, node);
#endif

    gcmFOOTER();
    return status;
}

#if gcdUSE_WCLIP_PATCH
gceSTATUS
sloCOMPILER_InsertWClipList(
    IN sloCOMPILER Compiler,
    IN gctINT Index,
    IN gctINT Data0,
    IN gctINT Data1
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    gcmONERROR(gcSHADER_InsertList(Compiler->binary, &Compiler->binary->wClipTempIndexList, Index, Data0, Data1));

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_InsertWClipForUniformList(
    IN sloCOMPILER Compiler,
    IN gctINT Index,
    IN gctINT Data0,
    IN gctINT Data1
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    gcmONERROR(gcSHADER_InsertList(Compiler->binary, &Compiler->binary->wClipUniformIndexList, Index, Data0, Data1));

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_FindWClipForUniformList(
    IN sloCOMPILER Compiler,
    IN gctINT Index,
    IN gctINT * UniformIndex1,
    IN gctINT * UniformIndex2
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcSHADER_LIST list = gcvNULL;

    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    status = gcSHADER_FindList(Compiler->binary, Compiler->binary->wClipUniformIndexList, Index, &list);

    if (status == gcvSTATUS_TRUE)
    {
        if (UniformIndex1)
        {
            *UniformIndex1 = list->data0;
        }
        if (UniformIndex2)
        {
            *UniformIndex2 = list->data1;
        }
    }

    gcmFOOTER();
    return status;
}
#endif

gceSTATUS
sloCOMPILER_GetUniformIndex(
    IN sloCOMPILER Compiler,
    IN gcUNIFORM Uniform,
    IN gctUINT16 * Index
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    gcmONERROR(gcUNIFORM_GetIndex(Uniform, Index));

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_GetAttributeIndex(
    IN sloCOMPILER Compiler,
    IN gcATTRIBUTE Attribute,
    IN gctUINT16 * Index
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    gcmONERROR(gcATTRIBUTE_GetIndex(Attribute, Index));

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_EmptyMemoryPool(
    IN sloCOMPILER Compiler,
    IN gctBOOL     IsGeneralMemoryPool
    )
{
#if __USE_VSC_MP__
    gcmHEADER();
#else
    slsDLINK_NODE * node;

    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    /* Free all unfreed memory block in the pool */
    if (IsGeneralMemoryPool)
    {
        while (!slsDLINK_LIST_IsEmpty(&Compiler->generalMemoryPool))
        {
            slsDLINK_LIST_DetachFirst(&Compiler->generalMemoryPool, slsDLINK_NODE, &node);

            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, node));
        }
    }
    else
    {
        while (!slsDLINK_LIST_IsEmpty(&Compiler->privateMemoryPool))
        {
            slsDLINK_LIST_DetachFirst(&Compiler->privateMemoryPool, slsDLINK_NODE, &node);

            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, node));
        }
    }
#endif

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_AllocatePoolString(
    IN sloCOMPILER Compiler,
    IN gctCONST_STRING String,
    OUT sltPOOL_STRING * PoolString
    )
{
    gceSTATUS               status;
    slsDLINK_NODE *         generalBucket = gcvNULL;
    slsDLINK_NODE *         privateBucket = gcvNULL;
    slsDLINK_NODE *         bucket = gcvNULL;
    gctSIZE_T               length;
    slsPOOL_STRING_NODE *   node;
    gctPOINTER              pointer = gcvNULL;
    gctUINT                 crc32Value = gcEvaluateCRC32ForShaderString(String,
                                                                        (gctUINT)gcoOS_StrLen(String, gcvNULL));

    gcmHEADER_ARG("Compiler=0x%x String=0x%x",
                  Compiler, String);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    if (!Compiler->context.loadingGeneralBuiltIns)
    {
        privateBucket = slsHASH_TABLE_Bucket(&Compiler->context.privateStringPool,
                                      slmBUCKET_INDEX(slHashString(String)));
        FOR_EACH_DLINK_NODE(privateBucket, slsPOOL_STRING_NODE, node)
        {
            if (node->crc32Value == crc32Value)
            {
                *PoolString = node->string;

                gcmFOOTER_ARG("*PoolString=0x%x", *PoolString);
                return gcvSTATUS_OK;
            }
        }
    }

    generalBucket = slsHASH_TABLE_Bucket(&Compiler->context.generalStringPool,
                                  slmBUCKET_INDEX(slHashString(String)));
    FOR_EACH_DLINK_NODE(generalBucket, slsPOOL_STRING_NODE, node)
    {
        if (node->crc32Value == crc32Value)
        {
            *PoolString = node->string;

            gcmFOOTER_ARG("*PoolString=0x%x", *PoolString);
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
    status = sloCOMPILER_Allocate(Compiler,
                                  (gctSIZE_T)sizeof(slsPOOL_STRING_NODE) + length + 1,
                                  &pointer);
    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    node = (slsPOOL_STRING_NODE*)pointer;
    node->crc32Value = crc32Value;
    node->string = (sltPOOL_STRING)((gctINT8 *)node + sizeof(slsPOOL_STRING_NODE));

    gcmVERIFY_OK(gcoOS_StrCopySafe(node->string, length + 1, String));

    slsDLINK_LIST_InsertFirst(bucket, &node->node);

    *PoolString = node->string;

    gcmFOOTER_ARG("*PoolString=0x%x", *PoolString);
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_GetChar(
    IN sloCOMPILER Compiler,
    OUT gctINT_PTR Char
    )
{
    gcmHEADER_ARG("Compiler=0x%x Char=0x%x",
                  Compiler, Char);

    gcmASSERT(Compiler);
    gcmASSERT(Compiler->context.strings);

    if (Compiler->context.strings[Compiler->context.currentStringNo][Compiler->context.currentCharNo] != '\0')
    {
        *Char = Compiler->context.strings[Compiler->context.currentStringNo][Compiler->context.currentCharNo++];
    }
    else if (Compiler->context.currentStringNo == Compiler->context.stringCount)
    {
        *Char = T_EOF;
    }
    else
    {
        gcmASSERT(Compiler->context.currentStringNo < Compiler->context.stringCount);

        Compiler->context.currentStringNo++;
        Compiler->context.currentCharNo = 0;

        while (Compiler->context.currentStringNo < Compiler->context.stringCount)
        {
            if (Compiler->context.strings[Compiler->context.currentStringNo][0] != '\0')
            {
                break;
            }

            Compiler->context.currentStringNo++;
        }

        if (Compiler->context.currentStringNo == Compiler->context.stringCount)
        {
            *Char = T_EOF;
        }
        else
        {
            gcmASSERT(Compiler->context.currentStringNo < Compiler->context.stringCount);
            *Char = Compiler->context.strings[Compiler->context.currentStringNo][Compiler->context.currentCharNo++];
        }
    }

    gcmVERIFY_OK(sloCOMPILER_SetCurrentStringNo(
                                                Compiler,
                                                Compiler->context.currentStringNo));

    gcmVERIFY_OK(sloCOMPILER_SetCurrentLineNo(
                                            Compiler,
                                            Compiler->context.currentLineNo));

    if (*Char == '\n')
    {
        Compiler->context.currentLineNo++;
    }

    gcmFOOTER_ARG("*Char=0x%x", *Char);
    return gcvSTATUS_OK;
}

static sloCOMPILER CurrentCompiler  = gcvNULL;

gceSTATUS
sloCOMPILER_MakeCurrent(
    IN sloCOMPILER Compiler,
    IN gctUINT StringCount,
    IN gctCONST_STRING Strings[]
    )
{
#ifndef SL_SCAN_NO_PREPROCESSOR
    gceSTATUS   status;
#endif

    gcmHEADER_ARG("Compiler=0x%x StringCount=%d Strings=0x%x",
                  Compiler, StringCount, Strings);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    Compiler->context.stringCount       = StringCount;
    Compiler->context.strings           = Strings;
    Compiler->context.currentLineNo     = 1;
    Compiler->context.currentStringNo   = 0;
    Compiler->context.currentCharNo     = 0;

    CurrentCompiler                     = Compiler;

#ifndef SL_SCAN_NO_PREPROCESSOR
    status = sloPREPROCESSOR_SetSourceStrings(
                                            Compiler->preprocessor,
                                            StringCount,
                                            Strings);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }
#endif

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gctINT
slInput(
    IN gctINT MaxSize,
    OUT gctSTRING Buffer
    )
{
#ifdef SL_SCAN_NO_PREPROCESSOR

    gctINT ch;

    gcmHEADER_ARG("MaxSize=0x%x Buffer=0x%x",
                  MaxSize, Buffer);

    gcmASSERT(CurrentCompiler);

    gcmVERIFY_OK(sloCOMPILER_GetChar(CurrentCompiler, &ch));

    if (ch == T_EOF)
    {
        gcmFOOTER_ARG("<return>=%d", 0);
        return 0;
    }

    Buffer[0] = (gctCHAR)ch;

    gcmFOOTER_ARG("<return>=%d", 1);
    return 1;

#else

    gceSTATUS   status;
    gctINT      actualSize;

    gcmHEADER_ARG("MaxSize=0x%x Buffer=0x%x",
                  MaxSize, Buffer);

    status = sloPREPROCESSOR_Parse(
                                CurrentCompiler->preprocessor,
                                MaxSize,
                                Buffer,
                                &actualSize);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%d", 0);
        return 0;
    }

    gcmFOOTER_ARG("<return>=%d", actualSize);
    return actualSize;

#endif
}

gctPOINTER
slMalloc(
    IN gctSIZE_T Bytes
    )
{
    gceSTATUS status;
    gctSIZE_T_PTR memory;
    gctPOINTER pointer = gcvNULL;

    gcmHEADER_ARG("Bytes=%u", Bytes);

    status = sloCOMPILER_Allocate(CurrentCompiler,
                                  Bytes + gcmSIZEOF(gctSIZE_T_PTR),
                                  &pointer);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return gcvNULL;
    }

    memory    = pointer;
    memory[0] = Bytes;

    gcmFOOTER_ARG("<return>=%d", &memory[1]);
    return &memory[1];
}

gctPOINTER
slRealloc(
    IN gctPOINTER Memory,
    IN gctSIZE_T NewBytes
    )
{
    gceSTATUS status;
    gctSIZE_T_PTR memory = gcvNULL;
    gctPOINTER pointer = gcvNULL;
    gcmHEADER_ARG("Memory=0x%x NewBytes=%u",
                  Memory, NewBytes);

    do
    {
        gcmERR_BREAK(
            sloCOMPILER_Allocate(CurrentCompiler,
                                 NewBytes + gcmSIZEOF(gctSIZE_T_PTR),
                                 &pointer));

        memory    = pointer;
        memory[0] = NewBytes;

        gcoOS_MemCopy(memory + 1,
                      Memory,
                      ((gctSIZE_T_PTR) Memory)[-1]);

        gcmERR_BREAK(
            sloCOMPILER_Free(CurrentCompiler,
                             &((gctSIZE_T_PTR) Memory)[-1]));

        gcmFOOTER_ARG("<return>=%d", &memory[1]);
        return &memory[1];
    }
    while (gcvFALSE);

    if (memory != gcvNULL)
    {
        gcmVERIFY_OK(sloCOMPILER_Free(CurrentCompiler, memory));
    }

    gcmFOOTER_ARG("<return>=%d", gcvNULL);
    return gcvNULL;
}

void
slFree(
    IN gctPOINTER Memory
    )
{
    gcmHEADER_ARG("Memory=0x%x", Memory);

    if (Memory != gcvNULL)
    {
        gcmVERIFY_OK(
            sloCOMPILER_Free(CurrentCompiler,
                             &((gctSIZE_T_PTR) Memory)[-1]));
    }

    gcmFOOTER_NO();
}

void
slReport(
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleREPORT_TYPE Type,
    IN gctSTRING Message,
    IN ...
    )
{
    gctARGUMENTS arguments;

    gcmHEADER_ARG("LineNo=%d StringNo=%d Type=%d Message=0x%x ...",
                  LineNo, StringNo, Type, Message);

    gcmASSERT(CurrentCompiler);

    gcmARGUMENTS_START(arguments, Message);
    gcmVERIFY_OK(sloCOMPILER_VReport(CurrentCompiler,
                                     LineNo,
                                     StringNo,
                                     Type,
                                     Message,
                                     arguments));
    gcmARGUMENTS_END(arguments);
    gcmFOOTER_NO();
}

gceSTATUS
sloCOMPILER_CreateDataType(
    IN sloCOMPILER Compiler,
    IN gctINT TokenType,
    IN slsNAME_SPACE * FieldSpace,
    OUT slsDATA_TYPE ** DataType
    )
{
    gceSTATUS       status;
    slsDATA_TYPE *  dataType;

    gcmHEADER_ARG("Compiler=0x%x TokenType=%d FieldSpace=0x%x",
               Compiler, TokenType, FieldSpace);
    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    status = slsDATA_TYPE_Construct(
                                    Compiler,
                                    TokenType,
                                    FieldSpace,
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
sloCOMPILER_CreateArrayDataType(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * ElementDataType,
    IN gctINT ArrayLength,
    OUT slsDATA_TYPE ** DataType
    )
{
    gceSTATUS       status;
    slsDATA_TYPE *  dataType;

    gcmHEADER_ARG("Compiler=0x%x ElementDataType=0x%x ArrayLength=%d",
                  Compiler, ElementDataType, ArrayLength);
    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    status = slsDATA_TYPE_ConstructArray(
                                    Compiler,
                                    ElementDataType,
                                    ArrayLength,
                                    &dataType);
    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    *DataType = dataType;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_CreateArraysOfArraysDataType(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * ElementDataType,
    IN gctINT ArrayLengthCount,
    IN gctINT * ArrayLengthList,
    IN gctBOOL IsAppend,
    OUT slsDATA_TYPE ** DataType
    )
{
    gceSTATUS       status;
    slsDATA_TYPE *  dataType;

    gcmHEADER_ARG("Compiler=0x%x ElementDataType=0x%x ArrayLengthCount=%d",
                  Compiler, ElementDataType, ArrayLengthCount);
    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    status = slsDATA_TYPE_ConstructArraysOfArrays(
                                    Compiler,
                                    ElementDataType,
                                    ArrayLengthCount,
                                    ArrayLengthList,
                                    IsAppend,
                                    &dataType);
    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    *DataType = dataType;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_CreateElementDataType(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * CompoundDataType,
    OUT slsDATA_TYPE ** DataType
    )
{
    gceSTATUS       status;
    slsDATA_TYPE *  dataType;

    gcmHEADER_ARG("Compiler=0x%x CompoundDataType=0x%x",
                  Compiler, CompoundDataType);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    status = slsDATA_TYPE_ConstructElement(
                                    Compiler,
                                    CompoundDataType,
                                    &dataType);
    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    *DataType = dataType;
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_CloneDataType(
    IN sloCOMPILER Compiler,
    IN sltSTORAGE_QUALIFIER Qualifier,
    IN sltPRECISION_QUALIFIER Precision,
    IN slsDATA_TYPE * Source,
    OUT slsDATA_TYPE ** DataType
    )
{
    gceSTATUS       status;
    slsDATA_TYPE *  dataType;

    gcmHEADER_ARG("Compiler=0x%x Qualifier=%d Source=0x%x",
                  Compiler, Qualifier, Source);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    status = slsDATA_TYPE_Clone(Compiler,
                                Qualifier,
                                Precision,
                                Source,
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
sloCOMPILER_DuplicateFieldSpaceForDataType(
    IN sloCOMPILER Compiler,
    IN gctBOOL isInput,
    IN OUT slsDATA_TYPE * DataType
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    slsNAME_SPACE *currentNameSpace = gcvNULL;
    slsNAME *orgFieldName = gcvNULL, *newFieldName = gcvNULL;
    slsDATA_TYPE *newDataType;
    sloEXTENSION extension = {0};
    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x", Compiler, DataType);

    gcmASSERT(DataType->elementType == slvTYPE_STRUCT);

    /* Create a new name space. */
    gcmONERROR(sloCOMPILER_CreateNameSpace(Compiler,
                                           DataType->fieldSpace ? DataType->fieldSpace->spaceName : gcvNULL,
                                           slvNAME_SPACE_TYPE_STRUCT,
                                           &currentNameSpace));
    /* Duplicate field list. */
    extension.extension1 = slvEXTENSION1_NONE;
    FOR_EACH_DLINK_NODE(&DataType->fieldSpace->names, slsNAME, orgFieldName)
    {
        sleSHADER_TYPE shaderType = Compiler->shaderType;
#ifndef __clang__
        orgFieldName = orgFieldName;
#endif
        /* Check data type */
        /* double/interger input must have "flat" qualifier. */
        if (isInput &&
            (slmIsElementTypeDouble(orgFieldName->dataType->elementType) ||
            slmIsElementTypeInteger(orgFieldName->dataType->elementType)) &&
            orgFieldName->dataType->qualifiers.interpolation != slvINTERPOLATION_QUALIFIER_FLAT &&
            DataType->qualifiers.interpolation != slvINTERPOLATION_QUALIFIER_FLAT &&
            shaderType == slvSHADER_TYPE_FRAGMENT &&
            sloCOMPILER_IsOGLVersion(Compiler))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            orgFieldName->lineNo,
                                            orgFieldName->stringNo,
                                            slvREPORT_ERROR,
                                            "double-precision floating-point or integer typed input '%s' has to have flat interpolation qualifier",
                                            orgFieldName->symbol));
            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmONERROR(status);
        }

        if (slsDATA_TYPE_IsOpaque(orgFieldName->dataType) &&
            sloCOMPILER_IsOGLVersion(Compiler))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            orgFieldName->lineNo,
                                            orgFieldName->stringNo,
                                            slvREPORT_ERROR,
                                            "%s of opaque data type is not allowed in interface block",
                                            orgFieldName->symbol));
            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmONERROR(status);
        }

        /* Create a new name. */
        gcmONERROR(sloCOMPILER_CreateName(Compiler,
                                          orgFieldName->lineNo,
                                          orgFieldName->stringNo,
                                          slvFIELD_NAME,
                                          gcvNULL,
                                          orgFieldName->symbol,
                                          extension,
                                          gcvFALSE,
                                          &newFieldName));
        /* Create a new datatype. */
        gcmONERROR(sloCOMPILER_CloneDataType(Compiler,
                                             orgFieldName->dataType->qualifiers.storage,
                                             orgFieldName->dataType->qualifiers.precision,
                                             orgFieldName->dataType,
                                             &newDataType));

        /* If this field is a struct too, duplicate it. */
        if (newDataType->elementType == slvTYPE_STRUCT)
        {
            gcmONERROR(sloCOMPILER_DuplicateFieldSpaceForDataType(Compiler,
                                                                  isInput,
                                                                  newDataType));
        }
        newFieldName->dataType = newDataType;
    }

    DataType->fieldSpace = currentNameSpace;

    /* Reset the name space. */
    gcmONERROR(sloCOMPILER_PopCurrentNameSpace(Compiler, &currentNameSpace));

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_InsertArrayForDataType(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * SourceDataType,
    IN gctINT ArrayLength,
    OUT slsDATA_TYPE ** DataType
    )
{
    gceSTATUS       status;
    slsDATA_TYPE *  dataType;

    gcmHEADER_ARG("Compiler=0x%x SourceDataType=0x%x ArrayLength=%d DataType = 0x%x",
                  Compiler, SourceDataType, ArrayLength, DataType);
    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    status = sloCOMPILER_CreateArraysOfArraysDataType(Compiler,
                                       SourceDataType,
                                       1,
                                       &ArrayLength,
                                       gcvFALSE,
                                       &dataType);
    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    *DataType = dataType;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_CreateName(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleNAME_TYPE Type,
    IN slsDATA_TYPE * DataType,
    IN sltPOOL_STRING Symbol,
    IN sloEXTENSION Extension,
    IN gctBOOL CheckExistedName,
    OUT slsNAME ** Name
    )
{
    gceSTATUS status;
    gctSIZE_T length = 0;
    gctBOOL isBuiltIn = Compiler->context.loadingBuiltIns;

    gcmHEADER_ARG("Compiler=0x%x LineNo=%d StringNo=%d "
                  "Type=%d DataType=0x%x Symbol=0x%x "
                  "Extension=%d Name=0x%x",
                  Compiler, LineNo, StringNo,
                  Type, DataType, Symbol,
                  Extension.extension1, Name);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    if (!Compiler->context.loadingBuiltIns &&
        !Compiler->context.redeclareBuiltInVar)
    {
        /* Some built-in variables can be redeclared, do some checking here. */
        length = gcoOS_StrLen(Symbol, gcvNULL);
        if (length >= 3 &&
            Symbol[0] == 'g' && Symbol[1] == 'l' && Symbol[2] == '_')
        {
            sleSHADER_TYPE              shaderType = Compiler->shaderType;
            slsREDECLARED_VARIABLE*     pRedeclaredVariableList = gcvNULL;
            slsREDECLARED_VARIABLE      redeclaredVariable = { {slvEXTENSION1_NONE, slvEXTENSION2_NONE}, gcvNULL, gcvNULL, gcvNULL };
            slsNAME*                    pBuiltinName = gcvNULL;
            gctUINT                     redeclaredVariableCount = 0, i = 0;
            gctBOOL                     bMatch = gcvFALSE;

            if (shaderType == slvSHADER_TYPE_VERTEX)
            {
                pRedeclaredVariableList = VSRedeclaredVariables;
                redeclaredVariableCount = VSRedeclaredVariableCount;
            }
            else if (shaderType == slvSHADER_TYPE_GS)
            {
                pRedeclaredVariableList = GSRedeclaredVariables;
                redeclaredVariableCount = GSRedeclaredVariableCount;
            }
            else if (shaderType == slvSHADER_TYPE_FRAGMENT)
            {
                pRedeclaredVariableList = FSRedeclaredVariables;
                redeclaredVariableCount = FSRedeclaredVariableCount;
            }

            for (i = 0; i < redeclaredVariableCount; i++)
            {
                redeclaredVariable = pRedeclaredVariableList[i];

                if (!sloCOMPILER_ExtensionEnabled(Compiler, &redeclaredVariable.extension))
                {
                    continue;
                }

                if (!gcmIS_SUCCESS(gcoOS_StrCmp(Symbol, redeclaredVariable.variableName)))
                {
                    continue;
                }

                gcmONERROR(slsNAME_SPACE_SearchBuiltinVariable(Compiler,
                                                               Compiler->context.builtinSpace,
                                                               Symbol,
                                                               Extension,
                                                               &pBuiltinName));

                if (pBuiltinName == gcvNULL)
                {
                    break;
                }

                if (redeclaredVariable.checkFunc == gcvNULL)
                {
                    bMatch = gcvTRUE;
                }
                else
                {
                    bMatch = redeclaredVariable.checkFunc(Compiler, pBuiltinName, DataType);
                }

                break;
            }

            if (!bMatch)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                LineNo,
                                                StringNo,
                                                slvREPORT_ERROR,
                                                "The identifier: '%s' starting with 'gl_' is reserved",
                                                Symbol));

                gcmFOOTER();
                return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            }

            if (redeclaredVariable.updateFunc)
            {
                gcmONERROR(redeclaredVariable.updateFunc(Compiler, pBuiltinName, DataType));
            }

            if (Name)
            {
                *Name = pBuiltinName;
            }

            gcmFOOTER();
            return gcvSTATUS_OK;
        }
    }

    status = slsNAME_SPACE_CreateName(Compiler,
                                      Compiler->context.currentSpace,
                                      LineNo,
                                      StringNo,
                                      Type,
                                      DataType,
                                      Symbol,
                                      isBuiltIn,
                                      Extension,
                                      CheckExistedName,
                                      Name);

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_SetCheckFunctionForBuiltInFunction(
    IN sloCOMPILER Compiler,
    IN slsBuiltInFuncCheck Function,
    IN slsNAME * FuncName
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x Function=0x%x FuncName=0x%x ",
                  Compiler, Function, FuncName);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(Function);
    gcmASSERT(FuncName);
    gcmASSERT(FuncName->type == slvFUNC_NAME);

    FuncName->u.funcInfo.function = Function;

    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_CreateAuxiliaryName(
    IN sloCOMPILER Compiler,
    IN slsNAME* refName,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN slsDATA_TYPE * DataType,
    OUT slsNAME ** Name
    )
{
    gceSTATUS                   status = gcvSTATUS_OK;
    sltPOOL_STRING              auxiArraySymbol = gcvNULL, tempSymbol = gcvNULL;
    gctPOINTER                  pointer = gcvNULL;
    slsNAME*                    name = gcvNULL;

    gcmHEADER_ARG("Compiler=0x%x refName=0x%x LineNo=%u StringNo=%u "
                  "DataType=0x%x Name=0x%x",
                  Compiler, refName, LineNo, StringNo,
                  DataType, Name);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    if (refName)
    {
        gctSIZE_T   symbolSize = 0;

        symbolSize = gcoOS_StrLen(refName->symbol, gcvNULL);
        status = gcoOS_Allocate(
                                gcvNULL,
                                symbolSize+16,
                                &pointer);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        tempSymbol = pointer;

        gcoOS_StrCopySafe(tempSymbol, symbolSize+1, refName->symbol);
        gcoOS_StrCatSafe(tempSymbol, symbolSize+16, "_scalarArray");

        gcmONERROR(sloCOMPILER_AllocatePoolString(Compiler,
                                                  tempSymbol,
                                                  &auxiArraySymbol));

        gcmONERROR(slsNAME_SPACE_Search(Compiler,
                                        refName->mySpace,
                                        auxiArraySymbol,
                                        gcvNULL,
                                        gcvNULL,
                                        gcvFALSE,
                                        gcvFALSE,
                                        &name));

        if (name == gcvNULL)
        {
            gcmONERROR(slsNAME_SPACE_CreateName(Compiler,
                                                refName->mySpace,
                                                refName->lineNo,
                                                refName->stringNo,
                                                slvVARIABLE_NAME, /* We can not use other types since it is auxiliary
                                                                  variable, so DONOT use refName->type here */
                                                DataType,
                                                auxiArraySymbol,
                                                gcvFALSE,
                                                refName->extension,
                                                gcvFALSE,
                                                &name));
        }
    }
    else
    {
        gctUINT       offset = 0;
        gctUINT64     curTime;
        sloEXTENSION  extension = {0};

        status = gcoOS_Allocate(
                                gcvNULL,
                                (gctSIZE_T)256,
                                &pointer);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        tempSymbol = pointer;

        gcoOS_GetTime(&curTime);
        gcoOS_PrintStrSafe(tempSymbol, 256, &offset, "%llu_scalarArray", curTime);

        gcmONERROR(sloCOMPILER_AllocatePoolString(Compiler,
                                                  tempSymbol,
                                                  &auxiArraySymbol));

        gcmONERROR(slsNAME_SPACE_Search(Compiler,
                                        Compiler->context.currentSpace,
                                        auxiArraySymbol,
                                        gcvNULL,
                                        gcvNULL,
                                        gcvFALSE,
                                        gcvFALSE,
                                        &name));

        if (name == gcvNULL)
        {
            extension.extension1 = slvEXTENSION1_NONE;
            gcmONERROR(slsNAME_SPACE_CreateName(Compiler,
                                                Compiler->context.currentSpace,
                                                LineNo,
                                                StringNo,
                                                slvVARIABLE_NAME,
                                                DataType,
                                                auxiArraySymbol,
                                                gcvFALSE,
                                                extension,
                                                gcvFALSE,
                                                &name));
        }
    }

OnError:
    if (Name != gcvNULL) *Name = name;
    if (pointer) {
        gcoOS_Free(gcvNULL, pointer);
        pointer= gcvNULL;
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_SearchName(
    IN sloCOMPILER Compiler,
    IN sltPOOL_STRING Symbol,
    IN gctBOOL Recursive,
    OUT slsNAME ** Name
    )
{
    gceSTATUS status;
    gctSIZE_T length;
    gctBOOL isBuiltInSpace = gcvFALSE;

    gcmHEADER_ARG("Compiler=0x%x Symbol=0x%x "
                  "Recursive=%d Name=0x%x",
                  Compiler, Symbol, Recursive, Name);

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_COMPILER, "Symbol=%s", gcmOPT_STRING(Symbol));
    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_COMPILER, "Name=%x", gcmOPT_POINTER(Name));

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    length = gcoOS_StrLen(Symbol, gcvNULL);

    if (length >= 3 &&
        Symbol[0] == 'g' && Symbol[1] == 'l' && Symbol[2] == '_')
    {
        isBuiltInSpace = gcvTRUE;
    }

    status = slsNAME_SPACE_Search(Compiler,
                                  isBuiltInSpace?
                                  Compiler->context.builtinSpace : Compiler->context.currentSpace,
                                  Symbol,
                                  gcvNULL,
                                  gcvNULL,
                                  Recursive,
                                  gcvFALSE,
                                  Name);

    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_SearchBuiltinName(
    IN sloCOMPILER Compiler,
    IN gctSTRING Symbol,
    OUT slsNAME ** Name
    )
{
    gceSTATUS status;
    sltPOOL_STRING symbolInPool;

    gcmHEADER_ARG("Compiler=0x%x Symbol=0x%x Name=0x%x",
                  Compiler, Symbol, Name);

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_COMPILER, "Symbol=%s", gcmOPT_STRING(Symbol));
    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_COMPILER, "Name=%x", gcmOPT_POINTER(Name));

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    status = sloCOMPILER_AllocatePoolString(Compiler,
                                            Symbol,
                                            &symbolInPool);

    if (gcmIS_ERROR(status)) return status;

    status = slsNAME_SPACE_Search(Compiler,
                                  Compiler->context.builtinSpace,
                                  symbolInPool,
                                  gcvNULL,
                                  gcvNULL,
                                  gcvTRUE,
                                  gcvFALSE,
                                  Name);

    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_SearchIntrinsicBuiltinName(
    IN sloCOMPILER Compiler,
    IN gctSTRING Symbol,
    OUT slsNAME ** Name
    )
{
    gceSTATUS status;
    sltPOOL_STRING symbolInPool;

    gcmHEADER_ARG("Compiler=0x%x Symbol=0x%x Name=0x%x",
                  Compiler, Symbol, Name);

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_COMPILER, "Symbol=%s", gcmOPT_STRING(Symbol));
    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_COMPILER, "Name=%x", gcmOPT_POINTER(Name));

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    status = sloCOMPILER_AllocatePoolString(Compiler,
                                            Symbol,
                                            &symbolInPool);

    if (gcmIS_ERROR(status)) return status;

    status = slsNAME_SPACE_Search(Compiler,
                                  Compiler->context.builtinSpace,
                                  symbolInPool,
                                  gcvNULL,
                                  gcvNULL,
                                  gcvTRUE,
                                  gcvTRUE,
                                  Name);

    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_CheckNewFuncName(
    IN sloCOMPILER Compiler,
    IN slsNAME * FuncName,
    OUT slsNAME ** FirstFuncName
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Compiler=0x%x FuncName=0x%x FirstFuncName=0x%x",
                  Compiler, FuncName, FirstFuncName);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(FuncName);

    /* check general built-in space. */
    status = slsNAME_SPACE_CheckNewFuncName(
                                        Compiler,
                                        Compiler->context.generalBuiltinSpace,
                                        FuncName,
                                        FirstFuncName);

    if (gcmIS_ERROR(status))
    {
        return status;
    }

    /* check built-in space. */
    status = slsNAME_SPACE_CheckNewFuncName(
                                        Compiler,
                                        Compiler->context.builtinSpace,
                                        FuncName,
                                        FirstFuncName);

    if (gcmIS_ERROR(status))
    {
        return status;
    }

    status = slsNAME_SPACE_CheckNewFuncName(
                                        Compiler,
                                        Compiler->context.globalSpace,
                                        FuncName,
                                        FirstFuncName);

    gcmFOOTER();
    return status;
}

/*
** Functions can only be declared within the global name space.
*/
gceSTATUS
slsNAME_SPACE_CheckFuncInGlobalNamespace(
    IN sloCOMPILER Compiler,
    IN slsNAME * FuncName
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x FuncName=0x%x ",
                  Compiler, FuncName);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(FuncName);

    if(FuncName->mySpace != Compiler->context.globalSpace)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        FuncName->lineNo,
                                        FuncName->stringNo,
                                        slvREPORT_ERROR,
                                        "require a constant expression"));
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_CreateNameSpace(
    IN sloCOMPILER Compiler,
    IN sltPOOL_STRING SpaceName,
    IN sleNAME_SPACE_TYPE NameSpaceType,
    OUT slsNAME_SPACE ** NameSpace
    )
{
    gceSTATUS       status;
    slsNAME_SPACE * nameSpace;

    gcmHEADER_ARG("Compiler=0x%x NameSpace=0x%x",
                  Compiler, NameSpace);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(NameSpace);

    status = slsNAME_SPACE_Construct(Compiler,
                                     SpaceName,
                                     Compiler->context.currentSpace,
                                     NameSpaceType,
                                     &nameSpace);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    Compiler->context.currentSpace = nameSpace;

    *NameSpace = nameSpace;

    gcmFOOTER_ARG("*NameSpace=0x%x", *NameSpace);
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_CreateAuxGlobalNameSpace(
    IN sloCOMPILER Compiler,
    IN sltPOOL_STRING SpaceName,
    IN sleNAME_SPACE_TYPE NameSpaceType,
    OUT slsNAME_SPACE ** NameSpace
    )
{
    gceSTATUS       status;

    gcmHEADER_ARG("Compiler=0x%x NameSpace=0x%x",
                  Compiler, NameSpace);

    /* check if an interface block name is reused globally as anything other than a block name. */
    if (sloCOMPILER_IsOGLVersion(Compiler) &&
        SpaceName && SpaceName[0] != '\0')
    {
        slsNAME *name;
        status = slsNAME_SPACE_Search(Compiler,
                                      Compiler->context.currentSpace,
                                      SpaceName,
                                      gcvNULL,
                                      gcvNULL,
                                      gcvFALSE,
                                      gcvFALSE,
                                      &name);

        if (status == gcvSTATUS_OK && name->type != slvINTERFACE_BLOCK_NAME)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Compiler->context.currentLineNo,
                                            Compiler->context.currentStringNo,
                                            slvREPORT_ERROR,
                                            "redefined identifier: '%s'",
                                            SpaceName));

            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }
    }

    status = sloCOMPILER_CreateNameSpace(Compiler,
                                         SpaceName,
                                         NameSpaceType,
                                         NameSpace);
    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    Compiler->context.auxGlobalSpace = *NameSpace;

    gcmFOOTER_ARG("*NameSpace=0x%x", *NameSpace);
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_PopCurrentNameSpace(
    IN sloCOMPILER Compiler,
    OUT slsNAME_SPACE ** PrevNameSpace
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Compiler=0x%x PrevNameSpace=0x%x",
                  Compiler, PrevNameSpace);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    if (Compiler->context.currentSpace == gcvNULL ||
        Compiler->context.currentSpace->parent == gcvNULL)
    {
        status = gcvSTATUS_INTERFACE_ERROR;
        gcmFOOTER();
        return status;
    }

    if (PrevNameSpace != gcvNULL)
    {
        *PrevNameSpace = Compiler->context.currentSpace;
    }

    Compiler->context.currentSpace = Compiler->context.currentSpace->parent;

    gcmFOOTER_ARG("*PrevNameSpace=0x%x", gcmOPT_POINTER(PrevNameSpace));
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_PushUnnamedSpace(
    IN sloCOMPILER Compiler,
    OUT slsNAME_SPACE ** UnnamedSpace
)
{
   gcmHEADER_ARG("Compiler=0x%x UnnamedSpace=0x%x",
                 Compiler, UnnamedSpace);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(UnnamedSpace);

    if (Compiler->context.unnamedSpace == gcvNULL)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_INTERFACE_ERROR;
    }

    *UnnamedSpace = Compiler->context.unnamedSpace;
    Compiler->context.unnamedSpace->parent = Compiler->context.currentSpace;
    Compiler->context.currentSpace = Compiler->context.unnamedSpace;

    gcmFOOTER_ARG("*UnnamedSpace=0x%x", *UnnamedSpace);
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_AtGlobalNameSpace(
    IN sloCOMPILER Compiler,
    OUT gctBOOL * AtGlobalNameSpace
    )
{
    gcmHEADER_ARG("Compiler=0x%x AtGlobalNameSpace=0x%x",
                  Compiler, AtGlobalNameSpace);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(AtGlobalNameSpace);
    gcmASSERT(Compiler->context.currentSpace);
    gcmASSERT(Compiler->context.globalSpace);

    *AtGlobalNameSpace = (Compiler->context.globalSpace == Compiler->context.currentSpace) ||
                         (Compiler->context.auxGlobalSpace == Compiler->context.currentSpace);

    gcmFOOTER_ARG("*AtGlobalNameSpace=%d", *AtGlobalNameSpace);
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_AddExternalDecl(
    IN sloCOMPILER Compiler,
    IN sloIR_BASE ExternalDecl
    )
{
    gceSTATUS   status;

    gcmHEADER_ARG("Compiler=0x%x ExternalDecl=0x%x",
                  Compiler, ExternalDecl);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    status = sloIR_SET_AddMember(
                            Compiler,
                            Compiler->context.rootSet,
                            ExternalDecl);

    gcmFOOTER();
    return status;
}

gceSTATUS
sloIR_BASE_UseAsTextureCoord(
    IN sloCOMPILER Compiler,
    IN sloIR_BASE Base
    )
{
    gceSTATUS               status;
    sloIR_SET               set             = gcvNULL;
    sloIR_UNARY_EXPR        unaryExpr       = gcvNULL;
    sloIR_BINARY_EXPR       binaryExpr      = gcvNULL;
    sloIR_SELECTION         selection       = gcvNULL;
    sloIR_POLYNARY_EXPR     polynaryExpr    = gcvNULL;
    sloIR_BASE              member;

    gcmHEADER_ARG("Compiler=0x%x Base=0x%x",
                  Compiler, Base);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(Base);

    switch (sloIR_OBJECT_GetType(Base))
    {
    case slvIR_SET:
        set = (sloIR_SET)Base;

        FOR_EACH_DLINK_NODE(&set->members, struct _sloIR_BASE, member)
        {
            /* Setup all members */
            status = sloIR_BASE_UseAsTextureCoord(
                                                Compiler,
                                                member);

            if (gcmIS_ERROR(status))
            {
                gcmFOOTER();
                return status;
            }
        }

        gcmFOOTER_NO();
        return gcvSTATUS_OK;

    case slvIR_VARIABLE:
        gcmFOOTER_NO();
        return gcvSTATUS_OK;

    case slvIR_UNARY_EXPR:
        unaryExpr = (sloIR_UNARY_EXPR)Base;
        gcmASSERT(unaryExpr->operand);

        switch (unaryExpr->type)
        {
        case slvUNARY_COMPONENT_SELECTION:
            /* Setup the operand */
            status = sloIR_BASE_UseAsTextureCoord(
                                                Compiler,
                                                &unaryExpr->operand->base);

            if (gcmIS_ERROR(status))
            {
                gcmFOOTER();
                return status;
            }

            break;
        default: break;
        }

        gcmFOOTER_NO();
        return gcvSTATUS_OK;

    case slvIR_BINARY_EXPR:
        binaryExpr = (sloIR_BINARY_EXPR)Base;
        gcmASSERT(binaryExpr->leftOperand);
        gcmASSERT(binaryExpr->rightOperand);

        switch (binaryExpr->type)
        {
        case slvBINARY_SUBSCRIPT:
            /* Setup the left operand */
            status = sloIR_BASE_UseAsTextureCoord(
                                                Compiler,
                                                &binaryExpr->leftOperand->base);

            if (gcmIS_ERROR(status))
            {
                gcmFOOTER();
                return status;
            }

            break;
        default: break;
        }

        switch (binaryExpr->type)
        {
        case slvBINARY_SEQUENCE:
            /* Setup the right operand */
            status = sloIR_BASE_UseAsTextureCoord(
                                                Compiler,
                                                &binaryExpr->rightOperand->base);

            if (gcmIS_ERROR(status))
            {
                gcmFOOTER();
                return status;
            }

            gcmFOOTER_NO();
            return gcvSTATUS_OK;

        default:
            break;
        }

        gcmFOOTER_NO();
        return gcvSTATUS_OK;

    case slvIR_SELECTION:
        selection = (sloIR_SELECTION)Base;
        gcmASSERT(selection->condExpr);

        /* Setup the true operand */
        if (selection->trueOperand != gcvNULL)
        {
            status = sloIR_BASE_UseAsTextureCoord(
                                                Compiler,
                                                selection->trueOperand);

            if (gcmIS_ERROR(status))
            {
                gcmFOOTER();
                return status;
            }
        }

        /* Setup the false operand */
        if (selection->falseOperand != gcvNULL)
        {
            status = sloIR_BASE_UseAsTextureCoord(
                                                Compiler,
                                                selection->falseOperand);

            if (gcmIS_ERROR(status))
            {
                gcmFOOTER();
                return status;
            }
        }

        gcmFOOTER_NO();
        return gcvSTATUS_OK;

    case slvIR_POLYNARY_EXPR:
        polynaryExpr = (sloIR_POLYNARY_EXPR)Base;

        if (polynaryExpr->type != slvPOLYNARY_FUNC_CALL
            && polynaryExpr->operands != gcvNULL)
        {
            /* Setup all operands */
            status = sloIR_BASE_UseAsTextureCoord(
                                                Compiler,
                                                &polynaryExpr->operands->base);

            if (gcmIS_ERROR(status))
            {
                gcmFOOTER();
                return status;
            }
        }

        gcmFOOTER_NO();
        return gcvSTATUS_OK;

    default:
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
}

gceSTATUS
sloCOMPILER_BindFuncCall(
    IN sloCOMPILER Compiler,
    IN OUT sloIR_POLYNARY_EXPR PolynaryExpr
    )
{
    gceSTATUS   status;
    sloIR_BASE  argument;

    gcmHEADER_ARG("Compiler=0x%x PolynaryExpr=0x%x",
                  Compiler, PolynaryExpr);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(PolynaryExpr->type == slvPOLYNARY_FUNC_CALL);

    /* Bind the function name */
    status = slsNAME_SPACE_BindFuncName(
                                        Compiler,
                                        Compiler->context.globalSpace,
                                        PolynaryExpr);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    /* Setup the texture coordinate */
    if (PolynaryExpr->type == slvPOLYNARY_FUNC_CALL
        && PolynaryExpr->funcName->isBuiltIn
        && slIsTextureLookupFunction(PolynaryExpr->funcSymbol))
    {
        gctINT i = 0;
        gcmASSERT(PolynaryExpr->operands);

        FOR_EACH_DLINK_NODE(&PolynaryExpr->operands->members, struct _sloIR_BASE, argument)
        {
            if(i == 0)
            {
                sloIR_VARIABLE sampler = (sloIR_VARIABLE)argument;
                PolynaryExpr->exprBase.dataType->qualifiers.precision = sampler->exprBase.dataType->qualifiers.precision;
            }
            /* check the second function argument */
            if(i == 1)
            {
                status = sloIR_BASE_UseAsTextureCoord(Compiler, argument);
                if (gcmIS_ERROR(status))
                {
                    gcmFOOTER();
                    return status;
                }
            }
            i++;
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gctREG_INDEX
slNewTempRegs(
    IN sloCOMPILER Compiler,
    IN gctUINT RegCount
    )
{
    gctREG_INDEX    regIndex;

    gcmHEADER_ARG("Compiler=0x%x RegCount=%u",
                  Compiler, RegCount);

    slmASSERT_OBJECT(Compiler, slvOBJ_COMPILER);

    regIndex = (gctREG_INDEX)  gcSHADER_NewTempRegs(Compiler->binary, RegCount, gcSHADER_FLOAT_X1);
    gcmFOOTER_ARG("<return>=%u", regIndex);
    return regIndex;
}


/*******************************************************************************
*/

gceSTATUS
sloCOMPILER_SetDebug(
    sloCOMPILER Compiler,
    gctBOOL     Debug
    )
{
    gcmHEADER_ARG("Compiler=0x%x Debug=%d",
                  Compiler, Debug);

    Compiler->context.debug = Debug;
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_SetOptimize(
    sloCOMPILER Compiler,
    gctBOOL     Optimize
    )
{
    gcmHEADER_ARG("Compiler=0x%x Optimize=%d",
                  Compiler, Optimize);

    Compiler->context.optimize = Optimize;
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_SetOutputInvariant(
    IN sloCOMPILER Compiler,
    IN gctBOOL Invariant
    )
{
   gcmHEADER_ARG("Compiler=0x%x Flag=%d",
                 Compiler, Invariant);

   Compiler->context.outputInvariant = Invariant;
   gcmFOOTER_NO();
   return gcvSTATUS_OK;
}

gctBOOL
sloCOMPILER_GetOutputInvariant(
    IN sloCOMPILER Compiler
    )
{
   gcmHEADER_ARG("Compiler=0x%x", Compiler);

   gcmFOOTER_NO();
   return Compiler->context.outputInvariant;
}

gceSTATUS
sloCOMPILER_SetLanguageVersion(
    IN sloCOMPILER Compiler,
    IN gctUINT32 LangVersion,
    IN gctBOOL  IsGLVersion
    )
{
   gcmHEADER_ARG("Compiler=0x%x LangVersion=%u IsGLVersion=0x%x",
                 Compiler, LangVersion, IsGLVersion);

   if (IsGLVersion)
   {
       Compiler->context.extensions.extension1 = 0;
       Compiler->context.extensions.extension1 |= slvEXTENSION1_SUPPORT_OGL;

       switch (LangVersion)
       {
       /* For GLSL only. */
       /* TODO: need to review the keyword table and built-ins and refine the extensions for each GLSL version*/
       case 100:
       case 110:
          Compiler->langVersion = _SHADER_GL11_VERSION;
          Compiler->context.extensions.extension1 &= ~slvEXTENSION1_ES_30_AND_ABOVE;
          Compiler->context.extensions.extension1 |= slvEXTENSION1_NON_HALTI;
          break;

       case 120:
          Compiler->langVersion = _SHADER_GL12_VERSION;
          Compiler->context.extensions.extension1 &= ~slvEXTENSION1_ES_30_AND_ABOVE;
          Compiler->context.extensions.extension1 |= slvEXTENSION1_NON_HALTI;
          break;

       case 130:
          Compiler->langVersion = _SHADER_GL13_VERSION;
          Compiler->context.extensions.extension1 |= (slvEXTENSION1_HALTI | slvEXTENSION1_NON_HALTI | slvEXTENSION1_ES_31
                                          |slvEXTENSION1_EXT_SHADER_IMPLICIT_CONVERSIONS);
          break;

       case 140:
          Compiler->langVersion = _SHADER_GL14_VERSION;
          Compiler->context.extensions.extension1 |= (slvEXTENSION1_HALTI | slvEXTENSION1_NON_HALTI | slvEXTENSION1_ES_31
                                          |slvEXTENSION1_EXT_SHADER_IMPLICIT_CONVERSIONS);
          break;

       case 150:
          Compiler->langVersion = _SHADER_GL15_VERSION;
          Compiler->context.extensions.extension1 |= (slvEXTENSION1_HALTI | slvEXTENSION1_NON_HALTI | slvEXTENSION1_ES_31
                                          |slvEXTENSION1_EXT_SHADER_IMPLICIT_CONVERSIONS | slvEXTENSION1_EXT_GEOMETRY_SHADER );
          break;

       case 330:
          Compiler->langVersion = _SHADER_GL33_VERSION;
          Compiler->context.extensions.extension1 |= (slvEXTENSION1_HALTI | slvEXTENSION1_NON_HALTI | slvEXTENSION1_ES_31
                                          |slvEXTENSION1_EXT_SHADER_IMPLICIT_CONVERSIONS |slvEXTENSION1_EXT_GEOMETRY_SHADER );
          break;

       case 400:
         {
             sleSHADER_TYPE shaderType;
             sloCOMPILER_GetShaderType(Compiler, &shaderType);
             Compiler->langVersion = _SHADER_GL40_VERSION;
             Compiler->context.extensions.extension1 |= (slvEXTENSION1_HALTI |slvEXTENSION1_NON_HALTI | slvEXTENSION1_DOUBLE_DATA_TYPE | slvEXTENSION1_ES_31
                                          | slvEXTENSION1_EXT_GEOMETRY_SHADER | slvEXTENSION1_TESSELLATION_SHADER | slvEXTENSION1_TEXTURE_CUBE_MAP_ARRAY );
             if (shaderType != slvSHADER_TYPE_LIBRARY)
             {
                 Compiler->context.extensions.extension1 |= slvEXTENSION1_EXT_SHADER_IMPLICIT_CONVERSIONS;
             }
             else
             {
                 Compiler->context.extensions.extension1 |= slvEXTENSION1_INTEGER_MIX;
             }
          }
          break;

       default:
          gcmASSERT(0);
          Compiler->langVersion = _SHADER_GL11_VERSION;
          Compiler->context.extensions.extension1 &= ~slvEXTENSION1_HALTI;
          Compiler->context.extensions.extension1 |= slvEXTENSION1_NON_HALTI;
          gcmFOOTER_NO();
          return gcvSTATUS_INVALID_DATA;
       }
   }
   else
   {
       switch (LangVersion)
       {
       case 320:
          Compiler->langVersion = _SHADER_ES32_VERSION;
          Compiler->context.extensions.extension1 &= ~slvEXTENSION1_NON_HALTI;
          Compiler->context.extensions.extension1 |= (slvEXTENSION1_HALTI | slvEXTENSION1_ES_31 | slvEXTENSION1_ES_32 | slvEXTENSION1_INTEGER_MIX);
          Compiler->clientApiVersion = gcvAPI_OPENGL_ES32;
          break;

       case 310:
          Compiler->langVersion = _SHADER_ES31_VERSION;
          Compiler->context.extensions.extension1 &= ~slvEXTENSION1_NON_HALTI;
          Compiler->context.extensions.extension1 |= (slvEXTENSION1_HALTI | slvEXTENSION1_ES_31 | slvEXTENSION1_INTEGER_MIX);
          Compiler->clientApiVersion = gcvAPI_OPENGL_ES31;
          break;

       case 300:
          Compiler->langVersion = _SHADER_HALTI_VERSION;
          Compiler->context.extensions.extension1 &= ~(slvEXTENSION1_NON_HALTI | slvEXTENSION1_ES_31);
          Compiler->context.extensions.extension1 |= slvEXTENSION1_HALTI;
          Compiler->clientApiVersion = gcvAPI_OPENGL_ES30;
          break;

       case 100:
          Compiler->langVersion = _SHADER_ES11_VERSION;
          Compiler->context.extensions.extension1 &= ~slvEXTENSION1_ES_30_AND_ABOVE;
          Compiler->context.extensions.extension1 |= slvEXTENSION1_NON_HALTI;
          Compiler->clientApiVersion = gcvAPI_OPENGL_ES20;
          break;

       default:
          gcmASSERT(0);
          Compiler->langVersion = _SHADER_ES11_VERSION;
          Compiler->context.extensions.extension1 &= ~slvEXTENSION1_HALTI;
          Compiler->context.extensions.extension1 |= slvEXTENSION1_NON_HALTI;
          gcmFOOTER_NO();
          return gcvSTATUS_INVALID_DATA;
       }
   }

   gcmFOOTER_NO();
   return gcvSTATUS_OK;
}

gctUINT32
sloCOMPILER_GetLanguageVersion(
    IN sloCOMPILER Compiler
    )
{
    return Compiler ? Compiler->langVersion : _SHADER_ES11_VERSION;
}

gctUINT32 *
sloCOMPILER_GetVersion(
    IN sloCOMPILER Compiler,
    IN sleSHADER_TYPE ShaderType
    )
{
    gctUINT32 version = _SHADER_ES11_VERSION;
    if (Compiler)
    {
        version = Compiler->langVersion;
    }
    if (sloCOMPILER_IsOGLVersion(Compiler))
    {
        _slCompilerVersion[0] = _SHADER_OGL_LANGUAGE_TYPE | (ShaderType << 16);
    }
    else
    {
        _slCompilerVersion[0] = _SHADER_GL_LANGUAGE_TYPE | (ShaderType << 16);
    }
    _slCompilerVersion[1] = version;
    return _slCompilerVersion;
}

gceAPI
sloCOMPILER_GetClientApiVersion(
    IN sloCOMPILER Compiler
    )
{
    return Compiler->clientApiVersion;
}

gctBOOL
sloCOMPILER_IsHaltiVersion(
    IN sloCOMPILER Compiler
    )
{
    return (sloCOMPILER_GetLanguageVersion(Compiler) >= _SHADER_HALTI_VERSION || sloCOMPILER_IsOGLVersion(Compiler));
}

gctBOOL
sloCOMPILER_IsES20Version(
    IN sloCOMPILER Compiler
    )
{
    return (sloCOMPILER_GetLanguageVersion(Compiler) == _SHADER_ES11_VERSION && !sloCOMPILER_IsOGLVersion(Compiler));
}

gctBOOL
sloCOMPILER_IsES30Version(
    IN sloCOMPILER Compiler
    )
{
    return sloCOMPILER_GetLanguageVersion(Compiler) == _SHADER_HALTI_VERSION;
}

gctBOOL
sloCOMPILER_IsES30VersionOrAbove(
    IN sloCOMPILER Compiler
    )
{
    return (sloCOMPILER_GetLanguageVersion(Compiler) >= _SHADER_HALTI_VERSION) && !sloCOMPILER_IsOGLVersion(Compiler);
}

gctBOOL
sloCOMPILER_IsES31VersionOrAbove(
    IN sloCOMPILER Compiler
    )
{
    return (sloCOMPILER_GetLanguageVersion(Compiler) >= _SHADER_ES31_VERSION) && !sloCOMPILER_IsOGLVersion(Compiler);
}

gctBOOL
sloCOMPILER_IsOGLVersion(
    IN sloCOMPILER Compiler
    )
{
    gctUINT32 version_sig = (sloCOMPILER_GetLanguageVersion(Compiler) >> 8) & 0xFF;
    return (version_sig == _SHADER_GL_VERSION_SIG);
}

gctBOOL
sloCOMPILER_IsOGL11Version(
    IN sloCOMPILER Compiler
    )
{
    return sloCOMPILER_GetLanguageVersion(Compiler) == _SHADER_GL11_VERSION;
}

gctBOOL
sloCOMPILER_IsOGL12Version(
    IN sloCOMPILER Compiler
    )
{
    return sloCOMPILER_GetLanguageVersion(Compiler) == _SHADER_GL12_VERSION;
}

gctBOOL
sloCOMPILER_IsOGL13Version(
    IN sloCOMPILER Compiler
    )
{
    return sloCOMPILER_GetLanguageVersion(Compiler) == _SHADER_GL13_VERSION;
}

gctBOOL
sloCOMPILER_IsOGL14Version(
    IN sloCOMPILER Compiler
    )
{
    return sloCOMPILER_GetLanguageVersion(Compiler) == _SHADER_GL14_VERSION;
}

gctBOOL
sloCOMPILER_IsOGL15Version(
    IN sloCOMPILER Compiler
    )
{
    return sloCOMPILER_GetLanguageVersion(Compiler) == _SHADER_GL15_VERSION;
}

gctBOOL
sloCOMPILER_IsOGL33VersionOrAbove(
    IN sloCOMPILER Compiler,
    IN gctBOOL     bCheckAbove
    )
{
    if (bCheckAbove)
    {
        return sloCOMPILER_GetLanguageVersion(Compiler) >= _SHADER_GL33_VERSION;
    }
    else
    {
        return sloCOMPILER_GetLanguageVersion(Compiler) == _SHADER_GL33_VERSION;
    }
}

gctBOOL
sloCOMPILER_IsOGL40Version(
    IN sloCOMPILER Compiler
    )
{
    return sloCOMPILER_GetLanguageVersion(Compiler) == _SHADER_GL40_VERSION;
}

gctLABEL
slNewLabel(
    IN sloCOMPILER Compiler
    )
{
    gctLABEL    label;

    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    slmASSERT_OBJECT(Compiler, slvOBJ_COMPILER);

    label = 1 + Compiler->context.labelCount;

    Compiler->context.labelCount++;

    gcmFOOTER_ARG("<return>=%u", label);
    return label;
}

gceSTATUS
sloCOMPILER_BackPatch(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator
    )
{
    gcmHEADER();
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    /* Issue on dEQP test dEQP-GLES2.functional.shaders.linkage.varying_4:
    ** Per GLSL spec, VS can declare varying output but not written within
    ** shader, then fragment shader can use this varying input from VS with
    ** undefined value. So we need add this type of varying output of VS in
    ** output table. We defer determination of whether this type of output of
    ** VS needs really to be removed to linkage time of BE. */
    if (Compiler->shaderType != slvSHADER_TYPE_COMPUTE &&
        Compiler->shaderType != slvSHADER_TYPE_FRAGMENT)
    {
        slsNAME_SPACE* globalNameSpace = Compiler->context.globalSpace;
        slAddUnusedOutputPatch(Compiler, CodeGenerator, globalNameSpace);
    }

    if (Compiler->shaderType != slvSHADER_TYPE_COMPUTE &&
        Compiler->shaderType != slvSHADER_TYPE_VERTEX)
    {
        slsNAME_SPACE* globalNameSpace = Compiler->context.globalSpace;
        slAddUnusedInputPatch(Compiler, CodeGenerator, globalNameSpace);
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_ActiveUniformsWithSharedOrStd140(
    IN sloCOMPILER Compiler
    )
{
    gceSTATUS status;
    gctUINT32 ubCount, i;
    gcSHADER shader = Compiler->binary;

    gcmHEADER();

    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    /* Pass 1: make uniform blocks active. */
    gcmONERROR(gcSHADER_GetUniformBlockCount(shader, &ubCount));
    for (i = 0; i < ubCount; i++)
    {
        gcsUNIFORM_BLOCK uniformBlock;
        gcUNIFORM ubUniform;

        gcmVERIFY_OK(gcSHADER_GetUniformBlock(shader, i, &uniformBlock));

        if (!uniformBlock) continue;

        if (GetUBMemoryLayout(uniformBlock) & gcvINTERFACE_BLOCK_SHARED ||
            GetUBMemoryLayout(uniformBlock) & gcvINTERFACE_BLOCK_STD140)
        {
            gcSHADER_GetUniform(shader, GetUBIndex(uniformBlock), &ubUniform);

            /* Set active flag. */
            ResetUniformFlag(ubUniform, gcvUNIFORM_FLAG_IS_INACTIVE);

            SetUniformFlag(ubUniform, gcvUNIFORM_FLAG_STD140_SHARED);
        }
    }

    /* Pass 2: make members of uniform block active. */
    for (i = 0; i < shader->uniformCount; i++)
    {
        gcUNIFORM uniform = shader->uniforms[i];
        gcsUNIFORM_BLOCK uniformBlock;

        if (!uniform || !isUniformBlockMember(uniform)) continue;

        gcmVERIFY_OK(gcSHADER_GetUniformBlock(shader, uniform->blockIndex, &uniformBlock));

        if (!uniformBlock) continue;

        if (GetUBMemoryLayout(uniformBlock) & gcvINTERFACE_BLOCK_SHARED ||
            GetUBMemoryLayout(uniformBlock) & gcvINTERFACE_BLOCK_STD140)
        {
            /* Set active flag. */
            ResetUniformFlag(uniform, gcvUNIFORM_FLAG_IS_INACTIVE);

            SetUniformFlag(uniform, gcvUNIFORM_FLAG_STD140_SHARED);
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_CheckAssignmentForGLFragData(
    IN sloCOMPILER Compiler
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcSHADER shader = Compiler->binary;
    slsNAME* name;
    gctBOOL useFragData = gcvFALSE;
    gctUINT16 regIndex = 0;
    gctINT i;
    gctUINT lastInst = shader->lastInstruction;

    gcmHEADER();

    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    if (shader->type != gcSHADER_TYPE_FRAGMENT ||
        shader->outputCount == 0 ||
        sloCOMPILER_IsHaltiVersion(Compiler))
    {
        gcmFOOTER_NO();
        return status;
    }

    /* Check if shader use gl_FragData. */
    FOR_EACH_DLINK_NODE(&sloCOMPILER_GetBuiltInSpace(Compiler)->names, slsNAME, name)
    {
        if (name->dataType->qualifiers.storage == slvSTORAGE_QUALIFIER_FRAGMENT_OUT &&
            name->type == slvVARIABLE_NAME &&
            name->u.variableInfo.isReferenced &&
            gcmIS_SUCCESS(gcoOS_StrCmp(name->symbol, "gl_FragData")))
        {
            useFragData = gcvTRUE;
            regIndex = (gctUINT16)name->context.logicalRegs->regIndex;
            break;
        }
    }

    if (!useFragData)
    {
        gcmFOOTER_NO();
        return status;
    }

    /* Check if there is dynamically used for gl_FrgaData. */
    for(i = lastInst; i >= 0; i--)
    {
        gcSL_INSTRUCTION code = &shader->code[i];
        gcSL_OPCODE opcode = gcmSL_OPCODE_GET(code->opcode, Opcode);
        gcSL_INDEXED indexed = gcmSL_TARGET_GET(code->temp, Indexed);
        gcSL_ENABLE  enable;
        gctUINT16 tempIndex = code->tempIndexed;
        gctFLOAT constZero = 0.0;

        /* Skip call and jmp. */
        if (opcode == gcSL_CALL || opcode == gcSL_JMP)
        {
            continue;
        }

        /* Skip non-indexed usage. */
        if (code->tempIndex != regIndex || indexed == gcSL_NOT_INDEXED)
        {
            continue;
        }

#ifndef __clang__
        tempIndex = tempIndex;
#endif
        gcmONERROR(gcSHADER_InsertNOP2BeforeCode(shader, i, 1, gcvTRUE, gcvTRUE));
        shader->lastInstruction = i;
        shader->instrIndex = gcSHADER_OPCODE;

        /* Only allow APP to change RT:0. */
        switch(indexed)
        {
        case gcSL_INDEXED_X:
            enable = gcSL_ENABLE_X;
            break;

        case gcSL_INDEXED_Y:
            enable = gcSL_ENABLE_Y;
            break;

        case gcSL_INDEXED_Z:
            enable = gcSL_ENABLE_Z;
            break;

        default:
            enable = gcSL_ENABLE_W;
            break;
        }

        gcmONERROR(gcSHADER_AddOpcode(shader, gcSL_MOV, tempIndex, enable, gcSL_FLOAT, gcSHADER_PRECISION_ANY, code->srcLoc));
        gcmONERROR(gcSHADER_AddSourceConstantFormatted(shader, &constZero, gcSL_FLOAT));

        shader->lastInstruction = i + 2;

        lastInst++;
        shader->lastInstruction = lastInst;
    }

    gcmFOOTER_NO();
    return status;

OnError:
    gcmFOOTER_NO();
    return status;
}

gceSTATUS
sloCOMPILER_UpdateBuiltinDataType(
    IN sloCOMPILER Compiler
    )
{
    gceSTATUS       status = gcvSTATUS_OK;
    gcSHADER        shader = Compiler->binary;
    gctUINT         i;
    gcATTRIBUTE     attr = gcvNULL;

    gcmHEADER();
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    /* Update CS. */
    if (GetShaderType(shader) == gcSHADER_TYPE_COMPUTE)
    {
        for (i = 0; i < shader->attributeCount; i++)
        {
            attr = shader->attributes[i];
            if (attr == gcvNULL)
            {
                continue;
            }

            /* HW uses uint4 for localId, update it. */
            if (GetATTRNameLength(attr) == gcSL_LOCAL_INVOCATION_ID)
            {
                SetATTRType(attr, gcSHADER_UINT_X4);
            }
        }
    }

    gcmFOOTER_NO();
    return status;
}

gceSTATUS
sloCOMPILER_ActiveSSBOWithSharedOrStd140OrStd430(
    IN sloCOMPILER Compiler
    )
{
    gceSTATUS status;
    gctUINT32 ssboCount, i;
    gcSHADER shader = Compiler->binary;

    gcmHEADER();

    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    /* Pass 1: add all ssbo with layout std140/std430/shared to shader binary. */
    gcmONERROR(slPackSSBOWithSharedOrStd140OrStd430(Compiler, Compiler->context.globalSpace));

    /* Pass 2: make uniform blocks active. */
    gcmONERROR(gcSHADER_GetStorageBlockCount(shader, &ssboCount));
    for (i = 0; i < ssboCount; i++)
    {
        gcsSTORAGE_BLOCK ssbo;
        gcUNIFORM ssboUniform;

        gcmVERIFY_OK(gcSHADER_GetStorageBlock(shader, i, &ssbo));

        if (!ssbo) continue;

        if (GetSBMemoryLayout(ssbo) & gcvINTERFACE_BLOCK_SHARED ||
            GetSBMemoryLayout(ssbo) & gcvINTERFACE_BLOCK_STD140 ||
            GetSBMemoryLayout(ssbo) & gcvINTERFACE_BLOCK_STD430)
        {
            gcSHADER_GetUniform(shader, GetSBIndex(ssbo), &ssboUniform);

            /* Set active flag. */
            ResetUniformFlag(ssboUniform, gcvUNIFORM_FLAG_IS_INACTIVE);

            SetUniformFlag(ssboUniform, gcvUNIFORM_FLAG_STD140_SHARED);
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_CleanUp(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER();

    /* Add some unused variables. */
    gcmONERROR(sloCOMPILER_BackPatch(Compiler, CodeGenerator));

    /* Make uniforms with shared or std140 active. */
    gcmONERROR(sloCOMPILER_ActiveUniformsWithSharedOrStd140(Compiler));

    /* Make SBOs with std430 or std140 active. */
    gcmONERROR(sloCOMPILER_ActiveSSBOWithSharedOrStd140OrStd430(Compiler));

    /* Check assignment for GL fragData. */
    gcmONERROR(sloCOMPILER_CheckAssignmentForGLFragData(Compiler));

    /* Update datatype of builtin variables to match our internal implementation. */
    gcmONERROR(sloCOMPILER_UpdateBuiltinDataType(Compiler));

OnError:
    gcmFOOTER_NO();
    return status;
}

gceSTATUS
sloCOMPILER_SetDefaultPrecision(
    IN sloCOMPILER Compiler,
    IN sltELEMENT_TYPE TypeToSet,
    IN sltPRECISION_QUALIFIER Precision
    )
{
    gceSTATUS   status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x TypeToSet=0x%x Precision=0x%x",
                  Compiler, TypeToSet, Precision);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    Compiler->context.currentSpace->defaultPrecision[TypeToSet] = Precision;

    if (TypeToSet == slvTYPE_INT)
    {
        Compiler->context.currentSpace->defaultPrecision[slvTYPE_UINT] = Precision;
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_GetDefaultPrecision(
    IN sloCOMPILER Compiler,
    IN sltELEMENT_TYPE TypeToGet,
    OUT sltPRECISION_QUALIFIER *Precision
    )
{
    gceSTATUS   status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x TypeToSet=0x%x Precision=0x%x",
                  Compiler, TypeToGet, Precision);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    if (slmIsElementTypeInteger(TypeToGet) &&
        !slmIsElementTypeBoolean(TypeToGet))
    {
        *Precision = Compiler->context.currentSpace->defaultPrecision[slvTYPE_INT];
    }
    else
    {
        *Precision = Compiler->context.currentSpace->defaultPrecision[TypeToGet];
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_PushDataTypeName(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE_NAME *    DataTypeName
    )
{
    gcmHEADER_ARG("Compiler=0x%x DataTypeName=0x%x", Compiler, DataTypeName);

    slsDLINK_LIST_InsertFirst(&Compiler->context.dataTypeNameList, &DataTypeName->node);

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_PopDataTypeName(
    IN sloCOMPILER Compiler,
    OUT slsDATA_TYPE_NAME **    DataTypeName
    )
{
    slsDATA_TYPE_NAME *dataTypeName;

    gcmHEADER_ARG("Compiler=0x%x DataTypeName=0x%x", Compiler, DataTypeName);

    slsDLINK_LIST_DetachFirst(&Compiler->context.dataTypeNameList, slsDATA_TYPE_NAME, &dataTypeName);

    if (DataTypeName)
    {
        *DataTypeName = dataTypeName;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gctBOOL
slNameIsLocal(
    IN sloCOMPILER Compiler,
    IN slsNAME * Name
)
{
    gctBOOL isLocal = (Name->mySpace != gcvNULL) &&
                      (Name->mySpace->parent != gcvNULL) &&
                      (Name->mySpace != Compiler->context.globalSpace);
    gcmHEADER_ARG("Name=0x%x", Name);

    gcmFOOTER_ARG("<return>=%d", isLocal);

    return isLocal;
}

slsNAME_SPACE *
sloCOMPILER_GetCurrentSpace(
    IN sloCOMPILER Compiler
)
{
    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    gcmFOOTER_ARG("<return>=0x%x", Compiler->context.currentSpace);

    return Compiler->context.currentSpace;
}

slsNAME_SPACE *
sloCOMPILER_GetCurrentFunctionSpace(
    IN sloCOMPILER Compiler
)
{
    slsNAME_SPACE *nameSpace = Compiler->context.currentSpace;
    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    while(nameSpace)
    {
        if(nameSpace->nameSpaceType == slvNAME_SPACE_TYPE_FUNCTION)
        {
            break;
        }
        nameSpace = nameSpace->parent;
    }

    gcmFOOTER_ARG("<return>=0x%x", nameSpace);

    return nameSpace;
}

slsNAME_SPACE *
sloCOMPILER_GetGlobalSpace(
    IN sloCOMPILER Compiler
)
{
    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    gcmFOOTER_ARG("<return>=0x%x", Compiler->context.globalSpace);

    return Compiler->context.globalSpace;
}

slsNAME_SPACE *
sloCOMPILER_GetBuiltInSpace(
    IN sloCOMPILER Compiler
)
{
    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    gcmFOOTER_ARG("<return>=0x%x", Compiler->context.builtinSpace);

    return Compiler->context.builtinSpace;
}

slsNAME_SPACE *
sloCOMPILER_GetUnnamedSpace(
    IN sloCOMPILER Compiler
)
{
    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    gcmFOOTER_ARG("<return>=0x%x", Compiler->context.unnamedSpace);

    return Compiler->context.unnamedSpace;
}

gceSTATUS
sloCOMPILER_MergeLayoutId(
    IN sloCOMPILER Compiler,
    IN slsLAYOUT_QUALIFIER *DefaultLayout,
    IN OUT slsLAYOUT_QUALIFIER *Layout
)
{
    gcmHEADER_ARG("Compiler=0x%x, DefaultLayout=0x%x Layout=0x%x",
                  Compiler, DefaultLayout, Layout);

    gcmASSERT(DefaultLayout && Layout);

    Layout->maxVerticesNumber = DefaultLayout->maxVerticesNumber;

    if(!(Layout->id & slvLAYOUT_LOCATION))
    {
       Layout->location = DefaultLayout->location;
    }

    if(!(Layout->id & sldLAYOUT_MEMORY_BIT_FIELDS))
    {
       Layout->id |= DefaultLayout->id & sldLAYOUT_MEMORY_BIT_FIELDS;
    }

    if(!(Layout->id & sldLAYOUT_MATRIX_BIT_FIELDS))
    {
       Layout->id |= DefaultLayout->id & sldLAYOUT_MATRIX_BIT_FIELDS;
    }

    if(!(Layout->id & sldLAYOUT_BLEND_SUPPORT_BIT_FIELDS ))
    {
       Layout->id |= DefaultLayout->id & sldLAYOUT_BLEND_SUPPORT_BIT_FIELDS;
    }

    if(!(Layout->id & slvLAYOUT_WORK_GROUP_SIZE_X))
    {
       Layout->id |= DefaultLayout->id & slvLAYOUT_WORK_GROUP_SIZE_X;
       Layout->workGroupSize[0] = DefaultLayout->workGroupSize[0];
    }

    if(!(Layout->id & slvLAYOUT_WORK_GROUP_SIZE_Y))
    {
       Layout->id |= DefaultLayout->id & slvLAYOUT_WORK_GROUP_SIZE_Y;
       Layout->workGroupSize[1] = DefaultLayout->workGroupSize[1];
    }

    if(!(Layout->id & slvLAYOUT_WORK_GROUP_SIZE_Z))
    {
       Layout->id |= DefaultLayout->id & slvLAYOUT_WORK_GROUP_SIZE_Z;
       Layout->workGroupSize[2] = DefaultLayout->workGroupSize[2];
    }

    if(!(Layout->id & slvLAYOUT_BINDING ))
    {
       Layout->id |= DefaultLayout->id & slvLAYOUT_BINDING;
    }

    if(!(Layout->id & slvLAYOUT_OFFSET ))
    {
        Layout->id |= DefaultLayout->id & slvLAYOUT_OFFSET;
    }

    if(!(Layout->id & slvLAYOUT_EARLY_FRAGMENT_TESTS ))
    {
        Layout->id |= DefaultLayout->id & slvLAYOUT_EARLY_FRAGMENT_TESTS;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_MergeInterFaceLayoutId(
    IN sloCOMPILER Compiler,
    IN slsLAYOUT_QUALIFIER *DefaultLayout,
    IN gctBOOL IsAtomicCounter,
    IN gctBOOL IsInterFace,
    IN OUT slsLAYOUT_QUALIFIER *Layout
)
{
    gcmHEADER_ARG("Compiler=0x%x, DefaultLayout=0x%x Layout=0x%x",
                  Compiler, DefaultLayout, Layout);

    gcmASSERT(DefaultLayout && Layout);

    if (!(Layout->id & slvLAYOUT_LOCATION) &&
        (DefaultLayout->id & slvLAYOUT_LOCATION))
    {
        Layout->location = DefaultLayout->location;
        Layout->id |= slvLAYOUT_LOCATION;
    }

    if (!(Layout->id & sldLAYOUT_MEMORY_BIT_FIELDS) &&
        (DefaultLayout->id & sldLAYOUT_MEMORY_BIT_FIELDS) &&
        IsInterFace)
    {
        Layout->id |= DefaultLayout->id & sldLAYOUT_MEMORY_BIT_FIELDS;
    }

    if (!(Layout->id & sldLAYOUT_MATRIX_BIT_FIELDS) &&
        (DefaultLayout->id & sldLAYOUT_MATRIX_BIT_FIELDS) &&
        IsInterFace)
    {
        Layout->id |= DefaultLayout->id & sldLAYOUT_MATRIX_BIT_FIELDS;
    }

    if (!(Layout->id & slvLAYOUT_BINDING) &&
        (DefaultLayout->id & slvLAYOUT_BINDING) &&
        !IsAtomicCounter)
    {
        Layout->binding = DefaultLayout->binding;
        Layout->id |= DefaultLayout->id & slvLAYOUT_BINDING;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_MergeExtLayoutId(
    IN sloCOMPILER Compiler,
    IN slsLAYOUT_QUALIFIER *DefaultLayout,
    IN OUT slsLAYOUT_QUALIFIER *Layout
)
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x, DefaultLayout=0x%x Layout=0x%x",
                  Compiler, DefaultLayout, Layout);

    gcmASSERT(DefaultLayout && Layout);

    if(!(Layout->ext_id & slvLAYOUT_EXT_GS_PRIMITIVE))
    {
       Layout->ext_id |= DefaultLayout->ext_id & slvLAYOUT_EXT_GS_PRIMITIVE;
       if (DefaultLayout->ext_id & slvLAYOUT_EXT_GS_PRIMITIVE)
       {
           Layout->gsPrimitive = DefaultLayout->gsPrimitive;
       }
    }

    if(!(Layout->ext_id & slvLAYOUT_EXT_INVOCATIONS ))
    {
        Layout->ext_id |= DefaultLayout->ext_id & slvLAYOUT_EXT_INVOCATIONS;
        if (DefaultLayout->ext_id & slvLAYOUT_EXT_INVOCATIONS)
        {
            Layout->gsInvocationTime = DefaultLayout->gsInvocationTime;
        }
    }

    /* If there are multiple layout declaration for TES input or TCS output, they must be matched. */
    if ((DefaultLayout->ext_id & slvLAYOUT_EXT_TS_PRIMITIVE_MODE)   &&
        (Layout->ext_id & slvLAYOUT_EXT_TS_PRIMITIVE_MODE)          &&
        (DefaultLayout->tesPrimitiveMode != Layout->tesPrimitiveMode))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        0,
                                        0,
                                        slvREPORT_ERROR,
                                        "primitive mode mismatch."));
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }
    if ((DefaultLayout->ext_id & slvlAYOUT_EXT_VERTEX_SPACING)   &&
        (Layout->ext_id & slvlAYOUT_EXT_VERTEX_SPACING)          &&
        (DefaultLayout->tesVertexSpacing != Layout->tesVertexSpacing))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        0,
                                        0,
                                        slvREPORT_ERROR,
                                        "spacing mode mismatch."));
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }
    if ((DefaultLayout->ext_id & slvlAYOUT_EXT_ORERING)   &&
        (Layout->ext_id & slvlAYOUT_EXT_ORERING)          &&
        (DefaultLayout->tesOrdering != Layout->tesOrdering))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        0,
                                        0,
                                        slvREPORT_ERROR,
                                        "vertex order mismatch."));
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }
    if ((DefaultLayout->ext_id & slvLAYOUT_EXT_VERTICES)   &&
        (Layout->ext_id & slvLAYOUT_EXT_VERTICES)          &&
        (DefaultLayout->verticesNumber != Layout->verticesNumber))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        0,
                                        0,
                                        slvREPORT_ERROR,
                                        "vertex count mismatch."));
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }

    gcmFOOTER_NO();
    return status;
}

gctBOOL
sloCOMPILER_DefaultComputeGroupLayoutMatch(
    IN sloCOMPILER Compiler,
    IN slsLAYOUT_QUALIFIER *Layout
)
{
    if((Compiler->context.inDefaultLayout.id & Layout->id & slvLAYOUT_WORK_GROUP_SIZE_X) && Compiler->context.inDefaultLayout.workGroupSize[0] != Layout->workGroupSize[0])
    {
        return gcvFALSE;
    }
    if((Compiler->context.inDefaultLayout.id & Layout->id & slvLAYOUT_WORK_GROUP_SIZE_Y) && Compiler->context.inDefaultLayout.workGroupSize[1] != Layout->workGroupSize[1])
    {
        return gcvFALSE;
    }
    if((Compiler->context.inDefaultLayout.id & Layout->id & slvLAYOUT_WORK_GROUP_SIZE_Z) && Compiler->context.inDefaultLayout.workGroupSize[2] != Layout->workGroupSize[2])
    {
        return gcvFALSE;
    };

    return gcvTRUE;
}

gceSTATUS
sloCOMPILER_SetDefaultLayout(
    IN sloCOMPILER Compiler,
    IN slsLAYOUT_QUALIFIER *Layout,
    IN sltSTORAGE_QUALIFIER StorageQualifier
    )
{
    gceSTATUS status;
    slsLAYOUT_QUALIFIER defaultLayout[1];
    slsLAYOUT_QUALIFIER layout[1];

    gcmHEADER_ARG("Compiler=0x%x, Layout=0x%x",
                  Compiler, Layout);

    gcmASSERT(Layout);

    switch(StorageQualifier)
    {
    case slvSTORAGE_QUALIFIER_UNIFORM:
        *defaultLayout = Compiler->context.uniformDefaultLayout;
        Compiler->context.uniformDefaultLayout = *Layout;
        status = sloCOMPILER_MergeLayoutId(Compiler,
                                           defaultLayout,
                                           &Compiler->context.uniformDefaultLayout);
        break;

    case slvSTORAGE_QUALIFIER_BUFFER:
        *defaultLayout = Compiler->context.bufferDefaultLayout;
        Compiler->context.bufferDefaultLayout = *Layout;
        status = sloCOMPILER_MergeLayoutId(Compiler,
                                           defaultLayout,
                                           &Compiler->context.bufferDefaultLayout);
        break;

    case slvSTORAGE_QUALIFIER_OUT:
    case slvSTORAGE_QUALIFIER_OUT_IO_BLOCK:
        *defaultLayout = Compiler->context.outDefaultLayout;
        Compiler->context.outDefaultLayout = *Layout;
        status = sloCOMPILER_MergeLayoutId(Compiler,
                                           defaultLayout,
                                           &Compiler->context.outDefaultLayout);

        status = sloCOMPILER_MergeExtLayoutId(Compiler,
                                              defaultLayout,
                                              &Compiler->context.outDefaultLayout);
        break;

    case slvSTORAGE_QUALIFIER_IN:
    case slvSTORAGE_QUALIFIER_IN_IO_BLOCK:
        *defaultLayout = Compiler->context.inDefaultLayout;
        *layout = *Layout;
        if(layout->id & sldLAYOUT_WORK_GROUP_SIZE_FIELDS)
        {
           if(!(layout->id & slvLAYOUT_WORK_GROUP_SIZE_X))
           {

              layout->id |= slvLAYOUT_WORK_GROUP_SIZE_X;
              layout->workGroupSize[0] = 1;
           }

           if(!(layout->id & slvLAYOUT_WORK_GROUP_SIZE_Y))
           {
              layout->id |= slvLAYOUT_WORK_GROUP_SIZE_Y;
              layout->workGroupSize[1] = 1;
           }

           if(!(layout->id & slvLAYOUT_WORK_GROUP_SIZE_Z))
           {
              layout->id |= slvLAYOUT_WORK_GROUP_SIZE_Z;
              layout->workGroupSize[2] = 1;
           }
        }

        Compiler->context.inDefaultLayout = *layout;
        status = sloCOMPILER_MergeLayoutId(Compiler,
                                           defaultLayout,
                                           &Compiler->context.inDefaultLayout);

        status = sloCOMPILER_MergeExtLayoutId(Compiler,
                                              defaultLayout,
                                              &Compiler->context.inDefaultLayout);
        break;

    default:
        gcmASSERT(0);
        status = gcvSTATUS_INVALID_DATA;
        break;
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_GetDefaultLayout(
    IN sloCOMPILER Compiler,
    IN slsLAYOUT_QUALIFIER *Layout,
    IN sltSTORAGE_QUALIFIER StorageQualifier
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x, Layout=0x%x",
                  Compiler, Layout);

    switch(StorageQualifier)
    {
    case slvSTORAGE_QUALIFIER_UNIFORM:
        *Layout = Compiler->context.uniformDefaultLayout;
        break;

    case slvSTORAGE_QUALIFIER_BUFFER:
        *Layout = Compiler->context.bufferDefaultLayout;
        break;

    case slvSTORAGE_QUALIFIER_OUT:
    case slvSTORAGE_QUALIFIER_OUT_IO_BLOCK:
        *Layout = Compiler->context.outDefaultLayout;
        break;

    case slvSTORAGE_QUALIFIER_IN:
    case slvSTORAGE_QUALIFIER_IN_IO_BLOCK:
        *Layout = Compiler->context.inDefaultLayout;
        break;

    default:
        gcmASSERT(0);
        status = gcvSTATUS_INVALID_DATA;
        break;
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_UpdateDefaultLayout(
    IN sloCOMPILER Compiler,
    IN slsLAYOUT_QUALIFIER Layout,
    IN sltSTORAGE_QUALIFIER StorageQualifier
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x, Layout=0x%x StorageQualifier=%d",
                  Compiler, Layout, StorageQualifier);

    switch(StorageQualifier)
    {
    case slvSTORAGE_QUALIFIER_UNIFORM:
        Compiler->context.uniformDefaultLayout = Layout;
        break;

    case slvSTORAGE_QUALIFIER_BUFFER:
        Compiler->context.bufferDefaultLayout = Layout;
        break;

    case slvSTORAGE_QUALIFIER_OUT:
        Compiler->context.outDefaultLayout = Layout;
        break;

    case slvSTORAGE_QUALIFIER_IN:
        Compiler->context.inDefaultLayout = Layout;
        break;

    default:
        gcmASSERT(0);
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        break;
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_SetInputLocationInUse(
    IN sloCOMPILER Compiler,
    IN gctINT Location,
    IN gctSIZE_T Length
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x, Location=0x%x Length=%lu",
                  Compiler, Location, Length);

    if ((Location + Length - 1) >= 32)
    {
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        return status;
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_SetOutputLocationInUse(
    IN sloCOMPILER Compiler,
    IN gctINT Location,
    IN gctSIZE_T Length
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctSIZE_T i;
    gctUINT mask;

    gcmHEADER_ARG("Compiler=0x%x, Location=0x%x Length=%lu",
                  Compiler, Location, Length);

    mask = 1 << Location;
    if ((Location + Length - 1) < 32)
    {
        for (i = 0; i < Length; i++)
        {
            if (Compiler->context.outputLocationSettings & mask)
            {
                break;
            }
            Compiler->context.outputLocationSettings |= mask;
            mask <<= 1;
        }
    }
    else
    {
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
sloCOMPILER_SetUniformLocationInUse(
    IN sloCOMPILER Compiler,
    IN gctINT Location,
    IN gctSIZE_T Length
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x, Location=0x%x Length=%lu",
                  Compiler, Location, Length);

    if ((Location + Length - 1) >= Compiler->context.uniformLocationMaxLength)
    {
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        return status;
    }

    gcmFOOTER();
    return status;
}

gctBOOL
sloCOMPILER_NeedCheckOutputLocationExist(
    IN sloCOMPILER Compiler
    )
{
    /* So far only OES requires that "If there is more than one fragment output, the location must be specified for all outputs." */
    return !sloCOMPILER_IsOGLVersion(Compiler);
}

gceSTATUS
sloCOMPILER_SetUnspecifiedOutputLocationExist(
    IN sloCOMPILER Compiler
    )
{
    gctBOOL isOK;
    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    isOK = !slsCOMPILER_HasUnspecifiedLocation(Compiler->context.compilerFlags) &&
           Compiler->context.outputLocationSettings == 0;

    if(isOK)
    {
       slsCOMPILER_SetUnspecifiedLocation(Compiler->context.compilerFlags);
       gcmFOOTER_NO();
       return gcvSTATUS_OK;
    }
    else
    {
       gcmFOOTER_NO();
       return gcvSTATUS_INVALID_DATA;
    }
}

gceSTATUS
sloCOMPILER_SetVecConstant(
    IN sloCOMPILER Compiler,
    IN slsNAME *ConstantVariable
    )
{
   gceSTATUS status = gcvSTATUS_OK;
   slsDLINK_LIST *constantList;
   gctINT8 componentCount;

   gcmHEADER_ARG("Compiler=0x%x ConstantVariable=0x%x", Compiler, ConstantVariable);
   gcmASSERT(ConstantVariable);

   componentCount = slmDATA_TYPE_vectorSize_GET(ConstantVariable->dataType);
   gcmASSERT(componentCount > 0);

   if (componentCount == 0)
   {
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
   }

   if (slsDATA_TYPE_IsVec(ConstantVariable->dataType))
   {
       constantList = &Compiler->context.vecConstants.typeFloat[componentCount - 1];
   }
   else if (slsDATA_TYPE_IsSignedIVec(ConstantVariable->dataType))
   {
       constantList = &Compiler->context.vecConstants.typeInt[componentCount - 1];
   }
   else if (slsDATA_TYPE_IsUnsignedIVec(ConstantVariable->dataType))
   {
       constantList = &Compiler->context.vecConstants.typeUInt[componentCount - 1];
   }
   else
   {
       gcmASSERT(slsDATA_TYPE_IsBVec(ConstantVariable->dataType));
       constantList = &Compiler->context.vecConstants.typeBool[componentCount - 1];
   }

   slsDLINK_LIST_InsertFirst(constantList, &ConstantVariable->node);

   gcmFOOTER();
   return status;
}

gceSTATUS
sloCOMPILER_GetVecConstant(
    IN sloCOMPILER Compiler,
    IN sloIR_CONSTANT Constant,
    OUT slsNAME **ConstantVariable
    )
{
   gceSTATUS status = gcvSTATUS_NOT_FOUND;
   slsDLINK_LIST *constantList;
   gctINT8 componentCount;
   slsNAME *constVar;
   gctUINT i;
   gctBOOL found;

   gcmHEADER_ARG("Compiler=0x%x Constant=0x%x", Compiler, Constant);
   gcmASSERT(ConstantVariable);

   componentCount = slmDATA_TYPE_vectorSize_GET(Constant->exprBase.dataType);
   gcmASSERT(componentCount > 0);

   if (componentCount == 0)
   {
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
   }

   if (slsDATA_TYPE_IsVec(Constant->exprBase.dataType))
   {
       constantList = &Compiler->context.vecConstants.typeFloat[componentCount - 1];
   }
   else if (slsDATA_TYPE_IsSignedIVec(Constant->exprBase.dataType))
   {
       constantList = &Compiler->context.vecConstants.typeInt[componentCount - 1];
   }
   else if (slsDATA_TYPE_IsUnsignedIVec(Constant->exprBase.dataType))
   {
       constantList = &Compiler->context.vecConstants.typeUInt[componentCount - 1];
   }
   else
   {
       gcmASSERT(slsDATA_TYPE_IsBVec(Constant->exprBase.dataType));
       constantList = &Compiler->context.vecConstants.typeBool[componentCount - 1];
   }

   FOR_EACH_DLINK_NODE(constantList, slsNAME, constVar)
   {
      found = gcvTRUE;
      for (i = 0; i < Constant->valueCount; i++)
      {
          if (Constant->values[i].intValue != constVar->u.variableInfo.constant->values[i].intValue)
          {
              found = gcvFALSE;
              break;
          }
      }
      if (found)
      {
         *ConstantVariable = constVar;
         gcmFOOTER_ARG("*ConstantVariable=0x%x", *ConstantVariable);
         return gcvSTATUS_OK;
      }
   }

   *ConstantVariable = gcvNULL;
   gcmFOOTER();
   return status;
}

gceSTATUS
sloCOMPILER_GetVecConstantLists(
    IN sloCOMPILER Compiler,
    IN sltELEMENT_TYPE ElementType,
    OUT slsDLINK_LIST **ConstantList
    )
{
   slsDLINK_LIST *constantList;

   gcmHEADER_ARG("Compiler=0x%x ElementType=%d", Compiler, ElementType);
   gcmASSERT(ConstantList);

   switch(ElementType)
   {
   case slvTYPE_FLOAT:
   case slvTYPE_DOUBLE:
       constantList = Compiler->context.vecConstants.typeFloat;
       break;

   case slvTYPE_INT:
       constantList = Compiler->context.vecConstants.typeInt;
       break;

   case slvTYPE_UINT:
       constantList = Compiler->context.vecConstants.typeUInt;
       break;

   case slvTYPE_BOOL:
       constantList = Compiler->context.vecConstants.typeBool;
       break;

   default:
       gcmASSERT(0);
       *ConstantList = gcvNULL;
       gcmFOOTER_NO();
       return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
   }

   *ConstantList = constantList;
   gcmFOOTER_ARG("*ConstantList=0x%x", *ConstantList);
   return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_GetSharedVariableList(
    IN sloCOMPILER Compiler,
    OUT slsSLINK_LIST **SharedVariableList
    )
{
   gcmHEADER_ARG("Compiler=0x%x", Compiler);
   gcmASSERT(SharedVariableList);

   *SharedVariableList = &Compiler->context.sharedVariables;
   gcmFOOTER_ARG("*SharedVariableList=0x%x", *SharedVariableList);
   return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_AddSharedVariable(
    IN sloCOMPILER Compiler,
    IN slsNAME *VariableName
    )
{
    gceSTATUS status;
    slsSHARED_VARIABLE *sharedVariable;
    gctPOINTER pointer;

    gcmHEADER_ARG("Compiler=0x%x VariableName=0x%x", Compiler, VariableName);

    /* Verify the arguments. */
    slmASSERT_OBJECT(Compiler, slvOBJ_COMPILER);

    status = sloCOMPILER_Allocate(Compiler,
                                  (gctSIZE_T)sizeof(slsSHARED_VARIABLE),
                                  (gctPOINTER *) &pointer);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OUT_OF_MEMORY;
    }

    sharedVariable = pointer;
    sharedVariable->name = VariableName;
    slsSLINK_LIST_InsertFirst(&Compiler->context.sharedVariables, &sharedVariable->node);
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_GetConstantVariableList(
    IN sloCOMPILER Compiler,
    OUT slsDLINK_LIST **ConstantVariableList
    )
{
   gcmHEADER_ARG("Compiler=0x%x", Compiler);
   gcmASSERT(ConstantVariableList);

   *ConstantVariableList = &Compiler->context.constantVariables;
   gcmFOOTER_ARG("*SharedVariableList=0x%x", *ConstantVariableList);
   return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_AddConstantVariable(
    IN sloCOMPILER Compiler,
    IN sloIR_CONSTANT Constant
    )
{
    gceSTATUS status;
    slsCONSTANT_VARIABLE *constantVar;
    gctPOINTER pointer;

    gcmHEADER_ARG("Compiler=0x%x Constant=0x%x", Compiler, Constant);

    /* Verify the arguments. */
    slmASSERT_OBJECT(Compiler, slvOBJ_COMPILER);

    status = sloCOMPILER_Allocate(Compiler,
                                  gcmSIZEOF(slsCONSTANT_VARIABLE),
                                  (gctPOINTER *)&pointer);
    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OUT_OF_MEMORY;
    }

    constantVar = (slsCONSTANT_VARIABLE*)pointer;
    constantVar->constantVar = Constant;
    slsDLINK_LIST_InsertLast(&Compiler->context.constantVariables, &constantVar->node);

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_DestroyConstantVariableList(
    IN sloCOMPILER Compiler
    )
{
    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    while (!slsDLINK_LIST_IsEmpty(&Compiler->context.constantVariables))
    {
        slsCONSTANT_VARIABLE *constantVar;
        slsDLINK_LIST_DetachFirst(&Compiler->context.constantVariables, slsCONSTANT_VARIABLE, &constantVar);
        gcmVERIFY_OK(sloCOMPILER_Free(Compiler, constantVar));
    }
    Compiler->context.constantBufferSize = 0;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_InitializeConstantBuffer(
    IN sloCOMPILER Compiler,
    INOUT gctCHAR* Buffer
    )
{
    slsCONSTANT_VARIABLE *constantVar;
    gctUINT bufferSize = Compiler->context.constantBufferSize;
    gctCHAR* buffer = Buffer;

    gcmHEADER_ARG("Compiler=0x%x, Buffer=0x%x", Compiler, Buffer);

    FOR_EACH_DLINK_NODE(&Compiler->context.constantVariables, slsCONSTANT_VARIABLE, constantVar)
    {
        gctUINT i;

        constantVar = constantVar;

        for (i = 0; i < constantVar->constantVar->valueCount; i++)
        {
            gcoOS_MemCopy(buffer, &constantVar->constantVar->values[i].floatValue, gcmSIZEOF(gctFLOAT));
            buffer += gcmSIZEOF(gctFLOAT);
        }

        bufferSize -= constantVar->constantVar->valueCount * gcmSIZEOF(gctFLOAT);
    }

    gcmASSERT(bufferSize == 0);

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
sloCOMPILER_SetLayout(
    IN sloCOMPILER Compiler
    )
{
    slsLAYOUT_QUALIFIER inLayout;
    slsLAYOUT_QUALIFIER outLayout;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    if (Compiler->shaderType == slvSHADER_TYPE_COMPUTE)
    {
        if(Compiler->context.inDefaultLayout.id & sldLAYOUT_WORK_GROUP_SIZE_FIELDS)
        {
            SetShaderWorkGroupSize(Compiler->binary, Compiler->context.inDefaultLayout.workGroupSize);
        }
    }
    else if (Compiler->shaderType == slvSHADER_TYPE_TCS)
    {
        gcmVERIFY_OK(sloCOMPILER_GetDefaultLayout(Compiler, &outLayout, slvSTORAGE_QUALIFIER_OUT));
        gcmVERIFY_OK(sloCOMPILER_GetDefaultLayout(Compiler, &inLayout, slvSTORAGE_QUALIFIER_IN));
        SetTcsShaderLayout(Compiler->binary,
                           (gctUINT)outLayout.verticesNumber,
                           inLayout.tcsInputVerticesUniform);
    }
    else if (Compiler->shaderType == slvSHADER_TYPE_TES)
    {
        gcmVERIFY_OK(sloCOMPILER_GetDefaultLayout(Compiler, &inLayout, slvSTORAGE_QUALIFIER_IN));
        SetTesShaderLayout(Compiler->binary,
                           inLayout.tesPrimitiveMode == slvTES_PRIMITIVE_MODE_NONE ? 0 : (gcTessPrimitiveMode)inLayout.tesPrimitiveMode,
                           inLayout.tesVertexSpacing == slvTES_VERTEX_SPACING_NONE ? slvTES_VERTEX_SPACING_DEFAULT : (gcTessVertexSpacing)inLayout.tesVertexSpacing,
                           inLayout.tesOrdering == slvTES_ORDERING_NONE ? slvTES_ORDERING_DEFAULT : (gcTessOrdering)inLayout.tesOrdering,
                           inLayout.tesPointMode == slvTES_POINT_MODE_NONE ? gcvFALSE : gcvTRUE,
                           0);
    }
    else if (Compiler->shaderType == slvSHADER_TYPE_GS)
    {
        gcmVERIFY_OK(sloCOMPILER_GetDefaultLayout(Compiler, &outLayout, slvSTORAGE_QUALIFIER_OUT));
        gcmVERIFY_OK(sloCOMPILER_GetDefaultLayout(Compiler, &inLayout, slvSTORAGE_QUALIFIER_IN));
        SetGeoShaderLayout(Compiler->binary,
                           inLayout.gsInvocationTime,
                           outLayout.maxGSVerticesNumber,
                           (gcGeoPrimitive)inLayout.gsPrimitive,
                           (gcGeoPrimitive)outLayout.gsPrimitive);
    }

    gcmFOOTER();
    return status;
}

gcePATCH_ID
sloCOMPILER_GetPatchID(
    IN sloCOMPILER Compiler
    )
{
    return Compiler->patchId;
}

