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


#include "gc_cl_parser.h"
#include "gc_hal_user_math.h"
#include "gc_cl_built_ins.h"
#include <string.h>

#define FULL_PROFILE_TEST   1
#define HAS_DOUBLE_SUPPORT  0

#define _LOGE_2                ((float)0.693147180559945309417232)
#define _LOG2_E_high        ((float)1.4426950f)
#define _LOG2_E_low            ((float)1.9259630e-008)

#define _LOG2_E                ((float)1.44269504088896340736)
#define _RCP_OF_LOG2_E        ((float)0.69314718055994530942)
#define _LOG10_2            ((float)0.30102999566398119521373889472449)

#define _LOG2_10_high        ((float)3.3219280f)
#define _LOG2_10_low        ((float)7.0595370e-008)
/*One LSB increase for 1/Pi */
#define pi_rcpPlus                (0.3183099031f)

#define pi_threeHalved            ((float)4.712388980384689857693965074919)
#define pi_halved                ((float)1.570796326794896619231321691639)
#define pi_constant                ((float)3.141592653589793238462643383279)
#define two_pi                    ((float)6.283185307179586476925286766559)
#define two_pi_lo                ((float)8.7422780126189537e-008)
#define pi_rcp                    ((float)0.318309886183790671537767526745)

#define LOAD_BUILT_IN_FUNCS_ONLY_ONCE 1

static clsHASH_TABLE  _BuiltinFunctionInfoHash;
static clsHASH_TABLE  _FastRelaxedMathMappingHash;


typedef struct _clsFAST_RELAXED_MATH_MAPPING
{
    gctCONST_STRING regFunc;
    gctCONST_STRING fastFunc;
}
clsFAST_RELAXED_MATH_MAPPING;

typedef struct _clsFAST_RELAXED_MATH_MAPPING_NODE
{
    slsDLINK_NODE                node;
    clsFAST_RELAXED_MATH_MAPPING relaxedMathMapping;
}
clsFAST_RELAXED_MATH_MAPPING_NODE;


static clsFAST_RELAXED_MATH_MAPPING _FastRelaxedMathMapping[] =
{
    {"sin",         "native_sin"},
    {"cos",         "native_cos"},
    {"tan",         "native_tan"},
    {"asin",        "native#asin"},
    {"acos",        "native#acos"},
    {"atan",        "native#atan"},

    /* Exponential Functions */
    {"powr",        "native_powr"},
    {"exp",         "native_exp"},
    {"exp10",       "native_exp10"},
    {"log",         "native_log"},
    {"exp2",        "native_exp2"},
    {"log2",        "native_log2"},
    {"log10",       "native_log10"},
    {"sqrt",        "native_sqrt"},
    {"rsqrt",       "native_rsqrt"},
    {"reciprocal",  "native_recip"},
    {"viv_reciprocal",  "viv_native_recip"},
    {"divide#",     "viv_native_divide"},


    /* Common Functions */


    /* Geometric Functions */
    {"length",      "fast_length"},
    {"distance",    "fast_distance"},
    {"normalize",   "fast_normalize"},
    {"fma",         "fast_fma"},
};

#define _cldFastRelaxedMathMappingCount \
    (sizeof(_FastRelaxedMathMapping) / sizeof(clsFAST_RELAXED_MATH_MAPPING))

/* Basic Built-In Types */
clsBUILTIN_DATATYPE_INFO clBuiltinDataTypes[] =
{
    {T_VOID, T_VOID, 0, {clvTYPE_VOID, {0,0}}, clvPOLYNARY_CONSTRUCT_NONE, gcvFALSE, {{gcvNULL, }, }, "v", VIR_TYPE_VOID},
    {T_MAT2,  T_MAT2, T_FLOAT2, {clvTYPE_FLOAT, {2,2}}, clvPOLYNARY_CONSTRUCT_MATRIX, gcvFALSE, {{gcvNULL, }, }, "", VIR_TYPE_FLOAT_2X2},
    {T_MAT3,  T_MAT3, T_FLOAT3, {clvTYPE_FLOAT, {3,3}}, clvPOLYNARY_CONSTRUCT_MATRIX, gcvFALSE, {{gcvNULL, }, }, "", VIR_TYPE_FLOAT_3X3},
    {T_MAT4,  T_MAT4, T_FLOAT4, {clvTYPE_FLOAT, {4,4}}, clvPOLYNARY_CONSTRUCT_MATRIX, gcvFALSE, {{gcvNULL, }, }, "", VIR_TYPE_FLOAT_4X4},
    {T_MAT8,  T_MAT8, T_FLOAT8, {clvTYPE_FLOAT, {8,8}}, clvPOLYNARY_CONSTRUCT_MATRIX, gcvFALSE, {{gcvNULL, }, }, "", VIR_TYPE_UNKNOWN},
    {T_MAT16, T_MAT16, T_FLOAT16, {clvTYPE_FLOAT, {16,16}}, clvPOLYNARY_CONSTRUCT_MATRIX, gcvFALSE, {{gcvNULL, }, }, "", VIR_TYPE_UNKNOWN},

    {T_BOOL,   T_BOOL, T_BOOL, {clvTYPE_BOOL, {0, 0}}, clvPOLYNARY_CONSTRUCT_SCALAR, gcvFALSE, {{gcvNULL, }, }, "b", VIR_TYPE_BOOLEAN},
    {T_BOOL2,  T_BOOL2_PACKED, T_BOOL, {clvTYPE_BOOL, {2, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv2_b", VIR_TYPE_BOOLEAN_X2},
    {T_BOOL3,  T_BOOL3_PACKED, T_BOOL, {clvTYPE_BOOL, {3, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv3_b", VIR_TYPE_BOOLEAN_X3},
    {T_BOOL4,  T_BOOL4_PACKED, T_BOOL, {clvTYPE_BOOL, {4, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv4_b", VIR_TYPE_BOOLEAN_X4},
    {T_BOOL8,  T_BOOL8_PACKED, T_BOOL, {clvTYPE_BOOL, {8, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv8_b", VIR_TYPE_BOOLEAN_X8},
    {T_BOOL16, T_BOOL16_PACKED, T_BOOL, {clvTYPE_BOOL, {16, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv16_b", VIR_TYPE_BOOLEAN_X16},
    {T_BOOL32, T_BOOL32_PACKED, T_BOOL, {clvTYPE_BOOL, {32, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv32_b", VIR_TYPE_BOOLEAN_X32},

    {T_HALF,   T_HALF, T_HALF, {clvTYPE_HALF, {0,0}}, clvPOLYNARY_CONSTRUCT_SCALAR, gcvFALSE, {{gcvNULL, }, }, "Dh", VIR_TYPE_FLOAT16},
    {T_HALF2,  T_HALF2_PACKED, T_HALF, {clvTYPE_HALF, {2,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv2_Dh", VIR_TYPE_FLOAT16_X2},
    {T_HALF3,  T_HALF3_PACKED, T_HALF, {clvTYPE_HALF, {3,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv3_Dh", VIR_TYPE_FLOAT16_X3},
    {T_HALF4,  T_HALF4_PACKED, T_HALF, {clvTYPE_HALF, {4,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv4_Dh", VIR_TYPE_FLOAT16_X4},
    {T_HALF8,  T_HALF8_PACKED, T_HALF, {clvTYPE_HALF, {8,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv8_Dh", VIR_TYPE_FLOAT16_X8},
    {T_HALF16, T_HALF16_PACKED, T_HALF, {clvTYPE_HALF, {16,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv16_Dh", VIR_TYPE_FLOAT16_X16},
    {T_HALF32, T_HALF32_PACKED, T_HALF, {clvTYPE_HALF, {32,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv32_Dh", VIR_TYPE_FLOAT16_X32},

    {T_FLOAT,   T_FLOAT, T_FLOAT, {clvTYPE_FLOAT, {0,0}}, clvPOLYNARY_CONSTRUCT_SCALAR, gcvFALSE, {{gcvNULL, }, }, "f", VIR_TYPE_FLOAT32},
    {T_FLOAT2,  T_FLOAT2, T_FLOAT, {clvTYPE_FLOAT, {2,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv2_f", VIR_TYPE_FLOAT_X2},
    {T_FLOAT3,  T_FLOAT3, T_FLOAT, {clvTYPE_FLOAT, {3,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv3_f", VIR_TYPE_FLOAT_X3},
    {T_FLOAT4,  T_FLOAT4, T_FLOAT, {clvTYPE_FLOAT, {4,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv4_f", VIR_TYPE_FLOAT_X4},
    {T_FLOAT8,  T_FLOAT8, T_FLOAT, {clvTYPE_FLOAT, {8,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv8_f", VIR_TYPE_FLOAT_X8},
    {T_FLOAT16, T_FLOAT16, T_FLOAT, {clvTYPE_FLOAT, {16,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv16_f", VIR_TYPE_FLOAT_X16},

    {T_DOUBLE,   T_DOUBLE, T_DOUBLE, {clvTYPE_DOUBLE, {0,0}}, clvPOLYNARY_CONSTRUCT_SCALAR, gcvFALSE, {{gcvNULL, }, }, "d", VIR_TYPE_FLOAT64},
    {T_DOUBLE2,  T_DOUBLE2, T_DOUBLE, {clvTYPE_DOUBLE, {2,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv2_d", VIR_TYPE_UNKNOWN},
    {T_DOUBLE3,  T_DOUBLE3, T_DOUBLE, {clvTYPE_DOUBLE, {3,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv3_d", VIR_TYPE_UNKNOWN},
    {T_DOUBLE4,  T_DOUBLE4, T_DOUBLE, {clvTYPE_DOUBLE, {4,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv4_d", VIR_TYPE_UNKNOWN},
    {T_DOUBLE8,  T_DOUBLE8, T_DOUBLE, {clvTYPE_DOUBLE, {8,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv8_d", VIR_TYPE_UNKNOWN},
    {T_DOUBLE16, T_DOUBLE16, T_DOUBLE, {clvTYPE_DOUBLE, {16,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv16_d", VIR_TYPE_FLOAT64},

    {T_QUAD,    T_QUAD, T_QUAD, {clvTYPE_QUAD, {0,0}}, clvPOLYNARY_CONSTRUCT_SCALAR, gcvFALSE, {{gcvNULL, }, }, "g", VIR_TYPE_UNKNOWN},
    {T_QUAD2,   T_QUAD2, T_QUAD, {clvTYPE_QUAD, {2,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv2_g", VIR_TYPE_UNKNOWN},
    {T_QUAD3,   T_QUAD3, T_QUAD, {clvTYPE_QUAD, {3,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv3_g", VIR_TYPE_UNKNOWN},
    {T_QUAD4,   T_QUAD4, T_QUAD, {clvTYPE_QUAD, {4,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv4_g", VIR_TYPE_UNKNOWN},
    {T_QUAD8,   T_QUAD8, T_QUAD, {clvTYPE_QUAD, {8,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv8_g", VIR_TYPE_UNKNOWN},
    {T_QUAD16,  T_QUAD16, T_QUAD, {clvTYPE_QUAD, {16,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv16_g", VIR_TYPE_UNKNOWN},

    {T_CHAR,   T_CHAR, T_CHAR, {clvTYPE_CHAR, {0,0}}, clvPOLYNARY_CONSTRUCT_SCALAR, gcvFALSE, {{gcvNULL, }, }, "c", VIR_TYPE_INT8},
    {T_CHAR2,  T_CHAR2_PACKED, T_CHAR, {clvTYPE_CHAR, {2,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv2_c", VIR_TYPE_INT8_X2},
    {T_CHAR3,  T_CHAR3_PACKED, T_CHAR, {clvTYPE_CHAR, {3,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv3_c", VIR_TYPE_INT8_X3},
    {T_CHAR4,  T_CHAR4_PACKED, T_CHAR, {clvTYPE_CHAR, {4,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv4_c", VIR_TYPE_INT8_X4},
    {T_CHAR8,  T_CHAR8_PACKED, T_CHAR, {clvTYPE_CHAR, {8,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv8_c", VIR_TYPE_INT8_X8},
    {T_CHAR16, T_CHAR16_PACKED, T_CHAR, {clvTYPE_CHAR, {16,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv16_c", VIR_TYPE_INT8_X16},
    {T_CHAR32, T_CHAR32_PACKED, T_CHAR, {clvTYPE_CHAR, {32,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv32_c", VIR_TYPE_INT8_X32},

    {T_UCHAR,   T_UCHAR, T_UCHAR,  {clvTYPE_UCHAR, {0,0}}, clvPOLYNARY_CONSTRUCT_SCALAR, gcvTRUE, {{gcvNULL, }, }, "h", VIR_TYPE_UINT8},
    {T_UCHAR2,  T_UCHAR2_PACKED, T_UCHAR, {clvTYPE_UCHAR, {2,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvTRUE, {{gcvNULL, }, }, "Dv2_h", VIR_TYPE_UINT8_X2},
    {T_UCHAR3,  T_UCHAR3_PACKED, T_UCHAR, {clvTYPE_UCHAR, {3,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvTRUE, {{gcvNULL, }, }, "Dv3_h", VIR_TYPE_UINT8_X3},
    {T_UCHAR4,  T_UCHAR4_PACKED, T_UCHAR, {clvTYPE_UCHAR, {4,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvTRUE, {{gcvNULL, }, }, "Dv4_h", VIR_TYPE_UINT8_X4},
    {T_UCHAR8,  T_UCHAR8_PACKED, T_UCHAR, {clvTYPE_UCHAR, {8,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvTRUE, {{gcvNULL, }, }, "Dv8_h", VIR_TYPE_UINT8_X8},
    {T_UCHAR16, T_UCHAR16_PACKED, T_UCHAR, {clvTYPE_UCHAR, {16,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvTRUE, {{gcvNULL, }, }, "Dv16_h", VIR_TYPE_UINT8_X16},
    {T_UCHAR32, T_UCHAR32_PACKED, T_UCHAR, {clvTYPE_UCHAR, {32,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvTRUE, {{gcvNULL, }, }, "Dv32_h", VIR_TYPE_UINT8_X32},

    {T_SHORT,   T_SHORT, T_SHORT, {clvTYPE_SHORT, {0,0}}, clvPOLYNARY_CONSTRUCT_SCALAR, gcvFALSE, {{gcvNULL, }, }, "s", VIR_TYPE_INT16},
    {T_SHORT2,  T_SHORT2_PACKED, T_SHORT, {clvTYPE_SHORT, {2,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv2_s", VIR_TYPE_INT16_X2},
    {T_SHORT3,  T_SHORT3_PACKED, T_SHORT, {clvTYPE_SHORT, {3,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv3_s", VIR_TYPE_INT16_X3},
    {T_SHORT4,  T_SHORT4_PACKED, T_SHORT, {clvTYPE_SHORT, {4,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv4_s", VIR_TYPE_INT16_X4},
    {T_SHORT8,  T_SHORT8_PACKED, T_SHORT, {clvTYPE_SHORT, {8,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv8_s", VIR_TYPE_INT16_X8},
    {T_SHORT16, T_SHORT16_PACKED, T_SHORT, {clvTYPE_SHORT, {16,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv16_s", VIR_TYPE_INT16_X16},
    {T_SHORT32, T_SHORT32_PACKED, T_SHORT, {clvTYPE_SHORT, {32,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv32_s", VIR_TYPE_INT16_X32},

    {T_USHORT,   T_USHORT, T_USHORT, {clvTYPE_USHORT, {0,0}}, clvPOLYNARY_CONSTRUCT_SCALAR, gcvTRUE, {{gcvNULL, }, }, "t", VIR_TYPE_UINT16},
    {T_USHORT2,  T_USHORT2_PACKED, T_USHORT, {clvTYPE_USHORT, {2,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvTRUE, {{gcvNULL, }, }, "Dv2_t", VIR_TYPE_UINT16_X2},
    {T_USHORT3,  T_USHORT3_PACKED, T_USHORT, {clvTYPE_USHORT, {3,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvTRUE, {{gcvNULL, }, }, "Dv3_t", VIR_TYPE_UINT16_X3},
    {T_USHORT4,  T_USHORT4_PACKED, T_USHORT, {clvTYPE_USHORT, {4,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvTRUE, {{gcvNULL, }, }, "Dv4_t", VIR_TYPE_UINT16_X4},
    {T_USHORT8,  T_USHORT8_PACKED, T_USHORT, {clvTYPE_USHORT, {8,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvTRUE, {{gcvNULL, }, }, "Dv8_t", VIR_TYPE_UINT16_X8},
    {T_USHORT16, T_USHORT16_PACKED, T_USHORT, {clvTYPE_USHORT, {16,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvTRUE, {{gcvNULL, }, }, "Dv16_t", VIR_TYPE_UINT16_X16},
    {T_USHORT32, T_USHORT32_PACKED, T_USHORT, {clvTYPE_USHORT, {32,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvTRUE, {{gcvNULL, }, }, "Dv32_t", VIR_TYPE_UINT16_X32},

    {T_INT,   T_INT, T_INT, {clvTYPE_INT, {0,0}}, clvPOLYNARY_CONSTRUCT_SCALAR, gcvFALSE, {{gcvNULL, }, }, "i", VIR_TYPE_INT32},
    {T_INT2,  T_INT2, T_INT, {clvTYPE_INT, {2,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv2_i", VIR_TYPE_INTEGER_X2},
    {T_INT3,  T_INT3, T_INT, {clvTYPE_INT, {3,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv3_i", VIR_TYPE_INTEGER_X3},
    {T_INT4,  T_INT4, T_INT, {clvTYPE_INT, {4,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv4_i", VIR_TYPE_INTEGER_X4},
    {T_INT8,  T_INT8, T_INT, {clvTYPE_INT, {8,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv8_i", VIR_TYPE_INTEGER_X8},
    {T_INT16, T_INT16, T_INT, {clvTYPE_INT, {16,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv16_i", VIR_TYPE_INTEGER_X16},

    {T_UINT,   T_UINT, T_UINT,  {clvTYPE_UINT, {0,0}}, clvPOLYNARY_CONSTRUCT_SCALAR, gcvTRUE, {{gcvNULL, }, }, "j", VIR_TYPE_UINT32},
    {T_UINT2,  T_UINT2, T_UINT, {clvTYPE_UINT, {2,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvTRUE, {{gcvNULL, }, }, "Dv2_j", VIR_TYPE_UINT_X2},
    {T_UINT3,  T_UINT3, T_UINT, {clvTYPE_UINT, {3,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvTRUE, {{gcvNULL, }, }, "Dv3_j", VIR_TYPE_UINT_X3},
    {T_UINT4,  T_UINT4, T_UINT, {clvTYPE_UINT, {4,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvTRUE, {{gcvNULL, }, }, "Dv4_j", VIR_TYPE_UINT_X4},
    {T_UINT8,  T_UINT8, T_UINT, {clvTYPE_UINT, {8,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvTRUE, {{gcvNULL, }, }, "Dv8_j", VIR_TYPE_UINT_X8},
    {T_UINT16, T_UINT16, T_UINT, {clvTYPE_UINT, {16,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvTRUE, {{gcvNULL, }, }, "Dv16_j", VIR_TYPE_UINT_X16},

    {T_LONG,   T_LONG, T_LONG, {clvTYPE_LONG, {0,0}}, clvPOLYNARY_CONSTRUCT_SCALAR, gcvFALSE, {{gcvNULL, }, }, "l", VIR_TYPE_INT64},
    {T_LONG2,  T_LONG2, T_LONG, {clvTYPE_LONG, {2,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv2_l", VIR_TYPE_INT64_X2},
    {T_LONG3,  T_LONG3, T_LONG, {clvTYPE_LONG, {3,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv3_l", VIR_TYPE_INT64_X3},
    {T_LONG4,  T_LONG4, T_LONG, {clvTYPE_LONG, {4,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv4_l", VIR_TYPE_INT64_X4},
    {T_LONG8,  T_LONG8, T_LONG, {clvTYPE_LONG, {8,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv8_l", VIR_TYPE_INT64_X8},
    {T_LONG16, T_LONG16, T_LONG, {clvTYPE_LONG, {16,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "Dv16_l", VIR_TYPE_INT64_X16},

    {T_ULONG,   T_ULONG, T_ULONG, {clvTYPE_ULONG, {0,0}}, clvPOLYNARY_CONSTRUCT_SCALAR, gcvTRUE, {{gcvNULL, }, }, "m", VIR_TYPE_UINT64},
    {T_ULONG2,  T_ULONG2, T_ULONG, {clvTYPE_ULONG, {2,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvTRUE, {{gcvNULL, }, }, "Dv2_m", VIR_TYPE_UINT64_X2},
    {T_ULONG3,  T_ULONG3, T_ULONG, {clvTYPE_ULONG, {3,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvTRUE, {{gcvNULL, }, }, "Dv3_m", VIR_TYPE_UINT64_X3},
    {T_ULONG4,  T_ULONG4, T_ULONG, {clvTYPE_ULONG, {4,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvTRUE, {{gcvNULL, }, }, "Dv4_m", VIR_TYPE_UINT64_X4},
    {T_ULONG8,  T_ULONG8, T_ULONG, {clvTYPE_ULONG, {8,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvTRUE, {{gcvNULL, }, }, "Dv8_m", VIR_TYPE_UINT64_X8},
    {T_ULONG16, T_ULONG16, T_ULONG, {clvTYPE_ULONG, {16,0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvTRUE, {{gcvNULL, }, }, "Dv16_m", VIR_TYPE_UINT64_X16},

    {T_SAMPLER_T, T_SAMPLER_T, T_SAMPLER_T, {clvTYPE_SAMPLER_T, {0,0}}, clvPOLYNARY_CONSTRUCT_NONE, gcvFALSE, {{gcvNULL, }, }, "13ocl_sampler_t", VIR_TYPE_SAMPLER},
    {T_VIV_GENERIC_GL_SAMPLER, T_VIV_GENERIC_GL_SAMPLER, T_VIV_GENERIC_GL_SAMPLER, {clvTYPE_VIV_GENERIC_GL_SAMPLER, {0,0}}, clvPOLYNARY_CONSTRUCT_NONE, gcvFALSE, {{gcvNULL, }, }, "13viv_generic_gl_sampler", VIR_TYPE_VIV_GENERIC_GL_SAMPLER},
    {T_IMAGE1D_T, T_IMAGE1D_T, T_IMAGE1D_T, {clvTYPE_IMAGE1D_T, {0,0}}, clvPOLYNARY_CONSTRUCT_NONE, gcvFALSE, {{gcvNULL, }, }, "13ocl_image1d_t", VIR_TYPE_IMAGE_1D},
    {T_IMAGE1D_ARRAY_T, T_IMAGE1D_ARRAY_T, T_IMAGE1D_ARRAY_T, {clvTYPE_IMAGE1D_ARRAY_T, {0,0}}, clvPOLYNARY_CONSTRUCT_NONE, gcvFALSE, {{gcvNULL, }, }, "19ocl_image1d_array_t", VIR_TYPE_IMAGE_1D_ARRAY},
    {T_IMAGE1D_BUFFER_T, T_IMAGE1D_BUFFER_T, T_IMAGE1D_BUFFER_T, {clvTYPE_IMAGE1D_BUFFER_T, {0,0}}, clvPOLYNARY_CONSTRUCT_NONE, gcvFALSE, {{gcvNULL, }, }, "20ocl_image1d_buffer_t", VIR_TYPE_IMAGE_1D_BUFFER},
    {T_IMAGE2D_ARRAY_T, T_IMAGE2D_ARRAY_T, T_IMAGE2D_ARRAY_T, {clvTYPE_IMAGE2D_ARRAY_T, {0,0}}, clvPOLYNARY_CONSTRUCT_NONE, gcvFALSE, {{gcvNULL, }, }, "19ocl_image2d_array_t", VIR_TYPE_IMAGE_2D_ARRAY},
    {T_IMAGE2D_T, T_IMAGE2D_T, T_IMAGE2D_T, {clvTYPE_IMAGE2D_T, {0,0}}, clvPOLYNARY_CONSTRUCT_NONE, gcvFALSE, {{gcvNULL, }, }, "13ocl_image2d_t", VIR_TYPE_IMAGE_2D},
    {T_IMAGE3D_T, T_IMAGE2D_T, T_IMAGE3D_T, {clvTYPE_IMAGE3D_T, {0,0}}, clvPOLYNARY_CONSTRUCT_NONE, gcvFALSE, {{gcvNULL, }, }, "13ocl_image3d_t", VIR_TYPE_IMAGE_3D},
    {T_IMAGE2D_PTR_T, T_IMAGE2D_PTR_T, T_IMAGE2D_PTR_T, {clvTYPE_IMAGE2D_T, {0,0}}, clvPOLYNARY_CONSTRUCT_NONE, gcvTRUE, {{gcvNULL, }, }, "18_viv_image2d_ptr_t", VIR_TYPE_UINT32},
    {T_IMAGE2D_DYNAMIC_ARRAY_T, T_IMAGE2D_DYNAMIC_ARRAY_T, T_IMAGE2D_DYNAMIC_ARRAY_T, {clvTYPE_IMAGE2D_T, {0,0}}, clvPOLYNARY_CONSTRUCT_NONE, gcvFALSE, {{gcvNULL, }, }, "20_viv_image2d_array_t", VIR_TYPE_UNKNOWN},
    {T_VIV_GENERIC_IMAGE_T, T_VIV_GENERIC_IMAGE_T, T_VIV_GENERIC_IMAGE_T, {clvTYPE_VIV_GENERIC_IMAGE_T, {0,0}}, clvPOLYNARY_CONSTRUCT_NONE, gcvFALSE, {{gcvNULL, }, }, "20_viv_generic_image_t", VIR_TYPE_VIV_GENERIC_IMAGE_T},
    {T_VIV_GENERIC_GL_IMAGE, T_VIV_GENERIC_GL_IMAGE, T_VIV_GENERIC_GL_IMAGE, {clvTYPE_VIV_GENERIC_GL_IMAGE, {0,0}}, clvPOLYNARY_CONSTRUCT_NONE, gcvFALSE, {{gcvNULL, }, }, "20_viv_generic_gl_image", VIR_TYPE_VIV_GENERIC_GL_IMAGE},

    {T_SIZE_T,  T_SIZE_T, T_SIZE_T, {clvTYPE_UINT, {0,0}}, clvPOLYNARY_CONSTRUCT_NONE, gcvTRUE, {{gcvNULL, }, }, "j", VIR_TYPE_INT32},
    {T_EVENT_T, T_EVENT_T, T_EVENT_T, {clvTYPE_EVENT_T, {0,0}}, clvPOLYNARY_CONSTRUCT_NONE, gcvTRUE, {{gcvNULL, }, }, "9ocl_event", VIR_TYPE_EVENT_T},
    {T_PTRDIFF_T, T_PTRDIFF_T, T_PTRDIFF_T, {clvTYPE_INT, {0,0}}, clvPOLYNARY_CONSTRUCT_NONE, gcvFALSE, {{gcvNULL, }, }, "i", VIR_TYPE_UINT32},
    {T_INTPTR_T, T_INTPTR_T, T_INTPTR_T, {clvTYPE_INT, {0,0}}, clvPOLYNARY_CONSTRUCT_NONE, gcvFALSE, {{gcvNULL, }, }, "i", VIR_TYPE_UINT32},
    {T_UINTPTR_T, T_UINTPTR_T, T_UINTPTR_T, {clvTYPE_UINT, {0,0}}, clvPOLYNARY_CONSTRUCT_NONE, gcvTRUE, {{gcvNULL, }, }, "j", VIR_TYPE_UINT32},
    {T_GENTYPE,   T_GENTYPE_PACKED, T_FLOAT, {clvTYPE_GEN, {0,0}}, clvPOLYNARY_CONSTRUCT_NONE, gcvFALSE, {{gcvNULL, }, }, "", VIR_TYPE_UNKNOWN},
    {T_F_GENTYPE, T_F_GENTYPE, T_FLOAT, {clvTYPE_F_GEN, {0,0}}, clvPOLYNARY_CONSTRUCT_NONE, gcvFALSE, {{gcvNULL, }, }, "", VIR_TYPE_UNKNOWN},
    {T_IU_GENTYPE, T_IU_GENTYPE, T_INT, {clvTYPE_IU_GEN, {0,0}}, clvPOLYNARY_CONSTRUCT_NONE, gcvFALSE, {{gcvNULL, }, }, "", VIR_TYPE_UNKNOWN},
    {T_I_GENTYPE, T_I_GENTYPE, T_INT, {clvTYPE_I_GEN, {0,0}}, clvPOLYNARY_CONSTRUCT_NONE, gcvFALSE, {{gcvNULL, }, }, "", VIR_TYPE_UNKNOWN},
    {T_U_GENTYPE, T_U_GENTYPE, T_UINT, {clvTYPE_U_GEN, {0,0}}, clvPOLYNARY_CONSTRUCT_NONE, gcvTRUE, {{gcvNULL, }, }, "", VIR_TYPE_UNKNOWN},
    {T_SIU_GENTYPE, T_SIU_GENTYPE, T_INT, {clvTYPE_SIU_GEN, {0,0}}, clvPOLYNARY_CONSTRUCT_NONE, gcvFALSE, {{gcvNULL, }, }, "", VIR_TYPE_UNKNOWN},

    {T_BOOL_PACKED,   T_BOOL, T_BOOL_PACKED, {clvTYPE_BOOL_PACKED, {0, 0}}, clvPOLYNARY_CONSTRUCT_SCALAR, gcvFALSE, {{gcvNULL, }, }, "16_viv_bool_packed", VIR_TYPE_BOOLEAN},
    {T_BOOL2_PACKED,  T_BOOL2, T_BOOL_PACKED, {clvTYPE_BOOL_PACKED, {2, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "17_viv_bool2_packed", VIR_TYPE_BOOLEAN_P2},
    {T_BOOL3_PACKED,  T_BOOL3, T_BOOL_PACKED, {clvTYPE_BOOL_PACKED, {3, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "17_viv_bool3_packed", VIR_TYPE_BOOLEAN_P3},
    {T_BOOL4_PACKED,  T_BOOL4, T_BOOL_PACKED, {clvTYPE_BOOL_PACKED, {4, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "17_viv_bool4_packed", VIR_TYPE_BOOLEAN_P4},
    {T_BOOL8_PACKED,  T_BOOL8, T_BOOL_PACKED, {clvTYPE_BOOL_PACKED, {8, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "17_viv_bool8_packed", VIR_TYPE_BOOLEAN_P8},
    {T_BOOL16_PACKED, T_BOOL16, T_BOOL_PACKED, {clvTYPE_BOOL_PACKED, {16, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "18_viv_bool16_packed", VIR_TYPE_BOOLEAN_P16},
    {T_BOOL32_PACKED, T_BOOL32, T_BOOL_PACKED, {clvTYPE_BOOL_PACKED, {32, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "18_viv_bool32_packed", VIR_TYPE_BOOLEAN_P32},

    {T_CHAR_PACKED,   T_CHAR, T_CHAR_PACKED, {clvTYPE_CHAR_PACKED, {0, 0}}, clvPOLYNARY_CONSTRUCT_SCALAR, gcvFALSE, {{gcvNULL, }, }, "16_viv_char_packed", VIR_TYPE_INT8},
    {T_CHAR2_PACKED,  T_CHAR2, T_CHAR_PACKED, {clvTYPE_CHAR_PACKED, {2, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "17_viv_char2_packed", VIR_TYPE_INT8_P2},
    {T_CHAR3_PACKED,  T_CHAR3, T_CHAR_PACKED, {clvTYPE_CHAR_PACKED, {3, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "17_viv_char3_packed", VIR_TYPE_INT8_P3},
    {T_CHAR4_PACKED,  T_CHAR4, T_CHAR_PACKED, {clvTYPE_CHAR_PACKED, {4, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "17_viv_char4_packed", VIR_TYPE_INT8_P4},
    {T_CHAR8_PACKED,  T_CHAR8, T_CHAR_PACKED, {clvTYPE_CHAR_PACKED, {8, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "17_viv_char8_packed", VIR_TYPE_INT8_P8},
    {T_CHAR16_PACKED, T_CHAR16, T_CHAR_PACKED, {clvTYPE_CHAR_PACKED, {16, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "18_viv_char16_packed", VIR_TYPE_INT8_P16},
    {T_CHAR32_PACKED, T_CHAR32, T_CHAR_PACKED, {clvTYPE_CHAR_PACKED, {32, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "18_viv_char32_packed", VIR_TYPE_INT8_P32},

    {T_UCHAR_PACKED,   T_UCHAR, T_UCHAR_PACKED, {clvTYPE_UCHAR_PACKED, {0, 0}}, clvPOLYNARY_CONSTRUCT_SCALAR, gcvFALSE, {{gcvNULL, }, }, "17_viv_uchar_packed", VIR_TYPE_UINT8},
    {T_UCHAR2_PACKED,  T_UCHAR2, T_UCHAR_PACKED, {clvTYPE_UCHAR_PACKED, {2, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "18_viv_uchar2_packed", VIR_TYPE_UINT8_P2},
    {T_UCHAR3_PACKED,  T_UCHAR3, T_UCHAR_PACKED, {clvTYPE_UCHAR_PACKED, {3, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "18_viv_uchar3_packed", VIR_TYPE_UINT8_P3},
    {T_UCHAR4_PACKED,  T_UCHAR4, T_UCHAR_PACKED, {clvTYPE_UCHAR_PACKED, {4, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "18_viv_uchar4_packed", VIR_TYPE_UINT8_P4},
    {T_UCHAR8_PACKED,  T_UCHAR8, T_UCHAR_PACKED, {clvTYPE_UCHAR_PACKED, {8, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "18_viv_uchar8_packed", VIR_TYPE_UINT8_P8},
    {T_UCHAR16_PACKED, T_UCHAR16, T_UCHAR_PACKED, {clvTYPE_UCHAR_PACKED, {16, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "19_viv_uchar16_packed", VIR_TYPE_UINT8_P16},
    {T_UCHAR32_PACKED, T_UCHAR32, T_UCHAR_PACKED, {clvTYPE_UCHAR_PACKED, {32, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "19_viv_uchar32_packed", VIR_TYPE_UINT8_P32},

    {T_SHORT_PACKED,   T_SHORT, T_SHORT_PACKED, {clvTYPE_SHORT_PACKED, {0, 0}}, clvPOLYNARY_CONSTRUCT_SCALAR, gcvFALSE, {{gcvNULL, }, }, "17_viv_short_packed", VIR_TYPE_INT16},
    {T_SHORT2_PACKED,  T_SHORT2, T_SHORT_PACKED, {clvTYPE_SHORT_PACKED, {2, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "18_viv_short2_packed", VIR_TYPE_INT16_P2},
    {T_SHORT3_PACKED,  T_SHORT3, T_SHORT_PACKED, {clvTYPE_SHORT_PACKED, {3, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "18_viv_short3_packed", VIR_TYPE_INT16_P3},
    {T_SHORT4_PACKED,  T_SHORT4, T_SHORT_PACKED, {clvTYPE_SHORT_PACKED, {4, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "18_viv_short4_packed", VIR_TYPE_INT16_P4},
    {T_SHORT8_PACKED,  T_SHORT8, T_SHORT_PACKED, {clvTYPE_SHORT_PACKED, {8, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "18_viv_short8_packed", VIR_TYPE_INT16_P8},
    {T_SHORT16_PACKED, T_SHORT16, T_SHORT_PACKED, {clvTYPE_SHORT_PACKED, {16, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "19_viv_short16_packed", VIR_TYPE_INT16_P16},
    {T_SHORT32_PACKED, T_SHORT32, T_SHORT_PACKED, {clvTYPE_SHORT_PACKED, {32, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "19_viv_short32_packed", VIR_TYPE_INT16_P32},

    {T_USHORT_PACKED,   T_USHORT, T_USHORT_PACKED, {clvTYPE_USHORT_PACKED, {0, 0}}, clvPOLYNARY_CONSTRUCT_SCALAR, gcvFALSE, {{gcvNULL, }, }, "18_viv_ushort_packed", VIR_TYPE_UINT16},
    {T_USHORT2_PACKED,  T_USHORT2, T_USHORT_PACKED, {clvTYPE_USHORT_PACKED, {2, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "19_viv_ushort2_packed", VIR_TYPE_UINT16_P2},
    {T_USHORT3_PACKED,  T_USHORT3, T_USHORT_PACKED, {clvTYPE_USHORT_PACKED, {3, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "19_viv_ushort3_packed", VIR_TYPE_UINT16_P3},
    {T_USHORT4_PACKED,  T_USHORT4, T_USHORT_PACKED, {clvTYPE_USHORT_PACKED, {4, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "19_viv_ushort4_packed", VIR_TYPE_UINT16_P4},
    {T_USHORT8_PACKED,  T_USHORT8, T_USHORT_PACKED, {clvTYPE_USHORT_PACKED, {8, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "19_viv_ushort8_packed", VIR_TYPE_UINT16_P8},
    {T_USHORT16_PACKED, T_USHORT16, T_USHORT_PACKED, {clvTYPE_USHORT_PACKED, {16, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "20_viv_ushort16_packed", VIR_TYPE_UINT16_P16},
    {T_USHORT32_PACKED, T_USHORT32, T_USHORT_PACKED, {clvTYPE_USHORT_PACKED, {32, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "20_viv_ushort32_packed", VIR_TYPE_UINT16_P32},

    {T_HALF_PACKED,   T_HALF, T_HALF_PACKED, {clvTYPE_HALF_PACKED, {0, 0}}, clvPOLYNARY_CONSTRUCT_SCALAR, gcvFALSE, {{gcvNULL, }, }, "16_viv_half_packed", VIR_TYPE_FLOAT16},
    {T_HALF2_PACKED,  T_HALF2, T_HALF_PACKED, {clvTYPE_HALF_PACKED, {2, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "17_viv_half2_packed", VIR_TYPE_FLOAT16_P2},
    {T_HALF3_PACKED,  T_HALF3, T_HALF_PACKED, {clvTYPE_HALF_PACKED, {3, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "17_viv_half3_packed", VIR_TYPE_FLOAT16_P3},
    {T_HALF4_PACKED,  T_HALF4, T_HALF_PACKED, {clvTYPE_HALF_PACKED, {4, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "17_viv_half4_packed", VIR_TYPE_FLOAT16_P4},
    {T_HALF8_PACKED,  T_HALF8, T_HALF_PACKED, {clvTYPE_HALF_PACKED, {8, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "17_viv_half8_packed", VIR_TYPE_FLOAT16_P8},
    {T_HALF16_PACKED, T_HALF16, T_HALF_PACKED, {clvTYPE_HALF_PACKED, {16, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "18_viv_half16_packed", VIR_TYPE_FLOAT16_P16},
    {T_HALF32_PACKED, T_HALF32, T_HALF_PACKED, {clvTYPE_HALF_PACKED, {32, 0}}, clvPOLYNARY_CONSTRUCT_VECTOR, gcvFALSE, {{gcvNULL, }, }, "18_viv_half32_packed", VIR_TYPE_FLOAT16_P32},

    {T_GENTYPE_PACKED, T_GENTYPE, T_GENTYPE_PACKED, {clvTYPE_GEN_PACKED, {0,0}}, clvPOLYNARY_CONSTRUCT_NONE, gcvFALSE, {{gcvNULL, }, }, "19_viv_gentype_packed", VIR_TYPE_UNKNOWN}
};

#define _cldBuiltinDataTypeCount  (sizeof(clBuiltinDataTypes) / sizeof(clsBUILTIN_DATATYPE_INFO))

/* Last built-in data type: macro for first cldFirstBuiltinDataType defined in gc_cl_built_ins.h */
#define cldLastBuiltinDataType    (T_VERY_FIRST_TERMINAL + _cldBuiltinDataTypeCount)
static gctBOOL _IsBuiltinDataTypeInfoReady = gcvFALSE;

static gceSTATUS _GenLog2_E_10Code(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand,
    IN gctINT logBaseIndicate /*2: log2(x), 1: log(x), 10: log10(x) */
    );
static gceSTATUS
_GenExp_E_10Code(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand,
    IN gctINT    baseIndex /*1: base E, 10: base 10, -1: E^x-1 (expm1(x)) */
    );


#if gcdDEBUG
static gctBOOL
_IsBuiltinDataTypeInfoSorted(
IN cloCOMPILER Compiler
)
{
   gctUINT i;

   for(i = 0; i < _cldBuiltinDataTypeCount; i++) {
      if(clBuiltinDataTypes[i].type == (gctINT)(i + cldFirstBuiltinDataType)) continue;
      else {
         if(Compiler) {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        0,
                        0,
                        clvREPORT_FATAL_ERROR,
                        "builtin data type info not sorted at %d"
                                            " with type %d",
                                            i, clBuiltinDataTypes[i].type));
         }
         return gcvFALSE;
      }
   }
   return gcvTRUE;
}
#endif

gctBOOL
_CheckBuiltinDataType(IN gctINT Type)
{
   return (Type >= (gctINT)cldFirstBuiltinDataType && Type <= (gctINT)cldLastBuiltinDataType) ? gcvTRUE : gcvFALSE;
}


static gceSTATUS
_ConstructBuiltinDataTypeInfos(
    IN cloCOMPILER Compiler
    )
{
    clsDATA_TYPE **dataType;
    gctSIZE_T i, j;

    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    /* Verify the arguments. */

    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    if(!_IsBuiltinDataTypeInfoReady) {
      for(i = 0; i < _cldBuiltinDataTypeCount; i++) { /* initialize the data type pointer storage */
        dataType = &clBuiltinDataTypes[i].typePtr[0][0];
        for(j = 0; j < cldQUALIFIER_ACCESS_COUNT * cldQUALIFIER_ADDRESS_SPACE_COUNT; j++) {
           *dataType++ = gcvNULL;
        }
      }
      cloIR_InitializeVecCompSelTypes(Compiler);

      _IsBuiltinDataTypeInfoReady = gcvTRUE;
    }
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
clCleanupBuiltins(
void
)
{
   gctSIZE_T i, j;
   gceSTATUS status = gcvSTATUS_OK;
   clsDATA_TYPE **dataType;
   clsBUILTIN_FUNCTION_INFO_NODE *node;
   clsFAST_RELAXED_MATH_MAPPING_NODE *node1;
   slsDLINK_NODE *bucket;
   cloCOMPILER compiler = *gcGetKernelCompiler();

   if(!compiler)
       return status;

   for(i = 0; i < _cldBuiltinDataTypeCount; i++) { /* free all datatype memory */
     dataType = &clBuiltinDataTypes[i].typePtr[0][0];
     for(j = 0; j < cldQUALIFIER_ACCESS_COUNT * cldQUALIFIER_ADDRESS_SPACE_COUNT; j++) {
        if(*dataType) {
            gcmONERROR(gcoOS_Free(gcvNULL, (gctPOINTER)(*dataType)));
        }
        *dataType++ = gcvNULL;
     }
   }

   /* Destroy builtin function info hash table */
   FOR_EACH_HASH_BUCKET(&_BuiltinFunctionInfoHash, bucket) {
      while (!slsDLINK_LIST_IsEmpty(bucket)) {
         slsDLINK_LIST_DetachFirst(bucket, clsBUILTIN_FUNCTION_INFO_NODE, &node);
         gcmONERROR(cloCOMPILER_Free(compiler, node));
      }
   }

   /* Destroy builtin function info hash table */
   FOR_EACH_HASH_BUCKET(&_FastRelaxedMathMappingHash, bucket) {
      while (!slsDLINK_LIST_IsEmpty(bucket)) {
         slsDLINK_LIST_DetachFirst(bucket, clsFAST_RELAXED_MATH_MAPPING_NODE, &node1);
         gcmONERROR(cloCOMPILER_Free(compiler, node1));
      }
   }

   cloCOMPILER_Destroy_General(compiler);
OnError:
   return status;
}

clsBUILTIN_DATATYPE_INFO *
clGetBuiltinDataTypeInfo(
IN gctINT Type
)
{
    gcmHEADER_ARG("Type=%d", Type);
    if(_CheckBuiltinDataType(Type)) {
        clsBUILTIN_DATATYPE_INFO *typeInfo;

        typeInfo = &clmBuiltinDataTypeInfo(Type);
        gcmASSERT(typeInfo->type == Type);
        gcmFOOTER_ARG("*<return>=0x%x", typeInfo);
        return typeInfo;
    }

    gcmFOOTER_ARG("<return>=0x%x", gcvNULL);
    return gcvNULL;
}

gctBOOL
clIsBuiltinDataType(IN gctINT Type)
{
   if((Type == T_BUILTIN_DATA_TYPE) ||
      (Type == T_RESERVED_DATA_TYPE)) return gcvTRUE;

   return _CheckBuiltinDataType(Type);
}

/* Built-In Constant */
static gceSTATUS
_LoadBuiltinConstants(IN cloCOMPILER Compiler)
{
    gceSTATUS status = gcvSTATUS_OK;
    clsDECL decl;
    cloIR_CONSTANT    constant;
    cluCONSTANT_VALUE value;
    cltPOOL_STRING    variableSymbol;
    clsNAME *variableName;
    gctUINT    i;
    struct _clsBUILTIN_CONSTANT {
      gctCONST_STRING symbol;
      gctUINT64 value;
      gctINT type;
    } _BuiltinConstants[] = {
/*openGL stuff */
    {"MAXFLOAT",    0x7F7FFFFF, T_FLOAT},
    {"HUGE_VALF",   0x7f800000, T_FLOAT},
    {"INFINITY",    0x7f800000, T_FLOAT},
    {"NAN",         0x7fc00000, T_FLOAT},
/* Sampler_T First byte: addressing mode. */
    {"CLK_ADDRESS_NONE",            0x0, T_SAMPLER_T},
    {"CLK_ADDRESS_CLAMP_TO_EDGE",   0x1, T_SAMPLER_T},
    {"CLK_ADDRESS_CLAMP",           0x2, T_SAMPLER_T},
    {"CLK_ADDRESS_REPEAT",          0x3, T_SAMPLER_T},
    {"CLK_ADDRESS_MIRRORED_REPEAT", 0x4, T_SAMPLER_T},
/* Sampler_T Second byte: filter mode. */
    {"CLK_FILTER_NEAREST",          0x000, T_SAMPLER_T},
    {"CLK_FILTER_LINEAR",           0x100, T_SAMPLER_T},
/* Sampler_T Third byte: normalized coords. */
    {"CLK_NORMALIZED_COORDS_FALSE", 0x00000, T_SAMPLER_T},
    {"CLK_NORMALIZED_COORDS_TRUE",  0x10000, T_SAMPLER_T},
/* cl_channel_order */
    {"CLK_R",         0x10B0, T_UINT},
    {"CLK_A",         0x10B1, T_UINT},
    {"CLK_RG",        0x10B2, T_UINT},
    {"CLK_RA",        0x10B3, T_UINT},
    {"CLK_RGB",       0x10B4, T_UINT},
    {"CLK_RGBA",      0x10B5, T_UINT},
    {"CLK_BGRA",      0x10B6, T_UINT},
    {"CLK_ARGB",      0x10B7, T_UINT},
    {"CLK_INTENSITY", 0x10B8, T_UINT},
    {"CLK_LUMINANCE", 0x10B9, T_UINT},
    {"CLK_Rx",        0x10BA, T_UINT},
    {"CLK_RGx",       0x10BB, T_UINT},
    {"CLK_RGBx",      0x10BC, T_UINT},
/* CLK_channel_type */
    {"CLK_SNORM_INT8",      0x10D0, T_UINT},
    {"CLK_SNORM_INT16",     0x10D1, T_UINT},
    {"CLK_UNORM_INT8",      0x10D2, T_UINT},
    {"CLK_UNORM_INT16",     0x10D3, T_UINT},
    {"CLK_UNORM_SHORT_565", 0x10D4, T_UINT},
    {"CLK_UNORM_SHORT_555", 0x10D5, T_UINT},
    {"CLK_UNORM_INT_101010",0x10D6, T_UINT},
    {"CLK_SIGNED_INT8",     0x10D7, T_UINT},
    {"CLK_SIGNED_INT16",    0x10D8, T_UINT},
    {"CLK_SIGNED_INT32",    0x10D9, T_UINT},
    {"CLK_UNSIGNED_INT8",   0x10DA, T_UINT},
    {"CLK_UNSIGNED_INT16",  0x10DB, T_UINT},
    {"CLK_UNSIGNED_INT32",  0x10DC, T_UINT},
    {"CLK_HALF_FLOAT",      0x10DD, T_UINT},
    {"CLK_FLOAT",           0x10DE, T_UINT},

    {"CLK_LOCAL_MEM_FENCE", 0x1, T_UINT},
    {"CLK_GLOBAL_MEM_FENCE", 0x2, T_UINT},

    /* Numerical constants. */
    {"CHAR_BIT",    0x00000008  /* 8 */,                T_UINT},
    {"SCHAR_MAX",   0x0000007F  /* 127 */,              T_CHAR},
    {"SCHAR_MIN",   0xFFFFFF80  /* (-127-1) */,         T_CHAR},
    {"CHAR_MAX",    0x0000007F  /* 127 */,              T_CHAR},
    {"CHAR_MIN",    0xFFFFFF80  /* (-127-1) */,         T_CHAR},
    {"UCHAR_MAX",   0x000000FF  /* 255 */,              T_UCHAR},
    {"SHRT_MAX",    0x00007FFF  /* 32767 */,            T_SHORT},
    {"SHRT_MIN",    0xFFFF8000  /* (-32767-1) */,       T_SHORT},
    {"USHRT_MAX",   0x0000FFFF  /* 65535 */,            T_USHORT},
    {"INT_MAX",     0x7FFFFFFF  /* 2147483647 */,       T_INT},
    {"INT_MIN",     0x80000000  /* (-2147483647-1) */,  T_INT},
    {"UINT_MAX",    0xFFFFFFFF, /* 4294967295 */        T_UINT},
    /* long/ulong are not supported. */
    {"LONG_MAX",    0x7FFFFFFFFFFFFFFFLL,          T_LONG},
    {"LONG_MIN",    (gctUINT64)(-0x7FFFFFFFFFFFFFFFLL - 1LL), T_LONG},
    {"ULONG_MAX",   0xFFFFFFFFFFFFFFFFULL,         T_ULONG},

    {"FLT_DIG",         0x6         /* 6 */,      T_UINT},
    {"FLT_MANT_DIG",    0x18        /* 24 */,     T_UINT},
    {"FLT_MAX_10_EXP",  0x26        /* +38 */,    T_INT},
    {"FLT_MAX_EXP",     0x80        /* +128 */,   T_INT},
    {"FLT_MIN_10_EXP",  0xFFFFFFDB  /* -37 */,    T_INT},
    {"FLT_MIN_EXP",     0xFFFFFF83  /* -125 */,   T_INT},
    {"FLT_RADIX",       0x2         /* 2 */,      T_UINT},
    {"FLT_MAX",         0x7F7FFFFF  /* 340282346638528859811704183484516925440.0f */, T_FLOAT},
    {"FLT_MIN",         0x00800000  /* 1.175494350822287507969e-38f */,               T_FLOAT},
    {"FLT_EPSILON",     0x34000000  /* 1.1920928955078125e-7f */,                     T_FLOAT},

    {"FP_ILOGB0",       0x80000000  /* (-2147483647-1) */,  T_INT},
    {"FP_ILOGBNAN",     0x7FFFFFFF  /* 2147483647 */,       T_INT},

    {"M_E_F",           0x402df854  /* 2.71828174591064f */, T_FLOAT},
    {"M_LOG2E_F",       0x3FB8AA3B  /* 1.44269502162933f */, T_FLOAT},
    {"M_LOG10E_F",      0x3EDE5BD9  /* 0.43429449200630f */, T_FLOAT},
    {"M_LN2_F",         0x3F317218  /* 0.69314718246460f */, T_FLOAT},
    {"M_LN10_F",        0x40135D8E  /* 2.30258512496948f */, T_FLOAT},
    {"M_PI_F",          0x40490FDB  /* 3.14159274101257f */, T_FLOAT},
    {"M_PI_2_F",        0x3FC90FDB  /* 1.57079637050629f */, T_FLOAT},
    {"M_PI_4_F",        0x3F490FDB  /* 0.78539818525314f */, T_FLOAT},
    {"M_1_PI_F",        0x3EA2F983  /* 0.31830987334251f */, T_FLOAT},
    {"M_2_PI_F",        0x3F22F983  /* 0.63661974668503f */, T_FLOAT},
    {"M_2_SQRTPI_F",    0x3F906EBB  /* 1.12837922573090f */, T_FLOAT},
    {"M_SQRT2_F",       0x3FB504F3  /* 1.41421353816986f */, T_FLOAT},
    {"M_SQRT1_2_F",     0x3F3504F3  /* 0.70710676908493f */, T_FLOAT},

/* For DOUBLE */
    {"M_E",           0x402df854  /* 2.71828174591064f */, HAS_DOUBLE_SUPPORT ? T_DOUBLE : T_FLOAT},
    {"M_LOG2E",       0x3FB8AA3B  /* 1.44269502162933f */, HAS_DOUBLE_SUPPORT ? T_DOUBLE : T_FLOAT},
    {"M_LOG10E",      0x3EDE5BD9  /* 0.43429449200630f */, HAS_DOUBLE_SUPPORT ? T_DOUBLE : T_FLOAT},
    {"M_LN2",         0x3F317218  /* 0.69314718246460f */, HAS_DOUBLE_SUPPORT ? T_DOUBLE : T_FLOAT},
    {"M_LN10",        0x40135D8E  /* 2.30258512496948f */, HAS_DOUBLE_SUPPORT ? T_DOUBLE : T_FLOAT},
    {"M_PI",          0x40490FDB  /* 3.14159274101257f */, HAS_DOUBLE_SUPPORT ? T_DOUBLE : T_FLOAT},
    {"M_PI_2",        0x3FC90FDB  /* 1.57079637050629f */, HAS_DOUBLE_SUPPORT ? T_DOUBLE : T_FLOAT},
    {"M_PI_4",        0x3F490FDB  /* 0.78539818525314f */, HAS_DOUBLE_SUPPORT ? T_DOUBLE : T_FLOAT},
    {"M_1_PI",        0x3EA2F983  /* 0.31830987334251f */, HAS_DOUBLE_SUPPORT ? T_DOUBLE : T_FLOAT},
    {"M_2_PI",        0x3F22F983  /* 0.63661974668503f */, HAS_DOUBLE_SUPPORT ? T_DOUBLE : T_FLOAT},
    {"M_2_SQRTPI",    0x3F906EBB  /* 1.12837922573090f */, HAS_DOUBLE_SUPPORT ? T_DOUBLE : T_FLOAT},
    {"M_SQRT2",       0x3FB504F3  /* 1.41421353816986f */, HAS_DOUBLE_SUPPORT ? T_DOUBLE : T_FLOAT},
    {"M_SQRT1_2",     0x3F3504F3  /* 0.70710676908493f */, HAS_DOUBLE_SUPPORT ? T_DOUBLE : T_FLOAT},
    };

#define _cldBuiltinConstantCount (sizeof(_BuiltinConstants) / sizeof(struct _clsBUILTIN_CONSTANT))


#define __BUILD_BUILT_IN(NAME) { #NAME, clvOPCODE_##NAME },

    struct _clsBUILTIN_ASM_CONSTANT {
      gctCONST_STRING symbol;
      gctUINT value;
    } _BuiltinAsmConstants[] = {
        __BUILD_BUILT_IN(NOP)

        __BUILD_BUILT_IN(ASSIGN)
        {"MOV", clvOPCODE_ASSIGN },  /* allow MOV be used in _viv_asm */

        __BUILD_BUILT_IN(COPY)
        __BUILD_BUILT_IN(CONV)
        __BUILD_BUILT_IN(CONV_RTE)
        __BUILD_BUILT_IN(CONV_RTZ)
        __BUILD_BUILT_IN(CONV_RTN)
        __BUILD_BUILT_IN(CONV_RTP)
        __BUILD_BUILT_IN(CONV_SAT)
        __BUILD_BUILT_IN(CONV_SAT_RTE)
        __BUILD_BUILT_IN(CONV_SAT_RTZ)
        __BUILD_BUILT_IN(CONV_SAT_RTN)
        __BUILD_BUILT_IN(CONV_SAT_RTP)

        /* Arithmetic Operations */
        __BUILD_BUILT_IN(ADD)
        __BUILD_BUILT_IN(SUB)
        __BUILD_BUILT_IN(MUL)
        __BUILD_BUILT_IN(MUL_Z)
        __BUILD_BUILT_IN(FADD)
        __BUILD_BUILT_IN(FSUB)
        __BUILD_BUILT_IN(FMUL)
        __BUILD_BUILT_IN(DIV)
        __BUILD_BUILT_IN(IDIV)
        __BUILD_BUILT_IN(IMUL)
        __BUILD_BUILT_IN(MOD)
        __BUILD_BUILT_IN(FMOD)
        __BUILD_BUILT_IN(SELECT)
        __BUILD_BUILT_IN(FMA)
        __BUILD_BUILT_IN(TEXTURE_LOAD)
        __BUILD_BUILT_IN(IMAGE_SAMPLER)
        __BUILD_BUILT_IN(IMAGE_READ)
        __BUILD_BUILT_IN(IMAGE_READ_3D)
        __BUILD_BUILT_IN(IMAGE_WRITE)
        __BUILD_BUILT_IN(IMAGE_WRITE_3D)

        /* Conversion Operations */
        __BUILD_BUILT_IN(FLOAT_TO_INT)
        __BUILD_BUILT_IN(FLOAT_TO_UINT)
        __BUILD_BUILT_IN(FLOAT_TO_BOOL)
        __BUILD_BUILT_IN(INT_TO_INT)
        __BUILD_BUILT_IN(INT_TO_UINT)
        __BUILD_BUILT_IN(INT_TO_BOOL)
        __BUILD_BUILT_IN(INT_TO_FLOAT)
        __BUILD_BUILT_IN(UINT_TO_UINT)
        __BUILD_BUILT_IN(UINT_TO_INT)
        __BUILD_BUILT_IN(UINT_TO_BOOL)
        __BUILD_BUILT_IN(UINT_TO_FLOAT)
        __BUILD_BUILT_IN(BOOL_TO_FLOAT)
        __BUILD_BUILT_IN(BOOL_TO_INT)
        __BUILD_BUILT_IN(BOOL_TO_UINT)

        __BUILD_BUILT_IN(IMPL_B2F)
        __BUILD_BUILT_IN(IMPL_U2F)
        __BUILD_BUILT_IN(IMPL_I2F)

        /* Other Calculation Operations */
        __BUILD_BUILT_IN(INVERSE)

        __BUILD_BUILT_IN(LESS_THAN)
        __BUILD_BUILT_IN(LESS_THAN_EQUAL)
        __BUILD_BUILT_IN(GREATER_THAN)
        __BUILD_BUILT_IN(GREATER_THAN_EQUAL)
        __BUILD_BUILT_IN(EQUAL)
        __BUILD_BUILT_IN(NOT_EQUAL)

        __BUILD_BUILT_IN(AND_BITWISE)
        __BUILD_BUILT_IN(OR_BITWISE)
        __BUILD_BUILT_IN(XOR_BITWISE)
        __BUILD_BUILT_IN(NOT_BITWISE)
        {"BITWISE_AND", clvOPCODE_AND_BITWISE},
        {"BITWISE_OR",  clvOPCODE_OR_BITWISE },
        {"BITWISE_XOR", clvOPCODE_XOR_BITWISE},
        {"BITWISE_NOT", clvOPCODE_NOT_BITWISE},

        __BUILD_BUILT_IN(RSHIFT)
        __BUILD_BUILT_IN(LSHIFT)
        __BUILD_BUILT_IN(RIGHT_SHIFT)
        __BUILD_BUILT_IN(LEFT_SHIFT)

        __BUILD_BUILT_IN(ADDR)
        __BUILD_BUILT_IN(INDIRECTION)
        __BUILD_BUILT_IN(NON_LVAL)

        __BUILD_BUILT_IN(BARRIER)
        __BUILD_BUILT_IN(MEM_FENCE)
        __BUILD_BUILT_IN(LOAD)
        __BUILD_BUILT_IN(STORE)
        __BUILD_BUILT_IN(STORE1)

        __BUILD_BUILT_IN(ANY)
        __BUILD_BUILT_IN(ALL)
        __BUILD_BUILT_IN(NOT)
        __BUILD_BUILT_IN(NEG)

        __BUILD_BUILT_IN(SIN)
        __BUILD_BUILT_IN(COS)
        __BUILD_BUILT_IN(TAN)

        __BUILD_BUILT_IN(ASIN)
        __BUILD_BUILT_IN(ACOS)
        __BUILD_BUILT_IN(ATAN)
        __BUILD_BUILT_IN(ATAN2)

        __BUILD_BUILT_IN(SINPI)
        __BUILD_BUILT_IN(COSPI)
        __BUILD_BUILT_IN(TANPI)

        __BUILD_BUILT_IN(ARCTRIG0)
        __BUILD_BUILT_IN(ARCTRIG1)

        __BUILD_BUILT_IN(POW)
        __BUILD_BUILT_IN(EXP2)
        __BUILD_BUILT_IN(LOG2)
        __BUILD_BUILT_IN(SQRT)
        __BUILD_BUILT_IN(INVERSE_SQRT)
        {"RSQ", clvOPCODE_INVERSE_SQRT},

        __BUILD_BUILT_IN(MULLO)
        __BUILD_BUILT_IN(ADDLO)

        __BUILD_BUILT_IN(ROTATE)
        __BUILD_BUILT_IN(LEADZERO)
        __BUILD_BUILT_IN(GETEXP)
        __BUILD_BUILT_IN(GETMANT)

        /*Integer only, get the overflow part*/
        __BUILD_BUILT_IN(MULHI)

        __BUILD_BUILT_IN(SET)
        __BUILD_BUILT_IN(CMP)

        __BUILD_BUILT_IN(ABS)
        __BUILD_BUILT_IN(SIGN)
        __BUILD_BUILT_IN(FLOOR)
        __BUILD_BUILT_IN(CEIL)
        __BUILD_BUILT_IN(FRACT)
        __BUILD_BUILT_IN(MIN)
        __BUILD_BUILT_IN(MAX)
        __BUILD_BUILT_IN(SATURATE)
        __BUILD_BUILT_IN(STEP)
        __BUILD_BUILT_IN(DOT)
        __BUILD_BUILT_IN(CROSS)
        __BUILD_BUILT_IN(NORMALIZE)
        __BUILD_BUILT_IN(POPCOUNT)

        /* Branch Operations */
        __BUILD_BUILT_IN(JUMP)
        __BUILD_BUILT_IN(CALL)
        __BUILD_BUILT_IN(RETURN)

        /* Derivative Operations */
        __BUILD_BUILT_IN(DFDX)
        __BUILD_BUILT_IN(DFDY)
        __BUILD_BUILT_IN(FWIDTH)
        __BUILD_BUILT_IN(SUBSAT)
        __BUILD_BUILT_IN(ADDSAT)
        __BUILD_BUILT_IN(MULSAT)

        __BUILD_BUILT_IN(ATOMADD)
        __BUILD_BUILT_IN(ATOMSUB)
        __BUILD_BUILT_IN(ATOMXCHG)
        __BUILD_BUILT_IN(ATOMCMPXCHG)
        __BUILD_BUILT_IN(ATOMMIN)
        __BUILD_BUILT_IN(ATOMMAX)
        __BUILD_BUILT_IN(ATOMOR)
        __BUILD_BUILT_IN(ATOMAND)
        __BUILD_BUILT_IN(ATOMXOR)

        __BUILD_BUILT_IN(ADD_RTZ)
        __BUILD_BUILT_IN(ADD_RTNE)
        __BUILD_BUILT_IN(ADDLO_RTZ)
        __BUILD_BUILT_IN(ADDLO_RTNE)
        __BUILD_BUILT_IN(SUB_RTZ)
        __BUILD_BUILT_IN(SUB_RTNE)
        __BUILD_BUILT_IN(MUL_RTZ)
        __BUILD_BUILT_IN(MUL_RTNE)
        __BUILD_BUILT_IN(MULLO_RTZ)
        __BUILD_BUILT_IN(MULLO_RTNE)
        __BUILD_BUILT_IN(FRACT_RTZ)
        __BUILD_BUILT_IN(FRACT_RTNE)
        __BUILD_BUILT_IN(INT_TO_FLOAT_RTZ)
        __BUILD_BUILT_IN(INT_TO_FLOAT_RTNE)
        __BUILD_BUILT_IN(UINT_TO_FLOAT_RTZ)
        __BUILD_BUILT_IN(UINT_TO_FLOAT_RTNE)

        __BUILD_BUILT_IN(UNPACK)
        __BUILD_BUILT_IN(ASTYPE)
        __BUILD_BUILT_IN(PARAM_CHAIN)
        __BUILD_BUILT_IN(INTRINSIC)
        __BUILD_BUILT_IN(INTRINSIC_ST)
        __BUILD_BUILT_IN(CLAMP0MAX)
        __BUILD_BUILT_IN(CLAMPCOORD)

        __BUILD_BUILT_IN(FMA_MUL)
        __BUILD_BUILT_IN(FMA_ADD)

        __BUILD_BUILT_IN(TEXU)
        __BUILD_BUILT_IN(GET_IMAGE_TYPE)
    };

#undef __BUILD_BUILT_IN
#define _cldBuiltinAsmConstantCount (sizeof(_BuiltinAsmConstants) / sizeof(struct _clsBUILTIN_ASM_CONSTANT))


    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    /* Setup the constant values */
/* comment out openGL stuff */

    for (i = 0; i < _cldBuiltinConstantCount; i++) {
        /* Create the data type */
        status = cloCOMPILER_CreateDecl(Compiler,
                                            _BuiltinConstants[i].type,
                                            gcvNULL,
                                            clvQUALIFIER_CONST,
                                            clvQUALIFIER_CONSTANT,
                                            &decl);
        if (gcmIS_ERROR(status)) return status;

        /* Create the constant */
        status = cloIR_CONSTANT_Construct(Compiler,
                          0,
                          0,
                          &decl,
                          &constant);
        if (gcmIS_ERROR(status)) break;

        (void)gcoOS_ZeroMemory(&value, sizeof(cluCONSTANT_VALUE));
        if(clmIsElementTypeHighPrecision(decl.dataType->elementType)) {
            value.ulongValue = _BuiltinConstants[i].value;
        }
        else {
            value.uintValue = ((gctUINT *)(&_BuiltinConstants[i].value))[0];
        }

        status = cloIR_CONSTANT_AddValues(Compiler,
                          constant,
                          1,
                          &value);
        if (gcmIS_ERROR(status)) break;

        gcmVERIFY_OK(cloCOMPILER_AddExternalDecl(Compiler, &constant->exprBase.base));
        /* Create the variable name */
        status = cloCOMPILER_AllocatePoolString(Compiler,
                            _BuiltinConstants[i].symbol,
                            &variableSymbol);
        if (gcmIS_ERROR(status)) break;

        status = cloCOMPILER_CreateName(Compiler,
                        0,
                        0,
                        clvVARIABLE_NAME,
                        &decl,
                        variableSymbol,
                        gcvNULL,
                        clvEXTENSION_NONE,
                        &variableName);
        if (gcmIS_ERROR(status)) break;

        variableName->u.variableInfo.u.constant = constant;
        variableName->u.variableInfo.u.constant->variable = variableName;
        variableName->isBuiltin = gcvTRUE;
    }


    for (i = 0; i < _cldBuiltinAsmConstantCount; i++) {
        /* Create the data type */
        status = cloCOMPILER_CreateDecl(Compiler,
                                        T_UINT,
                                        gcvNULL,
                                        clvQUALIFIER_CONST,
                                        clvQUALIFIER_NONE,
                                        &decl);
        if (gcmIS_ERROR(status)) return status;

        /* Create the constant */
        status = cloIR_CONSTANT_Construct(Compiler,
                                          0,
                                          0,
                                          &decl,
                                          &constant);
        if (gcmIS_ERROR(status)) break;

        value.uintValue = _BuiltinAsmConstants[i].value;
        status = cloIR_CONSTANT_AddValues(Compiler,
                                          constant,
                                          1,
                                          &value);
        if (gcmIS_ERROR(status)) break;

        gcmVERIFY_OK(cloCOMPILER_AddExternalDecl(Compiler, &constant->exprBase.base));
        /* Create the variable name */
        status = cloCOMPILER_AllocatePoolString(Compiler,
                                                _BuiltinAsmConstants[i].symbol,
                                                &variableSymbol);
        if (gcmIS_ERROR(status)) break;

        status = cloCOMPILER_CreateName(Compiler,
                                        0,
                                        0,
                                        clvVARIABLE_NAME,
                                        &decl,
                                        variableSymbol,
                                        gcvNULL,
                                        clvEXTENSION_VASM,
                                        &variableName);
        if (gcmIS_ERROR(status)) break;

        variableName->u.variableInfo.u.constant = constant;
        variableName->u.variableInfo.u.constant->variable = variableName;
    }
    return status;
}

/* Built-In Variables */
typedef struct _clsBUILTIN_UNNAMED_VARIABLE
{
    cleBUILTIN_VARIABLE id;
    cltQUALIFIER accessQualifier;
    cltQUALIFIER addrSpaceQualifier;
    gctINT type;
    gctBOOL isPtr;
    gctCONST_STRING    implSymbol;
}
clsBUILTIN_UNNAMED_VARIABLE;

static clsBUILTIN_UNNAMED_VARIABLE _BuiltinUnnamedVariables[] = {
    {clvBUILTIN_NONE, clvQUALIFIER_NONE, clvQUALIFIER_NONE, 0, gcvFALSE, ""},
    {clvBUILTIN_GLOBAL_ID, clvQUALIFIER_ATTRIBUTE, clvQUALIFIER_NONE, T_UINT4, gcvFALSE, "#global_id"},
    {clvBUILTIN_PRE_SCALE_GLOBAL_ID, clvQUALIFIER_NONE, clvQUALIFIER_NONE, T_UINT4, gcvFALSE, "#pre_scale_global_id"},
    {clvBUILTIN_LOCAL_ID, clvQUALIFIER_ATTRIBUTE, clvQUALIFIER_NONE, T_UINT4, gcvFALSE, "#local_id"},
    {clvBUILTIN_GROUP_ID, clvQUALIFIER_ATTRIBUTE, clvQUALIFIER_NONE, T_UINT4, gcvFALSE, "#group_id"},
    {clvBUILTIN_WORK_DIM, clvQUALIFIER_UNIFORM, clvQUALIFIER_NONE, T_UINT, gcvFALSE, "#work_dim"},
    {clvBUILTIN_GLOBAL_SIZE, clvQUALIFIER_UNIFORM, clvQUALIFIER_NONE, T_UINT4, gcvFALSE, "#global_size"},
    {clvBUILTIN_GLOBAL_WORK_SCALE, clvQUALIFIER_UNIFORM, clvQUALIFIER_NONE, T_UINT4, gcvFALSE, "#global_work_scale"},
    {clvBUILTIN_LOCAL_SIZE, clvQUALIFIER_UNIFORM, clvQUALIFIER_NONE, T_UINT4, gcvFALSE, "#local_size"},
    {clvBUILTIN_NUM_GROUPS, clvQUALIFIER_UNIFORM, clvQUALIFIER_NONE, T_UINT4, gcvFALSE, "#num_groups"},
    {clvBUILTIN_GLOBAL_OFFSET, clvQUALIFIER_UNIFORM, clvQUALIFIER_NONE, T_UINT4, gcvFALSE, "#global_offset"},
    {clvBUILTIN_LOCAL_ADDRESS_SPACE, clvQUALIFIER_UNIFORM, clvQUALIFIER_NONE, T_CHAR, gcvTRUE, _sldLocalStorageAddressName},
    {clvBUILTIN_PRIVATE_ADDRESS_SPACE, clvQUALIFIER_UNIFORM, clvQUALIFIER_NONE, T_CHAR, gcvTRUE, "#private_address"},
    {clvBUILTIN_CONSTANT_ADDRESS_SPACE, clvQUALIFIER_UNIFORM, clvQUALIFIER_NONE, T_CHAR, gcvTRUE, "#constant_address"},
    {clvBUILTIN_ARG_LOCAL_MEM_SIZE, clvQUALIFIER_UNIFORM, clvQUALIFIER_NONE, T_UINT, gcvFALSE, "#arg_local_mem_size"},
    {clvBUILTIN_PRINTF_ADDRESS, clvQUALIFIER_UNIFORM, clvQUALIFIER_NONE, T_CHAR, gcvTRUE, "#printf_address"},
    {clvBUILTIN_WORKITEM_PRINTF_BUFFER_SIZE, clvQUALIFIER_UNIFORM, clvQUALIFIER_NONE, T_UINT, gcvFALSE, "#workItem_printf_buffer_size"},
    {clvBUILTIN_CLUSTER_ID, clvQUALIFIER_ATTRIBUTE, clvQUALIFIER_NONE, T_UINT, gcvFALSE, "#cluster_id"},
};

#define _cldBuiltinUnnamedVariableStart 1
#define _cldBuiltinUnnamedVariableCount \
        (sizeof(_BuiltinUnnamedVariables) / sizeof(clsBUILTIN_UNNAMED_VARIABLE))

#define _cldUnnamedVariable(id)  cloCOMPILER_GetBuiltinVariable(Compiler, id)
#define _cldUnnamedVariableRegCount(id) _cldUnnamedVariable(id)->context.u.variable.logicalRegCount
#define _cldUnnamedVariableRegs(id) _cldUnnamedVariable(id)->context.u.variable.logicalRegs
#define _cldUnnamedVariableType(id) clmGenCodeDataType(_BuiltinUnnamedVariables[id].type)

/* Built-In Variables */
typedef struct _clsBUILT_IN_VARIABLE
{
    gctCONST_STRING    symbol;
    cltQUALIFIER    accessQualifier;
    cltQUALIFIER    addrSpaceQualifier;
    gctINT        type;
    clsARRAY    array;
}
clsBUILT_IN_VARIABLE;

/* comment out openGL stuff */

static gceSTATUS
_LoadBuiltinUnnamedVariables(
IN cloCOMPILER Compiler
)
{
  gceSTATUS status = gcvSTATUS_OK;
  gctUINT i;
  clsDATA_TYPE *dataType;
  clsDECL decl;
  clsNAME *unnamedVar;
  clsNAME_SPACE *nameSpace;

  gcmHEADER_ARG("Compiler=0x%x", Compiler);
/* Verify the arguments. */
  clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

  status = cloCOMPILER_PushUnnamedSpace(Compiler, &nameSpace);
  if (gcmIS_ERROR(status)) {
    gcmFOOTER();
    return status;
  }

  for (i = _cldBuiltinUnnamedVariableStart; i < _cldBuiltinUnnamedVariableCount; i++) {
    /* Setup the data type */
    gcmASSERT(i == (gctUINT)_BuiltinUnnamedVariables[i].id);
    status = cloCOMPILER_CreateDataType(Compiler,
                                        _BuiltinUnnamedVariables[i].type,
                                        gcvNULL,
                                        _BuiltinUnnamedVariables[i].accessQualifier,
                                        _BuiltinUnnamedVariables[i].addrSpaceQualifier,
                                        &dataType);
    if (gcmIS_ERROR(status)) break;

    clmDECL_Initialize(&decl, dataType, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);

    if(_BuiltinUnnamedVariables[i].isPtr) {
       status = clParseAddIndirectionOneLevel(Compiler,
                                              &decl.ptrDscr);
       if(gcmIS_ERROR(status))break;
    }

    status = cloCOMPILER_CreateName(Compiler,
                                    0,
                                    0,
                                    clvVARIABLE_NAME,
                                    &decl,
                                    "",
                                    decl.ptrDscr,
                                    clvEXTENSION_NONE,
                                    &unnamedVar);
    if (gcmIS_ERROR(status)) break;
    unnamedVar->u.variableInfo.builtinSpecific.s.variableType = _BuiltinUnnamedVariables[i].id;
    status = cloCOMPILER_RegisterBuiltinVariable(Compiler, i, unnamedVar);
    if (gcmIS_ERROR(status)) break;
  }

  status = cloCOMPILER_PopCurrentNameSpace(Compiler, &nameSpace);
  if (gcmIS_ERROR(status)) {
    gcmFOOTER();
    return status;
  }

  gcmFOOTER();
  return status;
}

/* Built-In Functions */
typedef struct _clsBUILTIN_FUNCTION
{
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
}
clsBUILTIN_FUNCTION;

/* Built-In Functions */
typedef struct _clsINTRINCSIC_BUILTIN_FUNCTION
{
    cleEXTENSION    extension;
    gctCONST_STRING symbol;
    gceINTRINSICS_KIND intrinsicKind;
    gctCONST_STRING    nameInLibrary;          /* "mangled" name inside the library */
    gctINT          returnType;
    gctUINT         paramCount;
    gctINT          paramTypes[clmMAX_BUILT_IN_PARAMETER_COUNT];
    gctUINT8        ptrLevels[clmMAX_BUILT_IN_PARAMETER_COUNT];
    gctUINT8        typeConvertible[clmMAX_BUILT_IN_PARAMETER_COUNT];
    gctBOOL         isInline;
    gctBOOL         hasWriteArg;
    gctBOOL         passArgByRef;
    gctBOOL         hasVarArg;
}
clsINTRINSIC_BUILTIN_FUNCTION;

#include "gc_cl_built_ins_ks.h"
#include "gc_cl_built_ins_common.h"
#include "gc_cl_built_ins_math.h"
#include "gc_cl_built_ins_int.h"
#include "gc_cl_built_ins_vector.h"
#include "gc_cl_built_ins_conv.h"
#include "gc_cl_built_ins_image.h"
#include "gc_cl_built_ins_intrinsic.h"

static gctINT
_ConvVectorBasicTypeToPacked(
cloCOMPILER Compiler,
gctINT TypeToken
)
{
    if(cloCOMPILER_IsBasicTypePacked(Compiler) ||
       cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX)) {
        clsBUILTIN_DATATYPE_INFO *typeInfo;

        typeInfo = clGetBuiltinDataTypeInfo(TypeToken);
        if(typeInfo &&
           typeInfo->type != typeInfo->dualType &&
           clmGEN_CODE_IsVectorDataType(typeInfo->dataType) &&
           !clmIsElementTypePacked(typeInfo->dataType.elementType)) {
           TypeToken = typeInfo->dualType;
        }
    }
    return TypeToken;
}

static gceSTATUS
_LoadBuiltinFunctions(
IN cloCOMPILER Compiler,
IN gctUINT BuiltinFunctionCount,
IN clsBUILTIN_FUNCTION * BuiltinFunctions
)
{
    gceSTATUS    status = gcvSTATUS_OK;
    gctUINT        i, j, k;
    cltPOOL_STRING    symbolInPool;
    clsNAME    *    funcName = gcvNULL;
    clsNAME    *    paramName = gcvNULL;
    clsDATA_TYPE *dataType;
    gctINT tok;
    clsDECL    decl;

    gcmHEADER_ARG("Compiler=0x%x "
              "BuiltinFunctionCount=%u BuiltinFunctions=0x%x",
              Compiler, BuiltinFunctionCount, BuiltinFunctions);

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmVERIFY_ARGUMENT(BuiltinFunctionCount > 0);
    gcmVERIFY_ARGUMENT(BuiltinFunctions);

    for (i = 0; i < BuiltinFunctionCount; i++) {
        /* Create function name */
        status = cloCOMPILER_AllocatePoolString(Compiler,
                                                BuiltinFunctions[i].symbol,
                                                &symbolInPool);
        if (gcmIS_ERROR(status)) break;

        /* convert basic vector type to packed if necessary */
        tok = _ConvVectorBasicTypeToPacked(Compiler,
                                           BuiltinFunctions[i].returnType);

        status = cloCOMPILER_CreateDataType(Compiler,
                                            tok,
                                            gcvNULL,
                                            clvQUALIFIER_NONE,
                                            clvQUALIFIER_NONE,
                                            &dataType);
        if (gcmIS_ERROR(status)) break;

        clmDECL_Initialize(&decl, dataType, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
        status = cloCOMPILER_CreateName(Compiler,
                                        0,
                                        0,
                                        clvFUNC_NAME,
                                        &decl,
                                        symbolInPool,
                                        gcvNULL,
                                        BuiltinFunctions[i].extension,
                                        &funcName);
        if (gcmIS_ERROR(status)) break;
        funcName->u.funcInfo.hasGenType = clmDATA_TYPE_IsGenType(dataType);

        status = cloCOMPILER_CreateNameSpace(Compiler,
                                             &funcName->u.funcInfo.localSpace);

        if (gcmIS_ERROR(status)) break;
        funcName->u.funcInfo.localSpace->scopeName = funcName;
        funcName->u.funcInfo.localSpace->die = funcName->die;

        for (j = 0; j < BuiltinFunctions[i].paramCount; j++) {
            /* convert basic vector type to packed if necessary */
            tok = _ConvVectorBasicTypeToPacked(Compiler,
                                               BuiltinFunctions[i].paramTypes[j]);

            /* Create parameter name */
            status = cloCOMPILER_CreateDataType(Compiler,
                                                tok,
                                                gcvNULL,
                                                clvQUALIFIER_NONE,
                                                clvQUALIFIER_NONE,
                                                &dataType);
            if (gcmIS_ERROR(status)) break;

            clmDECL_Initialize(&decl, dataType, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
            for(k = 0; k < BuiltinFunctions[i].ptrLevels[j]; k++) {
                status = clParseAddIndirectionOneLevel(Compiler,
                                                       &decl.ptrDscr);
                if(gcmIS_ERROR(status))break;
            }
            if(gcmIS_ERROR(status))break;

            status = cloCOMPILER_CreateName(Compiler,
                                            0,
                                            0,
                                            clvPARAMETER_NAME,
                                            &decl,
                                            "",
                                            decl.ptrDscr,
                                            clvEXTENSION_NONE,
                                            &paramName);
            if (gcmIS_ERROR(status)) break;
            paramName->u.variableInfo.builtinSpecific.s.isConvertibleType = BuiltinFunctions[i].typeConvertible[j];
            paramName->u.variableInfo.builtinSpecific.s.hasGenType = clmDATA_TYPE_IsGenType(dataType);
        }
        if (gcmIS_ERROR(status)) break;
        cloCOMPILER_PopCurrentNameSpace(Compiler, gcvNULL);
        funcName->u.funcInfo.isFuncDef = gcvFALSE;
        funcName->u.funcInfo.isInline = BuiltinFunctions[i].isInline;
        funcName->u.funcInfo.hasVarArg = BuiltinFunctions[i].hasVarArg;
        funcName->u.funcInfo.hasWriteArg = BuiltinFunctions[i].hasWriteArg;
        funcName->u.funcInfo.passArgByRef = BuiltinFunctions[i].passArgByRef;
    }
    gcmFOOTER();
    return status;
}

static gceSTATUS
_LoadIntrinsicBuiltinFunctions(
IN cloCOMPILER Compiler,
IN gctUINT IntrinsicFunctionCount,
IN clsINTRINSIC_BUILTIN_FUNCTION * IntrinsicFunctions
)
{
    gceSTATUS    status = gcvSTATUS_OK;
    gctUINT        i, j, k;
    cltPOOL_STRING    symbolInPool;
    clsNAME    *    funcName = gcvNULL;
    clsNAME    *    paramName = gcvNULL;
    clsDATA_TYPE *dataType;
    clsDECL    decl;

    gcmHEADER_ARG("Compiler=0x%x "
              "IntrinsicFunctionCount=%u IntrinsicFunctions=0x%x",
              Compiler, IntrinsicFunctionCount, IntrinsicFunctions);

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmVERIFY_ARGUMENT(IntrinsicFunctionCount > 0);
    gcmVERIFY_ARGUMENT(IntrinsicFunctions);

    for (i = 0; i < IntrinsicFunctionCount; i++) {
        /* Create function name */
        status = cloCOMPILER_AllocatePoolString(Compiler,
                                                IntrinsicFunctions[i].symbol,
                                                &symbolInPool);
        if (gcmIS_ERROR(status)) break;

        status = cloCOMPILER_CreateDataType(Compiler,
                                            IntrinsicFunctions[i].returnType,
                                            gcvNULL,
                                            clvQUALIFIER_NONE,
                                            clvQUALIFIER_NONE,
                                            &dataType);
        if (gcmIS_ERROR(status)) break;

        clmDECL_Initialize(&decl, dataType, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
        status = cloCOMPILER_CreateName(Compiler,
                                        0,
                                        0,
                                        clvFUNC_NAME,
                                        &decl,
                                        symbolInPool,
                                        gcvNULL,
                                        IntrinsicFunctions[i].extension,
                                        &funcName);
        if (gcmIS_ERROR(status)) break;
        funcName->u.funcInfo.hasGenType = clmDATA_TYPE_IsGenType(dataType);

        status = cloCOMPILER_CreateNameSpace(Compiler,
                                             &funcName->u.funcInfo.localSpace);

        if (gcmIS_ERROR(status)) break;
        funcName->u.funcInfo.localSpace->scopeName = funcName;
        funcName->u.funcInfo.localSpace->die = funcName->die;

        for (j = 0; j < IntrinsicFunctions[i].paramCount; j++) {
            /* Create parameter name */
            status = cloCOMPILER_CreateDataType(Compiler,
                                                IntrinsicFunctions[i].paramTypes[j],
                                                gcvNULL,
                                                clvQUALIFIER_NONE,
                                                clvQUALIFIER_NONE,
                                                &dataType);
            if (gcmIS_ERROR(status)) break;

            clmDECL_Initialize(&decl, dataType, (clsARRAY *)0, gcvNULL, gcvFALSE, clvSTORAGE_QUALIFIER_NONE);
            for(k = 0; k < IntrinsicFunctions[i].ptrLevels[j]; k++) {
                status = clParseAddIndirectionOneLevel(Compiler,
                                                       &decl.ptrDscr);
                if(gcmIS_ERROR(status))break;
            }
            if(gcmIS_ERROR(status))break;

            status = cloCOMPILER_CreateName(Compiler,
                                            0,
                                            0,
                                            clvPARAMETER_NAME,
                                            &decl,
                                            "",
                                            decl.ptrDscr,
                                            clvEXTENSION_NONE,
                                            &paramName);
            if (gcmIS_ERROR(status)) break;
            paramName->u.variableInfo.builtinSpecific.s.isConvertibleType = IntrinsicFunctions[i].typeConvertible[j];
            paramName->u.variableInfo.builtinSpecific.s.hasGenType = clmDATA_TYPE_IsGenType(dataType);
        }
        if (gcmIS_ERROR(status)) break;
        cloCOMPILER_PopCurrentNameSpace(Compiler, gcvNULL);
        funcName->u.funcInfo.isIntrinsicCall = gcvTRUE;
        funcName->u.funcInfo.intrinsicKind   = IntrinsicFunctions[i].intrinsicKind;
        /* set its managled_symbol */
        if(!funcName->u.funcInfo.hasGenType) {
            if(IntrinsicFunctions[i].nameInLibrary) {
                status = cloCOMPILER_AllocatePoolString(Compiler,
                                                        IntrinsicFunctions[i].nameInLibrary,
                                                        &symbolInPool);
            }
            else {
                symbolInPool = clCreateMangledFuncName(Compiler,
                                                       funcName);
                if(symbolInPool) {
                    IntrinsicFunctions[i].nameInLibrary = symbolInPool;
                }
                else {
                    status = gcvSTATUS_INVALID_ARGUMENT;
                }
            }
            if (gcmIS_ERROR(status)) break;
            funcName->u.funcInfo.mangledName = symbolInPool;
        }
        funcName->u.funcInfo.isFuncDef = gcvFALSE;
        funcName->u.funcInfo.isInline = IntrinsicFunctions[i].isInline;
        funcName->u.funcInfo.hasVarArg = IntrinsicFunctions[i].hasVarArg;
        funcName->u.funcInfo.hasWriteArg = IntrinsicFunctions[i].hasWriteArg;
        funcName->u.funcInfo.passArgByRef = IntrinsicFunctions[i].passArgByRef;
    }
    gcmFOOTER();
    return status;
}

static gceSTATUS
_ConstructBuiltinFunctionInfos(IN cloCOMPILER Compiler);

gceSTATUS
clLoadGeneralBuiltIns(
IN cloCOMPILER Compiler,
IN cleSHADER_TYPE ShaderType
)
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x ShaderType=0x%x",
                  Compiler, ShaderType);

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    status = _ConstructBuiltinDataTypeInfos(Compiler);
    if (gcmIS_ERROR(status)) {
        gcmFOOTER();
        return status;
    }

#if gcdDEBUG
    gcmASSERT(_IsBuiltinDataTypeInfoSorted(Compiler));
#endif

    status = _ConstructBuiltinFunctionInfos(Compiler);
    if (gcmIS_ERROR(status)) {
        gcmFOOTER();
        return status;
    }
#if LOAD_BUILT_IN_FUNCS_ONLY_ONCE
    do {
        /* Load built-in functions */
        status = _LoadIntrinsicBuiltinFunctions(Compiler,
                                                _cldIntrinsicBuiltinFunctionCount,
                                                IntrinsicBuiltinFunctions);
        if (gcmIS_ERROR(status)) break;

        status = _LoadBuiltinFunctions(Compiler,
                        _cldKSBuiltinFunctionCount,
                        KSBuiltinFunctions);
        if (gcmIS_ERROR(status)) break;

        status = _LoadBuiltinFunctions(Compiler,
                           _cldCommonBuiltinFunctionCount,
                           CommonBuiltinFunctions);
        if (gcmIS_ERROR(status)) break;

        status = _LoadBuiltinFunctions(Compiler,
                           _cldMathBuiltinFunctionCount,
                           MathBuiltinFunctions);
        if (gcmIS_ERROR(status)) break;

        status = _LoadBuiltinFunctions(Compiler,
                           _cldIntBuiltinFunctionCount,
                           IntBuiltinFunctions);
        if (gcmIS_ERROR(status)) break;

        status = _LoadBuiltinFunctions(Compiler,
                           _cldVectorBuiltinFunctionCount,
                           VectorBuiltinFunctions);
        if (gcmIS_ERROR(status)) break;

        status = _LoadBuiltinFunctions(Compiler,
                           _cldConvBuiltinFunctionCount,
                           ConvBuiltinFunctions);
        if (gcmIS_ERROR(status)) break;

        status = _LoadBuiltinFunctions(Compiler,
                           _cldImageBuiltinFunctionCount,
                           ImageBuiltinFunctions);
        if (gcmIS_ERROR(status)) break;

        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    } while (gcvFALSE);
#endif

    gcmFOOTER();
    return status;
}

gceSTATUS
clLoadBuiltins(
IN cloCOMPILER Compiler,
IN cleSHADER_TYPE ShaderType
)
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x ShaderType=0x%x",
                  Compiler, ShaderType);

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    do {
        /* Load built-in constants */
        status = _LoadBuiltinConstants(Compiler);
        if (gcmIS_ERROR(status)) break;

        /* Load unnamed variables: attributes, uniforms */
        status = _LoadBuiltinUnnamedVariables(Compiler);
        if (gcmIS_ERROR(status)) break;

#if !LOAD_BUILT_IN_FUNCS_ONLY_ONCE
        /* Load built-in functions */
        status = _LoadIntrinsicBuiltinFunctions(Compiler,
                                                _cldIntrinsicBuiltinFunctionCount,
                                                IntrinsicBuiltinFunctions);
        if (gcmIS_ERROR(status)) break;

        status = _LoadBuiltinFunctions(Compiler,
                        _cldKSBuiltinFunctionCount,
                        KSBuiltinFunctions);
        if (gcmIS_ERROR(status)) break;

        status = _LoadBuiltinFunctions(Compiler,
                           _cldCommonBuiltinFunctionCount,
                           CommonBuiltinFunctions);
        if (gcmIS_ERROR(status)) break;

        status = _LoadBuiltinFunctions(Compiler,
                           _cldMathBuiltinFunctionCount,
                           MathBuiltinFunctions);
        if (gcmIS_ERROR(status)) break;

        status = _LoadBuiltinFunctions(Compiler,
                           _cldIntBuiltinFunctionCount,
                           IntBuiltinFunctions);
        if (gcmIS_ERROR(status)) break;

        status = _LoadBuiltinFunctions(Compiler,
                           _cldVectorBuiltinFunctionCount,
                           VectorBuiltinFunctions);
        if (gcmIS_ERROR(status)) break;

        status = _LoadBuiltinFunctions(Compiler,
                           _cldConvBuiltinFunctionCount,
                           ConvBuiltinFunctions);
        if (gcmIS_ERROR(status)) break;

        status = _LoadBuiltinFunctions(Compiler,
                           _cldImageBuiltinFunctionCount,
                           ImageBuiltinFunctions);
        if (gcmIS_ERROR(status)) break;
#endif

        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    } while (gcvFALSE);

    gcmFOOTER();
    return status;
}

/* Built-In Variables */
typedef struct _clsBUILT_IN_VARIABLE_INFO
{
    gctCONST_STRING        symbol;
    gctCONST_STRING        implSymbol;
}
clsBUILT_IN_VARIABLE_INFO;

const clsBUILT_IN_VARIABLE_INFO    BuiltinVariableInfos[] =
{
    /* Vertex Shader Variables */
    {"gl_Position",            "#Position"},
    {"gl_PointSize",        "#PointSize"},

    /* Fragment Shader Variables */
    {"gl_FragCoord",        "#Position"},
    {"gl_FrontFacing",        "#FrontFacing"},
    {"gl_FragColor",        "#Color"},
    {"gl_FragData",            "#Color"},
    {"gl_PointCoord",        "#PointCoord"},

    /* Built-In Uniforms */
    {"gl_DepthRange.near",        "#DepthRange.near"},
    {"gl_DepthRange.far",        "#DepthRange.far"},
    {"gl_DepthRange.diff",        "#DepthRange.diff"}
};

#define _cldBuiltinVariableCount (sizeof(BuiltinVariableInfos) / sizeof(clsBUILT_IN_VARIABLE_INFO))

gceSTATUS
clGetBuiltinVariableImplSymbol(
IN cloCOMPILER Compiler,
IN clsNAME *Name,
IN gctCONST_STRING Symbol,
OUT gctCONST_STRING * ImplSymbol
)
{
   gctUINT    i;

   gcmHEADER_ARG("Name = 0x%x Symbol=0x%x ImplSymbol=0x%x", Name, Symbol, ImplSymbol);

/* Verify the arguments. */
   gcmASSERT(Name);
   gcmASSERT(Symbol);
   gcmASSERT(ImplSymbol);

   if(Symbol[0] != '\0') { /* Named variable */
      for (i = 0; i < _cldBuiltinVariableCount; i++) {
         if (gcmIS_SUCCESS(gcoOS_StrCmp(BuiltinVariableInfos[i].symbol, Symbol))) {
            *ImplSymbol = BuiltinVariableInfos[i].implSymbol;
            break;
         }
      }

      gcmASSERT(i < _cldBuiltinVariableCount);
   }
   else { /* unnamed */
      for (i = 0; i < _cldBuiltinUnnamedVariableCount; i++) {
         if(_cldUnnamedVariable(i) == Name) {
            *ImplSymbol = _BuiltinUnnamedVariables[i].implSymbol;
            break;
         }
      }

      gcmASSERT(i < _cldBuiltinUnnamedVariableCount);
   }

   gcmFOOTER_ARG("*ImplSymbol=%u", *ImplSymbol);
   return gcvSTATUS_OK;
}

clsNAME*
clGetPreScaleGlobalIDCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN clsNAME *KernelFunc
    )
{
    gceSTATUS       status = gcvSTATUS_OK;
    clsNAME*        pPreScaleGlobalId =_cldUnnamedVariable(clvBUILTIN_PRE_SCALE_GLOBAL_ID);
    clsNAME*        pGlobalId = _cldUnnamedVariable(clvBUILTIN_GLOBAL_ID);
    clsNAME*        pGlobalWorkScale = _cldUnnamedVariable(clvBUILTIN_GLOBAL_WORK_SCALE);
    clsROPERAND     globalIdOperand[1];
    clsROPERAND     scaleROperand[1];
    clsIOPERAND     iOperand[1];
    gctUINT         i;

    /* If we have already calculated the pre-scale-globalID, skip it. */
    if (cloCOMPILER_HasCalculatePreScaleGlobalId(Compiler))
    {
        return pPreScaleGlobalId;
    }

    /* Start to calculate the pre-scale-globalID. */
    gcmONERROR(clsNAME_AllocLogicalRegs(Compiler,
                                        CodeGenerator,
                                        pPreScaleGlobalId));

    gcmONERROR(clsNAME_AllocLogicalRegs(Compiler,
                                        CodeGenerator,
                                        pGlobalId));

    clsIOPERAND_Initialize(Compiler, iOperand, clmGenCodeDataType(T_UINT3), _cldUnnamedVariableRegs(clvBUILTIN_PRE_SCALE_GLOBAL_ID)->regIndex);
    clsROPERAND_InitializeReg(globalIdOperand, _cldUnnamedVariableRegs(clvBUILTIN_GLOBAL_ID));

    if (cloCOMPILER_ExtensionEnabled(Compiler, clvEXTENSION_VIV_VX))
    {
        gcmONERROR(clsNAME_AllocLogicalRegs(Compiler,
                                            CodeGenerator,
                                            pGlobalWorkScale));

        clsROPERAND_InitializeReg(scaleROperand, _cldUnnamedVariableRegs(clvBUILTIN_GLOBAL_WORK_SCALE));

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           KernelFunc->lineNo,
                                           KernelFunc->stringNo,
                                           clvOPCODE_IDIV,
                                           iOperand,
                                           globalIdOperand,
                                           scaleROperand));
    }
    /* If there is a optional KERNEL_SCALE_HINT, we also need to calcualte it. */
    else if ((KernelFunc->u.funcInfo.attrQualifier.attrFlags & clvATTR_KERNEL_SCALE_HINT)
             &&
             (KernelFunc->u.funcInfo.attrQualifier.kernelScaleHint[0] != 1 ||
              KernelFunc->u.funcInfo.attrQualifier.kernelScaleHint[1] != 1 ||
              KernelFunc->u.funcInfo.attrQualifier.kernelScaleHint[2] != 1))
    {
        cluCONSTANT_VALUE values[3];

        for (i = 0; i < 3; i++)
        {
            values[i].intValue = KernelFunc->u.funcInfo.attrQualifier.kernelScaleHint[i];
        }

        clsROPERAND_InitializeConstant(scaleROperand,
                                       clmGenCodeDataType(T_UINT3),
                                       3,
                                       values);

        gcmONERROR(clGenArithmeticExprCode(Compiler,
                                           KernelFunc->lineNo,
                                           KernelFunc->stringNo,
                                           clvOPCODE_IDIV,
                                           iOperand,
                                           globalIdOperand,
                                           scaleROperand));
    }
    else
    {
        gcmONERROR(clGenGenericCode1(Compiler,
                                     KernelFunc->lineNo,
                                     KernelFunc->stringNo,
                                     clvOPCODE_ASSIGN,
                                     iOperand,
                                     globalIdOperand));
    }

    gcmONERROR(cloCOMPILER_SetHasCalculatePreScaleGlobalId(Compiler));

OnError:
    return pPreScaleGlobalId;
}

#define clmHasPointerToAddressSpace(FuncName, AddrSpace, Has)  do { \
    clsNAME *paramName; \
    (Has) = 0; \
    FOR_EACH_DLINK_NODE(&((FuncName)->u.funcInfo.localSpace->names), clsNAME, paramName) { \
        if (paramName->type != clvPARAMETER_NAME) break; \
        if(clmDECL_IsPointerType(&paramName->decl) && \
           clGetAddrSpaceQualifier(&paramName->decl) == (AddrSpace)) { \
           (Has) = 1; \
           break; \
        } \
    } \
     } while (gcvFALSE)

gceSTATUS
clGenBaseMemoryAddressCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN clsNAME *KernelFunc
)
{
   gceSTATUS status = gcvSTATUS_OK;
   gctREG_INDEX tempRegIndex;
   clsIOPERAND addressOffset[1];
   clsROPERAND workItem[1];
   clsNAME *addressSpace;
   gctINT needMemoryBaseAddress;

   if(cloCOMPILER_IsPrivateMemoryNeeded(Compiler) ||
      cloCOMPILER_IsPrintfMemoryNeeded(Compiler)) {
      clsNAME *globalId;
      clsNAME *globalSize;
      clsIOPERAND iOperand[1];
      clsROPERAND rOperand1[1];
      clsROPERAND rOperand2[1];
      clsROPERAND globalIdOperand[1];
      clsROPERAND globalSizeOperand[1];

      globalId = clGetPreScaleGlobalIDCode(Compiler,
                                           CodeGenerator,
                                           KernelFunc);

/* Compute base address offset:
   Z * I * J + Y * I + X
   where global Id = (X, Y, Z) and
         global size = (I, J, K)  */

/* Compute: (Z, Y) * I */
      clsROPERAND_InitializeReg(globalIdOperand,
                                _cldUnnamedVariableRegs(clvBUILTIN_PRE_SCALE_GLOBAL_ID));

      clGetVectorROperandSlice(globalIdOperand,
                           1,
                           2,
                           rOperand1);

      globalSize = _cldUnnamedVariable(clvBUILTIN_GLOBAL_SIZE);
      gcmONERROR(clsNAME_AllocLogicalRegs(Compiler,
                                          CodeGenerator,
                                          globalSize));

      clsROPERAND_InitializeReg(globalSizeOperand,
                                _cldUnnamedVariableRegs(clvBUILTIN_GLOBAL_SIZE));

      tempRegIndex = clNewLocalTempRegs(Compiler, 1);
      clsIOPERAND_Initialize(Compiler, iOperand, clmGenCodeDataType(T_INT2), tempRegIndex);
      clmROPERAND_vectorComponent_GET(rOperand2, globalSizeOperand, clvCOMPONENT_X);

      gcmONERROR(clGenArithmeticExprCode(Compiler,
                                         KernelFunc->lineNo,
                                         KernelFunc->stringNo,
                                         clvOPCODE_MUL,
                                         iOperand,
                                         rOperand1,
                                         rOperand2));

/* Compute (Z * I) * J */
      clsROPERAND_InitializeUsingIOperand(rOperand1, iOperand);
      clmROPERAND_vectorComponent_GET(rOperand1, rOperand1, clvCOMPONENT_Y);
      clmROPERAND_vectorComponent_GET(rOperand2, globalSizeOperand, clvCOMPONENT_Y);
      clsIOPERAND_New(Compiler, addressOffset, clmGenCodeDataType(T_UINT));

      gcmONERROR(clGenArithmeticExprCode(Compiler,
                                         KernelFunc->lineNo,
                                         KernelFunc->stringNo,
                                         clvOPCODE_MUL,
                                         addressOffset,
                                         rOperand1,
                                         rOperand2));

/* Compute (Z * I * J) + (Y * I) */
      clsROPERAND_InitializeUsingIOperand(rOperand1, addressOffset);
      clsROPERAND_InitializeUsingIOperand(rOperand2, iOperand);
      clmROPERAND_vectorComponent_GET(rOperand2, rOperand2, clvCOMPONENT_X);
      clsIOPERAND_New(Compiler, addressOffset, clmGenCodeDataType(T_UINT));
      gcmONERROR(clGenArithmeticExprCode(Compiler,
                                         KernelFunc->lineNo,
                                         KernelFunc->stringNo,
                                         clvOPCODE_ADD,
                                         addressOffset,
                                         rOperand1,
                                         rOperand2));

/* Compute (Z * I * J) + (Y * I) + X */
      clsROPERAND_InitializeUsingIOperand(rOperand1, addressOffset);
      clmROPERAND_vectorComponent_GET(rOperand2, globalIdOperand, clvCOMPONENT_X);
      gcmONERROR(clGenArithmeticExprCode(Compiler,
                                         KernelFunc->lineNo,
                                         KernelFunc->stringNo,
                                         clvOPCODE_ADD,
                                         addressOffset,
                                         rOperand1,
                                         rOperand2));

      clsROPERAND_InitializeUsingIOperand(workItem, addressOffset);
      clResetLocalTempRegs(Compiler, workItem->u.reg.regIndex);

/* compute base address */
      if(cloCOMPILER_IsPrivateMemoryNeeded(Compiler)) {
          addressSpace = _cldUnnamedVariable(clvBUILTIN_PRIVATE_ADDRESS_SPACE);
          gcmONERROR(clsNAME_AllocLogicalRegs(Compiler,
                                              CodeGenerator,
                                              addressSpace));

          /* we don't generate lshift, but mul instead. Then we can change it to mad, which is
            needed for pattern match for generating imod for private memory base address */
          gcmONERROR(clGenScaledIndexOperand(Compiler,
                                             KernelFunc->lineNo,
                                             KernelFunc->stringNo,
                                             workItem,
                                             cloCOMPILER_GetPrivateMemoryNeeded(Compiler),
                                             gcvFALSE,
                                             rOperand2));
          if (gcmIS_ERROR(status)) return status;

          clsROPERAND_InitializeReg(rOperand1,
                                    _cldUnnamedVariableRegs(clvBUILTIN_PRIVATE_ADDRESS_SPACE));
          clsIOPERAND_Initialize(Compiler, addressOffset, clmGenCodeDataType(T_UINT), cldPrivateMemoryAddressRegIndex);
          gcmONERROR(clGenArithmeticExprCode(Compiler,
                                             KernelFunc->lineNo,
                                             KernelFunc->stringNo,
                                             clvOPCODE_ADD,
                                             addressOffset,
                                             rOperand1,
                                             rOperand2));
      }

      clResetLocalTempRegs(Compiler, workItem->u.reg.regIndex);
      if(cloCOMPILER_IsPrintfMemoryNeeded(Compiler)) {
          clsROPERAND printfBufferSize[1];
          clsIOPERAND startAddress[1];
          clsIOPERAND endAddress[1];
          clsNAME *printfVariable;

          printfVariable = _cldUnnamedVariable(clvBUILTIN_WORKITEM_PRINTF_BUFFER_SIZE);

          gcmONERROR(clsNAME_AllocLogicalRegs(Compiler,
                                              CodeGenerator,
                                              printfVariable));

          clsROPERAND_InitializeReg(printfBufferSize,
                                    _cldUnnamedVariableRegs(clvBUILTIN_WORKITEM_PRINTF_BUFFER_SIZE));
          clsIOPERAND_New(Compiler, addressOffset, clmGenCodeDataType(T_UINT));
          gcmONERROR(clGenArithmeticExprCode(Compiler,
                                             KernelFunc->lineNo,
                                             KernelFunc->stringNo,
                                             clvOPCODE_MUL,
                                             addressOffset,
                                             printfBufferSize,
                                             workItem));

          printfVariable = _cldUnnamedVariable(clvBUILTIN_PRINTF_ADDRESS);

          gcmONERROR(clsNAME_AllocLogicalRegs(Compiler,
                                              CodeGenerator,
                                              printfVariable));

          clsROPERAND_InitializeReg(rOperand1,
                                    _cldUnnamedVariableRegs(clvBUILTIN_PRINTF_ADDRESS));
          clsROPERAND_InitializeUsingIOperand(rOperand2, addressOffset);
          clsIOPERAND_Initialize(Compiler, startAddress, clmGenCodeDataType(T_UINT), cldPrintfStartMemoryAddressRegIndex);
          gcmONERROR(clGenArithmeticExprCode(Compiler,
                                             KernelFunc->lineNo,
                                             KernelFunc->stringNo,
                                             clvOPCODE_ADD,
                                             startAddress,
                                             rOperand1,
                                             rOperand2));

          clsROPERAND_InitializeUsingIOperand(rOperand1, startAddress);
          clsIOPERAND_Initialize(Compiler, endAddress, clmGenCodeDataType(T_UINT), cldPrintfEndMemoryAddressRegIndex);
          gcmONERROR(clGenArithmeticExprCode(Compiler,
                                             KernelFunc->lineNo,
                                             KernelFunc->stringNo,
                                             clvOPCODE_ADD,
                                             endAddress,
                                             rOperand1,
                                             printfBufferSize));
      }
   }

   clmHasPointerToAddressSpace(KernelFunc, clvQUALIFIER_LOCAL, needMemoryBaseAddress);

   clResetLocalTempRegs(Compiler, 0);
   if(cloCOMPILER_IsLocalMemoryNeeded(Compiler) ||
      needMemoryBaseAddress) {
      clsNAME *groupId;
      clsNAME *numGroups;
      clsIOPERAND iOperand[1];
      clsROPERAND rOperand1[1];
      clsROPERAND rOperand2[1];
      clsROPERAND groupIdOperand[1];
      clsROPERAND numGroupsOperand[1];
      clsNAME *funcName;

      gcmASSERT(CodeGenerator->currentFuncDefContext.isKernel);
      funcName = CodeGenerator->currentFuncDefContext.funcBody->funcName;

      if(cloCOMPILER_IsLocalMemoryNeeded(Compiler)) {
         addressSpace = _cldUnnamedVariable(clvBUILTIN_LOCAL_ADDRESS_SPACE);
         /* Allocate all logical registers */
         gcmONERROR(clsNAME_AllocLogicalRegs(Compiler,
                                             CodeGenerator,
                                             addressSpace));
      }

      if(needMemoryBaseAddress) {
         addressSpace = _cldUnnamedVariable(clvBUILTIN_ARG_LOCAL_MEM_SIZE);
         /* Allocate all logical registers */
         gcmONERROR(clsNAME_AllocLogicalRegs(Compiler,
                                             CodeGenerator,
                                             addressSpace));
      }

/* Compute base address offset:
   Z * I * J + Y * I + X
   where group Id = (X, Y, Z) and
         group size = (I, J, K)  */

      groupId = _cldUnnamedVariable(clvBUILTIN_GROUP_ID);
      gcmONERROR(clsNAME_AllocLogicalRegs(Compiler,
                                          CodeGenerator,
                                          groupId));

/* Compute: (Z, Y) * I */
      clsROPERAND_InitializeReg(groupIdOperand,
                                _cldUnnamedVariableRegs(clvBUILTIN_GROUP_ID));

      clGetVectorROperandSlice(groupIdOperand,
                           1,
                           2,
                           rOperand1);
      numGroups = _cldUnnamedVariable(clvBUILTIN_NUM_GROUPS);
      gcmONERROR(clsNAME_AllocLogicalRegs(Compiler,
                                          CodeGenerator,
                                          numGroups));

      clsROPERAND_InitializeReg(numGroupsOperand,
                                _cldUnnamedVariableRegs(clvBUILTIN_NUM_GROUPS));

      tempRegIndex = clNewLocalTempRegs(Compiler, 1);
      clsIOPERAND_Initialize(Compiler, iOperand, clmGenCodeDataType(T_UINT2), tempRegIndex);
      clmROPERAND_vectorComponent_GET(rOperand2, numGroupsOperand, clvCOMPONENT_X);

      gcmONERROR(clGenArithmeticExprCode(Compiler,
                                         KernelFunc->lineNo,
                                         KernelFunc->stringNo,
                                         clvOPCODE_MUL,
                                         iOperand,
                                         rOperand1,
                                         rOperand2));

/* Compute (Z * I) * J */
      clsROPERAND_InitializeUsingIOperand(rOperand1, iOperand);
      clmROPERAND_vectorComponent_GET(rOperand1, rOperand1, clvCOMPONENT_Y);
      clmROPERAND_vectorComponent_GET(rOperand2, numGroupsOperand, clvCOMPONENT_Y);
      clsIOPERAND_New(Compiler, addressOffset, clmGenCodeDataType(T_UINT));
      gcmONERROR(clGenArithmeticExprCode(Compiler,
                                         KernelFunc->lineNo,
                                         KernelFunc->stringNo,
                                         clvOPCODE_MUL,
                                         addressOffset,
                                         rOperand1,
                                         rOperand2));

/* Compute (Z * I * J) + (Y * I) */
      clsROPERAND_InitializeUsingIOperand(rOperand1, addressOffset);
      clsROPERAND_InitializeUsingIOperand(rOperand2, iOperand);
      clmROPERAND_vectorComponent_GET(rOperand2, rOperand2, clvCOMPONENT_X);
      clsIOPERAND_Initialize(Compiler, addressOffset, clmGenCodeDataType(T_UINT), cldLocalMemoryAddressRegIndex);
      gcmONERROR(clGenArithmeticExprCode(Compiler,
                                         KernelFunc->lineNo,
                                         KernelFunc->stringNo,
                                         clvOPCODE_ADD,
                                         addressOffset,
                                         rOperand1,
                                         rOperand2));

/* Compute (Z * I * J) + (Y * I) + X */
      clsROPERAND_InitializeUsingIOperand(rOperand1, addressOffset);
      clmROPERAND_vectorComponent_GET(rOperand2, groupIdOperand, clvCOMPONENT_X);
      gcmONERROR(clGenArithmeticExprCode(Compiler,
                                         KernelFunc->lineNo,
                                         KernelFunc->stringNo,
                                         clvOPCODE_ADD,
                                         addressOffset,
                                         rOperand1,
                                         rOperand2));

      /* Save the local memory address reg index for the local parameter.*/
      if (needMemoryBaseAddress)
      {
          clsLOPERAND lOperand[1];

          clsIOPERAND_Initialize(Compiler, addressOffset, clmGenCodeDataType(T_UINT), cldParmLocalMemoryAddressRegIndex);
          clsLOPERAND_InitializeUsingIOperand(lOperand, addressOffset);

          clsIOPERAND_Initialize(Compiler, addressOffset, clmGenCodeDataType(T_UINT), cldLocalMemoryAddressRegIndex);
          clsROPERAND_InitializeUsingIOperand(rOperand1, addressOffset);
          gcmONERROR(clGenAssignCode(Compiler,
                                     KernelFunc->lineNo,
                                     KernelFunc->stringNo,
                                     lOperand,
                                     rOperand1));
      }

      /* Add a variable to hold localMemoryAddress. */
      {
          clsGEN_CODE_DATA_TYPE varType;

          varType.elementType = clvTYPE_UINT;
          varType.matrixSize.rowCount = 0;
          varType.matrixSize.columnCount = 0;

          gcmONERROR(clNewVariable(Compiler,
                                   0,
                                   0,
                                   _sldLocalMemoryAddressName,
                                   clvQUALIFIER_NONE,
                                   clvQUALIFIER_NONE,
                                   clvSTORAGE_QUALIFIER_NONE,
                                   varType,
                                   1,
                                   gcvNULL,
                                   gcvFALSE,
                                   cldLocalMemoryAddressRegIndex,
                                   gcvNULL));
      }

/* compute base address */
/* delay computation of final base local memory addresses in cloIR_GenFuncCall() if kernel arg has pointer
  to local address space */

   }

   clResetLocalTempRegs(Compiler, 0);
   if(cloCOMPILER_IsConstantMemoryNeeded(Compiler)) {
      clsLOPERAND lOperand[1];
      clsROPERAND rOperand[1];

      addressSpace = _cldUnnamedVariable(clvBUILTIN_CONSTANT_ADDRESS_SPACE);
/* Allocate all logical registers */
      gcmONERROR(clsNAME_AllocLogicalRegs(Compiler,
                                          CodeGenerator,
                                          addressSpace));
      clsLOPERAND_InitializeTempReg(Compiler,
                                    lOperand,
                                    clvQUALIFIER_NONE,
                                    clmGenCodeDataType(T_UINT),
                                    cldConstantMemoryAddressRegIndex);
      clsROPERAND_InitializeReg(rOperand,
                                _cldUnnamedVariableRegs(clvBUILTIN_CONSTANT_ADDRESS_SPACE));
      gcmONERROR(clGenAssignCode(Compiler,
                                 KernelFunc->lineNo,
                                 KernelFunc->stringNo,
                                 lOperand,
                                 rOperand));
   }

OnError:
   return status;
}



static gceSTATUS
_GenNativeDivCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* div result, x/y */
    status = clGenGenericCode2(Compiler,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   clvOPCODE_DIV,
                   IOperand,
                   &OperandsParameters[0].rOperands[0],
                   &OperandsParameters[1].rOperands[0]);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;

}

static gceSTATUS
_GenNativeAddLoCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* div result, x/y */
    status = clGenGenericCode2(Compiler,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   clvOPCODE_ADDLO,
                   IOperand,
                   &OperandsParameters[0].rOperands[0],
                   &OperandsParameters[1].rOperands[0]);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;

}

static gceSTATUS
_GenNativeMulLoCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* div result, x/y */
    status = clGenGenericCode2(Compiler,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   clvOPCODE_MULLO,
                   IOperand,
                   &OperandsParameters[0].rOperands[0],
                   &OperandsParameters[1].rOperands[0]);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;

}

static gceSTATUS
_GenNativeAddRtzCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = clGenGenericCode2(Compiler,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   clvOPCODE_ADD_RTZ,
                   IOperand,
                   &OperandsParameters[0].rOperands[0],
                   &OperandsParameters[1].rOperands[0]);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;

}

static gceSTATUS
_GenNativeSubRtzCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = clGenGenericCode2(Compiler,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   clvOPCODE_SUB_RTZ,
                   IOperand,
                   &OperandsParameters[0].rOperands[0],
                   &OperandsParameters[1].rOperands[0]);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;

}

static gceSTATUS
_GenNativeMulRtzCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = clGenGenericCode2(Compiler,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   clvOPCODE_MUL_RTZ,
                   IOperand,
                   &OperandsParameters[0].rOperands[0],
                   &OperandsParameters[1].rOperands[0]);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;

}

static gceSTATUS
_GenCMADCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    gctBOOL   supportComplex = gcGetHWCaps()->hwFeatureFlags.supportComplex;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (!supportComplex)
    {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        0,
                        0,
                        clvREPORT_FATAL_ERROR,
                        "_viv_cmad is not supported for the Hardware chip Model=%X Revision=%X",
                        gcGetHWCaps()->chipModel, gcGetHWCaps()->chipRevision));
        return gcvSTATUS_NOT_SUPPORTED;
    }

    status = clGenArithmeticExprCode(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_CMAD,
                        IOperand,
                        &OperandsParameters[0].rOperands[0],
                        &OperandsParameters[1].rOperands[0]);
    if (gcmIS_ERROR(status)) return status;


    status = clGenArithmeticExprCode(Compiler,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   clvOPCODE_CMAD,
                   IOperand,
                   &OperandsParameters[0].rOperands[0],
                   &OperandsParameters[2].rOperands[0]);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;

}

static gceSTATUS
_GenCMADCJCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    gctBOOL   supportComplex = gcGetHWCaps()->hwFeatureFlags.supportComplex;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (!supportComplex)
    {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        0,
                        0,
                        clvREPORT_FATAL_ERROR,
                        "_viv_cmadcj is not supported for the Hardware chip Model=%X Revision=%X",
                        gcGetHWCaps()->chipModel, gcGetHWCaps()->chipRevision));
        return gcvSTATUS_NOT_SUPPORTED;
    }

    status = clGenArithmeticExprCode(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_CMADCJ,
                        IOperand,
                        &OperandsParameters[0].rOperands[0],
                        &OperandsParameters[1].rOperands[0]);
    if (gcmIS_ERROR(status)) return status;


    status = clGenArithmeticExprCode(Compiler,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   clvOPCODE_CMADCJ,
                   IOperand,
                   &OperandsParameters[0].rOperands[0],
                   &OperandsParameters[2].rOperands[0]);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;

}


static gceSTATUS
_GenCMULCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    gctBOOL   supportComplex = gcGetHWCaps()->hwFeatureFlags.supportComplex;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (!supportComplex)
    {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        0,
                        0,
                        clvREPORT_FATAL_ERROR,
                        "_viv_cmul is not supported for the Hardware chip Model=%X Revision=%X",
                        gcGetHWCaps()->chipModel, gcGetHWCaps()->chipRevision));
        return gcvSTATUS_NOT_SUPPORTED;
    }

    status = clGenArithmeticExprCode(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_CMUL,
                        IOperand,
                        &OperandsParameters[0].rOperands[0],
                        &OperandsParameters[1].rOperands[0]);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenCMULCJCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    gctBOOL   supportComplex = gcGetHWCaps()->hwFeatureFlags.supportComplex;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (!supportComplex)
    {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        0,
                        0,
                        clvREPORT_FATAL_ERROR,
                        "_viv_cmulcj is not supported for the Hardware chip Model=%X Revision=%X",
                        gcGetHWCaps()->chipModel, gcGetHWCaps()->chipRevision));
        return gcvSTATUS_NOT_SUPPORTED;
    }

    status = clGenArithmeticExprCode(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_CMULCJ,
                        IOperand,
                        &OperandsParameters[0].rOperands[0],
                        &OperandsParameters[1].rOperands[0]);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenCADDCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    gctBOOL   supportComplex = gcGetHWCaps()->hwFeatureFlags.supportComplex;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (!supportComplex)
    {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        0,
                        0,
                        clvREPORT_FATAL_ERROR,
                        "_viv_cadd is not supported for the Hardware chip Model=%X Revision=%X",
                        gcGetHWCaps()->chipModel, gcGetHWCaps()->chipRevision));
        return gcvSTATUS_NOT_SUPPORTED;
    }

    status = clGenArithmeticExprCode(Compiler,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   clvOPCODE_ADD,
                   IOperand,
                   &OperandsParameters[0].rOperands[0],
                   &OperandsParameters[1].rOperands[0]);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenCADDCJCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    gctBOOL   supportComplex = gcGetHWCaps()->hwFeatureFlags.supportComplex;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (!supportComplex)
    {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        0,
                        0,
                        clvREPORT_FATAL_ERROR,
                        "_viv_caddcj is not supported for the Hardware chip Model=%X Revision=%X",
                        gcGetHWCaps()->chipModel, gcGetHWCaps()->chipRevision));
        return gcvSTATUS_NOT_SUPPORTED;
    }

    status = clGenArithmeticExprCode(Compiler,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   clvOPCODE_CADDCJ,
                   IOperand,
                   &OperandsParameters[0].rOperands[0],
                   &OperandsParameters[1].rOperands[0]);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenCSUBCJCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    gctBOOL   supportComplex = gcGetHWCaps()->hwFeatureFlags.supportComplex;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (!supportComplex)
    {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        0,
                        0,
                        clvREPORT_FATAL_ERROR,
                        "_viv_csubcj is not supported for the Hardware chip Model=%X Revision=%X",
                        gcGetHWCaps()->chipModel, gcGetHWCaps()->chipRevision));
        return gcvSTATUS_NOT_SUPPORTED;
    }

    status = clGenArithmeticExprCode(Compiler,
                   PolynaryExpr->exprBase.base.lineNo,
                   PolynaryExpr->exprBase.base.stringNo,
                   clvOPCODE_CSUBCJ,
                   IOperand,
                   &OperandsParameters[0].rOperands[0],
                   &OperandsParameters[1].rOperands[0]);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenCCJCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    gctBOOL   supportComplex = gcGetHWCaps()->hwFeatureFlags.supportComplex;

    if (!supportComplex)
    {
        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                        0,
                        0,
                        clvREPORT_FATAL_ERROR,
                        "_viv_ccj is not supported for the Hardware chip Model=%X Revision=%X",
                        gcGetHWCaps()->chipModel, gcGetHWCaps()->chipRevision));
        return gcvSTATUS_NOT_SUPPORTED;
    }
    status = clGenGenericCode1(Compiler,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo,
                       clvOPCODE_CONJ,
                       IOperand,
                       &OperandsParameters[0].rOperands[0]);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

gceSTATUS
clGenInverseCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    clsROPERAND constantROperand;
    clsROPERAND    intermROperands[7];
    clsIOPERAND intermIOperands[7];
    int i;


    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    clsROPERAND_InitializeFloatOrVecOrMatConstant(&constantROperand,
                                                clmGenCodeDataType(T_FLOAT),
                                                (gctFLOAT)1.0);
    for(i = 0; i<7; i++){
        clsIOPERAND_New(Compiler, &intermIOperands[i], clmGenCodeDataType(T_FLOAT));
        clsROPERAND_InitializeUsingIOperand(&intermROperands[i], &intermIOperands[i]);
    }


    /* rcp result, x */

    status = clGenGenericCode1(Compiler,
                LineNo,
                StringNo,
                clvOPCODE_INVERSE,
                &intermIOperands[0],
                &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    {   /*If r0 is too small, N-R will fail since r0*(1-r0*x) = 0 */
        clsSELECTION_CONTEXT selectContextSmallRcp;
        clsROPERAND oneTo24ROperand, twenty5To1ROperand;

        /*2^24,*/
        clsROPERAND_InitializeFloatOrVecOrMatConstant(&oneTo24ROperand,
                                            clmGenCodeDataType(T_FLOAT),
                                            (float)(1<<24) );

        /*1/2^25,*/
        clsROPERAND_InitializeFloatOrVecOrMatConstant(&twenty5To1ROperand,
                                            clmGenCodeDataType(T_FLOAT),
                                            (float)(1.1920929e-007)/2.0f );
        status = clDefineSelectionBegin(Compiler,
                                    CodeGenerator,
                                    gcvTRUE,
                                    &selectContextSmallRcp);

        if (gcmIS_ERROR(status)) return status;

        status = clGenGenericCode1(Compiler,
                       LineNo,
                       StringNo,
                       clvOPCODE_ABS,
                       &intermIOperands[1],
                       &intermROperands[0]);

        /* Use integer comparison, avoid inf, Nan case */
        intermROperands[1].dataType.elementType = clvTYPE_INT;
        status = clGenSelectionCompareConditionCode(Compiler,
                                CodeGenerator,
                                &selectContextSmallRcp,
                                LineNo,
                                StringNo,
                                clvCONDITION_LESS_THAN,
                                &intermROperands[1],
                                &twenty5To1ROperand);
        if (gcmIS_ERROR(status)) return status;
        intermROperands[1].dataType.elementType = clvTYPE_FLOAT;
        status = clDefineSelectionTrueOperandBegin(Compiler,
                                            CodeGenerator,
                                            &selectContextSmallRcp);
        clGenGenericCode1(Compiler,
                LineNo,
                StringNo,
                clvOPCODE_ASSIGN,
                &intermIOperands[5],
                &oneTo24ROperand);

        clGenGenericCode1(Compiler,
                LineNo,
                StringNo,
                clvOPCODE_ASSIGN,
                &intermIOperands[6],
                &twenty5To1ROperand);
        status = clDefineSelectionTrueOperandEnd(Compiler,
                                            LineNo,
                                            StringNo,
                                            CodeGenerator,
                                            &selectContextSmallRcp, 0);
        /*Normal case, don't need enlarge, set enlarge factor = 1.0 */
        status = clDefineSelectionFalseOperandBegin(Compiler,
                                            CodeGenerator,
                                            &selectContextSmallRcp);
        clGenGenericCode1(
                            Compiler,
                            LineNo,
                            StringNo,
                            clvOPCODE_ASSIGN,
                            &intermIOperands[5],
                            &constantROperand);

        clGenGenericCode1(
                            Compiler,
                            LineNo,
                            StringNo,
                            clvOPCODE_ASSIGN,
                            &intermIOperands[6],
                            &constantROperand);

        status = clDefineSelectionFalseOperandEnd(Compiler,
                                            CodeGenerator,
                                            &selectContextSmallRcp);
        status = clDefineSelectionEnd(Compiler,
                                            CodeGenerator,
                                            &selectContextSmallRcp);
        if (gcmIS_ERROR(status)) return status;

    }


    /* mul x, 1/x */

    status = clGenArithmeticExprCode(Compiler,
                                    LineNo,
                                    StringNo,
                                    clvOPCODE_MUL,
                                    &intermIOperands[1],
                                    &intermROperands[0],
                                    &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    /* sub from 1, result */

    status = clGenArithmeticExprCode(Compiler,
                                    LineNo,
                                    StringNo,
                                    clvOPCODE_SUB,
                                    &intermIOperands[2],
                                    &constantROperand,
                                    &intermROperands[1]);

    if (gcmIS_ERROR(status)) return status;


    /*Enlarge r0 = 1/x, otherwise (1-r0*x)*r0 = 0 */
    status = clGenArithmeticExprCode(Compiler,
                                    LineNo,
                                    StringNo,
                                    clvOPCODE_MUL,
                                    &intermIOperands[0],
                                    &intermROperands[5],
                                    &intermROperands[0]);
    /* mul from result, 1/x */
    status = clGenArithmeticExprCode(Compiler,
                                    LineNo,
                                    StringNo,
                                    clvOPCODE_MUL,
                                    &intermIOperands[3],
                                    &intermROperands[2],
                                    &intermROperands[0]);

    if (gcmIS_ERROR(status)) return status;

    /* add from result, 1/x */

    { /* If the N-R generated Nan from 0.0*Inf, don't add back */
        clsROPERAND mantissaROperand, intOneROperand;
        clsROPERAND    intROperand;
        clsIOPERAND intIOperand;
        clsROPERAND_InitializeIntOrIVecConstant(&mantissaROperand,
                   clmGenCodeDataType(T_INT),
                   (gctUINT)0x007fffff);
        clsROPERAND_InitializeIntOrIVecConstant(&intOneROperand,
                   clmGenCodeDataType(T_INT),
                   (gctUINT)1);

        clsIOPERAND_New(Compiler, &intIOperand, clmGenCodeDataType(T_INT));
        clsROPERAND_InitializeUsingIOperand(&intROperand, &intIOperand);

        /*Get Mantissa */
        status = clGenBitwiseExprCode(Compiler,
                LineNo,
                StringNo,
                clvOPCODE_AND_BITWISE,
                &intIOperand,
                &mantissaROperand,
                &intermROperands[0]);
        /* if mantissa == 0, get 0, else get 1      */
        /* if mantissa = 0, Hw rcp good enough, also exclude Nan from 0*inf */
        status = clGenGenericCode2(
                        Compiler,
                        LineNo,
                        StringNo,
                        clvOPCODE_MIN,
                        &intIOperand,
                        &intOneROperand,
                        &intROperand);

        /* fix point multiple */
        intermROperands[3].dataType.elementType = clvTYPE_INT;
        intermIOperands[3].dataType.elementType = clvTYPE_INT;

        status = clGenArithmeticExprCode(Compiler,
                                        LineNo,
                                        StringNo,
                                        clvOPCODE_MUL,
                                        &intermIOperands[3],
                                        &intROperand,
                                        &intermROperands[3]);

        intermROperands[3].dataType.elementType = clvTYPE_FLOAT;
        intermIOperands[3].dataType.elementType = clvTYPE_FLOAT;

        status = clGenArithmeticExprCode(Compiler,
                                        LineNo,
                                        StringNo,
                                        clvOPCODE_ADD,
                                        &intermIOperands[1],
                                        &intermROperands[3],
                                        &intermROperands[0]);

        if (gcmIS_ERROR(status)) return status;
        /*Inverse Enlarge r0 = 1/x, otherwise (1-r0*x)*r0 = 0 */
        status = clGenArithmeticExprCode(Compiler,
                                        LineNo,
                                        StringNo,
                                        clvOPCODE_MUL,
                                        IOperand,
                                        &intermROperands[6],
                                        &intermROperands[1]);

    }


    return gcvSTATUS_OK;
}

static gceSTATUS
_GenInverseCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    if(clmGEN_CODE_IsScalarDataType(OperandsParameters[0].dataTypes[0].def)){
        status = clGenInverseCode(Compiler,
                    CodeGenerator,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    OperandCount,
                    OperandsParameters,
                    IOperand);
        if (gcmIS_ERROR(status)) return status;
    }
    else{
        gctUINT8 vectorSize = clmGEN_CODE_vectorSize_GET(OperandsParameters[0].dataTypes[0].def);
        gctUINT8 i;
        clsROPERAND    tempROperand, copyROperand;
        clsIOPERAND tempIOperand;
        clsLOPERAND tempLOperand, destLOperand;
        gctUINT8 copy_matrixSize_rowCount = OperandsParameters[0].dataTypes[0].def.matrixSize.rowCount;
        gctUINT8 copy_matrixSize_columnCount = OperandsParameters[0].dataTypes[0].def.matrixSize.columnCount;

        copyROperand = OperandsParameters[0].rOperands[0];

        /*force to scalar */
        OperandsParameters[0].dataTypes[0].def.matrixSize.rowCount = 0;
        OperandsParameters[0].dataTypes[0].def.matrixSize.columnCount = 0;

        clsLOPERAND_InitializeUsingIOperand(&tempLOperand, IOperand);
        clsIOPERAND_New(Compiler, &tempIOperand, clmGenCodeDataType(T_FLOAT));

        for(i = 0; i<vectorSize; i++){
            clmROPERAND_vectorComponent_GET(&tempROperand, &copyROperand, i);
            OperandsParameters[0].rOperands[0] = tempROperand;

            status = _GenInverseCode(Compiler,
                            CodeGenerator,
                            PolynaryExpr,
                            OperandCount,
                            OperandsParameters,
                            &tempIOperand);
            if (gcmIS_ERROR(status)) return status;

            clsROPERAND_InitializeUsingIOperand(&tempROperand, &tempIOperand);
            clmLOPERAND_vectorComponent_GET(&destLOperand, &tempLOperand, i);
            status = clGenAssignCode(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                    &destLOperand,
                    &tempROperand);
            if (gcmIS_ERROR(status)) return status;

        }
        OperandsParameters[0].dataTypes[0].def.matrixSize.rowCount = copy_matrixSize_rowCount;
        OperandsParameters[0].dataTypes[0].def.matrixSize.columnCount = copy_matrixSize_columnCount;
        OperandsParameters[0].rOperands[0] = copyROperand;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
clGenDivCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    clsSELECTION_CONTEXT selectContextBigY;
    clsROPERAND    intermROperands[4], tempROperand;
    clsIOPERAND intermIOperands[4];
    clsROPERAND unsignROperand, bigValueROperand, cnst_25ROperand, cnst1ROperand, infROperand, neg1ROperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    clsIOPERAND_New(Compiler, &intermIOperands[0], clmGenCodeDataType(T_FLOAT));
    clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);
    clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_FLOAT));
    clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);
    clsIOPERAND_New(Compiler, &intermIOperands[2], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);
    clsIOPERAND_New(Compiler, &intermIOperands[3], clmGenCodeDataType(T_FLOAT));
    clsROPERAND_InitializeUsingIOperand(&intermROperands[3], &intermIOperands[3]);

    clsROPERAND_InitializeIntOrIVecConstant(&unsignROperand,
               clmGenCodeDataType(T_UINT),
               (gctUINT)0x7fffffff);
    clsROPERAND_InitializeIntOrIVecConstant(&bigValueROperand,
               clmGenCodeDataType(T_UINT),
               (gctUINT)0x7e800000);
    clsROPERAND_InitializeIntOrIVecConstant(&infROperand,
               clmGenCodeDataType(T_UINT),
               (gctUINT)0x7f800000);
    clsROPERAND_InitializeIntOrIVecConstant(&neg1ROperand,
               clmGenCodeDataType(T_UINT),
               (gctUINT)0xffffffff);

    clsROPERAND_InitializeFloatOrVecOrMatConstant(&cnst_25ROperand,
                                                clmGenCodeDataType(T_FLOAT),
                                                (gctFLOAT)0.25f);

    clsROPERAND_InitializeFloatOrVecOrMatConstant(&cnst1ROperand,
                                                clmGenCodeDataType(T_FLOAT),
                                                (gctFLOAT)1.0f);
    tempROperand = OperandsParameters[1].rOperands[0];
    /*Get the unsigned part */
    status = clGenBitwiseExprCode(Compiler,
                                LineNo,
                                StringNo,
                                clvOPCODE_AND_BITWISE,
                                &intermIOperands[2],
                                &unsignROperand,
                                &OperandsParameters[1].rOperands[0]);

    status = clDefineSelectionBegin(Compiler,
                                    CodeGenerator,
                                    gcvTRUE,
                                    &selectContextBigY);
    if (gcmIS_ERROR(status)) return status;

    /* |Y| > 0X7E800000? 1/Y = 0, SO WE CALCULATE 2/Y */
    status = clGenSelectionCompareConditionCode(Compiler,
                                                CodeGenerator,
                                                &selectContextBigY,
                                                LineNo,
                                                StringNo,
                                                clvCONDITION_GREATER_THAN,
                                                &intermROperands[2],
                                                &bigValueROperand);

    if (gcmIS_ERROR(status)) return status;
    status = clDefineSelectionTrueOperandBegin(Compiler,
                                                CodeGenerator,
                                                &selectContextBigY);
    if (gcmIS_ERROR(status)) return status;


    /* y/4, so  */
    status = clGenArithmeticExprCode(Compiler,
                    LineNo,
                    StringNo,
                    clvOPCODE_MUL,
                    &intermIOperands[0],
                    &cnst_25ROperand,
                    &OperandsParameters[1].rOperands[0]);
    /*Final result should multiple by 0.25 */
    status = clGenGenericCode1(Compiler,
                LineNo,
                StringNo,
                clvOPCODE_ASSIGN,
                &intermIOperands[1],
                &cnst_25ROperand);

    status = clDefineSelectionTrueOperandEnd(Compiler,
                                            LineNo,
                                            StringNo,
                                            CodeGenerator,
                                            &selectContextBigY,
                                            0);
    if (gcmIS_ERROR(status)) return status;
    status = clDefineSelectionFalseOperandBegin(Compiler,
                                            CodeGenerator,
                                            &selectContextBigY);
    if (gcmIS_ERROR(status)) return status;
    status = clGenArithmeticExprCode(Compiler,
                    LineNo,
                    StringNo,
                    clvOPCODE_MUL,
                    &intermIOperands[0],
                    &cnst1ROperand,
                    &OperandsParameters[1].rOperands[0]);
    /*Normal Y, Final result should multiple by 1.0 */
    status = clGenGenericCode1(Compiler,
                LineNo,
                StringNo,
                clvOPCODE_ASSIGN,
                &intermIOperands[1],
                &cnst1ROperand);

    status = clDefineSelectionFalseOperandEnd(Compiler,
                                            CodeGenerator,
                                            &selectContextBigY);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionEnd(Compiler,
                                CodeGenerator,
                                &selectContextBigY);
    if (gcmIS_ERROR(status)) return status;

    /* rcp result, y */
    OperandsParameters[1].rOperands[0] = intermROperands[0];
    status = clGenInverseCode(Compiler,
                  CodeGenerator,
                  LineNo,
                  StringNo,
                  1,
                  &OperandsParameters[1],
                  &intermIOperands[0]);
    if (gcmIS_ERROR(status)) return status;

    /*Recover the orginal rOperands */
    OperandsParameters[1].rOperands[0] = tempROperand;
    /*Multiply to x */
    status = clGenArithmeticExprCode(Compiler,
                    LineNo,
                    StringNo,
                    clvOPCODE_MUL,
                    &intermIOperands[3],
                    &intermROperands[0],
                    &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    status = clGenArithmeticExprCode(Compiler,
                    LineNo,
                    StringNo,
                    clvOPCODE_MUL,
                    IOperand,
                    &intermROperands[3],
                    &intermROperands[1]);
    if (gcmIS_ERROR(status)) return status;

    /*perfect round to zero. If both 1/y and x are finite, x/y should be finite */
    if ((CodeGenerator->chipModel <= gcv2100 && CodeGenerator->chipRevision <= 0x5130) ||
        (CodeGenerator->chipModel == gcv4000 &&
         (CodeGenerator->chipRevision == 0x5208 || CodeGenerator->chipRevision == 0x5222
            || CodeGenerator->chipRevision == 0x4633)))
    {
        clsROPERAND_InitializeUsingIOperand(&tempROperand, IOperand);
        status = clGenGenericCode1(Compiler,
                       LineNo,
                       StringNo,
                       clvOPCODE_ABS,
                       &intermIOperands[1],
                       &intermROperands[0]);

        status = clGenGenericCode1(Compiler,
                       LineNo,
                       StringNo,
                       clvOPCODE_ABS,
                       &intermIOperands[0],
                       &OperandsParameters[0].rOperands[0]);

        status = clGenGenericCode2(Compiler,
                    LineNo,
                    StringNo,
                    clvOPCODE_MAX,
                    &intermIOperands[3],
                    &intermROperands[1],
                    &intermROperands[0]);
        /* compare max(x, 1/y) with inf*/
        clmGEN_CODE_IF(Compiler,
                     CodeGenerator,
                     LineNo,
                     StringNo,
                     &intermROperands[3],
                     clvCONDITION_LESS_THAN,
                     &infROperand);

        /*Get abs(x/y) */
        status = clGenGenericCode1(Compiler,
                       LineNo,
                       StringNo,
                       clvOPCODE_ABS,
                       &intermIOperands[1],
                       &tempROperand);

        clmGEN_CODE_IF(Compiler,
                     CodeGenerator,
                     LineNo,
                     StringNo,
                     &intermROperands[1],
                     clvCONDITION_EQUAL,
                     &infROperand);
        /*abs(x/y) == inf, should be max/min of float, we use Inf-1 */
        IOperand->dataType.elementType = clvTYPE_UINT;
        status = clGenArithmeticExprCode(Compiler,
                    LineNo,
                    StringNo,
                    clvOPCODE_ADD,
                    IOperand,
                    &tempROperand,
                    &neg1ROperand);
        IOperand->dataType.elementType = clvTYPE_FLOAT;
        clmGEN_CODE_ELSE(Compiler,
                           CodeGenerator,
                           LineNo,
                           StringNo);

       clmGEN_CODE_ENDIF(Compiler,
                         CodeGenerator,
                         LineNo,
                         StringNo);


       clmGEN_CODE_ELSE(Compiler,
                           CodeGenerator,
                           LineNo,
                           StringNo);

       clmGEN_CODE_ENDIF(Compiler,
                         CodeGenerator,
                         LineNo,
                         StringNo);

    }
OnError:
    return status;
}

static gceSTATUS
_GenDivideCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;

#if FULL_PROFILE_TEST
    if (CodeGenerator->hasNEW_SIN_COS_LOG_DIV)
    {
 /*       return _GenNativeDivCode(
                        Compiler,
                        CodeGenerator,
                        PolynaryExpr,
                        OperandCount,
                        OperandsParameters,
                        IOperand
                        );
  */

        /*Since current chip doesn't handle Inf/Inf well, we have to pactch */
        /*x/y = y >= 2^20? x/y: ( x*(4/y) ) *0.25  */
        clsIOPERAND intermIOperands[3];
        clsROPERAND intermROperands[3];
        clsROPERAND infROperand, unsignedROperand, zeroROperand, twoPow20ROperand, f4ROperand, f_25ROperand;

        clsROPERAND_InitializeIntOrIVecConstant(&infROperand,
                                            clmGenCodeDataType(T_UINT),
                                            0x7f800000);
        clsROPERAND_InitializeIntOrIVecConstant(&unsignedROperand,
                                            clmGenCodeDataType(T_UINT),
                                            0x7fffffff);

        clsROPERAND_InitializeIntOrIVecConstant(&twoPow20ROperand,
                                            clmGenCodeDataType(T_UINT),
                                            (0x3f800000 + (20<<23)));

        /*Intent to set a denorm number as zero, otherwise multiply zero optimized to move zero */
        clsROPERAND_InitializeFloatOrVecOrMatConstant(&zeroROperand,
                                            clmGenCodeDataType(T_FLOAT),
                                            (gctFLOAT)4.591774808e-041);

        clsROPERAND_InitializeFloatOrVecOrMatConstant(&f4ROperand,
                                            clmGenCodeDataType(T_FLOAT),
                                            (gctFLOAT)4.0);

        clsROPERAND_InitializeFloatOrVecOrMatConstant(&f_25ROperand,
                                            clmGenCodeDataType(T_FLOAT),
                                            (gctFLOAT)0.250);


        clsIOPERAND_New(Compiler, &intermIOperands[0], clmGenCodeDataType(T_FLOAT));
        clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);

        clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_UINT));
        clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

        clsIOPERAND_New(Compiler, &intermIOperands[2], clmGenCodeDataType(T_FLOAT));
        clsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);


        status = clGenBitwiseExprCode(Compiler,
                PolynaryExpr->exprBase.base.lineNo,
                PolynaryExpr->exprBase.base.stringNo,
                clvOPCODE_AND_BITWISE,
                &intermIOperands[1],
                &unsignedROperand,
                &OperandsParameters[1].rOperands[0]);

        clmGEN_CODE_IF(Compiler,
             CodeGenerator,
             PolynaryExpr->exprBase.base.lineNo,
             PolynaryExpr->exprBase.base.stringNo,
             &intermROperands[1],
             clvCONDITION_EQUAL,
             &infROperand);

        status = clGenGenericCode2(Compiler,
                           PolynaryExpr->exprBase.base.lineNo,
                           PolynaryExpr->exprBase.base.stringNo,
                           clvOPCODE_MUL,
                           IOperand,
                           &zeroROperand,
                           &OperandsParameters[0].rOperands[0]);


        clmGEN_CODE_ELSE(Compiler,
                           CodeGenerator,
                           PolynaryExpr->exprBase.base.lineNo,
                           PolynaryExpr->exprBase.base.stringNo);


        clmGEN_CODE_IF(Compiler,
             CodeGenerator,
             PolynaryExpr->exprBase.base.lineNo,
             PolynaryExpr->exprBase.base.stringNo,
             &intermROperands[1],
             clvCONDITION_LESS_THAN_EQUAL,
             &twoPow20ROperand);
        /*|Y| <= 2^20, We don't have exp adjustment, the divider is good */

        status = clGenGenericCode2(Compiler,
                           PolynaryExpr->exprBase.base.lineNo,
                           PolynaryExpr->exprBase.base.stringNo,
                           clvOPCODE_DIV,
                           IOperand,
                           &OperandsParameters[0].rOperands[0],
                           &OperandsParameters[1].rOperands[0]);


        clmGEN_CODE_ELSE(Compiler,
                           CodeGenerator,
                           PolynaryExpr->exprBase.base.lineNo,
                           PolynaryExpr->exprBase.base.stringNo);


        status = clGenGenericCode2(Compiler,
                           PolynaryExpr->exprBase.base.lineNo,
                           PolynaryExpr->exprBase.base.stringNo,
                           clvOPCODE_DIV,
                           &intermIOperands[0],
                           &f4ROperand,
                           &OperandsParameters[1].rOperands[0]);

        status = clGenGenericCode2(Compiler,
                           PolynaryExpr->exprBase.base.lineNo,
                           PolynaryExpr->exprBase.base.stringNo,
                           clvOPCODE_MUL,
                           &intermIOperands[2],
                           &intermROperands[0],
                           &OperandsParameters[0].rOperands[0]);

        status = clGenGenericCode2(Compiler,
                           PolynaryExpr->exprBase.base.lineNo,
                           PolynaryExpr->exprBase.base.stringNo,
                           clvOPCODE_MUL,
                           IOperand,
                           &intermROperands[2],
                           &f_25ROperand);


        clmGEN_CODE_ENDIF(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);

        clmGEN_CODE_ENDIF(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);



            if (gcmIS_ERROR(status)) return status;
OnError:
    return status;
  }
#endif

    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    status = clGenDivCode(Compiler,
                CodeGenerator,
                PolynaryExpr->exprBase.base.lineNo,
                PolynaryExpr->exprBase.base.stringNo,
                OperandCount,
                OperandsParameters,
                IOperand);
    if (gcmIS_ERROR(status)) return status;
    return gcvSTATUS_OK;
}

static clsBUILTIN_FUNCTION_INFO    _BuiltinFunctionInfos[] =
{
    /* Work-Item functions. */
    {"get_global_id",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenGetGlobalIdCode},
    {"get_local_id",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenGetLocalIdCode},
    {"get_group_id",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenGetGroupIdCode},
    {"get_work_dim",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenGetWorkDimCode},
    {"get_global_size",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenGetGlobalSizeCode},
    {"get_local_size",     gcvFALSE,    gcvFALSE,   gcvNULL, _GenGetLocalSizeCode},
    {"get_num_groups",     gcvFALSE,    gcvFALSE,   gcvNULL, _GenGetNumGroupsCode},
    {"get_global_offset",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenGetGlobalOffsetCode},

    /* Synchronization function. */
    {"barrier",            gcvFALSE,    gcvFALSE,   gcvNULL, _GenBarrierCode},
    {"mem_fence",          gcvFALSE,    gcvFALSE,   gcvNULL, _GenMemFenceCode},

    /* Conversion functions - non-saturated mode */
    {"convert_char",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtzCode},
    {"convert_uchar",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtzCode},
    {"convert_int",        gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtzCode},
    {"convert_uint",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtzCode},
    {"convert_long",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtzCode},
    {"convert_ulong",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtzCode},
    {"convert_short",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtzCode},
    {"convert_ushort",     gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtzCode},
    {"convert_float",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_FloatDefaultCode},

    {"convert_char_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rteCode},
    {"convert_uchar_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rteCode},
    {"convert_int_rte",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rteCode},
    {"convert_uint_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rteCode},
    {"convert_long_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rteCode},
    {"convert_ulong_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rteCode},
    {"convert_short_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rteCode},
    {"convert_ushort_rte", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rteCode},
    {"convert_float_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rteCode},

    {"convert_char_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtzCode},
    {"convert_uchar_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtzCode},
    {"convert_int_rtz",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtzCode},
    {"convert_uint_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtzCode},
    {"convert_long_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtzCode},
    {"convert_ulong_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtzCode},
    {"convert_short_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtzCode},
    {"convert_ushort_rtz", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtzCode},
    {"convert_float_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtzCode},

    {"convert_char_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtpCode},
    {"convert_uchar_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtpCode},
    {"convert_int_rtp",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtpCode},
    {"convert_uint_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtpCode},
    {"convert_long_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtpCode},
    {"convert_ulong_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtpCode},
    {"convert_short_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtpCode},
    {"convert_ushort_rtp", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtpCode},
    {"convert_float_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtpCode},

    {"convert_char_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtnCode},
    {"convert_uchar_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtnCode},
    {"convert_int_rtn",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtnCode},
    {"convert_uint_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtnCode},
    {"convert_long_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtnCode},
    {"convert_ulong_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtnCode},
    {"convert_short_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtnCode},
    {"convert_ushort_rtn", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtnCode},
    {"convert_float_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_rtnCode},

    /* Conversion functions - saturated mode */
    {"convert_char_sat",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtzCode},
    {"convert_uchar_sat",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtzCode},
    {"convert_int_sat",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtzCode},
    {"convert_uint_sat",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtzCode},
    {"convert_long_sat",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtzCode},
    {"convert_ulong_sat",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtzCode},
    {"convert_short_sat",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtzCode},
    {"convert_ushort_sat", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtzCode},

    {"convert_char_sat_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rteCode},
    {"convert_uchar_sat_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rteCode},
    {"convert_int_sat_rte",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rteCode},
    {"convert_uint_sat_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rteCode},
    {"convert_long_sat_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rteCode},
    {"convert_ulong_sat_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rteCode},
    {"convert_short_sat_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rteCode},
    {"convert_ushort_sat_rte", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rteCode},

    {"convert_char_sat_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtzCode},
    {"convert_uchar_sat_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtzCode},
    {"convert_int_sat_rtz",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtzCode},
    {"convert_uint_sat_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtzCode},
    {"convert_long_sat_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtzCode},
    {"convert_ulong_sat_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtzCode},
    {"convert_short_sat_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtzCode},
    {"convert_ushort_sat_rtz", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtzCode},

    {"convert_char_sat_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtpCode},
    {"convert_uchar_sat_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtpCode},
    {"convert_int_sat_rtp",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtpCode},
    {"convert_uint_sat_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtpCode},
    {"convert_long_sat_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtpCode},
    {"convert_ulong_sat_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtpCode},
    {"convert_short_sat_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtpCode},
    {"convert_ushort_sat_rtp", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtpCode},

    {"convert_char_sat_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtnCode},
    {"convert_uchar_sat_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtnCode},
    {"convert_int_sat_rtn",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtnCode},
    {"convert_uint_sat_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtnCode},
    {"convert_long_sat_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtnCode},
    {"convert_ulong_sat_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtnCode},
    {"convert_short_sat_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtnCode},
    {"convert_ushort_sat_rtn", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert_sat_rtnCode},

#if _USE_CONV_FOR_EXPLICIT_CONVERT_FUNCTION
    /* Conversion functions - non-saturated mode 2 */
    {"convert_char2",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_uchar2",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_int2",        gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_uint2",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_long2",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_ulong2",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_short2",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_ushort2",     gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_float2",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_FloatDefaultCode},

#if _DO_NOT_USE_CONV_FOR_RTNE_EXPLICIT_CONVERT_FUNCTION
    {"convert_char2_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rteCode},
    {"convert_uchar2_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rteCode},
    {"convert_int2_rte",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rteCode},
    {"convert_uint2_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rteCode},
    {"convert_long2_rte",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert2_rteCode},
    {"convert_ulong2_rte",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert2_rteCode},
    {"convert_short2_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rteCode},
    {"convert_ushort2_rte", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rteCode},
    {"convert_float2_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rteCode},
#else
    {"convert_char2_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rteCode},
    {"convert_uchar2_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rteCode},
    {"convert_int2_rte",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rteCode},
    {"convert_uint2_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rteCode},
    {"convert_long2_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rteCode},
    {"convert_ulong2_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rteCode},
    {"convert_short2_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rteCode},
    {"convert_ushort2_rte", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rteCode},
    {"convert_float2_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rteCode},
#endif

    {"convert_char2_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_uchar2_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_int2_rtz",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_uint2_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_long2_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_ulong2_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_short2_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_ushort2_rtz", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_float2_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtzCode},

    {"convert_char2_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtpCode},
    {"convert_uchar2_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtpCode},
    {"convert_int2_rtp",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtpCode},
    {"convert_uint2_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtpCode},
    {"convert_long2_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtpCode},
    {"convert_ulong2_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtpCode},
    {"convert_short2_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtpCode},
    {"convert_ushort2_rtp", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtpCode},
#if _DO_NOT_USE_CONV_FOR_FLOATN_RTP_RTN_EXPLICIT_CONVERT_FUNCTION
    {"convert_float2_rtp",  gcvFALSE,    gcvTRUE,    gcvNULL, _GenConvert2_rtpCode},
#else
    {"convert_float2_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtpCode},
#endif

    {"convert_char2_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtnCode},
    {"convert_uchar2_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtnCode},
    {"convert_int2_rtn",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtnCode},
    {"convert_uint2_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtnCode},
    {"convert_long2_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtnCode},
    {"convert_ulong2_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtnCode},
    {"convert_short2_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtnCode},
    {"convert_ushort2_rtn", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtnCode},
#if _DO_NOT_USE_CONV_FOR_FLOATN_RTP_RTN_EXPLICIT_CONVERT_FUNCTION
    {"convert_float2_rtn",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtnCode},
#else
    {"convert_float2_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_rtnCode},
#endif

    /* Conversion functions - saturated mode 2 */
    {"convert_char2_sat",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_uchar2_sat",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_int2_sat",        gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_uint2_sat",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_long2_sat",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_ulong2_sat",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_short2_sat",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_ushort2_sat",     gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtzCode},

#if _DO_NOT_USE_CONV_FOR_RTNE_EXPLICIT_CONVERT_FUNCTION
    {"convert_char2_sat_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rteCode},
    {"convert_uchar2_sat_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rteCode},
    {"convert_int2_sat_rte",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rteCode},
    {"convert_uint2_sat_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rteCode},
    {"convert_long2_sat_rte",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert2_sat_rteCode},
    {"convert_ulong2_sat_rte",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert2_sat_rteCode},
    {"convert_short2_sat_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rteCode},
    {"convert_ushort2_sat_rte", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rteCode},
#else
    {"convert_char2_sat_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rteCode},
    {"convert_uchar2_sat_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rteCode},
    {"convert_int2_sat_rte",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rteCode},
    {"convert_uint2_sat_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rteCode},
    {"convert_long2_sat_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rteCode},
    {"convert_ulong2_sat_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rteCode},
    {"convert_short2_sat_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rteCode},
    {"convert_ushort2_sat_rte", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rteCode},
#endif

    {"convert_char2_sat_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_uchar2_sat_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_int2_sat_rtz",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_uint2_sat_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_long2_sat_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_ulong2_sat_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_short2_sat_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_ushort2_sat_rtz", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtzCode},

    {"convert_char2_sat_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtpCode},
    {"convert_uchar2_sat_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtpCode},
    {"convert_int2_sat_rtp",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtpCode},
    {"convert_uint2_sat_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtpCode},
    {"convert_long2_sat_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtpCode},
    {"convert_ulong2_sat_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtpCode},
    {"convert_short2_sat_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtpCode},
    {"convert_ushort2_sat_rtp", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtpCode},

    {"convert_char2_sat_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtnCode},
    {"convert_uchar2_sat_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtnCode},
    {"convert_int2_sat_rtn",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtnCode},
    {"convert_uint2_sat_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtnCode},
    {"convert_long2_sat_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtnCode},
    {"convert_ulong2_sat_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtnCode},
    {"convert_short2_sat_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtnCode},
    {"convert_ushort2_sat_rtn", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert2_sat_rtnCode},

    /* Conversion functions - non-saturated mode 3 */
    {"convert_char3",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_uchar3",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_int3",        gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_uint3",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_long3",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_ulong3",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_short3",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_ushort3",     gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_float3",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_FloatDefaultCode},

#if _DO_NOT_USE_CONV_FOR_RTNE_EXPLICIT_CONVERT_FUNCTION
    {"convert_char3_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rteCode},
    {"convert_uchar3_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rteCode},
    {"convert_int3_rte",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rteCode},
    {"convert_uint3_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rteCode},
    {"convert_long3_rte",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert3_rteCode},
    {"convert_ulong3_rte",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert3_rteCode},
    {"convert_short3_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rteCode},
    {"convert_ushort3_rte", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rteCode},
    {"convert_float3_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rteCode},
#else
    {"convert_char3_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rteCode},
    {"convert_uchar3_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rteCode},
    {"convert_int3_rte",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rteCode},
    {"convert_uint3_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rteCode},
    {"convert_long3_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rteCode},
    {"convert_ulong3_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rteCode},
    {"convert_short3_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rteCode},
    {"convert_ushort3_rte", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rteCode},
    {"convert_float3_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rteCode},
#endif

    {"convert_char3_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_uchar3_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_int3_rtz",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_uint3_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_long3_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_ulong3_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_short3_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_ushort3_rtz", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_float3_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtzCode},

    {"convert_char3_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtpCode},
    {"convert_uchar3_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtpCode},
    {"convert_int3_rtp",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtpCode},
    {"convert_uint3_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtpCode},
    {"convert_long3_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtpCode},
    {"convert_ulong3_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtpCode},
    {"convert_short3_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtpCode},
    {"convert_ushort3_rtp", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtpCode},
#if _DO_NOT_USE_CONV_FOR_FLOATN_RTP_RTN_EXPLICIT_CONVERT_FUNCTION
    {"convert_float3_rtp",  gcvFALSE,    gcvTRUE,    gcvNULL, _GenConvert3_rtpCode},
#else
    {"convert_float3_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtpCode},
#endif

    {"convert_char3_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtnCode},
    {"convert_uchar3_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtnCode},
    {"convert_int3_rtn",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtnCode},
    {"convert_uint3_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtnCode},
    {"convert_long3_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtnCode},
    {"convert_ulong3_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtnCode},
    {"convert_short3_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtnCode},
    {"convert_ushort3_rtn", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtnCode},
#if _DO_NOT_USE_CONV_FOR_FLOATN_RTP_RTN_EXPLICIT_CONVERT_FUNCTION
    {"convert_float3_rtn",  gcvFALSE,    gcvTRUE,    gcvNULL, _GenConvert3_rtnCode},
#else
    {"convert_float3_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_rtnCode},
#endif

    /* Conversion functions - saturated mode 3 */
    {"convert_char3_sat",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_uchar3_sat",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_int3_sat",        gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_uint3_sat",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_long3_sat",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_ulong3_sat",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_short3_sat",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_ushort3_sat",     gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtzCode},

#if _DO_NOT_USE_CONV_FOR_RTNE_EXPLICIT_CONVERT_FUNCTION
    {"convert_char3_sat_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rteCode},
    {"convert_uchar3_sat_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rteCode},
    {"convert_int3_sat_rte",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rteCode},
    {"convert_uint3_sat_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rteCode},
    {"convert_long3_sat_rte",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert3_sat_rteCode},
    {"convert_ulong3_sat_rte",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert3_sat_rteCode},
    {"convert_short3_sat_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rteCode},
    {"convert_ushort3_sat_rte", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rteCode},
#else
    {"convert_char3_sat_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rteCode},
    {"convert_uchar3_sat_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rteCode},
    {"convert_int3_sat_rte",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rteCode},
    {"convert_uint3_sat_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rteCode},
    {"convert_long3_sat_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rteCode},
    {"convert_ulong3_sat_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rteCode},
    {"convert_short3_sat_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rteCode},
    {"convert_ushort3_sat_rte", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rteCode},
#endif

    {"convert_char3_sat_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_uchar3_sat_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_int3_sat_rtz",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_uint3_sat_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_long3_sat_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_ulong3_sat_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_short3_sat_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_ushort3_sat_rtz", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtzCode},

    {"convert_char3_sat_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtpCode},
    {"convert_uchar3_sat_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtpCode},
    {"convert_int3_sat_rtp",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtpCode},
    {"convert_uint3_sat_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtpCode},
    {"convert_long3_sat_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtpCode},
    {"convert_ulong3_sat_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtpCode},
    {"convert_short3_sat_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtpCode},
    {"convert_ushort3_sat_rtp", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtpCode},

    {"convert_char3_sat_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtnCode},
    {"convert_uchar3_sat_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtnCode},
    {"convert_int3_sat_rtn",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtnCode},
    {"convert_uint3_sat_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtnCode},
    {"convert_long3_sat_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtnCode},
    {"convert_ulong3_sat_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtnCode},
    {"convert_short3_sat_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtnCode},
    {"convert_ushort3_sat_rtn", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert3_sat_rtnCode},

    /* Conversion functions - non-saturated mode 4 */
    {"convert_char4",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_uchar4",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_int4",        gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_uint4",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_long4",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_ulong4",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_short4",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_ushort4",     gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_float4",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_FloatDefaultCode},

#if _DO_NOT_USE_CONV_FOR_RTNE_EXPLICIT_CONVERT_FUNCTION
    {"convert_char4_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rteCode},
    {"convert_uchar4_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rteCode},
    {"convert_int4_rte",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rteCode},
    {"convert_uint4_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rteCode},
    {"convert_long4_rte",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert4_rteCode},
    {"convert_ulong4_rte",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert4_rteCode},
    {"convert_short4_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rteCode},
    {"convert_ushort4_rte", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rteCode},
    {"convert_float4_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rteCode},
#else
    {"convert_char4_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rteCode},
    {"convert_uchar4_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rteCode},
    {"convert_int4_rte",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rteCode},
    {"convert_uint4_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rteCode},
    {"convert_long4_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rteCode},
    {"convert_ulong4_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rteCode},
    {"convert_short4_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rteCode},
    {"convert_ushort4_rte", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rteCode},
    {"convert_float4_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rteCode},
#endif

    {"convert_char4_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_uchar4_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_int4_rtz",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_uint4_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_long4_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_ulong4_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_short4_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_ushort4_rtz", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_float4_rtz",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvertFloat4_rtzCode},

    {"convert_char4_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtpCode},
    {"convert_uchar4_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtpCode},
    {"convert_int4_rtp",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtpCode},
    {"convert_uint4_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtpCode},
    {"convert_long4_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtpCode},
    {"convert_ulong4_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtpCode},
    {"convert_short4_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtpCode},
    {"convert_ushort4_rtp", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtpCode},
#if _DO_NOT_USE_CONV_FOR_FLOATN_RTP_RTN_EXPLICIT_CONVERT_FUNCTION
    {"convert_float4_rtp",  gcvFALSE,    gcvTRUE,    gcvNULL, _GenConvert4_rtpCode},
#else
    {"convert_float4_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtpCode},
#endif

    {"convert_char4_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtnCode},
    {"convert_uchar4_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtnCode},
    {"convert_int4_rtn",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtnCode},
    {"convert_uint4_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtnCode},
    {"convert_long4_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtnCode},
    {"convert_ulong4_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtnCode},
    {"convert_short4_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtnCode},
    {"convert_ushort4_rtn", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtnCode},
#if _DO_NOT_USE_CONV_FOR_FLOATN_RTP_RTN_EXPLICIT_CONVERT_FUNCTION
    {"convert_float4_rtn",  gcvFALSE,    gcvTRUE,    gcvNULL, _GenConvert4_rtnCode},
#else
    {"convert_float4_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_rtnCode},
#endif

    /* Conversion functions - saturated mode 4 */
    {"convert_char4_sat",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_uchar4_sat",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_int4_sat",        gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_uint4_sat",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_long4_sat",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_ulong4_sat",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_short4_sat",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_ushort4_sat",     gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtzCode},

#if _DO_NOT_USE_CONV_FOR_RTNE_EXPLICIT_CONVERT_FUNCTION
    {"convert_char4_sat_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rteCode},
    {"convert_uchar4_sat_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rteCode},
    {"convert_int4_sat_rte",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rteCode},
    {"convert_uint4_sat_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rteCode},
    {"convert_long4_sat_rte",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert4_sat_rteCode},
    {"convert_ulong4_sat_rte",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert4_sat_rteCode},
    {"convert_short4_sat_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rteCode},
    {"convert_ushort4_sat_rte", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rteCode},
#else
    {"convert_char4_sat_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rteCode},
    {"convert_uchar4_sat_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rteCode},
    {"convert_int4_sat_rte",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rteCode},
    {"convert_uint4_sat_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rteCode},
    {"convert_long4_sat_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rteCode},
    {"convert_ulong4_sat_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rteCode},
    {"convert_short4_sat_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rteCode},
    {"convert_ushort4_sat_rte", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rteCode},
#endif

    {"convert_char4_sat_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_uchar4_sat_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_int4_sat_rtz",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_uint4_sat_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_long4_sat_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_ulong4_sat_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_short4_sat_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_ushort4_sat_rtz", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtzCode},

    {"convert_char4_sat_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtpCode},
    {"convert_uchar4_sat_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtpCode},
    {"convert_long4_sat_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtpCode},
    {"convert_ulong4_sat_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtpCode},
    {"convert_int4_sat_rtp",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtpCode},
    {"convert_uint4_sat_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtpCode},
    {"convert_short4_sat_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtpCode},
    {"convert_ushort4_sat_rtp", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtpCode},

    {"convert_char4_sat_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtnCode},
    {"convert_uchar4_sat_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtnCode},
    {"convert_int4_sat_rtn",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtnCode},
    {"convert_uint4_sat_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtnCode},
    {"convert_long4_sat_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtnCode},
    {"convert_ulong4_sat_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtnCode},
    {"convert_short4_sat_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtnCode},
    {"convert_ushort4_sat_rtn", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert4_sat_rtnCode},

    /* Conversion functions - non-saturated mode 8 */
    {"convert_char8",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_uchar8",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_int8",        gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_uint8",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_long8",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_ulong8",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_short8",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_ushort8",     gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_float8",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_FloatDefaultCode},

#if _DO_NOT_USE_CONV_FOR_RTNE_EXPLICIT_CONVERT_FUNCTION
    {"convert_char8_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rteCode},
    {"convert_uchar8_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rteCode},
    {"convert_int8_rte",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rteCode},
    {"convert_uint8_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rteCode},
    {"convert_long8_rte",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert8_rteCode},
    {"convert_ulong8_rte",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert8_rteCode},
    {"convert_short8_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rteCode},
    {"convert_ushort8_rte", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rteCode},
    {"convert_float8_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rteCode},
#else
    {"convert_char8_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rteCode},
    {"convert_uchar8_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rteCode},
    {"convert_int8_rte",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rteCode},
    {"convert_uint8_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rteCode},
    {"convert_long8_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rteCode},
    {"convert_ulong8_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rteCode},
    {"convert_short8_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rteCode},
    {"convert_ushort8_rte", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rteCode},
    {"convert_float8_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rteCode},
#endif

    {"convert_char8_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_uchar8_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_int8_rtz",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_uint8_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_long8_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_ulong8_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_short8_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_ushort8_rtz", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_float8_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtzCode},

    {"convert_char8_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtpCode},
    {"convert_uchar8_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtpCode},
    {"convert_int8_rtp",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtpCode},
    {"convert_uint8_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtpCode},
    {"convert_long8_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtpCode},
    {"convert_ulong8_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtpCode},
    {"convert_short8_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtpCode},
    {"convert_ushort8_rtp", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtpCode},
#if _DO_NOT_USE_CONV_FOR_FLOATN_RTP_RTN_EXPLICIT_CONVERT_FUNCTION
    {"convert_float8_rtp",  gcvFALSE,    gcvTRUE,    gcvNULL, _GenConvert8_rtpCode},
#else
    {"convert_float8_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtpCode},
#endif

    {"convert_char8_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtnCode},
    {"convert_uchar8_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtnCode},
    {"convert_int8_rtn",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtnCode},
    {"convert_uint8_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtnCode},
    {"convert_long8_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtnCode},
    {"convert_ulong8_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtnCode},
    {"convert_short8_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtnCode},
    {"convert_ushort8_rtn", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtnCode},
#if _DO_NOT_USE_CONV_FOR_FLOATN_RTP_RTN_EXPLICIT_CONVERT_FUNCTION
    {"convert_float8_rtn",  gcvFALSE,    gcvTRUE,    gcvNULL, _GenConvert8_rtnCode},
#else
    {"convert_float8_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_rtnCode},
#endif

    /* Conversion functions - saturated mode 8 */
    {"convert_char8_sat",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_uchar8_sat",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_int8_sat",        gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_uint8_sat",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_long8_sat",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_ulong8_sat",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_short8_sat",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_ushort8_sat",     gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtzCode},

#if _DO_NOT_USE_CONV_FOR_RTNE_EXPLICIT_CONVERT_FUNCTION
    {"convert_char8_sat_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rteCode},
    {"convert_uchar8_sat_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rteCode},
    {"convert_int8_sat_rte",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rteCode},
    {"convert_uint8_sat_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rteCode},
    {"convert_long8_sat_rte",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert8_sat_rteCode},
    {"convert_ulong8_sat_rte",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert8_sat_rteCode},
    {"convert_short8_sat_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rteCode},
    {"convert_ushort8_sat_rte", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rteCode},
#else
    {"convert_char8_sat_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rteCode},
    {"convert_uchar8_sat_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rteCode},
    {"convert_int8_sat_rte",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rteCode},
    {"convert_uint8_sat_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rteCode},
    {"convert_long8_sat_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rteCode},
    {"convert_ulong8_sat_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rteCode},
    {"convert_short8_sat_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rteCode},
    {"convert_ushort8_sat_rte", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rteCode},
#endif

    {"convert_char8_sat_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_uchar8_sat_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_int8_sat_rtz",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_ulong8_sat_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_long8_sat_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_uint8_sat_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_short8_sat_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_ushort8_sat_rtz", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtzCode},

    {"convert_char8_sat_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtpCode},
    {"convert_uchar8_sat_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtpCode},
    {"convert_int8_sat_rtp",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtpCode},
    {"convert_uint8_sat_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtpCode},
    {"convert_long8_sat_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtpCode},
    {"convert_ulong8_sat_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtpCode},
    {"convert_short8_sat_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtpCode},
    {"convert_ushort8_sat_rtp", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtpCode},

    {"convert_char8_sat_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtnCode},
    {"convert_uchar8_sat_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtnCode},
    {"convert_int8_sat_rtn",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtnCode},
    {"convert_uint8_sat_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtnCode},
    {"convert_long8_sat_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtnCode},
    {"convert_ulong8_sat_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtnCode},
    {"convert_short8_sat_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtnCode},
    {"convert_ushort8_sat_rtn", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert8_sat_rtnCode},

    /* Conversion functions - non-saturated mode 16 */
    {"convert_char16",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_uchar16",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_int16",        gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_uint16",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_long16",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_ulong16",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_short16",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_ushort16",     gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_float16",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_FloatDefaultCode},

#if _DO_NOT_USE_CONV_FOR_RTNE_EXPLICIT_CONVERT_FUNCTION
    {"convert_char16_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rteCode},
    {"convert_uchar16_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rteCode},
    {"convert_int16_rte",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rteCode},
    {"convert_uint16_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rteCode},
    {"convert_long16_rte",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert16_rteCode},
    {"convert_ulong16_rte",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert16_rteCode},
    {"convert_short16_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rteCode},
    {"convert_ushort16_rte", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rteCode},
    {"convert_float16_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rteCode},
#else
    {"convert_char16_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rteCode},
    {"convert_uchar16_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rteCode},
    {"convert_int16_rte",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rteCode},
    {"convert_uint16_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rteCode},
    {"convert_long16_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rteCode},
    {"convert_ulong16_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rteCode},
    {"convert_short16_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rteCode},
    {"convert_ushort16_rte", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rteCode},
    {"convert_float16_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rteCode},
#endif

    {"convert_char16_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_uchar16_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_int16_rtz",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_uint16_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_long16_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_ulong16_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_short16_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_ushort16_rtz", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_float16_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtzCode},

    {"convert_char16_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtpCode},
    {"convert_uchar16_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtpCode},
    {"convert_int16_rtp",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtpCode},
    {"convert_uint16_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtpCode},
    {"convert_long16_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtpCode},
    {"convert_ulong16_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtpCode},
    {"convert_short16_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtpCode},
    {"convert_ushort16_rtp", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtpCode},
#if _DO_NOT_USE_CONV_FOR_FLOATN_RTP_RTN_EXPLICIT_CONVERT_FUNCTION
    {"convert_float16_rtp",  gcvFALSE,    gcvTRUE,    gcvNULL, _GenConvert16_rtpCode},
#else
    {"convert_float16_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtpCode},
#endif

    {"convert_char16_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtnCode},
    {"convert_uchar16_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtnCode},
    {"convert_int16_rtn",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtnCode},
    {"convert_uint16_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtnCode},
    {"convert_long16_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtnCode},
    {"convert_ulong16_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtnCode},
    {"convert_short16_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtnCode},
    {"convert_ushort16_rtn", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtnCode},
#if _DO_NOT_USE_CONV_FOR_FLOATN_RTP_RTN_EXPLICIT_CONVERT_FUNCTION
    {"convert_float16_rtn",  gcvFALSE,    gcvTRUE,    gcvNULL, _GenConvert16_rtnCode},
#else
    {"convert_float16_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_rtnCode},
#endif

    /* Conversion functions - saturated mode 16 */
    {"convert_char16_sat",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_uchar16_sat",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_int16_sat",        gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_uint16_sat",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_long16_sat",       gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_ulong16_sat",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_short16_sat",      gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_ushort16_sat",     gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtzCode},

#if _DO_NOT_USE_CONV_FOR_RTNE_EXPLICIT_CONVERT_FUNCTION
    {"convert_char16_sat_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rteCode},
    {"convert_uchar16_sat_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rteCode},
    {"convert_int16_sat_rte",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rteCode},
    {"convert_uint16_sat_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rteCode},
    {"convert_long16_sat_rte",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert16_sat_rteCode},
    {"convert_ulong16_sat_rte",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert16_sat_rteCode},
    {"convert_short16_sat_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rteCode},
    {"convert_ushort16_sat_rte", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rteCode},
#else
    {"convert_char16_sat_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rteCode},
    {"convert_uchar16_sat_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rteCode},
    {"convert_int16_sat_rte",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rteCode},
    {"convert_uint16_sat_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rteCode},
    {"convert_long16_sat_rte",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rteCode},
    {"convert_ulong16_sat_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rteCode},
    {"convert_short16_sat_rte",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rteCode},
    {"convert_ushort16_sat_rte", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rteCode},
#endif

    {"convert_char16_sat_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_uchar16_sat_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_int16_sat_rtz",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_uint16_sat_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_long16_sat_rtz",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_ulong16_sat_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_short16_sat_rtz",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_ushort16_sat_rtz", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtzCode},

    {"convert_char16_sat_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtpCode},
    {"convert_uchar16_sat_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtpCode},
    {"convert_int16_sat_rtp",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtpCode},
    {"convert_ulong16_sat_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtpCode},
    {"convert_long16_sat_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtpCode},
    {"convert_uint16_sat_rtp",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtpCode},
    {"convert_short16_sat_rtp",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtpCode},
    {"convert_ushort16_sat_rtp", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtpCode},

    {"convert_char16_sat_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtnCode},
    {"convert_uchar16_sat_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtnCode},
    {"convert_int16_sat_rtn",    gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtnCode},
    {"convert_ulong16_sat_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtnCode},
    {"convert_long16_sat_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtnCode},
    {"convert_uint16_sat_rtn",   gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtnCode},
    {"convert_short16_sat_rtn",  gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtnCode},
    {"convert_ushort16_sat_rtn", gcvFALSE,    gcvFALSE,   gcvNULL, _GenConvert16_sat_rtnCode},
#else
    /* Conversion functions - non-saturated mode 2 */
    {"convert_char2",       gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_uchar2",      gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_int2",        gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_uint2",       gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_long2",       gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert2_rtzCode},
    {"convert_ulong2",      gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert2_rtzCode},
    {"convert_short2",      gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_ushort2",     gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_float2",      gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_FloatDefaultCode},

    {"convert_char2_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rteCode},
    {"convert_uchar2_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rteCode},
    {"convert_int2_rte",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rteCode},
    {"convert_uint2_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rteCode},
    {"convert_long2_rte",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert2_rteCode},
    {"convert_ulong2_rte",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert2_rteCode},
    {"convert_short2_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rteCode},
    {"convert_ushort2_rte", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rteCode},
    {"convert_float2_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rteCode},

    {"convert_char2_rtz",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_uchar2_rtz",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_int2_rtz",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_uint2_rtz",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_long2_rtz",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert2_rtzCode},
    {"convert_ulong2_rtz",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert2_rtzCode},
    {"convert_short2_rtz",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_ushort2_rtz", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtzCode},
    {"convert_float2_rtz",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtzCode},

    {"convert_char2_rtp",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtpCode},
    {"convert_uchar2_rtp",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtpCode},
    {"convert_int2_rtp",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtpCode},
    {"convert_uint2_rtp",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtpCode},
    {"convert_long2_rtp",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert2_rtpCode},
    {"convert_ulong2_rtp",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert2_rtpCode},
    {"convert_short2_rtp",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtpCode},
    {"convert_ushort2_rtp", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtpCode},
    {"convert_float2_rtp",  gcvFALSE,    gcvTRUE,    gcvNULL, _GenConvert2_rtpCode},

    {"convert_char2_rtn",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtnCode},
    {"convert_uchar2_rtn",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtnCode},
    {"convert_int2_rtn",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtnCode},
    {"convert_uint2_rtn",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtnCode},
    {"convert_long2_rtn",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert2_rtnCode},
    {"convert_ulong2_rtn",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert2_rtnCode},
    {"convert_short2_rtn",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtnCode},
    {"convert_ushort2_rtn", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtnCode},
    {"convert_float2_rtn",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_rtnCode},

    /* Conversion functions - saturated mode 2 */
    {"convert_char2_sat",       gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_uchar2_sat",      gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_int2_sat",        gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_uint2_sat",       gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_long2_sat",       gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_ulong2_sat",      gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_short2_sat",      gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_ushort2_sat",     gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rtzCode},

    {"convert_char2_sat_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rteCode},
    {"convert_uchar2_sat_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rteCode},
    {"convert_int2_sat_rte",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rteCode},
    {"convert_uint2_sat_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rteCode},
    {"convert_long2_sat_rte",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert2_sat_rteCode},
    {"convert_ulong2_sat_rte",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert2_sat_rteCode},
    {"convert_short2_sat_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rteCode},
    {"convert_ushort2_sat_rte", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rteCode},

    {"convert_char2_sat_rtz",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_uchar2_sat_rtz",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_int2_sat_rtz",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_uint2_sat_rtz",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_long2_sat_rtz",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_ulong2_sat_rtz",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_short2_sat_rtz",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rtzCode},
    {"convert_ushort2_sat_rtz", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rtzCode},

    {"convert_char2_sat_rtp",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rtpCode},
    {"convert_uchar2_sat_rtp",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rtpCode},
    {"convert_int2_sat_rtp",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rtpCode},
    {"convert_uint2_sat_rtp",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rtpCode},
    {"convert_long2_sat_rtp",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert2_sat_rtpCode},
    {"convert_ulong2_sat_rtp",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert2_sat_rtpCode},
    {"convert_short2_sat_rtp",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rtpCode},
    {"convert_ushort2_sat_rtp", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rtpCode},

    {"convert_char2_sat_rtn",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rtnCode},
    {"convert_uchar2_sat_rtn",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rtnCode},
    {"convert_int2_sat_rtn",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rtnCode},
    {"convert_uint2_sat_rtn",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rtnCode},
    {"convert_long2_sat_rtn",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert2_sat_rtnCode},
    {"convert_ulong2_sat_rtn",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert2_sat_rtnCode},
    {"convert_short2_sat_rtn",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rtnCode},
    {"convert_ushort2_sat_rtn", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert2_sat_rtnCode},

    /* Conversion functions - non-saturated mode 3 */
    {"convert_char3",       gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_uchar3",      gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_int3",        gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_uint3",       gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_long3",       gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert3_rtzCode},
    {"convert_ulong3",      gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert3_rtzCode},
    {"convert_short3",      gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_ushort3",     gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_float3",      gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_FloatDefaultCode},

    {"convert_char3_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rteCode},
    {"convert_uchar3_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rteCode},
    {"convert_int3_rte",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rteCode},
    {"convert_uint3_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rteCode},
    {"convert_long3_rte",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert3_rteCode},
    {"convert_ulong3_rte",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert3_rteCode},
    {"convert_short3_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rteCode},
    {"convert_ushort3_rte", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rteCode},
    {"convert_float3_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rteCode},

    {"convert_char3_rtz",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_uchar3_rtz",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_int3_rtz",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_uint3_rtz",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_long3_rtz",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert3_rtzCode},
    {"convert_ulong3_rtz",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert3_rtzCode},
    {"convert_short3_rtz",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_ushort3_rtz", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rtzCode},
    {"convert_float3_rtz",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rtzCode},

    {"convert_char3_rtp",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rtpCode},
    {"convert_uchar3_rtp",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rtpCode},
    {"convert_int3_rtp",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rtpCode},
    {"convert_uint3_rtp",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rtpCode},
    {"convert_long3_rtp",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert3_rtpCode},
    {"convert_ulong3_rtp",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert3_rtpCode},
    {"convert_short3_rtp",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rtpCode},
    {"convert_ushort3_rtp", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rtpCode},
    {"convert_float3_rtp",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rtpCode},

    {"convert_char3_rtn",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rtnCode},
    {"convert_uchar3_rtn",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rtnCode},
    {"convert_int3_rtn",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rtnCode},
    {"convert_uint3_rtn",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rtnCode},
    {"convert_long3_rtn",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert3_rtnCode},
    {"convert_ulong3_rtn",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert3_rtnCode},
    {"convert_short3_rtn",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rtnCode},
    {"convert_ushort3_rtn", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_rtnCode},
    {"convert_float3_rtn",  gcvFALSE,    gcvTRUE,    gcvNULL, _GenConvert3_rtnCode},

    /* Conversion functions - saturated mode 3 */
    {"convert_char3_sat",       gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_uchar3_sat",      gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_int3_sat",        gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_uint3_sat",       gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_long3_sat",       gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_ulong3_sat",      gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_short3_sat",      gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_ushort3_sat",     gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rtzCode},

    {"convert_char3_sat_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rteCode},
    {"convert_uchar3_sat_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rteCode},
    {"convert_int3_sat_rte",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rteCode},
    {"convert_uint3_sat_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rteCode},
    {"convert_long3_sat_rte",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert3_sat_rteCode},
    {"convert_ulong3_sat_rte",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert3_sat_rteCode},
    {"convert_short3_sat_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rteCode},
    {"convert_ushort3_sat_rte", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rteCode},

    {"convert_char3_sat_rtz",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_uchar3_sat_rtz",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_int3_sat_rtz",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_uint3_sat_rtz",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_long3_sat_rtz",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_ulong3_sat_rtz",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_short3_sat_rtz",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rtzCode},
    {"convert_ushort3_sat_rtz", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rtzCode},

    {"convert_char3_sat_rtp",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rtpCode},
    {"convert_uchar3_sat_rtp",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rtpCode},
    {"convert_int3_sat_rtp",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rtpCode},
    {"convert_uint3_sat_rtp",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rtpCode},
    {"convert_long3_sat_rtp",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert3_sat_rtpCode},
    {"convert_ulong3_sat_rtp",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert3_sat_rtpCode},
    {"convert_short3_sat_rtp",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rtpCode},
    {"convert_ushort3_sat_rtp", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rtpCode},

    {"convert_char3_sat_rtn",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rtnCode},
    {"convert_uchar3_sat_rtn",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rtnCode},
    {"convert_int3_sat_rtn",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rtnCode},
    {"convert_uint3_sat_rtn",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rtnCode},
    {"convert_long3_sat_rtn",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert3_sat_rtnCode},
    {"convert_ulong3_sat_rtn",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert3_sat_rtnCode},
    {"convert_short3_sat_rtn",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rtnCode},
    {"convert_ushort3_sat_rtn", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert3_sat_rtnCode},

    /* Conversion functions - non-saturated mode 4 */
    {"convert_char4",       gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_uchar4",      gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_int4",        gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_uint4",       gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_long4",       gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert4_rtzCode},
    {"convert_ulong4",      gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert4_rtzCode},
    {"convert_short4",      gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_ushort4",     gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_float4",      gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_FloatDefaultCode},

    {"convert_char4_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rteCode},
    {"convert_uchar4_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rteCode},
    {"convert_int4_rte",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rteCode},
    {"convert_uint4_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rteCode},
    {"convert_long4_rte",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert4_rteCode},
    {"convert_ulong4_rte",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert4_rteCode},
    {"convert_short4_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rteCode},
    {"convert_ushort4_rte", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rteCode},
    {"convert_float4_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rteCode},

    {"convert_char4_rtz",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_uchar4_rtz",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_int4_rtz",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_uint4_rtz",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_long4_rtz",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert4_rtzCode},
    {"convert_ulong4_rtz",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert4_rtzCode},
    {"convert_short4_rtz",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_ushort4_rtz", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rtzCode},
    {"convert_float4_rtz",  gcvFALSE,    gcvTRUE,  gcvNULL, _GenConvertFloat4_rtzCode},

    {"convert_char4_rtp",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rtpCode},
    {"convert_uchar4_rtp",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rtpCode},
    {"convert_int4_rtp",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rtpCode},
    {"convert_uint4_rtp",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rtpCode},
    {"convert_long4_rtp",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert4_rtpCode},
    {"convert_ulong4_rtp",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert4_rtpCode},
    {"convert_short4_rtp",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rtpCode},
    {"convert_ushort4_rtp", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rtpCode},
    {"convert_float4_rtp",  gcvFALSE,    gcvTRUE,    gcvNULL, _GenConvert4_rtpCode},

    {"convert_char4_rtn",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rtnCode},
    {"convert_uchar4_rtn",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rtnCode},
    {"convert_int4_rtn",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rtnCode},
    {"convert_uint4_rtn",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rtnCode},
    {"convert_long4_rtn",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert4_rtnCode},
    {"convert_ulong4_rtn",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert4_rtnCode},
    {"convert_short4_rtn",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rtnCode},
    {"convert_ushort4_rtn", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_rtnCode},
    {"convert_float4_rtn",  gcvFALSE,    gcvTRUE,    gcvNULL, _GenConvert4_rtnCode},

    /* Conversion functions - saturated mode 4 */
    {"convert_char4_sat",       gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_uchar4_sat",      gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_int4_sat",        gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_uint4_sat",       gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_long4_sat",       gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_ulong4_sat",      gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_short4_sat",      gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_ushort4_sat",     gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rtzCode},

    {"convert_char4_sat_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rteCode},
    {"convert_uchar4_sat_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rteCode},
    {"convert_int4_sat_rte",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rteCode},
    {"convert_uint4_sat_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rteCode},
    {"convert_long4_sat_rte",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert4_sat_rteCode},
    {"convert_ulong4_sat_rte",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert4_sat_rteCode},
    {"convert_short4_sat_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rteCode},
    {"convert_ushort4_sat_rte", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rteCode},

    {"convert_char4_sat_rtz",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_uchar4_sat_rtz",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_int4_sat_rtz",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_uint4_sat_rtz",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_long4_sat_rtz",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_ulong4_sat_rtz",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_short4_sat_rtz",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rtzCode},
    {"convert_ushort4_sat_rtz", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rtzCode},

    {"convert_char4_sat_rtp",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rtpCode},
    {"convert_uchar4_sat_rtp",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rtpCode},
    {"convert_int4_sat_rtp",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rtpCode},
    {"convert_uint4_sat_rtp",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rtpCode},
    {"convert_long4_sat_rtp",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert4_sat_rtpCode},
    {"convert_ulong4_sat_rtp",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert4_sat_rtpCode},
    {"convert_short4_sat_rtp",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rtpCode},
    {"convert_ushort4_sat_rtp", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rtpCode},

    {"convert_char4_sat_rtn",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rtnCode},
    {"convert_uchar4_sat_rtn",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rtnCode},
    {"convert_int4_sat_rtn",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rtnCode},
    {"convert_uint4_sat_rtn",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rtnCode},
    {"convert_long4_sat_rtn",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert4_sat_rtnCode},
    {"convert_ulong4_sat_rtn",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert4_sat_rtnCode},
    {"convert_short4_sat_rtn",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rtnCode},
    {"convert_ushort4_sat_rtn", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert4_sat_rtnCode},

    /* Conversion functions - non-saturated mode 8 */
    {"convert_char8",       gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_uchar8",      gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_int8",        gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_uint8",       gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_long8",       gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert8_rtzCode},
    {"convert_ulong8",      gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert8_rtzCode},
    {"convert_short8",      gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_ushort8",     gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_float8",      gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_FloatDefaultCode},

    {"convert_char8_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rteCode},
    {"convert_uchar8_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rteCode},
    {"convert_int8_rte",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rteCode},
    {"convert_uint8_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rteCode},
    {"convert_long8_rte",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert8_rteCode},
    {"convert_ulong8_rte",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert8_rteCode},
    {"convert_short8_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rteCode},
    {"convert_ushort8_rte", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rteCode},
    {"convert_float8_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rteCode},

    {"convert_char8_rtz",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_uchar8_rtz",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_int8_rtz",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_uint8_rtz",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_long8_rtz",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert8_rtzCode},
    {"convert_ulong8_rtz",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert8_rtzCode},
    {"convert_short8_rtz",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_ushort8_rtz", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rtzCode},
    {"convert_float8_rtz",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rtzCode},

    {"convert_char8_rtp",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rtpCode},
    {"convert_uchar8_rtp",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rtpCode},
    {"convert_int8_rtp",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rtpCode},
    {"convert_uint8_rtp",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rtpCode},
    {"convert_long8_rtp",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert8_rtpCode},
    {"convert_ulong8_rtp",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert8_rtpCode},
    {"convert_short8_rtp",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rtpCode},
    {"convert_ushort8_rtp", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rtpCode},
    {"convert_float8_rtp",  gcvFALSE,    gcvTRUE,    gcvNULL, _GenConvert8_rtpCode},

    {"convert_char8_rtn",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rtnCode},
    {"convert_uchar8_rtn",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rtnCode},
    {"convert_int8_rtn",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rtnCode},
    {"convert_uint8_rtn",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rtnCode},
    {"convert_long8_rtn",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert8_rtnCode},
    {"convert_ulong8_rtn",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert8_rtnCode},
    {"convert_short8_rtn",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rtnCode},
    {"convert_ushort8_rtn", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_rtnCode},
    {"convert_float8_rtn",  gcvFALSE,    gcvTRUE,    gcvNULL, _GenConvert8_rtnCode},

    /* Conversion functions - saturated mode 8 */
    {"convert_char8_sat",       gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_uchar8_sat",      gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_int8_sat",        gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_uint8_sat",       gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_long8_sat",       gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_ulong8_sat",      gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_short8_sat",      gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_ushort8_sat",     gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rtzCode},

    {"convert_char8_sat_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rteCode},
    {"convert_uchar8_sat_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rteCode},
    {"convert_int8_sat_rte",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rteCode},
    {"convert_uint8_sat_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rteCode},
    {"convert_long8_sat_rte",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert8_sat_rteCode},
    {"convert_ulong8_sat_rte",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert8_sat_rteCode},
    {"convert_short8_sat_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rteCode},
    {"convert_ushort8_sat_rte", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rteCode},

    {"convert_char8_sat_rtz",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_uchar8_sat_rtz",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_int8_sat_rtz",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_ulong8_sat_rtz",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_long8_sat_rtz",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_uint8_sat_rtz",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_short8_sat_rtz",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rtzCode},
    {"convert_ushort8_sat_rtz", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rtzCode},

    {"convert_char8_sat_rtp",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rtpCode},
    {"convert_uchar8_sat_rtp",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rtpCode},
    {"convert_int8_sat_rtp",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rtpCode},
    {"convert_uint8_sat_rtp",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rtpCode},
    {"convert_long8_sat_rtp",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert8_sat_rtpCode},
    {"convert_ulong8_sat_rtp",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert8_sat_rtpCode},
    {"convert_short8_sat_rtp",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rtpCode},
    {"convert_ushort8_sat_rtp", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rtpCode},

    {"convert_char8_sat_rtn",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rtnCode},
    {"convert_uchar8_sat_rtn",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rtnCode},
    {"convert_int8_sat_rtn",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rtnCode},
    {"convert_uint8_sat_rtn",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rtnCode},
    {"convert_long8_sat_rtn",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert8_sat_rtnCode},
    {"convert_ulong8_sat_rtn",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert8_sat_rtnCode},
    {"convert_short8_sat_rtn",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rtnCode},
    {"convert_ushort8_sat_rtn", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert8_sat_rtnCode},

    /* Conversion functions - non-saturated mode 16 */
    {"convert_char16",       gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_uchar16",      gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_int16",        gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_uint16",       gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_long16",       gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert16_rtzCode},
    {"convert_ulong16",      gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert16_rtzCode},
    {"convert_short16",      gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_ushort16",     gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_float16",      gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_FloatDefaultCode},

    {"convert_char16_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rteCode},
    {"convert_uchar16_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rteCode},
    {"convert_int16_rte",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rteCode},
    {"convert_uint16_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rteCode},
    {"convert_long16_rte",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert16_rteCode},
    {"convert_ulong16_rte",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert16_rteCode},
    {"convert_short16_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rteCode},
    {"convert_ushort16_rte", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rteCode},
    {"convert_float16_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rteCode},

    {"convert_char16_rtz",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_uchar16_rtz",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_int16_rtz",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_uint16_rtz",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_long16_rtz",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert16_rtzCode},
    {"convert_ulong16_rtz",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert16_rtzCode},
    {"convert_short16_rtz",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_ushort16_rtz", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rtzCode},
    {"convert_float16_rtz",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rtzCode},

    {"convert_char16_rtp",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rtpCode},
    {"convert_uchar16_rtp",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rtpCode},
    {"convert_int16_rtp",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rtpCode},
    {"convert_uint16_rtp",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rtpCode},
    {"convert_long16_rtp",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert16_rtpCode},
    {"convert_ulong16_rtp",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert16_rtpCode},
    {"convert_short16_rtp",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rtpCode},
    {"convert_ushort16_rtp", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rtpCode},
    {"convert_float16_rtp",  gcvFALSE,    gcvTRUE,    gcvNULL, _GenConvert16_rtpCode},

    {"convert_char16_rtn",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rtnCode},
    {"convert_uchar16_rtn",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rtnCode},
    {"convert_int16_rtn",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rtnCode},
    {"convert_uint16_rtn",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rtnCode},
    {"convert_long16_rtn",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert16_rtnCode},
    {"convert_ulong16_rtn",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert16_rtnCode},
    {"convert_short16_rtn",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rtnCode},
    {"convert_ushort16_rtn", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_rtnCode},
    {"convert_float16_rtn",  gcvFALSE,    gcvTRUE,    gcvNULL, _GenConvert16_rtnCode},

    /* Conversion functions - saturated mode 16 */
    {"convert_char16_sat",       gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_uchar16_sat",      gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_int16_sat",        gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_uint16_sat",       gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_long16_sat",       gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_ulong16_sat",      gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_short16_sat",      gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_ushort16_sat",     gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rtzCode},

    {"convert_char16_sat_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rteCode},
    {"convert_uchar16_sat_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rteCode},
    {"convert_int16_sat_rte",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rteCode},
    {"convert_uint16_sat_rte",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rteCode},
    {"convert_long16_sat_rte",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert16_sat_rteCode},
    {"convert_ulong16_sat_rte",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert16_sat_rteCode},
    {"convert_short16_sat_rte",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rteCode},
    {"convert_ushort16_sat_rte", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rteCode},

    {"convert_char16_sat_rtz",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_uchar16_sat_rtz",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_int16_sat_rtz",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_uint16_sat_rtz",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_long16_sat_rtz",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_ulong16_sat_rtz",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_short16_sat_rtz",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rtzCode},
    {"convert_ushort16_sat_rtz", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rtzCode},

    {"convert_char16_sat_rtp",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rtpCode},
    {"convert_uchar16_sat_rtp",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rtpCode},
    {"convert_int16_sat_rtp",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rtpCode},
    {"convert_ulong16_sat_rtp",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert16_sat_rtpCode},
    {"convert_long16_sat_rtp",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert16_sat_rtpCode},
    {"convert_uint16_sat_rtp",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rtpCode},
    {"convert_short16_sat_rtp",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rtpCode},
    {"convert_ushort16_sat_rtp", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rtpCode},

    {"convert_char16_sat_rtn",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rtnCode},
    {"convert_uchar16_sat_rtn",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rtnCode},
    {"convert_int16_sat_rtn",    gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rtnCode},
    {"convert_ulong16_sat_rtn",  gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert16_sat_rtnCode},
    {"convert_long16_sat_rtn",   gcvFALSE,    gcvFALSE,  gcvNULL, _GenConvert16_sat_rtnCode},
    {"convert_uint16_sat_rtn",   gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rtnCode},
    {"convert_short16_sat_rtn",  gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rtnCode},
    {"convert_ushort16_sat_rtn", gcvFALSE,    gcvTRUE,   gcvNULL, _GenConvert16_sat_rtnCode},
#endif

    /* Reinterpreting types */
    {"as_char",       gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},
    {"as_char2",      gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},
    {"as_char3",      gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_Type3Code},
    {"as_char4",      gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},
    {"as_char8",      gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},
    {"as_char16",     gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},

    {"as_uchar",      gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},
    {"as_uchar2",     gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},
    {"as_uchar3",     gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_Type3Code},
    {"as_uchar4",     gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},
    {"as_uchar8",     gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},
    {"as_uchar16",    gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},

    {"as_short",      gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},
    {"as_short2",     gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},
    {"as_short3",     gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_Type3Code},
    {"as_short4",     gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},
    {"as_short8",     gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},
    {"as_short16",    gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},

    {"as_ushort",     gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},
    {"as_ushort2",    gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},
    {"as_ushort3",    gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_Type3Code},
    {"as_ushort4",    gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},
    {"as_ushort8",    gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},
    {"as_ushort16",   gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},

    {"as_int",        gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},
    {"as_int2",       gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},
    {"as_int3",       gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_Type3Code},
    {"as_int4",       gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},
    {"as_int8",       gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},
    {"as_int16",      gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},

    {"as_uint",       gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},
    {"as_uint2",      gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},
    {"as_uint3",      gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_Type3Code},
    {"as_uint4",      gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},
    {"as_uint8",      gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},
    {"as_uint16",     gcvFALSE,    gcvFALSE,    gcvNULL,    _GenAs_TypeCode},

    {"as_long",       gcvFALSE,    gcvFALSE,   gcvNULL,    _GenAs_TypeCode},
    {"as_long2",      gcvFALSE,    gcvFALSE,   gcvNULL,    _GenAs_TypeCode},
    {"as_long3",      gcvFALSE,    gcvFALSE,   gcvNULL,    _GenAs_Type3Code},
    {"as_long4",      gcvFALSE,    gcvFALSE,   gcvNULL,    _GenAs_TypeCode},
    {"as_long8",      gcvFALSE,    gcvFALSE,   gcvNULL,    _GenAs_TypeCode},
    {"as_long16",     gcvFALSE,    gcvFALSE,   gcvNULL,    _GenAs_TypeCode},

    {"as_ulong",      gcvFALSE,    gcvFALSE,   gcvNULL,    _GenAs_TypeCode},
    {"as_ulong2",     gcvFALSE,    gcvFALSE,   gcvNULL,    _GenAs_TypeCode},
    {"as_ulong3",     gcvFALSE,    gcvFALSE,   gcvNULL,    _GenAs_Type3Code},
    {"as_ulong4",     gcvFALSE,    gcvFALSE,   gcvNULL,    _GenAs_TypeCode},
    {"as_ulong8",     gcvFALSE,    gcvFALSE,   gcvNULL,    _GenAs_TypeCode},
    {"as_ulong16",    gcvFALSE,    gcvFALSE,   gcvNULL,    _GenAs_TypeCode},

    {"as_float",      gcvFALSE,    gcvFALSE,   gcvNULL,    _GenAs_TypeCode},
    {"as_float2",     gcvFALSE,    gcvFALSE,   gcvNULL,    _GenAs_TypeCode},
    {"as_float3",     gcvFALSE,    gcvFALSE,   gcvNULL,    _GenAs_Type3Code},
    {"as_float4",     gcvFALSE,    gcvFALSE,   gcvNULL,    _GenAs_TypeCode},
    {"as_float8",     gcvFALSE,    gcvTRUE,    gcvNULL,    _GenAs_TypeCode},
    {"as_float16",    gcvFALSE,    gcvTRUE,    gcvNULL,    _GenAs_TypeCode},

    /* Image functions. */
    {"read_imagef",                 gcvTRUE,       gcvFALSE,       gcvNULL, _GenReadImageFCode},
    {"read_imagei",                 gcvTRUE,       gcvFALSE,       gcvNULL, _GenReadImageICode},
    {"read_imageui",                gcvTRUE,       gcvFALSE,       gcvNULL, _GenReadImageUICode},
    {"write_imagef",                gcvTRUE,       gcvFALSE,       gcvNULL, _GenWriteImageFCode},
    {"write_imagei",                gcvTRUE,       gcvFALSE,       gcvNULL, _GenWriteImageICode},
    {"write_imageui",               gcvTRUE,       gcvFALSE,       gcvNULL, _GenWriteImageUICode},
    {"get_image_width",             gcvTRUE,       gcvFALSE,       gcvNULL, _GenGetImageWidthCode},
    {"get_image_height",            gcvTRUE,       gcvFALSE,       gcvNULL, _GenGetImageHeightCode},
    {"get_image_depth",             gcvTRUE,       gcvFALSE,       gcvNULL, _GenGetImageDepthCode},
    {"get_image_channel_data_type", gcvTRUE,       gcvFALSE,       gcvNULL, _GenGetImageChannelDataTypeCode},
    {"get_image_channel_order",     gcvTRUE,       gcvFALSE,       gcvNULL, _GenGetImageChannelOrderCode},
    {"get_image_dim",               gcvTRUE,       gcvFALSE,       gcvNULL, _GenGetImageDimCode},
    {"get_image_array_size",        gcvTRUE,       gcvFALSE,       gcvNULL, _GenGetImageArrayCode},
    {"viv_texld",                   gcvFALSE,      gcvFALSE,       gcvNULL, _GenVivTexldCode},
    {"viv_read_imagef",             gcvTRUE,       gcvFALSE,       gcvNULL, _GenVivReadImageFCode},
    {"viv_read_imagei",             gcvTRUE,       gcvFALSE,       gcvNULL, _GenVivReadImageICode},
    {"viv_read_imageui",            gcvTRUE,       gcvFALSE,       gcvNULL, _GenVivReadImageUICode},
    {"viv_write_imagef",            gcvTRUE,       gcvFALSE,       gcvNULL, _GenVivWriteImageFCode},
    {"viv_write_imagei",            gcvTRUE,       gcvFALSE,       gcvNULL, _GenVivWriteImageICode},
    {"viv_write_imageui",           gcvTRUE,       gcvFALSE,       gcvNULL, _GenVivWriteImageUICode},

    /* Vector functions. */
    {"vload2",          gcvFALSE,       gcvFALSE,       gcvNULL, _GenVloadCode},
    {"vload3",          gcvFALSE,       gcvFALSE,       gcvNULL, _GenVloadCode},
    {"vload4",          gcvFALSE,       gcvFALSE,       gcvNULL, _GenVloadCode},
    {"vload8",          gcvFALSE,       gcvFALSE,       gcvNULL, _GenVloadCode},
    {"vload16",         gcvFALSE,       gcvFALSE,       gcvNULL, _GenVloadCode},
    {"vload_half",      gcvFALSE,       gcvFALSE,       gcvNULL, _GenVloadHalfCode},
    {"vload_half2",     gcvFALSE,       gcvFALSE,       gcvNULL, _GenVloadHalfCode},
    {"vload_half3",     gcvFALSE,       gcvFALSE,       gcvNULL, _GenVloadHalfCode},
    {"vload_half4",     gcvFALSE,       gcvFALSE,       gcvNULL, _GenVloadHalfCode},
    {"vload_half8",     gcvFALSE,       gcvFALSE,       gcvNULL, _GenVloadHalfCode},
    {"vload_half16",    gcvFALSE,       gcvFALSE,       gcvNULL, _GenVloadHalfCode},
    {"vstore2",         gcvFALSE,       gcvFALSE,       gcvNULL, _GenVstoreCode},
    {"vstore3",         gcvFALSE,       gcvFALSE,       gcvNULL, _GenVstoreCode},
    {"vstore4",         gcvFALSE,       gcvFALSE,       gcvNULL, _GenVstoreCode},
    {"vstore8",         gcvFALSE,       gcvFALSE,       gcvNULL, _GenVstoreCode},
    {"vstore16",        gcvFALSE,       gcvFALSE,       gcvNULL, _GenVstoreCode},
    {"vstore_half",     gcvFALSE,       gcvFALSE,       gcvNULL, _GenVstoreHalfCode},
    {"vstore_half2",    gcvFALSE,       gcvFALSE,       gcvNULL, _GenVstoreHalfCode},
    {"vstore_half3",    gcvFALSE,       gcvFALSE,       gcvNULL, _GenVstoreHalfCode},
    {"vstore_half4",    gcvFALSE,       gcvFALSE,       gcvNULL, _GenVstoreHalfCode},
    {"vstore_half8",    gcvFALSE,       gcvFALSE,       gcvNULL, _GenVstoreHalfCode},
    {"vstore_half16",   gcvFALSE,       gcvFALSE,       gcvNULL, _GenVstoreHalfCode},

    {"viv_getlonglo",   gcvFALSE,       gcvFALSE,       gcvNULL, _GenGetLongLoCode},
    {"viv_getlonglo2",  gcvFALSE,       gcvFALSE,       gcvNULL, _GenGetLongLoCode},
    {"viv_getlonglo3",  gcvFALSE,       gcvFALSE,       gcvNULL, _GenGetLongLoCode},
    {"viv_getlonglo4",  gcvFALSE,       gcvFALSE,       gcvNULL, _GenGetLongLoCode},
    {"viv_getlonglo8",  gcvFALSE,       gcvFALSE,       gcvNULL, _GenGetLongLoCode},
    {"viv_getlonglo16", gcvFALSE,       gcvFALSE,       gcvNULL, _GenGetLongLoCode},

    {"viv_getlonghi",   gcvFALSE,       gcvFALSE,       gcvNULL, _GenGetLongHiCode},
    {"viv_getlonghi2",  gcvFALSE,       gcvFALSE,       gcvNULL, _GenGetLongHiCode},
    {"viv_getlonghi3",  gcvFALSE,       gcvFALSE,       gcvNULL, _GenGetLongHiCode},
    {"viv_getlonghi4",  gcvFALSE,       gcvFALSE,       gcvNULL, _GenGetLongHiCode},
    {"viv_getlonghi8",  gcvFALSE,       gcvFALSE,       gcvNULL, _GenGetLongHiCode},
    {"viv_getlonghi16", gcvFALSE,       gcvFALSE,       gcvNULL, _GenGetLongHiCode},

    {"viv_setlong",     gcvFALSE,       gcvFALSE,       gcvNULL, _GenSetLongCode},
    {"viv_setlong2",    gcvFALSE,       gcvFALSE,       gcvNULL, _GenSetLongCode},
    {"viv_setlong3",    gcvFALSE,       gcvFALSE,       gcvNULL, _GenSetLongCode},
    {"viv_setlong4",    gcvFALSE,       gcvFALSE,       gcvNULL, _GenSetLongCode},
    {"viv_setlong8",    gcvFALSE,       gcvFALSE,       gcvNULL, _GenSetLongCode},
    {"viv_setlong16",   gcvFALSE,       gcvFALSE,       gcvNULL, _GenSetLongCode},

    {"viv_unpack",      gcvFALSE,       gcvFALSE,       gcvNULL, _GenUnpackCode},

    /* Angle and Trigonometry Functions */
    {"radians",         gcvTRUE,       gcvFALSE,       gcvNULL, _GenRadiansCode},
    {"degrees",         gcvTRUE,       gcvFALSE,       gcvNULL, _GenDegreesCode},
    {"half_sin",        gcvTRUE,       gcvTRUE,        gcvNULL, _GenSinCode},
    {"native_sin",      gcvTRUE,       gcvTRUE,        gcvNULL, _GenNativeSinCode},
    {"sin",             gcvTRUE,       gcvTRUE,        gcvNULL, _GenSinCode},
    {"half_cos",        gcvTRUE,       gcvTRUE,        gcvNULL, _GenCosCode},
    {"native_cos",      gcvTRUE,       gcvTRUE,        gcvNULL, _GenNativeCosCode},
    {"cos",             gcvTRUE,       gcvTRUE,        gcvNULL, _GenCosCode},
    {"sincos",          gcvTRUE,       gcvTRUE,        gcvNULL, _GenSinCosCode},
    {"half_tan",        gcvTRUE,       gcvTRUE,        gcvNULL, _GenTanCode},
    {"native_tan",      gcvTRUE,       gcvTRUE,        gcvNULL, _GenNativeTanCode},
    {"tan",             gcvTRUE,       gcvTRUE,        gcvNULL, _GenTanCode},
    {"asin",            gcvTRUE,       gcvTRUE,        gcvNULL, _GenAsinCode},
    {"acos",            gcvTRUE,       gcvTRUE,        gcvNULL, _GenAcosCode},
    {"atan",            gcvTRUE,       gcvTRUE,        gcvNULL, _GenAtanCode},
    {"half_divide",     gcvTRUE,       gcvTRUE,        gcvNULL, _GenDivideCode},
    {"native_divide",   gcvTRUE,       gcvFALSE,       gcvNULL, _GenNativeDivCode},

    {"sinh",            gcvTRUE,       gcvTRUE,        gcvNULL, _GenSinhCode},
    {"cosh",            gcvTRUE,       gcvTRUE,        gcvNULL, _GenCoshCode},
    {"tanh",            gcvTRUE,       gcvTRUE,        gcvNULL, _GenTanhCode},
    {"asinh",           gcvTRUE,       gcvTRUE,        gcvNULL, _GenAsinhCode},
    {"acosh",           gcvTRUE,       gcvTRUE,        gcvNULL, _GenAcoshCode},
    {"atanh",           gcvTRUE,       gcvTRUE,        gcvNULL, _GenAtanhCode},

    {"sinpi",           gcvTRUE,       gcvTRUE,        gcvNULL, _GenSinPiCode},
    {"cospi",           gcvTRUE,       gcvTRUE,        gcvNULL, _GenCosPiCode},
    {"tanpi",           gcvTRUE,       gcvTRUE,        gcvNULL, _GenTanPiCode},
    {"asinpi",          gcvTRUE,       gcvTRUE,        gcvNULL, _GenAsinPiCode},
    {"acospi",          gcvTRUE,       gcvTRUE,        gcvNULL, _GenAcosPiCode},
    {"atanpi",          gcvTRUE,       gcvTRUE,        gcvNULL, _GenAtanPiCode},
    {"atan2",           gcvTRUE,       gcvTRUE,        gcvNULL, _GenAtan2Code},
    {"atan2pi",         gcvTRUE,       gcvTRUE,        gcvNULL, _GenAtan2PiCode},

    {"cbrt",            gcvTRUE,       gcvTRUE,        gcvNULL, _GenCbrtCode},
    {"hypot",           gcvTRUE,       gcvTRUE,        gcvNULL, _GenHypotCode},
    {"tgamma",          gcvTRUE,       gcvTRUE,        gcvNULL, _GenGammaCode},
    {"erfc",            gcvTRUE,       gcvTRUE,        gcvNULL, _GenErfcCode},
    {"erf",             gcvTRUE,       gcvTRUE,        gcvNULL, _GenErfCode},

    /* Exponential Functions */
    {"pow",             gcvTRUE,       gcvTRUE,        gcvNULL, _GenPowCode},
    {"half_powr",       gcvTRUE,       gcvTRUE,        gcvNULL, _GenPowrCode},
    {"native_powr",     gcvTRUE,       gcvTRUE,        gcvNULL, _GenPowrCode},
    {"powr",            gcvTRUE,       gcvTRUE,        gcvNULL, _GenPowrCode},
    {"pown",            gcvTRUE,       gcvTRUE,        gcvNULL, _GenPownCode},
    {"rootn",           gcvTRUE,       gcvTRUE,        gcvNULL, _GenRootnCode},
    {"half_exp",        gcvTRUE,       gcvTRUE,        gcvNULL, _GenExpCode},
    {"native_exp",      gcvTRUE,       gcvTRUE,        gcvNULL, _GenExpCode},
    {"exp",             gcvTRUE,       gcvTRUE,        gcvNULL, _GenExpCode},
    {"half_exp10",      gcvTRUE,       gcvTRUE,        gcvNULL, _GenExp10Code},
    {"native_exp10",    gcvTRUE,       gcvTRUE,        gcvNULL, _GenExp10Code},
    {"exp10",           gcvTRUE,       gcvTRUE,        gcvNULL, _GenExp10Code},
    {"expm1",           gcvTRUE,       gcvTRUE,        gcvNULL, _GenExpm1Code},
    {"half_log",        gcvTRUE,       gcvTRUE,        gcvNULL, _GenLogCode},
    {"native_log",      gcvTRUE,       gcvTRUE,        gcvNULL, _GenLogCode},
    {"log",             gcvTRUE,       gcvTRUE,        gcvNULL, _GenLogCode},
    {"half_exp2",       gcvTRUE,       gcvFALSE,       gcvNULL, _GenExp2Code},
    {"native_exp2",     gcvTRUE,       gcvFALSE,       gcvNULL, _GenExp2Code},
    {"exp2",            gcvTRUE,       gcvFALSE,       gcvNULL, _GenExp2Code},
    {"half_log2",       gcvTRUE,       gcvTRUE,        gcvNULL, _GenLog2Code},
    {"native_log2",     gcvTRUE,       gcvTRUE,        gcvNULL, _GenLog2Code},
    {"log2",            gcvTRUE,       gcvTRUE,        gcvNULL, _GenLog2Code},
    {"half_log10",      gcvTRUE,       gcvTRUE,        gcvNULL, _GenLog10Code},
    {"native_log10",    gcvTRUE,       gcvTRUE,        gcvNULL, _GenLog10Code},
    {"log10",           gcvTRUE,       gcvTRUE,        gcvNULL, _GenLog10Code},
    {"log1p",           gcvTRUE,       gcvTRUE,        gcvNULL, _GenLog1pCode},
    {"half_sqrt",       gcvTRUE,       gcvFALSE,       _EvaluateSqrt,    _GenSqrtCode},
    {"native_sqrt",     gcvTRUE,       gcvFALSE,       _EvaluateSqrt,    _GenSqrtCode},
    {"sqrt",            gcvTRUE,       gcvTRUE,        _EvaluateSqrt,    _GenSqrtCode},
    {"half_rsqrt",      gcvTRUE,       gcvFALSE,       _EvaluateInverseSqrt,    _GenInverseSqrtCode},
    {"native_rsqrt",    gcvTRUE,       gcvFALSE,       _EvaluateInverseSqrt,    _GenInverseSqrtCode},
    {"rsqrt",           gcvTRUE,       gcvFALSE,       _EvaluateInverseSqrt,    _GenInverseSqrtCode},
    {"half_recip",      gcvTRUE,       gcvTRUE,        gcvNULL, _GenNativeInverseCode},
    {"native_recip",    gcvTRUE,       gcvTRUE,        gcvNULL, _GenNativeInverseCode},
    {"reciprocal",      gcvTRUE,       gcvTRUE,        gcvNULL, _GenInverseCode},

    /* Common Functions */

    {"fabs",            gcvTRUE,        gcvFALSE,       gcvNULL, _GenFabsCode},
    {"abs",             gcvTRUE,        gcvFALSE,       gcvNULL, _GenAbsCode},
    {"abs_diff",        gcvTRUE,        gcvFALSE,       gcvNULL, _GenAbsDiffCode},
    {"rotate",          gcvTRUE,        gcvFALSE,       gcvNULL, _GenRotateCode},
    {"hadd",            gcvTRUE,        gcvFALSE,       gcvNULL, _GenHaddCode},
    {"rhadd",           gcvTRUE,        gcvFALSE,       gcvNULL, _GenRhaddCode},
    {"add_sat",         gcvTRUE,        gcvFALSE,       gcvNULL, _GenAddSubSatCode},
    {"sub_sat",         gcvTRUE,        gcvFALSE,       gcvNULL, _GenAddSubSatCode},
    {"mul_hi",          gcvTRUE,        gcvFALSE,       gcvNULL, _GenMulHiCode},
    {"mad_hi",          gcvTRUE,        gcvFALSE,       gcvNULL, _GenMadHiLoCode},
    {"mad_sat",         gcvTRUE,        gcvFALSE,       gcvNULL, _GenMadSatCode},
    {"mul24",           gcvTRUE,        gcvFALSE,       gcvNULL, _GenMul24Code},
    {"mad24",           gcvTRUE,        gcvFALSE,       gcvNULL, _GenMadHiLoCode},
    {"upsample",        gcvTRUE,        gcvFALSE,       gcvNULL, _GenUpsampleCode},
    {"sign",            gcvTRUE,        gcvFALSE,       gcvNULL, _GenSignCode},
    {"floor",           gcvTRUE,        gcvFALSE,       gcvNULL, _GenFloorCode},
    {"rint",            gcvTRUE,        gcvTRUE,        gcvNULL, _GenRintCode},
    {"ceil",            gcvTRUE,        gcvFALSE,       gcvNULL, _GenCeilCode},
    {"fract",           gcvTRUE,        gcvFALSE,       gcvNULL, _GenFractCode},
    {"modf",            gcvTRUE,        gcvFALSE,       gcvNULL, _GenModfCode},
    {"nextafter",       gcvTRUE,        gcvTRUE,        gcvNULL, _GenNextAfterCode},
    {"frexp",           gcvTRUE,        gcvTRUE,        gcvNULL, _GenFrexpCode},
    {"fmod",            gcvTRUE,        gcvTRUE,        gcvNULL, _GenFModCode},
    {"ilogb",           gcvTRUE,        gcvTRUE,        gcvNULL, _GenILogbCode},
    {"logb",            gcvTRUE,        gcvTRUE,        gcvNULL, _GenLogbCode},
    {"nan",             gcvTRUE,        gcvFALSE,       gcvNULL, _GenNanCode},
    {"ldexp",           gcvTRUE,        gcvTRUE,        gcvNULL, _GenLdexpCode},
    {"round",           gcvTRUE,        gcvFALSE,       gcvNULL, _GenRoundCode},
    {"trunc",           gcvTRUE,        gcvFALSE,       gcvNULL, _GenTruncCode},
    {"copysign",        gcvTRUE,        gcvFALSE,       gcvNULL, _GenCopySignCode},
    {"remainder",       gcvTRUE,        gcvTRUE,        gcvNULL, _GenRemainderCode},
    {"remquo",          gcvTRUE,        gcvTRUE,        gcvNULL, _GenRemquoCode},
    {"fmin",            gcvTRUE,        gcvFALSE,       gcvNULL, _GenMinCode},
    {"fmax",            gcvTRUE,        gcvFALSE,       gcvNULL, _GenMaxCode},
    {"min",             gcvTRUE,        gcvFALSE,       gcvNULL, _GenMinCode},
    {"max",             gcvTRUE,        gcvFALSE,       gcvNULL, _GenMaxCode},
    {"clamp",           gcvTRUE,        gcvFALSE,       gcvNULL, _GenClampCode},
    {"fdim",            gcvTRUE,        gcvTRUE,        gcvNULL, _GenFDimCode},
    {"fmix",            gcvTRUE,        gcvFALSE,       gcvNULL, _GenMixCode},
    {"mix",             gcvTRUE,        gcvFALSE,       gcvNULL, _GenMixCode},
    {"step",            gcvTRUE,        gcvFALSE,       gcvNULL, _GenStepCode},
    {"smoothstep",      gcvTRUE,        gcvFALSE,       gcvNULL, _GenSmoothStepCode},
    {"fma",             gcvTRUE,        gcvTRUE,        gcvNULL, _GenFmaCode},
    {"fast_fma",        gcvTRUE,        gcvTRUE,        gcvNULL, _GenFastFmaCode},
    {"mad",             gcvTRUE,        gcvFALSE,       gcvNULL, _GenMadCode},
    {"clz",             gcvTRUE,        gcvFALSE,       gcvNULL, _GenClzCode},
    {"popcount",        gcvTRUE,        gcvFALSE,       gcvNULL, _GenPopcountCode},

    /* Geometric Functions */
    {"fast_length",     gcvTRUE,        gcvFALSE,       gcvNULL, _GenFastLengthCode},
    {"length",          gcvTRUE,        gcvFALSE,       gcvNULL, _GenLengthCode},
    {"distance",        gcvTRUE,        gcvFALSE,       gcvNULL, _GenDistanceCode},
    {"fast_distance",   gcvTRUE,        gcvFALSE,       gcvNULL, _GenFastDistanceCode},
    {"dot",             gcvTRUE,        gcvFALSE,       gcvNULL, _GenDotCode},
    {"cross",           gcvTRUE,        gcvFALSE,       gcvNULL, _GenCrossCode},
    {"normalize",       gcvTRUE,        gcvFALSE,       gcvNULL, _GenNormalizeCode},
    {"fast_normalize",  gcvTRUE,        gcvFALSE,       gcvNULL, _GenFastNormalizeCode},
    {"faceforward",     gcvTRUE,        gcvTRUE,       gcvNULL, _GenFaceForwardCode},
    {"reflect",         gcvTRUE,        gcvFALSE,       gcvNULL, _GenReflectCode},
    {"refract",         gcvTRUE,        gcvFALSE,       gcvNULL, _GenRefractCode},

    /* Matrix Functions */
    {"matrixCompMult",  gcvTRUE,        gcvFALSE,       gcvNULL, _GenMatrixCompMultCode},

    /* Async copy and prefetch */
    {"async_work_group_copy",           gcvFALSE,       gcvFALSE,   gcvNULL, _GenAsyncCopyCode},
    {"async_work_group_strided_copy",   gcvFALSE,       gcvFALSE,   gcvNULL, _GenAsyncCopyStridedCode},
    {"wait_group_events",               gcvFALSE,       gcvFALSE,   gcvNULL, _GenWaitGroupEventsCode},
    {"prefetch",        gcvFALSE,       gcvFALSE,       gcvNULL, _GenPrefetchCode},

    {"radians",         gcvTRUE,       gcvFALSE,       gcvNULL, _GenRadiansCode},
    {"degrees",         gcvTRUE,       gcvFALSE,       gcvNULL, _GenDegreesCode},
    {"half_sin",        gcvTRUE,       gcvTRUE,        gcvNULL, _GenSinCode},
    {"native_sin",      gcvTRUE,       gcvTRUE,        gcvNULL, _GenNativeSinCode},
    {"sin",             gcvTRUE,       gcvTRUE,        gcvNULL, _GenSinCode},
    {"half_cos",        gcvTRUE,       gcvTRUE,        gcvNULL, _GenCosCode},
    {"native_cos",      gcvTRUE,       gcvTRUE,        gcvNULL, _GenNativeCosCode},
    {"cos",             gcvTRUE,       gcvTRUE,        gcvNULL, _GenCosCode},
    {"sincos",          gcvTRUE,       gcvTRUE,        gcvNULL, _GenSinCosCode},
    {"half_tan",        gcvTRUE,       gcvTRUE,        gcvNULL, _GenTanCode},
    {"native_tan",      gcvTRUE,       gcvTRUE,        gcvNULL, _GenNativeTanCode},
    {"tan",             gcvTRUE,       gcvTRUE,        gcvNULL, _GenTanCode},
    {"native#asin",     gcvFALSE,       gcvTRUE,        gcvNULL, _GenNativeAsinCode},
    {"asin",            gcvTRUE,       gcvTRUE,        gcvNULL, _GenAsinCode},
    {"native#acos",     gcvFALSE,       gcvTRUE,        gcvNULL, _GenNativeAcosCode},
    {"acos",            gcvTRUE,       gcvTRUE,        gcvNULL, _GenAcosCode},
    {"native#atan",     gcvFALSE,       gcvTRUE,        gcvNULL, _GenNativeAtanCode},
    {"atan",            gcvTRUE,       gcvTRUE,        gcvNULL, _GenAtanCode},
    {"half_divide",     gcvTRUE,       gcvTRUE,        gcvNULL, _GenDivideCode},
    {"native_divide",   gcvTRUE,       gcvFALSE,       gcvNULL, _GenNativeDivCode},

    {"sinh",            gcvTRUE,       gcvTRUE,        gcvNULL, _GenSinhCode},
    {"cosh",            gcvTRUE,       gcvTRUE,        gcvNULL, _GenCoshCode},
    {"tanh",            gcvTRUE,       gcvTRUE,        gcvNULL, _GenTanhCode},
    {"asinh",           gcvTRUE,       gcvTRUE,        gcvNULL, _GenAsinhCode},
    {"acosh",           gcvTRUE,       gcvTRUE,        gcvNULL, _GenAcoshCode},
    {"atanh",           gcvTRUE,       gcvTRUE,        gcvNULL, _GenAtanhCode},

    {"sinpi",           gcvTRUE,       gcvTRUE,        gcvNULL, _GenSinPiCode},
    {"cospi",           gcvTRUE,       gcvTRUE,        gcvNULL, _GenCosPiCode},
    {"tanpi",           gcvTRUE,       gcvTRUE,        gcvNULL, _GenTanPiCode},
    {"asinpi",          gcvTRUE,       gcvTRUE,        gcvNULL, _GenAsinPiCode},
    {"acospi",          gcvTRUE,       gcvTRUE,        gcvNULL, _GenAcosPiCode},
    {"atanpi",          gcvTRUE,       gcvTRUE,        gcvNULL, _GenAtanPiCode},
    {"atan2",           gcvTRUE,       gcvTRUE,        gcvNULL, _GenAtan2Code},
    {"atan2pi",         gcvTRUE,       gcvTRUE,        gcvNULL, _GenAtan2PiCode},

    {"cbrt",            gcvTRUE,       gcvTRUE,        gcvNULL, _GenCbrtCode},
    {"hypot",           gcvTRUE,       gcvTRUE,        gcvNULL, _GenHypotCode},
    {"tgamma",          gcvTRUE,       gcvTRUE,        gcvNULL, _GenGammaCode},
    {"erfc",            gcvTRUE,       gcvTRUE,        gcvNULL, _GenErfcCode},
    {"erf",             gcvTRUE,       gcvTRUE,        gcvNULL, _GenErfCode},

    /* Exponential Functions */
    {"pow",             gcvTRUE,       gcvTRUE,        gcvNULL, _GenPowCode},
    {"half_powr",       gcvTRUE,       gcvTRUE,        gcvNULL, _GenPowrCode},
    {"native_powr",     gcvTRUE,       gcvTRUE,        gcvNULL, _GenPowrCode},
    {"powr",            gcvTRUE,       gcvTRUE,        gcvNULL, _GenPowrCode},
    {"pown",            gcvTRUE,       gcvTRUE,        gcvNULL, _GenPownCode},
    {"rootn",           gcvTRUE,       gcvTRUE,        gcvNULL, _GenRootnCode},
    {"half_exp",        gcvTRUE,       gcvTRUE,        gcvNULL, _GenExpCode},
    {"native_exp",      gcvTRUE,       gcvTRUE,        gcvNULL, _GenExpCode},
    {"exp",             gcvTRUE,       gcvTRUE,        gcvNULL, _GenExpCode},
    {"half_exp10",      gcvTRUE,       gcvTRUE,        gcvNULL, _GenExp10Code},
    {"native_exp10",    gcvTRUE,       gcvTRUE,        gcvNULL, _GenExp10Code},
    {"exp10",           gcvTRUE,       gcvTRUE,        gcvNULL, _GenExp10Code},
    {"expm1",           gcvTRUE,       gcvTRUE,        gcvNULL, _GenExpm1Code},
    {"half_log",        gcvTRUE,       gcvTRUE,        gcvNULL, _GenLogCode},
    {"native_log",      gcvTRUE,       gcvTRUE,        gcvNULL, _GenLogCode},
    {"log",             gcvTRUE,       gcvTRUE,        gcvNULL, _GenLogCode},
    {"half_exp2",       gcvTRUE,       gcvFALSE,       gcvNULL, _GenExp2Code},
    {"native_exp2",     gcvTRUE,       gcvFALSE,       gcvNULL, _GenExp2Code},
    {"exp2",            gcvTRUE,       gcvFALSE,       gcvNULL, _GenExp2Code},
    {"half_log2",       gcvTRUE,       gcvTRUE,        gcvNULL, _GenLog2Code},
    {"native_log2",     gcvTRUE,       gcvTRUE,        gcvNULL, _GenLog2Code},
    {"log2",            gcvTRUE,       gcvTRUE,        gcvNULL, _GenLog2Code},
    {"half_log10",      gcvTRUE,       gcvTRUE,        gcvNULL, _GenLog10Code},
    {"native_log10",    gcvTRUE,       gcvTRUE,        gcvNULL, _GenLog10Code},
    {"log10",           gcvTRUE,       gcvTRUE,        gcvNULL, _GenLog10Code},
    {"log1p",           gcvTRUE,       gcvTRUE,        gcvNULL, _GenLog1pCode},
    {"half_sqrt",       gcvTRUE,       gcvFALSE,       _EvaluateSqrt,    _GenSqrtCode},
    {"native_sqrt",     gcvTRUE,       gcvFALSE,       _EvaluateSqrt,    _GenSqrtCode},
    {"sqrt",            gcvTRUE,       gcvTRUE,        _EvaluateSqrt,    _GenSqrtCode},
    {"half_rsqrt",      gcvTRUE,       gcvFALSE,       _EvaluateInverseSqrt, _GenInverseSqrtCode},
    {"native_rsqrt",    gcvTRUE,       gcvFALSE,       _EvaluateInverseSqrt, _GenInverseSqrtCode},
    {"rsqrt",           gcvTRUE,       gcvFALSE,       _EvaluateInverseSqrt, _GenInverseSqrtCode},
    {"half_recip",      gcvTRUE,       gcvTRUE,        gcvNULL, _GenNativeInverseCode},
    {"native_recip",    gcvTRUE,       gcvTRUE,        gcvNULL, _GenNativeInverseCode},
    {"reciprocal",      gcvTRUE,       gcvTRUE,        gcvNULL, _GenInverseCode},
    {"add#",            gcvFALSE,       gcvFALSE,       gcvNULL, _GenAddSubMulCode},
    {"sub#",            gcvFALSE,       gcvFALSE,       gcvNULL, _GenAddSubMulCode},
    {"mul#",            gcvFALSE,       gcvFALSE,       gcvNULL, _GenAddSubMulCode},
    {"divide#",         gcvFALSE,       gcvTRUE,        gcvNULL, _GenDivideCode},
    {"divide_int#",     gcvFALSE,       gcvTRUE,        gcvNULL, _GenModCode},

    {"viv_add_lo",      gcvFALSE,       gcvTRUE,        gcvNULL, _GenNativeAddLoCode},
    {"viv_mul_lo",      gcvFALSE,       gcvTRUE,        gcvNULL, _GenNativeMulLoCode},

    {"viv_add_rtz",     gcvFALSE,       gcvTRUE,        gcvNULL, _GenNativeAddRtzCode},
    {"viv_sub_rtz",     gcvFALSE,       gcvTRUE,        gcvNULL, _GenNativeSubRtzCode},

    {"viv_mul_rtz",     gcvFALSE,       gcvTRUE,        gcvNULL, _GenNativeMulRtzCode},

    {"viv_cmad",            gcvFALSE,       gcvFALSE,        gcvNULL, _GenCMADCode},
    {"viv_cmul",            gcvFALSE,       gcvFALSE,        gcvNULL, _GenCMULCode},
    {"viv_cadd",            gcvFALSE,       gcvFALSE,        gcvNULL, _GenCADDCode},
    {"viv_ccj",             gcvFALSE,       gcvFALSE,        gcvNULL, _GenCCJCode},
    {"viv_cmadcj",          gcvFALSE,       gcvFALSE,        gcvNULL, _GenCMADCJCode},
    {"viv_cmulcj",          gcvFALSE,       gcvFALSE,        gcvNULL, _GenCMULCJCode},
    {"viv_caddcj",          gcvFALSE,       gcvFALSE,        gcvNULL, _GenCADDCJCode},
    {"viv_csubcj",          gcvFALSE,       gcvFALSE,        gcvNULL, _GenCSUBCJCode},

    {"viv_findLSB",         gcvFALSE,       gcvFALSE,        gcvNULL, _GenFindLSBCode},
    {"viv_findMSB",         gcvFALSE,       gcvFALSE,        gcvNULL, _GenFindMSBCode},
    {"viv_bitfieldReverse", gcvFALSE,       gcvFALSE,        gcvNULL, _GenBitReversalCode},
    {"viv_byteReverse",     gcvFALSE,       gcvFALSE,        gcvNULL, _GenByteReversalCode},
    {"viv_bitfieldExtract", gcvFALSE,       gcvFALSE,        gcvNULL, _GenBitExtractCode},
    {"viv_bitfieldInsert",  gcvFALSE,       gcvFALSE,        gcvNULL, _GenBitInsertCode},

    /* Relational Functions */
    {"isequal",         gcvTRUE,        gcvTRUE,        gcvNULL, _GenEqualCode},
    {"isnotequal",      gcvTRUE,        gcvTRUE,        gcvNULL, _GenNotEqualCode},
    {"isgreater",       gcvTRUE,        gcvTRUE,        gcvNULL, _GenGreaterThanCode},
    {"isgreaterequal",  gcvTRUE,        gcvTRUE,        gcvNULL, _GenGreaterThanEqualCode},
    {"isless",          gcvTRUE,        gcvTRUE,        gcvNULL, _GenLessThanCode},
    {"islessequal",     gcvTRUE,        gcvTRUE,        gcvNULL, _GenLessThanEqualCode},
    {"islessgreater",   gcvTRUE,        gcvTRUE,        gcvNULL, _GenLessGreaterCode},
    {"isordered",       gcvTRUE,        gcvTRUE,        gcvNULL, _GenOrderedCode},
    {"isunordered",     gcvTRUE,        gcvTRUE,        gcvNULL, _GenUnOrderedCode},
    {"isfinite",        gcvTRUE,        gcvTRUE,        gcvNULL, _GenFiniteCode},
    {"isnan",           gcvTRUE,        gcvTRUE,        gcvNULL, _GenIsNanCode},
    {"isinf",           gcvTRUE,        gcvTRUE,        gcvNULL, _GenIsInfCode},
    {"isnormal",        gcvTRUE,        gcvTRUE,        gcvNULL, _GenIsNormalCode},
    {"signbit",         gcvTRUE,        gcvFALSE,       gcvNULL, _GenSignBitCode},
    {"lgamma",          gcvTRUE,        gcvFALSE,       gcvNULL, _GenLGammaCode},
    {"lgamma_r",        gcvTRUE,        gcvFALSE,       gcvNULL, _GenLGamma_RCode},
    {"shuffle",         gcvTRUE,        gcvFALSE,       gcvNULL, _GenShuffleCode},
    {"shuffle2",        gcvFALSE,       gcvFALSE,       gcvNULL, _GenShuffle2Code},
    {"maxmag",          gcvTRUE,        gcvFALSE,       gcvNULL, _GenMaxMagCode},
    {"minmag",          gcvTRUE,        gcvFALSE,       gcvNULL, _GenMinMagCode},

    {"select",          gcvTRUE,        gcvFALSE,       gcvNULL, _GenSelectCode},
    {"any",             gcvTRUE,        gcvFALSE,       gcvNULL, _GenAnyAllCode},
    {"all",             gcvTRUE,        gcvFALSE,       gcvNULL, _GenAnyAllCode},
    {"bitselect",       gcvTRUE,        gcvFALSE,       gcvNULL, _GenBitSelectCode},

    /* Vivante primitive Angle and Trigonometry Functions */
    {"viv_radians",         gcvFALSE,       gcvFALSE,       gcvNULL, _GenRadiansCode},
    {"viv_degrees",         gcvFALSE,       gcvFALSE,       gcvNULL, _GenDegreesCode},
    {"viv_half_sin",        gcvFALSE,       gcvTRUE,        gcvNULL, _GenSinCode},
    {"viv_native_sin",      gcvFALSE,       gcvTRUE,        gcvNULL, _GenNativeSinCode},
    {"viv_sin",             gcvFALSE,       gcvTRUE,        gcvNULL, _GenSinCode},
    {"viv_half_cos",        gcvFALSE,       gcvTRUE,        gcvNULL, _GenCosCode},
    {"viv_native_cos",      gcvFALSE,       gcvTRUE,        gcvNULL, _GenNativeCosCode},
    {"viv_cos",             gcvFALSE,       gcvTRUE,        gcvNULL, _GenCosCode},
    {"viv_sincos",          gcvFALSE,       gcvTRUE,        gcvNULL, _GenSinCosCode},
    {"viv_half_tan",        gcvFALSE,       gcvTRUE,        gcvNULL, _GenTanCode},
    {"viv_native_tan",      gcvFALSE,       gcvTRUE,        gcvNULL, _GenNativeTanCode},
    {"viv_tan",             gcvFALSE,       gcvTRUE,        gcvNULL, _GenTanCode},
    {"native#asin",         gcvFALSE,       gcvTRUE,        gcvNULL, _GenNativeAsinCode},
    {"viv_asin",            gcvFALSE,       gcvTRUE,        gcvNULL, _GenAsinCode},
    {"native#acos",         gcvFALSE,       gcvTRUE,        gcvNULL, _GenNativeAcosCode},
    {"viv_acos",            gcvFALSE,       gcvTRUE,        gcvNULL, _GenAcosCode},
    {"native#atan",         gcvFALSE,       gcvTRUE,        gcvNULL, _GenNativeAtanCode},
    {"viv_atan",            gcvFALSE,       gcvTRUE,        gcvNULL, _GenAtanCode},
    {"viv_half_divide",     gcvFALSE,       gcvTRUE,        gcvNULL, _GenDivideCode},
    {"viv_native_divide",   gcvFALSE,       gcvFALSE,       gcvNULL, _GenNativeDivCode},

    {"viv_sinh",            gcvFALSE,       gcvTRUE,        gcvNULL, _GenSinhCode},
    {"viv_cosh",            gcvFALSE,       gcvTRUE,        gcvNULL, _GenCoshCode},
    {"viv_tanh",            gcvFALSE,       gcvTRUE,        gcvNULL, _GenTanhCode},
    {"viv_asinh",           gcvFALSE,       gcvTRUE,        gcvNULL, _GenAsinhCode},
    {"viv_acosh",           gcvFALSE,       gcvTRUE,        gcvNULL, _GenAcoshCode},
    {"viv_atanh",           gcvFALSE,       gcvTRUE,        gcvNULL, _GenAtanhCode},

    {"viv_sinpi",           gcvFALSE,       gcvTRUE,        gcvNULL, _GenSinPiCode},
    {"viv_cospi",           gcvFALSE,       gcvTRUE,        gcvNULL, _GenCosPiCode},
    {"viv_tanpi",           gcvFALSE,       gcvTRUE,        gcvNULL, _GenTanPiCode},
    {"viv_asinpi",          gcvFALSE,       gcvTRUE,        gcvNULL, _GenAsinPiCode},
    {"viv_acospi",          gcvFALSE,       gcvTRUE,        gcvNULL, _GenAcosPiCode},
    {"viv_atanpi",          gcvFALSE,       gcvTRUE,        gcvNULL, _GenAtanPiCode},
    {"viv_atan2",           gcvFALSE,       gcvTRUE,        gcvNULL, _GenAtan2Code},
    {"viv_atan2pi",         gcvFALSE,       gcvTRUE,        gcvNULL, _GenAtan2PiCode},

    {"viv_cbrt",            gcvFALSE,       gcvTRUE,        gcvNULL, _GenCbrtCode},
    {"viv_hypot",           gcvFALSE,       gcvTRUE,        gcvNULL, _GenHypotCode},
    {"viv_tgamma",          gcvFALSE,       gcvTRUE,        gcvNULL, _GenGammaCode},
    {"viv_erfc",            gcvFALSE,       gcvTRUE,        gcvNULL, _GenErfcCode},
    {"viv_erf",             gcvFALSE,       gcvTRUE,        gcvNULL, _GenErfCode},

    /* Vivante Primitive Exponential Functions */
    {"viv_pow",             gcvFALSE,       gcvTRUE,        gcvNULL, _GenPowCode},
    {"viv_half_powr",       gcvFALSE,       gcvTRUE,        gcvNULL, _GenPowrCode},
    {"viv_native_powr",     gcvFALSE,       gcvTRUE,        gcvNULL, _GenPowrCode},
    {"viv_powr",            gcvFALSE,       gcvTRUE,        gcvNULL, _GenPowrCode},
    {"viv_pown",            gcvFALSE,       gcvTRUE,        gcvNULL, _GenPownCode},
    {"viv_rootn",           gcvFALSE,       gcvTRUE,        gcvNULL, _GenRootnCode},
    {"viv_half_exp",        gcvFALSE,       gcvTRUE,        gcvNULL, _GenExpCode},
    {"viv_native_exp",      gcvFALSE,       gcvTRUE,        gcvNULL, _GenExpCode},
    {"viv_exp",             gcvFALSE,       gcvTRUE,        gcvNULL, _GenExpCode},
    {"viv_half_exp10",      gcvFALSE,       gcvTRUE,        gcvNULL, _GenExp10Code},
    {"viv_native_exp10",    gcvFALSE,       gcvTRUE,        gcvNULL, _GenExp10Code},
    {"viv_exp10",           gcvFALSE,       gcvTRUE,        gcvNULL, _GenExp10Code},
    {"viv_expm1",           gcvFALSE,       gcvTRUE,        gcvNULL, _GenExpm1Code},
    {"viv_half_log",        gcvFALSE,       gcvTRUE,        gcvNULL, _GenLogCode},
    {"viv_native_log",      gcvFALSE,       gcvTRUE,        gcvNULL, _GenLogCode},
    {"viv_log",             gcvFALSE,       gcvTRUE,        gcvNULL, _GenLogCode},
    {"viv_half_exp2",       gcvFALSE,       gcvFALSE,       gcvNULL, _GenExp2Code},
    {"viv_native_exp2",     gcvFALSE,       gcvFALSE,       gcvNULL, _GenExp2Code},
    {"viv_exp2",            gcvFALSE,       gcvFALSE,       gcvNULL, _GenExp2Code},
    {"viv_half_log2",       gcvFALSE,       gcvTRUE,        gcvNULL, _GenLog2Code},
    {"viv_native_log2",     gcvFALSE,       gcvTRUE,        gcvNULL, _GenLog2Code},
    {"viv_log2",            gcvFALSE,       gcvTRUE,        gcvNULL, _GenLog2Code},
    {"viv_half_log10",      gcvFALSE,       gcvTRUE,        gcvNULL, _GenLog10Code},
    {"viv_native_log10",    gcvFALSE,       gcvTRUE,        gcvNULL, _GenLog10Code},
    {"viv_log10",           gcvFALSE,       gcvTRUE,        gcvNULL, _GenLog10Code},
    {"viv_log1p",           gcvFALSE,       gcvTRUE,        gcvNULL, _GenLog1pCode},
    {"viv_half_sqrt",       gcvFALSE,       gcvFALSE,       gcvNULL, _GenSqrtCode},
    {"viv_native_sqrt",     gcvFALSE,       gcvFALSE,       gcvNULL, _GenSqrtCode},
    {"viv_sqrt",            gcvFALSE,       gcvTRUE,        gcvNULL, _GenSqrtCode},
    {"viv_half_rsqrt",      gcvFALSE,       gcvFALSE,       gcvNULL, _GenInverseSqrtCode},
    {"viv_native_rsqrt",    gcvFALSE,       gcvFALSE,       gcvNULL, _GenInverseSqrtCode},
    {"viv_rsqrt",           gcvFALSE,       gcvFALSE,       gcvNULL, _GenInverseSqrtCode},
    {"viv_half_recip",      gcvFALSE,       gcvTRUE,        gcvNULL, _GenNativeInverseCode},
    {"viv_native_recip",    gcvFALSE,       gcvTRUE,        gcvNULL, _GenNativeInverseCode},
    {"viv_reciprocal",      gcvFALSE,       gcvTRUE,        gcvNULL, _GenInverseCode},
    {"add#",                gcvFALSE,       gcvFALSE,       gcvNULL, _GenAddSubMulCode},
    {"sub#",                gcvFALSE,       gcvFALSE,       gcvNULL, _GenAddSubMulCode},
    {"mul#",                gcvFALSE,       gcvFALSE,       gcvNULL, _GenAddSubMulCode},
    {"divide#",             gcvFALSE,       gcvTRUE,        gcvNULL, _GenDivideCode},
    {"divide_int#",         gcvFALSE,       gcvTRUE,        gcvNULL, _GenModCode},

    {"viv_add_lo",          gcvFALSE,       gcvTRUE,        gcvNULL, _GenNativeAddLoCode},
    {"viv_mul_lo",          gcvFALSE,       gcvTRUE,        gcvNULL, _GenNativeMulLoCode},

    {"viv_add_rtz",         gcvFALSE,       gcvTRUE,        gcvNULL, _GenNativeAddRtzCode},
    {"viv_sub_rtz",         gcvFALSE,       gcvTRUE,        gcvNULL, _GenNativeSubRtzCode},

    {"viv_mul_rtz",         gcvFALSE,       gcvTRUE,        gcvNULL, _GenNativeMulRtzCode},

    /* Vivante Primitive Common Functions */

    {"viv_fabs",        gcvFALSE,        gcvFALSE,       gcvNULL, _GenFabsCode},
    {"viv_abs",         gcvFALSE,        gcvFALSE,       gcvNULL, _GenAbsCode},
    {"left_shift#",     gcvFALSE,        gcvFALSE,       gcvNULL, _GenShiftCode},
    {"right_shift#",    gcvFALSE,        gcvFALSE,       gcvNULL, _GenShiftCode},
    {"viv_abs_diff",    gcvFALSE,        gcvFALSE,       gcvNULL, _GenAbsDiffCode},
    {"viv_rotate",      gcvFALSE,        gcvFALSE,       gcvNULL, _GenRotateCode},
    {"viv_hadd",        gcvFALSE,        gcvFALSE,       gcvNULL, _GenHaddCode},
    {"viv_rhadd",       gcvFALSE,        gcvFALSE,       gcvNULL, _GenRhaddCode},
    {"viv_add_sat",     gcvFALSE,        gcvFALSE,       gcvNULL, _GenAddSubSatCode},
    {"viv_sub_sat",     gcvFALSE,        gcvFALSE,       gcvNULL, _GenAddSubSatCode},
    {"viv_mul_hi",      gcvFALSE,        gcvFALSE,       gcvNULL, _GenMulHiCode},
    {"viv_mad_hi",      gcvFALSE,        gcvFALSE,       gcvNULL, _GenMadHiLoCode},
    {"viv_mad_sat",     gcvFALSE,        gcvFALSE,       gcvNULL, _GenMadSatCode},
    {"viv_mul24",       gcvFALSE,        gcvFALSE,       gcvNULL, _GenMul24Code},
    {"mul_int#",        gcvFALSE,        gcvFALSE,       gcvNULL, _GenMul24Code},
    {"viv_mad24",       gcvFALSE,        gcvFALSE,       gcvNULL, _GenMadHiLoCode},
    {"viv_upsample",    gcvFALSE,        gcvFALSE,       gcvNULL, _GenUpsampleCode},
    {"viv_sign",        gcvFALSE,        gcvFALSE,       gcvNULL, _GenSignCode},
    {"viv_floor",       gcvFALSE,        gcvFALSE,       gcvNULL, _GenFloorCode},
    {"viv_rint",        gcvFALSE,        gcvTRUE,        gcvNULL, _GenRintCode},
    {"viv_ceil",        gcvFALSE,        gcvFALSE,       gcvNULL, _GenCeilCode},
    {"viv_fract",       gcvFALSE,        gcvFALSE,       gcvNULL, _GenFractCode},
    {"viv_modf",        gcvFALSE,        gcvFALSE,       gcvNULL, _GenModfCode},
    {"viv_nextafter",   gcvFALSE,        gcvTRUE,        gcvNULL, _GenNextAfterCode},
    {"viv_frexp",       gcvFALSE,        gcvTRUE,        gcvNULL, _GenFrexpCode},
    {"viv_fmod",        gcvFALSE,        gcvTRUE,        gcvNULL, _GenFModCode},
    {"viv_ilogb",       gcvFALSE,        gcvTRUE,        gcvNULL, _GenILogbCode},
    {"viv_logb",        gcvFALSE,        gcvTRUE,        gcvNULL, _GenLogbCode},
    {"viv_nan",         gcvFALSE,        gcvFALSE,       gcvNULL, _GenNanCode},
    {"viv_ldexp",       gcvFALSE,        gcvTRUE,        gcvNULL, _GenLdexpCode},
    {"viv_round",       gcvFALSE,        gcvFALSE,       gcvNULL, _GenRoundCode},
    {"viv_trunc",       gcvFALSE,        gcvFALSE,       gcvNULL, _GenTruncCode},
    {"viv_copysign",    gcvFALSE,        gcvFALSE,       gcvNULL, _GenCopySignCode},
    {"viv_remainder",   gcvFALSE,        gcvTRUE,        gcvNULL, _GenRemainderCode},
    {"viv_remquo",      gcvFALSE,        gcvTRUE,        gcvNULL, _GenRemquoCode},
    {"mod#",            gcvFALSE,        gcvTRUE,        _EvaluateMod, _GenModCode},
    {"viv_fmin",        gcvFALSE,        gcvFALSE,       gcvNULL, _GenMinCode},
    {"viv_fmax",        gcvFALSE,        gcvFALSE,       gcvNULL, _GenMaxCode},
    {"viv_min",         gcvFALSE,        gcvFALSE,       gcvNULL, _GenMinCode},
    {"viv_max",         gcvFALSE,        gcvFALSE,       gcvNULL, _GenMaxCode},
    {"viv_clamp",       gcvFALSE,        gcvFALSE,       gcvNULL, _GenClampCode},
    {"viv_fdim",        gcvFALSE,        gcvTRUE,        gcvNULL, _GenFDimCode},
    {"viv_fmix",        gcvFALSE,        gcvFALSE,       gcvNULL, _GenMixCode},
    {"viv_mix",         gcvFALSE,        gcvFALSE,       gcvNULL, _GenMixCode},
    {"viv_step",        gcvFALSE,        gcvFALSE,       gcvNULL, _GenStepCode},
    {"viv_smoothstep",  gcvFALSE,        gcvFALSE,       gcvNULL, _GenSmoothStepCode},
    {"viv_fma",         gcvFALSE,        gcvTRUE,        gcvNULL, _GenFmaCode},
    {"fma#",            gcvFALSE,        gcvFALSE,       gcvNULL, _GenFmaPoundCode},
    {"viv_mad",         gcvFALSE,        gcvFALSE,       gcvNULL, _GenMadCode},
    {"viv_clz",         gcvFALSE,        gcvFALSE,       gcvNULL, _GenClzCode},

    /* Vivante Primitive Geometric Functions */
    {"viv_fast_length",     gcvFALSE,        gcvFALSE,       gcvNULL, _GenFastLengthCode},
    {"viv_length",          gcvFALSE,        gcvFALSE,       gcvNULL, _GenLengthCode},
    {"viv_distance",        gcvFALSE,        gcvFALSE,       gcvNULL, _GenDistanceCode},
    {"viv_fast_distance",   gcvFALSE,        gcvFALSE,       gcvNULL, _GenFastDistanceCode},
    {"viv_dot",             gcvFALSE,        gcvFALSE,       gcvNULL, _GenDotCode},
    {"viv_cross",           gcvFALSE,        gcvFALSE,       gcvNULL, _GenCrossCode},
    {"viv_normalize",       gcvFALSE,        gcvFALSE,       gcvNULL, _GenNormalizeCode},
    {"viv_fast_normalize",  gcvFALSE,        gcvFALSE,       gcvNULL, _GenFastNormalizeCode},
    {"viv_faceforward",     gcvFALSE,        gcvTRUE,       gcvNULL, _GenFaceForwardCode},
    {"viv_reflect",         gcvFALSE,        gcvFALSE,       gcvNULL, _GenReflectCode},
    {"viv_refract",         gcvFALSE,        gcvFALSE,       gcvNULL, _GenRefractCode},

    /* Vivante primitive Relational Functions */
    {"viv_isequal",         gcvFALSE,        gcvTRUE,        gcvNULL, _GenEqualCode},
    {"viv_isnotequal",      gcvFALSE,        gcvTRUE,        gcvNULL, _GenNotEqualCode},
    {"viv_isgreater",       gcvFALSE,        gcvTRUE,        gcvNULL, _GenGreaterThanCode},
    {"viv_isgreaterequal",  gcvFALSE,        gcvTRUE,        gcvNULL, _GenGreaterThanEqualCode},
    {"viv_isless",          gcvFALSE,        gcvTRUE,        gcvNULL, _GenLessThanCode},
    {"viv_islessequal",     gcvFALSE,        gcvTRUE,        gcvNULL, _GenLessThanEqualCode},
    {"viv_islessgreater",   gcvFALSE,        gcvTRUE,        gcvNULL, _GenLessGreaterCode},
    {"viv_isordered",       gcvFALSE,        gcvTRUE,        gcvNULL, _GenOrderedCode},
    {"viv_isunordered",     gcvFALSE,        gcvTRUE,        gcvNULL, _GenUnOrderedCode},
    {"viv_isfinite",        gcvFALSE,        gcvTRUE,        gcvNULL, _GenFiniteCode},
    {"viv_isnan",           gcvFALSE,        gcvTRUE,        gcvNULL, _GenIsNanCode},
    {"viv_isinf",           gcvFALSE,        gcvTRUE,        gcvNULL, _GenIsInfCode},
    {"viv_isnormal",        gcvFALSE,        gcvTRUE,        gcvNULL, _GenIsNormalCode},
    {"viv_signbit",         gcvFALSE,        gcvFALSE,       gcvNULL, _GenSignBitCode},
    {"viv_lgamma",          gcvFALSE,        gcvFALSE,       gcvNULL, _GenLGammaCode},
    {"viv_lgamma_r",        gcvFALSE,        gcvFALSE,       gcvNULL, _GenLGamma_RCode},
    {"viv_shuffle",         gcvFALSE,        gcvFALSE,       gcvNULL, _GenShuffleCode},
    {"shuffle#",            gcvFALSE,        gcvFALSE,       gcvNULL, _GenShufflePtrCode},
    {"shuffle#1",           gcvFALSE,        gcvFALSE,       gcvNULL, _GenShufflePtr1Code},
    {"viv_shuffle2",        gcvFALSE,        gcvFALSE,       gcvNULL, _GenShuffle2Code},
    {"shuffle2#",           gcvFALSE,        gcvFALSE,       gcvNULL, _GenShuffle2PtrCode},
    {"shuffle2#1",          gcvFALSE,        gcvFALSE,       gcvNULL, _GenShuffle2Ptr1Code},
    {"viv_maxmag",          gcvFALSE,        gcvFALSE,       gcvNULL, _GenMaxMagCode},
    {"viv_minmag",          gcvFALSE,        gcvFALSE,       gcvNULL, _GenMinMagCode},

    {"viv_select",          gcvFALSE,        gcvFALSE,       gcvNULL, _GenSelectCode},
    {"viv_any",             gcvFALSE,        gcvFALSE,       gcvNULL, _GenAnyAllCode},
    {"viv_all",             gcvFALSE,        gcvFALSE,       gcvNULL, _GenAnyAllCode},
    {"viv_bitselect",       gcvFALSE,        gcvFALSE,       gcvNULL, _GenBitSelectCode},

    /* Atomic Functions */
    {"atomic_add",      gcvFALSE,       gcvFALSE,       gcvNULL, _GenAtomCode},
    {"atomic_sub",      gcvFALSE,       gcvFALSE,       gcvNULL, _GenAtomCode},
    {"atomic_inc",      gcvFALSE,       gcvFALSE,       gcvNULL, _GenAtomCode},
    {"atomic_dec",      gcvFALSE,       gcvFALSE,       gcvNULL, _GenAtomCode},
    {"atomic_xchg",     gcvFALSE,       gcvFALSE,       gcvNULL, _GenAtomCode},
    {"atomic_cmpxchg",  gcvFALSE,       gcvFALSE,       gcvNULL, _GenAtomCode},
    {"atomic_min",      gcvFALSE,       gcvFALSE,       gcvNULL, _GenAtomCode},
    {"atomic_max",      gcvFALSE,       gcvFALSE,       gcvNULL, _GenAtomCode},
    {"atomic_or",       gcvFALSE,       gcvFALSE,       gcvNULL, _GenAtomCode},
    {"atomic_and",      gcvFALSE,       gcvFALSE,       gcvNULL, _GenAtomCode},
    {"atomic_xor",      gcvFALSE,       gcvFALSE,       gcvNULL, _GenAtomCode},

    /* Alias of atomic functions used in conformance tests */
    {"atom_add",        gcvFALSE,       gcvFALSE,       gcvNULL, _GenAtomCode},
    {"atom_sub",        gcvFALSE,       gcvFALSE,       gcvNULL, _GenAtomCode},
    {"atom_inc",        gcvFALSE,       gcvFALSE,       gcvNULL, _GenAtomCode},
    {"atom_dec",        gcvFALSE,       gcvFALSE,       gcvNULL, _GenAtomCode},
    {"atom_xchg",       gcvFALSE,       gcvFALSE,       gcvNULL, _GenAtomCode},
    {"atom_cmpxchg",    gcvFALSE,       gcvFALSE,       gcvNULL, _GenAtomCode},
    {"atom_min",        gcvFALSE,       gcvFALSE,       gcvNULL, _GenAtomCode},
    {"atom_max",        gcvFALSE,       gcvFALSE,       gcvNULL, _GenAtomCode},
    {"atom_or",         gcvFALSE,       gcvFALSE,       gcvNULL, _GenAtomCode},
    {"atom_and",        gcvFALSE,       gcvFALSE,       gcvNULL, _GenAtomCode},
    {"atom_xor",        gcvFALSE,       gcvFALSE,       gcvNULL, _GenAtomCode},
    {"printf",          gcvFALSE,       gcvFALSE,       gcvNULL, _GenPrintfCode},
};

#define _cldBuiltinFunctionCount \
    (sizeof(_BuiltinFunctionInfos) / sizeof(clsBUILTIN_FUNCTION_INFO))

static gctBOOL _IsBuiltinFunctionReady = gcvFALSE;

clsBUILTIN_FUNCTION_INFO *
clGetBuiltinFunctionInfo(
IN gctCONST_STRING Symbol
)
{
    slsDLINK_NODE *bucket;
    clsBUILTIN_FUNCTION_INFO_NODE *node;
    gctUINT crc32Value = clEvaluateCRC32ForShaderString(Symbol,
                                                             (gctUINT)gcoOS_StrLen(Symbol, gcvNULL));

    bucket = clsHASH_TABLE_Bucket(&_BuiltinFunctionInfoHash,
                                  clmBUCKET_INDEX(clHashString(Symbol)));

    FOR_EACH_DLINK_NODE(bucket, clsBUILTIN_FUNCTION_INFO_NODE, node) {
        if (node->crc32Value == crc32Value &&
            gcmIS_SUCCESS(gcoOS_StrCmp(node->builtinFuncInfo.symbol, Symbol))) {
            return &(node->builtinFuncInfo);
        }
    }

    return gcvNULL;
}

cltPOOL_STRING
clGetFastRelaxedMathFunction(
IN cloCOMPILER Compiler,
IN gctCONST_STRING Symbol
)
{
    slsDLINK_NODE *bucket;
    clsFAST_RELAXED_MATH_MAPPING_NODE *node;

    bucket = clsHASH_TABLE_Bucket(&_FastRelaxedMathMappingHash,
                                  clmBUCKET_INDEX(clHashString(Symbol)));

    FOR_EACH_DLINK_NODE(bucket, clsFAST_RELAXED_MATH_MAPPING_NODE, node) {
        if (gcmIS_SUCCESS(gcoOS_StrCmp(node->relaxedMathMapping.regFunc, Symbol))) {
            gceSTATUS status;
            cltPOOL_STRING symbolInPool;
            status = cloCOMPILER_AllocatePoolString(Compiler,
                                                    node->relaxedMathMapping.fastFunc,
                                                    &symbolInPool);
            if (gcmIS_ERROR(status)) return gcvNULL;
            return symbolInPool;
        }
    }

    return gcvNULL;
}

static gceSTATUS
_ConstructBuiltinFunctionInfos(
    IN cloCOMPILER Compiler
    )
{
    gctSIZE_T i;
    clsBUILTIN_FUNCTION_INFO_NODE *node;
    clsFAST_RELAXED_MATH_MAPPING_NODE *node1;
    slsDLINK_NODE *bucket;
    gceSTATUS    status;

    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);

    if(!_IsBuiltinFunctionReady) {
        /* init the builtin function info hash */
        clsHASH_TABLE_Initialize(&_BuiltinFunctionInfoHash);
        for(i=0; i< _cldBuiltinFunctionCount; i++) {
            bucket = clsHASH_TABLE_Bucket(&_BuiltinFunctionInfoHash,
                                          clmBUCKET_INDEX(clHashString(_BuiltinFunctionInfos[i].symbol)));

            status = cloCOMPILER_Allocate(Compiler,
                                    sizeof(clsBUILTIN_FUNCTION_INFO_NODE),
                                    (gctPOINTER *) &node);
            if (gcmIS_ERROR(status))
                break;
            node->builtinFuncInfo = _BuiltinFunctionInfos[i];
            node->crc32Value = clEvaluateCRC32ForShaderString(_BuiltinFunctionInfos[i].symbol,
                                                             (gctUINT)gcoOS_StrLen(_BuiltinFunctionInfos[i].symbol, gcvNULL));
            slsDLINK_LIST_InsertFirst(bucket, &node->node);
        }

        /* init the _FastRelaxedMathMapping hash */
        clsHASH_TABLE_Initialize(&_FastRelaxedMathMappingHash);
        for(i=0; i< _cldFastRelaxedMathMappingCount; i++) {
            bucket = clsHASH_TABLE_Bucket(&_FastRelaxedMathMappingHash,
                                          clmBUCKET_INDEX(clHashString(_FastRelaxedMathMapping[i].regFunc)));

            status = cloCOMPILER_Allocate(Compiler,
                                    sizeof(clsFAST_RELAXED_MATH_MAPPING_NODE),
                                    (gctPOINTER *) &node1);
            if (gcmIS_ERROR(status))
                break;
            node1->relaxedMathMapping = _FastRelaxedMathMapping[i];
            slsDLINK_LIST_InsertFirst(bucket, &node1->node);
        }

        _IsBuiltinFunctionReady = gcvTRUE;
    }
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
clEvaluateBuiltinFunction(
IN cloCOMPILER Compiler,
IN cloIR_POLYNARY_EXPR PolynaryExpr,
IN gctUINT OperandCount,
IN cloIR_CONSTANT * OperandConstants,
OUT cloIR_CONSTANT * ResultConstant
)
{
   gceSTATUS    status;
   cltBUILT_IN_EVALUATE_FUNC_PTR evaluate = gcvNULL;
   clsBUILTIN_FUNCTION_INFO *functionInfo;
   cloIR_CONSTANT resultConstant;
   clsDECL decl;

   gcmHEADER_ARG("Compiler=0x%x PolynaryExpr=0x%x "
                 "OperandCount=%u OperandConstants=0x%x ResultConstant=0x%x",
                 Compiler, PolynaryExpr,
                 OperandCount, OperandConstants, ResultConstant);

   /* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
   gcmASSERT(PolynaryExpr->type == clvPOLYNARY_FUNC_CALL);
   gcmASSERT(PolynaryExpr->funcName != gcvNULL);
   gcmASSERT(PolynaryExpr->funcName->isBuiltin);
   gcmASSERT(OperandConstants);
   gcmASSERT(ResultConstant);

   *ResultConstant = gcvNULL;

   functionInfo = clGetBuiltinFunctionInfo(PolynaryExpr->funcSymbol);
   if(functionInfo) {
      evaluate = functionInfo->evaluate;
   }
   else {
     gcmASSERT(0);
   }

   if (evaluate == gcvNULL) {
        gcmFOOTER_ARG("*ResultConstant=0x%x", *ResultConstant);
        return gcvSTATUS_OK;
   }

   /* Create the constant */
   status = cloCOMPILER_CloneDecl(Compiler,
                                  clvQUALIFIER_CONST,
                                  PolynaryExpr->exprBase.decl.dataType->addrSpaceQualifier,
                                  &PolynaryExpr->exprBase.decl,
                  &decl);
    if (gcmIS_ERROR(status)) {
        gcmFOOTER();
        return status;
    }


    /* Create the constant */
    status = cloIR_CONSTANT_Construct(Compiler,
                                      PolynaryExpr->exprBase.base.lineNo,
                                      PolynaryExpr->exprBase.base.stringNo,
                                      &decl,
                                      &resultConstant);
    if (gcmIS_ERROR(status)) {
        gcmFOOTER();
        return status;
    }

    /* Evaluate the built-in */
    status = (*evaluate)(Compiler,
            OperandCount,
            OperandConstants,
            resultConstant);

    if (gcmIS_ERROR(status)) {
        gcmFOOTER();
        return status;
    }

    *ResultConstant = resultConstant;
    gcmFOOTER_ARG("*ResultConstant=0x%x", *ResultConstant);
    return gcvSTATUS_OK;
}


/* Do allocate of the local variables instead of defining them as arrays to avoid potential
   overrun of run time stack */
gceSTATUS
clGenBuiltinVectorCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand,
    IN cltBUILT_IN_GEN_CODE_FUNC_PTR genCode
    )
{
    gceSTATUS    status = gcvSTATUS_OK;
    gctUINT8 vectorSize = clmGEN_CODE_vectorSize_GET(OperandsParameters[0].dataTypes[0].def);
    gctUINT8 i;
    gctUINT opCnt;
    clsROPERAND    *tempROperand, *copyROperand, *cntROperands, *inputROperands, destInitROperand;
    clsIOPERAND *tempIOperand, *cntIOperands, *inputIOperands;
    clsLOPERAND *tempLOperand, *destLOperands;
    clsROPERAND    vSizeROperand, *zero123ROperands, *zero123X4ROperands, negOneROperand;
    clsSELECTION_CONTEXT   selectionContextLoopBack;
    gctUINT8 copy_matrixSize_rowCount[16];
    gctUINT8 copy_matrixSize_columnCount[16];
    gctPOINTER rPointer = gcvNULL;
    gctPOINTER iPointer = gcvNULL;
    gctPOINTER lPointer = gcvNULL;
#if cldNoInlineVectorToScalar
    cloIR_POLYNARY_EXPR scalarFuncCall = gcvNULL;

    if(!PolynaryExpr->funcName->u.funcInfo.isInline) {
        gcmONERROR(cloIR_ScalarizeFuncCall(Compiler,
                           PolynaryExpr,
                           PolynaryExpr->funcName,
                           gcvTRUE,
                           &scalarFuncCall));
        if(scalarFuncCall->funcName->u.funcInfo.refCount < 2) {
           scalarFuncCall = gcvNULL;
        }
    }
#endif

    gcmASSERT(OperandCount < 5);
    status = cloCOMPILER_Allocate(Compiler,
                                  sizeof(clsROPERAND) * 130,
                                  &rPointer);
    if (gcmIS_ERROR(status)) return status;
    tempROperand = rPointer;
    copyROperand = tempROperand + 64;
    cntROperands = copyROperand + 16;
    inputROperands = cntROperands + 2;
    zero123ROperands = inputROperands + 16;
    zero123X4ROperands = zero123ROperands + 16;

    gcmONERROR(cloCOMPILER_Allocate(Compiler,
                                    sizeof(clsIOPERAND) * 19,
                                    &iPointer));
    tempIOperand = iPointer;
    cntIOperands = tempIOperand + 1;
    inputIOperands = cntIOperands + 2;

    gcmONERROR(cloCOMPILER_Allocate(Compiler,
                                    sizeof(clsLOPERAND) * 17,
                                    &lPointer));
    tempLOperand = lPointer;
    destLOperands = tempLOperand + 1;

    for(opCnt = 0; opCnt<OperandCount; opCnt++){
        copyROperand[opCnt] = OperandsParameters[opCnt].rOperands[0];
        copy_matrixSize_rowCount[opCnt] = OperandsParameters[opCnt].dataTypes[0].def.matrixSize.rowCount;
        copy_matrixSize_columnCount[opCnt] = OperandsParameters[opCnt].dataTypes[0].def.matrixSize.columnCount;
        /*force to scalar */
        OperandsParameters[opCnt].dataTypes[0].def.matrixSize.rowCount = 0;
        OperandsParameters[opCnt].dataTypes[0].def.matrixSize.columnCount = 0;
        clsIOPERAND_New(Compiler, &inputIOperands[opCnt], OperandsParameters[opCnt].dataTypes[0].def);
        clsROPERAND_InitializeUsingIOperand(&inputROperands[opCnt], &inputIOperands[opCnt]);

    }
    clsLOPERAND_InitializeUsingIOperand(tempLOperand, IOperand);
    clsIOPERAND_New(Compiler, tempIOperand, IOperand->dataType);
    /*Force to Scalar */
    tempIOperand->dataType.matrixSize.columnCount = 0;
    tempIOperand->dataType.matrixSize.rowCount = 0;

    for(i = 0; i<2; i++){
        clsIOPERAND_New(Compiler, &cntIOperands[i], clmGenCodeDataType(T_UINT));
        clsROPERAND_InitializeUsingIOperand(&cntROperands[i], &cntIOperands[i]);
    }

    clsROPERAND_InitializeIntOrIVecConstant(&destInitROperand,
                                            tempIOperand->dataType,
                                            (gctUINT) 0);
    for(i = 0; i<vectorSize; i++){
        for(opCnt = 0; opCnt<OperandCount; opCnt++){
            if(clmGEN_CODE_IsScalarDataType(copyROperand[opCnt].dataType) == 0 ){
                clmROPERAND_vectorComponent_GET(&tempROperand[4*i + opCnt], &copyROperand[opCnt], i);
            }
            else{
                tempROperand[4*i + opCnt] = copyROperand[opCnt];
            }
        }
        clmLOPERAND_vectorComponent_GET(&destLOperands[i], tempLOperand, i);
        gcmONERROR(clGenAssignCode(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   &destLOperands[i],
                                   &destInitROperand));
    }


    clsROPERAND_InitializeIntOrIVecConstant(&vSizeROperand,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) vectorSize);
    for(i = 0; i<16; i++){
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
    gcmONERROR(clGenGenericCode1(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_ASSIGN,
                            &cntIOperands[0],
                            &zero123ROperands[0]));

    for(opCnt = 0; opCnt<OperandCount; opCnt++){
        OperandsParameters[opCnt].rOperands[0] = inputROperands[opCnt];
    }

    gcmONERROR(clDefineSelectionBegin(Compiler,
                                    CodeGenerator,
                                    gcvTRUE,
                                    &selectionContextLoopBack));

    /* Loop End, jump back here*/
    gcmONERROR(clDefineSelectionFalseOperandBegin(Compiler,
                                                  CodeGenerator,
                                                  &selectionContextLoopBack));


    gcmONERROR(clDefineSelectionFalseOperandEnd(Compiler,
                                                CodeGenerator,
                                                &selectionContextLoopBack));

    /* Initialize the input value by using last argument.
    ** Otherwise it would create a uninitialize temp register.
    */
    for(opCnt = 0; opCnt<OperandCount; opCnt++)
    {
        if(clmGEN_CODE_IsScalarDataType(copyROperand[opCnt].dataType) == 0)
        {
            gcmONERROR(clGenGenericCode1(Compiler,
                                         PolynaryExpr->exprBase.base.lineNo,
                                         PolynaryExpr->exprBase.base.stringNo,
                                         clvOPCODE_ASSIGN,
                                         &inputIOperands[opCnt],
                                         &tempROperand[4*(vectorSize - 1) + opCnt]));
        }
        else
        {
            gcmONERROR(clGenArithmeticExprCode(Compiler,
                                               PolynaryExpr->exprBase.base.lineNo,
                                               PolynaryExpr->exprBase.base.stringNo,
                                               clvOPCODE_ADD,
                                               &inputIOperands[opCnt],
                                               &tempROperand[opCnt],
                                               &zero123X4ROperands[vectorSize - 1]));
        }
    }

    /* Initialize the other input value. */
    for(i = 0; i < vectorSize - 1; i++)
    {
        clmGEN_CODE_IF(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo,
                     &cntROperands[0],
                     clvCONDITION_EQUAL,
                     &zero123ROperands[i]);

        for(opCnt = 0; opCnt<OperandCount; opCnt++)
        {
            if(clmGEN_CODE_IsScalarDataType(copyROperand[opCnt].dataType) == 0)
            {
                gcmONERROR(clGenGenericCode1(Compiler,
                                             PolynaryExpr->exprBase.base.lineNo,
                                             PolynaryExpr->exprBase.base.stringNo,
                                             clvOPCODE_ASSIGN,
                                             &inputIOperands[opCnt],
                                             &tempROperand[4*i + opCnt]));
            }
            else
            {
                gcmONERROR(clGenArithmeticExprCode(Compiler,
                                                   PolynaryExpr->exprBase.base.lineNo,
                                                   PolynaryExpr->exprBase.base.stringNo,
                                                   clvOPCODE_ADD,
                                                   &inputIOperands[opCnt],
                                                   &tempROperand[opCnt],
                                                   &zero123X4ROperands[i]));
            }
        }

        /* The false part, "!==i" */
        clmGEN_CODE_ELSE(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);

        clmGEN_CODE_ENDIF(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);
        gcmONERROR(status);
    }
#if cldNoInlineVectorToScalar
    if(scalarFuncCall) {
        clsGEN_CODE_PARAMETERS rtnParameters[1];

        clsGEN_CODE_PARAMETERS_Initialize(rtnParameters,
                            gcvFALSE,
                            gcvTRUE);
        status = clGenBuiltinFunctionCode(Compiler,
                                            CodeGenerator,
                                            scalarFuncCall,
                                            OperandCount,
                                            OperandsParameters,
                                            tempIOperand,
                                            rtnParameters,
                                            gcvTRUE);
        clsGEN_CODE_PARAMETERS_Finalize(rtnParameters);
        gcmONERROR(status);
    }
    else {
        gcmONERROR((*genCode)(Compiler,
                      CodeGenerator,
                      PolynaryExpr,
                      OperandCount,
                      OperandsParameters,
                      tempIOperand));
    }
#else
    gcmONERROR((*genCode)(Compiler,
                  CodeGenerator,
                  PolynaryExpr,
                  OperandCount,
                  OperandsParameters,
                  tempIOperand));
#endif


    clsROPERAND_InitializeUsingIOperand(&tempROperand[0], tempIOperand);
    for(i = 0; i<vectorSize; i++){
      clmGEN_CODE_IF(Compiler,
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo,
                     &cntROperands[0],
                     clvCONDITION_EQUAL,
                     &zero123ROperands[i]);
      if(PolynaryExpr->funcName->symbol[0] == 'i' && PolynaryExpr->funcName->symbol[1] == 's'){
          /*isless, isequal, isnan, vector result is -1 and 0 */
          gcmONERROR(clGenArithmeticExprCode(Compiler,
                                             PolynaryExpr->exprBase.base.lineNo,
                                             PolynaryExpr->exprBase.base.stringNo,
                                             clvOPCODE_MUL,
                                             tempIOperand,
                                             &negOneROperand,
                                             &tempROperand[0]));

      }
      gcmONERROR(clGenAssignCode(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 &destLOperands[i],
                                 &tempROperand[0]));

      /* The false part, "!==i" */
      clmGEN_CODE_ELSE(Compiler,
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo);

      clmGEN_CODE_ENDIF(Compiler,
                        CodeGenerator,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo);
      gcmONERROR(status);
    }

    /* Cnt++ */
    gcmONERROR(clGenArithmeticExprCode(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvOPCODE_ADD,
                                       &cntIOperands[0],
                                       &cntROperands[0],
                                       &zero123ROperands[1]));

    gcmONERROR(clGenSelectionCompareConditionCode(Compiler,
                                                  CodeGenerator,
                                                  &selectionContextLoopBack,
                                                  PolynaryExpr->exprBase.base.lineNo,
                                                  PolynaryExpr->exprBase.base.stringNo,
                                                  clvCONDITION_GREATER_THAN_EQUAL,
                                                  &cntROperands[0],
                                                  &vSizeROperand));

    gcmONERROR(clDefineSelectionTrueOperandBegin(Compiler,
                                                 CodeGenerator,
                                                 &selectionContextLoopBack));

    gcmONERROR(clDefineSelectionTrueOperandEnd(Compiler,
                                               PolynaryExpr->exprBase.base.lineNo,
                                               PolynaryExpr->exprBase.base.stringNo,
                                               CodeGenerator,
                                               &selectionContextLoopBack,
                                               gcvFALSE));

    gcmONERROR(clDefineSelectionEnd(Compiler,
                                    CodeGenerator,
                                    &selectionContextLoopBack));

    for(opCnt = 0; opCnt<OperandCount; opCnt++){
        OperandsParameters[opCnt].rOperands[0] = copyROperand[opCnt];
        OperandsParameters[opCnt].dataTypes[0].def.matrixSize.rowCount = copy_matrixSize_rowCount[opCnt];
        OperandsParameters[opCnt].dataTypes[0].def.matrixSize.columnCount = copy_matrixSize_columnCount[opCnt];
    }

OnError:
    if(rPointer) {
        cloCOMPILER_Free(Compiler, rPointer);
    }
    if(iPointer) {
        cloCOMPILER_Free(Compiler, iPointer);
    }
    if(lPointer) {
        cloCOMPILER_Free(Compiler, lPointer);
    }
    return status;
}

gceSTATUS
clGenBuiltinFunctionCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_POLYNARY_EXPR PolynaryExpr,
IN gctUINT OperandCount,
IN clsGEN_CODE_PARAMETERS * OperandsParameters,
IN clsIOPERAND * IOperand,
IN OUT clsGEN_CODE_PARAMETERS * Parameters,
IN gctBOOL DoInlineCheck
)
{
   gceSTATUS status = gcvSTATUS_OK;
   cltBUILT_IN_GEN_CODE_FUNC_PTR genCode = gcvNULL;
   clsBUILTIN_FUNCTION_INFO *functionInfo;
   clsNAME *paramName;
   gctUINT i;

/* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
   gcmASSERT(PolynaryExpr->type == clvPOLYNARY_FUNC_CALL);
   gcmASSERT(PolynaryExpr->funcName != gcvNULL);
   gcmASSERT(PolynaryExpr->funcName->isBuiltin);
   gcmASSERT(Parameters);

   functionInfo = clGetBuiltinFunctionInfo(PolynaryExpr->funcSymbol);
   if(functionInfo) {
      genCode = functionInfo->genCode;
   }
   else {
     gcmASSERT(0);
   }
   gcmASSERT(genCode);

   /* Force to needROPerand if builtin function has return value */
   if(!Parameters->needROperand && !clmDECL_IsVoid(&PolynaryExpr->exprBase.decl)) {
      Parameters->needROperand = gcvTRUE;
   }
   if(DoInlineCheck &&
      !PolynaryExpr->funcName->u.funcInfo.isInline &&
      PolynaryExpr->funcName->u.funcInfo.refCount > 1) {
       gcmASSERT(PolynaryExpr->funcName->context.u.variable.u.function != gcvNULL);
       status = clGenFuncCallCode(Compiler,
                                  CodeGenerator,
                                  PolynaryExpr,
                                  OperandsParameters,
                                  Parameters);
       if (gcmIS_ERROR(status)) return status;

       if(Parameters->needROperand) {
          if(IOperand) {
             clsLOPERAND lOperand[1];

             clsLOPERAND_InitializeUsingIOperand(lOperand, IOperand);
             gcmASSERT(Parameters->operandCount == 1);
             status = clGenAssignCode(Compiler,
                                      PolynaryExpr->exprBase.base.lineNo,
                                      PolynaryExpr->exprBase.base.stringNo,
                                      lOperand,
                                      &Parameters->rOperands[0]);
             if (gcmIS_ERROR(status)) return status;
          }
       }
       return gcvSTATUS_OK;
   }
   else {
      if (Parameters->needROperand && Parameters->operandCount == 0) { /*Operands not yet allocated */
        clsIOPERAND iOperand[1];

        gcmASSERT(IOperand == gcvNULL);
    /* Allocate the register(s) */
        status = clsGEN_CODE_PARAMETERS_AllocateOperands(Compiler,
                                                         Parameters,
                                                         &PolynaryExpr->exprBase.decl);
        if (gcmIS_ERROR(status)) return status;
        gcmASSERT(Parameters->operandCount == 1);
        clmGEN_CODE_GetParametersIOperand(Compiler, iOperand, Parameters, Parameters->dataTypes[0].def);
        clsROPERAND_InitializeUsingIOperand(&Parameters->rOperands[0], iOperand);
        IOperand = iOperand;
     }
   }

   i = 0;
   FOR_EACH_DLINK_NODE(&PolynaryExpr->funcName->u.funcInfo.localSpace->names, struct _clsNAME, paramName) {
     gcmASSERT(i < OperandCount);
     status = clGenCheckAndImplicitConvertOperand(Compiler,
                                                  PolynaryExpr->exprBase.base.lineNo,
                                                  PolynaryExpr->exprBase.base.stringNo,
                                                  &paramName->decl,
                                                  OperandsParameters[i].rOperands);
     if (gcmIS_ERROR(status)) return status;
     i++;
   }

   if((IOperand && clmIsElementTypePacked(IOperand->dataType.elementType)) ||
      (OperandCount && clmIsElementTypePacked(OperandsParameters[0].dataTypes[0].def.elementType))) {
       return (*genCode)(Compiler,
                         CodeGenerator,
                         PolynaryExpr,
                         OperandCount,
                         OperandsParameters,
                         IOperand);
   }
   if(functionInfo->handleVector &&
      OperandCount &&
      Parameters->needROperand &&
      clmGEN_CODE_IsVectorDataType(OperandsParameters[0].dataTypes[0].def)) {
   /* special handling for vector case */
        return clGenBuiltinVectorCode(Compiler,
                                      CodeGenerator,
                                      PolynaryExpr,
                                      OperandCount,
                                      OperandsParameters,
                                      IOperand,
                                      genCode);
   }
   return (*genCode)(Compiler,
                     CodeGenerator,
                     PolynaryExpr,
                     OperandCount,
                     OperandsParameters,
                     IOperand);
}
