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


#ifndef __gc_cl_emit_code_h_
#define __gc_cl_emit_code_h_

#include "gc_cl_parser.h"
#include "gc_cl_gen_code.h"

#define _PI            ((gctFLOAT)3.14159265358979323846)
#define _HALF_PI        (_PI / (gctFLOAT)2.0)

#define cldMachineWordSize    32
#define cldMachineByteSize    8
#define cldMachineBytesPerWord    (cldMachineWordSize/cldMachineByteSize)
#define cldMachineByteAlignment    128

#define cldMaxWorkDim            3

/* Maximum number of 4-component unit in particular, associated with an
   extended size vector */
#define cldMaxFourComponentCount 8

gctSIZE_T
gcGetAddressableUnitSize(
    IN clsGEN_CODE_DATA_TYPE DataType
    );

gctSIZE_T
gcGetDataTypeRegSize(
    IN clsGEN_CODE_DATA_TYPE DataType
    );

gctUINT8
gcGetDataTypeComponentCount(
    IN clsGEN_CODE_DATA_TYPE DataType
    );

gctUINT8
gcGetDataTypeTargetComponentCount(
IN clsGEN_CODE_DATA_TYPE DataType
);

clsGEN_CODE_DATA_TYPE
gcGetComponentDataType(
    IN clsGEN_CODE_DATA_TYPE DataType
    );

gctBOOL
gcIsElementTypeEqual(
    IN clsGEN_CODE_DATA_TYPE DataType1,
    IN clsGEN_CODE_DATA_TYPE DataType2
    );

gctBOOL
gcIsDataTypeEqual(
    IN clsGEN_CODE_DATA_TYPE DataType1,
    IN clsGEN_CODE_DATA_TYPE DataType2
    );

gctCONST_STRING
gcGetDataTypeName(
    IN cloCOMPILER Compiler,
    IN clsGEN_CODE_DATA_TYPE Type
    );

gctCONST_STRING
gcGetShaderDataTypeName(
    IN gcSHADER_TYPE Type
    );

gctBOOL
gcIsSamplerDataType(
    IN clsGEN_CODE_DATA_TYPE DataType
    );

gctBOOL
gcIsScalarDataType(
    IN clsGEN_CODE_DATA_TYPE DataType
    );

gctBOOL
gcIsVectorDataType(
    IN clsGEN_CODE_DATA_TYPE DataType
    );

gctBOOL
gcIsMatrixDataType(
    IN clsGEN_CODE_DATA_TYPE DataType
    );

clsGEN_CODE_DATA_TYPE
gcGetVectorComponentDataType(
    IN clsGEN_CODE_DATA_TYPE DataType
    );

clsGEN_CODE_DATA_TYPE
gcGetVectorSliceDataType(
    IN clsGEN_CODE_DATA_TYPE DataType,
    IN gctUINT8 Components
    );

clsGEN_CODE_DATA_TYPE
gcConvScalarToVectorDataType(
    IN clsGEN_CODE_DATA_TYPE DataType,
    IN gctUINT8 Components
    );

gctUINT8
gcGetVectorDataTypeComponentCount(
    IN clsGEN_CODE_DATA_TYPE DataType
    );

clsGEN_CODE_DATA_TYPE
gcGetVectorComponentSelectionDataType(
    IN clsGEN_CODE_DATA_TYPE DataType,
    IN gctUINT8    Components
    );

clsGEN_CODE_DATA_TYPE
gcGetMatrixColumnDataType(
    IN clsGEN_CODE_DATA_TYPE DataType
    );

gctUINT
gcGetMatrixDataTypeColumnCount(
    IN clsGEN_CODE_DATA_TYPE DataType
    );

gctUINT
gcGetMatrixDataTypeRowCount(
    IN clsGEN_CODE_DATA_TYPE DataType
    );

gctSIZE_T
gcGetMatrixColumnRegSize(
IN clsGEN_CODE_DATA_TYPE DataType
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

gcSL_FORMAT
clConvDataTypeToFormat(
clsGEN_CODE_DATA_TYPE DataType
);

gctUINT8
clConvPackedTypeToSwizzle(
    IN cloCOMPILER Compiler,
    IN clsGEN_CODE_DATA_TYPE PackedType
    );

gctUINT8
clConvPackedTypeToEnable(
    IN cloCOMPILER Compiler,
    IN clsGEN_CODE_DATA_TYPE PackedType
    );

gctUINT8
gcGetDefaultEnable(
    IN cloCOMPILER compiler,
    IN clsGEN_CODE_DATA_TYPE DataType
    );

gctUINT8
gcGetVectorComponentEnable(
    IN gctUINT8 Enable,
    IN gctUINT8 Component
    );

gctUINT8
gcGetDefaultSwizzle(
    IN cloCOMPILER compiler,
    IN clsGEN_CODE_DATA_TYPE DataType
    );

gctUINT8
gcGetVectorComponentSwizzle(
    IN gctUINT8 Swizzle,
    IN gctUINT8 Component
    );

typedef struct _gcsTYPE_SIZE
{
    gcSHADER_TYPE    type;
    gctSIZE_T    length;
}
gcsTYPE_SIZE;

typedef struct _gcsTARGET
{
    clsGEN_CODE_DATA_TYPE    dataType; /* Excluding gcSHADER_FLOAT_2X2/3X3/4X4 */

    gctREG_INDEX    tempRegIndex;

    gctUINT8    enable;

    gcSL_INDEXED    indexMode;

    gctREG_INDEX    indexRegIndex;
}
gcsTARGET;

typedef struct _gcsSUPER_TARGET
{
    gctUINT8    numTargets;
    gcsTARGET    targets[cldMaxComponentCount];
}
gcsSUPER_TARGET;

#define gcsTARGET_Initialize(target, _dataType, _tempRegIndex, _enable, _indexMode, _indexRegIndex) \
    do \
    { \
        (target)->dataType    = (_dataType); \
        (target)->tempRegIndex    = (_tempRegIndex); \
        (target)->enable    = (_enable); \
        (target)->indexMode    = (_indexMode); \
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

#define gcsTARGET_InitializeUsingIOperand(compiler, target, iOperand) \
    gcsTARGET_Initialize( \
                        (target), \
                        (iOperand)->dataType, \
                        (iOperand)->tempRegIndex, \
                        gcGetDefaultEnable((compiler), (iOperand)->dataType), \
                        gcSL_NOT_INDEXED, \
                        0)

typedef enum _gceSOURCE_TYPE
{
    gcvSOURCE_TEMP,
    gcvSOURCE_ATTRIBUTE,
    gcvSOURCE_UNIFORM,
    gcvSOURCE_CONSTANT,
    gcvSOURCE_TARGET_FORMAT
}
gceSOURCE_TYPE;

typedef struct _gcsSOURCE_REG
{
    union {
    gcATTRIBUTE    attribute;
    gcUNIFORM    uniform;
    } u;

    gctREG_INDEX    regIndex;

    gctUINT8        swizzle;

    gcSL_INDEXED    indexMode;

    gctREG_INDEX    indexRegIndex;
}
gcsSOURCE_REG;

#define gcsSOURCE_CONSTANT cluCONSTANT_VALUE

typedef struct _gcsSOURCE
{
    gceSOURCE_TYPE type;

    clsGEN_CODE_DATA_TYPE dataType; /* Excluding gcSHADER_FLOAT_2X2/3X3/4X4 */

    union {
    gcsSOURCE_REG    sourceReg;
    gcsSOURCE_CONSTANT sourceConstant;
    } u;
}
gcsSOURCE;

typedef struct _gcsSUPER_SOURCE
{
    gctUINT8    numSources;
    gcsSOURCE    sources[cldMaxComponentCount];
}
gcsSUPER_SOURCE;

#define gcsSOURCE_InitializeTargetFormat(source, _dataType) \
    do \
    { \
        (source)->type        = gcvSOURCE_TARGET_FORMAT; \
        (source)->dataType    = (_dataType); \
    } \
    while (gcvFALSE)

#define gcsSOURCE_InitializeReg(source, _type, _dataType, _sourceReg) \
    do \
    { \
        (source)->type        = (_type); \
        (source)->dataType    = (_dataType); \
        (source)->u.sourceReg    = (_sourceReg); \
    } \
    while (gcvFALSE)

#define gcsSOURCE_InitializeConstant(source, _dataType, _sourceConstant) \
    do \
    { \
        (source)->type        = gcvSOURCE_CONSTANT; \
        (source)->dataType    = (_dataType); \
        (source)->u.sourceConstant = (_sourceConstant); \
    } \
    while (gcvFALSE)

#define gcsSOURCE_InitializeFloatConstant(source, _floatValue) \
    do \
    { \
        (source)->type        = gcvSOURCE_CONSTANT; \
        clmGEN_CODE_DATA_TYPE_Initialize((source)->dataType, 0, 0, clvTYPE_FLOAT); \
        (source)->u.sourceConstant.floatValue = (_floatValue); \
    } \
    while (gcvFALSE)

#define gcsSOURCE_InitializeBoolConstant(source, _boolValue) \
    do \
    { \
        (source)->type        = gcvSOURCE_CONSTANT; \
        clmGEN_CODE_DATA_TYPE_Initialize((source)->dataType, 0, 0, clvTYPE_BOOL); \
        (source)->u.sourceConstant.boolValue = (_boolValue); \
    } \
    while (gcvFALSE)

#define gcsSOURCE_InitializeIntConstant(source, _intValue) \
    do \
    { \
        (source)->type        = gcvSOURCE_CONSTANT; \
        clmGEN_CODE_DATA_TYPE_Initialize((source)->dataType, 0, 0, clvTYPE_INT); \
        (source)->u.sourceConstant.intValue = (_intValue); \
    } \
    while (gcvFALSE)

#define gcsSOURCE_InitializeUintConstant(source, _uintValue) \
    do \
    { \
        (source)->type        = gcvSOURCE_CONSTANT; \
        clmGEN_CODE_DATA_TYPE_Initialize((source)->dataType, 0, 0, clvTYPE_UINT); \
        (source)->u.sourceConstant.uintValue = (_uintValue); \
    } \
    while (gcvFALSE)

#define gcsSOURCE_InitializeTempReg(source, _dataType, _tempRegIndex, _swizzle, _indexMode, _indexRegIndex) \
    do \
    { \
        (source)->type                = gcvSOURCE_TEMP; \
        (source)->dataType            = (_dataType); \
        (source)->u.sourceReg.regIndex        = (_tempRegIndex); \
        (source)->u.sourceReg.swizzle        = (_swizzle); \
        (source)->u.sourceReg.indexMode        = (_indexMode); \
        (source)->u.sourceReg.indexRegIndex    = (_indexRegIndex); \
    } \
    while (gcvFALSE)

#define gcsSOURCE_InitializeAsVectorComponent(componentSource, source, component) \
    do \
    { \
        *(componentSource)    = *(source); \
        (componentSource)->dataType = gcGetVectorComponentDataType((source)->dataType); \
        \
        if ((source)->type != gcvSOURCE_CONSTANT) \
        { \
            (componentSource)->u.sourceReg.swizzle    = \
                    gcGetVectorComponentSwizzle((source)->u.sourceReg.swizzle, (gctUINT8) component); \
        } \
    } \
    while (gcvFALSE)

#define gcsSOURCE_InitializeUsingIOperand(compiler, source, iOperand) \
    gcsSOURCE_InitializeTempReg( \
                (source), \
                (iOperand)->dataType, \
                (iOperand)->tempRegIndex, \
                gcGetDefaultSwizzle((compiler), (iOperand)->dataType), \
                gcSL_NOT_INDEXED, \
                0)

#define gcsSOURCE_InitializeUsingIOperandAsVectorComponent(source, iOperand, swizzle) \
    gcsSOURCE_InitializeTempReg( \
                (source), \
                gcGetVectorComponentDataType((iOperand)->dataType), \
                (iOperand)->tempRegIndex, \
                (swizzle), \
                gcSL_NOT_INDEXED, \
                0)

gcsTYPE_SIZE
clConvToShaderDataType(
cloCOMPILER Compiler,
clsGEN_CODE_DATA_TYPE DataType
);

gceSTATUS
clNewAttribute(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctCONST_STRING Name,
    IN clsGEN_CODE_DATA_TYPE DataType,
    IN gctSIZE_T Length,
    IN gctBOOL IsTexture,
    OUT gcATTRIBUTE * Attribute
    );

gceSTATUS
clNewUniform(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctCONST_STRING Name,
    IN clsGEN_CODE_DATA_TYPE DataType,
    IN clsGEN_CODE_DATA_TYPE Format,
    IN gctUINT Flags,
    IN gctBOOL IsPointer,
    IN gctSIZE_T Length,
    IN clsARRAY *Array,
    OUT gcUNIFORM * Uniform
    );

gceSTATUS
clNewKernelUniformArgument(
    IN cloCOMPILER Compiler,
    IN clsNAME *KernelFuncName,
    IN gctCONST_STRING Name,
    IN clsGEN_CODE_DATA_TYPE DataType,
    IN clsGEN_CODE_DATA_TYPE Format,
    IN clsNAME *ParamName,
    IN gctSIZE_T Length,
    IN clsARRAY *Array,
    OUT gcUNIFORM * Uniform
    );

gceSTATUS
clGetUniformSamplerIndex(
    IN cloCOMPILER Compiler,
    IN gcUNIFORM UniformSampler,
    OUT gctREG_INDEX * Index
    );

gceSTATUS
clNewOutput(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctCONST_STRING Name,
    IN clsGEN_CODE_DATA_TYPE DataType,
    IN gctSIZE_T Length,
    IN gctREG_INDEX TempRegIndex
    );

gceSTATUS
clNewVariable(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctCONST_STRING Name,
    IN cltQUALIFIER AccessQualifier,
    IN cltQUALIFIER AddrSpaceQualifier,
    IN cltQUALIFIER StorageQualifier,
    IN clsGEN_CODE_DATA_TYPE DataType,
    IN gctSIZE_T Length,
    IN clsARRAY  *Array,
    IN gctBOOL   IsArray,
    IN gctREG_INDEX TempRegIndex,
    OUT gcVARIABLE *Variable
    );

gctTYPE_QUALIFIER
clConvStorageQualifierToShaderTypeQualifier(
cltQUALIFIER Qualifier
);

gctTYPE_QUALIFIER
clConvToShaderTypeQualifier(
cltQUALIFIER Qualifier
);

gctREG_INDEX
clNewTempRegs(
    IN cloCOMPILER Compiler,
    IN gctUINT RegCount,
    IN cltELEMENT_TYPE ElementType
    );

gctREG_INDEX
clNewLocalTempRegs(
IN cloCOMPILER Compiler,
IN gctUINT RegCount
);

void
clResetLocalTempRegs(
IN cloCOMPILER Compiler,
IN gctUINT TempIndex
);

gctLABEL
clNewLabel(
    IN cloCOMPILER Compiler
    );

gceSTATUS
clSetLabel(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctLABEL Label
    );

gceSTATUS
clEmitAssignCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source,
    IN gctBOOL IsUnionMember
    );

gceSTATUS
clEmitCode1(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source
    );

gceSTATUS
clEmitCode0(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode
    );

gceSTATUS
clEmitConvCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source,
    IN clsGEN_CODE_DATA_TYPE DataType
    );

gceSTATUS
clEmitCode2(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN gcsTARGET * Target,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    );

gceSTATUS
clEmitNullTargetCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    );

gceSTATUS
clEmitAlwaysBranchCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN gctLABEL Label
    );

gceSTATUS
clEmitTestBranchCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN gctLABEL Label,
    IN gctBOOL TrueBranch,
    IN gcsSOURCE * Source
    );

gceSTATUS
clEmitCompareBranchCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN cleCONDITION Condition,
    IN gctLABEL Label,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    );

gceSTATUS
clEmitCompareSetCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN gcsTARGET *Target,
    IN gcsSOURCE * Cond,
    IN gcsSOURCE * Source0,
    IN gcsSOURCE * Source1
    );

gceSTATUS
clBeginMainFunction(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo
    );

gceSTATUS
clEndMainFunction(
    IN cloCOMPILER Compiler
    );

gceSTATUS
clNewFunction(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctCONST_STRING Name,
    OUT gcFUNCTION * Function
    );

gceSTATUS
clNewFunctionArgument(
    IN cloCOMPILER Compiler,
    IN gcFUNCTION Function,
    IN gcVARIABLE Variable,
    IN clsGEN_CODE_DATA_TYPE DataType,
    IN gctSIZE_T Length,
    IN gctREG_INDEX TempRegIndex,
    IN gctUINT8 Qualifier
    );

gceSTATUS
clBeginFunction(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcFUNCTION Function
    );

gceSTATUS
clEndFunction(
    IN cloCOMPILER Compiler,
    IN gcFUNCTION Function
    );

gceSTATUS
clNewKernelFunction(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctCONST_STRING Name,
    OUT gcKERNEL_FUNCTION * Function
    );

gceSTATUS
clNewKernelFunctionArgument(
    IN cloCOMPILER Compiler,
    IN gcKERNEL_FUNCTION Function,
    IN gcVARIABLE Variable,
    IN clsGEN_CODE_DATA_TYPE DataType,
    IN gctSIZE_T Length,
    IN gctREG_INDEX TempRegIndex,
    IN gctUINT8 Qualifier
    );

gceSTATUS
clBeginKernelFunction(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gcKERNEL_FUNCTION Function
    );

gceSTATUS
clEndKernelFunction(
    IN cloCOMPILER Compiler,
    IN clsNAME *FuncName
    );

gceSTATUS
clGetKernelFunctionLabel(
    IN cloCOMPILER Compiler,
    IN gcKERNEL_FUNCTION Function,
    OUT gctLABEL * Label
    );

gceSTATUS
clGetFunctionLabel(
    IN cloCOMPILER Compiler,
    IN gcFUNCTION Function,
    OUT gctLABEL * Label
    );

gceSTATUS
cloCODE_EMITTER_Construct(
    IN cloCOMPILER Compiler,
    OUT cloCODE_EMITTER * CodeEmitter
    );

gceSTATUS
cloCODE_EMITTER_Destroy(
    IN cloCOMPILER Compiler,
    IN cloCODE_EMITTER CodeEmitter
    );

#endif /* __gc_cl_emit_code_h_ */
