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


#include "gc_cl_scanner.h"

#define T_RESERVED_KEYWORD        T_EOF
#define T_NOT_KEYWORD            T_IDENTIFIER

#define cldScanSupportLong        _SUPPORT_LONG_ULONG_DATA_TYPE

typedef struct _clsKEYWORD
{
    gctCONST_STRING     symbol;
    gctINT              token;
    gctINT              errCount;
    cltLANGUAGE_VERSION languageVersion;
    cleEXTENSION        extension;

}
clsKEYWORD;

static clsKEYWORD KeywordTable[] =
{
    {"aligned",              T_ALIGNED,              0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"endian",               T_ENDIAN,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"vec_type_hint",        T_VEC_TYPE_HINT,        0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"reqd_work_group_size", T_REQD_WORK_GROUP_SIZE, 0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"work_group_size_hint", T_WORK_GROUP_SIZE_HINT, 0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"always_inline",        T_ALWAYS_INLINE,        0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"asm",                  T_RESERVED_KEYWORD,     1,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"auto",                 T_RESERVED_KEYWORD,     1,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"_Bool",                T_BOOL,                 0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"bool",                 T_BOOL,                 0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"bool16",               T_BOOL16,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"bool2",                T_BOOL2,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"bool3",                T_BOOL3,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"bool4",                T_BOOL4,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"bool8",                T_BOOL8,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"char",                 T_CHAR,                 0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"char16",               T_CHAR16,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"char2",                T_CHAR2,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"char3",                T_CHAR3,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"char4",                T_CHAR4,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"char8",                T_CHAR8,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"_Complex",             T_RESERVED_KEYWORD,     1,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"complex",              T_RESERVED_KEYWORD,     1,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"break",                T_BREAK,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"case",                 T_CASE,                 0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"const",                T_CONST,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"constant",             T_CONSTANT,             0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"continue",             T_CONTINUE,             0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"default",              T_DEFAULT,              0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"do",                   T_DO,                   0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"double",               T_DOUBLE,               1,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"double16",             T_DOUBLE16,             1,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"double2",              T_DOUBLE2,              1,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"double3",              T_DOUBLE3,              1,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"double4",              T_DOUBLE4,              1,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"double8",              T_DOUBLE8,              1,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"else",                 T_ELSE,                 0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"enum",                 T_ENUM,                 0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"event_t",              T_EVENT_T,              0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"extern",               T_EXTERN,               1,    clvCL_12, clvEXTENSION_NONE},
    {"float",                T_FLOAT,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"float16",              T_FLOAT16,              0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"float2",               T_FLOAT2,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"float3",               T_FLOAT3,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"float4",               T_FLOAT4,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"float8",               T_FLOAT8,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"for",                  T_FOR,                  0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"global",               T_GLOBAL,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"goto",                 T_GOTO,                 0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"half",                 T_HALF,                 0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"half16",               T_HALF16,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"half2",                T_HALF2,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"half3",                T_HALF3,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"half4",                T_HALF4,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"half8",                T_HALF8,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"if",                   T_IF,                   0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"image1d_t",            T_IMAGE1D_T,            0,    clvCL_12, clvEXTENSION_NONE},
    {"image1d_array_t",      T_IMAGE1D_ARRAY_T,      0,    clvCL_12, clvEXTENSION_NONE},
    {"image1d_buffer_t",     T_IMAGE1D_BUFFER_T,     0,    clvCL_12, clvEXTENSION_NONE},
    {"image2d_t",            T_IMAGE2D_T,            0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"image2d_array_t",      T_IMAGE2D_ARRAY_T,      0,    clvCL_12, clvEXTENSION_NONE},
    {"image3d_t",            T_IMAGE3D_T,            0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"_Imaginary",           T_RESERVED_KEYWORD,     1,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"imaginary",            T_RESERVED_KEYWORD,     1,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"inline",               T_INLINE,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"int",                  T_INT,                  0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"int16",                T_INT16,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"int2",                 T_INT2,                 0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"int3",                 T_INT3,                 0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"int4",                 T_INT4,                 0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"int8",                 T_INT8,                 0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"interface",            T_RESERVED_KEYWORD,     1,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"intptr_t",             T_INTPTR_T,             0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"kernel",               T_KERNEL,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"local",                T_LOCAL,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"long",                 T_LONG,                 0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"long16",               T_LONG16,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"long2",                T_LONG2,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"long3",                T_LONG3,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"long4",                T_LONG4,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"long8",                T_LONG8,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"noinline",             T_RESERVED_KEYWORD,     1,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"packed",               T_PACKED,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"private",              T_PRIVATE,              0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"ptrdiff_t",            T_PTRDIFF_T,            0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"public",               T_RESERVED_KEYWORD,     1,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"quad",                 T_QUAD,                 1,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"quad16",               T_QUAD16,               1,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"quad2",                T_QUAD2,                1,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"quad3",                T_QUAD3,                1,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"quad4",                T_QUAD4,                1,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"quad8",                T_QUAD8,                1,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"register",             T_RESERVED_KEYWORD,     1,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"restrict",             T_RESTRICT,             0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"return",               T_RETURN,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"sampler_t",            T_SAMPLER_T,            0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"short",                T_SHORT,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"short16",              T_SHORT16,              0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"short2",               T_SHORT2,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"short3",               T_SHORT3,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"short4",               T_SHORT4,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"short8",               T_SHORT8,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"signed",               T_RESERVED_KEYWORD,     1,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"sizeof",               T_SIZEOF,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"vec_step",             T_VEC_STEP,             0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"size_t",               T_SIZE_T,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"static",               T_STATIC,               1,    clvCL_12, clvEXTENSION_NONE},
    {"struct",               T_STRUCT,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"switch",               T_SWITCH,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"typedef",              T_TYPEDEF,              0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"uchar",                T_UCHAR,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"uchar16",              T_UCHAR16,              0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"uchar2",               T_UCHAR2,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"uchar3",               T_UCHAR3,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"uchar4",               T_UCHAR4,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"uchar8",               T_UCHAR8,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"uint",                 T_UINT,                 0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"uint16",               T_UINT16,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"uint2",                T_UINT2,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"uint3",                T_UINT3,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"uint4",                T_UINT4,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"uint8",                T_UINT8,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"uintptr_t",            T_UINTPTR_T,            0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"ulong",                T_ULONG,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"ulong16",              T_ULONG16,              0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"ulong2",               T_ULONG2,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"ulong3",               T_ULONG3,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"ulong4",               T_ULONG4,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"ulong8",               T_ULONG8,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"union",                T_UNION,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"unsigned",             T_UNSIGNED,             0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"ushort",               T_USHORT,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"ushort16",             T_USHORT16,             0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"ushort2",              T_USHORT2,              0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"ushort3",              T_USHORT3,              0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"ushort4",              T_USHORT4,              0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"ushort8",              T_USHORT8,              0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"void",                 T_VOID,                 0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"volatile",             T_VOLATILE,             0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"while",                T_WHILE,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"read_only",            T_READ_ONLY,            0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"write_only",           T_WRITE_ONLY,           0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"read_write",           T_RESERVED_KEYWORD,     1,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"__attribute__",        T_ATTRIBUTE__,          0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"__constant",           T_CONSTANT,             0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"__global",             T_GLOBAL,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"__kernel",             T_KERNEL,               0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"__local",              T_LOCAL,                0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"__private",            T_PRIVATE,              0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"__read_only",          T_READ_ONLY,            0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"__write_only",         T_WRITE_ONLY,           0,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"__read_write",         T_RESERVED_KEYWORD,     1,    clvCL_11|clvCL_12, clvEXTENSION_NONE},
    {"_viv_bool_packed",     T_BOOL_PACKED,         0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_bool2_packed",    T_BOOL2_PACKED,        0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_bool3_packed",    T_BOOL3_PACKED,        0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_bool4_packed",    T_BOOL4_PACKED,        0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_bool8_packed",    T_BOOL8_PACKED,        0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_bool16_packed",   T_BOOL16_PACKED,       0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_bool32_packed",   T_BOOL32_PACKED,       0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_char_packed",     T_CHAR_PACKED,         0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_char2_packed",    T_CHAR2_PACKED,        0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_char3_packed",    T_CHAR3_PACKED,        0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_char4_packed",    T_CHAR4_PACKED,        0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_char8_packed",    T_CHAR8_PACKED,        0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_char16_packed",   T_CHAR16_PACKED,       0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_char32_packed",   T_CHAR32_PACKED,       0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_uchar_packed",    T_UCHAR_PACKED,        0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_uchar2_packed",   T_UCHAR2_PACKED,       0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_uchar3_packed",   T_UCHAR3_PACKED,       0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_uchar4_packed",   T_UCHAR4_PACKED,       0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_uchar8_packed",   T_UCHAR8_PACKED,       0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_uchar16_packed",  T_UCHAR16_PACKED,      0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_uchar32_packed",  T_UCHAR32_PACKED,      0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_short_packed",    T_SHORT_PACKED,        0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_short2_packed",   T_SHORT2_PACKED,       0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_short3_packed",   T_SHORT3_PACKED,       0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_short4_packed",   T_SHORT4_PACKED,       0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_short8_packed",   T_SHORT8_PACKED,       0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_short16_packed",  T_SHORT16_PACKED,      0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_short32_packed",  T_SHORT32_PACKED,      0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_ushort_packed",   T_USHORT_PACKED,       0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_ushort2_packed",  T_USHORT2_PACKED,      0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_ushort3_packed",  T_USHORT3_PACKED,      0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_ushort4_packed",  T_USHORT4_PACKED,      0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_ushort8_packed",  T_USHORT8_PACKED,      0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_ushort16_packed", T_USHORT16_PACKED,     0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_ushort32_packed", T_USHORT32_PACKED,     0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_half_packed",     T_HALF_PACKED,         0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_half2_packed",    T_HALF2_PACKED,        0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_half3_packed",    T_HALF3_PACKED,        0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_half4_packed",    T_HALF4_PACKED,        0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_half8_packed",    T_HALF8_PACKED,        0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_half16_packed",   T_HALF16_PACKED,       0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
    {"_viv_half32_packed",   T_HALF32_PACKED,       0,    clvCL_11|clvCL_12, clvEXTENSION_VIV_VX},
};

#define _cldMaxRepetitiveError   5
#define cldKeywordCount  (sizeof(KeywordTable) / sizeof(clsKEYWORD))

#define _clmSearchKeyword(Symbol, Index, Token)  do { \
     (Index) = _FindKeywordTableIndex(Symbol); \
     if((Index) < 0) { \
        (Token) =  T_NOT_KEYWORD; \
     } \
     else if (KeywordTable[(Index)].languageVersion | _CL_LanguageVersion) { \
        (Token) = KeywordTable[(Index)].token; \
     } \
     else { \
        (Token) =  T_RESERVED_KEYWORD; \
     } \
   } while (gcvFALSE)

static gctCONST_STRING  _IndexKeywordTableEntries[cldNumTerminalTokens];
static cltLANGUAGE_VERSION  _CL_LanguageVersion = clvCL_11;
static cleEXTENSION  _CL_LanguageExtension = clvEXTENSION_NONE;

/** function to compare keyword strings in KeywordTable for qsort **/
static gctINT
_Compare_Keywords(const void *T1, const void *T2)
{
   gctINT result;

   clsKEYWORD *i1, *i2;
   i1 = (clsKEYWORD *)T1;
   i2 = (clsKEYWORD *)T2;
   result = gcoOS_StrCmp(i1->symbol, i2->symbol);
   return (result == gcvSTATUS_SMALLER ? -1 :
         (result == gcvSTATUS_LARGER ? 1 : 0));
}

void
clScanInitLanguageVersion(
   IN gctUINT32 LanguageVersion,
   IN cleEXTENSION Extension
   )
{
   switch(LanguageVersion) {
   case _cldCL1Dot1:
       _CL_LanguageVersion = clvCL_11;
       break;

   case _cldCL1Dot2:
       _CL_LanguageVersion = clvCL_12;
       break;

   default:
       gcmASSERT(0);
       _CL_LanguageVersion = clvCL_11;
       break;
   }
   _CL_LanguageExtension = Extension;
   return;
}

gctCONST_STRING *
clScanInitIndexToKeywordTableEntries(void)
{
   gctSIZE_T i, index;
   gctCONST_STRING reserved= "reserved keyword";

   gcmHEADER();

/*** Sort the keyword table ***/
   clQuickSort(KeywordTable, cldKeywordCount, sizeof (clsKEYWORD), _Compare_Keywords);

/** Reserve the zero entry for T_RESERVED_KEYWORD **/
   gcmASSERT(T_RESERVED_KEYWORD == 0);
   for(i=0; i< cldKeywordCount; i++) {
      _IndexKeywordTableEntries[i] = reserved;
   }
   for(i=0; i< cldKeywordCount; i++) {
     if(KeywordTable[i].token == T_RESERVED_KEYWORD) continue;

     index = KeywordTable[i].token - T_VERY_FIRST_TERMINAL;
     gcmASSERT(index >0 && index < cldNumTerminalTokens);

     _IndexKeywordTableEntries[index] = KeywordTable[i].symbol;
   }

   gcmFOOTER_ARG("*<return>=0x%x", _IndexKeywordTableEntries);
   return _IndexKeywordTableEntries;
}

static gctINT
_SearchKeyword(
IN gctCONST_STRING Symbol
)
{
    gctINT       low, mid, high;
    gceSTATUS    result;

    low = 0;
    high = cldKeywordCount - 1;

    while (low <= high) {
        mid = (low + high) / 2;
        result = gcoOS_StrCmp(Symbol, KeywordTable[mid].symbol);
        if (result == gcvSTATUS_SMALLER) {
            high    = mid - 1;
        }
        else if (result == gcvSTATUS_LARGER) {
            low = mid + 1;
        }
        else {
            gcmASSERT(gcmIS_SUCCESS(result));
            if ((KeywordTable[mid].languageVersion & _CL_LanguageVersion) &&
                (KeywordTable[mid].extension == clvEXTENSION_NONE ||
                (KeywordTable[mid].extension & _CL_LanguageExtension))) {
                return KeywordTable[mid].token;
            }
            else {
                return T_RESERVED_KEYWORD;
            }
        }
    }

    return T_NOT_KEYWORD;
}

static gctINT
_FindKeywordTableIndex(
IN gctCONST_STRING Symbol
)
{
    gctINT       low, mid, high;
    gceSTATUS    result;

    low = 0;
    high = cldKeywordCount - 1;

    while (low <= high) {
        mid = (low + high) / 2;
        result = gcoOS_StrCmp(Symbol, KeywordTable[mid].symbol);
        if (result == gcvSTATUS_SMALLER) {
            high    = mid - 1;
        }
        else if (result == gcvSTATUS_LARGER) {
            low = mid + 1;
        }
        else {
            gcmASSERT(gcmIS_SUCCESS(result));
            return mid;
        }
    }

    return -1;
}

static gctINT _doubleMatrixErrCount = 1;

void
clScanInitErrorHandler(
IN cloCOMPILER Compiler
)
{
   gctSIZE_T i;

   _doubleMatrixErrCount = 1;
   for(i= 0; i < cldKeywordCount; i++) {
      if(KeywordTable[i].errCount > 0) {
         KeywordTable[i].errCount = 1;
      }
      else KeywordTable[i].errCount = 0;
   }
}

gctINT
clScanIdentifier(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Symbol,
OUT clsLexToken *Token
)
{
    gceSTATUS    status;
    gctINT        tokenType;
    cleSHADER_TYPE    shaderType;
    cltPOOL_STRING    symbolInPool;
    clsNAME *    typeName;

    gcmASSERT(Token);

    gcmVERIFY_OK(cloCOMPILER_GetShaderType(Compiler, &shaderType));

    Token->lineNo    = LineNo;
    Token->stringNo    = StringNo;

    /* Check as a reserved keyword */
    tokenType = _SearchKeyword(Symbol);

    if (tokenType == T_RESERVED_KEYWORD) {
        Token->type = T_RESERVED_KEYWORD;
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler, LineNo, StringNo,
                                        clvREPORT_ERROR,
                                        "reserved keyword : '%s'",
                                        Symbol));
        return T_RESERVED_KEYWORD;
    }
    else if (tokenType != T_NOT_KEYWORD) {
        Token->type = tokenType;
        switch (tokenType) {
        case T_CONSTANT:
                   Token->u.qualifier = clvQUALIFIER_CONSTANT;
                   break;
        case T_LOCAL:
                   Token->u.qualifier = clvQUALIFIER_LOCAL;
                   break;
        case T_GLOBAL:
                   Token->u.qualifier = clvQUALIFIER_GLOBAL;
                   break;
        case T_PRIVATE:
                   Token->u.qualifier = clvQUALIFIER_PRIVATE;
                   break;

        case T_CONST:
                   Token->u.qualifier = clvQUALIFIER_CONST;
                   break;

        case T_READ_ONLY:
                   Token->u.qualifier = clvQUALIFIER_READ_ONLY;
                   break;

        case T_WRITE_ONLY:
                   Token->u.qualifier = clvQUALIFIER_WRITE_ONLY;
                   break;

        case T_VOLATILE:
                   Token->u.qualifier = clvSTORAGE_QUALIFIER_VOLATILE;
                   break;

        case T_RESTRICT:
                   Token->u.qualifier = clvSTORAGE_QUALIFIER_RESTRICT;
                   break;

        case T_STATIC:
                   Token->u.qualifier = clvSTORAGE_QUALIFIER_STATIC;
                   break;

        case T_EXTERN:
                   Token->u.qualifier = clvSTORAGE_QUALIFIER_EXTERN;
                   break;

        default:
                   Token->u.qualifier = clvQUALIFIER_NONE;
                   break;

        }

        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                      clvDUMP_SCANNER,
                                      "<TOKEN line=\"%d\" string=\"%d\""
                                      " type=\"keyword\" symbol=\"%s\" />",
                                      LineNo,
                                      StringNo,
                                      Symbol));

        return tokenType;
    }

    status = cloCOMPILER_AllocatePoolString(Compiler,
                        Symbol,
                        &symbolInPool);
    if (gcmIS_ERROR(status)) return T_EOF;

    if (cloCOMPILER_GetScannerState(Compiler) == clvSCANNER_NORMAL) {
       /* Check as a type name */
      status = cloCOMPILER_SearchName(Compiler,
                      symbolInPool,
                      gcvTRUE,
                      &typeName);
      if (status == gcvSTATUS_OK && typeName->type == clvTYPE_NAME) {
        Token->type    = T_TYPE_NAME;
        Token->u.typeName = typeName;

        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                          clvDUMP_SCANNER,
                          "<TOKEN line=\"%d\" string=\"%d\" type=\"typeName\" symbol=\"%s\" />",
                          LineNo,
                          StringNo,
                          Symbol));
        return T_TYPE_NAME;
      }
    }

    /* Treat as an identifier */
    Token->type        = T_IDENTIFIER;
    Token->u.identifier.name = symbolInPool;
    Token->u.identifier.ptrDscr = gcvNULL;
    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_SCANNER,
                      "<TOKEN line=\"%d\" string=\"%d\" type=\"identifier\" symbol=\"%s\" />",
                      LineNo,
                      StringNo,
                      Token->u.identifier.name));
    return T_IDENTIFIER;
}

gctINT
clScanReservedDataType(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Symbol,
OUT clsLexToken * Token
)
{
    gceSTATUS  status;
    gctINT    tokenType;
    gctINT  key;
    cltPOOL_STRING    symbolInPool;

    gcmASSERT(Token);

    Token->lineNo    = LineNo;
    Token->stringNo    = StringNo;

    /* Check as a reserved keyword */
    _clmSearchKeyword(Symbol, key, tokenType);

    if (tokenType == T_RESERVED_KEYWORD) {
        Token->type = T_RESERVED_KEYWORD;
        gcmASSERT(key >= 0);
        if(KeywordTable[key].errCount < _cldMaxRepetitiveError) {
              if(++KeywordTable[key].errCount == _cldMaxRepetitiveError) {
                 gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                 LineNo, StringNo, clvREPORT_ERROR,
                                                 "unsupported reserved data type : '%s'\n"
                                                 "... further like errors suppressed ...",
                                                 Symbol));
              }
              else {
                  gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                  LineNo, StringNo, clvREPORT_ERROR,
                                                  "unsupported reserved data type : '%s'",
                                                  Symbol));
              }
        }
        return T_RESERVED_KEYWORD;
    }
    if (tokenType != T_NOT_KEYWORD) {
        Token->type = tokenType;
        gcmASSERT(key >= 0);
        if (KeywordTable[key].errCount &&
            KeywordTable[key].errCount < _cldMaxRepetitiveError) {
              if(++KeywordTable[key].errCount == _cldMaxRepetitiveError) {
                 gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                 LineNo, StringNo, clvREPORT_ERROR,
                                                 "unsupported reserved data type : '%s'\n"
                                                 "... further like errors suppressed ...",
                                                 Symbol));
              }
              else {
                  gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                  LineNo, StringNo, clvREPORT_ERROR,
                                                  "unsupported reserved data type : '%s'",
                                                  Symbol));
              }
        }
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                      clvDUMP_SCANNER,
                                      "<TOKEN line=\"%d\" string=\"%d\" type=\"keyword\" symbol=\"%s\" />",
                                      LineNo,
                                      StringNo,
                                      Symbol));
        return T_RESERVED_DATA_TYPE;
    }
    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                    LineNo,
                                    StringNo,
                                    clvREPORT_ERROR,
                                    "unsupported reserved data type : '%s'",
                                    Symbol));
    if (cloCOMPILER_GetScannerState(Compiler) == clvSCANNER_NORMAL) {
       status = cloCOMPILER_AllocatePoolString(Compiler, Symbol, &symbolInPool);
       if (gcmIS_ERROR(status)) return T_EOF;

       Token->type    = T_TYPE_NAME;
       Token->u.typeName->symbol= symbolInPool;
       return T_TYPE_NAME;
    }
    else {
       Token->type = T_RESERVED_KEYWORD;
       return T_RESERVED_KEYWORD;
    }
}

static gctSTRING
_ScanStrpbrkReverse(
IN gctSTRING InStr,
IN gctSTRING MatchChars
)
{
   gctSTRING matchPtr;
   gctCHAR chr;
   gctSIZE_T len;

   if(InStr == gcvNULL || MatchChars == gcvNULL) return gcvNULL;
   len = gcoOS_StrLen(InStr, gcvNULL);

   while(len > 0) {
     --len;
     chr = InStr[len];
     matchPtr = (gctSTRING) MatchChars;
     while(*matchPtr) {
        if(chr != *matchPtr++) continue;
        return InStr + len;
     }
   }
   return gcvNULL;
}

gctINT
clScanConvToUnsignedType(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Symbol,
OUT clsLexToken * Token
)
{
   gctSTRING symbol;
   gcmASSERT(Token);
   Token->lineNo   = LineNo;
   Token->stringNo = StringNo;

   symbol = _ScanStrpbrkReverse(Symbol, " \t\n");
   if(symbol) {
      symbol++;

      /* Check as a reserved keyword */
      switch(_SearchKeyword(symbol)) {
      case T_INT:
          Token->type = T_UINT;
          break;

      case T_LONG:
          Token->type = T_ULONG;
          break;

      case T_SHORT:
          Token->type = T_USHORT;
          break;

      case T_CHAR:
          Token->type = T_UCHAR;
          break;

      default:
          if(symbol[0] == '\0') {
             Token->type = T_UINT;
             break;
          }
          else {
             gcmASSERT(0);
             return T_EOF;
          }
      }
   }
   else {
      gcmASSERT(0);
      return T_EOF;
   }

   gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                 clvDUMP_SCANNER,
                                 "<TOKEN line=\"%d\" string=\"%d\" type=\"keyword\" symbol=\"%s\" />",
                                 LineNo,
                                 StringNo,
                                 symbol));
   return T_BUILTIN_DATA_TYPE;
}

gctINT
clScanBuiltinDataType(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Symbol,
OUT clsLexToken * Token
)
{
    gceSTATUS  status;
    gctINT    tokenType;
    gctINT    key;
    cltPOOL_STRING    symbolInPool;

    gcmASSERT(Token);
    Token->lineNo    = LineNo;
    Token->stringNo    = StringNo;

    /* Check as a reserved keyword */
    _clmSearchKeyword(Symbol, key, tokenType);

    if (tokenType == T_RESERVED_KEYWORD) {
        Token->type = T_RESERVED_KEYWORD;
        gcmASSERT(key >= 0);

        if(KeywordTable[key].errCount < _cldMaxRepetitiveError) {
            if(++KeywordTable[key].errCount == _cldMaxRepetitiveError) {
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                LineNo, StringNo, clvREPORT_ERROR,
                                                "unsupported built-in data type : '%s'\n",
                                                "... further like errors suppressed ...",
                                                Symbol));
            }
            else {
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                LineNo, StringNo, clvREPORT_ERROR,
                                                "unsupported built-in data type : '%s'",
                                                Symbol));
            }
        }
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                        LineNo, StringNo, clvREPORT_ERROR,
                                        "unsupported built-in data type : '%s'",
                                        Symbol));
        return T_RESERVED_KEYWORD;
    }
    if (tokenType != T_NOT_KEYWORD) {
        Token->type = tokenType;
        gcmASSERT(key >= 0);
        if (KeywordTable[key].errCount &&
            KeywordTable[key].errCount < _cldMaxRepetitiveError) {
              if(++KeywordTable[key].errCount == _cldMaxRepetitiveError) {
                 gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                 LineNo, StringNo, clvREPORT_ERROR,
                                                 "unsupported built-in data type : '%s'\n",
                                                 "... further like errors suppressed ...",
                                                 Symbol));
              }
              else {
                  gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                  LineNo, StringNo, clvREPORT_ERROR,
                                                  "unsupported built-in data type : '%s'",
                                                  Symbol));
              }
        }
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                      clvDUMP_SCANNER,
                                      "<TOKEN line=\"%d\" string=\"%d\" type=\"keyword\" symbol=\"%s\" />",
                                      LineNo,
                                      StringNo,
                                      Symbol));
        return T_BUILTIN_DATA_TYPE;
    }

    if (cloCOMPILER_GetScannerState(Compiler) == clvSCANNER_NORMAL) {
        clsNAME *typeName;

        status = cloCOMPILER_AllocatePoolString(Compiler,
                                                Symbol,
                                                &symbolInPool);
        if (gcmIS_ERROR(status)) return T_EOF;

        /* Check as a type name */
        status = cloCOMPILER_SearchName(Compiler,
                                        symbolInPool,
                                        gcvTRUE,
                                        &typeName);
        if (status == gcvSTATUS_OK && typeName->type == clvTYPE_NAME) {
            Token->type    = T_TYPE_NAME;
            Token->u.typeName = typeName;

            gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                          clvDUMP_SCANNER,
                                          "<TOKEN line=\"%d\" string=\"%d\" type=\"typeName\" symbol=\"%s\" />",
                                          LineNo,
                                          StringNo,
                                          Symbol));
            return T_TYPE_NAME;
        }
    }

    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                    LineNo,
                                    StringNo,
                                    clvREPORT_ERROR,
                                    "unsupported built-in data type : '%s'",
                                    Symbol));
    Token->type = T_RESERVED_KEYWORD;
    return T_RESERVED_KEYWORD;
}

gctINT
clScanVivPackedDataType(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Symbol,
OUT clsLexToken * Token
)
{
    gceSTATUS  status;
    gctINT    tokenType;
    gctINT    key;
    cltPOOL_STRING    symbolInPool;

    gcmASSERT(Token);
    Token->lineNo    = LineNo;
    Token->stringNo    = StringNo;

    /* Check as a reserved keyword */
    _clmSearchKeyword(Symbol, key, tokenType);

    if (tokenType == T_RESERVED_KEYWORD) {
        Token->type = T_RESERVED_KEYWORD;
        gcmASSERT(key >= 0);

        if(KeywordTable[key].errCount < _cldMaxRepetitiveError) {
            if(++KeywordTable[key].errCount == _cldMaxRepetitiveError) {
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                LineNo, StringNo, clvREPORT_ERROR,
                                                "unsupported vivante packed data type : '%s'\n",
                                                "... further like errors suppressed ...",
                                                Symbol));
            }
            else {
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                LineNo, StringNo, clvREPORT_ERROR,
                                                "unsupported vivante packed data type : '%s'",
                                                Symbol));
            }
        }
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                        LineNo, StringNo, clvREPORT_ERROR,
                                        "unsupported vivante packed data type : '%s'",
                                        Symbol));
        return T_RESERVED_KEYWORD;
    }
    if (tokenType != T_NOT_KEYWORD) {
        Token->type = tokenType;
        gcmASSERT(key >= 0);
        if (KeywordTable[key].errCount &&
            KeywordTable[key].errCount < _cldMaxRepetitiveError) {
              if(++KeywordTable[key].errCount == _cldMaxRepetitiveError) {
                 gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                 LineNo, StringNo, clvREPORT_ERROR,
                                                 "unsupported vivante packed data type : '%s'\n",
                                                 "... further like errors suppressed ...",
                                                 Symbol));
              }
              else {
                  gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                  LineNo, StringNo, clvREPORT_ERROR,
                                                  "unsupported vivante packed data type : '%s'",
                                                  Symbol));
              }
        }
        gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                      clvDUMP_SCANNER,
                                      "<TOKEN line=\"%d\" string=\"%d\" type=\"keyword\" symbol=\"%s\" />",
                                      LineNo,
                                      StringNo,
                                      Symbol));
        return T_VIV_PACKED_DATA_TYPE;
    }

    if (cloCOMPILER_GetScannerState(Compiler) == clvSCANNER_NORMAL) {
        clsNAME *typeName;

        status = cloCOMPILER_AllocatePoolString(Compiler,
                                                Symbol,
                                                &symbolInPool);
        if (gcmIS_ERROR(status)) return T_EOF;

        /* Check as a type name */
        status = cloCOMPILER_SearchName(Compiler,
                                        symbolInPool,
                                        gcvTRUE,
                                        &typeName);
        if (status == gcvSTATUS_OK && typeName->type == clvTYPE_NAME) {
            Token->type    = T_TYPE_NAME;
            Token->u.typeName = typeName;

            gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                                          clvDUMP_SCANNER,
                                          "<TOKEN line=\"%d\" string=\"%d\" type=\"typeName\" symbol=\"%s\" />",
                                          LineNo,
                                          StringNo,
                                          Symbol));
            return T_TYPE_NAME;
        }
    }

    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                    LineNo,
                                    StringNo,
                                    clvREPORT_ERROR,
                                    "unsupported vivante packed data type : '%s'",
                                    Symbol));
    Token->type = T_RESERVED_KEYWORD;
   return T_RESERVED_KEYWORD;
}

static gctINT nValue[] =  {2, 3, 4, 8, 16};  /* supported n-value of reserved vector data type*/
const gctINT nValueCount = sizeof(nValue) / sizeof(gctINT);

static gctSTRING
_ScanStrpbrk(
IN gctCONST_STRING InStr,
IN gctCONST_STRING MatchChars
)
{

   gctSTRING strPtr;
   gctSTRING matchPtr;
   gctCHAR chr;

   if(InStr == gcvNULL || MatchChars == gcvNULL) return gcvNULL;
   strPtr = (gctSTRING) InStr;
   while(*strPtr)  {
     chr = *strPtr;
     matchPtr = (gctSTRING) MatchChars;
     while(*matchPtr) {
        if(chr != *matchPtr++) continue;
        return strPtr;
     }
     ++strPtr;
   }
   return gcvNULL;
}


gctSIZE_T
clScanStrspn(
IN gctCONST_STRING InStr,
IN gctCONST_STRING MatchChars
)
{

   gctSTRING strPtr;
   gctSTRING matchPtr;
   gctCHAR chr;
   gctSIZE_T count = 0;

   if(InStr == gcvNULL || MatchChars == gcvNULL) return 0;
   strPtr = (gctSTRING) InStr;
   while(*strPtr)  {
     chr = *strPtr;
     matchPtr = (gctSTRING)MatchChars;
     while(*matchPtr) {
        if(chr == *matchPtr) break;
        ++matchPtr;
        continue;
     }
     if(*matchPtr == '\0') return count;
     ++count;
     ++strPtr;
   }
   return count;
}

/* Find n and m of the {float|double}nxm matrix data type */
gceSTATUS
clScanMatrixDimensions(
IN gctSTRING matrixType,
OUT gctUINT *RowCount,
OUT gctUINT *ColumnCount,
OUT gctINT *SquareMatrixType
)
{
    gctSTRING nptr, mptr;
        int i;
        gctINT row, column;
    gctBOOL isFloat;

    *SquareMatrixType = T_EOF;
    isFloat = matrixType[0] == 'f' ? gcvTRUE : gcvFALSE;
    nptr = _ScanStrpbrk(matrixType, "123456789");
    gcmASSERT(nptr);
    gcoOS_StrToInt(nptr, &row);
        gcoOS_StrFindReverse(nptr, 'x', &mptr);
    ++mptr;
    gcmASSERT(*mptr);
    gcoOS_StrToInt(mptr, &column);
    for(i=0; i<nValueCount; i++) {
       if(row != nValue[i]) continue;
        for(i=0; i<nValueCount; i++) {
          if(column != nValue[i]) continue;
              *RowCount = row;
              *ColumnCount = column;
          if(isFloat) {
                if(*RowCount == *ColumnCount) {
              switch(*ColumnCount) {
                  case 2:
                *SquareMatrixType = T_MAT2;
                break;
              case 3:
                *SquareMatrixType = T_MAT3;
                break;
              case 4:
                *SquareMatrixType = T_MAT4;
                break;
              case 8:
                *SquareMatrixType = T_MAT8;
                break;
              case 16:
                *SquareMatrixType = T_MAT16;
                break;
              }
                }
              }
              return gcvSTATUS_TRUE;
       }
    }
        if(row >= nValue[0] && row <= nValue[nValueCount -1] &&
           column >= nValue[0] && column <= nValue[nValueCount -1]) {
       *RowCount = row;
       *ColumnCount = column;
           return gcvSTATUS_TRUE;
    }
    else {
       *RowCount = 0;
       *ColumnCount = 0;
    }

        return gcvSTATUS_FALSE;

}

gctINT
clScanMatrixType(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctINT TokenType,
IN gctSTRING Symbol,
OUT clsLexToken * Token
)
{
    gceSTATUS    status;
    cltPOOL_STRING    symbolInPool;
        gctUINT row, column;
    gctINT squareMatrixType;

    gcmASSERT(Token);

    Token->lineNo    = LineNo;
    Token->stringNo    = StringNo;

    /* Get matrix dimension */
    status = clScanMatrixDimensions(Symbol, &row, &column, &squareMatrixType);
    if(status == gcvTRUE) {
       Token->u.matrixSize.rowCount = (gctUINT8) row;
           Token->u.matrixSize.columnCount = (gctUINT8) column;
       if(TokenType == T_DOUBLENXM) {
          if(_doubleMatrixErrCount < _cldMaxRepetitiveError) {
              if(++_doubleMatrixErrCount == _cldMaxRepetitiveError) {
                 gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                 LineNo, StringNo, clvREPORT_ERROR,
                             "unsupported reserved matrix data type : '%s'\n",
                             "... further like errors suppressed ...",
                                 Symbol));
              }
              else {
                  gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                  LineNo, StringNo, clvREPORT_ERROR,
                              "unsupported reserved matrix data type : '%s'",
                                  Symbol));
              }
          }
       }
       gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                    clvDUMP_SCANNER,
                    "<TOKEN line=\"%d\" string=\"%d\" type=\"keyword\" symbol=\"%s\" />",
                    LineNo,
                StringNo,
                Symbol));
       if(squareMatrixType == T_EOF) Token->type = TokenType;
           else Token->type = squareMatrixType;
       return TokenType;
    }

    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                    LineNo,
                    StringNo,
                    clvREPORT_ERROR,
                    "reserved matrix data type : '%s'",
                    Symbol));

    if (cloCOMPILER_GetScannerState(Compiler) == clvSCANNER_NORMAL) {
        status = cloCOMPILER_AllocatePoolString(Compiler,
                                                Symbol,
                                                &symbolInPool);
        if (gcmIS_ERROR(status)) return T_EOF;

        Token->type = T_TYPE_NAME;
        Token->u.typeName->symbol= symbolInPool;
        return T_TYPE_NAME;
    }
    else {
        Token->type = T_RESERVED_KEYWORD;
        return T_RESERVED_KEYWORD;
    }
}

gctINT
clScanBoolConstant(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctBOOL Value,
OUT clsLexToken * Token
)
{
    gcmASSERT(Token);

    Token->lineNo    = LineNo;
    Token->stringNo    = StringNo;
    Token->type    = T_BOOL;
    Token->u.constant.boolValue = Value;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_SCANNER,
                      "<TOKEN line=\"%d\" string=\"%d\" type=\"boolConstant\" value=\"%s\" />",
                      LineNo,
                      StringNo,
                      (Token->u.constant.boolValue)? "true" : "false"));
    return T_BOOLCONSTANT;
}

#define cldIntegerSuffix  "uUlL"
#define cldFloatingSuffix  "fFlL"
#define cldOctalDigits  "01234567"
#define cldHexadecimalDigits "0123456789ABCDEFabcdef"
#define cldINT_MAX      0x7FFFFFFF
#define cldUINT_MAX     0xFFFFFFFF
#define cldLONG_MAX     0x7FFFFFFFFFFFFFFFLL
#define cldULONG_MAX    0xFFFFFFFFFFFFFFFFULL

static gctSTRING
_ConvStringToIntConstant(
IN gctSTRING String,
IN gctINT Base,
OUT gctINT64 *IntConstant
)
{
    gctUINT32 digit = 0;
    gctUINT64 result = 0;
    gctCHAR ch;
    gctSTRING strEnd;

    gcmASSERT(String);
    gcmASSERT((Base == 8) || (Base == 10) || (Base == 16));

    strEnd = String;
    while(*strEnd != '\0') {
        ch = *strEnd++;
        switch (Base) {
        case 8:
            if ((ch >= '0') && (ch <= '7')) digit = ch - '0';
            else {
               strEnd = String;
               result = 0;
               goto EXIT;
            }
            break;

        case 10:
            if ((ch >= '0') && (ch <= '9')) digit = ch - '0';
            else {
               strEnd = String;
               result = 0;
               goto EXIT;
            }
            break;

        case 16:
            if ((ch >= 'a') && (ch <= 'f'))    digit = ch - 'a' + 10;
            else if ((ch >= 'A') && (ch <= 'F')) digit = ch - 'A' + 10;
            else if ((ch >= '0') && (ch <= '9')) digit = ch - '0';
            else {
               strEnd = String;
               result = 0;
               goto EXIT;
            }
            break;

        default:
            gcmASSERT(0);
            strEnd = String;
            goto EXIT;
        }

/*NO ERROR CHECKING */
        result = result * Base + digit;
    }

EXIT:  /*error invalid string */
    *IntConstant = result;
    return strEnd;
}

static gctSTRING
_ConvStringToUintConstant(
IN gctSTRING String,
IN gctINT Base,
OUT gctUINT64 *UintConstant
)
{
    gctUINT32 digit = 0;
    gctUINT64 result = 0;
    gctCHAR ch;
    gctSTRING strEnd;

    gcmASSERT(String);
    gcmASSERT((Base == 8) || (Base == 10) || (Base == 16));

    strEnd = String;
    while(*strEnd != '\0') {
        ch = *strEnd++;
        switch (Base) {
        case 8:
            if ((ch >= '0') && (ch <= '7')) digit = ch - '0';
            else {
               strEnd = String;
               result = 0;
               goto EXIT;
            }
            break;

        case 10:
            if ((ch >= '0') && (ch <= '9')) digit = ch - '0';
            else {
               strEnd = String;
               result = 0;
               goto EXIT;
            }
            break;

        case 16:
            if ((ch >= 'a') && (ch <= 'f'))    digit = ch - 'a' + 10;
            else if ((ch >= 'A') && (ch <= 'F')) digit = ch - 'A' + 10;
            else if ((ch >= '0') && (ch <= '9')) digit = ch - '0';
            else {
               strEnd = String;
               result = 0;
               goto EXIT;
            }
            break;

        default:
            gcmASSERT(0);
            strEnd = String;
            goto EXIT;
        }

/*NO ERROR CHECKING */
        result = result * Base + digit;
    }

EXIT:  /*error invalid string */
    *UintConstant = result;
    return strEnd;
}

static gctSTRING
_ConvEscapeSequenceToInt(
IN gctSTRING SeqStart,
IN gctINT64 *IntConstant
)
{
    gctSTRING seqStart;
    gctUINT base;
    gctUINT seqLen;

    gcmASSERT(SeqStart);

    seqStart = SeqStart;
    switch(*seqStart) {
    case 'x':
    case 'X':
       seqLen = clScanStrspn(++seqStart, cldHexadecimalDigits);
       base = 16;
       break;

    default:
       seqLen = clScanStrspn(seqStart, cldOctalDigits);
       seqLen = seqLen > 3 ? 3 : seqLen;
       base = 8;
       break;
    }
    if(seqLen) { /* non empty escape sequence?*/
       gctCHAR saveChr;
       gctSTRING seqEnd;

       saveChr = seqStart[seqLen];
       seqStart[seqLen] = '\0';

       seqEnd = _ConvStringToIntConstant(seqStart, base, IntConstant);
       seqStart[seqLen] = saveChr;
       if(seqEnd != seqStart) return seqEnd;
    }
    *IntConstant = '\0';
    return SeqStart;
}

static gctSTRING
_GetNextCharConstant(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Str,
IN gctCHAR Delimiter,
OUT gctCHAR *CharConstant
)
{
   gctCHAR *strPtr;
   gctCHAR *endPtr;
   gctCHAR chr;
   gctINT64 intConstant;

   if(!Str || !Str[0]) return Str;

   strPtr = Str;
   switch(chr = *strPtr) {
   case '\\':
     switch(chr = *++strPtr) {
     case '\'':
     case '?':
     case '\"':
     case '\\':
         break;

     case 'b':
         chr = '\b';
         break;

     case 'f':
         chr = '\f';
         break;

     case 'n':
         chr = '\n';
         break;

     case 'r':
         chr = '\r';
         break;

     case 't':
         chr = '\t';
         break;

     case 'v':
         chr = '\v';
         break;

     default:
         endPtr = _ConvEscapeSequenceToInt(strPtr, &intConstant);
         if(endPtr == strPtr) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            LineNo,
                            StringNo,
                            clvREPORT_ERROR,
                            "invalid escape sequence %c%s",
                            Delimiter, Str));
            return Str;
         }
         else  if(intConstant < (1 << 8)) { /* valid escape sequence conversion ?*/
            *CharConstant = (gctCHAR) intConstant;
            return endPtr;
         }
         else {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            LineNo,
                            StringNo,
                            clvREPORT_ERROR,
                            "escape sequence \"%s\" is multi-byte",
                            Str));
            return Str;
         }
     }
     break;

  default:
     if(*strPtr == Delimiter) chr = '\0';
     break;
  }
  *CharConstant = chr;
  return ++strPtr;
}

static gctINT
_GetIntegerCharConstant(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Str,
IN gctCHAR Delimiter
)
{
   gctCHAR *strPtr;
   gctCHAR *endPtr;
   gctCHAR chr;
   gctINT intConstant;
   gctINT64 escapedInt;
   gctINT byteCount = 0;
   gctBOOL truncated = gcvFALSE;

   if(!Str || !Str[0]) {
     gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                     LineNo,
                                     StringNo,
                                     clvREPORT_ERROR,
                                     "unterminated integer character constant"));
     return 0;
   }


   intConstant = 0;
   strPtr = Str;
   while((chr = *strPtr) != 0) {
      if(chr == Delimiter) break;
      switch(chr) {
      case '\\':
        switch(chr = *++strPtr) {
        case '\'':
        case '?':
        case '\"':
        case '\\':
            break;

        case 'b':
            chr = '\b';
            break;

        case 'f':
            chr = '\f';
            break;

        case 'n':
            chr = '\n';
            break;

        case 'r':
            chr = '\r';
            break;

        case 't':
            chr = '\t';
            break;

        case 'v':
            chr = '\v';
            break;

        default:
            endPtr = _ConvEscapeSequenceToInt(strPtr, &escapedInt);
            if(endPtr == strPtr) {
               gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                               LineNo,
                               StringNo,
                               clvREPORT_ERROR,
                               "invalid escape sequence %c%s",
                               Delimiter, Str));
               return 0;
            }
            else { /* valid escape sequence conversion ?*/
               chr = (gctCHAR) escapedInt;
            }
            break;
        }
        break;

     default:
        break;
     }
     intConstant = (intConstant << 8) | chr;
     byteCount++;
     if(byteCount > 4) {
        if(truncated == gcvFALSE) {
       gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                           LineNo,
                                           StringNo,
                                           clvREPORT_WARN,
                                           "integer character constant \"%s\" is multi-byte - \n"
                                           "value truncated to the 32-bit integer",
                                           Str));
           truncated = gcvTRUE;
        }
     }
     ++strPtr;
  }
  if(chr != Delimiter) {
     gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                     LineNo,
                                     StringNo,
                                     clvREPORT_ERROR,
                                     "unterminated integer character constant \"%s\"",
                                     Str));
     return 0;
  }
  return intConstant;
}

/** convert in place escape sequences in string literal and return the resulting number of
    characters in the string literal **/
void
clScanConvStringLiteralInPlace(
IN cloCOMPILER Compiler,
IN clsLexToken *StringLiteral
)
{
    gctSTRING strStart, strEnd;
    gctSTRING strPtr;
    gctSTRING newStrStart;

    gcmASSERT(StringLiteral);

    strStart = strPtr = StringLiteral->u.stringLiteral.value + 1;
    strEnd = strStart + StringLiteral->u.stringLiteral.len;
    while(strStart < strEnd) {
       newStrStart = _GetNextCharConstant(Compiler,
                                 StringLiteral->lineNo,
                                 StringLiteral->stringNo,
                                 strStart,
                          StringLiteral->u.stringLiteral.value[0],
                                 strPtr);
       if(newStrStart == strStart) break;  /* error */
       strStart = newStrStart;
       strPtr++;
    }
    StringLiteral->u.stringLiteral.len = strPtr - StringLiteral->u.stringLiteral.value;
    return;
}

gctINT
clScanCharConstant(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Text,
OUT clsLexToken *Token
)
{
    gctINT intValue;

    gcmASSERT(Token);
    gcmASSERT(Text[0] == '\'');

        (void)gcoOS_ZeroMemory((gctPOINTER)&Token->u.constant, sizeof(cluCONSTANT_VALUE));
    Token->lineNo    = LineNo;
    Token->stringNo    = StringNo;
    Token->type    = T_CHAR;

    intValue = _GetIntegerCharConstant(Compiler,
                                  LineNo,
                                  StringNo,
                                  Text + 1,
                                  Text[0]);
    Token->u.constant.intValue = intValue;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_SCANNER,
                      "<TOKEN line=\"%d\" string=\"%d\" type=\"charConstant\""
                      " value=\"%d\" />",
                      LineNo,
                      StringNo,
                      intValue));
    return T_CHARCONSTANT;
}

gctINT
clScanStringLiteral(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Text,
OUT clsLexToken * Token
)
{
    gceSTATUS status;
    gctSTRING strPtr;
    gctSTRING strStart, strEnd;
    gctPOINTER pointer;

    gcmHEADER_ARG("Compiler=0x%x LineNo=%u StringNo=%u Text=%s",
                       Compiler, LineNo, StringNo, Text);

    gcmASSERT(Token && Text);
    gcmASSERT(Text[0] == '\"');

    Token->lineNo    = LineNo;
    Token->stringNo    = StringNo;
    Token->type    = T_STRING_LITERAL;

    strStart = strPtr = Text + 1;
    do {
      strEnd = _GetNextCharConstant(Compiler,
                           LineNo,
                           StringNo,
                           strStart,
                           Text[0],
                           strPtr);
      if(strEnd == strStart) break;
      strStart = strEnd;
      strPtr++;
    } while (gcvTRUE);

    Token->u.stringLiteral.len = strPtr - (Text + 1);
    status = cloCOMPILER_Allocate(Compiler,
                      Token->u.stringLiteral.len,
                      (gctPOINTER *) &pointer);
    if(gcmIS_ERROR(status)) {
       gcmFOOTER();
       return T_EOF;
    }

    Token->u.stringLiteral.value = pointer;
    gcoOS_MemCopy((gctSTRING) pointer, Text + 1, Token->u.stringLiteral.len);

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_SCANNER,
                      "<TOKEN line=\"%d\" string=\"%d\" type=\"stringLiteral\""
                      " value=\"%s\" length=\"%d\" />",
                      LineNo,
                      StringNo,
                      Token->u.stringLiteral.value,
                      Token->u.stringLiteral.len));

    gcmFOOTER_ARG("Token 0x%x", Token);
    return T_STRING_LITERAL;
}

static gctINT
_StringToIntConstant(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING String,
IN gctINT Base,
IN gctINT Index,
OUT clsLexToken * Token
)
{
    gctINT orgIndex = Index;
    gctINT64 result = 0;
    gctSTRING strEnd;

    strEnd = _ConvStringToIntConstant(String + orgIndex, Base, &result);
    Token->u.constant.longValue = result;

    if(strEnd == String + orgIndex) { /*there is error*/
        gctBOOL overFlow = gcvFALSE;

        if(Token->type == T_LONG) {
            overFlow = Token->u.constant.longValue == cldLONG_MAX;
        }
        else {
            overFlow = Token->u.constant.intValue == cldINT_MAX;
        }

        if(overFlow) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            LineNo,
                            StringNo,
                            clvREPORT_ERROR,
                            "too large %s integer: %s",
                            (Base == 8)?
                            "octal" : ((Base == 10)? "decimal" : "hexadecimal"),
                            String + orgIndex));
        }
        else {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            LineNo,
                            StringNo,
                            clvREPORT_ERROR,
                            "invalid %s integer: %s",
                            (Base == 8)?
                            "octal" : ((Base == 10)? "decimal" : "hexadecimal"),
                            String + orgIndex));
        }
    }

    return (gctINT) gcoOS_StrLen(String, gcvNULL);
}

static gctINT
_StringToUintConstant(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING String,
IN gctINT Base,
IN gctINT Index,
OUT clsLexToken * Token
)
{
    gctINT orgIndex = Index;
    gctUINT64 result = 0;
    gctSTRING strEnd;

    strEnd = _ConvStringToUintConstant(String + orgIndex, Base, &result);
    Token->u.constant.ulongValue = result;

    if(strEnd == String + orgIndex) { /*there is error*/
        gctBOOL overFlow = gcvFALSE;

        if(Token->type == T_ULONG) {
            overFlow = Token->u.constant.ulongValue == cldULONG_MAX;
        }
        else {
            overFlow = Token->u.constant.uintValue == cldUINT_MAX;
        }

        if(overFlow) {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            LineNo,
                            StringNo,
                            clvREPORT_ERROR,
                            "too large %s unsigned integer: %s",
                            (Base == 8)?
                            "octal" : ((Base == 10)? "decimal" : "hexadecimal"),
                            String + orgIndex));
        }
        else {
            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                            LineNo,
                            StringNo,
                            clvREPORT_ERROR,
                            "invalid %s unsigned integer: %s",
                            (Base == 8)?
                            "octal" : ((Base == 10)? "decimal" : "hexadecimal"),
                            String + orgIndex));
        }
    }

    return (gctINT) gcoOS_StrLen(String, gcvNULL);
}

static gctSTRING
_ScanIntConstantType(
IN gctSTRING ConstStr,
OUT gctINT *Type,
OUT gctBOOL *IsUnsigned
)
{
  gctINT type = T_INT;
  gctBOOL isUnsigned = gcvFALSE;
  gctSTRING endPtr;

  endPtr = _ScanStrpbrk(ConstStr, cldIntegerSuffix);
  if(endPtr) {
     gctSTRING cPtr = endPtr;
     char ch;
     gctBOOL isLong = gcvFALSE;
     gctBOOL isLongLong = gcvFALSE;

     while((ch = *cPtr++) != '\0') {
       switch(ch) {
       case 'u':
       case 'U':
          isUnsigned = gcvTRUE;
          break;

       case 'l':
       case 'L':
          if(isLong) isLongLong = gcvTRUE;
      else isLong = gcvTRUE;
          break;
       }
     }
#if cldScanSupportLong
     if(isUnsigned) {
       if(isLong || isLongLong) {
         type = T_ULONG;
       }
       else type = T_UINT;
     }
     else if(isLong) {
       type = T_LONG;
     }
#else
     if(isUnsigned) {
       type = T_UINT;
     }
     else {
       type = T_INT;
     }
#endif
  }
  *Type = type;
  *IsUnsigned = isUnsigned;
  return endPtr;
}

static gctSTRING
_ScanFloatConstantType(
IN gctSTRING ConstStr,
IN gctINT *Type
)
{
  gctINT type = T_FLOAT; /*NEED TO CHANGE DEFAULT TO DOUBLE*/
  gctSTRING endPtr;

  endPtr = _ScanStrpbrk(ConstStr, cldFloatingSuffix);
  if(endPtr) {
     gctSTRING cPtr = endPtr;
     char ch;

     while((ch = *cPtr++) != '\0') {
       switch(ch) {
       case 'f':
       case 'F':
          type = T_FLOAT;
          break;

       case 'l':
       case 'L':
          type = T_DOUBLE;
          break;
       }
     }
  }
  *Type = type;
  return endPtr;
}

gctINT
clScanDecIntConstant(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Text,
OUT clsLexToken * Token
)
{
    gctINT index = 0;
    char saveChr = '\0';
    gctSTRING endPtr;
    gctINT type;
    gctBOOL isUnsigned;

    gcmASSERT(Token);
    (void)gcoOS_ZeroMemory((gctPOINTER)&Token->u.constant, sizeof(cluCONSTANT_VALUE));
    Token->lineNo    = LineNo;
    Token->stringNo    = StringNo;

    endPtr = _ScanIntConstantType(Text, &Token->type, &isUnsigned);
    if(endPtr) {
      saveChr = *endPtr;
      *endPtr = '\0';
    }

    if(isUnsigned) {
      type = T_UINTCONSTANT;
      index = _StringToUintConstant(Compiler, LineNo, StringNo, Text, 10, index, Token);

    }
    else {
      type = T_INTCONSTANT;
      index = _StringToIntConstant(Compiler, LineNo, StringNo, Text, 10, index, Token);
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_SCANNER,
                      "<TOKEN line=\"%d\" string=\"%d\" type=\"intConstant\""
                      " format=\"decimal\" value=\"%ld\" />",
                      LineNo,
                      StringNo,
                      Token->u.constant.longValue));
    if(endPtr) {
      *endPtr = saveChr;
    }
    return type;
}

gctINT
clScanOctIntConstant(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Text,
OUT clsLexToken * Token
)
{
    gctINT index = 0;
    char saveChr = '\0';
    gctSTRING endPtr;
    gctINT type;
    gctBOOL isUnsigned;

    gcmASSERT(Token);
    (void)gcoOS_ZeroMemory((gctPOINTER)&Token->u.constant, sizeof(cluCONSTANT_VALUE));
    Token->lineNo    = LineNo;
    Token->stringNo    = StringNo;
    endPtr = _ScanIntConstantType(Text, &Token->type, &isUnsigned);
    if(endPtr) {
        saveChr = *endPtr;
        *endPtr = '\0';
    }
    if(isUnsigned) {
        type = T_UINTCONSTANT;
        index = _StringToUintConstant(Compiler, LineNo, StringNo, Text, 8, index, Token);
    }
    else {
        type = T_INTCONSTANT;
        index = _StringToIntConstant(Compiler, LineNo, StringNo, Text, 8, index, Token);
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_SCANNER,
                      "<TOKEN line=\"%d\" string=\"%d\" type=\"intConstant\""
                      " format=\"octal\" value=\"%ld\" />",
                      LineNo,
                      StringNo,
                      Token->u.constant.longValue));
    if(endPtr) {
        *endPtr = saveChr;
    }
    return type;
}

gctINT
clScanHexIntConstant(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Text,
OUT clsLexToken * Token
)
{
    gctINT index = 2;
    gctSTRING endPtr;
    gctBOOL isUnsigned;
    gctINT type;
    char saveChr = '\0';;

    gcmASSERT(Token);
    (void)gcoOS_ZeroMemory((gctPOINTER)&Token->u.constant, sizeof(cluCONSTANT_VALUE));
    Token->lineNo    = LineNo;
    Token->stringNo    = StringNo;
    endPtr = _ScanIntConstantType(Text, &Token->type, &isUnsigned);
    if(endPtr) {
      saveChr = *endPtr;
      *endPtr = '\0';
    }
    if(isUnsigned) {
       type = T_UINTCONSTANT;
       index = _StringToUintConstant(Compiler, LineNo, StringNo, Text, 16, index, Token);
    }
    else {
       type = T_INTCONSTANT;
       index = _StringToIntConstant(Compiler, LineNo, StringNo, Text, 16, index, Token);
    }

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_SCANNER,
                      "<TOKEN line=\"%d\" string=\"%d\" type=\"intConstant\""
                      " format=\"hexadecimal\" value=\"%ld\" />",
                      LineNo,
                      StringNo,
                      Token->u.constant.longValue));
    if(endPtr) {
      *endPtr = saveChr;
    }
    return type;
}

gctINT
clScanFloatConstant(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Text,
OUT clsLexToken * Token
)
{
    char saveChr ='\0';
    gctSTRING endPtr;

    gcmASSERT(Token);

    (void)gcoOS_ZeroMemory((gctPOINTER)&Token->u.constant, sizeof(cluCONSTANT_VALUE));
    Token->lineNo    = LineNo;
    Token->stringNo    = StringNo;

    endPtr = _ScanFloatConstantType(Text, &Token->type);
    if(endPtr) {
      saveChr = *endPtr;
      *endPtr = '\0';
    }
    gcmVERIFY_OK(gcoOS_StrToFloat(Text, &Token->u.constant.floatValue));

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_SCANNER,
                      "<TOKEN line=\"%d\" string=\"%d\" type=\"floatConstant\""
                      " value=\"%f\" />",
                      LineNo,
                      StringNo,
                      Token->u.constant.floatValue));
    if(endPtr) {
      *endPtr = saveChr;
    }
    return T_FLOATCONSTANT;
}

gctINT
clScanHexFloatConstant(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Text,
OUT clsLexToken * Token
)
{
    gctCHAR saveChr ='\0';
    gctSTRING endPtr;

    gcmASSERT(Token);

    (void)gcoOS_ZeroMemory((gctPOINTER)&Token->u.constant, sizeof(cluCONSTANT_VALUE));
    Token->lineNo    = LineNo;
    Token->stringNo    = StringNo;
    Token->type = T_FLOAT;

    endPtr = Text;
    while(*endPtr)
        endPtr++;

    endPtr--;
    if(*endPtr == 'f' || *endPtr == 'F' ) {
      saveChr = *endPtr;
      *endPtr = '\0';
    }

    gcmVERIFY_OK(gcoOS_HexStrToFloat(Text, &Token->u.constant.floatValue));

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_SCANNER,
                      "<TOKEN line=\"%d\" string=\"%d\" type=\"hexFloatConstant\""
                      " value=\"%f\" />",
                      LineNo,
                      StringNo,
                      Token->u.constant.floatValue));
    if(endPtr)
      *endPtr = saveChr;

    return T_FLOATCONSTANT;
}

gctINT
clScanOperator(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Text,
IN gctINT tokenType,
OUT clsLexToken * Token
)
{
    gcmASSERT(Token);

    Token->lineNo    = LineNo;
    Token->stringNo    = StringNo;
    Token->type    = tokenType;
    Token->u.operator = tokenType;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_SCANNER,
                      "<TOKEN line=\"%d\" string=\"%d\""
                      " type=\"operator\" symbol=\"%s\" />",
                      LineNo,
                      StringNo,
                      Text));
    return tokenType;
}

gctINT
clScanFieldSelection(
IN cloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Symbol,
OUT clsLexToken * Token
)
{
    gceSTATUS    status;
    cltPOOL_STRING    symbolInPool;

    gcmASSERT(Token);

    Token->lineNo    = LineNo;
    Token->stringNo    = StringNo;

    status = cloCOMPILER_AllocatePoolString(Compiler,
                        Symbol,
                        &symbolInPool);
    if (gcmIS_ERROR(status)) return T_EOF;

    Token->type = T_FIELD_SELECTION;
    Token->u.fieldSelection = symbolInPool;

    gcmVERIFY_OK(cloCOMPILER_Dump(Compiler,
                      clvDUMP_SCANNER,
                      "<TOKEN line=\"%d\" string=\"%d\" type=\"fieldSelection\" symbol=\"%s\" />",
                      LineNo,
                      StringNo,
                      Token->u.fieldSelection));
    return T_FIELD_SELECTION;
}

int yywrap(void)
{
    return 1;
}
