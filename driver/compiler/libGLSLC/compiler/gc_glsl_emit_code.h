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


#ifndef __gc_glsl_emit_code_h_
#define __gc_glsl_emit_code_h_

#include "gc_glsl_parser.h"
#include "gc_glsl_gen_code.h"

#define _PI                ((gctFLOAT)3.14159265358979323846)
#define _HALF_PI        (_PI / (gctFLOAT)2.0)

gcSL_FORMAT
slConvDataTypeToFormat(
sloCOMPILER Compiler,
gcSHADER_TYPE DataType
);

gctUINT
gcGetDataTypeSize(
    IN gcSHADER_TYPE DataType
    );

gctUINT8
gcGetDataTypeComponentCount(
    IN gcSHADER_TYPE DataType
    );

gcSHADER_TYPE
gcGetComponentDataType(
    IN gcSHADER_TYPE DataType
    );

gctCONST_STRING
gcGetDataTypeName(
    IN gcSHADER_TYPE DataType
    );

gctBOOL
gcIsSamplerDataType(
    IN gcSHADER_TYPE DataType
    );

gctBOOL
gcIsImageDataType(
    IN gcSHADER_TYPE DataType
    );

gctBOOL
gcIsAtomicDataType(
    IN gcSHADER_TYPE DataType
    );

gctBOOL
gcIsSamplerArrayDataType(
    IN gcSHADER_TYPE DataType
    );

gctBOOL
gcIsImageArrayDataType(
    IN gcSHADER_TYPE DataType
    );

gctBOOL
gcIsScalarDataType(
    IN gcSHADER_TYPE DataType
    );

gctBOOL
gcIsVectorDataType(
    IN gcSHADER_TYPE DataType
    );

gctBOOL
gcIsFloatDataType(
    IN gcSHADER_TYPE DataType
    );

gctBOOL
gcIsIntegerDataType(
    IN gcSHADER_TYPE DataType
    );

gctBOOL
gcIsMatrixDataType(
    IN gcSHADER_TYPE DataType
    );

gctBOOL
gcIsSymmetricalMatrixDataType(
    IN gcSHADER_TYPE DataType
    );

gcSHADER_TYPE
gcGetVectorComponentDataType(
    IN gcSHADER_TYPE DataType
    );

gcSHADER_TYPE
gcGetVectorSliceDataType(
    IN gcSHADER_TYPE DataType,
    IN gctUINT8 Components
    );

gcSHADER_TYPE
gcConvScalarToVectorDataType(
    IN gcSHADER_TYPE DataType,
    IN gctUINT8 Components
    );


gcSHADER_TYPE
gcChangeElementDataType(
    IN gcSHADER_TYPE BaseDataType,
    IN gcSHADER_TYPE ToElementDataType
    );

gctUINT8
gcGetVectorDataTypeComponentCount(
    IN gcSHADER_TYPE DataType
    );

gcSHADER_TYPE
gcGetVectorComponentSelectionDataType(
    IN gcSHADER_TYPE DataType,
    IN gctUINT8    Components
    );

gcSHADER_TYPE
gcGetMatrixColumnDataType(
    IN gcSHADER_TYPE DataType
    );

gctUINT
gcGetMatrixDataTypeColumnCount(
    IN gcSHADER_TYPE DataType
    );

gctUINT
gcGetMatrixDataTypeRowCount(
    IN gcSHADER_TYPE DataType
    );

gctCONST_STRING
gcGetAttributeName(
    IN gcSHADER Shader,
    IN gcATTRIBUTE Attribute
    );

gctCONST_STRING
gcGetUniformName(
    IN gcUNIFORM Uniform
    );

gctUINT8
gcGetDefaultEnable(
    IN gcSHADER_TYPE DataType
    );

gctUINT8
gcGetVectorComponentEnable(
    IN gctUINT8 Enable,
    IN gctUINT8 Component
    );

gctUINT8
gcGetDefaultSwizzle(
    IN gcSHADER_TYPE DataType
    );

gctUINT8
gcGetVectorComponentSwizzle(
    IN gctUINT8 Swizzle,
    IN gctUINT8 Component
    );

typedef struct _gcsTARGET
{
    gcSL_FORMAT           format;

    gcSHADER_TYPE         dataType;        /* Excluding gcSHADER_FLOAT_2X2/3X3/4X4 */

    gcSHADER_PRECISION    precision;

    gctREG_INDEX          tempRegIndex;

    gctUINT8              enable;

    gcSL_INDEXED          indexMode;

    gctREG_INDEX          indexRegIndex;
}
gcsTARGET;

#define gcsTARGET_Initialize(target, _dataType, _precision,  _tempRegIndex, _enable, _indexMode, _indexRegIndex) \
    do \
    { \
        (target)->format           = -1; \
        (target)->dataType         = (_dataType); \
        (target)->precision        = (_precision); \
        (target)->tempRegIndex     = (_tempRegIndex); \
        (target)->enable           = (_enable); \
        (target)->indexMode        = (_indexMode); \
        (target)->indexRegIndex    = (_indexRegIndex); \
    } \
    while (gcvFALSE)

#define gcsTARGET_InitializeAsVectorComponent(componentTarget, target, component) \
    do \
    { \
        *(componentTarget)        = *(target); \
        (componentTarget)->dataType    = gcGetVectorComponentDataType((target)->dataType); \
        (componentTarget)->enable    = gcGetVectorComponentEnable((target)->enable, (gctUINT8) component); \
    } \
    while (gcvFALSE)

#define gcsTARGET_InitializeUsingIOperand(target, iOperand) \
    gcsTARGET_Initialize( \
                        (target), \
                        (iOperand)->dataType, \
                        (iOperand)->precision, \
                        (iOperand)->tempRegIndex, \
                        gcGetDefaultEnable((iOperand)->dataType), \
                        gcSL_NOT_INDEXED, \
                        0)

typedef enum _gceSOURCE_TYPE
{
    gcvSOURCE_TEMP,
    gcvSOURCE_ATTRIBUTE,
    gcvSOURCE_UNIFORM,
    gcvSOURCE_CONSTANT,
    gcvSOURCE_TARGET_FORMAT,
    gcvSOURCE_VERTEX_ID,
    gcvSOURCE_INSTANCE_ID,
    gcvSOURCE_OUTPUT
}
gceSOURCE_TYPE;

typedef struct _gcsSOURCE_REG
{
    union
    {
        gcATTRIBUTE    attribute;

        gcUNIFORM      uniform;

        gcOUTPUT       output;
    }
    u;

    gctREG_INDEX       regIndex;

    gctUINT8           swizzle;

    gcSL_INDEXED_LEVEL indexedLevel;

    gcSL_INDEXED       indexMode;

    gctREG_INDEX       indexRegIndex;
}
gcsSOURCE_REG;

typedef struct _gcsSOURCE_CONSTANT
{
    union
    {
        gctFLOAT    floatConstant;

        gctINT        intConstant;

        gctINT      uintConstant;

        gctBOOL     boolConstant;
    }
    u;
}
gcsSOURCE_CONSTANT;

typedef struct _gcsSOURCE
{
    gceSOURCE_TYPE    type;

    gcSHADER_TYPE     dataType;        /* Excluding gcSHADER_FLOAT_2X2/3X3/4X4 */

    gcSHADER_PRECISION    precision;

    union
    {
        gcsSOURCE_REG         sourceReg;

        gcsSOURCE_CONSTANT    sourceConstant;
    }
    u;
}
gcsSOURCE;

#define gcsSOURCE_InitializeTargetFormat(source, _dataType) \
    do \
    { \
        (source)->type        = gcvSOURCE_TARGET_FORMAT; \
        (source)->dataType    = (_dataType); \
        (source)->precision    = gcSHADER_PRECISION_LOW; \
    } \
    while (gcvFALSE)

#define gcsSOURCE_InitializeReg(source, _type, _dataType, _precision, _sourceReg) \
    do \
    { \
        (source)->type        = (_type); \
        (source)->dataType    = (_dataType); \
        (source)->precision    = (_precision); \
        (source)->u.sourceReg    = (_sourceReg); \
    } \
    while (gcvFALSE)

#define gcsSOURCE_InitializeConstant(source, _dataType, _precision, _sourceConstant) \
    do \
    { \
        (source)->type                = gcvSOURCE_CONSTANT; \
        (source)->dataType            = (_dataType); \
        (source)->precision           = (_precision); \
        (source)->u.sourceConstant    = (_sourceConstant); \
    } \
    while (gcvFALSE)

#define gcsSOURCE_InitializeFloatConstant(source, _precision, _floatValue) \
    do \
    { \
        (source)->type                = gcvSOURCE_CONSTANT; \
        (source)->dataType            = gcSHADER_FLOAT_X1; \
        (source)->precision           = (_precision); \
        (source)->u.sourceConstant.u.floatConstant    = (_floatValue); \
    } \
    while (gcvFALSE)

#define gcsSOURCE_InitializeBoolConstant(source, _precision, _boolValue) \
    do \
    { \
        (source)->type        = gcvSOURCE_CONSTANT; \
        (source)->dataType    = gcSHADER_BOOLEAN_X1; \
        (source)->precision     = (_precision); \
        (source)->u.sourceConstant.u.boolConstant = (_boolValue); \
    } \
    while (gcvFALSE)

#define gcsSOURCE_InitializeIntConstant(source, _precision, _intValue) \
    do \
    { \
        (source)->type        = gcvSOURCE_CONSTANT; \
        (source)->dataType    = gcSHADER_INTEGER_X1; \
        (source)->precision     = (_precision); \
        (source)->u.sourceConstant.u.intConstant = (_intValue); \
    } \
    while (gcvFALSE)

#define gcsSOURCE_InitializeUintConstant(source, _precision, _uintValue) \
    do \
    { \
        (source)->type        = gcvSOURCE_CONSTANT; \
        (source)->dataType    = gcSHADER_UINT_X1; \
        (source)->precision     = (_precision); \
        (source)->u.sourceConstant.u.uintConstant = (_uintValue); \
    } \
    while (gcvFALSE)

#define gcsSOURCE_InitializeTempReg(source, _dataType, _precision,  _tempRegIndex, _swizzle, _indexMode, _indexRegIndex) \
    do \
    { \
        (source)->type                = gcvSOURCE_TEMP; \
        (source)->dataType            = (_dataType); \
        (source)->precision            = (_precision); \
        (source)->u.sourceReg.regIndex        = (_tempRegIndex); \
        (source)->u.sourceReg.swizzle        = (_swizzle); \
        (source)->u.sourceReg.indexMode        = (_indexMode); \
        (source)->u.sourceReg.indexRegIndex    = (_indexRegIndex); \
    } \
    while (gcvFALSE)

#define gcsSOURCE_InitializeAsVectorComponent(componentSource, source, component) \
    do \
    { \
        *(componentSource)        = *(source); \
        (componentSource)->dataType    = gcGetVectorComponentDataType((source)->dataType); \
        \
        if ((source)->type != gcvSOURCE_CONSTANT) \
        { \
            (componentSource)->u.sourceReg.swizzle    = \
                    gcGetVectorComponentSwizzle((source)->u.sourceReg.swizzle, (gctUINT8) component); \
        } \
    } \
    while (gcvFALSE)

#define gcsSOURCE_InitializeUsingIOperand(source, iOperand) \
    gcsSOURCE_InitializeTempReg( \
                                (source), \
                                (iOperand)->dataType, \
                                (iOperand)->precision, \
                                (iOperand)->tempRegIndex, \
                                gcGetDefaultSwizzle((iOperand)->dataType), \
                                gcSL_NOT_INDEXED, \
                                0)

#define gcsSOURCE_InitializeUsingIOperandAsVectorComponent(source, iOperand, swizzle) \
    gcsSOURCE_InitializeTempReg( \
                                (source), \
                                gcGetVectorComponentDataType((iOperand)->dataType), \
                                (iOperand)->precision, \
                                (iOperand)->tempRegIndex, \
                                (swizzle), \
                                gcSL_NOT_INDEXED, \
                                0)

#define gcsPrecsionGet
gceSTATUS
slNewAttributeWithLocation(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctCONST_STRING Name,
    IN gcSHADER_TYPE DataType,
    IN gcSHADER_PRECISION Precision,
    IN gctUINT Length,
    IN gctUINT ArrayLengthCount,
    IN gctBOOL IsTexture,
    IN gcSHADER_SHADERMODE ShaderMode,
    IN gctINT Location,
    IN gctINT FieldIndex,
    IN gctBOOL IsInvariant,
    IN gctBOOL IsPrecise,
    OUT gcATTRIBUTE * Attribute
    );

gceSTATUS
slNewUniform(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctCONST_STRING Name,
    IN gcsSHADER_VAR_INFO *UniformInfo,
    OUT gctINT16* ThisUniformIndex,
    OUT gcUNIFORM * Uniform
    );

gceSTATUS
slGetUniformSamplerIndex(
    IN sloCOMPILER Compiler,
    IN gcUNIFORM UniformSampler,
    OUT gctREG_INDEX * Index
    );

gceSTATUS
slNewOutput(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctCONST_STRING Name,
    IN gcSHADER_TYPE DataType,
    IN gcSHADER_PRECISION Precision,
    IN gctBOOL IsArray,
    IN gctUINT Length,
    IN gctREG_INDEX TempRegIndex,
    IN gctINT FieldIndex,
    IN gctBOOL IsInvariant,
    IN gctBOOL IsPrecise,
    IN gcSHADER_SHADERMODE ShaderMode,
    IN gceLAYOUT_QUALIFIER LayoutQual,
    OUT gcOUTPUT* Output
    );

gceSTATUS
slNewOutputWithLocation(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctCONST_STRING Name,
    IN gcSHADER_TYPE DataType,
    IN gcSHADER_PRECISION Precision,
    IN gctBOOL IsArray,
    IN gctUINT Length,
    IN gctREG_INDEX TempRegIndex,
    IN gctINT Location,
    IN gctINT FieldIndex,
    IN gctBOOL IsInvariant,
    IN gctBOOL IsPrecise,
    IN gcSHADER_SHADERMODE ShaderMode,
    IN gceLAYOUT_QUALIFIER LayoutQual,
    OUT gcOUTPUT *         Output
    );

gceSTATUS
slNewVariable(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctCONST_STRING Name,
    IN gctREG_INDEX TempRegIndex,
    IN gcsSHADER_VAR_INFO *VarInfo,
    OUT gctINT16* ThisVarIndex
    );

gceSTATUS
slUpdateVariableTempReg(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctUINT varIndex,
    IN gctREG_INDEX newTempRegIndex
    );

gctREG_INDEX
slNewTempRegs(
    IN sloCOMPILER Compiler,
    IN gctUINT RegCount
    );

gctLABEL
slNewLabel(
    IN sloCOMPILER Compiler
    );

gceSTATUS
slSetLabel(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctLABEL Label
    );

gceSTATUS
slEmitAssignCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    );

gceSTATUS
slEmitConvCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source,
    IN gcSHADER_TYPE DataType
    );
gceSTATUS
slEmitCode1(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    );

gceSTATUS
slEmitCode2(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    );

gceSTATUS
slEmitBuiltinAsmCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcSL_OPCODE Opcode,
    IN slsASM_OPCODE * AsmOpcode,
    IN gcsTARGET * Target,
    IN slsASM_MODIFIERS *TargetModifiers,
    IN gcsSOURCE * Source0,
    IN slsASM_MODIFIERS *Source0Modifiers,
    IN gcsSOURCE * Source1,
    IN slsASM_MODIFIERS *Source1Modifiers
    );

gceSTATUS
slEmitNullTargetCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    );

gceSTATUS
slEmitAlwaysBranchCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN gctLABEL Label
    );

gceSTATUS
slEmitTestBranchCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN gctLABEL Label,
    IN gctBOOL TrueBranch,
    IN gcsSOURCE * Source
    );

gceSTATUS
slEmitCompareBranchCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN sleCONDITION Condition,
    IN gctLABEL Label,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    );

gceSTATUS
slEmitSelectCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET *Target,
    IN gcsSOURCE * Cond,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    );

gceSTATUS
slBeginMainFunction(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo
    );

gceSTATUS
slEndMainFunction(
    IN sloCOMPILER Compiler
    );

gceSTATUS
slNewFunction(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctCONST_STRING Name,
    OUT gcFUNCTION * Function
    );

gceSTATUS
slNewFunctionArgument(
    IN sloCOMPILER Compiler,
    IN gcFUNCTION Function,
    IN gctUINT16 VariableIndex,
    IN gcSHADER_TYPE DataType,
    IN gctUINT Length,
    IN gctREG_INDEX TempRegIndex,
    IN gctUINT8 Qualifier,
    IN gctUINT8 Precision,
    IN gctBOOL IsPrecise
    );

gceSTATUS
slBeginFunction(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcFUNCTION Function
    );

gceSTATUS
slEndFunction(
    IN sloCOMPILER Compiler,
    IN gcFUNCTION Function
    );

gceSTATUS
slGetFunctionLabel(
    IN sloCOMPILER Compiler,
    IN gcFUNCTION Function,
    OUT gctLABEL * Label
    );

gceSTATUS
sloCODE_EMITTER_Construct(
    IN sloCOMPILER Compiler,
    OUT sloCODE_EMITTER * CodeEmitter
    );

gceSTATUS
sloCODE_EMITTER_Destroy(
    IN sloCOMPILER Compiler,
    IN sloCODE_EMITTER CodeEmitter
    );

gctCONST_STRING
GetOpcodeName(
    IN gcSL_OPCODE Opcode
    );

gcSHADER_PRECISION
GetHigherPrecison(
    gcSHADER_PRECISION     precision1,
    gcSHADER_PRECISION     precision2
    );

#endif /* __gc_glsl_emit_code_h_ */
