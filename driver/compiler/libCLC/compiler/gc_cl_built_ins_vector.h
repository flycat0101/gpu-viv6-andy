/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_cl_built_ins_vector_h_
#define __gc_cl_built_ins_vector_h_

static clsBUILTIN_FUNCTION    VectorBuiltinFunctions[] =
{
    {clvEXTENSION_VIV_VX,    "vload2",          T_SHORT2_PACKED, 2, {T_SIZE_T, T_SHORT_PACKED}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_VIV_VX,    "vload4",          T_SHORT4_PACKED, 2, {T_SIZE_T, T_SHORT_PACKED}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_VIV_VX,    "vload8",          T_SHORT8_PACKED, 2, {T_SIZE_T, T_SHORT_PACKED}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_VIV_VX,    "vload16",         T_SHORT16_PACKED, 2, {T_SIZE_T, T_SHORT_PACKED}, {0, 1}, {1, 0}, 1},

    {clvEXTENSION_VIV_VX,    "vload2",          T_USHORT2_PACKED, 2, {T_SIZE_T, T_USHORT_PACKED}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_VIV_VX,    "vload4",          T_USHORT4_PACKED, 2, {T_SIZE_T, T_USHORT_PACKED}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_VIV_VX,    "vload8",          T_USHORT8_PACKED, 2, {T_SIZE_T, T_USHORT_PACKED}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_VIV_VX,    "vload16",         T_USHORT16_PACKED, 2, {T_SIZE_T, T_USHORT_PACKED}, {0, 1}, {1, 0}, 1},

    {clvEXTENSION_VIV_VX,    "vload2",          T_CHAR2_PACKED, 2, {T_SIZE_T, T_CHAR_PACKED}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_VIV_VX,    "vload4",          T_CHAR4_PACKED, 2, {T_SIZE_T, T_CHAR_PACKED}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_VIV_VX,    "vload8",          T_CHAR8_PACKED, 2, {T_SIZE_T, T_CHAR_PACKED}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_VIV_VX,    "vload16",         T_CHAR16_PACKED, 2, {T_SIZE_T, T_CHAR_PACKED}, {0, 1}, {1, 0}, 1},

    {clvEXTENSION_VIV_VX,    "vload2",          T_UCHAR2_PACKED, 2, {T_SIZE_T, T_UCHAR_PACKED}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_VIV_VX,    "vload4",          T_UCHAR4_PACKED, 2, {T_SIZE_T, T_UCHAR_PACKED}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_VIV_VX,    "vload8",          T_UCHAR8_PACKED, 2, {T_SIZE_T, T_UCHAR_PACKED}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_VIV_VX,    "vload16",         T_UCHAR16_PACKED, 2, {T_SIZE_T, T_UCHAR_PACKED}, {0, 1}, {1, 0}, 1},

    {clvEXTENSION_VIV_VX,    "vload2",          T_HALF2_PACKED, 2, {T_SIZE_T, T_HALF_PACKED}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_VIV_VX,    "vload4",          T_HALF4_PACKED, 2, {T_SIZE_T, T_HALF_PACKED}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_VIV_VX,    "vload8",          T_HALF8_PACKED, 2, {T_SIZE_T, T_HALF_PACKED}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_VIV_VX,    "vload16",         T_HALF16_PACKED, 2, {T_SIZE_T, T_HALF_PACKED}, {0, 1}, {1, 0}, 1},

    {clvEXTENSION_VIV_VX,    "vstore2",          T_VOID, 3, {T_SHORT2_PACKED, T_SIZE_T, T_SHORT_PACKED}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_VIV_VX,    "vstore4",          T_VOID, 3, {T_SHORT4_PACKED, T_SIZE_T, T_SHORT_PACKED}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_VIV_VX,    "vstore8",          T_VOID, 3, {T_SHORT8_PACKED, T_SIZE_T, T_SHORT_PACKED}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_VIV_VX,    "vstore16",         T_VOID, 3, {T_SHORT16_PACKED, T_SIZE_T, T_SHORT_PACKED}, {0, 0, 1}, {0, 1, 0}, 1, 1},

    {clvEXTENSION_VIV_VX,    "vstore2",          T_VOID, 3, {T_USHORT2_PACKED, T_SIZE_T, T_USHORT_PACKED}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_VIV_VX,    "vstore4",          T_VOID, 3, {T_USHORT4_PACKED, T_SIZE_T, T_USHORT_PACKED}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_VIV_VX,    "vstore8",          T_VOID, 3, {T_USHORT8_PACKED, T_SIZE_T, T_USHORT_PACKED}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_VIV_VX,    "vstore16",         T_VOID, 3, {T_USHORT16_PACKED, T_SIZE_T, T_USHORT_PACKED}, {0, 0, 1}, {0, 1, 0}, 1, 1},

    {clvEXTENSION_VIV_VX,    "vstore2",          T_VOID, 3, {T_CHAR2_PACKED, T_SIZE_T, T_CHAR_PACKED}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_VIV_VX,    "vstore4",          T_VOID, 3, {T_CHAR4_PACKED, T_SIZE_T, T_CHAR_PACKED}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_VIV_VX,    "vstore8",          T_VOID, 3, {T_CHAR8_PACKED, T_SIZE_T, T_CHAR_PACKED}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_VIV_VX,    "vstore16",         T_VOID, 3, {T_CHAR16_PACKED, T_SIZE_T, T_CHAR_PACKED}, {0, 0, 1}, {0, 1, 0}, 1, 1},

    {clvEXTENSION_VIV_VX,    "vstore2",          T_VOID, 3, {T_UCHAR2_PACKED, T_SIZE_T, T_UCHAR_PACKED}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_VIV_VX,    "vstore4",          T_VOID, 3, {T_UCHAR4_PACKED, T_SIZE_T, T_UCHAR_PACKED}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_VIV_VX,    "vstore8",          T_VOID, 3, {T_UCHAR8_PACKED, T_SIZE_T, T_UCHAR_PACKED}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_VIV_VX,    "vstore16",         T_VOID, 3, {T_UCHAR16_PACKED, T_SIZE_T, T_UCHAR_PACKED}, {0, 0, 1}, {0, 1, 0}, 1, 1},

    {clvEXTENSION_VIV_VX,    "vstore2",          T_VOID, 3, {T_HALF2_PACKED, T_SIZE_T, T_HALF_PACKED}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_VIV_VX,    "vstore4",          T_VOID, 3, {T_HALF4_PACKED, T_SIZE_T, T_HALF_PACKED}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_VIV_VX,    "vstore8",          T_VOID, 3, {T_HALF8_PACKED, T_SIZE_T, T_HALF_PACKED}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_VIV_VX,    "vstore16",         T_VOID, 3, {T_HALF16_PACKED, T_SIZE_T, T_HALF_PACKED}, {0, 0, 1}, {0, 1, 0}, 1, 1},

    {clvEXTENSION_NONE,    "vload2",          T_FLOAT2, 2, {T_SIZE_T, T_FLOAT}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload3",          T_FLOAT3, 2, {T_SIZE_T, T_FLOAT}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload4",          T_FLOAT4, 2, {T_SIZE_T, T_FLOAT}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload8",          T_FLOAT8, 2, {T_SIZE_T, T_FLOAT}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload16",         T_FLOAT16, 2, {T_SIZE_T, T_FLOAT}, {0, 1}, {1, 0}, 1},

    {clvEXTENSION_NONE,    "vload2",          T_INT2, 2, {T_SIZE_T, T_INT}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload3",          T_INT3, 2, {T_SIZE_T, T_INT}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload4",          T_INT4, 2, {T_SIZE_T, T_INT}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload8",          T_INT8, 2, {T_SIZE_T, T_INT}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload16",         T_INT16, 2, {T_SIZE_T, T_INT}, {0, 1}, {1, 0}, 1},

    {clvEXTENSION_NONE,    "vload2",          T_LONG2, 2, {T_SIZE_T, T_LONG}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload3",          T_LONG3, 2, {T_SIZE_T, T_LONG}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload4",          T_LONG4, 2, {T_SIZE_T, T_LONG}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload8",          T_LONG8, 2, {T_SIZE_T, T_LONG}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload16",         T_LONG16, 2, {T_SIZE_T, T_LONG}, {0, 1}, {1, 0}, 1},

    {clvEXTENSION_NONE,    "vload2",          T_UINT2, 2, {T_SIZE_T, T_UINT}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload3",          T_UINT3, 2, {T_SIZE_T, T_UINT}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload4",          T_UINT4, 2, {T_SIZE_T, T_UINT}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload8",          T_UINT8, 2, {T_SIZE_T, T_UINT}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload16",         T_UINT16, 2, {T_SIZE_T, T_UINT}, {0, 1}, {1, 0}, 1},

    {clvEXTENSION_NONE,    "vload2",          T_ULONG2, 2, {T_SIZE_T, T_ULONG}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload3",          T_ULONG3, 2, {T_SIZE_T, T_ULONG}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload4",          T_ULONG4, 2, {T_SIZE_T, T_ULONG}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload8",          T_ULONG8, 2, {T_SIZE_T, T_ULONG}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload16",         T_ULONG16, 2, {T_SIZE_T, T_ULONG}, {0, 1}, {1, 0}, 1},

    {clvEXTENSION_NONE,    "vload2",          T_SHORT2, 2, {T_SIZE_T, T_SHORT}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload3",          T_SHORT3, 2, {T_SIZE_T, T_SHORT}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload4",          T_SHORT4, 2, {T_SIZE_T, T_SHORT}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload8",          T_SHORT8, 2, {T_SIZE_T, T_SHORT}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload16",          T_SHORT16, 2, {T_SIZE_T, T_SHORT}, {0, 1}, {1, 0}, 1},

    {clvEXTENSION_NONE,    "vload2",          T_USHORT2, 2, {T_SIZE_T, T_USHORT}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload3",          T_USHORT3, 2, {T_SIZE_T, T_USHORT}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload4",          T_USHORT4, 2, {T_SIZE_T, T_USHORT}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload8",          T_USHORT8, 2, {T_SIZE_T, T_USHORT}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload16",          T_USHORT16, 2, {T_SIZE_T, T_USHORT}, {0, 1}, {1, 0}, 1},

    {clvEXTENSION_NONE,    "vload2",          T_CHAR2, 2, {T_SIZE_T, T_CHAR}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload3",          T_CHAR3, 2, {T_SIZE_T, T_CHAR}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload4",          T_CHAR4, 2, {T_SIZE_T, T_CHAR}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload8",          T_CHAR8, 2, {T_SIZE_T, T_CHAR}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload16",         T_CHAR16, 2, {T_SIZE_T, T_CHAR}, {0, 1}, {1, 0}, 1},

    {clvEXTENSION_NONE,    "vload2",          T_UCHAR2, 2, {T_SIZE_T, T_UCHAR}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload3",          T_UCHAR3, 2, {T_SIZE_T, T_UCHAR}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload4",          T_UCHAR4, 2, {T_SIZE_T, T_UCHAR}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload8",          T_UCHAR8, 2, {T_SIZE_T, T_UCHAR}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload16",         T_UCHAR16, 2, {T_SIZE_T, T_UCHAR}, {0, 1}, {1, 0}, 1},

    {clvEXTENSION_NONE,    "vload_half",       T_FLOAT, 2, {T_SIZE_T, T_HALF}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload_half2",      T_FLOAT2, 2, {T_SIZE_T, T_HALF}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload_half3",      T_FLOAT3, 2, {T_SIZE_T, T_HALF}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload_half4",      T_FLOAT4, 2, {T_SIZE_T, T_HALF}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload_half8",      T_FLOAT8, 2, {T_SIZE_T, T_HALF}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,    "vload_half16",     T_FLOAT16, 2, {T_SIZE_T, T_HALF}, {0, 1}, {1, 0}, 1},

    {clvEXTENSION_NONE,    "vstore2",          T_VOID, 3, {T_FLOAT2, T_SIZE_T, T_FLOAT}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore3",          T_VOID, 3, {T_FLOAT3, T_SIZE_T, T_FLOAT}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore4",          T_VOID, 3, {T_FLOAT4, T_SIZE_T, T_FLOAT}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore8",          T_VOID, 3, {T_FLOAT8, T_SIZE_T, T_FLOAT}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore16",         T_VOID, 3, {T_FLOAT16, T_SIZE_T, T_FLOAT}, {0, 0, 1}, {0, 1, 0}, 1, 1},

    {clvEXTENSION_NONE,    "vstore2",          T_VOID, 3, {T_INT2, T_SIZE_T, T_INT}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore3",          T_VOID, 3, {T_INT3, T_SIZE_T, T_INT}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore4",          T_VOID, 3, {T_INT4, T_SIZE_T, T_INT}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore8",          T_VOID, 3, {T_INT8, T_SIZE_T, T_INT}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore16",         T_VOID, 3, {T_INT16, T_SIZE_T, T_INT}, {0, 0, 1}, {0, 1, 0}, 1, 1},

    {clvEXTENSION_NONE,    "vstore2",          T_VOID, 3, {T_LONG2, T_SIZE_T, T_LONG}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore3",          T_VOID, 3, {T_LONG3, T_SIZE_T, T_LONG}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore4",          T_VOID, 3, {T_LONG4, T_SIZE_T, T_LONG}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore8",          T_VOID, 3, {T_LONG8, T_SIZE_T, T_LONG}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore16",         T_VOID, 3, {T_LONG16, T_SIZE_T, T_LONG}, {0, 0, 1}, {0, 1, 0}, 1, 1},

    {clvEXTENSION_NONE,    "vstore2",          T_VOID, 3, {T_UINT2, T_SIZE_T, T_UINT}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore3",          T_VOID, 3, {T_UINT3, T_SIZE_T, T_UINT}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore4",          T_VOID, 3, {T_UINT4, T_SIZE_T, T_UINT}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore8",          T_VOID, 3, {T_UINT8, T_SIZE_T, T_UINT}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore16",         T_VOID, 3, {T_UINT16, T_SIZE_T, T_UINT}, {0, 0, 1}, {0, 1, 0}, 1, 1},

    {clvEXTENSION_NONE,    "vstore2",          T_VOID, 3, {T_ULONG2, T_SIZE_T, T_ULONG}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore3",          T_VOID, 3, {T_ULONG3, T_SIZE_T, T_ULONG}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore4",          T_VOID, 3, {T_ULONG4, T_SIZE_T, T_ULONG}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore8",          T_VOID, 3, {T_ULONG8, T_SIZE_T, T_ULONG}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore16",         T_VOID, 3, {T_ULONG16, T_SIZE_T, T_ULONG}, {0, 0, 1}, {0, 1, 0}, 1, 1},

    {clvEXTENSION_NONE,    "vstore2",          T_VOID, 3, {T_SHORT2, T_SIZE_T, T_SHORT}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore3",          T_VOID, 3, {T_SHORT3, T_SIZE_T, T_SHORT}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore4",          T_VOID, 3, {T_SHORT4, T_SIZE_T, T_SHORT}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore8",          T_VOID, 3, {T_SHORT8, T_SIZE_T, T_SHORT}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore16",         T_VOID, 3, {T_SHORT16, T_SIZE_T, T_SHORT}, {0, 0, 1}, {0, 1, 0}, 1, 1},

    {clvEXTENSION_NONE,    "vstore2",          T_VOID, 3, {T_USHORT2, T_SIZE_T, T_USHORT}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore3",          T_VOID, 3, {T_USHORT3, T_SIZE_T, T_USHORT}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore4",          T_VOID, 3, {T_USHORT4, T_SIZE_T, T_USHORT}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore8",          T_VOID, 3, {T_USHORT8, T_SIZE_T, T_USHORT}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore16",         T_VOID, 3, {T_USHORT16, T_SIZE_T, T_USHORT}, {0, 0, 1}, {0, 1, 0}, 1, 1},

    {clvEXTENSION_NONE,    "vstore2",          T_VOID, 3, {T_CHAR2, T_SIZE_T, T_CHAR}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore3",          T_VOID, 3, {T_CHAR3, T_SIZE_T, T_CHAR}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore4",          T_VOID, 3, {T_CHAR4, T_SIZE_T, T_CHAR}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore8",          T_VOID, 3, {T_CHAR8, T_SIZE_T, T_CHAR}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore16",         T_VOID, 3, {T_CHAR16, T_SIZE_T, T_CHAR}, {0, 0, 1}, {0, 1, 0}, 1, 1},

    {clvEXTENSION_NONE,    "vstore2",          T_VOID, 3, {T_UCHAR2, T_SIZE_T, T_UCHAR}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore3",          T_VOID, 3, {T_UCHAR3, T_SIZE_T, T_UCHAR}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore4",          T_VOID, 3, {T_UCHAR4, T_SIZE_T, T_UCHAR}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore8",          T_VOID, 3, {T_UCHAR8, T_SIZE_T, T_UCHAR}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore16",         T_VOID, 3, {T_UCHAR16, T_SIZE_T, T_UCHAR}, {0, 0, 1}, {0, 1, 0}, 1, 1},

    {clvEXTENSION_NONE,    "vstore2",          T_VOID, 3, {T_HALF2, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore3",          T_VOID, 3, {T_HALF3, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore4",          T_VOID, 3, {T_HALF4, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore8",          T_VOID, 3, {T_HALF8, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore16",         T_VOID, 3, {T_HALF16, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},

    {clvEXTENSION_NONE,    "vstore_half",      T_VOID, 3, {T_FLOAT, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore_half2",     T_VOID, 3, {T_FLOAT2, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore_half3",     T_VOID, 3, {T_FLOAT3, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore_half4",     T_VOID, 3, {T_FLOAT4, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore_half8",     T_VOID, 3, {T_FLOAT8, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore_half16",    T_VOID, 3, {T_FLOAT16, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},

    {clvEXTENSION_NONE,    "viv_getlonglo",    T_UINT, 1, {T_LONG}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "viv_getlonglo2",   T_UINT2, 1, {T_LONG2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "viv_getlonglo3",   T_UINT3, 1, {T_LONG3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "viv_getlonglo4",   T_UINT4, 1, {T_LONG4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "viv_getlonglo8",   T_UINT8, 1, {T_LONG8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "viv_getlonglo16",  T_UINT16, 1, {T_LONG16}, {0}, {0}, 1},

    {clvEXTENSION_NONE,    "viv_getlonglo",    T_UINT, 1, {T_ULONG}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "viv_getlonglo2",   T_UINT2, 1, {T_ULONG2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "viv_getlonglo3",   T_UINT3, 1, {T_ULONG3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "viv_getlonglo4",   T_UINT4, 1, {T_ULONG4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "viv_getlonglo8",   T_UINT8, 1, {T_ULONG8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "viv_getlonglo16",  T_UINT16, 1, {T_ULONG16}, {0}, {0}, 1},

    {clvEXTENSION_NONE,    "viv_getlonghi",    T_UINT, 1, {T_LONG}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "viv_getlonghi2",   T_UINT2, 1, {T_LONG2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "viv_getlonghi3",   T_UINT3, 1, {T_LONG3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "viv_getlonghi4",   T_UINT4, 1, {T_LONG4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "viv_getlonghi8",   T_UINT8, 1, {T_LONG8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "viv_getlonghi16",  T_UINT16, 1, {T_LONG16}, {0}, {0}, 1},

    {clvEXTENSION_NONE,    "viv_getlonghi",    T_UINT, 1, {T_ULONG}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "viv_getlonghi2",   T_UINT2, 1, {T_ULONG2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "viv_getlonghi3",   T_UINT3, 1, {T_ULONG3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "viv_getlonghi4",   T_UINT4, 1, {T_ULONG4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "viv_getlonghi8",   T_UINT8, 1, {T_ULONG8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "viv_getlonghi16",  T_UINT16, 1, {T_ULONG16}, {0}, {0}, 1},

    {clvEXTENSION_NONE,    "viv_setlong",    T_VOID, 3, {T_LONG,  T_UINT,  T_UINT}, {0, 0, 0}, {0, 0, 0}, 1, 1},
    {clvEXTENSION_NONE,    "viv_setlong2",   T_VOID, 3, {T_LONG2, T_UINT2, T_UINT2}, {0, 0, 0}, {0, 0, 0}, 1, 1},
    {clvEXTENSION_NONE,    "viv_setlong3",   T_VOID, 3, {T_LONG3, T_UINT3, T_UINT3}, {0, 0, 0}, {0, 0, 0}, 1, 1},
    {clvEXTENSION_NONE,    "viv_setlong4",   T_VOID, 3, {T_LONG4, T_UINT4, T_UINT4}, {0, 0, 0}, {0, 0, 0}, 1, 1},
    {clvEXTENSION_NONE,    "viv_setlong8",   T_VOID, 3, {T_LONG8, T_UINT8, T_UINT8}, {0, 0, 0}, {0, 0, 0}, 1, 1},
    {clvEXTENSION_NONE,    "viv_setlong16",  T_VOID, 3, {T_LONG16, T_UINT16, T_UINT16}, {0, 0, 0}, {0, 0, 0}, 1, 1},
    {clvEXTENSION_NONE,    "viv_setlong",    T_VOID, 3, {T_ULONG,  T_UINT,  T_UINT}, {0, 0, 0}, {0, 0, 0}, 1, 1},
    {clvEXTENSION_NONE,    "viv_setlong2",   T_VOID, 3, {T_ULONG2, T_UINT2, T_UINT2}, {0, 0, 0}, {0, 0, 0}, 1, 1},
    {clvEXTENSION_NONE,    "viv_setlong3",   T_VOID, 3, {T_ULONG3, T_UINT3, T_UINT3}, {0, 0, 0}, {0, 0, 0}, 1, 1},
    {clvEXTENSION_NONE,    "viv_setlong4",   T_VOID, 3, {T_ULONG4, T_UINT4, T_UINT4}, {0, 0, 0}, {0, 0, 0}, 1, 1},
    {clvEXTENSION_NONE,    "viv_setlong8",   T_VOID, 3, {T_ULONG8, T_UINT8, T_UINT8}, {0, 0, 0}, {0, 0, 0}, 1, 1},
    {clvEXTENSION_NONE,    "viv_setlong16",  T_VOID, 3, {T_ULONG16, T_UINT16, T_UINT16}, {0, 0, 0}, {0, 0, 0}, 1, 1},
    {clvEXTENSION_NONE,    "viv_unpack",       T_UINT4, 2, {T_UINT4, T_UINT2}, {0, 0}, {0, 0}, 1},
};

#define _cldVectorBuiltinFunctionCount (sizeof(VectorBuiltinFunctions) / sizeof(clsBUILTIN_FUNCTION))

static gceSTATUS
_GenGetLongLoCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    gcmASSERT(OperandsParameters[0].rOperands[0].isReg);
    gcmASSERT(OperandsParameters[0].rOperands[0].dataType.elementType == clvTYPE_LONG ||
              OperandsParameters[0].rOperands[0].dataType.elementType == clvTYPE_ULONG);
    return clGenGenericCode1(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             clvOPCODE_LONGLO,
                             IOperand,
                             OperandsParameters[0].rOperands);
}

static gceSTATUS
_GenGetLongHiCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    gcmASSERT(OperandsParameters[0].rOperands[0].isReg);
    gcmASSERT(OperandsParameters[0].rOperands[0].dataType.elementType == clvTYPE_LONG ||
              OperandsParameters[0].rOperands[0].dataType.elementType == clvTYPE_ULONG);
    return clGenGenericCode1(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             clvOPCODE_LONGHI,
                             IOperand,
                             OperandsParameters[0].rOperands);
}

static gceSTATUS
_GenSetLongCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    clsIOPERAND iOperand[1];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand == gcvNULL);

    gcmASSERT(OperandsParameters[0].rOperands[0].isReg);
    gcmASSERT(OperandsParameters[0].rOperands[0].dataType.elementType == clvTYPE_LONG ||
              OperandsParameters[0].rOperands[0].dataType.elementType == clvTYPE_ULONG);

    clsIOPERAND_InitializeWithComponentSelection(iOperand,
                                                 OperandsParameters[0].rOperands[0].dataType,
                                                 OperandsParameters[0].rOperands[0].u.reg.dataType,
                                                 OperandsParameters[0].rOperands[0].u.reg.regIndex,
                                                 OperandsParameters[0].rOperands[0].u.reg.componentSelection);

    return clGenGenericCode2(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             clvOPCODE_MOV_LONG,
                             iOperand,
                             &OperandsParameters[1].rOperands[0],
                             &OperandsParameters[2].rOperands[0]);
}

#if !_GEN_PACKED_LOAD_STORE_AS_BUILTIN
static gceSTATUS
_GenPackedVloadFuncCall(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    cltPOOL_STRING symbolInPool;
    gctCHAR   nameBuf[32];
    gctSTRING funcNameString = nameBuf;
    gctUINT   offset = 0;
    gctUINT8 vectorSize;
    clsGEN_CODE_PARAMETERS parameters[1];
    clsGEN_CODE_PARAMETERS_Initialize(parameters,
                                      gcvFALSE,
                                      gcvTRUE);

    vectorSize = clmGEN_CODE_vectorSize_GET(IOperand->dataType);

    gcmVERIFY_OK(gcoOS_PrintStrSafe(funcNameString,
                                    32,
                                    &offset,
                                    "viv_intrinsic_vx_vload%d",
                                    vectorSize));
    gcmONERROR(cloCOMPILER_FindPoolString(Compiler,
                                          funcNameString,
                                          &symbolInPool));

    PolynaryExpr->funcSymbol = symbolInPool;
    PolynaryExpr->funcName = gcvNULL;
    gcmONERROR(cloCOMPILER_BindFuncCall(Compiler,
                                        PolynaryExpr));

    parameters->operandCount = 0;

    IOperand->dataType = clmGenCodeDataType(PolynaryExpr->exprBase.decl.dataType->type);
    clmGEN_CODE_SetParametersIOperand(Compiler,
                                      parameters,
                                      0,
                                      IOperand,
                                      0);
    status = clGenFuncCallCode(Compiler,
                               CodeGenerator,
                               PolynaryExpr,
                               OperandsParameters,
                               parameters);

OnError:
    clsGEN_CODE_PARAMETERS_Finalize(parameters);
    return status;
}
#else
static gceSTATUS
_GenPackedVloadAsmCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT dataTypeSize = 0;
    clsGEN_CODE_PARAMETERS operandsParameters[4];
    clsDECL  decl[1];
    clsIOPERAND iOperand[1];
    clvVIR_IK intrinsicsKind = CL_VIR_IK_UNKNOWN;
    cloIR_BASE addressArg;

    if(IOperand == gcvNULL) return gcvSTATUS_OK;

    clsGEN_CODE_PARAMETERS_Initialize(&operandsParameters[0],
                                      gcvFALSE,
                                      gcvTRUE);

    clsGEN_CODE_PARAMETERS_Initialize(&operandsParameters[1],
                                      gcvFALSE,
                                      gcvTRUE);

    clsGEN_CODE_PARAMETERS_Initialize(&operandsParameters[2],
                                      gcvFALSE,
                                      gcvTRUE);

    clsGEN_CODE_PARAMETERS_Initialize(&operandsParameters[3],
                                      gcvFALSE,
                                      gcvTRUE);

    /* opcode operand */
    gcmONERROR(cloCOMPILER_CreateDecl(Compiler,
                                      T_INT,
                                      gcvNULL,
                                      clvQUALIFIER_CONST,
                                      clvQUALIFIER_NONE,
                                      decl));
    gcmONERROR(clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                                       &operandsParameters[0],
                                                       decl));

    clsROPERAND_InitializeScalarConstant(&operandsParameters[0].rOperands[0],
                                         clmGenCodeDataType(T_INT),
                                         int,
                                         clvOPCODE_PARAM_CHAIN);

    /* parameter chain variable operand */
    clsGEN_CODE_PARAMETERS_Initialize(&operandsParameters[1],
                                      gcvFALSE,
                                      gcvTRUE);

    gcmONERROR(clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                                       &operandsParameters[1],
                                                       decl));

    clsIOPERAND_New(Compiler, iOperand, operandsParameters[1].dataTypes[0].def);
    clsROPERAND_InitializeUsingIOperand(&operandsParameters[1].rOperands[0], iOperand);

    /* source operand 1 */
    status = cloIR_SET_GetMember(Compiler,
                                 PolynaryExpr->operands,
                                 2,
                                 &addressArg);
    gcmONERROR(clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                                       &operandsParameters[2],
                                                       &(((cloIR_EXPR) addressArg)->decl)));
    operandsParameters[2].rOperands[0] = OperandsParameters[1].rOperands[0];

    /* source operand 2 */
    gcmONERROR(clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                                       &operandsParameters[3],
                                                       decl));

    if(clmCODE_GENERATOR_IsVectorMemoryDeref(CodeGenerator)) {
       dataTypeSize = (gctUINT)clGEN_CODE_DataTypeByteSize(Compiler, IOperand->dataType);
    }
    else {
       dataTypeSize = gcGetAddressableUnitSize(IOperand->dataType);
    }
    gcmONERROR(clGenScaledIndexOperand(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       &OperandsParameters[0].rOperands[0],
                                       dataTypeSize,
                                       gcvTRUE,
                                       &operandsParameters[3].rOperands[0]));
    gcmONERROR(clGenBuiltInAsmCode(Compiler,
                                   CodeGenerator,
                                   PolynaryExpr,
                                   4,
                                   operandsParameters,
                                   gcvNULL));

    /* need to release the operand corresponding to vload pointer as it cannot be reused in the
       following asm instruction */
    clsGEN_CODE_PARAMETERS_Finalize(&operandsParameters[2]);
    clsGEN_CODE_PARAMETERS_Initialize(&operandsParameters[2],
                                      gcvFALSE,
                                      gcvTRUE);
    gcmONERROR(clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                                       &operandsParameters[2],
                                                       decl));

    /* opcode operand*/
    clsROPERAND_InitializeScalarConstant(&operandsParameters[0].rOperands[0],
                                         clmGenCodeDataType(T_INT),
                                         int,
                                         clvOPCODE_INTRINSIC);

    /* don't care about the destination operand as it can be taken care of by the IOPERAND */

    switch(clmGEN_CODE_vectorSize_GET(IOperand->dataType)) {
    case 2:
        intrinsicsKind = CL_VIR_IK_evis_vload2;
        break;
     case 3:
        intrinsicsKind = CL_VIR_IK_evis_vload3;
        break;
    case 4:
        intrinsicsKind = CL_VIR_IK_evis_vload4;
        break;
     case 8:
        intrinsicsKind = CL_VIR_IK_evis_vload8;
        break;
     case 16:
        intrinsicsKind = CL_VIR_IK_evis_vload16;
        break;
     default:
        gcmASSERT(0);
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmONERROR(status);
    }

    clsROPERAND_InitializeScalarConstant(&operandsParameters[2].rOperands[0],
                                         clmGenCodeDataType(T_INT),
                                         int,
                                         intrinsicsKind);

    operandsParameters[3].rOperands[0] = operandsParameters[1].rOperands[0];
    gcmONERROR(clGenBuiltInAsmCode(Compiler,
                                   CodeGenerator,
                                   PolynaryExpr,
                                   4,
                                   operandsParameters,
                                   IOperand));

OnError:
    clsGEN_CODE_PARAMETERS_Finalize(&operandsParameters[0]);
    clsGEN_CODE_PARAMETERS_Finalize(&operandsParameters[1]);
    clsGEN_CODE_PARAMETERS_Finalize(&operandsParameters[2]);
    clsGEN_CODE_PARAMETERS_Finalize(&operandsParameters[3]);
    return status;
}
#endif

static gceSTATUS
_GenVloadCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    clsROPERAND scaledIndex[1];
    clsROPERAND operandBuffer[1];
    clsROPERAND *rOperand;
    gctBOOL loadNeeded;
    cloIR_BASE addressArg;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if(clmIsElementTypePacked(IOperand->dataType.elementType)) {
#if !_GEN_PACKED_LOAD_STORE_AS_BUILTIN
        return _GenPackedVloadFuncCall(Compiler,
                                       CodeGenerator,
                                       PolynaryExpr,
                                       OperandCount,
                                       OperandsParameters,
                                       IOperand);
#else
        return _GenPackedVloadAsmCode(Compiler,
                                      CodeGenerator,
                                      PolynaryExpr,
                                      OperandCount,
                                      OperandsParameters,
                                      IOperand);
#endif
    }
    status = clGenScaledIndexOperand(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     &OperandsParameters[0].rOperands[0],
                                     gcGetAddressableUnitSize(IOperand->dataType),
                                     gcvTRUE,
                                     scaledIndex);
    if (gcmIS_ERROR(status)) return status;

    rOperand = &OperandsParameters[1].rOperands[0];
    if(!clIsIntegerZero(&OperandsParameters[1].dataTypes[0].byteOffset)) { /* byte offset is non-zero */
       if(!scaledIndex->isReg) { /*index is a constant */
          status = clUpdateAddressOffset(Compiler,
                                         PolynaryExpr->exprBase.base.lineNo,
                                         PolynaryExpr->exprBase.base.stringNo,
                                         clGetIntegerValue(scaledIndex),
                                         &OperandsParameters[1].dataTypes[0].byteOffset,
                                         scaledIndex);
          if (gcmIS_ERROR(status)) return status;
       }
       else {
          clsIOPERAND intermIOperand[1];

          clsIOPERAND_New(Compiler, intermIOperand, clmGenCodeDataType(T_INT));
          status = clGenGenericCode2(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ADD,
                                     intermIOperand,
                                     rOperand,
                                     &OperandsParameters[1].dataTypes[0].byteOffset);
          if (gcmIS_ERROR(status)) return status;
          clsROPERAND_InitializeUsingIOperand(operandBuffer, intermIOperand);
          rOperand = operandBuffer;
       }
    }

    status = cloIR_SET_GetMember(Compiler,
                                 PolynaryExpr->operands,
                                 2,
                                 &addressArg);
    if (gcmIS_ERROR(status)) return status;
    loadNeeded = clGenNeedVectorUpdate(Compiler,
                                       CodeGenerator,
                                       (cloIR_EXPR) addressArg,
                                       IOperand->dataType,
                                       scaledIndex,
                                       IOperand);

    if(loadNeeded) {
       status = clGenGenericCode2(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_LOAD,
                                  IOperand,
                                  rOperand,
                                  scaledIndex);
       if (gcmIS_ERROR(status)) return status;
    }

    return gcvSTATUS_OK;
}

#if !_GEN_PACKED_LOAD_STORE_AS_BUILTIN
static gceSTATUS
_GenPackedVstoreFuncCall(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctCHAR   nameBuf[32];
    gctSTRING funcNameString = nameBuf;
    cltPOOL_STRING symbolInPool;
    gctUINT   offset = 0;
    gctUINT8 vectorSize;
    clsGEN_CODE_PARAMETERS parameters[1];
    clsGEN_CODE_PARAMETERS_Initialize(parameters,
                                      gcvFALSE,
                                      gcvFALSE);

    vectorSize = clmGEN_CODE_vectorSize_GET(OperandsParameters[0].rOperands[0].dataType);

    gcmVERIFY_OK(gcoOS_PrintStrSafe(funcNameString,
                                   32,
                                   &offset,
                                   "viv_intrinsic_vx_vstore%d",
                                   vectorSize));
    gcmONERROR(cloCOMPILER_FindPoolString(Compiler,
                                          funcNameString,
                                          &symbolInPool));

    PolynaryExpr->funcSymbol = symbolInPool;
    PolynaryExpr->funcName = gcvNULL;
    gcmONERROR(cloCOMPILER_BindFuncCall(Compiler,
                                        PolynaryExpr));

    parameters->operandCount = 0;
    status = clGenFuncCallCode(Compiler,
                               CodeGenerator,
                               PolynaryExpr,
                               OperandsParameters,
                               parameters);
OnError:
    clsGEN_CODE_PARAMETERS_Finalize(parameters);
    return status;
}
#else
static gceSTATUS
_GenPackedVstoreAsmCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT dataTypeSize = 0;
    clsGEN_CODE_PARAMETERS operandsParameters[4];
    clsDECL  decl[1];
    clsIOPERAND iOperand[1];
    clsIOPERAND intermIOperand[1];
    clsLOPERAND lOperand[1];
    clsROPERAND offsetOperand[1], dataOperand[1];
    clvVIR_IK intrinsicsKind = CL_VIR_IK_UNKNOWN;
    cloIR_BASE addressArg;
    clsROPERAND *offset;
    clsROPERAND *data;

    clsGEN_CODE_PARAMETERS_Initialize(&operandsParameters[0],
                                      gcvFALSE,
                                      gcvTRUE);

    clsGEN_CODE_PARAMETERS_Initialize(&operandsParameters[1],
                                      gcvFALSE,
                                      gcvTRUE);

    clsGEN_CODE_PARAMETERS_Initialize(&operandsParameters[2],
                                      gcvFALSE,
                                      gcvTRUE);

    clsGEN_CODE_PARAMETERS_Initialize(&operandsParameters[3],
                                      gcvFALSE,
                                      gcvTRUE);

    data = &OperandsParameters[0].rOperands[0];
    if(clmIsElementTypePacked(data->dataType.elementType) &&
       data->isReg &&
       !clIsDefaultComponentSelection(&data->u.reg.componentSelection)) {
        clsIOPERAND_New(Compiler, intermIOperand, data->dataType);
        clsLOPERAND_InitializeUsingIOperand(lOperand, intermIOperand);
        status = clGenAssignCode(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 lOperand,
                                 data);
        if (gcmIS_ERROR(status)) return status;
        clsROPERAND_InitializeUsingIOperand(dataOperand, intermIOperand);
        data = dataOperand;
    }

    offset = &OperandsParameters[1].rOperands[0];
    if(clmIsElementTypePacked(offset->dataType.elementType) &&
       offset->isReg &&
       !clIsDefaultComponentSelection(&offset->u.reg.componentSelection)) {
        clsIOPERAND_New(Compiler, intermIOperand, offset->dataType);
        clsLOPERAND_InitializeUsingIOperand(lOperand, intermIOperand);
        status = clGenAssignCode(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 lOperand,
                                 offset);
        if (gcmIS_ERROR(status)) return status;
        clsROPERAND_InitializeUsingIOperand(offsetOperand, intermIOperand);
        offset = offsetOperand;
    }

    /* opcode operand */
    gcmONERROR(cloCOMPILER_CreateDecl(Compiler,
                                      T_INT,
                                      gcvNULL,
                                      clvQUALIFIER_CONST,
                                      clvQUALIFIER_NONE,
                                      decl));
    gcmONERROR(clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                                       &operandsParameters[0],
                                                       decl));

    clsROPERAND_InitializeScalarConstant(&operandsParameters[0].rOperands[0],
                                         clmGenCodeDataType(T_INT),
                                         int,
                                         clvOPCODE_PARAM_CHAIN);

    /* parameter chain variable operand */
    clsGEN_CODE_PARAMETERS_Initialize(&operandsParameters[1],
                                      gcvFALSE,
                                      gcvTRUE);

    gcmONERROR(clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                                       &operandsParameters[1],
                                                       decl));

    clsIOPERAND_New(Compiler, iOperand, operandsParameters[1].dataTypes[0].def);
    clsROPERAND_InitializeUsingIOperand(&operandsParameters[1].rOperands[0], iOperand);

    /* source operand 1 */
    status = cloIR_SET_GetMember(Compiler,
                                 PolynaryExpr->operands,
                                 2,
                                 &addressArg);
    gcmONERROR(clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                                       &operandsParameters[2],
                                                       &(((cloIR_EXPR) addressArg)->decl)));
    operandsParameters[2].rOperands[0] = OperandsParameters[2].rOperands[0];

    /* source operand 2 */
    gcmONERROR(clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                                       &operandsParameters[3],
                                                       decl));

    if(clmCODE_GENERATOR_IsVectorMemoryDeref(CodeGenerator)) {
       dataTypeSize = (gctUINT)clGEN_CODE_DataTypeByteSize(Compiler, OperandsParameters[0].dataTypes[0].def);
    }
    else {
       dataTypeSize = gcGetAddressableUnitSize(OperandsParameters[0].dataTypes[0].def);
    }
    gcmONERROR(clGenScaledIndexOperand(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       offset,
                                       dataTypeSize,
                                       gcvTRUE,
                                       &operandsParameters[3].rOperands[0]));
    gcmONERROR(clGenBuiltInAsmCode(Compiler,
                                   CodeGenerator,
                                   PolynaryExpr,
                                   4,
                                   operandsParameters,
                                   gcvNULL));

    /* need to release the operands corresponding to vstore pointer and offset as it cannot be reused in the
       following asm instruction */
    clsGEN_CODE_PARAMETERS_Finalize(&operandsParameters[2]);
    clsGEN_CODE_PARAMETERS_Initialize(&operandsParameters[2],
                                      gcvFALSE,
                                      gcvTRUE);
    gcmONERROR(clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                                       &operandsParameters[2],
                                                       decl));
    clsGEN_CODE_PARAMETERS_Finalize(&operandsParameters[3]);
    clsGEN_CODE_PARAMETERS_Initialize(&operandsParameters[3],
                                      gcvFALSE,
                                      gcvTRUE);

    /* transfer previous parameter chain variable as second operand in current asm */
    operandsParameters[2].rOperands[0] = operandsParameters[1].rOperands[0];
    clsIOPERAND_New(Compiler, iOperand, operandsParameters[1].dataTypes[0].def);
    clsROPERAND_InitializeUsingIOperand(&operandsParameters[1].rOperands[0], iOperand);

    /* source operand 3 */
    status = cloIR_SET_GetMember(Compiler,
                                 PolynaryExpr->operands,
                                 3,
                                 &addressArg);
    gcmONERROR(clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                                       &operandsParameters[3],
                                                       &(((cloIR_EXPR) addressArg)->decl)));
    operandsParameters[3].rOperands[0] = data[0];

    gcmONERROR(clGenBuiltInAsmCode(Compiler,
                                   CodeGenerator,
                                   PolynaryExpr,
                                   4,
                                   operandsParameters,
                                   gcvNULL));

    /* need to release the operand corresponding to vstore value as it cannot be reused in the
       following asm instruction */
    clsGEN_CODE_PARAMETERS_Finalize(&operandsParameters[3]);
    clsGEN_CODE_PARAMETERS_Initialize(&operandsParameters[3],
                                      gcvFALSE,
                                      gcvTRUE);
    gcmONERROR(clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                                       &operandsParameters[3],
                                                       decl));

    /* opcode operand */
    clsROPERAND_InitializeScalarConstant(&operandsParameters[0].rOperands[0],
                                         clmGenCodeDataType(T_INT),
                                         int,
                                         clvOPCODE_INTRINSIC_ST);

    /* transfer previous parameter chain variable as third operand in current asm */
    operandsParameters[3].rOperands[0] = operandsParameters[1].rOperands[0];
    clsIOPERAND_New(Compiler, iOperand, operandsParameters[1].dataTypes[0].def);
    clsROPERAND_InitializeUsingIOperand(&operandsParameters[1].rOperands[0], iOperand);

    switch(clmGEN_CODE_vectorSize_GET(OperandsParameters[0].dataTypes[0].def)) {
    case 2:
        intrinsicsKind = CL_VIR_IK_evis_vstore2;
        break;

    case 3:
        intrinsicsKind = CL_VIR_IK_evis_vstore3;
        break;

    case 4:
        intrinsicsKind = CL_VIR_IK_evis_vstore4;
        break;

    case 8:
        intrinsicsKind = CL_VIR_IK_evis_vstore8;
        break;

    case 16:
        intrinsicsKind = CL_VIR_IK_evis_vstore16;
        break;

    default:
        gcmASSERT(0);
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmONERROR(status);
    }

    clsROPERAND_InitializeScalarConstant(&operandsParameters[2].rOperands[0],
                                         clmGenCodeDataType(T_INT),
                                         int,
                                         intrinsicsKind);

    gcmONERROR(clGenBuiltInAsmCode(Compiler,
                                   CodeGenerator,
                                   PolynaryExpr,
                                   4,
                                   operandsParameters,
                                   gcvNULL));

OnError:
    clsGEN_CODE_PARAMETERS_Finalize(&operandsParameters[0]);
    clsGEN_CODE_PARAMETERS_Finalize(&operandsParameters[1]);
    clsGEN_CODE_PARAMETERS_Finalize(&operandsParameters[2]);
    clsGEN_CODE_PARAMETERS_Finalize(&operandsParameters[3]);
    return status;
}
#endif

static gceSTATUS
_GenVstoreCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    clsLOPERAND lOperand[1];
    clsROPERAND scaledIndex[1];
    clsROPERAND operandBuffer[1];
    clsROPERAND *rOperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand == gcvNULL);

    if(clmIsElementTypePacked(OperandsParameters[0].rOperands[0].dataType.elementType)) {
#if !_GEN_PACKED_LOAD_STORE_AS_BUILTIN
        return _GenPackedVstoreFuncCall(Compiler,
                                        CodeGenerator,
                                        PolynaryExpr,
                                        OperandCount,
                                        OperandsParameters);
#else
        return _GenPackedVstoreAsmCode(Compiler,
                                       CodeGenerator,
                                       PolynaryExpr,
                                       OperandCount,
                                       OperandsParameters);
#endif
    }

    status = clGenScaledIndexOperand(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     &OperandsParameters[1].rOperands[0],
                                     gcGetAddressableUnitSize(OperandsParameters[0].rOperands[0].dataType),
                                     gcvTRUE,
                                     scaledIndex);
    if (gcmIS_ERROR(status)) return status;

    rOperand = &OperandsParameters[2].rOperands[0];
    if(!clIsIntegerZero(&OperandsParameters[2].dataTypes[0].byteOffset)) { /* byte offset is non-zero */
       if(!scaledIndex->isReg) { /*index is a constant */
          status = clUpdateAddressOffset(Compiler,
                                         PolynaryExpr->exprBase.base.lineNo,
                                         PolynaryExpr->exprBase.base.stringNo,
                                         clGetIntegerValue(scaledIndex),
                                         &OperandsParameters[2].dataTypes[0].byteOffset,
                                         scaledIndex);
          if (gcmIS_ERROR(status)) return status;
       }
       else {
          clsIOPERAND intermIOperand[1];

          clsIOPERAND_New(Compiler, intermIOperand, clmGenCodeDataType(T_INT));
          status = clGenGenericCode2(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ADD,
                                     intermIOperand,
                                     rOperand,
                                     &OperandsParameters[2].dataTypes[0].byteOffset);
          if (gcmIS_ERROR(status)) return status;
          clsROPERAND_InitializeUsingIOperand(operandBuffer, intermIOperand);
          rOperand = operandBuffer;
       }
    }

    clsLOPERAND_InitializeUsingROperand(lOperand, rOperand);
    status = clGenStoreCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                &OperandsParameters[0].rOperands[0],
                                lOperand,
                                OperandsParameters[2].rOperands[0].dataType,
                                scaledIndex);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenVloadHalfCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    clsROPERAND scaledIndex[1];
    clsIOPERAND iOperand[1];
        clsROPERAND operandBuffer[1];
        clsROPERAND *rOperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    iOperand[0] = *IOperand;
    iOperand->dataType.elementType = clvTYPE_HALF;
    status = clGenScaledIndexOperand(Compiler,
                                         PolynaryExpr->exprBase.base.lineNo,
                                         PolynaryExpr->exprBase.base.stringNo,
                                         &OperandsParameters[0].rOperands[0],
                                         gcGetAddressableUnitSize(iOperand->dataType),
                                         gcvTRUE,
                                         scaledIndex);
        if (gcmIS_ERROR(status)) return status;

    rOperand = &OperandsParameters[1].rOperands[0];
    if(!clIsIntegerZero(&OperandsParameters[1].dataTypes[0].byteOffset)) { /* byte offset is non-zero */
       if(!scaledIndex->isReg) { /*index is a constant */
          status = clUpdateAddressOffset(Compiler,
                                         PolynaryExpr->exprBase.base.lineNo,
                                         PolynaryExpr->exprBase.base.stringNo,
                                         clGetIntegerValue(scaledIndex),
                                         &OperandsParameters[1].dataTypes[0].byteOffset,
                                         scaledIndex);
          if (gcmIS_ERROR(status)) return status;
       }
       else {
          clsIOPERAND intermIOperand[1];

          clsIOPERAND_New(Compiler, intermIOperand, clmGenCodeDataType(T_INT));
          status = clGenGenericCode2(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ADD,
                                     intermIOperand,
                                     rOperand,
                                     &OperandsParameters[1].dataTypes[0].byteOffset);
          if (gcmIS_ERROR(status)) return status;
          clsROPERAND_InitializeUsingIOperand(operandBuffer, intermIOperand);
          rOperand = operandBuffer;
       }
    }
    status = clGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               clvOPCODE_LOAD,
                               iOperand,
                               rOperand,
                               scaledIndex);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenVstoreHalfCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    clsLOPERAND lOperand[1];
    clsROPERAND scaledIndex[1];
    clsROPERAND rOperand[1];
        clsROPERAND operandBuffer[1];
        clsROPERAND *addrOperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand == gcvNULL);

    rOperand[0] = OperandsParameters[0].rOperands[0];
    rOperand->dataType.elementType = clvTYPE_HALF;
    status = clGenScaledIndexOperand(Compiler,
                                         PolynaryExpr->exprBase.base.lineNo,
                                         PolynaryExpr->exprBase.base.stringNo,
                                         &OperandsParameters[1].rOperands[0],
                                         gcGetAddressableUnitSize(rOperand->dataType),
                                         gcvTRUE,
                                         scaledIndex);
    if (gcmIS_ERROR(status)) return status;

    addrOperand = &OperandsParameters[2].rOperands[0];
    if(!clIsIntegerZero(&OperandsParameters[2].dataTypes[0].byteOffset)) { /* byte offset is non-zero */
       if(!scaledIndex->isReg) { /*index is a constant */
          status = clUpdateAddressOffset(Compiler,
                                         PolynaryExpr->exprBase.base.lineNo,
                                         PolynaryExpr->exprBase.base.stringNo,
                                         clGetIntegerValue(scaledIndex),
                                         &OperandsParameters[2].dataTypes[0].byteOffset,
                                         scaledIndex);
          if (gcmIS_ERROR(status)) return status;
       }
       else {
          clsIOPERAND intermIOperand[1];

          clsIOPERAND_New(Compiler, intermIOperand, clmGenCodeDataType(T_INT));
          status = clGenGenericCode2(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_ADD,
                                     intermIOperand,
                                     addrOperand,
                                     &OperandsParameters[2].dataTypes[0].byteOffset);
          if (gcmIS_ERROR(status)) return status;
          clsROPERAND_InitializeUsingIOperand(operandBuffer, intermIOperand);
          addrOperand = operandBuffer;
       }
    }
    clsLOPERAND_InitializeUsingROperand(lOperand, addrOperand);
    status = clGenStoreCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            rOperand,
                            lOperand,
                            OperandsParameters[2].rOperands[0].dataType,
                            scaledIndex);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenUnpackCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    return  clGenGenericCode2(Compiler,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo,
                              clvOPCODE_UNPACK,
                              IOperand,
                              &OperandsParameters[0].rOperands[0],
                              &OperandsParameters[1].rOperands[0]);
}

#endif /* __gc_cl_built_ins_vector_h_ */
