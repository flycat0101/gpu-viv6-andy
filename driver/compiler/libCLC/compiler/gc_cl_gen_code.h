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


#ifndef __gc_cl_gen_code_h_
#define __gc_cl_gen_code_h_

#include "gc_cl_ir.h"
#define _GEN_PACKED_LOAD_STORE_AS_BUILTIN 1
#define _GEN_IMAGE_OR_SAMPLER_PARAMETER_VARIABLES 1

#define cldHandleHighPrecisionInFrontEnd gcvFALSE
#define cldSupportMultiKernelFunction  1
#define cldNoMain 1
#define cldNoInlineVectorToScalar 1
#define cldVector8And16 1
#define cldNeedFloatingPointAccuracy gcvFALSE
#define cldNeedFpRoundToNearest gcvFALSE
#define cldUseSTORE1 1

#define cldNumMemoryAddressRegs _gcdOCL_NumMemoryAddressRegs /*number of memory address registers */
#define cldPrivateMemoryAddressRegIndex _gcdOCL_PrivateMemoryAddressRegIndex  /*private memory address register index */
#define cldLocalMemoryAddressRegIndex _gcdOCL_LocalMemoryAddressRegIndex  /*local memory address register index for the local variables within kernel function */
#define cldParmLocalMemoryAddressRegIndex _gcdOCL_ParmLocalMemoryAddressRegIndex  /*local memory address register index for the local parameters */
#define cldConstantMemoryAddressRegIndex _gcdOCL_ConstantMemoryAddressRegIndex  /*constant memory address register index */
#define cldPrintfStartMemoryAddressRegIndex _gcdOCL_PrintfStartMemoryAddressRegIndex  /*printf start memory address register index */
#define cldPrintfEndMemoryAddressRegIndex   _gcdOCL_PrintfEndMemoryAddressRegIndex  /*printf end memory address register index */
#define cldPreScaleGlobalIdRegIndex _gcdOCL_PreScaleGlobalIdRegIndex
#define cldMaxLocalTempRegs _gcdOCL_MaxLocalTempRegs /* maximum number of local temp register */

gctUINT
clGetOperandCountForRegAlloc(
IN clsDECL * Decl
);

gctUINT
clGetOperandCountForRegAllocByName(
IN clsNAME * Name
);

/* Maximum operand count for a variable to stay in register */
#define _clmMaxOperandCountToUseMemory(Decl)  (((!_GEN_UNIFORMS_FOR_CONSTANT_ADDRESS_SPACE_VARIABLES && (Decl)->dataType->addrSpaceQualifier == clvQUALIFIER_CONSTANT) || !gcmOPT_CLUseVIRCodeGen()) \
                                               ? 8U : 16U)

#define _clmCheckVariableForMemory(Name) \
  (!clmDECL_IsPointerType(&((Name)->decl)) && \
   ((Name)->decl.dataType->addrSpaceQualifier == clvQUALIFIER_LOCAL || \
    (!gcmOPT_oclPassKernelStructArgByValue() && (Name)->decl.dataType->addrSpaceQualifier == clvQUALIFIER_GLOBAL) || \
    (((Name)->type == clvVARIABLE_NAME || (Name)->type == clvPARAMETER_NAME) && \
      (Name)->u.variableInfo.isAddressed)))

#define clmDECL_IsAggregateTypeOverRegLimit(Decl) \
   (clmDECL_IsAggregateType(Decl) && clGetOperandCountForRegAlloc(Decl) > _clmMaxOperandCountToUseMemory(Decl))

#define clmGEN_CODE_checkVariableForMemory(Name) \
  (_clmCheckVariableForMemory(Name) || \
   ((Name)->type == clvFUNC_NAME && clmDECL_IsAggregateTypeOverRegLimit(&((Name)->decl))))

#define clmIsVariableInConstantAddressSpaceMemory(Name) \
        ((Name)->decl.dataType->addrSpaceQualifier == clvQUALIFIER_CONSTANT && \
         clmGEN_CODE_checkVariableForMemory(Name) && \
         gcmOPT_CreateDefaultUBO())

typedef    gctUINT32        gctREG_INDEX;

typedef struct _clsGEN_CODE_DATA_TYPE
{
  cltELEMENT_TYPE elementType;
  clsMATRIX_SIZE  matrixSize;
}
clsGEN_CODE_DATA_TYPE;

#define clmGEN_CODE_elementType_GET(d) (d).elementType
#define clmGEN_CODE_elementType_SET(d, t) (d).elementType = (t)
#define clmGEN_CODE_vectorSize_GET(d) \
 ((d).matrixSize.columnCount? (gctUINT)0 : (d).matrixSize.rowCount)

#define clmGEN_CODE_vectorSize_NOCHECK_GET(d) ((d).matrixSize.rowCount)

#define clmGEN_CODE_vectorSize_SET(d, s) do \
        { (d).matrixSize.rowCount = s; \
       (d).matrixSize.columnCount = 0; \
        } \
        while (gcvFALSE)

#define clmGEN_CODE_scalarDataType_SET(d) do \
        { (d).matrixSize.rowCount = 0; \
       (d).matrixSize.columnCount = 0; \
        } \
        while (gcvFALSE)

#define clmGEN_CODE_matrixSize_GET(d) ((d).matrixSize.columnCount)
#define clmGEN_CODE_matrixRowCount_GET(d) ((d).matrixSize.rowCount)
#define clmGEN_CODE_matrixColumnCount_GET(d) ((d).matrixSize.columnCount)
#define clmGEN_CODE_matrixSize_SET(d, r, c) do \
        { (d).matrixSize.rowCount = (r); \
       (d).matrixSize.columnCount = (c); \
        } \
        while (gcvFALSE)

#define clmGEN_CODE_matrixRowCount_SET(d, c) (d).matrixSize.rowCount = (c)
#define clmGEN_CODE_matrixColumnCount_SET(d, c) (d).matrixSize.columnCount = (c)

#define clmGEN_CODE_IsScalarDataType(d) \
 ((clmGEN_CODE_matrixRowCount_GET(d) == 0  && clmGEN_CODE_matrixColumnCount_GET(d) == 0) && \
  !clmIsElementTypePackedGenType((d).elementType))

#define clmGEN_CODE_IsVectorDataType(d) ((clmGEN_CODE_vectorSize_GET(d) !=0) || clmIsElementTypePackedGenType((d).elementType))

#define clmGEN_CODE_IsGenTypeDataType(d)  clmIsElementTypeGenType((d).elementType)

#define clmGEN_CODE_IsExtendedVectorType(d) \
  (clmIsElementTypePacked((d).elementType) || \
   (clmGEN_CODE_IsVectorDataType(d) && \
    clmGEN_CODE_vectorSize_NOCHECK_GET(d) > cldBASIC_VECTOR_SIZE))

#define clmGEN_CODE_IsMatrixDataType(d) \
    ((d).matrixSize.rowCount != 0  && (d).matrixSize.columnCount != 0)

#define clmGEN_CODE_IsSamplerDataType(d) \
    (clmGEN_CODE_elementType_GET(d) == clvTYPE_SAMPLER2D || \
     clmGEN_CODE_elementType_GET(d) == clvTYPE_SAMPLER3D)

#define clmGEN_CODE_IsHighPrecisionDataType(d) \
    (cldHandleHighPrecisionInFrontEnd && \
     clmIsElementTypeHighPrecision(clmGEN_CODE_elementType_GET(d)))

#define clmGEN_CODE_DATA_TYPE_Initialize(d, r, c, t)  do \
  { clmGEN_CODE_matrixSize_SET((d), (r), (c)); \
    clmGEN_CODE_elementType_SET((d), (t)); \
  } while (gcvFALSE)

#define _clmGetTempRegIndexOffset(SectionIndex, ElementType) \
    ((clmIsElementTypeHighPrecision(ElementType) || clmIsElementTypeImage(ElementType)) \
     ? (SectionIndex) << 1 : (SectionIndex))

#define clmROPERAND_vectorComponent_GET(component, vector, idx) \
  clGetVectorROperandSlice(vector, idx, 1, component)

#define clmLOPERAND_vectorComponent_GET(component, vector, idx) \
  clGetVectorLOperandSlice(vector, idx, 1, component)

#define clmIOPERAND_vectorComponent_GET(component, vector, idx) \
  clGetVectorIOperandSlice(vector, idx, 1, component)


clsCOMPONENT_SELECTION
clGetComponentSelectionSlice(
    IN clsCOMPONENT_SELECTION ComponentSelection,
    IN gctUINT8 StartComponent,
    IN gctUINT8 SliceComponentCount
    );

clsCOMPONENT_SELECTION
clGetDefaultComponentSelection(
    IN cloCOMPILER Compiler,
    IN clsGEN_CODE_DATA_TYPE DataType
    );

gctBOOL
clIsDefaultComponentSelection(
IN clsCOMPONENT_SELECTION  *ComponentSelection
);

gceSTATUS
clGetStartComponentDefaultComponentSelection(
    IN gctUINT8 StartComponent,
    IN OUT clsCOMPONENT_SELECTION *ComponentSelection
    );

clsGEN_CODE_DATA_TYPE
clGetSubsetDataType(
IN clsGEN_CODE_DATA_TYPE Base,
IN gctUINT8 NumComponents
);

/* OPCODE */
typedef enum _cleOPCODE
{
    clvOPCODE_INVALID  = 0,
    clvOPCODE_NOP,

    /* Assignment Operation */
    clvOPCODE_ASSIGN,

    /* Following CONV opcodes need to be defined consecutively.
       No other non- CONV opcode can be inserted in between.
       If first and last CONV code is changed, the macro
       cldFirstConvOpcode and/or cldLastConvOpcode needs to be updated.
    */
    clvOPCODE_CONV,
    clvOPCODE_CONV_RTE,
    clvOPCODE_CONV_RTZ,
    clvOPCODE_CONV_RTN,
    clvOPCODE_CONV_RTP,
    clvOPCODE_CONV_SAT,
    clvOPCODE_CONV_SAT_RTE,
    clvOPCODE_CONV_SAT_RTZ,
    clvOPCODE_CONV_SAT_RTN,
    clvOPCODE_CONV_SAT_RTP,

    /* Arithmetic Operations */
    clvOPCODE_ADD,
    clvOPCODE_SUB,
    clvOPCODE_MUL,
    clvOPCODE_MUL_Z,
    clvOPCODE_FADD,
    clvOPCODE_FSUB,
    clvOPCODE_FMUL,
    clvOPCODE_DIV,
    clvOPCODE_IDIV,
    clvOPCODE_IMUL,
    clvOPCODE_MOD,
    clvOPCODE_FMOD,
    clvOPCODE_SELECT,
    clvOPCODE_FMA,
    clvOPCODE_TEXTURE_LOAD,
    clvOPCODE_IMAGE_SAMPLER,
    clvOPCODE_IMAGE_READ,
    clvOPCODE_IMAGE_READ_3D,
    clvOPCODE_IMAGE_WRITE,
    clvOPCODE_IMAGE_WRITE_3D,
    clvOPCODE_CLAMP0MAX,
    clvOPCODE_CLAMPCOORD,
    clvOPCODE_DP2,
    clvOPCODE_DP3,
    clvOPCODE_DP4,

    clvOPCODE_TEXU,

    /* Conversion Operations */
    clvOPCODE_FLOAT_TO_INT,
    clvOPCODE_FLOAT_TO_UINT,
    clvOPCODE_FLOAT_TO_BOOL,
    clvOPCODE_INT_TO_INT,
    clvOPCODE_INT_TO_UINT,
    clvOPCODE_INT_TO_BOOL,
    clvOPCODE_INT_TO_FLOAT,
    clvOPCODE_UINT_TO_UINT,
    clvOPCODE_UINT_TO_INT,
    clvOPCODE_UINT_TO_BOOL,
    clvOPCODE_UINT_TO_FLOAT,
    clvOPCODE_BOOL_TO_FLOAT,
    clvOPCODE_BOOL_TO_INT,
    clvOPCODE_BOOL_TO_UINT,

    clvOPCODE_IMPL_B2F,
    clvOPCODE_IMPL_U2F,
    clvOPCODE_IMPL_I2F,

    /* Other Calculation Operations */
    clvOPCODE_INVERSE,

    clvOPCODE_LESS_THAN,
    clvOPCODE_LESS_THAN_EQUAL,
    clvOPCODE_GREATER_THAN,
    clvOPCODE_GREATER_THAN_EQUAL,
    clvOPCODE_EQUAL,
    clvOPCODE_NOT_EQUAL,

    clvOPCODE_AND_BITWISE,
    clvOPCODE_OR_BITWISE,
    clvOPCODE_XOR_BITWISE,
    clvOPCODE_NOT_BITWISE,

    clvOPCODE_RSHIFT,
    clvOPCODE_LSHIFT,
    clvOPCODE_RIGHT_SHIFT,
    clvOPCODE_LEFT_SHIFT,

    clvOPCODE_ADDR,
    clvOPCODE_INDIRECTION,
    clvOPCODE_NON_LVAL,

    clvOPCODE_BARRIER,
    clvOPCODE_MEM_FENCE,
    clvOPCODE_LOAD,
    clvOPCODE_STORE,
    clvOPCODE_STORE1,
    clvOPCODE_STORE1_RTE,
    clvOPCODE_STORE1_RTZ,
    clvOPCODE_STORE1_RTP,
    clvOPCODE_STORE1_RTN,

    clvOPCODE_ANY,
    clvOPCODE_ALL,
    clvOPCODE_NOT,

    clvOPCODE_SIN,
    clvOPCODE_COS,
    clvOPCODE_TAN,

    clvOPCODE_ASIN,
    clvOPCODE_ACOS,
    clvOPCODE_ATAN,
    clvOPCODE_ATAN2,

    clvOPCODE_SINPI,
    clvOPCODE_COSPI,
    clvOPCODE_TANPI,

    clvOPCODE_ARCTRIG0,
    clvOPCODE_ARCTRIG1,

    clvOPCODE_POW,
    clvOPCODE_EXP2,
    clvOPCODE_LOG2,
    clvOPCODE_SQRT,
    clvOPCODE_INVERSE_SQRT,

    clvOPCODE_MULLO,
    clvOPCODE_ADDLO,

    clvOPCODE_ROTATE,
    clvOPCODE_LEADZERO,
    clvOPCODE_GETEXP,
    clvOPCODE_GETMANT,

    /*Integer only, get the overflow part*/
    clvOPCODE_MULHI,

    clvOPCODE_SET,
    clvOPCODE_CMP,

    clvOPCODE_ABS,
    clvOPCODE_SIGN,
    clvOPCODE_FLOOR,
    clvOPCODE_CEIL,
    clvOPCODE_FRACT,
    clvOPCODE_MIN,
    clvOPCODE_MAX,
    clvOPCODE_SATURATE,
    clvOPCODE_STEP,
    clvOPCODE_DOT,
    clvOPCODE_CROSS,
    clvOPCODE_NORMALIZE,

    clvOPCODE_POPCOUNT,

    /* Branch Operations */
    clvOPCODE_JUMP,
    clvOPCODE_CALL,
    clvOPCODE_RETURN,

    /* Derivative Operations */
    clvOPCODE_DFDX,
    clvOPCODE_DFDY,
    clvOPCODE_FWIDTH,
    clvOPCODE_SUBSAT,
    clvOPCODE_ADDSAT,
    clvOPCODE_MULSAT,

    clvOPCODE_ATOMADD,
    clvOPCODE_ATOMSUB,
    clvOPCODE_ATOMXCHG,
    clvOPCODE_ATOMCMPXCHG,
    clvOPCODE_ATOMMIN,
    clvOPCODE_ATOMMAX,
    clvOPCODE_ATOMOR,
    clvOPCODE_ATOMAND,
    clvOPCODE_ATOMXOR,

    clvOPCODE_ADD_RTZ,
    clvOPCODE_ADD_RTNE,
    clvOPCODE_ADDLO_RTZ,
    clvOPCODE_ADDLO_RTNE,
    clvOPCODE_SUB_RTZ,
    clvOPCODE_SUB_RTNE,
    clvOPCODE_MUL_RTZ,
    clvOPCODE_MUL_RTNE,
    clvOPCODE_MULLO_RTZ,
    clvOPCODE_MULLO_RTNE,
    clvOPCODE_FRACT_RTZ,
    clvOPCODE_FRACT_RTNE,
    clvOPCODE_INT_TO_FLOAT_RTZ,
    clvOPCODE_INT_TO_FLOAT_RTNE,
    clvOPCODE_UINT_TO_FLOAT_RTZ,
    clvOPCODE_UINT_TO_FLOAT_RTNE,

    clvOPCODE_UNPACK,
    clvOPCODE_ASTYPE,
    clvOPCODE_NEG,
    clvOPCODE_LONGLO,
    clvOPCODE_LONGHI,
    clvOPCODE_MOV_LONG,
    clvOPCODE_MADSAT,
    clvOPCODE_COPY,
    clvOPCODE_PARAM_CHAIN,
    clvOPCODE_INTRINSIC,
    clvOPCODE_INTRINSIC_ST,

    clvOPCODE_FMA_MUL,
    clvOPCODE_FMA_ADD,
    clvOPCODE_CMAD,
    clvOPCODE_CONJ,
    clvOPCODE_CMUL,
    clvOPCODE_CMADCJ,
    clvOPCODE_CMULCJ,
    clvOPCODE_CADDCJ,
    clvOPCODE_CSUBCJ,

    clvOPCODE_GET_IMAGE_TYPE,

    clvOPCODE_FINDLSB,
    clvOPCODE_FINDMSB,

    clvOPCODE_BIT_REVERSAL,
    clvOPCODE_BYTE_REVERSAL,

    clvOPCODE_MAXOPCODE    /* All new opcode should be inserted before this */
}
cleOPCODE;

/* VIR intrinsics kind */
#define VIR_INTRINSIC_INFO(Intrinsic)   CL_VIR_IK_##Intrinsic
typedef enum _clvVIR_IK
{
#include "vir/ir/gc_vsc_vir_intrinsic_kind.def.h"
} clvVIR_IK;
#undef VIR_INTRINSIC_INFO

#define cldFirstConvOpcode  clvOPCODE_CONV
#define cldLastConvOpcode   clvOPCODE_CONV_SAT_RTP
#define clmIsOpcodeConv(Opcode) \
    (((Opcode) >= cldFirstConvOpcode && (Opcode) <= cldLastConvOpcode) || \
     (Opcode) == clvOPCODE_ASTYPE)

gctCONST_STRING
clGetOpcodeName(
    IN cleOPCODE Opcode
    );

/* CONDITION */
typedef enum _cleCONDITION
{
    clvCONDITION_INVALID,

    clvCONDITION_EQUAL,
    clvCONDITION_NOT_EQUAL,
    clvCONDITION_LESS_THAN,
    clvCONDITION_LESS_THAN_EQUAL,
    clvCONDITION_GREATER_THAN,
    clvCONDITION_GREATER_THAN_EQUAL,
    clvCONDITION_XOR,
    clvCONDITION_AND,
    clvCONDITION_OR,
    clvCONDITION_NOT_ZERO,
    clvCONDITION_ZERO
}
cleCONDITION;

gctCONST_STRING
clGetConditionName(
    IN cleCONDITION Condition
    );

cleCONDITION
clGetNotCondition(
    IN cleCONDITION Condition
    );

/* clsLOPERAND and clsROPERAND */
typedef enum _cleINDEX_MODE
{
    clvINDEX_NONE,
    clvINDEX_REG,
    clvINDEX_CONSTANT
}
cleINDEX_MODE;

typedef struct _clsLOGICAL_REG
{
    cltQUALIFIER    qualifier;
    clsGEN_CODE_DATA_TYPE    dataType;
    gctBOOL        isUnionMember;

    union {
        gcATTRIBUTE    attribute;
        gcUNIFORM    uniform;
    }    u;

    gctREG_INDEX    regIndex;
    clsCOMPONENT_SELECTION    componentSelection;
}
clsLOGICAL_REG;

#define clsLOGICAL_REG_InitializeTemp(compiler, reg, _qualifier, _dataType, _regIndex, _isUnionMember) \
    do \
    { \
        (reg)->qualifier        = (_qualifier); \
        (reg)->dataType            = (_dataType); \
        (reg)->isUnionMember        = (_isUnionMember); \
        (reg)->regIndex            = (_regIndex); \
        (reg)->componentSelection    = clGetDefaultComponentSelection((compiler), (_dataType)); \
    } \
    while (gcvFALSE)

/* Note that the following macro does not have wrapping of do{}while(gcvFALSE)
   This is done to avoid the preprocessor on Android not being able to handle the nesting */
#define clsLOGICAL_REG_InitializeTempComponentSelection(reg, _qualifier, _dataType, _regIndex, _isUnionMember, _cs) \
        (reg)->qualifier        = (_qualifier); \
        (reg)->dataType            = (_dataType); \
        (reg)->isUnionMember        = (_isUnionMember); \
        (reg)->regIndex            = (_regIndex); \
        (reg)->componentSelection    = (_cs)

#define clsLOGICAL_REG_InitializeAttribute(compiler, reg, _qualifier, _dataType,  _attribute, _regIndex, _isUnionMember) \
    do \
    { \
        (reg)->qualifier        = (_qualifier); \
        (reg)->dataType            = (_dataType); \
        (reg)->isUnionMember        = (_isUnionMember); \
        (reg)->u.attribute        = (_attribute); \
        (reg)->regIndex            = (_regIndex); \
        (reg)->componentSelection    = clGetDefaultComponentSelection((compiler), (_dataType)); \
    } \
    while (gcvFALSE)

#define clsLOGICAL_REG_InitializeUniform(compiler, reg, _qualifier, _dataType, _uniform, _regIndex, _isUnionMember) \
    do \
    { \
        (reg)->qualifier        = (_qualifier); \
        (reg)->dataType            = (_dataType); \
        (reg)->isUnionMember        = (_isUnionMember); \
        (reg)->u.uniform        = (_uniform); \
        (reg)->regIndex            = (_regIndex); \
        (reg)->componentSelection    = clGetDefaultComponentSelection((compiler), (_dataType)); \
    } \
    while (gcvFALSE)

#define clmGEN_CODE_IF(Compiler, CodeGenerator, LineNo, StringNo, LOperand, Condition, ROperand)  \
    { clsSELECTION_CONTEXT SelectionContext[1]; \
        do { \
           status = clDefineSelectionBegin(Compiler, \
                       CodeGenerator, \
                       gcvFALSE, \
                       SelectionContext); \
           if (gcmIS_ERROR(status)) goto OnError; \
           status = clGenSelectionCompareConditionCode(Compiler, \
                                                       CodeGenerator, \
                                                       SelectionContext, \
                                                       LineNo, \
                                                       StringNo, \
                                                       Condition, \
                                                       LOperand, \
                                                       ROperand); \
           if (gcmIS_ERROR(status)) goto OnError; \
       status = clDefineSelectionTrueOperandBegin(Compiler, \
                                                      CodeGenerator, \
                                                      SelectionContext); \
           if (gcmIS_ERROR(status)) goto OnError; \
        }  while (gcvFALSE)

#define clmGEN_CODE_IF_NEXT_COND(Compiler, CodeGenerator, LineNo, StringNo, LOperand, Condition, ROperand)  \
        do { \
       if(SelectionContext->isNegativeCond == gcvTRUE) { \
          status = clSetLabel(Compiler, \
                  0, \
                  0, \
                  SelectionContext->endLabel); \
              if (gcmIS_ERROR(status)) goto OnError; \
              SelectionContext->endLabel = clNewLabel(Compiler); \
          SelectionContext->isNegativeCond = gcvFALSE; \
       } \
           status = clGenSelectionCompareConditionCode(Compiler, \
                                                       CodeGenerator, \
                                                       SelectionContext, \
                                                       LineNo, \
                                                       StringNo, \
                                                       Condition, \
                                                       LOperand, \
                                                       ROperand); \
           if (gcmIS_ERROR(status)) goto OnError; \
        }  while (gcvFALSE)

#define clmGEN_CODE_NOT_IF(Compiler, CodeGenerator, LineNo, StringNo, LOperand, Condition, ROperand)  \
    { clsSELECTION_CONTEXT SelectionContext[1]; \
        do { \
           status = clDefineSelectionBegin(Compiler, \
                       CodeGenerator, \
                       gcvFALSE, \
                       SelectionContext); \
           if (gcmIS_ERROR(status)) goto OnError; \
       SelectionContext->isNegativeCond = gcvTRUE; \
           status = clGenSelectionCompareConditionCode(Compiler, \
                                                       CodeGenerator, \
                                                       SelectionContext, \
                                                       LineNo, \
                                                       StringNo, \
                                                       clGetNotCondition((Condition)), \
                                                       LOperand, \
                                                       ROperand); \
           if (gcmIS_ERROR(status)) goto OnError; \
       status = clDefineSelectionTrueOperandBegin(Compiler, \
                                                      CodeGenerator, \
                                                      SelectionContext); \
           if (gcmIS_ERROR(status)) goto OnError; \
        }  while (gcvFALSE)

#define clmGEN_CODE_NOT_IF_NEXT_COND(Compiler, CodeGenerator, LineNo, StringNo, LOperand, Condition, ROperand)  \
        do { \
       if(SelectionContext->isNegativeCond == gcvFALSE) { \
          status = clSetLabel(Compiler, \
                  0, \
                  0, \
                  SelectionContext->endLabel); \
              if (gcmIS_ERROR(status)) goto OnError; \
              SelectionContext->endLabel = clNewLabel(Compiler); \
          SelectionContext->isNegativeCond = gcvTRUE; \
       } \
           status = clGenSelectionCompareConditionCode(Compiler, \
                                                       CodeGenerator, \
                                                       SelectionContext, \
                                                       LineNo, \
                                                       StringNo, \
                                                       clGetNotCondition((Condition)), \
                                                       LOperand, \
                                                       ROperand); \
           if (gcmIS_ERROR(status)) goto OnError; \
        }  while (gcvFALSE)

#define clmGEN_CODE_ELSE(Compiler, CodeGenerator, LineNo, StringNo)  \
        do { \
           SelectionContext->hasFalseOperand = gcvTRUE; \
           SelectionContext->beginLabelOfFalseOperand = SelectionContext->endLabel; \
           SelectionContext->endLabel = clNewLabel(Compiler); \
       status = clDefineSelectionTrueOperandEnd(Compiler, \
                                                LineNo,\
                                                StringNo,\
                                                    CodeGenerator, \
                                                    SelectionContext, \
                                                    gcvFALSE); \
           if (gcmIS_ERROR(status)) goto OnError; \
           status = clDefineSelectionFalseOperandBegin(Compiler, \
                                                       CodeGenerator, \
                                                       SelectionContext); \
           if (gcmIS_ERROR(status)) goto OnError; \
        } while (gcvFALSE)

#define clmGEN_CODE_ENDIF(Compiler, CodeGenerator, LineNo, StringNo)  \
        do { \
           if((SelectionContext)->hasFalseOperand) { \
               status = clDefineSelectionFalseOperandEnd(Compiler, \
                             CodeGenerator, \
                             SelectionContext); \
               if (gcmIS_ERROR(status)) goto OnError; \
           } \
           else { \
           status = clDefineSelectionTrueOperandEnd(Compiler, \
                                                    LineNo,\
                                                    StringNo,\
                                                        CodeGenerator, \
                                                        SelectionContext, \
                                                        gcvFALSE); \
               if (gcmIS_ERROR(status)) goto OnError; \
           } \
           status = clDefineSelectionEnd(Compiler, \
                                         CodeGenerator, \
                                         SelectionContext); \
           if (gcmIS_ERROR(status)) goto OnError; \
        } while (gcvFALSE); \
     }

gceSTATUS
clsNAME_AllocLogicalRegs(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN clsNAME * Name
    );

gceSTATUS
clsNAME_FreeLogicalRegs(
    IN cloCOMPILER Compiler,
    IN clsNAME * Name
    );

typedef struct _clsINDEX
{
    cleINDEX_MODE    mode;
    union {
        gctREG_INDEX    indexRegIndex;
        gctREG_INDEX    constant;
    }    u;
}
clsINDEX;

typedef struct _clsLOPERAND
{
    clsGEN_CODE_DATA_TYPE    dataType;
    clsLOGICAL_REG    reg;
    clsINDEX    arrayIndex;
    clsINDEX    matrixIndex;
    clsINDEX    vectorIndex;
}
clsLOPERAND;

/* Vector Index to machine targeted component */
#define clmVectorIndexToTargetComponent(DataType, VectorIndex) \
     ((gctUINT8)(clmGEN_CODE_IsHighPrecisionDataType((DataType)) ? ((VectorIndex) << 1) : (VectorIndex)))

/* Machine targeted component to vector index */
#define clmTargetComponentToVectorIndex(DataType, Component) \
     ((gctREG_INDEX)(clmGEN_CODE_IsHighPrecisionDataType((DataType)) ? ((Component) >> 1) : (Component)))

#define clsLOPERAND_Initialize(operand, _reg) \
    do { \
        (operand)->dataType        = (_reg)->dataType; \
        (operand)->reg            = *(_reg); \
        (operand)->arrayIndex.mode    = clvINDEX_NONE; \
        (operand)->matrixIndex.mode    = clvINDEX_NONE; \
        (operand)->vectorIndex.mode    = clvINDEX_NONE; \
    } \
    while (gcvFALSE)

#define clsLOPERAND_InitializeTempReg(compiler, operand, _qualifier, _dataType, _regIndex) \
    do  { \
        (operand)->dataType        = (_dataType); \
        clsLOGICAL_REG_InitializeTemp((compiler), &(operand)->reg, (_qualifier), (_dataType), (_regIndex), gcvFALSE); \
        (operand)->arrayIndex.mode    = clvINDEX_NONE; \
        (operand)->matrixIndex.mode    = clvINDEX_NONE; \
        (operand)->vectorIndex.mode    = clvINDEX_NONE; \
    } \
    while (gcvFALSE)

#define clsLOPERAND_InitializeTempRegWithComponentSelection(operand, _qualifier, _dataType, _regDataType, _regIndex, _cs) \
    do  { \
        (operand)->dataType        = (_dataType); \
        clsLOGICAL_REG_InitializeTempComponentSelection(&(operand)->reg, \
                                                                (_qualifier), \
                                                                (_regDataType), \
                                                                (_regIndex), \
                                                                gcvFALSE, \
                                                                (_cs)); \
        (operand)->arrayIndex.mode    = clvINDEX_NONE; \
        (operand)->matrixIndex.mode    = clvINDEX_NONE; \
        (operand)->vectorIndex.mode    = clvINDEX_NONE; \
    } \
    while (gcvFALSE)

#define clsLOPERAND_InitializeAsMatrixColumn(operand, _matrixOperand, _matrixIndex) \
    do { \
        gcmASSERT((_matrixOperand)->matrixIndex.mode == clvINDEX_NONE); \
        \
        *(operand)                = *(_matrixOperand); \
        (operand)->dataType            = \
                            gcGetMatrixColumnDataType((_matrixOperand)->dataType); \
        (operand)->matrixIndex.mode        = clvINDEX_CONSTANT; \
        (operand)->matrixIndex.u.constant    = (gctREG_INDEX) (_matrixIndex); \
    } \
    while (gcvFALSE)

#define clsLOPERAND_InitializeAsMatrixComponent(operand, _matrixOperand,  _matrixIndex, _vectorIndex) \
    do { \
        gcmASSERT((_matrixOperand)->matrixIndex.mode == clvINDEX_NONE); \
        gcmASSERT((_matrixOperand)->vectorIndex.mode == clvINDEX_NONE); \
        *(operand)                = *(_matrixOperand); \
        (operand)->dataType            = \
                            gcGetComponentDataType((_matrixOperand)->dataType); \
        (operand)->matrixIndex.mode        = clvINDEX_CONSTANT; \
        (operand)->matrixIndex.u.constant    = (gctREG_INDEX) (_matrixIndex); \
        (operand)->vectorIndex.mode        = clvINDEX_CONSTANT; \
        (operand)->vectorIndex.u.constant    = (gctREG_INDEX) (_vectorIndex); \
    } \
    while (gcvFALSE)

#define clsLOPERAND_InitializeAsVectorComponent(operand, _vectorOperand, _vectorIndex) \
    do { \
        gcmASSERT((_vectorOperand)->vectorIndex.mode == clvINDEX_NONE); \
        *(operand)                = *(_vectorOperand); \
        (operand)->dataType            = \
                            gcGetVectorComponentDataType((_vectorOperand)->dataType); \
        (operand)->vectorIndex.mode        = clvINDEX_CONSTANT; \
        (operand)->vectorIndex.u.constant    = (gctREG_INDEX) (_vectorIndex); \
    } \
    while (gcvFALSE)

#define clsLOPERAND_InitializeUsingIOperand(operand, iOperand) \
    do { \
        clsLOPERAND_InitializeTempRegWithComponentSelection((operand), \
                                                            clvQUALIFIER_NONE, \
                                                            (iOperand)->dataType, \
                                                            (iOperand)->regDataType, \
                                                            (iOperand)->tempRegIndex, \
                                                            (iOperand)->componentSelection); \
        if(clmGEN_CODE_IsScalarDataType((iOperand)->dataType) && \
            clmGEN_CODE_IsVectorDataType((iOperand)->regDataType)) { \
            (operand)->vectorIndex.mode = clvINDEX_CONSTANT; \
            (operand)->vectorIndex.u.constant = clmTargetComponentToVectorIndex((iOperand)->dataType, \
                                                                                (iOperand)->componentSelection.selection[0]); \
        } \
    } while (gcvFALSE)

#define clsLOPERAND_InitializeUsingROperand(operand, rOperand) \
        do { \
          (operand)->dataType = (rOperand)->dataType; \
          gcmASSERT((rOperand)->isReg); \
          (operand)->reg = (rOperand)->u.reg; \
          (operand)->arrayIndex = (rOperand)->arrayIndex; \
          (operand)->matrixIndex = (rOperand)->matrixIndex; \
          (operand)->vectorIndex = (rOperand)->vectorIndex; \
        } while (gcvFALSE)

typedef struct _clsOPERAND_CONSTANT
{
    clsGEN_CODE_DATA_TYPE dataType;
    gctUINT               valueCount;
    cluCONSTANT_VALUE     values[cldMAX_VECTOR_COMPONENT * cldMAX_VECTOR_COMPONENT];
    gctBOOL allValuesEqual;
}
clsOPERAND_CONSTANT;

typedef struct _clsROPERAND
{
    clsGEN_CODE_DATA_TYPE    dataType;
    gctBOOL        isReg;
    union {
        clsLOGICAL_REG    reg;
        clsOPERAND_CONSTANT    constant;
    }u;
    clsINDEX   arrayIndex;
    clsINDEX   matrixIndex;
    clsINDEX   vectorIndex;
}
clsROPERAND;

gctINT
clGetIntegerValue(
IN clsROPERAND *Operand
);

gctBOOL
clIsIntegerZero(
IN clsROPERAND *Operand
);

gceSTATUS
clUpdateAddressOffset(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctINT Incr,
IN OUT clsROPERAND *Offset,
OUT clsROPERAND *NewOffset
);

void
clGetVectorROperandSlice(
    IN clsROPERAND * ROperand,
    IN gctUINT8 StartComponent,
    IN gctUINT8 SliceComponentCoun,
    OUT clsROPERAND * ROperandSlice
    );

void
clGetVectorLOperandSlice(
    IN clsLOPERAND * LOperand,
    IN gctUINT8 StartComponent,
    IN gctUINT8 SliceComponentCount,
    OUT clsLOPERAND * LOperandSlice
    );

gctBOOL
clsROPERAND_CONSTANT_IsAllVectorComponentsEqual(
    IN clsROPERAND * ROperand
    );

#define clsROPERAND_InitializeReg(operand, _reg) \
    do { \
        (operand)->dataType        = (_reg)->dataType; \
        (operand)->isReg        = gcvTRUE; \
        (operand)->u.reg        = *(_reg); \
        (operand)->arrayIndex.mode    = clvINDEX_NONE; \
        (operand)->matrixIndex.mode    = clvINDEX_NONE; \
        (operand)->vectorIndex.mode    = clvINDEX_NONE; \
    } \
    while (gcvFALSE)

#define clsROPERAND_InitializeTempReg(compiler, operand, _qualifier, _dataType, _regIndex) \
    do { \
        (operand)->dataType        = (_dataType); \
        (operand)->isReg        = gcvTRUE; \
        clsLOGICAL_REG_InitializeTemp((compiler), &(operand)->u.reg, (_qualifier), (_dataType), (_regIndex), gcvFALSE); \
        (operand)->arrayIndex.mode    = clvINDEX_NONE; \
        (operand)->matrixIndex.mode    = clvINDEX_NONE; \
        (operand)->vectorIndex.mode    = clvINDEX_NONE; \
    } \
    while (gcvFALSE)

#define clsROPERAND_InitializeTempRegWithComponentSelection(operand, _qualifier, _dataType, _regDataType, _regIndex, _cs) \
    do  { \
        (operand)->dataType        = (_dataType); \
        (operand)->isReg        = gcvTRUE; \
        clsLOGICAL_REG_InitializeTempComponentSelection(&(operand)->u.reg, \
                                                                (_qualifier), \
                                                                (_regDataType), \
                                                                (_regIndex), \
                                                                gcvFALSE, \
                                                                (_cs)); \
        (operand)->arrayIndex.mode    = clvINDEX_NONE; \
        (operand)->matrixIndex.mode    = clvINDEX_NONE; \
        (operand)->vectorIndex.mode    = clvINDEX_NONE; \
    } \
    while (gcvFALSE)

#define clsROPERAND_InitializeUsingIOperand(operand, iOperand) \
    do { \
       clsROPERAND_InitializeTempRegWithComponentSelection((operand), \
                                                           clvQUALIFIER_NONE, \
                                                           (iOperand)->dataType, \
                                                           (iOperand)->regDataType, \
                                                           (iOperand)->tempRegIndex, \
                                                           (iOperand)->componentSelection); \
       if(clmGEN_CODE_IsScalarDataType((iOperand)->dataType) && \
           clmGEN_CODE_IsVectorDataType((iOperand)->regDataType)) { \
              (operand)->vectorIndex.mode = clvINDEX_CONSTANT; \
              (operand)->vectorIndex.u.constant = clmTargetComponentToVectorIndex((iOperand)->dataType, \
                                                                                  (iOperand)->componentSelection.selection[0]); \
       } \
    } while (gcvFALSE)

#define clsROPERAND_InitializeUsingLOperand(operand, lOperand) \
    clsROPERAND_InitializeReg((operand), &((lOperand)->reg))

#define clsROPERAND_InitializeConstant(operand, _dataType, _valueCount, _values) \
    do { \
        gctUINT _i; \
        (operand)->dataType        = (_dataType); \
        (operand)->isReg        = gcvFALSE; \
        (operand)->u.constant.dataType    = (_dataType); \
        (operand)->u.constant.valueCount= (_valueCount); \
        for (_i = 0; _i < ((operand)->u.constant.valueCount); _i++) { \
            (operand)->u.constant.values[_i] = (_values)[_i]; \
        } \
        (operand)->arrayIndex.mode    = clvINDEX_NONE; \
        (operand)->matrixIndex.mode    = clvINDEX_NONE; \
        (operand)->vectorIndex.mode    = clvINDEX_NONE; \
        (operand)->u.constant.allValuesEqual = clsROPERAND_CONSTANT_IsAllVectorComponentsEqual(operand); \
    } \
    while (gcvFALSE)

#define clsROPERAND_InitializeScalarConstant(operand, _dataType, _typeField, _val) \
    do { \
       cluCONSTANT_VALUE constant[1]; \
       constant-> _typeField## Value = (_val); \
       clsROPERAND_InitializeConstant(operand, _dataType, 1, constant); \
    } while (gcvFALSE)

#define clsROPERAND_InitializeFloatOrVecOrMatConstant(operand, _dataType, _floatValue) \
    do { \
        gctUINT _i; \
        (operand)->dataType    = (_dataType); \
        (operand)->isReg    = gcvFALSE; \
        (operand)->u.constant.dataType    = (_dataType); \
        (operand)->u.constant.valueCount= gcGetDataTypeComponentCount(_dataType); \
        for (_i = 0; _i < (operand)->u.constant.valueCount; _i++)  { \
            (operand)->u.constant.values[_i].floatValue    = (_floatValue); \
        } \
        (operand)->arrayIndex.mode    = clvINDEX_NONE; \
        (operand)->matrixIndex.mode    = clvINDEX_NONE; \
        (operand)->vectorIndex.mode    = clvINDEX_NONE; \
        (operand)->u.constant.allValuesEqual = clsROPERAND_CONSTANT_IsAllVectorComponentsEqual(operand); \
    } \
    while (gcvFALSE)

#define clsROPERAND_InitializeIntOrIVecConstant(operand, _dataType, _intValue) \
    do { \
        gctUINT _i; \
        (operand)->dataType    = (_dataType); \
        (operand)->isReg    = gcvFALSE; \
        (operand)->u.constant.dataType    = (_dataType); \
        (operand)->u.constant.valueCount= gcGetDataTypeComponentCount(_dataType); \
        for (_i = 0; _i < (operand)->u.constant.valueCount; _i++) { \
            (operand)->u.constant.values[_i].intValue    = (_intValue); \
        } \
        (operand)->arrayIndex.mode    = clvINDEX_NONE; \
        (operand)->matrixIndex.mode    = clvINDEX_NONE; \
        (operand)->vectorIndex.mode    = clvINDEX_NONE; \
        (operand)->u.constant.allValuesEqual = clsROPERAND_CONSTANT_IsAllVectorComponentsEqual(operand); \
    } \
    while (gcvFALSE)

#define clsROPERAND_InitializeUintConstant(operand, _dataType, _uintValue) \
    do { \
        gctUINT _i; \
        (operand)->dataType    = (_dataType); \
        (operand)->isReg    = gcvFALSE; \
        (operand)->u.constant.dataType    = (_dataType); \
        (operand)->u.constant.valueCount= gcGetDataTypeComponentCount(_dataType); \
        for (_i = 0; _i < (operand)->u.constant.valueCount; _i++) { \
            (operand)->u.constant.values[_i].uintValue    = (_uintValue); \
        } \
        (operand)->arrayIndex.mode    = clvINDEX_NONE; \
        (operand)->matrixIndex.mode    = clvINDEX_NONE; \
        (operand)->vectorIndex.mode    = clvINDEX_NONE; \
        (operand)->u.constant.allValuesEqual = clsROPERAND_CONSTANT_IsAllVectorComponentsEqual(operand); \
    } \
    while (gcvFALSE)

#define clsROPERAND_InitializeBoolOrBVecConstant(operand, _dataType, _boolValue) \
    do { \
        gctUINT _i; \
        (operand)->dataType    = (_dataType); \
        (operand)->isReg    = gcvFALSE; \
        (operand)->u.constant.dataType    = (_dataType); \
        (operand)->u.constant.valueCount= gcGetDataTypeComponentCount(_dataType); \
        for (_i = 0; _i < (operand)->u.constant.valueCount; _i++) { \
            (operand)->u.constant.values[_i].boolValue    = (_boolValue); \
        } \
        (operand)->arrayIndex.mode    = clvINDEX_NONE; \
        (operand)->matrixIndex.mode    = clvINDEX_NONE; \
        (operand)->vectorIndex.mode    = clvINDEX_NONE; \
        (operand)->u.constant.allValuesEqual = clsROPERAND_CONSTANT_IsAllVectorComponentsEqual(operand); \
    } \
    while (gcvFALSE)

#define clsROPERAND_InitializeCharConstant(operand, _dataType, _charValue) \
    do { \
        gctUINT _i; \
        (operand)->dataType    = (_dataType); \
        (operand)->isReg    = gcvFALSE; \
        (operand)->u.constant.dataType    = (_dataType); \
        (operand)->u.constant.valueCount= gcGetDataTypeComponentCount(_dataType); \
        for (_i = 0; _i < (operand)->u.constant.valueCount; _i++) { \
            (operand)->u.constant.values[_i].charValue = (_charValue); \
        } \
        (operand)->arrayIndex.mode    = clvINDEX_NONE; \
        (operand)->matrixIndex.mode    = clvINDEX_NONE; \
        (operand)->vectorIndex.mode    = clvINDEX_NONE; \
        (operand)->u.constant.allValuesEqual = clsROPERAND_CONSTANT_IsAllVectorComponentsEqual(operand); \
    } \
    while (gcvFALSE)

#define clsROPERAND_InitializeAsMatrixColumn(operand, _matrixOperand, _matrixIndex) \
    do { \
        gcmASSERT((_matrixOperand)->matrixIndex.mode == clvINDEX_NONE); \
        *(operand)        = *(_matrixOperand); \
        (operand)->dataType = \
                    gcGetMatrixColumnDataType((_matrixOperand)->dataType); \
        (operand)->matrixIndex.mode       = clvINDEX_CONSTANT; \
        (operand)->matrixIndex.u.constant = (gctREG_INDEX) (_matrixIndex); \
    } \
    while (gcvFALSE)

#define clsROPERAND_InitializeAsMatrixComponent(operand, _matrixOperand,  _matrixIndex, _vectorIndex) \
    do { \
        gcmASSERT((_matrixOperand)->matrixIndex.mode == clvINDEX_NONE); \
        gcmASSERT((_matrixOperand)->vectorIndex.mode == clvINDEX_NONE); \
        *(operand)        = *(_matrixOperand); \
        (operand)->dataType    = \
                    gcGetComponentDataType((_matrixOperand)->dataType); \
        (operand)->matrixIndex.mode     = clvINDEX_CONSTANT; \
        (operand)->matrixIndex.u.constant= (gctREG_INDEX) (_matrixIndex); \
        (operand)->vectorIndex.mode     = clvINDEX_CONSTANT; \
        (operand)->vectorIndex.u.constant= (gctREG_INDEX) (_vectorIndex); \
    } \
    while (gcvFALSE)

#define clsROPERAND_InitializeAsVectorComponent(operand, _vectorOperand, _vectorIndex) \
    do { \
        gcmASSERT((_vectorOperand)->vectorIndex.mode == clvINDEX_NONE); \
        *(operand)        = *(_vectorOperand); \
        (operand)->dataType    = \
                    gcGetVectorComponentDataType((_vectorOperand)->dataType); \
        (operand)->vectorIndex.mode    = clvINDEX_CONSTANT; \
        (operand)->vectorIndex.u.constant= (gctREG_INDEX) (_vectorIndex); \
    } \
    while (gcvFALSE)

gctBOOL
clsROPERAND_IsFloatOrVecConstant(
    IN clsROPERAND * ROperand,
    IN gctFLOAT FloatValue
    );

typedef struct _clsIOPERAND
{
    clsGEN_CODE_DATA_TYPE    dataType;
    gctREG_INDEX    tempRegIndex;
    clsGEN_CODE_DATA_TYPE    regDataType;
    clsCOMPONENT_SELECTION    componentSelection;
}
clsIOPERAND;

void
clGetVectorIOperandSlice(
    IN clsIOPERAND * IOperand,
    IN gctUINT8 StartComponent,
    IN gctUINT8 SliceComponentCount,
    OUT clsIOPERAND * IOperandSlice
    );

#define clsIOPERAND_Initialize(compiler, operand, _dataType, _tempRegIndex) \
    do { \
        (operand)->dataType    = (_dataType); \
        (operand)->tempRegIndex    = (_tempRegIndex); \
        (operand)->regDataType    = (_dataType); \
        (operand)->componentSelection = clGetDefaultComponentSelection((compiler), (_dataType)); \
    } \
    while (gcvFALSE)

#define clsIOPERAND_InitializeWithComponentSelection(operand, _dataType, _regDataType, _tempRegIndex, _componentSelection) \
    do { \
        (operand)->dataType    = (_dataType); \
        (operand)->tempRegIndex    = (_tempRegIndex); \
        (operand)->regDataType    = (_regDataType); \
        (operand)->componentSelection = (_componentSelection); \
    } \
    while (gcvFALSE)

#define clsIOPERAND_New(compiler, operand, _dataType) \
    do { \
        (operand)->dataType  = (_dataType); \
        (operand)->tempRegIndex    = clNewTempRegs((compiler), gcGetDataTypeRegSize(_dataType), (_dataType).elementType); \
        (operand)->regDataType    = (_dataType); \
        (operand)->componentSelection = clGetDefaultComponentSelection((compiler), (_dataType)); \
    } \
    while (gcvFALSE)

#define clsIOPERAND_InitializeAsMatrixColumn(operand, _matrixOperand, _matrixIndex) \
    do { \
        (operand)->dataType    = gcGetMatrixColumnDataType((_matrixOperand)->dataType); \
        (operand)->tempRegIndex    = (gctREG_INDEX) ((_matrixOperand)->tempRegIndex + \
                                                          gcGetDataTypeRegSize((operand)->dataType) * (_matrixIndex)); \
        (operand)->regDataType    = (operand)->dataType; \
        (operand)->componentSelection = (_matrixOperand)->componentSelection; \
    } \
    while (gcvFALSE)

gceSTATUS
clGenScaledIndexOperand(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsROPERAND *IndexOperand,
    IN gctINT32 ElementDataTypeSize,
    IN gctBOOL  needShift,
    OUT clsROPERAND *ScaledOperand
    );

gceSTATUS
clGenStoreCode(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN cleOPCODE Opcode,
IN clsROPERAND *ROperand,
IN clsLOPERAND *LOperand,
IN clsGEN_CODE_DATA_TYPE ResType,
IN clsROPERAND *Offset
);

void
clGenClearCurrentVectorCreation(
IN cloCOMPILER Compiler
);

gceSTATUS
clGenAtomicCode(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN cleOPCODE Opcode,
IN clsIOPERAND *IOperand,
IN clsROPERAND *ROperand,
IN clsROPERAND *CmpOperand,
IN clsROPERAND *ValOperand
);

gceSTATUS
clGenBitwiseExprCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN clsIOPERAND * IOperand,
    IN clsROPERAND * ROperand0,
    IN clsROPERAND * ROperand1
    );

gceSTATUS
clGenAssignCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsLOPERAND * LOperand,
    IN clsROPERAND * ROperand
    );

gceSTATUS
clGenArithmeticExprCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN clsIOPERAND * IOperand,
    IN clsROPERAND * ROperand0,
    IN clsROPERAND * ROperand1
    );

gceSTATUS
clGenGenericNullTargetCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN clsROPERAND * ROperand0,
    IN clsROPERAND * ROperand1
    );

gceSTATUS
clGenSelectExprCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN clsIOPERAND * IOperand,
    IN clsROPERAND * Cond,
    IN clsROPERAND * ROperand0,
    IN clsROPERAND * ROperand1
    );

gceSTATUS
clGenGenericCode1(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN clsIOPERAND * IOperand,
    IN clsROPERAND * ROperand
    );

gceSTATUS
clGenGenericCode2(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN clsIOPERAND * IOperand,
    IN clsROPERAND * ROperand0,
    IN clsROPERAND * ROperand1
    );

gceSTATUS
clGenDotCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsIOPERAND * IOperand,
    IN clsROPERAND * ROperand0,
    IN clsROPERAND * ROperand1
);

gceSTATUS
clGenShiftExprCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleOPCODE Opcode,
    IN clsIOPERAND * IOperand,
    IN clsROPERAND * ROperand0,
    IN clsROPERAND * ROperand1
    );

gceSTATUS
clGenTestJumpCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctLABEL Label,
    IN gctBOOL TrueJump,
    IN clsROPERAND * ROperand
    );

gceSTATUS
clGenCompareJumpCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctLABEL Label,
    IN gctBOOL TrueJump,
    IN cleCONDITION CompareCondition,
    IN clsROPERAND * ROperand0,
    IN clsROPERAND * ROperand1
    );

typedef struct _clsSELECTION_CONTEXT
{
    gctBOOL    hasFalseOperand;
    gctBOOL isNegativeCond;
    gctLABEL endLabel;
    gctLABEL beginLabelOfFalseOperand;
}
clsSELECTION_CONTEXT;

gceSTATUS
clDefineSelectionBegin(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN gctBOOL HasFalseOperand,
    OUT clsSELECTION_CONTEXT * SelectionContext
    );

gceSTATUS
clDefineSelectionEnd(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN clsSELECTION_CONTEXT * SelectionContext
    );

gceSTATUS
clGenSelectionTestConditionCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN clsSELECTION_CONTEXT * SelectionContext,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsROPERAND * ROperand
    );

gceSTATUS
clGenSelectionCompareConditionCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN clsSELECTION_CONTEXT * SelectionContext,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleCONDITION CompareCondition,
    IN clsROPERAND * ROperand0,
    IN clsROPERAND * ROperand1
    );

gceSTATUS
clDefineSelectionTrueOperandBegin(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN clsSELECTION_CONTEXT * SelectionContext
    );

gceSTATUS
clDefineSelectionTrueOperandEnd(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cloCODE_GENERATOR CodeGenerator,
    IN clsSELECTION_CONTEXT * SelectionContext,
    IN gctBOOL HasReturn
    );

gceSTATUS
clDefineSelectionFalseOperandBegin(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN clsSELECTION_CONTEXT * SelectionContext
    );

gceSTATUS
clDefineSelectionFalseOperandEnd(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN clsSELECTION_CONTEXT * SelectionContext
    );


gctLABEL
clGetSelectionConditionLabel(
    IN clsSELECTION_CONTEXT * SelectionContext
    );

typedef struct _clsITERATION_CONTEXT
{
    struct _clsITERATION_CONTEXT *    prevContext;

    gctBOOL    isUnrolled;
    union {
        struct {
            clsNAME *       loopIndexName;
            cluCONSTANT_VALUE  loopIndexValue;
            gctLABEL        bodyEndLabel;
            gctUINT         loopCount;
        }
        unrolledInfo;

        struct {
            gctBOOL     isTestFirst;
            gctBOOL     hasRestExpr;
            gctLABEL    loopBeginLabel;
            gctLABEL    restLabel;
        }
        genericInfo;
    }
    u;
    gctLABEL    endLabel;
}
clsITERATION_CONTEXT;

typedef struct _clsVECTOR_CREATION
{
    clsGEN_CODE_DATA_TYPE    dataType;
    gctREG_INDEX        tempRegIndex;
    struct {
        clsNAME        *pointer;
        gctINT        offset;
    } component[cldMaxComponentCount];
}
clsVECTOR_CREATION;

typedef struct _clsFUNC_DEF_CONTEXT
{
    gctBOOL        isMain;
    gctBOOL        isKernel;
    gctLABEL    mainEndLabel;
    cloIR_SET   funcBody;
}
clsFUNC_DEF_CONTEXT;

/* CODE_GENERATOR */
typedef enum _cleGEN_CODE_HINT
{
    clvGEN_NO_HINT               = 0x0,
    clvGEN_GENERIC_CODE          = 0x1,
    clvGEN_INDEX_CODE            = 0x2,
    clvGEN_DEREF_CODE            = 0x4,
    clvGEN_DEREF_STRUCT_CODE     = 0x8,
    clvGEN_COMPONENT_SELECT_CODE = 0x10,
    clvGEN_ADDR_CODE             = 0x20,
    clvGEN_LEFT_ASSIGN_CODE      = 0x40,
    clvGEN_FIELD_SELECT_CODE     = 0x80,
    clvGEN_SUBSCRIPT_CODE        = 0x100,
    clvGEN_FMA                   = 0x200,
    clvGEN_FOUND_MULTIPLICAND    = 0x400,
    clvGEN_ADDRESS_OFFSET        = 0x800,
    clvGEN_SELECTIVE_LOADED      = 0x1000,
    clvGEN_SELECTIVE_LOAD_TO     = 0x2000,
    clvGEN_SAVE_ADDRESS_OFFSET   = 0x4000,
    clvGEN_ARRAY_OF_CONSTANTS    = 0x8000,
    clvGEN_INITIALIZATION        = 0x10000,

    clvEVALUATE_ONLY             = 0x80000000
}
cleGEN_CODE_HINT;

typedef struct _clsGEN_CODE_PARAMETER_DATA_TYPE
{
    clsGEN_CODE_DATA_TYPE def;
    clsROPERAND byteOffset;
    gctINT savedByteOffset;
} clsGEN_CODE_PARAMETER_DATA_TYPE;

typedef struct _clsGEN_CODE_PARAMETERS
{
    gctBOOL        needLOperand;
    gctBOOL        needROperand;
    gctBOOL        hasIOperand;
    cleGEN_CODE_HINT  hint;
    cloIR_EXPR  expr;
    cloIR_CONSTANT constant;
    gctUINT    operandCount;

    clsGEN_CODE_PARAMETER_DATA_TYPE *dataTypes;
    clsLOPERAND *    lOperands;
    clsROPERAND *    rOperands;
    /* elementIndex and parentType should be used together. */
    clsROPERAND *   elementIndex;
    clsDECL         parentDataType;
    clsIOPERAND    iOperand[1];
    cloIR_CONSTANT constantArray; /* corresponds to hint of clvGEN_ARRAY_OF_CONSTANTS */
}
clsGEN_CODE_PARAMETERS;

#define clmGEN_CODE_GetParametersIOperand(Compiler, IOperand, Parameters, DataType)  do { \
            if((Parameters)->hasIOperand) { \
               gctUINT8 startComponent; \
               gctUINT8 componentCount; \
               startComponent = (Parameters)->iOperand->componentSelection.components; \
               componentCount = gcGetDataTypeTargetComponentCount((DataType)); \
               (Parameters)->iOperand->componentSelection.components =  \
                  gcGetDataTypeTargetComponentCount((Parameters)->iOperand->regDataType); \
               if(clmGEN_CODE_IsScalarDataType((Parameters)->iOperand->dataType)) { \
                  gcmASSERT(clmGEN_CODE_IsScalarDataType((DataType))); \
                  *(IOperand) = (Parameters)->iOperand[0]; \
                  (IOperand)->componentSelection = clGetComponentSelectionSlice((Parameters)->iOperand->componentSelection, \
                                                                                startComponent, \
                                                                                componentCount); \
               } \
               else clGetVectorIOperandSlice((Parameters)->iOperand, \
                                             startComponent, \
                                             componentCount, \
                                             (IOperand)); \
               (Parameters)->iOperand->componentSelection.components = startComponent + componentCount; \
            } \
            else { \
               clsIOPERAND_New((Compiler), (IOperand), (DataType)); \
            } \
   } while (gcvFALSE)

#define clmGEN_CODE_SetParametersIOperand(Compiler, Parameters, OperandIx, IOperand, ComponentIx)  do { \
    (Parameters)->hasIOperand = gcvTRUE; \
    (Parameters)->iOperand[(OperandIx)] = (IOperand)[0]; \
    (Parameters)->iOperand[(OperandIx)].componentSelection.components = (ComponentIx); \
   } while (gcvFALSE)

struct _clsGEN_CODE_PARAMETERS;

#define clsGEN_CODE_PARAMETERS_Initialize(parameters, _needLOperand, _needROperand) \
    do { \
        gcoOS_ZeroMemory((parameters), gcmSIZEOF(struct _clsGEN_CODE_PARAMETERS)); \
        (parameters)->needLOperand    = (_needLOperand); \
        (parameters)->needROperand    = (_needROperand); \
    } \
    while (gcvFALSE)

#define clsGEN_CODE_PARAMETERS_Finalize(parameters) \
    do { \
        if ((parameters)->constant != gcvNULL) { \
            gcmVERIFY_OK(cloIR_OBJECT_Destroy(Compiler, &(parameters)->constant->exprBase.base)); \
        } \
        if ((parameters)->dataTypes != gcvNULL) { \
            gcmVERIFY_OK(cloCOMPILER_Free(Compiler, (parameters)->dataTypes)); \
        } \
        if ((parameters)->lOperands != gcvNULL) { \
            gcmVERIFY_OK(cloCOMPILER_Free(Compiler, (parameters)->lOperands)); \
        } \
        if ((parameters)->rOperands != gcvNULL) { \
            gcmVERIFY_OK(cloCOMPILER_Free(Compiler, (parameters)->rOperands)); \
        } \
        if ((parameters)->elementIndex != gcvNULL) { \
            gcmVERIFY_OK(cloCOMPILER_Free(Compiler, (parameters)->elementIndex)); \
        } \
        gcoOS_ZeroMemory(&(parameters)->parentDataType, gcmSIZEOF(clsDECL)); \
    } \
    while (gcvFALSE)

#define clsGEN_CODE_PARAMETERS_MoveOperands(parameters0, parameters1) \
    do { \
        (parameters0)->dataTypes= (parameters1)->dataTypes; \
        (parameters0)->lOperands= (parameters1)->lOperands; \
        (parameters0)->rOperands= (parameters1)->rOperands; \
        (parameters0)->operandCount = (parameters1)->operandCount; \
        (parameters0)->elementIndex = (parameters1)->elementIndex; \
        (parameters0)->parentDataType = (parameters1)->parentDataType; \
        (parameters1)->dataTypes= gcvNULL; \
        (parameters1)->lOperands= gcvNULL; \
        (parameters1)->rOperands= gcvNULL; \
        (parameters1)->elementIndex= gcvNULL; \
        gcoOS_ZeroMemory(&(parameters1)->parentDataType, gcmSIZEOF(clsDECL)); \
    } \
    while (gcvFALSE)

gceSTATUS
clsGEN_CODE_PARAMETERS_AllocateOperands(
IN cloCOMPILER Compiler,
IN OUT clsGEN_CODE_PARAMETERS * Parameters,
IN clsDECL * Decl
);

gctSIZE_T
clGEN_CODE_DataTypeByteSize(
cloCOMPILER Compiler,
clsGEN_CODE_DATA_TYPE DataType
);

gceSTATUS
clGenIntrinsicAssignCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsLOPERAND * LOperand,
    IN clvVIR_IK IntrinsicKind,
    IN clsROPERAND * ROperand
    );

gceSTATUS
clGenIntrinsicAsmCode(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clvVIR_IK IntrinsicKind,
    IN clsLOPERAND * LOperand,
    IN gctUINT OperandCount,
    IN clsROPERAND * ROperands
);

gceSTATUS
clGenBuiltinToIntrinsicAsmCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_POLYNARY_EXPR PolynaryExpr,
IN gctUINT OperandCount,
IN clsGEN_CODE_PARAMETERS * OperandsParameters,
IN clsIOPERAND * IOperand,
IN clvVIR_IK IntrinsicKind
);

gceSTATUS
clAllocateFuncResources(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN clsNAME * FuncName
);

gceSTATUS
clGenFuncCallCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_POLYNARY_EXPR PolynaryExpr,
IN clsGEN_CODE_PARAMETERS * operandsParameters,
IN OUT clsGEN_CODE_PARAMETERS * Parameters
);

cloIR_POLYNARY_EXPR
clCreateFuncCallByName(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctCONST_STRING Name,
IN cloIR_EXPR Expr
);

gceSTATUS
clGenBuiltInAsmCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS *OperandsParameters,
    IN clsIOPERAND  *IOperand
    );

gceSTATUS
clGenCheckAndImplicitConvertOperand(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN clsDECL *TargetDecl,
IN OUT clsROPERAND *Operand
);


gctBOOL
clGenNeedVectorUpdate(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_EXPR Expr,
IN clsGEN_CODE_DATA_TYPE DataType,
IN clsROPERAND *ScaledIndex,
IN clsIOPERAND *IOperand
);

gceSTATUS
clGenGotoCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN gctBOOL IsNew,
IN OUT cloIR_LABEL Label
);

gceSTATUS
clGenLabelCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN gctBOOL IsNew,
IN cloIR_LABEL Label
);

gceSTATUS
clGenAddToOffset(
IN clsROPERAND *Offset,
IN gctINT Incr
);

/* cloCODE_GENERATOR */
struct _cloCODE_GENERATOR
{
    clsVISITOR        visitor;
    clsFUNC_DEF_CONTEXT    currentFuncDefContext;
    clsITERATION_CONTEXT * currentIterationContext;
    clsVECTOR_CREATION     currentVector;
    gceCHIPMODEL     chipModel;
    gctUINT32        chipRevision;
    gctUINT32        fpConfig;
    gctBOOL          needFloatingPointAccuracy;
    gctBOOL          hasNEW_SIN_COS_LOG_DIV;
    gctBOOL          supportRTNE;
    gctBOOL          supportAtomic;
    gctBOOL          fulllySupportIntegerBranch;
    gctUINT          derivedTypeNameBufferSize;
    slsSLINK_LIST    *derivedTypeVariables;
    gctINT16         currentUniformBlockMember;
    gcsUNIFORM_BLOCK uniformBlock;
    union {
       gctBOOL       vectorMemoryDeref;  /* tell shared codegen function _GenVloadCode, _GenVstoreCode that it is effective by vector memory dereference */
    } codeGenHandShakeFlag;
};

#define clmCODE_GENERATOR_IsVectorMemoryDeref(CG) \
   ((CG)->codeGenHandShakeFlag.vectorMemoryDeref)
#define clmCODE_GENERATOR_SetVectorMemoryDeref(CG) \
   ((CG)->codeGenHandShakeFlag.vectorMemoryDeref = gcvTRUE)
#define clmCODE_GENERATOR_ClrVectorMemoryDeref(CG) \
   ((CG)->codeGenHandShakeFlag.vectorMemoryDeref = gcvFALSE)

gceSTATUS
cloCODE_GENERATOR_Construct(
    IN cloCOMPILER Compiler,
    OUT cloCODE_GENERATOR * CodeGenerator
    );

gceSTATUS
cloCODE_GENERATOR_Destroy(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator
    );

clsGEN_CODE_DATA_TYPE
clConvToUnpackedType(
    IN cloCOMPILER Compiler,
    IN clsGEN_CODE_DATA_TYPE PackedType
    );

gceSTATUS
clCreateUnpackedDecl(
    IN cloCOMPILER Compiler,
    IN clsGEN_CODE_DATA_TYPE PackedType,
    IN clsDECL *Decl
    );

gceSTATUS
clUnpackROperand(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsROPERAND *From,
    OUT clsIOPERAND *To
    );

clsGEN_CODE_DATA_TYPE
clConvToPackedType(
    IN cloCOMPILER Compiler,
    IN clsGEN_CODE_DATA_TYPE UnpackedType
    );

gceSTATUS
clCreatePackedDecl(
    IN cloCOMPILER Compiler,
    IN clsGEN_CODE_DATA_TYPE UnPackedType,
    IN clsDECL *Decl
    );

gceSTATUS
clPackROperand(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN clsROPERAND *From,
    OUT clsIOPERAND *To
    );

gceSTATUS
clROperandInitializeConstant(
    IN cloCOMPILER Compiler,
    IN clsGEN_CODE_DATA_TYPE DataType,
    IN gcsValue *Value,
    OUT clsROPERAND *Operand
    );

gceSTATUS
clConvComponentSelectionToPackedSwizzle(
    IN cloCOMPILER Compiler,
    IN gctUINT8 LastComponent,
    IN clsCOMPONENT_SELECTION *ComponentSelection,
    OUT clsROPERAND *Operand
    );
#endif /* __gc_cl_gen_code_h_ */
