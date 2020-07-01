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


#include "gc_glsl_parser.h"
#include "gc_glsl_built_ins_def.h"

/* Basic Built-In Types */
static gctINT BasicBuiltInTypes[] =
{
    T_BOOL,
    T_INT,
    T_FLOAT,
    T_UINT,
    T_DOUBLE,

    T_BVEC2,
    T_BVEC3,
    T_BVEC4,

    T_IVEC2,
    T_IVEC3,
    T_IVEC4,

    T_UVEC2,
    T_UVEC3,
    T_UVEC4,

    T_VEC2,
    T_VEC3,
    T_VEC4,

    T_DVEC2,
    T_DVEC3,
    T_DVEC4,

    T_MAT2,
    T_MAT2X3,
    T_MAT2X4,

    T_MAT3,
    T_MAT3X2,
    T_MAT3X4,

    T_MAT4,
    T_MAT4X2,
    T_MAT4X3,

    T_DMAT2,
    T_DMAT2X3,
    T_DMAT2X4,

    T_DMAT3,
    T_DMAT3X2,
    T_DMAT3X4,

    T_DMAT4,
    T_DMAT4X2,
    T_DMAT4X3,

    T_SAMPLER2D,
    T_SAMPLERCUBE,
    T_SAMPLERCUBEARRAY,
    T_SAMPLER3D,
    T_SAMPLER1DARRAY,
    T_SAMPLER2DARRAY,
    T_SAMPLER1DARRAYSHADOW,
    T_SAMPLER2DARRAYSHADOW,

    T_SAMPLER2DSHADOW,
    T_SAMPLERCUBESHADOW,
    T_SAMPLERCUBEARRAYSHADOW,

    T_ISAMPLER2D,
    T_ISAMPLERCUBE,
    T_ISAMPLERCUBEARRAY,
    T_ISAMPLER3D,
    T_ISAMPLER2DARRAY,

    T_USAMPLER2D,
    T_USAMPLERCUBE,
    T_USAMPLERCUBEARRAY,
    T_USAMPLER3D,
    T_USAMPLER2DARRAY,
    T_SAMPLEREXTERNALOES,
    T_SAMPLER2DMS,
    T_ISAMPLER2DMS,
    T_USAMPLER2DMS,
    T_SAMPLER2DMSARRAY,
    T_ISAMPLER2DMSARRAY,
    T_USAMPLER2DMSARRAY,
    T_GEN_SAMPLER,
    T_GEN_ISAMPLER,
    T_GEN_USAMPLER,
    T_SAMPLERBUFFER,
    T_ISAMPLERBUFFER,
    T_USAMPLERBUFFER,

    T_SAMPLER1D,
    T_ISAMPLER1D,
    T_USAMPLER1D,
    T_SAMPLER1DSHADOW,
    T_SAMPLER2DRECT,
    T_ISAMPLER2DRECT,
    T_USAMPLER2DRECT,
    T_SAMPLER2DRECTSHADOW,
    T_ISAMPLER1DARRAY,
    T_USAMPLER1DARRAY,

    T_IMAGE2D,
    T_IIMAGE2D,
    T_UIMAGE2D,
    T_IMAGE2DARRAY,
    T_IIMAGE2DARRAY,
    T_UIMAGE2DARRAY,
    T_IMAGE3D,
    T_IIMAGE3D,
    T_UIMAGE3D,
    T_IMAGECUBE,
    T_IMAGECUBEARRAY,
    T_IIMAGECUBE,
    T_IIMAGECUBEARRAY,
    T_UIMAGECUBE,
    T_UIMAGECUBEARRAY,
    T_IMAGEBUFFER,
    T_IIMAGEBUFFER,
    T_UIMAGEBUFFER,

    T_ATOMIC_UINT,
    T_VOID,
};

static gctUINT  BasicBuiltInTypeCount   = sizeof(BasicBuiltInTypes) / sizeof(gctINT);

typedef struct _slsBASIC_BUILT_IN_TYPE_INFO
{
    gctINT              type;

    slsDATA_TYPE *      normalDataType;

    slsDATA_TYPE *      inDataType;

    slsDATA_TYPE *      anypDataType;

    slsDATA_TYPE *      anypInDataType;
}
slsBASIC_BUILT_IN_TYPE_INFO;

static gceSTATUS
_ConstructBasicBuiltInTypeInfos(
    IN sloCOMPILER Compiler,
    OUT slsBASIC_BUILT_IN_TYPE_INFO * * BasicBuiltInTypeInfos
   )
{
    gceSTATUS                       status;
    slsBASIC_BUILT_IN_TYPE_INFO *   basicBuiltInTypeInfos = gcvNULL;
    gctUINT                         i;

    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmVERIFY_ARGUMENT(BasicBuiltInTypeInfos);

    do
    {
        gctPOINTER pointer = gcvNULL;

        status = sloCOMPILER_Allocate(
                                    Compiler,
                                    (gctSIZE_T)sizeof(slsBASIC_BUILT_IN_TYPE_INFO)
                                    * BasicBuiltInTypeCount,
                                    &pointer);

        if (gcmIS_ERROR(status)) break;

        basicBuiltInTypeInfos = pointer;

        for (i = 0; i < BasicBuiltInTypeCount; i++)
        {
            basicBuiltInTypeInfos[i].type = BasicBuiltInTypes[i];

            status = sloCOMPILER_CreateDataType(
                                                Compiler,
                                                basicBuiltInTypeInfos[i].type,
                                                gcvNULL,
                                                &basicBuiltInTypeInfos[i].normalDataType);
            if (gcmIS_ERROR(status)) break;

            status = sloCOMPILER_CreateDataType(
                                                Compiler,
                                                basicBuiltInTypeInfos[i].type,
                                                gcvNULL,
                                                &basicBuiltInTypeInfos[i].inDataType);
            if (gcmIS_ERROR(status)) break;
            basicBuiltInTypeInfos[i].inDataType->qualifiers.storage = slvSTORAGE_QUALIFIER_IN;

            status = sloCOMPILER_CreateDataType(
                                                Compiler,
                                                basicBuiltInTypeInfos[i].type,
                                                gcvNULL,
                                                &basicBuiltInTypeInfos[i].anypDataType);
            if (gcmIS_ERROR(status)) break;
            basicBuiltInTypeInfos[i].anypDataType->qualifiers.precision = slvPRECISION_QUALIFIER_ANY;

            status = sloCOMPILER_CreateDataType(
                                                Compiler,
                                                basicBuiltInTypeInfos[i].type,
                                                gcvNULL,
                                                &basicBuiltInTypeInfos[i].anypInDataType);
            if (gcmIS_ERROR(status)) break;
            basicBuiltInTypeInfos[i].anypInDataType->qualifiers.storage = slvSTORAGE_QUALIFIER_IN;
            basicBuiltInTypeInfos[i].anypInDataType->qualifiers.precision = slvPRECISION_QUALIFIER_ANY;
        }

        if (gcmIS_ERROR(status)) break;

        *BasicBuiltInTypeInfos = basicBuiltInTypeInfos;

        gcmFOOTER_ARG("*BasicBuiltInTypeInfos=0x%x", *BasicBuiltInTypeInfos);
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    if (basicBuiltInTypeInfos != gcvNULL)
    {
        gcmVERIFY_OK(sloCOMPILER_Free(Compiler, basicBuiltInTypeInfos));
    }

    *BasicBuiltInTypeInfos = gcvNULL;

    gcmFOOTER();
    return status;
}

static gceSTATUS
_DestroyBasicBuiltInTypeInfos(
    IN sloCOMPILER Compiler,
    IN slsBASIC_BUILT_IN_TYPE_INFO * BasicBuiltInTypeInfos
    )
{
    gcmHEADER_ARG("Compiler=0x%x BasicBuiltInTypeInfos=0x%x",
                  Compiler, BasicBuiltInTypeInfos);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmVERIFY_ARGUMENT(BasicBuiltInTypeInfos);

    gcmVERIFY_OK(sloCOMPILER_Free(Compiler, BasicBuiltInTypeInfos));

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static slsBASIC_BUILT_IN_TYPE_INFO *
_GetBasicBuiltInTypeInfo(
    IN slsBASIC_BUILT_IN_TYPE_INFO * BasicBuiltInTypeInfos,
    IN gctINT Type
    )
{
    gctUINT i;

    for (i = 0; i < BasicBuiltInTypeCount; i++)
    {
        if (BasicBuiltInTypeInfos[i].type == Type)
        {
            return &BasicBuiltInTypeInfos[i];
        }
    }

    return gcvNULL;
}

/* Default Precision Declarations */
typedef struct _slsDEFAULT_PRECISION_DECL
{
    sltPRECISION_QUALIFIER        precision;

    sltELEMENT_TYPE     type;
}
slsDEFAULT_PRECISION_DECL;

static slsDEFAULT_PRECISION_DECL    VSDefaultPrecisionDecls[] =
{
    /* Default Precision Declarations */
    {slvPRECISION_QUALIFIER_MEDIUM,   slvTYPE_BOOL},
    {slvPRECISION_QUALIFIER_HIGH,     slvTYPE_INT},
    {slvPRECISION_QUALIFIER_HIGH,     slvTYPE_UINT},
    {slvPRECISION_QUALIFIER_HIGH,     slvTYPE_FLOAT},
    {slvPRECISION_QUALIFIER_LOW,      slvTYPE_SAMPLER2D},
    {slvPRECISION_QUALIFIER_LOW,      slvTYPE_SAMPLERCUBE},
    {slvPRECISION_QUALIFIER_LOW,      slvTYPE_SAMPLEREXTERNALOES},
    {slvPRECISION_QUALIFIER_HIGH,     slvTYPE_ATOMIC_UINT},
};

static gctUINT VSDefaultPrecisionDeclCount =
                    sizeof(VSDefaultPrecisionDecls) / sizeof(slsDEFAULT_PRECISION_DECL);

static slsDEFAULT_PRECISION_DECL    FSDefaultPrecisionDecls[] =
{
    /* Default Precision Declarations */
    {slvPRECISION_QUALIFIER_MEDIUM,   slvTYPE_BOOL},
    {slvPRECISION_QUALIFIER_MEDIUM,   slvTYPE_INT},
    {slvPRECISION_QUALIFIER_MEDIUM,   slvTYPE_UINT},
    {slvPRECISION_QUALIFIER_LOW,      slvTYPE_SAMPLER2D},
    {slvPRECISION_QUALIFIER_LOW,      slvTYPE_SAMPLERCUBE},
    {slvPRECISION_QUALIFIER_LOW,      slvTYPE_SAMPLEREXTERNALOES},
    {slvPRECISION_QUALIFIER_HIGH,     slvTYPE_ATOMIC_UINT},
};

static gctUINT FSDefaultPrecisionDeclCount =
                    sizeof(FSDefaultPrecisionDecls) / sizeof(slsDEFAULT_PRECISION_DECL);

static slsDEFAULT_PRECISION_DECL    LIBDefaultPrecisionDecls[] =
{
    /* Default Precision Declarations */
    {slvPRECISION_QUALIFIER_MEDIUM,   slvTYPE_BOOL},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_INT},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_UINT},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_FLOAT},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_SAMPLER2D},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_SAMPLERCUBE},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_SAMPLER3D},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_SAMPLER1DARRAY},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_SAMPLER2DARRAY},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_SAMPLERBUFFER},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_SAMPLER1DARRAYSHADOW},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_SAMPLER2DARRAYSHADOW},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_SAMPLER2DSHADOW},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_SAMPLERCUBESHADOW},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_SAMPLERCUBEARRAYSHADOW},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_SAMPLERCUBEARRAY},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_ISAMPLER2D},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_ISAMPLERCUBE},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_ISAMPLERCUBEARRAY},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_ISAMPLER3D},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_ISAMPLER2DARRAY},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_ISAMPLERBUFFER},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_USAMPLER2D},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_USAMPLERCUBE},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_USAMPLERCUBEARRAY},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_USAMPLER3D},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_USAMPLER2DARRAY},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_USAMPLERBUFFER},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_SAMPLER2DMS},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_ISAMPLER2DMS},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_USAMPLER2DMS},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_SAMPLER2DMSARRAY},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_ISAMPLER2DMSARRAY},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_USAMPLER2DMSARRAY},
    {slvPRECISION_QUALIFIER_ANY,      slvTYPE_SAMPLEREXTERNALOES},
    {slvPRECISION_QUALIFIER_HIGH,     slvTYPE_ATOMIC_UINT},
};

static gctUINT LIBDefaultPrecisionDeclCount =
                    sizeof(LIBDefaultPrecisionDecls) / sizeof(slsDEFAULT_PRECISION_DECL);

/* GLSL's default precisions are predeclared to be consistent with ES.
   Set the float default precision to highp to avoid being excuted under dual16.
   This is also because some CTS GL shaders do not declare the default float precision,
   though the GLSL spec (like GLSL1.30) requires this.
   */
static slsDEFAULT_PRECISION_DECL    GLVSDefaultPrecisionDecls[] =
{
    /* Default Precision Declarations */
    {slvPRECISION_QUALIFIER_MEDIUM,   slvTYPE_BOOL},
    {slvPRECISION_QUALIFIER_HIGH,     slvTYPE_INT},
    {slvPRECISION_QUALIFIER_HIGH,     slvTYPE_UINT},
    {slvPRECISION_QUALIFIER_HIGH,     slvTYPE_FLOAT},
    {slvPRECISION_QUALIFIER_LOW,      slvTYPE_SAMPLER2D},
    {slvPRECISION_QUALIFIER_LOW,      slvTYPE_SAMPLERCUBE},
};

static gctUINT GLVSDefaultPrecisionDeclCount =
                    sizeof(GLVSDefaultPrecisionDecls) / sizeof(slsDEFAULT_PRECISION_DECL);

static slsDEFAULT_PRECISION_DECL    GLFSDefaultPrecisionDecls[] =
{
    /* Default Precision Declarations */
    {slvPRECISION_QUALIFIER_MEDIUM,   slvTYPE_BOOL},
    {slvPRECISION_QUALIFIER_MEDIUM,   slvTYPE_INT},
    {slvPRECISION_QUALIFIER_MEDIUM,   slvTYPE_UINT},
    {slvPRECISION_QUALIFIER_HIGH,     slvTYPE_FLOAT},
    {slvPRECISION_QUALIFIER_LOW,      slvTYPE_SAMPLER2D},
    {slvPRECISION_QUALIFIER_LOW,      slvTYPE_SAMPLERCUBE},
};

static gctUINT GLFSDefaultPrecisionDeclCount =
                    sizeof(GLFSDefaultPrecisionDecls) / sizeof(slsDEFAULT_PRECISION_DECL);

static gceSTATUS
_LoadDefaultPrecisionDecls(
    IN sloCOMPILER Compiler,
    IN gctUINT DefaultPrecisionDeclCount,
    IN slsDEFAULT_PRECISION_DECL * DefaultPrecisionDecls
    )
{
    gceSTATUS status;
    gctUINT i;

    gcmHEADER_ARG("Compiler=0x%x "
                  "DefaultPrecisionDeclCount=%u DefaultPrecisionDecls=0x%x",
                  Compiler, DefaultPrecisionDeclCount, DefaultPrecisionDecls);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmVERIFY_ARGUMENT(DefaultPrecisionDeclCount > 0);
    gcmVERIFY_ARGUMENT(DefaultPrecisionDecls);

    for(i = 0; i < DefaultPrecisionDeclCount; i++)
    {
       status = sloCOMPILER_SetDefaultPrecision(Compiler,
                                                DefaultPrecisionDecls[i].type,
                                                DefaultPrecisionDecls[i].precision);

       if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    }
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

typedef struct
{
    gctCONST_STRING     symbol;
    gctINT              type;    /*lexical type*/
    sltPRECISION_QUALIFIER        precision;
    gctUINT             valueCount;
    sluCONSTANT_VALUE   value[sldMAX_VECTOR_COMPONENT];
    /* Only add this constant when extension is enabled. */
    gctUINT             extension;
} BuiltinConstInfo;

/* Built-In Constant */
static gceSTATUS
_AddBuiltInConstants(
    IN sloCOMPILER       Compiler,
    IN slsBASIC_BUILT_IN_TYPE_INFO * BasicBuiltInTypeInfos,
    IN BuiltinConstInfo *ConstantInfos,
    IN gctUINT           Length
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gctUINT             i;
    slsDATA_TYPE *      dataType;
    sloIR_CONSTANT      constant;
    sluCONSTANT_VALUE   value[sldMAX_VECTOR_COMPONENT];
    sltPOOL_STRING      variableSymbol;
    slsNAME *           variableName;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    for (i = 0; i < Length; i++)
    {
        gctUINT j;
        slsBASIC_BUILT_IN_TYPE_INFO *basicBuiltInTypeInfo;
        sloEXTENSION extension = {0};

        extension.extension1 = ConstantInfos[i].extension;
        /* If this variable is not enabled in this shader, then we don't need to load it. */
        if (!sloCOMPILER_ExtensionEnabled(Compiler, &extension))
        {
            continue;
        }

        /* find the basic builtin data type */
        basicBuiltInTypeInfo = _GetBasicBuiltInTypeInfo(BasicBuiltInTypeInfos,
                                                        ConstantInfos[i].type);

        gcmASSERT(basicBuiltInTypeInfo);

        if (basicBuiltInTypeInfo == gcvNULL)
        {
            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            break;
        }

        status = sloCOMPILER_CloneDataType(Compiler,
                                           slvSTORAGE_QUALIFIER_CONST,
                                           ConstantInfos[i].precision,
                                           basicBuiltInTypeInfo->normalDataType,
                                           &dataType);
        if (gcmIS_ERROR(status)) break;

        /* Create the constant */
        status = sloIR_CONSTANT_Construct(Compiler,
                                          0,
                                          0,
                                          dataType,
                                          &constant);

        if (gcmIS_ERROR(status)) break;

        /* Add the constant value */
        gcmASSERT(ConstantInfos[i].valueCount < sldMAX_VECTOR_COMPONENT);
        for (j = 0; j < ConstantInfos[i].valueCount; j++)
        {
            value[j].intValue = (gctINT32)ConstantInfos[i].value[j].intValue;
        }

        status = sloIR_CONSTANT_AddValues(Compiler,
                                          constant,
                                          ConstantInfos[i].valueCount,
                                          value);

        if (gcmIS_ERROR(status)) break;

        gcmVERIFY_OK(sloCOMPILER_AddExternalDecl(Compiler, &constant->exprBase.base));

        /* Create the variable name */
        status = sloCOMPILER_AllocatePoolString(Compiler,
                                                ConstantInfos[i].symbol,
                                                &variableSymbol);

        if (gcmIS_ERROR(status)) break;

        status = sloCOMPILER_CreateName(Compiler,
                                        0,
                                        0,
                                        slvVARIABLE_NAME,
                                        dataType,
                                        variableSymbol,
                                        extension,
                                        gcvFALSE,
                                        &variableName);

        if (gcmIS_ERROR(status)) break;

        variableName->u.variableInfo.constant = constant;
        variableName->u.variableInfo.constant->variable = variableName;
    }

    gcmFOOTER();
    return status;
}

/* Built-In Constant */
static gceSTATUS
_LoadBuiltInConstants(
    IN sloCOMPILER Compiler,
    IN slsBASIC_BUILT_IN_TYPE_INFO * BasicBuiltInTypeInfos
    )
{
    gceSTATUS           status;
    gceAPI              apiVersion;
    static BuiltinConstInfo    constantInfos[] =
    {
        {"gl_MaxVertexAttribs",                    T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, 0 },
        {"gl_MaxVertexUniformVectors",             T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, 0 },
        {"gl_MaxVertexOutputVectors",              T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, 0 },
        {"gl_MaxFragmentInputVectors",             T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, 0 },
        {"gl_MaxVertexTextureImageUnits",          T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, 0 },
        {"gl_MaxCombinedTextureImageUnits",        T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, 0 },
        {"gl_MaxTextureImageUnits",                T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, 0 },
        {"gl_MaxFragmentUniformVectors",           T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, 0 },
        {"gl_MaxDrawBuffers",                      T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, 0 },
        {"gl_MaxSamples",                          T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_SAMPLE_VARIABLES},
        {"gl_MinProgramTexelOffset",               T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_HALTI},
        {"gl_MaxProgramTexelOffset",               T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_HALTI},
        {"gl_MaxImageUnits",                       T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_ES_31},
        {"gl_MaxVertexImageUniforms",              T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_ES_31},
        {"gl_MaxFragmentImageUniforms",            T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_ES_31},
        {"gl_MaxComputeImageUniforms",             T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_ES_31},
        {"gl_MaxCombinedImageUniforms",            T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_ES_31},
        {"gl_MaxCombinedShaderOutputResources",    T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_ES_31},
        {"gl_MaxComputeWorkGroupCount",            T_IVEC3, slvPRECISION_QUALIFIER_HIGH, 3, {{0}}, slvEXTENSION1_ES_31},
        {"gl_MaxComputeWorkGroupSize",             T_IVEC3, slvPRECISION_QUALIFIER_HIGH, 3, {{0}}, slvEXTENSION1_ES_31},
        {"gl_MaxComputeUniformComponents",         T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_ES_31},
        {"gl_MaxComputeTextureImageUnits",         T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_ES_31},
        {"gl_MaxComputeAtomicCounters",            T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_ES_31},
        {"gl_MaxComputeAtomicCounterBuffers",      T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_ES_31},
        {"gl_MaxVertexAtomicCounters",             T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_ES_31},
        {"gl_MaxFragmentAtomicCounters",           T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_ES_31},
        {"gl_MaxCombinedAtomicCounters",           T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_ES_31},
        {"gl_MaxAtomicCounterBindings",            T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_ES_31},
        {"gl_MaxVertexAtomicCounterBuffers",       T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_ES_31},
        {"gl_MaxFragmentAtomicCounterBuffers",     T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_ES_31},
        {"gl_MaxCombinedAtomicCounterBuffers",     T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_ES_31},
        {"gl_MaxAtomicCounterBufferSize",          T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_ES_31},
        {"gl_MaxVaryingVectors",                   T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, 0},
        /* TS EXT constants */
        {"gl_MaxTessControlInputComponents",       T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_TESSELLATION_SHADER},
        {"gl_MaxTessControlOutputComponents",      T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_TESSELLATION_SHADER},
        {"gl_MaxTessControlTextureImageUnits",     T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_TESSELLATION_SHADER},
        {"gl_MaxTessControlImageUniforms",         T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_TESSELLATION_SHADER},
        {"gl_MaxTessControlUniformComponents",     T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_TESSELLATION_SHADER},
        {"gl_MaxTessControlTotalOutputComponents", T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_TESSELLATION_SHADER},
        {"gl_MaxTessControlAtomicCounters",        T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_TESSELLATION_SHADER},
        {"gl_MaxTessControlAtomicCounterBuffers",  T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_TESSELLATION_SHADER},
        {"gl_MaxTessEvaluationInputComponents",    T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_TESSELLATION_SHADER},
        {"gl_MaxTessEvaluationOutputComponents",   T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_TESSELLATION_SHADER},
        {"gl_MaxTessEvaluationTextureImageUnits",  T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_TESSELLATION_SHADER},
        {"gl_MaxTessEvaluationImageUniforms",      T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_TESSELLATION_SHADER},
        {"gl_MaxTessEvaluationUniformComponents",  T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_TESSELLATION_SHADER},
        {"gl_MaxTessEvaluationAtomicCounters",     T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_TESSELLATION_SHADER},
        {"gl_MaxTessEvaluationAtomicCounterBuffers", T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_TESSELLATION_SHADER},
        {"gl_MaxTessPatchComponents",              T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_TESSELLATION_SHADER},
        {"gl_MaxPatchVertices",                    T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_TESSELLATION_SHADER},
        {"gl_MaxTessGenLevel",                     T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_TESSELLATION_SHADER},
        /* GS EXT constants */
        {"gl_MaxGeometryInputComponents",          T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_EXT_GEOMETRY_SHADER},
        {"gl_MaxGeometryOutputComponents",         T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_EXT_GEOMETRY_SHADER},
        {"gl_MaxGeometryImageUniforms",            T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_EXT_GEOMETRY_SHADER},
        {"gl_MaxGeometryTextureImageUnits",        T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_EXT_GEOMETRY_SHADER},
        {"gl_MaxGeometryOutputVertices",           T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_EXT_GEOMETRY_SHADER},
        {"gl_MaxGeometryTotalOutputComponents",    T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_EXT_GEOMETRY_SHADER},
        {"gl_MaxGeometryUniformComponents",        T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_EXT_GEOMETRY_SHADER},
        {"gl_MaxGeometryAtomicCounters",           T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_EXT_GEOMETRY_SHADER},
        {"gl_MaxGeometryAtomicCounterBuffers",     T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_EXT_GEOMETRY_SHADER},
        /* Desktop GL constants */
        {"gl_MaxClipDistances",                    T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, 0 },
        {"gl_MaxClipPlanes",                       T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, 0 },
        {"gl_MaxFragmentUniformComponents",        T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, 0 },
        {"gl_MaxTextureCoords",                    T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, 0 },
        {"gl_MaxTextureUnits",                     T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, 0 },
        {"gl_MaxVaryingComponents",                T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, 0 },
        {"gl_MaxVaryingFloats",                    T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, 0 },
        {"gl_MaxVertexUniformComponents",          T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, 0 },
        {"gl_MaxFragmentInputComponents",          T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, 0 },
        {"gl_MaxVertexOutputComponents",           T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, 0 },
        {"gl_MaxGeometryVaryingComponents",        T_INT, slvPRECISION_QUALIFIER_MEDIUM, 1, {{0}}, slvEXTENSION1_EXT_GEOMETRY_SHADER},

    };

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

#if !GC_INIT_BUILTIN_CONSTANTS_BY_DRIVER
    /* initialize the builtin constant if it is not done by driver */
    gcmVERIFY_OK(gcInitGLSLCaps(gcGetGLSLCaps()));
#endif

    apiVersion = sloCOMPILER_GetClientApiVersion(Compiler);

    constantInfos[gcBIConst_MaxVertexAttribs].value[0].intValue = GetGLMaxVertexAttribs();
    constantInfos[gcBIConst_MaxVertexUniformVectors].value[0].intValue = GetGLMaxVertexUniformVectors();
    constantInfos[gcBIConst_MaxVertexOutputVectors].value[0].intValue = GetGLMaxVertexOutputVectors();

    constantInfos[gcBIConst_MaxFragmentInputVectors].value[0].intValue = GetGLMaxFragmentInputVectors();

    constantInfos[gcBIConst_MaxVertexTextureImageUnits].value[0].intValue = GetGLMaxVertexTextureImageUnits();
    constantInfos[gcBIConst_MaxCombinedTextureImageUnits].value[0].intValue = GetGLMaxCombinedTextureImageUnits();
    constantInfos[gcBIConst_MaxTextureImageUnits].value[0].intValue = GetGLMaxFragTextureImageUnits();

    constantInfos[gcBIConst_MaxFragmentUniformVectors].value[0].intValue = GetGLMaxFragmentUniformVectors();

    constantInfos[gcBIConst_MaxDrawBuffers].value[0].intValue = GetGLMaxDrawBuffers();

    if (apiVersion == gcvAPI_OPENGL_ES20)
    {
        constantInfos[gcBIConst_MaxDrawBuffers].value[0].intValue = 1;
    }
    else if (apiVersion == gcvAPI_OPENGL_ES30 || apiVersion == gcvAPI_OPENGL_ES31)
    {
        constantInfos[gcBIConst_MaxDrawBuffers].value[0].intValue = (constantInfos[gcBIConst_MaxDrawBuffers].value[0].intValue > 4)
            ? constantInfos[gcBIConst_MaxDrawBuffers].value[0].intValue : 4;
    } else if (apiVersion == gcvAPI_OPENGL)
    {
        constantInfos[gcBIConst_MaxDrawBuffers].value[0].intValue = (constantInfos[gcBIConst_MaxDrawBuffers].value[0].intValue > 8)
            ? constantInfos[gcBIConst_MaxDrawBuffers].value[0].intValue : 8;
    }

    constantInfos[gcBIConst_MaxSamples].value[0].intValue = GetGLMaxSamples();
    constantInfos[gcBIConst_MinProgramTexelOffset].value[0].intValue = GetGLMinProgramTexelOffset();
    constantInfos[gcBIConst_MaxProgramTexelOffset].value[0].intValue = GetGLMaxProgramTexelOffset();

    constantInfos[gcBIConst_MaxImageUnits].value[0].intValue = GetGLMaxImageUnits();
    constantInfos[gcBIConst_MaxVertexImageUniforms].value[0].intValue = GetGLMaxVertexImageUniforms();
    constantInfos[gcBIConst_MaxFragmentImageUniforms].value[0].intValue = GetGLMaxFragmentImageUniforms();
    constantInfos[gcBIConst_MaxComputeImageUniforms].value[0].intValue = GetGLMaxComputeImageUniforms();
    constantInfos[gcBIConst_MaxCombinedImageUniforms].value[0].intValue = GetGLMaxCombinedImageUniforms();

    constantInfos[gcBIConst_MaxCombinedShaderOutputResources].value[0].intValue = GetGLMaxCombinedShaderOutputResources();

    gcmASSERT(constantInfos[gcBIConst_MaxComputeWorkGroupCount].valueCount == 3);
    constantInfos[gcBIConst_MaxComputeWorkGroupCount].value[0].intValue = GetGLMaxComputeWorkGroupCount(0);
    constantInfos[gcBIConst_MaxComputeWorkGroupCount].value[1].intValue = GetGLMaxComputeWorkGroupCount(1);
    constantInfos[gcBIConst_MaxComputeWorkGroupCount].value[2].intValue = GetGLMaxComputeWorkGroupCount(2);

    gcmASSERT(constantInfos[gcBIConst_MaxComputeWorkGroupSize].valueCount == 3);
    constantInfos[gcBIConst_MaxComputeWorkGroupSize].value[0].intValue = GetGLMaxComputeWorkGroupSize(0);
    constantInfos[gcBIConst_MaxComputeWorkGroupSize].value[1].intValue = GetGLMaxComputeWorkGroupSize(1);
    constantInfos[gcBIConst_MaxComputeWorkGroupSize].value[2].intValue = GetGLMaxComputeWorkGroupSize(2);

    constantInfos[gcBIConst_MaxComputeUniformComponents].value[0].intValue = GetGLMaxComputeUniformComponents();
    constantInfos[gcBIConst_MaxComputeTextureImageUnits].value[0].intValue = GetGLMaxComputeTextureImageUnits();
    constantInfos[gcBIConst_MaxComputeAtomicCounters].value[0].intValue = GetGLMaxComputeAtomicCounters();

    constantInfos[gcBIConst_MaxComputeAtomicCounterBuffers].value[0].intValue = GetGLMaxComputeAtomicCounterBuffers();
    constantInfos[gcBIConst_MaxVertexAtomicCounters].value[0].intValue = GetGLMaxVertexAtomicCounters();
    constantInfos[gcBIConst_MaxFragmentAtomicCounters].value[0].intValue = GetGLMaxFragmentAtomicCounters();
    constantInfos[gcBIConst_MaxCombinedAtomicCounters].value[0].intValue = GetGLMaxCombinedAtomicCounters();
    constantInfos[gcBIConst_MaxAtomicCounterBindings].value[0].intValue = GetGLMaxAtomicCounterBindings();

    constantInfos[gcBIConst_MaxVertexAtomicCounterBuffers].value[0].intValue = GetGLMaxVertexAtomicCounterBuffers();
    constantInfos[gcBIConst_MaxFragmentAtomicCounterBuffers].value[0].intValue = GetGLMaxFragmentAtomicCounterBuffers();
    constantInfos[gcBIConst_MaxCombinedAtomicCounterBuffers].value[0].intValue = GetGLMaxCombinedAtomicCounterBuffers();

    constantInfos[gcBIConst_MaxAtomicCounterBufferSize].value[0].intValue = (gctINT32)GetGLMaxAtomicCounterBufferSize();

    constantInfos[gcBIConst_MaxVaryingVectors].value[0].intValue = GetGLMaxVaryingVectors();

    constantInfos[gcBIConst_MaxTessControlInputComponents].value[0].intValue = GetGLMaxTCSInputVectors() * 4;
    constantInfos[gcBIConst_MaxTessControlOutputComponents].value[0].intValue = GetGLMaxTCSOutputVectors() * 4;
    constantInfos[gcBIConst_MaxTessControlTextureImageUnits].value[0].intValue = GetGLMaxTCSTextureImageUnits();
    constantInfos[gcBIConst_MaxTessControlImageUniforms].value[0].intValue = GetGLMaxTCSImageUniforms();
    constantInfos[gcBIConst_MaxTessControlUniformComponents].value[0].intValue = GetGLMaxTCSUniformVectors() * 4;
    constantInfos[gcBIConst_MaxTessControlTotalOutputComponents].value[0].intValue = GetGLMaxTCSOutTotalVectors() * 4;
    constantInfos[gcBIConst_MaxTessControlAtomicCounters].value[0].intValue = GetGLMaxTCSAtomicCounters();
    constantInfos[gcBIConst_MaxTessControlAtomicCounterBuffers].value[0].intValue = GetGLMaxTCSAtomicCounterBuffers();

    constantInfos[gcBIConst_MaxTessEvaluationInputComponents].value[0].intValue = GetGLMaxTESInputVectors() * 4;
    constantInfos[gcBIConst_MaxTessEvaluationOutputComponents].value[0].intValue = GetGLMaxTESOutputVectors() * 4;
    constantInfos[gcBIConst_MaxTessEvaluationTextureImageUnits].value[0].intValue = GetGLMaxTESTextureImageUnits();
    constantInfos[gcBIConst_MaxTessEvaluationImageUniforms].value[0].intValue = GetGLMaxTESImageUniforms();
    constantInfos[gcBIConst_MaxTessEvaluationUniformComponents].value[0].intValue = GetGLMaxTESUniformVectors() * 4;
    constantInfos[gcBIConst_MaxTessEvaluationAtomicCounters].value[0].intValue = GetGLMaxTESAtomicCounters();
    constantInfos[gcBIConst_MaxTessEvaluationAtomicCounterBuffers].value[0].intValue = GetGLMaxTESAtomicCounterBuffers();
    constantInfos[gcBIConst_MaxTessPatchComponents].value[0].intValue = GetGLMaxTessPatchVectors() * 4;
    constantInfos[gcBIConst_MaxPatchVertices].value[0].intValue = GetGLMaxTessPatchVertices();
    constantInfos[gcBIConst_MaxTessGenLevel].value[0].intValue = GetGLMaxTessGenLevel();

    constantInfos[gcBIConst_MaxGSInputComponents].value[0].intValue = GetGLMaxGSInVectors() * 4;
    constantInfos[gcBIConst_MaxGSOutputComponents].value[0].intValue = GetGLMaxGSOutVectors() * 4;
    constantInfos[gcBIConst_MaxGSImageUniforms].value[0].intValue = GetGLMaxGSImageUniforms();
    constantInfos[gcBIConst_MaxGSTextureImageUnits].value[0].intValue = GetGLMaxGSTextureImageUnits();
    constantInfos[gcBIConst_MaxGSOutputVertices].value[0].intValue = GetGLMaxGSOutVertices();
    constantInfos[gcBIConst_MaxGSTotalOutputComponents].value[0].intValue = GetGLMaxGSOutTotalVectors() * 4;
    constantInfos[gcBIConst_MaxGSUniformComponents].value[0].intValue = GetGLMaxGSUniformVectors() * 4;
    constantInfos[gcBIConst_MaxGSAtomicCounters].value[0].intValue = GetGLMaxGSAtomicCounters();
    constantInfos[gcBIConst_MaxGSAtomicCounterBuffers].value[0].intValue = GetGLMaxGSAtomicCounterBuffers();
    if (apiVersion == gcvAPI_OPENGL)
    {
        constantInfos[gcBIConst_MaxClipDistances].value[0].intValue = GetGLMaxClipDistances();
        constantInfos[gcBIConst_MaxClipPlanes].value[0].intValue = GetGLMaxClipPlanes();
        constantInfos[gcBIConst_MaxFragmentUniformComponents].value[0].intValue = GetGLMaxFragmentUniformComponents();
        constantInfos[gcBIConst_MaxTextureCoords].value[0].intValue = GetGLMaxTextureCoords();
        constantInfos[gcBIConst_MaxTextureUnits].value[0].intValue = GetGLMaxTextureUnits();
        constantInfos[gcBIConst_MaxVaryingComponents].value[0].intValue = GetGLMaxVaryingComponents();
        constantInfos[gcBIConst_MaxVaryingFloats].value[0].intValue = GetGLMaxVaryingFloats();
        constantInfos[gcBIConst_MaxVertexUniformComponents].value[0].intValue = GetGLMaxVertexUniformComponents();
        constantInfos[gcBIConst_MaxFragmentInputComponents].value[0].intValue = GetGLMaxFragmentInputComponents();
        constantInfos[gcBIConst_MaxVertexOutputComponents].value[0].intValue = GetGLMaxVertexOutputComponents();
        constantInfos[gcBIConst_MaxGSVaryingComponents].value[0].intValue = GetGLMaxGSVaryingComponents();

        status = _AddBuiltInConstants(Compiler,
                                      BasicBuiltInTypeInfos,
                                      constantInfos,
                                      sizeof(constantInfos)/sizeof(BuiltinConstInfo));
    }
    else
    {
        status = _AddBuiltInConstants(Compiler,
                                      BasicBuiltInTypeInfos,
                                      constantInfos,
                                      sizeof(constantInfos)/sizeof(BuiltinConstInfo) - 11);
    }

    gcmFOOTER();
    return status;
}

/* Built-In Uniform State */
static gceSTATUS
_LoadBuiltInUniformState(
    IN sloCOMPILER Compiler,
    IN slsBASIC_BUILT_IN_TYPE_INFO * BasicBuiltInTypeInfos
    )
{
    gceSTATUS               status;
    gctUINT                 i;
    slsNAME_SPACE *         fieldNameSpace;
    slsDATA_TYPE *          fieldDataType;
    const gctCONST_STRING   fields[3] = {"near", "far", "diff"};
    sltPOOL_STRING          fieldSymbol;
    slsDATA_TYPE *          structDataType;
    sltPOOL_STRING          structSymbol;
    sltPOOL_STRING          variableSymbol;
    sloEXTENSION            extension = {0};

    gcmHEADER_ARG("Compiler=0x%x BasicBuiltInTypeInfos=0x%x",
                  Compiler, BasicBuiltInTypeInfos);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmVERIFY_ARGUMENT(BasicBuiltInTypeInfos);

    extension.extension1 = slvEXTENSION1_NONE;
    do
    {
        /* Create the struct field: near, far, diff */
        status = sloCOMPILER_CreateNameSpace(Compiler,
                                             gcvNULL,
                                             slvNAME_SPACE_TYPE_STRUCT,
                                             &fieldNameSpace);

        if (gcmIS_ERROR(status)) break;

        status = sloCOMPILER_CreateDataType(Compiler,
                                            T_FLOAT,
                                            gcvNULL,
                                            &fieldDataType);

        if (gcmIS_ERROR(status)) break;

        fieldDataType->qualifiers.precision = slvPRECISION_QUALIFIER_HIGH;
        for (i = 0; i < 3; i++)
        {
            status = sloCOMPILER_AllocatePoolString(Compiler,
                                                    fields[i],
                                                    &fieldSymbol);

            if (gcmIS_ERROR(status)) break;

            status = sloCOMPILER_CreateName(Compiler,
                                            0,
                                            0,
                                            slvFIELD_NAME,
                                            fieldDataType,
                                            fieldSymbol,
                                            extension,
                                            gcvFALSE,
                                            gcvNULL);

            if (gcmIS_ERROR(status)) break;
        }

        if (gcmIS_ERROR(status)) break;

        sloCOMPILER_PopCurrentNameSpace(Compiler, gcvNULL);

        /* Create the struct type: gl_DepthRangeParameters */
        status = sloCOMPILER_CreateDataType(Compiler,
                                            T_STRUCT,
                                            fieldNameSpace,
                                            &structDataType);

        /* Set it to correct qualifier */
        if (gcmIS_ERROR(status)) break;

        structDataType->qualifiers.storage = slvSTORAGE_QUALIFIER_UNIFORM;

        status = sloCOMPILER_AllocatePoolString(Compiler,
                                                "#DepthRangeParameters",
                                                &structSymbol);

        if (gcmIS_ERROR(status)) break;

        slsNAME_SPACE_SetSpaceName(fieldNameSpace, structSymbol);

        status = sloCOMPILER_CreateName(Compiler,
                                        0,
                                        0,
                                        slvSTRUCT_NAME,
                                        structDataType,
                                        structSymbol,
                                        extension,
                                        gcvFALSE,
                                        gcvNULL);

        if (gcmIS_ERROR(status)) break;

        /* Create the uniform variable: gl_DepthRange */
        status = sloCOMPILER_AllocatePoolString(Compiler,
                                                "gl_DepthRange",
                                                &variableSymbol);

        if (gcmIS_ERROR(status)) break;

        status = sloCOMPILER_CreateName(Compiler,
                                        0,
                                        0,
                                        slvVARIABLE_NAME,
                                        structDataType,
                                        variableSymbol,
                                        extension,
                                        gcvFALSE,
                                        gcvNULL);

        if (gcmIS_ERROR(status)) break;

        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    gcmFOOTER();
    return status;
}

gceSTATUS
updateForFragData(
    IN sloCOMPILER Compiler,
    OUT slsDATA_TYPE** DataType
    )
{
    gctUINT arrayLength = GetGLMaxDrawBuffers();

    if (Compiler->clientApiVersion != gcvAPI_OPENGL)
        arrayLength = (arrayLength > 4) ? 4 : arrayLength;

    /* Spec says: although gl_FragData is declared as an array in GLSL ES 1.00,
                    mrt are not supported in OpenGL ES 2.0 and are therefore not
                    available when using GLSL ES 1.00 in OpenGL ES 3.0. That means
                    true array size of gl_FragData non-es30 or above shader always
                    is 1. */
    if (!sloCOMPILER_IsHaltiVersion(Compiler))
    {
        arrayLength = 1;
    }

    (*DataType)->arrayLength = arrayLength;
    (*DataType)->arrayLengthList[0] = arrayLength;

    return gcvSTATUS_OK;
}

gceSTATUS
updateForTexCoord(
    IN sloCOMPILER Compiler,
    OUT slsDATA_TYPE** DataType
    )
{
    gctUINT arrayLength = GetGLMaxDrawBuffers();

    (*DataType)->arrayLength = arrayLength;
    (*DataType)->arrayLengthList[0] = arrayLength;

    return gcvSTATUS_OK;
}

gceSTATUS
updateForSampleMask(
    IN sloCOMPILER Compiler,
    OUT slsDATA_TYPE** DataType
    )
{
    gctUINT arrayLength = (GetGLMaxSamples() + 31) / 32;

    (*DataType)->arrayLength = arrayLength;
    (*DataType)->arrayLengthList[0] = arrayLength;

    return gcvSTATUS_OK;
}

static gceSTATUS
_LoadBuiltInVariablesForIOBlock(
    IN sloCOMPILER Compiler,
    IN slsBUILT_IN_VARIABLE  Variable
    )
{
    gceSTATUS                       status = gcvSTATUS_OK;
    slsNAME_SPACE *                 blockNameSpace;
    gctUINT                         i;
    slsNAME *                       field, *blockName, *instanceName, *memberName;
    slsBUILT_IN_VARIABLE *          fieldVar = Variable.fieldVariables;
    sltPOOL_STRING                  symbol;
    gctPOINTER                      pointer = gcvNULL;
    slsFieldDecl *                  fieldDecl;
    slsDATA_TYPE *                  blockDataType, *instanceDataType, *fieldDataType;
    slsLAYOUT_QUALIFIER             defaultLayout[1];
    sltSTORAGE_QUALIFIER            qualifier;
    gctBOOL                         addMember;
    slsINTERFACE_BLOCK_MEMBER *     blockMember;
    sloEXTENSION                    extension = {0};

    extension.extension1 = slvEXTENSION1_NONE;

    gcmONERROR(sloCOMPILER_CreateAuxGlobalNameSpace(Compiler,
                                                    gcvNULL,
                                                    slvNAME_SPACE_TYPE_IO_BLOCK,
                                                    &blockNameSpace));

    /* Create the filed name. */
    for (i = 0; i < Variable.fieldCount; i++)
    {
        gcmONERROR(sloCOMPILER_AllocatePoolString(Compiler,
                                                  fieldVar[i].symbol,
                                                  &symbol));

        gcmONERROR(sloCOMPILER_CreateDataType(Compiler,
                                              fieldVar[i].type,
                                              gcvNULL,
                                              &fieldDataType));

        gcmONERROR(sloCOMPILER_CreateName(Compiler,
                                          0,
                                          0,
                                          slvFIELD_NAME,
                                          fieldDataType,
                                          symbol,
                                          extension,
                                          gcvFALSE,
                                          &field));

        gcmONERROR(sloCOMPILER_Allocate(Compiler,
                                        (gctSIZE_T)sizeof(slsFieldDecl),
                                        &pointer));

        gcoOS_ZeroMemory(pointer, (gctSIZE_T)sizeof(slsFieldDecl));
        fieldDecl = pointer;

        fieldDecl->field    = field;
    }

    sloCOMPILER_PopCurrentNameSpace(Compiler, &blockNameSpace);

    /* Create the IO block name. */
    gcmONERROR(sloCOMPILER_CreateDataType(Compiler,
                                          T_IO_BLOCK,
                                          blockNameSpace,
                                          &blockDataType));
    blockDataType->qualifiers.storage = Variable.qualifier;

    gcmONERROR(sloCOMPILER_GetDefaultLayout(Compiler,
                                            defaultLayout,
                                            (Variable.qualifier == slvSTORAGE_QUALIFIER_IN_IO_BLOCK) ?
                                                slvSTORAGE_QUALIFIER_IN : slvSTORAGE_QUALIFIER_OUT));

    gcmONERROR(sloCOMPILER_MergeLayoutId(Compiler,
                                         defaultLayout,
                                         &blockDataType->qualifiers.layout));

    gcmONERROR(sloCOMPILER_AllocatePoolString(Compiler,
                                              Variable.blockSymbol,
                                              &symbol));

    status = slsNAME_SPACE_Search(Compiler,
                                  sloCOMPILER_GetCurrentSpace(Compiler),
                                  symbol,
                                  slsNAME_SPACE_CheckBlockNameForTheSameInterface,
                                  blockDataType,
                                  gcvFALSE,
                                  gcvFALSE,
                                  &blockName);

    /* There can be two blocks with the same name for a shader interface. */
    if (status == gcvSTATUS_OK)
    {
        gcmASSERT(gcvFALSE);
    }

    gcmONERROR(sloCOMPILER_CreateName(Compiler,
                                      0,
                                      0,
                                      slvINTERFACE_BLOCK_NAME,
                                      blockDataType,
                                      symbol,
                                      extension,
                                      gcvFALSE,
                                      &blockName));

    /* Create the instance name if needed. */
    if (Variable.symbol != gcvNULL)
    {
        gcmONERROR(slsDATA_TYPE_Clone(Compiler,
                                      Variable.qualifier,
                                      blockName->dataType->qualifiers.precision,
                                      blockName->dataType,
                                      &instanceDataType));

        gcmONERROR(sloCOMPILER_CreateArrayDataType(Compiler,
                                                   instanceDataType,
                                                   Variable.arrayLength,
                                                   &instanceDataType));

        gcmONERROR(sloCOMPILER_AllocatePoolString(Compiler,
                                                  Variable.symbol,
                                                  &symbol));

        gcmONERROR(sloCOMPILER_CreateName(Compiler,
                                          0,
                                          0,
                                          slvVARIABLE_NAME,
                                          instanceDataType,
                                          symbol,
                                          extension,
                                          gcvFALSE,
                                          &instanceName));

        instanceName->u.variableInfo.interfaceBlock = blockName;
    }

    if (Variable.qualifier == slvSTORAGE_QUALIFIER_IN_IO_BLOCK)
    {
        qualifier = slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER;
    }
    else
    {
        qualifier = slvSTORAGE_QUALIFIER_OUT_IO_BLOCK_MEMBER;
    }

    /* Create ALL IO block members. */
    addMember = slsDLINK_LIST_IsEmpty(&blockName->u.interfaceBlockContent.members);
    FOR_EACH_DLINK_NODE(&(blockName->dataType->fieldSpace->names), slsNAME, field)
    {
        memberName = field;
        memberName->dataType->qualifiers.storage = qualifier;

        if (addMember)
        {
            gcmONERROR(sloCOMPILER_Allocate(Compiler,
                                            (gctSIZE_T)sizeof(slsINTERFACE_BLOCK_MEMBER),
                                            (gctPOINTER *) &pointer));

            blockMember = pointer;
            blockMember->name = memberName;
            blockMember->isActive = gcvFALSE;
            if(qualifier == slvSTORAGE_QUALIFIER_UNIFORM_BLOCK_MEMBER)
            {
                blockMember->isActive = gcvFALSE;
            }
            else
            {
                blockMember->isActive = gcvTRUE;
            }

            slsDLINK_LIST_InsertLast(&blockName->u.interfaceBlockContent.members, &blockMember->node);
        }
    }

    if (Variable.symbol == gcvNULL)
    {
        blockName->dataType->orgFieldSpace = gcvNULL;
        blockName->dataType->fieldSpace = gcvNULL;
    }

OnError:
    return status;
}

static gceSTATUS
_LoadBuiltInVariables(
    IN sloCOMPILER Compiler,
    IN slsBASIC_BUILT_IN_TYPE_INFO * BasicBuiltInTypeInfos,
    IN gctUINT BuiltInVariableCount,
    IN slsBUILT_IN_VARIABLE * BuiltInVariables
    )
{
    gceSTATUS                       status = gcvSTATUS_OK;
    gctUINT                         i, arrayLength;
    slsBASIC_BUILT_IN_TYPE_INFO *   basicBuiltInTypeInfo;
    slsDATA_TYPE *                  dataType;
    sltPOOL_STRING                  symbolInPool;
    sloEXTENSION                    extension = {0};

    gcmHEADER_ARG("Compiler=0x%x BasicBuiltInTypeInfos=0x%x "
                  "BuiltInVariableCount=%u BuiltInVariables=0x%x",
                  Compiler, BasicBuiltInTypeInfos, BuiltInVariableCount,
                  BuiltInVariables);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmVERIFY_ARGUMENT(BasicBuiltInTypeInfos);
    gcmVERIFY_ARGUMENT(BuiltInVariableCount > 0);
    gcmVERIFY_ARGUMENT(BuiltInVariables);

    for (i = 0; i < BuiltInVariableCount; i++)
    {
        /* If this variable is not enabled in this shader, then we don't need to load it. */
        extension.extension1 = BuiltInVariables[i].extension;
        if (!sloCOMPILER_ExtensionEnabled(Compiler, &extension))
        {
            continue;
        }

        /* Load the filed variables for a struct. */
        if (BuiltInVariables[i].type == T_IO_BLOCK &&
            BuiltInVariables[i].fieldVariables != gcvNULL)
        {
            status = _LoadBuiltInVariablesForIOBlock(Compiler,
                                                     BuiltInVariables[i]);

            if (gcmIS_ERROR(status)) break;
            continue;
        }

        if (BuiltInVariables[i].arrayLength == -1)
        {
            continue;
        }

        /* Setup the data type */
        if (BuiltInVariables[i].precision != slvPRECISION_QUALIFIER_DEFAULT ||
            BuiltInVariables[i].qualifier != slvSTORAGE_QUALIFIER_NONE)
        {
            status = sloCOMPILER_CreateDataType(Compiler,
                                                BuiltInVariables[i].type,
                                                gcvNULL,
                                                &dataType);

            if (gcmIS_ERROR(status)) break;

            dataType->qualifiers.precision = BuiltInVariables[i].precision;
            dataType->qualifiers.storage = BuiltInVariables[i].qualifier;

            if (BuiltInVariables[i].qualifier == slvSTORAGE_QUALIFIER_VARYING_PATCH_OUT ||
                BuiltInVariables[i].qualifier == slvSTORAGE_QUALIFIER_VARYING_PATCH_IN)
            {
                slsQUALIFIERS_SET_FLAG(&dataType->qualifiers, slvQUALIFIERS_FLAG_PATCH);
                dataType->qualifiers.storage = BuiltInVariables[i].implQualifier;
            }
        }
        else
        {
            basicBuiltInTypeInfo = _GetBasicBuiltInTypeInfo(BasicBuiltInTypeInfos,
                                                            BuiltInVariables[i].type);

            gcmASSERT(basicBuiltInTypeInfo);

            if (basicBuiltInTypeInfo == gcvNULL)
                break;

            dataType = basicBuiltInTypeInfo->normalDataType;
        }

        if (BuiltInVariables[i].arrayLength != 0)
        {
            arrayLength = BuiltInVariables[i].arrayLength;

            status = sloCOMPILER_CreateArrayDataType(Compiler,
                                                     dataType,
                                                     arrayLength,
                                                     &dataType);

            if (gcmIS_ERROR(status)) break;
        }

        if (BuiltInVariables[i].updateVarFunc != gcvNULL)
        {
            status = (*BuiltInVariables[i].updateVarFunc)(Compiler,
                                                          &dataType);
            if (gcmIS_ERROR(status)) break;
        }

        /* Create the variable name */
        status = sloCOMPILER_AllocatePoolString(Compiler,
                                                BuiltInVariables[i].symbol,
                                                &symbolInPool);

        if (gcmIS_ERROR(status)) break;

        status = sloCOMPILER_CreateName(Compiler,
                                        0,
                                        0,
                                        slvVARIABLE_NAME,
                                        dataType,
                                        symbolInPool,
                                        extension,
                                        gcvFALSE,
                                        gcvNULL);

        if (gcmIS_ERROR(status)) break;
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
slConstructIVEC2Array4(
    IN sloCOMPILER Compiler,
    OUT slsDATA_TYPE** DataType
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    slsDATA_TYPE* dataType = gcvNULL;

    gcmASSERT(DataType);

    status = sloCOMPILER_CreateDataType(Compiler,
                                        T_IVEC2,
                                        gcvNULL,
                                        &dataType);

    if(gcmIS_ERROR(status))
    {
        return status;
    }

    status = sloCOMPILER_CreateArrayDataType(Compiler,
                                             dataType,
                                             4,
                                             &dataType);
    dataType->qualifiers.storage = slvSTORAGE_QUALIFIER_IN;
    *DataType = dataType;

    return status;
}

static sloIR_EXPR
_GetArgumentVariable(
    IN sloIR_EXPR Argument
    )
{
    sloIR_EXPR argument = Argument;
    sleIR_OBJECT_TYPE exprType = sloIR_OBJECT_GetType(&argument->base);

    /* If this argument is a BINARY_EXPR, then we cab accept "a[i]". */
    if (exprType == slvIR_BINARY_EXPR)
    {
        sloIR_BINARY_EXPR binaryExpr = (sloIR_BINARY_EXPR)(&argument->base);

        if (binaryExpr->type == slvBINARY_SUBSCRIPT)
        {
            argument = _GetArgumentVariable(binaryExpr->leftOperand);
        }
    }
    /* If this argument is a UNARY_EXPR, then we can accept "a.b". */
    else if (exprType == slvIR_UNARY_EXPR)
    {
        sloIR_UNARY_EXPR unaryExpr = (sloIR_UNARY_EXPR)(&argument->base);

        if (unaryExpr->type == slvUNARY_FIELD_SELECTION)
        {
            argument = _GetArgumentVariable(unaryExpr->operand);
        }
    }

    return argument;
}

gceSTATUS
slFuncCheckForAtrigAsIntrinsic(
    IN sloCOMPILER Compiler,
    IN struct _slsNAME * FuncName,
    IN struct _sloIR_POLYNARY_EXPR * PolynaryExpr
    )
{
    gceSTATUS    status = gcvSTATUS_OK;
    sloEXTENSION extension = {0};
    extension.extension1 = slvEXTENSION1_HALTI5_WITH_FMA_SUPPORT;

    if(sloCOMPILER_ExtensionEnabled(Compiler, &extension))
    {
        slsFUNC_SET_FLAG(&(FuncName->u.funcInfo), slvFUNC_SKIP_AS_INTRINSIC);
    }
    return status;
}

gceSTATUS
slFuncCheckForInterpolate(
    IN sloCOMPILER Compiler,
    IN struct _slsNAME * FuncName,
    IN struct _sloIR_POLYNARY_EXPR * PolynaryExpr
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    sloIR_EXPR argument;
    sloIR_VARIABLE variable;
    sleSTORAGE_QUALIFIER storage;
    sleCOMPILER_FLAGS flag;

    argument = _GetArgumentVariable((sloIR_EXPR)PolynaryExpr->operands->members.next);

    if (sloIR_OBJECT_GetType(&argument->base) != slvIR_VARIABLE)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        slvREPORT_ERROR,
                                        "The first argument of %s "
                                        "must be a input of a fragment shader.",
                                        PolynaryExpr->funcSymbol));
        return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }

    variable = (sloIR_VARIABLE)(&argument->base);
    storage = variable->name->dataType->qualifiers.storage;

    if ((storage != slvSTORAGE_QUALIFIER_VARYING_IN &&
         storage != slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER) ||
        variable->name->dataType->elementType == slvTYPE_STRUCT)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        slvREPORT_ERROR,
                                        "The first argument of %s "
                                        "must be a input of a fragment shader.",
                                        PolynaryExpr->funcSymbol));
        return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }

    /*
    ** If a shader uses a centroid varying as the argument of a interpolate function,
    ** we treat this varying as center in RA and evaluate its centroid value in PS.
    */
    if (slsQUALIFIERS_GET_AUXILIARY(&variable->name->dataType->qualifiers) == slvAUXILIARY_QUALIFIER_CENTROID)
    {
        flag = Compiler->context.compilerFlags;

        slsCOMPILER_SetPatchForCentroidVarying(flag);

        Compiler->context.compilerFlags = flag;
    }

    slsQUALIFIERS_SET_FLAG(&variable->name->dataType->qualifiers, slvQUALIFIERS_FLAG_USE_AS_INTERPOLATE_FUNCTION);

    return status;
}

gceSTATUS
slFuncCheckForTextureGatherOffsets(
    IN sloCOMPILER Compiler,
    IN struct _slsNAME * FuncName,
    IN struct _sloIR_POLYNARY_EXPR * PolynaryExpr
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    sloIR_EXPR argument;
    sloIR_EXPR offsetsArg = gcvNULL;
    sleIR_OBJECT_TYPE exprType;
    gctBOOL isOffsetsConstant = gcvFALSE;

    /* Get the offsets expr. */
    FOR_EACH_DLINK_NODE(&PolynaryExpr->operands->members, struct _sloIR_EXPR, argument)
    {
        if (argument->dataType->arrayLength == 4)
        {
            offsetsArg = argument;
            break;
        }
    }

    if (!offsetsArg)
    {
        gcmASSERT(gcvFALSE);
        return status;
    }

    /* The offsets of textureGatherOffsets must be an constant array. */
    offsetsArg = _GetArgumentVariable(offsetsArg);
    exprType = sloIR_OBJECT_GetType(&offsetsArg->base);

    if (exprType == slvIR_CONSTANT)
    {
        isOffsetsConstant = gcvTRUE;
    }

    if (!isOffsetsConstant)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        slvREPORT_ERROR,
                                        "The first argument of %s "
                                        "must be a input of a fragment shader.",
                                        PolynaryExpr->funcSymbol));
        return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }

    return status;
}

static gceSTATUS
_LoadBuiltInFunctions(
    IN sloCOMPILER Compiler,
    IN slsBASIC_BUILT_IN_TYPE_INFO * BasicBuiltInTypeInfos,
    IN gctUINT BuiltInFunctionCount,
    IN slsBUILT_IN_FUNCTION * BuiltInFunctions
    )
{
    gceSTATUS                       status = gcvSTATUS_OK;
    gctUINT                         i, j;
    sltPOOL_STRING                  symbolInPool;
    slsBASIC_BUILT_IN_TYPE_INFO *   basicBuiltInTypeInfo;
    slsNAME *                       funcName = gcvNULL;
    slsNAME *                       paramName = gcvNULL;
    gctBOOL                         hasMemAccess;
    sloEXTENSION                    extension = {0};

    gcmHEADER_ARG("Compiler=0x%x BasicBuiltInTypeInfos=0x%x "
                  "BuiltInFunctionCount=%u BuiltInFunctions=0x%x",
                  Compiler, BasicBuiltInTypeInfos, BuiltInFunctionCount,
                  BuiltInFunctions);


    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmVERIFY_ARGUMENT(BasicBuiltInTypeInfos);
    gcmVERIFY_ARGUMENT(BuiltInFunctionCount > 0);
    gcmVERIFY_ARGUMENT(BuiltInFunctions);

    for (i = 0; i < BuiltInFunctionCount; i++)
    {
        hasMemAccess = slsFUNC_HAS_FLAG(&(BuiltInFunctions[i]), slvFUNC_HAS_MEM_ACCESS);

        /* Create function name */
        basicBuiltInTypeInfo = _GetBasicBuiltInTypeInfo(BasicBuiltInTypeInfos,
                                                        BuiltInFunctions[i].returnType);

        gcmASSERT(basicBuiltInTypeInfo);

        if (basicBuiltInTypeInfo == gcvNULL) break;

        status = sloCOMPILER_AllocatePoolString(Compiler,
                                                BuiltInFunctions[i].symbol,
                                                &symbolInPool);

        if (gcmIS_ERROR(status)) break;

        extension.extension1 = BuiltInFunctions[i].extension;
        status = sloCOMPILER_CreateName(Compiler,
                                        0,
                                        0,
                                        slvFUNC_NAME,
                                        basicBuiltInTypeInfo->anypDataType,
                                        symbolInPool,
                                        extension,
                                        gcvFALSE,
                                        &funcName);

        if (gcmIS_ERROR(status)) break;

        status = sloCOMPILER_CreateNameSpace(Compiler,
                                             symbolInPool,
                                             slvNAME_SPACE_TYPE_FUNCTION,
                                             &funcName->u.funcInfo.localSpace);

        if (gcmIS_ERROR(status)) break;

        /* Create function parameters. */
        for (j = 0; j < BuiltInFunctions[i].paramCount; j++)
        {
            slsDATA_TYPE * dataType = gcvNULL;
            paramDataTypeConstructor callback = gcvNULL;

            /* Create parameter name */
            switch (BuiltInFunctions[i].paramTypes[j])
            {
                case T_TYPE_MATCH_CALLBACK0:
                    callback = BuiltInFunctions[i].paramDataTypeConstructors[0];
                    break;
                case T_TYPE_MATCH_CALLBACK1:
                    callback = BuiltInFunctions[i].paramDataTypeConstructors[1];
                    break;
                case T_TYPE_MATCH_CALLBACK2:
                    callback = BuiltInFunctions[i].paramDataTypeConstructors[2];
                    break;
            }

            if (callback)
            {
                callback(Compiler, &dataType);
            }
            else
            {
                basicBuiltInTypeInfo = _GetBasicBuiltInTypeInfo(BasicBuiltInTypeInfos,
                                                                BuiltInFunctions[i].paramTypes[j]);

                dataType = basicBuiltInTypeInfo->inDataType;
            }

            if (hasMemAccess)
            {
                status = slsDATA_TYPE_Clone(Compiler,
                                            dataType->qualifiers.storage,
                                            dataType->qualifiers.precision,
                                            dataType,
                                            &dataType);

                dataType->qualifiers.memoryAccess = BuiltInFunctions[i].paramMemoryAccess[j];
            }

            gcmASSERT(dataType);

            if (dataType == gcvNULL) break;

            extension.extension1 = slvEXTENSION1_NONE;
            status = sloCOMPILER_CreateName(Compiler,
                                            0,
                                            0,
                                            slvPARAMETER_NAME,
                                            dataType,
                                            "",
                                            extension,
                                            gcvFALSE,
                                            &paramName);

            if (gcmIS_ERROR(status)) break;
        }

        if (gcmIS_ERROR(status)) break;

        sloCOMPILER_PopCurrentNameSpace(Compiler, gcvNULL);

        funcName->u.funcInfo.evaluate             = (slsBuiltInEvaluateFunc)BuiltInFunctions[i].evaluate;
        funcName->u.funcInfo.genCode              = (slsBuiltInGenCodeFunc)BuiltInFunctions[i].genCode;
        funcName->u.funcInfo.flags                = BuiltInFunctions[i].flags;
    }

    gcmFOOTER();
    return status;
}

static gceSTATUS
_LoadIntrinsicBuiltInFunctions(
    IN sloCOMPILER Compiler,
    IN slsBASIC_BUILT_IN_TYPE_INFO *BasicBuiltInTypeInfos,
    IN gctUINT IntrinsicFunctionCount,
    IN slsINTRINSIC_BUILTIN_FUNCTION *IntrinsicFunctions
    )
{
    gceSTATUS                       status = gcvSTATUS_OK;
    gctUINT                         i, j;
    sltPOOL_STRING                  symbolInPool;
    slsBASIC_BUILT_IN_TYPE_INFO *   basicBuiltInTypeInfo;
    slsNAME *                       funcName = gcvNULL;
    slsNAME *                       paramName = gcvNULL;
    slsDATA_TYPE *                  dataType;
    gctBOOL                         hasMemAccess = gcvFALSE;
    sloEXTENSION                    extension = {0};

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmVERIFY_ARGUMENT(BasicBuiltInTypeInfos);
    gcmVERIFY_ARGUMENT(IntrinsicFunctionCount > 0);
    gcmVERIFY_ARGUMENT(IntrinsicFunctions);

    for (i = 0; i < IntrinsicFunctionCount; i++)
    {
        hasMemAccess = slsFUNC_HAS_FLAG(&(IntrinsicFunctions[i]), slvFUNC_HAS_MEM_ACCESS);

        /* now builtin function can return void, because its parameter can be OUT */
        if (IntrinsicFunctions[i].returnType != T_VOID)
        {
            basicBuiltInTypeInfo = _GetBasicBuiltInTypeInfo(BasicBuiltInTypeInfos,
                                                            IntrinsicFunctions[i].returnType);

            gcmASSERT(basicBuiltInTypeInfo);

            dataType = basicBuiltInTypeInfo->normalDataType;

            if(IntrinsicFunctions[i].returnTypePrecision == slvPRECISION_QUALIFIER_ANY)
            {
                dataType = basicBuiltInTypeInfo->anypDataType;
            }
            else if(IntrinsicFunctions[i].returnTypePrecision != slvPRECISION_QUALIFIER_DEFAULT &&
               IntrinsicFunctions[i].returnTypePrecision != dataType->qualifiers.precision)
            {
                slsDATA_TYPE_Clone(Compiler, dataType->qualifiers.storage, IntrinsicFunctions[i].returnTypePrecision, dataType, &dataType);
            }
        }
        else
        {
            /* Create the data type */
            status = sloCOMPILER_CreateDataType(Compiler,
                                                T_VOID,
                                                gcvNULL,
                                                &dataType);

            if (gcmIS_ERROR(status)) break;

        }

        status = sloCOMPILER_AllocatePoolString(Compiler,
                                                IntrinsicFunctions[i].symbol,
                                                &symbolInPool);

        if (gcmIS_ERROR(status)) break;

        /* Create function name */
        extension.extension1 = IntrinsicFunctions[i].extension;
        status = sloCOMPILER_CreateName(Compiler,
                                        0,
                                        0,
                                        slvFUNC_NAME,
                                        dataType,
                                        symbolInPool,
                                        extension,
                                        gcvFALSE,
                                        &funcName);

        if (gcmIS_ERROR(status)) break;

        if (IntrinsicFunctions[i].function != gcvNULL)
        {
            status = sloCOMPILER_SetCheckFunctionForBuiltInFunction(Compiler,
                                                                    IntrinsicFunctions[i].function,
                                                                    funcName);
            if (gcmIS_ERROR(status)) break;
        }

        status = sloCOMPILER_CreateNameSpace(Compiler,
                                             symbolInPool,
                                             slvNAME_SPACE_TYPE_FUNCTION,
                                             &funcName->u.funcInfo.localSpace);

        if (gcmIS_ERROR(status)) break;

        status = sloCOMPILER_AllocatePoolString(Compiler,
                                                IntrinsicFunctions[i].nameInLibrary,
                                                &symbolInPool);

        if (gcmIS_ERROR(status)) break;

        /* Create function parameters. */
        for (j = 0; j < IntrinsicFunctions[i].paramCount; j++)
        {
            slsDATA_TYPE *data_type, *param_data_type, *param_data_type_1;
            paramDataTypeConstructor callback = gcvNULL;

            switch (IntrinsicFunctions[i].paramTypes[j])
            {
                case T_TYPE_MATCH_CALLBACK0:
                    callback = IntrinsicFunctions[i].paramDataTypeConstructors[0];
                    break;
                case T_TYPE_MATCH_CALLBACK1:
                    callback = IntrinsicFunctions[i].paramDataTypeConstructors[1];
                    break;
                case T_TYPE_MATCH_CALLBACK2:
                    callback = IntrinsicFunctions[i].paramDataTypeConstructors[2];
                    break;
            }

            if (callback)
            {
                callback(Compiler, &data_type);
            }
            else
            {
                basicBuiltInTypeInfo = _GetBasicBuiltInTypeInfo(BasicBuiltInTypeInfos,
                                                                IntrinsicFunctions[i].paramTypes[j]);

                gcmASSERT(basicBuiltInTypeInfo);

                data_type = basicBuiltInTypeInfo->inDataType;
                if (IntrinsicFunctions[i].paramPrecisions[j] == slvPRECISION_QUALIFIER_ANY)
                {
                    data_type = basicBuiltInTypeInfo->anypInDataType;
                }
                else if (IntrinsicFunctions[i].paramPrecisions[j] != slvPRECISION_QUALIFIER_DEFAULT &&
                   IntrinsicFunctions[i].paramPrecisions[j] != data_type->qualifiers.precision)
                {
                    slsDATA_TYPE_Clone(Compiler,
                                       data_type->qualifiers.storage,
                                       IntrinsicFunctions[i].paramPrecisions[j],
                                       data_type,
                                       &data_type);
                }
            }

            if (data_type == gcvNULL) break;

            if (IntrinsicFunctions[i].paramQualifiers[j] == slvSTORAGE_QUALIFIER_OUT ||
                IntrinsicFunctions[i].paramQualifiers[j] == slvSTORAGE_QUALIFIER_INOUT)
            {
                /* clone a data type  with the paramQualifier, since the previous
                   builtin type assume qualifier is always slvSTORAGE_QUALIFIER_IN*/
                status = sloCOMPILER_CloneDataType(Compiler,
                                                   IntrinsicFunctions[i].paramQualifiers[j],
                                                   data_type->qualifiers.precision,
                                                   data_type,
                                                   &param_data_type);
            }
            else
            {
                param_data_type = data_type;
            }

            if (hasMemAccess)
            {
                status = sloCOMPILER_CloneDataType(Compiler,
                                                   param_data_type->qualifiers.storage,
                                                   param_data_type->qualifiers.precision,
                                                   param_data_type,
                                                   &param_data_type_1);

                param_data_type_1->qualifiers.memoryAccess = IntrinsicFunctions[i].paramMemoryAccess[j];
            }
            else
            {
                param_data_type_1 = param_data_type;
            }

            /* Create parameter name */
            extension.extension1 = slvEXTENSION1_NONE;
            status = sloCOMPILER_CreateName(Compiler,
                                            0,
                                            0,
                                            slvPARAMETER_NAME,
                                            param_data_type_1,
                                            "",
                                            extension,
                                            gcvFALSE,
                                            &paramName);

            if (gcmIS_ERROR(status)) break;

            /* paramName should not be builtin ? */
            paramName->isBuiltIn = gcvFALSE;
        }

        if (gcmIS_ERROR(status)) break;

        sloCOMPILER_PopCurrentNameSpace(Compiler, gcvNULL);

        funcName->u.funcInfo.intrinsicKind      = IntrinsicFunctions[i].intrinsicKind;
        /* set its managled_symbol */
        funcName->u.funcInfo.mangled_symbol     = symbolInPool;
        funcName->u.funcInfo.evaluate           = (slsBuiltInEvaluateFunc)IntrinsicFunctions[i].evaluate;
        funcName->u.funcInfo.genCode            = (slsBuiltInGenCodeFunc)IntrinsicFunctions[i].genCode;
        funcName->u.funcInfo.flags              = IntrinsicFunctions[i].flags;
        slsFUNC_SET_FLAG(&(funcName->u.funcInfo), slvFUNC_IS_INTRINSIC);
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
slLoadGeneralBuiltIns(
    IN sloCOMPILER Compiler,
    IN sleSHADER_TYPE ShaderType
    )
{
    gceSTATUS                       status;
    slsBASIC_BUILT_IN_TYPE_INFO *   basicBuiltInTypeInfos;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    status = _ConstructBasicBuiltInTypeInfos(Compiler,
                                             &basicBuiltInTypeInfos);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    do
    {
        /* Load default precision declarations */
        /* All languages except for the fragment language have the default precision. */
        if (ShaderType != slvSHADER_TYPE_FRAGMENT && ShaderType != slvSHADER_TYPE_LIBRARY)
        {
            if (Compiler->clientApiVersion == gcvAPI_OPENGL)
            {
                status = _LoadDefaultPrecisionDecls(Compiler,
                                                    GLVSDefaultPrecisionDeclCount,
                                                    GLVSDefaultPrecisionDecls);
            }
            else
            {
                status = _LoadDefaultPrecisionDecls(Compiler,
                                                    VSDefaultPrecisionDeclCount,
                                                    VSDefaultPrecisionDecls);
            }

            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        }
        else if (ShaderType != slvSHADER_TYPE_LIBRARY)
        {
            if (Compiler->clientApiVersion == gcvAPI_OPENGL)
            {
                status = _LoadDefaultPrecisionDecls(Compiler,
                                                    GLFSDefaultPrecisionDeclCount,
                                                    GLFSDefaultPrecisionDecls);
            }
            else
            {
                status = _LoadDefaultPrecisionDecls(Compiler,
                                                    FSDefaultPrecisionDeclCount,
                                                    FSDefaultPrecisionDecls);
            }

            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        }
        else
        {
            status = _LoadDefaultPrecisionDecls(Compiler,
                                                LIBDefaultPrecisionDeclCount,
                                                LIBDefaultPrecisionDecls);

            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        }

        /* Load built-in functions */
        if (ShaderType == slvSHADER_TYPE_VERTEX)
        {
            status = _LoadBuiltInFunctions(Compiler,
                                           basicBuiltInTypeInfos,
                                           VSBuiltInFunctionCount,
                                           VSBuiltInFunctions);

            if (gcmIS_ERROR(status)) break;
        }
        else
        {
            status = _LoadBuiltInFunctions(Compiler,
                                           basicBuiltInTypeInfos,
                                           FSBuiltInFunctionCount,
                                           FSBuiltInFunctions);

            if (gcmIS_ERROR(status)) break;
        }

        /* Load GS-only built-in functions. */
        if (ShaderType == slvSHADER_TYPE_GS)
        {
            status = _LoadBuiltInFunctions(Compiler,
                                           basicBuiltInTypeInfos,
                                           GSBuiltInFunctionCount,
                                           GSBuiltInFunctions);

            if (gcmIS_ERROR(status)) break;
        }

        status = _LoadBuiltInFunctions(Compiler,
                                       basicBuiltInTypeInfos,
                                       CommonBuiltInFunctionCount,
                                       CommonBuiltInFunctions);

        if (gcmIS_ERROR(status)) break;

        status = _LoadBuiltInFunctions(Compiler,
                                       basicBuiltInTypeInfos,
                                       ExtensionBuiltInFunctionCount,
                                       ExtensionBuiltInFunctions);

        if (gcmIS_ERROR(status)) break;

        status = _LoadIntrinsicBuiltInFunctions(Compiler,
                                                basicBuiltInTypeInfos,
                                                CommonIntrinsicBuiltInFunctionCount,
                                                CommonIntrinsicBuiltInFunctions);

        if (gcmIS_ERROR(status)) break;

        if (ShaderType == slvSHADER_TYPE_FRAGMENT)
        {
            status = _LoadIntrinsicBuiltInFunctions(Compiler,
                                                    basicBuiltInTypeInfos,
                                                    FSIntrinsicBuiltInFunctionCount,
                                                    FSIntrinsicBuiltInFunctions);

            if (gcmIS_ERROR(status)) break;
        }

        /* Return */
        gcmVERIFY_OK(_DestroyBasicBuiltInTypeInfos(Compiler, basicBuiltInTypeInfos));

        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    gcmVERIFY_OK(_DestroyBasicBuiltInTypeInfos(Compiler, basicBuiltInTypeInfos));

    gcmFOOTER();
    return status;
}

gceSTATUS
slLoadBuiltIns(
    IN sloCOMPILER Compiler,
    IN sleSHADER_TYPE ShaderType
    )
{
    gceSTATUS                       status;
    slsBASIC_BUILT_IN_TYPE_INFO *   basicBuiltInTypeInfos;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    status = _ConstructBasicBuiltInTypeInfos(Compiler,
                                             &basicBuiltInTypeInfos);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    do
    {
        /* Load built-in constant state */
        status = _LoadBuiltInConstants(Compiler,
                                       basicBuiltInTypeInfos);

        if (gcmIS_ERROR(status)) break;

        /* Load built-in uniform state */
        status = _LoadBuiltInUniformState(Compiler,
                                          basicBuiltInTypeInfos);

        if (gcmIS_ERROR(status)) break;

        /* Load built-in variables */
        if (ShaderType == slvSHADER_TYPE_VERTEX)
        {
            status = _LoadBuiltInVariables(Compiler,
                                           basicBuiltInTypeInfos,
                                           VSBuiltInVariableCount,
                                           VSBuiltInVariables);

            if (gcmIS_ERROR(status)) break;
        }
        else if (ShaderType == slvSHADER_TYPE_FRAGMENT)
        {
            status = _LoadBuiltInVariables(Compiler,
                                           basicBuiltInTypeInfos,
                                           FSBuiltInVariableCount,
                                           FSBuiltInVariables);

            if (gcmIS_ERROR(status)) break;
        }
        else if (ShaderType == slvSHADER_TYPE_COMPUTE)
        {
            status = _LoadBuiltInVariables(Compiler,
                                           basicBuiltInTypeInfos,
                                           CSBuiltInVariableCount,
                                           CSBuiltInVariables);

            if (gcmIS_ERROR(status)) break;
        }
        else if (ShaderType == slvSHADER_TYPE_TCS)
        {
            status = _LoadBuiltInVariables(Compiler,
                                           basicBuiltInTypeInfos,
                                           TCSBuiltInVariableCount,
                                           TCSBuiltInVariables);

            if (gcmIS_ERROR(status)) break;
        }
        else if (ShaderType == slvSHADER_TYPE_TES)
        {
            status = _LoadBuiltInVariables(Compiler,
                                           basicBuiltInTypeInfos,
                                           TESBuiltInVariableCount,
                                           TESBuiltInVariables);

            if (gcmIS_ERROR(status)) break;
        }
        else if (ShaderType == slvSHADER_TYPE_GS)
        {
            status = _LoadBuiltInVariables(Compiler,
                                           basicBuiltInTypeInfos,
                                           GSBuiltInVariableCount,
                                           GSBuiltInVariables);

            if (gcmIS_ERROR(status)) break;
        }

        /* Return */
        gcmVERIFY_OK(_DestroyBasicBuiltInTypeInfos(Compiler, basicBuiltInTypeInfos));

        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    gcmVERIFY_OK(_DestroyBasicBuiltInTypeInfos(Compiler, basicBuiltInTypeInfos));

    gcmFOOTER();
    return status;
}

gceSTATUS
slGetBuiltInVariableImplSymbol(
    IN sloCOMPILER Compiler,
    IN gctCONST_STRING Symbol,
    OUT gctCONST_STRING * ImplSymbol,
    OUT sltSTORAGE_QUALIFIER * ImplQualifier
    )
{
    gctUINT     i;
    sleSHADER_TYPE shaderType;
    gcmHEADER_ARG("Symbol=0x%x ImplSymbol=0x%x ImplQualifier=0x%x",
                  Symbol, ImplSymbol, ImplQualifier);

    /* Verify the arguments. */
    gcmASSERT(Symbol);
    gcmASSERT(ImplSymbol);
    gcmASSERT(ImplQualifier);

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_COMPILER, "Symbol=%s", gcmOPT_STRING(Symbol));
    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_COMPILER, "ImplSymbol=%x", gcmOPT_POINTER(ImplSymbol));
    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_COMPILER, "*ImplQualifier=%u", gcmOPT_VALUE(ImplQualifier));

    shaderType = Compiler->shaderType;

    if(shaderType == slvSHADER_TYPE_VERTEX)
    {
        for (i = 0; i < VSBuiltInVariableCount; i++)
        {
            if (VSBuiltInVariables[i].symbol &&
                gcmIS_SUCCESS(gcoOS_StrCmp(VSBuiltInVariables[i].symbol, Symbol)))
            {
                *ImplSymbol     = VSBuiltInVariables[i].implSymbol;
                *ImplQualifier  = VSBuiltInVariables[i].implQualifier;
                break;
            }
        }

        gcmASSERT(i < VSBuiltInVariableCount);

        if(!(i < VSBuiltInVariableCount))
        {
            *ImplSymbol = gcvNULL;
            *ImplQualifier = slvSTORAGE_QUALIFIER_NONE;
            gcmFOOTER_NO();
            return gcvSTATUS_NAME_NOT_FOUND;
        }
    }
    else if(shaderType == slvSHADER_TYPE_FRAGMENT)
    {
        for (i = 0; i < FSBuiltInVariableCount; i++)
        {
            if (FSBuiltInVariables[i].symbol &&
                gcmIS_SUCCESS(gcoOS_StrCmp(FSBuiltInVariables[i].symbol, Symbol)))
            {
                *ImplSymbol     = FSBuiltInVariables[i].implSymbol;
                *ImplQualifier  = FSBuiltInVariables[i].implQualifier;
                break;
            }
        }

        gcmASSERT(i < FSBuiltInVariableCount);

        if(!(i < FSBuiltInVariableCount))
        {
            *ImplSymbol = gcvNULL;
            *ImplQualifier = slvSTORAGE_QUALIFIER_NONE;
            gcmFOOTER_NO();
            return gcvSTATUS_NAME_NOT_FOUND;
        }
    }
    else if (shaderType == slvSHADER_TYPE_COMPUTE)
    {
        for (i = 0; i < CSBuiltInVariableCount; i++)
        {
            if (CSBuiltInVariables[i].symbol &&
                gcmIS_SUCCESS(gcoOS_StrCmp(CSBuiltInVariables[i].symbol, Symbol)))
            {
                *ImplSymbol     = CSBuiltInVariables[i].implSymbol;
                *ImplQualifier  = CSBuiltInVariables[i].implQualifier;
                break;
            }
        }

        gcmASSERT(i < CSBuiltInVariableCount);

        if(!(i < CSBuiltInVariableCount))
        {
            *ImplSymbol = gcvNULL;
            *ImplQualifier = slvSTORAGE_QUALIFIER_NONE;
            gcmFOOTER_NO();
            return gcvSTATUS_NAME_NOT_FOUND;
        }
    }
    else if (shaderType == slvSHADER_TYPE_TCS)
    {
        for (i = 0; i < TCSBuiltInVariableCount; i++)
        {
            if (TCSBuiltInVariables[i].symbol &&
                gcmIS_SUCCESS(gcoOS_StrCmp(TCSBuiltInVariables[i].symbol, Symbol)))
            {
                *ImplSymbol     = TCSBuiltInVariables[i].implSymbol;
                *ImplQualifier  = TCSBuiltInVariables[i].implQualifier;
                break;
            }
        }

        gcmASSERT(i < TCSBuiltInVariableCount);

        if(!(i < TCSBuiltInVariableCount))
        {
            *ImplSymbol = gcvNULL;
            *ImplQualifier = slvSTORAGE_QUALIFIER_NONE;
            gcmFOOTER_NO();
            return gcvSTATUS_NAME_NOT_FOUND;
        }
    }
    else if (shaderType == slvSHADER_TYPE_TES)
    {
        for (i = 0; i < TESBuiltInVariableCount; i++)
        {
            if (TESBuiltInVariables[i].symbol &&
                gcmIS_SUCCESS(gcoOS_StrCmp(TESBuiltInVariables[i].symbol, Symbol)))
            {
                *ImplSymbol     = TESBuiltInVariables[i].implSymbol;
                *ImplQualifier  = TESBuiltInVariables[i].implQualifier;
                break;
            }
        }

        gcmASSERT(i < TESBuiltInVariableCount);

        if(!(i < TESBuiltInVariableCount))
        {
            *ImplSymbol = gcvNULL;
            *ImplQualifier = slvSTORAGE_QUALIFIER_NONE;
            gcmFOOTER_NO();
            return gcvSTATUS_NAME_NOT_FOUND;
        }
    }
    else if (shaderType == slvSHADER_TYPE_GS)
    {
        for (i = 0; i < GSBuiltInVariableCount; i++)
        {
            if (GSBuiltInVariables[i].symbol &&
                gcmIS_SUCCESS(gcoOS_StrCmp(GSBuiltInVariables[i].symbol, Symbol)))
            {
                *ImplSymbol     = GSBuiltInVariables[i].implSymbol;
                *ImplQualifier  = GSBuiltInVariables[i].implQualifier;
                break;
            }
        }

        gcmASSERT(i < GSBuiltInVariableCount);

        if(!(i < GSBuiltInVariableCount))
        {
            *ImplSymbol = gcvNULL;
            *ImplQualifier = slvSTORAGE_QUALIFIER_NONE;
            gcmFOOTER_NO();
            return gcvSTATUS_NAME_NOT_FOUND;
        }
    }

    gcmFOOTER_ARG("*ImplSymbol=%u *ImplQualifier=%u", *ImplSymbol, *ImplQualifier);
    return gcvSTATUS_OK;
}

/* Built-In Functions */
gctBOOL
slIsTextureLookupFunction(
    IN gctCONST_STRING Symbol
    )
{
    gcmASSERT(Symbol);

    return (gcmIS_SUCCESS(gcoOS_StrNCmp(Symbol, "texture", 7))) ||
           (gcmIS_SUCCESS(gcoOS_StrNCmp(Symbol, "viv_texture", 11)));
}

gceSTATUS
_GenTexture2DCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2 || OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (OperandCount == 3)
    {
        status = slGenGenericCode2(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                slvOPCODE_TEXTURE_BIAS,
                                IOperand,
                                &OperandsParameters[0].rOperands[0],
                                &OperandsParameters[2].rOperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_TEXTURE_LOAD,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/***************************************************************************************************
  To be used for both sampler2DShadow and samplerCubeShadow
***************************************************************************************************/
static gceSTATUS
_GenTextureShadowCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2 || OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (OperandCount == 3)
    {
        status = slGenGenericCode2(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                slvOPCODE_TEXTURE_BIAS,
                                IOperand,
                                &OperandsParameters[0].rOperands[0],
                                &OperandsParameters[2].rOperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_TEXTURE_LOAD_PCF,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenAccessLayerCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN slsROPERAND *Coord,
    IN slsCOMPONENT_SELECTION ComponentSelection,
    OUT slsIOPERAND *Layer
    )
{
    gceSTATUS status;
    slsROPERAND intermROperand[1];
    slsROPERAND constantHalf[1];
    slsROPERAND constantZero[1];
    gcSHADER_TYPE componentDataType = gcGetComponentDataType(Coord->dataType);

    gcmHEADER();

    sloIR_ROperandComponentSelect(Compiler,
                                  Coord,
                                  ComponentSelection,
                                  intermROperand);

    slsIOPERAND_New(Compiler, Layer, intermROperand->dataType,
                                     intermROperand->u.reg.precision);

    if (componentDataType != gcSHADER_FLOAT_X1)
    {
        slsROPERAND_InitializeIntOrIVecConstant(constantZero,
                                                gcSHADER_INTEGER_X1,
                                                gcSHADER_PRECISION_HIGH,
                                                0);
        status = slGenGenericCode2(Compiler,
                                   LineNo,
                                   StringNo,
                                   slvOPCODE_MAX,
                                   Layer,
                                   constantZero,
                                   intermROperand);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else
    {
        slsROPERAND_InitializeFloatOrVecOrMatConstant(constantHalf,
                                                      gcSHADER_FLOAT_X1,
                                                      gcSHADER_PRECISION_MEDIUM,
                                                      (gctFLOAT)0.5);

        status = slGenArithmeticExprCode(Compiler,
                                         LineNo,
                                         StringNo,
                                         slvOPCODE_ADD,
                                         Layer,
                                         constantHalf,
                                         intermROperand);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        slsROPERAND_InitializeUsingIOperand(intermROperand, Layer);
        status = slGenGenericCode1(Compiler,
                                   LineNo,
                                   StringNo,
                                   slvOPCODE_FLOOR,
                                   Layer,
                                   intermROperand);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        slsROPERAND_InitializeFloatOrVecOrMatConstant(constantZero,
                                                      gcSHADER_FLOAT_X1,
                                                      gcSHADER_PRECISION_MEDIUM,
                                                      (gctFLOAT)0.0);
        status = slGenGenericCode2(Compiler,
                                   LineNo,
                                   StringNo,
                                   slvOPCODE_MAX,
                                   Layer,
                                   constantZero,
                                   intermROperand);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_ConvertCoordForSampler1D(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * NewCoordIOperand
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    sloIR_EXPR          samplerOperand = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _sloIR_EXPR);
    gcSHADER_TYPE       coordElementType;
    gcSHADER_TYPE       newCoordType;
    slsROPERAND         intermROperand[1];
    slsLOPERAND         intermLOperand[1];
    slsIOPERAND         layerOperand[1];
    slsIOPERAND         iOperand[1];
    slsROPERAND         rZero[1];
    sluCONSTANT_VALUE   cZero[1];
    gctUINT8            componentCount;
    gctBOOL             bIsSampler1DArray = gcvFALSE;

    /*
    ** 1) For 1D, change the coord from "int p" to "ivec2(p, 0)".
    ** 2) For 1DArray, change the coord from "ivec2 p" to "ivec3(p.x, 0, p.y)".
    */
    if (slsDATA_TYPE_IsSampler1D(samplerOperand->dataType))
    {
        componentCount = 2;
    }
    else
    {
        gcmASSERT(slsDATA_TYPE_IsSampler1DArray(samplerOperand->dataType));
        componentCount = 3;
        bIsSampler1DArray = gcvTRUE;
    }

    /* Get the component data type. */
    if (!bIsSampler1DArray)
    {
        coordElementType = OperandsParameters[1].rOperands[0].dataType;
    }
    else
    {
        coordElementType = gcGetComponentDataType(OperandsParameters[1].rOperands[0].dataType);
    }
    /* Generate the new coord data type. */
    newCoordType = gcConvScalarToVectorDataType(coordElementType, componentCount);

    /* Change the coordinate from "p" to "ivec2(p, 0)" or "ivec3(p.x, 0, p.y). */
    slsIOPERAND_New(Compiler,
                    iOperand,
                    newCoordType,
                    OperandsParameters[1].rOperands[0].u.reg.precision);

    /* newCoord.x = p.x */
    slsLOPERAND_InitializeUsingIOperand(intermLOperand, iOperand);
    sloIR_LOperandComponentSelect(Compiler,
                                  intermLOperand,
                                  ComponentSelection_X,
                                  intermLOperand);

    if (!bIsSampler1DArray)
    {
        intermROperand[0] = OperandsParameters[1].rOperands[0];
    }
    else
    {
        status = sloIR_ROperandComponentSelect(Compiler,
                                               &OperandsParameters[1].rOperands[0],
                                               ComponentSelection_X,
                                               intermROperand);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    status = slGenAssignCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             intermLOperand,
                             &intermROperand[0]);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* newCoord.y = 0 */
    slsLOPERAND_InitializeUsingIOperand(intermLOperand, iOperand);
    sloIR_LOperandComponentSelect(Compiler,
                                  intermLOperand,
                                  ComponentSelection_Y,
                                  intermLOperand);
    cZero[0].uintValue = 0;
    slsROPERAND_InitializeConstant(&rZero[0],
                                   coordElementType,
                                   OperandsParameters[1].rOperands[0].u.reg.precision,
                                   1,
                                   cZero);
    status = slGenAssignCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             intermLOperand,
                             &rZero[0]);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* Check if it is a gsampler1DArray. */
    if (bIsSampler1DArray)
    {
        status = _GenAccessLayerCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     &OperandsParameters[1].rOperands[0],
                                     ComponentSelection_Y,
                                     layerOperand);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        /* newCoord.z = p.y */
        slsLOPERAND_InitializeUsingIOperand(intermLOperand, iOperand);
        sloIR_LOperandComponentSelect(Compiler,
                                      intermLOperand,
                                      ComponentSelection_Z,
                                      intermLOperand);
        slsROPERAND_InitializeUsingIOperand(intermROperand, layerOperand);
        status = slGenAssignCode(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 intermLOperand,
                                 intermROperand);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    if (NewCoordIOperand)
    {
        *NewCoordIOperand = iOperand[0];
    }

    return status;
}

gceSTATUS
_GenTexture1DLodCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS           status;
    sleOPCODE           opcode = OperandsParameters[1].genTexldU ? slvOPCODE_TEXTURE_LOAD_U : slvOPCODE_TEXTURE_LOAD;
    slsIOPERAND         iOperand[1];
    slsROPERAND         rOperand[1];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* Change the coordinate from "int p" to "ivec2(p, 0). */
    status = _ConvertCoordForSampler1D(Compiler,
                                       CodeGenerator,
                                       PolynaryExpr,
                                       OperandsParameters,
                                       iOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_TEXTURE_LOD,
                               IOperand,
                               &OperandsParameters[0].rOperands[0],
                               &OperandsParameters[2].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsROPERAND_InitializeUsingIOperand(rOperand, iOperand);
    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               opcode,
                               IOperand,
                               &OperandsParameters[0].rOperands[0],
                               &rOperand[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenTexture2DLodCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    sleOPCODE   opcode = OperandsParameters[1].genTexldU ? slvOPCODE_TEXTURE_LOAD_U : slvOPCODE_TEXTURE_LOAD;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_TEXTURE_LOD,
                               IOperand,
                               &OperandsParameters[0].rOperands[0],
                               &OperandsParameters[2].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               opcode,
                               IOperand,
                               &OperandsParameters[0].rOperands[0],
                               &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenTextureShadowLodCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_TEXTURE_LOD,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[2].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_TEXTURE_LOAD_PCF,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenTexture2DProjCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2 || OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (OperandCount == 3)
    {
        status = slGenGenericCode2(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                slvOPCODE_TEXTURE_BIAS,
                                IOperand,
                                &OperandsParameters[0].rOperands[0],
                                &OperandsParameters[2].rOperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_TEXTURE_LOAD_PROJ,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenTextureShadowProjCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2 || OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (OperandCount == 3)
    {
        status = slGenGenericCode2(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                slvOPCODE_TEXTURE_BIAS,
                                IOperand,
                                &OperandsParameters[0].rOperands[0],
                                &OperandsParameters[2].rOperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_TEXTURE_LOAD_PCFPROJ,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenTexture2DProjLodCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_TEXTURE_LOD,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[2].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_TEXTURE_LOAD_PROJ,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenTextureShadowProjLodCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_TEXTURE_LOD,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[2].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_TEXTURE_LOAD_PCFPROJ,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenTexture3DCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2 || OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (OperandCount == 3)
    {
        status = slGenGenericCode2(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                slvOPCODE_TEXTURE_BIAS,
                                IOperand,
                                &OperandsParameters[0].rOperands[0],
                                &OperandsParameters[2].rOperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_TEXTURE_LOAD,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenTexture3DLodCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    sleOPCODE   opcode = OperandsParameters[1].genTexldU ? slvOPCODE_TEXTURE_LOAD_U : slvOPCODE_TEXTURE_LOAD;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_TEXTURE_LOD,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[2].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               opcode,
                               IOperand,
                               &OperandsParameters[0].rOperands[0],
                               &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenTexture3DProjCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2 || OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (OperandCount == 3)
    {
        status = slGenGenericCode2(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                slvOPCODE_TEXTURE_BIAS,
                                IOperand,
                                &OperandsParameters[0].rOperands[0],
                                &OperandsParameters[2].rOperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_TEXTURE_LOAD_PROJ,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenTexture3DProjLodCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_TEXTURE_LOD,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[2].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_TEXTURE_LOAD_PROJ,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenTexture1DCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    return _GenTexture1DArrayCode(
                                  Compiler,
                                  CodeGenerator,
                                  PolynaryExpr,
                                  OperandCount,
                                  OperandsParameters,
                                  IOperand
                                  );
}

gceSTATUS
_GenTexture1DArrayCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS           status;
    sleOPCODE           opcode = OperandsParameters[1].genTexldU ? slvOPCODE_TEXTURE_LOAD_U : slvOPCODE_TEXTURE_LOAD;
    slsROPERAND         rOperand[1];
    slsIOPERAND         iOperand[1];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2 || OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* Change the coordinate from "ivec2 p" to "ivec3(p.x, 0, p.y). */
    status = _ConvertCoordForSampler1D(Compiler,
                                       CodeGenerator,
                                       PolynaryExpr,
                                       OperandsParameters,
                                       iOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    if (OperandCount == 3)
    {
        status = slGenGenericCode2(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                slvOPCODE_TEXTURE_BIAS,
                                IOperand,
                                &OperandsParameters[0].rOperands[0],
                                &OperandsParameters[2].rOperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    slsROPERAND_InitializeUsingIOperand(rOperand, iOperand);
    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            opcode,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            rOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenTexture1DArrayLodCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS           status;
    sleOPCODE           opcode = OperandsParameters[1].genTexldU ? slvOPCODE_TEXTURE_LOAD_U : slvOPCODE_TEXTURE_LOAD;
    slsIOPERAND         iOperand[1];
    slsROPERAND         rOperand[1];
    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* Change the coordinate from "ivec2 p" to "ivec3(p.x, 0, p.y). */
    status = _ConvertCoordForSampler1D(Compiler,
                                       CodeGenerator,
                                       PolynaryExpr,
                                       OperandsParameters,
                                       iOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_TEXTURE_LOD,
                               IOperand,
                               &OperandsParameters[0].rOperands[0],
                               &OperandsParameters[2].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsROPERAND_InitializeUsingIOperand(rOperand, iOperand);
    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               opcode,
                               IOperand,
                               &OperandsParameters[0].rOperands[0],
                               rOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenTexture2DArrayCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    slsROPERAND intermROperand[1];
    slsLOPERAND intermLOperand[1];
    slsIOPERAND layerOperand[1];
    slsIOPERAND iOperand[1];
    slsROPERAND rOperand[1];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2 || OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = _GenAccessLayerCode(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 &OperandsParameters[1].rOperands[0],
                                 ComponentSelection_Z,
                                 layerOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsIOPERAND_New(Compiler, iOperand, OperandsParameters[1].rOperands[0].dataType,
                                        OperandsParameters[1].rOperands[0].u.reg.precision);
    slsLOPERAND_InitializeUsingIOperand(intermLOperand, iOperand);
    status = slGenAssignCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             intermLOperand,
                             &OperandsParameters[1].rOperands[0]);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    sloIR_LOperandComponentSelect(Compiler,
                                  intermLOperand,
                                  ComponentSelection_Z,
                                  intermLOperand);

    slsROPERAND_InitializeUsingIOperand(intermROperand, layerOperand);
    status = slGenAssignCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             intermLOperand,
                             intermROperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    if (OperandCount == 3)
    {
        status = slGenGenericCode2(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                slvOPCODE_TEXTURE_BIAS,
                                IOperand,
                                &OperandsParameters[0].rOperands[0],
                                &OperandsParameters[2].rOperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    slsROPERAND_InitializeUsingIOperand(rOperand, iOperand);
    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_TEXTURE_LOAD,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            rOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenTexture2DArrayLodCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    slsROPERAND intermROperand[1];
    slsLOPERAND intermLOperand[1];
    slsIOPERAND layerOperand[1];
    slsIOPERAND iOperand[1];
    slsROPERAND rOperand[1];
    sleOPCODE   opcode = OperandsParameters[1].genTexldU ? slvOPCODE_TEXTURE_LOAD_U : slvOPCODE_TEXTURE_LOAD;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = _GenAccessLayerCode(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 &OperandsParameters[1].rOperands[0],
                                 ComponentSelection_Z,
                                 layerOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsIOPERAND_New(Compiler, iOperand, OperandsParameters[1].rOperands[0].dataType,
                                        OperandsParameters[1].rOperands[0].u.reg.precision);
    slsLOPERAND_InitializeUsingIOperand(intermLOperand, iOperand);
    status = slGenAssignCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             intermLOperand,
                             &OperandsParameters[1].rOperands[0]);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    sloIR_LOperandComponentSelect(Compiler,
                                  intermLOperand,
                                  ComponentSelection_Z,
                                  intermLOperand);

    slsROPERAND_InitializeUsingIOperand(intermROperand, layerOperand);
    status = slGenAssignCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             intermLOperand,
                             intermROperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_TEXTURE_LOD,
                               IOperand,
                               &OperandsParameters[0].rOperands[0],
                               &OperandsParameters[2].rOperands[0]);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsROPERAND_InitializeUsingIOperand(rOperand, iOperand);
    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               opcode,
                               IOperand,
                               &OperandsParameters[0].rOperands[0],
                               rOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenShadow1DArrayCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    slsROPERAND intermROperand[1];
    slsLOPERAND intermLOperand[1];
    slsIOPERAND layerOperand[1];
    slsIOPERAND iOperand[1];
    slsROPERAND rOperand[1];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2 || OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = _GenAccessLayerCode(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 &OperandsParameters[1].rOperands[0],
                                 ComponentSelection_Y,
                                 layerOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsIOPERAND_New(Compiler, iOperand, OperandsParameters[1].rOperands[0].dataType,
                                        OperandsParameters[1].rOperands[0].u.reg.precision);
    slsLOPERAND_InitializeUsingIOperand(intermLOperand, iOperand);
    status = slGenAssignCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             intermLOperand,
                             &OperandsParameters[1].rOperands[0]);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    sloIR_LOperandComponentSelect(Compiler,
                                  intermLOperand,
                                  ComponentSelection_Y,
                                  intermLOperand);

    slsROPERAND_InitializeUsingIOperand(intermROperand, layerOperand);
    status = slGenAssignCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             intermLOperand,
                             intermROperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    if (OperandCount == 3)
    {
        status = slGenGenericCode2(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                slvOPCODE_TEXTURE_BIAS,
                                IOperand,
                                &OperandsParameters[0].rOperands[0],
                                &OperandsParameters[2].rOperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    slsROPERAND_InitializeUsingIOperand(rOperand, iOperand);
    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_TEXTURE_LOAD_PCF,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            rOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenShadow1DArrayLodCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    slsROPERAND intermROperand[1];
    slsLOPERAND intermLOperand[1];
    slsIOPERAND layerOperand[1];
    slsIOPERAND iOperand[1];
    slsROPERAND rOperand[1];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = _GenAccessLayerCode(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 &OperandsParameters[1].rOperands[0],
                                 ComponentSelection_Y,
                                 layerOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsIOPERAND_New(Compiler, iOperand, OperandsParameters[1].rOperands[0].dataType,
                                        OperandsParameters[1].rOperands[0].u.reg.precision);
    slsLOPERAND_InitializeUsingIOperand(intermLOperand, iOperand);
    status = slGenAssignCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             intermLOperand,
                             &OperandsParameters[1].rOperands[0]);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    sloIR_LOperandComponentSelect(Compiler,
                                  intermLOperand,
                                  ComponentSelection_Y,
                                  intermLOperand);

    slsROPERAND_InitializeUsingIOperand(intermROperand, layerOperand);
    status = slGenAssignCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             intermLOperand,
                             intermROperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }


    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_TEXTURE_LOD,
                               IOperand,
                               &OperandsParameters[0].rOperands[0],
                               &OperandsParameters[2].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsROPERAND_InitializeUsingIOperand(rOperand, iOperand);
    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_TEXTURE_LOAD,
                               IOperand,
                               &OperandsParameters[0].rOperands[0],
                               rOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenShadow2DArrayCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    slsROPERAND intermROperand[1];
    slsLOPERAND intermLOperand[1];
    slsIOPERAND layerOperand[1];
    slsIOPERAND iOperand[1];
    slsROPERAND rOperand[1];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = _GenAccessLayerCode(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 &OperandsParameters[1].rOperands[0],
                                 ComponentSelection_Z,
                                 layerOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsIOPERAND_New(Compiler, iOperand, OperandsParameters[1].rOperands[0].dataType,
                                        OperandsParameters[1].rOperands[0].u.reg.precision);
    slsLOPERAND_InitializeUsingIOperand(intermLOperand, iOperand);
    status = slGenAssignCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             intermLOperand,
                             &OperandsParameters[1].rOperands[0]);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    sloIR_LOperandComponentSelect(Compiler,
                                  intermLOperand,
                                  ComponentSelection_Z,
                                  intermLOperand);

    slsROPERAND_InitializeUsingIOperand(intermROperand, layerOperand);
    status = slGenAssignCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             intermLOperand,
                             intermROperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsROPERAND_InitializeUsingIOperand(rOperand, iOperand);
    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_TEXTURE_LOAD_PCF,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            rOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenTextureCubeCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    return _GenTexture2DCode(
                            Compiler,
                            CodeGenerator,
                            PolynaryExpr,
                            OperandCount,
                            OperandsParameters,
                            IOperand);
}

gceSTATUS
_GenTextureCubeLodCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    return _GenTexture2DLodCode(
                                Compiler,
                                CodeGenerator,
                                PolynaryExpr,
                                OperandCount,
                                OperandsParameters,
                                IOperand);
}

gceSTATUS
_GenTextureCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    sloIR_EXPR  expr;
    sltBUILT_IN_GEN_CODE_FUNC_PTR   genCode = gcvNULL;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2 || OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    gcmASSERT(!slsDLINK_LIST_IsEmpty(&PolynaryExpr->operands->members));

    expr = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _sloIR_EXPR);

    switch(expr->dataType->elementType)
    {
    case slvTYPE_SAMPLER2D:
    case slvTYPE_ISAMPLER2D:
    case slvTYPE_USAMPLER2D:
    case slvTYPE_SAMPLEREXTERNALOES:
    case slvTYPE_SAMPLER2DRECT:
        genCode = _GenTexture2DCode;
        break;

    case slvTYPE_SAMPLER3D:
    case slvTYPE_ISAMPLER3D:
    case slvTYPE_USAMPLER3D:
        genCode = _GenTexture3DCode;
        break;

    case slvTYPE_SAMPLERCUBE:
    case slvTYPE_SAMPLERCUBEARRAY:
    case slvTYPE_ISAMPLERCUBE:
    case slvTYPE_ISAMPLERCUBEARRAY:
    case slvTYPE_USAMPLERCUBE:
    case slvTYPE_USAMPLERCUBEARRAY:
        genCode = _GenTextureCubeCode;
        break;

    case slvTYPE_SAMPLER1DARRAY:
        genCode = _GenTexture1DArrayCode;
        break;

    case slvTYPE_SAMPLER2DARRAY:
    case slvTYPE_ISAMPLER2DARRAY:
    case slvTYPE_USAMPLER2DARRAY:
        genCode = _GenTexture2DArrayCode;
        break;

    case slvTYPE_SAMPLER1DARRAYSHADOW:
        genCode = _GenShadow1DArrayCode;
        break;

    case slvTYPE_SAMPLER2DARRAYSHADOW:
        genCode = _GenShadow2DArrayCode;
        break;

    case slvTYPE_SAMPLER2DSHADOW:
    case slvTYPE_SAMPLERCUBESHADOW:
    case slvTYPE_SAMPLERCUBEARRAYSHADOW:
    case slvTYPE_SAMPLER1DSHADOW:
    case slvTYPE_SAMPLER2DRECTSHADOW:
        genCode = _GenTextureShadowCode;
        break;

    default:
        gcmASSERT(0);
        break;
    }

    if(!genCode)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    status = (*genCode)(Compiler,
                        CodeGenerator,
                        PolynaryExpr,
                        OperandCount,
                        OperandsParameters,
                        IOperand);
    gcmFOOTER();
    return status;
}

gceSTATUS
_GenTextureLodCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    sloIR_EXPR  expr;
    sltBUILT_IN_GEN_CODE_FUNC_PTR   genCode = gcvNULL;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2 || OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    gcmASSERT(!slsDLINK_LIST_IsEmpty(&PolynaryExpr->operands->members));

    expr = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _sloIR_EXPR);

    switch(expr->dataType->elementType)
    {
    case slvTYPE_SAMPLER1D:
    case slvTYPE_ISAMPLER1D:
    case slvTYPE_USAMPLER1D:
        genCode = _GenTexture1DLodCode;
        break;

    case slvTYPE_SAMPLER2D:
    case slvTYPE_ISAMPLER2D:
    case slvTYPE_USAMPLER2D:
        genCode = _GenTexture2DLodCode;
        break;

    case slvTYPE_SAMPLER3D:
    case slvTYPE_ISAMPLER3D:
    case slvTYPE_USAMPLER3D:
        genCode = _GenTexture3DLodCode;
        break;

    case slvTYPE_SAMPLERCUBE:
    case slvTYPE_SAMPLERCUBEARRAY:
    case slvTYPE_ISAMPLERCUBE:
    case slvTYPE_ISAMPLERCUBEARRAY:
    case slvTYPE_USAMPLERCUBE:
    case slvTYPE_USAMPLERCUBEARRAY:
        genCode = _GenTextureCubeLodCode;
        break;

    case slvTYPE_SAMPLER1DARRAY:
    case slvTYPE_ISAMPLER1DARRAY:
    case slvTYPE_USAMPLER1DARRAY:
        genCode = _GenTexture1DArrayLodCode;
        break;

    case slvTYPE_SAMPLER2DARRAY:
    case slvTYPE_ISAMPLER2DARRAY:
    case slvTYPE_USAMPLER2DARRAY:
        genCode = _GenTexture2DArrayLodCode;
        break;

    case slvTYPE_SAMPLER1DARRAYSHADOW:
        genCode = _GenShadow1DArrayLodCode;
        break;

    case slvTYPE_SAMPLER2DSHADOW:
    case slvTYPE_SAMPLER1DSHADOW:
        genCode = _GenTextureShadowLodCode;
        break;

    default:
        gcmASSERT(0);
        break;
    }

    if(!genCode)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    status = (*genCode)(Compiler,
                        CodeGenerator,
                        PolynaryExpr,
                        OperandCount,
                        OperandsParameters,
                        IOperand);
    gcmFOOTER();
    return status;
}

gceSTATUS
_GenTextureProjCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    sloIR_EXPR  expr;
    sltBUILT_IN_GEN_CODE_FUNC_PTR   genCode = gcvNULL;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2 || OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    gcmASSERT(!slsDLINK_LIST_IsEmpty(&PolynaryExpr->operands->members));

    expr = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _sloIR_EXPR);
    switch(expr->dataType->elementType)
    {
    case slvTYPE_SAMPLER2D:
    case slvTYPE_ISAMPLER2D:
    case slvTYPE_USAMPLER2D:
    case slvTYPE_SAMPLEREXTERNALOES:
        genCode = _GenTexture2DProjCode;
        break;

    case slvTYPE_SAMPLER3D:
    case slvTYPE_ISAMPLER3D:
    case slvTYPE_USAMPLER3D:
        genCode = _GenTexture3DProjCode;
        break;

    case slvTYPE_SAMPLER2DSHADOW:
    case slvTYPE_SAMPLER2DRECTSHADOW:
        genCode = _GenTextureShadowProjCode;
        break;

    default:
        gcmASSERT(0);
        break;
    }

    if(!genCode)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_INVALID_ARGUMENT;
    }
    status = (*genCode)(Compiler,
                        CodeGenerator,
                        PolynaryExpr,
                        OperandCount,
                        OperandsParameters,
                        IOperand);
    gcmFOOTER();
    return status;
}

gceSTATUS
_GenTextureProjLodCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    sloIR_EXPR  expr;
    sltBUILT_IN_GEN_CODE_FUNC_PTR   genCode = gcvNULL;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2 || OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    gcmASSERT(!slsDLINK_LIST_IsEmpty(&PolynaryExpr->operands->members));

    expr = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _sloIR_EXPR);
    switch(expr->dataType->elementType)
    {
    case slvTYPE_SAMPLER2D:
    case slvTYPE_ISAMPLER2D:
    case slvTYPE_USAMPLER2D:
        genCode = _GenTexture2DProjLodCode;
        break;

    case slvTYPE_SAMPLER3D:
    case slvTYPE_ISAMPLER3D:
    case slvTYPE_USAMPLER3D:
        genCode = _GenTexture3DProjLodCode;
        break;

    case slvTYPE_SAMPLER2DSHADOW:
    case slvTYPE_SAMPLER1DSHADOW:
        genCode = _GenTextureShadowProjLodCode;
        break;

    default:
        gcmASSERT(0);
        break;
    }

    if(!genCode)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_INVALID_ARGUMENT;
    }
    status = (*genCode)(Compiler,
                        CodeGenerator,
                        PolynaryExpr,
                        OperandCount,
                        OperandsParameters,
                        IOperand);
    gcmFOOTER();
    return status;
}

static gctUINT8
_GetSamplerCoordComponentCount(
    IN slsDATA_TYPE * SamplerDataType
    )
{
    gcmASSERT(SamplerDataType);

    switch (SamplerDataType->elementType)
    {
    case slvTYPE_SAMPLER2D:
    case slvTYPE_SAMPLERCUBE:
    case slvTYPE_SAMPLER2DRECT:
    case slvTYPE_SAMPLER2DSHADOW:
    case slvTYPE_SAMPLER2DRECTSHADOW:
    case slvTYPE_SAMPLER2DARRAY:
    case slvTYPE_SAMPLER2DARRAYSHADOW:
    case slvTYPE_SAMPLERCUBESHADOW:
    case slvTYPE_ISAMPLERCUBE:
    case slvTYPE_ISAMPLER2D:
    case slvTYPE_ISAMPLER2DARRAY:
    case slvTYPE_USAMPLERCUBE:
    case slvTYPE_USAMPLER2D:
    case slvTYPE_USAMPLER2DARRAY:
    case slvTYPE_SAMPLER2DMS:
    case slvTYPE_ISAMPLER2DMS:
    case slvTYPE_USAMPLER2DMS:
    case slvTYPE_SAMPLER2DMSARRAY:
    case slvTYPE_ISAMPLER2DMSARRAY:
    case slvTYPE_USAMPLER2DMSARRAY:
        return 2;

    case slvTYPE_SAMPLER1DARRAY:
    case slvTYPE_SAMPLER1DARRAYSHADOW:
    case slvTYPE_SAMPLER1D:
    case slvTYPE_ISAMPLER1D:
    case slvTYPE_USAMPLER1D:
    case slvTYPE_SAMPLER1DSHADOW:
        return 1;

    case slvTYPE_SAMPLER3D:
    case slvTYPE_ISAMPLER3D:
    case slvTYPE_USAMPLER3D:
        return 3;

    case slvTYPE_SAMPLEREXTERNALOES:
        return 2;

    default:
        gcmASSERT(0);
        return 2;
    }
}

static gceSTATUS
_GenClampCodeInner(
    IN sloCOMPILER Compiler,
    IN gctINT LineNo,
    IN gctINT StringNo,
    IN slsROPERAND *Clamp,
    IN slsROPERAND *MinVal,
    IN slsROPERAND *MaxVal,
    IN slsIOPERAND *IOperand
    );

static gceSTATUS
_GenClampLod(
    IN sloCOMPILER Compiler,
    IN gctINT LineNo,
    IN gctINT StringNo,
    IN sloIR_EXPR SamplerOperand,
    IN slsROPERAND * SamplerROperand,
    IN slsROPERAND * Lod
    )
{
    gceSTATUS status;
    slsROPERAND intermROperand[1];
    slsROPERAND minLevel[1];
    slsROPERAND maxLevel[1];
    slsIOPERAND intermIOperand[1];
    slsNAME *sampler;

    gcmHEADER();

    gcmASSERT(sloIR_OBJECT_GetType(&SamplerOperand->base) == slvIR_VARIABLE);
    sampler = ((sloIR_VARIABLE)SamplerOperand)->name;
    if(sampler->type == slvVARIABLE_NAME)
    {
        gcUNIFORM lodMinMax = gcvNULL;
        slsLOGICAL_REG logicalReg[1];

        if(sampler->u.variableInfo.lodMinMax == gcvNULL)
        {
            status = slAllocSamplerLodMinMax(Compiler,
                                        sampler);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        }

        lodMinMax = sampler->u.variableInfo.lodMinMax;
        gcmASSERT(lodMinMax);

        slsLOGICAL_REG_InitializeUniform(logicalReg,
                                         slvSTORAGE_QUALIFIER_UNIFORM,
                                         lodMinMax->u.type,
                                         gcSHADER_PRECISION_MEDIUM,
                                         lodMinMax,
                                         0);
        slsROPERAND_InitializeReg(intermROperand, logicalReg);
    }
    else
    {
        gcSHADER shader = gcvNULL;

        slsIOPERAND_New(Compiler, intermIOperand, gcSHADER_FLOAT_X3, gcSHADER_PRECISION_MEDIUM);

        status = slGenGenericCode1(Compiler,
                                   SamplerOperand->base.lineNo,
                                   SamplerOperand->base.stringNo,
                                   slvOPCODE_GET_SAMPLER_LMM,
                                   intermIOperand,
                                   SamplerROperand);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        sloCOMPILER_GetBinary(Compiler, &shader);
        SetFunctionUsingSamplerVirtual(shader->currentFunction);
        slsROPERAND_InitializeUsingIOperand(intermROperand, intermIOperand);
    }
    slGetVectorROperandSlice(intermROperand,
                             0,
                             1,
                             minLevel);
    slGetVectorROperandSlice(intermROperand,
                             1,
                             1,
                             maxLevel);

    status = slsROPERAND_ChangeDataTypeFamily(Compiler,
                                              SamplerOperand->base.lineNo,
                                              SamplerOperand->base.stringNo,
                                              gcvFALSE,
                                              gcSHADER_INTEGER_X1,
                                              minLevel);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = slsROPERAND_ChangeDataTypeFamily(Compiler,
                                              SamplerOperand->base.lineNo,
                                              SamplerOperand->base.stringNo,
                                              gcvFALSE,
                                              gcSHADER_INTEGER_X1,
                                              maxLevel);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsIOPERAND_New(Compiler, intermIOperand, gcSHADER_INTEGER_X1, Lod->u.reg.precision);

    status = _GenClampCodeInner(Compiler,
                                SamplerOperand->base.lineNo,
                                SamplerOperand->base.stringNo,
                                Lod,
                                minLevel,
                                maxLevel,
                                intermIOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsROPERAND_InitializeUsingIOperand(Lod, intermIOperand);

    gcmFOOTER();
    return status;
}

static gceSTATUS
_GenSamplerSizeCode(
    IN sloCOMPILER Compiler,
    IN gctINT LineNo,
    IN gctINT StringNo,
    IN sloIR_EXPR SamplerOperand,
    IN slsROPERAND * SamplerROperand,
    IN slsROPERAND * Lod,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    slsNAME *sampler;
    slsROPERAND rOperand[1];
    gctUINT8 numCoordComponents, numComponents;
    slsIOPERAND intermIOperandLBS[1];
    slsIOPERAND intermIOperandMax[1];
    slsROPERAND constantOne[1], intermROperandMax[1];

    gcmHEADER();

    gcmASSERT(sloIR_OBJECT_GetType(&SamplerOperand->base) == slvIR_VARIABLE);
    sampler = ((sloIR_VARIABLE)SamplerOperand)->name;

    if(sampler->type == slvVARIABLE_NAME)
    {
        slsLOGICAL_REG logicalReg[1];
        if(sampler->u.variableInfo.levelBaseSize == gcvNULL)
        {
            status = slAllocSamplerLevelBaseSize(Compiler,
                                                 sampler,
                                                 gcSHADER_FLOAT_X4);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        }
        gcmASSERT(sampler->u.variableInfo.levelBaseSize);

        slsLOGICAL_REG_InitializeUniform(logicalReg,
                                         slvSTORAGE_QUALIFIER_UNIFORM,
                                         IOperand->dataType,
                                         gcSHADER_PRECISION_MEDIUM,
                                         sampler->u.variableInfo.levelBaseSize,
                                         0);
        slsROPERAND_InitializeReg(rOperand, logicalReg);
    }
    else
    {
        gcSHADER shader = gcvNULL;

        slsIOPERAND_New(Compiler, intermIOperandLBS, gcSHADER_INTEGER_X4, gcSHADER_PRECISION_MEDIUM);

        status = slGenGenericCode1(Compiler,
                                   SamplerOperand->base.lineNo,
                                   SamplerOperand->base.stringNo,
                                   slvOPCODE_GET_SAMPLER_LBS,
                                   intermIOperandLBS,
                                   SamplerROperand);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        sloCOMPILER_GetBinary(Compiler, &shader);
        SetFunctionUsingSamplerVirtual(shader->currentFunction);
        slsROPERAND_InitializeUsingIOperand(rOperand, intermIOperandLBS);
    }

    numCoordComponents = _GetSamplerCoordComponentCount(SamplerOperand->dataType);
    numComponents = gcGetDataTypeComponentCount(IOperand->dataType);

    slsIOPERAND_New(Compiler,
                    intermIOperandMax,
                    IOperand->dataType,
                    rOperand->u.reg.precision);

    if(numCoordComponents == numComponents)
    {
        status = slGenGenericCode2(Compiler,
                                   LineNo,
                                   StringNo,
                                   slvOPCODE_RSHIFT,
                                   intermIOperandMax,
                                   rOperand,
                                   Lod);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else
    {
        slsROPERAND intermROperand[1];
        slsIOPERAND intermIOperand[1];
        slsLOPERAND intermLOperand[1], lOperand[1];

        slGetVectorROperandSlice(rOperand,
                                 0,
                                 numCoordComponents,
                                 intermROperand);

        slsIOPERAND_New(Compiler, intermIOperand, intermROperand->dataType, intermROperand->u.reg.precision);

        status = slGenGenericCode2(Compiler,
                                   LineNo,
                                   StringNo,
                                   slvOPCODE_RSHIFT,
                                   intermIOperand,
                                   intermROperand,
                                   Lod);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        slsLOPERAND_InitializeUsingIOperand(lOperand, intermIOperandMax);
        slGetVectorLOperandSlice(lOperand,
                                 0,
                                 numCoordComponents,
                                 intermLOperand);

        slsROPERAND_InitializeUsingIOperand(intermROperand, intermIOperand);
        status = slGenAssignCode(Compiler,
                                 LineNo,
                                 StringNo,
                                 intermLOperand,
                                 intermROperand);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        slGetVectorROperandSlice(rOperand,
                                 numCoordComponents,
                                 numComponents - numCoordComponents,
                                 intermROperand);

        slGetVectorLOperandSlice(lOperand,
                                 numCoordComponents,
                                 numComponents - numCoordComponents,
                                 intermLOperand);

        status = slGenAssignCode(Compiler,
                                 LineNo,
                                 StringNo,
                                 intermLOperand,
                                 intermROperand);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    slsROPERAND_InitializeIntOrIVecConstant(constantOne,
                                            gcSHADER_INTEGER_X1,
                                            gcSHADER_PRECISION_MEDIUM,
                                            (gctINT) 1);

    slsROPERAND_InitializeUsingIOperand(intermROperandMax, intermIOperandMax);

    status = slGenGenericCode2(Compiler,
                               LineNo,
                               StringNo,
                               slvOPCODE_MAX,
                               IOperand,
                               intermROperandMax,
                               constantOne);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER();
    return status;
}

#define _LOG2_E             ((float)1.44269504088896340736)
#define _RCP_OF_LOG2_E      ((float)0.69314718055994530942)

static gceSTATUS
_ComputeLog(
    IN sloCOMPILER Compiler,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN slsROPERAND *LogArg,
    IN slsIOPERAND *IOperand
    )
{
    gceSTATUS status;
    slsIOPERAND intermIOperand[1];
    slsROPERAND intermROperand[1];
    slsROPERAND constantROperand[1];

    gcmHEADER();
    /* log2 t0, x */
    slsIOPERAND_New(Compiler, intermIOperand, LogArg->dataType, LogArg->u.reg.precision);

    status = slGenGenericCode1(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_LOG2,
                               intermIOperand,
                               LogArg);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* mul result, t0, _RCP_OF_LOG2_E */
    slsROPERAND_InitializeFloatOrVecOrMatConstant(constantROperand,
                                                  gcSHADER_FLOAT_X1,
                                                  gcSHADER_PRECISION_HIGH,
                                                  _RCP_OF_LOG2_E);

    slsROPERAND_InitializeUsingIOperand(intermROperand, intermIOperand);

    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    IOperand,
                                    intermROperand,
                                    constantROperand);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER();
    return status;
}

static gceSTATUS
_GenRoundCodeInner(
    IN sloCOMPILER Compiler,
    IN gctINT LineNo,
    IN gctINT StringNo,
    IN slsROPERAND *Round,
    IN slsIOPERAND *IOperand
    );

static gceSTATUS
_ComputeLod(
    IN sloCOMPILER Compiler,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctBOOL Is3D,
    IN slsROPERAND *Dx,
    IN slsROPERAND *Dy,
    IN slsROPERAND *Bias,
    IN OUT slsROPERAND *Lod
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    slsROPERAND absValDx[1], absValDy[1];
    slsROPERAND maxVal[1];
    slsIOPERAND intermIOperand[3];
    slsROPERAND minLevel[1];
    slsROPERAND maxLevel[1];
    slsROPERAND flod[1];
    slsROPERAND ilod[1];
    slsROPERAND cond[1];
    slsROPERAND floorLod[1];
    slsROPERAND tROperand[1];
    slsROPERAND fROperand[1];
    slsLOGICAL_REG logicalReg[1];
    slsROPERAND intermROperand[1];
    sloIR_EXPR samplerOperand;
    slsNAME *sampler;
    gcUNIFORM lodMinMax = gcvNULL;
    slsROPERAND addedExpROperand[1];
    slsROPERAND maxShortUIntROperand[1];
    slsROPERAND topBitofShortIntROperand[1];
    slsROPERAND hiIntBitsROperand[1];
    slsROPERAND n32ROperand[1];
    slsROPERAND dot5[1];
    gcmHEADER();

    gcmASSERT(PolynaryExpr->operands);
    samplerOperand = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _sloIR_EXPR);
    gcmASSERT(samplerOperand);

    gcmASSERT(sloIR_OBJECT_GetType(&samplerOperand->base) == slvIR_VARIABLE);
    sampler = ((sloIR_VARIABLE)samplerOperand)->name;
    gcmASSERT(sampler->type == slvVARIABLE_NAME);

    if(sampler->type == slvVARIABLE_NAME &&
       sampler->u.variableInfo.lodMinMax == gcvNULL)
    {
        status = slAllocSamplerLodMinMax(Compiler,
                                         sampler);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }


    slsIOPERAND_New(Compiler,
                    intermIOperand,
                    Dx->dataType,
                    Dx->u.reg.precision);
    slsROPERAND_InitializeUsingIOperand(absValDx, intermIOperand);

    status = slGenGenericCode2( Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                slvOPCODE_DOT,
                                intermIOperand,
                                Dx,
                                Dx);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsIOPERAND_New(Compiler,
                    intermIOperand,
                    Dy->dataType,
                    Dy->u.reg.precision);
    slsROPERAND_InitializeUsingIOperand(absValDy, intermIOperand);

    status = slGenGenericCode2( Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                slvOPCODE_DOT,
                                intermIOperand,
                                Dy,
                                Dy);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsIOPERAND_New(Compiler,
                    intermIOperand,
                    Dx->dataType,
                    Dx->u.reg.precision);
    slsROPERAND_InitializeUsingIOperand(maxVal, intermIOperand);

    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_MAX,
                               intermIOperand,
                               absValDx,
                               absValDy);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsROPERAND_InitializeUsingIOperand(maxVal, intermIOperand);
    slsIOPERAND_New(Compiler, intermIOperand, gcSHADER_FLOAT_X1, maxVal->u.reg.precision);

    status = slGenGenericCode1(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_LOG2,
                               intermIOperand,
                               maxVal);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }


    slsROPERAND_InitializeFloatOrVecOrMatConstant(dot5,
                                                  gcSHADER_FLOAT_X1,
                                                  Dx->u.reg.precision,
                                                  0.5);
    slsROPERAND_InitializeUsingIOperand(maxVal, intermIOperand);
    status = slGenArithmeticExprCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    intermIOperand,
                                    maxVal,
                                    dot5);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsROPERAND_InitializeFloatOrVecOrMatConstant(addedExpROperand,
                                                  gcSHADER_FLOAT_X1,
                                                  Dx->u.reg.precision,
                                                  393216.0);
    slsROPERAND_InitializeTempReg(
                                (ilod),
                                slvSTORAGE_QUALIFIER_NONE,
                                gcSHADER_FLOAT_X1,
                                (intermIOperand)->precision,
                                (intermIOperand)->tempRegIndex);

    slsIOPERAND_New(Compiler, intermIOperand, gcSHADER_FLOAT_X1, maxVal->u.reg.precision);
    status = slGenArithmeticExprCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_ADD,
                                    intermIOperand,
                                    ilod,
                                    addedExpROperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsROPERAND_InitializeIntOrIVecConstant(maxShortUIntROperand,
                                                  gcSHADER_UINT_X1,
                                                  Dx->u.reg.precision,
                                                  0xffff);
    slsROPERAND_InitializeUsingIOperand(ilod, intermIOperand);
    slsIOPERAND_New(Compiler, intermIOperand, gcSHADER_UINT_X1, maxVal->u.reg.precision);
    status = slGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 slvOPCODE_AND_BITWISE,
                                 intermIOperand,
                                 ilod,
                                 maxShortUIntROperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsROPERAND_InitializeIntOrIVecConstant(topBitofShortIntROperand,
                                                  gcSHADER_UINT_X1,
                                                  Dx->u.reg.precision,
                                                  0x8000);
    slsROPERAND_InitializeUsingIOperand(ilod, intermIOperand);
    slsIOPERAND_New(Compiler, &intermIOperand[1], gcSHADER_UINT_X1, maxVal->u.reg.precision);
    status = slGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 slvOPCODE_AND_BITWISE,
                                 &intermIOperand[1],
                                 ilod,
                                 topBitofShortIntROperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsROPERAND_InitializeIntOrIVecConstant(hiIntBitsROperand,
                                                  gcSHADER_UINT_X1,
                                                  Dx->u.reg.precision,
                                                  0xffff0000);
    slsROPERAND_InitializeUsingIOperand(ilod, intermIOperand);
    slsIOPERAND_New(Compiler, &intermIOperand[2], gcSHADER_UINT_X1, maxVal->u.reg.precision);
    status = slGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 slvOPCODE_OR_BITWISE,
                                 &intermIOperand[2],
                                 ilod,
                                 hiIntBitsROperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsROPERAND_InitializeTempReg(
                                (cond),
                                slvSTORAGE_QUALIFIER_NONE,
                                gcSHADER_BOOLEAN_X1,
                                (&intermIOperand[1])->precision,
                                (&intermIOperand[1])->tempRegIndex);
    slsROPERAND_InitializeUsingIOperand(fROperand, &intermIOperand[2]);
    slsROPERAND_InitializeUsingIOperand(tROperand, &intermIOperand[0]);
    status = slGenSelectExprCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                intermIOperand,
                                cond,
                                tROperand,
                                fROperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsROPERAND_InitializeUsingIOperand(ilod, intermIOperand);
    slsIOPERAND_New(Compiler, intermIOperand, gcSHADER_FLOAT_X1, maxVal->u.reg.precision);
    status = slGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 slvOPCODE_UINT_TO_FLOAT,
                                 intermIOperand,
                                 ilod);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsROPERAND_InitializeFloatOrVecOrMatConstant(n32ROperand,
                                                  gcSHADER_FLOAT_X1,
                                                  Dx->u.reg.precision,
                                                  32.0);
    slsROPERAND_InitializeTempReg(
                                (flod),
                                slvSTORAGE_QUALIFIER_NONE,
                                gcSHADER_FLOAT_X1,
                                (intermIOperand)->precision,
                                (intermIOperand)->tempRegIndex);
    slsIOPERAND_New(Compiler, intermIOperand, gcSHADER_FLOAT_X1, maxVal->u.reg.precision);
    status = slGenArithmeticExprCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     slvOPCODE_DIV,
                                     intermIOperand,
                                     flod,
                                     n32ROperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* round the value. */
    slsROPERAND_InitializeUsingIOperand(floorLod, intermIOperand);
    slsIOPERAND_New(Compiler, intermIOperand, gcSHADER_FLOAT_X1, maxVal->u.reg.precision);
    status = slGenArithmeticExprCode(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_ADD,
                               intermIOperand,
                               floorLod,
                               dot5);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* round the value. */
    slsROPERAND_InitializeUsingIOperand(floorLod, intermIOperand);
    slsIOPERAND_New(Compiler, intermIOperand, gcSHADER_FLOAT_X1, maxVal->u.reg.precision);
    status = slGenGenericCode1(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_FLOOR,
                               intermIOperand,
                               floorLod);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsROPERAND_InitializeUsingIOperand(Lod, intermIOperand);

    /* add bias if exists */
    if(Bias)
    {
        slsROPERAND minLodBias[1];
        slsROPERAND maxLodBias[1];
        slsROPERAND clampedBias[1];

        slsIOPERAND_New(Compiler, intermIOperand, gcSHADER_FLOAT_X1, Bias->u.reg.precision);
        slsROPERAND_InitializeFloatOrVecOrMatConstant(minLodBias,
                                                      gcSHADER_FLOAT_X1,
                                                      gcSHADER_PRECISION_MEDIUM,
                                                      (float)(gcdSL_MIN_TEXTURE_LOD_BIAS));
        slsROPERAND_InitializeFloatOrVecOrMatConstant(maxLodBias,
                                                      gcSHADER_FLOAT_X1,
                                                      gcSHADER_PRECISION_MEDIUM,
                                                      (float)gcdSL_MAX_TEXTURE_LOD_BIAS);
        status = _GenClampCodeInner(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    Bias,
                                    minLodBias,
                                    maxLodBias,
                                    intermIOperand);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        slsROPERAND_InitializeUsingIOperand(clampedBias, intermIOperand);
        slsIOPERAND_New(Compiler, intermIOperand, gcSHADER_FLOAT_X1, Lod->u.reg.precision);
        status = slGenArithmeticExprCode(Compiler,
                                         PolynaryExpr->exprBase.base.lineNo,
                                         PolynaryExpr->exprBase.base.stringNo,
                                         slvOPCODE_ADD,
                                         intermIOperand,
                                         Lod,
                                         clampedBias);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        slsROPERAND_InitializeUsingIOperand(Lod, intermIOperand);
    }

    slsIOPERAND_New(Compiler, intermIOperand, gcSHADER_FLOAT_X1, Lod->u.reg.precision);
    if(sampler->type == slvVARIABLE_NAME)
    {
        lodMinMax = sampler->u.variableInfo.lodMinMax;
    }
    gcmASSERT(lodMinMax);
    slsLOGICAL_REG_InitializeUniform(logicalReg,
                                     slvSTORAGE_QUALIFIER_UNIFORM,
                                     lodMinMax->u.type,
                                     gcSHADER_PRECISION_MEDIUM,
                                     lodMinMax,
                                     0);
    slsROPERAND_InitializeReg(intermROperand, logicalReg);
    slGetVectorROperandSlice(intermROperand,
                             0,
                             1,
                             minLevel);
    slGetVectorROperandSlice(intermROperand,
                             1,
                             1,
                             maxLevel);
    status = _GenClampCodeInner(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                Lod,
                                minLevel,
                                maxLevel,
                                intermIOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsROPERAND_InitializeUsingIOperand(Lod, intermIOperand);
    gcmFOOTER();
    return status;
}

#define _DO_MIPMAPPED_CHECK   0

static gceSTATUS
_GetMaxComponentFromCoord(
    IN sloCOMPILER Compiler,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN slsROPERAND *Coord,
    IN gctINT CoordCount,
    OUT slsIOPERAND *MaxCoord
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    slsROPERAND sliceComponentROperand[1] = {{0}};
    slsIOPERAND sliceComponentIOperand[1] = {{0}};
    slsROPERAND maxComponentROperand[1] = {{0}};
    slsIOPERAND maxComponentIOperand[1] = {{0}};
    gctINT i;

    for (i = 0; i < CoordCount; i++)
    {
        slGetVectorROperandSlice(Coord,
                                 (gctUINT8)i,
                                 1,
                                 sliceComponentROperand);

        slsIOPERAND_New(Compiler,
                        sliceComponentIOperand,
                        sliceComponentROperand->dataType,
                        sliceComponentROperand->u.reg.precision);

        status = slGenGenericCode1(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   slvOPCODE_ABS,
                                   sliceComponentIOperand,
                                   sliceComponentROperand);
        if (gcmIS_ERROR(status)) { return status; }

        if (i == 0)
        {
            maxComponentIOperand[0] = sliceComponentIOperand[0];
        }
        else
        {
            slsROPERAND intermROperand[1];
            slsIOPERAND intermIOperand[1];

            slsROPERAND_InitializeUsingIOperand(intermROperand, sliceComponentIOperand);
            slsROPERAND_InitializeUsingIOperand(maxComponentROperand,
                                                maxComponentIOperand);
            slsIOPERAND_New(Compiler,
                            intermIOperand,
                            sliceComponentIOperand->dataType,
                            intermROperand->u.reg.precision < maxComponentROperand->u.reg.precision ? intermROperand->u.reg.precision : maxComponentROperand->u.reg.precision);

            status = slGenGenericCode2(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       slvOPCODE_MAX,
                                       intermIOperand,
                                       intermROperand,
                                       maxComponentROperand);
            if (gcmIS_ERROR(status)) { return status; }

            maxComponentIOperand[0] = intermIOperand[0];
        }
    }

    if (MaxCoord)
    {
        *MaxCoord = maxComponentIOperand[0];
    }

    return status;
}

static gceSTATUS
_ComputeLodByTextureGrad(
    IN sloCOMPILER Compiler,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctBOOL NeedClamping,
    IN gctBOOL Is3D,
    IN slsROPERAND *Coord,
    IN slsROPERAND *Dx,
    IN slsROPERAND *Dy,
    IN OUT slsROPERAND *Lod
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    slsROPERAND absValDx[1], absValDy[1];
    slsROPERAND maxVal[1], max1[1], max2[1];
    slsIOPERAND intermIOperand[1];
    slsLOGICAL_REG logicalReg[1];
    slsLOGICAL_REG sizeReg[1];
    slsROPERAND sizeOperand[1];
    slsROPERAND intermROperand[1];
    slsLOPERAND intermLOperand[1];
    slsIOPERAND intermLod[1];
    slsROPERAND zeroROperand[1];
    slsROPERAND isMipMapped[1];
    slsROPERAND dx[1], dy[1];
    sloIR_EXPR samplerOperand;
    slsNAME *sampler;
    gcUNIFORM lodMinMax = gcvNULL;
/*klc*/
#if _DO_MIPMAPPED_CHECK
    sloCODE_GENERATOR codeGenerator = gcvNULL;
#endif
    gctUINT8 numCoordComponents;
    gctBOOL isCubeSampler = gcvFALSE;
    gcSHADER_TYPE ty;

    gcmHEADER();

    gcmASSERT(PolynaryExpr->operands);

    samplerOperand = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _sloIR_EXPR);
    gcmASSERT(samplerOperand);

    gcmASSERT(sloIR_OBJECT_GetType(&samplerOperand->base) == slvIR_VARIABLE);
    sampler = ((sloIR_VARIABLE)samplerOperand)->name;
    gcmASSERT(sampler->type == slvVARIABLE_NAME);

    /* Create lodMinMax and levelBaseSize uniforms if needed. */
    numCoordComponents = gcGetDataTypeComponentCount(Dx->dataType);
    if(sampler->type == slvVARIABLE_NAME)
    {
        if(sampler->u.variableInfo.lodMinMax == gcvNULL)
        {
            status = slAllocSamplerLodMinMax(Compiler, sampler);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        }
        if(sampler->u.variableInfo.levelBaseSize == gcvNULL)
        {
            gctUINT8 sizeComponents;
            gcSHADER_TYPE dataType;

            sizeComponents = 3;
            if(slmIsElementTypeSamplerArray(sampler->dataType->elementType))
            {
                sizeComponents++;
            }
            dataType = gcConvScalarToVectorDataType(gcSHADER_INTEGER_X1, sizeComponents);
            status = slAllocSamplerLevelBaseSize(Compiler,
                                                 sampler,
                                                 dataType);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        }

        lodMinMax = sampler->u.variableInfo.lodMinMax;
    }

    gcmASSERT(lodMinMax);
    gcmASSERT(sampler->u.variableInfo.levelBaseSize);

    ty = gcConvScalarToVectorDataType(gcSHADER_INTEGER_X1, numCoordComponents);
    slsLOGICAL_REG_InitializeUniform(sizeReg,
                                     slvSTORAGE_QUALIFIER_UNIFORM,
                                     ty,
                                     gcSHADER_PRECISION_MEDIUM,
                                     sampler->u.variableInfo.levelBaseSize,
                                     0);
    slsROPERAND_InitializeReg(sizeOperand, sizeReg);

    status = slsROPERAND_ChangeDataTypeFamily(Compiler,
                                              PolynaryExpr->exprBase.base.lineNo,
                                              PolynaryExpr->exprBase.base.stringNo,
                                              gcvFALSE,
                                              gcChangeElementDataType(sizeOperand->dataType,
                                                                      gcSHADER_FLOAT_X1),
                                              sizeOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }


    if(numCoordComponents != gcGetDataTypeComponentCount(sizeOperand->dataType))
    {
        slGetVectorROperandSlice(sizeOperand,
                                 0,
                                 numCoordComponents,
                                 sizeOperand);
    }

    slsLOGICAL_REG_InitializeUniform(logicalReg,
                                     slvSTORAGE_QUALIFIER_UNIFORM,
                                     lodMinMax->u.type,
                                     gcSHADER_PRECISION_MEDIUM,
                                     lodMinMax,
                                     0);
    slsROPERAND_InitializeReg(isMipMapped, logicalReg);
    slGetVectorROperandSlice(isMipMapped,
                             2,
                             1,
                             isMipMapped);

    slsROPERAND_InitializeFloatOrVecOrMatConstant(zeroROperand,
                                                  gcSHADER_FLOAT_X1,
                                                  gcSHADER_PRECISION_MEDIUM,
                                                  0.0);

    slsIOPERAND_New(Compiler,
                    intermLod,
                    gcSHADER_FLOAT_X1,
                    gcSHADER_PRECISION_MEDIUM);
    slsLOPERAND_InitializeUsingIOperand(intermLOperand, intermLod);

/*klc*/
#if _DO_MIPMAPPED_CHECK
    codeGenerator = Compiler->codeGenerator;
    slmGEN_CODE_IF(Compiler,
                   codeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   isMipMapped,
                   slvCONDITION_NOT_EQUAL,
                   zeroROperand);
#endif

    /* Get the max Dx/Dy. */
    slsIOPERAND_New(Compiler,
                    intermIOperand,
                    Dx->dataType,
                    Dx->u.reg.precision);
    status = slGenArithmeticExprCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     slvOPCODE_MUL,
                                     intermIOperand,
                                     Dx,
                                     sizeOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    slsROPERAND_InitializeUsingIOperand(dx, intermIOperand);

    slsIOPERAND_New(Compiler,
                    intermIOperand,
                    Dy->dataType,
                    Dy->u.reg.precision);
    status = slGenArithmeticExprCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     slvOPCODE_MUL,
                                     intermIOperand,
                                     Dy,
                                     sizeOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    slsROPERAND_InitializeUsingIOperand(dy, intermIOperand);

    slsIOPERAND_New(Compiler,
                    intermIOperand,
                    Dx->dataType,
                    Dx->u.reg.precision);
    slsROPERAND_InitializeUsingIOperand(absValDx, intermIOperand);

    status = slGenGenericCode1(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_ABS,
                               intermIOperand,
                               dx);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsIOPERAND_New(Compiler,
                    intermIOperand,
                    Dy->dataType,
                    Dy->u.reg.precision);
    slsROPERAND_InitializeUsingIOperand(absValDy, intermIOperand);

    status = slGenGenericCode1(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_ABS,
                               intermIOperand,
                               dy);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsIOPERAND_New(Compiler,
                    intermIOperand,
                    Dx->dataType,
                    Dx->u.reg.precision);

    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_MAX,
                               intermIOperand,
                               absValDx,
                               absValDy);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* If this is a cube sampler, we need to DIV the result by 2*max coord component */
    if (sampler->dataType->elementType == slvTYPE_SAMPLERCUBE ||
        sampler->dataType->elementType == slvTYPE_SAMPLERCUBESHADOW)
    {
        isCubeSampler = gcvTRUE;
    }

    if (isCubeSampler)
    {
        slsROPERAND tempROperand[1];
        slsROPERAND halfOneROperand[1];
        slsROPERAND maxCorrdComponentROperand[1];
        slsIOPERAND maxCoordComponentIOperand[1];
        slsIOPERAND intermTempIOperand[2];

        slsIOPERAND_New(Compiler,
                        &intermTempIOperand[0],
                        Dx->dataType,
                        Dx->u.reg.precision);
        slsIOPERAND_New(Compiler,
                        &intermTempIOperand[1],
                        Dx->dataType,
                        Dx->u.reg.precision);
        slsIOPERAND_New(Compiler,
                        maxCoordComponentIOperand,
                        gcSHADER_FLOAT_X1,
                        Dx->u.reg.precision);
        /* Get the max coord componenet. */
        status = _GetMaxComponentFromCoord(Compiler,
                                           PolynaryExpr,
                                           Coord,
                                           3,
                                           maxCoordComponentIOperand);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        slsROPERAND_InitializeUsingIOperand(maxCorrdComponentROperand, maxCoordComponentIOperand);
        /* DIV 2.0. */
        slsROPERAND_InitializeUsingIOperand(tempROperand, intermIOperand);
        slsROPERAND_InitializeFloatOrVecOrMatConstant(halfOneROperand,
                                                      gcSHADER_FLOAT_X1,
                                                      Dx->u.reg.precision,
                                                      0.5);
        status = slGenArithmeticExprCode(Compiler,
                                         PolynaryExpr->exprBase.base.lineNo,
                                         PolynaryExpr->exprBase.base.stringNo,
                                         slvOPCODE_MUL,
                                         &intermTempIOperand[0],
                                         tempROperand,
                                         halfOneROperand);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        /* DIV max coord component. */
        slsROPERAND_InitializeUsingIOperand(tempROperand, &intermTempIOperand[0]);
        status = slGenArithmeticExprCode(Compiler,
                                         PolynaryExpr->exprBase.base.lineNo,
                                         PolynaryExpr->exprBase.base.stringNo,
                                         slvOPCODE_DIV,
                                         &intermTempIOperand[1],
                                         tempROperand,
                                         maxCorrdComponentROperand);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        slsROPERAND_InitializeUsingIOperand(maxVal, &intermTempIOperand[1]);
    }
    else
    {
        slsROPERAND_InitializeUsingIOperand(maxVal, intermIOperand);
    }

    slGetVectorROperandSlice(maxVal,
                             0,
                             1,
                             max1);
    slGetVectorROperandSlice(maxVal,
                             1,
                             1,
                             max2);
    slsIOPERAND_New(Compiler,
                    intermIOperand,
                    gcSHADER_FLOAT_X1,
                    maxVal->u.reg.precision);

    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_MAX,
                               intermIOperand,
                               max1,
                               max2);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* 3D sampler */
    if(Is3D)
    {
        slsROPERAND_InitializeUsingIOperand(max1, intermIOperand);
        slsIOPERAND_New(Compiler,
                        intermIOperand,
                        gcSHADER_FLOAT_X1,
                        maxVal->u.reg.precision);
        slGetVectorROperandSlice(maxVal,
                                 2,
                                 1,
                                 max2);
        status = slGenGenericCode2(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   slvOPCODE_MAX,
                                   intermIOperand,
                                   max1,
                                   max2);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    slsROPERAND_InitializeUsingIOperand(maxVal, intermIOperand);
    slsIOPERAND_New(Compiler, intermIOperand, gcSHADER_FLOAT_X1, maxVal->u.reg.precision);

    status = slGenGenericCode1(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_LOG2,
                               intermIOperand,
                               maxVal);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsROPERAND_InitializeUsingIOperand(intermROperand, intermIOperand);
    gcmONERROR(slGenAssignCode(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               intermLOperand,
                               intermROperand));
/*klc*/
#if _DO_MIPMAPPED_CHECK
    slmGEN_CODE_ELSE(Compiler,
                     codeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);

        gcmONERROR(slGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   intermLOperand,
                                   zeroROperand));

    slmGEN_CODE_ENDIF(Compiler,
                      codeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);
#endif

    slsROPERAND_InitializeUsingIOperand(Lod, intermLod);

    if(NeedClamping)
    {
        slsROPERAND minLevel[1];
        slsROPERAND maxLevel[1];

        slsIOPERAND_New(Compiler, intermIOperand, gcSHADER_FLOAT_X1, Lod->u.reg.precision);
        slsROPERAND_InitializeReg(intermROperand, logicalReg);
        slGetVectorROperandSlice(intermROperand,
                                 0,
                                 1,
                                 minLevel);
        slGetVectorROperandSlice(intermROperand,
                                 1,
                                 1,
                                 maxLevel);
        status = _GenClampCodeInner(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    Lod,
                                    minLevel,
                                    maxLevel,
                                    intermIOperand);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        slsROPERAND_InitializeUsingIOperand(Lod, intermIOperand);
    }

OnError:
    gcmFOOTER();
    return status;
}

static gceSTATUS
_ComputeOffsetCoords(
    IN sloCOMPILER Compiler,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN slsROPERAND *Sampler,
    IN slsROPERAND *TextureCoords,
    IN slsROPERAND *Bias,
    IN slsROPERAND *Lod,
    IN slsROPERAND *Offset,
    IN OUT slsIOPERAND *OffsetCoords
    )
{
    gceSTATUS   status = gcvSTATUS_OK;
    sleSHADER_TYPE shaderType;
    slsROPERAND lodBuf[1];
    slsROPERAND *lod;
    slsROPERAND sizeOperand[1];
    slsROPERAND texCoords[1];
    slsROPERAND intermROperand[1];
    slsIOPERAND intermIOperand[1];
    slsLOPERAND intermLOperand[1], lOperand[1];
    sloIR_EXPR samplerOperand;
    gctUINT8 numCoordComponents, sizeComponents;
    gcSHADER_TYPE sizeElementType;
    slsNAME *samplerName;
    gcSHADER_TYPE floatType;
    gcSHADER_TYPE dataType;
    slsLOGICAL_REG logicalReg[1];
    slsROPERAND isMipMapped[1];
    gcUNIFORM lodMinMax = gcvNULL;

    gcmHEADER();

    /* compute texture size */
    shaderType = Compiler->shaderType;

    gcmASSERT(PolynaryExpr->operands);
    samplerOperand = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _sloIR_EXPR);
    gcmASSERT(samplerOperand);

    gcmASSERT(sloIR_OBJECT_GetType(&samplerOperand->base) == slvIR_VARIABLE);
    samplerName = ((sloIR_VARIABLE)samplerOperand)->name;
    gcmASSERT(samplerName->type == slvVARIABLE_NAME);

    numCoordComponents = gcGetDataTypeComponentCount(TextureCoords->dataType);

    if(samplerName->type == slvVARIABLE_NAME && samplerName->u.variableInfo.lodMinMax == gcvNULL)
    {
        status = slAllocSamplerLodMinMax(Compiler, samplerName);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    if(samplerName->type == slvVARIABLE_NAME)
    {
        lodMinMax = samplerName->u.variableInfo.lodMinMax;
    }
    gcmASSERT(lodMinMax);

    slsLOGICAL_REG_InitializeUniform(logicalReg,
                                     slvSTORAGE_QUALIFIER_UNIFORM,
                                     lodMinMax->u.type,
                                     gcSHADER_PRECISION_MEDIUM,
                                     lodMinMax,
                                     0);
    slsROPERAND_InitializeReg(isMipMapped, logicalReg);

    lod = lodBuf;
    if (Lod)
    {
        slsROPERAND minLevel[1];
        slsROPERAND maxLevel[1];

        slsIOPERAND_New(Compiler, intermIOperand, gcSHADER_FLOAT_X1, Lod->u.reg.precision);

        slGetVectorROperandSlice(isMipMapped,
                                 0,
                                 1,
                                 minLevel);
        slGetVectorROperandSlice(isMipMapped,
                                 1,
                                 1,
                                 maxLevel);
        status = _GenClampCodeInner(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    Lod,
                                    minLevel,
                                    maxLevel,
                                    intermIOperand);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        slsROPERAND_InitializeUsingIOperand(Lod, intermIOperand);
        slsIOPERAND_New(Compiler, intermIOperand, gcSHADER_FLOAT_X1, Lod->u.reg.precision);

        status = _GenRoundCodeInner(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    Lod,
                                    intermIOperand);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        slsROPERAND_InitializeUsingIOperand(Lod, intermIOperand);

        *lod = *Lod;
    }
    else
    {
        slsROPERAND integerZero[1];
        lod = lodBuf;

        slsROPERAND_InitializeIntOrIVecConstant(integerZero,
                                                gcSHADER_INTEGER_X1,
                                                gcSHADER_PRECISION_MEDIUM,
                                                (gctINT) 0);

        if (shaderType == slvSHADER_TYPE_FRAGMENT)
        {
            /* fragment shader */
            slsROPERAND dx[2], dy[2];
            slsROPERAND texCoords[1];
            slsROPERAND zeroROperand[1];
            sloCODE_GENERATOR codeGenerator;
            slsIOPERAND intermLod[1];
            gcUNIFORM levelBaseSize = gcvNULL;
            slsLOGICAL_REG logicalRegLevelBaseSize[1];
            slsROPERAND internRlevelBaseSize[1];
            gcSHADER_TYPE ty;

            if(samplerName->type == slvVARIABLE_NAME &&
                samplerName->u.variableInfo.levelBaseSize == gcvNULL)
            {
                gctUINT8 sizeComponents;
                gcSHADER_TYPE dataType;

                sizeComponents = 3;
                if(slmIsElementTypeSamplerArray(samplerName->dataType->elementType))
                {
                    sizeComponents++;
                }
                dataType = gcConvScalarToVectorDataType(gcSHADER_INTEGER_X1, sizeComponents);
                status = slAllocSamplerLevelBaseSize(Compiler,
                                                     samplerName,
                                                     dataType);
                if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
            }
            if(samplerName->type == slvVARIABLE_NAME)
            {
                levelBaseSize = samplerName->u.variableInfo.levelBaseSize;
            }
            gcmASSERT(levelBaseSize);

            slGetVectorROperandSlice(isMipMapped,
                                     2,
                                     1,
                                     isMipMapped);

            slsROPERAND_InitializeFloatOrVecOrMatConstant(zeroROperand,
                                                          gcSHADER_FLOAT_X1,
                                                          gcSHADER_PRECISION_MEDIUM,
                                                          0.0);

            slsIOPERAND_New(Compiler,
                            intermLod,
                            gcSHADER_INTEGER_X1,
                            gcSHADER_PRECISION_MEDIUM);
            codeGenerator = Compiler->codeGenerator;
            slmGEN_CODE_IF(Compiler,
                           codeGenerator,
                           PolynaryExpr->exprBase.base.lineNo,
                           PolynaryExpr->exprBase.base.stringNo,
                           isMipMapped,
                           slvCONDITION_NOT_EQUAL,
                           zeroROperand);

            slGetVectorROperandSlice(TextureCoords,
                                     0,
                                     numCoordComponents,
                                     texCoords);
           /* compute dx and dy */
            slsIOPERAND_New(Compiler,
                            intermIOperand,
                            texCoords->dataType,
                            texCoords->u.reg.precision);
            slsROPERAND_InitializeUsingIOperand(&dx[0], intermIOperand);

            status = slGenGenericCode1(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       slvOPCODE_DFDX,
                                       intermIOperand,
                                       texCoords);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

            slsIOPERAND_New(Compiler,
                            intermIOperand,
                            TextureCoords->dataType,
                            TextureCoords->u.reg.precision);
            slsROPERAND_InitializeUsingIOperand(&dy[0], intermIOperand);

            status = slGenGenericCode1(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       slvOPCODE_DFDY,
                                       intermIOperand,
                                       texCoords);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

            ty = gcConvScalarToVectorDataType(gcSHADER_INTEGER_X1, numCoordComponents);
            /* Multiply the derivatives with the base texture size. */
            slsLOGICAL_REG_InitializeUniform(logicalRegLevelBaseSize,
                                             slvSTORAGE_QUALIFIER_UNIFORM,
                                             ty,
                                             gcSHADER_PRECISION_MEDIUM,
                                             levelBaseSize,
                                             0);
            slsROPERAND_InitializeReg(internRlevelBaseSize, logicalRegLevelBaseSize);

            status = slsROPERAND_ChangeDataTypeFamily(Compiler,
                                                      PolynaryExpr->exprBase.base.lineNo,
                                                      PolynaryExpr->exprBase.base.stringNo,
                                                      gcvFALSE,
                                                      gcChangeElementDataType(internRlevelBaseSize->dataType,
                                                                              gcSHADER_FLOAT_X1),
                                                      internRlevelBaseSize);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

            if(numCoordComponents != gcGetDataTypeComponentCount(internRlevelBaseSize->dataType))
            {
                slGetVectorROperandSlice(internRlevelBaseSize,
                                         0,
                                         numCoordComponents,
                                         internRlevelBaseSize);
            }

            slsIOPERAND_New(Compiler,
                            intermIOperand,
                            TextureCoords->dataType,
                            TextureCoords->u.reg.precision);
            slsROPERAND_InitializeUsingIOperand(&dx[1], intermIOperand);

            status = slGenArithmeticExprCode(Compiler,
                                             PolynaryExpr->exprBase.base.lineNo,
                                             PolynaryExpr->exprBase.base.stringNo,
                                             slvOPCODE_MUL,
                                             intermIOperand,
                                             internRlevelBaseSize,
                                             &dx[0]);

            slsIOPERAND_New(Compiler,
                            intermIOperand,
                            TextureCoords->dataType,
                            TextureCoords->u.reg.precision);
            slsROPERAND_InitializeUsingIOperand(&dy[1], intermIOperand);

            status = slGenArithmeticExprCode(Compiler,
                                             PolynaryExpr->exprBase.base.lineNo,
                                             PolynaryExpr->exprBase.base.stringNo,
                                             slvOPCODE_MUL,
                                             intermIOperand,
                                             internRlevelBaseSize,
                                             &dy[0]);
            /* Compute lod value. */
            status = _ComputeLod(Compiler,
                                 PolynaryExpr,
                                 numCoordComponents > 2 ? gcvTRUE : gcvFALSE,
                                 &dx[1],
                                 &dy[1],
                                 Bias,
                                 lod);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

            status = slsROPERAND_ChangeDataTypeFamily(Compiler,
                                                      PolynaryExpr->exprBase.base.lineNo,
                                                      PolynaryExpr->exprBase.base.stringNo,
                                                      gcvFALSE,
                                                      gcSHADER_INTEGER_X1,
                                                      lod);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

            slsLOPERAND_InitializeUsingIOperand(intermLOperand, intermLod);
            gcmONERROR(slGenAssignCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       intermLOperand,
                                       lod));

            slmGEN_CODE_ELSE(Compiler,
                             codeGenerator,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo);

                slsLOPERAND_InitializeUsingIOperand(intermLOperand, intermLod);
                gcmONERROR(slGenAssignCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           intermLOperand,
                                           integerZero));

            slmGEN_CODE_ENDIF(Compiler,
                              codeGenerator,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo);

            slsROPERAND_InitializeUsingIOperand(lod, intermLod);
        }
        else
        {
            *lod = *integerZero;
        }
    }
    status = slsROPERAND_ChangeDataTypeFamily(Compiler,
                                              PolynaryExpr->exprBase.base.lineNo,
                                              PolynaryExpr->exprBase.base.stringNo,
                                              gcvFALSE,
                                              gcSHADER_INTEGER_X1,
                                              lod);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    numCoordComponents = gcGetDataTypeComponentCount(Offset->dataType);
    sizeComponents = numCoordComponents;
    if(slmIsElementTypeSamplerArray(samplerName->dataType->elementType))
    {
        sizeComponents++;
    }
    if (numCoordComponents > 1)
        sizeElementType = gcGetVectorComponentDataType(Offset->dataType);
    else
        sizeElementType = Offset->dataType;
    dataType = gcConvScalarToVectorDataType(sizeElementType, sizeComponents);
    slsIOPERAND_New(Compiler,
                    intermIOperand,
                    dataType,
                    gcSHADER_PRECISION_MEDIUM);

    status = _GenSamplerSizeCode(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 samplerOperand,
                                 Sampler,
                                 lod,
                                 intermIOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsROPERAND_InitializeUsingIOperand(sizeOperand, intermIOperand);

    status = slsROPERAND_ChangeDataTypeFamily(Compiler,
                                              PolynaryExpr->exprBase.base.lineNo,
                                              PolynaryExpr->exprBase.base.stringNo,
                                              gcvFALSE,
                                              gcChangeElementDataType(sizeOperand->dataType,
                                                                      gcSHADER_FLOAT_X1),
                                              sizeOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    floatType = gcChangeElementDataType(Offset->dataType,
                                        gcSHADER_FLOAT_X1);
    intermROperand[0] = *Offset;
    status = slsROPERAND_ChangeDataTypeFamily(Compiler,
                                              PolynaryExpr->exprBase.base.lineNo,
                                              PolynaryExpr->exprBase.base.stringNo,
                                              gcvFALSE,
                                              floatType,
                                              intermROperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slGetVectorROperandSlice(sizeOperand,
                             0,
                             gcGetVectorDataTypeComponentCount(floatType),
                             texCoords);
    slsIOPERAND_New(Compiler,
                    intermIOperand,
                    floatType,
                    OffsetCoords->precision);
    status = slGenArithmeticExprCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     slvOPCODE_DIV,
                                     intermIOperand,
                                     intermROperand,
                                     texCoords);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsROPERAND_InitializeUsingIOperand(intermROperand, intermIOperand);
    slsIOPERAND_New(Compiler, intermIOperand, floatType, OffsetCoords->precision);

    slGetVectorROperandSlice(TextureCoords,
                             0,
                             numCoordComponents,
                             texCoords);
    status = slGenArithmeticExprCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     slvOPCODE_ADD,
                                     intermIOperand,
                                     texCoords,
                                     intermROperand);

    slsROPERAND_InitializeUsingIOperand(texCoords, intermIOperand);
    slsLOPERAND_InitializeUsingIOperand(lOperand, OffsetCoords);
    status = slGenAssignCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             lOperand,
                             TextureCoords);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slGetVectorLOperandSlice(lOperand,
                             0,
                             numCoordComponents,
                             intermLOperand);

    status = slGenAssignCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             intermLOperand,
                             texCoords);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
_GenTextureOffsetCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status = gcvSTATUS_OK;
    slsGEN_CODE_PARAMETERS textureParameters[3];
    slsROPERAND rOperand[1];
    slsIOPERAND intermIOperand[1];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3 || OperandCount == 4);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    slsIOPERAND_New(Compiler,
                    intermIOperand,
                    OperandsParameters[1].dataTypes[0],
                    OperandsParameters[1].rOperands[0].u.reg.precision);
    status = _ComputeOffsetCoords(Compiler,
                                   PolynaryExpr,
                                   &OperandsParameters[0].rOperands[0],
                                   &OperandsParameters[1].rOperands[0],
                                   OperandCount == 4 ? &OperandsParameters[3].rOperands[0] : gcvNULL,
                                   gcvNULL,
                                   &OperandsParameters[2].rOperands[0],
                                   intermIOperand);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    textureParameters[0] = OperandsParameters[0];
    textureParameters[1] = OperandsParameters[1];
    slsROPERAND_InitializeUsingIOperand(rOperand, intermIOperand);
    textureParameters[1].dataTypes = &rOperand->dataType;
    textureParameters[1].rOperands = rOperand;

    if(OperandCount == 4) {
        textureParameters[2] = OperandsParameters[3];
    }

    status = _GenTextureCode(Compiler,
                             CodeGenerator,
                             PolynaryExpr,
                             OperandCount - 1,
                             textureParameters,
                             IOperand);

    gcmFOOTER();
    return status;
}

static gceSTATUS
_GenTexelFetchMSCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_TEXTURE_FETCH_MS,
                               IOperand,
                               &OperandsParameters[0].rOperands[0],
                               &OperandsParameters[2].rOperands[0]);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_TEXTURE_LOAD,
                               IOperand,
                               &OperandsParameters[0].rOperands[0],
                               &OperandsParameters[1].rOperands[0]);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

#if __USE_IMAGE_LOAD_TO_ACCESS_SAMPLER_BUFFER__
/*
** Since HW can only support 16K image size, so we use a image2D to save a textureBuffer.
** And the coord is ivec2(P%16K, P/16K).
*/
static gceSTATUS
_GenTexelFetchForTextureBufferCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    slsIOPERAND         coordXIOperand[1], coordYIOperand[1], coordIOperand[1];
    slsROPERAND         const16K[1], intermROperand[1];
    slsLOPERAND         intermLOperand[2];
    gcSHADER_PRECISION  precision = OperandsParameters[1].rOperands[0].u.reg.precision;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    slsROPERAND_InitializeIntOrIVecConstant(const16K,
                                            gcSHADER_INTEGER_X1,
                                            precision,
                                            16 * 1024);
    /* coordX = P % 16K. */
    slsIOPERAND_New(Compiler,
                    coordXIOperand,
                    gcSHADER_INTEGER_X1,
                    precision);

    status = slGenArithmeticExprCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     slvOPCODE_MOD,
                                     coordXIOperand,
                                     &OperandsParameters[1].rOperands[0],
                                     const16K);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* coordY = P / 16K. */
    slsIOPERAND_New(Compiler,
                    coordYIOperand,
                    gcSHADER_INTEGER_X1,
                    precision);

    status = slGenArithmeticExprCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     slvOPCODE_IDIV,
                                     coordYIOperand,
                                     &OperandsParameters[1].rOperands[0],
                                     const16K);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* Set the image coord to ivec2(coordX, coordY). */
    slsIOPERAND_New(Compiler,
                    coordIOperand,
                    gcSHADER_INTEGER_X2,
                    precision);

    slsLOPERAND_InitializeUsingIOperand(&intermLOperand[0], coordIOperand);
    slsROPERAND_InitializeUsingIOperand(&intermROperand[0], coordXIOperand);
    slGetVectorLOperandSlice(&intermLOperand[0],
                             0,
                             1,
                             &intermLOperand[1]);

    status = slGenAssignCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             &intermLOperand[1],
                             &intermROperand[0]);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsROPERAND_InitializeUsingIOperand(&intermROperand[0], coordYIOperand);
    slGetVectorLOperandSlice(&intermLOperand[0],
                             1,
                             1,
                             &intermLOperand[1]);

    status = slGenAssignCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             &intermLOperand[1],
                             &intermROperand[0]);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* Load the image. */
    slsROPERAND_InitializeUsingIOperand(&intermROperand[0], coordIOperand);
    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_IMAGE_READ,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &intermROperand[0]);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }


    gcmFOOTER_NO();
    return status;
}
#else
static gceSTATUS
_GenTexelFetchForTextureBufferCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status = gcvSTATUS_OK;
    sloIR_EXPR samplerOperand;
    slsNAME *sampler;
    slsLOGICAL_REG logicalReg[1];
    slsROPERAND intermROperand[2];
    slsROPERAND constantOne[1], constantZero[1];
    slsROPERAND sizeROperand[1];
    slsROPERAND coordROperand[1];
    slsIOPERAND intermIOperand[1];
    slsIOPERAND coordIOperand[1];
    slsLOPERAND intermLOperand[3];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* Create the sample size uniform. */
    samplerOperand = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _sloIR_EXPR);
    gcmASSERT(sloIR_OBJECT_GetType(&samplerOperand->base) == slvIR_VARIABLE);
    sampler = ((sloIR_VARIABLE)samplerOperand)->name;

    if (sampler->u.variableInfo.levelBaseSize == gcvNULL)
    {
        status = slAllocSamplerLevelBaseSize(Compiler,
                                             sampler,
                                             gcSHADER_FLOAT_X4);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    gcmASSERT(sampler->u.variableInfo.levelBaseSize);

    /* Make sure that the sample size is greater or equal than 1. */
    slsIOPERAND_New(Compiler,
                    intermIOperand,
                    OperandsParameters[1].dataTypes[0],
                    OperandsParameters[1].rOperands[0].u.reg.precision);

    slsLOGICAL_REG_InitializeUniform(logicalReg,
                                     slvSTORAGE_QUALIFIER_UNIFORM,
                                     IOperand->dataType,
                                     gcSHADER_PRECISION_MEDIUM,
                                     sampler->u.variableInfo.levelBaseSize,
                                     0);
    slsROPERAND_InitializeReg(&intermROperand[0], logicalReg);
    slGetVectorROperandSlice(&intermROperand[0],
                             0,
                             1,
                             &intermROperand[1]);

    slsROPERAND_InitializeIntOrIVecConstant(constantOne,
                                            gcSHADER_INTEGER_X1,
                                            gcSHADER_PRECISION_MEDIUM,
                                            (gctINT)1);

    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_MAX,
                               intermIOperand,
                               &intermROperand[1],
                               constantOne);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* Change the sampler size and coord to float. */
    slsROPERAND_InitializeUsingIOperand(sizeROperand, intermIOperand);
    status = slsROPERAND_ChangeDataTypeFamily(Compiler,
                                              PolynaryExpr->exprBase.base.lineNo,
                                              PolynaryExpr->exprBase.base.stringNo,
                                              gcvFALSE,
                                              gcSHADER_FLOAT_X1,
                                              sizeROperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    coordROperand[0] = OperandsParameters[1].rOperands[0];
    status = slsROPERAND_ChangeDataTypeFamily(Compiler,
                                              PolynaryExpr->exprBase.base.lineNo,
                                              PolynaryExpr->exprBase.base.stringNo,
                                              gcvFALSE,
                                              gcSHADER_FLOAT_X1,
                                              coordROperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* DIV with sampler size. */
    slsIOPERAND_New(Compiler,
                    intermIOperand,
                    gcSHADER_FLOAT_X1,
                    OperandsParameters[1].rOperands[0].u.reg.precision);
    status = slGenArithmeticExprCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     slvOPCODE_DIV,
                                     intermIOperand,
                                     coordROperand,
                                     sizeROperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* Construct the vec2(coord, 0.0). */
    slsIOPERAND_New(Compiler,
                    coordIOperand,
                    gcSHADER_FLOAT_X2,
                    coordROperand->u.reg.precision);
    slsLOPERAND_InitializeUsingIOperand(&intermLOperand[0], coordIOperand);
    slGetVectorLOperandSlice(&intermLOperand[0],
                             0,
                             1,
                             &intermLOperand[1]);

    slsROPERAND_InitializeUsingIOperand(&intermROperand[0], intermIOperand);
    status = slGenAssignCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             &intermLOperand[1],
                             &intermROperand[0]);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsROPERAND_InitializeFloatOrVecOrMatConstant(constantZero,
                                                  gcSHADER_FLOAT_X1,
                                                  gcSHADER_PRECISION_MEDIUM,
                                                  (gctFLOAT)0.0);
    slGetVectorLOperandSlice(&intermLOperand[0],
                             1,
                             1,
                             &intermLOperand[2]);

    slsROPERAND_InitializeUsingIOperand(&intermROperand[0], intermIOperand);
    status = slGenAssignCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             &intermLOperand[2],
                             constantZero);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* Do the TEXLD. */
    slsROPERAND_InitializeWithLOPERAND(&intermROperand[0], &intermLOperand[0]);
    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_TEXTURE_LOAD,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &intermROperand[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}
#endif

gceSTATUS
_GenTexelFetchCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    slsROPERAND sizeOperand[1];
    slsROPERAND intermROperand[1];
    slsLOPERAND intermLOperand[1];
    slsIOPERAND intermIOperand[1];
    sloIR_EXPR samplerOperand;
    slsGEN_CODE_PARAMETERS textureParameters[3];
    slsROPERAND texCoords[1];
    gcSHADER_TYPE floatType;
    gctUINT8 numCoordComponents;
    sleOPCODE_RES_TYPE resOpType = slvOPCODE_RES_TYPE_FETCH;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3 || OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);
    gcmASSERT(!slsDLINK_LIST_IsEmpty(&PolynaryExpr->operands->members));

    samplerOperand = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _sloIR_EXPR);

    if (slsDATA_TYPE_IsSamplerMS(samplerOperand->dataType) ||
        slsDATA_TYPE_IsSamplerMSARRAY(samplerOperand->dataType))
    {
       status = _GenTexelFetchMSCode(Compiler,
                                     CodeGenerator,
                                     PolynaryExpr,
                                     OperandCount,
                                     OperandsParameters,
                                     IOperand);
       if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

       resOpType = slvOPCODE_RES_TYPE_FETCH_MS;
    }

    if (slsDATA_TYPE_IsSamplerBuffer(samplerOperand->dataType))
    {
        gcmASSERT(OperandCount == 2);
        status = _GenTexelFetchForTextureBufferCode(Compiler,
                                                    CodeGenerator,
                                                    PolynaryExpr,
                                                    OperandsParameters,
                                                    IOperand);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else if (slsDATA_TYPE_IsSampler2DRect(samplerOperand->dataType))
    {
        gcmASSERT(OperandCount == 2);

        /*
        ** If HW can support INTEGER coord, we don't need to convert it to FLOAT.
        ** And we generate TEXLD_U instead of TEXLD.
        */
        if (GetHWHasUniversalTexldV2() && GetHWHasTexldUFix())
        {
            textureParameters[0] = OperandsParameters[0];
            textureParameters[1] = OperandsParameters[1];
            textureParameters[1].genTexldU = gcvTRUE;
        }
        else
        {
            slsIOPERAND_New(Compiler,
                            intermIOperand,
                            OperandsParameters[1].dataTypes[0],
                            OperandsParameters[1].rOperands[0].u.reg.precision);

            slsROPERAND_InitializeUsingIOperand(sizeOperand, intermIOperand);
            floatType = gcChangeElementDataType(OperandsParameters[1].dataTypes[0],
                                                gcSHADER_FLOAT_X1);

            status = slsROPERAND_ChangeDataTypeFamily(Compiler,
                                                      PolynaryExpr->exprBase.base.lineNo,
                                                      PolynaryExpr->exprBase.base.stringNo,
                                                      gcvFALSE,
                                                      floatType,
                                                      sizeOperand);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

            texCoords[0] = OperandsParameters[1].rOperands[0];

            status = slsROPERAND_ChangeDataTypeFamily(Compiler,
                                                      PolynaryExpr->exprBase.base.lineNo,
                                                      PolynaryExpr->exprBase.base.stringNo,
                                                      gcvFALSE,
                                                      floatType,
                                                      texCoords);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

            numCoordComponents = gcGetDataTypeComponentCount(floatType);
            if(gcIsSamplerArrayDataType(OperandsParameters[0].dataTypes[0]))
            {
                numCoordComponents--;
            }

            slsIOPERAND_New(Compiler,
                            intermIOperand,
                            texCoords->dataType,
                            texCoords->u.reg.precision);
            slsLOPERAND_InitializeUsingIOperand(intermLOperand, intermIOperand);
            status = slGenAssignCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     intermLOperand,
                                     texCoords);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

            slsROPERAND_InitializeUsingIOperand(texCoords, intermIOperand);

            slGetVectorROperandSlice(texCoords,
                                     0,
                                     numCoordComponents,
                                     intermROperand);
            slGetVectorROperandSlice(sizeOperand,
                                     0,
                                     numCoordComponents,
                                     sizeOperand);
            slsIOPERAND_New(Compiler,
                            intermIOperand,
                            intermROperand->dataType,
                            intermROperand->u.reg.precision);
            status = slGenArithmeticExprCode(Compiler,
                                             PolynaryExpr->exprBase.base.lineNo,
                                             PolynaryExpr->exprBase.base.stringNo,
                                             slvOPCODE_DIV,
                                             intermIOperand,
                                             intermROperand,
                                             sizeOperand);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

            slsROPERAND_InitializeUsingIOperand(intermROperand, intermIOperand);
            slGetVectorLOperandSlice(intermLOperand,
                                     0,
                                     numCoordComponents,
                                     intermLOperand);

            status = slGenAssignCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     intermLOperand,
                                     intermROperand);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

            textureParameters[0] = OperandsParameters[0];
            textureParameters[1] = OperandsParameters[1];
            textureParameters[1].dataTypes = &texCoords->dataType;
            textureParameters[1].rOperands = texCoords;
        }

        status = _GenTextureCode(Compiler,
                                 CodeGenerator,
                                 PolynaryExpr,
                                 2,
                                 textureParameters,
                                 IOperand);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else
    {
        status = _GenClampLod(Compiler,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo,
                              samplerOperand,
                              &OperandsParameters[0].rOperands[0],
                              &OperandsParameters[2].rOperands[0]);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        /*
        ** If HW can support INTEGER coord, we don't need to convert it to FLOAT.
        ** And we generate TEXLD_U instead of TEXLD.
        */
        if (GetHWHasUniversalTexldV2() && GetHWHasTexldUFix())
        {
            textureParameters[0] = OperandsParameters[0];
            textureParameters[1] = OperandsParameters[1];
            textureParameters[1].genTexldU = gcvTRUE;
        }
        else
        {
            slsIOPERAND_New(Compiler,
                            intermIOperand,
                            OperandsParameters[1].dataTypes[0],
                            OperandsParameters[1].rOperands[0].u.reg.precision);
            status = _GenSamplerSizeCode(Compiler,
                                         PolynaryExpr->exprBase.base.lineNo,
                                         PolynaryExpr->exprBase.base.stringNo,
                                         samplerOperand,
                                         &OperandsParameters[0].rOperands[0],
                                         &OperandsParameters[2].rOperands[0],
                                         intermIOperand);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

            slsROPERAND_InitializeUsingIOperand(sizeOperand, intermIOperand);
            floatType = gcChangeElementDataType(OperandsParameters[1].dataTypes[0],
                                                gcSHADER_FLOAT_X1);

            status = slsROPERAND_ChangeDataTypeFamily(Compiler,
                                                      PolynaryExpr->exprBase.base.lineNo,
                                                      PolynaryExpr->exprBase.base.stringNo,
                                                      gcvFALSE,
                                                      floatType,
                                                      sizeOperand);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

            texCoords[0] = OperandsParameters[1].rOperands[0];

            status = slsROPERAND_ChangeDataTypeFamily(Compiler,
                                                      PolynaryExpr->exprBase.base.lineNo,
                                                      PolynaryExpr->exprBase.base.stringNo,
                                                      gcvFALSE,
                                                      floatType,
                                                      texCoords);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

            numCoordComponents = gcGetDataTypeComponentCount(floatType);
            if(gcIsSamplerArrayDataType(OperandsParameters[0].dataTypes[0]))
            {
                numCoordComponents--;
            }

            slsIOPERAND_New(Compiler,
                            intermIOperand,
                            texCoords->dataType,
                            texCoords->u.reg.precision);
            slsLOPERAND_InitializeUsingIOperand(intermLOperand, intermIOperand);
            status = slGenAssignCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     intermLOperand,
                                     texCoords);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

            slsROPERAND_InitializeUsingIOperand(texCoords, intermIOperand);

            slGetVectorROperandSlice(texCoords,
                                     0,
                                     numCoordComponents,
                                     intermROperand);
            slGetVectorROperandSlice(sizeOperand,
                                     0,
                                     numCoordComponents,
                                     sizeOperand);
            slsIOPERAND_New(Compiler,
                            intermIOperand,
                            intermROperand->dataType,
                            intermROperand->u.reg.precision);
            status = slGenArithmeticExprCode(Compiler,
                                             PolynaryExpr->exprBase.base.lineNo,
                                             PolynaryExpr->exprBase.base.stringNo,
                                             slvOPCODE_DIV,
                                             intermIOperand,
                                             intermROperand,
                                             sizeOperand);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

            slsROPERAND_InitializeUsingIOperand(intermROperand, intermIOperand);
            slGetVectorLOperandSlice(intermLOperand,
                                     0,
                                     numCoordComponents,
                                     intermLOperand);

            status = slGenAssignCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     intermLOperand,
                                     intermROperand);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

            textureParameters[0] = OperandsParameters[0];
            textureParameters[1] = OperandsParameters[1];
            textureParameters[1].dataTypes = &texCoords->dataType;
            textureParameters[1].rOperands = texCoords;
        }

        status = slsROPERAND_ChangeDataTypeFamily(Compiler,
                                                  PolynaryExpr->exprBase.base.lineNo,
                                                  PolynaryExpr->exprBase.base.stringNo,
                                                  gcvFALSE,
                                                  gcSHADER_FLOAT_X1,
                                                  &OperandsParameters[2].rOperands[0]);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        textureParameters[2].rOperands = &OperandsParameters[2].rOperands[0];

    status = _GenTextureLodCode(Compiler,
                                CodeGenerator,
                                PolynaryExpr,
                                3,
                                textureParameters,
                                IOperand);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    /* Set the ResOpType. */
    status = slEmitCurrentCode(Compiler);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = slEmitOpCodeResType(Compiler, resOpType);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER();
    return status;
}

gceSTATUS
_GenTexelFetchOffsetCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    slsROPERAND sizeOperand[1];
    slsROPERAND intermROperand[1];
    slsLOPERAND intermLOperand[1], lOperand[1], vectorLOperand[1];
    slsIOPERAND intermIOperand[1];
    sloIR_EXPR samplerOperand;
    sloIR_EXPR operand;
    gctUINT    operandID = 0;
    slsGEN_CODE_PARAMETERS textureParameters[3];
    slsROPERAND texCoords[1], arrayLayer[1], offsetCoords[1];
    gctUINT8 numCoordComponents, numOffsetComponents;
    slsNAME *sampler;
    sleOPCODE_RES_TYPE resOpType = slvOPCODE_RES_TYPE_FETCH;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 4);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    gcmASSERT(!slsDLINK_LIST_IsEmpty(&PolynaryExpr->operands->members));

    samplerOperand = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _sloIR_EXPR);
    sampler = ((sloIR_VARIABLE)samplerOperand)->name;
    texCoords[0] = OperandsParameters[1].rOperands[0];
    numOffsetComponents = _GetSamplerCoordComponentCount(sampler->dataType);
    numCoordComponents = gcGetDataTypeComponentCount(OperandsParameters[1].rOperands[0].dataType);

    /* check if the offset operand is a constant expression */
    if (sloCOMPILER_IsOGLVersion(Compiler))
    {
        for (operand = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _sloIR_EXPR);
             operand != gcvNULL && operandID < 4;
             operand = slsDLINK_NODE_Next(&operand->base.node, struct _sloIR_EXPR))
        {
            if (operandID == 3 && operand->base.vptr->type != slvIR_CONSTANT)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                operand->base.lineNo,
                                                operand->base.stringNo,
                                                slvREPORT_ERROR,
                                                "The offset value must be a constant expression"));
                status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                gcmFOOTER();
                return status;
            }
            operandID ++;
        }
    }

    if (slsDATA_TYPE_IsSamplerMS(samplerOperand->dataType) ||
        slsDATA_TYPE_IsSamplerMSARRAY(samplerOperand->dataType))
    {
        resOpType = slvOPCODE_RES_TYPE_FETCH_MS;
    }

    status = _GenClampLod(Compiler,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo,
                          samplerOperand,
                          &OperandsParameters[0].rOperands[0],
                          &OperandsParameters[2].rOperands[0]);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /*
    ** If HW can support INTEGER coord, we don't need to convert it to FLOAT.
    ** And we generate TEXLD_U instead of TEXLD.
    */
    if (GetHWHasUniversalTexldV2() && GetHWHasTexldUFix())
    {
        /* Slice coords if needed. */
        if (numCoordComponents != numOffsetComponents)
        {
            slGetVectorROperandSlice(texCoords,
                                     0,
                                     numOffsetComponents,
                                     offsetCoords);
        }
        else
        {
            offsetCoords[0] = texCoords[0];
        }

        slsIOPERAND_New(Compiler,
                        intermIOperand,
                        offsetCoords->dataType,
                        offsetCoords->u.reg.precision);

        status = slGenArithmeticExprCode(Compiler,
                                         PolynaryExpr->exprBase.base.lineNo,
                                         PolynaryExpr->exprBase.base.stringNo,
                                         slvOPCODE_ADD,
                                         intermIOperand,
                                         offsetCoords,
                                         &OperandsParameters[3].rOperands[0]);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        slsLOPERAND_InitializeWithRegROPERAND(intermLOperand, offsetCoords);
        slsROPERAND_InitializeUsingIOperand(offsetCoords, intermIOperand);

        status = slGenAssignCode(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 intermLOperand,
                                 offsetCoords);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        textureParameters[0] = OperandsParameters[0];
        textureParameters[1] = OperandsParameters[1];
        textureParameters[1].rOperands = texCoords;
        textureParameters[1].dataTypes = &texCoords->dataType;
        textureParameters[1].genTexldU = gcvTRUE;
    }
    else
    {
        slsIOPERAND_New(Compiler,
                        intermIOperand,
                        OperandsParameters[1].dataTypes[0],
                        OperandsParameters[1].rOperands[0].u.reg.precision);
        status = _GenSamplerSizeCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     samplerOperand,
                                     &OperandsParameters[0].rOperands[0],
                                     &OperandsParameters[2].rOperands[0],
                                     intermIOperand);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        /* The array layer comes from the last component of P for the array forms. */
        if (numCoordComponents != numOffsetComponents)
        {
            slsLOPERAND_InitializeUsingIOperand(vectorLOperand, intermIOperand);
            slsLOPERAND_InitializeAsVectorComponent(lOperand, vectorLOperand, 2);
            slGetVectorROperandSlice(texCoords,
                                     numOffsetComponents,
                                     1,
                                     arrayLayer);

            status = slGenAssignCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     lOperand,
                                     arrayLayer);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        }

        slsROPERAND_InitializeUsingIOperand(sizeOperand, intermIOperand);

        status = slsROPERAND_ChangeDataTypeFamily(Compiler,
                                                  PolynaryExpr->exprBase.base.lineNo,
                                                  PolynaryExpr->exprBase.base.stringNo,
                                                  gcvFALSE,
                                                  gcChangeElementDataType(OperandsParameters[1].dataTypes[0],
                                                                          gcSHADER_FLOAT_X1),
                                                  sizeOperand);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        numCoordComponents = gcGetDataTypeComponentCount(OperandsParameters[3].dataTypes[0]);

        slGetVectorROperandSlice(texCoords,
                                 0,
                                 numCoordComponents,
                                 texCoords);
        slsIOPERAND_New(Compiler,
                        intermIOperand,
                        texCoords->dataType,
                        texCoords->u.reg.precision);
        status = slGenArithmeticExprCode(Compiler,
                                         PolynaryExpr->exprBase.base.lineNo,
                                         PolynaryExpr->exprBase.base.stringNo,
                                         slvOPCODE_ADD,
                                         intermIOperand,
                                         texCoords,
                                         &OperandsParameters[3].rOperands[0]);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        slsROPERAND_InitializeUsingIOperand(texCoords, intermIOperand);

        status = slsROPERAND_ChangeDataTypeFamily(Compiler,
                                                  PolynaryExpr->exprBase.base.lineNo,
                                                  PolynaryExpr->exprBase.base.stringNo,
                                                  gcvFALSE,
                                                  gcChangeElementDataType(texCoords->dataType,
                                                                          gcSHADER_FLOAT_X1),
                                                  texCoords);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        slGetVectorROperandSlice(sizeOperand,
                                 0,
                                 numCoordComponents,
                                 intermROperand);
        slsIOPERAND_New(Compiler,
                        intermIOperand,
                        intermROperand->dataType,
                        intermROperand->u.reg.precision);
        status = slGenArithmeticExprCode(Compiler,
                                         PolynaryExpr->exprBase.base.lineNo,
                                         PolynaryExpr->exprBase.base.stringNo,
                                         slvOPCODE_DIV,
                                         intermIOperand,
                                         texCoords,
                                         intermROperand);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        slsROPERAND_InitializeUsingIOperand(texCoords, intermIOperand);
        slsLOPERAND_InitializeWithRegROPERAND(intermLOperand, sizeOperand);
        slGetVectorLOperandSlice(intermLOperand,
                                 0,
                                 numCoordComponents,
                                 intermLOperand);

        status = slGenAssignCode(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 intermLOperand,
                                 texCoords);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        textureParameters[0] = OperandsParameters[0];
        textureParameters[1] = OperandsParameters[1];
        textureParameters[1].dataTypes = &sizeOperand->dataType;
        textureParameters[1].rOperands = sizeOperand;
        textureParameters[2].rOperands = &OperandsParameters[2].rOperands[0];
    }

    status = slsROPERAND_ChangeDataTypeFamily(Compiler,
                                              PolynaryExpr->exprBase.base.lineNo,
                                              PolynaryExpr->exprBase.base.stringNo,
                                              gcvFALSE,
                                              gcSHADER_FLOAT_X1,
                                              &OperandsParameters[2].rOperands[0]);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    textureParameters[2].rOperands = &OperandsParameters[2].rOperands[0];

    status = _GenTextureLodCode(Compiler,
                                CodeGenerator,
                                PolynaryExpr,
                                3,
                                textureParameters,
                                IOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* Set the ResOpType. */
    status = slEmitCurrentCode(Compiler);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = slEmitOpCodeResType(Compiler, resOpType);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER();
    return status;
}

gceSTATUS
_GenTextureProjOffsetCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status = gcvSTATUS_OK;
    slsROPERAND intermROperand[1];
    slsIOPERAND intermIOperand[1];
    slsGEN_CODE_PARAMETERS textureParameters[4];
    slsROPERAND texCoords[1];
    gctUINT8 numProjCoordComponents;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3 || OperandCount == 4);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* projection coordinates component count is determined from the texture coordinate's vector size */
    numProjCoordComponents = gcGetDataTypeComponentCount(OperandsParameters[1].dataTypes[0]) - 1;

    slGetVectorROperandSlice(&OperandsParameters[1].rOperands[0],
                             0,
                             numProjCoordComponents,
                             texCoords);

    slGetVectorROperandSlice(&OperandsParameters[1].rOperands[0],
                             numProjCoordComponents,
                             1,
                             intermROperand);

    slsIOPERAND_New(Compiler,
                    intermIOperand,
                    texCoords->dataType,
                    texCoords->u.reg.precision);

    status = slGenArithmeticExprCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     slvOPCODE_DIV,
                                     intermIOperand,
                                     texCoords,
                                     intermROperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsROPERAND_InitializeUsingIOperand(texCoords, intermIOperand);

    textureParameters[0] = OperandsParameters[0];
    textureParameters[1] = OperandsParameters[1];
    textureParameters[1].dataTypes = &texCoords->dataType;
    textureParameters[1].rOperands = texCoords;
    textureParameters[2] = OperandsParameters[2];

    if(OperandCount == 4) {
        textureParameters[3] = OperandsParameters[3];
    }

    status = _GenTextureOffsetCode(Compiler,
                                   CodeGenerator,
                                   PolynaryExpr,
                                   OperandCount,
                                   textureParameters,
                                   IOperand);

    gcmFOOTER();
    return status;
}

gceSTATUS
_GenTextureLodOffsetCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status = gcvSTATUS_OK;
    slsGEN_CODE_PARAMETERS textureParameters[3];
    slsROPERAND rOperand[1];
    slsIOPERAND intermIOperand[1];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 4);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    slsIOPERAND_New(Compiler,
                    intermIOperand,
                    OperandsParameters[1].dataTypes[0],
                    OperandsParameters[1].rOperands[0].u.reg.precision);
    status = _ComputeOffsetCoords(Compiler,
                                  PolynaryExpr,
                                  &OperandsParameters[0].rOperands[0],
                                  &OperandsParameters[1].rOperands[0],
                                  gcvNULL,
                                  &OperandsParameters[2].rOperands[0],
                                  &OperandsParameters[3].rOperands[0],
                                  intermIOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    textureParameters[0] = OperandsParameters[0];
    textureParameters[1] = OperandsParameters[1];
    slsROPERAND_InitializeUsingIOperand(rOperand, intermIOperand);
    textureParameters[1].dataTypes = &rOperand->dataType;
    textureParameters[1].rOperands = rOperand;
    textureParameters[2] = OperandsParameters[2];

    status = _GenTextureLodCode(Compiler,
                                CodeGenerator,
                                PolynaryExpr,
                                3,
                                textureParameters,
                                IOperand);

    gcmFOOTER();
    return status;
}

gceSTATUS
_GenTextureProjLodOffsetCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status = gcvSTATUS_OK;
    slsROPERAND intermROperand[1];
    slsIOPERAND intermIOperand[1];
    slsGEN_CODE_PARAMETERS textureParameters[4];
    slsROPERAND texCoords[1];
    gctUINT8 numProjCoordComponents;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 4);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* projection coordinates component count is determined from the offset's vector size */
    numProjCoordComponents = gcGetDataTypeComponentCount(OperandsParameters[1].dataTypes[0]) - 1;

    slGetVectorROperandSlice(&OperandsParameters[1].rOperands[0],
                             0,
                             numProjCoordComponents,
                             texCoords);

    slGetVectorROperandSlice(&OperandsParameters[1].rOperands[0],
                             numProjCoordComponents,
                             1,
                             intermROperand);

    slsIOPERAND_New(Compiler,
                    intermIOperand,
                    texCoords->dataType,
                    texCoords->u.reg.precision);

    status = slGenArithmeticExprCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     slvOPCODE_DIV,
                                     intermIOperand,
                                     texCoords,
                                     intermROperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsROPERAND_InitializeUsingIOperand(texCoords, intermIOperand);

    textureParameters[0] = OperandsParameters[0];
    textureParameters[1] = OperandsParameters[1];
    textureParameters[1].dataTypes = &texCoords->dataType;
    textureParameters[1].rOperands = texCoords;
    textureParameters[2] = OperandsParameters[2];

    textureParameters[3] = OperandsParameters[3];

    status = _GenTextureLodOffsetCode(Compiler,
                                      CodeGenerator,
                                      PolynaryExpr,
                                      4,
                                      textureParameters,
                                      IOperand);

    gcmFOOTER();
    return status;
}

static gceSTATUS
_GenTextureNonShadowGradCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    slsROPERAND lod[1];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 4);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (GetHWHasHalti2())
    {
        status = slGenGenericCode2(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   slvOPCODE_TEXTURE_GRAD,
                                   IOperand,
                                   &OperandsParameters[2].rOperands[0],
                                   &OperandsParameters[3].rOperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else
    {
        status = _ComputeLodByTextureGrad(Compiler,
                                          PolynaryExpr,
                                          gcvFALSE,
                                          gcGetDataTypeComponentCount(OperandsParameters[2].dataTypes[0]) > 2 ? gcvTRUE : gcvFALSE,
                                          &OperandsParameters[1].rOperands[0],
                                          &OperandsParameters[2].rOperands[0],
                                          &OperandsParameters[3].rOperands[0],
                                          lod);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        status = slGenGenericCode2(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   slvOPCODE_TEXTURE_LOD,
                                   IOperand,
                                   &OperandsParameters[0].rOperands[0],
                                   lod);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_TEXTURE_LOAD,
                               IOperand,
                               &OperandsParameters[0].rOperands[0],
                               &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenTextureShadowGradCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    slsROPERAND lod[1];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 4);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (GetHWHasHalti2())
    {
        status = slGenGenericCode2(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   slvOPCODE_TEXTURE_GRAD,
                                   IOperand,
                                   &OperandsParameters[2].rOperands[0],
                                   &OperandsParameters[3].rOperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else
    {
        status = _ComputeLodByTextureGrad(Compiler,
                                          PolynaryExpr,
                                          gcvFALSE,
                                          gcGetDataTypeComponentCount(OperandsParameters[2].dataTypes[0]) > 2 ? gcvTRUE : gcvFALSE,
                                          &OperandsParameters[1].rOperands[0],
                                          &OperandsParameters[2].rOperands[0],
                                          &OperandsParameters[3].rOperands[0],
                                          lod);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        status = slGenGenericCode2(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   slvOPCODE_TEXTURE_LOD,
                                   IOperand,
                                   &OperandsParameters[0].rOperands[0],
                                   lod);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_TEXTURE_LOAD_PCF,
                               IOperand,
                               &OperandsParameters[0].rOperands[0],
                               &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenTextureNonShadowGatherCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    slsROPERAND floatConstantZero[1];
    slsROPERAND intConstantZero[1];
    slsROPERAND *component;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    slsROPERAND_InitializeFloatOrVecOrMatConstant(floatConstantZero,
                                                  gcSHADER_FLOAT_X1,
                                                  gcSHADER_PRECISION_MEDIUM,
                                                  (gctFLOAT)0.0);
    if(OperandCount == 2)
    {
        slsROPERAND_InitializeIntOrIVecConstant(intConstantZero,
                                                gcSHADER_INTEGER_X1,
                                                gcSHADER_PRECISION_MEDIUM,
                                                0);
        component = intConstantZero;
    }
    else
    {
        component = &OperandsParameters[2].rOperands[0];
    }

    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_TEXTURE_GATHER,
                               IOperand,
                               component,
                               floatConstantZero);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_TEXTURE_LOAD,
                               IOperand,
                               &OperandsParameters[0].rOperands[0],
                               &OperandsParameters[1].rOperands[0]);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenTextureShadowGatherCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    slsROPERAND intConstantZero[1];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    slsROPERAND_InitializeIntOrIVecConstant(intConstantZero,
                                            gcSHADER_INTEGER_X1,
                                            gcSHADER_PRECISION_MEDIUM,
                                            0);

    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_TEXTURE_GATHER,
                               IOperand,
                               intConstantZero,
                               &OperandsParameters[2].rOperands[0]);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_TEXTURE_LOAD_PCF,
                               IOperand,
                               &OperandsParameters[0].rOperands[0],
                               &OperandsParameters[1].rOperands[0]);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenTextureGatherCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    sloIR_EXPR  expr;
    sltBUILT_IN_GEN_CODE_FUNC_PTR   genCode = gcvNULL;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2 || OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    gcmASSERT(!slsDLINK_LIST_IsEmpty(&PolynaryExpr->operands->members));

    expr = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _sloIR_EXPR);
    if(slsDATA_TYPE_IsSamplerShadow(expr->dataType))
    {
        genCode = _GenTextureShadowGatherCode;
    }
    else
    {
        genCode = _GenTextureNonShadowGatherCode;
    }

    if(!genCode)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    status = (*genCode)(Compiler,
                        CodeGenerator,
                        PolynaryExpr,
                        OperandCount,
                        OperandsParameters,
                        IOperand);
    gcmFOOTER();
    return status;
}

gceSTATUS
_GenVivTextureGatherCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2 || OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = _GenTextureGatherCode(Compiler,
                                   CodeGenerator,
                                   PolynaryExpr,
                                   OperandCount,
                                   OperandsParameters,
                                   IOperand);
    gcmFOOTER();
    return status;
}


gceSTATUS
_GenVivArrayLengthMethodCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcSHADER            binary;
    gcsSTORAGE_BLOCK    storageBlock;
    gcUNIFORM           blockAddressUniform;
    slsLOGICAL_REG      logicalReg[1];
    slsROPERAND         intermROperand[1];
    slsLOPERAND         intermLOperand[1];
    slsROPERAND         storageBlockUnsizedArrayLength[1];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    gcmASSERT(slmROPERAND_IsStorageBlockMember(&OperandsParameters->rOperands[0]));

    gcmVERIFY_OK(sloCOMPILER_GetBinary(Compiler, &binary));
    status = gcSHADER_GetStorageBlock(binary,
                                      OperandsParameters->rOperands->u.reg.u.variable->blockIndex,
                                      &storageBlock);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = gcSHADER_GetUniform(binary,
                                 GetSBIndex(storageBlock),
                                 &blockAddressUniform);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }


    /* force to use ivec2 type, the length() is singed integer type */
    slsLOGICAL_REG_InitializeUniform(logicalReg,
                                     slvSTORAGE_QUALIFIER_UNIFORM,
                                     gcSHADER_INTEGER_X2, /*GetUniformType(blockAddressUniform),*/
                                     GetUniformPrecision(blockAddressUniform),
                                     blockAddressUniform,
                                     0);
    slsROPERAND_InitializeReg(intermROperand, logicalReg);

    /* sbo_addr_uniform.x is start address of the SBO buffer
     * sbo_addr_uniform.y is the size of whole SBO buffer */
    slGetVectorROperandSlice(intermROperand,
                             1,  /* y component */
                             1,  /* vector length one */
                             storageBlockUnsizedArrayLength
                             );
    slsLOPERAND_InitializeUsingIOperand(intermLOperand, IOperand);

    status = slGenAssignCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             intermLOperand,
                             storageBlockUnsizedArrayLength);

    gcmFOOTER();
    return status;
}

gceSTATUS
_GenTextureGradCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    sloIR_EXPR  expr;
    sltBUILT_IN_GEN_CODE_FUNC_PTR   genCode = gcvNULL;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 4);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    gcmASSERT(!slsDLINK_LIST_IsEmpty(&PolynaryExpr->operands->members));

    expr = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _sloIR_EXPR);
    switch(expr->dataType->elementType)
    {
    case slvTYPE_SAMPLER2DSHADOW:
    case slvTYPE_SAMPLERCUBESHADOW:
    case slvTYPE_SAMPLER2DARRAYSHADOW:
        genCode = _GenTextureShadowGradCode;
        break;

    default:
        genCode = _GenTextureNonShadowGradCode;
        break;
    }

    if(!genCode)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    status = (*genCode)(Compiler,
                        CodeGenerator,
                        PolynaryExpr,
                        OperandCount,
                        OperandsParameters,
                        IOperand);
    gcmFOOTER();
    return status;
}

gceSTATUS
_GenTextureGradOffsetCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status = gcvSTATUS_OK;
    slsGEN_CODE_PARAMETERS textureParameters[4];
    slsROPERAND rOperand[1];
    slsIOPERAND intermIOperand[1];
    slsROPERAND lod[1];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 5);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = _ComputeLodByTextureGrad(Compiler,
                                      PolynaryExpr,
                                      gcvTRUE,
                                      gcGetDataTypeComponentCount(OperandsParameters[4].dataTypes[0]) > 2 ? gcvTRUE : gcvFALSE,
                                      &OperandsParameters[1].rOperands[0],
                                      &OperandsParameters[2].rOperands[0],
                                      &OperandsParameters[3].rOperands[0],
                                      lod);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsIOPERAND_New(Compiler,
                    intermIOperand,
                    OperandsParameters[1].dataTypes[0],
                    OperandsParameters[1].rOperands[0].u.reg.precision);
    status = _ComputeOffsetCoords(Compiler,
                                  PolynaryExpr,
                                  &OperandsParameters[0].rOperands[0],
                                  &OperandsParameters[1].rOperands[0],
                                  gcvNULL,
                                  lod,
                                  &OperandsParameters[4].rOperands[0],
                                  intermIOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    textureParameters[0] = OperandsParameters[0];
    textureParameters[1] = OperandsParameters[1];
    slsROPERAND_InitializeUsingIOperand(rOperand, intermIOperand);
    textureParameters[1].dataTypes = &rOperand->dataType;
    textureParameters[1].rOperands = rOperand;
    textureParameters[2] = OperandsParameters[2];
    textureParameters[3] = OperandsParameters[3];

    status = _GenTextureGradCode(Compiler,
                                 CodeGenerator,
                                 PolynaryExpr,
                                 4,
                                 textureParameters,
                                 IOperand);

    gcmFOOTER();
    return status;
}

static gceSTATUS
_GenTextureNonShadowProjGradCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    slsROPERAND lod[1];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 4);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (GetHWHasHalti2())
    {
        status = slGenGenericCode2(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   slvOPCODE_TEXTURE_GRAD,
                                   IOperand,
                                   &OperandsParameters[2].rOperands[0],
                                   &OperandsParameters[3].rOperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else
    {
        status = _ComputeLodByTextureGrad(Compiler,
                                          PolynaryExpr,
                                          gcvFALSE,
                                          gcGetDataTypeComponentCount(OperandsParameters[2].dataTypes[0]) > 2 ? gcvTRUE : gcvFALSE,
                                          &OperandsParameters[1].rOperands[0],
                                          &OperandsParameters[2].rOperands[0],
                                          &OperandsParameters[3].rOperands[0],
                                          lod);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        status = slGenGenericCode2(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   slvOPCODE_TEXTURE_LOD,
                                   IOperand,
                                   &OperandsParameters[0].rOperands[0],
                                   lod);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_TEXTURE_LOAD_PROJ,
                               IOperand,
                               &OperandsParameters[0].rOperands[0],
                               &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenTextureShadowProjGradCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    slsROPERAND lod[1];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 4);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (GetHWHasHalti2())
    {
        status = slGenGenericCode2(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   slvOPCODE_TEXTURE_GRAD,
                                   IOperand,
                                   &OperandsParameters[2].rOperands[0],
                                   &OperandsParameters[3].rOperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else
    {
        status = _ComputeLodByTextureGrad(Compiler,
                                          PolynaryExpr,
                                          gcvFALSE,
                                          gcGetDataTypeComponentCount(OperandsParameters[2].dataTypes[0]) > 2 ? gcvTRUE : gcvFALSE,
                                          &OperandsParameters[1].rOperands[0],
                                          &OperandsParameters[2].rOperands[0],
                                          &OperandsParameters[3].rOperands[0],
                                          lod);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        status = slGenGenericCode2(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   slvOPCODE_TEXTURE_LOD,
                                   IOperand,
                                   &OperandsParameters[0].rOperands[0],
                                   lod);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_TEXTURE_LOAD_PCFPROJ,
                               IOperand,
                               &OperandsParameters[0].rOperands[0],
                               &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenTextureProjGradCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    sloIR_EXPR  expr;
    sltBUILT_IN_GEN_CODE_FUNC_PTR   genCode = gcvNULL;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 4);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    gcmASSERT(!slsDLINK_LIST_IsEmpty(&PolynaryExpr->operands->members));

    expr = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _sloIR_EXPR);
    switch(expr->dataType->elementType)
    {
    case slvTYPE_SAMPLER2DSHADOW:
    case slvTYPE_SAMPLERCUBESHADOW:
        genCode = _GenTextureShadowProjGradCode;
        break;

    default:
        genCode = _GenTextureNonShadowProjGradCode;
        break;
    }

    if(!genCode)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    status = (*genCode)(Compiler,
                        CodeGenerator,
                        PolynaryExpr,
                        OperandCount,
                        OperandsParameters,
                        IOperand);
    gcmFOOTER();
    return status;
}

gceSTATUS
_GenTextureProjGradOffsetCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status = gcvSTATUS_OK;
    slsROPERAND intermROperand[1];
    slsIOPERAND intermIOperand[1];
    slsGEN_CODE_PARAMETERS textureParameters[5];
    slsROPERAND texCoords[1];
    gctUINT8 numProjCoordComponents;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 5);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* projection coordinates component count is determined from the offset's vector size */
    numProjCoordComponents = gcGetDataTypeComponentCount(OperandsParameters[1].dataTypes[0]) - 1;

    slGetVectorROperandSlice(&OperandsParameters[1].rOperands[0],
                             0,
                             numProjCoordComponents,
                             texCoords);

    slGetVectorROperandSlice(&OperandsParameters[1].rOperands[0],
                             numProjCoordComponents,
                             1,
                             intermROperand);

    slsIOPERAND_New(Compiler,
                    intermIOperand,
                    texCoords->dataType,
                    texCoords->u.reg.precision);

    status = slGenArithmeticExprCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     slvOPCODE_DIV,
                                     intermIOperand,
                                     texCoords,
                                     intermROperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsROPERAND_InitializeUsingIOperand(texCoords, intermIOperand);

    textureParameters[0] = OperandsParameters[0];
    textureParameters[1] = OperandsParameters[1];
    textureParameters[1].dataTypes = &texCoords->dataType;
    textureParameters[1].rOperands = texCoords;
    textureParameters[2] = OperandsParameters[2];

    textureParameters[3] = OperandsParameters[3];
    textureParameters[4] = OperandsParameters[4];

    status = _GenTextureGradOffsetCode(Compiler,
                                      CodeGenerator,
                                      PolynaryExpr,
                                      5,
                                      textureParameters,
                                      IOperand);

    gcmFOOTER();
    return status;
}

gceSTATUS
_EvaluateSin(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        values[i].floatValue = gcoMATH_Sine(OperandConstants[0]->values[i].floatValue);
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}


gceSTATUS
_GenSinCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_SIN,
                            IOperand,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateCos(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        values[i].floatValue = gcoMATH_Cosine(OperandConstants[0]->values[i].floatValue);
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenCosCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_COS,
                            IOperand,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateTan(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        values[i].floatValue = gcoMATH_Tangent(OperandConstants[0]->values[i].floatValue);
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenTanCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_TAN,
                            IOperand,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateAsin(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        values[i].floatValue = gcoMATH_ArcSine(OperandConstants[0]->values[i].floatValue);
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateAcos(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        values[i].floatValue = gcoMATH_ArcCosine(OperandConstants[0]->values[i].floatValue);
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateAtan(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount[2] = {0};
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount <= 2);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount[0] = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    if (OperandCount == 2)
    {
        componentCount[1] = (slmDATA_TYPE_vectorSize_GET(OperandConstants[1]->exprBase.dataType) == 0) ?
                            1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[1]->exprBase.dataType);
        gcmASSERT(componentCount[0] == componentCount[1]);
    }

    if (OperandCount == 1)
    {
        for (i = 0; i < componentCount[0]; i++)
        {
            values[i].floatValue = gcoMATH_ArcTangent(OperandConstants[0]->values[i].floatValue);
        }
    }
    else
    {
        for (i = 0; i < componentCount[0]; i++)
        {
            values[i].floatValue = OperandConstants[0]->values[i].floatValue / OperandConstants[1]->values[i].floatValue;
            values[i].floatValue = gcoMATH_ArcTangent(values[i].floatValue);
            if(OperandConstants[1]->values[i].floatValue < 0.0)
            {
                values[i].floatValue = values[i].floatValue > 0 ? values[i].floatValue - _PI: values[i].floatValue + _PI;
            }
        }
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount[0],
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EvaluateSinhOrCosh(
    IN sloCOMPILER Compiler,
    IN sloIR_CONSTANT * OperandConstants,
    IN gctBOOL isSinh,
    IN OUT sluCONSTANT_VALUE *values
    )
{
    gctUINT             i, componentCount;
    gcmHEADER();

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        values[i].floatValue = gcoMATH_Exp(OperandConstants[0]->values[i].floatValue);
        if (isSinh)
            values[i].floatValue -= gcoMATH_Exp(-OperandConstants[0]->values[i].floatValue);
        else
            values[i].floatValue += gcoMATH_Exp(-OperandConstants[0]->values[i].floatValue);
        values[i].floatValue /= 2;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenSingleExpCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gcSHADER_TYPE DataType,
    IN slsROPERAND * ROperand,
    OUT slsIOPERAND * IOperand
    )
{
    gceSTATUS       status;
    slsROPERAND     constantROperand;
    slsIOPERAND     intermIOperand;
    slsROPERAND     intermROperand;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(IOperand);

    /* mul t0, x, _LOG2_E */
    slsROPERAND_InitializeFloatOrVecOrMatConstant(&constantROperand,
                                                  gcSHADER_FLOAT_X1,
                                                  gcSHADER_PRECISION_HIGH,
                                                  _LOG2_E);

    slsIOPERAND_New(Compiler,
                    &intermIOperand,
                    DataType,
                    ROperand->u.reg.precision);

    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    &intermIOperand,
                                    ROperand,
                                    &constantROperand);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* exp2 result, t0 */
    slsROPERAND_InitializeUsingIOperand(&intermROperand, &intermIOperand);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_EXP2,
                            IOperand,
                            &intermROperand);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenSinhOrCoshCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand,
    IN gctBOOL isSinh
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    slsROPERAND     constantZero, constantTwo;
    slsIOPERAND     intermIOperand[4];
    slsROPERAND     intermROperand[4];
    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* exp t0, x*/
    slsIOPERAND_New(Compiler, &intermIOperand[0], OperandsParameters[0].dataTypes[0],
                    OperandsParameters[0].rOperands[0].u.reg.precision);

    _GenSingleExpCode(Compiler, CodeGenerator, PolynaryExpr, OperandsParameters[0].dataTypes[0],
                    &OperandsParameters[0].rOperands[0], &intermIOperand[0]);

    /* sub t1, 0.0, x */
    slsIOPERAND_New(Compiler, &intermIOperand[1], OperandsParameters[0].dataTypes[0],
                    OperandsParameters[0].rOperands[0].u.reg.precision);

    slsROPERAND_InitializeFloatOrVecOrMatConstant(&constantZero,
                                                  OperandsParameters[0].dataTypes[0],
                                                  gcSHADER_PRECISION_HIGH,
                                                  0.0);

    status = slGenArithmeticExprCode(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_SUB,
                            &intermIOperand[1],
                            &constantZero,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* exp t2, t1*/
    slsIOPERAND_New(Compiler, &intermIOperand[2], OperandsParameters[0].dataTypes[0],
                    OperandsParameters[0].rOperands[0].u.reg.precision);
    slsROPERAND_InitializeUsingIOperand(&intermROperand[1], &intermIOperand[1]);

    _GenSingleExpCode(Compiler, CodeGenerator, PolynaryExpr, OperandsParameters[0].dataTypes[0],
                    &intermROperand[1], &intermIOperand[2]);

    /*add/sub t3 t0, t2 */
    slsIOPERAND_New(Compiler, &intermIOperand[3], OperandsParameters[0].dataTypes[0],
                    OperandsParameters[0].rOperands[0].u.reg.precision);
    slsROPERAND_InitializeUsingIOperand(&intermROperand[0], &intermIOperand[0]);
    slsROPERAND_InitializeUsingIOperand(&intermROperand[2], &intermIOperand[2]);

    status = slGenArithmeticExprCode(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            isSinh ? slvOPCODE_SUB : slvOPCODE_ADD,
                            &intermIOperand[3],
                            &intermROperand[0],
                            &intermROperand[2]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /*div result t2, 2.0*/
    slsROPERAND_InitializeUsingIOperand(&intermROperand[3], &intermIOperand[3]);
    slsROPERAND_InitializeFloatOrVecOrMatConstant(&constantTwo,
                                                  OperandsParameters[0].dataTypes[0],
                                                  gcSHADER_PRECISION_HIGH,
                                                  2.0);

    status = slGenArithmeticExprCode(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_DIV,
                            IOperand,
                            &intermROperand[3],
                            &constantTwo);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return status;
}

gceSTATUS
_EvaluateSinh(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    sluCONSTANT_VALUE   values[4];
    gctUINT componentCount;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(ResultConstant->exprBase.dataType,
                                   OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    _EvaluateSinhOrCosh(Compiler, OperandConstants, gcvTRUE, values);

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenSinhCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);
    gcmASSERT(OperandCount == 1);

    status = _GenSinhOrCoshCode(Compiler, CodeGenerator, PolynaryExpr,
                            OperandsParameters, IOperand, gcvTRUE);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateCosh(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    sluCONSTANT_VALUE   values[4];
    gctUINT componentCount;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(ResultConstant->exprBase.dataType,
                                   OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    _EvaluateSinhOrCosh(Compiler, OperandConstants, gcvFALSE, values);

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenCoshCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);
    gcmASSERT(OperandCount == 1);

    status = _GenSinhOrCoshCode(Compiler, CodeGenerator, PolynaryExpr,
                            OperandsParameters, IOperand, gcvFALSE);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateTanh(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    sluCONSTANT_VALUE   valuesSinh[4], valuesCosh[4];
    gctUINT             i, componentCount;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(ResultConstant->exprBase.dataType,
                                   OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    _EvaluateSinhOrCosh(Compiler, OperandConstants, gcvTRUE, valuesSinh);

    _EvaluateSinhOrCosh(Compiler, OperandConstants, gcvFALSE, valuesCosh);

    for (i = 0; i < componentCount; i++)
    {
        valuesSinh[i].floatValue /= valuesCosh[i].floatValue;
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    valuesSinh);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenTanhCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    slsIOPERAND     intermIOperand[2];
    slsROPERAND     intermROperand[2];
    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);
    gcmASSERT(OperandCount == 1);

    /* sinh t0, x*/
    slsIOPERAND_New(Compiler, &intermIOperand[0], OperandsParameters[0].dataTypes[0],
                OperandsParameters[0].rOperands[0].u.reg.precision);

    status = _GenSinhOrCoshCode(Compiler, CodeGenerator, PolynaryExpr,
                            OperandsParameters, &intermIOperand[0], gcvTRUE);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* cosh t1, x*/
    slsIOPERAND_New(Compiler, &intermIOperand[1], OperandsParameters[0].dataTypes[0],
                OperandsParameters[0].rOperands[0].u.reg.precision);

    status = _GenSinhOrCoshCode(Compiler, CodeGenerator, PolynaryExpr,
                            OperandsParameters, &intermIOperand[1], gcvFALSE);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* div result t0, t1*/
    slsROPERAND_InitializeUsingIOperand(&intermROperand[0], &intermIOperand[0]);
    slsROPERAND_InitializeUsingIOperand(&intermROperand[1], &intermIOperand[1]);

    status = slGenArithmeticExprCode(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_DIV,
                            IOperand,
                            &intermROperand[0],
                            &intermROperand[1]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EvaluateAsinhOrAcosh(
    IN sloCOMPILER Compiler,
    IN sloIR_CONSTANT * OperandConstants,
    IN gctBOOL isAsinh,
    IN OUT sluCONSTANT_VALUE *values
    )
{
    gctUINT             i, componentCount;

    gcmHEADER();

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    /* asinh(x) = log(x + sqrt(x * x + 1)) */
    for (i = 0; i < componentCount; i++)
    {
        values[i].floatValue = OperandConstants[0]->values[i].floatValue * OperandConstants[0]->values[i].floatValue;
        if (isAsinh)
        {
            values[i].floatValue += 1.0;
            values[i].floatValue = gcoMATH_SquareRoot(values[i].floatValue);
            if (OperandConstants[0]->values[i].floatValue > 0.0)
            {
                values[i].floatValue += OperandConstants[0]->values[i].floatValue;
                values[i].floatValue = gcoMATH_Log(values[i].floatValue);
            }
            else
            {
                values[i].floatValue -= OperandConstants[0]->values[i].floatValue;
                values[i].floatValue = -gcoMATH_Log(values[i].floatValue);
            }
        }
        else
        {
            values[i].floatValue -= 1.0;
            values[i].floatValue = gcoMATH_SquareRoot(values[i].floatValue);
            values[i].floatValue += OperandConstants[0]->values[i].floatValue;
            values[i].floatValue = gcoMATH_Log(values[i].floatValue);
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenScalarAsinhCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN slsROPERAND * ROperand,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    slsROPERAND     constantOne;
    slsIOPERAND     intermIOperand[5];
    slsROPERAND     intermROperand[5];
    slsSELECTION_CONTEXT    selectionContext;
    slsROPERAND             zeroROperand;

    gcmHEADER();

    /* mul t0, x, x */
    slsIOPERAND_New(Compiler, &intermIOperand[0], ROperand->dataType,
                    ROperand->u.reg.precision);

    status = slGenArithmeticExprCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     slvOPCODE_MUL,
                                     &intermIOperand[0],
                                     ROperand,
                                     ROperand);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* add t1, t0, 1.0 */
    slsIOPERAND_New(Compiler, &intermIOperand[1], ROperand->dataType,
                    ROperand->u.reg.precision);
    slsROPERAND_InitializeUsingIOperand(&intermROperand[0], &intermIOperand[0]);

    slsROPERAND_InitializeFloatOrVecOrMatConstant(&constantOne,
                                                  ROperand->dataType,
                                                  gcSHADER_PRECISION_HIGH,
                                                  1.0);

    status = slGenArithmeticExprCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     slvOPCODE_ADD,
                                     &intermIOperand[1],
                                     &intermROperand[0],
                                     &constantOne);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* sqrt t2, t1*/
    slsIOPERAND_New(Compiler, &intermIOperand[2], ROperand->dataType,
                    ROperand->u.reg.precision);
    slsROPERAND_InitializeUsingIOperand(&intermROperand[1], &intermIOperand[1]);

    status = slGenGenericCode1(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_SQRT,
                               &intermIOperand[2],
                               &intermROperand[1]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* The selection begin */
    status = slDefineSelectionBegin(Compiler,
                                    CodeGenerator,
                                    gcvTRUE,
                                    &selectionContext);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* The condition part: x < 0.0 */
    slsROPERAND_InitializeUsingIOperand(&intermROperand[2], &intermIOperand[2]);

    if (gcIsDoubleDataType(ROperand->dataType))
    {
        slsROPERAND_InitializeFloatOrVecOrMatConstant(&zeroROperand,
                                                      gcSHADER_FLOAT64_X1,
                                                      gcSHADER_PRECISION_MEDIUM,
                                                      (gctFLOAT)0.0);
    }
    else
    {
        slsROPERAND_InitializeFloatOrVecOrMatConstant(&zeroROperand,
                                                      gcSHADER_FLOAT_X1,
                                                      gcSHADER_PRECISION_MEDIUM,
                                                      (gctFLOAT)0.0);
    }

    status = slGenSelectionCompareConditionCode(Compiler,
                                                CodeGenerator,
                                                &selectionContext,
                                                PolynaryExpr->exprBase.base.lineNo,
                                                PolynaryExpr->exprBase.base.stringNo,
                                                slvCONDITION_LESS_THAN,
                                                ROperand,
                                                &zeroROperand);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* The true part */
    status = slDefineSelectionTrueOperandBegin(Compiler,
                                                CodeGenerator,
                                                &selectionContext);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* sub t3, t2, x*/
    slsIOPERAND_New(Compiler, &intermIOperand[3], ROperand->dataType,
                    ROperand->u.reg.precision);

    status = slGenArithmeticExprCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        slvOPCODE_SUB,
                                        &intermIOperand[3],
                                        &intermROperand[2],
                                        ROperand
                                        );

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* log t4, t3*/
    slsIOPERAND_New(Compiler, &intermIOperand[4], ROperand->dataType,
                    ROperand->u.reg.precision);
    slsROPERAND_InitializeUsingIOperand(&intermROperand[3], &intermIOperand[3]);

    status = _ComputeLog(Compiler,
                         PolynaryExpr,
                         &intermROperand[3],
                         &intermIOperand[4]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* sub result, 0.0, t4 */
    slsROPERAND_InitializeUsingIOperand(&intermROperand[4], &intermIOperand[4]);
    status = slGenArithmeticExprCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     slvOPCODE_SUB,
                                     IOperand,
                                     &zeroROperand,
                                     &intermROperand[4]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = slDefineSelectionTrueOperandEnd(Compiler,
                                             CodeGenerator,
                                             &selectionContext,
                                             gcvFALSE);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* The false part */
    status = slDefineSelectionFalseOperandBegin(Compiler,
                                                CodeGenerator,
                                                &selectionContext);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* add t3, x, t2*/
    slsIOPERAND_New(Compiler, &intermIOperand[3], ROperand->dataType,
                    ROperand->u.reg.precision);

    status = slGenArithmeticExprCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     slvOPCODE_ADD,
                                     &intermIOperand[3],
                                     ROperand,
                                     &intermROperand[2]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* log result, t3*/
    slsROPERAND_InitializeUsingIOperand(&intermROperand[3], &intermIOperand[3]);

    status = _ComputeLog(Compiler,
                         PolynaryExpr,
                         &intermROperand[3],
                         IOperand);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = slDefineSelectionFalseOperandEnd(Compiler,
                                              CodeGenerator,
                                              &selectionContext);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* The selection end */
    status = slDefineSelectionEnd(Compiler,
                                  CodeGenerator,
                                  &selectionContext);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return status;
}

static gceSTATUS
_GenAsinhOrAcoshCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand,
    IN gctBOOL isAsinh
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    slsROPERAND     constantOne;
    slsIOPERAND     intermIOperand[4];
    slsROPERAND     intermROperand[4];
    slsLOPERAND     lOperand[1], intermLOperand[1];
    gctUINT8 i;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (isAsinh)
    {
        /* for scalar */
        if (OperandsParameters[0].dataTypes[0] == gcSHADER_FLOAT_X1 || OperandsParameters[0].dataTypes[0]  == gcSHADER_FLOAT64_X1)
        {
            status = _GenScalarAsinhCode(Compiler, CodeGenerator, PolynaryExpr, &OperandsParameters[0].rOperands[0], IOperand);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
            gcmFOOTER_NO();
            return status;
        }

        /* for vector */
        gcmASSERT(OperandsParameters[0].dataTypes[0] == gcSHADER_FLOAT_X2
               || OperandsParameters[0].dataTypes[0] == gcSHADER_FLOAT_X3
               || OperandsParameters[0].dataTypes[0] == gcSHADER_FLOAT_X4
               || OperandsParameters[0].dataTypes[0] == gcSHADER_FLOAT64_X2
               || OperandsParameters[0].dataTypes[0] == gcSHADER_FLOAT64_X3
               || OperandsParameters[0].dataTypes[0] == gcSHADER_FLOAT64_X4);

        for (i = 0; i < gcGetVectorDataTypeComponentCount(OperandsParameters[0].dataTypes[0]); i++)
        {
            slsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
            slGetVectorLOperandSlice(lOperand,
                                     i,
                                     1,
                                     intermLOperand);

            slsROPERAND_InitializeAsVectorComponent(&intermROperand[0], &OperandsParameters[0].rOperands[0], i);

            slsIOPERAND_New(Compiler, &intermIOperand[0], intermROperand[0].dataType,
                            intermROperand[0].u.reg.precision);

            status = _GenScalarAsinhCode(Compiler, CodeGenerator, PolynaryExpr, &intermROperand[0], &intermIOperand[0]);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

            slsROPERAND_InitializeUsingIOperand(&intermROperand[1], &intermIOperand[0]);

            status = slGenAssignCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     intermLOperand,
                                     &intermROperand[1]);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        }

        gcmFOOTER_NO();
        return status;
    }

    /* for Acosh */
    /* mul t0, x, x */
    slsIOPERAND_New(Compiler, &intermIOperand[0], OperandsParameters[0].dataTypes[0],
                    OperandsParameters[0].rOperands[0].u.reg.precision);

    status = slGenArithmeticExprCode(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_MUL,
                            &intermIOperand[0],
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* add/sub t1, t0, 1.0 */
    slsIOPERAND_New(Compiler, &intermIOperand[1], OperandsParameters[0].dataTypes[0],
                    OperandsParameters[0].rOperands[0].u.reg.precision);
    slsROPERAND_InitializeUsingIOperand(&intermROperand[0], &intermIOperand[0]);

    slsROPERAND_InitializeFloatOrVecOrMatConstant(&constantOne,
                                              OperandsParameters[0].dataTypes[0],
                                              gcSHADER_PRECISION_HIGH,
                                              1.0);

    status = slGenArithmeticExprCode(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            isAsinh ? slvOPCODE_ADD : slvOPCODE_SUB,
                            &intermIOperand[1],
                            &intermROperand[0],
                            &constantOne);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* sqrt t2, t1 */
    slsIOPERAND_New(Compiler, &intermIOperand[2], OperandsParameters[0].dataTypes[0],
                    OperandsParameters[0].rOperands[0].u.reg.precision);
    slsROPERAND_InitializeUsingIOperand(&intermROperand[1], &intermIOperand[1]);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_SQRT,
                            &intermIOperand[2],
                            &intermROperand[1]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* add t3, x, t2 */
    slsIOPERAND_New(Compiler, &intermIOperand[3], OperandsParameters[0].dataTypes[0],
                    OperandsParameters[0].rOperands[0].u.reg.precision);
    slsROPERAND_InitializeUsingIOperand(&intermROperand[2], &intermIOperand[2]);

    status = slGenArithmeticExprCode(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_ADD,
                            &intermIOperand[3],
                            &OperandsParameters[0].rOperands[0],
                            &intermROperand[2]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* log result, t3 */
    slsROPERAND_InitializeUsingIOperand(&intermROperand[3], &intermIOperand[3]);

    status = _ComputeLog(Compiler,
                     PolynaryExpr,
                     &intermROperand[3],
                     IOperand);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return status;
}

gceSTATUS
_EvaluateAsinh(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    sluCONSTANT_VALUE   values[4];
    gctUINT componentCount;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(ResultConstant->exprBase.dataType,
                                   OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    _EvaluateAsinhOrAcosh(Compiler, OperandConstants, gcvTRUE, values);

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenAsinhCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);
    gcmASSERT(OperandCount == 1);

    status = _GenAsinhOrAcoshCode(Compiler, CodeGenerator, PolynaryExpr,
                            OperandsParameters, IOperand, gcvTRUE);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenAcoshCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);
    gcmASSERT(OperandCount == 1);

    status = _GenAsinhOrAcoshCode(Compiler, CodeGenerator, PolynaryExpr,
                            OperandsParameters, IOperand, gcvFALSE);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateAcosh(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    sluCONSTANT_VALUE   values[4];
    gctUINT componentCount;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(ResultConstant->exprBase.dataType,
                                   OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    _EvaluateAsinhOrAcosh(Compiler, OperandConstants, gcvFALSE, values);

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateAtanh(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    /* atanh(x) = (log(1 + x) - log(1 - x)) / 2*/
    /* atanh(x) = log((1 + x) * (1 - x)) / 2 */
    for (i = 0; i < componentCount; i++)
    {
        values[i].floatValue = gcoMATH_Log((gctFLOAT)1.0 + OperandConstants[0]->values[i].floatValue);
        values[i].floatValue = values[i].floatValue - gcoMATH_Log((gctFLOAT)1.0 - OperandConstants[0]->values[i].floatValue);
        values[i].floatValue /= 2.0;
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenAtanhCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS       status = gcvSTATUS_OK;
    slsROPERAND     constantOne, constantTwo, smallestPositive;
    slsIOPERAND     intermIOperand[5];
    slsIOPERAND     intermMaxIOperand[2];
    slsROPERAND     intermMaxROperand[2];
    slsROPERAND     intermROperand[5];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);
    gcmASSERT(OperandCount == 1);

    /* The samllest positive constant, use it to invoid avoid log(0). */
    slsROPERAND_InitializeFloatOrVecOrMatConstant(&smallestPositive,
                                              gcSHADER_FLOAT_X1,
                                              gcSHADER_PRECISION_HIGH,
                                              1.175494351e-038f);

    /* add t0, 1.0, x*/
    slsIOPERAND_New(Compiler, &intermIOperand[0], OperandsParameters[0].dataTypes[0],
                    OperandsParameters[0].rOperands[0].u.reg.precision);

    slsROPERAND_InitializeFloatOrVecOrMatConstant(&constantOne,
                                              OperandsParameters[0].dataTypes[0],
                                              gcSHADER_PRECISION_HIGH,
                                              1.0);

    status = slGenArithmeticExprCode(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_ADD,
                            &intermIOperand[0],
                            &constantOne,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* sub t1, 1.0, x */
    slsIOPERAND_New(Compiler, &intermIOperand[1], OperandsParameters[0].dataTypes[0],
                    OperandsParameters[0].rOperands[0].u.reg.precision);

    status = slGenArithmeticExprCode(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_SUB,
                            &intermIOperand[1],
                            &constantOne,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* max t_max1, t0, constant */
    slsROPERAND_InitializeUsingIOperand(&intermMaxROperand[0], &intermIOperand[0]);
    slsIOPERAND_New(Compiler, &intermMaxIOperand[0], OperandsParameters[0].dataTypes[0],
                    OperandsParameters[0].rOperands[0].u.reg.precision);

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_MAX,
                            &intermMaxIOperand[0],
                            &intermMaxROperand[0],
                            &smallestPositive);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* log t2, t_max1*/
    slsIOPERAND_New(Compiler, &intermIOperand[2], OperandsParameters[0].dataTypes[0],
                    OperandsParameters[0].rOperands[0].u.reg.precision);
    slsROPERAND_InitializeUsingIOperand(&intermROperand[0], &intermMaxIOperand[0]);

    status = _ComputeLog(Compiler,
                         PolynaryExpr,
                         &intermROperand[0],
                         &intermIOperand[2]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* max t_max2, t1, constant */
    slsROPERAND_InitializeUsingIOperand(&intermMaxROperand[1], &intermIOperand[1]);
    slsIOPERAND_New(Compiler, &intermMaxIOperand[1], OperandsParameters[0].dataTypes[0],
                    OperandsParameters[0].rOperands[0].u.reg.precision);

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_MAX,
                            &intermMaxIOperand[1],
                            &intermMaxROperand[1],
                            &smallestPositive);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* log t3, t_max2*/
    slsIOPERAND_New(Compiler, &intermIOperand[3], OperandsParameters[0].dataTypes[0],
                    OperandsParameters[0].rOperands[0].u.reg.precision);
    slsROPERAND_InitializeUsingIOperand(&intermROperand[1], &intermMaxIOperand[1]);

    status = _ComputeLog(Compiler,
                         PolynaryExpr,
                         &intermROperand[1],
                         &intermIOperand[3]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* sub t4, t2, t3*/
    slsIOPERAND_New(Compiler, &intermIOperand[4], OperandsParameters[0].dataTypes[0],
                    OperandsParameters[0].rOperands[0].u.reg.precision);
    slsROPERAND_InitializeUsingIOperand(&intermROperand[2], &intermIOperand[2]);
    slsROPERAND_InitializeUsingIOperand(&intermROperand[3], &intermIOperand[3]);

    status = slGenArithmeticExprCode(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_SUB,
                            &intermIOperand[4],
                            &intermROperand[2],
                            &intermROperand[3]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /*div result, t4, 2.0*/
    slsROPERAND_InitializeUsingIOperand(&intermROperand[4], &intermIOperand[4]);
    slsROPERAND_InitializeFloatOrVecOrMatConstant(&constantTwo,
                                              OperandsParameters[0].dataTypes[0],
                                              gcSHADER_PRECISION_HIGH,
                                              2.0);

    status = slGenArithmeticExprCode(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_DIV,
                            IOperand,
                            &intermROperand[4],
                            &constantTwo);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateRadians(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        values[i].floatValue = OperandConstants[0]->values[i].floatValue * _PI / (gctFLOAT)(180.0f);
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenRadiansCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS           status;
    slsROPERAND         constantROperand;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);
    gcmASSERT(gcGetComponentDataType(IOperand->dataType) == gcSHADER_FLOAT_X1);

    /* mul result, degrees, _PI / 180 */
    slsROPERAND_InitializeFloatOrVecOrMatConstant(&constantROperand,
                                                  gcSHADER_FLOAT_X1,
                                                  gcSHADER_PRECISION_HIGH,
                                                  _PI / (gctFLOAT)180.0);

    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    IOperand,
                                    &OperandsParameters[0].rOperands[0],
                                    &constantROperand);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateDegrees(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        values[i].floatValue = OperandConstants[0]->values[i].floatValue * (gctFLOAT)(180.0f) / _PI;
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenDegreesCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS           status;
    slsROPERAND         constantROperand;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);
    gcmASSERT(gcGetComponentDataType(IOperand->dataType) == gcSHADER_FLOAT_X1);

    /* mul result, radians, 180 / _PI */
    slsROPERAND_InitializeFloatOrVecOrMatConstant(&constantROperand,
                                                  gcSHADER_FLOAT_X1,
                                                  gcSHADER_PRECISION_HIGH,
                                                  (gctFLOAT)180.0 / _PI);

    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    IOperand,
                                    &OperandsParameters[0].rOperands[0],
                                    &constantROperand);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenPow0Code(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS           status;
    slsLOPERAND         lOperand;
    slsROPERAND         constantROperand;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);
    gcmASSERT(gcGetComponentDataType(IOperand->dataType) == gcSHADER_FLOAT_X1);

    /* mov result, 1.0 */
    slsLOPERAND_InitializeUsingIOperand(&lOperand, IOperand);
    slsROPERAND_InitializeFloatOrVecOrMatConstant(&constantROperand,
                                                  IOperand->dataType,
                                                  gcSHADER_PRECISION_MEDIUM,
                                                  (gctFLOAT)1.0);

    status = slGenAssignCode(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            &lOperand,
                            &constantROperand);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenPow1Code(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS           status;
    slsLOPERAND         lOperand;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* mov result, x */
    slsLOPERAND_InitializeUsingIOperand(&lOperand, IOperand);

    status = slGenAssignCode(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            &lOperand,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenPow2Code(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS           status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* mul result, x, x */
    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    IOperand,
                                    &OperandsParameters[0].rOperands[0],
                                    &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenPow3Code(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS       status;
    slsIOPERAND     intermIOperand;
    slsROPERAND     intermROperand;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* mul t0, x, x */
    slsIOPERAND_New(Compiler,
                    &intermIOperand,
                    OperandsParameters[0].dataTypes[0],
                    OperandsParameters[0].rOperands[0].u.reg.precision);

    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    &intermIOperand,
                                    &OperandsParameters[0].rOperands[0],
                                    &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* mul result, t0, x */
    slsROPERAND_InitializeUsingIOperand(&intermROperand, &intermIOperand);

    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    IOperand,
                                    &intermROperand,
                                    &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenPow4Code(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS       status;
    slsIOPERAND     intermIOperand;
    slsROPERAND     intermROperand;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* mul t0, x, x */
    slsIOPERAND_New(Compiler,
                    &intermIOperand,
                    OperandsParameters[0].dataTypes[0],
                    OperandsParameters[0].rOperands[0].u.reg.precision);

    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    &intermIOperand,
                                    &OperandsParameters[0].rOperands[0],
                                    &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* mul result, t0, t0 */
    slsROPERAND_InitializeUsingIOperand(&intermROperand, &intermIOperand);

    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    IOperand,
                                    &intermROperand,
                                    &intermROperand);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenPow5Code(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS       status;
    slsIOPERAND     intermIOperands[2];
    slsROPERAND     intermROperands[2];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* mul t0, x, x */
    slsIOPERAND_New(Compiler,
                    &intermIOperands[0],
                    OperandsParameters[0].dataTypes[0],
                    OperandsParameters[0].rOperands[0].u.reg.precision);

    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    &intermIOperands[0],
                                    &OperandsParameters[0].rOperands[0],
                                    &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* mul t1, t0, t0 */
    slsIOPERAND_New(Compiler,
                    &intermIOperands[1],
                    OperandsParameters[0].dataTypes[0],
                    OperandsParameters[0].rOperands[0].u.reg.precision);
    slsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);

    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    &intermIOperands[1],
                                    &intermROperands[0],
                                    &intermROperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* mul result, t1, x */
    slsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    IOperand,
                                    &intermROperands[1],
                                    &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenPow6Code(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS       status;
    slsIOPERAND     intermIOperands[2];
    slsROPERAND     intermROperands[2];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* mul t0, x, x */
    slsIOPERAND_New(Compiler,
                    &intermIOperands[0],
                    OperandsParameters[0].dataTypes[0],
                    OperandsParameters[0].rOperands[0].u.reg.precision);

    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    &intermIOperands[0],
                                    &OperandsParameters[0].rOperands[0],
                                    &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* mul t1, t0, t0 */
    slsIOPERAND_New(Compiler,
                    &intermIOperands[1],
                    OperandsParameters[0].dataTypes[0],
                    OperandsParameters[0].rOperands[0].u.reg.precision);
    slsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);

    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    &intermIOperands[1],
                                    &intermROperands[0],
                                    &intermROperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* mul result, t1, t0 */
    slsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    IOperand,
                                    &intermROperands[1],
                                    &intermROperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenPow8Code(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS       status;
    slsIOPERAND *   intermIOperands = gcvNULL;
    slsROPERAND *   intermROperands = gcvNULL;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = gcoOS_Allocate(gcvNULL,
                                    2 * sizeof(slsIOPERAND),
                                    (gctPOINTER *)&intermIOperands);
    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    gcmONERROR(gcoOS_Allocate(gcvNULL,
                              2 * sizeof(slsROPERAND),
                              (gctPOINTER *)&intermROperands));

    /* mul t0, x, x */
    slsIOPERAND_New(Compiler,
                    intermIOperands,
                    OperandsParameters[0].dataTypes[0],
                    OperandsParameters[0].rOperands[0].u.reg.precision);

    gcmONERROR(slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    intermIOperands,
                                    &OperandsParameters[0].rOperands[0],
                                    &OperandsParameters[0].rOperands[0]));

    /* mul t1, t0, t0 */
    slsIOPERAND_New(Compiler,
                    intermIOperands + 1,
                    OperandsParameters[0].dataTypes[0],
                    OperandsParameters[0].rOperands[0].u.reg.precision);
    slsROPERAND_InitializeUsingIOperand(intermROperands, intermIOperands);

    gcmONERROR(slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    intermIOperands + 1,
                                    intermROperands,
                                    intermROperands));

    /* mul result, t1, t1 */
    slsROPERAND_InitializeUsingIOperand(intermROperands + 1, intermIOperands + 1);

    gcmONERROR(slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    IOperand,
                                    intermROperands + 1,
                                    intermROperands + 1));

OnError:
    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, intermIOperands));
    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, intermROperands));

    /* Return the status. */
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

typedef gceSTATUS
(* sltGEN_POW_N_CODE_FUNC_PTR)(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

#define POW_N_COUNT     9

const sltGEN_POW_N_CODE_FUNC_PTR        GenPowNCodeTable[POW_N_COUNT] =
{
    _GenPow0Code,
    _GenPow1Code,
    _GenPow2Code,
    _GenPow3Code,
    _GenPow4Code,
    _GenPow5Code,
    _GenPow6Code,
    gcvNULL,
    _GenPow8Code
};

gceSTATUS
_EvaluatePow(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount[2] = {0};
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    for (i = 0; i < OperandCount; i++)
    {
        componentCount[i] = (slmDATA_TYPE_vectorSize_GET(OperandConstants[i]->exprBase.dataType) == 0) ?
                            1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[i]->exprBase.dataType);
    }

    gcmASSERT(componentCount[0] == componentCount[1]);

    for (i = 0; i < componentCount[0]; i++)
    {
        values[i].floatValue = gcoMATH_Power(OperandConstants[0]->values[i].floatValue,
                                                    OperandConstants[1]->values[i].floatValue);
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount[0],
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenPowCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS                   status;
    gctUINT                     i;
    sltGEN_POW_N_CODE_FUNC_PTR  genCode = gcvNULL;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (Compiler->context.optimizationOptions & slvOPTIMIZATION_CALCULATION)
    {
        for (i = 0; i < POW_N_COUNT; i++)
        {
            if (slsROPERAND_IsFloatOrVecConstant(&OperandsParameters[1].rOperands[0], (gctFLOAT)i))
            {
                genCode = GenPowNCodeTable[i];
                break;
            }
        }
    }

    if (genCode != gcvNULL)
    {
        status = (*genCode)(
                            Compiler,
                            CodeGenerator,
                            PolynaryExpr,
                            OperandCount,
                            OperandsParameters,
                            IOperand);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else
    {
        status = slGenGenericCode2(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                slvOPCODE_POW,
                                IOperand,
                                &OperandsParameters[0].rOperands[0],
                                &OperandsParameters[1].rOperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateExp(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        values[i].floatValue = gcoMATH_Exp(OperandConstants[0]->values[i].floatValue);
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenExpCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS       status;
    slsROPERAND     constantROperand;
    slsIOPERAND     intermIOperand;
    slsROPERAND     intermROperand;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* mul t0, x, _LOG2_E */
    slsROPERAND_InitializeFloatOrVecOrMatConstant(&constantROperand,
                                                  gcSHADER_FLOAT_X1,
                                                  gcSHADER_PRECISION_HIGH,
                                                  _LOG2_E);

    slsIOPERAND_New(Compiler,
                    &intermIOperand,
                    OperandsParameters[0].dataTypes[0],
                    OperandsParameters[0].rOperands[0].u.reg.precision);

    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    &intermIOperand,
                                    &OperandsParameters[0].rOperands[0],
                                    &constantROperand);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* exp2 result, t0 */
    slsROPERAND_InitializeUsingIOperand(&intermROperand, &intermIOperand);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_EXP2,
                            IOperand,
                            &intermROperand);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateLog(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        values[i].floatValue = gcoMATH_Log(OperandConstants[0]->values[i].floatValue);
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenLogCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS       status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = _ComputeLog(Compiler,
                         PolynaryExpr,
                         &OperandsParameters[0].rOperands[0],
                         IOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateExp2(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        values[i].floatValue = gcoMATH_Exp2(OperandConstants[0]->values[i].floatValue);
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenExp2Code(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_EXP2,
                            IOperand,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateLog2(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        values[i].floatValue = gcoMATH_Log2(OperandConstants[0]->values[i].floatValue);
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenLog2Code(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_LOG2,
                            IOperand,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateSqrt(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        values[i].floatValue = gcoMATH_SquareRoot(OperandConstants[0]->values[i].floatValue);
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenSqrtCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_SQRT,
                            IOperand,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateInverseSqrt(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        values[i].floatValue = gcoMATH_ReciprocalSquareRoot(OperandConstants[0]->values[i].floatValue);
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenInverseSqrtCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_INVERSE_SQRT,
                            IOperand,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateAbs(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];
    gctINT resultType   = T_FLOAT;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType)
                    || slsDATA_TYPE_IsIntOrIVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    if (slsDATA_TYPE_IsIntOrIVec(OperandConstants[0]->exprBase.dataType))
        resultType = T_INT;

    for (i = 0; i < componentCount; i++)
    {
        if (resultType == T_FLOAT)
        {
            values[i].floatValue = OperandConstants[0]->values[i].floatValue > 0.0f ?
                OperandConstants[0]->values[i].floatValue : -OperandConstants[0]->values[i].floatValue;
        }
        else if (resultType == T_INT)
        {
            values[i].intValue = OperandConstants[0]->values[i].intValue > 0 ?
                OperandConstants[0]->values[i].intValue : -OperandConstants[0]->values[i].intValue;
        }
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenAbsCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_ABS,
                            IOperand,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateSign(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType)
                    || slsDATA_TYPE_IsIntOrIVec(OperandConstants[0]->exprBase.dataType));

    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    if (slsDATA_TYPE_IsIntOrIVec(OperandConstants[0]->exprBase.dataType))
    {
        for (i = 0; i < componentCount; i++)
        {
            if (OperandConstants[0]->values[i].intValue > 0)
            {
                values[i].intValue = 1;
            }
            else if (OperandConstants[0]->values[i].intValue < 0)
            {
                values[i].intValue = -1;
            }
            else
            {
                values[i].intValue = 0;
            }
        }
    }
    else
    {
        for (i = 0; i < componentCount; i++)
        {
            values[i].floatValue = (OperandConstants[0]->values[i].floatValue > 0.0f) ? 1.0f :
                (OperandConstants[0]->values[i].floatValue < 0.0f) ? -1.0f : 0.0f;
        }
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenSignCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_SIGN,
                            IOperand,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateFloor(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        values[i].floatValue = gcoMATH_Floor(OperandConstants[0]->values[i].floatValue);
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenFloorCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_FLOOR,
                            IOperand,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateTrunc(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        float absVal;

        absVal = OperandConstants[0]->values[i].floatValue > 0.0f ?
                 OperandConstants[0]->values[i].floatValue : -OperandConstants[0]->values[i].floatValue;

        values[i].floatValue = gcoMATH_Floor(absVal);
        if(OperandConstants[0]->values[i].floatValue < 0.0f)
        {
            values[i].floatValue = -values[i].floatValue;
        }

    }

    status = sloIR_CONSTANT_AddValues(Compiler,
                                      ResultConstant,
                                      componentCount,
                                      values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenTruncCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    slsIOPERAND intermIOperands[2];
    slsROPERAND intermROperands[2];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    slsIOPERAND_New(Compiler,
                    &intermIOperands[0],
                    IOperand->dataType,
                    OperandsParameters[0].rOperands[0].u.reg.precision);
    slsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);
    slsIOPERAND_New(Compiler,
                    &intermIOperands[1],
                    IOperand->dataType,
                    OperandsParameters[0].rOperands[0].u.reg.precision);
    slsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

    status = slGenGenericCode1(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_SIGN,
                               &intermIOperands[0],
                               &OperandsParameters[0].rOperands[0]);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }


    status = slGenGenericCode1(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_ABS,
                               &intermIOperands[1],
                               &OperandsParameters[0].rOperands[0]);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = slGenGenericCode1(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_FLOOR,
                               &intermIOperands[1],
                               &intermROperands[1]);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }


    status = slGenArithmeticExprCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     slvOPCODE_MUL,
                                     IOperand,
                                     &intermROperands[0],
                                     &intermROperands[1]);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateRound(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        float absVal;

        absVal = OperandConstants[0]->values[i].floatValue > 0.0f ?
                 OperandConstants[0]->values[i].floatValue : -OperandConstants[0]->values[i].floatValue;

        values[i].floatValue = gcoMATH_Floor(absVal + 0.5f);
        if(OperandConstants[0]->values[i].floatValue < 0.0f)
        {
            values[i].floatValue = -values[i].floatValue;
        }
    }

    status = sloIR_CONSTANT_AddValues(Compiler,
                                      ResultConstant,
                                      componentCount,
                                      values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenRoundCodeInner(
    IN sloCOMPILER Compiler,
    IN gctINT LineNo,
    IN gctINT StringNo,
    IN slsROPERAND *Round,
    IN slsIOPERAND *IOperand
    )
{
    gceSTATUS   status;
    slsROPERAND dot5ROperand;
    slsIOPERAND intermIOperands[1];
    slsROPERAND intermROperands[1];

    gcmHEADER();

    /* floor(lod + 0.5) */
    slsROPERAND_InitializeFloatOrVecOrMatConstant(&dot5ROperand,
                                                  gcSHADER_FLOAT_X1,
                                                  gcSHADER_PRECISION_MEDIUM,
                                                  0.5f);

    slsIOPERAND_New(Compiler,
                    &intermIOperands[0],
                    IOperand->dataType,
                    Round->u.reg.precision);

    status = slGenArithmeticExprCode(Compiler,
                                     LineNo,
                                     StringNo,
                                     slvOPCODE_ADD,
                                     &intermIOperands[0],
                                     &dot5ROperand,
                                     Round);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);

    status = slGenGenericCode1(Compiler,
                               LineNo,
                               StringNo,
                               slvOPCODE_FLOOR,
                               IOperand,
                               &intermROperands[0]);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateRoundEven(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS   status;
    gctUINT     i, componentCount;
    sluCONSTANT_VALUE   values[4];
    sluCONSTANT_VALUE intermRes;
    gctFLOAT  r1, r2;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        intermRes.uintValue = OperandConstants[0]->values[i].uintValue & 0x7fffffff;
        if(intermRes.floatValue >= (float) (1 << 24))
        {
            values[i].floatValue = OperandConstants[0]->values[i].floatValue;
        }
        else
        {
            r1 = intermRes.floatValue - gcoMATH_Floor(intermRes.floatValue);
            r2 = gcoMATH_Floor(intermRes.floatValue + 0.5f);
            if(r1 == 0.5)
            {
                 intermRes.intValue = ((int) r2) & 0x00000001;
                 r2 -= (float) intermRes.intValue;
            }
            if(OperandConstants[0]->values[i].floatValue > 0.0f)
            {
                values[i].floatValue = r2;
            }
            else
            {
                values[i].floatValue = -r2;
            }
        }
    }

    status = sloIR_CONSTANT_AddValues(Compiler,
                                      ResultConstant,
                                      componentCount,
                                      values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}


gceSTATUS
_EvaluateCeil(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        values[i].floatValue = gcoMATH_Ceiling(OperandConstants[0]->values[i].floatValue);
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenCeilCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_CEIL,
                            IOperand,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateFract(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        values[i].floatValue = OperandConstants[0]->values[i].floatValue -
            gcoMATH_Floor(OperandConstants[0]->values[i].floatValue);
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenFractCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_FRACT,
                            IOperand,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gctFLOAT
_Mod(
     IN gctFLOAT Operand0,
     IN gctFLOAT Operand1
     )
{
    return Operand0 - Operand1 * gcoMATH_Floor(Operand0 / Operand1);
}

gceSTATUS
_EvaluateMod(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        if (slsDATA_TYPE_IsFloat(OperandConstants[1]->exprBase.dataType))
        {
            values[i].floatValue = _Mod(
                                        OperandConstants[0]->values[i].floatValue,
                                        OperandConstants[1]->values[0].floatValue);
        }
        else
        {
            values[i].floatValue = _Mod(
                                        OperandConstants[0]->values[i].floatValue,
                                        OperandConstants[1]->values[i].floatValue);
        }
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenModCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS       status;
    slsIOPERAND     intermIOperands[3];
    slsROPERAND     intermROperands[3];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if(_GEN_MOD_IN_BACKEND)
    {
        status = slGenGenericCode2(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   slvOPCODE_MOD,
                                   IOperand,
                                   &OperandsParameters[0].rOperands[0],
                                   &OperandsParameters[1].rOperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    /* div t0, x (operand0), y (operand1) */
    slsIOPERAND_New(Compiler,
                    &intermIOperands[0],
                    OperandsParameters[0].dataTypes[0],
                    OperandsParameters[0].rOperands[0].u.reg.precision);

    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_DIV,
                                    &intermIOperands[0],
                                    &OperandsParameters[0].rOperands[0],
                                    &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* floor t1, t0 */
    slsIOPERAND_New(Compiler,
                    &intermIOperands[1],
                    intermIOperands[0].dataType,
                    intermIOperands[0].precision);
    slsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_FLOOR,
                            &intermIOperands[1],
                            &intermROperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* mul t2, y (operand1), t1 */
    slsIOPERAND_New(Compiler,
                    &intermIOperands[2],
                    intermIOperands[1].dataType,
                    intermIOperands[1].precision);
    slsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    &intermIOperands[2],
                                    &OperandsParameters[1].rOperands[0],
                                    &intermROperands[1]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* sub result, x (operand0), t2 */
    slsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);

    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_SUB,
                                    IOperand,
                                    &OperandsParameters[0].rOperands[0],
                                    &intermROperands[2]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}


gceSTATUS
_EvaluateMin(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        if (slsDATA_TYPE_IsFloatOrVec(OperandConstants[1]->exprBase.dataType))
        {
            if (slsDATA_TYPE_IsFloat(OperandConstants[1]->exprBase.dataType))
            {
                values[i].floatValue = OperandConstants[1]->values[0].floatValue < OperandConstants[0]->values[i].floatValue ?
                    OperandConstants[1]->values[0].floatValue : OperandConstants[0]->values[i].floatValue;
            }
            else
            {
                values[i].floatValue = OperandConstants[1]->values[i].floatValue < OperandConstants[0]->values[i].floatValue ?
                    OperandConstants[1]->values[i].floatValue : OperandConstants[0]->values[i].floatValue;
            }
        }
        else if (slsDATA_TYPE_IsIntOrIVec(OperandConstants[1]->exprBase.dataType))
        {
            if (slsDATA_TYPE_IsInt(OperandConstants[1]->exprBase.dataType))
            {
                values[i].intValue = OperandConstants[1]->values[0].intValue < OperandConstants[0]->values[i].intValue ?
                    OperandConstants[1]->values[0].intValue : OperandConstants[0]->values[i].intValue;
            }
            else
            {
                values[i].intValue = OperandConstants[1]->values[i].intValue < OperandConstants[0]->values[i].intValue ?
                    OperandConstants[1]->values[i].intValue : OperandConstants[0]->values[i].intValue;
            }
        }
        else
        {
            gcmASSERT(gcvFALSE);
            values[i].floatValue = OperandConstants[1]->values[i].floatValue < OperandConstants[0]->values[i].floatValue ?
                OperandConstants[1]->values[i].floatValue : OperandConstants[0]->values[i].floatValue;
        }
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenMinCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_MIN,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateMax(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        if (slsDATA_TYPE_IsFloatOrVec(OperandConstants[1]->exprBase.dataType))
        {
            if (slsDATA_TYPE_IsFloat(OperandConstants[1]->exprBase.dataType))
            {
                values[i].floatValue =  OperandConstants[0]->values[i].floatValue < OperandConstants[1]->values[0].floatValue ?
                    OperandConstants[1]->values[0].floatValue : OperandConstants[0]->values[i].floatValue;
            }
            else
            {
                values[i].floatValue = OperandConstants[0]->values[i].floatValue < OperandConstants[1]->values[i].floatValue ?
                    OperandConstants[1]->values[i].floatValue : OperandConstants[0]->values[i].floatValue;
            }
        }
        else if (slsDATA_TYPE_IsIntOrIVec(OperandConstants[1]->exprBase.dataType))
        {
            if (slsDATA_TYPE_IsInt(OperandConstants[1]->exprBase.dataType))
            {
                values[i].intValue = OperandConstants[0]->values[i].intValue < OperandConstants[1]->values[0].intValue ?
                    OperandConstants[1]->values[0].intValue : OperandConstants[0]->values[i].intValue;
            }
            else if (slsDATA_TYPE_IsUInt(OperandConstants[1]->exprBase.dataType))
            {
                values[i].uintValue = OperandConstants[0]->values[i].uintValue < OperandConstants[1]->values[0].uintValue ?
                    OperandConstants[1]->values[0].uintValue : OperandConstants[0]->values[i].uintValue;
            }
            else
            {
                values[i].intValue = OperandConstants[0]->values[i].intValue < OperandConstants[1]->values[i].intValue ?
                    OperandConstants[1]->values[i].intValue : OperandConstants[0]->values[i].intValue;
            }
        }
        else
        {
            gcmASSERT(gcvFALSE);
            values[i].floatValue = OperandConstants[0]->values[i].floatValue < OperandConstants[1]->values[i].floatValue ?
                OperandConstants[1]->values[i].floatValue : OperandConstants[0]->values[i].floatValue;
        }
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenMaxCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_MAX,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateClamp(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount[3];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType) ||
              slsDATA_TYPE_IsIntOrIVec(OperandConstants[0]->exprBase.dataType));

    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    for (i = 0; i < OperandCount; i++)
    {
        componentCount[i] = (slmDATA_TYPE_vectorSize_GET(OperandConstants[i]->exprBase.dataType) == 0) ?
                            1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[i]->exprBase.dataType);
    }

    gcmASSERT(componentCount[1] == componentCount[2]);

    status = _EvaluateMax(Compiler, 2, OperandConstants, ResultConstant);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    for (i = 0; i < componentCount[0]; i++)
    {
        if (slsDATA_TYPE_IsFloatOrVec(OperandConstants[2]->exprBase.dataType))
        {
            if (slsDATA_TYPE_IsFloat(OperandConstants[2]->exprBase.dataType))
            {
                ResultConstant->values[i].floatValue = OperandConstants[2]->values[0].floatValue < ResultConstant->values[i].floatValue ?
                    OperandConstants[2]->values[0].floatValue : ResultConstant->values[i].floatValue;
            }
            else
            {
                ResultConstant->values[i].floatValue = OperandConstants[2]->values[i].floatValue < ResultConstant->values[i].floatValue ?
                    OperandConstants[2]->values[i].floatValue : ResultConstant->values[i].floatValue;
            }
        }
        else if (slsDATA_TYPE_IsIntOrIVec(OperandConstants[2]->exprBase.dataType))
        {
            if (slsDATA_TYPE_IsInt(OperandConstants[2]->exprBase.dataType))
            {
                ResultConstant->values[i].intValue = OperandConstants[2]->values[0].intValue < ResultConstant->values[i].intValue ?
                    OperandConstants[2]->values[0].intValue : ResultConstant->values[i].intValue;
            }
            else if (slsDATA_TYPE_IsUInt(OperandConstants[2]->exprBase.dataType))
            {
                ResultConstant->values[i].uintValue = OperandConstants[2]->values[0].uintValue < ResultConstant->values[i].uintValue ?
                    OperandConstants[2]->values[0].uintValue : ResultConstant->values[i].uintValue;
            }
            else
            {
                ResultConstant->values[i].intValue = OperandConstants[2]->values[i].intValue < ResultConstant->values[i].intValue ?
                    OperandConstants[2]->values[i].intValue : ResultConstant->values[i].intValue;
            }
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenClampCodeInner(
    IN sloCOMPILER Compiler,
    IN gctINT LineNo,
    IN gctINT StringNo,
    IN slsROPERAND *Clamp,
    IN slsROPERAND *MinVal,
    IN slsROPERAND *MaxVal,
    IN slsIOPERAND *IOperand
    )
{
    gceSTATUS status;
    gcmHEADER();

    if ((Compiler->context.optimizationOptions & slvOPTIMIZATION_CALCULATION)
        && slsROPERAND_IsFloatOrVecConstant(MinVal, 0.0)
        && slsROPERAND_IsFloatOrVecConstant(MaxVal, 1.0))
    {
        status = slGenGenericCode1(Compiler,
                                   LineNo,
                                   StringNo,
                                   slvOPCODE_SATURATE,
                                   IOperand,
                                   Clamp);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else
    {
        slsIOPERAND intermIOperand;
        slsROPERAND intermROperand;

        /* max t0, opd0, opd1 */
        slsIOPERAND_New(Compiler,
                        &intermIOperand,
                        IOperand->dataType,
                        GetHigherPrecison(Clamp->u.reg.precision, MinVal->u.reg.precision));

        status = slGenGenericCode2(Compiler,
                                   LineNo,
                                   StringNo,
                                   slvOPCODE_MAX,
                                   &intermIOperand,
                                   Clamp,
                                   MinVal);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        /* min result, t0, opd2 */
        slsROPERAND_InitializeUsingIOperand(&intermROperand, &intermIOperand);

        status = slGenGenericCode2(Compiler,
                                   LineNo,
                                   StringNo,
                                   slvOPCODE_MIN,
                                   IOperand,
                                   &intermROperand,
                                   MaxVal);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenClampCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS       status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status =  _GenClampCodeInner(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 &OperandsParameters[0].rOperands[0],
                                 &OperandsParameters[1].rOperands[0],
                                 &OperandsParameters[2].rOperands[0],
                                 IOperand);

    gcmFOOTER();
    return status;
}

static gceSTATUS
_Vec_Mul_VecOrFlat(
    IN sloIR_CONSTANT Operand0,
    IN sloIR_CONSTANT Operand1,
    IN gctUINT componentCount,
    IN gctBOOL isMinus,
    IN gctBOOL isFloat,
    IN OUT sluCONSTANT_VALUE *values)
{
    gctUINT i;
    for (i = 0; i < componentCount; i++)
    {
        if (isMinus)
        {
            if (isFloat)
                values[i].floatValue = Operand0->values[i].floatValue * (1.0f - Operand1->values[0].floatValue);
            else
                values[i].floatValue = Operand0->values[i].floatValue * (1.0f - Operand1->values[i].floatValue);
        }
        else
        {
            if (isFloat)
                values[i].floatValue = Operand0->values[i].floatValue * Operand1->values[0].floatValue;
            else
                values[i].floatValue = Operand0->values[i].floatValue * Operand1->values[i].floatValue;
        }
    }
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateMix(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount[3];
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    for (i = 0; i < OperandCount; i++)
    {
        componentCount[i] = (slmDATA_TYPE_vectorSize_GET(OperandConstants[i]->exprBase.dataType) == 0) ?
                            1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[i]->exprBase.dataType);
    }

    gcmASSERT(componentCount[0] == componentCount[1]);

    if(slmIsElementTypeBoolean(OperandConstants[2]->exprBase.dataType->elementType))
    {
        for (i = 0; i < componentCount[0]; i++)
        {
            if(OperandConstants[2]->values[i].boolValue)
            {
                values[i].intValue = OperandConstants[1]->values[i].intValue;
            }
            else
            {
                values[i].intValue = OperandConstants[0]->values[i].intValue;
            }
        }

        status = sloIR_CONSTANT_AddValues(Compiler,
                                          ResultConstant,
                                          componentCount[0],
                                          values);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));

    status = _Vec_Mul_VecOrFlat(OperandConstants[0], OperandConstants[2],
        componentCount[0], gcvTRUE, componentCount[2] == 1, values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount[0],
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = _Vec_Mul_VecOrFlat(OperandConstants[1], OperandConstants[2],
        componentCount[0], gcvFALSE, componentCount[2] == 1, values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    for (i = 0; i < componentCount[0]; i++)
    {
        ResultConstant->values[i].floatValue += values[i].floatValue;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenMixCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS       status;

    gcmHEADER();
    if (!sloCOMPILER_IsES31VersionOrAbove(Compiler))
    {
        slsIOPERAND     intermIOperands[2];
        slsROPERAND     intermROperands[2];


        /* Verify the arguments. */
        slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
        slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
        gcmASSERT(OperandCount == 3);
        gcmASSERT(OperandsParameters);
        gcmASSERT(IOperand);

        if(gcGetComponentDataType(OperandsParameters[2].dataTypes[0]) == gcSHADER_BOOLEAN_X1)
        {
            status = slGenSelectExprCode(Compiler,
                PolynaryExpr->exprBase.base.lineNo,
                PolynaryExpr->exprBase.base.stringNo,
                IOperand,
                OperandsParameters[2].rOperands,
                OperandsParameters[0].rOperands,
                OperandsParameters[1].rOperands);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

            gcmFOOTER_NO();
            return gcvSTATUS_OK;
        }

        /* sub t0, y, x */
        slsIOPERAND_New(Compiler,
            &intermIOperands[0],
            OperandsParameters[0].dataTypes[0],
            OperandsParameters[0].rOperands[0].u.reg.precision);

        status = slGenArithmeticExprCode(
            Compiler,
            PolynaryExpr->exprBase.base.lineNo,
            PolynaryExpr->exprBase.base.stringNo,
            slvOPCODE_SUB,
            &intermIOperands[0],
            &OperandsParameters[1].rOperands[0],
            &OperandsParameters[0].rOperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        /* mul t1, a, t0 */
        slsIOPERAND_New(Compiler,
            &intermIOperands[1],
            OperandsParameters[0].dataTypes[0],
            OperandsParameters[0].rOperands[0].u.reg.precision);
        slsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);

        status = slGenArithmeticExprCode(
            Compiler,
            PolynaryExpr->exprBase.base.lineNo,
            PolynaryExpr->exprBase.base.stringNo,
            slvOPCODE_MUL,
            &intermIOperands[1],
            &OperandsParameters[2].rOperands[0],
            &intermROperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        /* add result, t1, x */
        slsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

        status = slGenArithmeticExprCode(
            Compiler,
            PolynaryExpr->exprBase.base.lineNo,
            PolynaryExpr->exprBase.base.stringNo,
            slvOPCODE_ADD,
            IOperand,
            &OperandsParameters[0].rOperands[0],
            &intermROperands[1]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    }
    else
    {
        slsIOPERAND     intermIOperands[3];
        slsROPERAND     intermROperands[3];
        slsROPERAND     one;

        /* Verify the arguments. */
        slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
        slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
        gcmASSERT(OperandCount == 3);
        gcmASSERT(OperandsParameters);
        gcmASSERT(IOperand);

        if(gcGetComponentDataType(OperandsParameters[2].dataTypes[0]) == gcSHADER_BOOLEAN_X1)
        {
            status = slGenSelectExprCode(Compiler,
                PolynaryExpr->exprBase.base.lineNo,
                PolynaryExpr->exprBase.base.stringNo,
                IOperand,
                OperandsParameters[2].rOperands,
                OperandsParameters[0].rOperands,
                OperandsParameters[1].rOperands);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

            gcmFOOTER_NO();
            return gcvSTATUS_OK;
        }
        if (gcIsDoubleDataType(OperandsParameters[2].dataTypes[0]))
        {
            slsROPERAND_InitializeFloatOrVecOrMatConstant(&one,
                gcSHADER_FLOAT64_X1,
                gcSHADER_PRECISION_MEDIUM,
                (gctFLOAT) 1.0);
        }
        else
        {
            slsROPERAND_InitializeFloatOrVecOrMatConstant(&one,
                gcSHADER_FLOAT_X1,
                gcSHADER_PRECISION_MEDIUM,
                (gctFLOAT) 1.0);
        }
        /* sub t0, 1, a */
        slsIOPERAND_New(Compiler,
            &intermIOperands[0],
            OperandsParameters[2].dataTypes[0],
            OperandsParameters[2].rOperands[0].u.reg.precision);

        status = slGenArithmeticExprCode(
            Compiler,
            PolynaryExpr->exprBase.base.lineNo,
            PolynaryExpr->exprBase.base.stringNo,
            slvOPCODE_SUB,
            &intermIOperands[0],
            &one,
            &OperandsParameters[2].rOperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        /* mul t1, x, t0 */
        slsIOPERAND_New(Compiler,
            &intermIOperands[1],
            OperandsParameters[0].dataTypes[0],
            OperandsParameters[0].rOperands[0].u.reg.precision);
        slsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);

        status = slGenArithmeticExprCode(
            Compiler,
            PolynaryExpr->exprBase.base.lineNo,
            PolynaryExpr->exprBase.base.stringNo,
            slvOPCODE_MUL,
            &intermIOperands[1],
            &OperandsParameters[0].rOperands[0],
            &intermROperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        /* mul t2, y, a */
        slsIOPERAND_New(Compiler,
            &intermIOperands[2],
            OperandsParameters[1].dataTypes[0],
            OperandsParameters[1].rOperands[0].u.reg.precision);

        status = slGenArithmeticExprCode(
            Compiler,
            PolynaryExpr->exprBase.base.lineNo,
            PolynaryExpr->exprBase.base.stringNo,
            slvOPCODE_MUL,
            &intermIOperands[2],
            &OperandsParameters[1].rOperands[0],
            &OperandsParameters[2].rOperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        /* add result, t1, t2 */
        slsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);
        slsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);
        status = slGenArithmeticExprCode(
            Compiler,
            PolynaryExpr->exprBase.base.lineNo,
            PolynaryExpr->exprBase.base.stringNo,
            slvOPCODE_ADD,
            IOperand,
            &intermROperands[1],
            &intermROperands[2]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenAcosCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();
    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_ACOS,
                            IOperand,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenAsinCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_ASIN,
                            IOperand,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenAtanCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_ATAN,
                            IOperand,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateStep(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[1]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[1]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[1]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[1]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        if (slsDATA_TYPE_IsFloat(OperandConstants[0]->exprBase.dataType))
        {
            values[i].floatValue = OperandConstants[1]->values[i].floatValue < OperandConstants[0]->values[0].floatValue ?
                0.0f : 1.0f;
        }
        else
        {
            values[i].floatValue = OperandConstants[1]->values[i].floatValue < OperandConstants[0]->values[i].floatValue ?
                0.0f : 1.0f;
        }
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenStepCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_STEP,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateSmoothStep(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount[3];
    sluCONSTANT_VALUE   values[4];
    sloIR_CONSTANT tempConstants[3];
    slsDATA_TYPE *      dataType;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[2]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[2]->exprBase.dataType));

    for (i = 0; i < OperandCount; i++)
    {
        componentCount[i] = (slmDATA_TYPE_vectorSize_GET(OperandConstants[i]->exprBase.dataType) == 0) ?
                            1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[i]->exprBase.dataType);
    }

    gcmASSERT(componentCount[0] == componentCount[1]);

    status = sloIR_CONSTANT_Clone(
                                    Compiler,
                                    OperandConstants[2]->exprBase.base.lineNo,
                                    OperandConstants[2]->exprBase.base.stringNo,
                                    OperandConstants[2],
                                    &tempConstants[0]);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    for (i = 0; i < componentCount[2]; i++)
    {
        if (slsDATA_TYPE_IsFloat(OperandConstants[0]->exprBase.dataType))
        {
            tempConstants[0]->values[i].floatValue =
                (OperandConstants[2]->values[i].floatValue - OperandConstants[0]->values[0].floatValue) /
                (OperandConstants[1]->values[0].floatValue - OperandConstants[0]->values[0].floatValue);
        }
        else
        {
            tempConstants[0]->values[i].floatValue =
                (OperandConstants[2]->values[i].floatValue - OperandConstants[0]->values[i].floatValue) /
                (OperandConstants[1]->values[i].floatValue - OperandConstants[0]->values[i].floatValue);
        }
    }

    status = sloCOMPILER_CreateDataType(
                                        Compiler,
                                        T_FLOAT,
                                        gcvNULL,
                                        &dataType);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    dataType->qualifiers.storage = slvSTORAGE_QUALIFIER_CONST;

    for (i = 0; i < 2; i++)
    {
        values[0].floatValue = (gctFLOAT)i;
        status = sloIR_CONSTANT_Construct(
                                        Compiler,
                                        OperandConstants[0]->exprBase.base.lineNo,
                                        OperandConstants[0]->exprBase.base.stringNo,
                                        dataType,
                                        &tempConstants[i + 1]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        status = sloIR_CONSTANT_AddValues(
                                        Compiler,
                                        tempConstants[i + 1],
                                        1,
                                        values);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    status = _EvaluateClamp(Compiler, 3, tempConstants, ResultConstant);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    for (i = 0; i < componentCount[2]; i++)
    {
        ResultConstant->values[i].floatValue =
            ResultConstant->values[i].floatValue * ResultConstant->values[i].floatValue *
            (3.0f - 2.0f * ResultConstant->values[i].floatValue);
    }

    for (i = 0; i < 3; i++)
    {
        status = sloIR_CONSTANT_Destroy(Compiler, &tempConstants[i]->exprBase.base);
    }

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenSmoothStepCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS       status;
    slsIOPERAND     intermIOperands[7];
    slsROPERAND     intermROperands[7];
    slsROPERAND     constantROperand;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* sub t0, x (operand2), edge0 (operand0) */
    slsIOPERAND_New(Compiler,
                    &intermIOperands[0],
                    OperandsParameters[2].dataTypes[0],
                    OperandsParameters[2].rOperands[0].u.reg.precision);

    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_SUB,
                                    &intermIOperands[0],
                                    &OperandsParameters[2].rOperands[0],
                                    &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* sub t1, edge1 (operand1), edge0 (operand0) */
    slsIOPERAND_New(Compiler,
                    &intermIOperands[1],
                    OperandsParameters[1].dataTypes[0],
                    OperandsParameters[1].rOperands[0].u.reg.precision);

    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_SUB,
                                    &intermIOperands[1],
                                    &OperandsParameters[1].rOperands[0],
                                    &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* div t2, t0, t1 */
    slsIOPERAND_New(Compiler,
                    &intermIOperands[2],
                    intermIOperands[0].dataType,
                    intermIOperands[0].precision);
    slsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);
    slsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_DIV,
                                    &intermIOperands[2],
                                    &intermROperands[0],
                                    &intermROperands[1]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* sat t3, t2 */
    slsIOPERAND_New(Compiler,
                    &intermIOperands[3],
                    intermIOperands[2].dataType,
                    intermIOperands[2].precision);
    slsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_SATURATE,
                            &intermIOperands[3],
                            &intermROperands[2]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* mul t4, t3, t3 */
    slsIOPERAND_New(Compiler,
                    &intermIOperands[4],
                    intermIOperands[3].dataType,
                    intermIOperands[3].precision);
    slsROPERAND_InitializeUsingIOperand(&intermROperands[3], &intermIOperands[3]);

    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    &intermIOperands[4],
                                    &intermROperands[3],
                                    &intermROperands[3]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* add t5, t3, t3   ==> mul t5, t3, 2 */
    slsIOPERAND_New(Compiler,
                    &intermIOperands[5],
                    intermIOperands[3].dataType,
                    intermIOperands[3].precision);

    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_ADD,
                                    &intermIOperands[5],
                                    &intermROperands[3],
                                    &intermROperands[3]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* sub t6, 3.0, t5 */
    slsIOPERAND_New(Compiler,
                    &intermIOperands[6],
                    intermIOperands[5].dataType,
                    intermIOperands[5].precision);
    if (gcIsDoubleDataType(intermIOperands[5].dataType))
    {
        slsROPERAND_InitializeFloatOrVecOrMatConstant(&constantROperand,
                                                      gcSHADER_FLOAT64_X1,
                                                      gcSHADER_PRECISION_MEDIUM,
                                                      (gctFLOAT)3.0);
    }
    else
    {
        slsROPERAND_InitializeFloatOrVecOrMatConstant(&constantROperand,
                                                      gcSHADER_FLOAT_X1,
                                                      gcSHADER_PRECISION_MEDIUM,
                                                      (gctFLOAT)3.0);
    }
    slsROPERAND_InitializeUsingIOperand(&intermROperands[5], &intermIOperands[5]);

    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_SUB,
                                    &intermIOperands[6],
                                    &constantROperand,
                                    &intermROperands[5]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* mul result, t4, t6 */
    slsROPERAND_InitializeUsingIOperand(&intermROperands[4], &intermIOperands[4]);
    slsROPERAND_InitializeUsingIOperand(&intermROperands[6], &intermIOperands[6]);

    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    IOperand,
                                    &intermROperands[4],
                                    &intermROperands[6]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateIsNan(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        values[i].boolValue = ((OperandConstants[0]->values[i].uintValue & (gctUINT) 0x7FFFFFFF)
                                                                         > (gctUINT) 0x7F800000);
    }

    status = sloIR_CONSTANT_AddValues(Compiler,
                                      ResultConstant,
                                      componentCount,
                                      values);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenIsNanCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    slsIOPERAND intermIOperand[1];
    slsROPERAND    infROperand, unsignROperand, intermROperand[1];
    slsLOPERAND lOperand[1];
    gcSHADER_TYPE type;
    gctUINT8 componentCount;
    gcSHADER_PRECISION origPrecision = IOperand->precision;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    slsROPERAND_InitializeIntOrIVecConstant(&infROperand,
                                            gcSHADER_UINT_X1,
                                            gcSHADER_PRECISION_HIGH,
                                            (gctUINT) 0x7F800000);
    slsROPERAND_InitializeIntOrIVecConstant(&unsignROperand,
                                            gcSHADER_UINT_X1,
                                            gcSHADER_PRECISION_HIGH,
                                            (gctUINT) 0x7FFFFFFF);

    componentCount = gcGetVectorDataTypeComponentCount(OperandsParameters[0].rOperands[0].dataType);
    type = gcConvScalarToVectorDataType(gcSHADER_UINT_X1, componentCount);

    /* Move the FLOAT source to a UINT source first. */
    slsIOPERAND_New(Compiler,
                    intermIOperand,
                    type,
                    gcSHADER_PRECISION_HIGH);
    slsLOPERAND_InitializeUsingIOperand(lOperand, intermIOperand);

    intermROperand[0] = OperandsParameters[0].rOperands[0];
    intermROperand[0].dataType = type;
    status = slGenAssignCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             lOperand,
                             intermROperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsIOPERAND_New(Compiler,
                    intermIOperand,
                    type,
                    gcSHADER_PRECISION_HIGH);
    slsROPERAND_InitializeUsingIOperand(intermROperand, intermIOperand);

    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_AND_BITWISE,
                               intermIOperand,
                               &unsignROperand,
                               &OperandsParameters[0].rOperands[0]);
    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    /* Use HIGHP precision so it won't execute under dual16. */
    IOperand->precision = gcSHADER_PRECISION_HIGH;
    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_GREATER_THAN,
                               IOperand,
                               intermROperand,
                               &infROperand);
    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }
    IOperand->precision = origPrecision;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateIsInf(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        values[i].boolValue = ((OperandConstants[0]->values[i].uintValue & (gctUINT) 0x7FFFFFFF)
                                                                        == (gctUINT) 0x7F800000);
    }

    status = sloIR_CONSTANT_AddValues(Compiler,
                                      ResultConstant,
                                      componentCount,
                                      values);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenIsInfCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    slsIOPERAND intermIOperand[1];
    slsROPERAND    infROperand, unsignROperand, intermROperand[1];
    gcSHADER_TYPE type;
    gctUINT8 componentCount;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    slsROPERAND_InitializeIntOrIVecConstant(&infROperand,
                                            gcSHADER_UINT_X1,
                                            gcSHADER_PRECISION_HIGH,
                                            (gctUINT) 0x7F800000);
    slsROPERAND_InitializeIntOrIVecConstant(&unsignROperand,
                                            gcSHADER_UINT_X1,
                                            gcSHADER_PRECISION_HIGH,
                                            (gctUINT) 0x7FFFFFFF);

    componentCount = gcGetVectorDataTypeComponentCount(OperandsParameters[0].rOperands[0].dataType);
    type = gcConvScalarToVectorDataType(gcSHADER_UINT_X1, componentCount);

    slsIOPERAND_New(Compiler,
                    intermIOperand,
                    type,
                    gcSHADER_PRECISION_HIGH);
    slsROPERAND_InitializeUsingIOperand(intermROperand, intermIOperand);

    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_AND_BITWISE,
                               intermIOperand,
                               &unsignROperand,
                               &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    status = slGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_EQUAL,
                               IOperand,
                               intermROperand,
                               &infROperand);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateFloatBitsToInteger(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS   status;
    gctUINT     componentCount;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slmIsElementTypeFloatingOrDouble(OperandConstants[0]->exprBase.dataType->elementType));
    gcmASSERT(slmIsElementTypeInteger(ResultConstant->exprBase.dataType->elementType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    status = sloIR_CONSTANT_AddValues(Compiler,
                                      ResultConstant,
                                      componentCount,
                                      OperandConstants[0]->values);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenFloatBitsToIntCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    slsROPERAND rOperand[1];
    slsLOPERAND lOperand[1];
    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    rOperand[0] = OperandsParameters[0].rOperands[0];
    rOperand->dataType = IOperand->dataType;
    IOperand->precision = gcSHADER_PRECISION_HIGH;          /* Force precision to high as required by spec */
    slsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);

    status = slGenAssignCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             lOperand,
                             rOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenFloatBitsToUintCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    slsROPERAND rOperand[1];
    slsLOPERAND lOperand[1];
    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    rOperand[0] = OperandsParameters[0].rOperands[0];
    rOperand->dataType = IOperand->dataType;

    /* Force precision to high as required by spec */
    IOperand->precision = gcSHADER_PRECISION_HIGH;
    slsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);

    status = slGenAssignCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             lOperand,
                             rOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateIntegerBitsToFloat(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS     status;
    gctUINT       componentCount;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slmIsElementTypeInteger(OperandConstants[0]->exprBase.dataType->elementType));
    gcmASSERT(slmIsElementTypeFloatingOrDouble(ResultConstant->exprBase.dataType->elementType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    status = sloIR_CONSTANT_AddValues(Compiler,
                                      ResultConstant,
                                      componentCount,
                                      OperandConstants[0]->values);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenIntBitsToFloatCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    slsROPERAND rOperand[1];
    slsLOPERAND lOperand[1];
    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    rOperand[0] = OperandsParameters[0].rOperands[0];
    rOperand->dataType = IOperand->dataType;
    slsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);

    /* Force precision to high as required by spec */
    IOperand->precision = gcSHADER_PRECISION_HIGH;
    status = slGenAssignCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             lOperand,
                             rOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenUintBitsToFloatCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    slsROPERAND rOperand[1];
    slsLOPERAND lOperand[1];
    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    rOperand[0] = OperandsParameters[0].rOperands[0];
    rOperand->dataType = IOperand->dataType;
    slsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);

    /* Force precision to high as required by spec */
    IOperand->precision = gcSHADER_PRECISION_HIGH;
    status = slGenAssignCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             lOperand,
                             rOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluatePackSnorm2x16(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   minValue[2];
    sluCONSTANT_VALUE   maxValue[2];
    struct _sloIR_CONSTANT minConst[1];
    struct _sloIR_CONSTANT maxConst[1];
    sloIR_CONSTANT tempConst = gcvNULL, resConst = gcvNULL;
    sloIR_CONSTANT constArgs[3];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsInt(ResultConstant->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    gcmASSERT(componentCount == 2);

    minValue[0].floatValue = minValue[1].floatValue = -1.0;
    maxValue[0].floatValue = maxValue[1].floatValue = 1.0;
    status = sloIR_CONSTANT_Initialize(Compiler,
                                       0,
                                       0,
                                       OperandConstants[0]->exprBase.dataType,
                                       2,
                                       minValue,
                                       minConst);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    status = sloIR_CONSTANT_Initialize(Compiler,
                                       0,
                                       0,
                                       OperandConstants[0]->exprBase.dataType,
                                       2,
                                       maxValue,
                                       maxConst);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = sloIR_CONSTANT_Construct(Compiler,
                                      0,
                                      0,
                                      OperandConstants[0]->exprBase.dataType,
                                      &tempConst);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    constArgs[0] = OperandConstants[0];
    constArgs[1] = minConst;
    constArgs[2] = maxConst;

    gcmONERROR(_EvaluateClamp(Compiler,
                              3,
                              constArgs,
                              tempConst));

    for (i = 0; i < componentCount; i++)
    {
        tempConst->values[i].floatValue *= 32767.0;
    }

    status = sloIR_CONSTANT_Construct(Compiler,
                                      0,
                                      0,
                                      OperandConstants[0]->exprBase.dataType,
                                      &resConst);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    constArgs[0] = tempConst;
    gcmONERROR(_EvaluateRound(Compiler,
                              1,
                              constArgs,
                              resConst));

    resConst->values[0].uintValue = (gctUINT)(((gctINT)(resConst->values[0].floatValue) & 0x0000FFFF) |
                                              ((gctINT)(resConst->values[1].floatValue) << 16));
    gcmONERROR(sloIR_CONSTANT_AddValues(Compiler,
                                        ResultConstant,
                                        1,
                                        resConst->values));

OnError:
    if(tempConst)
    {
       sloIR_CONSTANT_Destroy(Compiler, &tempConst->exprBase.base);
    }

    if(resConst)
    {
       sloIR_CONSTANT_Destroy(Compiler, &resConst->exprBase.base);
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateUnpackSnorm2x16(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   minValue[2];
    sluCONSTANT_VALUE   maxValue[2];
    sluCONSTANT_VALUE   unpackedValue[2];
    struct _sloIR_CONSTANT minConst[1];
    struct _sloIR_CONSTANT maxConst[1];
    struct _sloIR_CONSTANT unpackedConst[1];
    sloIR_CONSTANT constArgs[3];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsInt(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(ResultConstant->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(ResultConstant->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(ResultConstant->exprBase.dataType);

    gcmASSERT(componentCount == 2);

    minValue[0].floatValue = minValue[1].floatValue = -1.0;
    maxValue[0].floatValue = maxValue[1].floatValue = 1.0;
    status = sloIR_CONSTANT_Initialize(Compiler,
                                       0,
                                       0,
                                       ResultConstant->exprBase.dataType,
                                       2,
                                       minValue,
                                       minConst);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    status = sloIR_CONSTANT_Initialize(Compiler,
                                       0,
                                       0,
                                       ResultConstant->exprBase.dataType,
                                       2,
                                       maxValue,
                                       maxConst);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    unpackedValue[0].floatValue = (gctFLOAT)((OperandConstants[0]->values[0].intValue << 16) >> 16);
    unpackedValue[1].floatValue = (gctFLOAT)(OperandConstants[0]->values[0].intValue >> 16);

    status = sloIR_CONSTANT_Initialize(Compiler,
                                       0,
                                       0,
                                       ResultConstant->exprBase.dataType,
                                       2,
                                       unpackedValue,
                                       unpackedConst);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    for (i = 0; i < componentCount; i++)
    {
        unpackedValue[i].floatValue *= (gctFLOAT)(1/32767.0);
    }
    constArgs[0] = unpackedConst;
    constArgs[1] = minConst;
    constArgs[2] = maxConst;

    status = _EvaluateClamp(Compiler,
                            3,
                            constArgs,
                            ResultConstant);

    gcmFOOTER();
    return status;
}

gceSTATUS
_EvaluatePackUnorm2x16(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   minValue[2];
    sluCONSTANT_VALUE   maxValue[2];
    struct _sloIR_CONSTANT minConst[1];
    struct _sloIR_CONSTANT maxConst[1];
    sloIR_CONSTANT tempConst = gcvNULL, resConst = gcvNULL;
    sloIR_CONSTANT constArgs[3];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsInt(ResultConstant->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    gcmASSERT(componentCount == 2);

    minValue[0].floatValue = minValue[1].floatValue = 0.0;
    maxValue[0].floatValue = maxValue[1].floatValue = 1.0;
    status = sloIR_CONSTANT_Initialize(Compiler,
                                       0,
                                       0,
                                       OperandConstants[0]->exprBase.dataType,
                                       2,
                                       minValue,
                                       minConst);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    status = sloIR_CONSTANT_Initialize(Compiler,
                                       0,
                                       0,
                                       OperandConstants[0]->exprBase.dataType,
                                       2,
                                       maxValue,
                                       maxConst);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = sloIR_CONSTANT_Construct(Compiler,
                                      0,
                                      0,
                                      OperandConstants[0]->exprBase.dataType,
                                      &tempConst);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    constArgs[0] = OperandConstants[0];
    constArgs[1] = minConst;
    constArgs[2] = maxConst;

    gcmONERROR(_EvaluateClamp(Compiler,
                              3,
                              constArgs,
                              tempConst));

    for (i = 0; i < componentCount; i++)
    {
        tempConst->values[i].floatValue *= 65535.0;
    }

    status = sloIR_CONSTANT_Construct(Compiler,
                                      0,
                                      0,
                                      OperandConstants[0]->exprBase.dataType,
                                      &resConst);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    constArgs[0] = tempConst;
    gcmONERROR(_EvaluateRound(Compiler,
                              1,
                              constArgs,
                              resConst));

    resConst->values[0].uintValue = (gctUINT)(((gctUINT)(resConst->values[0].floatValue) & 0x0000FFFF) |
                                              ((gctUINT)(resConst->values[1].floatValue) << 16));
    gcmONERROR(sloIR_CONSTANT_AddValues(Compiler,
                                        ResultConstant,
                                        1,
                                        resConst->values));

OnError:
    if(tempConst)
    {
       sloIR_CONSTANT_Destroy(Compiler, &tempConst->exprBase.base);
    }

    if(resConst)
    {
       sloIR_CONSTANT_Destroy(Compiler, &resConst->exprBase.base);
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateUnpackUnorm2x16(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             componentCount;
    sluCONSTANT_VALUE   unpackedValue[2];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsInt(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(ResultConstant->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(ResultConstant->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(ResultConstant->exprBase.dataType);
    gcmASSERT(componentCount == 2);

    unpackedValue[0].floatValue = (gctFLOAT)(OperandConstants[0]->values[0].uintValue & 0x0000FFFF) * (gctFLOAT)(1.0/65535.0);
    unpackedValue[1].floatValue = (gctFLOAT)(OperandConstants[0]->values[0].uintValue >> 16) * (gctFLOAT)(1.0/65535.0);

    status = sloIR_CONSTANT_AddValues(Compiler,
                                      ResultConstant,
                                      componentCount,
                                      unpackedValue);
    gcmFOOTER();
    return status;
}


static void
_ConvertFP32ToFP16(
    IN gctUINT FP32,
    OUT gctUINT * FP16
    )
{
    gctUINT fp16 = 0u;

    fp16 = ((FP32 & 0x80000000u) >> 16)  /* sign bit. */
         | ((FP32 & 0x0F800000u) >> 13)  /* Exponent bits */
         | ((FP32 & 0x007FE000u) >> 13); /* Mantissa bits */

    if (FP16)
    {
        *FP16 = fp16;
    }
}

static void
_ConvertFP16ToFP32(
    IN gctUINT FP16,
    OUT gctUINT * FP32
    )
{
    gctUINT fp32 = 0u;
    gctUINT bias = 0x30000000u;

    fp32 = ((FP16 & 0x8000u)  << 16)          /* sign bit. */
         | (((FP16 & 0x7C00u) << 13) + bias)  /* Exponent bits */
         | ((FP16 & 0x03FFu)  << 13);         /* Mantissa bits */

    if (FP32)
    {
        *FP32 = fp32;
    }
}

gceSTATUS
_EvaluatePackHalf2x16(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gctUINT32           fp32[2] = {0u, 0u};
    sluCONSTANT_VALUE   value = {0};

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 2);

    /* Convert FP32 to FP16. */
    _ConvertFP32ToFP16(OperandConstants[0]->values[0].uintValue, &fp32[0]);
    _ConvertFP32ToFP16(OperandConstants[0]->values[1].uintValue, &fp32[1]);

    value.uintValue = fp32[0] | (fp32[1] << 16);

    status = sloIR_CONSTANT_AddValues(Compiler,
                                      ResultConstant,
                                      1,
                                      &value);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return status;
}

gceSTATUS
_EvaluateUnpackHalf2x16(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    sluCONSTANT_VALUE   values[2] = {{0}, {0}};

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsUInt(OperandConstants[0]->exprBase.dataType));

    _ConvertFP16ToFP32(OperandConstants[0]->values[0].uintValue & 0xFFFF, &values[0].uintValue);
    _ConvertFP16ToFP32((OperandConstants[0]->values[0].uintValue >> 16) & 0xFFFF, &values[1].uintValue);

    status = sloIR_CONSTANT_AddValues(Compiler,
                                      ResultConstant,
                                      2,
                                      values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return status;
}

gceSTATUS
_EvaluateLength(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsFloat(ResultConstant->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    values[0].floatValue = 0.0f;

    for (i = 0; i < componentCount; i++)
    {
        values[0].floatValue += OperandConstants[0]->values[i].floatValue * OperandConstants[0]->values[i].floatValue;
    }

    values[0].floatValue = gcoMATH_SquareRoot(values[0].floatValue);

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    1,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenLengthCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS       status;
    slsIOPERAND     intermIOperand;
    slsROPERAND     intermROperand;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (gcIsScalarDataType(OperandsParameters[0].dataTypes[0]))
    {
        /* abs result, x */
        status = slGenGenericCode1(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                slvOPCODE_ABS,
                                IOperand,
                                &OperandsParameters[0].rOperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else
    {
        /* dot t0, x, x */
        slsIOPERAND_New(Compiler,
                        &intermIOperand,
                        gcSHADER_FLOAT_X1,
                        OperandsParameters[0].rOperands[0].u.reg.precision);

        status = slGenGenericCode2(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                slvOPCODE_DOT,
                                &intermIOperand,
                                &OperandsParameters[0].rOperands[0],
                                &OperandsParameters[0].rOperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        /* sqrt result, t0 */
        slsROPERAND_InitializeUsingIOperand(&intermROperand, &intermIOperand);

        status = slGenGenericCode1(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                slvOPCODE_SQRT,
                                IOperand,
                                &intermROperand);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateDistance(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount[2] = {0};
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloat(ResultConstant->exprBase.dataType));

    for (i = 0; i < OperandCount; i++)
    {
        gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
        componentCount[i] = (slmDATA_TYPE_vectorSize_GET(OperandConstants[i]->exprBase.dataType) == 0) ?
                            1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[i]->exprBase.dataType);
    }

    gcmASSERT(componentCount[0] == componentCount[1]);

    values[0].floatValue = 0.0f;

    for (i = 0; i < componentCount[0]; i++)
    {
        values[0].floatValue +=
            (OperandConstants[0]->values[i].floatValue - OperandConstants[1]->values[i].floatValue) *
            (OperandConstants[0]->values[i].floatValue - OperandConstants[1]->values[i].floatValue);
    }

    values[0].floatValue = gcoMATH_SquareRoot(values[0].floatValue);

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    1,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenDistanceCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS       status;
    slsIOPERAND     intermIOperands[2];
    slsROPERAND     intermROperands[2];
    gcSHADER_PRECISION precision;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* mul t0, m[0], v.x */
    if(slmROPERAND_IsHigherPrecision(OperandsParameters[1].rOperands,
                                     OperandsParameters[0].rOperands))
    {
        precision = OperandsParameters[1].rOperands[0].u.reg.precision;
    }
    else
    {
        precision = OperandsParameters[0].rOperands[0].u.reg.precision;
    }
    /* sub t0, p0, p1 */
    slsIOPERAND_New(Compiler,
                    &intermIOperands[0],
                    OperandsParameters[0].dataTypes[0],
                    precision);

    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_SUB,
                                    &intermIOperands[0],
                                    &OperandsParameters[0].rOperands[0],
                                    &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    slsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);

    if (gcIsScalarDataType(OperandsParameters[0].dataTypes[0]))
    {
        /* abs result, t0 */
        status = slGenGenericCode1(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                slvOPCODE_ABS,
                                IOperand,
                                &intermROperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else
    {
        /* dot t1, t0, t0 */
        slsIOPERAND_New(Compiler,
                        &intermIOperands[1],
                        gcSHADER_FLOAT_X1,
                        intermROperands[0].u.reg.precision);

        status = slGenGenericCode2(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                slvOPCODE_DOT,
                                &intermIOperands[1],
                                &intermROperands[0],
                                &intermROperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        /* sqrt result, t1 */
        slsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

        status = slGenGenericCode1(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                slvOPCODE_SQRT,
                                IOperand,
                                &intermROperands[1]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateDot(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount[2] = {0};
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloat(ResultConstant->exprBase.dataType));

    for (i = 0; i < OperandCount; i++)
    {
        gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
        componentCount[i] = (slmDATA_TYPE_vectorSize_GET(OperandConstants[i]->exprBase.dataType) == 0) ?
                            1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[i]->exprBase.dataType);
    }

    gcmASSERT(componentCount[0] == componentCount[1]);

    values[0].floatValue = 0.0f;

    for (i = 0; i < componentCount[0]; i++)
    {
        values[0].floatValue += OperandConstants[0]->values[i].floatValue * OperandConstants[1]->values[i].floatValue;
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    1,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenDotCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_DOT,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateCross(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount[2] = {0};
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    for (i = 0; i < OperandCount; i++)
    {
        gcmASSERT(slsDATA_TYPE_IsVec(OperandConstants[0]->exprBase.dataType));
        componentCount[i] = (slmDATA_TYPE_vectorSize_GET(OperandConstants[i]->exprBase.dataType) == 0) ?
                            1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[i]->exprBase.dataType);
    }

    gcmASSERT(componentCount[0] == componentCount[1] && componentCount[0] == 3);

    values[0].floatValue = OperandConstants[0]->values[1].floatValue * OperandConstants[1]->values[2].floatValue -
        OperandConstants[0]->values[2].floatValue * OperandConstants[1]->values[1].floatValue;

    values[1].floatValue = OperandConstants[0]->values[2].floatValue * OperandConstants[1]->values[0].floatValue -
        OperandConstants[0]->values[0].floatValue * OperandConstants[1]->values[2].floatValue;

    values[2].floatValue = OperandConstants[0]->values[0].floatValue * OperandConstants[1]->values[1].floatValue -
        OperandConstants[0]->values[1].floatValue * OperandConstants[1]->values[0].floatValue;

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount[0],
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenCrossCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_CROSS,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateNormalize(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sloIR_CONSTANT tempConstant;
    slsDATA_TYPE *      dataType;
    sluCONSTANT_VALUE values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    status = sloCOMPILER_CreateDataType(
                                        Compiler,
                                        T_FLOAT,
                                        gcvNULL,
                                        &dataType);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    dataType->qualifiers.storage = slvSTORAGE_QUALIFIER_CONST;

    status = sloIR_CONSTANT_Construct(
                                    Compiler,
                                    OperandConstants[0]->exprBase.base.lineNo,
                                    OperandConstants[0]->exprBase.base.stringNo,
                                    dataType,
                                    &tempConstant);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = _EvaluateLength(Compiler, OperandCount, OperandConstants, tempConstant);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    for (i = 0; i < componentCount; i++)
    {
        values[i].floatValue = OperandConstants[0]->values[i].floatValue / tempConstant->values[0].floatValue;
    }

   status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = sloIR_CONSTANT_Destroy(Compiler, &tempConstant->exprBase.base);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenNormalizeCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    gcePATCH_ID patchId = sloCOMPILER_GetPatchID(Compiler);

    gcmHEADER();

    if (patchId == gcvPATCH_CARCHALLENGE || patchId == gcvPATCH_HEROESCALL || patchId == gcvPATCH_GLOFTF3HM || patchId == gcvPATCH_CRAZYRACING || patchId == gcvPATCH_MONOPOLY || patchId == gcvPATCH_SNOWCOLD || patchId == gcvPATCH_BM3 || patchId == gcvPATCH_CTGL20
        || patchId == gcvPATCH_FSBHAWAIIF || patchId == gcvPATCH_EADGKEEPER || patchId == gcvPATCH_WHRKYZIXOVAN || patchId == gcvPATCH_UIMARK || patchId == gcvPATCH_BASEMARKOSIICN || gcdPROC_IS_WEBGL(patchId))
    {
        /* Add some special code for normalize(x).
        target:
        normalize(x)  ==>  if(length2(x) == 0.0)
            return 0.0;
        else
            return  normalize(x).
        reason:
        when the param x 's length of normalize(x) is equal to 0, just the x  is a zero vectro.
        It is a invalid input.
        The return value is undefined. It depend on the vendor implement. Our driver return
        a very large vector, but the desktop graphic card return a zero vector. So we want to make the result
        to be same.
        */
        slsSELECTION_CONTEXT    selectionContext = {0};
        slsROPERAND             constantROperand;
        slsLOPERAND             lOperand;
        slsIOPERAND             intermIOperand;
        slsROPERAND             intermROperand;

        /* Verify the arguments. */
        slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
        slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
        gcmASSERT(OperandCount == 1);
        gcmASSERT(OperandsParameters);
        gcmASSERT(IOperand);

        if (patchId == gcvPATCH_CTGL20)
        {
            /* we found underflow problem in ctgl20 in dual16, thus need highp */
            slsIOPERAND_New(Compiler,
                            &intermIOperand,
                            gcSHADER_FLOAT_X1,
                            gcSHADER_PRECISION_HIGH);
        }
        else
        {
             slsIOPERAND_New(Compiler,
                        &intermIOperand,
                        gcSHADER_FLOAT_X1,
                        OperandsParameters[0].rOperands[0].u.reg.precision);
        }
        slsROPERAND_InitializeFloatOrVecOrMatConstant(&constantROperand,
                                                      IOperand->dataType,
                                                      gcSHADER_PRECISION_MEDIUM,
                                                      (gctFLOAT)0.0);
        /* Compute the length of Roperand. */
        status = slGenGenericCode2(
            Compiler,
            PolynaryExpr->exprBase.base.lineNo,
            PolynaryExpr->exprBase.base.stringNo,
            slvOPCODE_DOT,
            &intermIOperand,
            &OperandsParameters[0].rOperands[0],
            &OperandsParameters[0].rOperands[0]);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }

        slsROPERAND_InitializeUsingIOperand(&intermROperand, &intermIOperand);
        /* Selection Begin */
        status = slDefineSelectionBegin(
            Compiler,
            CodeGenerator,
            gcvTRUE,
            &selectionContext);

        /* Generate the code of the condition expression */
        status = slGenCompareJumpCode(
            Compiler,
            CodeGenerator,
            PolynaryExpr->exprBase.base.lineNo,
            PolynaryExpr->exprBase.base.stringNo,
            (selectionContext.hasFalseOperand) ?
            selectionContext.beginLabelOfFalseOperand : selectionContext.endLabel,
            gcvFALSE,
            slvCONDITION_EQUAL,
            &intermROperand,
            &constantROperand);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }

        /* Generate the code of the true operand:
        mov intermIOperand 0.0;
        */
        status = slDefineSelectionTrueOperandBegin(
            Compiler,
            CodeGenerator,
            &selectionContext);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }

        slsLOPERAND_InitializeUsingIOperand(&lOperand, IOperand);
        status = slGenAssignCode(
            Compiler,
            PolynaryExpr->exprBase.base.lineNo,
            PolynaryExpr->exprBase.base.stringNo,
            &lOperand,
            &constantROperand);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }

        status = slDefineSelectionTrueOperandEnd(
            Compiler,
            CodeGenerator,
            &selectionContext,
            0);
        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }
        /* Generate the code of the false operand:
        mov IOperand pow();
        */
        status = slDefineSelectionFalseOperandBegin(
            Compiler,
            CodeGenerator,
            &selectionContext);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        status = slGenGenericCode1(
            Compiler,
            PolynaryExpr->exprBase.base.lineNo,
            PolynaryExpr->exprBase.base.stringNo,
            slvOPCODE_NORMALIZE,
            IOperand,
            &OperandsParameters[0].rOperands[0]);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }

        status = slDefineSelectionFalseOperandEnd(
            Compiler,
            CodeGenerator,
            &selectionContext);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }

        /* Selection End */
        status = slDefineSelectionEnd(
            Compiler,
            CodeGenerator,
            &selectionContext);
    }
    else
    {
        /* Verify the arguments. */
        slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
        slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
        gcmASSERT(OperandCount == 1);
        gcmASSERT(OperandsParameters);
        gcmASSERT(IOperand);

        status = slGenGenericCode1(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                slvOPCODE_NORMALIZE,
                                IOperand,
                                &OperandsParameters[0].rOperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateFaceForward(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount[3];
    sluCONSTANT_VALUE   values[4];
    sloIR_CONSTANT tempConstants[3];
    slsDATA_TYPE *      dataType;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    for (i = 0; i < OperandCount; i++)
    {
        gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[i]->exprBase.dataType));
        componentCount[i] = (slmDATA_TYPE_vectorSize_GET(OperandConstants[i]->exprBase.dataType) == 0) ?
                            1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[i]->exprBase.dataType);
    }

    gcmASSERT(componentCount[0] == componentCount[1] && componentCount[1] == componentCount[2]);

    tempConstants[0] = OperandConstants[2];
    tempConstants[1] = OperandConstants[1];

    status = sloCOMPILER_CreateDataType(
                                        Compiler,
                                        T_FLOAT,
                                        gcvNULL,
                                        &dataType);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    dataType->qualifiers.storage = slvSTORAGE_QUALIFIER_CONST;


    status = sloIR_CONSTANT_Construct(
                                    Compiler,
                                    OperandConstants[0]->exprBase.base.lineNo,
                                    OperandConstants[0]->exprBase.base.stringNo,
                                    dataType,
                                    &tempConstants[2]);

    status = _EvaluateDot(Compiler, 2, tempConstants, tempConstants[2]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    if (tempConstants[2]->values[0].floatValue < 0.0f)
    {
        for (i = 0; i < componentCount[0]; i++)
        {
            values[i].floatValue = OperandConstants[0]->values[i].floatValue;
        }
    }
    else
    {
        for (i = 0; i < componentCount[0]; i++)
        {
            values[i].floatValue = -OperandConstants[0]->values[i].floatValue;
        }
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount[0],
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = sloIR_CONSTANT_Destroy(Compiler, &tempConstants[2]->exprBase.base);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenFaceForwardCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS               status;
    slsIOPERAND             intermIOperand;
    slsROPERAND             intermROperand;
    slsLOPERAND             intermLOperand;
    slsSELECTION_CONTEXT    selectionContext;
    slsROPERAND             zeroROperand;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* dot t0, Nref (operand2), I (operand1) */
    slsIOPERAND_New(Compiler,
                    &intermIOperand,
                    gcSHADER_FLOAT_X1,
                    GetHigherPrecison(OperandsParameters[2].rOperands[0].u.reg.precision,
                                      OperandsParameters[1].rOperands[0].u.reg.precision));

    status = slGenGenericCode2(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                slvOPCODE_DOT,
                                &intermIOperand,
                                &OperandsParameters[2].rOperands[0],
                                &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* The selection begin*/
    status = slDefineSelectionBegin(
                                    Compiler,
                                    CodeGenerator,
                                    gcvTRUE,
                                    &selectionContext);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* The condition part: t0 < 0.0 */
    slsROPERAND_InitializeUsingIOperand(&intermROperand, &intermIOperand);
    if (gcIsDoubleDataType(OperandsParameters[0].rOperands[0].dataType))
    {
        slsROPERAND_InitializeFloatOrVecOrMatConstant(&zeroROperand,
                                                      gcSHADER_FLOAT64_X1,
                                                      gcSHADER_PRECISION_MEDIUM,
                                                      (gctFLOAT)0.0);
    }
    else
    {
        slsROPERAND_InitializeFloatOrVecOrMatConstant(&zeroROperand,
                                                      gcSHADER_FLOAT_X1,
                                                      gcSHADER_PRECISION_MEDIUM,
                                                      (gctFLOAT)0.0);
    }

    status = slGenSelectionCompareConditionCode(
                                                Compiler,
                                                CodeGenerator,
                                                &selectionContext,
                                                PolynaryExpr->exprBase.base.lineNo,
                                                PolynaryExpr->exprBase.base.stringNo,
                                                slvCONDITION_LESS_THAN,
                                                &intermROperand,
                                                &zeroROperand);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* The true part */
    status = slDefineSelectionTrueOperandBegin(
                                            Compiler,
                                            CodeGenerator,
                                            &selectionContext);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* mov result, N (operand0) */
    slsLOPERAND_InitializeUsingIOperand(&intermLOperand, IOperand);

    status = slGenAssignCode(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            &intermLOperand,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }


    status = slDefineSelectionTrueOperandEnd(
                                            Compiler,
                                            CodeGenerator,
                                            &selectionContext,
                                            gcvFALSE);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* The false part */
    status = slDefineSelectionFalseOperandBegin(
                                            Compiler,
                                            CodeGenerator,
                                            &selectionContext);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* sub result, 0.0, N (operand0) */
    status = slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_SUB,
                                    IOperand,
                                    &zeroROperand,
                                    &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = slDefineSelectionFalseOperandEnd(
                                            Compiler,
                                            CodeGenerator,
                                            &selectionContext);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* The selection end */
    status = slDefineSelectionEnd(
                                Compiler,
                                CodeGenerator,
                                &selectionContext);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateReflect(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount[2] = {0};
    slsDATA_TYPE *           dataType;
    sloIR_CONSTANT      tempConstant;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsEqual(
                                ResultConstant->exprBase.dataType,
                                OperandConstants[0]->exprBase.dataType));

    for (i = 0; i < OperandCount; i++)
    {
        gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[i]->exprBase.dataType));
        componentCount[i] = (slmDATA_TYPE_vectorSize_GET(OperandConstants[i]->exprBase.dataType) == 0) ?
                            1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[i]->exprBase.dataType);
    }

    gcmASSERT(componentCount[0] == componentCount[1]);

    status = sloCOMPILER_CreateDataType(
                                        Compiler,
                                        T_FLOAT,
                                        gcvNULL,
                                        &dataType);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    dataType->qualifiers.storage = slvSTORAGE_QUALIFIER_CONST;

    status = sloIR_CONSTANT_Construct(
                                    Compiler,
                                    OperandConstants[0]->exprBase.base.lineNo,
                                    OperandConstants[0]->exprBase.base.stringNo,
                                    dataType,
                                    &tempConstant);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = _EvaluateDot(Compiler, 2, OperandConstants, tempConstant);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    for (i = 0; i < componentCount[0]; i++)
    {
        values[i].floatValue = OperandConstants[0]->values[i].floatValue -
            2.0f * tempConstant->values[0].floatValue * OperandConstants[1]->values[i].floatValue;
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount[0],
                                    values);

    status = sloIR_CONSTANT_Destroy(Compiler, &tempConstant->exprBase.base);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}


gceSTATUS
_EvaluateRefract(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount[3];
    sluCONSTANT_VALUE   values[4];
    slsDATA_TYPE *           dataType;
    sloIR_CONSTANT tempConstant;
    gctFLOAT k;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandConstants);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandConstants[0]->exprBase.dataType->elementType == slvTYPE_FLOAT
           || OperandConstants[0]->exprBase.dataType->elementType == slvTYPE_DOUBLE);

    for (i = 0; i < OperandCount; i++)
    {
        gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[i]->exprBase.dataType));
        componentCount[i] = (slmDATA_TYPE_vectorSize_GET(OperandConstants[i]->exprBase.dataType) == 0) ?
                            1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[i]->exprBase.dataType);
    }

    gcmASSERT(componentCount[0] == componentCount[1] && componentCount[2] == 1);

    status = sloCOMPILER_CreateDataType(
                                        Compiler,
                                        T_FLOAT,
                                        gcvNULL,
                                        &dataType);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    dataType->qualifiers.storage = slvSTORAGE_QUALIFIER_CONST;


    status = sloIR_CONSTANT_Construct(
                                    Compiler,
                                    OperandConstants[0]->exprBase.base.lineNo,
                                    OperandConstants[0]->exprBase.base.stringNo,
                                    dataType,
                                    &tempConstant);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    status = _EvaluateDot(Compiler, 2, OperandConstants, tempConstant);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    k = 1.0f - OperandConstants[2]->values[0].floatValue * OperandConstants[2]->values[0].floatValue *
        (1.0f - tempConstant->values[0].floatValue * tempConstant->values[0].floatValue);

    if (k < 0.0f)
    {
        for (i = 0; i < componentCount[0]; i++)
        {
            values[i].floatValue = 0.0f;
        }
    }
    else
    {
        for (i = 0; i < componentCount[0]; i++)
        {
            values[i].floatValue = OperandConstants[2]->values[0].floatValue * OperandConstants[0]->values[i].floatValue -
                (OperandConstants[2]->values[0].floatValue * tempConstant->values[0].floatValue +
                gcoMATH_SquareRoot(k)) * OperandConstants[1]->values[i].floatValue;
        }
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount[0],
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenRefractCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS               status;
    slsIOPERAND *           intermIOperands = gcvNULL;
    slsROPERAND *           intermROperands = gcvNULL;
    slsLOPERAND             intermLOperand;
    slsSELECTION_CONTEXT    selectionContext;
    slsROPERAND             oneROperand, zeroROperand;
    gcSHADER_PRECISION      *opPrecision = gcvNULL;
    gcSHADER_TYPE           dataType = gcSHADER_FLOAT_X1;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    gcmONERROR(gcoOS_Allocate(gcvNULL, 11 * sizeof(slsIOPERAND), (gctPOINTER*)&intermIOperands));
    gcmONERROR(gcoOS_Allocate(gcvNULL, 11 * sizeof(slsROPERAND), (gctPOINTER*)&intermROperands));
    gcmONERROR(gcoOS_Allocate(gcvNULL, 11 * sizeof(gcSHADER_PRECISION), (gctPOINTER*)&opPrecision));

    if (gcIsDoubleDataType(IOperand->dataType))
    {
        dataType = gcSHADER_FLOAT64_X1;
    }

    slsROPERAND_InitializeFloatOrVecOrMatConstant(&oneROperand,
                                                  dataType,
                                                  gcSHADER_PRECISION_MEDIUM,
                                                  (gctFLOAT)1.0);

    slsROPERAND_InitializeFloatOrVecOrMatConstant(&zeroROperand,
                                                  dataType,
                                                  gcSHADER_PRECISION_MEDIUM,
                                                  (gctFLOAT)0.0);

    /* dot t0, N (operand1), I (operand0) */
    /* precision is coming from the operands' highest precision */
    *opPrecision = GetHigherPrecison(OperandsParameters[1].rOperands[0].u.reg.precision,
                                     OperandsParameters[0].rOperands[0].u.reg.precision);
    slsIOPERAND_New(Compiler,
                    intermIOperands,
                    dataType,
                    *opPrecision);

    gcmONERROR(slGenGenericCode2(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                slvOPCODE_DOT,
                                intermIOperands,
                                &OperandsParameters[1].rOperands[0],
                                &OperandsParameters[0].rOperands[0]));

    /* mul t1, t0, t0 */
    *(opPrecision + 1) = *opPrecision;
    slsIOPERAND_New(Compiler,
                    intermIOperands + 1,
                    dataType,
                    *(opPrecision + 1));
    slsROPERAND_InitializeUsingIOperand(intermROperands, intermIOperands);

    gcmONERROR(slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    intermIOperands + 1,
                                    intermROperands,
                                    intermROperands));

    /* sub t2, 1.0, t1 */
    *(opPrecision + 2) = *(opPrecision + 1);
    slsIOPERAND_New(Compiler,
                    intermIOperands + 2,
                    dataType,
                    *(opPrecision + 2));
    slsROPERAND_InitializeUsingIOperand(intermROperands + 1, intermIOperands + 1);

    gcmONERROR(slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_SUB,
                                    intermIOperands + 2,
                                    &oneROperand,
                                    intermROperands + 1));

    /* mul t3, eta (operand2), eta (operand2) */
    *(opPrecision + 3) = OperandsParameters[2].rOperands[0].u.reg.precision;
    slsIOPERAND_New(Compiler,
                    intermIOperands + 3,
                    dataType,
                    *(opPrecision + 3));

    gcmONERROR(slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    intermIOperands + 3,
                                    &OperandsParameters[2].rOperands[0],
                                    &OperandsParameters[2].rOperands[0]));

    /* mul t4, t3, t2 */
    *(opPrecision + 4) = GetHigherPrecison(*(opPrecision + 3), *(opPrecision + 2));
    slsIOPERAND_New(Compiler,
                    intermIOperands + 4,
                    dataType,
                    *(opPrecision + 4));
    slsROPERAND_InitializeUsingIOperand(intermROperands + 3, intermIOperands + 3);
    slsROPERAND_InitializeUsingIOperand(intermROperands + 2, intermIOperands + 2);

    gcmONERROR(slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    intermIOperands + 4,
                                    intermROperands + 3,
                                    intermROperands + 2));

    /* sub t5, 1.0, t4 */
    *(opPrecision + 5) = *(opPrecision + 4);
    slsIOPERAND_New(Compiler,
                    intermIOperands + 5,
                    dataType,
                    *(opPrecision + 5));
    slsROPERAND_InitializeUsingIOperand(intermROperands + 4, intermIOperands + 4);

    gcmONERROR(slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_SUB,
                                    intermIOperands + 5,
                                    &oneROperand,
                                    intermROperands + 4));

    /* The selection begin*/
    gcmONERROR(slDefineSelectionBegin(
                                    Compiler,
                                    CodeGenerator,
                                    gcvTRUE,
                                    &selectionContext));

    /* The condition part: t5 < 0.0 */
    slsROPERAND_InitializeUsingIOperand(intermROperands + 5, intermIOperands + 5);

    gcmONERROR(slGenSelectionCompareConditionCode(
                                                Compiler,
                                                CodeGenerator,
                                                &selectionContext,
                                                PolynaryExpr->exprBase.base.lineNo,
                                                PolynaryExpr->exprBase.base.stringNo,
                                                slvCONDITION_LESS_THAN,
                                                intermROperands + 5,
                                                &zeroROperand));

    /* The true part */
    gcmONERROR(slDefineSelectionTrueOperandBegin(
                                            Compiler,
                                            CodeGenerator,
                                            &selectionContext));

    /* mov result, 0.0 */
    slsLOPERAND_InitializeUsingIOperand(&intermLOperand, IOperand);
    slsROPERAND_InitializeFloatOrVecOrMatConstant(&zeroROperand,
                                                  IOperand->dataType,
                                                  gcSHADER_PRECISION_MEDIUM,
                                                  (gctFLOAT)0.0);

    gcmONERROR(slGenAssignCode(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            &intermLOperand,
                            &zeroROperand));

    gcmONERROR(slDefineSelectionTrueOperandEnd(
                                            Compiler,
                                            CodeGenerator,
                                            &selectionContext,
                                            gcvFALSE));

    /* The false part */
    gcmONERROR(slDefineSelectionFalseOperandBegin(
                                            Compiler,
                                            CodeGenerator,
                                            &selectionContext));

    /* mul t6, eta (operand2), I (operand0) */
    *(opPrecision + 6) = GetHigherPrecison(OperandsParameters[2].rOperands[0].u.reg.precision,
                                           OperandsParameters[0].rOperands[0].u.reg.precision);
    slsIOPERAND_New(Compiler,
                    intermIOperands + 6,
                    OperandsParameters[0].dataTypes[0],
                    *(opPrecision + 6));

    gcmONERROR(slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    intermIOperands + 6,
                                    &OperandsParameters[2].rOperands[0],
                                    &OperandsParameters[0].rOperands[0]));

    /* mul t7, eta (operand2), t0 */
    *(opPrecision + 7) = GetHigherPrecison(OperandsParameters[2].rOperands[0].u.reg.precision,
                                           OperandsParameters[0].rOperands[0].u.reg.precision);
    slsIOPERAND_New(Compiler,
                    intermIOperands + 7,
                    dataType,
                    *(opPrecision + 7));

    gcmONERROR(slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    intermIOperands + 7,
                                    &OperandsParameters[2].rOperands[0],
                                    intermROperands));

    /* sqrt t8, t5 */
    *(opPrecision + 8) = *(opPrecision + 5);
    slsIOPERAND_New(Compiler,
                    intermIOperands + 8,
                    dataType,
                    *(opPrecision + 8));

    gcmONERROR(slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_SQRT,
                            intermIOperands + 8,
                            intermROperands + 5));

    /* add t9, t7, t8 */
    *(opPrecision + 9) = GetHigherPrecison(*(opPrecision + 7), *(opPrecision + 8));
    slsIOPERAND_New(Compiler,
                    intermIOperands + 9,
                    dataType,
                    *(opPrecision + 9));
    slsROPERAND_InitializeUsingIOperand(intermROperands + 7, intermIOperands + 7);
    slsROPERAND_InitializeUsingIOperand(intermROperands + 8, intermIOperands + 8);

    gcmONERROR(slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_ADD,
                                    intermIOperands + 9,
                                    intermROperands + 7,
                                    intermROperands + 8));

    /* mul t10, t9, N (operand1) */
    *(opPrecision + 10) = GetHigherPrecison(*(opPrecision + 9),
                                            OperandsParameters[1].rOperands[0].u.reg.precision);
    slsIOPERAND_New(Compiler,
                    intermIOperands + 10,
                    OperandsParameters[1].dataTypes[0],
                    *(opPrecision + 10));
    slsROPERAND_InitializeUsingIOperand(intermROperands + 9, intermIOperands + 9);

    gcmONERROR(slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_MUL,
                                    intermIOperands + 10,
                                    intermROperands + 9,
                                    &OperandsParameters[1].rOperands[0]));

    /* sub result, t6, t10 */
    slsROPERAND_InitializeUsingIOperand(intermROperands + 6, intermIOperands + 6);
    slsROPERAND_InitializeUsingIOperand(intermROperands + 10, intermIOperands + 10);

    gcmONERROR(slGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    slvOPCODE_SUB,
                                    IOperand,
                                    intermROperands + 6,
                                    intermROperands + 10));

    gcmONERROR(slDefineSelectionFalseOperandEnd(
                                            Compiler,
                                            CodeGenerator,
                                            &selectionContext));

    /* The selection end */
    gcmONERROR(slDefineSelectionEnd(
                                Compiler,
                                CodeGenerator,
                                &selectionContext));

    if (intermIOperands != gcvNULL)
    {
        gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, intermIOperands));
    }
    if (intermROperands != gcvNULL)
    {
        gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, intermROperands));
    }
    if (opPrecision != gcvNULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, opPrecision));
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    if (intermIOperands != gcvNULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, intermIOperands));
    }
    if (intermROperands != gcvNULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, intermROperands));
    }
    if (opPrecision != gcvNULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, opPrecision));
    }
    gcmFOOTER();
    return status;
}

gceSTATUS
_EvaluateMatrixCompMult(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, j, componentColCount[2] = {0}, componentRowCount[2] = {0};
    sluCONSTANT_VALUE   values[16];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandConstants);
    gcmASSERT(OperandCount == 2);

    for (i = 0; i < OperandCount; i++)
    {
        gcmASSERT(slsDATA_TYPE_IsMat(OperandConstants[i]->exprBase.dataType));
        componentColCount[i] = slmDATA_TYPE_matrixColumnCount_GET(OperandConstants[i]->exprBase.dataType);
        componentRowCount[i] = slmDATA_TYPE_matrixRowCount_GET(OperandConstants[i]->exprBase.dataType);
    }

    gcmASSERT(componentColCount[0] == componentColCount[1]);
    gcmASSERT(componentRowCount[0] == componentRowCount[1]);

    for (i = 0; i < componentColCount[0]; i++)
    {
        for (j = 0; j < componentRowCount[0]; j++)
        {
            values[componentRowCount[0] * i + j].floatValue = OperandConstants[0]->values[componentRowCount[0] * i + j].floatValue *
                OperandConstants[1]->values[componentRowCount[0] * i + j].floatValue;
        }
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentColCount[0] * componentRowCount[0],
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenMatrixCompMultCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS       status;
    gctUINT         i;
    slsIOPERAND     columnIOperand;
    slsROPERAND     columnROperand0, columnROperand1;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    for (i = 0; i < gcGetMatrixDataTypeColumnCount(IOperand->dataType); i++)
    {
        slsIOPERAND_InitializeAsMatrixColumn(
                                            &columnIOperand,
                                            IOperand,
                                            i);

        slsROPERAND_InitializeAsMatrixColumn(
                                            &columnROperand0,
                                            &OperandsParameters[0].rOperands[0],
                                            i);

        slsROPERAND_InitializeAsMatrixColumn(
                                            &columnROperand1,
                                            &OperandsParameters[1].rOperands[0],
                                            i);

        status = slGenArithmeticExprCode(
                                        Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        slvOPCODE_MUL,
                                        &columnIOperand,
                                        &columnROperand0,
                                        &columnROperand1);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateOuterProduct(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS status;
    gctUINT  i, j, componentCount;
    gctUINT8 rowCount, columnCount;
    sluCONSTANT_VALUE *values;
    sluCONSTANT_VALUE *leftValues;
    sluCONSTANT_VALUE *rightValues;
    sluCONSTANT_VALUE *res;
    gctPOINTER pointer;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandConstants);

    rowCount = slmDATA_TYPE_matrixRowCount_GET(ResultConstant->exprBase.dataType);
    columnCount = slmDATA_TYPE_matrixColumnCount_GET(ResultConstant->exprBase.dataType);
    gcmASSERT(rowCount == slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType));
    gcmASSERT(columnCount == slmDATA_TYPE_vectorSize_GET(OperandConstants[1]->exprBase.dataType));
    componentCount = rowCount * columnCount;

    status = sloCOMPILER_Allocate(Compiler,
                                  (gctSIZE_T)sizeof(sluCONSTANT_VALUE) * componentCount,
                                  &pointer);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    values = pointer;
    res = values;

    rightValues = OperandConstants[1]->values;

    for(i = 0; i < columnCount; i++)
    {
       leftValues = OperandConstants[0]->values;

       for(j = 0; j < rowCount; j++)
       {
          res->floatValue = rightValues->floatValue * leftValues->floatValue;
          leftValues++;
          res++;
       }

       rightValues++;
    }

    status = sloIR_CONSTANT_SetValues(Compiler,
                                      ResultConstant,
                                      componentCount,
                                      values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenOuterProductCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    gctUINT8     i, j, rowCount, columnCount;
    slsIOPERAND  columnIOperand[1], intermIOperand[1];
    slsROPERAND  rOperand1[1], rOperand2[1], intermROperand[1];
    slsLOPERAND  lOperand[1], intermLOperand[1];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    rowCount = slmDATA_TYPE_matrixRowCount_GET(PolynaryExpr->funcName->dataType);
    columnCount = slmDATA_TYPE_matrixColumnCount_GET(PolynaryExpr->funcName->dataType);
    gcmASSERT(rowCount == gcGetVectorDataTypeComponentCount(OperandsParameters[0].rOperands[0].dataType));
    gcmASSERT(columnCount == gcGetVectorDataTypeComponentCount(OperandsParameters[1].rOperands[0].dataType));

    if (gcIsDoubleDataType(OperandsParameters[0].rOperands[0].dataType))
    {
        slsIOPERAND_New(Compiler, intermIOperand, gcSHADER_FLOAT64_X1,
            GetHigherPrecison(OperandsParameters[1].rOperands[0].u.reg.precision,
                              OperandsParameters[0].rOperands[0].u.reg.precision));
    }
    else
    {
        slsIOPERAND_New(Compiler, intermIOperand, gcSHADER_FLOAT_X1,
            GetHigherPrecison(OperandsParameters[1].rOperands[0].u.reg.precision,
                              OperandsParameters[0].rOperands[0].u.reg.precision));
    }

    for (i = 0; i < columnCount; i++)
    {
        slsIOPERAND_InitializeAsMatrixColumn(columnIOperand,
                                             IOperand,
                                             i);

        slsROPERAND_InitializeAsVectorComponent(rOperand1, &OperandsParameters[1].rOperands[0], i);

        for (j = 0; j < rowCount; j++)
        {
            slsLOPERAND_InitializeUsingIOperand(lOperand, columnIOperand);
            slGetVectorLOperandSlice(lOperand,
                                     j,
                                     1,
                                     intermLOperand);

            slsROPERAND_InitializeAsVectorComponent(rOperand2, &OperandsParameters[0].rOperands[0], j);

            status = slGenArithmeticExprCode(Compiler,
                                             PolynaryExpr->exprBase.base.lineNo,
                                             PolynaryExpr->exprBase.base.stringNo,
                                             slvOPCODE_MUL,
                                             intermIOperand,
                                             rOperand1,
                                             rOperand2);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

            slsROPERAND_InitializeUsingIOperand(intermROperand, intermIOperand);

            status = slGenAssignCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     intermLOperand,
                                     intermROperand);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateTranspose(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS         status;
    gctUINT           i, componentCount;
    sluCONSTANT_VALUE *values;
    sluCONSTANT_VALUE *fromValues;
    gctPOINTER pointer;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    componentCount = slsDATA_TYPE_GetSize(OperandConstants[0]->exprBase.dataType);
    gcmASSERT(OperandConstants[0]->valueCount == 1 || OperandConstants[0]->valueCount == componentCount);

    gcmASSERT(slmDATA_TYPE_matrixRowCount_GET(ResultConstant->exprBase.dataType) ==
              slmDATA_TYPE_matrixColumnCount_GET(OperandConstants[0]->exprBase.dataType) &&
              slmDATA_TYPE_matrixColumnCount_GET(ResultConstant->exprBase.dataType) ==
              slmDATA_TYPE_matrixRowCount_GET(OperandConstants[0]->exprBase.dataType));

    status = sloCOMPILER_Allocate(Compiler,
                                  (gctSIZE_T)sizeof(sluCONSTANT_VALUE) * componentCount,
                                  &pointer);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    values = pointer;
    fromValues = OperandConstants[0]->values;

    if(OperandConstants[0]->valueCount == 1)
    {
       for(i = 0; i < componentCount; i++)
       {
          values[i].floatValue = fromValues[0].floatValue;
       }
    }
    else
    {
       gctUINT columnCount, rowCount, j;

       columnCount = slmDATA_TYPE_matrixColumnCount_GET(OperandConstants[0]->exprBase.dataType);
       rowCount = slmDATA_TYPE_matrixRowCount_GET(OperandConstants[0]->exprBase.dataType);

       for (i = 0; i < columnCount; i++)
       {
           for(j = 0; j < rowCount; j++)
           {
               values[j * columnCount + i].floatValue = fromValues[i * rowCount + j].floatValue;
           }
       }
    }

    status = sloIR_CONSTANT_SetValues(Compiler,
                                      ResultConstant,
                                      componentCount,
                                      values);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_ComputeTranspose(
    IN sloCOMPILER Compiler,
    IN gctINT LineNo,
    IN gctINT StringNo,
    IN slsROPERAND *Matrix,
    OUT slsIOPERAND *TransposedMatrix
    )
{
    gceSTATUS    status;
    gctUINT      i, j;
    slsIOPERAND  columnIOperand[1];
    slsROPERAND  columnROperand[1];
    slsLOPERAND  columnLOperand[1];
    slsROPERAND  rOperand[1];
    slsLOPERAND  lOperand[1];
    gctUINT columnCount, rowCount;

    gcmHEADER();
    columnCount = gcGetMatrixDataTypeColumnCount(TransposedMatrix->dataType);
    rowCount = gcGetMatrixDataTypeRowCount(TransposedMatrix->dataType);

    for (i = 0; i < columnCount; i++)
    {
        slsIOPERAND_InitializeAsMatrixColumn(columnIOperand,
                                             TransposedMatrix,
                                             i);
        slsLOPERAND_InitializeUsingIOperand(columnLOperand, columnIOperand);

        for (j = 0; j < rowCount; j++)
        {
            slsLOPERAND_InitializeAsVectorComponent(lOperand, columnLOperand, j);

            slsROPERAND_InitializeAsMatrixColumn(columnROperand,
                                                 Matrix,
                                                 j);

            slsROPERAND_InitializeAsVectorComponent(rOperand, columnROperand, i);

            status = slGenAssignCode(Compiler,
                                     LineNo,
                                     StringNo,
                                     lOperand,
                                     rOperand);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenTransposeCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS    status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = _ComputeTranspose(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               OperandsParameters[0].rOperands,
                               IOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}


/* Compute determinant of a 2x2 matrix */
#define _slmCompute2x2Determinant(Compiler, LineNo, StringNo, Matrix, Row1, Row2, Column1, Column2, Negate, Det, Status) do { \
       slsROPERAND columnVec1[1]; \
       slsROPERAND columnVec2[1]; \
       slsIOPERAND iOperand[1]; \
       slsROPERAND rOperand[1]; \
       slsROPERAND rOperand1[1]; \
       slsROPERAND rOperand2[1]; \
       gcSHADER_TYPE componentSelectionDataType; \
       slsROPERAND_InitializeAsMatrixColumn(columnVec1, \
                                            (Matrix),   \
                                            (Column1)); \
       componentSelectionDataType = gcGetVectorComponentSelectionDataType(columnVec1->dataType, \
                                                                          2); \
       columnVec1->dataType = componentSelectionDataType; \
       columnVec1->u.reg.componentSelection.components = 2; \
       columnVec1->u.reg.componentSelection.x = (Row1); \
       columnVec1->u.reg.componentSelection.y = (Row2); \
       slsROPERAND_InitializeAsMatrixColumn(columnVec2, \
                                            (Matrix),   \
                                            (Column2)); \
       columnVec2->dataType = componentSelectionDataType; \
       columnVec2->u.reg.componentSelection.components = 2; \
       columnVec2->u.reg.componentSelection.x = (Row2); \
       columnVec2->u.reg.componentSelection.y = (Row1); \
       slsIOPERAND_New((Compiler), iOperand, gcSHADER_FLOAT_X2, columnVec1->u.reg.precision); \
       (Status) = slGenArithmeticExprCode((Compiler), \
                                           (LineNo), \
                                           (StringNo), \
                                           slvOPCODE_MUL, \
                                           iOperand, \
                                           columnVec1, \
                                           columnVec2); \
       if (gcmIS_ERROR(Status)) break; \
       slsROPERAND_InitializeUsingIOperand(rOperand, iOperand); \
       slsROPERAND_InitializeAsVectorComponent(rOperand1, rOperand, 0); \
       slsROPERAND_InitializeAsVectorComponent(rOperand2, rOperand, 1); \
       if(Negate < 0) { \
           (Status) = slGenArithmeticExprCode((Compiler), \
                                              (LineNo), \
                                              (StringNo), \
                                              slvOPCODE_SUB, \
                                              (Det), \
                                              rOperand2, \
                                              rOperand1); \
           if (gcmIS_ERROR(Status)) break; \
       } \
       else { \
           (Status) = slGenArithmeticExprCode((Compiler), \
                                              (LineNo), \
                                              (StringNo), \
                                              slvOPCODE_SUB, \
                                              (Det), \
                                              rOperand1, \
                                              rOperand2); \
           if (gcmIS_ERROR(Status)) break; \
       } \
   } while (gcvFALSE)

static gceSTATUS
_EvalConstDet(
    IN sloIR_CONSTANT ConstMatrix,
    IN gctUINT8 ActiveSize,
    IN gctUINT8 *ActiveRows,
    IN gctUINT8 *ActiveColumns,
    IN gctINT Negate,
    OUT float *Det
    )
{
    sluCONSTANT_VALUE *values;
    gctUINT8 rowCount;
    gcmHEADER();

    rowCount = (gctUINT8)slmDATA_TYPE_matrixRowCount_GET(ConstMatrix->exprBase.dataType);
    values = ConstMatrix->values;
    if(ActiveSize == 2)
    {
        *Det = Negate * (values[ActiveColumns[0] * rowCount + ActiveRows[0]].floatValue *
                         values[ActiveColumns[1] * rowCount + ActiveRows[1]].floatValue -
                         values[ActiveColumns[1] * rowCount + ActiveRows[0]].floatValue *
                         values[ActiveColumns[0] * rowCount + ActiveRows[1]].floatValue);
    }
    else
    {
        gceSTATUS status;
        gctUINT8 activeRows[sldMAX_MATRIX_SIZE];
        gctUINT8 activeColumns[sldMAX_MATRIX_SIZE];
        gctUINT8 i, j, k;
        gctUINT8 activeSize = ActiveSize - 1;
        gctINT negate = Negate;
        float minor;
        float det;

        for(i = 0; i < activeSize; i++)
        {
            activeColumns[i] = ActiveColumns[i+1];
        }

        det = 0;
        for(i = 0; i < ActiveSize; i++)
        {
            k = 0;
            for(j = 0; j < ActiveSize; j++)
            {
                if(j == i) continue;
                activeRows[k] =  ActiveRows[j];
                k++;
            }
            gcmASSERT(k == activeSize);

            status = _EvalConstDet(ConstMatrix,
                                   activeSize,
                                   activeRows,
                                   activeColumns,
                                   negate,
                                   &minor);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

            det += values[ActiveColumns[0] * rowCount + ActiveRows[i]].floatValue * minor;
            negate *= -1;
        }
        *Det = det;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_ComputeDeterminant(
    IN sloCOMPILER Compiler,
    IN gctINT LineNo,
    IN gctINT StringNo,
    IN slsROPERAND *Matrix,
    IN gctUINT8 ActiveSize,
    IN gctUINT8 *ActiveRows,
    IN gctUINT8 *ActiveColumns,
    IN gctINT Negate,
    OUT slsIOPERAND *Det
    )
{
    gceSTATUS status;
    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    if(ActiveSize == 2)
    {
        _slmCompute2x2Determinant(Compiler,
                                  LineNo,
                                  StringNo,
                                  Matrix,
                                  ActiveRows[0],
                                  ActiveRows[1],
                                  ActiveColumns[0],
                                  ActiveColumns[1],
                                  Negate,
                                  Det,
                                  status);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else
    {
        gctUINT8 activeRows[sldMAX_MATRIX_SIZE];
        gctUINT8 activeColumns[sldMAX_MATRIX_SIZE];
        gctUINT8 i, j, k;
        gctUINT8 activeSize = ActiveSize - 1;
        gctINT negate = Negate;
        slsIOPERAND minor[1];
        slsIOPERAND minorColumnIOperand[1];
        slsROPERAND minorColumnROperand[1];
        slsLOPERAND minorColumnLOperand[1];
        slsROPERAND columnROperand[1];
        slsROPERAND rOperand[1];
        slsLOPERAND lOperand[1];
        gcSHADER_TYPE columnType;
        gcSHADER_TYPE componentSelectionDataType;

        for(i = 0; i < activeSize; i++)
        {
            activeColumns[i] = ActiveColumns[i+1];
        }

        columnType = gcGetMatrixColumnDataType(Matrix->dataType);

        if (gcIsDoubleDataType(columnType))
        {
            slsIOPERAND_New(Compiler, minor, gcSHADER_FLOAT64_X1, Det->precision);
        }
        else
        {
            slsIOPERAND_New(Compiler, minor, gcSHADER_FLOAT_X1, Det->precision);
        }
        slsROPERAND_InitializeUsingIOperand(rOperand, minor);
        slsIOPERAND_New(Compiler, minorColumnIOperand, columnType, Det->precision);
        slsLOPERAND_InitializeUsingIOperand(minorColumnLOperand, minorColumnIOperand);
        slsROPERAND_InitializeUsingIOperand(minorColumnROperand, minorColumnIOperand);

        for(i = 0; i < ActiveSize; i++)
        {
            k = 0;
            for(j = 0; j < ActiveSize; j++)
            {
                if(j == i) continue;
                activeRows[k] =  ActiveRows[j];
                k++;
            }
            gcmASSERT(k == activeSize);

            status = _ComputeDeterminant(Compiler,
                                         LineNo,
                                         StringNo,
                                         Matrix,
                                         activeSize,
                                         activeRows,
                                         activeColumns,
                                         negate,
                                         minor);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

            slsLOPERAND_InitializeAsVectorComponent(lOperand, minorColumnLOperand, ActiveRows[i]);
            status = slGenAssignCode(Compiler,
                                     LineNo,
                                     StringNo,
                                     lOperand,
                                     rOperand);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

            negate *= -1;
        }

        slsROPERAND_InitializeAsMatrixColumn(columnROperand,
                                             Matrix,
                                             ActiveColumns[0]);

        componentSelectionDataType = gcGetVectorComponentSelectionDataType(columnType,
                                                                           ActiveSize);
        columnROperand->dataType = componentSelectionDataType;
        minorColumnROperand->dataType = componentSelectionDataType;

        columnROperand->u.reg.componentSelection.components = ActiveSize;
        minorColumnROperand->u.reg.componentSelection.components = ActiveSize;

        for(i = 0; i < ActiveSize; i++)
        {
            switch(i) {
            case 0:
                 columnROperand->u.reg.componentSelection.x = ActiveRows[0];
                 minorColumnROperand->u.reg.componentSelection.x = ActiveRows[0];
                 break;

            case 1:
                 columnROperand->u.reg.componentSelection.y = ActiveRows[1];
                 minorColumnROperand->u.reg.componentSelection.y = ActiveRows[1];
                 break;

            case 2:
                 columnROperand->u.reg.componentSelection.z = ActiveRows[2];
                 minorColumnROperand->u.reg.componentSelection.z = ActiveRows[2];
                 break;

            case 3:
                 columnROperand->u.reg.componentSelection.w = ActiveRows[3];
                 minorColumnROperand->u.reg.componentSelection.w = ActiveRows[3];
                 break;

            default:
                gcmASSERT(0);
                status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                gcmFOOTER();
                return status;
            }
        }

        status = slGenGenericCode2(Compiler,
                                   LineNo,
                                   StringNo,
                                   slvOPCODE_DOT,
                                   Det,
                                   columnROperand,
                                   minorColumnROperand);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EvaluateAdjunct(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS  status              = gcvSTATUS_OK;
    sluCONSTANT_VALUE *savedValues = gcvNULL;
    gctUINT8 columnCount = 0, rowCount = 0;
    gctUINT  componentCount      = 0;
    gctUINT8   i, j;
    sluCONSTANT_VALUE *values      = gcvNULL;
    gctPOINTER pointer             = gcvNULL;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    columnCount = (gctUINT8)slmDATA_TYPE_matrixColumnCount_GET(OperandConstants[0]->exprBase.dataType);
    rowCount = (gctUINT8)slmDATA_TYPE_matrixRowCount_GET(OperandConstants[0]->exprBase.dataType);
    gcmASSERT(columnCount == rowCount);
    gcmASSERT(slmDATA_TYPE_matrixRowCount_GET(ResultConstant->exprBase.dataType) == rowCount &&
              slmDATA_TYPE_matrixColumnCount_GET(ResultConstant->exprBase.dataType) == columnCount);

    componentCount = slsDATA_TYPE_GetSize(OperandConstants[0]->exprBase.dataType);

    status = sloCOMPILER_Allocate(Compiler,
                                  (gctSIZE_T)sizeof(sluCONSTANT_VALUE) * componentCount,
                                  &pointer);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    values = pointer;

    /* Special handling for 2x2 matrix */
    if(columnCount == 2)
    {
       sluCONSTANT_VALUE *fromValues = gcvNULL;

       fromValues = OperandConstants[0]->values;
       /* swap the diagonal elements */
       values[0].floatValue = fromValues[3].floatValue;
       values[3].floatValue = fromValues[0].floatValue;
       /* negate the other entries */
       values[1].floatValue = -fromValues[1].floatValue;
       values[2].floatValue = -fromValues[2].floatValue;
       gcmONERROR(sloIR_CONSTANT_SetValues(Compiler,
                                           ResultConstant,
                                           componentCount,
                                           values));
       gcmFOOTER();
       return status;
    }
    else
    {
       gctINT columnNegate                        = 1;
       gctINT negate                              = 0;
       gctUINT8 activeRows[sldMAX_MATRIX_SIZE]    = {0};
       gctUINT8 activeColumns[sldMAX_MATRIX_SIZE] = {0};
       gctUINT8 activeSize, k;
       sluCONSTANT_VALUE *coFactor                = gcvNULL;

       coFactor = values;
       for (i = 0; i < columnCount; i++)
       {
            activeSize = 0;
            for(k = 0; k < columnCount; k++)
            {
               if(k == i) continue;
               activeColumns[activeSize] = k;
               activeSize++;
            }

            negate = columnNegate;
            /* Compute the co-factors */
            for (j = 0; j < rowCount; j++)
            {
                activeSize = 0;
                for(k = 0; k < rowCount; k++)
                {
                    if(k == j) continue;
                    activeRows[activeSize] = k;
                    activeSize++;
                }
                gcmONERROR(_EvalConstDet(OperandConstants[0],
                                         activeSize,
                                         activeRows,
                                         activeColumns,
                                         negate,
                                         &coFactor->floatValue));

                coFactor++;
                negate *= -1;
            }
            columnNegate *= -1;
        }

        savedValues = OperandConstants[0]->values;
        OperandConstants[0]->values = values;

        gcmONERROR(_EvaluateTranspose(Compiler,
                                      1,
                                      OperandConstants,
                                      ResultConstant));
    }

OnError:
    if(savedValues) {
        OperandConstants[0]->values = savedValues;
    }
    gcmVERIFY_OK(sloCOMPILER_Free(Compiler, values));

    gcmFOOTER();
    return status;
}

static gceSTATUS
_GenAdjunctCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    gctUINT8      i, j;
    slsIOPERAND  columnIOperand[1];
    slsROPERAND  columnROperand[1];
    slsLOPERAND  columnLOperand[1];
    slsROPERAND  rOperand[1];
    slsLOPERAND  lOperand[1];
    slsIOPERAND intermIOperand[1];
    gctUINT8 columnCount, rowCount;
    gcSHADER_TYPE dataType = gcSHADER_FLOAT_X1;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    columnCount = (gctUINT8)gcGetMatrixDataTypeColumnCount(IOperand->dataType);
    rowCount = (gctUINT8)gcGetMatrixDataTypeRowCount(IOperand->dataType);
    gcmASSERT(columnCount == rowCount);

    if (gcIsDoubleDataType(IOperand->dataType))
    {
        dataType = gcSHADER_FLOAT64_X1;
    }

    /* Special handling for 2x2 matrix */
    if(columnCount == 2)
    {
       slsIOPERAND  columnIOperand1[1];
       slsROPERAND  columnROperand1[1];
       slsLOPERAND  columnLOperand1[1];
       slsROPERAND  constantZero[1];

       slsROPERAND_InitializeFloatOrVecOrMatConstant(constantZero,
                                                     dataType,
                                                     gcSHADER_PRECISION_MEDIUM,
                                                     (gctFLOAT)0.0);

       /* swap the diagonal elements */
       slsIOPERAND_InitializeAsMatrixColumn(columnIOperand,
                                            IOperand,
                                            0);
       slsLOPERAND_InitializeUsingIOperand(columnLOperand, columnIOperand);
       slsLOPERAND_InitializeAsVectorComponent(lOperand, columnLOperand, 0);

       slsROPERAND_InitializeAsMatrixColumn(columnROperand1,
                                            &OperandsParameters[0].rOperands[0],
                                            1);
       slsROPERAND_InitializeAsVectorComponent(rOperand, columnROperand1, 1);
       status = slGenAssignCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                lOperand,
                                rOperand);
       if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

       slsIOPERAND_InitializeAsMatrixColumn(columnIOperand1,
                                            IOperand,
                                            1);
       slsLOPERAND_InitializeUsingIOperand(columnLOperand1, columnIOperand1);
       slsLOPERAND_InitializeAsVectorComponent(lOperand, columnLOperand1, 1);

       slsROPERAND_InitializeAsMatrixColumn(columnROperand,
                                            &OperandsParameters[0].rOperands[0],
                                            0);
       slsROPERAND_InitializeAsVectorComponent(rOperand, columnROperand, 0);
       status = slGenAssignCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                lOperand,
                                rOperand);
       if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

       /* negate the other entries */
       slsIOPERAND_New(Compiler, intermIOperand, dataType, rOperand->u.reg.precision);
       slsROPERAND_InitializeAsVectorComponent(rOperand, columnROperand, 1);
       status = slGenArithmeticExprCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        slvOPCODE_SUB,
                                        intermIOperand,
                                        constantZero,
                                        rOperand);
       if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

       slsROPERAND_InitializeUsingIOperand(rOperand, intermIOperand);
       slsLOPERAND_InitializeAsVectorComponent(lOperand, columnLOperand, 1);
       status = slGenAssignCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                lOperand,
                                rOperand);
       if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

       slsIOPERAND_New(Compiler, intermIOperand, dataType, rOperand->u.reg.precision);
       slsROPERAND_InitializeAsVectorComponent(rOperand, columnROperand1, 0);
       status = slGenArithmeticExprCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        slvOPCODE_SUB,
                                        intermIOperand,
                                        constantZero,
                                        rOperand);
       if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

       slsROPERAND_InitializeUsingIOperand(rOperand, intermIOperand);
       slsLOPERAND_InitializeAsVectorComponent(lOperand, columnLOperand1, 0);
       status = slGenAssignCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                lOperand,
                                rOperand);
       if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else
    {
       gctINT columnNegate = 1;
       gctINT negate;
       gctUINT8 activeRows[sldMAX_MATRIX_SIZE] = {0};
       gctUINT8 activeColumns[sldMAX_MATRIX_SIZE] = {0};
       gctUINT8 activeSize = 0, k = 0;
       slsIOPERAND coFactor[1];
       slsIOPERAND coFactorMatrix[1];
       gcSHADER_TYPE componentType;

       componentType = gcGetComponentDataType(OperandsParameters[0].rOperands[0].dataType);
       slsIOPERAND_New(Compiler, coFactorMatrix, IOperand->dataType, OperandsParameters[0].rOperands[0].u.reg.precision);
       slsIOPERAND_New(Compiler, coFactor, componentType, OperandsParameters[0].rOperands[0].u.reg.precision);

       for (i = 0; i < columnCount; i++)
       {
            slsIOPERAND_InitializeAsMatrixColumn(columnIOperand,
                                                 coFactorMatrix,
                                                 i);
            slsLOPERAND_InitializeUsingIOperand(columnLOperand, columnIOperand);

            activeSize = 0;
            for(k = 0; k < columnCount; k++)
            {
               if(k == i) continue;
               activeColumns[activeSize] = k;
               activeSize++;
            }

            negate = columnNegate;
            /* Compute the co-factors */
            for (j = 0; j < rowCount; j++)
            {
                activeSize = 0;
                for(k = 0; k < rowCount; k++)
                {
                    if(k == j) continue;
                    activeRows[activeSize] = k;
                    activeSize++;
                }
                status = _ComputeDeterminant(Compiler,
                                             PolynaryExpr->exprBase.base.lineNo,
                                             PolynaryExpr->exprBase.base.stringNo,
                                             OperandsParameters->rOperands,
                                             activeSize,
                                             activeRows,
                                             activeColumns,
                                             negate,
                                             coFactor);
                if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

                slsROPERAND_InitializeUsingIOperand(rOperand, coFactor);
                slsLOPERAND_InitializeAsVectorComponent(lOperand, columnLOperand, j);

                status = slGenAssignCode(Compiler,
                                         PolynaryExpr->exprBase.base.lineNo,
                                         PolynaryExpr->exprBase.base.stringNo,
                                         lOperand,
                                         rOperand);
                if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

                negate *= -1;
            }
            columnNegate *= -1;
        }

        /* This can be eliminated if embedding the transpose in cofactor computation */
        slsROPERAND_InitializeUsingIOperand(rOperand, coFactorMatrix);
        status = _ComputeTranspose(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   rOperand,
                                   IOperand);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateDeterminant(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS  status = gcvSTATUS_OK;
    gctUINT8 columnCount;
    sluCONSTANT_VALUE det;
    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    columnCount = (gctUINT8)slmDATA_TYPE_matrixColumnCount_GET(OperandConstants[0]->exprBase.dataType);
    gcmASSERT((gctUINT8)slmDATA_TYPE_matrixRowCount_GET(OperandConstants[0]->exprBase.dataType) == columnCount);

    /* Special handling for 2x2 matrix */
    if(columnCount == 2)
    {
       sluCONSTANT_VALUE *values;

       values = OperandConstants[0]->values;
       det.floatValue = values[0].floatValue * values[3].floatValue -
                        values[2].floatValue * values[1].floatValue;
    }
    else
    {
        gctUINT8 activeRows[sldMAX_MATRIX_SIZE];
        gctUINT8 activeColumns[sldMAX_MATRIX_SIZE];
        gctUINT8 i;

        for(i = 0; i < columnCount; i++)
        {
           activeRows[i] = activeColumns[i] = i;
        }
        status = _EvalConstDet(OperandConstants[0],
                               columnCount,
                               activeRows,
                               activeColumns,
                               1,
                               &det.floatValue);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    status = sloIR_CONSTANT_AddValues(Compiler,
                                      ResultConstant,
                                      1,
                                      &det);
    gcmFOOTER();
    return status;
}

gceSTATUS
_GenDeterminantCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT8 columnCount;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    columnCount = (gctUINT8)gcGetMatrixDataTypeColumnCount(OperandsParameters->rOperands[0].dataType);
    gcmASSERT((gctUINT8)gcGetMatrixDataTypeRowCount(OperandsParameters->rOperands[0].dataType) == columnCount);

    if(columnCount == 2)
    {
        /* 2x2 matrix */
        _slmCompute2x2Determinant(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  OperandsParameters->rOperands,
                                  0, 1, 0, 1, gcvFALSE, IOperand, status);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else
    {
        gctUINT8 activeRows[sldMAX_MATRIX_SIZE];
        gctUINT8 activeColumns[sldMAX_MATRIX_SIZE];
        gctUINT8 i;

        for(i = 0; i < columnCount; i++)
        {
           activeRows[i] = activeColumns[i] = i;
        }
        status = _ComputeDeterminant(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     OperandsParameters->rOperands,
                                     columnCount,
                                     activeRows,
                                     activeColumns,
                                     1,
                                     IOperand);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateInverse(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS  status = gcvSTATUS_OK;
    gctUINT8 columnCount, rowCount;
    gctUINT8   i, componentCount;
    sluCONSTANT_VALUE *values, *matrixValues;
    float det;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    columnCount = (gctUINT8)slmDATA_TYPE_matrixColumnCount_GET(OperandConstants[0]->exprBase.dataType);
    rowCount = (gctUINT8)slmDATA_TYPE_matrixRowCount_GET(OperandConstants[0]->exprBase.dataType);
    gcmASSERT(columnCount == rowCount);
    gcmASSERT(slmDATA_TYPE_matrixRowCount_GET(ResultConstant->exprBase.dataType) == rowCount &&
              slmDATA_TYPE_matrixColumnCount_GET(ResultConstant->exprBase.dataType) == columnCount);
    status = _EvaluateAdjunct(Compiler,
                              OperandCount,
                              OperandConstants,
                              ResultConstant);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    values = ResultConstant->values;
    matrixValues = OperandConstants[0]->values;
    det = 0;

    for(i = 0; i < columnCount; i++)
    {
       det += values[i * rowCount].floatValue * matrixValues[i].floatValue;
    }

    componentCount = columnCount * rowCount;
    det = 1/det;

    gcmASSERT(det != 0.0);
    if(det == 0.0)
    {
       gcmFOOTER_NO();
       return gcvSTATUS_INVALID_ARGUMENT;
    }
    for(i = 0; i < componentCount; i++)
    {
        values[i].floatValue *= det;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenInverseCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    slsIOPERAND  columnIOperand[1];
    slsROPERAND  columnROperand[1];
    slsROPERAND  minorColumn[1];
    slsROPERAND  minor[1];
    slsROPERAND  rOperand[1];
    slsIOPERAND  iOperand[1];
    slsROPERAND  determinant[1];
    slsROPERAND  determinantInverse[1];
    slsIOPERAND  adjunctMatrix[1];
    gcSHADER_TYPE componentType;
    gctUINT columnCount, i;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    slsIOPERAND_New(Compiler, adjunctMatrix, IOperand->dataType, IOperand->precision);
    /* compute the adjunct */
    status =  _GenAdjunctCode(Compiler,
                              CodeGenerator,
                              PolynaryExpr,
                              OperandCount,
                              OperandsParameters,
                              adjunctMatrix);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* compute determinant */
    slsROPERAND_InitializeAsMatrixColumn(columnROperand,
                                         OperandsParameters->rOperands,
                                         0);

    componentType = gcGetVectorComponentDataType(columnROperand->dataType);

    columnCount = gcGetMatrixDataTypeColumnCount(OperandsParameters->rOperands->dataType);
    for (i = 0; i < columnCount; i++)
    {
        slsIOPERAND_InitializeAsMatrixColumn(columnIOperand,
                                             adjunctMatrix,
                                             i);
        slsROPERAND_InitializeUsingIOperand(minorColumn, columnIOperand);
        slsROPERAND_InitializeAsVectorComponent(minor, minorColumn, 0);

        slsROPERAND_InitializeAsVectorComponent(rOperand, columnROperand, i);

        slsIOPERAND_New(Compiler, iOperand, componentType, GetHigherPrecison(rOperand->u.reg.precision, minor->u.reg.precision));
        status = slGenArithmeticExprCode(Compiler,
                                         PolynaryExpr->exprBase.base.lineNo,
                                         PolynaryExpr->exprBase.base.stringNo,
                                         slvOPCODE_MUL,
                                         iOperand,
                                         minor,
                                         rOperand);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        if(i != 0)
        {
            slsROPERAND_InitializeUsingIOperand(rOperand, iOperand);
            slsIOPERAND_New(Compiler, iOperand, componentType, GetHigherPrecison(rOperand->u.reg.precision, determinant->u.reg.precision));
            status = slGenArithmeticExprCode(Compiler,
                                             PolynaryExpr->exprBase.base.lineNo,
                                             PolynaryExpr->exprBase.base.stringNo,
                                             slvOPCODE_ADD,
                                             iOperand,
                                             determinant,
                                             rOperand);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
        }
        slsROPERAND_InitializeUsingIOperand(determinant, iOperand);
    }

    /* compute the inverse */
    slsIOPERAND_New(Compiler, iOperand, componentType, determinant->u.reg.precision);
    status = slGenGenericCode1(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               slvOPCODE_INVERSE,
                               iOperand,
                               determinant);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* multiply 1/determinant with adjunct matrix */
    slsROPERAND_InitializeUsingIOperand(determinantInverse, iOperand);
    slsROPERAND_InitializeUsingIOperand(rOperand, adjunctMatrix);
    status = slGenArithmeticExprCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     slvOPCODE_MUL,
                                     IOperand,
                                     determinantInverse,
                                     rOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateLessThan(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount[2] = {0};
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandConstants);
    gcmASSERT(OperandCount == 2);

    gcmASSERT(slsDATA_TYPE_IsBVec(ResultConstant->exprBase.dataType));

    for (i = 0; i < OperandCount; i++)
    {
        gcmASSERT(slsDATA_TYPE_IsIVec(OperandConstants[i]->exprBase.dataType) ||
            slsDATA_TYPE_IsVec(OperandConstants[i]->exprBase.dataType));
        componentCount[i] = (slmDATA_TYPE_vectorSize_GET(OperandConstants[i]->exprBase.dataType) == 0) ?
                            1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[i]->exprBase.dataType);
    }

    gcmASSERT(componentCount[0] == componentCount[1]);

    if (slsDATA_TYPE_IsIVec(OperandConstants[0]->exprBase.dataType))
    {
        for (i = 0; i < componentCount[0]; i++)
        {
            values[i].boolValue =
                OperandConstants[0]->values[i].intValue < OperandConstants[1]->values[i].intValue ? gcvTRUE : gcvFALSE;
        }
    }
    else if (slsDATA_TYPE_IsVec(OperandConstants[0]->exprBase.dataType))
    {
        for (i = 0; i < componentCount[0]; i++)
        {
            values[i].boolValue =
                OperandConstants[0]->values[i].floatValue < OperandConstants[1]->values[i].floatValue ? gcvTRUE : gcvFALSE;
        }
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount[0],
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenLessThanCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_LESS_THAN,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateLessThanEqual(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount[2] = {0};
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandConstants);
    gcmASSERT(OperandCount == 2);

    gcmASSERT(slsDATA_TYPE_IsBVec(ResultConstant->exprBase.dataType));

    for (i = 0; i < OperandCount; i++)
    {
        gcmASSERT(slsDATA_TYPE_IsIVec(OperandConstants[i]->exprBase.dataType) ||
            slsDATA_TYPE_IsVec(OperandConstants[i]->exprBase.dataType));
        componentCount[i] = (slmDATA_TYPE_vectorSize_GET(OperandConstants[i]->exprBase.dataType) == 0) ?
                            1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[i]->exprBase.dataType);
    }

    gcmASSERT(componentCount[0] == componentCount[1]);

    if (slsDATA_TYPE_IsIVec(OperandConstants[0]->exprBase.dataType))
    {
        for (i = 0; i < componentCount[0]; i++)
        {
            values[i].boolValue =
                OperandConstants[0]->values[i].intValue <= OperandConstants[1]->values[i].intValue ? gcvTRUE : gcvFALSE;
        }
    }
    else if (slsDATA_TYPE_IsVec(OperandConstants[0]->exprBase.dataType))
    {
        for (i = 0; i < componentCount[0]; i++)
        {
            values[i].boolValue =
                OperandConstants[0]->values[i].floatValue <= OperandConstants[1]->values[i].floatValue ? gcvTRUE : gcvFALSE;
        }
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount[0],
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenLessThanEqualCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_LESS_THAN_EQUAL,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateGreaterThan(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount[2] = {0};
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandConstants);
    gcmASSERT(OperandCount == 2);

    gcmASSERT(slsDATA_TYPE_IsBVec(ResultConstant->exprBase.dataType));

    for (i = 0; i < OperandCount; i++)
    {
        gcmASSERT(slsDATA_TYPE_IsIVec(OperandConstants[i]->exprBase.dataType) ||
            slsDATA_TYPE_IsVec(OperandConstants[i]->exprBase.dataType));
        componentCount[i] = (slmDATA_TYPE_vectorSize_GET(OperandConstants[i]->exprBase.dataType) == 0) ?
                            1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[i]->exprBase.dataType);
    }

    gcmASSERT(componentCount[0] == componentCount[1]);

    if (slsDATA_TYPE_IsIVec(OperandConstants[0]->exprBase.dataType))
    {
        for (i = 0; i < componentCount[0]; i++)
        {
            values[i].boolValue =
                OperandConstants[0]->values[i].intValue > OperandConstants[1]->values[i].intValue ? gcvTRUE : gcvFALSE;
        }
    }
    else if (slsDATA_TYPE_IsVec(OperandConstants[0]->exprBase.dataType))
    {
        for (i = 0; i < componentCount[0]; i++)
        {
            values[i].boolValue =
                OperandConstants[0]->values[i].floatValue > OperandConstants[1]->values[i].floatValue ? gcvTRUE : gcvFALSE;
        }
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount[0],
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenGreaterThanCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_GREATER_THAN,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateGreaterThanEqual(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount[2] = {0};
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandConstants);
    gcmASSERT(OperandCount == 2);

    gcmASSERT(slsDATA_TYPE_IsBVec(ResultConstant->exprBase.dataType));

    for (i = 0; i < OperandCount; i++)
    {
        gcmASSERT(slsDATA_TYPE_IsIVec(OperandConstants[i]->exprBase.dataType) ||
            slsDATA_TYPE_IsVec(OperandConstants[i]->exprBase.dataType));
        componentCount[i] = (slmDATA_TYPE_vectorSize_GET(OperandConstants[i]->exprBase.dataType) == 0) ?
                            1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[i]->exprBase.dataType);
    }

    gcmASSERT(componentCount[0] == componentCount[1]);

    if (slsDATA_TYPE_IsIVec(OperandConstants[0]->exprBase.dataType))
    {
        for (i = 0; i < componentCount[0]; i++)
        {
            values[i].boolValue =
                OperandConstants[0]->values[i].intValue >= OperandConstants[1]->values[i].intValue ? gcvTRUE : gcvFALSE;
        }
    }
    else if (slsDATA_TYPE_IsVec(OperandConstants[0]->exprBase.dataType))
    {
        for (i = 0; i < componentCount[0]; i++)
        {
            values[i].boolValue =
                OperandConstants[0]->values[i].floatValue >= OperandConstants[1]->values[i].floatValue ? gcvTRUE : gcvFALSE;
        }
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount[0],
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenGreaterThanEqualCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_GREATER_THAN_EQUAL,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateEqual(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount[2] = {0};
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandConstants);
    gcmASSERT(OperandCount == 2);

    gcmASSERT(slsDATA_TYPE_IsBVec(ResultConstant->exprBase.dataType));

    for (i = 0; i < OperandCount; i++)
    {
        gcmASSERT(slsDATA_TYPE_IsIVec(OperandConstants[i]->exprBase.dataType) ||
            slsDATA_TYPE_IsVec(OperandConstants[i]->exprBase.dataType) ||
            slsDATA_TYPE_IsBVec(OperandConstants[i]->exprBase.dataType));
        componentCount[i] = (slmDATA_TYPE_vectorSize_GET(OperandConstants[i]->exprBase.dataType) == 0) ?
                            1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[i]->exprBase.dataType);
    }

    gcmASSERT(componentCount[0] == componentCount[1]);

    if (slsDATA_TYPE_IsIVec(OperandConstants[0]->exprBase.dataType))
    {
        for (i = 0; i < componentCount[0]; i++)
        {
            values[i].boolValue =
                OperandConstants[0]->values[i].intValue == OperandConstants[1]->values[i].intValue ? gcvTRUE : gcvFALSE;
        }
    }
    else if (slsDATA_TYPE_IsVec(OperandConstants[0]->exprBase.dataType))
    {
        for (i = 0; i < componentCount[0]; i++)
        {
            values[i].boolValue =
                OperandConstants[0]->values[i].floatValue == OperandConstants[1]->values[i].floatValue ? gcvTRUE : gcvFALSE;
        }
    }
    else if (slsDATA_TYPE_IsBVec(OperandConstants[0]->exprBase.dataType))
    {
        for (i = 0; i < componentCount[0]; i++)
        {
            values[i].boolValue =
                OperandConstants[0]->values[i].boolValue == OperandConstants[1]->values[i].boolValue ? gcvTRUE : gcvFALSE;
        }
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount[0],
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenEqualCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_EQUAL,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateNotEqual(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gctUINT             i, componentCount;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandConstants);
    gcmASSERT(OperandCount == 2);

    gcmASSERT(slsDATA_TYPE_IsBVec(ResultConstant->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    status = _EvaluateEqual(Compiler, OperandCount, OperandConstants, ResultConstant);

    for (i = 0; i < componentCount; i++)
    {
        ResultConstant->values[i].boolValue = !ResultConstant->values[i].boolValue;
    }

    gcmFOOTER_NO();
    return status;
}

gceSTATUS
_GenNotEqualCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_NOT_EQUAL,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateAny(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandConstants);
    gcmASSERT(OperandCount == 1);

    gcmASSERT(slsDATA_TYPE_IsBool(ResultConstant->exprBase.dataType) &&
        slsDATA_TYPE_IsBVec(OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    values[0].boolValue = gcvFALSE;

    for (i = 0; i < componentCount; i++)
    {
        if (OperandConstants[0]->values[i].boolValue)
        {
            values[0].boolValue = gcvTRUE;
            break;
        }
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    1,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenAnyCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_ANY,
                            IOperand,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateAll(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandConstants);
    gcmASSERT(OperandCount == 1);

    gcmASSERT(slsDATA_TYPE_IsBool(ResultConstant->exprBase.dataType) &&
        slsDATA_TYPE_IsBVec(OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    values[0].boolValue = gcvTRUE;

    for (i = 0; i < componentCount; i++)
    {
        if (!OperandConstants[0]->values[i].boolValue)
        {
            values[0].boolValue = gcvFALSE;
            break;
        }
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    1,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenAllCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    slsIOPERAND iOperand[2];
    slsROPERAND rOperand[2];
    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    slsIOPERAND_New(Compiler, iOperand, OperandsParameters[0].rOperands[0].dataType,
                                        OperandsParameters[0].rOperands[0].u.reg.precision);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_NOT,
                            iOperand,
                            &OperandsParameters[0].rOperands[0]);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }


    slsROPERAND_InitializeUsingIOperand(rOperand, iOperand);
    slsIOPERAND_New(Compiler, iOperand + 1, IOperand->dataType, rOperand->u.reg.precision);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_ANY,
                            iOperand + 1,
                            rOperand);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }


    slsROPERAND_InitializeUsingIOperand(rOperand + 1, iOperand + 1);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_NOT,
                            IOperand,
                            rOperand + 1);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateNot(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandConstants);
    gcmASSERT(OperandCount == 1);

    gcmASSERT(slsDATA_TYPE_IsBVec(ResultConstant->exprBase.dataType) &&
        slsDATA_TYPE_IsBVec(OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        values[i].boolValue = !OperandConstants[0]->values[i].boolValue;
    }

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    ResultConstant,
                                    componentCount,
                                    values);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenNotCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_NOT,
                            IOperand,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_EvaluateDerivatives(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gctUINT             i, componentCount;
    sluCONSTANT_VALUE   values[4];

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandConstants);

    gcmASSERT(slsDATA_TYPE_IsFloatOrVec(OperandConstants[0]->exprBase.dataType));

    componentCount = (slmDATA_TYPE_vectorSize_GET(OperandConstants[0]->exprBase.dataType) == 0) ?
                        1 : slmDATA_TYPE_vectorSize_NOCHECK_GET(OperandConstants[0]->exprBase.dataType);

    for (i = 0; i < componentCount; i++)
    {
        values[i].floatValue = 0.0;
    }

    status = sloIR_CONSTANT_AddValues(Compiler,
                                      ResultConstant,
                                      componentCount,
                                      values);
    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return status;
}

gceSTATUS
_GenDFdxCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_DFDX,
                            IOperand,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenDFdyCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_DFDY,
                            IOperand,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenFwidthCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_FWIDTH,
                            IOperand,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
_GenBitCountCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = slGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_POPCOUNT,
                            IOperand,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}


gceSTATUS
_GenAtomicCounterCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS         status            = gcvSTATUS_OK;
    slsROPERAND       rOne[1], rZero[1];
    sluCONSTANT_VALUE cone[1], czero[1];
    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    cone->uintValue = 1;
    slsROPERAND_InitializeConstant(&rOne[0],
                                    gcSHADER_UINT_X1,
                                    gcSHADER_PRECISION_HIGH,
                                    1,
                                    cone);

    czero->uintValue = 0;
    slsROPERAND_InitializeConstant(&rZero[0],
                                    gcSHADER_UINT_X1,
                                    gcSHADER_PRECISION_HIGH,
                                    1,
                                    czero);

    if (gcmIS_SUCCESS(gcoOS_StrCmp(PolynaryExpr->funcSymbol, "atomicCounterIncrement")))
    {
        status = slGenAtomicCode(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                slvOPCODE_ATOMADD,
                                IOperand,
                                &OperandsParameters[0].rOperands[0],
                                &rOne[0]);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(PolynaryExpr->funcSymbol, "atomicCounterDecrement")))
    {
        slsIOPERAND iOperand[1];
        slsROPERAND intermROperand[1];

        slsIOPERAND_New(Compiler, &iOperand[0], IOperand[0].dataType, IOperand[0].precision);
        slsROPERAND_InitializeUsingIOperand(intermROperand, iOperand);

        status = slGenAtomicCode(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                slvOPCODE_ATOMSUB,
                                iOperand,
                                &OperandsParameters[0].rOperands[0],
                                &rOne[0]);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        status = slGenArithmeticExprCode(
                                        Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        slvOPCODE_SUB,
                                        IOperand,
                                        &intermROperand[0],
                                        &rOne[0]);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(PolynaryExpr->funcSymbol, "atomicCounter")))
    {
        status = slGenAtomicCode(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                slvOPCODE_ATOMADD,
                                IOperand,
                                &OperandsParameters[0].rOperands[0],
                                &rZero[0]);
        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else
    {
        gcmASSERT(0);
    }

    gcmFOOTER_NO();
    return status;
}

gceSTATUS
_GenAtomicOpCode(
    IN sloCOMPILER              Compiler,
    IN sloCODE_GENERATOR        CodeGenerator,
    IN sloIR_POLYNARY_EXPR      PolynaryExpr,
    IN gctUINT                  OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND *            IOperand
    )
{
    gceSTATUS         status            = gcvSTATUS_OK;
    sleOPCODE         opcode            = slvOPCODE_INVALID;
    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (gcmIS_SUCCESS(gcoOS_StrCmp(PolynaryExpr->funcSymbol, "atomicAdd")))
    {
        gcmASSERT(OperandCount == 2);
        opcode = slvOPCODE_ATOMADD;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(PolynaryExpr->funcSymbol, "atomicMin")))
    {
        gcmASSERT(OperandCount == 2);
        opcode = slvOPCODE_ATOMMIN;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(PolynaryExpr->funcSymbol, "atomicMax")))
    {
        gcmASSERT(OperandCount == 2);
        opcode = slvOPCODE_ATOMMAX;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(PolynaryExpr->funcSymbol, "atomicAnd")))
    {
        gcmASSERT(OperandCount == 2);
        opcode = slvOPCODE_ATOMAND;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(PolynaryExpr->funcSymbol, "atomicOr")))
    {
        gcmASSERT(OperandCount == 2);
        opcode = slvOPCODE_ATOMOR;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(PolynaryExpr->funcSymbol, "atomicXor")))
    {
        gcmASSERT(OperandCount == 2);
        opcode = slvOPCODE_ATOMXOR;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(PolynaryExpr->funcSymbol, "atomicExchange")))
    {
        gcmASSERT(OperandCount == 2);
        opcode = slvOPCODE_ATOMXCHG;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(PolynaryExpr->funcSymbol, "atomicCompSwap")))
    {
        gcmASSERT(OperandCount == 3);
        opcode = slvOPCODE_ATOMCMPXCHG;
    }
    else
    {
        gcmASSERT(0);
    }

    if (!OperandsParameters[0].rOperands[0].isReg ||
        (OperandsParameters[0].rOperands[0].u.reg.qualifier != slvSTORAGE_QUALIFIER_STORAGE_BLOCK_MEMBER &&
         OperandsParameters[0].rOperands[0].u.reg.qualifier != slvSTORAGE_QUALIFIER_SHARED))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        slvREPORT_ERROR,
                                        "Atomic memory functions are supported only for SSBO member and shared variables.",
                                        PolynaryExpr->funcSymbol));
        return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }

    if(opcode == slvOPCODE_ATOMCMPXCHG)
    {
        status = slGenGenericCode3AtomicCmpXchg(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                IOperand,
                                &OperandsParameters[0].rOperands[0],
                                &OperandsParameters[1].rOperands[0],
                                &OperandsParameters[2].rOperands[0]);
    }
    else
    {
        status = slGenGenericCode2Atomic(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                opcode,
                                IOperand,
                                &OperandsParameters[0].rOperands[0],
                                &OperandsParameters[1].rOperands[0]);
    }

    gcmFOOTER_NO();
    return status;
}

gceSTATUS
_GenBarrierOpCode(
    IN sloCOMPILER              Compiler,
    IN sloCODE_GENERATOR        CodeGenerator,
    IN sloIR_POLYNARY_EXPR      PolynaryExpr,
    IN gctUINT                  OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND *            IOperand
    )
{
    gceSTATUS         status            = gcvSTATUS_OK;
    sleOPCODE         opcode            = slvOPCODE_INVALID;
    slsROPERAND       memoryScopeConstant[1];
    slsROPERAND       memorySemantic[1];
    gctBOOL           vertexShaderOK = gcvFALSE,
                      fragmentShaderOK = gcvFALSE,
                      computeShaderOK = gcvFALSE,
                      tesOK = gcvFALSE,
                      tcsOK = gcvFALSE;
    gcSL_MEMORY_SCOPE memoryScope = gcSL_MEMORY_SCOPE_QUEUE_FAMILY;
    gcSL_MEMORY_SEMANTIC_FLAG memorySemanticFlag = gcSL_MEMORY_SEMANTIC_ACQUIRERELEASE;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 0);

    if (gcmIS_SUCCESS(gcoOS_StrCmp(PolynaryExpr->funcSymbol, "barrier")))
    {
        opcode = slvOPCODE_BARRIER;
        computeShaderOK = gcvTRUE;
        tcsOK = gcvTRUE;
        memoryScope = gcSL_MEMORY_SCOPE_WORKGROUP;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(PolynaryExpr->funcSymbol, "memoryBarrier")))
    {
        opcode = slvOPCODE_MEMORY_BARRIER;
        vertexShaderOK = gcvTRUE;
        fragmentShaderOK = gcvTRUE;
        computeShaderOK = gcvTRUE;
        tcsOK = gcvTRUE;
        tesOK = gcvTRUE;
        memorySemanticFlag |=
            (gcSL_MEMORY_SEMANTIC_UNIFORMMEMORY | gcSL_MEMORY_SEMANTIC_WORKGROUPMEMORY | gcSL_MEMORY_SEMANTIC_ATOMICCOUNTERMEMORY | gcSL_MEMORY_SEMANTIC_IMAGEMEMORY);
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(PolynaryExpr->funcSymbol, "memoryBarrierAtomicCounter")))
    {
        opcode = slvOPCODE_MEMORY_BARRIER;
        vertexShaderOK = gcvTRUE;
        fragmentShaderOK = gcvTRUE;
        computeShaderOK = gcvTRUE;
        tcsOK = gcvTRUE;
        tesOK = gcvTRUE;
        memorySemanticFlag |= gcSL_MEMORY_SEMANTIC_ATOMICCOUNTERMEMORY;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(PolynaryExpr->funcSymbol, "memoryBarrierBuffer")))
    {
        opcode = slvOPCODE_MEMORY_BARRIER;
        vertexShaderOK = gcvTRUE;
        fragmentShaderOK = gcvTRUE;
        computeShaderOK = gcvTRUE;
        tcsOK = gcvTRUE;
        tesOK = gcvTRUE;
        memorySemanticFlag |= gcSL_MEMORY_SEMANTIC_UNIFORMMEMORY;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(PolynaryExpr->funcSymbol, "memoryBarrierImage")))
    {
        opcode = slvOPCODE_MEMORY_BARRIER;
        vertexShaderOK = gcvTRUE;
        fragmentShaderOK = gcvTRUE;
        computeShaderOK = gcvTRUE;
        tcsOK = gcvTRUE;
        tesOK = gcvTRUE;
        memorySemanticFlag |= gcSL_MEMORY_SEMANTIC_IMAGEMEMORY;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(PolynaryExpr->funcSymbol, "memoryBarrierShared")))
    {
        opcode = slvOPCODE_MEMORY_BARRIER;
        computeShaderOK = gcvTRUE;
        memorySemanticFlag |= gcSL_MEMORY_SEMANTIC_WORKGROUPMEMORY;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(PolynaryExpr->funcSymbol, "groupMemoryBarrier")))
    {
        opcode = slvOPCODE_MEMORY_BARRIER;
        computeShaderOK = gcvTRUE;
        memoryScope = gcSL_MEMORY_SCOPE_WORKGROUP;
        memorySemanticFlag |=
            (gcSL_MEMORY_SEMANTIC_UNIFORMMEMORY | gcSL_MEMORY_SEMANTIC_WORKGROUPMEMORY | gcSL_MEMORY_SEMANTIC_ATOMICCOUNTERMEMORY | gcSL_MEMORY_SEMANTIC_IMAGEMEMORY);
    }
    else
    {
        gcmASSERT(0);
    }

    if (!((vertexShaderOK && (Compiler->shaderType == slvSHADER_TYPE_VERTEX)) ||
        (fragmentShaderOK && (Compiler->shaderType == slvSHADER_TYPE_FRAGMENT)) ||
        (computeShaderOK && (Compiler->shaderType == slvSHADER_TYPE_COMPUTE)) ||
        (tcsOK && (Compiler->shaderType == slvSHADER_TYPE_TCS)) ||
        (tesOK && (Compiler->shaderType == slvSHADER_TYPE_TES))
         )
       )
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        slvREPORT_ERROR,
                                        "Built-in function \"%s\" is available only in compute shader",
                                        PolynaryExpr->funcSymbol));
        return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }

    slsROPERAND_InitializeIntOrIVecConstant(memoryScopeConstant,
                                            gcSHADER_UINT_X1,
                                            gcSHADER_PRECISION_MEDIUM,
                                            (gctUINT) memoryScope);

    slsROPERAND_InitializeIntOrIVecConstant(memorySemantic,
                                            gcSHADER_UINT_X1,
                                            gcSHADER_PRECISION_MEDIUM,
                                            (gctUINT) memorySemanticFlag);

    status = slGenGenericNullTargetCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        opcode,
                                        memoryScopeConstant,
                                        memorySemantic);

    gcmFOOTER_NO();
    return status;
}

gceSTATUS
_GenFtransformCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    slsROPERAND  rOperand0[1];
    slsROPERAND  rOperand1[1];
    slsNAME     *position;
    slsNAME     *matrix;
    sltPOOL_STRING  position_pool;
    sltPOOL_STRING  matrix_pool;
    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(IOperand);

    gcmONERROR(sloCOMPILER_AllocatePoolString(Compiler, "gl_Vertex", &position_pool));
    gcmONERROR(sloCOMPILER_AllocatePoolString(Compiler, "gl_ModelViewProjectionMatrix", &matrix_pool));

    gcmONERROR(slsNAME_SPACE_Search(
                                Compiler,
                                sloCOMPILER_GetBuiltInSpace(Compiler),
                                position_pool,
                                gcvNULL,
                                gcvNULL,
                                gcvTRUE,
                                gcvFALSE,
                                &position));

    gcmONERROR(slsNAME_SPACE_Search(
                                Compiler,
                                sloCOMPILER_GetBuiltInSpace(Compiler),
                                matrix_pool,
                                gcvNULL,
                                gcvNULL,
                                gcvTRUE,
                                gcvFALSE,
                                &matrix));

    gcmONERROR(slsNAME_AllocLogicalRegs(Compiler, CodeGenerator, position));
    gcmONERROR(slsNAME_AllocLogicalRegs(Compiler, CodeGenerator, matrix));

    slsROPERAND_InitializeReg(rOperand0, position->context.logicalRegs);
    slsROPERAND_InitializeReg(rOperand1, matrix->context.logicalRegs);

    gcmONERROR(slGenArithmeticExprCode(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            slvOPCODE_MUL,
                            IOperand,
                            rOperand1,
                            rOperand0));

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
_GenEmitVertexCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER();

    status = slGenGenericNullTargetCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        slvOPCODE_EMIT_VERTEX,
                                        gcvNULL,
                                        gcvNULL);

    gcmFOOTER();
    return status;
}

gceSTATUS
_GenEndPrimitiveCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER();

    status = slGenGenericNullTargetCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        slvOPCODE_END_PRIMITIVE,
                                        gcvNULL,
                                        gcvNULL);

    gcmFOOTER();
    return status;
}

gceSTATUS
slEvaluateBuiltInFunction(
    IN sloCOMPILER Compiler,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    OUT sloIR_CONSTANT * ResultConstant
    )
{
    gceSTATUS                       status;
    sltBUILT_IN_EVALUATE_FUNC_PTR   evaluate = gcvNULL;
    sloIR_CONSTANT                  resultConstant;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(PolynaryExpr->type == slvPOLYNARY_FUNC_CALL);
    gcmASSERT(PolynaryExpr->funcName != gcvNULL);
    gcmASSERT(PolynaryExpr->funcName->isBuiltIn);
    gcmASSERT(OperandCount > 0);
    gcmASSERT(OperandConstants);
    gcmASSERT(ResultConstant);

    *ResultConstant = gcvNULL;

    evaluate = (sltBUILT_IN_EVALUATE_FUNC_PTR)(PolynaryExpr->funcName->u.funcInfo.evaluate);

    if (evaluate == gcvNULL) { gcmFOOTER_NO(); return gcvSTATUS_OK; }

    /* Create the constant */
    PolynaryExpr->exprBase.dataType->qualifiers.storage = slvSTORAGE_QUALIFIER_CONST;

    status = sloIR_CONSTANT_Construct(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    PolynaryExpr->exprBase.dataType,
                                    &resultConstant);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    /* Evaluate the built-in */
    status = (*evaluate)(
                        Compiler,
                        OperandCount,
                        OperandConstants,
                        resultConstant);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    *ResultConstant = resultConstant;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
slGenBuiltInFunctionCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand,
    IN OUT slsGEN_CODE_PARAMETERS * Parameters
    )
{
    sltBUILT_IN_GEN_CODE_FUNC_PTR   genCode = gcvNULL;
    gceSTATUS                       status;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(PolynaryExpr->type == slvPOLYNARY_FUNC_CALL);
    gcmASSERT(PolynaryExpr->funcName != gcvNULL);
    gcmASSERT(PolynaryExpr->funcName->isBuiltIn);
    gcmASSERT(Parameters);

    genCode = (sltBUILT_IN_GEN_CODE_FUNC_PTR)(PolynaryExpr->funcName->u.funcInfo.genCode);

    gcmASSERT(genCode);

    if (genCode == gcvNULL)
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_COMPILER_FE_PARSER_ERROR);
        return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }

    status = (*genCode)(
                    Compiler,
                    CodeGenerator,
                    PolynaryExpr,
                    OperandCount,
                    OperandsParameters,
                    IOperand);
    gcmFOOTER();
    return status;
}

sloEXTENSION
slGetBuiltinFunctionExtension(
    IN gctSTRING Symbol
)
{
    sloEXTENSION extension = {0};
    gctUINT i;

    extension.extension1 = slvEXTENSION1_NONE;
    for (i = 0; i < builtinFunctionExtensionCount; i++)
    {
        if (gcmIS_SUCCESS(gcoOS_StrCmp(Symbol, builtinFunctionExtensionTable[i].symbol)))
        {
            extension.extension1 = builtinFunctionExtensionTable[i].extension;
            break;
        }
    }

    return extension;
}
