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


#include "gc_cl_compiler.h"
#include "gc_hal_user.h"

#define clmOPCODE_U8(opcode, temp, enable) \
gcmONERROR(gcSHADER_AddOpcode(shader, gcSL_##opcode, (tempStart + temp), gcSL_ENABLE_##enable, gcSL_UINT8, gcSHADER_PRECISION_DEFAULT, 0))

#define clmOPCODE_U16(opcode, temp, enable) \
gcmONERROR(gcSHADER_AddOpcode(shader, gcSL_##opcode, (tempStart + temp), gcSL_ENABLE_##enable, gcSL_UINT16, gcSHADER_PRECISION_DEFAULT, 0))

#define clmOPCODE_U32(opcode, temp, enable) \
gcmONERROR(gcSHADER_AddOpcode(shader, gcSL_##opcode, (tempStart + temp), gcSL_ENABLE_##enable, gcSL_UINT32, gcSHADER_PRECISION_DEFAULT, 0))

#define clmOPCODE_S16(opcode, temp, enable) \
gcmONERROR(gcSHADER_AddOpcode(shader, gcSL_##opcode, (tempStart + temp), gcSL_ENABLE_##enable, gcSL_INT16, gcSHADER_PRECISION_DEFAULT, 0))

#define clmOPCODE_S32(opcode, temp, enable) \
gcmONERROR(gcSHADER_AddOpcode(shader, gcSL_##opcode, (tempStart + temp), gcSL_ENABLE_##enable, gcSL_INT32, gcSHADER_PRECISION_DEFAULT, 0))

#define clmOPCODE(opcode, temp, enable) \
gcmONERROR(gcSHADER_AddOpcode(shader, gcSL_##opcode, (tempStart + temp), gcSL_ENABLE_##enable, gcSL_FLOAT, gcSHADER_PRECISION_DEFAULT, 0))

#define clmJUMP(condition, label) \
gcmONERROR(gcSHADER_AddOpcodeConditional(shader, gcSL_JMP, gcSL_##condition, label, 0))

#define clmJUMP_U32(condition, label) \
gcmONERROR(gcSHADER_AddOpcodeConditionalFormatted(shader, gcSL_JMP, gcSL_##condition, gcSL_UINT32, label, 0))

#define clmJUMP_S32(condition, label) \
gcmONERROR(gcSHADER_AddOpcodeConditionalFormatted(shader, gcSL_JMP, gcSL_##condition, gcSL_INT32, label, 0))

#define clmTEMP(temp, swizzle) \
gcmONERROR(gcSHADER_AddSource(shader, gcSL_TEMP, (tempStart + temp), gcSL_SWIZZLE_##swizzle, gcSL_FLOAT, gcSHADER_PRECISION_DEFAULT))

#define clmTEMP_U8(temp, swizzle) \
gcmONERROR(gcSHADER_AddSource(shader, gcSL_TEMP, (tempStart + temp), gcSL_SWIZZLE_##swizzle, gcSL_UINT8, gcSHADER_PRECISION_DEFAULT))

#define clmTEMP_U16(temp, swizzle) \
gcmONERROR(gcSHADER_AddSource(shader, gcSL_TEMP, (tempStart + temp), gcSL_SWIZZLE_##swizzle, gcSL_UINT16, gcSHADER_PRECISION_DEFAULT))

#define clmTEMP_S16(temp, swizzle) \
gcmONERROR(gcSHADER_AddSource(shader, gcSL_TEMP, (tempStart + temp), gcSL_SWIZZLE_##swizzle, gcSL_INT16, gcSHADER_PRECISION_DEFAULT))

#define clmTEMP_U32(temp, swizzle) \
gcmONERROR(gcSHADER_AddSource(shader, gcSL_TEMP, (tempStart + temp), gcSL_SWIZZLE_##swizzle, gcSL_UINT32, gcSHADER_PRECISION_DEFAULT))

#define clmTEMP_S32(temp, swizzle) \
gcmONERROR(gcSHADER_AddSource(shader, gcSL_TEMP, (tempStart + temp), gcSL_SWIZZLE_##swizzle, gcSL_INT32, gcSHADER_PRECISION_DEFAULT))

#define clmFLOAT(value) \
gcmONERROR(gcSHADER_AddSourceConstant(shader, (gctFLOAT) (value)))

#define clmU32(value) \
{ gctUINT32 v = value; gcmONERROR(gcSHADER_AddSourceConstantFormatted(shader, &v, gcSL_UINT32)); } \
for (;;) break

#define clmS32(value) \
{ gctINT32 v = value; gcmONERROR(gcSHADER_AddSourceConstantFormatted(shader, &v, gcSL_INT32)); } \
for (;;) break

#define clmATTRIBUTE_U32(attribute, swizzle) \
gcmONERROR(gcSHADER_AddSourceAttributeFormatted(shader, attribute, gcSL_SWIZZLE_##swizzle, 0, gcSL_UINT32))

#define clmATTRIBUTE_S32(attribute, swizzle) \
gcmONERROR(gcSHADER_AddSourceAttributeFormatted(shader, attribute, gcSL_SWIZZLE_##swizzle, 0, gcSL_INT32))

#define clmUNIFORM_U32(uniform, swizzle) \
gcmONERROR(gcSHADER_AddSourceUniformFormatted(shader, uniform, gcSL_SWIZZLE_##swizzle, 0, gcSL_UINT32))

#define clmUNIFORM_S32(uniform, swizzle) \
gcmONERROR(gcSHADER_AddSourceUniformFormatted(shader, uniform, gcSL_SWIZZLE_##swizzle, 0, gcSL_INT32))

#define clmLABEL(label) \
gcmONERROR(gcSHADER_AddLabel(shader, label))

#define clmKERNEL(name, tempCount) \
gcmONERROR(gcSHADER_AddKernelFunction(shader, name, &kernel)); \
gcmONERROR(gcSHADER_BeginKernelFunction(shader, kernel)); \
tempStart = gcSHADER_NewTempRegs(shader, tempCount, gcSHADER_FLOAT_X4)

#define clmMAIN() \
gcmONERROR(gcSHADER_AddOpcode(shader, gcSL_RET, 0, gcSL_ENABLE_NONE, gcSL_FLOAT, gcSHADER_PRECISION_DEFAULT, 0)); \
gcmONERROR(gcKERNEL_FUNCTION_SetCodeEnd(kernel)); \
{ gctUINT32 label; gcKERNEL_FUNCTION_GetLabel(kernel, &label); \
gcmONERROR(gcSHADER_AddOpcodeConditional(shader, gcSL_CALL, gcSL_ALWAYS, label, 0)); } \
gcmONERROR(gcKERNEL_FUNCTION_AddKernelFunctionProperties(kernel, 1, 3, property)); \
gcmONERROR(gcSHADER_EndKernelFunction(shader, kernel, 0))

#define clmARGUMENT_U32(name, temp, isConst, format, isPointer) \
gcmONERROR(gcKERNEL_FUNCTION_AddArgument(kernel, 0xffff, (tempStart + temp), gcSL_ENABLE_X, gcvTYPE_QUALIFIER_NONE)); \
gcmONERROR(gcKERNEL_FUNCTION_AddUniformArgument(kernel, #name, gcSHADER_INTEGER_X1, 1, &name)); \
gcmONERROR(gcUNIFORM_SetFlags(name, isConst ? gcvUNIFORM_KIND_KERNEL_ARG_CONSTANT : gcvUNIFORM_KIND_KERNEL_ARG)); \
gcmONERROR(gcUNIFORM_SetFormat(name, gcSL_##format, isPointer))

#define clmARGUMENT_IMAGE(name, temp, isConst) \
gcmONERROR(gcKERNEL_FUNCTION_AddArgument(kernel, 0xffff, (tempStart + temp), gcSL_ENABLE_X, gcvTYPE_QUALIFIER_NONE)); \
gcmONERROR(gcKERNEL_FUNCTION_AddUniformArgument(kernel, #name, gcSHADER_IMAGE_2D, 1, &name)); \
gcmONERROR(gcUNIFORM_SetFlags(name, isConst ? gcvUNIFORM_KIND_KERNEL_ARG_CONSTANT : gcvUNIFORM_KIND_KERNEL_ARG)); \
gcmONERROR(gcUNIFORM_SetFormat(name, gcSL_UINT32, gcvTRUE))

#define clmSWIZZLE_MUL(temp, load, swizzle, const, coef, coef_swizzle) \
clmOPCODE_U8(UNPACK, temp, XYZW); clmTEMP_U32(load, swizzle); clmTEMP_U32(const, XYYY); \
clmOPCODE_U8(AND_BITWISE, temp + 1, XYZW); clmTEMP_U8(temp, XYZW); clmU32(0xFFFF); \
clmOPCODE_U32(AND_BITWISE, temp + 2, XYZW); clmTEMP_U8(temp + 1, XYZW); clmU32(0x000000FF); \
clmOPCODE_U8(I2F, temp + 3, XYZW); clmTEMP_U8(temp + 2, XYZW); \
clmOPCODE(MUL, temp + 4, XYZW); clmTEMP(temp + 3, XYZW); clmTEMP(coef, coef_swizzle)

#define clmSWIZZLE_MAD(temp, load, swizzle, const, coef, coef_swizzle, acc) \
clmOPCODE_U8(UNPACK, temp, XYZW); clmTEMP_U32(load, swizzle); clmTEMP_U32(const, XYYY); \
clmOPCODE_U8(AND_BITWISE, temp + 1, XYZW); clmTEMP_U8(temp, XYZW); clmU32(0xFFFF); \
clmOPCODE_U32(AND_BITWISE, temp + 2, XYZW); clmTEMP_U8(temp + 1, XYZW); clmU32(0x000000FF); \
clmOPCODE_U8(I2F, temp + 3, XYZW); clmTEMP_U8(temp + 2, XYZW); \
clmOPCODE(MUL, temp + 4, XYZW); clmTEMP(temp + 3, XYZW); clmTEMP(coef, coef_swizzle); \
clmOPCODE(ADD, temp + 5, XYZW); clmTEMP(acc, XYZW); clmTEMP(temp + 4, XYZW)

struct clsTUNE
{
    const gctCHAR * source;
    const gctCHAR * options;
    gcSHADER (*function)(IN gcSHADER Shader);
};

static gcSHADER clTune_10(IN gcSHADER Shader)
{
    gcSHADER shader = gcvNULL;
    gceSTATUS status;
    gcKERNEL_FUNCTION kernel;
    gctUINT tempStart;
    gcATTRIBUTE global_id;
    gcUNIFORM global_size;
    gctINT property[3] = { 0, 0, 0 };
    gcUNIFORM input, coefficients, output;
    gctUINT32 compilerVersion[2];

    /* Construct a new shader. */
    gcmONERROR(gcSHADER_Construct(gcSHADER_TYPE_CL, &shader));

    /* Set OpenCL arguments. */
    cloCOMPILER_GetVersion(gcvNULL, clvSHADER_TYPE_CL, compilerVersion);
    gcmONERROR(gcSHADER_SetCompilerVersion(shader, compilerVersion));
    gcmONERROR(gcSHADER_SetMaxKernelFunctionArgs(shader, 3));

    /* Add attributes and uniforms. */
    gcmONERROR(gcSHADER_AddAttribute(shader, "#global_id", gcSHADER_INTEGER_X4, 1, gcvFALSE, gcSHADER_SHADER_DEFAULT,
                                     gcSHADER_PRECISION_DEFAULT, &global_id));
    gcmONERROR(gcSHADER_AddUniform(shader, "#global_size", gcSHADER_INTEGER_X4, 1, gcSHADER_PRECISION_DEFAULT, &global_size));
    gcmONERROR(gcUNIFORM_SetFlags(global_size, gcvUNIFORM_KIND_GLOBAL_SIZE));
    gcmONERROR(gcUNIFORM_SetFormat(global_size, gcSL_UINT32, gcvFALSE));

    /*
     *  __kernel void HorizontalGaussFilter(
     *      __global const uchar4 *restrict input,
     *      __constant float *coefficients,
     *      __global uchar4 *output
     *      )
     *  {
     *      const int pX = get_global_id(0);
     *      const int pY = get_global_id(1);
     *
     *      const int widthPrec = get_global_size(0) - 1;
     *      const int pY_width = pY*get_global_size(0);
     *
     *      float4 aggregate = convert_float4(input[pY_width + pX]) * coefficients[samplingOffset];
     *
     *      for(int o = 1; o <= samplingOffset; ++o)
     *      {
     *          aggregate += convert_float4(input[pY_width + max(pX - o, 0)]) * coefficients[samplingOffset - o];
     *          aggregate += convert_float4(input[pY_width + min(pX + o, widthPrec)]) * coefficients[samplingOffset + o];
     *      }
     *
     *      output[pY_width + pX] = convert_uchar4_sat(aggregate);
     *  }
     */
    clmKERNEL("HorizontalGaussFilter", 155);
    clmARGUMENT_U32(input, 0, gcvFALSE, UINT8, gcvTRUE);
    clmARGUMENT_U32(coefficients, 1, gcvTRUE, FLOAT, gcvTRUE);
    clmARGUMENT_U32(output, 2, gcvFALSE, UINT8, gcvTRUE);

    clmOPCODE_U32(MUL, 3, X); clmATTRIBUTE_U32(global_id, YYYY); clmUNIFORM_U32(global_size, XXXX);
    clmOPCODE_U32(ADD, 4, X); clmTEMP_U32(3, XXXX); clmATTRIBUTE_U32(global_id, XXXX);
    clmOPCODE_U32(LSHIFT, 5, X); clmTEMP_U32(4, XXXX); clmU32(2);
    clmJUMP_U32(LESS, 1); clmATTRIBUTE_U32(global_id, XXXX); clmU32(5);
    clmOPCODE_S32(SUB, 6, X); clmUNIFORM_S32(global_size, XXXX); clmS32(5);
    clmJUMP_S32(LESS, 2); clmATTRIBUTE_S32(global_id, XXXX); clmTEMP_S32(6, XXXX);
    clmLABEL(1);
    clmOPCODE(LOAD, 7, XYZW); clmUNIFORM_U32(coefficients, XXXX); clmU32(4 * 4);
    clmOPCODE_U32(MUL, 79, X); clmATTRIBUTE_U32(global_id, YYYY); clmUNIFORM_U32(global_size, XXXX);
    clmOPCODE_U32(LSHIFT, 8, X); clmTEMP_U32(79, XXXX); clmU32(2);
    clmOPCODE_U8(LOAD, 9, XYZW); clmUNIFORM_U32(input, XXXX); clmTEMP_U32(5, XXXX);
    clmOPCODE_U8(I2F, 10, XYZW); clmTEMP_U8(9, XYZW);
    clmOPCODE(MUL, 11, XYZW); clmTEMP(10, XYZW); clmTEMP(7, YYYY);
    clmOPCODE_S32(SUB, 12, X); clmTEMP_S32(5, XXXX); clmS32(1 * 4);
    clmOPCODE_S32(MAX, 13, X); clmTEMP_S32(12, XXXX); clmTEMP_S32(8, XXXX);
    clmOPCODE_U8(LOAD, 14, XYZW); clmUNIFORM_U32(input, XXXX); clmTEMP_U32(13, XXXX);
    clmOPCODE_U8(I2F, 15, XYZW); clmTEMP_U8(14, XYZW);
    clmOPCODE(MUL, 16, XYZW); clmTEMP(15, XYZW); clmTEMP(7, XXXX);
    clmOPCODE(ADD, 17, XYZW); clmTEMP(11, XYZW); clmTEMP(16, XYZW);
    clmOPCODE(LOAD, 18, XYZW); clmUNIFORM_U32(coefficients, XXXX); clmU32(0 * 4);
    clmOPCODE_S32(SUB, 19, X); clmTEMP_S32(5, XXXX); clmS32(2 * 4);
    clmOPCODE_S32(MAX, 20, X); clmTEMP_S32(19, XXXX); clmTEMP_S32(8, XXXX);
    clmOPCODE_U8(LOAD, 21, XYZW); clmUNIFORM_U32(input, XXXX); clmTEMP_U32(20, XXXX);
    clmOPCODE_U8(I2F, 22, XYZW); clmTEMP_U8(21, XYZW);
    clmOPCODE(MUL, 23, XYZW); clmTEMP(22, XYZW); clmTEMP(18, WWWW);
    clmOPCODE(ADD, 24, XYZW); clmTEMP(17, XYZW); clmTEMP(23, XYZW);
    clmOPCODE_S32(SUB, 25, X); clmTEMP_S32(5, XXXX); clmS32(3 * 4);
    clmOPCODE_S32(MAX, 26, X); clmTEMP_S32(25, XXXX); clmTEMP_S32(8, XXXX);
    clmOPCODE_U8(LOAD, 27, XYZW); clmUNIFORM_U32(input, XXXX); clmTEMP_U32(26, XXXX);
    clmOPCODE_U8(I2F, 28, XYZW); clmTEMP_U8(27, XYZW);
    clmOPCODE(MUL, 29, XYZW); clmTEMP(28, XYZW); clmTEMP(18, ZZZZ);
    clmOPCODE(ADD, 30, XYZW); clmTEMP(24, XYZW); clmTEMP(29, XYZW);
    clmOPCODE_S32(SUB, 31, X); clmTEMP_S32(5, XXXX); clmS32(4 * 4);
    clmOPCODE_S32(MAX, 32, X); clmTEMP_S32(31, XXXX); clmTEMP_S32(8, XXXX);
    clmOPCODE_U8(LOAD, 33, XYZW); clmUNIFORM_U32(input, XXXX); clmTEMP_U32(32, XXXX);
    clmOPCODE_U8(I2F, 34, XYZW); clmTEMP_U8(33, XYZW);
    clmOPCODE(MUL, 35, XYZW); clmTEMP(34, XYZW); clmTEMP(18, YYYY);
    clmOPCODE(ADD, 36, XYZW); clmTEMP(30, XYZW); clmTEMP(35, XYZW);
    clmOPCODE_S32(SUB, 37, X); clmTEMP_S32(5, XXXX); clmS32(5 * 4);
    clmOPCODE_S32(MAX, 38, X); clmTEMP_S32(37, XXXX); clmTEMP_S32(8, XXXX);
    clmOPCODE_U8(LOAD, 39, XYZW); clmUNIFORM_U32(input, XXXX); clmTEMP_U32(38, XXXX);
    clmOPCODE_U8(I2F, 40, XYZW); clmTEMP_U8(39, XYZW);
    clmOPCODE(MUL, 41, XYZW); clmTEMP(40, XYZW); clmTEMP(18, XXXX);
    clmOPCODE(ADD, 42, XYZW); clmTEMP(36, XYZW); clmTEMP(41, XYZW);
    clmOPCODE_U32(MUL, 79, X); clmUNIFORM_U32(global_size, XXXX); clmU32(4);
    clmOPCODE_U32(ADD, 43, X); clmTEMP_U32(8, XXXX); clmTEMP_U32(79, XXXX);
    clmOPCODE_U32(SUB, 44, X); clmTEMP_U32(43, XXXX); clmU32(4);
    clmOPCODE_U32(ADD, 45, X); clmTEMP_U32(5, XXXX); clmU32(1 * 4);
    clmOPCODE_U32(MIN, 46, X); clmTEMP_U32(45, XXXX); clmTEMP_U32(44, XXXX);
    clmOPCODE_U8(LOAD, 47, XYZW); clmUNIFORM_U32(input, XXXX); clmTEMP_U32(46, XXXX);
    clmOPCODE_U8(I2F, 48, XYZW); clmTEMP_U8(47, XYZW);
    clmOPCODE(MUL, 49, XYZW); clmTEMP(48, XYZW); clmTEMP(7, ZZZZ);
    clmOPCODE(ADD, 50, XYZW); clmTEMP(42, XYZW); clmTEMP(49, XYZW);
    clmOPCODE_U32(ADD, 51, X); clmTEMP_U32(5, XXXX); clmU32(2 * 4);
    clmOPCODE_U32(MIN, 52, X); clmTEMP_U32(51, XXXX); clmTEMP_U32(44, XXXX);
    clmOPCODE_U8(LOAD, 53, XYZW); clmUNIFORM_U32(input, XXXX); clmTEMP_U32(52, XXXX);
    clmOPCODE_U8(I2F, 54, XYZW); clmTEMP_U8(53, XYZW);
    clmOPCODE(MUL, 55, XYZW); clmTEMP(54, XYZW); clmTEMP(7, WWWW);
    clmOPCODE(ADD, 56, XYZW); clmTEMP(50, XYZW); clmTEMP(55, XYZW);
    clmOPCODE(LOAD, 57, XYZW); clmUNIFORM_U32(coefficients, XXXX); clmU32(7 * 4);
    clmOPCODE_U32(ADD, 58, X); clmTEMP_U32(5, XXXX); clmU32(3 * 4);
    clmOPCODE_U32(MIN, 59, X); clmTEMP_U32(58, XXXX); clmTEMP_U32(44, XXXX);
    clmOPCODE_U8(LOAD, 60, XYZW); clmUNIFORM_U32(input, XXXX); clmTEMP_U32(59, XXXX);
    clmOPCODE_U8(I2F, 61, XYZW); clmTEMP_U8(60, XYZW);
    clmOPCODE(MUL, 62, XYZW); clmTEMP(61, XYZW); clmTEMP(57, YYYY);
    clmOPCODE(ADD, 63, XYZW); clmTEMP(56, XYZW); clmTEMP(62, XYZW);
    clmOPCODE_U32(ADD, 64, X); clmTEMP_U32(5, XXXX); clmU32(4 * 4);
    clmOPCODE_U32(MIN, 65, X); clmTEMP_U32(64, XXXX); clmTEMP_U32(44, XXXX);
    clmOPCODE_U8(LOAD, 66, XYZW); clmUNIFORM_U32(input, XXXX); clmTEMP_U32(65, XXXX);
    clmOPCODE_U8(I2F, 67, XYZW); clmTEMP_U8(66, XYZW);
    clmOPCODE(MUL, 68, XYZW); clmTEMP(67, XYZW); clmTEMP(57, ZZZZ);
    clmOPCODE(ADD, 69, XYZW); clmTEMP(63, XYZW); clmTEMP(68, XYZW);
    clmOPCODE_U32(ADD, 70, X); clmTEMP_U32(5, XXXX); clmU32(5 * 4);
    clmOPCODE_U32(MIN, 71, X); clmTEMP_U32(70, XXXX); clmTEMP_U32(44, XXXX);
    clmOPCODE_U8(LOAD, 72, XYZW); clmUNIFORM_U32(input, XXXX); clmTEMP_U32(71, XXXX);
    clmOPCODE_U8(I2F, 73, XYZW); clmTEMP_U8(72, XYZW);
    clmOPCODE(MUL, 74, XYZW); clmTEMP(73, XYZW); clmTEMP(57, WWWW);
    clmOPCODE(ADD, 75, XYZW); clmTEMP(69, XYZW); clmTEMP(74, XYZW);
    clmJUMP(ALWAYS, 3);
    clmLABEL(2);
    clmOPCODE_S32(SUB, 77, X); clmTEMP_S32(5, XXXX); clmS32(5 * 4);
    clmOPCODE_U32(LOAD, 78, XYZW); clmUNIFORM_U32(input, XXXX); clmTEMP_S32(77, XXXX);
    clmOPCODE(LOAD, 76, XYZW); clmUNIFORM_U32(coefficients, XXXX); clmU32(0);
    clmOPCODE_U32(MOV, 79, X); clmU32(0x11110000);
    clmOPCODE_U32(MOV, 79, Y); clmU32(0x33332222);
    clmSWIZZLE_MUL(80, 78, XXXX, 79, 76, XXXX);
    clmSWIZZLE_MAD(85, 78, YYYY, 79, 76, YYYY, 84);
    clmSWIZZLE_MAD(91, 78, ZZZZ, 79, 76, ZZZZ, 90);
    clmOPCODE_S32(SUB, 97, X); clmTEMP_S32(5, XXXX); clmS32(1 * 4);
    clmOPCODE_U32(LOAD, 98, XYZW); clmUNIFORM_U32(input, XXXX); clmTEMP_S32(97, XXXX);
    clmSWIZZLE_MAD(99, 78, WWWW, 79, 76, WWWW, 96);
    clmOPCODE(LOAD, 105, XYZW); clmUNIFORM_U32(coefficients, XXXX); clmU32(4 * 4);
    clmSWIZZLE_MAD(106, 98, XXXX, 79, 105, XXXX, 104);
    clmSWIZZLE_MAD(112, 98, YYYY, 79, 105, YYYY, 111);
    clmSWIZZLE_MAD(118, 98, ZZZZ, 79, 105, ZZZZ, 117);
    clmOPCODE_S32(ADD, 124, X); clmTEMP_S32(5, XXXX); clmS32(2 * 4);
    clmOPCODE_U32(LOAD, 125, XYZW); clmUNIFORM_U32(input, XXXX); clmTEMP_S32(124, XXXX);
    clmSWIZZLE_MAD(126, 98, WWWW, 79, 105, WWWW, 123);
    clmOPCODE(LOAD, 132, XYZW); clmUNIFORM_U32(coefficients, XXXX); clmU32(7 * 4);
    clmSWIZZLE_MAD(133, 125, YYYY, 79, 132, YYYY, 131);
    clmSWIZZLE_MAD(139, 125, ZZZZ, 79, 132, ZZZZ, 138);
    clmSWIZZLE_MUL(145, 125, WWWW, 79, 132, WWWW);
    clmOPCODE(ADD, 75, XYZW); clmTEMP(144, XYZW); clmTEMP(149, XYZW);
    clmLABEL(3);
    clmOPCODE(MAX, 150, XYZW); clmTEMP(75, XYZW); clmFLOAT(0.0f);
    clmOPCODE(MIN, 151, XYZW); clmTEMP(150, XYZW); clmFLOAT(255.0f);
    clmOPCODE_U8(F2I, 152, XYZW); clmTEMP_U8(151, XYZW);
    clmOPCODE_U32(ADD, 153, X); clmUNIFORM_U32(output, XXXX); clmTEMP_S32(5, XXXX);
    clmOPCODE_U8(STORE1, 154, XYZW); clmTEMP_U32(153, XXXX); clmTEMP_U8(152, XYZW);

    /*  main */
    clmMAIN();

    /*
     *  __kernel void VerticalGaussFilter
     *  (
     *      __global const uchar4 *restrict input,
     *      __constant float *coefficients,
     *      __global uchar4 *output
     *      )
     *  {
     *      const int pX = get_global_id(0);
     *      const int pY = get_global_id(1);
     *
     *      const int width = get_global_size(0);
     *      const int heightPrec = get_global_size(1) - 1;
     *
     *      float4 aggregate = convert_float4(input[pY*width + pX]) * coefficients[samplingOffset];
     *
     *      for(int o = 1; o <= samplingOffset; ++o)
     *      {
     *          aggregate += convert_float4(input[max(pY - o, 0)*width + pX]) * coefficients[samplingOffset - o];
     *          aggregate += convert_float4(input[min(pY + o, heightPrec)*width + pX]) * coefficients[samplingOffset + o];
     *      }
     *
     *      output[pY*width + pX] = convert_uchar4_sat(aggregate);
     *  }
     */
    clmKERNEL("VerticalGaussFilter", 327);
    clmARGUMENT_U32(input, 0, gcvFALSE, UINT8, gcvTRUE);
    clmARGUMENT_U32(coefficients, 1, gcvTRUE, FLOAT, gcvTRUE);
    clmARGUMENT_U32(output, 2, gcvFALSE, UINT8, gcvTRUE);

    clmOPCODE_S32(LSHIFT, 3, X); clmATTRIBUTE_S32(global_id, XXXX); clmS32(2);
    clmJUMP_S32(GREATER_OR_EQUAL, 4); clmTEMP_S32(3, XXXX); clmUNIFORM_S32(global_size, XXXX);
    clmOPCODE_S32(MUL, 4, X); clmATTRIBUTE_S32(global_id, YYYY); clmUNIFORM_S32(global_size, XXXX);
    clmOPCODE_S32(ADD, 5, X); clmTEMP_S32(4, XXXX); clmTEMP_S32(3, XXXX);
    clmOPCODE_S32(LSHIFT, 6, X); clmTEMP_S32(5, XXXX); clmS32(2);
    clmOPCODE_S32(LSHIFT, 7, X); clmTEMP_S32(3, XXXX); clmS32(2);
    clmOPCODE_S32(LSHIFT, 8, X); clmUNIFORM_S32(global_size, XXXX); clmS32(2);
    clmOPCODE_U32(LOAD, 9, XYZW); clmUNIFORM_U32(input, XXXX); clmTEMP_S32(6, XXXX);
    clmOPCODE(LOAD, 10, XYZW); clmUNIFORM_U32(coefficients, XXXX); clmU32(4 * 4);
    clmOPCODE_U32(MOV, 11, X); clmU32(0x11110000);
    clmOPCODE_U32(MOV, 11, Y); clmU32(0x33332222);
    clmSWIZZLE_MUL(12, 9, XXXX, 11, 10, YYYY);
    clmSWIZZLE_MUL(17, 9, YYYY, 11, 10, YYYY);
    clmSWIZZLE_MUL(22, 9, ZZZZ, 11, 10, YYYY);
    clmSWIZZLE_MUL(27, 9, WWWW, 11, 10, YYYY);
    clmOPCODE_S32(SUB, 32, X); clmTEMP_S32(6, XXXX); clmTEMP_S32(8, XXXX);
    clmOPCODE_S32(MAX, 33, X); clmTEMP_S32(32, XXXX); clmTEMP_S32(7, XXXX);
    clmOPCODE_U32(LOAD, 34, XYZW); clmUNIFORM_U32(input, XXXX); clmTEMP_S32(33, XXXX);
    clmSWIZZLE_MAD(35, 34, XXXX, 11, 10, XXXX, 16);
    clmSWIZZLE_MAD(41, 34, YYYY, 11, 10, XXXX, 21);
    clmSWIZZLE_MAD(47, 34, ZZZZ, 11, 10, XXXX, 26);
    clmSWIZZLE_MAD(53, 34, WWWW, 11, 10, XXXX, 31);
    clmOPCODE_S32(SUB, 59, X); clmTEMP_S32(33, XXXX); clmTEMP_S32(8, XXXX);
    clmOPCODE_S32(MAX, 60, X); clmTEMP_S32(59, XXXX); clmTEMP_S32(7, XXXX);
    clmOPCODE_U32(LOAD, 61, XYZW); clmUNIFORM_U32(input, XXXX); clmTEMP_S32(60, XXXX);
    clmOPCODE(LOAD, 62, XYZW); clmUNIFORM_U32(coefficients, XXXX); clmU32(0);
    clmSWIZZLE_MAD(63, 61, XXXX, 11, 62, WWWW, 40);
    clmSWIZZLE_MAD(69, 61, YYYY, 11, 62, WWWW, 46);
    clmSWIZZLE_MAD(75, 61, ZZZZ, 11, 62, WWWW, 52);
    clmSWIZZLE_MAD(81, 61, WWWW, 11, 62, WWWW, 58);
    clmOPCODE_S32(SUB, 87, X); clmTEMP_S32(60, XXXX); clmTEMP_S32(8, XXXX);
    clmOPCODE_S32(MAX, 88, X); clmTEMP_S32(87, XXXX); clmTEMP_S32(7, XXXX);
    clmOPCODE_U32(LOAD, 89, XYZW); clmUNIFORM_U32(input, XXXX); clmTEMP_S32(88, XXXX);
    clmSWIZZLE_MAD(90, 89, XXXX, 11, 62, ZZZZ, 68);
    clmSWIZZLE_MAD(96, 89, YYYY, 11, 62, ZZZZ, 74);
    clmSWIZZLE_MAD(102, 89, ZZZZ, 11, 62, ZZZZ, 80);
    clmSWIZZLE_MAD(108, 89, WWWW, 11, 62, ZZZZ, 86);
    clmOPCODE_S32(SUB, 114, X); clmTEMP_S32(88, XXXX); clmTEMP_S32(8, XXXX);
    clmOPCODE_S32(MAX, 115, X); clmTEMP_S32(114, XXXX); clmTEMP_S32(7, XXXX);
    clmOPCODE_U32(LOAD, 116, XYZW); clmUNIFORM_U32(input, XXXX); clmTEMP_S32(115, XXXX);
    clmSWIZZLE_MAD(117, 116, XXXX, 11, 62, YYYY, 95);
    clmSWIZZLE_MAD(123, 116, YYYY, 11, 62, YYYY, 101);
    clmSWIZZLE_MAD(129, 116, ZZZZ, 11, 62, YYYY, 107);
    clmSWIZZLE_MAD(135, 116, WWWW, 11, 62, YYYY, 113);
    clmOPCODE_S32(SUB, 141, X); clmTEMP_S32(115, XXXX); clmTEMP_S32(8, XXXX);
    clmOPCODE_S32(MAX, 142, X); clmTEMP_S32(141, XXXX); clmTEMP_S32(7, XXXX);
    clmOPCODE_U32(LOAD, 143, XYZW); clmUNIFORM_U32(input, XXXX); clmTEMP_S32(142, XXXX);
    clmSWIZZLE_MAD(144, 143, XXXX, 11, 62, XXXX, 122);
    clmSWIZZLE_MAD(150, 143, YYYY, 11, 62, XXXX, 128);
    clmSWIZZLE_MAD(156, 143, ZZZZ, 11, 62, XXXX, 134);
    clmSWIZZLE_MAD(162, 143, WWWW, 11, 62, XXXX, 140);
    clmOPCODE_S32(SUB, 168, X); clmUNIFORM_S32(global_size, YYYY); clmS32(1);
    clmOPCODE_S32(MUL, 169, X); clmUNIFORM_S32(global_size, XXXX); clmTEMP_S32(168, XXXX);
    clmOPCODE_S32(ADD, 170, X); clmTEMP_S32(169, XXXX); clmTEMP_S32(3, XXXX);
    clmOPCODE_S32(LSHIFT, 171, X); clmTEMP_S32(170, XXXX); clmS32(2);
    clmOPCODE_S32(ADD, 172, X); clmTEMP_S32(6, XXXX); clmTEMP_S32(8, XXXX);
    clmOPCODE_S32(MIN, 173, X); clmTEMP_S32(172, XXXX); clmTEMP_S32(171, XXXX);
    clmOPCODE_U32(LOAD, 174, XYZW); clmUNIFORM_U32(input, XXXX); clmTEMP_S32(173, XXXX);
    clmSWIZZLE_MAD(175, 174, XXXX, 11, 10, ZZZZ, 149);
    clmSWIZZLE_MAD(181, 174, YYYY, 11, 10, ZZZZ, 155);
    clmSWIZZLE_MAD(187, 174, ZZZZ, 11, 10, ZZZZ, 161);
    clmSWIZZLE_MAD(193, 174, WWWW, 11, 10, ZZZZ, 167);
    clmOPCODE_S32(ADD, 199, X); clmTEMP_S32(173, XXXX); clmTEMP_S32(8, XXXX);
    clmOPCODE_S32(MIN, 200, X); clmTEMP_S32(199, XXXX); clmTEMP_S32(171, XXXX);
    clmOPCODE_U32(LOAD, 201, XYZW); clmUNIFORM_U32(input, XXXX); clmTEMP_S32(200, XXXX);
    clmSWIZZLE_MAD(202, 201, XXXX, 11, 10, WWWW, 180);
    clmSWIZZLE_MAD(208, 201, YYYY, 11, 10, WWWW, 186);
    clmSWIZZLE_MAD(214, 201, ZZZZ, 11, 10, WWWW, 192);
    clmSWIZZLE_MAD(220, 201, WWWW, 11, 10, WWWW, 198);
    clmOPCODE_U32(ADD, 226, X); clmTEMP_S32(200, XXXX); clmTEMP_S32(8, XXXX);
    clmOPCODE_U32(MIN, 227, X); clmTEMP_S32(226, XXXX); clmTEMP_S32(171, XXXX);
    clmOPCODE_U32(LOAD, 228, XYZW); clmUNIFORM_U32(input, XXXX); clmTEMP_S32(227, XXXX);
    clmOPCODE(LOAD, 229, XYZW); clmUNIFORM_U32(coefficients, XXXX); clmU32(7 * 4);
    clmSWIZZLE_MAD(230, 228, XXXX, 11, 229, YYYY, 207);
    clmSWIZZLE_MAD(236, 228, YYYY, 11, 229, YYYY, 213);
    clmSWIZZLE_MAD(242, 228, ZZZZ, 11, 229, YYYY, 219);
    clmSWIZZLE_MAD(248, 228, WWWW, 11, 229, YYYY, 225);
    clmOPCODE_S32(ADD, 254, X); clmTEMP_S32(227, XXXX); clmTEMP_S32(8, XXXX);
    clmOPCODE_S32(MIN, 255, X); clmTEMP_S32(254, XXXX); clmTEMP_S32(171, XXXX);
    clmOPCODE_U32(LOAD, 256, XYZW); clmUNIFORM_U32(input, XXXX); clmTEMP_S32(255, XXXX);
    clmSWIZZLE_MAD(257, 256, XXXX, 11, 229, ZZZZ, 235);
    clmSWIZZLE_MAD(263, 256, YYYY, 11, 229, ZZZZ, 241);
    clmSWIZZLE_MAD(269, 256, ZZZZ, 11, 229, ZZZZ, 247);
    clmSWIZZLE_MAD(275, 256, WWWW, 11, 229, ZZZZ, 253);
    clmOPCODE_S32(ADD, 281, X); clmTEMP_S32(255, XXXX); clmTEMP_S32(8, XXXX);
    clmOPCODE_S32(MIN, 282, X); clmTEMP_S32(281, XXXX); clmTEMP_S32(171, XXXX);
    clmOPCODE_U32(LOAD, 283, XYZW); clmUNIFORM_U32(input, XXXX); clmTEMP_S32(282, XXXX);
    clmSWIZZLE_MAD(284, 283, XXXX, 11, 229, WWWW, 262);
    clmSWIZZLE_MAD(290, 283, YYYY, 11, 229, WWWW, 268);
    clmSWIZZLE_MAD(296, 283, ZZZZ, 11, 229, WWWW, 274);
    clmSWIZZLE_MAD(302, 283, WWWW, 11, 229, WWWW, 280);
    clmOPCODE(MAX, 308, XYZW); clmTEMP(289, XYZW); clmFLOAT(0.0f);
    clmOPCODE(MAX, 309, XYZW); clmTEMP(295, XYZW); clmFLOAT(0.0f);
    clmOPCODE(MAX, 310, XYZW); clmTEMP(301, XYZW); clmFLOAT(0.0f);
    clmOPCODE(MAX, 311, XYZW); clmTEMP(307, XYZW); clmFLOAT(0.0f);
    clmOPCODE(MIN, 312, XYZW); clmTEMP(308, XYZW); clmFLOAT(255.0f);
    clmOPCODE(MIN, 313, XYZW); clmTEMP(309, XYZW); clmFLOAT(255.0f);
    clmOPCODE(MIN, 314, XYZW); clmTEMP(310, XYZW); clmFLOAT(255.0f);
    clmOPCODE(MIN, 315, XYZW); clmTEMP(311, XYZW); clmFLOAT(255.0f);
    clmOPCODE_U8(F2I, 316, XYZW); clmTEMP(312, XYZW);
    clmOPCODE_U8(F2I, 317, XYZW); clmTEMP(313, XYZW);
    clmOPCODE_U8(F2I, 318, XYZW); clmTEMP(314, XYZW);
    clmOPCODE_U8(F2I, 319, XYZW); clmTEMP(315, XYZW);
    clmOPCODE_U8(UNPACK, 320, XYZW); clmTEMP(316, XYZW); clmU32(0xC840C840);
    clmOPCODE_U8(AND_BITWISE, 321, X); clmTEMP_U8(320, XYZW); clmU32(0x000F);
    clmOPCODE_U8(UNPACK, 322, XYZW); clmTEMP(317, XYZW); clmU32(0xC840C840);
    clmOPCODE_U8(AND_BITWISE, 321, Y); clmTEMP_U8(322, XYZW); clmU32(0x00F0);
    clmOPCODE_U8(UNPACK, 323, XYZW); clmTEMP(318, XYZW); clmU32(0xC840C840);
    clmOPCODE_U8(AND_BITWISE, 321, Z); clmTEMP_U8(323, XYZW); clmU32(0x0F00);
    clmOPCODE_U8(UNPACK, 324, XYZW); clmTEMP(319, XYZW); clmU32(0xC840C840);
    clmOPCODE_U8(AND_BITWISE, 321, W); clmTEMP_U8(324, XYZW); clmU32(0xF000);
    clmOPCODE_U32(ADD, 325, X); clmUNIFORM_U32(output, XXXX); clmTEMP_S32(6, XXXX);
    clmOPCODE_U32(STORE1, 326, XYZW); clmTEMP_S32(325, XXXX); clmTEMP_U32(321, XYZW);

    clmLABEL(4);

    /*  main. */
    clmMAIN();

    /*
     *  __kernel void TranspositionalGaussFilter
     *  (
     *      __global const uchar4 *restrict input,
     *      __constant float *coefficients,
     *      __global uchar4 *output
     *      )
     *  {
     *      const int pX = get_global_id(0);
     *      const int pY = get_global_id(1);
     *
     *      const int widthPrec = get_global_size(0) - 1;
     *      const int pY_width = pY*get_global_size(0);
     *
     *      float4 aggregate = convert_float4(input[pY_width + pX]) * coefficients[samplingOffset];
     *
     *      for(int o = 1; o <= samplingOffset; ++o)
     *      {
     *          aggregate += convert_float4(input[pY_width + max(pX - o, 0)]) * coefficients[samplingOffset - o];
     *          aggregate += convert_float4(input[pY_width + min(pX + o, widthPrec)]) * coefficients[samplingOffset + o];
     *      }
     *
     *      output[pX*get_global_size(1) + pY] = convert_uchar4_sat(aggregate); //uncoalesced transpositional write
     *  }
     */

    /*
     *  __kernel void TranspositionalGaussFilterLuminance
     *  (
     *      __global const uchar *input,
     *      __constant float *coefficients,
     *      __global uchar *output
     *      )
     *  {
     *      const int pX = get_global_id(0);
     *      const int pY = get_global_id(1);
     *      const int widthPrec = get_global_size(0) - 1;
     *      const int pY_width = pY*get_global_size(0);
     *
     *      float aggregate = convert_float(input[pY_width + pX]) * coefficients[samplingOffset];
     *
     *      for(int o = 1; o <= samplingOffset; ++o)
     *      {
     *          aggregate += convert_float(input[pY_width + max(pX - o, 0)]) * coefficients[samplingOffset - o];
     *          aggregate += convert_float(input[pY_width + min(pX + o, widthPr ec)]) * coefficients[samplingOffset + o];
     *      }
     *
     *      output[pX*get_global_size(1) + pY] = convert_uchar_sat(aggregate); //uncoalesced transpositional write
     *  }
     */

    /* Pack the shader. */
    gcmONERROR(gcSHADER_Pack(shader));
    return shader;

OnError:
    if (shader != gcvNULL)
    {
        gcSHADER_Destroy(shader);
    }

    return gcvNULL;
}

static gcSHADER clTune_20(IN gcSHADER Shader)
{
    gcSHADER shader = gcvNULL;
    gceSTATUS status;
    gcKERNEL_FUNCTION kernel;
    gctUINT tempStart;
    gcATTRIBUTE global_id;
    gctINT property[3] = { 0, 0, 0 };
    gcUNIFORM stageNodes, stagesCount, stageNodeCounts, stageThresholds, resRectangles, rectCount, haarRects;
    gcUNIFORM integralImage, integralImage_size;
    gcUNIFORM integral2Image, integral2Image_size;
    gcUNIFORM yChannel, uChannel, vChannel, yStride, uStride, vStride, hdec, vdec;
    gcUNIFORM output, output_size;
    gctUINT32 compilerVersion[2];

    /* Construct a new shader. */
    gcmONERROR(gcSHADER_Construct(gcSHADER_TYPE_CL, &shader));

    /* Set OpenCL arguments. */
    cloCOMPILER_GetVersion(gcvNULL, clvSHADER_TYPE_CL, compilerVersion);
    gcmONERROR(gcSHADER_SetCompilerVersion(shader, compilerVersion));
    gcmONERROR(gcSHADER_SetMaxKernelFunctionArgs(shader, 9));

    /* Add attributes and uniforms. */
    gcmONERROR(gcSHADER_AddAttribute(shader, "#global_id", gcSHADER_INTEGER_X4, 1, gcvFALSE, gcSHADER_SHADER_DEFAULT, gcSHADER_PRECISION_DEFAULT,
                                     &global_id));
    gcmONERROR(gcSHADER_AddUniform(shader, "integralImage#size", gcSHADER_INTEGER_X4, 1, gcSHADER_PRECISION_DEFAULT, &integralImage_size));
    gcmONERROR(gcSHADER_AddUniform(shader, "integral2Image#size", gcSHADER_INTEGER_X4, 1, gcSHADER_PRECISION_DEFAULT, &integral2Image_size));
    gcmONERROR(gcSHADER_AddUniform(shader, "output#size", gcSHADER_INTEGER_X4, 1, gcSHADER_PRECISION_DEFAULT, &output_size));

    /*
     *  // calculate the integral of a rectangle
     *  inline uint getIntegralSum
     *  (
     *      read_only image2d_t integralImage,
     *      sampler_t integralSampler,
     *      int x,
     *      int y,
     *      int width,
     *      int height)
     *  {
     *      //  sampler_t smp = CLK_ADDRESS_CLAMP;
     *
     *      return  read_imageui(integralImage, integralSampler, (int2)(x, y)).w +
     *          read_imageui(integralImage, integralSampler, (int2)(x + width, y + height)).w -
     *          read_imageui(integralImage, integralSampler, (int2)(x, y + height)).w -
     *          read_imageui(integralImage, integralSampler, (int2)(x + width, y)).w;
     *  }
     *
     *  __kernel void violaJones(
     *      __global HaarFeatureNode* restrict stageNodes,
     *      int stagesCount,
     *      __global int* restrict stageNodeCounts,
     *      __global float* restrict stageThresholds,
     *      read_only image2d_t integralImage,
     *      read_only image2d_t integral2Image,
     *      __global int* restrict resRectangles,
     *      const int rectCount,
     *      __global const HaarRect* restrict haarRects)
     *  {
     *      int id = get_global_id(0);
     *
     *      if (id >= rectCount)
     *          return;
     *
     *      const sampler_t integralSampler = CLK_ADDRESS_CLAMP;
     *
     *      // try to detect an object inside the window
     *      float scale = haarRects[id].scale;
     *      const int x = haarRects[id].x;
     *      const int y = haarRects[id].y;
     *      const int w = haarRects[id].width;
     *      const int h = haarRects[id].height;
     *
     *      float invArea = 1 / ((float)w * (float)h);
     *
     *      const float mean = getIntegralSum(integralImage, integralSampler, x, y, w, h) * invArea;
     *      const float normFactor = getIntegralSum(integral2Image, integralSampler, x, y, w, h) * invArea
     *          - (mean * mean);
     *
     *      normFactor = (normFactor >= 0) ? sqrt(normFactor) : 1;
     *
     *      int result = 1;
     *      int pos = 0;
     *      for (int j = 0; j < stagesCount; j++)
     *      {
     *          int count = stageNodeCounts[j];
     *          float threshold = stageThresholds[j];
     *
     *          float value = 0;
     *          for (int i = 0; i < count; i++)
     *          {
     *              __global HaarFeatureNode* node = &stageNodes[pos++];
     *
     *              // evaluate the node's feature
     *              float sum = 0;
     *
     *              // calculate feature node sum (= sum of all rectangles in node)
     *              float4 rects[3];
     *              float weights[3];
     *
     *              // scale first rectangle
     *              rects[0].x = node->rect[0].x * scale;
     *              rects[0].y = node->rect[0].y * scale;
     *              rects[0].z = node->rect[0].width * scale;
     *              rects[0].w = node->rect[0].height * scale;
     *
     *              // scale and weight second rectangle
     *              rects[1].x = node->rect[1].x * scale;
     *              rects[1].y = node->rect[1].y * scale;
     *              rects[1].z = node->rect[1].width * scale;
     *              rects[1].w = node->rect[1].height * scale;
     *              weights[1] = node->rect[1].weight * invArea;
     *
     *              switch (node->rectCount)
     *              {
     *              case 2:
     *                  weights[0] = -(rects[1].z * rects[1].w * weights[1]) / (rects[0].z * rects[0].w);
     *                  sum = getIntegralSum(integralImage, integralSampler, x + rects[0].x, y + rects[0].y,
     *                      rects[0].z, rects[0].w) * weights[0];
     *                  sum += getIntegralSum(integralImage, integralSampler, x + rects[1].x, y + rects[1].y,
     *                      rects[1].z, rects[1].w) * weights[1];
     *                  break;
     *
     *              case 3:
     *                  // scale and weight third rectangle
     *                  rects[2].x = node->rect[2].x * scale;
     *                  rects[2].y = node->rect[2].y * scale;
     *                  rects[2].z = node->rect[2].width * scale;
     *                  rects[2].w = node->rect[2].height * scale;
     *                  weights[2] = node->rect[2].weight * invArea;
     *
     *                  weights[0] = -(rects[1].z * rects[1].w * weights[1] + rects[2].z * rects[2].w * weights[2])
     *                      / (rects[0].z * rects[0].w);
     *
     *                  sum = getIntegralSum(integralImage, integralSampler, x + rects[0].x, y + rects[0].y,
     *                      rects[0].z, rects[0].w) * weights[0];
     *                  sum += getIntegralSum(integralImage, integralSampler, x + rects[1].x, y + rects[1].y,
     *                      rects[1].z, rects[1].w) * weights[1];
     *                  sum += getIntegralSum(integralImage, integralSampler, x + rects[2].x, y + rects[2].y,
     *                      rects[2].z, rects[2].w) * weights[2];
     *                  break;
     *              }
     *
     *              // and increase the value accumulator
     *              value += ((sum < node->threshold * normFactor) ? node->leftValue : node->rightValue);
     *          }
     *
     *          if (value <= threshold)
     *          {
     *              // window has been rejected
     *              result = 0;
     *              break;
     *          }
     *      }
     *
     *      resRectangles[id] = result;
     *  }
     */
    clmKERNEL("violaJones", 123);
    clmARGUMENT_U32(stageNodes, 0, gcvFALSE, INT8, gcvTRUE);
    clmARGUMENT_U32(stagesCount, 1, gcvFALSE, INTEGER, gcvFALSE);
    clmARGUMENT_U32(stageNodeCounts, 2, gcvFALSE, INTEGER, gcvTRUE);
    clmARGUMENT_U32(stageThresholds, 3, gcvFALSE, FLOAT, gcvTRUE);
    clmARGUMENT_IMAGE(integralImage, 4, gcvTRUE);
    clmARGUMENT_IMAGE(integral2Image, 5, gcvTRUE);
    clmARGUMENT_U32(resRectangles, 6, gcvFALSE, INTEGER, gcvTRUE);
    clmARGUMENT_U32(rectCount, 7, gcvTRUE, INTEGER, gcvFALSE);
    clmARGUMENT_U32(haarRects, 8, gcvTRUE, INT8, gcvTRUE);

    /*  mov     temp.u32(9).x, attribute.u32(global_id).x
     *  jmp.ge  8, temp.s32(9).x, uniform.s32(rectCount).x

     *  mul     temp.u32(10).x, temp.u32(9).x, 20
     *  load    temp.s32(11), uniform.u32(haarRects).x, temp.u32(10).x
     *  add     temp.u32(12).x, temp.u32(10).x, 16
     *  load    temp(13).x, uniform.u32(haarRects).x, temp.u32(12).x

     *  i2f     temp(14).xy, temp.s32(11).zw
     *  mul     temp(15).x, temp(14).x, temp(14).y
     *  rcp     temp(16).x, temp(15).x

     *  mov     temp.s32(17).xy, temp.s32(11).xy
     *  add     temp.s32(17).zw, temp.s32(11).xxxy, temp.s32(11).zzzw
     *  max     temp.s32(18), temp.s32(17), 0
     *  min     temp.u32(19), temp.u32(18), uniform.u32(integralImage#size).zwzw
     *  mov     temp.u32(20).x, 2
     *  mov     temp.u32(20).y, 0
     *  lshift  temp.u32(21), temp.u32(19), temp.u32(20).xyxy
     *  mul     temp.u32(22), temp.u32(21).ywwy, uniform.u32(integralImage#size).y
     *  add     temp.u32(23), temp.u32(22), temp.u32(21).xzxz
     *  load    temp.u32(24).x, uniform.u32(integralImage).x, temp.u32(23).x
     *  load    temp.u32(24).y, uniform.u32(integralImage).x, temp.u32(23).y
     *  load    temp.u32(24).z, uniform.u32(integralImage).x, temp.u32(23).z
     *  load    temp.u32(24).w, uniform.u32(integralImage).x, temp.u32(23).w
     *  add     temp.u32(25).xy, temp.u32(24).xz, temp.u32(24).yw
     *  sub     temp.u32(26).x, temp.u32(25).x, temp.u32(25).y
     *  min     temp.u32(27), temp.u32(18), uniform.u32(integral2Image#size).zwzw
     *  lshift  temp.u32(28), temp.u32(27), temp.u32(20).xyxy
     *  mul     temp.u32(29), temp.u32(28).ywwy, uniform.u32(integral2Image#size).y
     *  add     temp.u32(30), temp.u32(29), temp.u32(28).xzxz
     *  load    temp.u32(31).x, uniform.u32(integral2Image).x, temp.u32(30).x
     *  load    temp.u32(31).y, uniform.u32(integral2Image).x, temp.u32(30).y
     *  load    temp.u32(31).z, uniform.u32(integral2Image).x, temp.u32(30).z
     *  load    temp.u32(31).w, uniform.u32(integral2Image).x, temp.u32(30).w
     *  add     temp.u32(32).xy, temp.u32(31).xz, temp.u32(31).yw
     *  sub     temp.u32(26).y, temp.u32(32).x, temp.u32(32).y
     *  i2f     temp(33).xy, temp.u32(26).xy
     *  mul     temp(34).xy, temp(33).xy, temp(16).x
     *  mul     temp(35).x, temp(34).x, temp(34).x
     *  sub     temp(36).x, temp(34).y, temp(35).x

     *  sqrt    temp(37).x, temp(36).x
     *  jmp.ge  1, temp(36).x, 0
     *  mov     temp(37).x, 1
     *  1:

     *  mov     temp.u32(38).x, 0
     *  mov     temp.u32(39).xy, 0

     *  2:
     *  load    temp.s32(40).x, uniform.u32(stageNodeCounts).x, temp.u32(39).y
     *  load    temp(41).x, uniform.u32(stageThresholds).x, temp.u32(39).y

     *  mov     temp(42).x, 0

     *  3:
     *  load    temp(43), uniform.u32(stageNodes).x, temp.u32(38).x
     *  add     temp.u32(44).x, temp.u32(38).x, 136
     *  mov     temp.u32(38).x, temp.u32(44).x

     *  mov     temp(45).x, 0
     *  mov     temp(46).x, 0

     *  jmp.eq  4, temp.s32(43).w, 2
     *  jmp.ne  5, temp.s32(43).w, 3

     *  sub     temp.u32(47).x, temp.u32(38).x, 40
     *  load    temp.s32(48), uniform.u32(stageNodes).x, temp.u32(47).x
     *  add     temp.u32(49).x, temp.u32(47).x, 16
     *  load    temp(50).x, uniform.u32(stageNodes).x, temp.u32(49).x
     *  i2f     temp(51), temp.s32(48)
     *  mul     temp(52), temp(51), temp(13).x
     *  mul     temp(53).x, temp(50).x, temp(16).x

     *  mul     temp(54).x, temp(52).z, temp(52).w
     *  mul     temp(46).x, temp(54).x, temp(53).x

     *  f2i     temp.s32(55), temp(52)
     *  add     temp.s32(56).xy, temp.s32(55).xy, temp.s32(11).xy
     *  mov     temp.s32(57).xy, temp.s32(56).xy
     *  add     temp.s32(57).zw, temp.s32(56).xxxy, temp.s32(55).zzzw
     *  max     temp.s32(58), temp.s32(57), 0
     *  min     temp.u32(59), temp.u32(58), uniform.u32(integralImage#size).zwzw
     *  lshift  temp.u32(60), temp.u32(59), temp.u32(20).xyxy
     *  mul     temp.u32(61), temp.u32(60).ywwy, uniform.u32(integralImage#size).y
     *  add     temp.u32(62), temp.u32(61), temp.u32(60).xzxz
     *  load    temp.u32(63).x, uniform.u32(integralImage).x, temp.u32(62).x
     *  load    temp.u32(63).y, uniform.u32(integralImage).x, temp.u32(62).y
     *  load    temp.u32(63).z, uniform.u32(integralImage).x, temp.u32(62).z
     *  load    temp.u32(63).w, uniform.u32(integralImage).x, temp.u32(62).w
     *  add     temp.u32(64).xy, temp.u32(63).xz, temp.u32(63).yw
     *  sub     temp.u32(65).x, temp.u32(64).x, temp.u32(64).y
     *  i2f     temp(66).x, temp.u32(65).x
     *  mul     temp(45).x, temp(66).x, temp(53).x

     *  4:
     *  sub     temp.u32(67).x, temp.u32(38).x, 80
     *  load    temp.s32(68), uniform.u32(stageNodes).x, temp.u32(67).x
     *  add     temp.u32(69).x, temp.u32(67).x, 16
     *  load    temp(70).x, uniform.u32(stageNodes).x, temp.u32(69).x
     *  i2f     temp(71), temp.s32(68)
     *  mul     temp(72), temp(71), temp(13).x
     *  mul     temp(73).x, temp(70).x, temp(16).x

     *  mul     temp(74).x, temp(72).z, temp(72).w
     *  mul     temp(75).x, temp(74).x, temp(73).x
     *  add     temp(76).x, temp(75).x, temp(46).x

     *  f2i     temp.s32(77), temp(72)
     *  add     temp.s32(78).xy, temp.s32(77).xy, temp.s32(11).xy
     *  mov     temp.s32(79).xy, temp.s32(78).xy
     *  add     temp.s32(79).zw, temp.s32(78).xxxy, temp.s32(77).zzzw
     *  max     temp.s32(80), temp.s32(79), 0
     *  min     temp.u32(81), temp.u32(80), uniform.u32(integralImage#size).zwzw
     *  lshift  temp.u32(82), temp.u32(81), temp.u32(20).xyxy
     *  mul     temp.u32(83), temp.u32(82).ywwy, uniform.u32(integralImage#size).y
     *  add     temp.u32(84), temp.u32(83), temp.u32(107).xzxz
     *  load    temp.u8(85).x, uniform.u32(integralImage).x, temp.u32(84).x
     *  load    temp.u8(85).y, uniform.u32(integralImage).x, temp.u32(84).y
     *  load    temp.u8(85).z, uniform.u32(integralImage).x, temp.u32(84).z
     *  load    temp.u8(85).w, uniform.u32(integralImage).x, temp.u32(84).w
     *  add     temp.u32(86).xy, temp.u32(85).xz, temp.u32(85).yw
     *  sub     temp.u32(87).x, temp.u32(86).x, temp.u32(120).y
     *  i2f     temp(88).x, temp.u32(87).x
     *  mul     temp(89).x, temp(88).x, temp(73).x
     *  add     temp(90).x, temp(89).x, temp(45).x

     *  sub     temp.u32(91).x, temp.u32(38).x, 120
     *  load    temp.s32(92), uniform.u32(stageNodes).x, temp.u32(91).x
     *  i2f     temp(93), temp.s32(92)
     *  mul     temp(94), temp(93), temp(13).x

     *  mul     temp(95).x, temp(94).z, temp(94).w
     *  rcp     temp(96).x, temp(95).x
     *  mul     temp(97).x, temp(76).x, -1
     *  mul     temp(98).x, temp(97).x, temp(96).x

     *  f2i     temp.s32(99), temp(94)
     *  add     temp.s32(100).xy, temp.s32(99).xy, temp.s32(11).xy
     *  mov     temp.s32(101).xy, temp.s32(100).xy
     *  add     temp.s32(101).zw, temp.s32(100).xxxy, temp.s32(99).zzzw
     *  max     temp.s32(102), temp.s32(101), 0
     *  min     temp.u32(103), temp.u32(102), uniform.u32(integralImage#size).zwzw
     *  lshift  temp.u32(104), temp.u32(103), temp.u32(20).xyxy
     *  mul     temp.u32(105), temp.u32(104).ywwy, uniform.u32(integralImage#size).y
     *  add     temp.u32(106), temp.u32(105), temp.u32(104).xzxz
     *  load    temp.u32(107).x, uniform.u32(integralImage).x, temp.u32(106).x
     *  load    temp.u32(107).y, uniform.u32(integralImage).x, temp.u32(106).y
     *  load    temp.u32(107).z, uniform.u32(integralImage).x, temp.u32(106).z
     *  load    temp.u32(107).w, uniform.u32(integralImage).x, temp.u32(106).w
     *  add     temp.u32(108).xy, temp.u32(107).xz, temp.u32(107).yw
     *  sub     temp.u32(109).x, temp.u32(108).x, temp.u32(108).y
     *  i2f     temp(110).x, temp.u32(109).x
     *  mul     temp(111).x, temp(110).x, temp(98).x
     *  add     temp(112).x, temp(111).x, temp(90).x
     *  mov     temp(45).x, temp(112).x

     *  5:
     *  mul     temp(113).x, temp(43).x, temp(37).x
     *  mov     temp(114).x, temp(43).y
     *  jmp.lt  6, temp(45).x, temp(113).x
     *  mov     temp(114).x, temp(43).z
     *  6:
     *  add     temp(115).x, temp(42).x, temp(114).x
     *  mov     temp(42).x, temp(115).x

     *  sub     temp.s32(116).x, temp.s32(40).x, 1
     *  mov     temp.s32(40).x, temp.s32(116).x
     *  jmp.gt  3, temp.s32(40).x, 0

     *  mov     temp.s32(117).x, 0
     *  jmp.le  7, temp(42).x, temp(41).x

     *  mov     temp.u32(118).x, 1
     *  mov     temp.u32(118).y, 4
     *  add     temp.u32(119).xy, temp.u32(39).xy, temp.u32(118).xy
     *  mov     temp.u32(39).xy, temp.u32(119).xy
     *  jmp.lt  2, temp.s32(39).x, uniform.s32(stagesCount).x

     *  mov     temp.s32(117).x, 1

     *  7:
     *  lshift  temp.u32(120).x, temp.u32(9).x, 2
     *  add     temp.u32(121).x, uniform.u32(resRectangles).x, temp(120).x
     *  store1  temp.s32(122).x, temp.u32(121).x, temp.s32(117).x
     *  8:
     */
    clmOPCODE_U32(MOV, 9, X); clmATTRIBUTE_U32(global_id, XXXX);
    clmJUMP_S32(GREATER_OR_EQUAL, 8); clmTEMP_S32(9, XXXX); clmUNIFORM_S32(rectCount, XXXX);

    clmOPCODE_U32(MUL, 10, X); clmTEMP_U32(9, XXXX); clmU32(20);
    clmOPCODE_S32(LOAD, 11, XYZW); clmUNIFORM_U32(haarRects, XXXX); clmTEMP_U32(10, XXXX);
    clmOPCODE_U32(ADD, 12, X); clmTEMP_U32(10, XXXX); clmU32(16);
    clmOPCODE(LOAD, 13, X); clmUNIFORM_U32(haarRects, XXXX); clmTEMP_U32(12, XXXX);

    clmOPCODE_S32(I2F, 14, XY); clmTEMP_S32(11, ZWWW);
    clmOPCODE(MUL, 15, X); clmTEMP(14, XXXX); clmTEMP(14, YYYY);
    clmOPCODE(RCP, 16, X); clmTEMP(15, XXXX);

    clmOPCODE_S32(MOV, 17, XY); clmTEMP_S32(11, XYYY);
    clmOPCODE_S32(ADD, 17, ZW); clmTEMP_S32(11, XXXY); clmTEMP_S32(11, ZZZW);
    clmOPCODE_S32(MAX, 18, XYZW); clmTEMP_S32(17, XYZW); clmS32(0);
    clmOPCODE_U32(MIN, 19, XYZW); clmTEMP_U32(18, XYZW); clmUNIFORM_U32(integralImage_size, ZWZW);
    clmOPCODE_U32(MOV, 20, X); clmU32(2);
    clmOPCODE_U32(MOV, 20, Y); clmU32(0);
    clmOPCODE_U32(LSHIFT, 21, XYZW); clmTEMP_U32(19, XYZW); clmTEMP_U32(20, XYXY);
    clmOPCODE_U32(MUL, 22, XYZW); clmTEMP_U32(21, YWWY); clmUNIFORM_U32(integralImage_size, YYYY);
    clmOPCODE_U32(ADD, 23, XYZW); clmTEMP_U32(22, XYZW); clmTEMP_U32(21, XZXZ);
    clmOPCODE_U32(LOAD, 24, X); clmUNIFORM_U32(integralImage, XXXX); clmTEMP_U32(23, XXXX);
    clmOPCODE_U32(LOAD, 24, Y); clmUNIFORM_U32(integralImage, XXXX); clmTEMP_U32(23, YYYY);
    clmOPCODE_U32(LOAD, 24, Z); clmUNIFORM_U32(integralImage, XXXX); clmTEMP_U32(23, ZZZZ);
    clmOPCODE_U32(LOAD, 24, W); clmUNIFORM_U32(integralImage, XXXX); clmTEMP_U32(23, WWWW);
    clmOPCODE_U32(ADD, 25, XY); clmTEMP_U32(24, XZZZ); clmTEMP_U32(24, YWWW);
    clmOPCODE_U32(SUB, 26, X); clmTEMP_U32(25, XXXX); clmTEMP_U32(25, YYYY);
    clmOPCODE_U32(MIN, 27, XYZW); clmTEMP_U32(18, XYZW); clmUNIFORM_U32(integral2Image_size, ZWZW);
    clmOPCODE_U32(LSHIFT, 28, XYZW); clmTEMP_U32(27, XYZW); clmTEMP_U32(20, XYXY);
    clmOPCODE_U32(MUL, 29, XYZW); clmTEMP_U32(28, YWWY); clmUNIFORM_U32(integral2Image_size, YYYY);
    clmOPCODE_U32(ADD, 30, XYZW); clmTEMP_U32(29, XYZW); clmTEMP_U32(28, XZXZ);
    clmOPCODE_U32(LOAD, 31, X); clmUNIFORM_U32(integral2Image, XXXX); clmTEMP_U32(30, XXXX);
    clmOPCODE_U32(LOAD, 31, Y); clmUNIFORM_U32(integral2Image, XXXX); clmTEMP_U32(30, YYYY);
    clmOPCODE_U32(LOAD, 31, Z); clmUNIFORM_U32(integral2Image, XXXX); clmTEMP_U32(30, ZZZZ);
    clmOPCODE_U32(LOAD, 31, W); clmUNIFORM_U32(integral2Image, XXXX); clmTEMP_U32(30, WWWW);
    clmOPCODE_U32(ADD, 32, XY); clmTEMP_U32(31, XZZZ); clmTEMP_U32(31, YWWW);
    clmOPCODE_U32(SUB, 26, Y); clmTEMP_U32(32, XXXX); clmTEMP_U32(32, YYYY);
    clmOPCODE_U32(I2F, 33, XY); clmTEMP_U32(26, XYYY);
    clmOPCODE(MUL, 34, XY); clmTEMP(33, XYYY); clmTEMP(16, XXXX);
    clmOPCODE(MUL, 35, X); clmTEMP(34, XXXX); clmTEMP(34, XXXX);
    clmOPCODE(SUB, 36, X); clmTEMP(34, YYYY); clmTEMP(35, XXXX);

    clmOPCODE(SQRT, 37, X); clmTEMP(36, XXXX);
    clmJUMP(GREATER_OR_EQUAL, 1); clmTEMP(36, XXXX); clmFLOAT(0.0f);
    clmOPCODE(MOV, 37, X); clmFLOAT(1.0f);
    clmLABEL(1);

    clmOPCODE_U32(MOV, 38, X); clmU32(0);
    clmOPCODE_U32(MOV, 39, XY); clmU32(0);

    clmLABEL(2);
    clmOPCODE_S32(LOAD, 40, X); clmUNIFORM_U32(stageNodeCounts, XXXX); clmTEMP_U32(39, YYYY);
    clmOPCODE(LOAD, 41, X); clmUNIFORM_U32(stageThresholds, XXXX); clmTEMP_U32(39, YYYY);

    clmOPCODE(MOV, 42, X); clmFLOAT(0.0f);

    clmLABEL(3);
    clmOPCODE(LOAD, 43, XYZW); clmUNIFORM_U32(stageNodes, XXXX); clmTEMP_U32(38, XXXX);
    clmOPCODE_U32(ADD, 44, X); clmTEMP_U32(38, XXXX); clmU32(136);
    clmOPCODE_U32(MOV, 38, X); clmTEMP_U32(44, XXXX);

    clmOPCODE(MOV, 45, X); clmFLOAT(0.0f);
    clmOPCODE(MOV, 46, X); clmFLOAT(0.0f);

    clmJUMP_S32(EQUAL, 4); clmTEMP_S32(43, WWWW); clmS32(2);
    clmJUMP_S32(NOT_EQUAL, 5); clmTEMP_S32(43, WWWW); clmS32(3);

    clmOPCODE_U32(SUB, 47, X); clmTEMP_U32(38, XXXX); clmU32(40);
    clmOPCODE_S32(LOAD, 48, XYZW); clmUNIFORM_U32(stageNodes, XXXX); clmTEMP_U32(47, XXXX);
    clmOPCODE_U32(ADD, 49, X); clmTEMP_U32(47, XXXX); clmU32(16);
    clmOPCODE(LOAD, 50, X); clmUNIFORM_U32(stageNodes, XXXX); clmTEMP_U32(49, XXXX);
    clmOPCODE_S32(I2F, 51, XYZW); clmTEMP_S32(48, XYZW);
    clmOPCODE(MUL, 52, XYZW); clmTEMP(51, XYZW); clmTEMP(13, XXXX);
    clmOPCODE(MUL, 53, X); clmTEMP(50, XXXX); clmTEMP(16, XXXX);

    clmOPCODE(MUL, 54, X); clmTEMP(69, ZZZZ); clmTEMP(52, WWWW);
    clmOPCODE(MUL, 46, X); clmTEMP(54, XXXX); clmTEMP(53, XXXX);

    clmOPCODE_S32(F2I, 55, XYZW); clmTEMP(52, XYZW);
    clmOPCODE_S32(ADD, 56, XY); clmTEMP_S32(55, XYYY); clmTEMP_S32(11, XYYY);
    clmOPCODE_S32(MOV, 57, XY); clmTEMP_S32(56, XYYY);
    clmOPCODE_S32(ADD, 57, ZW); clmTEMP_S32(56, XXXY); clmTEMP_S32(55, ZZZW);
    clmOPCODE_S32(MAX, 58, XYZW); clmTEMP_S32(57, XYZW); clmS32(0);
    clmOPCODE_U32(MIN, 59, XYZW); clmTEMP_U32(58, XYZW); clmUNIFORM_U32(integralImage_size, ZWZW);
    clmOPCODE_U32(LSHIFT, 60, XYZW); clmTEMP_U32(59, XYZW); clmTEMP_U32(20, XYXY);
    clmOPCODE_U32(MUL, 61, XYZW); clmTEMP_U32(60, YWWY); clmUNIFORM_U32(integralImage_size, YYYY);
    clmOPCODE_U32(ADD, 62, XYZW); clmTEMP_U32(61, XYZW); clmTEMP_U32(60, XZXZ);
    clmOPCODE_U32(LOAD, 63, X); clmUNIFORM_U32(integralImage, XXXX); clmTEMP_U32(62, XXXX);
    clmOPCODE_U32(LOAD, 63, Y); clmUNIFORM_U32(integralImage, XXXX); clmTEMP_U32(62, YYYY);
    clmOPCODE_U32(LOAD, 63, Z); clmUNIFORM_U32(integralImage, XXXX); clmTEMP_U32(62, ZZZZ);
    clmOPCODE_U32(LOAD, 63, W); clmUNIFORM_U32(integralImage, XXXX); clmTEMP_U32(62, WWWW);
    clmOPCODE_U32(ADD, 64, XY); clmTEMP_U32(63, XZZZ); clmTEMP_U32(63, YWWW);
    clmOPCODE_U32(SUB, 65, X); clmTEMP_U32(64, XXXX); clmTEMP_U32(64, YYYY);
    clmOPCODE_U32(I2F, 66, X); clmTEMP_U32(65, XXXX);
    clmOPCODE(MUL, 45, X); clmTEMP(66, XXXX); clmTEMP(53, XXXX);

    clmLABEL(4);
    clmOPCODE_U32(SUB, 67, X); clmTEMP_U32(38, XXXX); clmU32(80);
    clmOPCODE_S32(LOAD, 68, XYZW); clmUNIFORM_U32(stageNodes, XXXX); clmTEMP_U32(67, XXXX);
    clmOPCODE_U32(ADD, 69, X); clmTEMP_U32(67, XXXX); clmU32(16);
    clmOPCODE(LOAD, 70, X); clmUNIFORM_U32(stageNodes, XXXX); clmTEMP_U32(69, XXXX);
    clmOPCODE_S32(I2F, 71, XYZW); clmTEMP_S32(68, XYZW);
    clmOPCODE(MUL, 72, XYZW); clmTEMP(71, XYZW); clmTEMP(13, XXXX);
    clmOPCODE(MUL, 73, X); clmTEMP(70, XXXX); clmTEMP(16, XXXX);

    clmOPCODE(MUL, 74, X); clmTEMP(72, ZZZZ); clmTEMP(72, WWWW);
    clmOPCODE(MUL, 75, X); clmTEMP(74, XXXX); clmTEMP(73, XXXX);
    clmOPCODE(ADD, 76, X); clmTEMP(75, XXXX); clmTEMP(46, XXXX);

    clmOPCODE_S32(F2I, 77, XYZW); clmTEMP(72, XYZW);
    clmOPCODE_S32(ADD, 78, XY); clmTEMP_S32(77, XYYY); clmTEMP_S32(11, XYYY);
    clmOPCODE_S32(MOV, 79, XY); clmTEMP_S32(78, XYYY);
    clmOPCODE_S32(ADD, 79, ZW); clmTEMP_S32(78, XXXY); clmTEMP_S32(77, ZZZW);
    clmOPCODE_S32(MAX, 80, XYZW); clmTEMP_S32(79, XYZW); clmS32(0);
    clmOPCODE_U32(MIN, 81, XYZW); clmTEMP_U32(80, XYZW); clmUNIFORM_U32(integralImage_size, ZWZW);
    clmOPCODE_U32(LSHIFT, 82, XYZW); clmTEMP_U32(81, XYZW); clmTEMP_U32(20, XYXY);
    clmOPCODE_U32(MUL, 83, XYZW); clmTEMP_U32(82, YWWY); clmUNIFORM_U32(integralImage_size, YYYY);
    clmOPCODE_U32(ADD, 84, XYZW); clmTEMP_U32(83, XYZW); clmTEMP_U32(82, XZXZ);
    clmOPCODE_U32(LOAD, 85, X); clmUNIFORM_U32(integralImage, XXXX); clmTEMP_U32(84, XXXX);
    clmOPCODE_U32(LOAD, 85, Y); clmUNIFORM_U32(integralImage, XXXX); clmTEMP_U32(84, YYYY);
    clmOPCODE_U32(LOAD, 85, Z); clmUNIFORM_U32(integralImage, XXXX); clmTEMP_U32(84, ZZZZ);
    clmOPCODE_U32(LOAD, 85, W); clmUNIFORM_U32(integralImage, XXXX); clmTEMP_U32(84, WWWW);
    clmOPCODE_U32(ADD, 86, XY); clmTEMP_U32(85, XZZZ); clmTEMP_U32(85, YWWW);
    clmOPCODE_U32(SUB, 87, X); clmTEMP_U32(86, XXXX); clmTEMP_U32(86, YYYY);
    clmOPCODE_U32(I2F, 88, X); clmTEMP_U32(87, XXXX);
    clmOPCODE(MUL, 89, X); clmTEMP(88, XXXX); clmTEMP(73, XXXX);
    clmOPCODE(ADD, 90, X); clmTEMP(89, XXXX); clmTEMP(45, XXXX);

    clmOPCODE_U32(SUB, 91, X); clmTEMP_U32(38, XXXX); clmU32(120);
    clmOPCODE_S32(LOAD, 92, XYZW); clmUNIFORM_U32(stageNodes, XXXX); clmTEMP_U32(91, XXXX);
    clmOPCODE_S32(I2F, 93, XYZW); clmTEMP_S32(92, XYZW);
    clmOPCODE(MUL, 94, XYZW); clmTEMP(93, XYZW); clmTEMP(13, XXXX);

    clmOPCODE(MUL, 95, X); clmTEMP(94, ZZZZ); clmTEMP(94, WWWW);
    clmOPCODE(RCP, 96, X); clmTEMP(95, XXXX);
    clmOPCODE(MUL, 97, X); clmTEMP(76, XXXX); clmFLOAT(-1.0f);
    clmOPCODE(MUL, 98, X); clmTEMP(97, XXXX); clmTEMP(96, XXXX);

    clmOPCODE_S32(F2I, 99, XYZW); clmTEMP(94, XYZW);
    clmOPCODE_S32(ADD, 100, XY); clmTEMP_S32(99, XYYY); clmTEMP_S32(11, XYYY);
    clmOPCODE_S32(MOV, 101, XY); clmTEMP_S32(100, XYYY);
    clmOPCODE_S32(ADD, 101, ZW); clmTEMP_S32(100, XXXY); clmTEMP_S32(99, ZZZW);
    clmOPCODE_S32(MAX, 102, XYZW); clmTEMP_S32(101, XYZW); clmS32(0);
    clmOPCODE_U32(MIN, 103, XYZW); clmTEMP_U32(102, XYZW); clmUNIFORM_U32(integralImage_size, ZWZW);
    clmOPCODE_U32(LSHIFT, 104, XYZW); clmTEMP_U32(103, XYZW); clmTEMP_U32(20, XYXY);
    clmOPCODE_U32(MUL, 105, XYZW); clmTEMP_U32(104, YWWY); clmUNIFORM_U32(integralImage_size, YYYY);
    clmOPCODE_U32(ADD, 106, XYZW); clmTEMP_U32(105, XYZW); clmTEMP_U32(104, XZXZ);
    clmOPCODE_U32(LOAD, 107, X); clmUNIFORM_U32(integralImage, XXXX); clmTEMP_U32(106, XXXX);
    clmOPCODE_U32(LOAD, 107, Y); clmUNIFORM_U32(integralImage, XXXX); clmTEMP_U32(106, YYYY);
    clmOPCODE_U32(LOAD, 107, Z); clmUNIFORM_U32(integralImage, XXXX); clmTEMP_U32(106, ZZZZ);
    clmOPCODE_U32(LOAD, 107, W); clmUNIFORM_U32(integralImage, XXXX); clmTEMP_U32(106, WWWW);
    clmOPCODE_U32(ADD, 108, XY); clmTEMP_U32(107, XZZZ); clmTEMP_U32(107, YWWW);
    clmOPCODE_U32(SUB, 109, X); clmTEMP_U32(108, XXXX); clmTEMP_U32(108, YYYY);
    clmOPCODE_U32(I2F, 110, X); clmTEMP_U32(109, XXXX);
    clmOPCODE(MUL, 111, X); clmTEMP(110, XXXX); clmTEMP(98, XXXX);
    clmOPCODE(ADD, 112, X); clmTEMP(111, XXXX); clmTEMP(90, XXXX);
    clmOPCODE(MOV, 45, X); clmTEMP(112, XXXX);

    clmLABEL(5);
    clmOPCODE(MUL, 113, X); clmTEMP(43, XXXX); clmTEMP(37, XXXX);
    clmOPCODE(MOV, 114, X); clmTEMP(43, YYYY);
    clmJUMP(LESS, 6); clmTEMP(45, XXXX); clmTEMP(113, XXXX);
    clmOPCODE(MOV, 114, X); clmTEMP(43, ZZZZ);
    clmLABEL(6);
    clmOPCODE(ADD, 115, X); clmTEMP(42, XXXX); clmTEMP(114, XXXX);
    clmOPCODE(MOV, 42, X); clmTEMP(115, XXXX);

    clmOPCODE_S32(SUB, 116, X); clmTEMP_S32(40, XXXX); clmS32(1);
    clmOPCODE_S32(MOV, 40, X); clmTEMP_S32(116, XXXX);
    clmJUMP_S32(GREATER, 3); clmTEMP_S32(40, XXXX); clmS32(0);

    clmOPCODE_S32(MOV, 117, X); clmS32(0);
    clmJUMP(LESS_OR_EQUAL, 7); clmTEMP(42, XXXX); clmTEMP(41, XXXX);

    clmOPCODE_U32(MOV, 118, X); clmU32(1);
    clmOPCODE_U32(MOV, 118, Y); clmU32(4);
    clmOPCODE_U32(ADD, 119, XY); clmTEMP_U32(39, XYYY); clmTEMP_U32(118, XYYY);
    clmOPCODE_U32(MOV, 39, XY); clmTEMP_U32(119, XYYY);
    clmJUMP_S32(LESS, 2); clmTEMP_S32(39, XXXX); clmUNIFORM_S32(stagesCount, XXXX);

    clmOPCODE_S32(MOV, 117, X); clmS32(1);

    clmLABEL(7);
    clmOPCODE_U32(LSHIFT, 120, X); clmTEMP_U32(9, XXXX); clmU32(2);
    clmOPCODE_U32(ADD, 121, X); clmUNIFORM_U32(resRectangles, XXXX); clmTEMP_U32(120, XXXX);
    clmOPCODE_S32(STORE1, 122, X); clmTEMP_U32(121, XXXX); clmTEMP_S32(117, XXXX);
    clmLABEL(8);

    /* main. */
    clmMAIN();

    /*
     *  __kernel void oggDecode(
     *      __write_only image2d_t output,
     *      const __global uchar* yChannel,
     *      const __global uchar* uChannel,
     *      const __global uchar* vChannel,
     *      const int yStride,
     *      const int uStride,
     *      const int vStride,
     *      const int hdec,
     *      const int vdec)
     *  {
     *      const int x = get_global_id(0);
     *      const int y = get_global_id(1);
     *
     *      //int row = y/2;//y % 2 ? (y+1) % 2 : (y/2);
     *      uchar yCh = yChannel[y*yStride+x];
     *      uchar uCh = uChannel[((y/2)*uStride)+(x>>hdec)];
     *      uchar vCh = vChannel[((y/2)*vStride)+(x>>hdec)];
     *      int r = (1904000 * yCh + 2609823 * vCh - 363703744) / 1635200;
     *      int g = (3827562 * yCh - 1287801 * uCh - 2672387 * vCh + 447306710) / 3287200;
     *      int b = (952000  * yCh + 1649289 * uCh - 225932192) / 817600;
     *
     *      write_imageui(output, (int2)(x, y), (uint4)(OC_CLAMP255(r), OC_CLAMP255(g), OC_CLAMP255(b), 255) );
     *  }
     */

    clmKERNEL("oggDecode", 36);
    clmARGUMENT_IMAGE(output, 0, gcvFALSE);
    clmARGUMENT_U32(yChannel, 1, gcvTRUE, UINT8, gcvTRUE);
    clmARGUMENT_U32(uChannel, 2, gcvTRUE, UINT8, gcvTRUE);
    clmARGUMENT_U32(vChannel, 3, gcvTRUE, UINT8, gcvTRUE);
    clmARGUMENT_U32(yStride, 4, gcvFALSE, INTEGER, gcvFALSE);
    clmARGUMENT_U32(uStride, 5, gcvFALSE, INTEGER, gcvFALSE);
    clmARGUMENT_U32(vStride, 6, gcvFALSE, INTEGER, gcvFALSE);
    clmARGUMENT_U32(hdec, 7, gcvFALSE, INTEGER, gcvFALSE);
    clmARGUMENT_U32(vdec, 8, gcvFALSE, INTEGER, gcvFALSE);

    /*  mul     temp.u32(9).x, attribute.u32(global_id).y, uniform.u32(yStride).x
     *  add     temp.u32(10).x, temp.u32(9).x, attribute.u32(global_id).x
     *  load    temp.u8(11).x, uniform.u32(yChannel).x, temp.u32(10).x
     *  rshift  temp.u32(12).x, attribute.u32(global_id).x, uniform.u32(hdec).x
     *  rshift  temp.u32(13).x, attribute.u32(global_id).y, 1
     *  mul     temp.u32(14).x, temp.u32(13).x, uniform.u32(uStride).x
     *  add     temp.u32(15).x, temp.u32(14).x, temp.u32(12).x
     *  load    temp.u8(16).x, uniform.u32(uChannel).x, temp.u32(15).x
     *  mul     temp.u32(17).x, temp.u32(13).x, uniform.u32(vStride).x
     *  add     temp.u32(18).x, temp.u32(17).x, temp.u32(12).x
     *  load    temp.u8(19).x, uniform.u32(vChannel).x, temp.u32(18).x
     *  lshift  temp.u32(20).x, attribute.u32(global_id).x, 2
     *  mul     temp.u32(21).x, attribute.u32(global_id).y, uniform.u32(output#size).y
     *  add     temp.u32(22).x, temp.u32(21).x, temp.u32(20).x
     *  mov     temp.u32(23).x, temp.u8(11).x
     *  mov     temp.u32(23).y, temp.u8(16).x
     *  mov     temp.u32(23).z, temp.u8(19).x
     *  mov     temp.u32(23).w, 1
     *  i2f     temp(24), temp.u8(23)
     *  mov     temp(25).x, 1904000 / 1635200
     *  mov     temp(25).y, 0 / 1635200
     *  mov     temp(25).z, 2609823 / 1635200
     *  mov     temp(25).w, -363703744 / 1635200
     *  dp4     temp(28).x, temp(24), temp(25)
     *  mov     temp(26).x, 3827562 / 3287200
     *  mov     temp(26).y, -1287801 / 3287200
     *  mov     temp(26).z, -2672887 / 3287200
     *  mov     temp(26).w, 447306710 / 3287200
     *  dp4     temp(28).y, temp(24), temp(26)
     *  mov     temp(27).x, 952000 / 817600
     *  mov     temp(27).y, 1649289 / 817600
     *  mov     temp(27).z, 0 / 817600
     *  mov     temp(27).w, -225932192 / 817600
     *  dp4     temp(28).z, temp(24), temp(27)
     *  mov     temp(29).x, 0.2989
     *  mov     temp(29).y, 0.5870
     *  mov     temp(29).z, 0.1140
     *  dp3     temp(30).x. temp(28).xyz, temp(29).xyz
     *  mov     temp(28).w, temp(30).x
     *  f2i     temp.s32(31), temp(28)
     *  max     temp.s32(32), temp.s32(31), 0
     *  min     temp.s32(33), temp.s32(32), 255
     *  add     temp.u32(34).x, uniform.u32(output).x, temp.u32(22).x
     *  store1  temp.u8(35), temp.u32(34).x, temp.s32(33)
     */
    clmOPCODE_U32(MUL, 9, X); clmATTRIBUTE_U32(global_id, YYYY); clmUNIFORM_U32(yStride, XXXX);
    clmOPCODE_U32(ADD, 10, X); clmTEMP_U32(9, XXXX); clmATTRIBUTE_U32(global_id, XXXX);
    clmOPCODE_U8(LOAD, 11, X); clmUNIFORM_U32(yChannel, XXXX); clmTEMP_U32(10, XXXX);
    clmOPCODE_U32(RSHIFT, 12, X); clmATTRIBUTE_U32(global_id, XXXX); clmUNIFORM_U32(hdec, XXXX);
    clmOPCODE_U32(RSHIFT, 13, X); clmATTRIBUTE_U32(global_id, YYYY); clmU32(1);
    clmOPCODE_U32(MUL, 14, X); clmTEMP_U32(13, XXXX); clmUNIFORM_U32(uStride, XXXX);
    clmOPCODE_U32(ADD, 15, X); clmTEMP_U32(14, XXXX); clmTEMP_U32(12, XXXX);
    clmOPCODE_U8(LOAD, 16, X); clmUNIFORM_U32(uChannel, XXXX); clmTEMP_U32(15, XXXX);
    clmOPCODE_U32(MUL, 17, X); clmTEMP_U32(13, XXXX); clmUNIFORM_U32(vStride, XXXX);
    clmOPCODE_U32(ADD, 18, X); clmTEMP_U32(17, XXXX); clmTEMP_U32(12, XXXX);
    clmOPCODE_U8(LOAD, 19, X); clmUNIFORM_U32(vChannel, XXXX); clmTEMP_U32(18, XXXX);
    clmOPCODE_U32(LSHIFT, 20, X); clmATTRIBUTE_U32(global_id, XXXX); clmU32(2);
    clmOPCODE_U32(MUL, 21, X); clmATTRIBUTE_U32(global_id, YYYY); clmUNIFORM_U32(output_size, YYYY);
    clmOPCODE_U32(ADD, 22, X); clmTEMP_U32(21, XXXX); clmTEMP_U32(20, XXXX);
    clmOPCODE_U32(MOV, 23, X); clmTEMP_U8(11, XXXX);
    clmOPCODE_U32(MOV, 23, Y); clmTEMP_U8(16, XXXX);
    clmOPCODE_U32(MOV, 23, Z); clmTEMP_U8(19, XXXX);
    clmOPCODE_U32(MOV, 23, W); clmU32(1);
    clmOPCODE_U8(I2F, 24, XYZW); clmTEMP_U8(23, XYZW);
    clmOPCODE(MOV, 25, X); clmFLOAT(1904000.0f / 1635200.0f);
    clmOPCODE(MOV, 25, Y); clmFLOAT(0.0f / 1635200.0f);
    clmOPCODE(MOV, 25, Z); clmFLOAT(2609823.0f / 1635200.0f);
    clmOPCODE(MOV, 25, W); clmFLOAT(-363703744.0f / 1635200.0f);
    clmOPCODE(DP4, 28, X); clmTEMP(24, XYZW); clmTEMP(25, XYZW);
    clmOPCODE(MOV, 26, X); clmFLOAT(3827562.0f / 3287200.0f);
    clmOPCODE(MOV, 26, Y); clmFLOAT(-1287801.0f / 3287200.0f);
    clmOPCODE(MOV, 26, Z); clmFLOAT(-2672387.0f / 3287200.0f);
    clmOPCODE(MOV, 26, W); clmFLOAT(447306710.0f / 3287200.0f);
    clmOPCODE(DP4, 28, Y); clmTEMP(24, XYZW); clmTEMP(26, XYZW);
    clmOPCODE(MOV, 27, X); clmFLOAT(952000.0f / 817600.0f);
    clmOPCODE(MOV, 27, Y); clmFLOAT(1649289.0f / 817600.0f);
    clmOPCODE(MOV, 27, Z); clmFLOAT(0.0f / 817600.0f);
    clmOPCODE(MOV, 27, W); clmFLOAT(-225932192.0f / 817600.0f);
    clmOPCODE(DP4, 28, Z); clmTEMP(24, XYZW); clmTEMP(27, XYZW);
    clmOPCODE(MOV, 29, X); clmFLOAT(0.2989f);
    clmOPCODE(MOV, 29, Y); clmFLOAT(0.5870f);
    clmOPCODE(MOV, 29, Z); clmFLOAT(0.1140f);
    clmOPCODE(DP3, 30, X); clmTEMP(28, XYZZ); clmTEMP(29, XYZZ);
    clmOPCODE(MOV, 28, W); clmTEMP(30, XXXX);
    clmOPCODE_S32(F2I, 31, XYZW); clmTEMP(28, XYZW);
    clmOPCODE_S32(MAX, 32, XYZW); clmTEMP_S32(31, XYZW); clmS32(0);
    clmOPCODE_S32(MIN, 33, XYZW); clmTEMP_S32(32, XYZW); clmS32(255);
    clmOPCODE_U32(ADD, 34, X); clmUNIFORM_U32(output, XXXX); clmTEMP_U32(22, XXXX);
    clmOPCODE_U8(STORE1, 35, XYZW); clmTEMP_U32(34, XXXX); clmTEMP_S32(33, XYZW);

    /* main. */
    clmMAIN();

    /* Pack the shader. */
    gcmONERROR(gcSHADER_Pack(shader));
    return shader;

OnError:
    if (shader != gcvNULL)
    {
        gcSHADER_Destroy(shader);
    }

    return gcvNULL;
}

static gcSHADER clTune_21(IN gcSHADER Shader)
{
    gcSHADER shader = gcvNULL;
    gceSTATUS status;
    gcKERNEL_FUNCTION kernel;
    gctUINT tempStart;
    gcATTRIBUTE global_id;
    gcUNIFORM global_size;
    gctINT property[3] = { 0, 0, 0 };
    gcUNIFORM source, source_size;
    gcUNIFORM result, result_size;
    gcUNIFORM dstImage, dstImage_size;
    gcUNIFORM value, height;
    gcUNIFORM srcImage, srcImage_size;
    gcUNIFORM sumImage, sumImage_size;
    gctUINT32 compilerVersion[2];

    /* Construct a new shader. */
    gcmONERROR(gcSHADER_Construct(gcSHADER_TYPE_CL, &shader));

    /* Set OpenCL arguments. */
    cloCOMPILER_GetVersion(gcvNULL, clvSHADER_TYPE_CL, compilerVersion);
    gcmONERROR(gcSHADER_SetCompilerVersion(shader, compilerVersion));
    gcmONERROR(gcSHADER_SetMaxKernelFunctionArgs(shader, 4));

    /* Allocate attributes and uniforms. */
    gcmONERROR(gcSHADER_AddAttribute(shader, "#global_id", gcSHADER_INTEGER_X4, 1, gcvFALSE, gcSHADER_SHADER_DEFAULT, gcSHADER_PRECISION_DEFAULT,
                                     &global_id));
    gcmONERROR(gcSHADER_AddUniform(shader, "#global_size", gcSHADER_INTEGER_X4, 1, gcSHADER_PRECISION_DEFAULT, &global_size));
    gcmONERROR(gcUNIFORM_SetFlags(global_size, gcvUNIFORM_KIND_GLOBAL_SIZE));
    gcmONERROR(gcUNIFORM_SetFormat(global_size, gcSL_UINT32, gcvFALSE));
    gcmONERROR(gcSHADER_AddUniform(shader, "source#size", gcSHADER_INTEGER_X4, 1, gcSHADER_PRECISION_DEFAULT, &source_size));
    gcmONERROR(gcSHADER_AddUniform(shader, "result#size", gcSHADER_INTEGER_X4, 1, gcSHADER_PRECISION_DEFAULT, &result_size));
    gcmONERROR(gcSHADER_AddUniform(shader, "dstImage#size", gcSHADER_INTEGER_X4, 1, gcSHADER_PRECISION_DEFAULT, &dstImage_size));
    gcmONERROR(gcSHADER_AddUniform(shader, "srcImage#size", gcSHADER_INTEGER_X4, 1, gcSHADER_PRECISION_DEFAULT, &srcImage_size));
    gcmONERROR(gcSHADER_AddUniform(shader, "sumImage#size", gcSHADER_INTEGER_X4, 1, gcSHADER_PRECISION_DEFAULT, &sumImage_size));

    /*
     *  __kernel void createIntensityImage(
     *      read_only image2d_t source,
     *      write_only image2d_t result)
     *  {
     *      uint x = get_global_id(0);
     *      uint y = get_global_id(1);
     *      int2 coords = (int2)(x, y);
     *
     *      const sampler_t smp = CLK_FILTER_NEAREST;
     *
     *      uint4 color = read_imageui(source, smp, coords);
     *      color.w = color.x * 0.2989f + color.y * 0.5870f + color.z * 0.1140f;
     *      write_imageui(result, coords, color);
     *  }
     */
    clmKERNEL("createIntensityImage", 14);
    clmARGUMENT_IMAGE(source, 0, gcvTRUE);
    clmARGUMENT_IMAGE(result, 1, gcvFALSE);

    /*  mov     temp.u32(2).x, 2
     *  mov     temp.u32(2).y, 4
     *  lshift  temp.u32(3).xy, attribute.u32(global_id).x, temp.u32(2).xy
     *  jmp.ge  1, temp.u32(3).x, uniform.u32(source#size).x
     *  mul     temp.u32(4).x, attribute.u32(global_id).y, uniform.u32(source#size).y
     *  add     temp.u32(5).x, temp.u32(4).x, temp.u32(3).y
     *  load    temp.u32(6), uniform.u32(source).x, temp.u32(5).x
     *  mul     temp.u32(7).x, attribute.u32(global_id).y, uniform.u32(result#size).y
     *  add     temp.u32(8).x, temp.u32(7).x, temp.u32(3).x
     *  mov     temp.u32(9).x, 0x0000FB73
     *  mov     temp.u32(9).y, 0x00000000
     *  mov     temp.u32(9).z, 0x000F
     *  unpack  temp.u8(10), temp.u32(6), temp.u32(9).xyz
     *  and     temp.u32(11), temp.u8(10), temp.u32(9).z
     *  add     temp.u32(12).x, uniform.u32(result).x, temp.u32(8).x
     *  store1  temp.u32(13).x, temp.u32(12).x, temp.u32(11).x
     *  1:
     */
    clmOPCODE_U32(MOV, 2, X); clmU32(2);
    clmOPCODE_U32(MOV, 2, Y); clmU32(4);
    clmOPCODE_U32(LSHIFT, 3, XY); clmATTRIBUTE_U32(global_id, XXXX); clmTEMP_U32(2, XYYY);
    clmJUMP_U32(GREATER_OR_EQUAL, 1); clmTEMP_U32(3, XXXX); clmUNIFORM_U32(source_size, XXXX);
    clmOPCODE_U32(MUL, 4, X); clmATTRIBUTE_U32(global_id, YYYY); clmUNIFORM_U32(source_size, YYYY);
    clmOPCODE_U32(ADD, 5, X); clmTEMP_U32(4, XXXX); clmTEMP_U32(3, YYYY);
    clmOPCODE_U32(LOAD, 6, XYZW); clmUNIFORM_U32(source, XXXX); clmTEMP_U32(5, XXXX);
    clmOPCODE_U32(MUL, 7, X); clmATTRIBUTE_U32(global_id, YYYY); clmUNIFORM_U32(result_size, YYYY);
    clmOPCODE_U32(ADD, 8, X); clmTEMP_U32(7, XXXX); clmTEMP_U32(3, XXXX);
    clmOPCODE_U32(MOV, 9, X); clmU32(0x0000FB73);
    clmOPCODE_U32(MOV, 9, Y); clmU32(0x00000000);
    clmOPCODE_U32(MOV, 9, Z); clmU32(0x000F);
    clmOPCODE_U8(UNPACK, 10, XYZW); clmTEMP_U32(6, XYZW); clmTEMP_U32(9, XYZZ);
    clmOPCODE_U32(AND_BITWISE, 11, XYZW); clmTEMP_U8(10, XYZW); clmTEMP_U32(9, ZZZZ);
    clmOPCODE_U32(ADD, 12, X); clmUNIFORM_U32(result, XXXX); clmTEMP_U32(8, XXXX);
    clmOPCODE_U32(STORE1, 13, X); clmTEMP_U32(12, XXXX); clmTEMP_U32(11, XXXX);
    clmLABEL(1);

    /*  main */
    clmMAIN();

    /*
     *  __kernel void ImageUIntSetValueA(
     *      write_only image2d_t dstImage,
     *      const uint value
     *      )
     *  {
     *      uint x = get_global_id(0);
     *      uint y = get_global_id(1);
     *      int2 coords = (int2)(x, y);
     *      uint4 color;
     *      color.w = value;
     *
     *      write_imageui(dstImage, coords, color);
     *  }
     */
    clmKERNEL("ImageUIntSetValueA", 8);
    clmARGUMENT_IMAGE(dstImage, 0, gcvTRUE);
    clmARGUMENT_U32(value, 1, gcvTRUE, UINT32, gcvFALSE);

    /*  mov     temp.u32(2).x, 2
     *  mov     temp.u32(2).y, 4
     *  lshift  temp.u32(3).xy, attribute.u32(global_id).x, temp.u32(2).xy
     *  jmp.ge  3, temp.u32(3).x, uniform.u32(dstImage#size).x
     *  jmp.eq  2, attribute.u32(global_id).x, 0
     *  jmp.ne  3, attribute.u32(global_id).y, 0
     *  2:
     *  mul     temp.u32(4).x, attribute.u32(global_id).y, uniform.u32(dstImage#size).y
     *  add     temp.u32(5).x, temp.u32(4).x, temp.u32(3).y
     *  add     temp.u32(6).x, uniform.u32(dstImage).x, temp.u32(5).x
     *  store1  temp.u32(7), temp.u32(6).x, uniform.u32(value).x
     *  3:
     */
    clmOPCODE_U32(MOV, 2, X); clmU32(2);
    clmOPCODE_U32(MOV, 2, Y); clmU32(4);
    clmOPCODE_U32(LSHIFT, 3, XY); clmATTRIBUTE_U32(global_id, XXXX); clmTEMP_U32(2, XYYY);
    clmJUMP_U32(GREATER_OR_EQUAL, 3); clmTEMP_U32(3, XXXX); clmUNIFORM_U32(dstImage_size, XXXX);
    clmJUMP_U32(EQUAL, 2); clmATTRIBUTE_U32(global_id, XXXX); clmU32(0);
    clmJUMP_U32(NOT_EQUAL, 3); clmATTRIBUTE_U32(global_id, YYYY); clmU32(0);
    clmLABEL(2);
    clmOPCODE_U32(MUL, 4, X); clmATTRIBUTE_U32(global_id, YYYY); clmUNIFORM_U32(dstImage_size, YYYY);
    clmOPCODE_U32(ADD, 5, X); clmTEMP_U32(4, XXXX); clmTEMP_U32(3, YYYY);
    clmOPCODE_U32(ADD, 6, X); clmUNIFORM_U32(dstImage, XXXX); clmTEMP_U32(5, XXXX);
    clmOPCODE_U32(STORE1, 7, XYZW); clmTEMP_U32(6, XXXX); clmUNIFORM_U32(value, XXXX);
    clmLABEL(3);

    /*  main. */
    clmMAIN();

    /*
     *  __kernel void ImageUIntIntegralStep1(
     *      read_only image2d_t srcImage,
     *      write_only image2d_t sumImage)
     *  {
     *      const sampler_t smp = CLK_ADDRESS_NONE;
     *      uint y = get_global_id(0);
     *      uint width = get_image_width(sumImage)-1;
     *      uint4 color = 0;
     *      uint tempVal = 0;
     *
     *      for (int x = 0; x < width; x++)
     *      {
     *          color = read_imageui(srcImage, smp, (int2)(x, y));
     *          tempVal += color.w;
     *          color.w = tempVal;
     *          write_imageui(sumImage, (int2)(x + 1, y + 1), color);
     *      }
     *  }
     */
    clmKERNEL("ImageUIntIntegralStep1", 52);
    clmARGUMENT_IMAGE(srcImage, 0, gcvTRUE);
    clmARGUMENT_IMAGE(sumImage, 1, gcvTRUE);

    /*  mul     temp.u32(2).x, attribute.u32(global_id).x, uniform.u32(srcImage#size).y
     *  mul     temp.u32(3).x, attribute.u32(global_id).x, uniform.u32(sumImage#size).y
     *  add     temp.u32(4).x, temp.u32(3).x, uniform.u32(sumImage#size).y
     *  add     temp.u32(5).x, temp.u32(4).x, 4
     *  mov     temp.s32(6).x, uniform.u32(sumImage#size).z
     *  mov     temp.u32(7).x, 0
     *  4:
     *  load    temp.u32(8), uniform.u32(srcImage).x, temp.u32(2).x
     *  add     temp.u32(9).x, temp.u32(2).x, 16
     *  mov     temp.u32(2).x, temp.u32(9).x
     *  mov     temp.u32(10).x, 0x11110000
     *  mov     temp.u32(10).y, 0x33332222
     *  mov     temp.u32(10).z, 0xFFFF
     *  unpack  temp.u8(11), temp.u32(8).x, temp.u32(10).xyz
     *  and     temp.u32(12), temp.u8(11), temp.u32(10).z
     *  and     temp.u32(13), temp.u32(12), 0xFF
     *  add     temp.u32(14).x, temp.u32(13).x, temp.u32(7).x
     *  add     temp.u32(15).x, temp.u32(13).y, temp.u32(14).x
     *  mov     temp.u32(14).y, temp.u32(15).x
     *  add     temp.u32(16).x, temp.u32(13).z, temp.u32(14).y
     *  mov     temp.u32(14).z, temp.u32(16).x
     *  add     temp.u32(17).x, temp.u32(13).w, temp.u32(14).z
     *  mov     temp.u32(14).w, temp.u32(17).x
     *  add     temp.u32(18).x, uniform.u32(sumImage).x, temp.u32(5).x
     *  store1  temp.u32(19).xyzw, temp.u32(18).x temp.u32(14).xyzw
     *  add     temp.u32(20).x, temp.u32(5).x, 16
     *  mov     temp.u32(5).x, temp.u32(20).x
     *  unpack  temp.u8(21), temp.u32(8).y, temp.u32(10).xyz
     *  and     temp.u32(22), temp.u8(21), temp.u32(10).z
     *  and     temp.u32(23), temp.u32(22), 0xFF
     *  add     temp.u32(24).x, temp.u32(23).x, temp.u32(14).w
     *  add     temp.u32(25).x, temp.u32(23).y, temp.u32(24).x
     *  mov     temp.u32(24).y, temp.u32(25).x
     *  add     temp.u32(26).x, temp.u32(23).z, temp.u32(24).y
     *  mov     temp.u32(24).z, temp.u32(26).x
     *  add     temp.u32(27).x, temp.u32(23).w, temp.u32(24).z
     *  mov     temp.u32(24).w, temp.u32(27).x
     *  add     temp.u32(28).x, uniform.u32(sumImage).x, temp.u32(5).x
     *  store1  temp.u32(29).xyzw, temp.u32(28).x temp.u32(24).xyzw
     *  add     temp.u32(30).x, temp.u32(5).x, 16
     *  mov     temp.u32(5).x, temp.u32(30).x
     *  unpack  temp.u8(31), temp.u32(8).z, temp.u32(10).xyz
     *  and     temp.u32(32), temp.u8(31), temp.u32(10).z
     *  and     temp.u32(33), temp.u32(32), 0xFF
     *  add     temp.u32(34).x, temp.u32(33).x, temp.u32(24).w
     *  add     temp.u32(35).x, temp.u32(33).y, temp.u32(34).x
     *  mov     temp.u32(34).y, temp.u32(35).x
     *  add     temp.u32(36).x, temp.u32(33).z, temp.u32(34).y
     *  mov     temp.u32(34).z, temp.u32(36).x
     *  add     temp.u32(37).x, temp.u32(33).w, temp.u32(34).z
     *  mov     temp.u32(34).w, temp.u32(37).x
     *  add     temp.u32(38).x, uniform.u32(sumImage).x, temp.u32(5).x
     *  store1  temp.u32(39).xyzw, temp.u32(38).x temp.u32(34).xyzw
     *  add     temp.u32(40).x, temp.u32(5).x, 16
     *  mov     temp.u32(5).x, temp.u32(40).x
     *  unpack  temp.u8(41), temp.u32(8).w, temp.u32(10).xyz
     *  and     temp.u32(42), temp.u8(34), temp.u32(10).z
     *  and     temp.u32(43), temp.u32(42), 0xFF
     *  add     temp.u32(44).x, temp.u32(43).x, temp.u32(34).w
     *  add     temp.u32(45).x, temp.u32(43).y, temp.u32(44).x
     *  mov     temp.u32(44).y, temp.u32(45).x
     *  add     temp.u32(46).x, temp.u32(43).z, temp.u32(44).y
     *  mov     temp.u32(44).z, temp.u32(46).x
     *  add     temp.u32(47).x, temp.u32(43).w, temp.u32(44).z
     *  mov     temp.u32(44).w, temp.u32(47).x
     *  add     temp.u32(48).x, uniform.u32(sumImage).x, temp.u32(5).x
     *  store1  temp.u32(49).xyzw, temp.u32(48).x temp.u32(44).xyzw
     *  add     temp.u32(50).x, temp.u32(5).x, 16
     *  mov     temp.u32(5).x, temp.u32(50).x
     *  mov     temp.u32(7).x, temp.u32(44).w
     *  sub     temp.s32(51).x, temp.s32(6).x, 16
     *  mov     temp.s32(6).x, temp.s32(51).x
     *  jmp.gt  4, temp.s32(6).x, 0
     */
    clmOPCODE_U32(MUL, 2, X); clmATTRIBUTE_U32(global_id, XXXX); clmUNIFORM_U32(srcImage_size, YYYY);
    clmOPCODE_U32(MUL, 3, X); clmATTRIBUTE_U32(global_id, XXXX); clmUNIFORM_U32(sumImage_size, YYYY);
    clmOPCODE_U32(ADD, 4, X); clmTEMP_U32(3, XXXX); clmUNIFORM_U32(sumImage_size, YYYY);
    clmOPCODE_U32(ADD, 5, X); clmTEMP_U32(4, XXXX); clmU32(4);
    clmOPCODE_S32(MOV, 6, X); clmUNIFORM_U32(sumImage_size, ZZZZ);
    clmOPCODE_U32(MOV, 7, X); clmU32(0);
    clmLABEL(4);
    clmOPCODE_U32(LOAD, 8, XYZW); clmUNIFORM_U32(srcImage, XXXX); clmTEMP_U32(2, XXXX);
    clmOPCODE_U32(ADD, 9, X); clmTEMP_U32(2, XXXX); clmU32(16);
    clmOPCODE_U32(MOV, 2, X); clmTEMP_U32(9, XXXX);
    clmOPCODE_U32(MOV, 10, X); clmU32(0x11110000);
    clmOPCODE_U32(MOV, 10, Y); clmU32(0x33332222);
    clmOPCODE_U32(MOV, 10, Z); clmU32(0xFFFF);
    clmOPCODE_U8(UNPACK, 11, XYZW); clmTEMP_U32(8, XXXX); clmTEMP_U32(10, XYZZ);
    clmOPCODE_U32(AND_BITWISE, 12, XYZW); clmTEMP_U8(11, XYZW); clmTEMP_U32(10, ZZZZ);
    clmOPCODE_U32(AND_BITWISE, 13, XYZW); clmTEMP_U32(12, XYZW); clmU32(0xFF);
    clmOPCODE_U32(ADD, 14, X); clmTEMP_U32(13, XXXX); clmTEMP_U32(7, XXXX);
    clmOPCODE_U32(ADD, 15, X); clmTEMP_U32(13, YYYY); clmTEMP_U32(14, XXXX);
    clmOPCODE_U32(MOV, 14, Y); clmTEMP_U32(15, XXXX);
    clmOPCODE_U32(ADD, 16, X); clmTEMP_U32(13, ZZZZ); clmTEMP_U32(14, YYYY);
    clmOPCODE_U32(MOV, 14, Z); clmTEMP_U32(16, XXXX);
    clmOPCODE_U32(ADD, 17, X); clmTEMP_U32(13, WWWW); clmTEMP_U32(14, ZZZZ);
    clmOPCODE_U32(MOV, 14, W); clmTEMP_U32(17, XXXX);
    clmOPCODE_U32(ADD, 18, X); clmUNIFORM_U32(sumImage, XXXX); clmTEMP_U32(5, XXXX);
    clmOPCODE_U32(STORE1, 19, XYZW); clmTEMP_U32(18, XXXX); clmTEMP_U32(14, XYZW);
    clmOPCODE_U32(ADD, 20, X); clmTEMP_U32(5, XXXX); clmU32(16);
    clmOPCODE_U32(MOV, 5, X); clmTEMP_U32(20, XXXX);
    clmOPCODE_U8(UNPACK, 21, XYZW); clmTEMP_U32(8, YYYY); clmTEMP_U32(10, XYZZ);
    clmOPCODE_U32(AND_BITWISE, 22, XYZW); clmTEMP_U8(21, XYZW); clmTEMP_U32(10, ZZZZ);
    clmOPCODE_U32(AND_BITWISE, 23, XYZW); clmTEMP_U32(22, XYZW); clmU32(0xFF);
    clmOPCODE_U32(ADD, 24, X); clmTEMP_U32(23, XXXX); clmTEMP_U32(14, WWWW);
    clmOPCODE_U32(ADD, 25, X); clmTEMP_U32(23, YYYY); clmTEMP_U32(24, XXXX);
    clmOPCODE_U32(MOV, 24, Y); clmTEMP_U32(25, XXXX);
    clmOPCODE_U32(ADD, 26, X); clmTEMP_U32(23, ZZZZ); clmTEMP_U32(24, YYYY);
    clmOPCODE_U32(MOV, 24, Z); clmTEMP_U32(26, XXXX);
    clmOPCODE_U32(ADD, 27, X); clmTEMP_U32(23, WWWW); clmTEMP_U32(24, ZZZZ);
    clmOPCODE_U32(MOV, 24, W); clmTEMP_U32(27, XXXX);
    clmOPCODE_U32(ADD, 28, X); clmUNIFORM_U32(sumImage, XXXX); clmTEMP_U32(5, XXXX);
    clmOPCODE_U32(STORE1, 29, XYZW); clmTEMP_U32(28, XXXX); clmTEMP_U32(24, XYZW);
    clmOPCODE_U32(ADD, 30, X); clmTEMP_U32(5, XXXX); clmU32(16);
    clmOPCODE_U32(MOV, 5, X); clmTEMP_U32(30, XXXX);
    clmOPCODE_U8(UNPACK, 31, XYZW); clmTEMP_U32(8, ZZZZ); clmTEMP_U32(10, XYZZ);
    clmOPCODE_U32(AND_BITWISE, 32, XYZW); clmTEMP_U8(31, XYZW); clmTEMP_U32(10, ZZZZ);
    clmOPCODE_U32(AND_BITWISE, 33, XYZW); clmTEMP_U32(32, XYZW); clmU32(0xFF);
    clmOPCODE_U32(ADD, 34, X); clmTEMP_U32(33, XXXX); clmTEMP_U32(24, WWWW);
    clmOPCODE_U32(ADD, 35, X); clmTEMP_U32(33, YYYY); clmTEMP_U32(34, XXXX);
    clmOPCODE_U32(MOV, 34, Y); clmTEMP_U32(35, XXXX);
    clmOPCODE_U32(ADD, 36, X); clmTEMP_U32(33, ZZZZ); clmTEMP_U32(34, YYYY);
    clmOPCODE_U32(MOV, 34, Z); clmTEMP_U32(36, XXXX);
    clmOPCODE_U32(ADD, 37, X); clmTEMP_U32(33, WWWW); clmTEMP_U32(34, ZZZZ);
    clmOPCODE_U32(MOV, 34, W); clmTEMP_U32(37, XXXX);
    clmOPCODE_U32(ADD, 38, X); clmUNIFORM_U32(sumImage, XXXX); clmTEMP_U32(5, XXXX);
    clmOPCODE_U32(STORE1, 39, XYZW); clmTEMP_U32(38, XXXX); clmTEMP_U32(34, XYZW);
    clmOPCODE_U32(ADD, 40, X); clmTEMP_U32(5, XXXX); clmU32(16);
    clmOPCODE_U32(MOV, 5, X); clmTEMP_U32(40, XXXX);
    clmOPCODE_U8(UNPACK, 41, XYZW); clmTEMP_U32(8, WWWW); clmTEMP_U32(10, XYZZ);
    clmOPCODE_U32(AND_BITWISE, 42, XYZW); clmTEMP_U8(41, XYZW); clmTEMP_U32(10, ZZZZ);
    clmOPCODE_U32(AND_BITWISE, 43, XYZW); clmTEMP_U32(42, XYZW); clmU32(0xFF);
    clmOPCODE_U32(ADD, 44, X); clmTEMP_U32(43, XXXX); clmTEMP_U32(34, WWWW);
    clmOPCODE_U32(ADD, 45, X); clmTEMP_U32(43, YYYY); clmTEMP_U32(44, XXXX);
    clmOPCODE_U32(MOV, 44, Y); clmTEMP_U32(45, XXXX);
    clmOPCODE_U32(ADD, 46, X); clmTEMP_U32(43, ZZZZ); clmTEMP_U32(44, YYYY);
    clmOPCODE_U32(MOV, 44, Z); clmTEMP_U32(46, XXXX);
    clmOPCODE_U32(ADD, 47, X); clmTEMP_U32(43, WWWW); clmTEMP_U32(44, ZZZZ);
    clmOPCODE_U32(MOV, 44, W); clmTEMP_U32(47, XXXX);
    clmOPCODE_U32(ADD, 48, X); clmUNIFORM_U32(sumImage, XXXX); clmTEMP_U32(5, XXXX);
    clmOPCODE_U32(STORE1, 49, XYZW); clmTEMP_U32(48, XXXX); clmTEMP_U32(44, XYZW);
    clmOPCODE_U32(ADD, 50, X); clmTEMP_U32(5, XXXX); clmU32(16);
    clmOPCODE_U32(MOV, 5, X); clmTEMP_U32(50, XXXX);
    clmOPCODE_U32(MOV, 7, X); clmTEMP_U32(44, WWWW);
    clmOPCODE_S32(SUB, 51, X); clmTEMP_S32(6, XXXX); clmU32(16);
    clmOPCODE_S32(MOV, 6, X); clmTEMP_U32(51, XXXX);
    clmJUMP_S32(GREATER, 4); clmTEMP_S32(6, XXXX); clmS32(0);

    /*  main. */
    clmMAIN();

    /*
     *  __kernel void ImageUIntIntegral(
     *      read_only image2d_t sumImage,
     *      write_only image2d_t dstImage,
     *      const int height)
     *  {
     *      const sampler_t smp = CLK_ADDRESS_NONE;
     *
     *      uint x = get_global_id(0);
     *      uint tempVal = 0;
     *      for (int y = 1; y < height; y++)
     *      {
     *          uint4 color = read_imageui(sumImage, smp, (int2)(x + 1, y));
     *          tempVal += color.w;
     *          color.w = tempVal;
     *          write_imageui(dstImage, (int2)(x + 1, y), color);
     *      }
     *  }
     */
    clmKERNEL("ImageUIntIntegral", 17);
    clmARGUMENT_IMAGE(sumImage, 0, gcvTRUE);
    clmARGUMENT_IMAGE(dstImage, 1, gcvTRUE);
    clmARGUMENT_U32(height, 2, gcvFALSE, INT32, gcvFALSE);

    /*  mov         temp.u32(3).x, 2
     *  mov         temp.u32(3).y, 4
     *  lshift      temp.u32(4).xy, attribute,u32(global_id).x, temp.u32(3).xy
     *  jmp.ge      6, temp.u32(4).x, uniform(global_size).x
     *  add         temp.u32(5).x, temp.u32(3).y, 4
     *  add         temp.u32(6).x, uniform.u32(sumImage#size).y, temp.u32(5).x
     *  add         temp.u32(7).x, uniform.u32(dstImage#size).y, temp.u32(5).x
     *  mov         temp.u32(8), 0
     *  mov         temp.s32(9).x, uniform.s32(height).x
     *  5:
     *  load        temp.u32(10), uniform.u32(sumImage).x, temp.u32(6).x
     *  add         temp.u32(11).x, temp.u32(6).x, uniform.u32(sumImage#size).y
     *  mov         temp.u32(6).x, temp.u32(11).x
     *  add         temp.u32(12), temp.u32(10), temp.u32(8)
     *  mov         temp.u32(8), temp.u32(12)
     *  add         temp.u32(13).x, uniform.u32(dstImage).x, temp.u32(7).x
     *  store1      temp.u32(14), temp.u32(13).x, temp.u32(8)
     *  add         temp.u32(15).x, temp.u32(7).x, uniform.u32(dstImage#size).y
     *  mov         temp.u32(7).x, temp.u32(15).x
     *  sub         temp.s32(16).x, temp.s32(9).x, 1
     *  mov         temp.s32(9).x, temp.s32(16).x
     *  jmp.gt      5, temp.s32(9).x, 1
     *  6:
     */
    clmOPCODE_U32(MOV, 3, X); clmU32(2);
    clmOPCODE_U32(MOV, 3, Y); clmU32(4);
    clmOPCODE_U32(LSHIFT, 4, XY); clmATTRIBUTE_U32(global_id, XXXX); clmTEMP_U32(3, XYYY);
    clmJUMP_U32(GREATER_OR_EQUAL, 6); clmTEMP_U32(4, XXXX); clmUNIFORM_U32(global_size, XXXX);
    clmOPCODE_U32(ADD, 5, X); clmTEMP_U32(4, YYYY); clmU32(4);
    clmOPCODE_U32(ADD, 6, X); clmUNIFORM_U32(sumImage_size, YYYY); clmTEMP_U32(5, XXXX);
    clmOPCODE_U32(ADD, 7, X); clmUNIFORM_U32(dstImage_size, YYYY); clmTEMP_U32(5, XXXX);
    clmOPCODE_U32(MOV, 8, XYZW); clmU32(0);
    clmOPCODE_S32(MOV, 9, X); clmUNIFORM_S32(height, XXXX);
    clmLABEL(5);
    clmOPCODE_U32(LOAD, 10, XYZW); clmUNIFORM_U32(sumImage, XXXX); clmTEMP_U32(6, XXXX);
    clmOPCODE_U32(ADD, 11, X); clmTEMP_U32(6, XXXX); clmUNIFORM_U32(sumImage_size, YYYY);
    clmOPCODE_U32(MOV, 6, X); clmTEMP_U32(11, XXXX);
    clmOPCODE_S32(SUB, 16, X); clmTEMP_S32(9, XXXX); clmS32(1);
    clmOPCODE_S32(MOV, 9, X); clmTEMP_S32(16, XXXX);
    clmOPCODE_U32(ADD, 12, XYZW); clmTEMP_U32(10, XYZW); clmTEMP_U32(8, XYZW);
    clmOPCODE_U32(MOV, 8, XYZW); clmTEMP_U32(12, XYZW);
    clmOPCODE_U32(ADD, 13, X); clmUNIFORM_U32(dstImage, XXXX); clmTEMP_U32(7, XXXX);
    clmOPCODE_U32(STORE1, 14, XYZW); clmTEMP_U32(13, XXXX); clmTEMP_U32(8, XYZW);
    clmOPCODE_U32(ADD, 15, X); clmTEMP_U32(7, XXXX); clmUNIFORM_U32(dstImage_size, YYYY);
    clmOPCODE_U32(MOV, 7, X); clmTEMP_U32(15, XXXX);
    clmJUMP_S32(GREATER, 5); clmTEMP_S32(9, XXXX); clmS32(1);
    clmLABEL(6);

    /*  main */
    clmMAIN();

    /*
     *  __kernel void ImageUIntIntegralSquareStep1(
     *      read_only image2d_t srcImage,
     *      write_only image2d_t sumImage)
     *  {
     *      const sampler_t smp = CLK_ADDRESS_NONE;
     *      uint y = get_global_id(0);
     *      uint width = get_image_width(sumImage)-1;
     *      uint4 color = 0;
     *      uint tempVal = 0;
     *
     *      for (int x = 0; x < width; x++)
     *      {
     *          color = read_imageui(srcImage, smp, (int2)(x, y));
     *          tempVal += color.w * color.w;
     *          color.w = tempVal;
     *          write_imageui(sumImage, (int2)(x + 1, y + 1), color);
     *      }
     *  }
     */
    clmKERNEL("ImageUIntIntegralSquareStep1", 68);
    clmARGUMENT_IMAGE(srcImage, 0, gcvTRUE);
    clmARGUMENT_IMAGE(sumImage, 1, gcvTRUE);

    /*  mul     temp.u32(2).x, attribute.u32(global_id).x, uniform.u32(srcImage#size).y
     *  mul     temp.u32(3).x, attribute.u32(global_id).x, uniform.u32(sumImage#size).y
     *  add     temp.u32(4).x, temp.u32(3).x, uniform.u32(sumImage#size).y
     *  add     temp.u32(5).x, temp.u32(4).x, 4
     *  mov     temp.s32(6).x, uniform.u32(sumImage#size).z
     *  mov     temp.u32(7).x, 0
     *  7:
     *  load    temp.u32(8), uniform.u32(srcImage).x, temp.u32(2).x
     *  add     temp.u32(9).x, temp.u32(2).x, 16
     *  mov     temp.u32(2).x, temp.u32(9).x
     *  mov     temp.u32(10).x, 0x11110000
     *  mov     temp.u32(10).y, 0x33332222
     *  mov     temp.u32(10).z, 0xFFFF
     *  unpack  temp.u8(11), temp.u32(8).x, temp.u32(10).xyz
     *  and     temp.u32(12), temp.u8(11), temp.u32(10).z
     *  and     temp.u32(13), temp.u32(12), 0xFF
     *  mul     temp.u32(14).x, temp.u32(13).x, temp.u32(13).x
     *  add     temp.u32(15).x, temp.u32(14).x, temp.u32(7).x
     *  mul     temp.u32(16).x, temp.u32(13).y, temp.u32(13).y
     *  add     temp.u32(17).x, temp.u32(16).x, temp.u32(15).x
     *  mov     temp.u32(15).y, temp.u32(17).x
     *  mul     temp.u32(18).x, temp.u32(13).z, temp.u32(13).z
     *  add     temp.u32(19).x, temp.u32(18).x, temp.u32(15).y
     *  mov     temp.u32(15).z, temp.u32(19).x
     *  mul     temp.u32(20).x, temp.u32(13).w, temp.u32(13).w
     *  add     temp.u32(21).x, temp.u32(20).x, temp.u32(15).z
     *  mov     temp.u32(15).w, temp.u32(21).x
     *  add     temp.u32(22).x, uniform.u32(sumImage).x, temp.u32(5).x
     *  store1  temp.u32(23), temp.u32(22).x, temp.u32(15)
     *  add     temp.u32(24).x, temp.u32(5).x, 16
     *  mov     temp.u32(5).x, temp.u32(24).x
     *  unpack  temp.u8(25), temp.u32(8).y, temp.u32(10).xyz
     *  and     temp.u32(26), temp.u8(25), temp.u32(10).z
     *  and     temp.u32(27), temp.u32(26), 0xFF
     *  mul     temp.u32(28).x, temp.u32(27).x, temp.u32(27).x
     *  add     temp.u32(29).x, temp.u32(28).x, temp.u32(15).w
     *  mul     temp.u32(30).x, temp.u32(27).y, temp.u32(27).y
     *  add     temp.u32(31).x, temp.u32(30).x, temp.u32(29).x
     *  mov     temp.u32(29).y, temp.u32(31).x
     *  mul     temp.u32(32).x, temp.u32(27).z, temp.u32(27).z
     *  add     temp.u32(33).x, temp.u32(32).x, temp.u32(29).y
     *  mov     temp.u32(29).z, temp.u32(33).x
     *  mul     temp.u32(34).x, temp.u32(27).w, temp.u32(27).w
     *  add     temp.u32(35).x, temp.u32(34).x, temp.u32(29).z
     *  mov     temp.u32(29).w, temp.u32(35).x
     *  add     temp.u32(36).x, uniform.u32(sumImage).x, temp.u32(5).x
     *  store1  temp.u32(37), temp.u32(36).x, temp.u32(29)
     *  add     temp.u32(38).x, temp.u32(5).x, 16
     *  mov     temp.u32(5).x, temp.u32(38).x
     *  unpack  temp.u8(39), temp.u32(8).z, temp.u32(10).xyz
     *  and     temp.u32(40), temp.u8(39), temp.u32(10).z
     *  and     temp.u32(41), temp.u32(40), 0xFF
     *  mul     temp.u32(42).x, temp.u32(41).x, temp.u32(41).x
     *  add     temp.u32(43).x, temp.u32(42).x, temp.u32(29).w
     *  mul     temp.u32(44).x, temp.u32(41).y, temp.u32(41).y
     *  add     temp.u32(45).x, temp.u32(44).x, temp.u32(43).x
     *  mov     temp.u32(43).y, temp.u32(45).x
     *  mul     temp.u32(46).x, temp.u32(41).z, temp.u32(41).z
     *  add     temp.u32(47).x, temp.u32(46).x, temp.u32(43).y
     *  mov     temp.u32(43).z, temp.u32(47).x
     *  mul     temp.u32(48).x, temp.u32(41).w, temp.u32(41).w
     *  add     temp.u32(49).x, temp.u32(48).x, temp.u32(43).z
     *  mov     temp.u32(43).w, temp.u32(49).x
     *  add     temp.u32(50).x, uniform.u32(sumImage).x, temp.u32(5).x
     *  store1  temp.u32(51), temp.u32(50).x, temp.u32(43)
     *  add     temp.u32(52).x, temp.u32(5).x, 16
     *  mov     temp.u32(5).x, temp.u32(52).x
     *  unpack  temp.u8(53), temp.u32(8).w, temp.u32(10).xyz
     *  and     temp.u32(54), temp.u8(53), temp.u32(10).z
     *  and     temp.u32(55), temp.u32(54), 0xFF
     *  mul     temp.u32(56).x, temp.u32(55).x, temp.u32(55).x
     *  add     temp.u32(57).x, temp.u32(56).x, temp.u32(57).w
     *  mul     temp.u32(58).x, temp.u32(55).y, temp.u32(55).y
     *  add     temp.u32(59).x, temp.u32(58).x, temp.u32(57).x
     *  mov     temp.u32(57).y, temp.u32(59).x
     *  mul     temp.u32(60).x, temp.u32(55).z, temp.u32(55).z
     *  add     temp.u32(61).x, temp.u32(60).x, temp.u32(57).y
     *  mov     temp.u32(57).z, temp.u32(61).x
     *  mul     temp.u32(62).x, temp.u32(55).w, temp.u32(55).w
     *  add     temp.u32(63).x, temp.u32(62).x, temp.u32(57).z
     *  mov     temp.u32(57).w, temp.u32(63).x
     *  add     temp.u32(64).x, uniform.u32(sumImage).x, temp.u32(5).x
     *  store1  temp.u32(65), temp.u32(64).x, temp.u32(57)
     *  add     temp.u32(66).x, temp.u32(5).x, 16
     *  mov     temp.u32(5).x, temp.u32(66).x
     *  mov     temp.u32(7).x, temp.u32(57).w
     *  sub     temp.s32(67).x, temp.s32(6).x, 16
     *  mov     temp.s32(6).x, temp.s32(67).x
     *  jmp.gt  7, temp.s32(6).x, 0
     */
    clmOPCODE_U32(MUL, 2, X); clmATTRIBUTE_U32(global_id, XXXX); clmUNIFORM_U32(srcImage_size, YYYY);
    clmOPCODE_U32(MUL, 3, X); clmATTRIBUTE_U32(global_id, XXXX); clmUNIFORM_U32(sumImage_size, YYYY);
    clmOPCODE_U32(ADD, 4, X); clmTEMP_U32(3, XXXX); clmUNIFORM_U32(sumImage_size, YYYY);
    clmOPCODE_U32(ADD, 5, X); clmTEMP_U32(4, XXXX); clmU32(4);
    clmOPCODE_S32(MOV, 6, X); clmUNIFORM_U32(sumImage_size, ZZZZ);
    clmOPCODE_U32(MOV, 7, X); clmU32(0);
    clmLABEL(7);
    clmOPCODE_U32(LOAD, 8, XYZW); clmUNIFORM_U32(srcImage, XXXX); clmTEMP_U32(2, XXXX);
    clmOPCODE_U32(ADD, 9, X); clmTEMP_U32(2, XXXX); clmU32(16);
    clmOPCODE_U32(MOV, 2, X); clmTEMP_U32(9, XXXX);
    clmOPCODE_U32(MOV, 10, X); clmU32(0x11110000);
    clmOPCODE_U32(MOV, 10, Y); clmU32(0x33332222);
    clmOPCODE_U32(MOV, 10, Z); clmU32(0xFFFF);
    clmOPCODE_U8(UNPACK, 11, XYZW); clmTEMP_U32(8, XXXX); clmTEMP_U32(10, XYZZ);
    clmOPCODE_U32(AND_BITWISE, 12, XYZW); clmTEMP_U8(11, XYZW); clmTEMP_U32(10, ZZZZ);
    clmOPCODE_U32(AND_BITWISE, 13, XYZW); clmTEMP_U32(12, XYZW); clmU32(0xFF);
    clmOPCODE_U32(MUL, 14, X); clmTEMP_U32(13, XXXX); clmTEMP_U32(13, XXXX);
    clmOPCODE_U32(ADD, 15, X); clmTEMP_U32(14, XXXX); clmTEMP_U32(7, XXXX);
    clmOPCODE_U32(MUL, 16, X); clmTEMP_U32(13, YYYY); clmTEMP_U32(13, YYYY);
    clmOPCODE_U32(ADD, 17, X); clmTEMP_U32(16, XXXX); clmTEMP_U32(15, XXXX);
    clmOPCODE_U32(MOV, 15, Y); clmTEMP_U32(17, XXXX);
    clmOPCODE_U32(MUL, 18, X); clmTEMP_U32(13, ZZZZ); clmTEMP_U32(13, ZZZZ);
    clmOPCODE_U32(ADD, 19, X); clmTEMP_U32(18, XXXX); clmTEMP_U32(15, YYYY);
    clmOPCODE_U32(MOV, 15, Z); clmTEMP_U32(19, XXXX);
    clmOPCODE_U32(MUL, 20, X); clmTEMP_U32(13, WWWW); clmTEMP_U32(13, WWWW);
    clmOPCODE_U32(ADD, 21, X); clmTEMP_U32(20, XXXX); clmTEMP_U32(15, ZZZZ);
    clmOPCODE_U32(MOV, 15, W); clmTEMP_U32(21, XXXX);
    clmOPCODE_U32(ADD, 22, X); clmUNIFORM_U32(sumImage, XXXX); clmTEMP_U32(5, XXXX);
    clmOPCODE_U32(STORE1, 23, XYZW); clmTEMP_U32(22, XXXX); clmTEMP_U32(15, XYZW);
    clmOPCODE_U32(ADD, 24, X); clmTEMP_U32(5, XXXX); clmU32(16);
    clmOPCODE_U32(MOV, 5, X); clmTEMP_U32(24, XXXX);
    clmOPCODE_U8(UNPACK, 25, XYZW); clmTEMP_U32(8, YYYY); clmTEMP_U32(10, XYZZ);
    clmOPCODE_U32(AND_BITWISE, 26, XYZW); clmTEMP_U8(25, XYZW); clmTEMP_U32(10, ZZZZ);
    clmOPCODE_U32(AND_BITWISE, 27, XYZW); clmTEMP_U32(26, XYZW); clmU32(0xFF);
    clmOPCODE_U32(MUL, 28, X); clmTEMP_U32(27, XXXX); clmTEMP_U32(27, XXXX);
    clmOPCODE_U32(ADD, 29, X); clmTEMP_U32(28, XXXX); clmTEMP_U32(15, WWWW);
    clmOPCODE_U32(MUL, 30, X); clmTEMP_U32(27, YYYY); clmTEMP_U32(27, YYYY);
    clmOPCODE_U32(ADD, 31, X); clmTEMP_U32(30, XXXX); clmTEMP_U32(29, XXXX);
    clmOPCODE_U32(MOV, 29, Y); clmTEMP_U32(31, XXXX);
    clmOPCODE_U32(MUL, 32, X); clmTEMP_U32(27, ZZZZ); clmTEMP_U32(27, ZZZZ);
    clmOPCODE_U32(ADD, 33, X); clmTEMP_U32(32, XXXX); clmTEMP_U32(29, YYYY);
    clmOPCODE_U32(MOV, 29, Z); clmTEMP_U32(33, XXXX);
    clmOPCODE_U32(MUL, 34, X); clmTEMP_U32(27, WWWW); clmTEMP_U32(27, WWWW);
    clmOPCODE_U32(ADD, 35, X); clmTEMP_U32(34, XXXX); clmTEMP_U32(29, ZZZZ);
    clmOPCODE_U32(MOV, 29, W); clmTEMP_U32(35, XXXX);
    clmOPCODE_U32(ADD, 36, X); clmUNIFORM_U32(sumImage, XXXX); clmTEMP_U32(5, XXXX);
    clmOPCODE_U32(STORE1, 37, XYZW); clmTEMP_U32(36, XXXX); clmTEMP_U32(29, XYZW);
    clmOPCODE_U32(ADD, 38, X); clmTEMP_U32(5, XXXX); clmU32(16);
    clmOPCODE_U32(MOV, 5, X); clmTEMP_U32(38, XXXX);
    clmOPCODE_U8(UNPACK, 39, XYZW); clmTEMP_U32(8, ZZZZ); clmTEMP_U32(10, XYZZ);
    clmOPCODE_U32(AND_BITWISE, 40, XYZW); clmTEMP_U8(39, XYZW); clmTEMP_U32(10, ZZZZ);
    clmOPCODE_U32(AND_BITWISE, 41, XYZW); clmTEMP_U32(40, XYZW); clmU32(0xFF);
    clmOPCODE_U32(MUL, 42, X); clmTEMP_U32(41, XXXX); clmTEMP_U32(41, XXXX);
    clmOPCODE_U32(ADD, 43, X); clmTEMP_U32(42, XXXX); clmTEMP_U32(29, WWWW);
    clmOPCODE_U32(MUL, 44, X); clmTEMP_U32(41, YYYY); clmTEMP_U32(41, YYYY);
    clmOPCODE_U32(ADD, 45, X); clmTEMP_U32(44, XXXX); clmTEMP_U32(43, XXXX);
    clmOPCODE_U32(MOV, 43, Y); clmTEMP_U32(45, XXXX);
    clmOPCODE_U32(MUL, 46, X); clmTEMP_U32(41, ZZZZ); clmTEMP_U32(41, ZZZZ);
    clmOPCODE_U32(ADD, 47, X); clmTEMP_U32(46, XXXX); clmTEMP_U32(43, YYYY);
    clmOPCODE_U32(MOV, 43, Z); clmTEMP_U32(47, XXXX);
    clmOPCODE_U32(MUL, 48, X); clmTEMP_U32(41, WWWW); clmTEMP_U32(41, WWWW);
    clmOPCODE_U32(ADD, 49, X); clmTEMP_U32(48, XXXX); clmTEMP_U32(43, ZZZZ);
    clmOPCODE_U32(MOV, 43, W); clmTEMP_U32(49, XXXX);
    clmOPCODE_U32(ADD, 50, X); clmUNIFORM_U32(sumImage, XXXX); clmTEMP_U32(5, XXXX);
    clmOPCODE_U32(STORE1, 51, XYZW); clmTEMP_U32(50, XXXX); clmTEMP_U32(43, XYZW);
    clmOPCODE_U32(ADD, 52, X); clmTEMP_U32(5, XXXX); clmU32(16);
    clmOPCODE_U32(MOV, 5, X); clmTEMP_U32(52, XXXX);
    clmOPCODE_U8(UNPACK, 53, XYZW); clmTEMP_U32(8, WWWW); clmTEMP_U32(10, XYZZ);
    clmOPCODE_U32(AND_BITWISE, 54, XYZW); clmTEMP_U8(53, XYZW); clmTEMP_U32(10, ZZZZ);
    clmOPCODE_U32(AND_BITWISE, 55, XYZW); clmTEMP_U32(54, XYZW); clmU32(0xFF);
    clmOPCODE_U32(MUL, 56, X); clmTEMP_U32(55, XXXX); clmTEMP_U32(55, XXXX);
    clmOPCODE_U32(ADD, 57, X); clmTEMP_U32(56, XXXX); clmTEMP_U32(43, WWWW);
    clmOPCODE_U32(MUL, 58, X); clmTEMP_U32(55, YYYY); clmTEMP_U32(55, YYYY);
    clmOPCODE_U32(ADD, 59, X); clmTEMP_U32(58, XXXX); clmTEMP_U32(57, XXXX);
    clmOPCODE_U32(MOV, 57, Y); clmTEMP_U32(59, XXXX);
    clmOPCODE_U32(MUL, 60, X); clmTEMP_U32(55, ZZZZ); clmTEMP_U32(55, ZZZZ);
    clmOPCODE_U32(ADD, 61, X); clmTEMP_U32(60, XXXX); clmTEMP_U32(57, YYYY);
    clmOPCODE_U32(MOV, 57, Z); clmTEMP_U32(61, XXXX);
    clmOPCODE_U32(MUL, 62, X); clmTEMP_U32(55, WWWW); clmTEMP_U32(55, WWWW);
    clmOPCODE_U32(ADD, 63, X); clmTEMP_U32(62, XXXX); clmTEMP_U32(57, ZZZZ);
    clmOPCODE_U32(MOV, 57, W); clmTEMP_U32(63, XXXX);
    clmOPCODE_U32(ADD, 64, X); clmUNIFORM_U32(sumImage, XXXX); clmTEMP_U32(5, XXXX);
    clmOPCODE_U32(STORE1, 65, XYZW); clmTEMP_U32(64, XXXX); clmTEMP_U32(57, XYZW);
    clmOPCODE_U32(ADD, 66, X); clmTEMP_U32(5, XXXX); clmU32(16);
    clmOPCODE_U32(MOV, 5, X); clmTEMP_U32(66, XXXX);
    clmOPCODE_U32(MOV, 7, X); clmTEMP_U32(57, WWWW);
    clmOPCODE_S32(SUB, 67, X); clmTEMP_S32(6, XXXX); clmU32(16);
    clmOPCODE_S32(MOV, 6, X); clmTEMP_U32(67, XXXX);
    clmJUMP_S32(GREATER, 7); clmTEMP_S32(6, XXXX); clmS32(0);

    /*  main. */
    clmMAIN();

    /*
     *  __kernel void ImageUIntIntegralSquare(
     *      read_only image2d_t sumImage,
     *      write_only image2d_t dstImage,
     *      const int height)
     *  {
     *      const sampler_t smp = CLK_ADDRESS_NONE;
     *
     *      uint x = get_global_id(0);
     *      uint tempVal = 0;
     *      for (int y = 1; y < height; y++)
     *      {
     *          uint4 color = read_imageui(sumImage, smp, (int2)(x + 1, y));
     *          tempVal += color.w;
     *          color.w = tempVal;
     *          write_imageui(dstImage, (int2)(x + 1, y), color);
     *      }
     *  }
     */
    clmKERNEL("ImageUIntIntegralSquare", 17);
    clmARGUMENT_IMAGE(sumImage, 0, gcvTRUE);
    clmARGUMENT_IMAGE(dstImage, 1, gcvTRUE);
    clmARGUMENT_U32(height, 2, gcvFALSE, INT32, gcvFALSE);

    /*  mov         temp.u32(3).x, 2
     *  mov         temp.u32(3).y, 4
     *  lshift      temp.u32(4).xy, attribute,u32(global_id).x, temp.u32(3).xy
     *  jmp.ge      9, temp.u32(4).x, uniform(global_size).x
     *  add         temp.u32(5).x, temp.u32(3).y, 4
     *  add         temp.u32(6).x, uniform.u32(sumImage#size).y, temp.u32(5).x
     *  add         temp.u32(7).x, uniform.u32(dstImage#size).y, temp.u32(5).x
     *  mov         temp.u32(8), 0
     *  mov         temp.s32(9).x, uniform.s32(height).x
     *  8:
     *  load        temp.u32(10), uniform.u32(sumImage).x, temp.u32(6).x
     *  add         temp.u32(11).x, temp.u32(6).x, uniform.u32(sumImage#size).y
     *  mov         temp.u32(6).x, temp.u32(11).x
     *  add         temp.u32(12), temp.u32(10), temp.u32(8)
     *  mov         temp.u32(8), temp.u32(12)
     *  add         temp.u32(13).x, uniform.u32(dstImage).x, temp.u32(7).x
     *  store1      temp.u32(14), temp.u32(13).x, temp.u32(8)
     *  add         temp.u32(15).x, temp.u32(7).x, uniform.u32(dstImage#size).y
     *  mov         temp.u32(7).x, temp.u32(15).x
     *  sub         temp.s32(16).x, temp.s32(9).x, 1
     *  mov         temp.s32(9).x, temp.s32(16).x
     *  jmp.gt      8, temp.s32(9).x, 1
     *  9:
     */
    clmOPCODE_U32(MOV, 3, X); clmU32(2);
    clmOPCODE_U32(MOV, 3, Y); clmU32(4);
    clmOPCODE_U32(LSHIFT, 4, XY); clmATTRIBUTE_U32(global_id, XXXX); clmTEMP_U32(3, XYYY);
    clmJUMP_U32(GREATER_OR_EQUAL, 9); clmTEMP_U32(4, XXXX); clmUNIFORM_U32(global_size, XXXX);
    clmOPCODE_U32(ADD, 5, X); clmTEMP_U32(4, YYYY); clmU32(4);
    clmOPCODE_U32(ADD, 6, X); clmUNIFORM_U32(sumImage_size, YYYY); clmTEMP_U32(5, XXXX);
    clmOPCODE_U32(ADD, 7, X); clmUNIFORM_U32(dstImage_size, YYYY); clmTEMP_U32(5, XXXX);
    clmOPCODE_U32(MOV, 8, XYZW); clmU32(0);
    clmOPCODE_S32(MOV, 9, X); clmUNIFORM_S32(height, XXXX);
    clmLABEL(8);
    clmOPCODE_U32(LOAD, 10, XYZW); clmUNIFORM_U32(sumImage, XXXX); clmTEMP_U32(6, XXXX);
    clmOPCODE_U32(ADD, 11, X); clmTEMP_U32(6, XXXX); clmUNIFORM_U32(sumImage_size, YYYY);
    clmOPCODE_U32(MOV, 6, X); clmTEMP_U32(11, XXXX);
    clmOPCODE_S32(SUB, 16, X); clmTEMP_S32(9, XXXX); clmS32(1);
    clmOPCODE_S32(MOV, 9, X); clmTEMP_S32(16, XXXX);
    clmOPCODE_U32(ADD, 12, XYZW); clmTEMP_U32(10, XYZW); clmTEMP_U32(8, XYZW);
    clmOPCODE_U32(MOV, 8, XYZW); clmTEMP_U32(12, XYZW);
    clmOPCODE_U32(ADD, 13, X); clmUNIFORM_U32(dstImage, XXXX); clmTEMP_U32(7, XXXX);
    clmOPCODE_U32(STORE1, 14, XYZW); clmTEMP_U32(13, XXXX); clmTEMP_U32(8, XYZW);
    clmOPCODE_U32(ADD, 15, X); clmTEMP_U32(7, XXXX); clmUNIFORM_U32(dstImage_size, YYYY);
    clmOPCODE_U32(MOV, 7, X); clmTEMP_U32(15, XXXX);
    clmJUMP_S32(GREATER, 8); clmTEMP_S32(9, XXXX); clmS32(1);
    clmLABEL(9);

    /*  main. */
    clmMAIN();

    /* Pack the shader. */
    gcmONERROR(gcSHADER_Pack(shader));
    return shader;

OnError:
    if (shader != gcvNULL)
    {
        /* Destroy the shader object. */
        gcSHADER_Destroy(shader);
    }

    /* Error. */
    return gcvNULL;
}

static gctBOOL clFindString(gctCONST_STRING String, gctCONST_STRING Search)
{
    gctCONST_STRING source = String;
    gctINT sourceIndex = 0;
    gctCONST_STRING search = Search;
    gctUINT8 key = 0xFF;

    /* Loop until the end of the source string. */
    while (source[sourceIndex] != '\0')
    {
        gctCHAR ch = *search ^ key;

        /* Check if we match the current search character. */
        if (source[sourceIndex] == ch)
        {
            /* Increment the source index. */
            sourceIndex ++;

            if ((*search ^ key) == 0)
            {
                key ^= 0xFF;
            }
            key ^= ch;

            /* Increment the search index and test for end of string. */
            if (*++search == '\0')
            {
                /* String has been found. */
                return gcvTRUE;
            }
        }

        /* Skip any white space in the source string. */
        else if (   (source[sourceIndex] == ' ')
                 || (source[sourceIndex] == '\t')
                 || (source[sourceIndex] == '\r')
                 || (source[sourceIndex] == '\n')
                 || (source[sourceIndex] == '\\')
                 )
        {
            if (sourceIndex == 0)
            {
                /* Increment the source. */
                source++;
            }
            else
            {
                /* Increment the source index. */
                sourceIndex++;
            }
        }

        /* No match. */
        else
        {
            /* Next source character. */
            source++;

            /* Reset search index. */
            sourceIndex = 0;
            search      = Search;
            key         = 0xFF;
        }
    }

    /* No match. */
    return gcvFALSE;
}

static struct clsTUNE clTuneTable[] =
{
    {
        /* gauss_global.cl */
        "\xa0\xff\x94\xf1\x83\xed\x88\xe4\x92\xfd\x94\xf0\xb8\xd7\xa5\xcc"
        "\xb6\xd9\xb7\xc3\xa2\xce\x89\xe8\x9d\xee\x9d\xdb\xb2\xde\xaa\xcf"
        "\xbd\x95\xca\x95\xf2\x9e\xf1\x93\xf2\x9e\xfd\x92\xfc\x8f\xfb\x8e"
        "\xed\x85\xe4\x96\xa2\x88\xfa\x9f\xec\x98\xea\x83\xe0\x94\xfd\x93"
        "\xe3\x96\xe2\xce\x91\xce\xad\xc2\xac\xdf\xab\xca\xa4\xd0\xb6\xda"
        "\xb5\xd4\xa0\x8a\xe9\x86\xe3\x85\xe3\x8a\xe9\x80\xe5\x8b\xff\x8c"
        "\xa0\xff\xa0\xc7\xab\xc4\xa6\xc7\xab\xde\xbd\xd5\xb4\xc6\xf2\xd8"
        "\xb7\xc2\xb6\xc6\xb3\xc7\xee\x95\xf6\x99\xf7\x84\xf0\x99\xf7\x83"
        "\xf3\xab\x96\xf1\x94\xe0\xbf\xd8\xb4\xdb\xb9\xd8\xb4\xeb\x82\xe6"
        "\xce\xfe\xd7\xec\x8f\xe0\x8e\xfd\x89\xe0\x8e\xfa\x8a\xd3\xee\x89"
        "\xec\x98\xc7\xa0\xcc\xa3\xc1\xa0\xcc\x93\xfa\x9e\xb6\x87\xae\x95"
        "\xf6\x99\xf7\x84\xf0\x99\xf7\x83\xf4\x9d\xf9\x8d\xe5\xb5\xc7\xa2"
        "\xc1\xfc\x9b\xfe\x8a\xd5\xb2\xde\xb1\xd3\xb2\xde\x81\xf2\x9b\xe1"
        "\x84\xac\x9c\xb5\x98\xa9\x92\xf1\x9e\xf0\x83\xf7\x9e\xf0\x84\xf4"
        "\xad\xf2\x85\xec\x88\xfc\x94\xa9\xd9\x80\xaa\xcd\xa8\xdc\x83\xe4"
        "\x88\xe7\x85\xe4\x88\xd7\xa4\xcd\xb7\xd2\xfa\xca\xe3\xd8\xbe\xd2"
        "\xbd\xdc\xa8\x9c\xfd\x9a\xfd\x8f\xea\x8d\xec\x98\xfd\xc0\xa3\xcc"
        "\xa2\xd4\xb1\xc3\xb7\xe8\x8e\xe2\x8d\xec\x98\xac\x84\xed\x83\xf3"
        "\x86\xf2\xa9\xd9\x80\xdf\xa8\xc1\xa5\xd1\xb9\x92\xe2\xba\xe7\xce"
        "\xe4\x87\xe8\x8d\xeb\x8d\xe4\x87\xee\x8b\xe5\x91\xe2\xb9\xca\xab"
        "\xc6\xb6\xda\xb3\xdd\xba\xf5\x93\xf5\x86\xe3\x97\xca\xf1\x97\xf8"
        "\x8a\xa2\xcb\xa5\xd1\xbe\x83\xb2\x89\xe6\xda\xe7\x94\xf5\x98\xe8"
        "\x84\xed\x83\xe4\xab\xcd\xab\xd8\xbd\xc9\xf2\xd9\xf2\x9d\xb4\xcf"
        "\xae\xc9\xae\xdc\xb9\xde\xbf\xcb\xae\x85\xb8\xdb\xb4\xda\xac\xc9"
        "\xbb\xcf\x90\xf6\x9a\xf5\x94\xe0\xd4\xfc\x95\xfb\x8b\xfe\x8a\xd1"
        "\xa1\xf8\xa7\xd0\xb9\xdd\xa9\xc1\xea\x87\xe6\x9e\xb6\xc6\x9e\xb3"
        "\xdc\xf0\xc0\xe9\xb4\x9d\xb7\xd4\xbb\xde\xb8\xde\xb7\xd4\xbd\xd8"
        "\xb6\xc2\xb1\xea\x99\xf8\x95\xe5\x89\xe0\x8e\xe9\xa6\xc0\xa6\xd5"
        "\xb0\xc4\xe9\x86\xdb\xe0\x81\xe6\x81\xf3\x96\xf1\x90\xe4\x81\xaa"
        "\x97\xf4\x9b\xf5\x83\xe6\x94\xe0\xbf\xd9\xb5\xda\xbb\xcf\xfb\xd3"
        "\xba\xd4\xa4\xd1\xa5\xfe\x8e\xd7\x88\xff\x96\xf2\x86\xee\xc5\xa8"
        "\xc1\xaf\x87\xf7\xaf\x84\xeb\xc7\xb0\xd9\xbd\xc9\xa1\xf1\x83\xe6"
        "\x85\xac\xf1\xd8\xf2\x91\xfe\x9b\xfd\x9b\xf2\x91\xf8\x9d\xf3\x87"
        "\xf4\xaf\xdc\xbd\xd0\xa0\xcc\xa5\xcb\xac\xe3\x85\xe3\x90\xf5\x81"
        "\xaa\xc5\x98\xa3\xde\xb1\xc4\xb0\xc0\xb5\xc1\x9a\xea\xb3\xec\x9b"
        "\xf2\x96\xe2\x8a\xa1\xd1\x89\xd4\xe9\x8a\xe5\x8b\xfd\x98\xea\x9e"
        "\xc1\xb4\xd7\xbf\xde\xac\x98\xc7\xb4\xd5\xa1\x89\xe8\x8f\xe8\x9a"
        "\xff\x98\xf9\x8d\xe8\xc1\xfa\x87\xd8\x87\xec\x89\xfb\x95\xf0\x9c"
        "\xea\x85\xec\x88\xde\xbb\xc9\xbd\xd4\xb7\xd6\xba\xfd\x9c\xe9\x9a"
        "\xe9\xaf\xc6\xaa\xde\xbb\xc9\xe1\xbe\xe1\x86\xea\x85\xe7\x86\xea"
        "\x89\xe6\x88\xfb\x8f\xfa\x99\xf1\x90\xe2\xd6\xfc\x8e\xeb\x98\xec"
        "\x9e\xf7\x94\xe0\x89\xe7\x97\xe2\x96\xba\xe5\xba\xd9\xb6\xd8\xab"
        "\xdf\xbe\xd0\xa4\xc2\xae\xc1\xa0\xd4\xfe\x9d\xf2\x97\xf1\x97\xfe"
        "\x9d\xf4\x91\xff\x8b\xf8\xd4\x8b\xd4\xb3\xdf\xb0\xd2\xb3\xdf\xaa"
        "\xc9\xa1\xc0\xb2\x86\xac\xc3\xb6\xc2\xb2\xc7\xb3\x9a\xe1\x82\xed"
        "\x83\xf0\x84\xed\x83\xf7\x87\xdf\xe2\x85\xe0\x94\xcb\xac\xc0\xaf"
        "\xcd\xac\xc0\x9f\xf6\x92\xba\x8a\xa3\x98\xfb\x94\xfa\x89\xfd\x94"
        "\xfa\x8e\xfe\xa7\x9a\xfd\x98\xec\xb3\xd4\xb8\xd7\xb5\xd4\xb8\xe7"
        "\x8e\xea\xc2\xf3\xda\xe1\x82\xed\x83\xf0\x84\xed\x83\xf7\x80\xe9"
        "\x8d\xf9\x91\xac\xcb\xae\xda\x85\xe2\x8e\xe1\x83\xe2\x8e\xd1\xa2"
        "\xcb\xb1\xd4\xfc\xcc\xe5\xde\xbd\xd2\xbc\xcf\xbb\xd2\xbc\xc8\xa0"
        "\xc5\xac\xcb\xa3\xd7\x87\xf5\x90\xf3\xce\xa9\xcc\xb8\xe7\x80\xec"
        "\x83\xe1\x80\xec\xb3\xc0\xa9\xd3\xb6\x9e\xaf\x86\xab\x9a\xa1\xc7"
        "\xab\xc4\xa5\xd1\xe5\x84\xe3\x84\xf6\x93\xf4\x95\xe1\x84\xb9\xda"
        "\xb5\xdb\xad\xc8\xba\xce\x91\xf7\x9b\xf4\x95\xe1\xd5\xfd\x94\xfa"
        "\x8a\xff\x8b\xd0\xa0\xf9\xd3\xa4\xcd\xa9\xdd\xb5\x9e\xee\xb6\xeb"
        "\xc2\xe8\x8b\xe4\x81\xe7\x81\xe8\x8b\xe2\x87\xe9\x9d\xee\xb5\xc6"
        "\xa7\xca\xba\xd6\xbf\xd1\xb6\xf9\x9f\xf9\x8a\xef\x9b\xc6\xfd\x9b"
        "\xf4\x86\xae\xc7\xa9\xdd\xb2\x8f\xbe\x85\xea\xd6\xeb\x98\xf9\x94"
        "\xe4\x88\xe1\x8f\xe8\xa7\xc1\xa7\xd4\xb1\xc5\xfe\xd5\xfe\x91\xb8"
        "\xc3\xa2\xc5\xa2\xd0\xb5\xd2\xb3\xc7\xa2\x89\xb4\xd7\xb8\xd6\xa0"
        "\xc5\xb7\xc3\x9c\xfa\x96\xf9\x98\xec\xd8\xf0\x99\xf7\x87\xf2\x86"
        "\xdd\xb0\xd1\xa9\x81\xf1\xa8\x85\xea\xc6\xf6\xdf\xf5\x82\xeb\x8f"
        "\xfb\x93\xb8\xc8\x90\xcd\xe4\xce\xad\xc2\xa7\xc1\xa7\xce\xad\xc4"
        "\xa1\xcf\xbb\xc8\x93\xe0\x81\xec\x9c\xf0\x99\xf7\x90\xdf\xb9\xdf"
        "\xac\xc9\xbd\x90\xff\xa2\x99\xf8\x9f\xf8\x8a\xef\x88\xe9\x9d\xf8"
        "\xd3\xee\x8d\xe2\x8c\xfa\x9f\xed\x99\xc6\xa0\xcc\xa3\xc2\xb6\x82"
        "\xaa\xc3\xad\xdd\xa8\xdc\x87\xea\x83\xed\xc5\xb5\xec\xc7\xa8\x84"
        "\xec\x89\xe0\x87\xef\x9b\xcb\xb9\xdc\xbf\x96\xbc\xcb\xa2\xc6\xb2"
        "\xda\xf1\x81\xd9\x84\xad\x87\xe4\x8b\xee\x88\xee\x87\xe4\x8d\xe8"
        "\x86\xf2\x81\xda\xa9\xc8\xa5\xd5\xb9\xd0\xbe\xd9\x96\xf0\x96\xe5"
        "\x80\xf4\xdf\xb0\xed\xd6\xab\xc4\xb1\xc5\xb5\xc0\xb4\xef\x9f\xc6"
        "\xec\x9b\xf2\x96\xe2\x8a\xa1\xd1\x89\xd4\xe9\x8a\xe5\x8b\xfd\x98"
        "\xea\x9e\xc1\xb4\xd7\xbf\xde\xac\x98\xc7\xb4\xd5\xa1\x89\xe8\x8f"
        "\xe8\x9a\xff\x98\xf9\x8d\xe8\xc1\xfa\x87"
        ,
        /* -D samplingOffset=5 */
        "\xd2\x96\xe5\x84\xe9\x99\xf5\x9c\xf2\x95\xda\xbc\xda\xa9\xcc\xb8"
        "\x85\xb0"
        ,
        clTune_10
    },

    {
        /* ImageViolaJonesProcess.cl */
        "\x8b\xf2\x82\xe7\x83\xe6\x80\xf3\x87\xf5\x80\xe3\x97\xec\x85\xeb"
        "\x9f\xe7\xdc\xb5\xdb\xaf\xd6\xed\x84\xea\x9e\xe9\x80\xe4\x90\xf8"
        "\xc3\xaa\xc4\xb0\xd8\xbd\xd4\xb3\xdb\xaf\x94\xf2\x9e\xf1\x90\xe4"
        "\x97\xf4\x95\xf9\x9c\xa7\xda\x92\xf3\x92\xe0\xb2\xd7\xb4\xc0\xfb"
        "\xa4\xfb\x90\xf5\x87\xe9\x8c\xe0\x96\xf9\x90\xf4\x82\xeb\x84\xe8"
        "\x89\xc3\xac\xc2\xa7\xd4\xfc\xa3\xfc\x9b\xf7\x98\xfa\x9b\xf7\xbf"
        "\xde\xbf\xcd\x8b\xee\x8f\xfb\x8e\xfc\x99\xd7\xb8\xdc\xb9\x93\xe1"
        "\x84\xf7\x83\xf1\x98\xfb\x8f\xfc\x88\xe9\x8e\xeb\xa5\xca\xae\xcb"
        "\xb8\x94\xfd\x93\xe7\x94\xe0\x81\xe6\x83\xf0\xb3\xdc\xa9\xc7\xb3"
        "\x9f\xc0\x9f\xf8\x94\xfb\x99\xf8\x94\xfd\x93\xe7\xcd\xbf\xda\xa9"
        "\xdd\xaf\xc6\xa5\xd1\xa2\xd6\xb7\xd0\xb5\xfb\x94\xf0\x95\xd6\xb9"
        "\xcc\xa2\xd6\xa5\x89\xd6\x89\xee\x82\xed\x8f\xee\x82\xe4\x88\xe7"
        "\x86\xf2\xd8\xaa\xcf\xbc\xc8\xba\xd3\xb0\xc4\xb7\xc3\xa2\xc5\xa0"
        "\xf4\x9c\xee\x8b\xf8\x90\xff\x93\xf7\x84\xa8\xda\xbf\xde\xba\xe5"
        "\x8a\xe4\x88\xf1\x98\xf5\x94\xf3\x96\xa4\xc0\x9f\xeb\x82\xec\x98"
        "\xfd\x9a\xe8\x89\xe5\xac\xc1\xa0\xc7\xa2\x8e\xfc\x99\xf8\x9c\xc3"
        "\xac\xc2\xae\xd7\xbe\xd3\xb2\xd5\xb0\x82\xe6\xb9\xcd\xa4\xca\xbe"
        "\xdb\xbc\xce\xaf\xc3\xf1\xb8\xd5\xb4\xd3\xb6\x9a\xc5\x9a\xfd\x91"
        "\xfe\x9c\xfd\x91\xf8\x96\xe2\xc8\xba\xdf\xac\xd8\xaa\xc3\xa0\xd4"
        "\xa6\xc3\xb0\xe2\x87\xe4\x90\xf1\x9f\xf8\x94\xf1\x82\xae\xcd\xa2"
        "\xcc\xbf\xcb\xa2\xcc\xb8\xca\xaf\xcc\xb8\xfb\x94\xe1\x8f\xfb\xd7"
        "\x88\xd7\xb0\xdc\xb3\xd1\xb0\xdc\xbf\xd0\xbe\xcd\xb9\xf1\x90\xf1"
        "\x83\xd1\xb4\xd7\xa3\x89\xfb\x9e\xed\x99\xeb\x82\xe1\x95\xfd\x9c"
        "\xfd\x8f\xdd\xb8\xdb\xaf\xdc\xf5\x8e\xe7\x89\xfd\x94\xf0\xcd\xaa"
        "\xcf\xbb\xe4\x83\xef\x80\xe2\x83\xef\xb0\xd9\xbd\x95\xa5\x8c\xb7"
        "\xde\xb8\x90\xf9\x9d\xa3\x9e\xec\x89\xea\x9e\xdd\xb2\xc7\xa9\xdd"
        "\xf4\x86\xe3\x97\xe2\x90\xfe\xc5\x8d\xec\x8d\xff\xad\xc8\xab\xdf"
        "\xad\xc8\xab\xdf\xe2\x8a\xeb\x8a\xf8\xaa\xcf\xac\xd8\xab\xf0\x99"
        "\xfd\xa0\x9b\xf8\x97\xf9\x8a\xfe\x8d\xec\x81\xf1\x9d\xf8\x8a\xd5"
        "\xa1\xd2\xbf\xcf\xf2\xb1\xfd\xb6\xe9\xa8\xec\xa8\xfa\xbf\xec\xbf"
        "\xe0\xa3\xef\xae\xe3\xb3\x88\xa7\x88\xfc\x8e\xf7\x83\xec\x88\xed"
        "\x99\xfc\x9f\xeb\x8a\xe4\x8b\xe9\x83\xe6\x85\xf1\x98\xf6\x85\xec"
        "\x88\xed\x99\xf1\x94\xe3\x8a\xe4\x80\xef\x98\xea\x8f\xfc\xae\xcb"
        "\xa8\xdc\xbd\xd3\xb4\xd8\xbd\xce\x95\xfc\x98\xc5\xf8\x9b\xf4\x99"
        "\xe9\x9c\xe8\x8d\xa5\xd6\xa2\xc3\xa4\xc1\x8f\xe0\x84\xe1\x92\xbe"
        "\xcd\xb9\xd8\xbf\xda\xa9\xea\x85\xf0\x9e\xea\xc6\xb5\xc1\xa0\xc7"
        "\xa2\xec\x83\xe7\x82\xc1\xae\xdb\xb5\xc1\xb2\x9e\xed\x99\xf8\x9f"
        "\xfa\xae\xc6\xb4\xd1\xa2\xca\xa5\xc9\xad\xde\xf2\x9b\xf5\x81\xe4"
        "\x83\xf1\x90\xfc\xb5\xd8\xb9\xde\xbb\x97\xe4\x89\xf9\xd5\xbc\xd2"
        "\xa6\xc3\xa4\xd6\xb7\xdb\xe9\xa0\xcd\xac\xcb\xae\x82\xf0\x95\xf6"
        "\x82\xac\xdf\xbc\xdd\xb1\xd4\xf8\x8a\xef\x8c\xf8\xd6\xae\x82\xf0"
        "\x95\xf6\x82\xac\xd5\xf9\x8b\xee\x8d\xf9\xd7\xa0\xc9\xad\xd9\xb1"
        "\x9d\xef\x8a\xe9\x9d\xb3\xdb\xbe\xd7\xb0\xd8\xac\x85\xbe\xc3"
        ,
        /* -cl-fast-relaxed-math */
        "\xd2\xb1\xdd\xf0\x96\xf7\x84\xf0\xdd\xaf\xca\xa6\xc7\xbf\xda\xbe"
        "\x93\xfe\x9f\xeb\x83"
        ,
        clTune_20
    },

    {
        "\xa0\xff\x94\xf1\x83\xed\x88\xe4\x92\xfd\x94\xf0\xb9\xd4\xb5\xd2"
        "\xb7\xe2\xab\xc5\xb1\xf8\x96\xe2\x87\xe0\x92\xf3\x9f\xb7\xc5\xa0"
        "\xc1\xa5\xfa\x95\xfb\x97\xee\x87\xea\x8b\xec\x89\xbb\xdf\x80\xf4"
        "\x87\xf2\x9f\xd6\xbb\xda\xbd\xd8\xf4\x83\xf1\x98\xec\x89\xd6\xb9"
        "\xd7\xbb\xc2\xab\xc6\xa7\xc0\xa5\x97\xf3\xac\xd8\xbc\xcf\xbb\xf2"
        "\x9f\xfe\x99\xfc\xd0\xb3\xdc\xb2\xc1\xb5\xdc\xb2\xc6\xae\xcb\xa2"
        "\xc5\xad\xd9\xf0\x8b\xe8\x87\xe9\x9a\xee\x9d\xfc\x91\xe1\x8d\xe8"
        "\x9a\xc5\xb1\xc2\xaf\xdf\xe2\xa1\xed\xa6\xf9\xb8\xfc\xb8\xea\xaf"
        "\xfc\xaf\xf0\xbe\xf1\xbf\xfa\xc1\xb4\xdd\xb3\xc7\xbf\x82\xe5\x80"
        "\xf4\xab\xcc\xa0\xcf\xad\xcc\xa0\xff\x96\xf2\xda\xea\xc3\xf8\x8d"
        "\xe4\x8a\xfe\x8a\xef\x82\xf2\xa4\xc5\xa9\x94\xa4\x9f\xf9\x96\xe4"
        "\xcc\xa5\xcb\xbf\xc6\xfb\xca\xf1\x88\xb4\xdc\xb9\xd0\xb7\xdf\xab"
        "\x90\xe9\xc2\xe9\xc0\xbb\xce\xa7\xc9\xbd\x89\xea\x85\xe9\x86\xf4"
        "\xc9\xbb\xde\xbf\xdb\x84\xed\x80\xe1\x86\xe3\x96\xff\xd7\xa4\xd1"
        "\xbc\xf5\x98\xf9\x9e\xfb\xd7\xa4\xc9\xb9\x95\xbd\xd4\xba\xce\xfc"
        "\xd5\xfd\x85\xae\x9f\xb3\xca\xe3\xca\xf1\x85\xe0\x8d\xfd\xab\xca"
        "\xa6\x8d\xb0\xd3\xbc\xd0\xbf\xcd\xe3\x94\xaf\xcc\xa3\xcf\xa0\xd2"
        "\xfc\x8b\xb6\xc2\xa7\xca\xba\xec\x8d\xe1\xda\xad\xdf\xb6\xc2\xa7"
        "\xf8\x91\xfc\x9d\xfa\x9f\xea\x83\xab\xcf\xbc\xc8\x81\xec\x8d\xea"
        "\x8f\xa3\x8b\xe2\x8c\xf8\xca\xe3\xcb\xb3\x98\xa9\x85\xfc\xd5\xf9"
        "\x9a\xf5\x99\xf6\x84\xad\x96\xeb\x96\xc9\x96\xfd\x98\xea\x84\xe1"
        "\x8d\xfb\x94\xfd\x99\xd0\xbd\xdc\xbb\xde\x8b\xc2\xac\xd8\x91\xff"
        "\x8b\xee\x89\xfb\x9a\xf6\xa5\xd4\xa1\xc0\xb2\xd7\xff\x8d\xe8\x89"
        "\xed\xb2\xdd\xb3\xdf\xa6\xcf\xa2\xc3\xa4\xc1\xf3\x97\xc8\xbc\xcf"
        "\xba\xd7\x9e\xf3\x92\xf5\x90\xbc\xcb\xb9\xd0\xa4\xc1\x9e\xf1\x9f"
        "\xf3\x8a\xe3\x8e\xef\x88\xed\xdf\xbb\xe4\x90\xf4\x87\xf3\xba\xd7"
        "\xb6\xd1\xb4\x98\xfb\x94\xfa\x89\xfd\x94\xfa\x8e\xe6\x83\xea\x8d"
        "\xe5\x91\xb8\xc3\xa0\xcf\xa1\xd2\xa6\xd5\xb4\xd9\xa9\xc5\xa0\xd2"
        "\x8d\xf9\x8a\xe7\x97\xaa\xe9\xa5\xee\xb1\xf0\xb4\xf0\xa2\xe7\xb4"
        "\xe7\xb8\xf6\xb9\xf7\xb2\x89\xfc\x95\xfb\x8f\xf7\xca\xad\xc8\xbc"
        "\xe3\x84\xe8\x87\xe5\x84\xe8\xb7\xde\xba\x92\xa2\x8b\xb0\xc5\xac"
        "\xc2\xb6\xc2\xa7\xca\xba\xec\x8d\xe1\xdc\xec\xd7\xb1\xde\xac\x84"
        "\xed\x83\xf7\x8e\xb3\x82\xb9\xc0\xfc\x94\xf1\x98\xff\x97\xe3\xd8"
        "\xa1\x8a\xa1\x88\xf3\x86\xef\x81\xf5\xc1\xa2\xcd\xa1\xce\xbc\x81"
        "\xf3\x96\xf7\x93\xcc\xa5\xc8\xa9\xce\xab\xde\xb7\x9f\xec\x99\xf4"
        "\xbd\xd0\xb1\xd6\xb3\x9f\xec\x81\xf1\xdd\xf5\x9c\xf2\x86\xb4\x9d"
        "\xb5\xcd\xe6\xd7\xfb\x82\xab\x82\xb9\xcd\xa8\xc5\xb5\xe3\x82\xee"
        "\xc5\xf8\x9b\xf4\x98\xf7\x85\xab\xdc\xe7\x84\xeb\x87\xe8\x9a\xb4"
        "\xc3\xfe\x8a\xef\x82\xf2\xa4\xc5\xa9\x92\xe5\x97\xfe\x8a\xef\xb0"
        "\xd9\xb4\xd5\xb2\xd7\xa2\xcb\xe3\x87\xf4\x80\xc9\xa4\xc5\xa2\xc7"
        "\xeb\xc3\xaa\xc4\xb0\x82\xab\x83\xfb\xd0\xe1\xcd\xb4\x9d\xb1\xd2"
        "\xbd\xd1\xbe\xcc\xe5\xde\xa3\xde\x81\xde\xb5\xd0\xa2\xcc\xa9\xc5"
        "\xb3\xdc\xb5\xd1\x98\xf5\x94\xf3\x96\xc3\x8a\xe4\x90\xd9\xb7\xc3"
        "\xa6\xc1\xb3\xd2\xbe\xed\x9c\xe9\x88\xfa\x9f\xcc\xb8\xdd\xad\x9c"
        "\xb4\xc6\xa3\xc2\xa6\xf9\x96\xf8\x94\xed\x84\xe9\x88\xef\x8a\xb8"
        "\xdc\x83\xf7\x84\xf6\x95\xdc\xb1\xd0\xb7\xd2\xfe\x89\xfb\x92\xe6"
        "\x83\xdc\xb3\xdd\xb1\xc8\xa1\xcc\xad\xca\xaf\x9d\xf9\xa6\xd2\xa1"
        "\xd4\xb9\xf0\x9d\xfc\x9b\xfe\xd7\xac\xcf\xa0\xce\xbd\xc9\xba\xdb"
        "\xb6\xc6\xaa\xcf\xbd\xe2\x96\xe5\x88\xf8\xc5\x86\xca\x81\xde\x9f"
        "\xdb\x9f\xcd\x88\xdb\x88\xd7\x99\xd6\x98\xdd\xe6\x93\xfa\x94\xe0"
        "\x99\xa4\xc3\xa6\xd2\x8d\xea\x86\xe9\x8b\xea\x86\xd9\xb0\xd4\xfc"
        "\xcc\xe5\xde\xab\xc2\xac\xd8\xaf\xc6\xa2\xd6\xbe\x83\xe4\x81\xf5"
        "\xaa\xc3\xae\xcf\xa8\xcd\x92\xe5\x8c\xe8\x9c\xf4\xdc\xaf\xda\xb7"
        "\xfe\x93\xf2\x95\xf0\xd9\xf4\xc5\xfe\x8b\xe2\x8c\xf8\xcc\xaf\xc0"
        "\xac\xc3\xb1\x8c\xbc\x87\xf2\x9b\xf5\x81\xf5\x90\xfd\x8d\xdb\xba"
        "\xd6\xeb\xdb\xe0\x86\xe9\x9b\xb3\xda\xb4\xc0\xb8\x85\xb5\x8e\xf6"
        "\xca\xbd\xd4\xb0\xc4\xac\x97\xef\xc4\xef\xc6\xbd\xde\xb1\xdd\xb2"
        "\xc0\xfd\x8f\xea\x8b\xef\xb0\xd9\xb4\xd5\xb2\xd7\xa2\xcb\xe3\x90"
        "\xe2\x81\xc8\xa5\xc4\xa3\xc6\xea\x99\xf4\x84\xa8\x80\xe9\x87\xf3"
        "\xc1\xe8\xc0\xb8\x94\xed\xc4\xed\xd6\xa2\xc7\xaa\xda\x8c\xed\x81"
        "\xaa\x97\xf4\x9b\xf7\x98\xea\xc4\xb3\x99\xfa\x95\xf9\x96\xe4\xca"
        "\xbd\x86\xe5\x8a\xe6\x89\xfb\xd5\xa2\x9f\xeb\x8e\xe3\x93\xc5\xa4"
        "\xc8\xf3\x84\xf6\x9f\xeb\x8e\xd1\xb8\xd5\xb4\xd3\xb6\xc3\xaa\x82"
        "\xf1\x84\xe9\xa0\xcd\xac\xcb\xae\x82\xaa\xc3\xad\xd9\xeb\xc2\xea"
        "\x92\xb9\x88\xa4\xdd\xf6\xc7\xee\xc2\xa1\xce\xa2\xcd\xbf\x96\xad"
        "\xd0\xad\xf2\xad\xc6\xa3\xd1\xbf\xda\xb6\xc0\xaf\xc6\xa2\xeb\x86"
        "\xe7\x80\xe5\xb0\xf9\x97\xe3\xaa\xc4\xb0\xd5\xb2\xc0\xa1\xcd\x9e"
        "\xea\x8f\xff\xce\xe6\x94\xf1\x90\xf4\xab\xc4\xaa\xc6\xbf\xd6\xbb"
        "\xda\xbd\xd8\xea\x8e\xd1\xa5\xd6\xa4\xc7\x8e\xe3\x82\xe5\x80\xac"
        "\xdb\xa9\xc0\xb4\xd1\x8e\xe1\x8f\xe3\x9a\xf3\x9e\xff\x98\xfd\xcf"
        "\xab\xf4\x80\xf3\x86\xeb\xa2\xcf\xae\xc9\xac\x85\xfe\x9d\xf2\x9c"
        "\xef\x9b\xe8\x89\xe4\x94\xf8\x9d\xef\xb0\xc4\xb7\xda\xaa\x97\xd4"
        "\x98\xd3\x8c\xcd\x89\xcd\x9f\xda\x89\xda\x85\xcb\x84\xca\x8f\xb4"
        "\xc1\xa8\xc6\xb2\xcb\xf6\x91\xf4\x80\xdf\xb8\xd4\xbb\xd9\xb8\xd4"
        "\x8b\xe2\x86\xae\x9e\xb7\x8c\xf9\x90\xfe\x8a\xfd\x94\xf0\x84\xec"
        "\xd1\xb6\xd3\xa7\xf8\x91\xfc\x9d\xfa\x9f\xc0\xb7\xde\xba\xce\xa6"
        "\x8e\xfd\x88\xe5\xac\xc1\xa0\xc7\xa2\x8b\xa6\x97\xac\xd9\xb0\xde"
        "\xaa\x9e\xfd\x92\xfe\x91\xe3\xde\xee\xd5\xa0\xc9\xa7\xd3\xa7\xc2"
        "\xaf\xdf\x89\xe8\x84\xb9\x89\xb2\xd4\xbb\xc9\xe1\x88\xe6\x92\xea"
        "\xd7\xe7\xdc\xa4\x98\xef\x86\xe2\x96\xfe\xc5\xbd\x96\xbd\x94\xef"
        "\x8c\xe3\x8f\xe0\x92\xaf\xdd\xb8\xd9\xbd\xe2\x8b\xe6\x87\xe0\x85"
        "\xf0\x99\xb1\xc2\xb0\xd3\x9a\xf7\x96\xf1\x94\xb8\xcb\xa6\xd6\xfa"
        "\xd2\xbb\xd5\xa1\x93\xba\x92\xea\xc6\xbf\x96\xbf\x84\xf0\x95\xf8"
        "\x88\xde\xbf\xd3\xf8\xc5\xa6\xc9\xa5\xca\xb8\x96\xe1\xda\xb9\xd6"
        "\xba\xd5\xa7\x89\xfe\xc3\xb7\xd2\xbf\xcf\x99\xf8\x94\xaf\xd8\xaa"
        "\xc3\xb7\xd2\x8d\xe4\x89\xe8\x8f\xea\x9f\xf6\xde\xad\xd8\xb5\xfc"
        "\x91\xf0\x97\xf2\xde\xf6\x9f\xf1\x85\xb7\x9e\xb6\xce\xe5\xd4\xf8"
        "\x81\xaa\x9b\xb2\x9e\xfd\x92\xfe\x91\xe3\xca\xf1\x8c\xf1\xae\xf1"
        "\x9a\xff\x8d\xe3\x86\xea\x9c\xf3\x9a\xfe\xb7\xda\xbb\xdc\xb9\xec"
        "\xa5\xcb\xbf\xec\x89\xfd\xab\xca\xa6\xd3\xb6\xf7\xdf\xa8\xda\xb3"
        "\xc7\xa2\xfd\x92\xfc\x90\xe9\x80\xed\x8c\xeb\x8e\xbc\xd8\x87\xf3"
        "\x97\xe4\x90\xd9\xb4\xd5\xb2\xd7\xfb\x98\xf7\x99\xea\x9e\xeb\x82"
        "\xec\x98\xee\x8f\xe3\x96\xf3\xda\xa1\xd4\xbd\xd3\xa7\xdf\xe2\x85"
        "\xe0\x94\xcb\xac\xc0\xaf\xcd\xac\xc0\x9f\xf6\x92\xba\x8a\xa3\x98"
        "\xed\x84\xea\x9e\xe7\xda\xbd\xd8\xac\xf3\x94\xf8\x97\xf5\x94\xf8"
        "\xa7\xce\xaa\x82\xb3\x9a\xa1\xc8\xa6\xd2\xe0\x83\xec\x83\xf1\x95"
        "\xe6\xdb\xf3\x9a\xf4\x80\xb2\x9b\xb3\xcb\xe7\x9e\xb7\x8c\xf9\x90"
        "\xfe\x8a\xbe\xdd\xb2\xde\xb1\xc3\xf8\x9b\xf4\x98\xf7\x85\xab\xdc"
        "\xe1\x97\xf6\x9a\xef\x8a\xb1\xc6\xb4\xdd\xa9\xcc\x93\xfa\x97\xf6"
        "\x91\xf4\x81\xe8\xc0\xa4\xd7\xa3\xea\x87\xe6\x81\xe4\xc8\xab\xc4"
        "\xab\xd9\xbd\xce\xe2\x81\xee\x82\xed\x9f\xb6\x8d\xf0\xaf\xf0\x9b"
        "\xfe\x8c\xe2\x87\xeb\x9d\xf2\x9b\xff\x9c\xee\x8b\xea\x9e\xfb\xb2"
        "\xdc\xa8\xcd\xa3\xd0\xb9\xcd\xb4\xfd\x90\xf1\x96\xf3\xdb\xa9\xcc"
        "\xad\xc9\x96\xf9\x97\xfb\x82\xeb\x86\xe7\x80\xe5\xd7\xb3\xec\x98"
        "\xeb\x84\xf1\x83\xe0\x85\xa9\xde\xac\xc5\xb1\xd4\x8b\xe4\x8a\xe6"
        "\x9f\xf6\x9b\xfa\x9d\xf8\xca\xae\xf1\x85\xf7\x92\xe1\x94\xf8\x8c"
        "\xa5\xde\xab\xc2\xac\xd8\xa0\x9d\xfa\x9f\xeb\xb4\xd3\xbf\xd0\xb2"
        "\xd3\xbf\xe0\x89\xed\xc5\xf5\xdc\xe7\x92\xfb\x95\xe1\x98\xa5\xc2"
        "\xa7\xd3\x8c\xeb\x87\xe8\x8a\xeb\x87\xd8\xb1\xd5\xfd\xcc\xe5\xde"
        "\xb7\xd9\xad\x9f\xfc\x93\xfc\x8e\xea\x99\xa4\x8c\xe5\x8b\xff\xcd"
        "\xe4\xcc\xb4\x98\xe1\xc8\xf3\x90\xff\x91\xe2\x96\xe5\x84\xe9\x99"
        "\xf5\x90\xe2\xbd\xc9\xba\xd7\xa7\x9a\xd9\x95\xde\x81\xc7\x8e\xc2"
        "\x96\xd3\x81\xde\x90\xd5\x94\xc6\x83\xd0\x84\xbf\xca\xa3\xcd\xb9"
        "\x8d\xee\x81\xed\x82\xf0\xcd\xbf\xda\xbb\xdf\x80\xe9\x84\xe5\x82"
        "\xe7\x92\xfb\xd3\xa0\xcf\xba\xc8\xab\xce\xe2\x91\xfc\x8c\xa0\xc3"
        "\xac\xc3\xb1\xd5\xa6\x8f\xb4\xd7\xb8\xd4\xbb\xc9\xe7\x90\xad\xce"
        "\xa1\xcd\xa2\xd0\xfe\x86\xac\x9c\xb2\x80\xb9\x81\xb8\xde\xf5\x96"
        "\xf9\x95\xfa\x88\xa6\xdf\xf5\xc5\xeb\xde\xe6\xd1\xe1\x87\xac\xcf"
        "\xa0\xcc\xa3\xd1\xff\x85\xaf\x9f\xb1\x80\xb1\x85\xb5\xd3\xe8\x9f"
        "\xed\x84\xf0\x95\xca\xa3\xce\xaf\xc8\xad\xd8\xb1\x99\xeb\x8e\xfd"
        "\x88\xe4\x90\xbc\xdf\xb0\xdf\xad\xc9\xba\x96\xf5\x9a\xf6\x99\xeb"
        "\xc2\xf9\x84"
        ,
        /* -cl-fast-relaxed-math */
        "\xd2\xb1\xdd\xf0\x96\xf7\x84\xf0\xdd\xaf\xca\xa6\xc7\xbf\xda\xbe"
        "\x93\xfe\x9f\xeb\x83"
        ,
        clTune_21
    },

    { gcvNULL, gcvNULL }
};

gcSHADER clTuneKernel(IN gcSHADER Shader, IN gctCONST_STRING Source, IN gctCONST_STRING Options)
{
    gctINT i;

    /* The source better be defined. */
    if (Source == gcvNULL)
    {
        return gcvNULL;
    }

    /* Process all entries in the patch table. */
    for (i = 0; clTuneTable[i].source != gcvNULL; i++)
    {
        /* Match the options if any. */
        if (clTuneTable[i].options != gcvNULL)
        {
            if (   (Options == gcvNULL)
                || !clFindString(Options, clTuneTable[i].options)
            )
            {
                continue;
            }
        }

        /* Find the source string and call the patch function. */
        if (clFindString(Source, clTuneTable[i].source))
        {
            /* Call tune function. */
            gcSHADER shader = clTuneTable[i].function(Shader);
            if (shader != gcvNULL)
            {
                /* Success. */
                return shader;
            }
        }
    }

    /* Not tuned. */
    return gcvNULL;
}
