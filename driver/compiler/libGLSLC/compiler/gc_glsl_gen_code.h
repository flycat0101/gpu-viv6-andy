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


#ifndef __gc_glsl_gen_code_h_
#define __gc_glsl_gen_code_h_

#include "gc_glsl_ir.h"

typedef    gctUINT32        gctREG_INDEX;

slsCOMPONENT_SELECTION
slGetDefaultComponentSelection(
    IN gcSHADER_TYPE DataType
    );

/* OPCODE */
/* Always add a new opcode at the end!!! */
typedef enum _sleOPCODE
{
    slvOPCODE_INVALID                    = 0,

    /* Assignment Operation */
    slvOPCODE_ASSIGN,
    slvOPCODE_CONV,

    /* Arithmetic Operations */
    slvOPCODE_ADD,
    slvOPCODE_SUB,
    slvOPCODE_MUL,
    slvOPCODE_MULHI,
    slvOPCODE_DIV,
    slvOPCODE_IDIV,
    slvOPCODE_MOD,
    slvOPCODE_SAMPLER_ASSIGN,

    /* Texture Operations */
    slvOPCODE_TEXTURE_LOAD,
    slvOPCODE_TEXTURE_LOAD_U,
    slvOPCODE_TEXTURE_LOAD_PROJ,
    slvOPCODE_TEXTURE_LOAD_PCF,
    slvOPCODE_TEXTURE_LOAD_PCFPROJ,
    slvOPCODE_TEXTURE_BIAS,
    slvOPCODE_TEXTURE_GRAD,
    slvOPCODE_TEXTURE_LOD,
    slvOPCODE_TEXTURE_GATHER,
    slvOPCODE_TEXTURE_FETCH_MS,
    slvOPCODE_TEXTURE_U,
    slvOPCODE_TEXTURE_U_LOD,

    /* Conversion Operations */
    slvOPCODE_FLOAT_TO_INT,
    slvOPCODE_FLOAT_TO_BOOL,
    slvOPCODE_FLOAT_TO_UINT,
    slvOPCODE_FLOAT_TO_HALF,
    slvOPCODE_HALF_TO_FLOAT,

    slvOPCODE_INT_TO_BOOL,
    slvOPCODE_INT_TO_UINT,
    slvOPCODE_INT_TO_FLOAT,

    slvOPCODE_UINT_TO_BOOL,
    slvOPCODE_UINT_TO_INT,
    slvOPCODE_UINT_TO_FLOAT,

    slvOPCODE_BOOL_TO_INT,
    slvOPCODE_BOOL_TO_UINT,
    slvOPCODE_BOOL_TO_FLOAT,

    /* Other Calculation Operations */
    slvOPCODE_INVERSE,

    slvOPCODE_LESS_THAN,
    slvOPCODE_LESS_THAN_EQUAL,
    slvOPCODE_GREATER_THAN,
    slvOPCODE_GREATER_THAN_EQUAL,
    slvOPCODE_EQUAL,
    slvOPCODE_NOT_EQUAL,

    slvOPCODE_BITWISE_AND,
    slvOPCODE_BITWISE_OR,
    slvOPCODE_BITWISE_XOR,
    slvOPCODE_BITWISE_NOT,

    slvOPCODE_LSHIFT,
    slvOPCODE_RSHIFT,

    slvOPCODE_ANY,
    slvOPCODE_ALL,
    slvOPCODE_NOT,

    slvOPCODE_SIN,
    slvOPCODE_COS,
    slvOPCODE_TAN,

    slvOPCODE_ASIN,
    slvOPCODE_ACOS,
    slvOPCODE_ATAN,
    slvOPCODE_ATAN2,

    slvOPCODE_POW,
    slvOPCODE_EXP2,
    slvOPCODE_LOG2,
    slvOPCODE_SQRT,
    slvOPCODE_INVERSE_SQRT,

    slvOPCODE_ABS,
    slvOPCODE_SIGN,
    slvOPCODE_FLOOR,
    slvOPCODE_CEIL,
    slvOPCODE_FRACT,
    slvOPCODE_MIN,
    slvOPCODE_MAX,
    slvOPCODE_SATURATE,
    slvOPCODE_STEP,
    slvOPCODE_DOT,
    slvOPCODE_CROSS,
    slvOPCODE_NORMALIZE,

    /* Branch Operations */
    slvOPCODE_JUMP,
    slvOPCODE_CALL,
    slvOPCODE_RETURN,
    slvOPCODE_DISCARD,

    /* Derivative Operations */
    slvOPCODE_DFDX,
    slvOPCODE_DFDY,
    slvOPCODE_FWIDTH,

    /* HALTI Load */
    slvOPCODE_LOAD,
    slvOPCODE_STORE1,

    slvOPCODE_POPCOUNT,
    slvOPCODE_FINDLSB,
    slvOPCODE_FINDMSB,
    slvOPCODE_BIT_REVERSAL,
    slvOPCODE_BIT_EXTRACT,
    slvOPCODE_BIT_INSERT,
    slvOPCODE_BIT_RANGE,
    slvOPCODE_BIT_RANGE1,
    slvOPCODE_UCARRY,

    slvOPCODE_ATOMADD,
    slvOPCODE_ATOMSUB,
    slvOPCODE_ATOMMIN,
    slvOPCODE_ATOMMAX,
    slvOPCODE_ATOMOR,
    slvOPCODE_ATOMAND,
    slvOPCODE_ATOMXOR,
    slvOPCODE_ATOMXCHG,
    slvOPCODE_ATOMCMPXCHG,

    slvOPCODE_SET,
    slvOPCODE_CMP,

    slvOPCODE_BARRIER,
    slvOPCODE_MEMORY_BARRIER,

    slvOPCODE_IMAGE_READ,
    slvOPCODE_IMAGE_WRITE,
    slvOPCODE_IMAGE_ADDRESS,
    slvOPCODE_IMAGE_ADDRESS_3D,
    slvOPCODE_GET_SAMPLER_IDX,
    slvOPCODE_GET_SAMPLER_LMM,
    slvOPCODE_GET_SAMPLER_LBS,
    slvOPCODE_IMAGE_READ_3D,
    slvOPCODE_IMAGE_WRITE_3D,
    slvOPCODE_CLAMP0MAX,
    /* attr ld/st. */
    slvOPCODE_ATTR_LD,
    slvOPCODE_ATTR_ST,
    /* GS-only opcode. */
    slvOPCODE_EMIT_VERTEX,
    slvOPCODE_END_PRIMITIVE,

    /* local memory */
    slvOPCODE_LOAD_L,
    slvOPCODE_STORE_L,

    /* The max opcode. */
    slvOPCODE_MAXOPCODE
}
sleOPCODE;

gctCONST_STRING
slGetOpcodeName(
    IN sleOPCODE Opcode
    );

/* CONDITION */
typedef enum _sleCONDITION
{
    slvCONDITION_INVALID,

    slvCONDITION_EQUAL,
    slvCONDITION_NOT_EQUAL,
    slvCONDITION_LESS_THAN,
    slvCONDITION_LESS_THAN_EQUAL,
    slvCONDITION_GREATER_THAN,
    slvCONDITION_GREATER_THAN_EQUAL,
    slvCONDITION_ZERO,
    slvCONDITION_NOT_ZERO,
    slvCONDITION_AND,
    slvCONDITION_OR,
    slvCONDITION_XOR
}
sleCONDITION;

gctCONST_STRING
slGetConditionName(
    IN sleCONDITION Condition
    );

sleCONDITION
slGetNotCondition(
    IN sleCONDITION Condition
    );

/* slsLOPERAND and slsROPERAND */
typedef enum _sleINDEX_MODE
{
    slvINDEX_NONE,
    slvINDEX_REG,           /* this is actually slvINDEX_REG_X */
    slvINDEX_CONSTANT,
    slvINDEX_REG_Y,
    slvINDEX_REG_Z,
    slvINDEX_REG_W
}
sleINDEX_MODE;

/* slsLOPERAND and slsROPERAND */
typedef enum _sleINDEX_LEVEL
{
    slvINDEX_LEVEL_NONE,
    slvINDEX_LEVEL_LEAF,
    slvINDEX_LEVEL_NODE,
    slvINDEX_LEVEL_LEAF_AND_NODE
}
sleINDEX_LEVEL;

typedef struct _slsLOGICAL_REG
{
    gcSHADER_TYPE           dataType;    /* the first two fields: dataType, and precision need to be
                                            maintained in same order as the corresponding field in
                                            struct _slsOPERAND_CONSTANT */

    gcSHADER_PRECISION      precision;

    sltSTORAGE_QUALIFIER    qualifier;

    union
    {
        gcATTRIBUTE         attribute;

        gcUNIFORM           uniform;

        gcVARIABLE          variable;

        gcOUTPUT            output;

        gctPOINTER          pointer;
    }    u;

    gctREG_INDEX            regIndex;

    slsCOMPONENT_SELECTION  componentSelection;
}
slsLOGICAL_REG;

#define slsLOGICAL_REG_InitializeTemp(reg, _qualifier, _dataType, _precision, _regIndex) \
    do \
    { \
        (reg)->qualifier            = (_qualifier); \
        (reg)->dataType                = (_dataType); \
        (reg)->precision            = (_precision); \
        (reg)->regIndex                = (_regIndex); \
        (reg)->componentSelection    = slGetDefaultComponentSelection(_dataType); \
    } \
    while (gcvFALSE)

#define slsLOGICAL_REG_InitializeVariable(reg, _qualifier, _dataType, _precision, _variable, _regIndex) \
    do \
    { \
        slsLOGICAL_REG_InitializeTemp((reg), (_qualifier), (_dataType), (_precision), (_regIndex)); \
        (reg)->u.variable = (_variable); \
    } \
    while (gcvFALSE)

#define slsLOGICAL_REG_InitializeAttribute(reg, _qualifier, _dataType, _precision, _attribute, _regIndex) \
    do \
    { \
        (reg)->qualifier            = (_qualifier); \
        (reg)->dataType                = (_dataType); \
        (reg)->precision            = (_precision); \
        (reg)->u.attribute            = (_attribute); \
        (reg)->regIndex                = (_regIndex); \
        (reg)->componentSelection    = slGetDefaultComponentSelection(_dataType); \
    } \
    while (gcvFALSE)

#define slsLOGICAL_REG_InitializeUniform(reg, _qualifier, _dataType, _precision, _uniform, _regIndex) \
    do \
    { \
        (reg)->qualifier            = (_qualifier); \
        (reg)->dataType                = (_dataType); \
        (reg)->precision            = (_precision); \
        (reg)->u.uniform            = (_uniform); \
        (reg)->regIndex                = (_regIndex); \
        (reg)->componentSelection    = slGetDefaultComponentSelection(_dataType); \
    } \
    while (gcvFALSE)

#define slsLOGICAL_REG_InitializeOutput(reg, _qualifier, _dataType, _precision, _output, _regIndex) \
    do \
    { \
        (reg)->qualifier            = (_qualifier); \
        (reg)->dataType                = (_dataType); \
        (reg)->precision            = (_precision); \
        (reg)->u.output            = (_output); \
        (reg)->regIndex                = (_regIndex); \
        (reg)->componentSelection    = slGetDefaultComponentSelection(_dataType); \
    } \
    while (gcvFALSE)

#define slmGEN_CODE_IF(Compiler, CodeGenerator, LineNo, StringNo, LOperand, Condition, ROperand)  \
    { slsSELECTION_CONTEXT SelectionContext[1]; \
        do { \
           gcmONERROR(slDefineSelectionBegin(Compiler, \
                         CodeGenerator, \
                         gcvFALSE, \
                         SelectionContext)); \
           gcmONERROR(slGenSelectionCompareConditionCode(Compiler, \
                                                         CodeGenerator, \
                                                         SelectionContext, \
                                                         (LineNo), \
                                                         (StringNo), \
                                                         Condition, \
                                                         LOperand, \
                                                         ROperand)); \
       gcmONERROR(slDefineSelectionTrueOperandBegin(Compiler, \
                                                        CodeGenerator, \
                                                        SelectionContext)); \
        }  while (gcvFALSE)

#define slmGEN_CODE_IF_NEXT_COND(Compiler, CodeGenerator, LineNo, StringNo, LOperand, Condition, ROperand)  \
        do { \
       if(SelectionContext->isNegativeCond == gcvTRUE) { \
          gcmONERROR(slSetLabel(Compiler, \
                    (LineNo), \
                    (StringNo), \
                    SelectionContext->endLabel)); \
              SelectionContext->endLabel = slNewLabel(Compiler); \
          SelectionContext->isNegativeCond = gcvFALSE; \
       } \
           gcmONERROR(slGenSelectionCompareConditionCode(Compiler, \
                                                         CodeGenerator, \
                                                         SelectionContext, \
                                                         (LineNo), \
                                                         (StringNo), \
                                                         Condition, \
                                                         LOperand, \
                                                         ROperand)); \
        }  while (gcvFALSE)

#define slmGEN_CODE_NOT_IF(Compiler, CodeGenerator, LineNo, StringNo, LOperand, Condition, ROperand)  \
    { slsSELECTION_CONTEXT SelectionContext[1]; \
        do { \
           gcmONERROR(slDefineSelectionBegin(Compiler, \
                         CodeGenerator, \
                         gcvFALSE, \
                         SelectionContext)); \
       SelectionContext->isNegativeCond = gcvTRUE; \
           gcmONERROR(slGenSelectionCompareConditionCode(Compiler, \
                                                         CodeGenerator, \
                                                         SelectionContext, \
                                                         (LineNo), \
                                                         (StringNo), \
                                                         slGetNotCondition((Condition)), \
                                                         LOperand, \
                                                         ROperand)); \
       gcmONERROR(slDefineSelectionTrueOperandBegin(Compiler, \
                                                        CodeGenerator, \
                                                        SelectionContext)); \
        }  while (gcvFALSE)

#define slmGEN_CODE_NOT_IF_NEXT_COND(Compiler, CodeGenerator, LineNo, StringNo, LOperand, Condition, ROperand)  \
        do { \
       if(SelectionContext->isNegativeCond == gcvFALSE) { \
          gcmONERROR(slSetLabel(Compiler, \
                    (LineNo), \
                    (StringNo), \
                    SelectionContext->endLabel)); \
              SelectionContext->endLabel = slNewLabel(Compiler); \
          SelectionContext->isNegativeCond = gcvTRUE; \
       } \
           gcmONERROR(slGenSelectionCompareConditionCode(Compiler, \
                                                         CodeGenerator, \
                                                         SelectionContext, \
                                                         (LineNo), \
                                                         (StringNo), \
                                                         slGetNotCondition((Condition)), \
                                                         LOperand, \
                                                         ROperand)); \
        }  while (gcvFALSE)

#define slmGEN_CODE_ELSE(Compiler, CodeGenerator, LineNo, StringNo)  \
        do { \
           SelectionContext->hasFalseOperand = gcvTRUE; \
           SelectionContext->beginLabelOfFalseOperand = SelectionContext->endLabel; \
           SelectionContext->endLabel = slNewLabel(Compiler); \
       gcmONERROR(slDefineSelectionTrueOperandEnd(Compiler, \
                                                      CodeGenerator, \
                                                      SelectionContext, \
                                                      gcvFALSE)); \
           gcmONERROR(slDefineSelectionFalseOperandBegin(Compiler, \
                                                         CodeGenerator, \
                                                         SelectionContext)); \
        } while (gcvFALSE)

#define slmGEN_CODE_ENDIF(Compiler, CodeGenerator, LineNo, StringNo)  \
        do { \
           if((SelectionContext)->hasFalseOperand) { \
               gcmONERROR(slDefineSelectionFalseOperandEnd(Compiler, \
                               CodeGenerator, \
                               SelectionContext)); \
           } \
           else { \
           gcmONERROR(slDefineSelectionTrueOperandEnd(Compiler, \
                                                          CodeGenerator, \
                                                          SelectionContext, \
                                                          gcvFALSE)); \
           } \
           gcmONERROR(slDefineSelectionEnd(Compiler, \
                                           CodeGenerator, \
                                           SelectionContext)); \
        } while (gcvFALSE); \
     }

gceSTATUS
slsNAME_AllocLogicalRegs(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN slsNAME * Name
    );

gceSTATUS
slsNAME_FreeLogicalRegs(
    IN sloCOMPILER Compiler,
    IN slsNAME * Name
    );

typedef struct _slsINDEX
{
    sleINDEX_MODE    mode;

    union
    {
        gctREG_INDEX    indexRegIndex;

        gctREG_INDEX    constant;
    }    u;
}
slsINDEX;

typedef struct _slsLOPERAND
{
    gcSHADER_TYPE        dataType;

    slsLOGICAL_REG        reg;

    gctBOOL         componentSelected;

    sleINDEX_LEVEL  indexLevel;

    slsINDEX        arrayIndex;

    slsINDEX        matrixIndex;

    slsINDEX        vectorIndex;

    /* This is only used for IO block array. */
    slsINDEX        vertexIndex;
}
slsLOPERAND;

#define slsLOPERAND_Initialize(operand, _reg) \
    do \
    { \
        (operand)->dataType        = (_reg)->dataType; \
        (operand)->reg            = *(_reg); \
        (operand)->componentSelected   = gcvFALSE; \
        (operand)->indexLevel          = slvINDEX_LEVEL_NONE; \
        (operand)->arrayIndex.mode     = slvINDEX_NONE; \
        (operand)->matrixIndex.mode    = slvINDEX_NONE; \
        (operand)->vectorIndex.mode    = slvINDEX_NONE; \
        (operand)->vertexIndex.mode    = slvINDEX_NONE; \
    } \
    while (gcvFALSE)

#define slsLOPERAND_InitializeTempReg(operand, _qualifier, _dataType, _precision, _regIndex) \
    do \
    { \
        (operand)->dataType        = (_dataType); \
        slsLOGICAL_REG_InitializeTemp(&(operand)->reg, (_qualifier), (_dataType), (_precision), (_regIndex)); \
        (operand)->componentSelected   = gcvFALSE; \
        (operand)->indexLevel          = slvINDEX_LEVEL_NONE; \
        (operand)->arrayIndex.mode     = slvINDEX_NONE; \
        (operand)->matrixIndex.mode    = slvINDEX_NONE; \
        (operand)->vectorIndex.mode    = slvINDEX_NONE; \
        (operand)->vertexIndex.mode    = slvINDEX_NONE; \
    } \
    while (gcvFALSE)

#define slsLOPERAND_InitializeAsMatrixColumn(operand, _matrixOperand, _matrixIndex) \
    do \
    { \
        gcmASSERT((_matrixOperand)->matrixIndex.mode == slvINDEX_NONE); \
        \
        *(operand)                        = *(_matrixOperand); \
        (operand)->dataType               = \
                                   gcGetMatrixColumnDataType((_matrixOperand)->dataType); \
        (operand)->matrixIndex.mode       = slvINDEX_CONSTANT; \
        (operand)->matrixIndex.u.constant = (gctREG_INDEX) (_matrixIndex); \
        (operand)->componentSelected   = gcvFALSE; \
        (operand)->indexLevel          = (_matrixOperand)->indexLevel; \
    } \
    while (gcvFALSE)

#define slsLOPERAND_InitializeAsMatrixComponent(operand, _matrixOperand,  _matrixIndex, _vectorIndex) \
    do \
    { \
        gcmASSERT((_matrixOperand)->matrixIndex.mode == slvINDEX_NONE); \
        gcmASSERT((_matrixOperand)->vectorIndex.mode == slvINDEX_NONE); \
        \
        *(operand)                        = *(_matrixOperand); \
        (operand)->dataType               = \
                                    gcGetComponentDataType((_matrixOperand)->dataType); \
        (operand)->matrixIndex.mode       = slvINDEX_CONSTANT; \
        (operand)->matrixIndex.u.constant = (gctREG_INDEX) (_matrixIndex); \
        (operand)->vectorIndex.mode       = slvINDEX_CONSTANT; \
        (operand)->vectorIndex.u.constant = (gctREG_INDEX) (_vectorIndex); \
        (operand)->componentSelected   = gcvFALSE; \
        (operand)->indexLevel          = (_matrixOperand)->indexLevel; \
    } \
    while (gcvFALSE)

#define slsLOPERAND_InitializeAsVectorComponent(operand, _vectorOperand, _vectorIndex) \
    do \
    { \
        gcmASSERT((_vectorOperand)->vectorIndex.mode == slvINDEX_NONE); \
        \
        *(operand)                        = *(_vectorOperand); \
        (operand)->dataType               = \
                                    gcGetVectorComponentDataType((_vectorOperand)->dataType); \
        (operand)->vectorIndex.mode       = slvINDEX_CONSTANT; \
        (operand)->vectorIndex.u.constant = (gctREG_INDEX) (_vectorIndex); \
        (operand)->componentSelected   = gcvFALSE; \
        (operand)->indexLevel          = (_vectorOperand)->indexLevel; \
    } \
    while (gcvFALSE)

#define slsLOPERAND_InitializeUsingIOperand(operand, iOperand) \
    slsLOPERAND_InitializeTempReg( \
                    (operand), \
                    slvSTORAGE_QUALIFIER_NONE, \
                    (iOperand)->dataType, \
                    (iOperand)->precision, \
                    (iOperand)->tempRegIndex)

#define slsLOPERAND_InitializeWithRegROPERAND(operand, _rOperand) \
    do \
    { \
        gcmASSERT((_rOperand)->isReg); \
        (operand)->dataType         = (_rOperand)->dataType; \
        (operand)->reg              = (_rOperand)->u.reg; \
        (operand)->vectorIndex      = (_rOperand)->vectorIndex; \
        (operand)->arrayIndex       = (_rOperand)->arrayIndex; \
        (operand)->matrixIndex      = (_rOperand)->matrixIndex; \
        (operand)->vertexIndex      = (_rOperand)->vertexIndex; \
        (operand)->componentSelected   = (_rOperand)->componentSelected; \
        (operand)->indexLevel          = (_rOperand)->indexLevel; \
    } \
    while (gcvFALSE)

typedef struct _slsVEC2ARRAY
{
    slsNAME*            scalarArrayName;
    slsLOPERAND         vecOperand;
}
slsVEC2ARRAY;

typedef struct _slsOPERAND_CONSTANT
{
    gcSHADER_TYPE        dataType;    /* the first two fields: dataType, and precision need to be
                                                maintained in same order as the corresponding field in
                                                struct _slsLOGICAL_REG */

    gcSHADER_PRECISION    precision;

    gctUINT            valueCount;

    sluCONSTANT_VALUE    values[16];
}
slsOPERAND_CONSTANT;

typedef struct _slsROPERAND
{
    gcSHADER_TYPE        dataType;

    gctBOOL            isReg;

    union
    {
        slsLOGICAL_REG        reg;

        slsOPERAND_CONSTANT    constant;
    }u;

    gctBOOL         componentSelected;

    sleINDEX_LEVEL  indexLevel;

    slsINDEX        arrayIndex;

    slsINDEX        matrixIndex;

    slsINDEX        vectorIndex;

    slsINDEX        vertexIndex;
}
slsROPERAND;

#define slsROPERAND_InitializeReg(operand, _reg) \
    do \
    { \
        (operand)->dataType            = (_reg)->dataType; \
        (operand)->isReg            = gcvTRUE; \
        (operand)->u.reg            = *(_reg); \
        (operand)->componentSelected       = gcvFALSE; \
        (operand)->indexLevel              = slvINDEX_LEVEL_NONE; \
        (operand)->arrayIndex.mode         = slvINDEX_NONE; \
        (operand)->matrixIndex.mode        = slvINDEX_NONE; \
        (operand)->vectorIndex.mode        = slvINDEX_NONE; \
        (operand)->vertexIndex.mode        = slvINDEX_NONE; \
    } \
    while (gcvFALSE)

#define slsROPERAND_InitializeTempReg(operand, _qualifier, _dataType, _precision, _regIndex) \
    do \
    { \
        (operand)->dataType            = (_dataType); \
        (operand)->isReg            = gcvTRUE; \
        slsLOGICAL_REG_InitializeTemp(&(operand)->u.reg, (_qualifier), (_dataType), (_precision), (_regIndex)); \
        (operand)->componentSelected       = gcvFALSE; \
        (operand)->indexLevel              = slvINDEX_LEVEL_NONE; \
        (operand)->arrayIndex.mode         = slvINDEX_NONE; \
        (operand)->matrixIndex.mode        = slvINDEX_NONE; \
        (operand)->vectorIndex.mode        = slvINDEX_NONE; \
        (operand)->vertexIndex.mode        = slvINDEX_NONE; \
    } \
    while (gcvFALSE)

#define slsROPERAND_InitializeUsingIOperand(operand, iOperand) \
    slsROPERAND_InitializeTempReg( \
                                (operand), \
                                slvSTORAGE_QUALIFIER_NONE, \
                                (iOperand)->dataType, \
                                (iOperand)->precision, \
                                (iOperand)->tempRegIndex)

#define slsROPERAND_InitializeConstant(operand, _dataType, _precision, _valueCount, _values) \
    do \
    { \
        gctUINT _i; \
        \
        (operand)->dataType            = (_dataType); \
        (operand)->isReg            = gcvFALSE; \
        (operand)->u.constant.dataType        = (_dataType); \
        (operand)->u.constant.precision        = (_precision); \
        (operand)->u.constant.valueCount    = (_valueCount); \
        \
        for (_i = 0; _i < (_valueCount); _i++) \
        { \
            (operand)->u.constant.values[_i]    = (_values)[_i]; \
        } \
        \
        (operand)->componentSelected       = gcvFALSE; \
        (operand)->indexLevel              = slvINDEX_LEVEL_NONE; \
        (operand)->arrayIndex.mode         = slvINDEX_NONE; \
        (operand)->matrixIndex.mode        = slvINDEX_NONE; \
        (operand)->vectorIndex.mode        = slvINDEX_NONE; \
        (operand)->vertexIndex.mode        = slvINDEX_NONE; \
    } \
    while (gcvFALSE)

#define slsROPERAND_InitializeFloatOrVecOrMatConstant(operand, _dataType, _precision, _floatValue) \
    do \
    { \
        gctUINT _i; \
        gctUINT componentCount;\
        (operand)->dataType            = (_dataType); \
        (operand)->isReg            = gcvFALSE; \
        (operand)->u.constant.dataType        = (_dataType); \
        (operand)->u.constant.precision        = (_precision); \
        (operand)->u.constant.valueCount    = gcGetDataTypeComponentCount(_dataType); \
        componentCount = (operand)->u.constant.valueCount;\
        \
        for (_i = 0; _i < componentCount; _i++) \
        { \
            (operand)->u.constant.values[_i].floatValue    = (gctFLOAT)(_floatValue); \
        } \
        \
        (operand)->componentSelected       = gcvFALSE; \
        (operand)->indexLevel              = slvINDEX_LEVEL_NONE; \
        (operand)->arrayIndex.mode         = slvINDEX_NONE; \
        (operand)->matrixIndex.mode        = slvINDEX_NONE; \
        (operand)->vectorIndex.mode        = slvINDEX_NONE; \
        (operand)->vertexIndex.mode        = slvINDEX_NONE; \
    } \
    while (gcvFALSE)

#define slsROPERAND_InitializeIntOrIVecConstant(operand, _dataType, _precision,  _intValue) \
    do \
    { \
        gctUINT _i; \
        \
        (operand)->dataType            = (_dataType); \
        (operand)->isReg            = gcvFALSE; \
        (operand)->u.constant.dataType        = (_dataType); \
        (operand)->u.constant.precision        = (_precision); \
        (operand)->u.constant.valueCount    = gcGetDataTypeComponentCount(_dataType); \
        \
        for (_i = 0; _i < gcGetDataTypeComponentCount(_dataType); _i++) \
        { \
            (operand)->u.constant.values[_i].intValue    = (_intValue); \
        } \
        \
        (operand)->componentSelected       = gcvFALSE; \
        (operand)->indexLevel              = slvINDEX_LEVEL_NONE; \
        (operand)->arrayIndex.mode         = slvINDEX_NONE; \
        (operand)->matrixIndex.mode        = slvINDEX_NONE; \
        (operand)->vectorIndex.mode        = slvINDEX_NONE; \
        (operand)->vertexIndex.mode        = slvINDEX_NONE; \
    } \
    while (gcvFALSE)

#define slsROPERAND_InitializeUintOrUvecConstant(operand, _dataType, _precision, _uintValue) \
    do \
    { \
        gctUINT _i; \
        \
        (operand)->dataType            = (_dataType); \
        (operand)->isReg            = gcvFALSE; \
        (operand)->u.constant.dataType        = (_dataType); \
        (operand)->u.constant.precision        = (_precision); \
        (operand)->u.constant.valueCount    = gcGetDataTypeComponentCount(_dataType); \
        \
        for (_i = 0; _i < gcGetDataTypeComponentCount(_dataType); _i++) \
        { \
            (operand)->u.constant.values[_i].uintValue    = (_uintValue); \
        } \
        \
        (operand)->componentSelected       = gcvFALSE; \
        (operand)->indexLevel              = slvINDEX_LEVEL_NONE; \
        (operand)->arrayIndex.mode         = slvINDEX_NONE; \
        (operand)->matrixIndex.mode        = slvINDEX_NONE; \
        (operand)->vectorIndex.mode        = slvINDEX_NONE; \
        (operand)->vertexIndex.mode        = slvINDEX_NONE; \
    } \
    while (gcvFALSE)

#define slsROPERAND_InitializeBoolOrBVecConstant(operand, _dataType, _precision, _boolValue) \
    do \
    { \
        gctUINT _i; \
        \
        (operand)->dataType            = (_dataType); \
        (operand)->isReg            = gcvFALSE; \
        (operand)->u.constant.dataType        = (_dataType); \
        (operand)->u.constant.precision        = (_precision); \
        (operand)->u.constant.valueCount    = gcGetDataTypeComponentCount(_dataType); \
        \
        for (_i = 0; _i < gcGetDataTypeComponentCount(_dataType); _i++) \
        { \
            (operand)->u.constant.values[_i].boolValue    = (_boolValue); \
        } \
        \
        (operand)->componentSelected       = gcvFALSE; \
        (operand)->indexLevel              = slvINDEX_LEVEL_NONE; \
        (operand)->arrayIndex.mode         = slvINDEX_NONE; \
        (operand)->matrixIndex.mode        = slvINDEX_NONE; \
        (operand)->vectorIndex.mode        = slvINDEX_NONE; \
        (operand)->vertexIndex.mode        = slvINDEX_NONE; \
    } \
    while (gcvFALSE)

#define slsROPERAND_InitializeAsMatrixColumn(operand, _matrixOperand, _matrixIndex) \
    do \
    { \
        gcmASSERT((_matrixOperand)->matrixIndex.mode == slvINDEX_NONE); \
        *(operand)                        = *(_matrixOperand); \
        (operand)->dataType               = \
                                    gcGetMatrixColumnDataType((_matrixOperand)->dataType); \
        (operand)->matrixIndex.mode       = slvINDEX_CONSTANT; \
        (operand)->matrixIndex.u.constant = (gctREG_INDEX) (_matrixIndex); \
        (operand)->componentSelected       = gcvFALSE; \
        (operand)->indexLevel              = (_matrixOperand)->indexLevel; \
    } \
    while (gcvFALSE)

#define slsROPERAND_InitializeAsMatrixComponent(operand, _matrixOperand,  _matrixIndex, _vectorIndex) \
    do \
    { \
        gcmASSERT((_matrixOperand)->matrixIndex.mode == slvINDEX_NONE); \
        gcmASSERT((_matrixOperand)->vectorIndex.mode == slvINDEX_NONE); \
        \
        *(operand)                        = *(_matrixOperand); \
        (operand)->dataType               = \
                                    gcGetComponentDataType((_matrixOperand)->dataType); \
        (operand)->matrixIndex.mode       = slvINDEX_CONSTANT; \
        (operand)->matrixIndex.u.constant = (gctREG_INDEX) (_matrixIndex); \
        (operand)->vectorIndex.mode       = slvINDEX_CONSTANT; \
        (operand)->vectorIndex.u.constant = (gctREG_INDEX) (_vectorIndex); \
        (operand)->componentSelected       = gcvFALSE; \
        (operand)->indexLevel              = (_matrixOperand)->indexLevel; \
    } \
    while (gcvFALSE)

#define slsROPERAND_InitializeAsVectorComponent(operand, _vectorOperand, _vectorIndex) \
    do \
    { \
        gcmASSERT((_vectorOperand)->vectorIndex.mode == slvINDEX_NONE); \
        *(operand)                        = *(_vectorOperand); \
        (operand)->dataType               = \
                                    gcGetVectorComponentDataType((_vectorOperand)->dataType); \
        (operand)->vectorIndex.mode       = slvINDEX_CONSTANT; \
        (operand)->vectorIndex.u.constant = (gctREG_INDEX) (_vectorIndex); \
        (operand)->componentSelected       = gcvFALSE; \
        (operand)->indexLevel              = (_vectorOperand)->indexLevel; \
    } \
    while (gcvFALSE)

#define slsROPERAND_InitializeWithLOPERAND(operand, _lOperand) \
    do \
    { \
        (operand)->isReg       = gcvTRUE; \
        (operand)->dataType    = (_lOperand)->dataType; \
        (operand)->u.reg       = (_lOperand)->reg; \
        (operand)->componentSelected       = (_lOperand)->componentSelected; \
        (operand)->indexLevel              = (_lOperand)->indexLevel; \
        (operand)->vectorIndex = (_lOperand)->vectorIndex; \
        (operand)->arrayIndex  = (_lOperand)->arrayIndex; \
        (operand)->matrixIndex = (_lOperand)->matrixIndex; \
        (operand)->vertexIndex = (_lOperand)->vertexIndex; \
    } \
    while (gcvFALSE)

#define slmROPERAND_IsHigherPrecision(_operand1, _operand2) \
        (((_operand1)->u.reg.precision == gcSHADER_PRECISION_DEFAULT) ? gcvFALSE : \
         (((_operand2)->u.reg.precision == gcSHADER_PRECISION_DEFAULT) ? gcvTRUE : \
          ((_operand1)->u.reg.precision < (_operand2)->u.reg.precision)))


#define slmROPERAND_IsStorageBlockMember(ROperand) \
    ((ROperand)->isReg && \
     (ROperand)->u.reg.qualifier == slvSTORAGE_QUALIFIER_STORAGE_BLOCK_MEMBER)

gctBOOL
slsROPERAND_IsFloatOrVecConstant(
    IN slsROPERAND * ROperand,
    IN gctFLOAT FloatValue
    );

typedef struct _slsIOPERAND
{
    gcSHADER_TYPE        dataType;

    gcSHADER_PRECISION    precision;

    gctREG_INDEX        tempRegIndex;
}
slsIOPERAND;

void
slGetVectorROperandSlice(
    IN slsROPERAND * ROperand,
    IN gctUINT8 StartComponent,
    IN gctUINT8 SliceComponentCount,
    OUT slsROPERAND * ROperandSlice
    );

void
slGetVectorLOperandSlice(
    IN slsLOPERAND * LOperand,
    IN gctUINT8 StartComponent,
    IN gctUINT8 SliceComponentCount,
    OUT slsLOPERAND * LOperandSlice
    );

#define slmROPERAND_vectorComponent_GET(component, vector, idx) \
  slGetVectorROperandSlice(vector, idx, 1, component)

#define slmLOPERAND_vectorComponent_GET(component, vector, idx) \
  slGetVectorLOperandSlice(vector, idx, 1, component)

#define slsIOPERAND_Initialize(operand, _dataType, _precision, _tempRegIndex) \
    do \
    { \
        (operand)->dataType    = (_dataType); \
        (operand)->precision    = (_precision); \
        (operand)->tempRegIndex    = (_tempRegIndex); \
    } \
    while (gcvFALSE)

#define slsIOPERAND_New(compiler, operand, _dataType, _precision) \
    do \
    { \
        (operand)->dataType        = (_dataType); \
        (operand)->precision        = (_precision); \
        (operand)->tempRegIndex    = slNewTempRegs((compiler), gcGetDataTypeSize(_dataType)); \
    } \
    while (gcvFALSE)

#define slsIOPERAND_InitializeAsMatrixColumn(operand, _matrixOperand, _matrixIndex) \
    do \
    { \
        (operand)->dataType        = gcGetMatrixColumnDataType((_matrixOperand)->dataType); \
        (operand)->precision        = (_matrixOperand)->precision; \
        (operand)->tempRegIndex    = (gctREG_INDEX) ((_matrixOperand)->tempRegIndex + (_matrixIndex)); \
    } \
    while (gcvFALSE)

gceSTATUS
slGenAssignCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN slsLOPERAND * LOperand,
    IN slsROPERAND * ROperand
    );

gceSTATUS
slGenArithmeticExprCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN slsIOPERAND * IOperand,
    IN slsROPERAND * ROperand0,
    IN slsROPERAND * ROperand1
    );

gceSTATUS
slGenGenericCode1(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN slsIOPERAND * IOperand,
    IN slsROPERAND * ROperand
    );

gceSTATUS
slGenGenericNullTargetCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN slsROPERAND * ROperand0,
    IN slsROPERAND * ROperand1
    );

gceSTATUS
slGenAtomicCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN slsIOPERAND *IOperand,
    IN slsROPERAND *ROperand0,
    IN slsROPERAND *ValOperand
    );

gceSTATUS
slGenGenericCode2(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN slsIOPERAND * IOperand,
    IN slsROPERAND * ROperand0,
    IN slsROPERAND * ROperand1
    );

gceSTATUS
slGenGenericCode2Atomic(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN slsIOPERAND * IOperand,
    IN slsROPERAND * ROperand0,
    IN slsROPERAND * ROperand1
    );

gceSTATUS
slGenGenericCode2WithFormat(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode,
    IN slsIOPERAND * IOperand,
    IN slsROPERAND * ROperand0,
    IN slsROPERAND * ROperand1,
    IN gcSL_FORMAT Format
    );

gceSTATUS
slGenGenericCode3(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleOPCODE Opcode1,
    IN sleOPCODE Opcode2,
    IN slsIOPERAND * IOperand,
    IN slsROPERAND * ROperand0,
    IN slsROPERAND * ROperand1,
    IN slsROPERAND * ROperand2
    );

gceSTATUS
slGenGenericCode3AtomicCmpXchg(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN slsIOPERAND * IOperand,
    IN slsROPERAND * ROperand0,
    IN slsROPERAND * ROperand1,
    IN slsROPERAND * ROperand2
    );

gceSTATUS
slGenTestJumpCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctLABEL Label,
    IN gctBOOL TrueJump,
    IN slsROPERAND * ROperand
    );

gceSTATUS
slGenCompareJumpCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctLABEL Label,
    IN gctBOOL TrueJump,
    IN sleCONDITION CompareCondition,
    IN slsROPERAND * ROperand0,
    IN slsROPERAND * ROperand1
    );

gceSTATUS
slGenSelectExprCode(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN slsIOPERAND * IOperand,
    IN slsROPERAND * Cond,
    IN slsROPERAND * ROperand0,
    IN slsROPERAND * ROperand1
);

typedef struct _slsSELECTION_CONTEXT
{
    gctBOOL    hasFalseOperand;
    gctBOOL    isNegativeCond;
    gctLABEL   endLabel;
    gctLABEL   beginLabelOfFalseOperand;
}
slsSELECTION_CONTEXT;

gceSTATUS
slDefineSelectionBegin(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN gctBOOL HasFalseOperand,
    OUT slsSELECTION_CONTEXT * SelectionContext
    );

gceSTATUS
slDefineSelectionEnd(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN slsSELECTION_CONTEXT * SelectionContext
    );

gceSTATUS
slGenSelectionTestConditionCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN slsSELECTION_CONTEXT * SelectionContext,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN slsROPERAND * ROperand
    );

gceSTATUS
slGenSelectionCompareConditionCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN slsSELECTION_CONTEXT * SelectionContext,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleCONDITION CompareCondition,
    IN slsROPERAND * ROperand0,
    IN slsROPERAND * ROperand1
    );

gceSTATUS
slDefineSelectionTrueOperandBegin(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN slsSELECTION_CONTEXT * SelectionContext
    );

gceSTATUS
slDefineSelectionTrueOperandEnd(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN slsSELECTION_CONTEXT * SelectionContext,
    IN gctBOOL HasReturn
    );

gceSTATUS
slDefineSelectionFalseOperandBegin(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN slsSELECTION_CONTEXT * SelectionContext
    );

gceSTATUS
slDefineSelectionFalseOperandEnd(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN slsSELECTION_CONTEXT * SelectionContext
    );

gceSTATUS
slAddUnusedOutputPatch(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN slsNAME_SPACE* globalNameSpace
    );

gceSTATUS
slAddUnusedInputPatch(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN slsNAME_SPACE* globalNameSpace
    );

gceSTATUS
slPackSSBOWithSharedOrStd140OrStd430(
    IN sloCOMPILER Compiler,
    IN slsNAME_SPACE* globalNameSpace
    );

typedef struct _slsITERATION_CONTEXT
{
    struct _slsITERATION_CONTEXT *    prevContext;

    gctBOOL                            isUnrolled;

    union
    {
        struct
        {
            slsNAME *                loopIndexName;

            sluCONSTANT_VALUE        loopIndexValue;

            gctLABEL                bodyEndLabel;
        }
        unrolledInfo;

        struct
        {
            gctBOOL                    isTestFirst;

            gctBOOL                    hasRestExpr;

            gctLABEL                loopBeginLabel;

            gctLABEL                restBeginLabel;
        }
        genericInfo;
    }
    u;

    gctLABEL                        endLabel;
}
slsITERATION_CONTEXT;

typedef struct _slsFUNC_DEF_CONTEXT
{
    gctBOOL                isMain;

    union
    {
        gctLABEL        mainEndLabel;

        sloIR_SET        funcBody;
    }
    u;
}
slsFUNC_DEF_CONTEXT;

/* CODE_GENERATOR */
typedef enum _sleGEN_CODE_HINT
{
    slvGEN_GENERIC_CODE,
    slvGEN_INDEX_CODE,

    slvEVALUATE_ONLY
}
sleGEN_CODE_HINT;

typedef struct _slsGEN_CODE_PARAMETERS
{
    gctBOOL            needLOperand;
    gctBOOL            needROperand;
    sleGEN_CODE_HINT   hint;
    sloIR_CONSTANT     constant;
    gctUINT            offsetInParent;
    gctUINT            operandCount;
    gcSHADER_TYPE *    dataTypes;
    slsLOPERAND *      lOperands;
    slsROPERAND *      rOperands;
    slsNAME            *constantVariable;
    gctBOOL            treatFloatAsInt;
    gctBOOL            genTexldU;
}
slsGEN_CODE_PARAMETERS;

#define slsGEN_CODE_PARAMETERS_Initialize(parameters, _needLOperand, _needROperand) \
    do \
    { \
        (parameters)->needLOperand        = (_needLOperand); \
        (parameters)->needROperand        = (_needROperand); \
        (parameters)->hint                = slvGEN_GENERIC_CODE; \
        (parameters)->constant            = gcvNULL; \
        (parameters)->operandCount        = 0; \
        (parameters)->offsetInParent      = 0; \
        (parameters)->dataTypes           = gcvNULL; \
        (parameters)->lOperands           = gcvNULL; \
        (parameters)->rOperands           = gcvNULL; \
        (parameters)->constantVariable    = gcvNULL; \
        (parameters)->treatFloatAsInt     = gcvFALSE; \
        (parameters)->genTexldU           = gcvFALSE; \
    } \
    while (gcvFALSE)

#define slsGEN_CODE_PARAMETERS_Finalize(parameters) \
    do \
    { \
        if ((parameters)->constant != gcvNULL) \
        { \
            gcmVERIFY_OK(sloIR_OBJECT_Destroy(Compiler, &(parameters)->constant->exprBase.base)); \
        } \
        \
        if ((parameters)->dataTypes != gcvNULL) \
        { \
            gcmVERIFY_OK(sloCOMPILER_Free(Compiler, (parameters)->dataTypes)); \
        } \
        \
        if ((parameters)->lOperands != gcvNULL) \
        { \
            gcmVERIFY_OK(sloCOMPILER_Free(Compiler, (parameters)->lOperands)); \
        } \
        \
        if ((parameters)->rOperands != gcvNULL) \
        { \
            gcmVERIFY_OK(sloCOMPILER_Free(Compiler, (parameters)->rOperands)); \
        } \
    } \
    while (gcvFALSE)

#define slsGEN_CODE_PARAMETERS_MoveOperands(parameters0, parameters1) \
    do \
    { \
        *(parameters0)                = *(parameters1); \
        (parameters1)->dataTypes    = gcvNULL; \
        (parameters1)->lOperands    = gcvNULL; \
        (parameters1)->rOperands    = gcvNULL; \
    } \
    while (gcvFALSE)

struct _slsMATRIX_ARR_INDEX;
typedef struct _slsMATRIX_ARR_INDEX
{
    slsSLINK_NODE node;
    slsNAME *arrIndexVar;
    gctREG_INDEX indexReg[sldMAX_VECTOR_COMPONENT - 1];
}
slsMATRIX_ARR_INDEX;

/* sloCODE_GENERATOR */
struct _sloCODE_GENERATOR
{
    slsVISITOR              visitor;
    slsFUNC_DEF_CONTEXT     currentFuncDefContext;
    slsITERATION_CONTEXT *  currentIterationContext;
    gctINT                  layoutLocation;
    gctINT                  layoutUniformLocation;
    gctBOOL                 createDefaultUBO;

    gctUINT                 attributeCount;
    gctUINT                 uniformCount;
    gctUINT                 variableCount;
    gctUINT                 outputCount;
    gctUINT                 functionCount;
    gctUINT                 opcodeCount[slvOPCODE_MAXOPCODE];
};

gceSTATUS
sloIR_ROperandComponentSelect(
    IN sloCOMPILER Compiler,
    IN slsROPERAND *From,
    IN slsCOMPONENT_SELECTION ComponentSelection,
    OUT slsROPERAND *To
    );

gceSTATUS
sloIR_LOperandComponentSelect(
    IN sloCOMPILER Compiler,
    IN slsLOPERAND *From,
    IN slsCOMPONENT_SELECTION ComponentSelection,
    OUT slsLOPERAND *To
    );

gceSTATUS
slsROPERAND_ChangeDataTypeFamily(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctBOOL TreatFloatAsInt,
    IN gcSHADER_TYPE NewDataType,
    IN OUT slsROPERAND * ROperand
    );

gceSTATUS
sloIR_AllocObjectPointerArrays(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator
    )
;

gceSTATUS
sloCODE_GENERATOR_Construct(
    IN sloCOMPILER Compiler,
    OUT sloCODE_GENERATOR * CodeGenerator
    );

gceSTATUS
sloCODE_GENERATOR_Destroy(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator
    );

gceSTATUS
slAllocSamplerLevelBaseSize(
    IN sloCOMPILER Compiler,
    IN slsNAME * Sampler,
    IN gcSHADER_TYPE BinaryDataType
    );

gceSTATUS
slAllocImageSizeUniform(
    IN sloCOMPILER Compiler,
    IN slsNAME * Sampler,
    IN gcSHADER_TYPE BinaryDataType
    );

gceSTATUS
slAllocSamplerLodMinMax(
    IN sloCOMPILER Compiler,
    IN slsNAME * Sampler
    );

gceSTATUS
slGenDefineUnrolledIterationBegin(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN slsNAME * LoopIndexName,
    IN gctBOOL OnlyDoTheCheck,
    OUT slsITERATION_CONTEXT * CurrentIterationContext
    );

gceSTATUS
slGenDefineUnrolledIterationEnd(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN gctBOOL OnlyDoTheCheck
    );

#endif /* __gc_glsl_gen_code_h_ */
