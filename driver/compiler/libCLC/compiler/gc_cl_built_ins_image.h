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


#ifndef __gc_cl_built_ins_image_h_
#define __gc_cl_built_ins_image_h_

#define _SUPPORT_SNORM_FORMATS_     0
#define _USE_TEXLD_FOR_IMAGE_SAMPLER     1
#define _USE_NEW_INSTRUCTION_FOR_IMAGE_RD_SAMPLER     1
#define _USE_NEW_INSTRUCTION_FOR_IMAGE_WR_SAMPLER     1

/*** Fields of struct _clsBUILTIN_FUNCTION
 ** {
    cleEXTENSION    extension;
    gctCONST_STRING symbol;
    gctINT          returnType;
    gctUINT         paramCount;
    gctINT          paramTypes[clmMAX_BUILT_IN_PARAMETER_COUNT];
    gctUINT8        ptrLevels[clmMAX_BUILT_IN_PARAMETER_COUNT];
    gctUINT8        typeConvertible[clmMAX_BUILT_IN_PARAMETER_COUNT];
    gctBOOL         isInline;
    gctBOOL         hasWriteArg;
    gctBOOL         passArgByRef;
    gctBOOL         hasVarArg;
 ** }
***/

static clsBUILTIN_FUNCTION    ImageBuiltinFunctions[] =
{
#if (_OCL_USE_INTRINSIC_FOR_IMAGE || _USE_TEXLD_FOR_IMAGE_SAMPLER || _USE_NEW_INSTRUCTION_FOR_IMAGE_RD_SAMPLER)
    {clvEXTENSION_NONE,    "read_imagef",         T_FLOAT4, 3, {T_IMAGE2D_T, T_SAMPLER_T, T_INT2}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagef",         T_FLOAT4, 2, {T_IMAGE2D_T, T_INT2}, {0, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagef",         T_FLOAT4, 3, {T_IMAGE2D_T, T_SAMPLER_T, T_FLOAT2}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagei",         T_INT4, 3, {T_IMAGE2D_T, T_SAMPLER_T, T_INT2}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagei",         T_INT4, 2, {T_IMAGE2D_T, T_INT2}, {0, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagei",         T_INT4, 3, {T_IMAGE2D_T, T_SAMPLER_T, T_FLOAT2}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imageui",        T_UINT4, 3, {T_IMAGE2D_T, T_SAMPLER_T, T_INT2}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imageui",        T_UINT4, 2, {T_IMAGE2D_T, T_INT2}, {0, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imageui",        T_UINT4, 3, {T_IMAGE2D_T, T_SAMPLER_T, T_FLOAT2}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagef",         T_FLOAT4, 3, {T_IMAGE3D_T, T_SAMPLER_T, T_INT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagef",         T_FLOAT4, 2, {T_IMAGE3D_T, T_INT4}, {0, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagef",         T_FLOAT4, 3, {T_IMAGE3D_T, T_SAMPLER_T, T_FLOAT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagei",         T_INT4, 3, {T_IMAGE3D_T, T_SAMPLER_T, T_INT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagei",         T_INT4, 2, {T_IMAGE3D_T, T_INT4}, {0, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagei",         T_INT4, 3, {T_IMAGE3D_T, T_SAMPLER_T, T_FLOAT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imageui",        T_UINT4, 3, {T_IMAGE3D_T, T_SAMPLER_T, T_INT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imageui",        T_UINT4, 2, {T_IMAGE3D_T, T_INT4}, {0, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imageui",        T_UINT4, 3, {T_IMAGE3D_T, T_SAMPLER_T, T_FLOAT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagef",         T_FLOAT4, 3, {T_IMAGE2D_ARRAY_T, T_SAMPLER_T, T_INT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagef",         T_FLOAT4, 2, {T_IMAGE2D_ARRAY_T, T_INT4}, {0, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagef",         T_FLOAT4, 3, {T_IMAGE2D_ARRAY_T, T_SAMPLER_T, T_FLOAT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagei",         T_INT4, 3, {T_IMAGE2D_ARRAY_T, T_SAMPLER_T, T_INT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagei",         T_INT4, 2, {T_IMAGE2D_ARRAY_T, T_INT4}, {0, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagei",         T_INT4, 3, {T_IMAGE2D_ARRAY_T, T_SAMPLER_T, T_FLOAT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imageui",        T_UINT4, 3, {T_IMAGE2D_ARRAY_T, T_SAMPLER_T, T_INT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imageui",        T_UINT4, 2, {T_IMAGE2D_ARRAY_T, T_INT4}, {0, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imageui",        T_UINT4, 3, {T_IMAGE2D_ARRAY_T, T_SAMPLER_T, T_FLOAT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagef",         T_FLOAT4, 3, {T_IMAGE1D_T, T_SAMPLER_T, T_INT}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagef",         T_FLOAT4, 2, {T_IMAGE1D_T, T_INT}, {0, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagef",         T_FLOAT4, 3, {T_IMAGE1D_T, T_SAMPLER_T, T_FLOAT}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagei",         T_INT4, 3, {T_IMAGE1D_T, T_SAMPLER_T, T_INT}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagei",         T_INT4, 2, {T_IMAGE1D_T, T_INT}, {0, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagei",         T_INT4, 3, {T_IMAGE1D_T, T_SAMPLER_T, T_FLOAT}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imageui",        T_UINT4, 3, {T_IMAGE1D_T, T_SAMPLER_T, T_INT}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imageui",        T_UINT4, 2, {T_IMAGE1D_T, T_INT}, {0, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imageui",        T_UINT4, 3, {T_IMAGE1D_T, T_SAMPLER_T, T_FLOAT}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagef",         T_FLOAT4, 3, {T_IMAGE1D_ARRAY_T, T_SAMPLER_T, T_INT2}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagef",         T_FLOAT4, 2, {T_IMAGE1D_ARRAY_T, T_INT2}, {0, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagef",         T_FLOAT4, 3, {T_IMAGE1D_ARRAY_T, T_SAMPLER_T, T_FLOAT2}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagei",         T_INT4, 3, {T_IMAGE1D_ARRAY_T, T_SAMPLER_T, T_INT2}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagei",         T_INT4, 2, {T_IMAGE1D_ARRAY_T, T_INT2}, {0, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagei",         T_INT4, 3, {T_IMAGE1D_ARRAY_T, T_SAMPLER_T, T_FLOAT2}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imageui",        T_UINT4, 3, {T_IMAGE1D_ARRAY_T, T_SAMPLER_T, T_INT2}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imageui",        T_UINT4, 2, {T_IMAGE1D_ARRAY_T, T_INT2}, {0, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imageui",        T_UINT4, 3, {T_IMAGE1D_ARRAY_T, T_SAMPLER_T, T_FLOAT2}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagef",         T_FLOAT4, 2, {T_IMAGE1D_BUFFER_T, T_INT}, {0, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imagei",         T_INT4, 2, {T_IMAGE1D_BUFFER_T, T_INT}, {0, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,    "read_imageui",        T_UINT4, 2, {T_IMAGE1D_BUFFER_T, T_INT}, {0, 0}, {0, 1}, 1},
    {clvEXTENSION_CL_KHR_FP16, "read_imageh",     T_HALF4, 3, {T_IMAGE2D_T, T_SAMPLER_T, T_INT2}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_CL_KHR_FP16, "read_imageh",     T_HALF4, 2, {T_IMAGE2D_T, T_INT2}, {0, 0}, {0, 1}, 1},
    {clvEXTENSION_CL_KHR_FP16, "read_imageh",     T_HALF4, 3, {T_IMAGE2D_T, T_SAMPLER_T, T_FLOAT2}, {0, 0, 0}, {0, 0, 1}, 1},
#else
    {clvEXTENSION_NONE,    "read_imagef",         T_FLOAT4, 3, {T_IMAGE2D_T, T_SAMPLER_T, T_INT2}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imagef",         T_FLOAT4, 3, {T_IMAGE2D_T, T_SAMPLER_T, T_FLOAT2}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imagei",         T_INT4, 3, {T_IMAGE2D_T, T_SAMPLER_T, T_INT2}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imagei",         T_INT4, 3, {T_IMAGE2D_T, T_SAMPLER_T, T_FLOAT2}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imageui",        T_UINT4, 3, {T_IMAGE2D_T, T_SAMPLER_T, T_INT2}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imageui",        T_UINT4, 3, {T_IMAGE2D_T, T_SAMPLER_T, T_FLOAT2}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imagef",         T_FLOAT4, 3, {T_IMAGE3D_T, T_SAMPLER_T, T_INT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imagef",         T_FLOAT4, 3, {T_IMAGE3D_T, T_SAMPLER_T, T_FLOAT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imagei",         T_INT4, 3, {T_IMAGE3D_T, T_SAMPLER_T, T_INT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imagei",         T_INT4, 3, {T_IMAGE3D_T, T_SAMPLER_T, T_FLOAT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imageui",        T_UINT4, 3, {T_IMAGE3D_T, T_SAMPLER_T, T_INT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imageui",        T_UINT4, 3, {T_IMAGE3D_T, T_SAMPLER_T, T_FLOAT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imagef",         T_FLOAT4, 3, {T_IMAGE2D_ARRAY_T, T_SAMPLER_T, T_INT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imagef",         T_FLOAT4, 3, {T_IMAGE2D_ARRAY_T, T_SAMPLER_T, T_FLOAT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imagei",         T_INT4, 3, {T_IMAGE2D_ARRAY_T, T_SAMPLER_T, T_INT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imagei",         T_INT4, 3, {T_IMAGE2D_ARRAY_T, T_SAMPLER_T, T_FLOAT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imageui",        T_UINT4, 3, {T_IMAGE2D_ARRAY_T, T_SAMPLER_T, T_INT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imageui",        T_UINT4, 3, {T_IMAGE2D_ARRAY_T, T_SAMPLER_T, T_FLOAT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imagef",         T_FLOAT4, 3, {T_IMAGE1D_T, T_SAMPLER_T, T_INT}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imagef",         T_FLOAT4, 3, {T_IMAGE1D_T, T_SAMPLER_T, T_FLOAT}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imagei",         T_INT4, 3, {T_IMAGE1D_T, T_SAMPLER_T, T_INT}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imagei",         T_INT4, 3, {T_IMAGE1D_T, T_SAMPLER_T, T_FLOAT}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imageui",        T_UINT4, 3, {T_IMAGE1D_T, T_SAMPLER_T, T_INT}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imageui",        T_UINT4, 3, {T_IMAGE1D_T, T_SAMPLER_T, T_FLOAT}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imagef",         T_FLOAT4, 3, {T_IMAGE1D_ARRAY_T, T_SAMPLER_T, T_INT2}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imagef",         T_FLOAT4, 3, {T_IMAGE1D_ARRAY_T, T_SAMPLER_T, T_FLOAT2}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imagei",         T_INT4, 3, {T_IMAGE1D_ARRAY_T, T_SAMPLER_T, T_INT2}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imagei",         T_INT4, 3, {T_IMAGE1D_ARRAY_T, T_SAMPLER_T, T_FLOAT2}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imageui",        T_UINT4, 3, {T_IMAGE1D_ARRAY_T, T_SAMPLER_T, T_INT2}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "read_imageui",        T_UINT4, 3, {T_IMAGE1D_ARRAY_T, T_SAMPLER_T, T_FLOAT2}, {0, 0, 0}, {0, 0, 1}, 0},
#endif
#if (_OCL_USE_INTRINSIC_FOR_IMAGE || _USE_NEW_INSTRUCTION_FOR_IMAGE_WR_SAMPLER)
    {clvEXTENSION_NONE,    "write_imagef",        T_VOID, 3, {T_IMAGE2D_T, T_INT2, T_FLOAT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "write_imagei",        T_VOID, 3, {T_IMAGE2D_T, T_INT2, T_INT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "write_imageui",       T_VOID, 3, {T_IMAGE2D_T, T_INT2, T_UINT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "write_imagef",        T_VOID, 3, {T_IMAGE2D_ARRAY_T, T_INT4, T_FLOAT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "write_imagei",        T_VOID, 3, {T_IMAGE2D_ARRAY_T, T_INT4, T_INT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "write_imageui",       T_VOID, 3, {T_IMAGE2D_ARRAY_T, T_INT4, T_UINT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "write_imagef",        T_VOID, 3, {T_IMAGE1D_T, T_INT, T_FLOAT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "write_imagei",        T_VOID, 3, {T_IMAGE1D_T, T_INT, T_INT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "write_imageui",       T_VOID, 3, {T_IMAGE1D_T, T_INT, T_UINT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "write_imagef",        T_VOID, 3, {T_IMAGE1D_BUFFER_T, T_INT, T_FLOAT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "write_imagei",        T_VOID, 3, {T_IMAGE1D_BUFFER_T, T_INT, T_INT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "write_imageui",       T_VOID, 3, {T_IMAGE1D_BUFFER_T, T_INT, T_UINT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "write_imagef",        T_VOID, 3, {T_IMAGE1D_ARRAY_T, T_INT2, T_FLOAT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "write_imagei",        T_VOID, 3, {T_IMAGE1D_ARRAY_T, T_INT2, T_INT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_NONE,    "write_imageui",       T_VOID, 3, {T_IMAGE1D_ARRAY_T, T_INT2, T_UINT4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_CL_KHR_FP16, "write_imageh",    T_VOID, 3, {T_IMAGE2D_T, T_INT2, T_HALF4}, {0, 0, 0}, {0, 0, 1}, 1},
    {clvEXTENSION_CL_KHR_FP16, "write_imageh",    T_VOID, 3, {T_IMAGE2D_ARRAY_T, T_INT4, T_HALF4}, {0, 0, 0}, {0, 0, 1}, 1},
#else
    {clvEXTENSION_NONE,    "write_imagef",        T_VOID, 3, {T_IMAGE2D_T, T_INT2, T_FLOAT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "write_imagei",        T_VOID, 3, {T_IMAGE2D_T, T_INT2, T_INT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "write_imageui",       T_VOID, 3, {T_IMAGE2D_T, T_INT2, T_UINT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "write_imagef",        T_VOID, 3, {T_IMAGE2D_ARRAY_T, T_INT4, T_FLOAT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "write_imagei",        T_VOID, 3, {T_IMAGE2D_ARRAY_T, T_INT4, T_INT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "write_imageui",       T_VOID, 3, {T_IMAGE2D_ARRAY_T, T_INT4, T_UINT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "write_imagef",        T_VOID, 3, {T_IMAGE1D_T, T_INT, T_FLOAT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "write_imagei",        T_VOID, 3, {T_IMAGE1D_T, T_INT, T_INT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "write_imageui",       T_VOID, 3, {T_IMAGE1D_T, T_INT, T_UINT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "write_imagef",        T_VOID, 3, {T_IMAGE1D_BUFFER_T, T_INT, T_FLOAT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "write_imagei",        T_VOID, 3, {T_IMAGE1D_BUFFER_T, T_INT, T_INT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "write_imageui",       T_VOID, 3, {T_IMAGE1D_BUFFER_T, T_INT, T_UINT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "write_imagef",        T_VOID, 3, {T_IMAGE1D_ARRAY_T, T_INT2, T_FLOAT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "write_imagei",        T_VOID, 3, {T_IMAGE1D_ARRAY_T, T_INT2, T_INT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "write_imageui",       T_VOID, 3, {T_IMAGE1D_ARRAY_T, T_INT2, T_UINT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_CL_KHR_FP16, "write_imageh",    T_VOID, 3, {T_IMAGE2D_T, T_INT2, T_HALF4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_CL_KHR_FP16, "write_imageh",    T_VOID, 3, {T_IMAGE2D_ARRAY_T, T_INT4, T_HALF4}, {0, 0, 0}, {0, 0, 1}, 0},
#endif
    {clvEXTENSION_NONE,    "get_image_width",     T_INT, 1, {T_IMAGE2D_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_width",     T_INT, 1, {T_IMAGE3D_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_width",     T_INT, 1, {T_IMAGE2D_ARRAY_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_width",     T_INT, 1, {T_IMAGE1D_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_width",     T_INT, 1, {T_IMAGE1D_ARRAY_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_width",     T_INT, 1, {T_IMAGE1D_BUFFER_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_width",     T_INT, 1, {T_VIV_GENERIC_IMAGE_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_height",    T_INT, 1, {T_IMAGE2D_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_height",    T_INT, 1, {T_IMAGE3D_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_height",    T_INT, 1, {T_IMAGE2D_ARRAY_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_height",    T_INT, 1, {T_VIV_GENERIC_IMAGE_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_depth",     T_INT, 1, {T_IMAGE3D_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_depth",     T_INT, 1, {T_VIV_GENERIC_IMAGE_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_channel_data_type",   T_INT, 1, {T_IMAGE2D_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_channel_data_type",   T_INT, 1, {T_IMAGE3D_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_channel_data_type",   T_INT, 1, {T_IMAGE1D_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_channel_data_type",   T_INT, 1, {T_IMAGE1D_ARRAY_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_channel_data_type",   T_INT, 1, {T_IMAGE1D_BUFFER_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_channel_data_type",   T_INT, 1, {T_IMAGE2D_ARRAY_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_channel_data_type",   T_INT, 1, {T_VIV_GENERIC_IMAGE_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_channel_order",       T_INT, 1, {T_IMAGE2D_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_channel_order",       T_INT, 1, {T_IMAGE3D_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_channel_order",       T_INT, 1, {T_IMAGE1D_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_channel_order",       T_INT, 1, {T_IMAGE1D_ARRAY_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_channel_order",       T_INT, 1, {T_IMAGE1D_BUFFER_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_channel_order",       T_INT, 1, {T_IMAGE2D_ARRAY_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_channel_order",       T_INT, 1, {T_VIV_GENERIC_IMAGE_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_dim",       T_INT2, 1, {T_IMAGE2D_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_dim",       T_INT4, 1, {T_IMAGE3D_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_dim",       T_INT2, 1, {T_IMAGE2D_ARRAY_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_dim",       T_INT4, 1, {T_VIV_GENERIC_IMAGE_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_array_size",       T_INT, 1, {T_IMAGE1D_ARRAY_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_array_size",       T_INT, 1, {T_IMAGE2D_ARRAY_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "get_image_array_size",       T_INT, 1, {T_VIV_GENERIC_IMAGE_T}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "viv_texld",           T_FLOAT4, 2, {T_UINT, T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "viv_read_imagef",     T_FLOAT4, 3, {T_IMAGE2D_T, T_SAMPLER_T, T_INT2}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "viv_read_imagef",     T_FLOAT4, 3, {T_IMAGE2D_T, T_SAMPLER_T, T_FLOAT2}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "viv_read_imagei",     T_INT4, 3, {T_IMAGE2D_T, T_SAMPLER_T, T_INT2}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "viv_read_imagei",     T_INT4, 3, {T_IMAGE2D_T, T_SAMPLER_T, T_FLOAT2}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "viv_read_imageui",    T_UINT4, 3, {T_IMAGE2D_T, T_SAMPLER_T, T_INT2}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "viv_read_imageui",    T_UINT4, 3, {T_IMAGE2D_T, T_SAMPLER_T, T_FLOAT2}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "viv_read_imagef",     T_FLOAT4, 3, {T_IMAGE3D_T, T_SAMPLER_T, T_INT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "viv_read_imagef",     T_FLOAT4, 3, {T_IMAGE3D_T, T_SAMPLER_T, T_FLOAT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "viv_read_imagei",     T_INT4, 3, {T_IMAGE3D_T, T_SAMPLER_T, T_INT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "viv_read_imagei",     T_INT4, 3, {T_IMAGE3D_T, T_SAMPLER_T, T_FLOAT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "viv_read_imageui",    T_UINT4, 3, {T_IMAGE3D_T, T_SAMPLER_T, T_INT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "viv_read_imageui",    T_UINT4, 3, {T_IMAGE3D_T, T_SAMPLER_T, T_FLOAT4}, {0, 0, 0}, {0, 0, 1}, 0},

    {clvEXTENSION_NONE,    "viv_write_imagef",    T_VOID, 3, {T_IMAGE2D_T, T_INT2, T_FLOAT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "viv_write_imagei",    T_VOID, 3, {T_IMAGE2D_T, T_INT2, T_INT4}, {0, 0, 0}, {0, 0, 1}, 0},
    {clvEXTENSION_NONE,    "viv_write_imageui",   T_VOID, 3, {T_IMAGE2D_T, T_INT2, T_UINT4}, {0, 0, 0}, {0, 0, 1}, 0},

};

#define _cldImageBuiltinFunctionCount (sizeof(ImageBuiltinFunctions) / sizeof(clsBUILTIN_FUNCTION))

static gceSTATUS
_GenOldReadImageSamplerCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    cleOPCODE   opcode;

    if (clmIsElementTypeImage3D(clmGEN_CODE_elementType_GET(OperandsParameters[0].rOperands[0].dataType)))
    {
        opcode = clvOPCODE_IMAGE_READ_3D;
    }
    else
    {
        opcode = clvOPCODE_IMAGE_READ;
    }

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3 || OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (OperandCount == 3)
    {
        status = clGenGenericCode2(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   clvOPCODE_IMAGE_SAMPLER,
                                   IOperand,
                                   &OperandsParameters[0].rOperands[0],
                                   &OperandsParameters[1].rOperands[0]);
        if (gcmIS_ERROR(status)) { return status; }

        status = clGenGenericCode2(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   opcode,
                                   IOperand,
                                   &OperandsParameters[0].rOperands[0],
                                   &OperandsParameters[2].rOperands[0]);
        if (gcmIS_ERROR(status)) { return status; }
    }
    else
    {
        clsROPERAND constantROperand;
        clsROPERAND_InitializeIntOrIVecConstant(&constantROperand,
                   clmGenCodeDataType(T_INT),
                   0);  /* i.e. CLK_ADDRESS_NONE | CLK_FILTER_NEAREST | CLK_NORMALIZED_COORDS_FALSE */
        status = clGenGenericCode2(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   clvOPCODE_IMAGE_SAMPLER,
                                   IOperand,
                                   &OperandsParameters[0].rOperands[0],
                                   &constantROperand);
        if (gcmIS_ERROR(status)) { return status; }
        status = clGenGenericCode2(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   opcode,
                                   IOperand,
                                   &OperandsParameters[0].rOperands[0],
                                   &OperandsParameters[1].rOperands[0]);
        if (gcmIS_ERROR(status)) { return status; }
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenOldWriteImageCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters
    )
{
    gceSTATUS status;
    clsIOPERAND iOperand[1];
    clsLOPERAND lOperand[1];
    cleOPCODE   opcode;
    clsGEN_CODE_DATA_TYPE dataType = OperandsParameters[2].rOperands[0].dataType;

    if (clmIsElementTypeImage3D(clmGEN_CODE_elementType_GET(OperandsParameters[0].rOperands[0].dataType)))
    {
        opcode = clvOPCODE_IMAGE_WRITE_3D;
    }
    else
    {
        opcode = clvOPCODE_IMAGE_WRITE;
    }

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);

    if (clmIsElementTypeFloating(dataType.elementType))
    {
        dataType.elementType = clvTYPE_FLOAT;
    }
    else if (clmIsElementTypeSigned(dataType.elementType))
    {
        dataType.elementType = clvTYPE_INT;
    }
    else if (clmIsElementTypeUnsigned(dataType.elementType))
    {
        dataType.elementType = clvTYPE_UINT;
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    clsIOPERAND_New(Compiler, iOperand, dataType);
    clsLOPERAND_InitializeUsingIOperand(lOperand, iOperand);
    status = clGenAssignCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             lOperand,
                             &OperandsParameters[2].rOperands[0]);
    if (gcmIS_ERROR(status)) return status;

    return clGenGenericCode2(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             opcode,
                             iOperand,
                             &OperandsParameters[0].rOperands[0],
                             &OperandsParameters[1].rOperands[0]);
}

#if _OCL_USE_INTRINSIC_FOR_IMAGE
static gceSTATUS
_GenReadImageSamplerCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    if(cloCOMPILER_IsGcslDriverImage(Compiler)) {
        return _GenOldReadImageSamplerCode(Compiler,
                                           CodeGenerator,
                                           PolynaryExpr,
                                           OperandCount,
                                           OperandsParameters,
                                           IOperand);
    }
    else {
        clvVIR_IK   intrinsicKind;
        clsLOPERAND lOperand[1];
        clsROPERAND rOperands[3];

        /* Verify the arguments. */
        clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
        clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
        gcmASSERT(OperandCount == 3 || OperandCount == 2);
        gcmASSERT(OperandsParameters);
        gcmASSERT(IOperand);

        if (clmIsElementTypeImage3D(clmGEN_CODE_elementType_GET(OperandsParameters[0].rOperands[0].dataType)))
        {
            intrinsicKind = CL_VIR_IK_image_load_3d;
        }
        else
        {
            intrinsicKind = CL_VIR_IK_image_load;
        }

        clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
        rOperands[0] = OperandsParameters[0].rOperands[0];
        if(OperandCount == 3) {
            rOperands[1] = OperandsParameters[2].rOperands[0];
            rOperands[2] = OperandsParameters[1].rOperands[0];
        }
        else rOperands[1] = OperandsParameters[1].rOperands[0];

        return  clGenIntrinsicAsmCode(Compiler,
                                      PolynaryExpr->exprBase.base.lineNo,
                                      PolynaryExpr->exprBase.base.stringNo,
                                      intrinsicKind,
                                      lOperand,
                                      OperandCount,
                                      rOperands);
    }
}

#else
#if _USE_NEW_INSTRUCTION_FOR_IMAGE_RD_SAMPLER
static gceSTATUS
_GenReadImageSamplerCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    return _GenOldReadImageSamplerCode(Compiler,
                                       CodeGenerator,
                                       PolynaryExpr,
                                       OperandCount,
                                       OperandsParameters,
                                       IOperand);
}
#else
#if _USE_TEXLD_FOR_IMAGE_SAMPLER
static gceSTATUS
_GenTextureCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsSAMPLER_TYPES *SamplerType,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    clsGEN_CODE_DATA_TYPE dataType;
    cltELEMENT_TYPE samplerType;
    clsNAME *imageName;
    cloIR_BASE arg;
    clsROPERAND samplerOperand[1];
    clsLOGICAL_REG reg[1];
    gctSIZE_T symbolLength = 0;
    gctUINT offset = 0;
    gctSTRING symbol;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    switch(clmGEN_CODE_elementType_GET(OperandsParameters[0].rOperands[0].dataType)) {
    case clvTYPE_IMAGE2D_T:
        samplerType = clvTYPE_SAMPLER2D;
        break;

    case clvTYPE_IMAGE3D_T:
        samplerType = clvTYPE_SAMPLER3D;
        break;

    default:
        gcmASSERT(0);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    clmGEN_CODE_DATA_TYPE_Initialize(dataType, 0, 0, samplerType);

    if(!SamplerType->imageSampler) {
        status = cloIR_SET_GetMember(Compiler,
                                     PolynaryExpr->operands,
                                     1,
                                     &arg);
        if (gcmIS_ERROR(status)) return status;

        imageName = clParseFindLeafName(Compiler,
                                        (cloIR_EXPR) arg);
        gcmASSERT(imageName);
        if(!imageName) {
            return gcvSTATUS_INVALID_ARGUMENT;
        }

        symbolLength = gcoOS_StrLen(imageName->symbol, gcvNULL) +
                       gcoOS_StrLen(SamplerType->member->symbol, gcvNULL) +
                       10;
        status = cloCOMPILER_Allocate(Compiler,
                                      symbolLength,
                                      (gctPOINTER *) &symbol);
        if (gcmIS_ERROR(status)) return status;

        gcmVERIFY_OK(gcoOS_PrintStrSafe(symbol,
                                        symbolLength,
                                        &offset,
                                        "%s%s%c%s",
                                        "sampler#",
                                        imageName->symbol,
                                        '#',
                                        SamplerType->member->symbol));

        status = clNewUniform(Compiler,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo,
                              symbol,
                              dataType,
                              dataType,
                              clvBUILTIN_NONE,
                              gcvFALSE,
                              1,
                              &SamplerType->imageSampler);

        gcmVERIFY_OK(cloCOMPILER_Free(Compiler, symbol));
        if (gcmIS_ERROR(status)) return status;
    }

    clsLOGICAL_REG_InitializeUniform(Compiler,
                                     reg,
                                     clvQUALIFIER_UNIFORM,
                                     dataType,
                                     SamplerType->imageSampler,
                                     0,
                                     gcvFALSE);

    clsROPERAND_InitializeReg(samplerOperand, reg);
    status = clGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               clvOPCODE_TEXTURE_LOAD,
                               IOperand,
                               samplerOperand,
                               &OperandsParameters[2].rOperands[0]);

    if (gcmIS_ERROR(status)) { return status; }

    return gcvSTATUS_OK;
}
#endif
#endif
#endif

#if _OCL_USE_INTRINSIC_FOR_IMAGE
static gceSTATUS
_GenWriteImageCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters
    )
{
    if(cloCOMPILER_IsGcslDriverImage(Compiler)) {
        return _GenOldWriteImageCode(Compiler,
                                     CodeGenerator,
                                     PolynaryExpr,
                                     OperandCount,
                                     OperandsParameters);
    }
    else {
        clvVIR_IK   intrinsicKind;
        clsIOPERAND iOperand[1];
        clsLOPERAND lOperand[1];
        clsROPERAND rOperands[3];

        /* Verify the arguments. */
        clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
        clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
        gcmASSERT(OperandCount == 3);
        gcmASSERT(OperandsParameters);

        if (clmIsElementTypeImage3D(clmGEN_CODE_elementType_GET(OperandsParameters[0].rOperands[0].dataType)))
        {
            intrinsicKind = CL_VIR_IK_image_store_3d;
        }
        else
        {
            intrinsicKind = CL_VIR_IK_image_store;
        }

        clsIOPERAND_New(Compiler, iOperand, OperandsParameters[2].rOperands[0].dataType);
        clsLOPERAND_InitializeUsingIOperand(lOperand, iOperand);
        rOperands[0] = OperandsParameters[0].rOperands[0];
        rOperands[1] = OperandsParameters[1].rOperands[0];
        rOperands[2] = OperandsParameters[2].rOperands[0];

        return  clGenIntrinsicAsmCode(Compiler,
                                      PolynaryExpr->exprBase.base.lineNo,
                                      PolynaryExpr->exprBase.base.stringNo,
                                      intrinsicKind,
                                      lOperand,
                                      OperandCount,
                                      rOperands);
    }
}

#else
#if _USE_NEW_INSTRUCTION_FOR_IMAGE_WR_SAMPLER
static gceSTATUS
_GenWriteImageCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters
    )
{
    return _GenOldWriteImageCode(Compiler,
                                 CodeGenerator,
                                 PolynaryExpr,
                                 OperandCount,
                                 OperandsParameters);
}
#endif
#endif

static gceSTATUS
_FindImageSampler(
    IN cloCOMPILER Compiler,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN OUT clsSAMPLER_TYPES **SamplerTypes
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    cloIR_EXPR imageOperand = gcvNULL;
    cloIR_EXPR samplerOperand = gcvNULL;
    clsNAME *image;
    clsNAME *sampler;
    clsSAMPLER_TYPES *samplerTypes = gcvNULL;

    clsSAMPLER_TYPES *prev;
    clsSAMPLER_TYPES *next;
    gctBOOL found;

    /* Try to get the image and sampler arguments */
    FOR_EACH_DLINK_NODE(&PolynaryExpr->operands->members, struct _cloIR_EXPR, samplerOperand) {
        if(imageOperand == gcvNULL)
        {
            imageOperand = samplerOperand;
        }
        else
        {
            break;
        }
    }

    gcmASSERT(imageOperand && samplerOperand);
    gcmASSERT(cloIR_OBJECT_GetType(&imageOperand->base) == clvIR_VARIABLE);
    image = ((cloIR_VARIABLE) &imageOperand->base)->name;
    gcmASSERT(image->type == clvPARAMETER_NAME);
    if(image->decl.dataType->accessQualifier == clvQUALIFIER_WRITE_ONLY) {
        cloCOMPILER_Report(Compiler,
                           PolynaryExpr->exprBase.base.lineNo,
                           PolynaryExpr->exprBase.base.stringNo,
                           clvREPORT_ERROR,
                           "image \"%s\" has WRITE_ONLY access",
                           image->symbol);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    switch(cloIR_OBJECT_GetType(&samplerOperand->base)) {
    case clvIR_VARIABLE:
        sampler = ((cloIR_VARIABLE) &samplerOperand->base)->name;
        gcmASSERT(sampler->type == clvPARAMETER_NAME);
        break;

    case clvIR_CONSTANT:
        {
            cloIR_CONSTANT samplerConstant;
            samplerConstant = (cloIR_CONSTANT) &samplerOperand->base;
            sampler = samplerConstant->variable;
            gcmASSERT(sampler);
        }
        break;

    default:
        cloCOMPILER_Report(Compiler,
                           PolynaryExpr->exprBase.base.lineNo,
                           PolynaryExpr->exprBase.base.stringNo,
                           clvREPORT_ERROR,
                           "expression 0x%x is not a image type variable",
                           samplerOperand);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    found = gcvFALSE;
    FOR_EACH_SLINK_NODE(image->u.variableInfo.samplers, clsSAMPLER_TYPES, prev, next)
    {
        if(next->member == sampler)
        {
            found = gcvTRUE;
            samplerTypes = next;
            break;
        }
    }

    if(!found) {
        gctPOINTER pointer;

        status = cloCOMPILER_Allocate(Compiler,
                                      (gctSIZE_T)sizeof(clsSAMPLER_TYPES),
                                      (gctPOINTER *) &pointer);
        if (gcmIS_ERROR(status)) return status;
        samplerTypes = pointer;
        samplerTypes->member = sampler;
        samplerTypes->imageSampler = gcvNULL;
        slmSLINK_LIST_InsertFirst(image->u.variableInfo.samplers, &samplerTypes->node);

    }
    if(SamplerTypes) {
        *SamplerTypes = samplerTypes;
    }
    return status;
}

static gceSTATUS
_GenSoftReadImageFCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    clsROPERAND constROperand0, constROperand1;
    clsROPERAND constROperand2, constROperand3;
    clsROPERAND constROperand4, constROperand12;
    clsROPERAND constROperand24, constROperand36;
    clsROPERAND constROperandMinus1;
    clsROPERAND constROperandAddr;
    clsROPERAND constROperandNorm;
    clsROPERAND constROperandFilter;
    clsROPERAND constROperandHalf, constROperandOne;
    clsROPERAND constROperand255, constROperand65535;
#if _SUPPORT_SNORM_FORMATS_
    clsROPERAND constROperand127, constROperand32767;
    clsROPERAND constROperandSNORM8, constROperandSNORM16;
#endif
    clsROPERAND constROperandUNORM8, constROperandUNORM16;
    clsROPERAND constROperandFLOAT16, constROperandFLOAT;
    clsIOPERAND iOperandAddr[1], iOperandNorm[1], iOperandFilt[1];
    clsROPERAND rOperandAddr[1], rOperandNorm[1], rOperandFilt[1];
    clsIOPERAND iOperand[10], iOperandD[1];
    clsROPERAND rOperand[10], rOperandD[1];
    clsROPERAND rOperandX[1], rOperandY[1];
    clsIOPERAND iOperandWH[1], iOperandWHM1[1];
    clsROPERAND rOperandWH[1], rOperandWHM1[1];
    clsROPERAND rOperandW[1], rOperandH[1];
    clsROPERAND rOperandWM1[1], rOperandHM1[1];
    clsIOPERAND iOperandI0J0[1], iOperandI1J1[1];
    clsROPERAND rOperandI0J0[1], rOperandI1J1[1];
    clsROPERAND rOperandI0[1], rOperandJ0[1];
    clsROPERAND rOperandI1[1], rOperandJ1[1];
    clsLOPERAND lOperandI0J0[1], lOperandI0[1], lOperandJ0[1];
    clsLOPERAND lOperandI1J1[1], lOperandI1[1], lOperandJ1[1];
    clsIOPERAND iOperand0[1], iOperand1[1], iOperand2[1];
    clsROPERAND rOperand0[1], rOperand1[1], rOperand2[1];
    clsROPERAND rOperand0X[1], rOperand1X[1], rOperand2X[1];
    clsROPERAND rOperand0Y[1], rOperand1Y[1], rOperand2Y[1];
    clsIOPERAND iOperandOffsetT[4];
    clsROPERAND rOperandOffsetT[4];
    clsIOPERAND iOperandTemp[4];
    clsROPERAND rOperandTemp[4];
    clsIOPERAND iFOperand[9], iFOperandD[1];
    clsROPERAND rFOperand[9], rFOperandD[1];
    clsIOPERAND iFOperand0[1], iFOperand1[1], iFOperand2[1];
    clsROPERAND rFOperand0[1], rFOperand1[1], rFOperand2[1];
    clsIOPERAND iFOperand3[1], iFOperand4[1], iFOperand5[1];
    clsROPERAND rFOperand3[1], rFOperand4[1], rFOperand5[1];
    clsROPERAND rFOperand0X[1], rFOperand1X[1], rFOperand2X[1];
    clsROPERAND rFOperand3X[1], rFOperand4X[1], rFOperand5X[1];
    clsROPERAND rFOperand0Y[1], rFOperand1Y[1], rFOperand2Y[1];
    clsROPERAND rFOperand3Y[1], rFOperand4Y[1], rFOperand5Y[1];
    clsLOPERAND lFOperand2[1], lFOperand2X[1], lFOperand2Y[1];
    clsIOPERAND iFOperandQ[5];
    clsROPERAND rFOperandQ[5];
    struct _cloIR_LABEL lastLabel[1];
    gctLABEL endLabel = clNewLabel(Compiler);

    clsIOPERAND_New(Compiler, iOperandAddr, clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(rOperandAddr, iOperandAddr);

    clsIOPERAND_New(Compiler, iOperandNorm, clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(rOperandNorm, iOperandNorm);

    clsIOPERAND_New(Compiler, iOperandFilt, clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(rOperandFilt, iOperandFilt);

    clsIOPERAND_New(Compiler, iOperandWH, clmGenCodeDataType(T_INT2));
    clsROPERAND_InitializeUsingIOperand(rOperandWH, iOperandWH);
    clmROPERAND_vectorComponent_GET(rOperandW, rOperandWH, clvCOMPONENT_X);
    clmROPERAND_vectorComponent_GET(rOperandH, rOperandWH, clvCOMPONENT_Y);

    clsIOPERAND_New(Compiler, iOperandWHM1, clmGenCodeDataType(T_INT2));
    clsROPERAND_InitializeUsingIOperand(rOperandWHM1, iOperandWHM1);
    clmROPERAND_vectorComponent_GET(rOperandWM1, rOperandWHM1, clvCOMPONENT_X);
    clmROPERAND_vectorComponent_GET(rOperandHM1, rOperandWHM1, clvCOMPONENT_Y);

    clsIOPERAND_New(Compiler, iOperandI0J0, clmGenCodeDataType(T_INT2));
    clsROPERAND_InitializeUsingIOperand(rOperandI0J0, iOperandI0J0);
    clmROPERAND_vectorComponent_GET(rOperandI0, rOperandI0J0, clvCOMPONENT_X);
    clmROPERAND_vectorComponent_GET(rOperandJ0, rOperandI0J0, clvCOMPONENT_Y);
    clsLOPERAND_InitializeUsingIOperand(lOperandI0J0, iOperandI0J0);
    clmLOPERAND_vectorComponent_GET(lOperandI0, lOperandI0J0, clvCOMPONENT_X);
    clmLOPERAND_vectorComponent_GET(lOperandJ0, lOperandI0J0, clvCOMPONENT_Y);

    clsIOPERAND_New(Compiler, iOperandI1J1, clmGenCodeDataType(T_INT2));
    clsROPERAND_InitializeUsingIOperand(rOperandI1J1, iOperandI1J1);
    clmROPERAND_vectorComponent_GET(rOperandI1, rOperandI1J1, clvCOMPONENT_X);
    clmROPERAND_vectorComponent_GET(rOperandJ1, rOperandI1J1, clvCOMPONENT_Y);
    clsLOPERAND_InitializeUsingIOperand(lOperandI1J1, iOperandI1J1);
    clmLOPERAND_vectorComponent_GET(lOperandI1, lOperandI1J1, clvCOMPONENT_X);
    clmLOPERAND_vectorComponent_GET(lOperandJ1, lOperandI1J1, clvCOMPONENT_Y);

    clsIOPERAND_New(Compiler, iOperand0, clmGenCodeDataType(T_INT2));
    clsROPERAND_InitializeUsingIOperand(rOperand0, iOperand0);
    clmROPERAND_vectorComponent_GET(rOperand0X, rOperand0, clvCOMPONENT_X);
    clmROPERAND_vectorComponent_GET(rOperand0Y, rOperand0, clvCOMPONENT_Y);

    clsIOPERAND_New(Compiler, iOperand1, clmGenCodeDataType(T_INT2));
    clsROPERAND_InitializeUsingIOperand(rOperand1, iOperand1);
    clmROPERAND_vectorComponent_GET(rOperand1X, rOperand1, clvCOMPONENT_X);
    clmROPERAND_vectorComponent_GET(rOperand1Y, rOperand1, clvCOMPONENT_Y);

    clsIOPERAND_New(Compiler, iOperand2, clmGenCodeDataType(T_INT2));
    clsROPERAND_InitializeUsingIOperand(rOperand2, iOperand2);
    clmROPERAND_vectorComponent_GET(rOperand2X, rOperand2, clvCOMPONENT_X);
    clmROPERAND_vectorComponent_GET(rOperand2Y, rOperand2, clvCOMPONENT_Y);

    clsIOPERAND_New(Compiler, &iOperand[0], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[0], &iOperand[0]);

    clsIOPERAND_New(Compiler, &iOperand[1], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[1], &iOperand[1]);

    clsIOPERAND_New(Compiler, &iOperand[2], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[2], &iOperand[2]);

    clsIOPERAND_New(Compiler, &iOperand[3], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[3], &iOperand[3]);

    clsIOPERAND_New(Compiler, &iOperand[4], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[4], &iOperand[4]);

    clsIOPERAND_New(Compiler, &iOperand[5], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[5], &iOperand[5]);

    clsIOPERAND_New(Compiler, &iOperand[6], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[6], &iOperand[6]);

    clsIOPERAND_New(Compiler, &iOperand[7], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[7], &iOperand[7]);

    clsIOPERAND_New(Compiler, &iOperand[8], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[8], &iOperand[8]);

    clsIOPERAND_New(Compiler, &iOperand[9], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[9], &iOperand[9]);

    clsIOPERAND_New(Compiler, &iOperandOffsetT[0], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperandOffsetT[0], &iOperandOffsetT[0]);

    clsIOPERAND_New(Compiler, &iOperandOffsetT[1], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperandOffsetT[1], &iOperandOffsetT[1]);

    clsIOPERAND_New(Compiler, &iOperandOffsetT[2], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperandOffsetT[2], &iOperandOffsetT[2]);

    clsIOPERAND_New(Compiler, &iOperandOffsetT[3], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperandOffsetT[3], &iOperandOffsetT[3]);

    clsIOPERAND_New(Compiler, &iOperandTemp[0], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperandTemp[0], &iOperandTemp[0]);

    clsIOPERAND_New(Compiler, &iOperandTemp[1], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperandTemp[1], &iOperandTemp[1]);

    clsIOPERAND_New(Compiler, &iOperandTemp[2], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperandTemp[2], &iOperandTemp[2]);

    clsIOPERAND_New(Compiler, &iOperandTemp[3], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperandTemp[3], &iOperandTemp[3]);

    clsIOPERAND_New(Compiler, &iFOperand[0], clmGenCodeDataType(T_FLOAT));
    clsROPERAND_InitializeUsingIOperand(&rFOperand[0], &iFOperand[0]);

    clsIOPERAND_New(Compiler, &iFOperand[1], clmGenCodeDataType(T_FLOAT));
    clsROPERAND_InitializeUsingIOperand(&rFOperand[1], &iFOperand[1]);

    clsIOPERAND_New(Compiler, &iFOperand[2], clmGenCodeDataType(T_FLOAT));
    clsROPERAND_InitializeUsingIOperand(&rFOperand[2], &iFOperand[2]);

    clsIOPERAND_New(Compiler, &iFOperand[3], clmGenCodeDataType(T_FLOAT));
    clsROPERAND_InitializeUsingIOperand(&rFOperand[3], &iFOperand[3]);

    clsIOPERAND_New(Compiler, &iFOperand[4], clmGenCodeDataType(T_FLOAT));
    clsROPERAND_InitializeUsingIOperand(&rFOperand[4], &iFOperand[4]);

    clsIOPERAND_New(Compiler, &iFOperand[5], clmGenCodeDataType(T_FLOAT));
    clsROPERAND_InitializeUsingIOperand(&rFOperand[5], &iFOperand[5]);

    clsIOPERAND_New(Compiler, &iFOperand[6], clmGenCodeDataType(T_FLOAT));
    clsROPERAND_InitializeUsingIOperand(&rFOperand[6], &iFOperand[6]);

    clsIOPERAND_New(Compiler, &iFOperand[7], clmGenCodeDataType(T_FLOAT));
    clsROPERAND_InitializeUsingIOperand(&rFOperand[7], &iFOperand[7]);

    clsIOPERAND_New(Compiler, &iFOperand[8], clmGenCodeDataType(T_FLOAT));
    clsROPERAND_InitializeUsingIOperand(&rFOperand[8], &iFOperand[8]);

    clsIOPERAND_New(Compiler, iFOperandD, clmGenCodeDataType(T_FLOAT4));
    clsROPERAND_InitializeUsingIOperand(rFOperandD, iFOperandD);

    clsIOPERAND_New(Compiler, &iFOperandQ[0], clmGenCodeDataType(T_FLOAT4));
    clsROPERAND_InitializeUsingIOperand(&rFOperandQ[0], &iFOperandQ[0]);

    clsIOPERAND_New(Compiler, &iFOperandQ[1], clmGenCodeDataType(T_FLOAT4));
    clsROPERAND_InitializeUsingIOperand(&rFOperandQ[1], &iFOperandQ[1]);

    clsIOPERAND_New(Compiler, &iFOperandQ[2], clmGenCodeDataType(T_FLOAT4));
    clsROPERAND_InitializeUsingIOperand(&rFOperandQ[2], &iFOperandQ[2]);

    clsIOPERAND_New(Compiler, &iFOperandQ[3], clmGenCodeDataType(T_FLOAT4));
    clsROPERAND_InitializeUsingIOperand(&rFOperandQ[3], &iFOperandQ[3]);

    clsIOPERAND_New(Compiler, &iFOperandQ[4], clmGenCodeDataType(T_FLOAT4));
    clsROPERAND_InitializeUsingIOperand(&rFOperandQ[4], &iFOperandQ[4]);

    clsIOPERAND_New(Compiler, iFOperand0, clmGenCodeDataType(T_FLOAT2));
    clsROPERAND_InitializeUsingIOperand(rFOperand0, iFOperand0);
    clmROPERAND_vectorComponent_GET(rFOperand0X, rFOperand0, clvCOMPONENT_X);
    clmROPERAND_vectorComponent_GET(rFOperand0Y, rFOperand0, clvCOMPONENT_Y);

    clsIOPERAND_New(Compiler, iFOperand1, clmGenCodeDataType(T_FLOAT2));
    clsROPERAND_InitializeUsingIOperand(rFOperand1, iFOperand1);
    clmROPERAND_vectorComponent_GET(rFOperand1X, rFOperand1, clvCOMPONENT_X);
    clmROPERAND_vectorComponent_GET(rFOperand1Y, rFOperand1, clvCOMPONENT_Y);

    clsIOPERAND_New(Compiler, iFOperand2, clmGenCodeDataType(T_FLOAT2));
    clsROPERAND_InitializeUsingIOperand(rFOperand2, iFOperand2);
    clmROPERAND_vectorComponent_GET(rFOperand2X, rFOperand2, clvCOMPONENT_X);
    clmROPERAND_vectorComponent_GET(rFOperand2Y, rFOperand2, clvCOMPONENT_Y);
    clsLOPERAND_InitializeUsingIOperand(lFOperand2, iFOperand2);
    clmLOPERAND_vectorComponent_GET(lFOperand2X, lFOperand2, clvCOMPONENT_X);
    clmLOPERAND_vectorComponent_GET(lFOperand2Y, lFOperand2, clvCOMPONENT_Y);

    clsIOPERAND_New(Compiler, iFOperand3, clmGenCodeDataType(T_FLOAT2));
    clsROPERAND_InitializeUsingIOperand(rFOperand3, iFOperand3);
    clmROPERAND_vectorComponent_GET(rFOperand3X, rFOperand3, clvCOMPONENT_X);
    clmROPERAND_vectorComponent_GET(rFOperand3Y, rFOperand3, clvCOMPONENT_Y);

    clsIOPERAND_New(Compiler, iFOperand4, clmGenCodeDataType(T_FLOAT2));
    clsROPERAND_InitializeUsingIOperand(rFOperand4, iFOperand4);
    clmROPERAND_vectorComponent_GET(rFOperand4X, rFOperand4, clvCOMPONENT_X);
    clmROPERAND_vectorComponent_GET(rFOperand4Y, rFOperand4, clvCOMPONENT_Y);

    clsIOPERAND_New(Compiler, iFOperand5, clmGenCodeDataType(T_FLOAT2));
    clsROPERAND_InitializeUsingIOperand(rFOperand5, iFOperand5);
    clmROPERAND_vectorComponent_GET(rFOperand5X, rFOperand5, clvCOMPONENT_X);
    clmROPERAND_vectorComponent_GET(rFOperand5Y, rFOperand5, clvCOMPONENT_Y);

    clsROPERAND_InitializeFloatOrVecOrMatConstant(&constROperandHalf,
                                                  clmGenCodeDataType(T_FLOAT),
                                                  (gctFLOAT) 0.5f);

    clsROPERAND_InitializeFloatOrVecOrMatConstant(&constROperandOne,
                                                  clmGenCodeDataType(T_FLOAT),
                                                  (gctFLOAT) 1.0f);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand0,
                                            clmGenCodeDataType(T_INT),
                                            (gctUINT) 0);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand1,
                                            clmGenCodeDataType(T_INT),
                                            (gctUINT) 1);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand2,
                                            clmGenCodeDataType(T_INT),
                                            (gctUINT) 2);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand3,
                                            clmGenCodeDataType(T_INT),
                                            (gctUINT) 3);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand4,
                                            clmGenCodeDataType(T_INT),
                                            (gctUINT) 4);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandAddr,
                                            clmGenCodeDataType(T_INT),
                                            (gctUINT) 7);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandMinus1,
                                            clmGenCodeDataType(T_INT),
                                            (gctINT) -1);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandFilter,
                                            clmGenCodeDataType(T_INT),
                                            (gctUINT) 0x100);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandNorm,
                                            clmGenCodeDataType(T_INT),
                                            (gctUINT) 0x10000);

    /* Get width and height. */
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 iOperandWH,
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand0));

    /* Calculate width - 1 and height - 1. */
    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_SUB,
                                       iOperandWHM1,
                                       rOperandWH,
                                       &constROperand1));

    /* Get addressing mode. */
    gcmONERROR(clGenBitwiseExprCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_AND_BITWISE,
                                    iOperandAddr,
                                    &OperandsParameters[1].rOperands[0],
                                    &constROperandAddr));

    /* Get coord x. */
    clmROPERAND_vectorComponent_GET(rOperandX,
                                    &OperandsParameters[2].rOperands[0],
                                    clvCOMPONENT_X);

    /* Get coord y. */
    clmROPERAND_vectorComponent_GET(rOperandY,
                                    &OperandsParameters[2].rOperands[0],
                                    clvCOMPONENT_Y);

    /* Check coord data type. */
    if (OperandsParameters[2].rOperands[0].dataType.elementType == clvTYPE_FLOAT)
    {
        /* Get filter mode. */
        gcmONERROR(clGenBitwiseExprCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        clvOPCODE_AND_BITWISE,
                                        iOperandFilt,
                                        &OperandsParameters[1].rOperands[0],
                                        &constROperandFilter));

        /* If CLK_ADDRESS_REPEAT. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandAddr,
                       clvCONDITION_EQUAL,
                       &constROperand3);

        /* If filter mode is CLK_FILTER_LINEAR. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandFilt,
                       clvCONDITION_EQUAL,
                       &constROperandFilter);

        /* Process coord. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_INT_TO_FLOAT,
                                     iFOperand0,
                                     rOperandWH));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FRACT,
                                     iFOperand1,
                                     &OperandsParameters[2].rOperands[0]));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_MUL,
                                           iFOperand2,
                                           rFOperand0,
                                           rFOperand1));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_SUB,
                                           iFOperand1,
                                           rFOperand2,
                                           &constROperandHalf));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FRACT,
                                     iFOperand4,
                                     rFOperand1));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     iFOperand0,
                                     rFOperand1));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOAT_TO_INT,
                                     iOperandI0J0,
                                     rFOperand0));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_ADD,
                                           iOperandI1J1,
                                           rOperandI0J0,
                                           &constROperand1));

        /* Check if i0 < 0. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandI0,
                       clvCONDITION_LESS_THAN,
                       &constROperand0);

        /* Wrap i0 to width - 1. */
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lOperandI0,
                                   rOperandWM1));

        /* For i0 < 0. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Check if i1 >= width. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandI1,
                       clvCONDITION_GREATER_THAN_EQUAL,
                       rOperandW);

        /* Wrap i1 to 0. */
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lOperandI1,
                                   &constROperand0));

        /* For i1 >= width. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Check if j0 < 0. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandJ0,
                       clvCONDITION_LESS_THAN,
                       &constROperand0);

        /* Wrap j0 to height - 1. */
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lOperandJ0,
                                   rOperandHM1));

        /* For j0 < 0. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Check if j1 >= height. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandJ1,
                       clvCONDITION_GREATER_THAN_EQUAL,
                       rOperandH);

        /* Wrap j1 to 0. */
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lOperandJ1,
                                   &constROperand0));

        /* For j1 >= height. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For else part of CLK_FILTER_LINEAR. */
        clmGEN_CODE_ELSE(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        /* Process coord. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_INT_TO_FLOAT,
                                     iFOperand0,
                                     rOperandWH));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FRACT,
                                     iFOperand1,
                                     &OperandsParameters[2].rOperands[0]));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_MUL,
                                           iFOperand2,
                                           rFOperand0,
                                           rFOperand1));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     iFOperand0,
                                     rFOperand2));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOAT_TO_INT,
                                     iOperandI0J0,
                                     rFOperand0));

        /* Check if i >= width. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandI0,
                       clvCONDITION_GREATER_THAN_EQUAL,
                       rOperandW);

        /* Wrap i to 0. */
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lOperandI0,
                                   &constROperand0));

        /* For i >= width. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Check if j >= height. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandJ0,
                       clvCONDITION_GREATER_THAN_EQUAL,
                       rOperandH);

        /* Wrap j to 0. */
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lOperandJ0,
                                   &constROperand0));

        /* For j >= height. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For CLK_FILTER_LINEAR. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For else part of CLK_ADDRESS_REPEAT. */
        clmGEN_CODE_ELSE(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        /* If CLK_ADDRESS_MIRRORED_REPEAT. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandAddr,
                       clvCONDITION_EQUAL,
                       &constROperand4);

        /* If filter mode is CLK_FILTER_LINEAR. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandFilt,
                       clvCONDITION_EQUAL,
                       &constROperandFilter);

        /* Process coord. */
        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_MUL,
                                           iFOperand1,
                                           &constROperandHalf,
                                           &OperandsParameters[2].rOperands[0]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ABS,
                                     iFOperand0,
                                     rFOperand1));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_ADD,
                                           iFOperand1,
                                           &constROperandHalf,
                                           rFOperand0));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     iFOperand2,
                                     rFOperand1));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FRACT,
                                     iFOperand1,
                                     rFOperand0));

        /* Check if x fraction is 0.5. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rFOperand1X,
                       clvCONDITION_EQUAL,
                       &constROperandHalf);

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOAT_TO_INT,
                                     &iOperand[4],
                                     rFOperand2X));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_AND_BITWISE,
                                           &iOperand[5],
                                           &rOperand[4],
                                           &constROperand1));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_INT_TO_FLOAT,
                                     &iFOperand[0],
                                     &rOperand[5]));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_SUB,
                                           &iFOperand[1],
                                           rFOperand2X,
                                           &rFOperand[0]));

        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lFOperand2X,
                                   &rFOperand[1]));

        /* For x fraction is 0.5. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Check if y fraction is 0.5. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rFOperand1Y,
                       clvCONDITION_EQUAL,
                       &constROperandHalf);

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOAT_TO_INT,
                                     &iOperand[4],
                                     rFOperand2Y));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_AND_BITWISE,
                                           &iOperand[5],
                                           &rOperand[4],
                                           &constROperand1));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_INT_TO_FLOAT,
                                     &iFOperand[0],
                                     &rOperand[5]));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_SUB,
                                           &iFOperand[1],
                                           rFOperand2Y,
                                           &rFOperand[0]));

        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lFOperand2Y,
                                   &rFOperand[1]));

        /* For y fraction is 0.5. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_SIGN,
                                     iFOperand1,
                                     &OperandsParameters[2].rOperands[0]));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_MUL,
                                           iFOperand0,
                                           rFOperand2,
                                           rFOperand1));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_ADD,
                                           iFOperand1,
                                           rFOperand0,
                                           rFOperand0));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_SUB,
                                           iFOperand0,
                                           &OperandsParameters[2].rOperands[0],
                                           rFOperand1));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ABS,
                                     iFOperand1,
                                     rFOperand0));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_INT_TO_FLOAT,
                                     iFOperand0,
                                     rOperandWH));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_MUL,
                                           iFOperand2,
                                           rFOperand0,
                                           rFOperand1));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_SUB,
                                           iFOperand1,
                                           rFOperand2,
                                           &constROperandHalf));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FRACT,
                                     iFOperand4,
                                     rFOperand1));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     iFOperand0,
                                     rFOperand1));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOAT_TO_INT,
                                     iOperandI0J0,
                                     rFOperand0));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_ADD,
                                           iOperand1,
                                           rOperandI0J0,
                                           &constROperand1));

        /* Clamp (i0, j0) to (0..width-1, 0..height-1). */
        gcmONERROR(clGenGenericCode2(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_MAX,
                                     iOperand0,
                                     rOperandI0J0,
                                     &constROperand0));

        gcmONERROR(clGenGenericCode2(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_MIN,
                                     iOperandI0J0,
                                     rOperand0,
                                     rOperandWHM1));

        /* Clamp (i1, j1) to (0..width-1, 0..height-1). */
        gcmONERROR(clGenGenericCode2(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_MIN,
                                     iOperandI1J1,
                                     rOperand1,
                                     rOperandWHM1));

        /* For else part of CLK_FILTER_LINEAR. */
        clmGEN_CODE_ELSE(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        /* Process coord. */
        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_MUL,
                                           iFOperand0,
                                           &OperandsParameters[2].rOperands[0],
                                           &constROperandHalf));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_ADD,
                                           iFOperand1,
                                           rFOperand0,
                                           &constROperandHalf));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     iFOperand0,
                                     rFOperand1));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_ADD,
                                           iFOperand1,
                                           rFOperand0,
                                           rFOperand0));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_SUB,
                                           iFOperand0,
                                           &OperandsParameters[2].rOperands[0],
                                           rFOperand1));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ABS,
                                     iFOperand1,
                                     rFOperand0));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_INT_TO_FLOAT,
                                     iFOperand0,
                                     rOperandWH));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_MUL,
                                           iFOperand2,
                                           rFOperand0,
                                           rFOperand1));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     iFOperand0,
                                     rFOperand2));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOAT_TO_INT,
                                     iOperand0,
                                     rFOperand0));

        /* Clamp i0, j0 to (0, width - 1), (0, height - 1). */
        gcmONERROR(clGenGenericCode2(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_MIN,
                                     iOperandI0J0,
                                     rOperand0,
                                     rOperandWHM1));

        /* For CLK_FILTER_LINEAR. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For else part of CLK_ADDRESS_MIRRORED_REPEAT. */
        clmGEN_CODE_ELSE(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        /* Get normalized coords mode. */
        gcmONERROR(clGenBitwiseExprCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        clvOPCODE_AND_BITWISE,
                                        iOperandNorm,
                                        &OperandsParameters[1].rOperands[0],
                                        &constROperandNorm));

        /* If filter mode is CLK_FILTER_LINEAR. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandFilt,
                       clvCONDITION_EQUAL,
                       &constROperandFilter);

        /* If CLK_NORMALIZED_COORDS_TRUE. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandNorm,
                       clvCONDITION_EQUAL,
                       &constROperandNorm);

        /* Multiply coord.x by width. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_INT_TO_FLOAT,
                                     iFOperand0,
                                     rOperandWH));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_MUL,
                                           iFOperand4,
                                           rFOperand0,
                                           &OperandsParameters[2].rOperands[0]));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_SUB,
                                           iFOperand2,
                                           rFOperand4,
                                           &constROperandHalf));

        /* For else part of CLK_NORMALIZED_COORDS_TRUE. */
        clmGEN_CODE_ELSE(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_SUB,
                                           iFOperand2,
                                           &OperandsParameters[2].rOperands[0],
                                           &constROperandHalf));

        /* For CLK_NORMALIZED_COORDS_TRUE. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FRACT,
                                     iFOperand4,
                                     rFOperand2));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     iFOperand0,
                                     rFOperand2));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOAT_TO_INT,
                                     iOperandI0J0,
                                     rFOperand0));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_ADD,
                                           iOperandI1J1,
                                           rOperandI0J0,
                                           &constROperand1));

        /* If CLK_ADDRESS_CLAMP_TO_EDGE. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandAddr,
                       clvCONDITION_EQUAL,
                       &constROperand1);

        /* Clamp (i0, j0) to (0..width-1, 0..height-1). */
        gcmONERROR(clGenGenericCode2(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_MAX,
                                     iOperand0,
                                     rOperandI0J0,
                                     &constROperand0));

        gcmONERROR(clGenGenericCode2(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_MIN,
                                     iOperandI0J0,
                                     rOperand0,
                                     rOperandWHM1));

        /* Clamp (i1, j1) to (0..width-1, 0..height-1). */
        gcmONERROR(clGenGenericCode2(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_MAX,
                                     iOperand1,
                                     rOperandI1J1,
                                     &constROperand0));

        gcmONERROR(clGenGenericCode2(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_MIN,
                                     iOperandI1J1,
                                     rOperand1,
                                     rOperandWHM1));

        /* For else part of CLK_ADDRESS_CLAMP_TO_EDGE. */
        clmGEN_CODE_ELSE(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        /* If CLK_ADDRESS_CLAMP. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandAddr,
                       clvCONDITION_EQUAL,
                       &constROperand2);

        /* Clamp (i0, j0) to (-1..width, -1..height). */
        gcmONERROR(clGenGenericCode2(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_MAX,
                                     iOperand0,
                                     rOperandI0J0,
                                     &constROperandMinus1));

        gcmONERROR(clGenGenericCode2(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_MIN,
                                     iOperandI0J0,
                                     rOperand0,
                                     rOperandWH));

        /* Clamp (i1, j1) to (-1..width, -1..height). */
        gcmONERROR(clGenGenericCode2(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_MAX,
                                     iOperand1,
                                     rOperandI1J1,
                                     &constROperandMinus1));

        gcmONERROR(clGenGenericCode2(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_MIN,
                                     iOperandI1J1,
                                     rOperand1,
                                     rOperandWH));

        /* Check if i0 is greater than or equal to width. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandI0,
                       clvCONDITION_GREATER_THAN_EQUAL,
                       rOperandW);

        /* Set i0 to -1 (border). */
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lOperandI0,
                                   &constROperandMinus1));

        /* For i0 greater than or equal to width. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Check if j0 is greater than or equal to height. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandJ0,
                       clvCONDITION_GREATER_THAN_EQUAL,
                       rOperandH);

        /* Set j0 to -1 (border). */
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lOperandJ0,
                                   &constROperandMinus1));

        /* For j0 greater than or equal to width. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Check if i1 is greater than or equal to width. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandI1,
                       clvCONDITION_GREATER_THAN_EQUAL,
                       rOperandW);

        /* Set i1 to -1 (border). */
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lOperandI1,
                                   &constROperandMinus1));

        /* For i1 greater than or equal to width. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Check if j1 is greater than or equal to height. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandJ1,
                       clvCONDITION_GREATER_THAN_EQUAL,
                       rOperandH);

        /* Set j1 to -1 (border). */
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lOperandJ1,
                                   &constROperandMinus1));

        /* For j1 greater than or equal to width. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For CLK_ADDRESS_CLAMP. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For CLK_ADDRESS_CLAMP_TO_EDGE. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For else part of CLK_FILTER_LINEAR. */
        clmGEN_CODE_ELSE(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        /* If CLK_NORMALIZED_COORDS_TRUE. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandNorm,
                       clvCONDITION_EQUAL,
                       &constROperandNorm);

        /* Multiply coord by width and height. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_INT_TO_FLOAT,
                                     iFOperand0,
                                     rOperandWH));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_MUL,
                                           iFOperand1,
                                           rFOperand0,
                                           &OperandsParameters[2].rOperands[0]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     iFOperand0,
                                     rFOperand1));

        /* For else part of CLK_NORMALIZED_COORDS_TRUE. */
        clmGEN_CODE_ELSE(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     iFOperand0,
                                     &OperandsParameters[2].rOperands[0]));

        /* For CLK_NORMALIZED_COORDS_TRUE. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOAT_TO_INT,
                                     iOperandI0J0,
                                     rFOperand0));

        /* Check if coord.x is greater than or equal to width. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandI0,
                       clvCONDITION_GREATER_THAN_EQUAL,
                       rOperandW);

        /* If CLK_ADDRESS_CLAMP, return border color. */
        /* Return border color (all 0.0). */
        gcmONERROR(clGenCompareJumpCode(Compiler,
                            CodeGenerator,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            endLabel,
                            gcvTRUE,
                            clvCONDITION_EQUAL,
                            rOperandAddr,
                            &constROperand2));

        /* Clamp to edge (width - 1). */
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lOperandI0,
                                   rOperandWM1));

        /* For else part of coord.x greater than or equal to width. */
        clmGEN_CODE_ELSE(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        /* Check if coord.x is less than 0. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandI0,
                       clvCONDITION_LESS_THAN,
                       &constROperand0);

        /* If CLK_ADDRESS_CLAMP, return border color. */

        /* Return border color (all 0.0). */
        gcmONERROR(clGenCompareJumpCode(Compiler,
                            CodeGenerator,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            endLabel,
                            gcvTRUE,
                            clvCONDITION_EQUAL,
                            rOperandAddr,
                            &constROperand2));

        /* Clamp coord.x to 0. */
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lOperandI0,
                                   &constROperand0));

        /* For coord.x less than 0. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For coord.x greater than or equal to width. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Check if coord.y is greater than or equal to height. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandJ0,
                       clvCONDITION_GREATER_THAN_EQUAL,
                       rOperandH);

        /* Return border color (all 0.0). */
        gcmONERROR(clGenCompareJumpCode(Compiler,
                            CodeGenerator,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            endLabel,
                            gcvTRUE,
                            clvCONDITION_EQUAL,
                            rOperandAddr,
                            &constROperand2));

        /* Clamp to edge (height - 1). */
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lOperandJ0,
                                   rOperandHM1));

        /* For else part of coord.y greater than or equal to height. */
        clmGEN_CODE_ELSE(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        /* Check if coord.y is less than 0. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandJ0,
                       clvCONDITION_LESS_THAN,
                       &constROperand0);

        /* Return border color (all 0.0). */
        gcmONERROR(clGenCompareJumpCode(Compiler,
                            CodeGenerator,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            endLabel,
                            gcvTRUE,
                            clvCONDITION_EQUAL,
                            rOperandAddr,
                            &constROperand2));

        /* Clamp coord.y to 0. */
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lOperandJ0,
                                   &constROperand0));

        /* For coord.y less than 0. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For coord.y greater than or equal to height. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For CLK_FILTER_LINEAR. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For CLK_ADDRESS_MIRRORED_REPEAT. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For CLK_ADDRESS_REPEAT. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);
    }
    else
    {
        /* Force filter mode to CLK_FILTER_NEAREST. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     iOperandFilt,
                                     &constROperand0));

        /* Move coord to I0J0. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     iOperandI0J0,
                                     &OperandsParameters[2].rOperands[0]));

        /* Check if coord.x is greater than or equal to width. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandI0,
                       clvCONDITION_GREATER_THAN_EQUAL,
                       rOperandW);

        /* If CLK_ADDRESS_CLAMP, return border color. */

        /* Return border color (all 0.0). */

        gcmONERROR(clGenCompareJumpCode(Compiler,
                            CodeGenerator,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            endLabel,
                            gcvTRUE,
                            clvCONDITION_EQUAL,
                            rOperandAddr,
                            &constROperand2));

        /* Clamp to edge (width - 1). */
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lOperandI0,
                                   rOperandWM1));

        /* For else part of coord.x greater than or equal to width. */
        clmGEN_CODE_ELSE(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        /* Check if coord.x is less than 0. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandI0,
                       clvCONDITION_LESS_THAN,
                       &constROperand0);

        /* If CLK_ADDRESS_CLAMP, return border color. */

        /* Return border color (all 0.0). */
        gcmONERROR(clGenCompareJumpCode(Compiler,
                            CodeGenerator,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            endLabel,
                            gcvTRUE,
                            clvCONDITION_EQUAL,
                            rOperandAddr,
                            &constROperand2));

        /* Clamp coord.x to 0. */
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lOperandI0,
                                   &constROperand0));

        /* For coord.x less than 0. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For coord.x greater than or equal to width. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Check if coord.y is greater than or equal to height. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandJ0,
                       clvCONDITION_GREATER_THAN_EQUAL,
                       rOperandH);

        /* Return border color (all 0.0). */
        gcmONERROR(clGenCompareJumpCode(Compiler,
                            CodeGenerator,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            endLabel,
                            gcvTRUE,
                            clvCONDITION_EQUAL,
                            rOperandAddr,
                            &constROperand2));

        /* Clamp to edge (height - 1). */
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lOperandJ0,
                                   rOperandHM1));

        /* For else part of coord.y greater than or equal to height. */
        clmGEN_CODE_ELSE(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        /* Check if coord.y is less than 0. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       rOperandJ0,
                       clvCONDITION_LESS_THAN,
                       &constROperand0);

        /* If CLK_ADDRESS_CLAMP, return border color. */

        /* Return border color (all 0.0). */
        gcmONERROR(clGenCompareJumpCode(Compiler,
                            CodeGenerator,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            endLabel,
                            gcvTRUE,
                            clvCONDITION_EQUAL,
                            rOperandAddr,
                            &constROperand2));

        /* Clamp coord.y to 0. */
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lOperandJ0,
                                   &constROperand0));

        /* For coord.y less than 0. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For coord.y greater than or equal to height. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);
    }

    /* Get rowPitch. */
    clsROPERAND_InitializeIntOrIVecConstant(&constROperand24,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 24);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iOperand[8],
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand24));

    /* Calculate offset of rows (coord y). */
    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iOperand[0],
                                       &rOperand[8],
                                       rOperandJ0));

    /* Get image physical address. */
    clsROPERAND_InitializeIntOrIVecConstant(&constROperand36,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 36);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iOperand[4],
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand36));

    /* Get image channel data type. */
    clsROPERAND_InitializeIntOrIVecConstant(&constROperand12,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 12);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iOperand[1],
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand12));

    clsROPERAND_InitializeFloatOrVecOrMatConstant(&constROperand255,
                                                  clmGenCodeDataType(T_FLOAT),
                                                  1.0f / 255.0f);

    clsROPERAND_InitializeFloatOrVecOrMatConstant(&constROperand65535,
                                                  clmGenCodeDataType(T_FLOAT),
                                                  1.0f / 65535.0f);

#if _SUPPORT_SNORM_FORMATS_
    clsROPERAND_InitializeFloatOrVecOrMatConstant(&constROperand127,
                                                  clmGenCodeDataType(T_FLOAT),
                                                  1.0f / 127.0f);

    clsROPERAND_InitializeFloatOrVecOrMatConstant(&constROperand32767,
                                                  clmGenCodeDataType(T_FLOAT),
                                                  1.0f / 32767.0f);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandSNORM8,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0x10D0 /* CLK_SNORM_INT8 */);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandSNORM16,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0x10D1 /* CLK_SNORM_INT16 */);
#endif

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandUNORM8,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0x10D2 /* CLK_UNORM_INT8 */);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandUNORM16,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0x10D3 /* CLK_UNORM_INT16 */);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandFLOAT16,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0x10DD /* CLK_HALF_FLOAT */);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandFLOAT,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0x10DE /* CLK_FLOAT */);

    /* If filter mode is CLK_FILTER_LINEAR. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   rOperandFilt,
                   clvCONDITION_EQUAL,
                   &constROperandFilter);

    /* Calculate offset of rows (coord y1). */
    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iOperand[9],
                                       &rOperand[8],
                                       rOperandJ1));

    /* Calculate the address offsets of T00, T01, T10, T11. */
    /* Check if type is CLK_UNORM_INT8. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandUNORM8);

    /* Element size is 4. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[5],
                                  rOperandI0,
                                  &constROperand2));

    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[8],
                                  rOperandI1,
                                  &constROperand2));

    clmGEN_CODE_ELSE(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);

    /* Check if type is CLK_UNORM_INT16. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandUNORM16);

    /* Element size is 8. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[5],
                                  rOperandI0,
                                  &constROperand3));

    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[8],
                                  rOperandI1,
                                  &constROperand3));

    clmGEN_CODE_ELSE(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);

    /* Check if type is CLK_HALF_FLOAT. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandFLOAT16);

    /* Element size is 8. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[5],
                                  rOperandI0,
                                  &constROperand3));

    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[8],
                                  rOperandI1,
                                  &constROperand3));

    clmGEN_CODE_ELSE(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);

    /* Check if type is CLK_FLOAT. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandFLOAT);

    /* Element size is 16. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[5],
                                  rOperandI0,
                                  &constROperand4));

    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[8],
                                  rOperandI1,
                                  &constROperand4));

    /* For CLK_FLOAT. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* For CLK_HALF_FLOAT. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* For UNORM_INT16. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* For UNORM_INT8. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperandOffsetT[0],
                                       &rOperand[0],
                                       &rOperand[5]));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperandOffsetT[1],
                                       &rOperand[0],
                                       &rOperand[8]));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperandOffsetT[2],
                                       &rOperand[9],
                                       &rOperand[5]));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperandOffsetT[3],
                                       &rOperand[9],
                                       &rOperand[8]));

    /* If address mode is CLK_ADDRESS_CLAMP. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   rOperandAddr,
                   clvCONDITION_EQUAL,
                   &constROperand2);

    /* Clamp to (0..). */
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_MAX,
                                 &iOperandTemp[0],
                                 &rOperandOffsetT[0],
                                 &constROperand0));

    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_ASSIGN,
                                 &iOperandOffsetT[0],
                                 &rOperandTemp[0]));

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_MAX,
                                 &iOperandTemp[1],
                                 &rOperandOffsetT[1],
                                 &constROperand0));

    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_ASSIGN,
                                 &iOperandOffsetT[1],
                                 &rOperandTemp[1]));

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_MAX,
                                 &iOperandTemp[2],
                                 &rOperandOffsetT[2],
                                 &constROperand0));

    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_ASSIGN,
                                 &iOperandOffsetT[2],
                                 &rOperandTemp[2]));

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_MAX,
                                 &iOperandTemp[3],
                                 &rOperandOffsetT[3],
                                 &constROperand0));

    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_ASSIGN,
                                 &iOperandOffsetT[3],
                                 &rOperandTemp[3]));

    /* For CLK_ADDRESS_CLAMP. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* Check if type is CLK_UNORM_INT8. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandUNORM8);

    /* Load color in format of UNORM_INT8. */
    clsIOPERAND_New(Compiler, iOperandD, clmGenCodeDataType(T_UCHAR4));
    clsROPERAND_InitializeUsingIOperand(rOperandD, iOperandD);

    /* Load T(i0, j0). */
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 iOperandD,
                                 &rOperand[4],
                                 &rOperandOffsetT[0]));

    /* Convert color to float. */
    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_INT_TO_FLOAT,
                                 iFOperandD,
                                 rOperandD));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iFOperandQ[0],
                                       rFOperandD,
                                       &constROperand255));

    /* Load T(i1, j0). */
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 iOperandD,
                                 &rOperand[4],
                                 &rOperandOffsetT[1]));

    /* Convert color to float. */
    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_INT_TO_FLOAT,
                                 iFOperandD,
                                 rOperandD));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iFOperandQ[1],
                                       rFOperandD,
                                       &constROperand255));

    /* Load T(i0, j1). */
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 iOperandD,
                                 &rOperand[4],
                                 &rOperandOffsetT[2]));

    /* Convert color to float. */
    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_INT_TO_FLOAT,
                                 iFOperandD,
                                 rOperandD));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iFOperandQ[2],
                                       rFOperandD,
                                       &constROperand255));

    /* Load T(i1, j1). */
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 iOperandD,
                                 &rOperand[4],
                                 &rOperandOffsetT[3]));

    /* Convert color to float. */
    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_INT_TO_FLOAT,
                                 iFOperandD,
                                 rOperandD));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iFOperandQ[3],
                                       rFOperandD,
                                       &constROperand255));

    clmGEN_CODE_ELSE(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);

    /* Check if type is CLK_UNORM_INT16. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandUNORM16);

    /* Load color in format of UNORM_INT16. */
    clsIOPERAND_New(Compiler, iOperandD, clmGenCodeDataType(T_USHORT4));
    clsROPERAND_InitializeUsingIOperand(rOperandD, iOperandD);

    /* Load T(i0, j0). */
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 iOperandD,
                                 &rOperand[4],
                                 &rOperandOffsetT[0]));

    /* Convert color to float. */
    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_INT_TO_FLOAT,
                                 iFOperandD,
                                 rOperandD));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iFOperandQ[0],
                                       rFOperandD,
                                       &constROperand65535));

    /* Load T(i1, j0). */
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 iOperandD,
                                 &rOperand[4],
                                 &rOperandOffsetT[1]));

    /* Convert color to float. */
    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_INT_TO_FLOAT,
                                 iFOperandD,
                                 rOperandD));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iFOperandQ[1],
                                       rFOperandD,
                                       &constROperand65535));

    /* Load T(i0, j1). */
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 iOperandD,
                                 &rOperand[4],
                                 &rOperandOffsetT[2]));

    /* Convert color to float. */
    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_INT_TO_FLOAT,
                                 iFOperandD,
                                 rOperandD));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iFOperandQ[2],
                                       rFOperandD,
                                       &constROperand65535));

    /* Load T(i1, j1). */
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 iOperandD,
                                 &rOperand[4],
                                 &rOperandOffsetT[3]));

    /* Convert color to float. */
    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_INT_TO_FLOAT,
                                 iFOperandD,
                                 rOperandD));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iFOperandQ[3],
                                       rFOperandD,
                                       &constROperand65535));

    clmGEN_CODE_ELSE(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);

    /* Check if type is CLK_HALF_FLOAT. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandFLOAT16);

    /* Load color in format half float. */

    /* Load T(i0, j0). */
    clmGEN_CODE_elementType_SET(iFOperandQ[0].dataType, clvTYPE_HALF);
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iFOperandQ[0],
                                 &rOperand[4],
                                 &rOperandOffsetT[0]));
    clmGEN_CODE_elementType_SET(iFOperandQ[0].dataType, clvTYPE_FLOAT);

    /* Load T(i1, j0). */
    clmGEN_CODE_elementType_SET(iFOperandQ[1].dataType, clvTYPE_HALF);
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iFOperandQ[1],
                                 &rOperand[4],
                                 &rOperandOffsetT[1]));
    clmGEN_CODE_elementType_SET(iFOperandQ[1].dataType, clvTYPE_FLOAT);

    /* Load T(i0, j1). */
    clmGEN_CODE_elementType_SET(iFOperandQ[2].dataType, clvTYPE_HALF);
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iFOperandQ[2],
                                 &rOperand[4],
                                 &rOperandOffsetT[2]));
    clmGEN_CODE_elementType_SET(iFOperandQ[2].dataType, clvTYPE_FLOAT);

    /* Load T(i1, j1). */
    clmGEN_CODE_elementType_SET(iFOperandQ[3].dataType, clvTYPE_HALF);
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iFOperandQ[3],
                                 &rOperand[4],
                                 &rOperandOffsetT[3]));
    clmGEN_CODE_elementType_SET(iFOperandQ[3].dataType, clvTYPE_FLOAT);

    clmGEN_CODE_ELSE(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);

    /* Check if type is CLK_FLOAT. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandFLOAT);

    /* Load T(i0, j0). */
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iFOperandQ[0],
                                 &rOperand[4],
                                 &rOperandOffsetT[0]));

    /* Load T(i1, j0). */
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iFOperandQ[1],
                                 &rOperand[4],
                                 &rOperandOffsetT[1]));

    /* Load T(i0, j1). */
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iFOperandQ[2],
                                 &rOperand[4],
                                 &rOperandOffsetT[2]));

    /* Load T(i1, j1). */
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iFOperandQ[3],
                                 &rOperand[4],
                                 &rOperandOffsetT[3]));

    /* For CLK_FLOAT. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* For CLK_HALF_FLOAT. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* For UNORM_INT16. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* For UNORM_INT8. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* If address mode is CLK_ADDRESS_CLAMP. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   rOperandAddr,
                   clvCONDITION_EQUAL,
                   &constROperand2);

    /* Check if i0 out of bound. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   rOperandI0,
                   clvCONDITION_EQUAL,
                   &constROperandMinus1);

    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_ASSIGN,
                                 &iFOperandQ[0],
                                 &constROperand0));

    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_ASSIGN,
                                 &iFOperandQ[2],
                                 &constROperand0));

    /* For i0 out of bound. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* Check if i1 out of bound. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   rOperandI1,
                   clvCONDITION_EQUAL,
                   &constROperandMinus1);

    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_ASSIGN,
                                 &iFOperandQ[1],
                                 &constROperand0));

    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_ASSIGN,
                                 &iFOperandQ[3],
                                 &constROperand0));

    /* For i1 out of bound. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* Check if j0 out of bound. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   rOperandJ0,
                   clvCONDITION_EQUAL,
                   &constROperandMinus1);

    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_ASSIGN,
                                 &iFOperandQ[0],
                                 &constROperand0));

    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_ASSIGN,
                                 &iFOperandQ[1],
                                 &constROperand0));

    /* For j0 out of bound. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* Check if j1 out of bound. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   rOperandJ1,
                   clvCONDITION_EQUAL,
                   &constROperandMinus1);

    gcmONERROR(clGenGenericCode1(Compiler,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo,
                     clvOPCODE_ASSIGN,
                     &iFOperandQ[2],
                     &constROperand0));

    gcmONERROR(clGenGenericCode1(Compiler,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo,
                     clvOPCODE_ASSIGN,
                     &iFOperandQ[3],
                     &constROperand0));

    /* For j1 out of bound. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* For CLK_ADDRESS_CLAMP. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* Perform linear filtering. */
    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_SUB,
                                       &iFOperand[6],
                                       &constROperandOne,
                                       rFOperand4X /*&rFOperand[4]*/));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_SUB,
                                       &iFOperand[7],
                                       &constROperandOne,
                                       rFOperand4Y /*&rFOperand[5]*/));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iFOperand[0],
                                       &rFOperand[6],
                                       &rFOperand[7]));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iFOperand[1],
                                       rFOperand4X /*&rFOperand[4]*/,
                                       &rFOperand[7]));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iFOperand[2],
                                       &rFOperand[6],
                                       rFOperand4Y /*&rFOperand[5]*/));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iFOperand[3],
                                       rFOperand4X /*&rFOperand[4]*/,
                                       rFOperand4Y /*&rFOperand[5]*/));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iFOperandQ[4],
                                       &rFOperandQ[3],
                                       &rFOperand[3]));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iFOperandQ[3],
                                       &rFOperandQ[2],
                                       &rFOperand[2]));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       iFOperandD,
                                       &rFOperandQ[4],
                                       &rFOperandQ[3]));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iFOperandQ[2],
                                       &rFOperandQ[1],
                                       &rFOperand[1]));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iFOperandQ[4],
                                       rFOperandD,
                                       &rFOperandQ[2]));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iFOperandQ[1],
                                       &rFOperandQ[0],
                                       &rFOperand[0]));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       IOperand,
                                       &rFOperandQ[4],
                                       &rFOperandQ[1]));

    /* For else part of CLK_FILTER_LINEAR. */
    clmGEN_CODE_ELSE(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);

    /* Check if type is CLK_UNORM_INT8. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandUNORM8);

    /* Element size is 4. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[3],
                                  rOperandI0,
                                  &constROperand2));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperand[1],
                                       &rOperand[0],
                                       &rOperand[3]));

    /* Load color in format of UNORM_INT8. */
    clsIOPERAND_New(Compiler, iOperandD, clmGenCodeDataType(T_UCHAR4));
    clsROPERAND_InitializeUsingIOperand(rOperandD, iOperandD);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 iOperandD,
                                 &rOperand[4],
                                 &rOperand[1]));

    /* Convert color to float. */
    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_INT_TO_FLOAT,
                                 iFOperandD,
                                 rOperandD));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       IOperand,
                                       rFOperandD,
                                       &constROperand255));

    clmGEN_CODE_ELSE(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);

    /* Check if type is CLK_UNORM_INT16. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandUNORM16);

    /* Element size is 8. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[3],
                                  rOperandI0,
                                  &constROperand3));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperand[1],
                                       &rOperand[0],
                                       &rOperand[3]));

    /* Load color in format of UNORM_INT16. */
    clsIOPERAND_New(Compiler, iOperandD, clmGenCodeDataType(T_USHORT4));
    clsROPERAND_InitializeUsingIOperand(rOperandD, iOperandD);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 iOperandD,
                                 &rOperand[4],
                                 &rOperand[1]));

    /* Convert color to float. */
    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_INT_TO_FLOAT,
                                 iFOperandD,
                                 rOperandD));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       IOperand,
                                       rFOperandD,
                                       &constROperand65535));

    clmGEN_CODE_ELSE(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);

#if _SUPPORT_SNORM_FORMATS_
    /* Check if type is CLK_SNORM_INT8. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandSNORM8);

    /* Element size is 4. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[3],
                                  rOperandI0,
                                  &constROperand2));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperand[1],
                                       &rOperand[0],
                                       &rOperand[3]));

    /* Load color in format of SNORM_INT8. */
    clsIOPERAND_New(Compiler, iOperandD, clmGenCodeDataType(T_CHAR4));
    clsROPERAND_InitializeUsingIOperand(rOperandD, iOperandD);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 iOperandD,
                                 &rOperand[4],
                                 &rOperand[1]));

    /* Convert color to float. */
    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_INT_TO_FLOAT,
                                 iFOperandD,
                                 rOperandD));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       IOperand,
                                       rFOperandD,
                                       &constROperand127));

    clmGEN_CODE_ELSE(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);

    /* Check if type is CLK_SNORM_INT16. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandSNORM16);

    /* Element size is 8. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[3],
                                  rOperandI0,
                                  &constROperand3));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperand[1],
                                       &rOperand[0],
                                       &rOperand[3]));

    /* Load color in format of SNORM_INT16. */
    clsIOPERAND_New(Compiler, iOperandD, clmGenCodeDataType(T_SHORT4));
    clsROPERAND_InitializeUsingIOperand(rOperandD, iOperandD);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 iOperandD,
                                 &rOperand[4],
                                 &rOperand[1]));

    /* Convert color to float. */
    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_INT_TO_FLOAT,
                                 iFOperandD,
                                 rOperandD));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       IOperand,
                                       rFOperandD,
                                       &constROperand32767));

    clmGEN_CODE_ELSE(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);
#endif

    /* Check if type is CLK_HALF_FLOAT. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandFLOAT16);

    /* Element size is 8. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[3],
                                  rOperandI0,
                                  &constROperand3));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperand[1],
                                       &rOperand[0],
                                       &rOperand[3]));

    /* Load color in format half float. */
    gcmASSERT(clmGEN_CODE_elementType_GET(IOperand->dataType) == clvTYPE_FLOAT);
    clmGEN_CODE_elementType_SET(IOperand->dataType, clvTYPE_HALF);
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 IOperand,
                                 &rOperand[4],
                                 &rOperand[1]));
    clmGEN_CODE_elementType_SET(IOperand->dataType, clvTYPE_FLOAT);

    clmGEN_CODE_ELSE(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);

    /* Check if type is CLK_FLOAT. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandFLOAT);

    /* Element size is 16. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[3],
                                  rOperandI0,
                                  &constROperand4));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperand[1],
                                       &rOperand[0],
                                       &rOperand[3]));

    /* Load color in format float. */
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 IOperand,
                                 &rOperand[4],
                                 &rOperand[1]));

    /* For CLK_FLOAT. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* For CLK_HALF_FLOAT. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

#if _SUPPORT_SNORM_FORMATS_
    /* For SNORM_INT16. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* For SNORM_INT8. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);
#endif

    /* For UNORM_INT16. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* For UNORM_INT8. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* For CLK_FILTER_LINEAR. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* Go to real end */
    clGenGotoCode(Compiler, CodeGenerator, gcvTRUE, lastLabel);

    /* set the label for the common code */
    gcmONERROR(clSetLabel(Compiler,
              PolynaryExpr->exprBase.base.lineNo,
              PolynaryExpr->exprBase.base.stringNo,
              endLabel));
    gcmONERROR(clGenGenericCode1(Compiler,
                 PolynaryExpr->exprBase.base.lineNo,
                 PolynaryExpr->exprBase.base.stringNo,
                 clvOPCODE_ASSIGN,
                 IOperand,
                 &constROperand0));

    clGenLabelCode(Compiler, CodeGenerator, gcvFALSE, lastLabel);

OnError:
    return status;
}

static gceSTATUS
_GenReadImageFCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    clsSAMPLER_TYPES *samplerTypes = gcvNULL;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3 || OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (OperandCount == 3)
    {
        status = _FindImageSampler(Compiler,
                               PolynaryExpr,
                               &samplerTypes);
        if (gcmIS_ERROR(status)) return status;
    }

#if (_OCL_USE_INTRINSIC_FOR_IMAGE || _USE_NEW_INSTRUCTION_FOR_IMAGE_RD_SAMPLER)
    return _GenReadImageSamplerCode(Compiler,
                                    CodeGenerator,
                                    PolynaryExpr,
                                    OperandCount,
                                    OperandsParameters,
                                    IOperand);
#else
#if _USE_TEXLD_FOR_IMAGE_SAMPLER

    return _GenTextureCode(Compiler,
                           CodeGenerator,
                           PolynaryExpr,
                           OperandCount,
                           OperandsParameters,
                           samplerTypes,
                           IOperand);
#else
    return _GenSoftReadImageFCode(Compiler,
                                  CodeGenerator,
                                  PolynaryExpr,
                                  OperandsParameters,
                                  IOperand);
#endif
#endif
}

static gceSTATUS
_GenSoftReadImageICode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    clsROPERAND constROperand0, constROperand1;
    clsROPERAND constROperand2, constROperand3;
    clsROPERAND constROperand4, constROperand12;
    clsROPERAND constROperand24, constROperand36;
    clsROPERAND constROperandAddr;
    clsROPERAND constROperandNorm;
    clsROPERAND constROperandINT8, constROperandINT16, constROperandINT32;
    clsIOPERAND iOperand[5], iOperandD[1];
    clsROPERAND rOperand[5], rOperandD[1];
    clsROPERAND rOperandX[1], rOperandY[1];
    struct _cloIR_LABEL endLabel[1];

    clsIOPERAND_New(Compiler, &iOperand[0], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[0], &iOperand[0]);

    clsIOPERAND_New(Compiler, &iOperand[1], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[1], &iOperand[1]);

    clsIOPERAND_New(Compiler, &iOperand[2], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[2], &iOperand[2]);

    clsIOPERAND_New(Compiler, &iOperand[3], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[3], &iOperand[3]);

    clsIOPERAND_New(Compiler, &iOperand[4], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[4], &iOperand[4]);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand0,
                                            clmGenCodeDataType(T_INT),
                                            (gctUINT) 0);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand1,
                                            clmGenCodeDataType(T_INT),
                                            (gctUINT) 1);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand2,
                                            clmGenCodeDataType(T_INT),
                                            (gctUINT) 2);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand3,
                                            clmGenCodeDataType(T_INT),
                                            (gctUINT) 3);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand4,
                                            clmGenCodeDataType(T_INT),
                                            (gctUINT) 4);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandAddr,
                                            clmGenCodeDataType(T_INT),
                                            (gctUINT) 7);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandNorm,
                                            clmGenCodeDataType(T_INT),
                                            (gctUINT) 0x10000);

    /* Get width and height. */
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iOperand[0],
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand0));
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iOperand[1],
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand4));

    /* Get addressing mode. */
    gcmONERROR(clGenBitwiseExprCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_AND_BITWISE,
                                    &iOperand[4],
                                    &OperandsParameters[1].rOperands[0],
                                    &constROperandAddr));

    /* Get coord x. */
    clmROPERAND_vectorComponent_GET(rOperandX,
                                    &OperandsParameters[2].rOperands[0],
                                    clvCOMPONENT_X);

    /* Get coord y. */
    clmROPERAND_vectorComponent_GET(rOperandY,
                                    &OperandsParameters[2].rOperands[0],
                                    clvCOMPONENT_Y);

    /* read_imagei supports CLK_FILTER_NEAREST only. */

    /* Check coord data type. */
    if (OperandsParameters[2].rOperands[0].dataType.elementType == clvTYPE_FLOAT)
    {
        clsIOPERAND iFOperand[3];
        clsROPERAND rFOperand[3];
        clsROPERAND constROperandHalf;

        clsIOPERAND_New(Compiler, &iFOperand[0], clmGenCodeDataType(T_FLOAT));
        clsROPERAND_InitializeUsingIOperand(&rFOperand[0], &iFOperand[0]);

        clsIOPERAND_New(Compiler, &iFOperand[1], clmGenCodeDataType(T_FLOAT));
        clsROPERAND_InitializeUsingIOperand(&rFOperand[1], &iFOperand[1]);

        clsIOPERAND_New(Compiler, &iFOperand[2], clmGenCodeDataType(T_FLOAT));
        clsROPERAND_InitializeUsingIOperand(&rFOperand[2], &iFOperand[2]);

        clsROPERAND_InitializeFloatOrVecOrMatConstant(&constROperandHalf,
                                                      clmGenCodeDataType(T_FLOAT),
                                                      (gctFLOAT) 0.5f);

        /* If CLK_ADDRESS_REPEAT. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[4],
                       clvCONDITION_EQUAL,
                       &constROperand3);

        /* Process coord.x. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_INT_TO_FLOAT,
                                     &iFOperand[0],
                                     &rOperand[0]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FRACT,
                                     &iFOperand[1],
                                     rOperandX));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_MUL,
                                           &iFOperand[2],
                                           &rFOperand[0],
                                           &rFOperand[1]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     &iFOperand[0],
                                     &rFOperand[2]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOAT_TO_INT,
                                     &iOperand[2],
                                     &rFOperand[0]));

        /* Check if i >= width. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[2],
                       clvCONDITION_GREATER_THAN_EQUAL,
                       &rOperand[0]);

        /* Wrap i to 0. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     &iOperand[2],
                                     &constROperand0));

        /* For i >= width. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Process coord.y. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_INT_TO_FLOAT,
                                     &iFOperand[0],
                                     &rOperand[1]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FRACT,
                                     &iFOperand[1],
                                     rOperandY));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_MUL,
                                           &iFOperand[2],
                                           &rFOperand[0],
                                           &rFOperand[1]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     &iFOperand[0],
                                     &rFOperand[2]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOAT_TO_INT,
                                     &iOperand[3],
                                     &rFOperand[0]));

        /* Check if j >= height. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[3],
                       clvCONDITION_GREATER_THAN_EQUAL,
                       &rOperand[1]);

        /* Wrap j to 0. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     &iOperand[3],
                                     &constROperand0));

        /* For j >= height. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For else part of CLK_ADDRESS_REPEAT. */
        clmGEN_CODE_ELSE(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        /* If CLK_ADDRESS_MIRRORED_REPEAT. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[4],
                       clvCONDITION_EQUAL,
                       &constROperand4);

        /* Process coord.x. */
        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_MUL,
                                           &iFOperand[0],
                                           &constROperandHalf,
                                           rOperandX));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_ADD,
                                           &iFOperand[1],
                                           &constROperandHalf,
                                           &rFOperand[0]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     &iFOperand[0],
                                     &rFOperand[1]));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_ADD,
                                           &iFOperand[1],
                                           &rFOperand[0],
                                           &rFOperand[0]));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_SUB,
                                           &iFOperand[0],
                                           rOperandX,
                                           &rFOperand[1]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ABS,
                                     &iFOperand[1],
                                     &rFOperand[0]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_INT_TO_FLOAT,
                                     &iFOperand[0],
                                     &rOperand[0]));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_MUL,
                                           &iFOperand[2],
                                           &rFOperand[0],
                                           &rFOperand[1]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     &iFOperand[0],
                                     &rFOperand[2]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOAT_TO_INT,
                                     &iOperand[2],
                                     &rFOperand[0]));

        /* Check if i >= width. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[2],
                       clvCONDITION_GREATER_THAN_EQUAL,
                       &rOperand[0]);

        /* Clamp i to width - 1. */
        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_SUB,
                                           &iOperand[2],
                                           &rOperand[0],
                                           &constROperand1));

        /* For i >= width. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Process coord.y. */
        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_MUL,
                                           &iFOperand[0],
                                           &constROperandHalf,
                                           rOperandY));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_ADD,
                                           &iFOperand[1],
                                           &constROperandHalf,
                                           &rFOperand[0]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     &iFOperand[0],
                                     &rFOperand[1]));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_ADD,
                                           &iFOperand[1],
                                           &rFOperand[0],
                                           &rFOperand[0]));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_SUB,
                                           &iFOperand[0],
                                           rOperandY,
                                           &rFOperand[1]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ABS,
                                     &iFOperand[1],
                                     &rFOperand[0]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_INT_TO_FLOAT,
                                     &iFOperand[0],
                                     &rOperand[1]));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_MUL,
                                           &iFOperand[2],
                                           &rFOperand[0],
                                           &rFOperand[1]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     &iFOperand[0],
                                     &rFOperand[2]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOAT_TO_INT,
                                     &iOperand[3],
                                     &rFOperand[0]));

        /* Check if j >= height. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[3],
                       clvCONDITION_GREATER_THAN_EQUAL,
                       &rOperand[1]);

        /* Clamp j to width - 1. */
        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_SUB,
                                           &iOperand[3],
                                           &rOperand[1],
                                           &constROperand1));

        /* For j >= height. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For else part of CLK_ADDRESS_MIRRORED_REPEAT. */
        clmGEN_CODE_ELSE(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        /* Get normalized coords mode. */
        gcmONERROR(clGenBitwiseExprCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        clvOPCODE_AND_BITWISE,
                                        &iOperand[3],
                                        &OperandsParameters[1].rOperands[0],
                                        &constROperandNorm));

        /* If CLK_NORMALIZED_COORDS_TRUE. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[3],
                       clvCONDITION_EQUAL,
                       &constROperandNorm);

        /* Multiply coord.x by width. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_INT_TO_FLOAT,
                                     &iFOperand[0],
                                     &rOperand[0]));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_MUL,
                                           &iFOperand[2],
                                           &rFOperand[0],
                                           rOperandX));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     &iFOperand[0],
                                     &rFOperand[2]));

        /* Multiply coord.y by height. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_INT_TO_FLOAT,
                                     &iFOperand[1],
                                     &rOperand[1]));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_MUL,
                                           &iFOperand[2],
                                           &rFOperand[1],
                                           rOperandY));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     &iFOperand[1],
                                     &rFOperand[2]));

        /* For else part of CLK_NORMALIZED_COORDS_TRUE. */
        clmGEN_CODE_ELSE(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     &iFOperand[0],
                                     rOperandX));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     &iFOperand[1],
                                     rOperandY));

        /* For CLK_NORMALIZED_COORDS_TRUE. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOAT_TO_INT,
                                     &iOperand[2],
                                     &rFOperand[0]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOAT_TO_INT,
                                     &iOperand[3],
                                     &rFOperand[1]));

        /* Check if coord.x is greater than or equal to width. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[2],
                       clvCONDITION_GREATER_THAN_EQUAL,
                       &rOperand[0]);

        /* If CLK_ADDRESS_CLAMP, return border color. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[4],
                       clvCONDITION_EQUAL,
                       &constROperand2);

        /* Return border color (all 0.0). */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     IOperand,
                                     &constROperand0));

        /* Done - goto end. */
        clGenGotoCode(Compiler, CodeGenerator, gcvTRUE, endLabel);

        /* For CLK_ADDRESS_CLAMP. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Clamp to edge (width - 1). */
        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_SUB,
                                           &iOperand[2],
                                           &rOperand[0],
                                           &constROperand1));

        /* For else part of coord.x greater than or equal to width. */
        clmGEN_CODE_ELSE(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        /* Check if coord.x is less than 0. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[2],
                       clvCONDITION_LESS_THAN,
                       &constROperand0);

        /* If CLK_ADDRESS_CLAMP, return border color. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[4],
                       clvCONDITION_EQUAL,
                       &constROperand2);

        /* Return border color (all 0.0). */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     IOperand,
                                     &constROperand0));

        /* Done - goto end. */
        clGenGotoCode(Compiler, CodeGenerator, gcvFALSE, endLabel);

        /* For CLK_ADDRESS_CLAMP. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Clamp coord.x to 0. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     &iOperand[2],
                                     &constROperand0));

        /* For coord.x less than 0. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For coord.x greater than or equal to width. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Check if coord.y is greater than or equal to height. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[3],
                       clvCONDITION_GREATER_THAN_EQUAL,
                       &rOperand[1]);

        /* If CLK_ADDRESS_CLAMP, return border color. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[4],
                       clvCONDITION_EQUAL,
                       &constROperand2);

        /* Return border color (all 0.0). */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     IOperand,
                                     &constROperand0));

        /* Done - goto end. */
        clGenGotoCode(Compiler, CodeGenerator, gcvFALSE, endLabel);

        /* For CLK_ADDRESS_CLAMP. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Clamp to edge (height - 1). */
        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_SUB,
                                           &iOperand[3],
                                           &rOperand[1],
                                           &constROperand1));

        /* For else part of coord.y greater than or equal to height. */
        clmGEN_CODE_ELSE(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        /* Check if coord.y is less than 0. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[3],
                       clvCONDITION_LESS_THAN,
                       &constROperand0);

        /* If CLK_ADDRESS_CLAMP, return border color. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[4],
                       clvCONDITION_EQUAL,
                       &constROperand2);

        /* Return border color (all 0.0). */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     IOperand,
                                     &constROperand0));

        /* Done - goto end. */
        clGenGotoCode(Compiler, CodeGenerator, gcvFALSE, endLabel);

        /* For CLK_ADDRESS_CLAMP. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Clamp coord.y to 0. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     &iOperand[3],
                                     &constROperand0));

        /* For coord.y less than 0. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For coord.y greater than or equal to height. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For CLK_ADDRESS_MIRRORED_REPEAT. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For CLK_ADDRESS_REPEAT. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);
    }
    else
    {
        /* Move coord.x to iOperand[2]. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     &iOperand[2],
                                     rOperandX));

        /* Move coord.y to iOperand[3]. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     &iOperand[3],
                                     rOperandY));

        /* Check if coord.x is greater than or equal to width. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[2],
                       clvCONDITION_GREATER_THAN_EQUAL,
                       &rOperand[0]);

        /* If CLK_ADDRESS_CLAMP, return border color. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[4],
                       clvCONDITION_EQUAL,
                       &constROperand2);

        /* Return border color (all 0.0). */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     IOperand,
                                     &constROperand0));

        /* Done - goto end. */
        clGenGotoCode(Compiler, CodeGenerator, gcvTRUE, endLabel);

        /* For CLK_ADDRESS_CLAMP. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Clamp to edge (width - 1). */
        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_SUB,
                                           &iOperand[2],
                                           &rOperand[0],
                                           &constROperand1));

        /* For else part of coord.x greater than or equal to width. */
        clmGEN_CODE_ELSE(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        /* Check if coord.x is less than 0. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[2],
                       clvCONDITION_LESS_THAN,
                       &constROperand0);

        /* If CLK_ADDRESS_CLAMP, return border color. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[4],
                       clvCONDITION_EQUAL,
                       &constROperand2);

        /* Return border color (all 0.0). */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     IOperand,
                                     &constROperand0));

        /* Done - goto end. */
        clGenGotoCode(Compiler, CodeGenerator, gcvFALSE, endLabel);

        /* For CLK_ADDRESS_CLAMP. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Clamp coord.x to 0. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     &iOperand[2],
                                     &constROperand0));

        /* For coord.x less than 0. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For coord.x greater than or equal to width. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Check if coord.y is greater than or equal to height. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[3],
                       clvCONDITION_GREATER_THAN_EQUAL,
                       &rOperand[1]);

        /* If CLK_ADDRESS_CLAMP, return border color. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[4],
                       clvCONDITION_EQUAL,
                       &constROperand2);

        /* Return border color (all 0.0). */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     IOperand,
                                     &constROperand0));

        /* Done - goto end. */
        clGenGotoCode(Compiler, CodeGenerator, gcvFALSE, endLabel);

        /* For CLK_ADDRESS_CLAMP. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Clamp to edge (height - 1). */
        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_SUB,
                                           &iOperand[3],
                                           &rOperand[1],
                                           &constROperand1));

        /* For else part of coord.y greater than or equal to height. */
        clmGEN_CODE_ELSE(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        /* Check if coord.y is less than 0. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[3],
                       clvCONDITION_LESS_THAN,
                       &constROperand0);

        /* If CLK_ADDRESS_CLAMP, return border color. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[4],
                       clvCONDITION_EQUAL,
                       &constROperand2);

        /* Return border color (all 0.0). */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     IOperand,
                                     &constROperand0));

        /* Done - goto end. */
        clGenGotoCode(Compiler, CodeGenerator, gcvFALSE, endLabel);

        /* For CLK_ADDRESS_CLAMP. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Clamp coord.y to 0. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     &iOperand[3],
                                     &constROperand0));

        /* For coord.y less than 0. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For coord.y greater than or equal to height. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);
    }

    /* Get rowPitch. */
    clsROPERAND_InitializeIntOrIVecConstant(&constROperand24,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 24);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iOperand[1],
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand24));

    /* Calculate offset of rows (coord y). */
    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iOperand[0],
                                       &rOperand[1],
                                       &rOperand[3]));

    /* Get image physical address. */
    clsROPERAND_InitializeIntOrIVecConstant(&constROperand36,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 36);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iOperand[4],
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand36));

    /* Get image channel data type. */
    clsROPERAND_InitializeIntOrIVecConstant(&constROperand12,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 12);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iOperand[1],
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand12));

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandINT8,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0x10D7 /* CLK_SIGNED_INT8 */);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandINT16,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0x10D8 /* CLK_SIGNED_INT16 */);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandINT32,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0x10D9 /* CLK_SIGNED_INT32 */);

    /* Check if type is CLK_SIGNED_INT8. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandINT8);

    /* Element size is 4. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[3],
                                  &rOperand[2],
                                  &constROperand2));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperand[1],
                                       &rOperand[0],
                                       &rOperand[3]));

    /* Load color in format of CLK_SIGNED_INT8. */
    clsIOPERAND_New(Compiler, iOperandD, clmGenCodeDataType(T_CHAR4));
    clsROPERAND_InitializeUsingIOperand(rOperandD, iOperandD);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 iOperandD,
                                 &rOperand[4],
                                 &rOperand[1]));

    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_ASSIGN,
                                 IOperand,
                                 rOperandD));

    clmGEN_CODE_ELSE(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);

    /* Check if type is CLK_SIGNED_INT16. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandINT16);

    /* Element size is 8. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[3],
                                  &rOperand[2],
                                  &constROperand3));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperand[1],
                                       &rOperand[0],
                                       &rOperand[3]));

    /* Load color in format of CLK_SIGNED_INT16. */
    clsIOPERAND_New(Compiler, iOperandD, clmGenCodeDataType(T_SHORT4));
    clsROPERAND_InitializeUsingIOperand(rOperandD, iOperandD);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 iOperandD,
                                 &rOperand[4],
                                 &rOperand[1]));

    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_ASSIGN,
                                 IOperand,
                                 rOperandD));

    clmGEN_CODE_ELSE(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);

    /* Check if type is CLK_SIGNED_INT32. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandINT32);

    /* Element size is 16. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[3],
                                  &rOperand[2],
                                  &constROperand4));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperand[1],
                                       &rOperand[0],
                                       &rOperand[3]));

    /* Load color in format CLK_SIGNED_INT32. */
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 IOperand,
                                 &rOperand[4],
                                 &rOperand[1]));

    /* For CLK_SIGNED_INT32. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* For CLK_SIGNED_INT16. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* For CLK_SIGNED_INT8. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* Create end lable. */
    clGenLabelCode(Compiler, CodeGenerator, gcvFALSE, endLabel);

OnError:
    return status;
}

static gceSTATUS
_GenReadImageICode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    clsSAMPLER_TYPES *samplerTypes = gcvNULL;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3 || OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);
    if (OperandCount == 3)
    {
        status = _FindImageSampler(Compiler,
                                   PolynaryExpr,
                                   &samplerTypes);
        if (gcmIS_ERROR(status)) return status;
    }

#if (_OCL_USE_INTRINSIC_FOR_IMAGE || _USE_NEW_INSTRUCTION_FOR_IMAGE_RD_SAMPLER)
    return _GenReadImageSamplerCode(Compiler,
                                    CodeGenerator,
                                    PolynaryExpr,
                                    OperandCount,
                                    OperandsParameters,
                                    IOperand);
#else
#if _USE_TEXLD_FOR_IMAGE_SAMPLER

    return _GenTextureCode(Compiler,
                           CodeGenerator,
                           PolynaryExpr,
                           OperandCount,
                           OperandsParameters,
                           samplerTypes,
                           IOperand);
#else
    return _GenSoftReadImageICode(Compiler,
                                  CodeGenerator,
                                  PolynaryExpr,
                                  OperandsParameters,
                                  IOperand);
#endif
#endif
}

static gceSTATUS
_GenSoftReadImageUICode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    clsROPERAND constROperand0, constROperand1;
    clsROPERAND constROperand2, constROperand3;
    clsROPERAND constROperand4, constROperand12;
    clsROPERAND constROperand24, constROperand36;
    clsROPERAND constROperandAddr;
    clsROPERAND constROperandNorm;
    clsROPERAND constROperandUINT8, constROperandUINT16, constROperandUINT32;
    clsIOPERAND iOperand[6], iOperandD[1];
    clsROPERAND rOperand[6], rOperandD[1];
    clsROPERAND rOperandX[1], rOperandY[1];
    struct _cloIR_LABEL endLabel[1];

    clsIOPERAND_New(Compiler, &iOperand[0], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[0], &iOperand[0]);

    clsIOPERAND_New(Compiler, &iOperand[1], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[1], &iOperand[1]);

    clsIOPERAND_New(Compiler, &iOperand[2], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[2], &iOperand[2]);

    clsIOPERAND_New(Compiler, &iOperand[3], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[3], &iOperand[3]);

    clsIOPERAND_New(Compiler, &iOperand[4], clmGenCodeDataType(T_INT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[4], &iOperand[4]);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand0,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand1,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 1);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand2,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 2);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand3,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 3);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand4,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 4);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandAddr,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 7);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandNorm,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0x10000);

    /* Get width and height. */
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iOperand[0],
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand0));
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iOperand[1],
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand4));

    /* Get addressing mode. */
    gcmONERROR(clGenBitwiseExprCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_AND_BITWISE,
                                    &iOperand[4],
                                    &OperandsParameters[1].rOperands[0],
                                    &constROperandAddr));

    /* Get coord x. */
    clmROPERAND_vectorComponent_GET(rOperandX,
                                    &OperandsParameters[2].rOperands[0],
                                    clvCOMPONENT_X);

    /* Get coord y. */
    clmROPERAND_vectorComponent_GET(rOperandY,
                                    &OperandsParameters[2].rOperands[0],
                                    clvCOMPONENT_Y);

    /* read_imageui supports CLK_FILTER_NEAREST only. */

    /* Check coord data type. */
    if (OperandsParameters[2].rOperands[0].dataType.elementType == clvTYPE_FLOAT)
    {
        clsIOPERAND iFOperand[3];
        clsROPERAND rFOperand[3];
        clsROPERAND constROperandHalf;

        clsIOPERAND_New(Compiler, &iFOperand[0], clmGenCodeDataType(T_FLOAT));
        clsROPERAND_InitializeUsingIOperand(&rFOperand[0], &iFOperand[0]);

        clsIOPERAND_New(Compiler, &iFOperand[1], clmGenCodeDataType(T_FLOAT));
        clsROPERAND_InitializeUsingIOperand(&rFOperand[1], &iFOperand[1]);

        clsIOPERAND_New(Compiler, &iFOperand[2], clmGenCodeDataType(T_FLOAT));
        clsROPERAND_InitializeUsingIOperand(&rFOperand[2], &iFOperand[2]);

        clsROPERAND_InitializeFloatOrVecOrMatConstant(&constROperandHalf,
                                                      clmGenCodeDataType(T_FLOAT),
                                                      (gctFLOAT) 0.5f);

        /* If CLK_ADDRESS_REPEAT. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[4],
                       clvCONDITION_EQUAL,
                       &constROperand3);

        /* Process coord.x. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_INT_TO_FLOAT,
                                     &iFOperand[0],
                                     &rOperand[0]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FRACT,
                                     &iFOperand[1],
                                     rOperandX));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_MUL,
                                           &iFOperand[2],
                                           &rFOperand[0],
                                           &rFOperand[1]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     &iFOperand[0],
                                     &rFOperand[2]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOAT_TO_INT,
                                     &iOperand[2],
                                     &rFOperand[0]));

        /* Check if i >= width. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[2],
                       clvCONDITION_GREATER_THAN_EQUAL,
                       &rOperand[0]);

        /* Wrap i to 0. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     &iOperand[2],
                                     &constROperand0));

        /* For i >= width. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Process coord.y. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_INT_TO_FLOAT,
                                     &iFOperand[0],
                                     &rOperand[1]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FRACT,
                                     &iFOperand[1],
                                     rOperandY));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_MUL,
                                           &iFOperand[2],
                                           &rFOperand[0],
                                           &rFOperand[1]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     &iFOperand[0],
                                     &rFOperand[2]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOAT_TO_INT,
                                     &iOperand[3],
                                     &rFOperand[0]));

        /* Check if j >= height. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[3],
                       clvCONDITION_GREATER_THAN_EQUAL,
                       &rOperand[1]);

        /* Wrap j to 0. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     &iOperand[3],
                                     &constROperand0));

        /* For j >= height. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For else part of CLK_ADDRESS_REPEAT. */
        clmGEN_CODE_ELSE(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        /* If CLK_ADDRESS_MIRRORED_REPEAT. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[4],
                       clvCONDITION_EQUAL,
                       &constROperand4);

        /* Process coord.x. */
        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_MUL,
                                           &iFOperand[0],
                                           &constROperandHalf,
                                           rOperandX));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_ADD,
                                           &iFOperand[1],
                                           &constROperandHalf,
                                           &rFOperand[0]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     &iFOperand[0],
                                     &rFOperand[1]));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_ADD,
                                           &iFOperand[1],
                                           &rFOperand[0],
                                           &rFOperand[0]));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_SUB,
                                           &iFOperand[0],
                                           rOperandX,
                                           &rFOperand[1]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ABS,
                                     &iFOperand[1],
                                     &rFOperand[0]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_INT_TO_FLOAT,
                                     &iFOperand[0],
                                     &rOperand[0]));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_MUL,
                                           &iFOperand[2],
                                           &rFOperand[0],
                                           &rFOperand[1]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     &iFOperand[0],
                                     &rFOperand[2]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOAT_TO_INT,
                                     &iOperand[2],
                                     &rFOperand[0]));

        /* Check if i >= width. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[2],
                       clvCONDITION_GREATER_THAN_EQUAL,
                       &rOperand[0]);

        /* Clamp i to width - 1. */
        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_SUB,
                                           &iOperand[2],
                                           &rOperand[0],
                                           &constROperand1));

        /* For i >= width. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Process coord.y. */
        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_MUL,
                                           &iFOperand[0],
                                           &constROperandHalf,
                                           rOperandY));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_ADD,
                                           &iFOperand[1],
                                           &constROperandHalf,
                                           &rFOperand[0]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     &iFOperand[0],
                                     &rFOperand[1]));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_ADD,
                                           &iFOperand[1],
                                           &rFOperand[0],
                                           &rFOperand[0]));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_SUB,
                                           &iFOperand[0],
                                           rOperandY,
                                           &rFOperand[1]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ABS,
                                     &iFOperand[1],
                                     &rFOperand[0]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_INT_TO_FLOAT,
                                     &iFOperand[0],
                                     &rOperand[1]));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_MUL,
                                           &iFOperand[2],
                                           &rFOperand[0],
                                           &rFOperand[1]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     &iFOperand[0],
                                     &rFOperand[2]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOAT_TO_INT,
                                     &iOperand[3],
                                     &rFOperand[0]));

        /* Check if j >= height. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[3],
                       clvCONDITION_GREATER_THAN_EQUAL,
                       &rOperand[1]);

        /* Clamp j to width - 1. */
        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_SUB,
                                           &iOperand[3],
                                           &rOperand[1],
                                           &constROperand1));

        /* For j >= height. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For else part of CLK_ADDRESS_MIRRORED_REPEAT. */
        clmGEN_CODE_ELSE(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        /* Get normalized coords mode. */
        gcmONERROR(clGenBitwiseExprCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        clvOPCODE_AND_BITWISE,
                                        &iOperand[3],
                                        &OperandsParameters[1].rOperands[0],
                                        &constROperandNorm));

        /* If CLK_NORMALIZED_COORDS_TRUE. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[3],
                       clvCONDITION_EQUAL,
                       &constROperandNorm);

        /* Multiply coord.x by width. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_INT_TO_FLOAT,
                                     &iFOperand[0],
                                     &rOperand[0]));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_MUL,
                                           &iFOperand[2],
                                           &rFOperand[0],
                                           rOperandX));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     &iFOperand[0],
                                     &rFOperand[2]));

        /* Multiply coord.y by height. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_INT_TO_FLOAT,
                                     &iFOperand[1],
                                     &rOperand[1]));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_MUL,
                                           &iFOperand[2],
                                           &rFOperand[1],
                                           rOperandY));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     &iFOperand[1],
                                     &rFOperand[2]));

        /* For else part of CLK_NORMALIZED_COORDS_TRUE. */
        clmGEN_CODE_ELSE(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     &iFOperand[0],
                                     rOperandX));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOOR,
                                     &iFOperand[1],
                                     rOperandY));

        /* For CLK_NORMALIZED_COORDS_TRUE. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOAT_TO_INT,
                                     &iOperand[2],
                                     &rFOperand[0]));

        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_FLOAT_TO_INT,
                                     &iOperand[3],
                                     &rFOperand[1]));

        /* Check if coord.x is greater than or equal to width. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[2],
                       clvCONDITION_GREATER_THAN_EQUAL,
                       &rOperand[0]);

        /* If CLK_ADDRESS_CLAMP, return border color. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[4],
                       clvCONDITION_EQUAL,
                       &constROperand2);

        /* Return border color (all 0.0). */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     IOperand,
                                     &constROperand0));

        /* Done - goto end. */
        clGenGotoCode(Compiler, CodeGenerator, gcvTRUE, endLabel);

        /* For CLK_ADDRESS_CLAMP. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Clamp to edge (width - 1). */
        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_SUB,
                                           &iOperand[2],
                                           &rOperand[0],
                                           &constROperand1));

        /* For else part of coord.x greater than or equal to width. */
        clmGEN_CODE_ELSE(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        /* Check if coord.x is less than 0. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[2],
                       clvCONDITION_LESS_THAN,
                       &constROperand0);

        /* If CLK_ADDRESS_CLAMP, return border color. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[4],
                       clvCONDITION_EQUAL,
                       &constROperand2);

        /* Return border color (all 0.0). */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     IOperand,
                                     &constROperand0));

        /* Done - goto end. */
        clGenGotoCode(Compiler, CodeGenerator, gcvFALSE, endLabel);

        /* For CLK_ADDRESS_CLAMP. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Clamp coord.x to 0. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     &iOperand[2],
                                     &constROperand0));

        /* For coord.x less than 0. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For coord.x greater than or equal to width. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Check if coord.y is greater than or equal to height. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[3],
                       clvCONDITION_GREATER_THAN_EQUAL,
                       &rOperand[1]);

        /* If CLK_ADDRESS_CLAMP, return border color. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[4],
                       clvCONDITION_EQUAL,
                       &constROperand2);

        /* Return border color (all 0.0). */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     IOperand,
                                     &constROperand0));

        /* Done - goto end. */
        clGenGotoCode(Compiler, CodeGenerator, gcvFALSE, endLabel);

        /* For CLK_ADDRESS_CLAMP. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Clamp to edge (height - 1). */
        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_SUB,
                                           &iOperand[3],
                                           &rOperand[1],
                                           &constROperand1));

        /* For else part of coord.y greater than or equal to height. */
        clmGEN_CODE_ELSE(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        /* Check if coord.y is less than 0. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[3],
                       clvCONDITION_LESS_THAN,
                       &constROperand0);

        /* If CLK_ADDRESS_CLAMP, return border color. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[4],
                       clvCONDITION_EQUAL,
                       &constROperand2);

        /* Return border color (all 0.0). */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     IOperand,
                                     &constROperand0));

        /* Done - goto end. */
        clGenGotoCode(Compiler, CodeGenerator, gcvFALSE, endLabel);

        /* For CLK_ADDRESS_CLAMP. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Clamp coord.y to 0. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     &iOperand[3],
                                     &constROperand0));

        /* For coord.y less than 0. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For coord.y greater than or equal to height. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For CLK_ADDRESS_MIRRORED_REPEAT. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For CLK_ADDRESS_REPEAT. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);
    }
    else
    {
        /* Move coord.x to iOperand[2]. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     &iOperand[2],
                                     rOperandX));

        /* Move coord.y to iOperand[3]. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     &iOperand[3],
                                     rOperandY));

        /* Check if coord.x is greater than or equal to width. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[2],
                       clvCONDITION_GREATER_THAN_EQUAL,
                       &rOperand[0]);

        /* If CLK_ADDRESS_CLAMP, return border color. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[4],
                       clvCONDITION_EQUAL,
                       &constROperand2);

        /* Return border color (all 0.0). */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     IOperand,
                                     &constROperand0));

        /* Done - goto end. */
        clGenGotoCode(Compiler, CodeGenerator, gcvTRUE, endLabel);

        /* For CLK_ADDRESS_CLAMP. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Clamp to edge (width - 1). */
        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_SUB,
                                           &iOperand[2],
                                           &rOperand[0],
                                           &constROperand1));

        /* For else part of coord.x greater than or equal to width. */
        clmGEN_CODE_ELSE(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        /* Check if coord.x is less than 0. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[2],
                       clvCONDITION_LESS_THAN,
                       &constROperand0);

        /* If CLK_ADDRESS_CLAMP, return border color. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[4],
                       clvCONDITION_EQUAL,
                       &constROperand2);

        /* Return border color (all 0.0). */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     IOperand,
                                     &constROperand0));

        /* Done - goto end. */
        clGenGotoCode(Compiler, CodeGenerator, gcvFALSE, endLabel);

        /* For CLK_ADDRESS_CLAMP. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Clamp coord.x to 0. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     &iOperand[2],
                                     &constROperand0));

        /* For coord.x less than 0. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For coord.x greater than or equal to width. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Check if coord.y is greater than or equal to height. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[3],
                       clvCONDITION_GREATER_THAN_EQUAL,
                       &rOperand[1]);

        /* If CLK_ADDRESS_CLAMP, return border color. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[4],
                       clvCONDITION_EQUAL,
                       &constROperand2);

        /* Return border color (all 0.0). */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     IOperand,
                                     &constROperand0));

        /* Done - goto end. */
        clGenGotoCode(Compiler, CodeGenerator, gcvFALSE, endLabel);

        /* For CLK_ADDRESS_CLAMP. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Clamp to edge (height - 1). */
        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_SUB,
                                           &iOperand[3],
                                           &rOperand[1],
                                           &constROperand1));

        /* For else part of coord.y greater than or equal to height. */
        clmGEN_CODE_ELSE(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        /* Check if coord.y is less than 0. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[3],
                       clvCONDITION_LESS_THAN,
                       &constROperand0);

        /* If CLK_ADDRESS_CLAMP, return border color. */
        clmGEN_CODE_IF(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       &rOperand[4],
                       clvCONDITION_EQUAL,
                       &constROperand2);

        /* Return border color (all 0.0). */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     IOperand,
                                     &constROperand0));

        /* Done - goto end. */
        clGenGotoCode(Compiler, CodeGenerator, gcvFALSE, endLabel);

        /* For CLK_ADDRESS_CLAMP. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* Clamp coord.y to 0. */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ASSIGN,
                                     &iOperand[3],
                                     &constROperand0));

        /* For coord.y less than 0. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);

        /* For coord.y greater than or equal to height. */
        clmGEN_CODE_ENDIF(Compiler,
                          CodeGenerator,
                          PolynaryExpr->exprBase.base.lineNo,
                          PolynaryExpr->exprBase.base.stringNo);
    }

    /* Get rowPitch. */
    clsROPERAND_InitializeIntOrIVecConstant(&constROperand24,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 24);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iOperand[1],
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand24));

    /* Calculate offset of rows (coord y). */
    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iOperand[0],
                                       &rOperand[1],
                                       &rOperand[3]));

    /* Get image physical address. */
    clsROPERAND_InitializeIntOrIVecConstant(&constROperand36,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 36);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iOperand[4],
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand36));

    /* Get image channel data type. */
    clsROPERAND_InitializeIntOrIVecConstant(&constROperand12,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 12);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iOperand[1],
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand12));

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandUINT8,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0x10DA /* CLK_UNSIGNED_INT8 */);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandUINT16,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0x10DB /* CLK_UNSIGNED_INT16 */);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandUINT32,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0x10DC /* CLK_UNSIGNED_INT32 */);

    /* Check if type is CLK_UNSIGNED_INT8. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandUINT8);

    /* Element size is 4. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[3],
                                  &rOperand[2],
                                  &constROperand2));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperand[1],
                                       &rOperand[0],
                                       &rOperand[3]));

    /* Load color in format of CLK_UNSIGNED_INT8. */
    clsIOPERAND_New(Compiler, iOperandD, clmGenCodeDataType(T_UCHAR4));
    clsROPERAND_InitializeUsingIOperand(rOperandD, iOperandD);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 iOperandD,
                                 &rOperand[4],
                                 &rOperand[1]));

    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_ASSIGN,
                                 IOperand,
                                 rOperandD));

    clmGEN_CODE_ELSE(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);

    /* Check if type is CLK_UNSIGNED_INT16. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandUINT16);

    /* Element size is 8. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[3],
                                  &rOperand[2],
                                  &constROperand3));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperand[1],
                                       &rOperand[0],
                                       &rOperand[3]));

    /* Load color in format of CLK_UNSIGNED_INT16. */
    clsIOPERAND_New(Compiler, iOperandD, clmGenCodeDataType(T_USHORT4));
    clsROPERAND_InitializeUsingIOperand(rOperandD, iOperandD);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 iOperandD,
                                 &rOperand[4],
                                 &rOperand[1]));

    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_ASSIGN,
                                 IOperand,
                                 rOperandD));

    clmGEN_CODE_ELSE(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);

    /* Check if type is CLK_UNSIGNED_INT32. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandUINT32);

    /* Element size is 16. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[3],
                                  &rOperand[2],
                                  &constROperand4));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperand[1],
                                       &rOperand[0],
                                       &rOperand[3]));

    /* Load color in format CLK_UNSIGNED_INT32. */
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 IOperand,
                                 &rOperand[4],
                                 &rOperand[1]));

    /* For CLK_UNSIGNED_INT32. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* For CLK_UNSIGNED_INT16. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* For CLK_UNSIGNED_INT8. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* Create end lable. */
    clGenLabelCode(Compiler, CodeGenerator, gcvFALSE, endLabel);

OnError:
    /* Return the status. */
    return status;
}

static gceSTATUS
_GenReadImageUICode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    clsSAMPLER_TYPES *samplerTypes = gcvNULL;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3 || OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (OperandCount == 3)
    {
        status = _FindImageSampler(Compiler,
                                   PolynaryExpr,
                                   &samplerTypes);
        if (gcmIS_ERROR(status)) return status;
    }

#if (_OCL_USE_INTRINSIC_FOR_IMAGE || _USE_NEW_INSTRUCTION_FOR_IMAGE_RD_SAMPLER)
    return _GenReadImageSamplerCode(Compiler,
                                    CodeGenerator,
                                    PolynaryExpr,
                                    OperandCount,
                                    OperandsParameters,
                                    IOperand);
#else
#if _USE_TEXLD_FOR_IMAGE_SAMPLER

    return _GenTextureCode(Compiler,
                           CodeGenerator,
                           PolynaryExpr,
                           OperandCount,
                           OperandsParameters,
                           samplerTypes,
                           IOperand);
#else
    return _GenSoftReadImageUICode(Compiler,
                                  CodeGenerator,
                                  PolynaryExpr,
                                  OperandsParameters,
                                  IOperand);
#endif
#endif
}

static gceSTATUS
_GenSoftWriteImageFCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    clsROPERAND constROperand2, constROperand3;
    clsROPERAND constROperand4, constROperand12;
    clsROPERAND constROperand24, constROperand36;
    clsROPERAND constROperandHalf;
    clsROPERAND constROperand255, constROperand65535;
#if _SUPPORT_SNORM_FORMATS_
    clsROPERAND constROperand127, constROperand32767;
    clsROPERAND constROperandSNORM8, constROperandSNORM16;
#endif
    clsROPERAND constROperandUNORM8, constROperandUNORM16;
    clsROPERAND constROperandFLOAT16, constROperandFLOAT;
    clsIOPERAND iOperand[5];
    clsROPERAND rOperand[5];
    clsROPERAND rOperandX[1], rOperandY[1];
    clsIOPERAND iFOperand[2];
    clsROPERAND rFOperand[2];
    clsLOPERAND lOperand[1];

    clsIOPERAND_New(Compiler, &iOperand[0], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[0], &iOperand[0]);

    clsIOPERAND_New(Compiler, &iOperand[1], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[1], &iOperand[1]);

    clsIOPERAND_New(Compiler, &iOperand[2], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[2], &iOperand[2]);

    clsIOPERAND_New(Compiler, &iOperand[3], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[3], &iOperand[3]);

    /* Get rowPitch. */
    clsROPERAND_InitializeIntOrIVecConstant(&constROperand24,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 24);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iOperand[1],
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand24));

    /* Get coord x. */
    clmROPERAND_vectorComponent_GET(rOperandX,
                                    &OperandsParameters[1].rOperands[0],
                                    clvCOMPONENT_X);

    /* Get coord y. */
    clmROPERAND_vectorComponent_GET(rOperandY,
                                    &OperandsParameters[1].rOperands[0],
                                    clvCOMPONENT_Y);

    /* Calculate offset of rows (coord y). */
    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iOperand[0],
                                       &rOperand[1],
                                       rOperandY));

    /* Get image physical address. */
    clsROPERAND_InitializeIntOrIVecConstant(&constROperand36,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 36);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iOperand[2],
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand36));

    /* Get image channel data type. */
    clsROPERAND_InitializeIntOrIVecConstant(&constROperand12,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 12);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iOperand[1],
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand12));

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand2,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 2);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand3,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 3);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand4,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 4);

    clsROPERAND_InitializeFloatOrVecOrMatConstant(&constROperandHalf,
                                                  clmGenCodeDataType(T_FLOAT),
                                                  (gctFLOAT) 0.5f);

    clsROPERAND_InitializeFloatOrVecOrMatConstant(&constROperand255,
                                                  clmGenCodeDataType(T_FLOAT),
                                                  (gctFLOAT) 255.0f);

    clsROPERAND_InitializeFloatOrVecOrMatConstant(&constROperand65535,
                                                  clmGenCodeDataType(T_FLOAT),
                                                  (gctFLOAT) 65535.0f);

#if _SUPPORT_SNORM_FORMATS_
    clsROPERAND_InitializeFloatOrVecOrMatConstant(&constROperand127,
                                                  clmGenCodeDataType(T_FLOAT),
                                                  (gctFLOAT) 127.0f);

    clsROPERAND_InitializeFloatOrVecOrMatConstant(&constROperand32767,
                                                  clmGenCodeDataType(T_FLOAT),
                                                  (gctFLOAT) 32767.0f);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandSNORM8,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0x10D0 /* CLK_SNORM_INT8 */);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandSNORM16,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0x10D1 /* CLK_SNORM_INT16 */);
#endif

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandUNORM8,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0x10D2 /* CLK_UNORM_INT8 */);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandUNORM16,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0x10D3 /* CLK_UNORM_INT16 */);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandFLOAT16,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0x10DD /* CLK_HALF_FLOAT */);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandFLOAT,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0x10DE /* CLK_FLOAT */);

    clsIOPERAND_New(Compiler, &iFOperand[0], clmGenCodeDataType(T_FLOAT4));
    clsROPERAND_InitializeUsingIOperand(&rFOperand[0], &iFOperand[0]);

    clsIOPERAND_New(Compiler, &iFOperand[1], clmGenCodeDataType(T_FLOAT4));
    clsROPERAND_InitializeUsingIOperand(&rFOperand[1], &iFOperand[1]);

    /* Check if type is CLK_UNORM_INT8. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandUNORM8);

    /* Element size is 4. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[3],
                                  rOperandX,
                                  &constROperand2));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperand[1],
                                       &rOperand[0],
                                       &rOperand[3]));

    /* Convert color to UNORM_INT8. */
    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_SATURATE,
                                 &iFOperand[1],
                                 &OperandsParameters[2].rOperands[0]));
    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iFOperand[0],
                                       &rFOperand[1],
                                       &constROperand255));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iFOperand[1],
                                       &rFOperand[0],
                                       &constROperandHalf));

    /* Store data as T_UCHAR4. */
    clsIOPERAND_New(Compiler, &iOperand[4], clmGenCodeDataType(T_UCHAR4));
    clsROPERAND_InitializeUsingIOperand(&rOperand[4], &iOperand[4]);

    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_FLOAT_TO_UINT,
                                 &iOperand[4],
                                 &rFOperand[1]));

    clsLOPERAND_InitializeUsingROperand(lOperand, &rOperand[2]);
    gcmONERROR(clGenStoreCode(Compiler,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo,
                              clvOPCODE_STORE1,
                              &rOperand[4],
                              lOperand,
                              clmGenCodeDataType(T_UCHAR),
                              &rOperand[1]));

    clmGEN_CODE_ELSE(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);

    /* Check if type is CLK_UNORM_INT16. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandUNORM16);

    /* Element size is 8. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[3],
                                  rOperandX,
                                  &constROperand3));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperand[1],
                                       &rOperand[0],
                                       &rOperand[3]));

    /* Convert color to UNORM_INT16. */
    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_SATURATE,
                                 &iFOperand[1],
                                 &OperandsParameters[2].rOperands[0]));
    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iFOperand[0],
                                       &rFOperand[1],
                                       &constROperand65535));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iFOperand[1],
                                       &rFOperand[0],
                                       &constROperandHalf));

    /* Store data as T_USHORT4. */
    clsIOPERAND_New(Compiler, &iOperand[4], clmGenCodeDataType(T_USHORT4));
    clsROPERAND_InitializeUsingIOperand(&rOperand[4], &iOperand[4]);

    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_FLOAT_TO_UINT,
                                 &iOperand[4],
                                 &rFOperand[1]));

    clsLOPERAND_InitializeUsingROperand(lOperand, &rOperand[2]);
    gcmONERROR(clGenStoreCode(Compiler,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo,
                              clvOPCODE_STORE1,
                              &rOperand[4],
                              lOperand,
                              clmGenCodeDataType(T_USHORT),
                              &rOperand[1]));

    clmGEN_CODE_ELSE(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);

#if _SUPPORT_SNORM_FORMATS_
    /* Check if type is CLK_SNORM_INT8. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandSNORM8);

    /* Element size is 4. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[3],
                                  rOperandX,
                                  &constROperand2));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperand[1],
                                       &rOperand[0],
                                       &rOperand[3]));

    /* Convert color to SNORM_INT8. */
    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_SATURATE,
                                 &iFOperand[1],
                                 &OperandsParameters[2].rOperands[0]));
    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iFOperand[0],
                                       &rFOperand[1],
                                       &constROperand127));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iFOperand[1],
                                       &rFOperand[0],
                                       &constROperandHalf));

    /* Store data as T_CHAR4. */
    clsIOPERAND_New(Compiler, &iOperand[4], clmGenCodeDataType(T_CHAR4));
    clsROPERAND_InitializeUsingIOperand(&rOperand[4], &iOperand[4]);

    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_FLOAT_TO_INT,
                                 &iOperand[4],
                                 &rFOperand[1]));

    clsLOPERAND_InitializeUsingROperand(lOperand, &rOperand[2]);
    gcmONERROR(clGenStoreCode(Compiler,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo,
                              clvOPCODE_STORE1,
                              &rOperand[4],
                              lOperand,
                              clmGenCodeDataType(T_CHAR),
                              &rOperand[1]));

    clmGEN_CODE_ELSE(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);

    /* Check if type is CLK_SNORM_INT16. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandSNORM16);

    /* Element size is 8. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[3],
                                  rOperandX,
                                  &constROperand3));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperand[1],
                                       &rOperand[0],
                                       &rOperand[3]));

    /* Convert color to SNORM_INT16. */
    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_SATURATE,
                                 &iFOperand[1],
                                 &OperandsParameters[2].rOperands[0]));
    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iFOperand[0],
                                       &rFOperand[1],
                                       &constROperand32767));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iFOperand[1],
                                       &rFOperand[0],
                                       &constROperandHalf));

    /* Store data as T_SHORT4. */
    clsIOPERAND_New(Compiler, &iOperand[4], clmGenCodeDataType(T_SHORT4));
    clsROPERAND_InitializeUsingIOperand(&rOperand[4], &iOperand[4]);

    gcmONERROR(clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_FLOAT_TO_INT,
                                 &iOperand[4],
                                 &rFOperand[1]));

    clsLOPERAND_InitializeUsingROperand(lOperand, &rOperand[2]);
    gcmONERROR(clGenStoreCode(Compiler,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo,
                              clvOPCODE_STORE1,
                              &rOperand[4],
                              lOperand,
                              clmGenCodeDataType(T_SHORT),
                              &rOperand[1]));

    clmGEN_CODE_ELSE(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);
#endif

    /* Check if type is CLK_HALF_FLOAT. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandFLOAT16);

    /* Element size is 8. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[3],
                                  rOperandX,
                                  &constROperand3));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperand[1],
                                       &rOperand[0],
                                       &rOperand[3]));

    /* Store data as T_HALF4. */
    clsLOPERAND_InitializeUsingROperand(lOperand, &rOperand[2]);
    gcmASSERT(clmGEN_CODE_elementType_GET(OperandsParameters[2].rOperands[0].dataType) == clvTYPE_FLOAT);
    clmGEN_CODE_elementType_SET(OperandsParameters[2].rOperands[0].dataType, clvTYPE_HALF);
    gcmONERROR(clGenStoreCode(Compiler,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo,
                              clvOPCODE_STORE1,
                              &OperandsParameters[2].rOperands[0],
                              lOperand,
                              clmGenCodeDataType(T_HALF),
                              &rOperand[1]));
    clmGEN_CODE_elementType_SET(OperandsParameters[2].rOperands[0].dataType, clvTYPE_FLOAT);

    clmGEN_CODE_ELSE(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);

    /* Check if type is CLK_FLOAT. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandFLOAT);

    /* Element size is 16. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[3],
                                  rOperandX,
                                  &constROperand4));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperand[1],
                                       &rOperand[0],
                                       &rOperand[3]));

    /* Store color as float. */
    clsLOPERAND_InitializeUsingROperand(lOperand, &rOperand[2]);
    gcmONERROR(clGenStoreCode(Compiler,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo,
                              clvOPCODE_STORE1,
                              &OperandsParameters[2].rOperands[0],
                              lOperand,
                              clmGenCodeDataType(T_FLOAT),
                              &rOperand[1]));

    /* For CLK_FLOAT. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* For CLK_HALF_FLOAT. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

#if _SUPPORT_SNORM_FORMATS_
    /* For SNORM_INT16. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* For SNORM_INT8. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);
#endif

    /* For UNORM_INT16. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* For UNORM_INT8. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

OnError:
    return status;
}

static gceSTATUS
_GenWriteImageFCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    cloIR_EXPR imageOperand = gcvNULL;
    clsNAME *image;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand == gcvNULL);

    /* Try to get the image argument */
    imageOperand = (cloIR_EXPR)PolynaryExpr->operands->members.next;

    gcmASSERT(imageOperand);
    gcmASSERT(cloIR_OBJECT_GetType(&imageOperand->base) == clvIR_VARIABLE);
    image = ((cloIR_VARIABLE) &imageOperand->base)->name;
    gcmASSERT(image->type == clvPARAMETER_NAME);
    if(image->decl.dataType->accessQualifier == clvQUALIFIER_READ_ONLY)
    {
        cloCOMPILER_Report(Compiler,
                           PolynaryExpr->exprBase.base.lineNo,
                           PolynaryExpr->exprBase.base.stringNo,
                           clvREPORT_ERROR,
                           "image \"%s\" has READ_ONLY access",
                           image->symbol);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

#if (_OCL_USE_INTRINSIC_FOR_IMAGE || _USE_NEW_INSTRUCTION_FOR_IMAGE_WR_SAMPLER)

    return _GenWriteImageCode(Compiler,
                              CodeGenerator,
                              PolynaryExpr,
                              OperandCount,
                              OperandsParameters);
#else
    return _GenSoftWriteImageFCode(Compiler,
                                   CodeGenerator,
                                   PolynaryExpr,
                                   OperandsParameters);
#endif
}

static gceSTATUS
_GenSoftWriteImageICode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    clsROPERAND constROperand2, constROperand3;
    clsROPERAND constROperand4, constROperand12;
    clsROPERAND constROperand24, constROperand36;
    clsROPERAND constROperand127, constROperand32767;
    clsROPERAND constROperandMinus128, constROperandMinus32768;
    clsROPERAND constROperandINT8, constROperandINT16, constROperandINT32;
    clsIOPERAND iOperand[6];
    clsROPERAND rOperand[6];
    clsROPERAND rOperandX[1], rOperandY[1];
    clsLOPERAND lOperand[1];
    clsIOPERAND_New(Compiler, &iOperand[0], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[0], &iOperand[0]);

    clsIOPERAND_New(Compiler, &iOperand[1], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[1], &iOperand[1]);

    clsIOPERAND_New(Compiler, &iOperand[2], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[2], &iOperand[2]);

    clsIOPERAND_New(Compiler, &iOperand[3], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[3], &iOperand[3]);

    /* Get rowPitch. */
    clsROPERAND_InitializeIntOrIVecConstant(&constROperand24,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 24);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iOperand[1],
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand24));

    /* Get coord x. */
    clmROPERAND_vectorComponent_GET(rOperandX,
                                    &OperandsParameters[1].rOperands[0],
                                    clvCOMPONENT_X);

    /* Get coord y. */
    clmROPERAND_vectorComponent_GET(rOperandY,
                                    &OperandsParameters[1].rOperands[0],
                                    clvCOMPONENT_Y);

    /* Calculate offset of rows (coord y). */
    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iOperand[0],
                                       &rOperand[1],
                                       rOperandY));

    /* Get image physical address. */
    clsROPERAND_InitializeIntOrIVecConstant(&constROperand36,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 36);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iOperand[2],
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand36));

    /* Get image channel data type. */
    clsROPERAND_InitializeIntOrIVecConstant(&constROperand12,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 12);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iOperand[1],
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand12));

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand2,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 2);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand3,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 3);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand4,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 4);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand127,
                                            clmGenCodeDataType(T_INT),
                                            (gctINT) 127);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand32767,
                                            clmGenCodeDataType(T_INT),
                                            (gctINT) 32767);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandMinus128,
                                            clmGenCodeDataType(T_INT),
                                            (gctINT) -128);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandMinus32768,
                                            clmGenCodeDataType(T_INT),
                                            (gctINT) -32768);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandINT8,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0x10D7 /* CLK_SIGNED_INT8 */);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandINT16,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0x10D8 /* CLK_SIGNED_INT16 */);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandINT32,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0x10D9 /* CLK_SIGNED_INT32 */);

    /* Check if type is CLK_SIGNED_INT8. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandINT8);

    /* Element size is 4. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[3],
                                  rOperandX,
                                  &constROperand2));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperand[1],
                                       &rOperand[0],
                                       &rOperand[3]));

    /* Store color as CLK_SIGNED_INT8. */
    clsIOPERAND_New(Compiler, &iOperand[4], clmGenCodeDataType(T_CHAR4));
    clsROPERAND_InitializeUsingIOperand(&rOperand[4], &iOperand[4]);

    clsIOPERAND_New(Compiler, &iOperand[5], clmGenCodeDataType(T_CHAR4));
    clsROPERAND_InitializeUsingIOperand(&rOperand[5], &iOperand[5]);

    /* Saturate the data. */
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_MIN,
                                 &iOperand[4],
                                 &OperandsParameters[2].rOperands[0],
                                 &constROperand127));
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_MAX,
                                 &iOperand[5],
                                 &rOperand[4],
                                 &constROperandMinus128));

    clsLOPERAND_InitializeUsingROperand(lOperand, &rOperand[2]);
    gcmASSERT(clmGEN_CODE_elementType_GET(OperandsParameters[2].rOperands[0].dataType) == clvTYPE_INT);
    clmGEN_CODE_elementType_SET(OperandsParameters[2].rOperands[0].dataType, clvTYPE_CHAR);
    gcmONERROR(clGenStoreCode(Compiler,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo,
                              clvOPCODE_STORE1,
                              &rOperand[5],
                              lOperand,
                              clmGenCodeDataType(T_CHAR),
                              &rOperand[1]));
    clmGEN_CODE_elementType_SET(OperandsParameters[2].rOperands[0].dataType, clvTYPE_INT);

    clmGEN_CODE_ELSE(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);

    /* Check if type is CLK_SIGNED_INT16. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandINT16);

    /* Element size is 8. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[3],
                                  rOperandX,
                                  &constROperand3));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperand[1],
                                       &rOperand[0],
                                       &rOperand[3]));

    /* Store color as CLK_SIGNED_INT16. */
    clsIOPERAND_New(Compiler, &iOperand[4], clmGenCodeDataType(T_SHORT4));
    clsROPERAND_InitializeUsingIOperand(&rOperand[4], &iOperand[4]);

    clsIOPERAND_New(Compiler, &iOperand[5], clmGenCodeDataType(T_SHORT4));
    clsROPERAND_InitializeUsingIOperand(&rOperand[5], &iOperand[5]);

    /* Saturate the data. */
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_MIN,
                                 &iOperand[4],
                                 &OperandsParameters[2].rOperands[0],
                                 &constROperand32767));
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_MAX,
                                 &iOperand[5],
                                 &rOperand[4],
                                 &constROperandMinus32768));

    clsLOPERAND_InitializeUsingROperand(lOperand, &rOperand[2]);
    gcmASSERT(clmGEN_CODE_elementType_GET(OperandsParameters[2].rOperands[0].dataType) == clvTYPE_INT);
    clmGEN_CODE_elementType_SET(OperandsParameters[2].rOperands[0].dataType, clvTYPE_SHORT);
    gcmONERROR(clGenStoreCode(Compiler,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo,
                              clvOPCODE_STORE1,
                              &rOperand[5],
                              lOperand,
                              clmGenCodeDataType(T_SHORT),
                              &rOperand[1]));
    clmGEN_CODE_elementType_SET(OperandsParameters[2].rOperands[0].dataType, clvTYPE_INT);

    clmGEN_CODE_ELSE(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);

    /* Check if type is CLK_SIGNED_INT32. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandINT32);

    /* Element size is 16. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[3],
                                  rOperandX,
                                  &constROperand4));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperand[1],
                                       &rOperand[0],
                                       &rOperand[3]));

    /* Store color as CLK_SIGNED_INT32. */
    clsLOPERAND_InitializeUsingROperand(lOperand, &rOperand[2]);
    gcmONERROR(clGenStoreCode(Compiler,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo,
                              clvOPCODE_STORE1,
                              &OperandsParameters[2].rOperands[0],
                              lOperand,
                              clmGenCodeDataType(T_INT),
                              &rOperand[1]));

    /* For CLK_SIGNED_INT32. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* For CLK_SIGNED_INT16. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* For CLK_SIGNED_INT8. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

OnError:
    return status;
}

static gceSTATUS
_GenWriteImageICode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    cloIR_EXPR imageOperand = gcvNULL;
    clsNAME *image;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand == gcvNULL);

    /* Try to get the image argument */
    imageOperand = (cloIR_EXPR)PolynaryExpr->operands->members.next;

    gcmASSERT(imageOperand);
    gcmASSERT(cloIR_OBJECT_GetType(&imageOperand->base) == clvIR_VARIABLE);
    image = ((cloIR_VARIABLE) &imageOperand->base)->name;
    gcmASSERT(image->type == clvPARAMETER_NAME);
    if(image->decl.dataType->accessQualifier == clvQUALIFIER_READ_ONLY)
    {
        cloCOMPILER_Report(Compiler,
                           PolynaryExpr->exprBase.base.lineNo,
                           PolynaryExpr->exprBase.base.stringNo,
                           clvREPORT_ERROR,
                           "image \"%s\" has READ_ONLY access",
                           image->symbol);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

#if (_OCL_USE_INTRINSIC_FOR_IMAGE || _USE_NEW_INSTRUCTION_FOR_IMAGE_WR_SAMPLER)
    return _GenWriteImageCode(Compiler,
                              CodeGenerator,
                              PolynaryExpr,
                              OperandCount,
                              OperandsParameters);
#else
    return _GenSoftWriteImageICode(Compiler,
                                   CodeGenerator,
                                   PolynaryExpr,
                                   OperandsParameters);
#endif
}

static gceSTATUS
_GenSoftWriteImageUICode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    clsROPERAND constROperand2, constROperand3;
    clsROPERAND constROperand4, constROperand12;
    clsROPERAND constROperand24, constROperand36;
    clsROPERAND constROperand255, constROperand65535;
    clsROPERAND constROperandUINT8, constROperandUINT16, constROperandUINT32;
    clsIOPERAND iOperand[5];
    clsROPERAND rOperand[5];
    clsROPERAND rOperandX[1], rOperandY[1];
    clsLOPERAND lOperand[1];
    clsIOPERAND_New(Compiler, &iOperand[0], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[0], &iOperand[0]);

    clsIOPERAND_New(Compiler, &iOperand[1], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[1], &iOperand[1]);

    clsIOPERAND_New(Compiler, &iOperand[2], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[2], &iOperand[2]);

    clsIOPERAND_New(Compiler, &iOperand[3], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&rOperand[3], &iOperand[3]);

    /* Get rowPitch. */
    clsROPERAND_InitializeIntOrIVecConstant(&constROperand24,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 24);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iOperand[1],
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand24));

    /* Get coord x. */
    clmROPERAND_vectorComponent_GET(rOperandX,
                                    &OperandsParameters[1].rOperands[0],
                                    clvCOMPONENT_X);

    /* Get coord y. */
    clmROPERAND_vectorComponent_GET(rOperandY,
                                    &OperandsParameters[1].rOperands[0],
                                    clvCOMPONENT_Y);

    /* Calculate offset of rows (coord y). */
    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_MUL,
                                       &iOperand[0],
                                       &rOperand[1],
                                       rOperandY));

    /* Get image physical address. */
    clsROPERAND_InitializeIntOrIVecConstant(&constROperand36,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 36);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iOperand[2],
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand36));

    /* Get image channel data type. */
    clsROPERAND_InitializeIntOrIVecConstant(&constROperand12,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 12);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &iOperand[1],
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand12));

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand2,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 2);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand3,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 3);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand4,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 4);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand255,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 255);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperand65535,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 65535);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandUINT8,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0x10DA /* CLK_UNSIGNED_INT8 */);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandUINT16,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0x10DB /* CLK_UNSIGNED_INT16 */);

    clsROPERAND_InitializeIntOrIVecConstant(&constROperandUINT32,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0x10DC /* CLK_UNSIGNED_INT32 */);

    /* Check if type is CLK_UNSIGNED_INT8. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandUINT8);

    /* Element size is 4. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[3],
                                  rOperandX,
                                  &constROperand2));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperand[1],
                                       &rOperand[0],
                                       &rOperand[3]));

    /* Store color as CLK_UNSIGNED_INT8. */
    clsIOPERAND_New(Compiler, &iOperand[4], clmGenCodeDataType(T_UCHAR4));
    clsROPERAND_InitializeUsingIOperand(&rOperand[4], &iOperand[4]);

    /* Saturate the data. */
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_MIN,
                                 &iOperand[4],
                                 &OperandsParameters[2].rOperands[0],
                                 &constROperand255));

    clsLOPERAND_InitializeUsingROperand(lOperand, &rOperand[2]);
    gcmASSERT(clmGEN_CODE_elementType_GET(OperandsParameters[2].rOperands[0].dataType) == clvTYPE_UINT);
    clmGEN_CODE_elementType_SET(OperandsParameters[2].rOperands[0].dataType, clvTYPE_UCHAR);
    gcmONERROR(clGenStoreCode(Compiler,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo,
                              clvOPCODE_STORE1,
                              &rOperand[4],
                              lOperand,
                              clmGenCodeDataType(T_UCHAR),
                              &rOperand[1]));
    clmGEN_CODE_elementType_SET(OperandsParameters[2].rOperands[0].dataType, clvTYPE_UINT);

    clmGEN_CODE_ELSE(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);

    /* Check if type is CLK_UNSIGNED_INT16. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandUINT16);

    /* Element size is 8. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[3],
                                  rOperandX,
                                  &constROperand3));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperand[1],
                                       &rOperand[0],
                                       &rOperand[3]));

    /* Store color as CLK_UNSIGNED_INT16. */
    clsIOPERAND_New(Compiler, &iOperand[4], clmGenCodeDataType(T_USHORT4));
    clsROPERAND_InitializeUsingIOperand(&rOperand[4], &iOperand[4]);

    /* Saturate the data. */
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_MIN,
                                 &iOperand[4],
                                 &OperandsParameters[2].rOperands[0],
                                 &constROperand65535));

    clsLOPERAND_InitializeUsingROperand(lOperand, &rOperand[2]);
    gcmASSERT(clmGEN_CODE_elementType_GET(OperandsParameters[2].rOperands[0].dataType) == clvTYPE_UINT);
    clmGEN_CODE_elementType_SET(OperandsParameters[2].rOperands[0].dataType, clvTYPE_USHORT);
    gcmONERROR(clGenStoreCode(Compiler,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo,
                              clvOPCODE_STORE1,
                              &rOperand[4],
                              lOperand,
                              clmGenCodeDataType(T_USHORT),
                              &rOperand[1]));
    clmGEN_CODE_elementType_SET(OperandsParameters[2].rOperands[0].dataType, clvTYPE_UINT);

    clmGEN_CODE_ELSE(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);

    /* Check if type is CLK_UNSIGNED_INT32. */
    clmGEN_CODE_IF(Compiler,
                   CodeGenerator,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   &rOperand[1],
                   clvCONDITION_EQUAL,
                   &constROperandUINT32);

    /* Element size is 16. */
    gcmONERROR(clGenShiftExprCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LSHIFT,
                                  &iOperand[3],
                                  rOperandX,
                                  &constROperand4));

    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &iOperand[1],
                                       &rOperand[0],
                                       &rOperand[3]));

    /* Store color as CLK_UNSIGNED_INT32. */
    clsLOPERAND_InitializeUsingROperand(lOperand, &rOperand[2]);
    gcmONERROR(clGenStoreCode(Compiler,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo,
                              clvOPCODE_STORE1,
                              &OperandsParameters[2].rOperands[0],
                              lOperand,
                              clmGenCodeDataType(T_UINT),
                              &rOperand[1]));

    /* For CLK_UNSIGNED_INT32. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* For CLK_UNSIGNED_INT16. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

    /* For CLK_UNSIGNED_INT8. */
    clmGEN_CODE_ENDIF(Compiler,
                      CodeGenerator,
                      PolynaryExpr->exprBase.base.lineNo,
                      PolynaryExpr->exprBase.base.stringNo);

OnError:
    return status;
}

static gceSTATUS
_GenWriteImageUICode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    cloIR_EXPR imageOperand = gcvNULL;
    clsNAME *image;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand == gcvNULL);

    /* Try to get the image argument */
    imageOperand = (cloIR_EXPR)PolynaryExpr->operands->members.next;

    gcmASSERT(imageOperand);
    gcmASSERT(cloIR_OBJECT_GetType(&imageOperand->base) == clvIR_VARIABLE);
    image = ((cloIR_VARIABLE) &imageOperand->base)->name;
    gcmASSERT(image->type == clvPARAMETER_NAME);
    if(image->decl.dataType->accessQualifier == clvQUALIFIER_READ_ONLY)
    {
        cloCOMPILER_Report(Compiler,
                           PolynaryExpr->exprBase.base.lineNo,
                           PolynaryExpr->exprBase.base.stringNo,
                           clvREPORT_ERROR,
                           "image \"%s\" has READ_ONLY access",
                           image->symbol);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

#if (_OCL_USE_INTRINSIC_FOR_IMAGE || _USE_NEW_INSTRUCTION_FOR_IMAGE_WR_SAMPLER)

    return _GenWriteImageCode(Compiler,
                              CodeGenerator,
                              PolynaryExpr,
                              OperandCount,
                              OperandsParameters);
#else
    return _GenSoftWriteImageUICode(Compiler,
                                    CodeGenerator,
                                    PolynaryExpr,
                                    OperandsParameters);
#endif
}

static gceSTATUS
_GenVivReadImageFCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    clsSAMPLER_TYPES *samplerTypes = gcvNULL;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = _FindImageSampler(Compiler,
                               PolynaryExpr,
                               &samplerTypes);
    if (gcmIS_ERROR(status)) return status;

    return _GenSoftReadImageFCode(Compiler,
                                  CodeGenerator,
                                  PolynaryExpr,
                                  OperandsParameters,
                                  IOperand);
}

static gceSTATUS
_GenVivReadImageICode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    clsSAMPLER_TYPES *samplerTypes = gcvNULL;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = _FindImageSampler(Compiler,
                               PolynaryExpr,
                               &samplerTypes);
    if (gcmIS_ERROR(status)) return status;

    return _GenSoftReadImageICode(Compiler,
                                  CodeGenerator,
                                  PolynaryExpr,
                                  OperandsParameters,
                                  IOperand);
}

static gceSTATUS
_GenVivReadImageUICode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    clsSAMPLER_TYPES *samplerTypes = gcvNULL;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = _FindImageSampler(Compiler,
                               PolynaryExpr,
                               &samplerTypes);
    if (gcmIS_ERROR(status)) return status;

    return _GenSoftReadImageUICode(Compiler,
                                  CodeGenerator,
                                  PolynaryExpr,
                                  OperandsParameters,
                                  IOperand);
}

static gceSTATUS
_GenVivWriteImageFCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    cloIR_EXPR imageOperand = gcvNULL;
    clsNAME *image;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand == gcvNULL);

    /* Try to get the image argument */
    imageOperand = (cloIR_EXPR)PolynaryExpr->operands->members.next;

    gcmASSERT(imageOperand);
    gcmASSERT(cloIR_OBJECT_GetType(&imageOperand->base) == clvIR_VARIABLE);
    image = ((cloIR_VARIABLE) &imageOperand->base)->name;
    gcmASSERT(image->type == clvPARAMETER_NAME);
    if(image->decl.dataType->accessQualifier == clvQUALIFIER_READ_ONLY)
    {
        cloCOMPILER_Report(Compiler,
                           PolynaryExpr->exprBase.base.lineNo,
                           PolynaryExpr->exprBase.base.stringNo,
                           clvREPORT_ERROR,
                           "image \"%s\" has READ_ONLY access",
                           image->symbol);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    return _GenSoftWriteImageFCode(Compiler,
                                   CodeGenerator,
                                   PolynaryExpr,
                                   OperandsParameters);
}

static gceSTATUS
_GenVivWriteImageICode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    cloIR_EXPR imageOperand = gcvNULL;
    clsNAME *image;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand == gcvNULL);

    /* Try to get the image argument */
    imageOperand = (cloIR_EXPR)PolynaryExpr->operands->members.next;

    gcmASSERT(imageOperand);
    gcmASSERT(cloIR_OBJECT_GetType(&imageOperand->base) == clvIR_VARIABLE);
    image = ((cloIR_VARIABLE) &imageOperand->base)->name;
    gcmASSERT(image->type == clvPARAMETER_NAME);
    if(image->decl.dataType->accessQualifier == clvQUALIFIER_READ_ONLY)
    {
        cloCOMPILER_Report(Compiler,
                           PolynaryExpr->exprBase.base.lineNo,
                           PolynaryExpr->exprBase.base.stringNo,
                           clvREPORT_ERROR,
                           "image \"%s\" has READ_ONLY access",
                           image->symbol);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    return _GenSoftWriteImageICode(Compiler,
                                   CodeGenerator,
                                   PolynaryExpr,
                                   OperandsParameters);
}

static gceSTATUS
_GenVivWriteImageUICode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    cloIR_EXPR imageOperand = gcvNULL;
    clsNAME *image;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand == gcvNULL);

    /* Try to get the image argument */
    imageOperand = (cloIR_EXPR)PolynaryExpr->operands->members.next;

    gcmASSERT(imageOperand);
    gcmASSERT(cloIR_OBJECT_GetType(&imageOperand->base) == clvIR_VARIABLE);
    image = ((cloIR_VARIABLE) &imageOperand->base)->name;
    gcmASSERT(image->type == clvPARAMETER_NAME);
    if(image->decl.dataType->accessQualifier == clvQUALIFIER_READ_ONLY)
    {
        cloCOMPILER_Report(Compiler,
                           PolynaryExpr->exprBase.base.lineNo,
                           PolynaryExpr->exprBase.base.stringNo,
                           clvREPORT_ERROR,
                           "image \"%s\" has READ_ONLY access",
                           image->symbol);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    return _GenSoftWriteImageUICode(Compiler,
                                    CodeGenerator,
                                    PolynaryExpr,
                                    OperandsParameters);
}

static gceSTATUS
_GenQueryImageCallCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctCONST_STRING Query,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    cloIR_POLYNARY_EXPR  funcCall = gcvNULL;
    gctCHAR   nameBuf[64];
    gctSTRING funcNameString = nameBuf;
    gctUINT offset = 0;
    cloIR_UNARY_EXPR nullExpr = gcvNULL;
    clsDECL decl[1];
    cltELEMENT_TYPE elementType;
    clsGEN_CODE_PARAMETERS parameters[1];

    elementType = clmGEN_CODE_elementType_GET(OperandsParameters[0].dataTypes[0].def);

    gcmASSERT(clmIsElementTypeImage(elementType));

    clsGEN_CODE_PARAMETERS_Initialize(parameters,
                                      gcvFALSE,
                                      gcvTRUE);
    gcmVERIFY_OK(gcoOS_PrintStrSafe(funcNameString,
                                    64,
                                    &offset,
                                    "_viv_image_query_%s_%s",
                                    Query,
                                    clGetElementTypeName(elementType)));
    funcCall = clCreateFuncCallByName(Compiler,
                                      PolynaryExpr->exprBase.base.lineNo,
                                      PolynaryExpr->exprBase.base.stringNo,
                                      funcNameString,
                                      &PolynaryExpr->exprBase);
    if(!funcCall) {
      status = gcvSTATUS_INVALID_ARGUMENT;
      gcmONERROR(status);
    }

    gcmASSERT(funcCall->operands);
    gcmONERROR(cloCOMPILER_CreateDecl(Compiler,
                                      T_UINT8,
                                      gcvNULL,
                                      clvQUALIFIER_NONE,
                                      clvQUALIFIER_NONE,
                                      decl));

    gcmONERROR(cloIR_NULL_EXPR_Construct(Compiler,
                                         PolynaryExpr->exprBase.base.lineNo,
                                         PolynaryExpr->exprBase.base.stringNo,
                                         decl,
                                         &nullExpr));
    gcmVERIFY_OK(cloIR_SET_AddMember(Compiler,
                                     funcCall->operands,
                                     &nullExpr->exprBase.base));

    gcmONERROR(cloCOMPILER_BindFuncCall(Compiler,
                                        funcCall));

    clmGEN_CODE_SetParametersIOperand(Compiler,
                                      parameters,
                                      0,
                                      IOperand,
                                      IOperand->componentSelection.selection[0]);
    /* Allocate the function resources */
    gcmONERROR(clAllocateFuncResources(Compiler,
                                       CodeGenerator,
                                       funcCall->funcName));

    gcmONERROR(clGenFuncCallCode(Compiler,
                                 CodeGenerator,
                                 funcCall,
                                 OperandsParameters,
                                 parameters));
    gcmONERROR(cloCOMPILER_SetHasImageQuery(Compiler));

OnError:
    clsGEN_CODE_PARAMETERS_Finalize(parameters);
    return status;
}

static gceSTATUS
_GenGetImageWidthCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    clsROPERAND constROperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (_OCL_USE_INTRINSIC_FOR_IMAGE && !cloCOMPILER_IsGcslDriverImage(Compiler)) {
        clsLOPERAND lOperand[1];

        clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
        return  clGenIntrinsicAsmCode(Compiler,
                                      PolynaryExpr->exprBase.base.lineNo,
                                      PolynaryExpr->exprBase.base.stringNo,
                                      CL_VIR_IK_image_query_width,
                                      lOperand,
                                      OperandCount,
                                      &OperandsParameters[0].rOperands[0]);
    }

    if (cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX) ||
        gcmOPT_oclUseImgIntrinsicQuery())
    {
        return _GenQueryImageCallCode(Compiler,
                                      CodeGenerator,
                                      PolynaryExpr,
                                      "width",
                                      OperandCount,
                                      OperandsParameters,
                                      IOperand);
    }
    clsROPERAND_InitializeIntOrIVecConstant(&constROperand,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 IOperand,
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand));

    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    return status;
}

static gceSTATUS
_GenGetImageHeightCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    clsROPERAND constROperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (_OCL_USE_INTRINSIC_FOR_IMAGE && !cloCOMPILER_IsGcslDriverImage(Compiler)) {
        clsLOPERAND lOperand[1];

        clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
        return  clGenIntrinsicAsmCode(Compiler,
                                      PolynaryExpr->exprBase.base.lineNo,
                                      PolynaryExpr->exprBase.base.stringNo,
                                      CL_VIR_IK_image_query_height,
                                      lOperand,
                                      OperandCount,
                                      &OperandsParameters[0].rOperands[0]);
    }
    if (cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX) ||
        gcmOPT_oclUseImgIntrinsicQuery())
    {
        return _GenQueryImageCallCode(Compiler,
                                      CodeGenerator,
                                      PolynaryExpr,
                                      "height",
                                      OperandCount,
                                      OperandsParameters,
                                      IOperand);
    }
    clsROPERAND_InitializeIntOrIVecConstant(&constROperand,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 4);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 IOperand,
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand));

    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    return status;
}

static gceSTATUS
_GenGetImageDepthCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    clsROPERAND constROperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (_OCL_USE_INTRINSIC_FOR_IMAGE && !cloCOMPILER_IsGcslDriverImage(Compiler)) {
        clsLOPERAND lOperand[1];

        clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
        return  clGenIntrinsicAsmCode(Compiler,
                                      PolynaryExpr->exprBase.base.lineNo,
                                      PolynaryExpr->exprBase.base.stringNo,
                                      CL_VIR_IK_image_query_depth,
                                      lOperand,
                                      OperandCount,
                                      &OperandsParameters[0].rOperands[0]);
    }
    clsROPERAND_InitializeIntOrIVecConstant(&constROperand,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 8);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 IOperand,
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand));

    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    return status;
}

static gceSTATUS
_GenGetImageChannelDataTypeCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    clsROPERAND constROperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (_OCL_USE_INTRINSIC_FOR_IMAGE && !cloCOMPILER_IsGcslDriverImage(Compiler)) {
        clsLOPERAND lOperand[1];

        clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
        return  clGenIntrinsicAsmCode(Compiler,
                                      PolynaryExpr->exprBase.base.lineNo,
                                      PolynaryExpr->exprBase.base.stringNo,
                                      CL_VIR_IK_image_query_format,
                                      lOperand,
                                      OperandCount,
                                      &OperandsParameters[0].rOperands[0]);
    }
    if (cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX) ||
        gcmOPT_oclUseImgIntrinsicQuery())
    {
        return _GenQueryImageCallCode(Compiler,
                                      CodeGenerator,
                                      PolynaryExpr,
                                      "format",
                                      OperandCount,
                                      OperandsParameters,
                                      IOperand);
    }
    clsROPERAND_InitializeIntOrIVecConstant(&constROperand,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 12);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 IOperand,
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand));

    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    return status;
}

static gceSTATUS
_GenGetImageChannelOrderCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    clsROPERAND constROperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (_OCL_USE_INTRINSIC_FOR_IMAGE && !cloCOMPILER_IsGcslDriverImage(Compiler)) {
        clsLOPERAND lOperand[1];

        clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
        return  clGenIntrinsicAsmCode(Compiler,
                                      PolynaryExpr->exprBase.base.lineNo,
                                      PolynaryExpr->exprBase.base.stringNo,
                                      CL_VIR_IK_image_query_order,
                                      lOperand,
                                      OperandCount,
                                      &OperandsParameters[0].rOperands[0]);
    }
    if (cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX) ||
        gcmOPT_oclUseImgIntrinsicQuery())
    {
        return _GenQueryImageCallCode(Compiler,
                                      CodeGenerator,
                                      PolynaryExpr,
                                      "order",
                                      OperandCount,
                                      OperandsParameters,
                                      IOperand);
    }
    clsROPERAND_InitializeIntOrIVecConstant(&constROperand,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 16);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 IOperand,
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand));

    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    return status;
}

static gceSTATUS
_GenGetImageDimCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    clsROPERAND constROperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (_OCL_USE_INTRINSIC_FOR_IMAGE && !cloCOMPILER_IsGcslDriverImage(Compiler)) {
        clsLOPERAND lOperand[1];

        clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
        return  clGenIntrinsicAsmCode(Compiler,
                                      PolynaryExpr->exprBase.base.lineNo,
                                      PolynaryExpr->exprBase.base.stringNo,
                                      CL_VIR_IK_image_query_size,
                                      lOperand,
                                      OperandCount,
                                      &OperandsParameters[0].rOperands[0]);
    }
    if (cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX) ||
        gcmOPT_oclUseImgIntrinsicQuery())
    {
        return _GenQueryImageCallCode(Compiler,
                                      CodeGenerator,
                                      PolynaryExpr,
                                      "size",
                                      OperandCount,
                                      OperandsParameters,
                                      IOperand);
    }
    clsROPERAND_InitializeIntOrIVecConstant(&constROperand,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 0);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 IOperand,
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand));

    if (clmGEN_CODE_elementType_GET(OperandsParameters[0].dataTypes[0].def) == clvTYPE_IMAGE3D_T)
    {
        clsLOPERAND iOperand[1];
        clsLOPERAND lOperand[1];

        /* Set component w to 0. */
        clsLOPERAND_InitializeUsingIOperand(iOperand, IOperand);
        clmLOPERAND_vectorComponent_GET(lOperand, iOperand, clvCOMPONENT_W);
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   lOperand,
                                   &constROperand));
    }

    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    return status;
}

static gceSTATUS
_GenGetImageArrayCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    clsROPERAND constROperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (_OCL_USE_INTRINSIC_FOR_IMAGE && !cloCOMPILER_IsGcslDriverImage(Compiler)) {
        clsLOPERAND lOperand[1];

        clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
        return  clGenIntrinsicAsmCode(Compiler,
                                      PolynaryExpr->exprBase.base.lineNo,
                                      PolynaryExpr->exprBase.base.stringNo,
                                      CL_VIR_IK_image_query_array_size,
                                      lOperand,
                                      OperandCount,
                                      &OperandsParameters[0].rOperands[0]);
    }
    if (cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX) ||
        gcmOPT_oclUseImgIntrinsicQuery())
    {
        return _GenQueryImageCallCode(Compiler,
                                      CodeGenerator,
                                      PolynaryExpr,
                                      "array_size",
                                      OperandCount,
                                      OperandsParameters,
                                      IOperand);
    }
    clsROPERAND_InitializeIntOrIVecConstant(&constROperand,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 32);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 IOperand,
                                 &OperandsParameters[0].rOperands[0],
                                 &constROperand));

    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    return status;
}



static gceSTATUS
_GenVivTexldCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    clsIOPERAND iOperand[1];
    clsLOPERAND lOperand[1];
    clsROPERAND samplerOperand[1];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);

    clsIOPERAND_New(Compiler, iOperand, OperandsParameters[0].rOperands[0].dataType);
    clsLOPERAND_InitializeUsingIOperand(lOperand, iOperand);
    status = clGenAssignCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             lOperand,
                             &OperandsParameters[0].rOperands[0]);
    if (gcmIS_ERROR(status)) return status;

    clsROPERAND_InitializeUsingIOperand(samplerOperand, iOperand);
    samplerOperand->dataType.elementType = clvTYPE_SAMPLER2D;

    return  clGenGenericCode2(Compiler,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo,
                              clvOPCODE_TEXTURE_LOAD,
                              IOperand,
                              samplerOperand,
                              &OperandsParameters[1].rOperands[0]);
}

#endif /* __gc_cl_built_ins_image_h_ */
