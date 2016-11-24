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


/*
    This file is used to define VIR independent types for Spirv
*/

#ifndef __gc_spirv_types_h_
#define __gc_spirv_types_h_

#include "gc_hal_user_precomp.h"
#include <SPIRV/spirv.h>

/* max number of opcode define in spirv.h, it change, so anytime change spirv.h,
   make sure change this accordingly*/
#define SPV_MAX_OPCODE_NUM                          321

#define SPV_MAX_OPERAND_NUM                         16

#define SPV_INVALID_ID                              0

/* For grouping opcodes into subsections */
enum SpvOpcodeClass {
    OpClassMisc,            /* default, until opcode is classified */
    OpClassDebug,
    OpClassAnnotate,
    OpClassExtension,
    OpClassMode,
    OpClassType,
    OpClassConstant,
    OpClassMemory,
    OpClassFunction,
    OpClassImage,
    OpClassConvert,
    OpClassComposite,
    OpClassArithmetic,
    OpClassBit,
    OpClassRelationalLogical,
    OpClassDerivative,
    OpClassFlowControl,
    OpClassAtomic,
    OpClassPrimitive,
    OpClassBarrier,
    OpClassGroup,
    OpClassDeviceSideEnqueue,
    OpClassPipe,

    OpClassCount,
    OpClassMissing
};

typedef enum SpvOpcodeClass SpvOpcodeClass;

/* For parameterizing operands.*/
enum SpvOperandClass {
    OperandNone,
    OperandId,
    OperandVariableIds,
    OperandOptionalLiteral,
    OperandOptionalLiteralString,
    OperandVariableLiterals,
    OperandVariableIdLiteral,
    OperandVariableLiteralId,
    OperandLiteralNumber,
    OperandLiteralString,
    OperandSource,
    OperandExecutionModel,
    OperandAddressing,
    OperandMemory,
    OperandExecutionMode,
    OperandStorage,
    OperandDimensionality,
    OperandSamplerAddressingMode,
    OperandSamplerFilterMode,
    OperandSamplerImageFormat,
    OperandImageChannelOrder,
    OperandImageChannelDataType,
    OperandImageOperands,
    OperandFPFastMath,
    OperandFPRoundingMode,
    OperandLinkageType,
    OperandAccessQualifier,
    OperandFuncParamAttr,
    OperandDecoration,
    OperandBuiltIn,
    OperandSelect,
    OperandLoop,
    OperandFunction,
    OperandMemorySemantics,
    OperandMemoryAccess,
    OperandScope,
    OperandGroupOperation,
    OperandKernelEnqueueFlags,
    OperandKernelProfilingInfo,
    OperandCapability,

    OperandOpcode,

    OperandCount
};

typedef enum SpvOperandClass SpvOperandClass;

enum SpvOperandFormat {
    OpFormat_FLOAT = 0,                     /* 0 */
    OpFormat_INTEGER = 1,                   /* 1 */
    OpFormat_INT32 = 1,                     /* 1 */
    OpFormat_BOOLEAN = 2,                   /* 2 */
    OpFormat_UINT32 = 3,                    /* 3 */
    OpFormat_INT8,                          /* 4 */
    OpFormat_UINT8,                         /* 5 */
    OpFormat_INT16,                         /* 6 */
    OpFormat_UINT16,                        /* 7 */
    OpFormat_INT64,                         /* 8 */
    OpFormat_UINT64,                        /* 9 */
    OpFormat_SNORM8,                        /* 10 */
    OpFormat_UNORM8,                        /* 11 */
    OpFormat_FLOAT16,                       /* 12 */
    OpFormat_FLOAT64,                       /* 13 */    /* Reserved for future enhancement. */
    OpFormat_SNORM16,                       /* 14 */
    OpFormat_UNORM16,                       /* 15 */

    /* the following is extended for OCL 1.2 for type name querying
    the lower 4 bits are defined as above.
    An aggregate format can be formed from or'ing together a value from above
    and another value from below
    */
    OpFormat_VOID = 0x10,                   /* to support OCL 1.2 for querying */
    OpFormat_STRUCT = 0x20,                 /* OCL struct */
    OpFormat_UNION = 0x30,                  /* OCL union */
    OpFormat_ENUM = 0x40,                   /* OCL enum */
    OpFormat_TYPEDEF = 0x50,                /* OCL typedef */
    OpFormat_SAMPLER_T = 0x60,              /* OCL sampler_t */
    OpFormat_SIZE_T = 0x70,                 /* OCL size_t */
    OpFormat_EVENT_T = 0x80,                /* OCL event */
    OpFormat_PTRDIFF_T = 0x90,              /* OCL prtdiff_t */
    OpFormat_INTPTR_T = 0xA0,               /* OCL intptr_t */
    OpFormat_UINTPTR_T = 0xB0,              /* OCL uintptr_t */

    OpFormat_INVALID = 0xFFFFFFFF,
};

typedef enum SpvOperandFormat SpvOperandFormat;

/* we cannot get operand class directly, use this function instead */
SpvOperandClass __SpvGetOperandClassFromOpCode(SpvOp opCode, gctUINT opndIndex);
gctUINT __SpvGetOperandNumFromOpCode(SpvOp opCode);
gctBOOL __SpvOpCodeHasResult(SpvOp opCode);
gctBOOL __SpvOpCodeHasType(SpvOp opCode);

#endif
