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


#ifndef __gc_cl_built_ins_int_h_
#define __gc_cl_built_ins_int_h_

static clsBUILTIN_FUNCTION    IntBuiltinFunctions[] =
{

    {clvEXTENSION_NONE,     "abs",                  T_U_GENTYPE,     1, {T_IU_GENTYPE}, {0}, {1}, 1},
    {clvEXTENSION_NONE,     "rotate",               T_IU_GENTYPE,    2, {T_IU_GENTYPE, T_IU_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "hadd",                 T_IU_GENTYPE,    2, {T_IU_GENTYPE, T_IU_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "rhadd",                T_IU_GENTYPE,    2, {T_IU_GENTYPE, T_IU_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "add_sat",              T_IU_GENTYPE,    2, {T_IU_GENTYPE, T_IU_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "sub_sat",              T_IU_GENTYPE,    2, {T_IU_GENTYPE, T_IU_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "mad_sat",              T_IU_GENTYPE,    3, {T_IU_GENTYPE, T_IU_GENTYPE, T_IU_GENTYPE}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "mad_hi",               T_IU_GENTYPE,    3, {T_IU_GENTYPE, T_IU_GENTYPE, T_IU_GENTYPE}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "mul_hi",               T_IU_GENTYPE,    2, {T_IU_GENTYPE, T_IU_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "mul24",                T_INT,           2, {T_INT, T_INT}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "mul24",                T_INT2,          2, {T_INT2, T_INT2}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "mul24",                T_INT3,          2, {T_INT3, T_INT3}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "mul24",                T_INT4,          2, {T_INT4, T_INT4}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "mul24",                T_INT8,          2, {T_INT8, T_INT8}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "mul24",                T_INT16,         2, {T_INT16, T_INT16}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "mul24",                T_UINT,          2, {T_UINT, T_UINT}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "mul24",                T_UINT2,         2, {T_UINT2, T_UINT2}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "mul24",                T_UINT3,         2, {T_UINT3, T_UINT3}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "mul24",                T_UINT4,         2, {T_UINT4, T_UINT4}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "mul24",                T_UINT8,         2, {T_UINT8, T_UINT8}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "mul24",                T_UINT16,        2, {T_UINT16, T_UINT16}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "mad24",                T_INT,           3, {T_INT, T_INT, T_INT}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "mad24",                T_INT2,          3, {T_INT2, T_INT2, T_INT2}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "mad24",                T_INT3,          3, {T_INT3, T_INT3, T_INT3}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "mad24",                T_INT4,          3, {T_INT4, T_INT4, T_INT4}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "mad24",                T_INT8,          3, {T_INT8, T_INT8, T_INT8}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "mad24",                T_INT16,         3, {T_INT16, T_INT16, T_INT16}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "mad24",                T_UINT,          3, {T_UINT, T_UINT, T_UINT}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "mad24",                T_UINT2,         3, {T_UINT2, T_UINT2, T_UINT2}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "mad24",                T_UINT3,         3, {T_UINT3, T_UINT3, T_UINT3}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "mad24",                T_UINT4,         3, {T_UINT4, T_UINT4, T_UINT4}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "mad24",                T_UINT8,         3, {T_UINT8, T_UINT8, T_UINT8}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "mad24",                T_UINT16,        3, {T_UINT16, T_UINT16, T_UINT16}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "mad24",                T_CHAR,          3, {T_CHAR, T_CHAR, T_CHAR}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "mad24",                T_CHAR2,         3, {T_CHAR2, T_CHAR2, T_CHAR2}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "mad24",                T_CHAR3,         3, {T_CHAR3, T_CHAR3, T_CHAR3}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "mad24",                T_CHAR4,         3, {T_CHAR4, T_CHAR4, T_CHAR4}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "mad24",                T_CHAR8,         3, {T_CHAR8, T_CHAR8, T_CHAR8}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "mad24",                T_CHAR16,        3, {T_CHAR16, T_CHAR16, T_CHAR16}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "mad24",                T_UCHAR,         3, {T_UCHAR, T_UCHAR, T_UCHAR}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "mad24",                T_UCHAR2,        3, {T_UCHAR2, T_UCHAR2, T_UCHAR2}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "mad24",                T_UCHAR3,        3, {T_UCHAR3, T_UCHAR3, T_UCHAR3}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "mad24",                T_UCHAR4,        3, {T_UCHAR4, T_UCHAR4, T_UCHAR4}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "mad24",                T_UCHAR8,        3, {T_UCHAR8, T_UCHAR8, T_UCHAR8}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "mad24",                T_UCHAR16,       3, {T_UCHAR16, T_UCHAR16, T_UCHAR16}, {0}, {1, 1, 1}, 1},

    {clvEXTENSION_NONE,     "max",                  T_IU_GENTYPE,    2, {T_IU_GENTYPE, T_IU_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "min",                  T_IU_GENTYPE,    2, {T_IU_GENTYPE, T_IU_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "abs_diff",             T_U_GENTYPE,     2, {T_IU_GENTYPE, T_IU_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_SHORT,         2, {T_CHAR, T_UCHAR}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_USHORT,        2, {T_UCHAR, T_UCHAR}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_INT,           2, {T_SHORT, T_USHORT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_UINT,          2, {T_USHORT, T_USHORT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_LONG,          2, {T_INT, T_UINT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_ULONG,         2, {T_UINT, T_UINT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_SHORT2,        2, {T_CHAR2, T_UCHAR2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_USHORT2,       2, {T_UCHAR2, T_UCHAR2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_INT2,          2, {T_SHORT2, T_USHORT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_UINT2,         2, {T_USHORT2, T_USHORT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_LONG2,         2, {T_INT2, T_UINT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_ULONG2,        2, {T_UINT2, T_UINT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_SHORT3,        2, {T_CHAR3, T_UCHAR3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_USHORT3,       2, {T_UCHAR3, T_UCHAR3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_INT3,          2, {T_SHORT3, T_USHORT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_UINT3,         2, {T_USHORT3, T_USHORT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_LONG3,         2, {T_INT3, T_UINT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_ULONG3,        2, {T_UINT3, T_UINT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_SHORT4,        2, {T_CHAR4, T_UCHAR4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_USHORT4,       2, {T_UCHAR4, T_UCHAR4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_INT4,          2, {T_SHORT4, T_USHORT4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_UINT4,         2, {T_USHORT4, T_USHORT4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_LONG4,         2, {T_INT4, T_UINT4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_ULONG4,        2, {T_UINT4, T_UINT4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_SHORT8,        2, {T_CHAR8, T_UCHAR8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_USHORT8,       2, {T_UCHAR8, T_UCHAR8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_INT8,          2, {T_SHORT8, T_USHORT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_UINT8,         2, {T_USHORT8, T_USHORT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_LONG8,         2, {T_INT8, T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_ULONG8,        2, {T_UINT8, T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_SHORT16,       2, {T_CHAR16, T_UCHAR16}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_USHORT16,      2, {T_UCHAR16, T_UCHAR16}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_INT16,         2, {T_SHORT16, T_USHORT16}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_UINT16,        2, {T_USHORT16, T_USHORT16}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_LONG16,        2, {T_INT16, T_UINT16}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "upsample",             T_ULONG16,       2, {T_UINT16, T_UINT16}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "clz",                  T_IU_GENTYPE,    1, {T_IU_GENTYPE}, {0}, {1}, 1},
    {clvEXTENSION_NONE,     "popcount",             T_GENTYPE,       1, {T_GENTYPE}, {0}, {1}, 1},

    {clvEXTENSION_NONE,     "viv_abs",              T_U_GENTYPE,        1, {T_IU_GENTYPE}, {0}, {1}, 1},
    {clvEXTENSION_NONE,     "left_shift#",          T_GENTYPE,        2, {T_GENTYPE, T_IU_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "right_shift#",         T_GENTYPE,        2, {T_GENTYPE, T_IU_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "mul_int#",             T_IU_GENTYPE,    2, {T_IU_GENTYPE, T_IU_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_rotate",           T_IU_GENTYPE,    2, {T_IU_GENTYPE, T_IU_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "viv_hadd",             T_IU_GENTYPE,    2, {T_IU_GENTYPE, T_IU_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "viv_rhadd",            T_IU_GENTYPE,    2, {T_IU_GENTYPE, T_IU_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "viv_add_sat",          T_IU_GENTYPE,    2, {T_IU_GENTYPE, T_IU_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "viv_sub_sat",          T_IU_GENTYPE,    2, {T_IU_GENTYPE, T_IU_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "viv_mad_sat",          T_IU_GENTYPE,    3, {T_IU_GENTYPE, T_IU_GENTYPE, T_IU_GENTYPE}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "viv_mad_hi",           T_IU_GENTYPE,    3, {T_IU_GENTYPE, T_IU_GENTYPE, T_IU_GENTYPE}, {0}, {1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "viv_mul_hi",           T_IU_GENTYPE,    2, {T_IU_GENTYPE, T_IU_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "viv_mul24",            T_IU_GENTYPE,    2, {T_IU_GENTYPE, T_IU_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "viv_mad24",            T_IU_GENTYPE,    3, {T_IU_GENTYPE, T_IU_GENTYPE, T_IU_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "viv_max",              T_IU_GENTYPE,    2, {T_IU_GENTYPE, T_IU_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "viv_min",              T_IU_GENTYPE,    2, {T_IU_GENTYPE, T_IU_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "viv_abs_diff",         T_U_GENTYPE,     2, {T_IU_GENTYPE, T_IU_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_SHORT,         2, {T_CHAR, T_UCHAR}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_USHORT,        2, {T_UCHAR, T_UCHAR}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_INT,           2, {T_SHORT, T_USHORT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_UINT,          2, {T_USHORT, T_USHORT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_LONG,          2, {T_INT, T_UINT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_ULONG,         2, {T_UINT, T_UINT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_SHORT2,        2, {T_CHAR2, T_UCHAR2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_USHORT2,       2, {T_UCHAR2, T_UCHAR2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_INT2,          2, {T_SHORT2, T_USHORT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_UINT2,         2, {T_USHORT2, T_USHORT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_LONG2,         2, {T_INT2, T_UINT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_ULONG2,        2, {T_UINT2, T_UINT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_SHORT3,        2, {T_CHAR3, T_UCHAR3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_USHORT3,       2, {T_UCHAR3, T_UCHAR3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_INT3,          2, {T_SHORT3, T_USHORT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_UINT3,         2, {T_USHORT3, T_USHORT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_LONG3,         2, {T_INT3, T_UINT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_ULONG3,        2, {T_UINT3, T_UINT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_SHORT4,        2, {T_CHAR4, T_UCHAR4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_USHORT4,       2, {T_UCHAR4, T_UCHAR4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_INT4,          2, {T_SHORT4, T_USHORT4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_UINT4,         2, {T_USHORT4, T_USHORT4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_LONG4,         2, {T_INT4, T_UINT4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_ULONG4,        2, {T_UINT4, T_UINT4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_SHORT8,        2, {T_CHAR8, T_UCHAR8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_USHORT8,       2, {T_UCHAR8, T_UCHAR8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_INT8,          2, {T_SHORT8, T_USHORT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_UINT8,         2, {T_USHORT8, T_USHORT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_LONG8,         2, {T_INT8, T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_ULONG8,        2, {T_UINT8, T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_SHORT16,       2, {T_CHAR16, T_UCHAR16}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_USHORT16,      2, {T_UCHAR16, T_UCHAR16}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_INT16,         2, {T_SHORT16, T_USHORT16}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_UINT16,        2, {T_USHORT16, T_USHORT16}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_LONG16,        2, {T_INT16, T_UINT16}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_upsample",         T_ULONG16,       2, {T_UINT16, T_UINT16}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_clz",              T_IU_GENTYPE,    1, {T_IU_GENTYPE}, {0}, {1}, 1},
    {clvEXTENSION_NONE,     "viv_popcount",         T_GENTYPE,       1, {T_GENTYPE}, {0}, {1}, 1},

};

#define _cldIntBuiltinFunctionCount (sizeof(IntBuiltinFunctions) / sizeof(clsBUILTIN_FUNCTION))

static gceSTATUS
_GenClzCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* grab leading zero */
    status = clGenGenericCode1(Compiler,
                PolynaryExpr->exprBase.base.lineNo,
                PolynaryExpr->exprBase.base.stringNo,
                clvOPCODE_LEADZERO,
                IOperand,
                &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenShiftCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    cleOPCODE    shiftCode = strstr(PolynaryExpr->funcName->symbol,"left_shift#")? clvOPCODE_LSHIFT: clvOPCODE_RSHIFT;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if( clmGEN_CODE_IsScalarDataType(OperandsParameters[0].dataTypes[0].def) &&
        (IOperand->dataType.elementType == clvTYPE_CHAR ||
        IOperand->dataType.elementType == clvTYPE_CHAR_PACKED ||
        IOperand->dataType.elementType == clvTYPE_UCHAR ||
        IOperand->dataType.elementType == clvTYPE_UCHAR_PACKED ||
        IOperand->dataType.elementType == clvTYPE_SHORT ||
        IOperand->dataType.elementType == clvTYPE_SHORT_PACKED ||
        IOperand->dataType.elementType == clvTYPE_USHORT ||
        IOperand->dataType.elementType == clvTYPE_USHORT_PACKED)){ /*Need to promote the data type to integer/unsigned integer */

        if(clmIsElementTypeUnsigned(IOperand->dataType.elementType)) {
            IOperand->dataType.elementType = clvTYPE_UINT;
        }
        else
            IOperand->dataType.elementType = clvTYPE_INT;

        status = clGenShiftExprCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    shiftCode,
                                    IOperand,
                                    &OperandsParameters[0].rOperands[0],
                                    &OperandsParameters[1].rOperands[0]);
        if (gcmIS_ERROR(status)) return status;
    }
    else{
        status = clGenShiftExprCode(Compiler,
            PolynaryExpr->exprBase.base.lineNo,
            PolynaryExpr->exprBase.base.stringNo,
            shiftCode,
            IOperand,
            &OperandsParameters[0].rOperands[0],
            &OperandsParameters[1].rOperands[0]);
        if (gcmIS_ERROR(status)) return status;
    }

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}



static gceSTATUS
_GenRotateCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = clGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_ROTATE,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenPopcountCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = clGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_POPCOUNT,
                            IOperand,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}



static gceSTATUS
_GenHaddCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;

    clsIOPERAND intermIOperands[7];
    clsROPERAND intermROperands[7];

    clsROPERAND oneROperand;
    clsROPERAND maskROperand;

    clsROPERAND_InitializeIntOrIVecConstant(&oneROperand,
                              clmGenCodeDataType(T_UINT),
                              (gctUINT) 1);

    clsROPERAND_InitializeIntOrIVecConstant(&maskROperand,
                              clmGenCodeDataType(T_UINT),
                              (gctUINT) 0x00000001);

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);



    clsIOPERAND_New(Compiler, &intermIOperands[0], OperandsParameters[0].dataTypes[0].def);
    status = clGenShiftExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_RSHIFT,
                            &intermIOperands[0],
                            &OperandsParameters[0].rOperands[0],
                            &oneROperand);

    if (gcmIS_ERROR(status)) return status;


    clsIOPERAND_New(Compiler, &intermIOperands[1], OperandsParameters[0].dataTypes[0].def);
    status = clGenShiftExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_RSHIFT,
                            &intermIOperands[1],
                            &OperandsParameters[1].rOperands[0],
                            &oneROperand);

    if (gcmIS_ERROR(status)) return status;

    clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);
    clsIOPERAND_New(Compiler, &intermIOperands[2], OperandsParameters[0].dataTypes[0].def);
    status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_ADD,
                            &intermIOperands[2],
                            &intermROperands[0],
                            &intermROperands[1]);

    if (gcmIS_ERROR(status)) return status;


    clsIOPERAND_New(Compiler, &intermIOperands[3], OperandsParameters[0].dataTypes[0].def);
    status = clGenArithmeticExprCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_AND_BITWISE,
                                &intermIOperands[3],
                                &OperandsParameters[0].rOperands[0],
                                &maskROperand);

    if (gcmIS_ERROR(status)) return status;

    clsIOPERAND_New(Compiler, &intermIOperands[4], OperandsParameters[0].dataTypes[0].def);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[3], &intermIOperands[3]);
    status = clGenArithmeticExprCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_AND_BITWISE,
                                &intermIOperands[4],
                                &OperandsParameters[1].rOperands[0],
                                &intermROperands[3]);

    if (gcmIS_ERROR(status)) return status;

    clsROPERAND_InitializeUsingIOperand(&intermROperands[4], &intermIOperands[4]);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);
    status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_ADD,
                            IOperand,
                            &intermROperands[2],
                            &intermROperands[4]);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;

}


static gceSTATUS
_GenRhaddCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;

    clsIOPERAND intermIOperands[8];
    clsROPERAND intermROperands[8];

    clsROPERAND oneROperand;
    clsROPERAND maskROperand;

    clsROPERAND_InitializeIntOrIVecConstant(&oneROperand,
                              clmGenCodeDataType(T_UINT),
                              (gctUINT) 1);

    clsROPERAND_InitializeIntOrIVecConstant(&maskROperand,
                              clmGenCodeDataType(T_UINT),
                              (gctUINT) 0x00000001);

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);



    clsIOPERAND_New(Compiler, &intermIOperands[0], OperandsParameters[0].dataTypes[0].def);
    status = clGenShiftExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_RSHIFT,
                            &intermIOperands[0],
                            &OperandsParameters[0].rOperands[0],
                            &oneROperand);

    if (gcmIS_ERROR(status)) return status;

    clsIOPERAND_New(Compiler, &intermIOperands[1], OperandsParameters[0].dataTypes[0].def);
    status = clGenShiftExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_RSHIFT,
                            &intermIOperands[1],
                            &OperandsParameters[1].rOperands[0],
                            &oneROperand);

    if (gcmIS_ERROR(status)) return status;

    clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);
    clsIOPERAND_New(Compiler, &intermIOperands[3], OperandsParameters[0].dataTypes[0].def);
    status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_ADD,
                            &intermIOperands[3],
                            &intermROperands[0],
                            &intermROperands[1]);

    if (gcmIS_ERROR(status)) return status;


    clsIOPERAND_New(Compiler, &intermIOperands[4], OperandsParameters[0].dataTypes[0].def);
    status = clGenArithmeticExprCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_AND_BITWISE,
                                &intermIOperands[4],
                                &OperandsParameters[0].rOperands[0],
                                &maskROperand);

    if (gcmIS_ERROR(status)) return status;

    clsIOPERAND_New(Compiler, &intermIOperands[5], OperandsParameters[0].dataTypes[0].def);
    status = clGenArithmeticExprCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_AND_BITWISE,
                                &intermIOperands[5],
                                &OperandsParameters[1].rOperands[0],
                                &maskROperand);

    if (gcmIS_ERROR(status)) return status;


    clsROPERAND_InitializeUsingIOperand(&intermROperands[5], &intermIOperands[5]);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[4], &intermIOperands[4]);
    clsIOPERAND_New(Compiler, &intermIOperands[6], OperandsParameters[0].dataTypes[0].def);
    status = clGenArithmeticExprCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_OR_BITWISE,
                                &intermIOperands[6],
                                &intermROperands[5],
                                &intermROperands[4]);

    if (gcmIS_ERROR(status)) return status;

    clsROPERAND_InitializeUsingIOperand(&intermROperands[6], &intermIOperands[6]);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[3], &intermIOperands[3]);
    status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_ADD,
                            IOperand,
                            &intermROperands[6],
                            &intermROperands[3]);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;

}



static gceSTATUS
_GenAddSubSatCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    clsIOPERAND intermIOperands[8];
    clsROPERAND intermROperands[8];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    clsIOPERAND_New(Compiler, &intermIOperands[0], OperandsParameters[0].rOperands[0].dataType);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);
    clsIOPERAND_New(Compiler, &intermIOperands[1], OperandsParameters[0].rOperands[0].dataType);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);
    clsIOPERAND_New(Compiler, &intermIOperands[2], OperandsParameters[0].rOperands[0].dataType);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);
    clsIOPERAND_New(Compiler, &intermIOperands[3], OperandsParameters[0].rOperands[0].dataType);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[3], &intermIOperands[3]);
    clsIOPERAND_New(Compiler, &intermIOperands[4], OperandsParameters[0].rOperands[0].dataType);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[4], &intermIOperands[4]);



    if (IOperand->dataType.elementType == clvTYPE_INT &&
        ((CodeGenerator->chipModel <= gcv2100 && CodeGenerator->chipRevision <= 0x5130) ||
         (CodeGenerator->chipModel == gcv4000 &&
          (CodeGenerator->chipRevision == 0x5208 || CodeGenerator->chipRevision == 0x5222 ||
                CodeGenerator->chipRevision == 0x4633))))
    {
            clsROPERAND thirtyOneROperand, maxIntROperand, zeroROperand;
            clsROPERAND_InitializeIntOrIVecConstant(&maxIntROperand,
                                            clmGenCodeDataType(T_INT),
                                            0x7fffffff);
            clsROPERAND_InitializeIntOrIVecConstant(&thirtyOneROperand,
                                            clmGenCodeDataType(T_INT),
                                            31);

            clsROPERAND_InitializeIntOrIVecConstant(&zeroROperand,
                                            clmGenCodeDataType(T_INT),
                                            0);

            if(PolynaryExpr->funcName->symbol[0] == 's')
                   status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_SUB,
                            &intermIOperands[4],
                            &zeroROperand,
                            &OperandsParameters[1].rOperands[0]);

            status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_ADD ,
                            &intermIOperands[0],
                            &OperandsParameters[0].rOperands[0],
                            PolynaryExpr->funcName->symbol[0] == 's' ?&intermROperands[4]: &OperandsParameters[1].rOperands[0]);

            status = clGenGenericCode1(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_NOT_BITWISE,
                        &intermIOperands[1],
                        PolynaryExpr->funcName->symbol[0] == 's' ?&intermROperands[4]: &OperandsParameters[1].rOperands[0]);

            intermROperands[1].dataType.elementType = clvTYPE_UINT;

            /*Comparison in UINT, carry bit, set to high 32 */
            status = clGenGenericCode2(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_LESS_THAN,
                                    &intermIOperands[1],
                                    &intermROperands[1],
                                    &OperandsParameters[0].rOperands[0]);

            /*-1 --> 1, 0 keep, carry bit doesn't allow -1 */
            if(clmGEN_CODE_IsVectorDataType(OperandsParameters[0].dataTypes[0].def)){
                status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_SUB,
                            &intermIOperands[1],
                            &zeroROperand,
                            &intermROperands[1]);
            }

            /*Extend bit to high 32*/
            status = clGenShiftExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_RSHIFT,
                            &intermIOperands[2],
                            &OperandsParameters[0].rOperands[0],
                            &thirtyOneROperand);

            status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_ADD,
                            &intermIOperands[1],
                            &intermROperands[1],
                            &intermROperands[2]);

            /*Extend bit to high 32*/
            status = clGenShiftExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_RSHIFT,
                            &intermIOperands[2],
                            PolynaryExpr->funcName->symbol[0] == 's' ?&intermROperands[4]: &OperandsParameters[1].rOperands[0],
                            &thirtyOneROperand);

            status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_ADD,
                            &intermIOperands[1],
                            &intermROperands[1],
                            &intermROperands[2]);

            /*Check the highest bit same as the high32 bit */
            status = clGenShiftExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_RSHIFT,
                            &intermIOperands[2],
                            &intermROperands[0],
                            &thirtyOneROperand);

            /*r3 == 1, not overflow, else overflow */
            status = clGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_NOT_EQUAL,
                            &intermIOperands[3],
                            &intermROperands[1],
                            &intermROperands[2]);

            /*1 --> -1, 0 keep */
            if(clmGEN_CODE_IsVectorDataType(OperandsParameters[0].dataTypes[0].def) == 0){
                status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_SUB,
                            &intermIOperands[3],
                            &zeroROperand,
                            &intermROperands[3]);
            }
            /*Now calculate the overflow value */
            intermIOperands[2].dataType.elementType = clvTYPE_UINT;


            status = clGenShiftExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_RSHIFT,
                            &intermIOperands[2],
                            &intermROperands[1],
                            &thirtyOneROperand);

            status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_ADD,
                            &intermIOperands[2],
                            &intermROperands[2],
                            &maxIntROperand);

            /*r1 = ~r3 */
            status = clGenGenericCode1(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_NOT_BITWISE,
                        &intermIOperands[1],
                        &intermROperands[3]);

            /*bits combination */
            status = clGenArithmeticExprCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_AND_BITWISE,
                                &intermIOperands[0],
                                &intermROperands[0],
                                &intermROperands[1]);

            status = clGenArithmeticExprCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_AND_BITWISE,
                                &intermIOperands[1],
                                &intermROperands[2],
                                &intermROperands[3]);

            status = clGenArithmeticExprCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_OR_BITWISE,
                                IOperand,
                                &intermROperands[1],
                                &intermROperands[0]);

    }
    else if (IOperand->dataType.elementType == clvTYPE_UINT &&
             ((CodeGenerator->chipModel <= gcv2100 && CodeGenerator->chipRevision <= 0x5130) ||
              (CodeGenerator->chipModel == gcv4000 &&
               (CodeGenerator->chipRevision == 0x5208 || CodeGenerator->chipRevision == 0x5222 ||
                    CodeGenerator->chipRevision == 0x4633))))
    {
            clsROPERAND thirtyOneROperand, maxUintROperand, zeroROperand;
            clsROPERAND_InitializeIntOrIVecConstant(&maxUintROperand,
                                            clmGenCodeDataType(T_UINT),
                                            0xffffffff);
            clsROPERAND_InitializeIntOrIVecConstant(&thirtyOneROperand,
                                            clmGenCodeDataType(T_INT),
                                            31);

            clsROPERAND_InitializeIntOrIVecConstant(&zeroROperand,
                                            clmGenCodeDataType(T_INT),
                                            0);

            status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            PolynaryExpr->funcName->symbol[0] == 's'? clvOPCODE_SUB : clvOPCODE_ADD ,
                            &intermIOperands[0],
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

            /* r3: indicate not overflow*/
            if(PolynaryExpr->funcName->symbol[0] == 's'){
                    status = clGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_GREATER_THAN_EQUAL,
                            &intermIOperands[3],
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);
            }
            else{
                status = clGenGenericCode1(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_NOT_BITWISE,
                            &intermIOperands[1],
                            &OperandsParameters[1].rOperands[0]);
                status = clGenGenericCode2(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_LESS_THAN_EQUAL,
                                    &intermIOperands[3],
                                    &OperandsParameters[0].rOperands[0],
                                    &intermROperands[1]);

            }

            /*-1 --> 1, 0 keep */
            if(clmGEN_CODE_IsVectorDataType(OperandsParameters[0].dataTypes[0].def)==0){
                intermIOperands[3].dataType.elementType = clvTYPE_INT;
                intermROperands[3].dataType.elementType = clvTYPE_INT;
                status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_SUB,
                            &intermIOperands[3],
                            &zeroROperand,
                            &intermROperands[3]);
            }

            if(PolynaryExpr->funcName->symbol[0] == 's'){
                status = clGenArithmeticExprCode(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_AND_BITWISE,
                    IOperand,
                    &intermROperands[0],
                    &intermROperands[3]);
            }
            else{
                /*r1 = ~r3 */
                status = clGenGenericCode1(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_NOT_BITWISE,
                            &intermIOperands[1],
                            &intermROperands[3]);


                /*bits combination */
                status = clGenArithmeticExprCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_AND_BITWISE,
                                    &intermIOperands[0],
                                    &intermROperands[0],
                                    &intermROperands[3]);

                status = clGenArithmeticExprCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_AND_BITWISE,
                                    &intermIOperands[1],
                                    &maxUintROperand,
                                    &intermROperands[1]);

                status = clGenArithmeticExprCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_OR_BITWISE,
                                    IOperand,
                                    &intermROperands[1],
                                    &intermROperands[0]);
            }

    }
    else
        status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            (PolynaryExpr->funcName->symbol[0] == 'a')?clvOPCODE_ADDSAT : clvOPCODE_SUBSAT,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);


    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;

}

static gceSTATUS
_GenMadSatVec4Code(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status = gcvSTATUS_OK;
    clsIOPERAND intermIOperands[8];
    clsROPERAND intermROperands[8];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    clsIOPERAND_New(Compiler, &intermIOperands[0], OperandsParameters[0].rOperands[0].dataType);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);
    clsIOPERAND_New(Compiler, &intermIOperands[1], OperandsParameters[0].rOperands[0].dataType);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);
    clsIOPERAND_New(Compiler, &intermIOperands[2], OperandsParameters[0].rOperands[0].dataType);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);
    clsIOPERAND_New(Compiler, &intermIOperands[3], OperandsParameters[0].rOperands[0].dataType);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[3], &intermIOperands[3]);

    if ((IOperand->dataType.elementType == clvTYPE_INT ||
         IOperand->dataType.elementType == clvTYPE_LONG) &&
        ((CodeGenerator->chipModel <= gcv2100 && CodeGenerator->chipRevision <= 0x5130) ||
         (CodeGenerator->chipModel == gcv4000 &&
          (CodeGenerator->chipRevision == 0x5208 || CodeGenerator->chipRevision == 0x5222 ||
           CodeGenerator->chipRevision == 0x4633))))
    {
            clsROPERAND thirtyOneROperand, maxIntROperand, zeroROperand;
            clsROPERAND_InitializeIntOrIVecConstant(&maxIntROperand,
                                            clmGenCodeDataType(T_INT),
                                            0x7fffffff);
            clsROPERAND_InitializeIntOrIVecConstant(&thirtyOneROperand,
                                            clmGenCodeDataType(T_INT),
                                            31);

            clsROPERAND_InitializeIntOrIVecConstant(&zeroROperand,
                                            clmGenCodeDataType(T_INT),
                                            0);

            status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_MULHI,
                            &intermIOperands[1],
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

            status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_MUL,
                            &intermIOperands[0],
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

            status = clGenGenericCode1(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_NOT_BITWISE,
                        &intermIOperands[2],
                        &OperandsParameters[2].rOperands[0]);

            intermROperands[0].dataType.elementType = clvTYPE_UINT;
            intermROperands[2].dataType.elementType = clvTYPE_UINT;

            status = clGenGenericCode2(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_GREATER_THAN,
                                    &intermIOperands[3],
                                    &intermROperands[0],
                                    &intermROperands[2]);


            status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_ADD,
                            &intermIOperands[1],
                            &intermROperands[1],
                            &intermROperands[3]);

            /*Extend the negative c to high 32 bits*/
            status = clGenShiftExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_RSHIFT,
                            &intermIOperands[3],
                            &OperandsParameters[2].rOperands[0],
                            &thirtyOneROperand);
            status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_ADD,
                            &intermIOperands[1],
                            &intermROperands[1],
                            &intermROperands[3]);


            status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_ADD,
                            &intermIOperands[0],
                            &intermROperands[0],
                            &OperandsParameters[2].rOperands[0]);

            intermROperands[0].dataType.elementType = clvTYPE_INT;

            status = clGenShiftExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_RSHIFT,
                            &intermIOperands[2],
                            &intermROperands[0],
                            &thirtyOneROperand);
            /*r3 == 1, not overflow, else overflow */
            status = clGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_NOT_EQUAL,
                            &intermIOperands[3],
                            &intermROperands[1],
                            &intermROperands[2]);

            /*1 --> -1, 0 keep */
            if(clmGEN_CODE_IsVectorDataType(OperandsParameters[0].dataTypes[0].def) == 0){
                status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_SUB,
                            &intermIOperands[3],
                            &zeroROperand,
                            &intermROperands[3]);
            }

            /*Now calculate the overflow value */
            intermIOperands[2].dataType.elementType = clvTYPE_UINT;


            status = clGenShiftExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_RSHIFT,
                            &intermIOperands[2],
                            &intermROperands[1],
                            &thirtyOneROperand);

            status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_ADD,
                            &intermIOperands[2],
                            &intermROperands[2],
                            &maxIntROperand);

            /*r1 = ~r3 */
            status = clGenGenericCode1(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_NOT_BITWISE,
                        &intermIOperands[1],
                        &intermROperands[3]);

            /*bits combination */
            status = clGenArithmeticExprCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_AND_BITWISE,
                                &intermIOperands[0],
                                &intermROperands[0],
                                &intermROperands[1]);

            status = clGenArithmeticExprCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_AND_BITWISE,
                                &intermIOperands[1],
                                &intermROperands[2],
                                &intermROperands[3]);

            status = clGenArithmeticExprCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_OR_BITWISE,
                                IOperand,
                                &intermROperands[1],
                                &intermROperands[0]);

    }
    else if ((IOperand->dataType.elementType == clvTYPE_UINT ||
              IOperand->dataType.elementType == clvTYPE_ULONG) &&
             ((CodeGenerator->chipModel <= gcv2100 && CodeGenerator->chipRevision <= 0x5130) ||
              (CodeGenerator->chipModel == gcv4000 &&
               (CodeGenerator->chipRevision == 0x5208 || CodeGenerator->chipRevision == 0x5222 ||
                    CodeGenerator->chipRevision == 0x4633))))
    {
            clsROPERAND maxUintROperand, zeroROperand;
            clsROPERAND_InitializeIntOrIVecConstant(&maxUintROperand,
                                            clmGenCodeDataType(T_INT),
                                            0xffffffff);

            clsROPERAND_InitializeIntOrIVecConstant(&zeroROperand,
                                            clmGenCodeDataType(T_INT),
                                            0);

            status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_MULHI,
                            &intermIOperands[1],
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

            status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_MUL,
                            &intermIOperands[0],
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

            status = clGenGenericCode1(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_NOT_BITWISE,
                        &intermIOperands[2],
                        &OperandsParameters[2].rOperands[0]);

            intermROperands[0].dataType.elementType = clvTYPE_UINT;
            intermROperands[2].dataType.elementType = clvTYPE_UINT;

            status = clGenGenericCode2(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_GREATER_THAN,
                                    &intermIOperands[3],
                                    &intermROperands[0],
                                    &intermROperands[2]);


            status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_ADD,
                            &intermIOperands[1],
                            &intermROperands[1],
                            &intermROperands[3]);

            status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_ADD,
                            &intermIOperands[0],
                            &intermROperands[0],
                            &OperandsParameters[2].rOperands[0]);

            /*r3 == 1, not overflow, else overflow */
            status = clGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_NOT_EQUAL,
                            &intermIOperands[3],
                            &intermROperands[1],
                            &zeroROperand);

            /*1 --> -1, 0 keep */
            if(clmGEN_CODE_IsVectorDataType(OperandsParameters[0].dataTypes[0].def) == 0){
                status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_SUB,
                            &intermIOperands[3],
                            &zeroROperand,
                            &intermROperands[3]);
            }

            /*r1 = ~r3 */
            status = clGenGenericCode1(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_NOT_BITWISE,
                        &intermIOperands[1],
                        &intermROperands[3]);

            /*bits combination */
            status = clGenArithmeticExprCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_AND_BITWISE,
                                &intermIOperands[0],
                                &intermROperands[0],
                                &intermROperands[1]);

            status = clGenArithmeticExprCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_AND_BITWISE,
                                &intermIOperands[1],
                                &maxUintROperand,
                                &intermROperands[3]);

            status = clGenArithmeticExprCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_OR_BITWISE,
                                IOperand,
                                &intermROperands[1],
                                &intermROperands[0]);

    }
    else if(IOperand->dataType.elementType == clvTYPE_LONG ||
            IOperand->dataType.elementType == clvTYPE_ULONG)
    {
        status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_MADSAT,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);
    }
    else
    {
        status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_MULSAT,
                            &intermIOperands[0],
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

        status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_ADDSAT,
                            IOperand,
                            &intermROperands[0],
                            &OperandsParameters[2].rOperands[0]);
    }


    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenMadSatCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    if(clmIsElementTypePacked(IOperand->dataType.elementType)) {
        clsLOPERAND lOperand[1];
        clsROPERAND rOperands[3];

        clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
        rOperands[0] = OperandsParameters[0].rOperands[0];
        rOperands[1] = OperandsParameters[1].rOperands[0];
        rOperands[2] = OperandsParameters[2].rOperands[0];
        return  clGenIntrinsicAsmCode(Compiler,
                                      PolynaryExpr->exprBase.base.lineNo,
                                      PolynaryExpr->exprBase.base.stringNo,
                                      CL_VIR_IK_madsat,
                                      lOperand,
                                      OperandCount,
                                      rOperands);
    }
    else {
        gceSTATUS    status = gcvSTATUS_OK;

        clsROPERAND sliceROperand0, sliceROperand1, sliceROperand2, cpyROperand[3];
        clsLOPERAND tempLOperand, destLOperand;
        clsIOPERAND tempIOperand;
        clsROPERAND tempROperand;
        clsGEN_CODE_DATA_TYPE dataType;
        gctUINT8 i, vectorComponentCount = gcGetDataTypeComponentCount(OperandsParameters[0].rOperands[0].dataType);

        if(vectorComponentCount != 8 && vectorComponentCount != 16)
            return _GenMadSatVec4Code(
                    Compiler,
                    CodeGenerator,
                    PolynaryExpr,
                    OperandCount,
                    OperandsParameters,
                    IOperand
                    );
        for(i = 0; i<3; i++)
            cpyROperand[i] = OperandsParameters[i].rOperands[0];

        clsLOPERAND_InitializeUsingIOperand(&tempLOperand, IOperand);
        dataType = clGetSubsetDataType(IOperand->dataType, 4);
        clsIOPERAND_New(Compiler, &tempIOperand, dataType);
        clsROPERAND_InitializeUsingIOperand(&tempROperand, &tempIOperand);

        for(i = 0; i<vectorComponentCount; i += 4){
            clGetVectorROperandSlice(&cpyROperand[0],
                    i,
                    4,
                    &sliceROperand0);
            clGetVectorROperandSlice(&cpyROperand[1],
                    i,
                    4,
                    &sliceROperand1);
            clGetVectorROperandSlice(&cpyROperand[2],
                    i,
                    4,
                    &sliceROperand2);

            clGetVectorLOperandSlice(&tempLOperand,
                        i,
                        4,
                        &destLOperand);

            OperandsParameters[0].rOperands[0] = sliceROperand0;
            OperandsParameters[1].rOperands[0] = sliceROperand1;
            OperandsParameters[2].rOperands[0] = sliceROperand2;
            status = _GenMadSatVec4Code(
                    Compiler,
                    CodeGenerator,
                    PolynaryExpr,
                    OperandCount,
                    OperandsParameters,
                    &tempIOperand
                    );

            status = clGenAssignCode(
                    Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    &destLOperand,
                    &tempROperand);
            if (gcmIS_ERROR(status)) return status;
        }

        OperandsParameters[0].rOperands[0] = cpyROperand[0];
        OperandsParameters[1].rOperands[0] = cpyROperand[1];
        OperandsParameters[2].rOperands[0] = cpyROperand[2];
        return status;
    }
}


static gceSTATUS
_GenMadHiLoCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    int madHiFlag = strstr(PolynaryExpr->funcName->symbol, "hi")? 1:0;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if(clmIsElementTypePacked(IOperand->dataType.elementType)) {
        clvVIR_IK intrinsicKind = madHiFlag ? CL_VIR_IK_imadhi0 : CL_VIR_IK_imadlo0;
        clsLOPERAND lOperand[1];
        clsROPERAND rOperands[3];

        clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
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
    else {
        gceSTATUS    status = gcvSTATUS_OK;
        clsIOPERAND intermIOperands[1];
        clsROPERAND intermROperands[1];

        clsIOPERAND_New(Compiler, &intermIOperands[0], OperandsParameters[0].dataTypes[0].def);
        clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);

        {
            status = clGenArithmeticExprCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                madHiFlag ? clvOPCODE_MULHI : clvOPCODE_MUL,
                                &intermIOperands[0],
                                &OperandsParameters[0].rOperands[0],
                                &OperandsParameters[1].rOperands[0]);

            status = clGenArithmeticExprCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_ADD,
                                IOperand,
                                &intermROperands[0],
                                &OperandsParameters[2].rOperands[0]);

        }

        if (gcmIS_ERROR(status)) return status;

        return gcvSTATUS_OK;
    }
}

static gceSTATUS
_GenMul24Code(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status = gcvSTATUS_OK;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = clGenArithmeticExprCode(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_MUL,
                        IOperand,
                        &OperandsParameters[0].rOperands[0],
                        &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenAbsDiffCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    clsIOPERAND intermIOperands[2];
    clsROPERAND intermROperands[2];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    clsIOPERAND_New(Compiler, &intermIOperands[0], OperandsParameters[0].dataTypes[0].def);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);
    clsIOPERAND_New(Compiler, &intermIOperands[1], OperandsParameters[0].dataTypes[0].def);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);


    status = clGenGenericCode2(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_MAX,
                        &intermIOperands[0],
                        &OperandsParameters[0].rOperands[0],
                        &OperandsParameters[1].rOperands[0]);

    status = clGenGenericCode2(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_MIN,
                        &intermIOperands[1],
                        &OperandsParameters[0].rOperands[0],
                        &OperandsParameters[1].rOperands[0]);

    status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_SUB,
                            IOperand,
                            &intermROperands[0],
                            &intermROperands[1]);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;

}


static gceSTATUS
_GenMulHiCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status = gcvSTATUS_OK;
    clsROPERAND zeroROperand;


    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);
    clsROPERAND_InitializeIntOrIVecConstant(&zeroROperand, clmGenCodeDataType(T_UINT), 0x00000000);

    {
        status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_MULHI,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);
    }

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenAbsCode(
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

    if(clmIsElementTypeHighPrecision(OperandsParameters[0].dataTypes[0].def.elementType) &&
       clmIsElementTypeUnsigned(OperandsParameters[0].dataTypes[0].def.elementType))
    {
        return clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_ASSIGN,
                                 IOperand,
                                 &OperandsParameters[0].rOperands[0]);
    }
    else
    {
        return clGenGenericCode1(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_ABS,
                                 IOperand,
                                 &OperandsParameters[0].rOperands[0]);
    }
}

static gceSTATUS
_GenUpsampleCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    clsROPERAND destROperand, shiftROperand, *upperOperand, *lowerOperand;
    clsIOPERAND lowerIOperand, upperIOperand ;
    clsROPERAND lowerROperandBuf, upperROperandBuf;
    cltELEMENT_TYPE orgType;
    cltELEMENT_TYPE saveType = IOperand->dataType.elementType;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    upperOperand = &OperandsParameters[0].rOperands[0];
    lowerOperand = &OperandsParameters[1].rOperands[0];
    switch(lowerOperand->dataType.elementType) {
    case clvTYPE_UCHAR:
        clsROPERAND_InitializeIntOrIVecConstant(&shiftROperand,
                                                clmGenCodeDataType(T_USHORT),
                                                (gctUINT16) 0x00000008);
        break;

    case clvTYPE_UINT:
        gcmASSERT(clmIsElementTypeHighPrecision(IOperand->dataType.elementType));
        clsROPERAND_InitializeIntOrIVecConstant(&shiftROperand,
                                                clmGenCodeDataType(T_UINT),
                                                (gctUINT16) 0x00000020);
        clsIOPERAND_New(Compiler, &upperIOperand, IOperand->dataType);

        orgType = upperOperand->dataType.elementType;
        upperOperand->dataType.elementType = clvTYPE_UINT;
        upperIOperand.dataType.elementType = clvTYPE_ULONG;

        status = clGenGenericCode1(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   clvOPCODE_CONV,
                                   &upperIOperand,
                                   upperOperand);
        if (gcmIS_ERROR(status)) return status;


        if(orgType == clvTYPE_INT)
        {
            upperIOperand.dataType.elementType = clvTYPE_LONG;
        }
        else
        {
            upperIOperand.dataType.elementType = clvTYPE_ULONG;
        }
        upperOperand->dataType.elementType = orgType;

        clsROPERAND_InitializeUsingIOperand(&upperROperandBuf, &upperIOperand);
        upperOperand = &upperROperandBuf;

        clsIOPERAND_New(Compiler, &lowerIOperand, IOperand->dataType);

        orgType = lowerOperand->dataType.elementType;
        lowerOperand->dataType.elementType = clvTYPE_UINT;
        lowerIOperand.dataType.elementType = clvTYPE_ULONG;

        status = clGenGenericCode1(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   clvOPCODE_CONV,
                                   &lowerIOperand,
                                   lowerOperand);
        if (gcmIS_ERROR(status)) return status;

        if(orgType == clvTYPE_INT)
        {
            lowerIOperand.dataType.elementType = clvTYPE_LONG;
        }
        else
        {
            lowerIOperand.dataType.elementType = clvTYPE_ULONG;
        }
        lowerOperand->dataType.elementType = orgType;

        clsROPERAND_InitializeUsingIOperand(&lowerROperandBuf, &lowerIOperand);
        lowerOperand = &lowerROperandBuf;
        break;

    default:
        clsROPERAND_InitializeIntOrIVecConstant(&shiftROperand,
                                                clmGenCodeDataType(T_UINT),
                                                (gctUINT16) 0x00000010);
        IOperand->dataType.elementType = IOperand->dataType.elementType == clvTYPE_INT? clvTYPE_INT : clvTYPE_UINT;
        break;
    }

    clsROPERAND_InitializeUsingIOperand(&destROperand, IOperand);

    status = clGenShiftExprCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_LSHIFT,
                                IOperand,
                                upperOperand,
                                &shiftROperand);
    if (gcmIS_ERROR(status)) return status;

    status = clGenArithmeticExprCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_OR_BITWISE,
                                     IOperand,
                                     &destROperand,
                                     lowerOperand);
    if (gcmIS_ERROR(status)) return status;

    IOperand->dataType.elementType = saveType;
    return gcvSTATUS_OK;
}

#endif /* __gc_cl_built_ins_int_h_ */
