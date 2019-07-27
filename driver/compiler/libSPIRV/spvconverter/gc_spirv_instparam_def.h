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


    /* SpvOpNop = 0 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassMisc,
        __SpvEmitNop,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpUndef = 1 */
    {
        gcvFALSE, gcvTRUE, gcvTRUE, OpClassConstant,
        __SpvEmitUndef,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpSourceContinued = 2 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassDebug,
        __SpvEmitNop,
        1,
        { OperandLiteralString, },
        { "Continued Source", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpSource = 3 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassDebug,
        gcvNULL,
        4,
        { OperandSource, OperandLiteralNumber, OperandId, OperandLiteralString, },
        { "", "'Version'", "'File'", "'Source'" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpSourceExtension = 4 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassDebug,
        gcvNULL,
        1,
        { OperandLiteralString, },
        { "'Extension'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpName = 5 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassDebug,
        __SpvEmitName,
        2,
        { OperandId, OperandLiteralString, },
        { "'Target'", "'Name'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpMemberName = 6 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassDebug,
        __SpvEmitName,
        3,
        { OperandId, OperandLiteralNumber, OperandLiteralString, },
        { "'Type'", "'Member'", "'Name'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpString = 7 */
    {
        gcvFALSE, gcvFALSE, gcvTRUE, OpClassDebug,
        __SpvEmitNop,
        1,
        { OperandLiteralString, },
        { "'String'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpLine = 8 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassDebug,
        __SpvEmitNop,
        3,
        { OperandId, OperandLiteralNumber, OperandLiteralNumber, },
        { "'File'", "'Line'", "'Column'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* Unknown = 9 */
    SPV_MISSING_OP,

    /* SpvOpExtension = 10 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassExtension,
        gcvNULL,
        1,
        { OperandLiteralString, },
        { "'Name'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpExtInstImport = 11 */
    {
        gcvFALSE, gcvFALSE, gcvTRUE, OpClassExtension,
        __SpvEmitExtInst,
        1,
        { OperandLiteralString, },
        { "'Name'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpExtInst = 12 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassExtension,
        __SpvEmitIntrisicFunction,
        3,
        { OperandId, OperandLiteralNumber, OperandVariableIds, },
        { "'Set'", "'Instruction'", "'Operand 1', +\n'Operand 2', +\n...", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* Unknown = 13 */
    SPV_MISSING_OP,

    /* SpvOpMemoryModel = 14 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassMode,
        gcvNULL,
        2,
        { OperandAddressing, OperandMemory, },
        { "", "", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpEntryPoint = 15 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassMode,
        gcvNULL,
        4,
        { OperandExecutionModel, OperandId, OperandLiteralString, OperandVariableIds },
        { "", "'Entry Point'", "'Name'", "Interface", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpExecutionMode = 16 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassMode,
        gcvNULL,
        3,
        { OperandId, OperandExecutionMode, OperandOptionalLiteral },
        { "'Entry Point'", "'Mode'", "See Execution Mode" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpCapability = 17 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassMode,
        gcvNULL,
        1,
        { OperandCapability },
        { "Capablity" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* Unknown = 18 */
    SPV_MISSING_OP,

    /* SpvOpTypeVoid = 19 */
    {
        gcvFALSE, gcvFALSE, gcvTRUE, OpClassType,
        __SpvEmitType,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpTypeBool = 20 */
    {
        gcvFALSE, gcvFALSE, gcvTRUE, OpClassType,
        __SpvEmitType,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpTypeInt = 21 */
    {
        gcvFALSE, gcvFALSE, gcvTRUE, OpClassType,
        __SpvEmitType,
        2,
        { OperandLiteralNumber, OperandLiteralNumber, },
        { "'Width'", "'Signedness'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpTypeFloat = 22 */
    {
        gcvFALSE, gcvFALSE, gcvTRUE, OpClassType,
        __SpvEmitType,
        1,
        { OperandLiteralNumber, },
        { "'Width'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpTypeVector = 23 */
    {
        gcvFALSE, gcvFALSE, gcvTRUE, OpClassType,
        __SpvEmitType,
        2,
        { OperandId, OperandLiteralNumber, },
        { "'Component Type'", "'Component Count'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpTypeMatrix = 24 */
    {
        gcvFALSE, gcvFALSE, gcvTRUE, OpClassType,
        __SpvEmitType,
        2,
        { OperandId, OperandLiteralNumber, },
        { "'Column Type'", "'Column Count'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpTypeImage = 25 */
    {
        gcvFALSE, gcvFALSE, gcvTRUE, OpClassType,
        __SpvEmitType,
        8,
        { OperandId, OperandDimensionality, OperandLiteralNumber, OperandLiteralNumber, OperandLiteralNumber, OperandLiteralNumber, OperandSamplerImageFormat, OperandAccessQualifier },
        { "Sampled Type", "", "Depth", "Arrayed", "MS", "Sampled", "", "Access Qualifier" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpTypeSampler = 26 */
    {
        gcvFALSE, gcvFALSE, gcvTRUE, OpClassType,
        __SpvEmitType,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpTypeSampledImage = 27 */
    {
        gcvFALSE, gcvFALSE, gcvTRUE, OpClassType,
        __SpvEmitType,
        1,
        { OperandId },
        { "Image Type" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpTypeArray = 28 */
    {
        gcvFALSE, gcvFALSE, gcvTRUE, OpClassType,
        __SpvEmitType,
        2,
        { OperandId, OperandId, },
        { "'Element Type'", "'Length'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpTypeRuntimeArray = 29 */
    {
        gcvFALSE, gcvFALSE, gcvTRUE, OpClassType,
        __SpvEmitType,
        1,
        { OperandId, },
        { "'Element Type'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpTypeStruct = 30 */
    {
        gcvFALSE, gcvFALSE, gcvTRUE, OpClassType,
        __SpvEmitType,
        1,
        { OperandVariableIds, },
        { "'Member 0 type', +\n'member 1 type', +\n...", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpTypeOpaque = 31 */
    {
        gcvFALSE, gcvFALSE, gcvTRUE, OpClassType,
        __SpvEmitType,
        1,
        { OperandLiteralString, },
        { "The name of the opaque type.", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpTypePointer = 32 */
    {
        gcvFALSE, gcvFALSE, gcvTRUE, OpClassType,
        __SpvEmitType,
        2,
        { OperandStorage, OperandId, },
        { "", "'Type'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpTypeFunction = 33 */
    {
        gcvFALSE, gcvFALSE, gcvTRUE, OpClassType,
        __SpvEmitType,
        2,
        { OperandId, OperandVariableIds, },
        { "'Return Type'", "'Parameter 0 Type', +\n'Parameter 1 Type', +\n...", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpTypeEvent = 34 */
    {
        gcvFALSE, gcvFALSE, gcvTRUE, OpClassType,
        __SpvEmitType,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpTypeDeviceEvent = 35 */
    {
        gcvFALSE, gcvFALSE, gcvTRUE, OpClassType,
        __SpvEmitType,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpTypeReserveId = 36 */
    {
        gcvFALSE, gcvFALSE, gcvTRUE, OpClassType,
        __SpvEmitType,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpTypeQueue = 37 */
    {
        gcvFALSE, gcvFALSE, gcvTRUE, OpClassType,
        __SpvEmitType,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpTypePipe = 38 */
    {
        gcvFALSE, gcvFALSE, gcvTRUE, OpClassType,
        __SpvEmitType,
        1,
        { OperandAccessQualifier, },
        { "'Qualifier'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpTypeForwardPointer = 39 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassType,
        __SpvEmitType,
        2,
        { OperandId, OperandStorage, },
        { "Pointer Type", "" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* Unknown = 40 */
    SPV_MISSING_OP,

    /* SpvOpConstantTrue = 41 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConstant,
        __SpvEmitConstant,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpConstantFalse = 42 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConstant,
        __SpvEmitConstant,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpConstant = 43 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConstant,
        __SpvEmitConstant,
        1,
        { OperandVariableLiterals, },
        { "'Value'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpConstantComposite = 44 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConstant,
        __SpvEmitConstant,
        1,
        { OperandVariableIds, },
        { "'Constituents'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpConstantSampler = 45 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConstant,
        __SpvEmitConstant,
        3,
        { OperandSamplerAddressingMode, OperandLiteralNumber, OperandSamplerFilterMode, },
        { "", "'Param'", "", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpConstantNull = 46 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConstant,
        __SpvEmitConstant,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* Unknown = 47 */
    SPV_MISSING_OP,

    /* SpvOpSpecConstantTrue = 48 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConstant,
        __SpvEmitConstant,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpSpecConstantFalse = 49 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConstant,
        __SpvEmitConstant,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpSpecConstant = 50 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConstant,
        __SpvEmitConstant,
        1,
        { OperandVariableLiterals, },
        { "'Value'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpSpecConstantComposite = 51 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConstant,
        __SpvEmitConstant,
        1,
        { OperandVariableIds, },
        { "'Constituents'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpSpecConstantOp = 52 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConstant,
        __SpvEmitSpecConstantOp,
        2,
        { OperandLiteralNumber, OperandVariableIds },
        { "OpCode", "Operands" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* Unknown = 53 */
    SPV_MISSING_OP,

    /* SpvOpFunction = 54 */
    {
        gcvFALSE, gcvTRUE, gcvTRUE, OpClassFunction,
        __SpvEmitFunction,
        2,
        { OperandFunction, OperandId, },
        { "", "'Function Type'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpFunctionParameter = 55 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassFunction,
        __SpvEmitFunctionParameter,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpFunctionEnd = 56 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassFunction,
        __SpvEmitFunctionEnd,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpFunctionCall = 57 */
    {
        gcvFALSE, gcvTRUE, gcvTRUE, OpClassFunction,
        __SpvEmitFunctionCall,
        2,
        { OperandId, OperandVariableIds, },
        { "'Function'", "'Argument 0', +\n'Argument 1', +\n...", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* Unknown = 58 */
    SPV_MISSING_OP,

    /* SpvOpVariable = 59 */
    {
        gcvFALSE, gcvTRUE, gcvTRUE, OpClassMemory,
        __SpvEmitVariable,
        2,
        { OperandStorage, OperandId, },
        { "", "'Initializer'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageTexelPointer = 60 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassMemory,
        __SpvEmitIntrisicCall,
        3,
        { OperandId, OperandId, OperandId, },
        { "Image", "Coordinate", "Sample", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpLoad = 61 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassMemory,
        __SpvEmitLoad,
        2,
        { OperandId, OperandMemoryAccess, },
        { "'Pointer'", "Optional Memory Access" },
        { gcvNULL },
        VIR_OP_MOV, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpStore = 62 */
    {
        gcvTRUE, gcvFALSE, gcvFALSE, OpClassMemory,
        __SpvEmitStore,
        3,
        { OperandId, OperandId, OperandMemoryAccess },
        { "'Pointer'", "'Object'", "Optional Memory Access" },
        { gcvNULL },
        VIR_OP_MOV, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpCopyMemory = 63 */
    {
        gcvTRUE, gcvFALSE, gcvFALSE, OpClassMemory,
        __SpvEmitCopyMemory,
        3,
        { OperandId, OperandId, OperandMemoryAccess },
        { "'Target'", "'Source'", "Optional Memory Access" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpCopyMemorySized = 64 */
    {
        gcvTRUE, gcvFALSE, gcvFALSE, OpClassMemory,
        __SpvEmitCopyMemory,
        4,
        { OperandId, OperandId, OperandId, OperandMemoryAccess },
        { "'Target'", "'Source'", "'Size'", "Optional Memory Access" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpAccessChain = 65 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassMemory,
        __SpvEmitAccessChain,
        2,
        { OperandId, OperandVariableIds, },
        { "'Base'", "'Indexes'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpInBoundsAccessChain = 66 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassMemory,
        __SpvEmitAccessChain,
        2,
        { OperandId, OperandVariableIds, },
        { "'Base'", "'Indexes'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpPtrAccessChain = 67 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassMemory,
        __SpvEmitPtrAccessChain,
        3,
        { OperandId, OperandId, OperandVariableIds, },
        { "'Base'", "Element", "'Indexes'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpArrayLength = 68 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassMemory,
        __SpvEmitArrayLength,
        2,
        { OperandId, OperandLiteralNumber, },
        { "'Structure'", "'Array member'", },
        { gcvNULL },
        VIR_OP_MOV, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGenericPtrMemSemantics = 69 */
    {
        gcvFALSE, gcvTRUE, gcvTRUE, OpClassMemory,
        gcvNULL,
        1,
        { OperandId, },
        { "'Pointer'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpInBoundsPtrAccessChain = 70 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassMemory,
        gcvNULL,
        3,
        { OperandId, OperandId, OperandVariableIds },
        { "Base", "Element", "Indexes" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpDecorate = 71 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassAnnotate,
        __SpvEmitDecorator,
        3,
        { OperandId, OperandDecoration, OperandVariableLiterals, },
        { "'Target'", "", "See <<Decoration,'Decoration'>>.", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpMemberDecorate = 72 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassAnnotate,
        __SpvEmitDecorator,
        4,
        { OperandId, OperandLiteralNumber, OperandDecoration, OperandVariableLiterals, },
        { "'Structure Type'", "'Member'", "", "See <<Decoration,'Decoration'>>.", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpDecorationGroup = 73 */
    {
        gcvFALSE, gcvFALSE, gcvTRUE, OpClassAnnotate,
        __SpvEmitDecorator,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGroupDecorate = 74 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassAnnotate,
        __SpvEmitDecorator,
        2,
        { OperandId, OperandVariableIds, },
        { "'Decoration Group'", "'Targets'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGroupMemberDecorate = 75 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassAnnotate,
        __SpvEmitDecorator,
        2,
        { OperandId, OperandVariableIdLiteral },
        { "'Decoration Group'", "Targets", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* Unknown = 76 */
    SPV_MISSING_OP,

    /* SpvOpVectorExtractDynamic = 77 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassComposite,
        __SpvEmitVectorExtractDynamic,
        2,
        { OperandId, OperandId, },
        { "'Vector'", "'Index'", },
        { gcvNULL },
        VIR_OP_MOV, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpVectorInsertDynamic = 78 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassComposite,
        __SpvEmitVectorInsertDynamic,
        3,
        { OperandId, OperandId, OperandId, },
        { "'Vector'", "'Component'", "'Index'", },
        { gcvNULL },
        VIR_OP_MOV, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpVectorShuffle = 79 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassComposite,
        __SpvEmitVectorShuffle,
        3,
        { OperandId, OperandId, OperandVariableLiterals, },
        { "'Vector 1'", "'Vector 2'", "'Components'", },
        { gcvNULL },
        VIR_OP_MOV, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpCompositeConstruct = 80 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassComposite,
        __SpvEmitCompositeConstruct,
        1,
        { OperandVariableIds, },
        { "'Constituents'", },
        { gcvNULL },
        VIR_OP_MOV, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpCompositeExtract = 81 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassComposite,
        __SpvEmitCompositeExtract,
        2,
        { OperandId, OperandVariableLiterals, },
        { "'Composite'", "'Indexes'", },
        { gcvNULL },
        VIR_OP_MOV, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpCompositeInsert = 82 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassComposite,
        __SpvEmitCompositeInsert,
        3,
        { OperandId, OperandId, OperandVariableLiterals, },
        { "'Object'", "'Composite'", "'Indexes'", },
        { gcvNULL },
        VIR_OP_MOV, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpCopyObject = 83 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassComposite,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'Operand'", },
        { gcvNULL },
        VIR_OP_MOV, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpTranspose = 84 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassComposite,
        __SpvEmitIntrisicCall,
        1,
        { OperandId, },
        { "'Matrix'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* Unknown = 85 */
    SPV_MISSING_OP,

    /* SpvOpSampledImage = 86 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        __SpvEmitSampledImage,
        2,
        { OperandId, OperandId, },
        { "Image", "Sampler", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageSampleImplicitLod = 87 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        __SpvEmitImageSample,
        4,
        { OperandId, OperandId, OperandImageOperands, OperandVariableIds },
        { "Sampled Image", "Coordinate", "Optional Image Operands", "" },
        { gcvNULL },
        VIR_OP_TEXLD, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageSampleExplicitLod = 88 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        __SpvEmitImageSample,
        4,
        { OperandId, OperandId, OperandImageOperands, OperandVariableIds },
        { "Sampled Image", "Coordinate", "Optional Image Operands", "" },
        { gcvNULL },
        VIR_OP_TEXLD, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageSampleDrefImplicitLod = 89 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        __SpvEmitImageSample,
        5,
        { OperandId, OperandId, OperandId, OperandImageOperands, OperandVariableIds },
        { "Sampled Image", "Coordinate", "Dref", "Optional Image Operands", "" },
        { gcvNULL },
        VIR_OP_TEXLDPCF, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageSampleDrefExplicitLod = 90 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        __SpvEmitImageSample,
        5,
        { OperandId, OperandId, OperandId, OperandImageOperands, OperandVariableIds },
        { "Sampled Image", "Coordinate", "Dref", "Optional Image Operands", "" },
        { gcvNULL },
        VIR_OP_TEXLDPCF, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageSampleProjImplicitLod = 91 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        __SpvEmitImageSample,
        4,
        { OperandId, OperandId, OperandImageOperands, OperandVariableIds },
        { "Sampled Image", "Coordinate", "Optional Image Operands", "" },
        { gcvNULL },
        VIR_OP_TEXLDPROJ, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageSampleProjExplicitLod = 92 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        __SpvEmitImageSample,
        4,
        { OperandId, OperandId, OperandImageOperands, OperandVariableIds },
        { "Sampled Image", "Coordinate", "Optional Image Operands", "" },
        { gcvNULL },
        VIR_OP_TEXLDPROJ, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageSampleProjDrefImplicitLod = 93 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        __SpvEmitImageSample,
        5,
        { OperandId, OperandId, OperandId, OperandImageOperands, OperandVariableIds },
        { "Sampled Image", "Coordinate", "Dref", "Optional Image Operands", "" },
        { gcvNULL },
        VIR_OP_TEXLDPCFPROJ, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageSampleProjDrefExplicitLod = 94 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        __SpvEmitImageSample,
        5,
        { OperandId, OperandId, OperandId, OperandImageOperands, OperandVariableIds },
        { "Sampled Image", "Coordinate", "Dref", "Optional Image Operands", "" },
        { gcvNULL },
        VIR_OP_TEXLDPCFPROJ, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageFetch = 95 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        __SpvEmitIntrisicCall,
        4,
        { OperandId, OperandId, OperandImageOperands, OperandVariableIds },
        { "Image", "Coordinate", "Optional Image Operands", "" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageGather = 96 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        __SpvEmitImageSample,
        5,
        { OperandId, OperandId, OperandId, OperandImageOperands, OperandVariableIds },
        { "Image", "Coordinate", "Component", "Optional Image Operands", "" },
        { gcvNULL },
        VIR_OP_TEXLD, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageDrefGather = 97 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        __SpvEmitImageSample,
        5,
        { OperandId, OperandId, OperandId, OperandImageOperands, OperandVariableIds },
        { "Sampled Image", "Coordinate", "Dref", "Optional Image Operands", "" },
        { gcvNULL },
        VIR_OP_TEXLDPCF, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageRead = 98 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        __SpvEmitIntrisicCall,
        4,
        { OperandId, OperandId, OperandImageOperands, OperandVariableIds },
        { "Image", "Coordinate", "Optional Image Operands", "" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageWrite = 99 */
    {
        gcvTRUE, gcvFALSE, gcvFALSE, OpClassImage,
        __SpvEmitIntrisicCall,
        5,
        { OperandId, OperandId, OperandId, OperandImageOperands, OperandVariableIds },
        { "Image", "Coordinate", "Texel", "Optional Image Operands", "" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImage = 100 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        __SpvEmitOpImage,
        1,
        { OperandId },
        { "Sampled Image" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageQueryFormat = 101 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        __SpvEmitIntrisicCall,
        1,
        { OperandId },
        { "Image" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageQueryOrder = 102 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        __SpvEmitIntrisicCall,
        1,
        { OperandId },
        { "Image" },
        { gcvNULL },
        VIR_OP_IMG_QUERY, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageQuerySizeLod = 103 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        __SpvEmitIntrisicCall,
        2,
        { OperandId, OperandId },
        { "Image", "Level of Detail" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageQuerySize = 104 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        __SpvEmitIntrisicCall,
        1,
        { OperandId },
        { "Image" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageQueryLod = 105 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        __SpvEmitIntrisicCall,
        2,
        { OperandId, OperandId },
        { "Image", "Coordinate" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageQueryLevels = 106 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        __SpvEmitIntrisicCall,
        1,
        { OperandId },
        { "Image" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageQuerySamples = 107 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        __SpvEmitIntrisicCall,
        1,
        { OperandId },
        { "Image" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* Unknown = 108 */
    SPV_MISSING_OP,

    /* SpvOpConvertFToU = 109 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConvert,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'Float Value'", },
        { gcvNULL },
        VIR_OP_CONVERT, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpConvertFToS = 110 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConvert,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'Float Value'", },
        { gcvNULL },
        VIR_OP_CONVERT, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpConvertSToF = 111 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConvert,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'Signed Value'", },
        { gcvNULL },
        VIR_OP_CONVERT, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpConvertUToF = 112 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConvert,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'Unsigned Value'", },
        { gcvNULL },
        VIR_OP_CONVERT, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpUConvert = 113 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConvert,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'Unsigned Value Convert'", },
        { gcvNULL },
        VIR_OP_CONVERT, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpSConvert = 114 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConvert,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'Signed Value Convert'", },
        { gcvNULL },
        VIR_OP_CONVERT, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpFConvert = 115 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConvert,
        __SpvEmitIntrisicCall,
        1,
        { OperandId, },
        { "'Float Value Convert'", },
        { gcvNULL },
        VIR_OP_CONVERT, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpQuantizeToF16 = 116 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConvert,
        __SpvEmitIntrisicCall,
        1,
        { OperandId, },
        { "Value", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpConvertPtrToU = 117 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConvert,
        gcvNULL,
        1,
        { OperandId, },
        { "'Pointer'", },
        { gcvNULL },
        VIR_OP_MOV, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpSatConvertSToU = 118 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConvert,
        gcvNULL,
        1,
        { OperandId, },
        { "'Signed Value'", },
        { gcvNULL },
        VIR_OP_MOV, VIR_TYPE_UNKNOWN, VIR_MOD_SAT_TO_MAX_UINT
    },

    /* SpvOpSatConvertUToS = 119 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConvert,
        gcvNULL,
        1,
        { OperandId, },
        { "'Unsigned Value'", },
        { gcvNULL },
        VIR_OP_MOV, VIR_TYPE_UNKNOWN, VIR_MOD_SAT_TO_MAX_UINT
    },

    /* SpvOpConvertUToPtr = 120 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConvert,
        gcvNULL,
        1,
        { OperandId, },
        { "'Integer Value'", },
        { gcvNULL },
        VIR_OP_MOV, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpPtrCastToGeneric = 121 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConvert,
        gcvNULL,
        1,
        { OperandId, },
        { "'Pointer'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGenericCastToPtr = 122 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConvert,
        gcvNULL,
        1,
        { OperandId, },
        { "'Pointer'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGenericCastToPtrExplicit = 123 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConvert,
        gcvNULL,
        2,
        { OperandId, OperandStorage },
        { "'Pointer'", "Storage" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpBitcast = 124 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassConvert,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'Operand'", },
        { gcvNULL },
        VIR_OP_BITCAST, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* Unknown = 125 */
    SPV_MISSING_OP,

    /* SpvOpSNegate = 126 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'Operand'", },
        { gcvNULL },
        VIR_OP_NEG, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpFNegate = 127 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'Operand'", },
        { gcvNULL },
        VIR_OP_NEG, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpIAdd = 128 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_ADD, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpFAdd = 129 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_ADD, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpISub = 130 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_SUB, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpFSub = 131 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_SUB, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpIMul = 132 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_MUL, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpFMul = 133 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_MUL, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpUDiv = 134 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_DIV, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpSDiv = 135 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_DIV, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpFDiv = 136 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_DIV, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpUMod = 137 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_MOD, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpSRem = 138 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_REM, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpSMod = 139 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_MOD, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpFRem = 140 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_REM, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpFMod = 141 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_MOD, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpVectorTimesScalar = 142 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Vector'", "'Scalar'", },
        { gcvNULL },
        VIR_OP_MUL, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpMatrixTimesScalar = 143 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Matrix'", "'Scalar'", },
        { gcvNULL },
        VIR_OP_MUL, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpVectorTimesMatrix = 144 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Vector'", "'Matrix'", },
        { gcvNULL },
        VIR_OP_MUL, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpMatrixTimesVector = 145 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Matrix'", "'Vector'", },
        { gcvNULL },
        VIR_OP_MUL, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpMatrixTimesMatrix = 146 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'LeftMatrix'", "'RightMatrix'", },
        { gcvNULL },
        VIR_OP_MUL, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpOuterProduct = 147 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Vector 1'", "'Vector 2'", },
        { gcvNULL },
        VIR_OP_MUL, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpDot = 148 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Vector 1'", "'Vector 2'", },
        { gcvNULL },
        VIR_OP_DOT, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpIAddCarry = 149 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitIntrisicCall,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpISubBorrow = 150 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitIntrisicCall,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpUMulExtended = 151 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitIntrisicCall,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpSMulExtended = 152 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassArithmetic,
        __SpvEmitIntrisicCall,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* Unknown = 153 */
    SPV_MISSING_OP,

    /* SpvOpAny = 154 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'Vector'", },
        { gcvNULL },
        VIR_OP_ANY, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpAll = 155 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'Vector'", },
        { gcvNULL },
        VIR_OP_ALL, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpIsNan = 156 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'x'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpIsInf = 157 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'x'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpIsFinite = 158 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'x'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpIsNormal = 159 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'x'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpSignBitSet = 160 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'x'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpLessOrGreater = 161 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'x'", "'y'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpOrdered = 162 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'x'", "'y'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpUnordered = 163 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'x'", "'y'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpLogicalEqual = 164 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpLogicalNotEqual = 165 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_XOR_BITWISE, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpLogicalOr = 166 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_OR_BITWISE, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpLogicalAnd = 167 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_AND_BITWISE, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpLogicalNot = 168 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'Operand'" },
        { gcvNULL },
        VIR_OP_LOGICAL_NOT, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpSelect = 169 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        3,
        { OperandId, OperandId, OperandId, },
        { "'Condition'", "'Object 1'", "'Object 2'", },
        { gcvNULL },
        VIR_OP_CSELECT, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpIEqual = 170 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpINotEqual = 171 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpUGreaterThan = 172 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpSGreaterThan = 173 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpUGreaterThanEqual = 174 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpSGreaterThanEqual = 175 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpULessThan = 176 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpSLessThan = 177 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpULessThanEqual = 178 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpSLessThanEqual = 179 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpFOrdEqual = 180 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpFUnordEqual = 181 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpFOrdNotEqual = 182 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpFUnordNotEqual = 183 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpFOrdLessThan = 184 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpFUnordLessThan = 185 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpFOrdGreaterThan = 186 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpFUnordGreaterThan = 187 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpFOrdLessThanEqual = 188 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpFUnordLessThanEqual = 189 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpFOrdGreaterThanEqual = 190 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpFUnordGreaterThanEqual = 191 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassRelationalLogical,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* Unknown = 192 */
    SPV_MISSING_OP,

    /* Unknown = 193 */
    SPV_MISSING_OP,

    /* SpvOpShiftRightLogical = 194 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassBit,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Base'", "'Shift'", },
        { gcvNULL },
        VIR_OP_LOGICAL_RSHIFT, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpShiftRightArithmetic = 195 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassBit,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Base'", "'Shift'", },
        { gcvNULL },
        VIR_OP_RSHIFT, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpShiftLeftLogical = 196 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassBit,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Base'", "'Shift'", },
        { gcvNULL },
        VIR_OP_LSHIFT, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpBitwiseOr = 197 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassBit,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_OR_BITWISE, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpBitwiseXor = 198 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassBit,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_XOR_BITWISE, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpBitwiseAnd = 199 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassBit,
        __SpvEmitInstructions,
        2,
        { OperandId, OperandId, },
        { "'Operand 1'", "'Operand 2'", },
        { gcvNULL },
        VIR_OP_AND_BITWISE, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpNot = 200 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassBit,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'Operand'", },
        { gcvNULL },
        VIR_OP_NOT_BITWISE, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpBitFieldInsert = 201 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassBit,
        __SpvEmitInstructions,
        4,
        { OperandId, OperandId, OperandId, OperandId, },
        { "Base", "Insert", "Offset", "Count" },
        { gcvNULL },
        VIR_OP_BITINSERT, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpBitFieldSExtract = 202 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassBit,
        __SpvEmitInstructions,
        3,
        { OperandId, OperandId, OperandId, },
        { "Base", "Offset", "Count" },
        { gcvNULL },
        VIR_OP_BITEXTRACT, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpBitFieldUExtract = 203 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassBit,
        __SpvEmitInstructions,
        3,
        { OperandId, OperandId, OperandId, },
        { "Base", "Offset", "Count" },
        { gcvNULL },
        VIR_OP_BITEXTRACT, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpBitReverse = 204 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassBit,
        __SpvEmitInstructions,
        1,
        { OperandId },
        { "Base" },
        { gcvNULL },
        VIR_OP_BITREV, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpBitCount = 205 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassBit,
        __SpvEmitInstructions,
        1,
        { OperandId },
        { "Base" },
        { gcvNULL },
        VIR_OP_POPCOUNT, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* Unknown = 206 */
    SPV_MISSING_OP,

    /* SpvOpDPdx = 207 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassDerivative,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'P'", },
        { gcvNULL },
        VIR_OP_DSX, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpDPdy = 208 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassDerivative,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'P'", },
        { gcvNULL },
        VIR_OP_DSY, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpFwidth = 209 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassDerivative,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'P'", },
        { gcvNULL },
        VIR_OP_FWIDTH, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpDPdxFine = 210 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassDerivative,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'P'", },
        { gcvNULL },
        VIR_OP_DSX, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpDPdyFine = 211 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassDerivative,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'P'", },
        { gcvNULL },
        VIR_OP_DSY, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpFwidthFine = 212 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassDerivative,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'P'", },
        { gcvNULL },
        VIR_OP_FWIDTH, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpDPdxCoarse = 213 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassDerivative,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'P'", },
        { gcvNULL },
        VIR_OP_DSX, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpDPdyCoarse = 214 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassDerivative,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'P'", },
        { gcvNULL },
        VIR_OP_DSY, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpFwidthCoarse = 215 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassDerivative,
        __SpvEmitInstructions,
        1,
        { OperandId, },
        { "'P'", },
        { gcvNULL },
        VIR_OP_FWIDTH, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* Unknown = 216 */
    SPV_MISSING_OP,

    /* Unknown = 217 */
    SPV_MISSING_OP,

    /* SpvOpEmitVertex = 218 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassPrimitive,
        __SpvEmitVertexPrimitive,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_EMIT0, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpEndPrimitive = 219 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassPrimitive,
        __SpvEmitVertexPrimitive,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_RESTART0, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpEmitStreamVertex = 220 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassPrimitive,
        __SpvEmitVertexPrimitive,
        1,
        { OperandId, },
        { "'Stream'", },
        { gcvNULL },
        VIR_OP_EMIT0, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpEndStreamPrimitive = 221 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassPrimitive,
        __SpvEmitVertexPrimitive,
        1,
        { OperandId, },
        { "'Stream'", },
        { gcvNULL },
        VIR_OP_RESTART0, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* Unknown = 222 */
    SPV_MISSING_OP,

    /* Unknown = 223 */
    SPV_MISSING_OP,

    /* SpvOpControlBarrier = 224 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassBarrier,
        __SpvEmitInstructions,
        3,
        { OperandScope, OperandScope, OperandMemorySemantics, },
        { "'Execution'", "'Memory'", "'Semantics'", },
        { gcvNULL },
        VIR_OP_BARRIER, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpMemoryBarrier = 225 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassBarrier,
        __SpvEmitInstructions,
        2,
        { OperandScope, OperandMemorySemantics, },
        { "'Memory'", "'Semantics'", },
        { gcvNULL },
        VIR_OP_MEM_BARRIER, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* Unknown = 226 */
    SPV_MISSING_OP,

    /* SpvOpAtomicLoad = 227 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassAtomic,
        __SpvEmitAtomic,
        3,
        { OperandId, OperandScope, OperandMemorySemantics, },
        { "'Pointer'", "'Scope'", "'Semantics'", },
        { gcvNULL },
        VIR_OP_ATOMADD, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpAtomicStore = 228 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassAtomic,
        __SpvEmitAtomic,
        4,
        { OperandId, OperandScope, OperandMemorySemantics, OperandId, },
        { "'Pointer'", "'Scope'", "'Semantics'", "'Value'", },
        { gcvNULL },
        VIR_OP_ATOMXCHG, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpAtomicExchange = 229 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassAtomic,
        __SpvEmitAtomic,
        4,
        { OperandId, OperandScope, OperandMemorySemantics, OperandId, },
        { "'Pointer'", "'Scope'", "'Semantics'", "'Value'", },
        { gcvNULL },
        VIR_OP_ATOMXCHG, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpAtomicCompareExchange = 230 */
    {
        gcvFALSE, gcvTRUE, gcvTRUE, OpClassAtomic,
        __SpvEmitAtomic,
        6,
        { OperandId, OperandScope, OperandMemorySemantics, OperandMemorySemantics, OperandId, OperandId, },
        { "'Pointer'", "'Scope'", "'Equal'", "'Unequal'", "'Value'", "'Comparator'", },
        { gcvNULL },
        VIR_OP_ATOMCMPXCHG, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpAtomicCompareExchangeWeak = 231 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassAtomic,
        gcvNULL,
        6,
        { OperandId, OperandScope, OperandMemorySemantics, OperandMemorySemantics, OperandId, OperandId, },
        { "'Pointer'", "'Scope'", "'Equal'", "'Unequal'", "'Value'", "'Comparator'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpAtomicIIncrement = 232 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassAtomic,
        __SpvEmitAtomic,
        3,
        { OperandId, OperandScope, OperandMemorySemantics, },
        { "'Pointer'", "'Scope'", "'Semantics'", },
        { gcvNULL },
        VIR_OP_ATOMADD, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpAtomicIDecrement = 233 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassAtomic,
        __SpvEmitAtomic,
        3,
        { OperandId, OperandScope, OperandMemorySemantics, },
        { "'Pointer'", "'Scope'", "'Semantics'", },
        { gcvNULL },
        VIR_OP_ATOMSUB, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpAtomicIAdd = 234 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassAtomic,
        __SpvEmitAtomic,
        4,
        { OperandId, OperandScope, OperandMemorySemantics, OperandId, },
        { "'Pointer'", "'Scope'", "'Semantics'", "'Value'", },
        { gcvNULL },
        VIR_OP_ATOMADD, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpAtomicISub = 235 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassAtomic,
        __SpvEmitAtomic,
        4,
        { OperandId, OperandScope, OperandMemorySemantics, OperandId, },
        { "'Pointer'", "'Scope'", "'Semantics'", "'Value'", },
        { gcvNULL },
        VIR_OP_ATOMSUB, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpAtomicSMin = 236 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassAtomic,
        __SpvEmitAtomic,
        4,
        { OperandId, OperandScope, OperandMemorySemantics, OperandId, },
        { "'Pointer'", "'Scope'", "'Semantics'", "'Value'", },
        { gcvNULL },
        VIR_OP_ATOMMIN, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpAtomicUMin = 237 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassAtomic,
        __SpvEmitAtomic,
        4,
        { OperandId, OperandScope, OperandMemorySemantics, OperandId, },
        { "'Pointer'", "'Scope'", "'Semantics'", "'Value'", },
        { gcvNULL },
        VIR_OP_ATOMMIN, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpAtomicSMax = 238 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassAtomic,
        __SpvEmitAtomic,
        4,
        { OperandId, OperandScope, OperandMemorySemantics, OperandId, },
        { "'Pointer'", "'Scope'", "'Semantics'", "'Value'", },
        { gcvNULL },
        VIR_OP_ATOMMAX, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpAtomicUMax = 239 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassAtomic,
        __SpvEmitAtomic,
        4,
        { OperandId, OperandScope, OperandMemorySemantics, OperandId, },
        { "'Pointer'", "'Scope'", "'Semantics'", "'Value'", },
        { gcvNULL },
        VIR_OP_ATOMMAX, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpAtomicAnd = 240 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassAtomic,
        __SpvEmitAtomic,
        4,
        { OperandId, OperandScope, OperandMemorySemantics, OperandId, },
        { "'Pointer'", "'Scope'", "'Semantics'", "'Value'", },
        { gcvNULL },
        VIR_OP_ATOMAND, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpAtomicOr = 241 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassAtomic,
        __SpvEmitAtomic,
        4,
        { OperandId, OperandScope, OperandMemorySemantics, OperandId, },
        { "'Pointer'", "'Scope'", "'Semantics'", "'Value'", },
        { gcvNULL },
        VIR_OP_ATOMOR, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpAtomicXor = 242 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassAtomic,
        __SpvEmitAtomic,
        4,
        { OperandId, OperandScope, OperandMemorySemantics, OperandId, },
        { "'Pointer'", "'Scope'", "'Semantics'", "'Value'", },
        { gcvNULL },
        VIR_OP_ATOMXOR, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* Unknown = 243 */
    SPV_MISSING_OP,

    /* Unknown = 244 */
    SPV_MISSING_OP,

    /* SpvOpPhi = 245 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassFlowControl,
        __SpvEmitPhi,
        1,
        { OperandVariableIds, },
        { "'Variable, Parent, ...'", },
        { gcvNULL },
        VIR_OP_SPV_PHI, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpLoopMerge = 246 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassFlowControl,
        __SpvEmitLoopMerge,
        3,
        { OperandId, OperandId, OperandVariableIds },
        { "'Merge Block'", "Continue Target", "", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpSelectionMerge = 247 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassFlowControl,
        __SpvEmitNop,
        2,
        { OperandId, OperandSelect },
        { "'Merge Block'", "", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpLabel = 248 */
    {
        gcvFALSE, gcvFALSE, gcvTRUE, OpClassFlowControl,
        __SpvEmitLabel,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_LABEL, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpBranch = 249 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassFlowControl,
        __SpvEmitBranch,
        1,
        { OperandId, },
        { "'Target Label'", },
        { gcvNULL },
        VIR_OP_JMP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpBranchConditional = 250 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassFlowControl,
        __SpvEmitBranchConditional,
        4,
        { OperandId, OperandId, OperandId, OperandVariableLiterals, },
        { "'Condition'", "'True Label'", "'False Label'", "'Branch weights'", },
        { gcvNULL },
        VIR_OP_JMPC, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpSwitch = 251 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassFlowControl,
        __SpvEmitSwitch,
        3,
        { OperandId, OperandId, OperandVariableLiteralId, },
        { "'Selector'", "'Default'", "'Target'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpKill = 252 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassFlowControl,
        __SpvEmitInstructions,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_KILL, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpReturn = 253 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassFlowControl,
        __SpvEmitReturn,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_RET, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpReturnValue = 254 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassFlowControl,
        __SpvEmitReturnValue,
        1,
        { OperandId, },
        { "'Value'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpUnreachable = 255 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassFlowControl,
        __SpvEmitInstructions,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_UNREACHABLE, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpLifetimeStart = 256 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassFlowControl,
        gcvNULL,
        2,
        { OperandId, OperandLiteralNumber, },
        { "'Pointer'", "'Size'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpLifetimeStop = 257 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassFlowControl,
        gcvNULL,
        2,
        { OperandId, OperandLiteralNumber, },
        { "'Pointer'", "'Size'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* Unknown = 258 */
    SPV_MISSING_OP,

    /* SpvOpGroupAsyncCopy = 259 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassGroup,
        gcvNULL,
        6,
        { OperandScope, OperandId, OperandId, OperandId, OperandId, OperandId, },
        { "'Execution'", "'Destination'", "'Source'", "'Num Elements'", "'Stride'", "'Event'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGroupWaitEvents = 260 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassGroup,
        gcvNULL,
        3,
        { OperandScope, OperandId, OperandId, },
        { "'Execution'", "'Num Events'", "'Events List'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGroupAll = 261 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassGroup,
        gcvNULL,
        2,
        { OperandScope, OperandId, },
        { "'Execution'", "'Predicate'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGroupAny = 262 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassGroup,
        gcvNULL,
        2,
        { OperandScope, OperandId, },
        { "'Execution'", "'Predicate'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGroupBroadcast = 263 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassGroup,
        gcvNULL,
        3,
        { OperandScope, OperandId, OperandId, },
        { "'Execution'", "'Value'", "'LocalId'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGroupIAdd = 264 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassGroup,
        gcvNULL,
        3,
        { OperandScope, OperandGroupOperation, OperandId, },
        { "'Execution'", "'Operation'", "'X'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGroupFAdd = 265 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassGroup,
        gcvNULL,
        3,
        { OperandScope, OperandGroupOperation, OperandId, },
        { "'Execution'", "'Operation'", "'X'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGroupFMin = 266 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassGroup,
        gcvNULL,
        3,
        { OperandScope, OperandGroupOperation, OperandId, },
        { "'Execution'", "'Operation'", "X", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGroupUMin = 267 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassGroup,
        gcvNULL,
        3,
        { OperandScope, OperandGroupOperation, OperandId, },
        { "'Execution'", "'Operation'", "'X'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGroupSMin = 268 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassGroup,
        gcvNULL,
        3,
        { OperandScope, OperandGroupOperation, OperandId, },
        { "'Execution'", "'Operation'", "X", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGroupFMax = 269 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassGroup,
        gcvNULL,
        3,
        { OperandScope, OperandGroupOperation, OperandId, },
        { "'Execution'", "'Operation'", "X", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGroupUMax = 270 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassGroup,
        gcvNULL,
        3,
        { OperandScope, OperandGroupOperation, OperandId, },
        { "'Execution'", "'Operation'", "X", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGroupSMax = 271 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassGroup,
        gcvNULL,
        3,
        { OperandScope, OperandGroupOperation, OperandId, },
        { "'Execution'", "'Operation'", "X", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* Unknown = 272 */
    SPV_MISSING_OP,

    /* Unknown = 273 */
    SPV_MISSING_OP,

    /* SpvOpReadPipe = 274 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassPipe,
        gcvNULL,
        4,
        { OperandId, OperandId, OperandId, OperandId },
        { "'Pipe'", "'Pointer'", "'Packet Size'", "'Packet Alignment'" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpWritePipe = 275 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassPipe,
        gcvNULL,
        4,
        { OperandId, OperandId, OperandId, OperandId, },
        { "'Pipe'", "'Pointer'", "'Packet Size'", "'Packet Alignment'" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpReservedReadPipe = 276 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassPipe,
        gcvNULL,
        6,
        { OperandId, OperandId, OperandId, OperandId, OperandId, OperandId },
        { "'Pipe'", "'Reserve Id'", "'Index'", "'Pointer'", "'Packet Size'", "'Packet Alignment'" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpReservedWritePipe = 277 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassPipe,
        gcvNULL,
        6,
        { OperandId, OperandId, OperandId, OperandId, OperandId, OperandId, },
        { "'Pipe'", "'Reserve Id'", "'Index'", "'Pointer'", "'Packet Size'", "'Packet Alignment'" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpReserveReadPipePackets = 278 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassPipe,
        gcvNULL,
        4,
        { OperandId, OperandId, OperandId, OperandId },
        { "'Pipe'", "'Num Packets'", "'Packet Size'", "'Packet Alignment'" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpReserveWritePipePackets = 279 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassPipe,
        gcvNULL,
        4,
        { OperandId, OperandId, OperandId, OperandId },
        { "'Pipe'", "'Num Packets'", "'Packet Size'", "'Packet Alignment'" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpCommitReadPipe = 280 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassPipe,
        gcvNULL,
        4,
        { OperandId, OperandId, OperandId, OperandId },
        { "'Pipe'", "'Reserve Id'", "'Packet Size'", "'Packet Alignment'" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpCommitWritePipe = 281 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassPipe,
        gcvNULL,
        4,
        { OperandId, OperandId, OperandId, OperandId },
        { "'Pipe'", "'Reserve Id'", "'Packet Size'", "'Packet Alignment'" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpIsValidReserveId = 282 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassPipe,
        gcvNULL,
        1,
        { OperandId, },
        { "'Reserve Id'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGetNumPipePackets = 283 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassPipe,
        gcvNULL,
        3,
        { OperandId, OperandId, OperandId },
        { "'Pipe'", "'Packet Size'", "'Packet Alignment'" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGetMaxPipePackets = 284 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassPipe,
        gcvNULL,
        3,
        { OperandId, OperandId, OperandId },
        { "'Pipe'", "'Packet Size'", "'Packet Alignment'" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGroupReserveReadPipePackets = 285 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassPipe,
        gcvNULL,
        5,
        { OperandScope, OperandId, OperandId, OperandId, OperandId },
        { "'Execution'", "'Pipe'", "'Num Packets'", "'Packet Size'", "'Packet Alignment'" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGroupReserveWritePipePackets = 286 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassPipe,
        gcvNULL,
        5,
        { OperandScope, OperandId, OperandId, OperandId, OperandId },
        { "'Execution'", "'Pipe'", "'Num Packets'", "'Packet Size'", "'Packet Alignment'" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGroupCommitReadPipe = 287 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassPipe,
        gcvNULL,
        5,
        { OperandScope, OperandId, OperandId, OperandId, OperandId },
        { "'Execution'", "'Pipe'", "'Reserve Id'", "'Packet Size'", "'Packet Alignment'" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGroupCommitWritePipe = 288 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassPipe,
        gcvNULL,
        5,
        { OperandScope, OperandId, OperandId, OperandId, OperandId },
        { "'Execution'", "'Pipe'", "'Reserve Id'", "'Packet Size'", "'Packet Alignment'" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* Unknown = 289 */
    SPV_MISSING_OP,

    /* Unknown = 290 */
    SPV_MISSING_OP,

    /* SpvOpEnqueueMarker = 291 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassDeviceSideEnqueue,
        gcvNULL,
        4,
        { OperandId, OperandId, OperandId, OperandId, },
        { "'Queue'", "'Num Events'", "'Wait Events'", "'Ret Event'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpEnqueueKernel = 292 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassDeviceSideEnqueue,
        gcvNULL,
        11,
        { OperandId, OperandId, OperandId, OperandId, OperandId, OperandId, OperandId, OperandId, OperandId, OperandId, OperandVariableIds, },
        { "'Queue'", "'Flags'", "'ND Range'", "'Num Events'", "'Wait Events'", "'Ret Event'", "'Invoke'", "'Param'", "'Param Size'", "'Param Align'", "'Local Size'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGetKernelNDrangeSubGroupCount = 293 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassDeviceSideEnqueue,
        gcvNULL,
        5,
        { OperandId, OperandId, OperandId, OperandId, OperandId, },
        { "'ND Range'", "'Invoke'", "'Param'", "'Param Size'", "'Param Align'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGetKernelNDrangeMaxSubGroupSize = 294 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassDeviceSideEnqueue,
        gcvNULL,
        5,
        { OperandId, OperandId, OperandId, OperandId, OperandId, },
        { "'ND Range'", "'Invoke'", "'Param'", "'Param Size'", "'Param Align'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGetKernelWorkGroupSize = 295 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassDeviceSideEnqueue,
        gcvNULL,
        4,
        { OperandId, OperandId, OperandId, OperandId, },
        { "'Invoke'", "'Param'", "'Param Size'", "'Param Align'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGetKernelPreferredWorkGroupSizeMultiple = 296 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassDeviceSideEnqueue,
        gcvNULL,
        4,
        { OperandId, OperandId, OperandId, OperandId, },
        { "'Invoke'", "'Param'", "'Param Size'", "'Param Align'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpRetainEvent = 297 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassDeviceSideEnqueue,
        gcvNULL,
        1,
        { OperandId, },
        { "'Event'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpReleaseEvent = 298 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassDeviceSideEnqueue,
        gcvNULL,
        1,
        { OperandId, },
        { "'Event'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpCreateUserEvent = 299 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassDeviceSideEnqueue,
        gcvNULL,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpIsValidEvent = 300 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassDeviceSideEnqueue,
        gcvNULL,
        1,
        { OperandId, },
        { "'Event'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpSetUserEventStatus = 301 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassDeviceSideEnqueue,
        gcvNULL,
        2,
        { OperandId, OperandId, },
        { "'Event'", "'Status'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpCaptureEventProfilingInfo = 302 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassDeviceSideEnqueue,
        gcvNULL,
        3,
        { OperandId, OperandId, OperandId, },
        { "'Event'", "'Profiling Info'", "'Value'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGetDefaultQueue = 303 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassDeviceSideEnqueue,
        gcvNULL,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpBuildNDRange = 304 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassDeviceSideEnqueue,
        gcvNULL,
        3,
        { OperandId, OperandId, OperandId, },
        { "'GlobalWorkSize'", "'LocalWorkSize'", "'GlobalWorkOffset'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageSparseSampleImplicitLod = 305 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        gcvNULL,
        4,
        { OperandId, OperandId, OperandImageOperands, OperandVariableIds },
        { "Sampled Image", "Coordinate", "", "" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageSparseSampleExplicitLod = 306 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        gcvNULL,
        4,
        { OperandId, OperandId, OperandImageOperands, OperandVariableIds },
        { "Sampled Image", "Coordinate", "", "" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageSparseSampleDrefImplicitLod = 307 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        gcvNULL,
        5,
        { OperandId, OperandId, OperandId, OperandImageOperands, OperandVariableIds },
        { "Sampled Image", "Coordinate", "Dref", "Optional Image Operands", "" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageSparseSampleDrefExplicitLod = 308 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        gcvNULL,
        5,
        { OperandId, OperandId, OperandId, OperandImageOperands, OperandVariableIds },
        { "Sampled Image", "Coordinate", "Dref", "Optional Image Operands", "" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageSparseSampleProjImplicitLod = 309 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        gcvNULL,
        4,
        { OperandId, OperandId, OperandImageOperands, OperandVariableIds },
        { "Sampled Image", "Coordinate", "Optional Image Operands", "" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageSparseSampleProjExplicitLod = 310 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        gcvNULL,
        4,
        { OperandId, OperandId, OperandImageOperands, OperandVariableIds },
        { "Sampled Image", "Coordinate", "Optional Image Operands", "" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageSparseSampleProjDrefImplicitLod = 311 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        gcvNULL,
        5,
        { OperandId, OperandId, OperandId, OperandImageOperands, OperandVariableIds },
        { "Sampled Image", "Coordinate", "Dref", "Optional Image Operands", "" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageSparseSampleProjDrefExplicitLod = 312 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        gcvNULL,
        5,
        { OperandId, OperandId, OperandId, OperandImageOperands, OperandVariableIds },
        { "Sampled Image", "Coordinate", "Dref", "Optional Image Operands", "" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageSparseFetch = 313 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        gcvNULL,
        4,
        { OperandId, OperandId, OperandImageOperands, OperandVariableIds },
        { "Sampled Image", "Coordinate", "Optional Image Operands", "" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageSparseGather = 314 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        gcvNULL,
        5,
        { OperandId, OperandId, OperandId, OperandImageOperands, OperandVariableIds },
        { "Sampled Image", "Coordinate", "Component", "Optional Image Operands", "" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageSparseDrefGather = 315 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        gcvNULL,
        5,
        { OperandId, OperandId, OperandId, OperandImageOperands, OperandVariableIds },
        { "Sampled Image", "Coordinate", "Dref", "Optional Image Operands", "" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageSparseTexelsResident = 316 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        gcvNULL,
        1,
        { OperandId },
        { "Resident Code" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpNoLine = 317 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassDebug,
        __SpvEmitNop,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE,
    },

    /* SpvOpAtomicFlagTestAndSet = 318 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassAtomic,
        __SpvEmitAtomic,
        3,
        { OperandId, OperandScope, OperandMemorySemantics, },
        { "'Pointer'", "'Scope'", "'Semantics'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpAtomicFlagClear = 319 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassAtomic,
        __SpvEmitAtomic,
        3,
        { OperandId, OperandScope, OperandMemorySemantics, },
        { "'Pointer'", "'Scope'", "'Semantics'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpImageSparseRead = 320 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassImage,
        gcvNULL,
        4,
        { OperandId, OperandId, OperandImageOperands, OperandVariableIds },
        { "Image", "Coordinate", "Image Operands", "" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpSizeOf = 321 */
    {
        gcvFALSE, gcvTRUE, gcvTRUE, OpClassMisc,
        __SpvEmitUnsupported,
        1,
        { OperandId },
        { "Pointer" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpTypePipeStorage = 322 */
    {
        gcvFALSE, gcvFALSE, gcvTRUE, OpClassType,
        __SpvEmitUnsupported,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpConstantPipeStorage = 323 */
    {
        gcvFALSE, gcvTRUE, gcvTRUE, OpClassMisc,
        __SpvEmitUnsupported,
        3,
        { OperandLiteralNumber, OperandLiteralNumber, OperandLiteralNumber },
        { "Packet Size", "Packet Alignment", "Capacity" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpCreatePipeFromPipeStorage = 324 */
    {
        gcvFALSE, gcvTRUE, gcvTRUE, OpClassMisc,
        __SpvEmitUnsupported,
        1,
        { OperandId },
        { "Pipe Storage" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGetKernelLocalSizeForSubgroupCount = 325 */
    {
        gcvFALSE, gcvTRUE, gcvTRUE, OpClassMisc,
        __SpvEmitUnsupported,
        5,
        { OperandId, OperandId, OperandId, OperandId, OperandId },
        { "Subgroup Count", "Invoke", "Param", "Param Size", "Param Align"},
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGetKernelMaxNumSubgroups = 326 */
    {
        gcvFALSE, gcvTRUE, gcvTRUE, OpClassMisc,
        __SpvEmitUnsupported,
        4,
        { OperandId, OperandId, OperandId, OperandId },
        { "Invoke", "Param", "Param Size", "Param Align"},
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpTypeNamedBarrier = 327 */
    {
        gcvFALSE, gcvFALSE, gcvTRUE, OpClassType,
        __SpvEmitUnsupported,
        0,
        { OperandNone },
        { gcvNULL },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpNamedBarrierInitialize = 328 */
    {
        gcvFALSE, gcvTRUE, gcvTRUE, OpClassBarrier,
        __SpvEmitUnsupported,
        1,
        { OperandId },
        { "Subgroup Count"},
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpMemoryNamedBarrier = 329 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassBarrier,
        __SpvEmitUnsupported,
        3,
        { OperandId, OperandScope, OperandMemorySemantics, },
        { "Named Barrier", "Memory", "Semantics", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpModuleProcessed = 330 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassDebug,
        gcvNULL,
        1,
        { OperandLiteralString, },
        { "'Process'", },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpExecutionModeId = 331 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassMode,
        gcvNULL,
        3,
        { OperandId, OperandExecutionMode, OperandVariableIds },
        { "Entry Point", "Mode", "See Execution Mode" },
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpDecorateId = 332 */
    {
        gcvFALSE, gcvFALSE, gcvFALSE, OpClassAnnotate,
        __SpvEmitUnsupported,
        3,
        { OperandId, OperandDecoration, OperandVariableIds, },
        { "Target", "Decoration", "See Decoration"},
        { gcvNULL },
        VIR_OP_NOP, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    },

    /* SpvOpGroupNonUniformElect = 333 */
    {
        gcvTRUE, gcvTRUE, gcvTRUE, OpClassNonUniform,
        __SpvEmitInstructions,
        1,
        { OperandScope },
        { "Scope" },
        { gcvNULL },
        VIR_OP_NONUNIFORM_ELECT, VIR_TYPE_UNKNOWN, VIR_MOD_NONE
    }
