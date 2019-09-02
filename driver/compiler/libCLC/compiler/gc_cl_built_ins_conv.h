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


#ifndef __gc_cl_built_ins_conv_h_
#define __gc_cl_built_ins_conv_h_

#define _USE_CONV_FOR_EXPLICIT_CONVERT_FUNCTION                         1
#define _DO_NOT_USE_CONV_FOR_RTNE_EXPLICIT_CONVERT_FUNCTION             0
#define _DO_NOT_USE_CONV_FOR_FLOATN_RTP_RTN_EXPLICIT_CONVERT_FUNCTION   1

static clsBUILTIN_FUNCTION    ConvBuiltinFunctions[] =
{
    /* Non-saturated mode */
    {clvEXTENSION_NONE, "convert_char",       T_CHAR,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar",      T_UCHAR,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int",        T_INT,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint",       T_UINT,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long",       T_LONG,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong",      T_ULONG,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short",      T_SHORT,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort",     T_USHORT, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float",      T_FLOAT,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_half",       T_HALF,   1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char_rte",   T_CHAR,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar_rte",  T_UCHAR,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int_rte",    T_INT,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint_rte",   T_UINT,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long_rte",   T_LONG,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong_rte",  T_ULONG,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short_rte",  T_SHORT,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort_rte", T_USHORT, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float_rte",  T_FLOAT,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_half_rte",   T_HALF,   1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char_rtz",   T_CHAR,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar_rtz",  T_UCHAR,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int_rtz",    T_INT,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint_rtz",   T_UINT,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long_rtz",   T_LONG,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong_rtz",  T_ULONG,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short_rtz",  T_SHORT,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort_rtz", T_USHORT, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float_rtz",  T_FLOAT,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_half_rtz",   T_HALF,   1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char_rtp",   T_CHAR,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar_rtp",  T_UCHAR,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int_rtp",    T_INT,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint_rtp",   T_UINT,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long_rtp",   T_LONG,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong_rtp",  T_ULONG,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short_rtp",  T_SHORT,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort_rtp", T_USHORT, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float_rtp",  T_FLOAT,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_half_rtp",   T_HALF,   1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char_rtn",   T_CHAR,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar_rtn",  T_UCHAR,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int_rtn",    T_INT,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint_rtn",   T_UINT,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long_rtn",   T_LONG,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong_rtn",  T_ULONG,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short_rtn",  T_SHORT,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort_rtn", T_USHORT, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float_rtn",  T_FLOAT,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_half_rtn",   T_HALF,   1, {T_GENTYPE}, {0}, {0}, 1},

    /* Saturated mode */
    {clvEXTENSION_NONE,    "convert_char_sat",       T_CHAR,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_uchar_sat",      T_UCHAR,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_int_sat",        T_INT,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_uint_sat",       T_UINT,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_long_sat",       T_LONG,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_ulong_sat",      T_ULONG,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_short_sat",      T_SHORT,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_ushort_sat",     T_USHORT, 1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE,    "convert_char_sat_rte",   T_CHAR,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_uchar_sat_rte",  T_UCHAR,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_int_sat_rte",    T_INT,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_uint_sat_rte",   T_UINT,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_long_sat_rte",   T_LONG,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_ulong_sat_rte",  T_ULONG,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_short_sat_rte",  T_SHORT,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_ushort_sat_rte", T_USHORT, 1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE,    "convert_char_sat_rtz",   T_CHAR,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_uchar_sat_rtz",  T_UCHAR,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_int_sat_rtz",    T_INT,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_uint_sat_rtz",   T_UINT,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_long_sat_rtz",   T_LONG,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_ulong_sat_rtz",  T_ULONG,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_short_sat_rtz",  T_SHORT,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_ushort_sat_rtz", T_USHORT, 1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE,    "convert_char_sat_rtp",   T_CHAR,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_uchar_sat_rtp",  T_UCHAR,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_int_sat_rtp",    T_INT,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_uint_sat_rtp",   T_UINT,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_long_sat_rtp",   T_LONG,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_ulong_sat_rtp",  T_ULONG,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_short_sat_rtp",  T_SHORT,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_ushort_sat_rtp", T_USHORT, 1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE,    "convert_char_sat_rtn",   T_CHAR,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_uchar_sat_rtn",  T_UCHAR,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_int_sat_rtn",    T_INT,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_uint_sat_rtn",   T_UINT,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_long_sat_rtn",   T_LONG,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_ulong_sat_rtn",  T_ULONG,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_short_sat_rtn",  T_SHORT,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "convert_ushort_sat_rtn", T_USHORT, 1, {T_GENTYPE}, {0}, {0}, 1},

    /* Non-saturated mode 2 */
    {clvEXTENSION_NONE, "convert_char2",       T_CHAR2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar2",      T_UCHAR2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int2",        T_INT2,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint2",       T_UINT2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long2",       T_LONG2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong2",      T_ULONG2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short2",      T_SHORT2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort2",     T_USHORT2, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float2",      T_FLOAT2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_half2",       T_HALF2,  1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char2_rte",   T_CHAR2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar2_rte",  T_UCHAR2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int2_rte",    T_INT2,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint2_rte",   T_UINT2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long2_rte",   T_LONG2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong2_rte",  T_ULONG2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short2_rte",  T_SHORT2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort2_rte", T_USHORT2, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float2_rte",  T_FLOAT2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_half2_rte",       T_HALF2,  1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char2_rtz",   T_CHAR2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar2_rtz",  T_UCHAR2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int2_rtz",    T_INT2,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint2_rtz",   T_UINT2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long2_rtz",   T_LONG2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong2_rtz",  T_ULONG2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short2_rtz",  T_SHORT2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort2_rtz", T_USHORT2, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float2_rtz",  T_FLOAT2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_half2_rtz",       T_HALF2,  1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char2_rtp",   T_CHAR2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar2_rtp",  T_UCHAR2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int2_rtp",    T_INT2,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint2_rtp",   T_UINT2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long2_rtp",   T_LONG2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong2_rtp",  T_ULONG2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short2_rtp",  T_SHORT2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort2_rtp", T_USHORT2, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float2_rtp",  T_FLOAT2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_half2_rtp",       T_HALF2,  1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char2_rtn",   T_CHAR2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar2_rtn",  T_UCHAR2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int2_rtn",    T_INT2,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint2_rtn",   T_UINT2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long2_rtn",   T_LONG2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong2_rtn",  T_ULONG2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short2_rtn",  T_SHORT2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort2_rtn", T_USHORT2, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float2_rtn",  T_FLOAT2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_half2_rtz",       T_HALF2,  1, {T_GENTYPE}, {0}, {0}, 1},

    /* Saturated mode 2 */
    {clvEXTENSION_NONE, "convert_char2_sat",       T_CHAR2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar2_sat",      T_UCHAR2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int2_sat",        T_INT2,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint2_sat",       T_UINT2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long2_sat",       T_LONG2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong2_sat",      T_ULONG2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short2_sat",      T_SHORT2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort2_sat",     T_USHORT2, 1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char2_sat_rte",   T_CHAR2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar2_sat_rte",  T_UCHAR2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int2_sat_rte",    T_INT2,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint2_sat_rte",   T_UINT2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long2_sat_rte",   T_LONG2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong2_sat_rte",  T_ULONG2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short2_sat_rte",  T_SHORT2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort2_sat_rte", T_USHORT2, 1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char2_sat_rtz",   T_CHAR2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar2_sat_rtz",  T_UCHAR2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int2_sat_rtz",    T_INT2,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint2_sat_rtz",   T_UINT2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long2_sat_rtz",   T_LONG2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong2_sat_rtz",  T_ULONG2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short2_sat_rtz",  T_SHORT2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort2_sat_rtz", T_USHORT2, 1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char2_sat_rtp",   T_CHAR2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar2_sat_rtp",  T_UCHAR2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int2_sat_rtp",    T_INT2,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint2_sat_rtp",   T_UINT2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long2_sat_rtp",   T_LONG2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong2_sat_rtp",  T_ULONG2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short2_sat_rtp",  T_SHORT2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort2_sat_rtp", T_USHORT2, 1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char2_sat_rtn",   T_CHAR2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar2_sat_rtn",  T_UCHAR2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int2_sat_rtn",    T_INT2,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint2_sat_rtn",   T_UINT2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long2_sat_rtn",   T_LONG2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong2_sat_rtn",  T_ULONG2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short2_sat_rtn",  T_SHORT2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort2_sat_rtn", T_USHORT2, 1, {T_GENTYPE}, {0}, {0}, 1},

    /* Non-saturated mode 3 */
    {clvEXTENSION_NONE, "convert_char3",       T_CHAR3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar3",      T_UCHAR3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int3",        T_INT3,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint3",       T_UINT3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long3",       T_LONG3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong3",      T_ULONG3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short3",      T_SHORT3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort3",     T_USHORT3, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float3",      T_FLOAT3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_float3",       T_HALF3,  1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char3_rte",   T_CHAR3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar3_rte",  T_UCHAR3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int3_rte",    T_INT3,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint3_rte",   T_UINT3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long3_rte",   T_LONG3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong3_rte",  T_ULONG3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short3_rte",  T_SHORT3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort3_rte", T_USHORT3, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float3_rte",  T_FLOAT3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_float3_rte",       T_HALF3,  1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char3_rtz",   T_CHAR3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar3_rtz",  T_UCHAR3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int3_rtz",    T_INT3,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint3_rtz",   T_UINT3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long3_rtz",   T_LONG3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong3_rtz",  T_ULONG3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short3_rtz",  T_SHORT3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort3_rtz", T_USHORT3, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float3_rtz",  T_FLOAT3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_float3_rtz",       T_HALF3,  1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char3_rtp",   T_CHAR3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar3_rtp",  T_UCHAR3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int3_rtp",    T_INT3,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint3_rtp",   T_UINT3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long3_rtp",   T_LONG3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong3_rtp",  T_ULONG3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short3_rtp",  T_SHORT3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort3_rtp", T_USHORT3, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float3_rtp",  T_FLOAT3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_float3_rtp",       T_HALF3,  1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char3_rtn",   T_CHAR3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar3_rtn",  T_UCHAR3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int3_rtn",    T_INT3,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint3_rtn",   T_UINT3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long3_rtn",   T_LONG3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong3_rtn",  T_ULONG3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short3_rtn",  T_SHORT3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort3_rtn", T_USHORT3, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float3_rtn",  T_FLOAT3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_float3_rtn",       T_HALF3,  1, {T_GENTYPE}, {0}, {0}, 1},

    /* Saturated mode 3 */
    {clvEXTENSION_NONE, "convert_char3_sat",       T_CHAR3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar3_sat",      T_UCHAR3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int3_sat",        T_INT3,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint3_sat",       T_UINT3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long3_sat",       T_LONG3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong3_sat",      T_ULONG3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short3_sat",      T_SHORT3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort3_sat",     T_USHORT3, 1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char3_sat_rte",   T_CHAR3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar3_sat_rte",  T_UCHAR3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int3_sat_rte",    T_INT3,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint3_sat_rte",   T_UINT3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long3_sat_rte",   T_LONG3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong3_sat_rte",  T_ULONG3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short3_sat_rte",  T_SHORT3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort3_sat_rte", T_USHORT3, 1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char3_sat_rtz",   T_CHAR3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar3_sat_rtz",  T_UCHAR3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int3_sat_rtz",    T_INT3,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint3_sat_rtz",   T_UINT3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long3_sat_rtz",   T_LONG3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong3_sat_rtz",  T_ULONG3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short3_sat_rtz",  T_SHORT3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort3_sat_rtz", T_USHORT3, 1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char3_sat_rtp",   T_CHAR3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar3_sat_rtp",  T_UCHAR3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int3_sat_rtp",    T_INT3,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint3_sat_rtp",   T_UINT3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long3_sat_rtp",   T_LONG3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong3_sat_rtp",  T_ULONG3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short3_sat_rtp",  T_SHORT3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort3_sat_rtp", T_USHORT3, 1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char3_sat_rtn",   T_CHAR3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar3_sat_rtn",  T_UCHAR3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int3_sat_rtn",    T_INT3,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint3_sat_rtn",   T_UINT3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long3_sat_rtn",   T_LONG3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong3_sat_rtn",  T_ULONG3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short3_sat_rtn",  T_SHORT3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort3_sat_rtn", T_USHORT3, 1, {T_GENTYPE}, {0}, {0}, 1},

    /* Non-saturated mode 4 */
    {clvEXTENSION_NONE, "convert_char4",       T_CHAR4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar4",      T_UCHAR4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int4",        T_INT4,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint4",       T_UINT4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long4",       T_LONG4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong4",      T_ULONG4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short4",      T_SHORT4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort4",     T_USHORT4, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float4",      T_FLOAT4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_half4",       T_HALF4,  1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char4_rte",   T_CHAR4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar4_rte",  T_UCHAR4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int4_rte",    T_INT4,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint4_rte",   T_UINT4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long4_rte",   T_LONG4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong4_rte",  T_ULONG4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short4_rte",  T_SHORT4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort4_rte", T_USHORT4, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float4_rte",  T_FLOAT4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_half4_rte",       T_HALF4,  1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char4_rtz",   T_CHAR4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar4_rtz",  T_UCHAR4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int4_rtz",    T_INT4,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint4_rtz",   T_UINT4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long4_rtz",   T_LONG4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong4_rtz",  T_ULONG4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short4_rtz",  T_SHORT4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort4_rtz", T_USHORT4, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float4_rtz",  T_FLOAT4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_half4_rtz",       T_HALF4,  1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char4_rtp",   T_CHAR4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar4_rtp",  T_UCHAR4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int4_rtp",    T_INT4,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint4_rtp",   T_UINT4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long4_rtp",   T_LONG4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong4_rtp",  T_ULONG4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short4_rtp",  T_SHORT4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort4_rtp", T_USHORT4, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float4_rtp",  T_FLOAT4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_half4_rtp",       T_HALF4,  1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char4_rtn",   T_CHAR4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar4_rtn",  T_UCHAR4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int4_rtn",    T_INT4,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint4_rtn",   T_UINT4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long4_rtn",   T_LONG4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong4_rtn",  T_ULONG4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short4_rtn",  T_SHORT4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort4_rtn", T_USHORT4, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float4_rtn",  T_FLOAT4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_half4_rtn",       T_HALF4,  1, {T_GENTYPE}, {0}, {0}, 1},

    /* Saturated mode 4 */
    {clvEXTENSION_NONE, "convert_char4_sat",       T_CHAR4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar4_sat",      T_UCHAR4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int4_sat",        T_INT4,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint4_sat",       T_UINT4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long4_sat",       T_LONG4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong4_sat",      T_ULONG4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short4_sat",      T_SHORT4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort4_sat",     T_USHORT4, 1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char4_sat_rte",   T_CHAR4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar4_sat_rte",  T_UCHAR4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int4_sat_rte",    T_INT4,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint4_sat_rte",   T_UINT4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long4_sat_rte",   T_LONG4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong4_sat_rte",  T_ULONG4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short4_sat_rte",  T_SHORT4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort4_sat_rte", T_USHORT4, 1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char4_sat_rtz",   T_CHAR4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar4_sat_rtz",  T_UCHAR4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int4_sat_rtz",    T_INT4,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint4_sat_rtz",   T_UINT4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long4_sat_rtz",   T_LONG4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong4_sat_rtz",  T_ULONG4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short4_sat_rtz",  T_SHORT4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort4_sat_rtz", T_USHORT4, 1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char4_sat_rtp",   T_CHAR4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar4_sat_rtp",  T_UCHAR4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int4_sat_rtp",    T_INT4,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint4_sat_rtp",   T_UINT4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long4_sat_rtp",   T_LONG4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong4_sat_rtp",  T_ULONG4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short4_sat_rtp",  T_SHORT4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort4_sat_rtp", T_USHORT4, 1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char4_sat_rtn",   T_CHAR4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar4_sat_rtn",  T_UCHAR4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int4_sat_rtn",    T_INT4,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint4_sat_rtn",   T_UINT4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long4_sat_rtn",   T_LONG4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong4_sat_rtn",  T_ULONG4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short4_sat_rtn",  T_SHORT4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort4_sat_rtn", T_USHORT4, 1, {T_GENTYPE}, {0}, {0}, 1},

    /* Non-saturated mode 8 */
    {clvEXTENSION_NONE, "convert_char8",       T_CHAR8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar8",      T_UCHAR8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int8",        T_INT8,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint8",       T_UINT8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long8",       T_LONG8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong8",      T_ULONG8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short8",      T_SHORT8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort8",     T_USHORT8, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float8",      T_FLOAT8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_half8",       T_HALF8,  1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char8_rte",   T_CHAR8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar8_rte",  T_UCHAR8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int8_rte",    T_INT8,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint8_rte",   T_UINT8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long8_rte",   T_LONG8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong8_rte",  T_ULONG8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short8_rte",  T_SHORT8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort8_rte", T_USHORT8, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float8_rte",  T_FLOAT8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_half8_rte",       T_HALF8,  1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char8_rtz",   T_CHAR8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar8_rtz",  T_UCHAR8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int8_rtz",    T_INT8,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint8_rtz",   T_UINT8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long8_rtz",   T_LONG8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong8_rtz",  T_ULONG8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short8_rtz",  T_SHORT8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort8_rtz", T_USHORT8, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float8_rtz",  T_FLOAT8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_half8_rtz",       T_HALF8,  1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char8_rtp",   T_CHAR8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar8_rtp",  T_UCHAR8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int8_rtp",    T_INT8,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint8_rtp",   T_UINT8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long8_rtp",   T_LONG8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong8_rtp",  T_ULONG8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short8_rtp",  T_SHORT8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort8_rtp", T_USHORT8, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float8_rtp",  T_FLOAT8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_half8_rtp",       T_HALF8,  1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char8_rtn",   T_CHAR8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar8_rtn",  T_UCHAR8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int8_rtn",    T_INT8,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint8_rtn",   T_UINT8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long8_rtn",   T_LONG8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong8_rtn",  T_ULONG8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short8_rtn",  T_SHORT8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort8_rtn", T_USHORT8, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float8_rtn",  T_FLOAT8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_half8_rtn",       T_HALF8,  1, {T_GENTYPE}, {0}, {0}, 1},

    /* Saturated mode 8 */
    {clvEXTENSION_NONE, "convert_char8_sat",       T_CHAR8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar8_sat",      T_UCHAR8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int8_sat",        T_INT8,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint8_sat",       T_UINT8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long8_sat",       T_LONG8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong8_sat",      T_ULONG8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short8_sat",      T_SHORT8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort8_sat",     T_USHORT8, 1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char8_sat_rte",   T_CHAR8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar8_sat_rte",  T_UCHAR8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int8_sat_rte",    T_INT8,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint8_sat_rte",   T_UINT8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long8_sat_rte",   T_LONG8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong8_sat_rte",  T_ULONG8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short8_sat_rte",  T_SHORT8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort8_sat_rte", T_USHORT8, 1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char8_sat_rtz",   T_CHAR8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar8_sat_rtz",  T_UCHAR8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int8_sat_rtz",    T_INT8,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint8_sat_rtz",   T_UINT8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long8_sat_rtz",   T_LONG8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong8_sat_rtz",  T_ULONG8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short8_sat_rtz",  T_SHORT8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort8_sat_rtz", T_USHORT8, 1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char8_sat_rtp",   T_CHAR8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar8_sat_rtp",  T_UCHAR8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int8_sat_rtp",    T_INT8,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint8_sat_rtp",   T_UINT8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long8_sat_rtp",   T_LONG8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong8_sat_rtp",  T_ULONG8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short8_sat_rtp",  T_SHORT8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort8_sat_rtp", T_USHORT8, 1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char8_sat_rtn",   T_CHAR8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar8_sat_rtn",  T_UCHAR8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int8_sat_rtn",    T_INT8,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint8_sat_rtn",   T_UINT8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long8_sat_rtn",   T_LONG8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong8_sat_rtn",  T_ULONG8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short8_sat_rtn",  T_SHORT8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort8_sat_rtn", T_USHORT8, 1, {T_GENTYPE}, {0}, {0}, 1},

    /* Non-saturated mode 16 */
    {clvEXTENSION_NONE, "convert_char16",       T_CHAR16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar16",      T_UCHAR16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int16",        T_INT16,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint16",       T_UINT16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long16",       T_LONG16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong16",      T_ULONG16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short16",      T_SHORT16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort16",     T_USHORT16, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float16",      T_FLOAT16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_half16",       T_HALF16,  1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char16_rte",   T_CHAR16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar16_rte",  T_UCHAR16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int16_rte",    T_INT16,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint16_rte",   T_UINT16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long16_rte",   T_LONG16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong16_rte",  T_ULONG16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short16_rte",  T_SHORT16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort16_rte", T_USHORT16, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float16_rte",  T_FLOAT16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_half16_rte",       T_HALF16,  1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char16_rtz",   T_CHAR16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar16_rtz",  T_UCHAR16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int16_rtz",    T_INT16,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint16_rtz",   T_UINT16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long16_rtz",   T_LONG16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong16_rtz",  T_ULONG16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short16_rtz",  T_SHORT16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort16_rtz", T_USHORT16, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float16_rtz",  T_FLOAT16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_half16_rtz",       T_HALF16,  1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char16_rtp",   T_CHAR16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar16_rtp",  T_UCHAR16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int16_rtp",    T_INT16,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint16_rtp",   T_UINT16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long16_rtp",   T_LONG16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong16_rtp",  T_ULONG16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short16_rtp",  T_SHORT16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort16_rtp", T_USHORT16, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float16_rtp",  T_FLOAT16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_half16_rtp",       T_HALF16,  1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char16_rtn",   T_CHAR16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar16_rtn",  T_UCHAR16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int16_rtn",    T_INT16,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint16_rtn",   T_UINT16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long16_rtn",   T_LONG16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong16_rtn",  T_ULONG16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short16_rtn",  T_SHORT16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort16_rtn", T_USHORT16, 1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_float16_rtn",  T_FLOAT16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_CL_KHR_FP16, "convert_half16_rtn",       T_HALF16,  1, {T_GENTYPE}, {0}, {0}, 1},

    /* Saturated mode 16 */
    {clvEXTENSION_NONE, "convert_char16_sat",       T_CHAR16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar16_sat",      T_UCHAR16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int16_sat",        T_INT16,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint16_sat",       T_UINT16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long16_sat",       T_LONG16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong16_sat",      T_ULONG16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short16_sat",      T_SHORT16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort16_sat",     T_USHORT16, 1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char16_sat_rte",   T_CHAR16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar16_sat_rte",  T_UCHAR16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int16_sat_rte",    T_INT16,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint16_sat_rte",   T_UINT16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long16_sat_rte",   T_LONG16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong16_sat_rte",  T_ULONG16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short16_sat_rte",  T_SHORT16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort16_sat_rte", T_USHORT16, 1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char16_sat_rtz",   T_CHAR16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar16_sat_rtz",  T_UCHAR16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int16_sat_rtz",    T_INT16,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint16_sat_rtz",   T_UINT16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long16_sat_rtz",   T_LONG16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong16_sat_rtz",  T_ULONG16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short16_sat_rtz",  T_SHORT16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort16_sat_rtz", T_USHORT16, 1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char16_sat_rtp",   T_CHAR16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar16_sat_rtp",  T_UCHAR16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int16_sat_rtp",    T_INT16,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint16_sat_rtp",   T_UINT16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long16_sat_rtp",   T_LONG16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong16_sat_rtp",  T_ULONG16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short16_sat_rtp",  T_SHORT16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort16_sat_rtp", T_USHORT16, 1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "convert_char16_sat_rtn",   T_CHAR16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uchar16_sat_rtn",  T_UCHAR16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_int16_sat_rtn",    T_INT16,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_uint16_sat_rtn",   T_UINT16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_long16_sat_rtn",   T_LONG16,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ulong16_sat_rtn",  T_ULONG16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_short16_sat_rtn",  T_SHORT16,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "convert_ushort16_sat_rtn", T_USHORT16, 1, {T_GENTYPE}, {0}, {0}, 1},

    /* Reinterpreting types */
    {clvEXTENSION_NONE, "as_char",     T_CHAR,     1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_char2",    T_CHAR2,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_char3",    T_CHAR3,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_char4",    T_CHAR4,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_char8",    T_CHAR8,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_char16",   T_CHAR16,   1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "as_uchar",    T_UCHAR,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_uchar2",   T_UCHAR2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_uchar3",   T_UCHAR3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_uchar4",   T_UCHAR4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_uchar8",   T_UCHAR8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_uchar16",  T_UCHAR16,  1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "as_short",    T_SHORT,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_short2",   T_SHORT2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_short3",   T_SHORT3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_short4",   T_SHORT4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_short8",   T_SHORT8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_short16",  T_SHORT16,  1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "as_ushort",   T_USHORT,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_ushort2",  T_USHORT2,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_ushort3",  T_USHORT3,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_ushort4",  T_USHORT4,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_ushort8",  T_USHORT8,  1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_ushort16", T_USHORT16, 1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "as_int",      T_INT,      1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_int2",     T_INT2,     1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_int3",     T_INT3,     1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_int4",     T_INT4,     1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_int8",     T_INT8,     1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_int16",    T_INT16,    1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "as_uint",     T_UINT,     1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_uint2",    T_UINT2,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_uint3",    T_UINT3,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_uint4",    T_UINT4,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_uint8",    T_UINT8,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_uint16",   T_UINT16,   1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "as_long",     T_LONG,     1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_long2",    T_LONG2,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_long3",    T_LONG3,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_long4",    T_LONG4,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_long8",    T_LONG8,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_long16",   T_LONG16,   1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "as_ulong",    T_ULONG,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_ulong2",   T_ULONG2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_ulong3",   T_ULONG3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_ulong4",   T_ULONG4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_ulong8",   T_ULONG8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_ulong16",  T_ULONG16,  1, {T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE, "as_float",    T_FLOAT,    1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_float2",   T_FLOAT2,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_float3",   T_FLOAT3,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_float4",   T_FLOAT4,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_float8",   T_FLOAT8,   1, {T_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE, "as_float16",  T_FLOAT16,  1, {T_GENTYPE}, {0}, {0}, 1},

};

#define _cldConvBuiltinFunctionCount (sizeof(ConvBuiltinFunctions) / sizeof(clsBUILTIN_FUNCTION))

#include "CL/cl_platform.h"

/* Rounding Mode */
typedef enum _cleBUILTIN_ROUNDING_MODE
{
    clvBUILTIN_DEFAULT_ROUNDING_MODE     = 0,
    clvBUILTIN_ROUND_TO_NEAREST_EVEN,
    clvBUILTIN_ROUND_TO_ZERO,
    clvBUILTIN_ROUND_TO_POS_INF,
    clvBUILTIN_ROUND_TO_NEG_INF,
}
cleBUILTIN_ROUNDING_MODE;

/*****************************************************************************\
|*                         Supporting functions                              *|
\*****************************************************************************/

#if _USE_CONV_FOR_EXPLICIT_CONVERT_FUNCTION
static gceSTATUS
_GenConvert_Code(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand,
    IN gctBOOL Saturation,
    IN cleBUILTIN_ROUNDING_MODE RoundingMode
    )
{
    gceSTATUS    status;
    cleOPCODE opcode;

    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* No conversion needed for same types */
    if (OperandsParameters[0].rOperands[0].dataType.elementType == IOperand->dataType.elementType) {
        status = clGenGenericCode1(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   clvOPCODE_ASSIGN,
                                   IOperand,
                                   &OperandsParameters[0].rOperands[0]);
        if (gcmIS_ERROR(status)) return status;
        return gcvSTATUS_OK;
    }

    switch (RoundingMode) {
    case clvBUILTIN_ROUND_TO_NEAREST_EVEN :
        if(Saturation) opcode = clvOPCODE_CONV_SAT_RTE;
        else opcode = clvOPCODE_CONV_RTE;
        break;

    case clvBUILTIN_ROUND_TO_POS_INF :
        if(Saturation) opcode = clvOPCODE_CONV_SAT_RTP;
        else opcode = clvOPCODE_CONV_RTP;
        break;

    case clvBUILTIN_ROUND_TO_NEG_INF :
        if(Saturation) opcode = clvOPCODE_CONV_SAT_RTN;
        else opcode = clvOPCODE_CONV_RTN;
        break;

    case clvBUILTIN_ROUND_TO_ZERO :
        if(Saturation) opcode = clvOPCODE_CONV_SAT_RTZ;
        else opcode = clvOPCODE_CONV_RTZ;
        break;

    default:
        if(Saturation) opcode = clvOPCODE_CONV_SAT;
        else opcode = clvOPCODE_CONV;
        break;
    }

    status = clGenGenericCode1(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               opcode,
                               IOperand,
                               &OperandsParameters[0].rOperands[0]);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}
#else
static gceSTATUS
_GenConvert_Code(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand,
    IN gctBOOL Saturation,
    IN cleBUILTIN_ROUNDING_MODE RoundingMode
    )
{
    gceSTATUS    status;
    clsSELECTION_CONTEXT selectionContextMin, selectionContextMax;
    clsROPERAND dot5ROperand, unsignROperand, oneROperand, zeroROperand;
    clsROPERAND outputTypeMaskROperand, outputTypeMinROperand, outputTypeMaxROperand;
    clsIOPERAND intermIOperands[6];
    clsROPERAND intermROperands[6];
    gctBOOL isInputFloat = gcvFALSE;
    gctBOOL isInputSigned = gcvFALSE;
    gctBOOL isOutputFloat = gcvFALSE;
    gctBOOL isOutputINT = gcvFALSE;
    gctBOOL isOutputUINT = gcvFALSE;

    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    clsIOPERAND_New(Compiler, &intermIOperands[0], OperandsParameters[0].rOperands[0].dataType);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);

    /* No conversion needed for same types */
    if (OperandsParameters[0].rOperands[0].dataType.elementType == IOperand->dataType.elementType) {
        status = clGenGenericCode1(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_ASSIGN,
                    IOperand,
                    &OperandsParameters[0].rOperands[0]);
        if (gcmIS_ERROR(status)) return status;
        return gcvSTATUS_OK;
    }

    switch (OperandsParameters[0].rOperands[0].dataType.elementType) {
        case clvTYPE_FLOAT:
            isInputFloat  = gcvTRUE;
            isInputSigned = gcvTRUE;
            break;

        case clvTYPE_CHAR:
            isInputSigned = gcvTRUE;
            break;

        case clvTYPE_UCHAR:
            break;

        case clvTYPE_SHORT:
            isInputSigned = gcvTRUE;
            break;

        case clvTYPE_USHORT:
            break;

        case clvTYPE_INT:
            isInputSigned = gcvTRUE;
            break;

        case clvTYPE_UINT:
            break;

        default:
            return gcvSTATUS_INVALID_ARGUMENT;
    }

    switch (IOperand->dataType.elementType) {

        case clvTYPE_CHAR:
            clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMaskROperand,
                                    clmGenCodeDataType(T_INT),
                                    0x000000FF);
            if (isInputFloat) {

                clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_INT));
                clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

                clsROPERAND_InitializeFloatOrVecOrMatConstant(&outputTypeMinROperand,
                                        clmGenCodeDataType(T_FLOAT),
                                        (gctFLOAT) CL_CHAR_MIN);

                clsROPERAND_InitializeFloatOrVecOrMatConstant(&outputTypeMaxROperand,
                                        clmGenCodeDataType(T_FLOAT),
                                        (gctFLOAT) CL_CHAR_MAX);
            } else {
                clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMinROperand,
                                        clmGenCodeDataType(T_INT),
                                        (gctINT8) CL_CHAR_MIN);

                clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMaxROperand,
                                        clmGenCodeDataType(T_INT),
                                        (gctINT8) CL_CHAR_MAX);
            }
            break;

        case clvTYPE_UCHAR:
            clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMaskROperand,
                                    clmGenCodeDataType(T_INT),
                                    0x000000FF);
            if (isInputFloat) {

                clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_INT));
                clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

                clsROPERAND_InitializeFloatOrVecOrMatConstant(&outputTypeMinROperand,
                                        clmGenCodeDataType(T_FLOAT),
                                        0.0f);

                clsROPERAND_InitializeFloatOrVecOrMatConstant(&outputTypeMaxROperand,
                                        clmGenCodeDataType(T_FLOAT),
                                        (gctFLOAT) CL_UCHAR_MAX);
            } else {
                clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMinROperand,
                                        clmGenCodeDataType(T_INT),
                                        (gctUINT8) 0);

                clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMaxROperand,
                                        clmGenCodeDataType(T_INT),
                                        (gctUINT8) CL_UCHAR_MAX);
            }
            break;

        case clvTYPE_SHORT:
            clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMaskROperand,
                                    clmGenCodeDataType(T_INT),
                                    0x0000FFFF);
            if (isInputFloat) {

                clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_INT));
                clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

                clsROPERAND_InitializeFloatOrVecOrMatConstant(&outputTypeMinROperand,
                                        clmGenCodeDataType(T_FLOAT),
                                        (gctFLOAT) CL_SHRT_MIN);

                clsROPERAND_InitializeFloatOrVecOrMatConstant(&outputTypeMaxROperand,
                                        clmGenCodeDataType(T_FLOAT),
                                        (gctFLOAT) CL_SHRT_MAX);
            } else {
                clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMinROperand,
                                        clmGenCodeDataType(T_INT),
                                        (gctINT16) CL_SHRT_MIN);

                clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMaxROperand,
                                        clmGenCodeDataType(T_INT),
                                        (gctINT16) CL_SHRT_MAX);
            }
            break;

        case clvTYPE_USHORT:
            clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMaskROperand,
                                    clmGenCodeDataType(T_UINT),
                                    0x0000FFFF);
            if (isInputFloat) {

                clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_INT));
                clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

                clsROPERAND_InitializeFloatOrVecOrMatConstant(&outputTypeMinROperand,
                                        clmGenCodeDataType(T_FLOAT),
                                        (gctFLOAT) 0.0f);

                clsROPERAND_InitializeFloatOrVecOrMatConstant(&outputTypeMaxROperand,
                                        clmGenCodeDataType(T_FLOAT),
                                        (gctFLOAT) CL_USHRT_MAX);
            } else {
                clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMinROperand,
                                        clmGenCodeDataType(T_INT),
                                        (gctUINT16) 0);

                clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMaxROperand,
                                        clmGenCodeDataType(T_INT),
                                        (gctUINT16) CL_USHRT_MAX);
            }
            break;

        case clvTYPE_INT:
            isOutputINT = gcvTRUE;

            if (isInputFloat) {

                clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_INT));
                clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

                clsROPERAND_InitializeFloatOrVecOrMatConstant(&outputTypeMinROperand,
                                        clmGenCodeDataType(T_FLOAT),
                                        (gctFLOAT) CL_INT_MIN);

                clsROPERAND_InitializeFloatOrVecOrMatConstant(&outputTypeMaxROperand,
                                        clmGenCodeDataType(T_FLOAT),
                                        (gctFLOAT) CL_INT_MAX);
            } else {
                clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMinROperand,
                                        clmGenCodeDataType(T_INT),
                                        (gctINT) CL_INT_MIN);

                clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMaxROperand,
                                        clmGenCodeDataType(T_INT),
                                        (gctINT) CL_INT_MAX);
            }
            break;

        case clvTYPE_UINT:
            isOutputUINT = gcvTRUE;

            if (isInputFloat) {

                clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_UINT));
                clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

                clsROPERAND_InitializeFloatOrVecOrMatConstant(&outputTypeMinROperand,
                                        clmGenCodeDataType(T_FLOAT),
                                        (gctFLOAT) 0.0f);

                clsROPERAND_InitializeFloatOrVecOrMatConstant(&outputTypeMaxROperand,
                                        clmGenCodeDataType(T_FLOAT),
                                        (gctFLOAT) CL_UINT_MAX);
            } else {
                clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMinROperand,
                                        clmGenCodeDataType(T_UINT),
                                        (gctUINT) 0);

                clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMaxROperand,
                                        clmGenCodeDataType(T_UINT),
                                        (gctUINT) CL_UINT_MAX);
            }
            break;

        case clvTYPE_FLOAT:

            if (Saturation) {
                return gcvSTATUS_INVALID_ARGUMENT;
            }

            isOutputFloat = gcvTRUE;
            break;

        default:
            return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (Saturation) {
        /* Clamp to <TYPE_MIN, TYPE_MAX> */

        if (isInputSigned && !isOutputINT) {

            /* The selection begins */
            status = clDefineSelectionBegin(Compiler,
                                            CodeGenerator,
                                            gcvTRUE,
                                            &selectionContextMin);
            if (gcmIS_ERROR(status)) return status;

            /* The condition part: arg0 <= TYPE_MIN */
            status = clGenSelectionCompareConditionCode(Compiler,
                                                        CodeGenerator,
                                                        &selectionContextMin,
                                                        PolynaryExpr->exprBase.base.lineNo,
                                                        PolynaryExpr->exprBase.base.stringNo,
                                                        clvCONDITION_LESS_THAN_EQUAL,
                                                        &OperandsParameters[0].rOperands[0],
                                                        &outputTypeMinROperand);
            if (gcmIS_ERROR(status)) return status;

            /* The true part for arg0 <= TYPE_MIN */
            status = clDefineSelectionTrueOperandBegin(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextMin);
            if (gcmIS_ERROR(status)) return status;

            status = clGenGenericCode1(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_ASSIGN,
                        &intermIOperands[0],
                        &outputTypeMinROperand);
            if (gcmIS_ERROR(status)) return status;

            status = clDefineSelectionTrueOperandEnd(Compiler,
                                                    PolynaryExpr->exprBase.base.lineNo,
                                                    PolynaryExpr->exprBase.base.stringNo,
                                                    CodeGenerator,
                                                    &selectionContextMin,
                                                    gcvFALSE);
            if (gcmIS_ERROR(status)) return status;

            /* The false part for arg0 <= TYPE_MIN */
            status = clDefineSelectionFalseOperandBegin(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextMin);
            if (gcmIS_ERROR(status)) return status;
        }

        if (isInputSigned && isOutputUINT) {

            /* Special case - do nothing */

        } else {

            status = clDefineSelectionBegin(Compiler,
                                            CodeGenerator,
                                            gcvTRUE,
                                            &selectionContextMax);
            if (gcmIS_ERROR(status)) return status;

            /* The condition part: arg0 >= TYPE_MAX */
            status = clGenSelectionCompareConditionCode(Compiler,
                                                        CodeGenerator,
                                                        &selectionContextMax,
                                                        PolynaryExpr->exprBase.base.lineNo,
                                                        PolynaryExpr->exprBase.base.stringNo,
                                                        clvCONDITION_GREATER_THAN_EQUAL,
                                                        &OperandsParameters[0].rOperands[0],
                                                        &outputTypeMaxROperand);
            if (gcmIS_ERROR(status)) return status;

            /* The true part for arg0 >= TYPE_MAX */
            status = clDefineSelectionTrueOperandBegin(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextMax);
            if (gcmIS_ERROR(status)) return status;

            status = clGenGenericCode1(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_ASSIGN,
                        &intermIOperands[0],
                        &outputTypeMaxROperand);
            if (gcmIS_ERROR(status)) return status;

            status = clDefineSelectionTrueOperandEnd(Compiler,
                                                    PolynaryExpr->exprBase.base.lineNo,
                                                    PolynaryExpr->exprBase.base.stringNo,
                                                    CodeGenerator,
                                                    &selectionContextMax,
                                                    gcvFALSE);
            if (gcmIS_ERROR(status)) return status;

            /* The false part for arg0 >= TYPE_MAX */
            status = clDefineSelectionFalseOperandBegin(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextMax);
            if (gcmIS_ERROR(status)) return status;

        }
    }

    if (isInputFloat) {

        switch (RoundingMode) {

            case clvBUILTIN_ROUND_TO_NEAREST_EVEN :
                {
                    clsIOPERAND_New(Compiler, &intermIOperands[2], clmGenCodeDataType(T_FLOAT));
                    clsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);

                    clsROPERAND_InitializeFloatOrVecOrMatConstant(&dot5ROperand, clmGenCodeDataType(T_FLOAT), 0.5f);
                    clsROPERAND_InitializeIntOrIVecConstant(&unsignROperand, clmGenCodeDataType(T_UINT), 0x7fffffff);
                    clsROPERAND_InitializeIntOrIVecConstant(&oneROperand, clmGenCodeDataType(T_UINT), 0x00000001);

                    /* r1 = |X| */
                    status = clGenBitwiseExprCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_AND_BITWISE,
                                &intermIOperands[1],
                                &unsignROperand,
                                &OperandsParameters[0].rOperands[0]);
                    if (gcmIS_ERROR(status)) return status;

                    /* r0 = r1 + 0.5 */
                    status = clGenArithmeticExprCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        (CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST) ?
                                            clvOPCODE_ADD_RTZ : clvOPCODE_ADD,
                                        &intermIOperands[0],
                                        &dot5ROperand,
                                        &intermROperands[1]);
                    if (gcmIS_ERROR(status)) return status;

                    /* r2 = floor(r0) */
                    status = clGenGenericCode1(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_FLOOR,
                                &intermIOperands[2],
                                &intermROperands[0]);
                    if (gcmIS_ERROR(status)) return status;

                    /* r0 = frac(r1) */
                    status = clGenGenericCode1(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_FRACT,
                                &intermIOperands[0],
                                &intermROperands[1]);
                    if (gcmIS_ERROR(status)) return status;

                    {
                        clsSELECTION_CONTEXT selectionContextFrac05;
                        status = clDefineSelectionBegin(Compiler,
                                                        CodeGenerator,
                                                        gcvTRUE,
                                                        &selectionContextFrac05);
                        if (gcmIS_ERROR(status)) return status;

                        status = clGenSelectionCompareConditionCode(Compiler,
                                                                CodeGenerator,
                                                                &selectionContextFrac05,
                                                                PolynaryExpr->exprBase.base.lineNo,
                                                                PolynaryExpr->exprBase.base.stringNo,
                                                                clvCONDITION_EQUAL,
                                                                &intermROperands[0],
                                                                &dot5ROperand);
                        if (gcmIS_ERROR(status)) return status;

                        status = clDefineSelectionTrueOperandBegin(Compiler,
                                                                CodeGenerator,
                                                                &selectionContextFrac05);
                        if (gcmIS_ERROR(status)) return status;

                        /* r1 = int(r2) */
                        status = clGenGenericCode1(Compiler,
                                            PolynaryExpr->exprBase.base.lineNo,
                                            PolynaryExpr->exprBase.base.stringNo,
                                            clvOPCODE_FLOAT_TO_INT,
                                            &intermIOperands[1],
                                            &intermROperands[2]);
                        if (gcmIS_ERROR(status)) return status;

                        /* r1 &= 0x01 */
                        status = clGenBitwiseExprCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_AND_BITWISE,
                                    &intermIOperands[1],
                                    &oneROperand,
                                    &intermROperands[1]);
                        if (gcmIS_ERROR(status)) return status;

                        /* r0 = r2 - float(int(r2)%2) */
                        status = clGenGenericCode1(Compiler,
                                            PolynaryExpr->exprBase.base.lineNo,
                                            PolynaryExpr->exprBase.base.stringNo,
                                            (CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST) ?
                                                clvOPCODE_INT_TO_FLOAT_RTZ : clvOPCODE_INT_TO_FLOAT,
                                            &intermIOperands[0],
                                            &intermROperands[1]);
                        if (gcmIS_ERROR(status)) return status;

                        status = clGenArithmeticExprCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        (CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST) ?
                                            clvOPCODE_SUB_RTZ : clvOPCODE_SUB,
                                        &intermIOperands[2],
                                        &intermROperands[2],
                                        &intermROperands[0]);
                        if (gcmIS_ERROR(status)) return status;

                        status = clDefineSelectionTrueOperandEnd(Compiler,
                                                        PolynaryExpr->exprBase.base.lineNo,
                                                        PolynaryExpr->exprBase.base.stringNo,
                                                        CodeGenerator,
                                                        &selectionContextFrac05,
                                                        gcvFALSE);
                        if (gcmIS_ERROR(status)) return status;

                        status = clDefineSelectionFalseOperandBegin(Compiler,
                                                        CodeGenerator,
                                                        &selectionContextFrac05);
                        if (gcmIS_ERROR(status)) return status;

                        status = clDefineSelectionFalseOperandEnd(Compiler,
                                                        CodeGenerator,
                                                        &selectionContextFrac05);
                        if (gcmIS_ERROR(status)) return status;

                        status = clDefineSelectionEnd(Compiler,
                                                        CodeGenerator,
                                                        &selectionContextFrac05);
                        if (gcmIS_ERROR(status)) return status;
                    }

                    status = clGenGenericCode1(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_SIGN,
                                &intermIOperands[0],
                                &OperandsParameters[0].rOperands[0]);
                    if (gcmIS_ERROR(status)) return status;

                    status = clGenArithmeticExprCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        clvOPCODE_MUL,
                                        &intermIOperands[0],
                                        &intermROperands[0],
                                        &intermROperands[2]);
                    if (gcmIS_ERROR(status)) return status;
                }

                break;

            case clvBUILTIN_ROUND_TO_POS_INF :

                status = clGenGenericCode1(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_CEIL,
                                &intermIOperands[0],
                                &OperandsParameters[0].rOperands[0]);
                if (gcmIS_ERROR(status)) return status;

                break;

            case clvBUILTIN_ROUND_TO_NEG_INF :

                status = clGenGenericCode1(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_FLOOR,
                                &intermIOperands[0],
                                &OperandsParameters[0].rOperands[0]);
                if (gcmIS_ERROR(status)) return status;

                break;

            case clvBUILTIN_ROUND_TO_ZERO :
                /* Just assign */

                status = clGenGenericCode1(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_ASSIGN,
                            &intermIOperands[0],
                            &OperandsParameters[0].rOperands[0]);
                if (gcmIS_ERROR(status)) return status;

                break;

            default:
                break;
        }

    } else {

        status = clGenGenericCode1(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_ASSIGN,
                    &intermIOperands[0],
                    &OperandsParameters[0].rOperands[0]);
        if (gcmIS_ERROR(status)) return status;
    }

    if (Saturation) {

        if (isInputSigned && isOutputUINT) {

            /* Special case - do nothing */

        } else {

            status = clDefineSelectionFalseOperandEnd(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextMax);
            if (gcmIS_ERROR(status)) return status;

            /* The selection end */
            status = clDefineSelectionEnd(Compiler,
                                        CodeGenerator,
                                        &selectionContextMax);
            if (gcmIS_ERROR(status)) return status;

        }

        if (isInputSigned && !isOutputINT) {

            status = clDefineSelectionFalseOperandEnd(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextMin);
            if (gcmIS_ERROR(status)) return status;

            /* The selection end */
            status = clDefineSelectionEnd(Compiler,
                                        CodeGenerator,
                                        &selectionContextMin);
            if (gcmIS_ERROR(status)) return status;
        }
    }

    if (isInputFloat) {
        /* Float to integer conversion */

        status = clGenGenericCode1(
                        Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_FLOAT_TO_INT,
                        &intermIOperands[1],
                        &intermROperands[0]);
        if (gcmIS_ERROR(status)) return status;

        if (!isOutputINT && !isOutputUINT)
        {
            status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_AND_BITWISE,
                            IOperand,
                            &outputTypeMaskROperand,
                            &intermROperands[1]);
            if (gcmIS_ERROR(status)) return status;
        }
        else
        {
            status = clGenGenericCode1(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_ASSIGN,
                            IOperand,
                            &intermROperands[1]);
            if (gcmIS_ERROR(status)) return status;
        }

    } else if (isOutputFloat) {
        /* Integer to float conversion */

        switch (RoundingMode) {

            case clvBUILTIN_ROUND_TO_NEAREST_EVEN :
                if (CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST)
                {
                    status = clGenGenericCode1(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_INT_TO_FLOAT_RTNE,
                                    IOperand,
                                    &intermROperands[0]);
                    if (gcmIS_ERROR(status)) return status;
                }
                else
                {
                    clsSELECTION_CONTEXT selectionContextIntConv, selectionContextDiff, selectionContextHalfway, selectionContextZero;

                    clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_FLOAT));
                    clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

                    if (isInputSigned) {
                        clsIOPERAND_New(Compiler, &intermIOperands[2], clmGenCodeDataType(T_INT));
                        clsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);

                        clsIOPERAND_New(Compiler, &intermIOperands[3], clmGenCodeDataType(T_INT));
                        clsROPERAND_InitializeUsingIOperand(&intermROperands[3], &intermIOperands[3]);
                    } else {
                        clsIOPERAND_New(Compiler, &intermIOperands[2], clmGenCodeDataType(T_UINT));
                        clsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);

                        clsIOPERAND_New(Compiler, &intermIOperands[3], clmGenCodeDataType(T_UINT));
                        clsROPERAND_InitializeUsingIOperand(&intermROperands[3], &intermIOperands[3]);
                    }

                    clsIOPERAND_New(Compiler, &intermIOperands[4], clmGenCodeDataType(T_FLOAT));
                    clsROPERAND_InitializeUsingIOperand(&intermROperands[4], &intermIOperands[4]);

                    clsIOPERAND_New(Compiler, &intermIOperands[5], clmGenCodeDataType(T_UINT));
                    clsROPERAND_InitializeUsingIOperand(&intermROperands[5], &intermIOperands[5]);

                    clsROPERAND_InitializeIntOrIVecConstant(&oneROperand, clmGenCodeDataType(T_UINT), 0x00000001);
                    clsROPERAND_InitializeIntOrIVecConstant(&zeroROperand, clmGenCodeDataType(T_UINT), 0x00000000);

                    /* r1 = float(r0) */
                    status = clGenGenericCode1(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    (CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST) ?
                                        clvOPCODE_INT_TO_FLOAT_RTZ : clvOPCODE_INT_TO_FLOAT,
                                    &intermIOperands[1],
                                    &intermROperands[0]);
                    if (gcmIS_ERROR(status)) return status;

                    /* r2 = int(r1) */
                    status = clGenGenericCode1(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_FLOAT_TO_INT,
                                    &intermIOperands[2],
                                    &intermROperands[1]);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionBegin(Compiler,
                                                    CodeGenerator,
                                                    gcvTRUE,
                                                    &selectionContextIntConv);
                    if (gcmIS_ERROR(status)) return status;

                    /* r0 == r2 ? */
                    status = clGenSelectionCompareConditionCode(Compiler,
                                                            CodeGenerator,
                                                            &selectionContextIntConv,
                                                            PolynaryExpr->exprBase.base.lineNo,
                                                            PolynaryExpr->exprBase.base.stringNo,
                                                            clvCONDITION_NOT_EQUAL,
                                                            &intermROperands[0],
                                                            &intermROperands[2]);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionTrueOperandBegin(Compiler,
                                                            CodeGenerator,
                                                            &selectionContextIntConv);
                    if (gcmIS_ERROR(status)) return status;

                    /* r5 = (uint) r1 */
                    status = clGenGenericCode1(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_ASSIGN,
                                &intermIOperands[5],
                                &intermROperands[1]);
                    if (gcmIS_ERROR(status)) return status;

                    /* r5 = ((uint)r1) + 0x1 */
                    status = clGenArithmeticExprCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        clvOPCODE_ADD,
                                        &intermIOperands[5],
                                        &oneROperand,
                                        &intermROperands[5]);
                    if (gcmIS_ERROR(status)) return status;

                    /* r4 = (float) r5 */
                    status = clGenGenericCode1(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_ASSIGN,
                                &intermIOperands[4],
                                &intermROperands[5]);
                    if (gcmIS_ERROR(status)) return status;

                    /* r3 = int(r4) */
                    status = clGenGenericCode1(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_FLOAT_TO_INT,
                                    &intermIOperands[3],
                                    &intermROperands[4]);
                    if (gcmIS_ERROR(status)) return status;

                    /* r3 = r3 - r0 */
                    status = clGenArithmeticExprCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        clvOPCODE_SUB,
                                        &intermIOperands[3],
                                        &intermROperands[3],
                                        &intermROperands[0]);
                    if (gcmIS_ERROR(status)) return status;

                    /* r0 = r0 - r2 */
                    status = clGenArithmeticExprCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        clvOPCODE_SUB,
                                        &intermIOperands[0],
                                        &intermROperands[0],
                                        &intermROperands[2]);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionBegin(Compiler,
                                                    CodeGenerator,
                                                    gcvTRUE,
                                                    &selectionContextHalfway);
                    if (gcmIS_ERROR(status)) return status;

                    /* r3 == r0 ? */
                    status = clGenSelectionCompareConditionCode(Compiler,
                                                            CodeGenerator,
                                                            &selectionContextHalfway,
                                                            PolynaryExpr->exprBase.base.lineNo,
                                                            PolynaryExpr->exprBase.base.stringNo,
                                                            clvCONDITION_EQUAL,
                                                            &intermROperands[3],
                                                            &intermROperands[0]);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionTrueOperandBegin(Compiler,
                                                            CodeGenerator,
                                                            &selectionContextHalfway);
                    if (gcmIS_ERROR(status)) return status;

                    /* r0 = r4 & 0x1 */
                    status = clGenBitwiseExprCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_AND_BITWISE,
                                &intermIOperands[0],
                                &intermROperands[4],
                                &oneROperand);

                    status = clDefineSelectionBegin(Compiler,
                                                    CodeGenerator,
                                                    gcvTRUE,
                                                    &selectionContextZero);
                    if (gcmIS_ERROR(status)) return status;

                    /* r0 == 0 ? */
                    status = clGenSelectionCompareConditionCode(Compiler,
                                                            CodeGenerator,
                                                            &selectionContextZero,
                                                            PolynaryExpr->exprBase.base.lineNo,
                                                            PolynaryExpr->exprBase.base.stringNo,
                                                            clvCONDITION_EQUAL,
                                                            &intermROperands[0],
                                                            &zeroROperand);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionTrueOperandBegin(Compiler,
                                                            CodeGenerator,
                                                            &selectionContextZero);
                    if (gcmIS_ERROR(status)) return status;

                    /* r4 is even, pick r4 */
                    status = clGenGenericCode1(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_ASSIGN,
                                &intermIOperands[1],
                                &intermROperands[4]);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionTrueOperandEnd(Compiler,
                                                    PolynaryExpr->exprBase.base.lineNo,
                                                    PolynaryExpr->exprBase.base.stringNo,
                                                    CodeGenerator,
                                                    &selectionContextZero,
                                                    gcvFALSE);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionFalseOperandBegin(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextZero);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionFalseOperandEnd(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextZero);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionEnd(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextZero);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionTrueOperandEnd(Compiler,
                                                    PolynaryExpr->exprBase.base.lineNo,
                                                    PolynaryExpr->exprBase.base.stringNo,
                                                    CodeGenerator,
                                                    &selectionContextHalfway,
                                                    gcvFALSE);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionFalseOperandBegin(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextHalfway);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionBegin(Compiler,
                                                    CodeGenerator,
                                                    gcvTRUE,
                                                    &selectionContextDiff);
                    if (gcmIS_ERROR(status)) return status;

                    if (isInputSigned) {
                        status = clGenGenericCode1(Compiler,
                                            PolynaryExpr->exprBase.base.lineNo,
                                            PolynaryExpr->exprBase.base.stringNo,
                                            clvOPCODE_ABS,
                                            &intermIOperands[0],
                                            &intermROperands[0]);
                        if (gcmIS_ERROR(status)) return status;

                        status = clGenGenericCode1(Compiler,
                                            PolynaryExpr->exprBase.base.lineNo,
                                            PolynaryExpr->exprBase.base.stringNo,
                                            clvOPCODE_ABS,
                                            &intermIOperands[3],
                                            &intermROperands[3]);
                        if (gcmIS_ERROR(status)) return status;
                    }

                    /* r3 < r0 ? */
                    status = clGenSelectionCompareConditionCode(Compiler,
                                                            CodeGenerator,
                                                            &selectionContextDiff,
                                                            PolynaryExpr->exprBase.base.lineNo,
                                                            PolynaryExpr->exprBase.base.stringNo,
                                                            clvCONDITION_LESS_THAN,
                                                            &intermROperands[3],
                                                            &intermROperands[0]);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionTrueOperandBegin(Compiler,
                                                            CodeGenerator,
                                                            &selectionContextDiff);
                    if (gcmIS_ERROR(status)) return status;

                    /* r4 is nearer, pick r4 */
                    status = clGenGenericCode1(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_ASSIGN,
                                &intermIOperands[1],
                                &intermROperands[4]);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionTrueOperandEnd(Compiler,
                                                    PolynaryExpr->exprBase.base.lineNo,
                                                    PolynaryExpr->exprBase.base.stringNo,
                                                    CodeGenerator,
                                                    &selectionContextDiff,
                                                    gcvFALSE);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionFalseOperandBegin(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextDiff);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionFalseOperandEnd(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextDiff);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionEnd(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextDiff);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionFalseOperandEnd(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextHalfway);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionEnd(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextHalfway);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionTrueOperandEnd(Compiler,
                                                    PolynaryExpr->exprBase.base.lineNo,
                                                    PolynaryExpr->exprBase.base.stringNo,
                                                    CodeGenerator,
                                                    &selectionContextIntConv,
                                                    gcvFALSE);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionFalseOperandBegin(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextIntConv);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionFalseOperandEnd(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextIntConv);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionEnd(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextIntConv);
                    if (gcmIS_ERROR(status)) return status;

                    status = clGenGenericCode1(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_ASSIGN,
                                IOperand,
                                &intermROperands[1]);
                    if (gcmIS_ERROR(status)) return status;

                }
                break;

            case clvBUILTIN_ROUND_TO_POS_INF :
            case clvBUILTIN_ROUND_TO_NEG_INF :
                {
                    clsSELECTION_CONTEXT selectionContextIntConv;
                    clsSELECTION_CONTEXT selectionContextNeg;
                    clsROPERAND zeroROperand;

                    clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_FLOAT));
                    clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

                    if (isInputSigned) {
                        clsIOPERAND_New(Compiler, &intermIOperands[2], clmGenCodeDataType(T_INT));
                        clsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);
                    } else {
                        clsIOPERAND_New(Compiler, &intermIOperands[2], clmGenCodeDataType(T_UINT));
                        clsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);
                    }

                    clsROPERAND_InitializeIntOrIVecConstant(&oneROperand, clmGenCodeDataType(T_UINT), 0x00000001);
                    clsROPERAND_InitializeIntOrIVecConstant(&zeroROperand, clmGenCodeDataType(T_UINT), 0);

                    if (isInputSigned) {

                        status = clDefineSelectionBegin(Compiler,
                                                        CodeGenerator,
                                                        gcvTRUE,
                                                        &selectionContextNeg);
                        if (gcmIS_ERROR(status)) return status;

                        status = clGenSelectionCompareConditionCode(Compiler,
                                                                CodeGenerator,
                                                                &selectionContextNeg,
                                                                PolynaryExpr->exprBase.base.lineNo,
                                                                PolynaryExpr->exprBase.base.stringNo,
                                                                ((RoundingMode == clvBUILTIN_ROUND_TO_POS_INF) ?
                                                                      clvCONDITION_GREATER_THAN :
                                                                      clvCONDITION_LESS_THAN),
                                                                &intermROperands[0],
                                                                &zeroROperand);
                        if (gcmIS_ERROR(status)) return status;

                        status = clDefineSelectionTrueOperandBegin(Compiler,
                                                                CodeGenerator,
                                                                &selectionContextNeg);
                        if (gcmIS_ERROR(status)) return status;

                    }

                    if (isInputSigned || (RoundingMode == clvBUILTIN_ROUND_TO_POS_INF)) {

                        /* r1 = float(r0) */
                        status = clGenGenericCode1(
                                        Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        (CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST) ?
                                            clvOPCODE_INT_TO_FLOAT_RTZ : clvOPCODE_INT_TO_FLOAT,
                                        &intermIOperands[1],
                                        &intermROperands[0]);
                        if (gcmIS_ERROR(status)) return status;

                        /* r2 = int(r1) */
                        status = clGenGenericCode1(
                                        Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        clvOPCODE_FLOAT_TO_INT,
                                        &intermIOperands[2],
                                        &intermROperands[1]);
                        if (gcmIS_ERROR(status)) return status;

                        status = clDefineSelectionBegin(Compiler,
                                                        CodeGenerator,
                                                        gcvTRUE,
                                                        &selectionContextIntConv);
                        if (gcmIS_ERROR(status)) return status;

                        /* r0 == r2 ? */
                        status = clGenSelectionCompareConditionCode(Compiler,
                                                                CodeGenerator,
                                                                &selectionContextIntConv,
                                                                PolynaryExpr->exprBase.base.lineNo,
                                                                PolynaryExpr->exprBase.base.stringNo,
                                                                clvCONDITION_NOT_EQUAL,
                                                                &intermROperands[0],
                                                                &intermROperands[2]);
                        if (gcmIS_ERROR(status)) return status;

                        status = clDefineSelectionTrueOperandBegin(Compiler,
                                                                CodeGenerator,
                                                                &selectionContextIntConv);
                        if (gcmIS_ERROR(status)) return status;

                        /* r2 = ((uint)r1) */
                        status = clGenGenericCode1(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_ASSIGN,
                                    &intermIOperands[2],
                                    &intermROperands[1]);
                        if (gcmIS_ERROR(status)) return status;

                        /* r2 = ((uint)r1) + 0x1 */
                        status = clGenArithmeticExprCode(Compiler,
                                            PolynaryExpr->exprBase.base.lineNo,
                                            PolynaryExpr->exprBase.base.stringNo,
                                            clvOPCODE_ADD,
                                            &intermIOperands[2],
                                            &oneROperand,
                                            &intermROperands[2]);
                        if (gcmIS_ERROR(status)) return status;

                        status = clGenGenericCode1(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_ASSIGN,
                                    IOperand,
                                    &intermROperands[2]);
                        if (gcmIS_ERROR(status)) return status;

                        status = clDefineSelectionTrueOperandEnd(Compiler,
                                                       PolynaryExpr->exprBase.base.lineNo,
                                                       PolynaryExpr->exprBase.base.stringNo,
                                                       CodeGenerator,
                                                        &selectionContextIntConv,
                                                        gcvFALSE);
                        if (gcmIS_ERROR(status)) return status;

                        status = clDefineSelectionFalseOperandBegin(Compiler,
                                                        CodeGenerator,
                                                        &selectionContextIntConv);
                        if (gcmIS_ERROR(status)) return status;

                        status = clGenGenericCode1(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_ASSIGN,
                                    IOperand,
                                    &intermROperands[1]);
                        if (gcmIS_ERROR(status)) return status;

                        status = clDefineSelectionFalseOperandEnd(Compiler,
                                                        CodeGenerator,
                                                        &selectionContextIntConv);
                        if (gcmIS_ERROR(status)) return status;

                        status = clDefineSelectionEnd(Compiler,
                                                        CodeGenerator,
                                                        &selectionContextIntConv);
                        if (gcmIS_ERROR(status)) return status;

                        if (isInputSigned ) {

                            status = clDefineSelectionTrueOperandEnd(Compiler,
                                                            0,
                                                            0,
                                                            CodeGenerator,
                                                            &selectionContextNeg,
                                                            gcvFALSE);
                            if (gcmIS_ERROR(status)) return status;

                            status = clDefineSelectionFalseOperandBegin(Compiler,
                                                            CodeGenerator,
                                                            &selectionContextNeg);
                            if (gcmIS_ERROR(status)) return status;

                            status = clGenGenericCode1(
                                            Compiler,
                                            PolynaryExpr->exprBase.base.lineNo,
                                            PolynaryExpr->exprBase.base.stringNo,
                                            (CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST) ?
                                                clvOPCODE_INT_TO_FLOAT_RTZ : clvOPCODE_INT_TO_FLOAT,
                                            IOperand,
                                            &intermROperands[0]);
                            if (gcmIS_ERROR(status)) return status;

                            status = clDefineSelectionFalseOperandEnd(Compiler,
                                                            CodeGenerator,
                                                            &selectionContextNeg);
                            if (gcmIS_ERROR(status)) return status;

                            status = clDefineSelectionEnd(Compiler,
                                                            CodeGenerator,
                                                            &selectionContextNeg);
                            if (gcmIS_ERROR(status)) return status;

                        }

                    } else {

                        status = clGenGenericCode1(
                                        Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        (CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST) ?
                                            clvOPCODE_INT_TO_FLOAT_RTZ : clvOPCODE_INT_TO_FLOAT,
                                        IOperand,
                                        &intermROperands[0]);
                        if (gcmIS_ERROR(status)) return status;
                    }
                }
                break;

           case clvBUILTIN_ROUND_TO_ZERO :

                    status = clGenGenericCode1(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    (CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST) ?
                                        clvOPCODE_INT_TO_FLOAT_RTZ : clvOPCODE_INT_TO_FLOAT,
                                    IOperand,
                                    &intermROperands[0]);
                    if (gcmIS_ERROR(status)) return status;

                break;

            default:
                break;
        }

    } else {
        /* Integer to integer */

        if (!isOutputINT && !isOutputUINT)
        {
            status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_AND_BITWISE,
                            IOperand,
                            &outputTypeMaskROperand,
                            &intermROperands[0]);
            if (gcmIS_ERROR(status)) return status;
        }
        else
        {
            status = clGenGenericCode1(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_ASSIGN,
                            IOperand,
                            &intermROperands[0]);
            if (gcmIS_ERROR(status)) return status;
        }
    }

    return gcvSTATUS_OK;
}
#endif

#if (_DO_NOT_USE_CONV_FOR_RTNE_EXPLICIT_CONVERT_FUNCTION || _DO_NOT_USE_CONV_FOR_FLOATN_RTP_RTN_EXPLICIT_CONVERT_FUNCTION)
static gceSTATUS
_GenOldConvert_Code(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand,
    IN gctBOOL Saturation,
    IN cleBUILTIN_ROUNDING_MODE RoundingMode
    )
{
    gceSTATUS    status;
    clsSELECTION_CONTEXT selectionContextMin, selectionContextMax;
    clsROPERAND dot5ROperand, unsignROperand, oneROperand, zeroROperand;
    clsROPERAND outputTypeMaskROperand, outputTypeMinROperand, outputTypeMaxROperand;
    clsIOPERAND intermIOperands[6];
    clsROPERAND intermROperands[6];
    gctBOOL isInputFloat = gcvFALSE;
    gctBOOL isInputSigned = gcvFALSE;
    gctBOOL isOutputFloat = gcvFALSE;
    gctBOOL isOutputSigned = gcvFALSE;
    gctBOOL isOutputINT = gcvFALSE;
    gctBOOL isOutputUINT = gcvFALSE;
    cltELEMENT_TYPE elementType;
    gctBOOL isOutputMinOverFlow = gcvFALSE, isOutputMaxOverFlow = gcvFALSE;

    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    clsIOPERAND_New(Compiler, &intermIOperands[0], OperandsParameters[0].rOperands[0].dataType);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);

    /* No conversion needed for same types */
    if (OperandsParameters[0].rOperands[0].dataType.elementType == IOperand->dataType.elementType) {
        status = clGenGenericCode1(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_ASSIGN,
                    IOperand,
                    &OperandsParameters[0].rOperands[0]);
        if (gcmIS_ERROR(status)) return status;
        return gcvSTATUS_OK;
    }

    elementType = OperandsParameters[0].rOperands[0].dataType.elementType;
    if (clmIsElementTypeHighPrecision(elementType) ||
        clmIsElementTypeHighPrecision(IOperand->dataType.elementType) ||
        clmIsElementTypePacked(elementType) ||
        clmIsElementTypePacked(IOperand->dataType.elementType)) {
        return _GenConvert_Code(Compiler,
                                CodeGenerator,
                                PolynaryExpr,
                                OperandCount,
                                OperandsParameters,
                                IOperand,
                                Saturation,
                                RoundingMode);
    }

    switch (elementType) {
        case clvTYPE_FLOAT:
            isInputFloat  = gcvTRUE;
            isInputSigned = gcvTRUE;
            break;

        case clvTYPE_CHAR:
            isInputSigned = gcvTRUE;
            break;

        case clvTYPE_UCHAR:
            break;

        case clvTYPE_SHORT:
            isInputSigned = gcvTRUE;
            break;

        case clvTYPE_USHORT:
            break;

        case clvTYPE_INT:
            isInputSigned = gcvTRUE;
            break;

        case clvTYPE_UINT:
            break;

        case clvTYPE_LONG:
        case clvTYPE_ULONG:
            break;

        default:
            return gcvSTATUS_INVALID_ARGUMENT;
    }

    switch (IOperand->dataType.elementType) {

        case clvTYPE_CHAR:
            isOutputSigned = gcvTRUE;

            clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMaskROperand,
                                    clmGenCodeDataType(T_INT),
                                    0x000000FF);
            if (isInputFloat) {

                clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_INT));
                clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

                clsROPERAND_InitializeFloatOrVecOrMatConstant(&outputTypeMinROperand,
                                        clmGenCodeDataType(T_FLOAT),
                                        (gctFLOAT) CL_CHAR_MIN);

                clsROPERAND_InitializeFloatOrVecOrMatConstant(&outputTypeMaxROperand,
                                        clmGenCodeDataType(T_FLOAT),
                                        (gctFLOAT) CL_CHAR_MAX);
            } else {
                clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMinROperand,
                                        clmGenCodeDataType(T_INT),
                                        (gctINT8) CL_CHAR_MIN);

                clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMaxROperand,
                                        clmGenCodeDataType(T_INT),
                                        (gctINT8) CL_CHAR_MAX);
            }
            break;

        case clvTYPE_UCHAR:
            clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMaskROperand,
                                    clmGenCodeDataType(T_INT),
                                    0x000000FF);
            if (isInputFloat) {

                clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_INT));
                clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

                clsROPERAND_InitializeFloatOrVecOrMatConstant(&outputTypeMinROperand,
                                        clmGenCodeDataType(T_FLOAT),
                                        0.0f);

                clsROPERAND_InitializeFloatOrVecOrMatConstant(&outputTypeMaxROperand,
                                        clmGenCodeDataType(T_FLOAT),
                                        (gctFLOAT) CL_UCHAR_MAX);
            } else {
                clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMinROperand,
                                        clmGenCodeDataType(T_INT),
                                        (gctUINT8) 0);

                clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMaxROperand,
                                        clmGenCodeDataType(T_INT),
                                        (gctUINT8) CL_UCHAR_MAX);
            }
            break;

        case clvTYPE_SHORT:
            isOutputSigned = gcvTRUE;

            clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMaskROperand,
                                    clmGenCodeDataType(T_INT),
                                    0x0000FFFF);
            if (isInputFloat) {

                clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_INT));
                clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

                clsROPERAND_InitializeFloatOrVecOrMatConstant(&outputTypeMinROperand,
                                        clmGenCodeDataType(T_FLOAT),
                                        (gctFLOAT) CL_SHRT_MIN);

                clsROPERAND_InitializeFloatOrVecOrMatConstant(&outputTypeMaxROperand,
                                        clmGenCodeDataType(T_FLOAT),
                                        (gctFLOAT) CL_SHRT_MAX);
            } else {
                clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMinROperand,
                                        clmGenCodeDataType(T_INT),
                                        (gctINT16) CL_SHRT_MIN);

                clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMaxROperand,
                                        clmGenCodeDataType(T_INT),
                                        (gctINT16) CL_SHRT_MAX);
            }
            break;

        case clvTYPE_USHORT:
            clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMaskROperand,
                                    clmGenCodeDataType(T_UINT),
                                    0x0000FFFF);
            if (isInputFloat) {

                clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_INT));
                clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

                clsROPERAND_InitializeFloatOrVecOrMatConstant(&outputTypeMinROperand,
                                        clmGenCodeDataType(T_FLOAT),
                                        (gctFLOAT) 0.0f);

                clsROPERAND_InitializeFloatOrVecOrMatConstant(&outputTypeMaxROperand,
                                        clmGenCodeDataType(T_FLOAT),
                                        (gctFLOAT) CL_USHRT_MAX);
            } else {
                clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMinROperand,
                                        clmGenCodeDataType(T_INT),
                                        (gctUINT16) 0);

                clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMaxROperand,
                                        clmGenCodeDataType(T_INT),
                                        (gctUINT16) CL_USHRT_MAX);
            }
            break;

        case clvTYPE_INT:
            isOutputSigned = gcvTRUE;
            isOutputINT = gcvTRUE;

            if (isInputFloat) {

                clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_INT));
                clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

                clsROPERAND_InitializeFloatOrVecOrMatConstant(&outputTypeMinROperand,
                                        clmGenCodeDataType(T_FLOAT),
                                        (gctFLOAT) CL_INT_MIN);

                clsROPERAND_InitializeFloatOrVecOrMatConstant(&outputTypeMaxROperand,
                                        clmGenCodeDataType(T_FLOAT),
                                        (gctFLOAT) CL_INT_MAX);
            } else {
                clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMinROperand,
                                        clmGenCodeDataType(T_INT),
                                        (gctINT) CL_INT_MIN);

                clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMaxROperand,
                                        clmGenCodeDataType(T_INT),
                                        (gctINT) CL_INT_MAX);
            }
            break;

        case clvTYPE_UINT:
            isOutputUINT = gcvTRUE;

            if (isInputFloat) {

                clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_UINT));
                clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

                clsROPERAND_InitializeFloatOrVecOrMatConstant(&outputTypeMinROperand,
                                        clmGenCodeDataType(T_FLOAT),
                                        (gctFLOAT) 0.0f);

                clsROPERAND_InitializeFloatOrVecOrMatConstant(&outputTypeMaxROperand,
                                        clmGenCodeDataType(T_FLOAT),
                                        (gctFLOAT) CL_UINT_MAX);
            } else {
                clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMinROperand,
                                        clmGenCodeDataType(T_UINT),
                                        (gctUINT) 0);

                clsROPERAND_InitializeIntOrIVecConstant(&outputTypeMaxROperand,
                                        clmGenCodeDataType(T_UINT),
                                        (gctUINT) CL_UINT_MAX);
            }
            break;

        case clvTYPE_FLOAT:

            if (Saturation) {
                return gcvSTATUS_INVALID_ARGUMENT;
            }

            isOutputFloat = gcvTRUE;
            break;

        default:
            return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (IOperand->dataType.elementType > elementType)
    {
        isOutputMaxOverFlow = gcvTRUE;

        if (isInputSigned && isOutputSigned)
        {
            isOutputMinOverFlow = gcvTRUE;
        }
    }

    if (Saturation) {
        /* Clamp to <TYPE_MIN, TYPE_MAX> */

        if (isInputSigned && !isOutputINT) {

            /* The selection begins */
            status = clDefineSelectionBegin(Compiler,
                                            CodeGenerator,
                                            gcvTRUE,
                                            &selectionContextMin);
            if (gcmIS_ERROR(status)) return status;

            /* The condition part: arg0 <= TYPE_MIN */
            if (isOutputMinOverFlow)
            {
                OperandsParameters[0].rOperands[0].dataType.elementType =
                    IOperand->dataType.elementType;
            }
            status = clGenSelectionCompareConditionCode(Compiler,
                                                        CodeGenerator,
                                                        &selectionContextMin,
                                                        PolynaryExpr->exprBase.base.lineNo,
                                                        PolynaryExpr->exprBase.base.stringNo,
                                                        clvCONDITION_LESS_THAN_EQUAL,
                                                        &OperandsParameters[0].rOperands[0],
                                                        &outputTypeMinROperand);
            if (gcmIS_ERROR(status)) return status;

            if (isOutputMinOverFlow)
            {
                OperandsParameters[0].rOperands[0].dataType.elementType = elementType;
            }

            /* The true part for arg0 <= TYPE_MIN */
            status = clDefineSelectionTrueOperandBegin(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextMin);
            if (gcmIS_ERROR(status)) return status;

            status = clGenGenericCode1(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_ASSIGN,
                        &intermIOperands[0],
                        &outputTypeMinROperand);
            if (gcmIS_ERROR(status)) return status;

            status = clDefineSelectionTrueOperandEnd(Compiler,
                                                    PolynaryExpr->exprBase.base.lineNo,
                                                    PolynaryExpr->exprBase.base.stringNo,
                                                    CodeGenerator,
                                                    &selectionContextMin,
                                                    gcvFALSE);
            if (gcmIS_ERROR(status)) return status;

            /* The false part for arg0 <= TYPE_MIN */
            status = clDefineSelectionFalseOperandBegin(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextMin);
            if (gcmIS_ERROR(status)) return status;
        }

        if (isInputSigned && isOutputUINT) {

            /* Special case - do nothing */

        } else {

            status = clDefineSelectionBegin(Compiler,
                                            CodeGenerator,
                                            gcvTRUE,
                                            &selectionContextMax);
            if (gcmIS_ERROR(status)) return status;

            /* The condition part: arg0 >= TYPE_MAX */
            if (isOutputMaxOverFlow)
            {
                OperandsParameters[0].rOperands[0].dataType.elementType =
                    IOperand->dataType.elementType;
            }

            status = clGenSelectionCompareConditionCode(Compiler,
                                                        CodeGenerator,
                                                        &selectionContextMax,
                                                        PolynaryExpr->exprBase.base.lineNo,
                                                        PolynaryExpr->exprBase.base.stringNo,
                                                        clvCONDITION_GREATER_THAN_EQUAL,
                                                        &OperandsParameters[0].rOperands[0],
                                                        &outputTypeMaxROperand);
            if (gcmIS_ERROR(status)) return status;

            if (isOutputMaxOverFlow)
            {
                OperandsParameters[0].rOperands[0].dataType.elementType = elementType;
            }

            /* The true part for arg0 >= TYPE_MAX */
            status = clDefineSelectionTrueOperandBegin(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextMax);
            if (gcmIS_ERROR(status)) return status;

            status = clGenGenericCode1(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_ASSIGN,
                        &intermIOperands[0],
                        &outputTypeMaxROperand);
            if (gcmIS_ERROR(status)) return status;

            status = clDefineSelectionTrueOperandEnd(Compiler,
                                                    PolynaryExpr->exprBase.base.lineNo,
                                                    PolynaryExpr->exprBase.base.stringNo,
                                                    CodeGenerator,
                                                    &selectionContextMax,
                                                    gcvFALSE);
            if (gcmIS_ERROR(status)) return status;

            /* The false part for arg0 >= TYPE_MAX */
            status = clDefineSelectionFalseOperandBegin(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextMax);
            if (gcmIS_ERROR(status)) return status;

        }
    }

    if (isInputFloat) {

        switch (RoundingMode) {

            case clvBUILTIN_ROUND_TO_NEAREST_EVEN :
                {
                    clsIOPERAND tempIOperand[1];
                    clsROPERAND tempROperand[1];

                    clsIOPERAND_New(Compiler, &intermIOperands[2], clmGenCodeDataType(T_FLOAT));
                    clsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);

                    clsROPERAND_InitializeFloatOrVecOrMatConstant(&dot5ROperand, clmGenCodeDataType(T_FLOAT), 0.5f);
                    clsROPERAND_InitializeIntOrIVecConstant(&unsignROperand, clmGenCodeDataType(T_UINT), 0x7fffffff);
                    clsROPERAND_InitializeIntOrIVecConstant(&oneROperand, clmGenCodeDataType(T_UINT), 0x00000001);

                    /* r1 = |X| */
                    status = clGenBitwiseExprCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_AND_BITWISE,
                                &intermIOperands[1],
                                &unsignROperand,
                                &OperandsParameters[0].rOperands[0]);
                    if (gcmIS_ERROR(status)) return status;

                    /* r0 = r1 + 0.5 */
                    /* Make sure that all source type is FLOAT. */
                    clsIOPERAND_Initialize(Compiler, &tempIOperand[0], clmGenCodeDataType(T_FLOAT), intermIOperands[1].tempRegIndex);
                    clsROPERAND_InitializeUsingIOperand(&tempROperand[0], &tempIOperand[0]);

                    status = clGenArithmeticExprCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        (CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST) ?
                                            clvOPCODE_ADD_RTZ : clvOPCODE_ADD,
                                        &intermIOperands[0],
                                        &dot5ROperand,
                                        &tempROperand[0]);
                    if (gcmIS_ERROR(status)) return status;

                    /* r2 = floor(r0) */
                    status = clGenGenericCode1(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_FLOOR,
                                &intermIOperands[2],
                                &intermROperands[0]);
                    if (gcmIS_ERROR(status)) return status;

                    /* r0 = frac(r1) */
                    status = clGenGenericCode1(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_FRACT,
                                &intermIOperands[0],
                                &tempROperand[0]);
                    if (gcmIS_ERROR(status)) return status;

                    {
                        clsSELECTION_CONTEXT selectionContextFrac05;
                        status = clDefineSelectionBegin(Compiler,
                                                        CodeGenerator,
                                                        gcvTRUE,
                                                        &selectionContextFrac05);
                        if (gcmIS_ERROR(status)) return status;

                        status = clGenSelectionCompareConditionCode(Compiler,
                                                                CodeGenerator,
                                                                &selectionContextFrac05,
                                                                PolynaryExpr->exprBase.base.lineNo,
                                                                PolynaryExpr->exprBase.base.stringNo,
                                                                clvCONDITION_EQUAL,
                                                                &intermROperands[0],
                                                                &dot5ROperand);
                        if (gcmIS_ERROR(status)) return status;

                        status = clDefineSelectionTrueOperandBegin(Compiler,
                                                                CodeGenerator,
                                                                &selectionContextFrac05);
                        if (gcmIS_ERROR(status)) return status;

                        /* r1 = int(r2) */
                        status = clGenGenericCode1(Compiler,
                                            PolynaryExpr->exprBase.base.lineNo,
                                            PolynaryExpr->exprBase.base.stringNo,
                                            clvOPCODE_FLOAT_TO_INT,
                                            &intermIOperands[1],
                                            &intermROperands[2]);
                        if (gcmIS_ERROR(status)) return status;

                        /* r1 &= 0x01 */
                        status = clGenBitwiseExprCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_AND_BITWISE,
                                    &intermIOperands[1],
                                    &oneROperand,
                                    &intermROperands[1]);
                        if (gcmIS_ERROR(status)) return status;

                        /* r0 = r2 - float(int(r2)%2) */
                        status = clGenGenericCode1(Compiler,
                                            PolynaryExpr->exprBase.base.lineNo,
                                            PolynaryExpr->exprBase.base.stringNo,
                                            (CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST) ?
                                                clvOPCODE_INT_TO_FLOAT_RTZ : clvOPCODE_INT_TO_FLOAT,
                                            &intermIOperands[0],
                                            &intermROperands[1]);
                        if (gcmIS_ERROR(status)) return status;

                        status = clGenArithmeticExprCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        (CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST) ?
                                            clvOPCODE_SUB_RTZ : clvOPCODE_SUB,
                                        &intermIOperands[2],
                                        &intermROperands[2],
                                        &intermROperands[0]);
                        if (gcmIS_ERROR(status)) return status;

                        status = clDefineSelectionTrueOperandEnd(Compiler,
                                                        PolynaryExpr->exprBase.base.lineNo,
                                                        PolynaryExpr->exprBase.base.stringNo,
                                                        CodeGenerator,
                                                        &selectionContextFrac05,
                                                        gcvFALSE);
                        if (gcmIS_ERROR(status)) return status;

                        status = clDefineSelectionFalseOperandBegin(Compiler,
                                                        CodeGenerator,
                                                        &selectionContextFrac05);
                        if (gcmIS_ERROR(status)) return status;

                        status = clDefineSelectionFalseOperandEnd(Compiler,
                                                        CodeGenerator,
                                                        &selectionContextFrac05);
                        if (gcmIS_ERROR(status)) return status;

                        status = clDefineSelectionEnd(Compiler,
                                                        CodeGenerator,
                                                        &selectionContextFrac05);
                        if (gcmIS_ERROR(status)) return status;
                    }

                    status = clGenGenericCode1(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_SIGN,
                                &intermIOperands[0],
                                &OperandsParameters[0].rOperands[0]);
                    if (gcmIS_ERROR(status)) return status;

                    status = clGenArithmeticExprCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        clvOPCODE_MUL,
                                        &intermIOperands[0],
                                        &intermROperands[0],
                                        &intermROperands[2]);
                    if (gcmIS_ERROR(status)) return status;
                }

                break;

            case clvBUILTIN_ROUND_TO_POS_INF :

                status = clGenGenericCode1(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_CEIL,
                                &intermIOperands[0],
                                &OperandsParameters[0].rOperands[0]);
                if (gcmIS_ERROR(status)) return status;

                break;

            case clvBUILTIN_ROUND_TO_NEG_INF :

                status = clGenGenericCode1(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_FLOOR,
                                &intermIOperands[0],
                                &OperandsParameters[0].rOperands[0]);
                if (gcmIS_ERROR(status)) return status;

                break;

            case clvBUILTIN_ROUND_TO_ZERO :
                /* Just assign */

                status = clGenGenericCode1(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_ASSIGN,
                            &intermIOperands[0],
                            &OperandsParameters[0].rOperands[0]);
                if (gcmIS_ERROR(status)) return status;

                break;

            default:
                break;
        }

    } else {

        status = clGenGenericCode1(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_ASSIGN,
                    &intermIOperands[0],
                    &OperandsParameters[0].rOperands[0]);
        if (gcmIS_ERROR(status)) return status;
    }

    if (Saturation) {

        if (isInputSigned && isOutputUINT) {

            /* Special case - do nothing */

        } else {

            status = clDefineSelectionFalseOperandEnd(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextMax);
            if (gcmIS_ERROR(status)) return status;

            /* The selection end */
            status = clDefineSelectionEnd(Compiler,
                                        CodeGenerator,
                                        &selectionContextMax);
            if (gcmIS_ERROR(status)) return status;

        }

        if (isInputSigned && !isOutputINT) {

            status = clDefineSelectionFalseOperandEnd(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextMin);
            if (gcmIS_ERROR(status)) return status;

            /* The selection end */
            status = clDefineSelectionEnd(Compiler,
                                        CodeGenerator,
                                        &selectionContextMin);
            if (gcmIS_ERROR(status)) return status;
        }
    }

    if (isInputFloat) {
        /* Float to integer conversion */

        status = clGenGenericCode1(
                        Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_FLOAT_TO_INT,
                        &intermIOperands[1],
                        &intermROperands[0]);
        if (gcmIS_ERROR(status)) return status;

        if (!isOutputINT && !isOutputUINT)
        {
            status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_AND_BITWISE,
                            IOperand,
                            &outputTypeMaskROperand,
                            &intermROperands[1]);
            if (gcmIS_ERROR(status)) return status;
        }
        else
        {
            status = clGenGenericCode1(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_ASSIGN,
                            IOperand,
                            &intermROperands[1]);
            if (gcmIS_ERROR(status)) return status;
        }

    } else if (isOutputFloat) {
        /* Integer to float conversion */

        switch (RoundingMode) {

            case clvBUILTIN_ROUND_TO_NEAREST_EVEN :
                if (CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST)
                {
                    status = clGenGenericCode1(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_INT_TO_FLOAT_RTNE,
                                    IOperand,
                                    &intermROperands[0]);
                    if (gcmIS_ERROR(status)) return status;
                }
                else
                {
                    clsSELECTION_CONTEXT selectionContextIntConv, selectionContextDiff, selectionContextHalfway, selectionContextZero;

                    clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_FLOAT));
                    clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

                    if (isInputSigned) {
                        clsIOPERAND_New(Compiler, &intermIOperands[2], clmGenCodeDataType(T_INT));
                        clsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);

                        clsIOPERAND_New(Compiler, &intermIOperands[3], clmGenCodeDataType(T_INT));
                        clsROPERAND_InitializeUsingIOperand(&intermROperands[3], &intermIOperands[3]);
                    } else {
                        clsIOPERAND_New(Compiler, &intermIOperands[2], clmGenCodeDataType(T_UINT));
                        clsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);

                        clsIOPERAND_New(Compiler, &intermIOperands[3], clmGenCodeDataType(T_UINT));
                        clsROPERAND_InitializeUsingIOperand(&intermROperands[3], &intermIOperands[3]);
                    }

                    clsIOPERAND_New(Compiler, &intermIOperands[4], clmGenCodeDataType(T_FLOAT));
                    clsROPERAND_InitializeUsingIOperand(&intermROperands[4], &intermIOperands[4]);

                    clsIOPERAND_New(Compiler, &intermIOperands[5], clmGenCodeDataType(T_UINT));
                    clsROPERAND_InitializeUsingIOperand(&intermROperands[5], &intermIOperands[5]);

                    clsROPERAND_InitializeIntOrIVecConstant(&oneROperand, clmGenCodeDataType(T_UINT), 0x00000001);
                    clsROPERAND_InitializeIntOrIVecConstant(&zeroROperand, clmGenCodeDataType(T_UINT), 0x00000000);

                    /* r1 = float(r0) */
                    status = clGenGenericCode1(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    (CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST) ?
                                        clvOPCODE_INT_TO_FLOAT_RTZ : clvOPCODE_INT_TO_FLOAT,
                                    &intermIOperands[1],
                                    &intermROperands[0]);
                    if (gcmIS_ERROR(status)) return status;

                    /* r2 = int(r1) */
                    status = clGenGenericCode1(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_FLOAT_TO_INT,
                                    &intermIOperands[2],
                                    &intermROperands[1]);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionBegin(Compiler,
                                                    CodeGenerator,
                                                    gcvTRUE,
                                                    &selectionContextIntConv);
                    if (gcmIS_ERROR(status)) return status;

                    /* r0 == r2 ? */
                    status = clGenSelectionCompareConditionCode(Compiler,
                                                            CodeGenerator,
                                                            &selectionContextIntConv,
                                                            PolynaryExpr->exprBase.base.lineNo,
                                                            PolynaryExpr->exprBase.base.stringNo,
                                                            clvCONDITION_NOT_EQUAL,
                                                            &intermROperands[0],
                                                            &intermROperands[2]);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionTrueOperandBegin(Compiler,
                                                            CodeGenerator,
                                                            &selectionContextIntConv);
                    if (gcmIS_ERROR(status)) return status;

                    /* r5 = (uint) r1 */
                    status = clGenGenericCode1(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_ASSIGN,
                                &intermIOperands[5],
                                &intermROperands[1]);
                    if (gcmIS_ERROR(status)) return status;

                    /* r5 = ((uint)r1) + 0x1 */
                    status = clGenArithmeticExprCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        clvOPCODE_ADD,
                                        &intermIOperands[5],
                                        &oneROperand,
                                        &intermROperands[5]);
                    if (gcmIS_ERROR(status)) return status;

                    /* r4 = (float) r5 */
                    status = clGenGenericCode1(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_ASSIGN,
                                &intermIOperands[4],
                                &intermROperands[5]);
                    if (gcmIS_ERROR(status)) return status;

                    /* r3 = int(r4) */
                    status = clGenGenericCode1(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_FLOAT_TO_INT,
                                    &intermIOperands[3],
                                    &intermROperands[4]);
                    if (gcmIS_ERROR(status)) return status;

                    /* r3 = r3 - r0 */
                    status = clGenArithmeticExprCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        clvOPCODE_SUB,
                                        &intermIOperands[3],
                                        &intermROperands[3],
                                        &intermROperands[0]);
                    if (gcmIS_ERROR(status)) return status;

                    /* r0 = r0 - r2 */
                    status = clGenArithmeticExprCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        clvOPCODE_SUB,
                                        &intermIOperands[0],
                                        &intermROperands[0],
                                        &intermROperands[2]);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionBegin(Compiler,
                                                    CodeGenerator,
                                                    gcvTRUE,
                                                    &selectionContextHalfway);
                    if (gcmIS_ERROR(status)) return status;

                    /* r3 == r0 ? */
                    status = clGenSelectionCompareConditionCode(Compiler,
                                                            CodeGenerator,
                                                            &selectionContextHalfway,
                                                            PolynaryExpr->exprBase.base.lineNo,
                                                            PolynaryExpr->exprBase.base.stringNo,
                                                            clvCONDITION_EQUAL,
                                                            &intermROperands[3],
                                                            &intermROperands[0]);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionTrueOperandBegin(Compiler,
                                                            CodeGenerator,
                                                            &selectionContextHalfway);
                    if (gcmIS_ERROR(status)) return status;

                    /* r0 = r4 & 0x1 */
                    status = clGenBitwiseExprCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_AND_BITWISE,
                                &intermIOperands[0],
                                &intermROperands[4],
                                &oneROperand);

                    status = clDefineSelectionBegin(Compiler,
                                                    CodeGenerator,
                                                    gcvTRUE,
                                                    &selectionContextZero);
                    if (gcmIS_ERROR(status)) return status;

                    /* r0 == 0 ? */
                    status = clGenSelectionCompareConditionCode(Compiler,
                                                            CodeGenerator,
                                                            &selectionContextZero,
                                                            PolynaryExpr->exprBase.base.lineNo,
                                                            PolynaryExpr->exprBase.base.stringNo,
                                                            clvCONDITION_EQUAL,
                                                            &intermROperands[0],
                                                            &zeroROperand);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionTrueOperandBegin(Compiler,
                                                            CodeGenerator,
                                                            &selectionContextZero);
                    if (gcmIS_ERROR(status)) return status;

                    /* r4 is even, pick r4 */
                    status = clGenGenericCode1(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_ASSIGN,
                                &intermIOperands[1],
                                &intermROperands[4]);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionTrueOperandEnd(Compiler,
                                                    PolynaryExpr->exprBase.base.lineNo,
                                                    PolynaryExpr->exprBase.base.stringNo,
                                                    CodeGenerator,
                                                    &selectionContextZero,
                                                    gcvFALSE);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionFalseOperandBegin(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextZero);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionFalseOperandEnd(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextZero);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionEnd(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextZero);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionTrueOperandEnd(Compiler,
                                                    PolynaryExpr->exprBase.base.lineNo,
                                                    PolynaryExpr->exprBase.base.stringNo,
                                                    CodeGenerator,
                                                    &selectionContextHalfway,
                                                    gcvFALSE);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionFalseOperandBegin(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextHalfway);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionBegin(Compiler,
                                                    CodeGenerator,
                                                    gcvTRUE,
                                                    &selectionContextDiff);
                    if (gcmIS_ERROR(status)) return status;

                    if (isInputSigned) {
                        status = clGenGenericCode1(Compiler,
                                            PolynaryExpr->exprBase.base.lineNo,
                                            PolynaryExpr->exprBase.base.stringNo,
                                            clvOPCODE_ABS,
                                            &intermIOperands[0],
                                            &intermROperands[0]);
                        if (gcmIS_ERROR(status)) return status;

                        status = clGenGenericCode1(Compiler,
                                            PolynaryExpr->exprBase.base.lineNo,
                                            PolynaryExpr->exprBase.base.stringNo,
                                            clvOPCODE_ABS,
                                            &intermIOperands[3],
                                            &intermROperands[3]);
                        if (gcmIS_ERROR(status)) return status;
                    }

                    /* r3 < r0 ? */
                    status = clGenSelectionCompareConditionCode(Compiler,
                                                            CodeGenerator,
                                                            &selectionContextDiff,
                                                            PolynaryExpr->exprBase.base.lineNo,
                                                            PolynaryExpr->exprBase.base.stringNo,
                                                            clvCONDITION_LESS_THAN,
                                                            &intermROperands[3],
                                                            &intermROperands[0]);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionTrueOperandBegin(Compiler,
                                                            CodeGenerator,
                                                            &selectionContextDiff);
                    if (gcmIS_ERROR(status)) return status;

                    /* r4 is nearer, pick r4 */
                    status = clGenGenericCode1(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_ASSIGN,
                                &intermIOperands[1],
                                &intermROperands[4]);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionTrueOperandEnd(Compiler,
                                                    PolynaryExpr->exprBase.base.lineNo,
                                                    PolynaryExpr->exprBase.base.stringNo,
                                                    CodeGenerator,
                                                    &selectionContextDiff,
                                                    gcvFALSE);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionFalseOperandBegin(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextDiff);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionFalseOperandEnd(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextDiff);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionEnd(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextDiff);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionFalseOperandEnd(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextHalfway);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionEnd(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextHalfway);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionTrueOperandEnd(Compiler,
                                                    PolynaryExpr->exprBase.base.lineNo,
                                                    PolynaryExpr->exprBase.base.stringNo,
                                                    CodeGenerator,
                                                    &selectionContextIntConv,
                                                    gcvFALSE);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionFalseOperandBegin(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextIntConv);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionFalseOperandEnd(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextIntConv);
                    if (gcmIS_ERROR(status)) return status;

                    status = clDefineSelectionEnd(Compiler,
                                                    CodeGenerator,
                                                    &selectionContextIntConv);
                    if (gcmIS_ERROR(status)) return status;

                    status = clGenGenericCode1(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_ASSIGN,
                                IOperand,
                                &intermROperands[1]);
                    if (gcmIS_ERROR(status)) return status;

                }
                break;

            case clvBUILTIN_ROUND_TO_POS_INF :
            case clvBUILTIN_ROUND_TO_NEG_INF :
                {
                    clsSELECTION_CONTEXT selectionContextIntConv;
                    clsSELECTION_CONTEXT selectionContextNeg;
                    clsROPERAND zeroROperand;

                    clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_FLOAT));
                    clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

                    if (isInputSigned) {
                        clsIOPERAND_New(Compiler, &intermIOperands[2], clmGenCodeDataType(T_INT));
                        clsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);
                    } else {
                        clsIOPERAND_New(Compiler, &intermIOperands[2], clmGenCodeDataType(T_UINT));
                        clsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);
                    }

                    clsROPERAND_InitializeIntOrIVecConstant(&oneROperand, clmGenCodeDataType(T_UINT), 0x00000001);
                    clsROPERAND_InitializeIntOrIVecConstant(&zeroROperand, clmGenCodeDataType(T_UINT), 0);

                    if (isInputSigned) {

                        status = clDefineSelectionBegin(Compiler,
                                                        CodeGenerator,
                                                        gcvTRUE,
                                                        &selectionContextNeg);
                        if (gcmIS_ERROR(status)) return status;

                        status = clGenSelectionCompareConditionCode(Compiler,
                                                                CodeGenerator,
                                                                &selectionContextNeg,
                                                                PolynaryExpr->exprBase.base.lineNo,
                                                                PolynaryExpr->exprBase.base.stringNo,
                                                                ((RoundingMode == clvBUILTIN_ROUND_TO_POS_INF) ?
                                                                      clvCONDITION_GREATER_THAN :
                                                                      clvCONDITION_LESS_THAN),
                                                                &intermROperands[0],
                                                                &zeroROperand);
                        if (gcmIS_ERROR(status)) return status;

                        status = clDefineSelectionTrueOperandBegin(Compiler,
                                                                CodeGenerator,
                                                                &selectionContextNeg);
                        if (gcmIS_ERROR(status)) return status;

                    }

                    if (isInputSigned || (RoundingMode == clvBUILTIN_ROUND_TO_POS_INF)) {

                        /* r1 = float(r0) */
                        status = clGenGenericCode1(
                                        Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        (CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST) ?
                                            clvOPCODE_INT_TO_FLOAT_RTZ : clvOPCODE_INT_TO_FLOAT,
                                        &intermIOperands[1],
                                        &intermROperands[0]);
                        if (gcmIS_ERROR(status)) return status;

                        /* r2 = int(r1) */
                        status = clGenGenericCode1(
                                        Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        clvOPCODE_FLOAT_TO_INT,
                                        &intermIOperands[2],
                                        &intermROperands[1]);
                        if (gcmIS_ERROR(status)) return status;

                        status = clDefineSelectionBegin(Compiler,
                                                        CodeGenerator,
                                                        gcvTRUE,
                                                        &selectionContextIntConv);
                        if (gcmIS_ERROR(status)) return status;

                        /* r0 == r2 ? */
                        status = clGenSelectionCompareConditionCode(Compiler,
                                                                CodeGenerator,
                                                                &selectionContextIntConv,
                                                                PolynaryExpr->exprBase.base.lineNo,
                                                                PolynaryExpr->exprBase.base.stringNo,
                                                                clvCONDITION_NOT_EQUAL,
                                                                &intermROperands[0],
                                                                &intermROperands[2]);
                        if (gcmIS_ERROR(status)) return status;

                        status = clDefineSelectionTrueOperandBegin(Compiler,
                                                                CodeGenerator,
                                                                &selectionContextIntConv);
                        if (gcmIS_ERROR(status)) return status;

                        /* r2 = ((uint)r1) */
                        status = clGenGenericCode1(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_ASSIGN,
                                    &intermIOperands[2],
                                    &intermROperands[1]);
                        if (gcmIS_ERROR(status)) return status;

                        /* r2 = ((uint)r1) + 0x1 */
                        status = clGenArithmeticExprCode(Compiler,
                                            PolynaryExpr->exprBase.base.lineNo,
                                            PolynaryExpr->exprBase.base.stringNo,
                                            clvOPCODE_ADD,
                                            &intermIOperands[2],
                                            &oneROperand,
                                            &intermROperands[2]);
                        if (gcmIS_ERROR(status)) return status;

                        status = clGenGenericCode1(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_ASSIGN,
                                    IOperand,
                                    &intermROperands[2]);
                        if (gcmIS_ERROR(status)) return status;

                        status = clDefineSelectionTrueOperandEnd(Compiler,
                                                        PolynaryExpr->exprBase.base.lineNo,
                                                        PolynaryExpr->exprBase.base.stringNo,
                                                        CodeGenerator,
                                                        &selectionContextIntConv,
                                                        gcvFALSE);
                        if (gcmIS_ERROR(status)) return status;

                        status = clDefineSelectionFalseOperandBegin(Compiler,
                                                        CodeGenerator,
                                                        &selectionContextIntConv);
                        if (gcmIS_ERROR(status)) return status;

                        status = clGenGenericCode1(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_ASSIGN,
                                    IOperand,
                                    &intermROperands[1]);
                        if (gcmIS_ERROR(status)) return status;

                        status = clDefineSelectionFalseOperandEnd(Compiler,
                                                        CodeGenerator,
                                                        &selectionContextIntConv);
                        if (gcmIS_ERROR(status)) return status;

                        status = clDefineSelectionEnd(Compiler,
                                                        CodeGenerator,
                                                        &selectionContextIntConv);
                        if (gcmIS_ERROR(status)) return status;

                        if (isInputSigned ) {

                            status = clDefineSelectionTrueOperandEnd(Compiler,
                                                            PolynaryExpr->exprBase.base.lineNo,
                                                            PolynaryExpr->exprBase.base.stringNo,
                                                            CodeGenerator,
                                                            &selectionContextNeg,
                                                            gcvFALSE);
                            if (gcmIS_ERROR(status)) return status;

                            status = clDefineSelectionFalseOperandBegin(Compiler,
                                                            CodeGenerator,
                                                            &selectionContextNeg);
                            if (gcmIS_ERROR(status)) return status;

                            status = clGenGenericCode1(
                                            Compiler,
                                            PolynaryExpr->exprBase.base.lineNo,
                                            PolynaryExpr->exprBase.base.stringNo,
                                            (CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST) ?
                                                clvOPCODE_INT_TO_FLOAT_RTZ : clvOPCODE_INT_TO_FLOAT,
                                            IOperand,
                                            &intermROperands[0]);
                            if (gcmIS_ERROR(status)) return status;

                            status = clDefineSelectionFalseOperandEnd(Compiler,
                                                            CodeGenerator,
                                                            &selectionContextNeg);
                            if (gcmIS_ERROR(status)) return status;

                            status = clDefineSelectionEnd(Compiler,
                                                            CodeGenerator,
                                                            &selectionContextNeg);
                            if (gcmIS_ERROR(status)) return status;

                        }

                    } else {

                        status = clGenGenericCode1(
                                        Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        (CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST) ?
                                            clvOPCODE_INT_TO_FLOAT_RTZ : clvOPCODE_INT_TO_FLOAT,
                                        IOperand,
                                        &intermROperands[0]);
                        if (gcmIS_ERROR(status)) return status;
                    }
                }
                break;

           case clvBUILTIN_ROUND_TO_ZERO :

                    status = clGenGenericCode1(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    (CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST) ?
                                        clvOPCODE_INT_TO_FLOAT_RTZ : clvOPCODE_INT_TO_FLOAT,
                                    IOperand,
                                    &intermROperands[0]);
                    if (gcmIS_ERROR(status)) return status;

                break;

            default:
                break;
        }

    } else {
        /* Integer to integer */

        if (!isOutputINT && !isOutputUINT)
        {
            status = clGenArithmeticExprCode(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_AND_BITWISE,
                            IOperand,
                            &outputTypeMaskROperand,
                            &intermROperands[0]);
            if (gcmIS_ERROR(status)) return status;
        }
        else
        {
            status = clGenGenericCode1(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_ASSIGN,
                            IOperand,
                            &intermROperands[0]);
            if (gcmIS_ERROR(status)) return status;
        }
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenOldVectorConvert_Code(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand,
    IN gctBOOL Saturation,
    IN cleBUILTIN_ROUNDING_MODE RoundingMode,
    IN cltBUILT_IN_GEN_CODE_FUNC_PTR GenCode
    )
{
    if(!CodeGenerator->supportRTNE) {
        clsBUILTIN_FUNCTION_INFO *functionInfo = clGetBuiltinFunctionInfo(PolynaryExpr->funcSymbol);

        if(!functionInfo->handleVector &&
           OperandCount &&
           OperandsParameters->needROperand &&
           clmGEN_CODE_IsVectorDataType(OperandsParameters[0].dataTypes[0].def)) {
/* special handling for vector case */
             return clGenBuiltinVectorCode(Compiler,
                                          CodeGenerator,
                                          PolynaryExpr,
                                          OperandCount,
                                          OperandsParameters,
                                          IOperand,
                                          GenCode);
        }
    }

    if(!CodeGenerator->supportRTNE ||
       (cloCOMPILER_IsLongUlongPatch(Compiler) &&
       !(clmIsElementTypeHighPrecision(OperandsParameters->rOperands[0].dataType.elementType) ||
         clmIsElementTypeHighPrecision(IOperand->dataType.elementType)))) {
        return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, Saturation, RoundingMode);
    }
    return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, Saturation, RoundingMode);
}
#endif

static gceSTATUS
_GenConvertFloat4_rtzCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    clsIOPERAND intermIOperands[1];
    clsROPERAND intermROperands[1];

    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);
    clsIOPERAND_New(Compiler, &intermIOperands[0], OperandsParameters[0].rOperands[0].dataType);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);

    /* No conversion needed for same types */
    if (OperandsParameters[0].rOperands[0].dataType.elementType == IOperand->dataType.elementType) {
        status = clGenGenericCode1(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_ASSIGN,
                    IOperand,
                    &OperandsParameters[0].rOperands[0]);
        if (gcmIS_ERROR(status)) return status;
        return gcvSTATUS_OK;
    }

    status = clGenGenericCode1(Compiler,
            PolynaryExpr->exprBase.base.lineNo,
            PolynaryExpr->exprBase.base.stringNo,
            clvOPCODE_ASSIGN,
            &intermIOperands[0],
            &OperandsParameters[0].rOperands[0]);
    if (gcmIS_ERROR(status)) return status;

    status = clGenGenericCode1(
                    Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    (CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST) ?
                        clvOPCODE_INT_TO_FLOAT_RTZ : clvOPCODE_INT_TO_FLOAT,
                    IOperand,
                    &intermROperands[0]);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

/*****************************************************************************\
|*                         Supporting vector functions                       *|
\*****************************************************************************/

static gceSTATUS
_GenConvert2_Code(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand,
    IN gctBOOL Saturation,
    IN cleBUILTIN_ROUNDING_MODE RoundingMode
    )
{
    return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, Saturation, RoundingMode);
}

static gceSTATUS
_GenConvert3_Code(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand,
    IN gctBOOL Saturation,
    IN cleBUILTIN_ROUNDING_MODE RoundingMode
    )
{
    return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, Saturation, RoundingMode);
}

static gceSTATUS
_GenConvert4_Code(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand,
    IN gctBOOL Saturation,
    IN cleBUILTIN_ROUNDING_MODE RoundingMode
    )
{
    return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, Saturation, RoundingMode);
}

static gceSTATUS
_GenConvert8_Code(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand,
    IN gctBOOL Saturation,
    IN cleBUILTIN_ROUNDING_MODE RoundingMode
    )
{
    return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, Saturation, RoundingMode);
}

static gceSTATUS
_GenConvert16_Code(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand,
    IN gctBOOL Saturation,
    IN cleBUILTIN_ROUNDING_MODE RoundingMode
    )
{
    return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, Saturation, RoundingMode);
}

/*****************************************************************************\
|*                         ConvBuiltinFunctions                              *|
\*****************************************************************************/

static gceSTATUS
_GenConvert_rteCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
#if _DO_NOT_USE_CONV_FOR_RTNE_EXPLICIT_CONVERT_FUNCTION
    return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEAREST_EVEN);
#else
    if(!CodeGenerator->supportRTNE ||
       (cloCOMPILER_IsLongUlongPatch(Compiler) &&
       !(clmIsElementTypeHighPrecision(OperandsParameters->rOperands[0].dataType.elementType) ||
         clmIsElementTypeHighPrecision(IOperand->dataType.elementType)))) {
        return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEAREST_EVEN);
    }
    return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEAREST_EVEN);
#endif
}

static gceSTATUS
_GenConvert_rtzCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_ZERO);
}

static gceSTATUS
_GenConvert_rtpCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
#if _DO_NOT_USE_CONV_FOR_FLOATN_RTP_RTN_EXPLICIT_CONVERT_FUNCTION
    if(clmIsElementTypeFloating(IOperand->dataType.elementType)) {
        return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_POS_INF);
    }
    else {
        return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_POS_INF);
    }
#else
    return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_POS_INF);
#endif
}

static gceSTATUS
_GenConvert_rtnCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
#if _DO_NOT_USE_CONV_FOR_FLOATN_RTP_RTN_EXPLICIT_CONVERT_FUNCTION
    if(clmIsElementTypeFloating(IOperand->dataType.elementType)) {
        return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEG_INF);
    }
    else {
        return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEG_INF);
    }
#else
    return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEG_INF);
#endif
}

static gceSTATUS
_GenConvert_sat_rteCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
#if _DO_NOT_USE_CONV_FOR_RTNE_EXPLICIT_CONVERT_FUNCTION
    return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvTRUE, clvBUILTIN_ROUND_TO_NEAREST_EVEN);
#else
    if(!CodeGenerator->supportRTNE ||
       (cloCOMPILER_IsLongUlongPatch(Compiler) &&
       !(clmIsElementTypeHighPrecision(OperandsParameters->rOperands[0].dataType.elementType) ||
         clmIsElementTypeHighPrecision(IOperand->dataType.elementType)))) {
         return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvTRUE, clvBUILTIN_ROUND_TO_NEAREST_EVEN);
    }
    return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvTRUE, clvBUILTIN_ROUND_TO_NEAREST_EVEN);
#endif
}

static gceSTATUS
_GenConvert_sat_rtzCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvTRUE, clvBUILTIN_ROUND_TO_ZERO);
}

static gceSTATUS
_GenConvert_sat_rtpCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvTRUE, clvBUILTIN_ROUND_TO_POS_INF);
}

static gceSTATUS
_GenConvert_sat_rtnCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvTRUE, clvBUILTIN_ROUND_TO_NEG_INF);
}

static gceSTATUS
_GenConvert2_rteCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
#if _DO_NOT_USE_CONV_FOR_RTNE_EXPLICIT_CONVERT_FUNCTION
    return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEAREST_EVEN);
#else
    return _GenOldVectorConvert_Code(Compiler,
                                     CodeGenerator,
                                     PolynaryExpr,
                                     OperandCount,
                                     OperandsParameters,
                                     IOperand,
                                     gcvFALSE,
                                     clvBUILTIN_ROUND_TO_NEAREST_EVEN,
                                     _GenConvert2_rteCode);
#endif
}

static gceSTATUS
_GenConvert2_rtzCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    return _GenConvert2_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_ZERO);
}

static gceSTATUS
_GenConvert2_rtpCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
#if _DO_NOT_USE_CONV_FOR_FLOATN_RTP_RTN_EXPLICIT_CONVERT_FUNCTION
    if(clmIsElementTypeFloating(IOperand->dataType.elementType)) {
        return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_POS_INF);
    }
    else {
        return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_POS_INF);
    }
#else
    return _GenConvert2_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_POS_INF);
#endif
}

static gceSTATUS
_GenConvert2_rtnCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
#if _DO_NOT_USE_CONV_FOR_FLOATN_RTP_RTN_EXPLICIT_CONVERT_FUNCTION
    if(clmIsElementTypeFloating(IOperand->dataType.elementType)) {
        return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEG_INF);
    }
    else {
        return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEG_INF);
    }
#else
    return _GenConvert2_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEG_INF);
#endif
}

static gceSTATUS
_GenConvert2_sat_rteCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
#if _DO_NOT_USE_CONV_FOR_RTNE_EXPLICIT_CONVERT_FUNCTION
    return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvTRUE, clvBUILTIN_ROUND_TO_NEAREST_EVEN);
#else
    return _GenOldVectorConvert_Code(Compiler,
                                     CodeGenerator,
                                     PolynaryExpr,
                                     OperandCount,
                                     OperandsParameters,
                                     IOperand,
                                     gcvTRUE,
                                     clvBUILTIN_ROUND_TO_NEAREST_EVEN,
                                     _GenConvert2_sat_rteCode);
#endif
}

static gceSTATUS
_GenConvert2_sat_rtzCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    return _GenConvert2_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvTRUE, clvBUILTIN_ROUND_TO_ZERO);
}

static gceSTATUS
_GenConvert2_sat_rtpCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    return _GenConvert2_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvTRUE, clvBUILTIN_ROUND_TO_POS_INF);
}

static gceSTATUS
_GenConvert2_sat_rtnCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    return _GenConvert2_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvTRUE, clvBUILTIN_ROUND_TO_NEG_INF);
}

static gceSTATUS
_GenConvert3_rteCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
#if _DO_NOT_USE_CONV_FOR_RTNE_EXPLICIT_CONVERT_FUNCTION
    return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEAREST_EVEN);
#else
    return _GenOldVectorConvert_Code(Compiler,
                                     CodeGenerator,
                                     PolynaryExpr,
                                     OperandCount,
                                     OperandsParameters,
                                     IOperand,
                                     gcvFALSE,
                                     clvBUILTIN_ROUND_TO_NEAREST_EVEN,
                                     _GenConvert3_rteCode);
#endif
}

static gceSTATUS
_GenConvert3_rtzCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    return _GenConvert3_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_ZERO);
}

static gceSTATUS
_GenConvert3_rtpCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
#if _DO_NOT_USE_CONV_FOR_FLOATN_RTP_RTN_EXPLICIT_CONVERT_FUNCTION
    if(clmIsElementTypeFloating(IOperand->dataType.elementType)) {
        return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_POS_INF);
    }
    else {
        return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_POS_INF);
    }
#else
    return _GenConvert3_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_POS_INF);
#endif
}

static gceSTATUS
_GenConvert3_rtnCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
#if _DO_NOT_USE_CONV_FOR_FLOATN_RTP_RTN_EXPLICIT_CONVERT_FUNCTION
    if(clmIsElementTypeFloating(IOperand->dataType.elementType)) {
        return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEG_INF);
    }
    else {
        return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEG_INF);
    }
#else
    return _GenConvert3_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEG_INF);
#endif
}

static gceSTATUS
_GenConvert3_sat_rteCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
#if _DO_NOT_USE_CONV_FOR_RTNE_EXPLICIT_CONVERT_FUNCTION
    return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvTRUE, clvBUILTIN_ROUND_TO_NEAREST_EVEN);
#else
    return _GenOldVectorConvert_Code(Compiler,
                                     CodeGenerator,
                                     PolynaryExpr,
                                     OperandCount,
                                     OperandsParameters,
                                     IOperand,
                                     gcvTRUE,
                                     clvBUILTIN_ROUND_TO_NEAREST_EVEN,
                                     _GenConvert3_sat_rteCode);
#endif
}

static gceSTATUS
_GenConvert3_sat_rtzCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    return _GenConvert3_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvTRUE, clvBUILTIN_ROUND_TO_ZERO);
}

static gceSTATUS
_GenConvert3_sat_rtpCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    return _GenConvert3_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvTRUE, clvBUILTIN_ROUND_TO_POS_INF);
}

static gceSTATUS
_GenConvert3_sat_rtnCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    return _GenConvert3_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvTRUE, clvBUILTIN_ROUND_TO_NEG_INF);
}

static gceSTATUS
_GenConvert4_rteCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
#if _DO_NOT_USE_CONV_FOR_RTNE_EXPLICIT_CONVERT_FUNCTION
    return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEAREST_EVEN);
#else
    return _GenOldVectorConvert_Code(Compiler,
                                     CodeGenerator,
                                     PolynaryExpr,
                                     OperandCount,
                                     OperandsParameters,
                                     IOperand,
                                     gcvFALSE,
                                     clvBUILTIN_ROUND_TO_NEAREST_EVEN,
                                     _GenConvert4_rteCode);
#endif
}

static gceSTATUS
_GenConvert4_rtzCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    return _GenConvert4_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_ZERO);
}

static gceSTATUS
_GenConvert4_rtpCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
#if _DO_NOT_USE_CONV_FOR_FLOATN_RTP_RTN_EXPLICIT_CONVERT_FUNCTION
    if(clmIsElementTypeFloating(IOperand->dataType.elementType)) {
        return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_POS_INF);
    }
    else {
        return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_POS_INF);
    }
#else
    return _GenConvert4_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_POS_INF);
#endif
}

static gceSTATUS
_GenConvert4_rtnCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
#if _DO_NOT_USE_CONV_FOR_FLOATN_RTP_RTN_EXPLICIT_CONVERT_FUNCTION
    if(clmIsElementTypeFloating(IOperand->dataType.elementType)) {
        return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEG_INF);
    }
    else {
        return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEG_INF);
    }
#else
    return _GenConvert4_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEG_INF);
#endif
}

static gceSTATUS
_GenConvert4_sat_rteCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
#if _DO_NOT_USE_CONV_FOR_RTNE_EXPLICIT_CONVERT_FUNCTION
    return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvTRUE, clvBUILTIN_ROUND_TO_NEAREST_EVEN);
#else
    return _GenOldVectorConvert_Code(Compiler,
                                     CodeGenerator,
                                     PolynaryExpr,
                                     OperandCount,
                                     OperandsParameters,
                                     IOperand,
                                     gcvTRUE,
                                     clvBUILTIN_ROUND_TO_NEAREST_EVEN,
                                     _GenConvert4_sat_rteCode);
#endif
}

static gceSTATUS
_GenConvert4_sat_rtzCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    return _GenConvert4_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvTRUE, clvBUILTIN_ROUND_TO_ZERO);
}

static gceSTATUS
_GenConvert4_sat_rtpCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    return _GenConvert4_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvTRUE, clvBUILTIN_ROUND_TO_POS_INF);
}

static gceSTATUS
_GenConvert4_sat_rtnCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    return _GenConvert4_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvTRUE, clvBUILTIN_ROUND_TO_NEG_INF);
}

static gceSTATUS
_GenConvert8_rteCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
#if _DO_NOT_USE_CONV_FOR_RTNE_EXPLICIT_CONVERT_FUNCTION
    return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEAREST_EVEN);
#else
    return _GenOldVectorConvert_Code(Compiler,
                                     CodeGenerator,
                                     PolynaryExpr,
                                     OperandCount,
                                     OperandsParameters,
                                     IOperand,
                                     gcvFALSE,
                                     clvBUILTIN_ROUND_TO_NEAREST_EVEN,
                                     _GenConvert8_rteCode);
#endif
}

static gceSTATUS
_GenConvert8_rtzCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    return _GenConvert8_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_ZERO);
}

static gceSTATUS
_GenConvert8_rtpCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
#if _DO_NOT_USE_CONV_FOR_FLOATN_RTP_RTN_EXPLICIT_CONVERT_FUNCTION
    if(clmIsElementTypeFloating(IOperand->dataType.elementType)) {
        return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_POS_INF);
    }
    else {
        return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_POS_INF);
    }
#else
    return _GenConvert8_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_POS_INF);
#endif
}

static gceSTATUS
_GenConvert8_rtnCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
#if _DO_NOT_USE_CONV_FOR_FLOATN_RTP_RTN_EXPLICIT_CONVERT_FUNCTION
    if(clmIsElementTypeFloating(IOperand->dataType.elementType)) {
        return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEG_INF);
    }
    else {
        return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEG_INF);
    }
#else
    return _GenConvert8_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEG_INF);
#endif
}

static gceSTATUS
_GenConvert8_sat_rteCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
#if _DO_NOT_USE_CONV_FOR_RTNE_EXPLICIT_CONVERT_FUNCTION
    return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvTRUE, clvBUILTIN_ROUND_TO_NEAREST_EVEN);
#else
    return _GenOldVectorConvert_Code(Compiler,
                                     CodeGenerator,
                                     PolynaryExpr,
                                     OperandCount,
                                     OperandsParameters,
                                     IOperand,
                                     gcvTRUE,
                                     clvBUILTIN_ROUND_TO_NEAREST_EVEN,
                                     _GenConvert8_sat_rteCode);
#endif
}

static gceSTATUS
_GenConvert8_sat_rtzCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    return _GenConvert8_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvTRUE, clvBUILTIN_ROUND_TO_ZERO);
}

static gceSTATUS
_GenConvert8_sat_rtpCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    return _GenConvert8_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvTRUE, clvBUILTIN_ROUND_TO_POS_INF);
}

static gceSTATUS
_GenConvert8_sat_rtnCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    return _GenConvert8_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvTRUE, clvBUILTIN_ROUND_TO_NEG_INF);
}

static gceSTATUS
_GenConvert16_rteCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
#if _DO_NOT_USE_CONV_FOR_RTNE_EXPLICIT_CONVERT_FUNCTION
    return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEAREST_EVEN);
#else
    return _GenOldVectorConvert_Code(Compiler,
                                     CodeGenerator,
                                     PolynaryExpr,
                                     OperandCount,
                                     OperandsParameters,
                                     IOperand,
                                     gcvFALSE,
                                     clvBUILTIN_ROUND_TO_NEAREST_EVEN,
                                     _GenConvert16_rteCode);
#endif
}

static gceSTATUS
_GenConvert16_rtzCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    return _GenConvert16_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_ZERO);
}

static gceSTATUS
_GenConvert16_rtpCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
#if _DO_NOT_USE_CONV_FOR_FLOATN_RTP_RTN_EXPLICIT_CONVERT_FUNCTION
    if(clmIsElementTypeFloating(IOperand->dataType.elementType)) {
        return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_POS_INF);
    }
    else {
        return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_POS_INF);
    }
#else
    return _GenConvert16_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_POS_INF);
#endif
}

static gceSTATUS
_GenConvert16_rtnCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
#if _DO_NOT_USE_CONV_FOR_FLOATN_RTP_RTN_EXPLICIT_CONVERT_FUNCTION
    if(clmIsElementTypeFloating(IOperand->dataType.elementType)) {
        return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEG_INF);
    }
    else {
        return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEG_INF);
    }
#else
    return _GenConvert16_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEG_INF);
#endif
}

static gceSTATUS
_GenConvert16_sat_rteCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
#if _DO_NOT_USE_CONV_FOR_RTNE_EXPLICIT_CONVERT_FUNCTION
    return _GenOldConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvTRUE, clvBUILTIN_ROUND_TO_NEAREST_EVEN);
#else
    return _GenOldVectorConvert_Code(Compiler,
                                     CodeGenerator,
                                     PolynaryExpr,
                                     OperandCount,
                                     OperandsParameters,
                                     IOperand,
                                     gcvTRUE,
                                     clvBUILTIN_ROUND_TO_NEAREST_EVEN,
                                     _GenConvert16_sat_rteCode);
#endif
}

static gceSTATUS
_GenConvert16_sat_rtzCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    return _GenConvert16_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvTRUE, clvBUILTIN_ROUND_TO_ZERO);
}

static gceSTATUS
_GenConvert16_sat_rtpCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    return _GenConvert16_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvTRUE, clvBUILTIN_ROUND_TO_POS_INF);
}

static gceSTATUS
_GenConvert16_sat_rtnCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    return _GenConvert16_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvTRUE, clvBUILTIN_ROUND_TO_NEG_INF);
}

static gceSTATUS
_GenConvert_FloatDefaultCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    if (CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST)
    {
        return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEAREST_EVEN);
    }
    else
    {
        return _GenConvert_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_ZERO);
    }
}

static gceSTATUS
_GenConvert2_FloatDefaultCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    if (CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST)
    {
        return _GenConvert2_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEAREST_EVEN);
    }
    else
    {
        return _GenConvert2_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_ZERO);
    }
}

static gceSTATUS
_GenConvert3_FloatDefaultCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    if (CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST)
    {
        return _GenConvert3_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEAREST_EVEN);
    }
    else
    {
        return _GenConvert3_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_ZERO);
    }
}

static gceSTATUS
_GenConvert4_FloatDefaultCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    if (CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST)
    {
        return _GenConvert4_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEAREST_EVEN);
    }
    else
    {
        return _GenConvert4_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_ZERO);
    }
}

static gceSTATUS
_GenConvert8_FloatDefaultCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    if (CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST)
    {
        return _GenConvert8_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEAREST_EVEN);
    }
    else
    {
        return _GenConvert8_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_ZERO);
    }
}

static gceSTATUS
_GenConvert16_FloatDefaultCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    if (CodeGenerator->fpConfig & cldFpROUND_TO_NEAREST)
    {
        return _GenConvert16_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_NEAREST_EVEN);
    }
    else
    {
        return _GenConvert16_Code(Compiler, CodeGenerator, PolynaryExpr, OperandCount, OperandsParameters, IOperand, gcvFALSE, clvBUILTIN_ROUND_TO_ZERO);
    }
}

static gceSTATUS
_GenAs_TypeCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    clsROPERAND rOperand;
    clsIOPERAND iOperandBuf[1];
    clsIOPERAND *iOperand;

    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* return input parameter as output */
    rOperand = OperandsParameters[0].rOperands[0];
    iOperand = IOperand;

    if(clmIsElementTypeHighPrecision(IOperand->dataType.elementType))
    {
        if(clmGEN_CODE_IsScalarDataType(IOperand->dataType))
        {
            if(clmGEN_CODE_IsVectorDataType(rOperand.dataType)) {
                rOperand.vectorIndex.mode = clvINDEX_CONSTANT;
                rOperand.vectorIndex.u.constant = 0;
                rOperand.dataType = gcGetVectorSliceDataType(rOperand.dataType, 1);
            }
        }
        else
        {
            clmGEN_CODE_DATA_TYPE_Initialize(rOperand.dataType,
                                             clmGEN_CODE_vectorSize_GET(IOperand->dataType),
                                             0,
                                             rOperand.dataType.elementType);
        }
    }
    else {
        if(clmGEN_CODE_IsScalarDataType(IOperand->dataType))
        {
            if(clmGEN_CODE_IsVectorDataType(rOperand.dataType)) {
                rOperand.vectorIndex.mode = clvINDEX_CONSTANT;
                rOperand.vectorIndex.u.constant = 0;
                rOperand.dataType = gcGetVectorSliceDataType(rOperand.dataType, 1);
            }
        }
        else if(clmGEN_CODE_IsScalarDataType(rOperand.dataType))
        {
            clsIOPERAND intermIOperand[1];
            clsLOPERAND intermLOperand[1];
            clsGEN_CODE_DATA_TYPE dataType;

            /* just to match the expected vector size */
            dataType = gcConvScalarToVectorDataType(rOperand.dataType,
                                                    clmGEN_CODE_vectorSize_GET(IOperand->dataType));
            clsIOPERAND_New(Compiler, intermIOperand, dataType);
            clsLOPERAND_InitializeUsingIOperand(intermLOperand, intermIOperand);
            status = clGenAssignCode(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     intermLOperand,
                                     &rOperand);
            if (gcmIS_ERROR(status)) return status;
            clsROPERAND_InitializeUsingIOperand(&rOperand, intermIOperand);
        }
        else {
            gctSIZE_T lSize, rSize;

            lSize = clGEN_CODE_DataTypeByteSize(Compiler, IOperand->dataType);
            rSize = clGEN_CODE_DataTypeByteSize(Compiler, rOperand.dataType);
            if(lSize != rSize) {
                gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                PolynaryExpr->exprBase.base.lineNo,
                                                PolynaryExpr->exprBase.base.stringNo,
                                                clvREPORT_ERROR,
                                                "As_type reinterpretation of data to a type of different byte size"));
                return gcvSTATUS_INVALID_DATA;
            }

            if(!(clmIsElementTypePacked(IOperand->dataType.elementType) ||
                 clmIsElementTypePacked(rOperand.dataType.elementType))) {
                if(clmGEN_CODE_vectorSize_GET(rOperand.dataType) >
                   clmGEN_CODE_vectorSize_GET(IOperand->dataType)) {
                    clGetVectorROperandSlice(&rOperand,
                                             0,
                                             clmGEN_CODE_vectorSize_GET(IOperand->dataType),
                                             &rOperand);
                }
                else if(clmGEN_CODE_vectorSize_GET(IOperand->dataType) >
                        clmGEN_CODE_vectorSize_GET(rOperand.dataType)) {
                    clGetVectorIOperandSlice(IOperand,
                                             0,
                                             clmGEN_CODE_vectorSize_GET(rOperand.dataType),
                                             iOperandBuf);
                    iOperand = iOperandBuf;
                }
            }
        }
    }

    status = clGenGenericCode1(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               clvOPCODE_ASTYPE,
                               iOperand,
                               &rOperand);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

/*
 * Special case for output as vector3
 * Copied over from _GenBuiltinVectorCode with vectorSize fixed to 3
 */
static gceSTATUS
_GenAs_Type3Code(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;

    gctUINT8 i;
    clsROPERAND    tempROperand[3*4], copyROperand[3], cntROperands[2], inputROperands[3];
    clsIOPERAND tempIOperand, cntIOperands[2], inputIOperands[3];
    clsLOPERAND tempLOperand, destLOperands[3];
    clsROPERAND    vSizeROperand, zero123ROperands[3], zero123X4ROperands[3], negOneROperand;
    clsROPERAND rOperand;
    clsSELECTION_CONTEXT   selectionContextLoopBack;
    gctUINT8 copy_matrixSize_rowCount[3];
    gctUINT8 copy_matrixSize_columnCount[3];

    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* Force to scalar */
    OperandsParameters[0].dataTypes[0].def.matrixSize.rowCount = 0;
    OperandsParameters[0].dataTypes[0].def.matrixSize.columnCount = 0;

    copyROperand[0] = OperandsParameters[0].rOperands[0];
    copy_matrixSize_rowCount[0] = OperandsParameters[0].dataTypes[0].def.matrixSize.rowCount;
    copy_matrixSize_columnCount[0] = OperandsParameters[0].dataTypes[0].def.matrixSize.columnCount;

    clsIOPERAND_New(Compiler, &inputIOperands[0], OperandsParameters[0].dataTypes[0].def);
    clsROPERAND_InitializeUsingIOperand(&inputROperands[0], &inputIOperands[0]);

    clsLOPERAND_InitializeUsingIOperand(&tempLOperand, IOperand);
    clsIOPERAND_New(Compiler, &tempIOperand, IOperand->dataType);

    /* Force to scalar */
    tempIOperand.dataType.matrixSize.columnCount = 0;
    tempIOperand.dataType.matrixSize.rowCount = 0;

    clsIOPERAND_New(Compiler, &cntIOperands[0], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&cntROperands[0], &cntIOperands[0]);

    clsIOPERAND_New(Compiler, &cntIOperands[1], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&cntROperands[1], &cntIOperands[1]);

    for(i = 0; i<3; i++){
        clmROPERAND_vectorComponent_GET(&tempROperand[4*i], &copyROperand[0], i);
        clmLOPERAND_vectorComponent_GET(&destLOperands[i], &tempLOperand, i);
    }

    clsROPERAND_InitializeIntOrIVecConstant(&vSizeROperand,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) 3);
    for(i = 0; i<3; i++){
        clsROPERAND_InitializeIntOrIVecConstant(&zero123ROperands[i],
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) i);
        clsROPERAND_InitializeIntOrIVecConstant(&zero123X4ROperands[i],
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) i*4);
    }

    clsROPERAND_InitializeIntOrIVecConstant(&negOneROperand,
                                            clmGenCodeDataType(T_INT),
                                            (gctINT) -1);
    status = clGenGenericCode1(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_ASSIGN,
                            &cntIOperands[0],
                            &zero123ROperands[0]);

    OperandsParameters[0].rOperands[0] = inputROperands[0];

    status = clDefineSelectionBegin(Compiler,
                                    CodeGenerator,
                                    gcvTRUE,
                                    &selectionContextLoopBack);
    if (gcmIS_ERROR(status)) return status;

    /* Loop End, jump back here*/
    status = clDefineSelectionFalseOperandBegin(Compiler,
                                            CodeGenerator,
                                            &selectionContextLoopBack);

    status = clDefineSelectionFalseOperandEnd(Compiler,
                                    CodeGenerator,
                                    &selectionContextLoopBack);

    if (gcmIS_ERROR(status)) return status;

    for(i = 0; i<3; i++){

        clmGEN_CODE_IF(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo,
                     &cntROperands[0],
                     clvCONDITION_EQUAL,
                     &zero123ROperands[i]);

        status = clGenGenericCode1(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_ASSIGN,
                        &inputIOperands[0],
                        &tempROperand[4*i + 0]);

        /* The false part, "!==i" */
        clmGEN_CODE_ELSE(Compiler,
                           CodeGenerator,
                           PolynaryExpr->exprBase.base.lineNo,
                           PolynaryExpr->exprBase.base.stringNo);

       clmGEN_CODE_ENDIF(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);
       if (gcmIS_ERROR(status)) return status;
    }

    rOperand = OperandsParameters[0].rOperands[0];
    rOperand.dataType.elementType = tempIOperand.dataType.elementType;

    status = clGenGenericCode1(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               clvOPCODE_ASTYPE,
                               &tempIOperand,
                               &rOperand);

    clsROPERAND_InitializeUsingIOperand(&tempROperand[0], &tempIOperand);

    for(i = 0; i<3; i++){

        clmGEN_CODE_IF(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo,
                         &cntROperands[0],
                         clvCONDITION_EQUAL,
                         &zero123ROperands[i]);

        status = clGenAssignCode(
                    Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    &destLOperands[i],
                    &tempROperand[0]);
        if (gcmIS_ERROR(status)) return status;

        /* The false part, "!==i" */
        clmGEN_CODE_ELSE(Compiler,
                               CodeGenerator,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo);

        clmGEN_CODE_ENDIF(Compiler,
                             CodeGenerator,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo);
        if (gcmIS_ERROR(status)) return status;
    }

    /* Cnt++ */
    status = clGenArithmeticExprCode(Compiler,
            PolynaryExpr->exprBase.base.lineNo,
            PolynaryExpr->exprBase.base.stringNo,
            clvOPCODE_ADD,
            &cntIOperands[0],
            &cntROperands[0],
            &zero123ROperands[1]);

    status = clGenSelectionCompareConditionCode(Compiler,
                                                CodeGenerator,
                                                &selectionContextLoopBack,
                                                PolynaryExpr->exprBase.base.lineNo,
                                                PolynaryExpr->exprBase.base.stringNo,
                                                clvCONDITION_GREATER_THAN_EQUAL,
                                                &cntROperands[0],
                                                &vSizeROperand);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionTrueOperandBegin(Compiler,
                                            CodeGenerator,
                                            &selectionContextLoopBack);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionTrueOperandEnd(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    CodeGenerator,
                                    &selectionContextLoopBack,
                                    gcvFALSE);

    status = clDefineSelectionEnd(Compiler,
                                CodeGenerator,
                                &selectionContextLoopBack);
    if (gcmIS_ERROR(status)) return status;

    OperandsParameters[0].rOperands[0] = copyROperand[0];
    OperandsParameters[0].dataTypes[0].def.matrixSize.rowCount = copy_matrixSize_rowCount[0];
    OperandsParameters[0].dataTypes[0].def.matrixSize.columnCount = copy_matrixSize_columnCount[0];

    return gcvSTATUS_OK;

OnError:
   return status;
}

#endif /* __gc_cl_built_ins_conv_h_ */
