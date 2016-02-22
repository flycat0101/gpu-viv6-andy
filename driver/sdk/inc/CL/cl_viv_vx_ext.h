#ifndef _GC_VX_H
#define _GC_VX_H 1

#ifdef _VIV_VX_EXTENSION

#pragma OPENCL EXTENSION  CL_VIV_asm : enable

typedef enum _VXC_FilterMode
{
    VXC_FM_BOX      = 0,
    VXC_FM_Guassian = 1,
    VXC_FM_SobelX   = 2,
    VXC_FM_SobelY   = 3,
    VXC_FM_ScharrX  = 4,
    VXC_FM_ScharrY  = 5,
    VXC_FM_Max      = 8,
    VXC_FM_Min      = 9,
    VXC_FM_Median   = 10
} vxc_filter_mode;

typedef enum _VXC_RoundMode
{
    VXC_RM_TowardZero    = 0,
    VXC_RM_TowardInf     = 1,
    VXC_RM_ToNearestEven = 2
} vxc_round_mode;

#define VXC_CLAMP_BITMASK           0x400000     /* shift 22 */
#define VXC_PREADJ_BITMASK          0x200000     /* shift 21 */
#define VXC_RANGEPI_BITMASK         0x100000     /* shift 20 */
#define VXC_FILTER_BITMASK          0x0F0000     /* shift 16 */
#define VXC_START_BIN_BITMASK       0x00F000     /* shift 12 */
#define VXC_END_BIN_BITMASK         0x000F00     /* shift 8 */
#define VXC_SOURCE_BIN_BITMASK      0x0000F0     /* shift 4 */
#define VXC_ROUNDING_MODE_BITMASK   0x00000C     /* shift 2 */
#define VXC_ENABLEBOOL_BITMASK      0x000002     /* shift 1 */
#define VXC_SIGNEXT_BITMASK         0x000001     /* shift 0 */

#define VXC_MODIFIER(StartBin, EndBin, SourceBin, RoundingMode, Clamp)    \
         (                                                                \
          (((Clamp) << 22)&VXC_CLAMP_BITMASK)          |                  \
          (((StartBin) << 12)&VXC_START_BIN_BITMASK)   |                  \
          (((EndBin) << 8)&VXC_END_BIN_BITMASK)        |                  \
          (((SourceBin) << 4)&VXC_SOURCE_BIN_BITMASK)  |                  \
          ((RoundingMode << 2)&VXC_ROUNDING_MODE_BITMASK)                 \
         )

#define VXC_MODIFIER_SIGNEXT(StartBin, EndBin, SourceBin, SignExt, Clamp) \
         (                                                                \
          (((Clamp) << 22)&VXC_CLAMP_BITMASK)          |                  \
          ((SignExt)&VXC_SIGNEXT_BITMASK)              |                  \
          (((StartBin) << 12)&VXC_START_BIN_BITMASK)   |                  \
          (((EndBin) << 8)&VXC_END_BIN_BITMASK)        |                  \
          (((SourceBin) << 4)&VXC_SOURCE_BIN_BITMASK)                     \
         )

#define VXC_MODIFIER_MAGPHASE(StartBin, EndBin, SourceBin, NoPreAdjust, RangePi)  \
         (                                                                \
          (VXC_CLAMP_BITMASK)                          |                  \
          (((RangePi) << 20)&VXC_RANGEPI_BITMASK)      |                  \
          (((NoPreAdjust) << 21)&VXC_PREADJ_BITMASK)   |                  \
          (((StartBin) << 12)&VXC_START_BIN_BITMASK)   |                  \
          (((EndBin) << 8)&VXC_END_BIN_BITMASK)        |                  \
          (((SourceBin) << 4)&VXC_SOURCE_BIN_BITMASK)                     \
         )

#define VXC_MODIFIER_CLAMP(StartBin, EndBin, SourceBin, EnableBool)       \
         (                                                                \
          (((EnableBool) << 1)&VXC_ENABLEBOOL_BITMASK)   |                \
          (((StartBin) << 12)&VXC_START_BIN_BITMASK)     |                \
          (((EndBin) << 8)&VXC_END_BIN_BITMASK)          |                \
          (((SourceBin) << 4)&VXC_SOURCE_BIN_BITMASK)                     \
         )

#define VXC_MODIFIER_FILTER(StartBin, EndBin, SourceBin, Filter, Clamp)   \
         (                                                                \
          (((Clamp) << 22)&VXC_CLAMP_BITMASK)            |                \
          (((Filter) << 16)&VXC_FILTER_BITMASK)          |                \
          (((StartBin) << 12)&VXC_START_BIN_BITMASK)     |                \
          (((EndBin) << 8)&VXC_END_BIN_BITMASK)          |                \
          (((SourceBin) << 4)&VXC_SOURCE_BIN_BITMASK)                     \
         )

#define VXC_MODIFIER_BIN(StartBin, EndBin, Clamp)                         \
         (                                                                \
          (((Clamp) << 22)&VXC_CLAMP_BITMASK)          |                  \
          (((StartBin) << 12)&VXC_START_BIN_BITMASK)   |                  \
          (((EndBin) << 8)&VXC_END_BIN_BITMASK)                           \
         )

/*
 * Clamp : 0 ==> result is truncated to fit result type
 *         1 ==> result is clamp to fit result type
 */
#define VXC_MODIFIER_SetDestClamp(VxModifier, Clamp)   ((VxModifier) | (((Clamp) << 22)&VXC_CLAMP_BITMASK))

#define VXC_DEFAULT_MODIFIER   (-1)

typedef unsigned int vxc_modifier;

/* packed char vector 2/4/8/16 */
typedef _viv_char2_packed     vxc_char2;
typedef _viv_char4_packed     vxc_char4;
typedef _viv_char8_packed     vxc_char8;
typedef _viv_char16_packed    vxc_char16;
typedef struct _vxc_char32
{
    vxc_char16  hi;
    vxc_char16  lo;
} vxc_char32;

/* packed uchar vector 2/4/8/16 */
typedef _viv_uchar2_packed    vxc_uchar2;
typedef _viv_uchar4_packed    vxc_uchar4;
typedef _viv_uchar8_packed    vxc_uchar8;
typedef _viv_uchar16_packed   vxc_uchar16;
typedef struct _vxc_uchar32
{
    vxc_uchar16  hi;
    vxc_uchar16  lo;
} vxc_uchar32;

/* packed short vector 2/4/8 */
typedef _viv_short2_packed    vxc_short2;
typedef _viv_short4_packed    vxc_short4;
typedef _viv_short8_packed    vxc_short8;
typedef struct _vxc_short16
{
    vxc_short8  hi;
    vxc_short8  lo;
} vxc_short16;

/* packed ushort vector 2/4/8 */
typedef _viv_ushort2_packed   vxc_ushort2;
typedef _viv_ushort4_packed   vxc_ushort4;
typedef _viv_ushort8_packed   vxc_ushort8;
typedef struct _vxc_ushort16
{
    vxc_ushort8  hi;
    vxc_ushort8  lo;
} vxc_ushort16;

/* int vector 2/4/8/16 */
typedef int      vxc_int;
typedef int2     vxc_int2;
typedef int4     vxc_int4;
typedef int8     vxc_int8;
typedef int16    vxc_int16;

/* uint vector 2/4/8/16 */
typedef uint     vxc_uint;
typedef uint2    vxc_uint2;
typedef uint4    vxc_uint4;
typedef uint8    vxc_uint8;
typedef uint16   vxc_uint16;

/* float vector 2/4/8/16 */
typedef float     vxc_float;
typedef float2    vxc_float2;
typedef float4    vxc_float4;
typedef float8    vxc_float8;
typedef float16   vxc_float16;

typedef uint16 vxc_512bits;

typedef struct
{
    size_t size;
    global char* item;
} vx_array_char;

typedef struct
{
    size_t size;
    global unsigned char* item;
} vx_array_uchar;

typedef struct
{
    size_t size;
    global short* item;
} vx_array_short;

typedef struct
{
    size_t size;
    global unsigned short* item;
} vx_array_ushort;

typedef struct
{
    size_t size;
    global int* item;
} vx_array_int;

typedef struct
{
    size_t size;
    global unsigned int* item;
} vx_array_uint;


typedef struct
{
    size_t size;
    global float * item;
} vx_array_float;

typedef struct
{
    size_t size;
    global unsigned char* item;
} vx_lut_uchar;

typedef struct
{
    size_t size;
    global unsigned short* item;
} vx_lut_ushort;

typedef struct
{
    size_t columns;
    size_t rows;
    global short* matrix;
    uint scale;
} vx_convolution;

typedef struct
{
    size_t columns;
    size_t rows;
    global char* matrix;
} vx_matrix_char;

typedef struct
{
    size_t columns;
    size_t rows;
    global unsigned char* matrix;
} vx_matrix_uchar;

typedef struct
{
    size_t columns;
    size_t rows;
    global short* matrix;
} vx_matrix_short;

typedef struct
{
    size_t columns;
    size_t rows;
    global unsigned short* matrix;
} vx_matrix_ushort;

typedef struct
{
    size_t columns;
    size_t rows;
    global int* matrix;
} vx_matrix_int;

typedef struct
{
    size_t columns;
    size_t rows;
    global unsigned int* matrix;
} vx_matrix_uint;

typedef struct
{
    size_t columns;
    size_t rows;
    global float* matrix;
} vx_matrix_float;

typedef struct
{
    int type;
    uint value;
    uint lower;
    uint upper;
    uint trueValue;
    uint falseValue;
} vx_threshold;

typedef struct  {
    int dst_width;
    int dst_height;
    global float* ptr;
} vx_remap;

typedef struct
{
    int bins;
    int rang;
    int offset;
    float window_r;
    global int* ptr;
} vx_distribution;

typedef struct
{
    float scale;
    uint width;
    uint height;
    uint format;
    uint levelCount;
/*    global image2d_t* levels;*/
} vx_pyramid;

typedef vxc_512bits VXC_512Bits;
typedef vxc_modifier VXC_Modifier_t ;
typedef vxc_round_mode VXC_RoundMode;
typedef vxc_filter_mode VXC_FilterMode;

#ifdef __cplusplus
extern "c" {
#endif

#define viv_vx_api_only 0

#if viv_vx_api_only
#define _RET0_        ;
#define _RET_         ;
#define _EXT_         extern
#else
#define _RET0_        { return (0); }
#define _RET_         { return ; }
#define _EXT_
#endif

/* implicit cast for vx_inst parameter*/
/* uchar */
_EXT_ vxc_uchar16 viv_intrinsic_vx_icastP_uc(vxc_char16 a) _RET0_
_EXT_ vxc_uchar8  viv_intrinsic_vx_icastP_uc(vxc_char8 a)  _RET0_

_EXT_ vxc_uchar16  viv_intrinsic_vx_icastP_uc(vxc_short8 a)  _RET0_
_EXT_ vxc_uchar8  viv_intrinsic_vx_icastP_uc(vxc_short4 a)  _RET0_

_EXT_ vxc_uchar16  viv_intrinsic_vx_icastP_uc(vxc_ushort8 a)  _RET0_
_EXT_ vxc_uchar8  viv_intrinsic_vx_icastP_uc(vxc_ushort4 a)  _RET0_

/* char */
_EXT_ vxc_char16 viv_intrinsic_vx_icastP_c(vxc_uchar16 a) _RET0_
_EXT_ vxc_char8  viv_intrinsic_vx_icastP_c(vxc_uchar8 a)  _RET0_

_EXT_ vxc_char16  viv_intrinsic_vx_icastP_c(vxc_short8 a)  _RET0_
_EXT_ vxc_char8  viv_intrinsic_vx_icastP_c(vxc_short4 a)  _RET0_

_EXT_ vxc_char16  viv_intrinsic_vx_icastP_c(vxc_ushort8 a)  _RET0_
_EXT_ vxc_char8  viv_intrinsic_vx_icastP_c(vxc_ushort4 a)  _RET0_

/* ushort */
_EXT_ vxc_ushort8  viv_intrinsic_vx_icastP_us(vxc_uchar16 a) _RET0_
_EXT_ vxc_ushort4  viv_intrinsic_vx_icastP_us(vxc_uchar8 a)  _RET0_

_EXT_ vxc_ushort8  viv_intrinsic_vx_icastP_us(vxc_char16 a) _RET0_
_EXT_ vxc_ushort4  viv_intrinsic_vx_icastP_us(vxc_char8 a)  _RET0_

_EXT_ vxc_ushort8  viv_intrinsic_vx_icastP_us(vxc_short8 a)  _RET0_
_EXT_ vxc_ushort4  viv_intrinsic_vx_icastP_us(vxc_short4 a)  _RET0_

/* short */
_EXT_ vxc_short8  viv_intrinsic_vx_icastP_s(vxc_uchar16 a) _RET0_
_EXT_ vxc_short4  viv_intrinsic_vx_icastP_s(vxc_uchar8 a)  _RET0_

_EXT_ vxc_short8  viv_intrinsic_vx_icastP_s(vxc_char16 a) _RET0_
_EXT_ vxc_short4  viv_intrinsic_vx_icastP_s(vxc_char8 a)  _RET0_

_EXT_ vxc_short8  viv_intrinsic_vx_icastP_s(vxc_ushort8 a)  _RET0_
_EXT_ vxc_short4  viv_intrinsic_vx_icastP_s(vxc_ushort4 a)  _RET0_

/* implicit cast for vx_inst dest */
/* uchar */
_EXT_ vxc_uchar16 viv_intrinsic_vx_icastD_uc(vxc_char16 a) _RET0_
_EXT_ vxc_uchar8  viv_intrinsic_vx_icastD_uc(vxc_char8 a)  _RET0_

_EXT_ vxc_uchar8 viv_intrinsic_vx_icastD_uc(vxc_short8 a)  _RET0_
_EXT_ vxc_uchar4  viv_intrinsic_vx_icastD_uc(vxc_short4 a)  _RET0_

_EXT_ vxc_uchar8 viv_intrinsic_vx_icastD_uc(vxc_ushort8 a)  _RET0_
_EXT_ vxc_uchar4  viv_intrinsic_vx_icastD_uc(vxc_ushort4 a)  _RET0_

_EXT_ vxc_uchar4 viv_intrinsic_vx_icastD_uc(vxc_int4 a)  _RET0_
_EXT_ vxc_uchar2  viv_intrinsic_vx_icastD_uc(vxc_int2 a)  _RET0_

_EXT_ vxc_uchar4 viv_intrinsic_vx_icastD_uc(vxc_uint4 a)  _RET0_
_EXT_ vxc_uchar2  viv_intrinsic_vx_icastD_uc(vxc_uint2 a)  _RET0_

/* char */
_EXT_ vxc_char16 viv_intrinsic_vx_icastD_c(vxc_uchar16 a) _RET0_
_EXT_ vxc_char8  viv_intrinsic_vx_icastD_c(vxc_uchar8 a)  _RET0_

_EXT_ vxc_char8 viv_intrinsic_vx_icastD_c(vxc_short8 a)  _RET0_
_EXT_ vxc_char4  viv_intrinsic_vx_icastD_c(vxc_short4 a)  _RET0_

_EXT_ vxc_char8 viv_intrinsic_vx_icastD_c(vxc_ushort8 a)  _RET0_
_EXT_ vxc_char4  viv_intrinsic_vx_icastD_c(vxc_ushort4 a)  _RET0_

_EXT_ vxc_char4 viv_intrinsic_vx_icastD_c(vxc_int4 a)  _RET0_
_EXT_ vxc_char2  viv_intrinsic_vx_icastD_c(vxc_int2 a)  _RET0_

_EXT_ vxc_char4 viv_intrinsic_vx_icastD_c(vxc_uint4 a)  _RET0_
_EXT_ vxc_char2  viv_intrinsic_vx_icastD_c(vxc_uint2 a)  _RET0_

/* ushort */
_EXT_ vxc_ushort8  viv_intrinsic_vx_icastD_us(vxc_uchar8 a) _RET0_
_EXT_ vxc_ushort4  viv_intrinsic_vx_icastD_us(vxc_uchar4 a)  _RET0_

_EXT_ vxc_ushort8  viv_intrinsic_vx_icastD_us(vxc_char8 a) _RET0_
_EXT_ vxc_ushort4  viv_intrinsic_vx_icastD_us(vxc_char4 a)  _RET0_

_EXT_ vxc_ushort8  viv_intrinsic_vx_icastD_us(vxc_short8 a)  _RET0_
_EXT_ vxc_ushort4  viv_intrinsic_vx_icastD_us(vxc_short4 a)  _RET0_

_EXT_ vxc_ushort4  viv_intrinsic_vx_icastD_us(vxc_int4 a)  _RET0_
_EXT_ vxc_ushort2  viv_intrinsic_vx_icastD_us(vxc_int2 a)  _RET0_

_EXT_ vxc_ushort4  viv_intrinsic_vx_icastD_us(vxc_uint4 a)  _RET0_
_EXT_ vxc_ushort2  viv_intrinsic_vx_icastD_us(vxc_uint2 a)  _RET0_

/* short */
_EXT_ vxc_short8  viv_intrinsic_vx_icastD_s(vxc_uchar8 a) _RET0_
_EXT_ vxc_short4  viv_intrinsic_vx_icastD_s(vxc_uchar4 a)  _RET0_

_EXT_ vxc_short8  viv_intrinsic_vx_icastD_s(vxc_char8 a) _RET0_
_EXT_ vxc_short4  viv_intrinsic_vx_icastD_s(vxc_char4 a)  _RET0_

_EXT_ vxc_short8  viv_intrinsic_vx_icastD_s(vxc_ushort8 a)  _RET0_
_EXT_ vxc_short4  viv_intrinsic_vx_icastD_s(vxc_ushort4 a)  _RET0_

_EXT_ vxc_short4  viv_intrinsic_vx_icastD_s(vxc_int4 a)  _RET0_
_EXT_ vxc_short2  viv_intrinsic_vx_icastD_s(vxc_int2 a)  _RET0_

_EXT_ vxc_short4  viv_intrinsic_vx_icastD_s(vxc_uint4 a)  _RET0_
_EXT_ vxc_short2  viv_intrinsic_vx_icastD_s(vxc_uint2 a)  _RET0_

/* int32 */
_EXT_ vxc_int4  viv_intrinsic_vx_icastD_i(vxc_char4 a)  _RET0_
_EXT_ vxc_int4  viv_intrinsic_vx_icastD_i(vxc_uchar4 a)  _RET0_
_EXT_ vxc_int4  viv_intrinsic_vx_icastD_i(vxc_short4 a)  _RET0_
_EXT_ vxc_int4  viv_intrinsic_vx_icastD_i(vxc_ushort4 a)  _RET0_
_EXT_ vxc_int4  viv_intrinsic_vx_icastD_i(vxc_uint4 a)  _RET0_
_EXT_ vxc_int4  viv_intrinsic_vx_icastD_i(vxc_float4 a)  _RET0_

_EXT_ vxc_int2  viv_intrinsic_vx_icastD_i(vxc_char2 a)  _RET0_
_EXT_ vxc_int2  viv_intrinsic_vx_icastD_i(vxc_uchar2 a)  _RET0_
_EXT_ vxc_int2  viv_intrinsic_vx_icastD_i(vxc_short2 a)  _RET0_
_EXT_ vxc_int2  viv_intrinsic_vx_icastD_i(vxc_ushort2 a)  _RET0_
_EXT_ vxc_int2  viv_intrinsic_vx_icastD_i(vxc_uint2 a)  _RET0_
_EXT_ vxc_int2  viv_intrinsic_vx_icastD_i(vxc_float2 a)  _RET0_

/* uint32 */
_EXT_ vxc_uint4  viv_intrinsic_vx_icastD_ui(vxc_char4 a)  _RET0_
_EXT_ vxc_uint4  viv_intrinsic_vx_icastD_ui(vxc_uchar4 a)  _RET0_
_EXT_ vxc_uint4  viv_intrinsic_vx_icastD_ui(vxc_short4 a)  _RET0_
_EXT_ vxc_uint4  viv_intrinsic_vx_icastD_ui(vxc_ushort4 a)  _RET0_
_EXT_ vxc_uint4  viv_intrinsic_vx_icastD_ui(vxc_int4 a)  _RET0_
_EXT_ vxc_uint4  viv_intrinsic_vx_icastD_ui(vxc_float4 a)  _RET0_

_EXT_ vxc_uint2  viv_intrinsic_vx_icastD_ui(vxc_char2 a)  _RET0_
_EXT_ vxc_uint2  viv_intrinsic_vx_icastD_ui(vxc_uchar2 a)  _RET0_
_EXT_ vxc_uint2  viv_intrinsic_vx_icastD_ui(vxc_short2 a)  _RET0_
_EXT_ vxc_uint2  viv_intrinsic_vx_icastD_ui(vxc_ushort2 a)  _RET0_
_EXT_ vxc_uint2  viv_intrinsic_vx_icastD_ui(vxc_int2 a)  _RET0_
_EXT_ vxc_uint2  viv_intrinsic_vx_icastD_ui(vxc_float2 a)  _RET0_

/* float32 */
_EXT_ vxc_float4  viv_intrinsic_vx_icastD_f(vxc_char4 a)  _RET0_
_EXT_ vxc_float4  viv_intrinsic_vx_icastD_f(vxc_uchar4 a)  _RET0_
_EXT_ vxc_float4  viv_intrinsic_vx_icastD_f(vxc_short4 a)  _RET0_
_EXT_ vxc_float4  viv_intrinsic_vx_icastD_f(vxc_ushort4 a)  _RET0_
_EXT_ vxc_float4  viv_intrinsic_vx_icastD_f(vxc_int4 a)  _RET0_

_EXT_ vxc_float2  viv_intrinsic_vx_icastD_f(vxc_char2 a)  _RET0_
_EXT_ vxc_float2  viv_intrinsic_vx_icastD_f(vxc_uchar2 a)  _RET0_
_EXT_ vxc_float2  viv_intrinsic_vx_icastD_f(vxc_short2 a)  _RET0_
_EXT_ vxc_float2  viv_intrinsic_vx_icastD_f(vxc_ushort2 a)  _RET0_
_EXT_ vxc_float2  viv_intrinsic_vx_icastD_f(vxc_int2 a)  _RET0_

/* data selection */
/* swizzle:
 *   16 elements: i E [0-15], if mask[i] == '1 then result[i] = a[ swzl[i*4 : i*4 + 3] ];
 *   8  elements: i E [0-7],  if mask[2*i : 2*i+1] == '11 then result[i] = a[ swzl[i*4 : i*4 + 3] & 0x7 ];
 */
#define VXC_SWIZZLE_MASK8_ALL()   0xFFFF
#define VXC_SWIZZLE_MASK8(E0, E1, E2, E3, E4, E5, E6, E7)                       \
        (((E0) * 0x3)         | ((E1) * (0x3 << 2))  | ((E2) * (0x3 << 4))  |  \
         ((E3) * (0x3 << 6))  | ((E4) * (0x3 << 8))  | ((E5) * (0x3 << 10)) |  \
         ((E6) * (0x3 << 12)) | ((E7) * (0x3 << 14))  )

#define VXC_SWIZZLE_MASK16_ALL()   0xFFFF
#define VXC_SWIZZLE_MASK16(E0, E1, E2, E3, E4, E5, E6, E7, E9, E10, E11, E12, E13, E14, E15)  \
        (((E0) * 0x1)          | ((E1) * (0x1 << 1))   | ((E2) * (0x1 << 2))    |  \
         ((E3) * (0x1 << 1))   | ((E4) * (0x1 << 4))   | ((E5) * (0x1 << 5))    |  \
         ((E6) * (0x1 << 6))   | ((E7) * (0x1 << 7))   | ((E8) * (0x1 << 8))    |  \
         ((E9) * (0x1 << 9))   | ((E10) * (0x1 << 10)) | ((E11) * (0x1 << 11))  |  \
         ((E12) * (0x1 << 12)) | ((E13) * (0x1 << 13)) | ((E14) * (0x1 << 14))  |  \
         ((E15) * (0x1 << 15))  )
/* E0 - E15 must be 0 or 1 */
#define VXC_SWIZZLE8(S0, S1, S2, S3, S4, S5, S6, S7)           \
        (uint)((S0) << 0  | (S1) << 4  | (S2) << 8  | (S3) << 12 |  \
               (S4) << 16 | (S5) << 20 | (S6) << 24 | (S7) << 28  )


/* samplerless image read */

_EXT_ vxc_char16  viv_intrinsic_vx_read_imagec   (image2d_t image, int2 coord) _RET0_
_EXT_ vxc_uchar16 viv_intrinsic_vx_read_imageuc  (image2d_t image, int2 coord) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vx_read_images   (image2d_t image, int2 coord) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vx_read_imageus  (image2d_t image, int2 coord) _RET0_

_EXT_ vxc_char16  viv_intrinsic_vx_read_imagec   (image1d_t image, int coord) _RET0_
_EXT_ vxc_uchar16 viv_intrinsic_vx_read_imageuc  (image1d_t image, int coord) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vx_read_images   (image1d_t image, int coord) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vx_read_imageus  (image1d_t image, int coord) _RET0_

_EXT_ vxc_char16  viv_intrinsic_vx_read_imagec   (image1d_array_t image, int2 coord) _RET0_
_EXT_ vxc_uchar16 viv_intrinsic_vx_read_imageuc  (image1d_array_t image, int2 coord) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vx_read_images   (image1d_array_t image, int2 coord) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vx_read_imageus  (image1d_array_t image, int2 coord) _RET0_

/* image write */
_EXT_ void viv_intrinsic_vx_write_imagec  (image2d_t image, int2 coord, vxc_char16 color) _RET_
_EXT_ void viv_intrinsic_vx_write_imageuc (image2d_t image, int2 coord, vxc_uchar16 color) _RET_
_EXT_ void viv_intrinsic_vx_write_images  (image2d_t image, int2 coord, vxc_short8 color) _RET_
_EXT_ void viv_intrinsic_vx_write_imageus (image2d_t image, int2 coord, vxc_ushort8 color) _RET_

_EXT_ void viv_intrinsic_vx_write_imagec  (image1d_t image, int coord, vxc_char16 color) _RET_
_EXT_ void viv_intrinsic_vx_write_imageuc (image1d_t image, int coord, vxc_uchar16 color) _RET_
_EXT_ void viv_intrinsic_vx_write_images  (image1d_t image, int coord, vxc_short8 color) _RET_
_EXT_ void viv_intrinsic_vx_write_imageus (image1d_t image, int coord, vxc_ushort8 color) _RET_

_EXT_ void viv_intrinsic_vx_write_imagec  (image1d_array_t image, int2 coord, vxc_char16 color) _RET_
_EXT_ void viv_intrinsic_vx_write_imageuc (image1d_array_t image, int2 coord, vxc_uchar16 color) _RET_
_EXT_ void viv_intrinsic_vx_write_images  (image1d_array_t image, int2 coord, vxc_short8 color) _RET_
_EXT_ void viv_intrinsic_vx_write_imageus (image1d_array_t image, int2 coord, vxc_ushort8 color) _RET_

/* AbsDiff
 *
 * Syntax:
 *      r = AbsDiff(a, b) ;
 *
 * Semantics:
 *      r[i] = |a[i] - b[i]| ;   i E [0, elem(r) )
 */
_EXT_ vxc_uchar16 viv_intrinsic_vx_AbsDiff_uc(vxc_uchar16 a, vxc_uchar16 b) _RET0_
_EXT_ vxc_char16  viv_intrinsic_vx_AbsDiff_c(vxc_char16 a, vxc_char16 b) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vx_AbsDiff_s(vxc_short8 a, vxc_short8 b) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vx_AbsDiff_us(vxc_ushort8 a, vxc_ushort8 b) _RET0_

/* IAdd
 *
 * Syntax:
 *      r = IAdd(a, b, c) ;
 *
 * Semantics:
 *      r[i] = a[i] + b[i] + c[i] ;  i E [0, elem(r) )
 */
_EXT_ vxc_uchar16 viv_intrinsic_vx_IAdd_uc(vxc_uchar16 a, vxc_uchar16 b, vxc_uchar16 c) _RET0_
_EXT_ vxc_char16  viv_intrinsic_vx_IAdd_c(vxc_char16 a, vxc_char16 b, vxc_char16 c) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vx_IAdd_s(vxc_short8 a, vxc_short8 b, vxc_short8 c) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vx_IAdd_us(vxc_ushort8 a, vxc_ushort8 b, vxc_ushort8 c) _RET0_

/* IAccSq: squares a value and adds it to an accumulator
 *
 * Syntax:
 *      r = IAccSq(a, b, Imm) ;    // Imm must be an immediate value
 *
 * Semantics:
 *      r[i] = a[i] + (b[i]^2 >> Imm) ;  i E [0, elem(r) )
 */
_EXT_ vxc_uchar16 viv_intrinsic_vx_IAccSq_uc(vxc_uchar16 a, vxc_uchar16 b, uint Imm) _RET0_
_EXT_ vxc_char16  viv_intrinsic_vx_IAccSq_c(vxc_char16 a, vxc_char16 b, uint Imm) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vx_IAccSq_s(vxc_short8 a, vxc_short8 b, uint Imm) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vx_IAccSq_us(vxc_ushort8 a, vxc_ushort8 b, uint Imm) _RET0_

/* Lerp: linear interpolation between two values
 *
 * Syntax:
 *      r = Lerp(a, b, c) ;
 *
 * Semantics:
 *      r[i] = (1.0 - c) * a[i] + c * b[i] ;  i E [0, elem(r) )
 */
_EXT_ vxc_uchar16 viv_intrinsic_vx_Lerp_uc(vxc_uchar16 a, vxc_uchar16 b, vxc_float c) _RET0_
_EXT_ vxc_char16  viv_intrinsic_vx_Lerp_c(vxc_char16 a, vxc_char16 b, vxc_float c) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vx_Lerp_s(vxc_short8 a, vxc_short8 b, vxc_float c) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vx_Lerp_us(vxc_ushort8 a, vxc_ushort8 b, vxc_float c) _RET0_

/* Filter: performs a specific filter on a 3x3 pixel block
 *
 * Syntax:
 *      r = Filter(a, b, c, f) ;  f E { BOX, Guassian, SobelX, SobelY,
 *                                     ScharrX, ScharrY, Max, Min, Median }
 *
 * Semantics:
 *      S[i] = { {a[i], a[i+1], a[i+2]},
                 {b[i], b[i+1], b[i+2]},
                 {c[i], c[i+1], c[i+2]} } ;
 *      r[i] = (*f)(U, S[i])  ;  i E [0, elem(r) - 2 )
 */
_EXT_ vxc_uchar16 viv_intrinsic_vx_Filter_uc(vxc_uchar16 a, vxc_uchar16 b, vxc_uchar16 c, vxc_filter_mode f) _RET0_
_EXT_ vxc_char16  viv_intrinsic_vx_Filter_c(vxc_char16 a, vxc_char16 b, vxc_char16 c, vxc_filter_mode f) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vx_Filter_s(vxc_short8 a, vxc_short8 b, vxc_short8 c, vxc_filter_mode f) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vx_Filter_us(vxc_ushort8 a, vxc_ushort8 b, vxc_ushort8 c, vxc_filter_mode f) _RET0_

/* MagPhase:  computes the magnitude and phase of two incoming values.
 *
 * Syntax:
 *      r = MagPhase(a, b) ;
 *
 * Semantics:
 *      r[i] = sqrt(a[i]^2 + b[i]^2) ;  i E [0, 3]
 *      r[4+i] = arctan(b[i] / a[i]) ;
 */
_EXT_ vxc_uchar16 viv_intrinsic_vx_MagPhase_uc(vxc_uchar16 a, vxc_uchar16 b) _RET0_
_EXT_ vxc_char16  viv_intrinsic_vx_MagPhase_c(vxc_char16 a, vxc_char16 b) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vx_MagPhase_s(vxc_short8 a, vxc_short8 b) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vx_MagPhase_us(vxc_ushort8 a, vxc_ushort8 b) _RET0_

/* MulShift: squares a value and adds it to an accumulator
 *
 * Syntax:
 *      r = MulShift(a, b, Imm) ;    // Imm must be an immediate value
 *
 * Semantics:
 *      r[i] = (a[i] * b[i]) >> Imm ;  i E [0, elem(r) )
 */
_EXT_ vxc_uchar16 viv_intrinsic_vx_MulShift_uc(vxc_uchar16 a, vxc_uchar16 b, uint Imm) _RET0_
_EXT_ vxc_char16  viv_intrinsic_vx_MulShift_c(vxc_char16 a, vxc_char16 b, uint Imm) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vx_MulShift_s(vxc_short8 a, vxc_short8 b, uint Imm) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vx_MulShift_us(vxc_ushort8 a, vxc_ushort8 b, uint Imm) _RET0_

/* Clamp: clamps up to 16 values to a min and.or max value
 *
 * Syntax:
 *      r = Clamp(a, b, c) ;
 *      r = ClampBoolean(a, b, c) ; // boolean mode
 * Semantics:
 *      r[i] = clamp(a[i], b[i], c[i]) ;  i E [0, elem(r) )
 *
 *  In boolean mode it will write a 0 in the result if the value
 *  is inside the specified min/max range, otherwise all 1’s will
 *  be written to the result.
 */
_EXT_ vxc_uchar16 viv_intrinsic_vx_Clamp_uc(vxc_uchar16 a, vxc_uchar16 b, vxc_uchar16 c) _RET0_
_EXT_ vxc_char16  viv_intrinsic_vx_Clamp_c(vxc_char16 a, vxc_char16 b, vxc_char16 c) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vx_Clamp_s(vxc_short8 a, vxc_short8 b, vxc_short8 c) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vx_Clamp_us(vxc_ushort8 a, vxc_ushort8 b, vxc_ushort8 c) _RET0_

_EXT_ vxc_uchar16 viv_intrinsic_vx_ClampBoolean_uc(vxc_uchar16 a, vxc_uchar16 b, vxc_uchar16 c) _RET0_
_EXT_ vxc_char16  viv_intrinsic_vx_ClampBoolean_c(vxc_char16 a, vxc_char16 b, vxc_char16 c) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vx_ClampBoolean_s(vxc_short8 a, vxc_short8 b, vxc_short8 c) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vx_ClampBoolean_us(vxc_ushort8 a, vxc_ushort8 b, vxc_ushort8 c) _RET0_

/* BiLinear: computes a bi-linear interpolation of 4 pixel values.
 *
 * Syntax:
 *      r = BiLinear(a, b, c) ;
 * Semantics:
 *      r[i] =   a[i]   * (1 ? c.x) * (1 ? c.y)
 *             + a[i+1] * c.x * (1 ? c.y)
 *             + b[i]   * (1 ? c.x) * c.y
 *             + b[i+1] * c.x * c.y
 */
_EXT_ vxc_uchar16 viv_intrinsic_vx_BiLinear_uc(vxc_uchar16 a, vxc_uchar16 b, float2 c) _RET0_
_EXT_ vxc_char16  viv_intrinsic_vx_BiLinear_c(vxc_char16 a, vxc_char16 b, float2 c) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vx_BiLinear_s(vxc_short8 a, vxc_short8 b, float2 c) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vx_BiLinear_us(vxc_ushort8 a, vxc_ushort8 b, float2 c) _RET0_


/* SelectAdd: either adds the pixel value or increments a counter
 *            inside a number of distribution (histogram) bins
 *
 * Syntax:
 *      r = SelectAdd(a, b, c) ;
 * Semantics:
 *      r[i] =   a[c[i]]  + b[c[i]] ;  i E [0, 7]
 */
_EXT_ vxc_uchar16 viv_intrinsic_vx_SelectAdd_uc(vxc_uchar16 a, vxc_uchar16 b, vxc_512bits c) _RET0_
_EXT_ vxc_char16  viv_intrinsic_vx_SelectAdd_c(vxc_char16 a, vxc_char16 b, vxc_512bits c) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vx_SelectAdd_s(vxc_short8 a, vxc_short8 b, vxc_512bits c) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vx_SelectAdd_us(vxc_ushort8 a, vxc_ushort8 b, vxc_512bits c) _RET0_

/* AtomicAdd: adds a valid atomically to a given address.
 *            It is infact a read/modify/write instruction
 *            that executes atomically
 *
 * Syntax:
 *       AtomicAdd(a, b, c) ;  // a -> base, b -> offset, c -> add value
 * Semantics:
 *      (a + offset)[i] =  (a + offset)[i] + c[j];  i E [0, 7]
 */
_EXT_ vxc_uchar16 viv_intrinsic_vx_AtomicAdd_uc(vxc_uchar16 * a, vxc_int offset, vxc_uchar16 c) _RET0_
_EXT_ vxc_char16  viv_intrinsic_vx_AtomicAdd_c(vxc_char16 * a,  vxc_int offset, vxc_char16 c) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vx_AtomicAdd_s(vxc_short8 * a,  vxc_int offset, vxc_short8 c) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vx_AtomicAdd_us(vxc_ushort8 * a, vxc_int offset, vxc_ushort8 c) _RET0_

/* BitExtract: extracts up to 8 bitfields from a packed data stream.
 *           The input is is a 256-bit blob of data.
 *
 * Syntax:
 *       r = BitExtract(a, b, c) ;
 * Semantics:
 *       r[i] = ({b, a} >> c[i]) ^ ((1 << c[i+8]) ? 1) ;  i E [0, 7]
 */
_EXT_ vxc_ushort8 viv_intrinsic_vx_BitExtract_us(vxc_ushort8 a, vxc_ushort8 b, vxc_uchar16 c) _RET0_
_EXT_ vxc_uchar8  viv_intrinsic_vx_BitExtract_uc(vxc_uchar16 a, vxc_uchar16 b, vxc_uchar16 c) _RET0_

/* BitReplace: replaces up to 8 bitfields inside a packed data stream.
 * Syntax:
 *       r = BitReplace(a, b, c) ;
 * Semantics:
 */
_EXT_ vxc_ushort8 viv_intrinsic_vx_BitReplace_us(vxc_ushort8 a, vxc_ushort8 b, vxc_uchar16 c) _RET0_
_EXT_ vxc_uchar16 viv_intrinsic_vx_BitReplace_uc(vxc_uchar16 a, vxc_uchar16 b, vxc_uchar16 c) _RET0_

/* vloadn: read packed vector type from memory as packed in register
           where n takes the value of 2, 3, 4, 8, 16
 * Syntax:
 *    _viv_gentypen_packed dest;
 *    dest = vloadn(offset, gentype *p);
 * Semantics:
 *    gentype is the generic type to indicate the built-in data types
 *    char, uchar, short, ushort.
 *    Return sizeof(gentypen) bytes of data read
 *    from address (p + (offset * n)). The
 *    address computed as (p + (offset * n)) must
 *    be 8-bit aligned if gentype is char, uchar;
 *    16-bit aligned if gentype is short, ushort;
 */
_EXT_ vxc_char2 viv_intrinsic_vx_vload2(size_t offset, char *p)  _RET0_
_EXT_ vxc_char4 viv_intrinsic_vx_vload4(size_t offset, char *p)  _RET0_
_EXT_ vxc_char8 viv_intrinsic_vx_vload8(size_t offset, char *p)  _RET0_
_EXT_ vxc_char16 viv_intrinsic_vx_vload16(size_t offset, char *p)  _RET0_

_EXT_ vxc_uchar2 viv_intrinsic_vx_vload2(size_t offset, uchar *p)  _RET0_
_EXT_ vxc_uchar4 viv_intrinsic_vx_vload4(size_t offset, uchar *p)  _RET0_
_EXT_ vxc_uchar8 viv_intrinsic_vx_vload8(size_t offset, uchar *p)  _RET0_
_EXT_ vxc_uchar16 viv_intrinsic_vx_vload16(size_t offset, uchar *p)  _RET0_

_EXT_ vxc_short2 viv_intrinsic_vx_vload2(size_t offset, short *p)  _RET0_
_EXT_ vxc_short4 viv_intrinsic_vx_vload4(size_t offset, short *p)  _RET0_
_EXT_ vxc_short8 viv_intrinsic_vx_vload8(size_t offset, short *p)  _RET0_

_EXT_ vxc_ushort2 viv_intrinsic_vx_vload2(size_t offset, ushort *p)  _RET0_
_EXT_ vxc_ushort4 viv_intrinsic_vx_vload4(size_t offset, ushort *p)  _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vx_vload8(size_t offset, ushort *p)  _RET0_

/* storen: write packed vector type to memory
          where n takes the value of 2, 3, 4, 8, 16
* Syntax:
*    _viv_gentypen_packed dest;
*    void vloadn(_viv_gentypen_packed data, offset, gentype *p);
* Semantics:
*    gentype is the generic type to indicate the built-in data types
*    char, uchar, short, ushort.
*    Write sizeof (_viv_gentypen_packed) bytes given by
*    data to address (p + (offset * n)). The
*    address computed as (p + (offset * n)) must
*    be 8-bit aligned if gentype is char, uchar;
*    16-bit aligned if gentype is short, ushort;
*/
_EXT_ void viv_intrinsic_vx_vstore2(vxc_char2, size_t offset, char *p) _RET_
_EXT_ void viv_intrinsic_vx_vstore4(vxc_char4, size_t offset, char *p) _RET_
_EXT_ void viv_intrinsic_vx_vstore8(vxc_char8, size_t offset, char *p) _RET_
_EXT_ void viv_intrinsic_vx_vstore16(vxc_char16, size_t offset, char *p) _RET_

_EXT_ void viv_intrinsic_vx_vstore2(vxc_uchar2, size_t offset, uchar *p) _RET_
_EXT_ void viv_intrinsic_vx_vstore4(vxc_uchar4, size_t offset, uchar *p) _RET_
_EXT_ void viv_intrinsic_vx_vstore8(vxc_uchar8, size_t offset, uchar *p) _RET_
_EXT_ void viv_intrinsic_vx_vstore16(vxc_uchar16, size_t offset, uchar *p) _RET_

_EXT_ void viv_intrinsic_vx_vstore2(vxc_short2, size_t offset, short *p) _RET_
_EXT_ void viv_intrinsic_vx_vstore4(vxc_short4, size_t offset, short *p) _RET_
_EXT_ void viv_intrinsic_vx_vstore8(vxc_short8, size_t offset, short *p) _RET_

_EXT_ void viv_intrinsic_vx_vstore2(vxc_ushort2, size_t offset, ushort *p) _RET_
_EXT_ void viv_intrinsic_vx_vstore4(vxc_ushort4, size_t offset, ushort *p) _RET_
_EXT_ void viv_intrinsic_vx_vstore8(vxc_ushort8, size_t offset, ushort *p) _RET_

/* direct mapping to machine code, with bin and rounding mode info */
/* samplerless image read */

/*  offsetXY [ 4: 0] S05 relative x offset
 *           [ 9: 5] S05 relative y offset
 */
_EXT_ vxc_char16  viv_intrinsic_vxmc_read_imagec   (image2d_t image, int2 coord, int offsetXY, vxc_modifier modifier) _RET0_
_EXT_ vxc_uchar16 viv_intrinsic_vxmc_read_imageuc  (image2d_t image, int2 coord, int offsetXY, vxc_modifier modifier) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vxmc_read_images   (image2d_t image, int2 coord, int offsetXY, vxc_modifier modifier) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vxmc_read_imageus  (image2d_t image, int2 coord, int offsetXY, vxc_modifier modifier) _RET0_

_EXT_ vxc_char16  viv_intrinsic_vxmc_read_imagec   (image1d_t image, int coord, int offsetX, vxc_modifier modifier) _RET0_
_EXT_ vxc_uchar16 viv_intrinsic_vxmc_read_imageuc  (image1d_t image, int coord, int offsetX, vxc_modifier modifier) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vxmc_read_images   (image1d_t image, int coord, int offsetX, vxc_modifier modifier) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vxmc_read_imageus  (image1d_t image, int coord, int offsetX, vxc_modifier modifier) _RET0_

_EXT_ vxc_char16  viv_intrinsic_vxmc_read_imagec   (image1d_array_t image, int2 coord, int offsetXY, vxc_modifier modifier) _RET0_
_EXT_ vxc_uchar16 viv_intrinsic_vxmc_read_imageuc  (image1d_array_t image, int2 coord, int offsetXY, vxc_modifier modifier) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vxmc_read_images   (image1d_array_t image, int2 coord, int offsetXY, vxc_modifier modifier) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vxmc_read_imageus  (image1d_array_t image, int2 coord, int offsetXY, vxc_modifier modifier) _RET0_

/* image write */
_EXT_ void viv_intrinsic_vxmc_write_imagec  (image2d_t image, int2 coord, vxc_char16 color, vxc_modifier modifier) _RET_
_EXT_ void viv_intrinsic_vxmc_write_imageuc (image2d_t image, int2 coord, vxc_uchar16 color, vxc_modifier modifier) _RET_
_EXT_ void viv_intrinsic_vxmc_write_images  (image2d_t image, int2 coord, vxc_short8 color, vxc_modifier modifier) _RET_
_EXT_ void viv_intrinsic_vxmc_write_imageus (image2d_t image, int2 coord, vxc_ushort8 color, vxc_modifier modifier) _RET_

_EXT_ void viv_intrinsic_vxmc_write_imagec  (image1d_t image, int coord, vxc_char16 color, vxc_modifier modifier) _RET_
_EXT_ void viv_intrinsic_vxmc_write_imageuc (image1d_t image, int coord, vxc_uchar16 color, vxc_modifier modifier) _RET_
_EXT_ void viv_intrinsic_vxmc_write_images  (image1d_t image, int coord, vxc_short8 color, vxc_modifier modifier) _RET_
_EXT_ void viv_intrinsic_vxmc_write_imageus (image1d_t image, int coord, vxc_ushort8 color, vxc_modifier modifier) _RET_

_EXT_ void viv_intrinsic_vxmc_write_imagec  (image1d_array_t image, int2 coord, vxc_char16 color, vxc_modifier modifier) _RET_
_EXT_ void viv_intrinsic_vxmc_write_imageuc (image1d_array_t image, int2 coord, vxc_uchar16 color, vxc_modifier modifier) _RET_
_EXT_ void viv_intrinsic_vxmc_write_images  (image1d_array_t image, int2 coord, vxc_short8 color, vxc_modifier modifier) _RET_
_EXT_ void viv_intrinsic_vxmc_write_imageus (image1d_array_t image, int2 coord, vxc_ushort8 color, vxc_modifier modifier) _RET_

/* AbsDiff */
_EXT_ vxc_uchar16 viv_intrinsic_vxmc_AbsDiff_uc(vxc_uchar16 a, vxc_uchar16 b, vxc_modifier modifier) _RET0_
_EXT_ vxc_char16  viv_intrinsic_vxmc_AbsDiff_c(vxc_char16 a, vxc_char16 b,   vxc_modifier modifier) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vxmc_AbsDiff_s(vxc_short8 a, vxc_short8 b,   vxc_modifier modifier) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vxmc_AbsDiff_us(vxc_ushort8 a, vxc_ushort8 b, vxc_modifier modifier) _RET0_

/* IAdd */
_EXT_ vxc_uchar16 viv_intrinsic_vxmc_IAdd_uc(vxc_uchar16 a, vxc_uchar16 b, vxc_uchar16 c, vxc_modifier modifier) _RET0_
_EXT_ vxc_char16  viv_intrinsic_vxmc_IAdd_c(vxc_char16 a, vxc_char16 b, vxc_char16 c, vxc_modifier modifier) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vxmc_IAdd_s(vxc_short8 a, vxc_short8 b, vxc_short8 c, vxc_modifier modifier) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vxmc_IAdd_us(vxc_ushort8 a, vxc_ushort8 b, vxc_ushort8 c, vxc_modifier modifier) _RET0_

/* IAccSq */
_EXT_ vxc_uchar16 viv_intrinsic_vxmc_IAccSq_uc(vxc_uchar16 a, vxc_uchar16 b, uint Imm, vxc_modifier modifier) _RET0_
_EXT_ vxc_char16  viv_intrinsic_vxmc_IAccSq_c(vxc_char16 a, vxc_char16 b, uint Imm, vxc_modifier modifier) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vxmc_IAccSq_s(vxc_short8 a, vxc_short8 b, uint Imm, vxc_modifier modifier) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vxmc_IAccSq_us(vxc_ushort8 a, vxc_ushort8 b, uint Imm, vxc_modifier modifier) _RET0_

/* Lerp */
_EXT_ vxc_uchar16 viv_intrinsic_vxmc_Lerp_uc(vxc_uchar16 a, vxc_uchar16 b, vxc_float c, vxc_modifier modifier) _RET0_
_EXT_ vxc_char16  viv_intrinsic_vxmc_Lerp_c(vxc_char16 a, vxc_char16 b, vxc_float c, vxc_modifier modifier) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vxmc_Lerp_s(vxc_short8 a, vxc_short8 b, vxc_float c, vxc_modifier modifier) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vxmc_Lerp_us(vxc_ushort8 a, vxc_ushort8 b, vxc_float c, vxc_modifier modifier) _RET0_

/* Filter */
_EXT_ vxc_uchar16 viv_intrinsic_vxmc_Filter_uc(vxc_uchar16 a, vxc_uchar16 b, vxc_uchar16 c, vxc_modifier modifier) _RET0_
_EXT_ vxc_char16  viv_intrinsic_vxmc_Filter_c(vxc_char16 a, vxc_char16 b, vxc_char16 c, vxc_modifier modifier) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vxmc_Filter_s(vxc_short8 a, vxc_short8 b, vxc_short8 c, vxc_modifier modifier) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vxmc_Filter_us(vxc_ushort8 a, vxc_ushort8 b, vxc_ushort8 c, vxc_modifier modifier) _RET0_

/* MagPhase */
_EXT_ vxc_uchar16 viv_intrinsic_vxmc_MagPhase_uc(vxc_uchar16 a, vxc_uchar16 b, vxc_modifier modifier) _RET0_
_EXT_ vxc_char16  viv_intrinsic_vxmc_MagPhase_c(vxc_char16 a, vxc_char16 b, vxc_modifier modifier) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vxmc_MagPhase_s(vxc_short8 a, vxc_short8 b, vxc_modifier modifier) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vxmc_MagPhase_us(vxc_ushort8 a, vxc_ushort8 b, vxc_modifier modifier) _RET0_

/* MulShift */
_EXT_ vxc_uchar16 viv_intrinsic_vxmc_MulShift_uc(vxc_uchar16 a, vxc_uchar16 b, uint Imm, vxc_modifier modifier) _RET0_
_EXT_ vxc_char16  viv_intrinsic_vxmc_MulShift_c(vxc_char16 a, vxc_char16 b, uint Imm, vxc_modifier modifier) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vxmc_MulShift_s(vxc_short8 a, vxc_short8 b, uint Imm, vxc_modifier modifier) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vxmc_MulShift_us(vxc_ushort8 a, vxc_ushort8 b, uint Imm, vxc_modifier modifier) _RET0_

/* DP16x1: performs a dot-product of two 16-component values
 *
 * Syntax:
 *      r = DP16x1(a, b);
 *
 * Semantics:
 *      r = Sigma(a[i] * b[i]);  i E [0, 15]
 */
_EXT_ vxc_uint   viv_intrinsic_vxmc_DP16x1(vxc_uchar16 a,  vxc_uchar16 b, vxc_modifier modifier, vxc_512bits u) _RET0_
_EXT_ vxc_int    viv_intrinsic_vxmc_DP16x1(vxc_char16 a,   vxc_char16 b, vxc_modifier modifier, vxc_512bits u) _RET0_

/* DP8x2: performs two dot-product of two 8-component values.
 *
 * Syntax:
 *      r = DP8x2(a, b);
 *
 * Semantics:
 *      r[0] = Sigma(a[i] * b[i]);  i E [0, 7]
 *      r[1] = Sigma(a[i] * b[i]);  i E [8, 15]
 */
_EXT_ vxc_uint2   viv_intrinsic_vxmc_DP8x2(vxc_uchar16 a,  vxc_uchar16 b, vxc_modifier modifier, vxc_512bits u) _RET0_
_EXT_ vxc_int2    viv_intrinsic_vxmc_DP8x2(vxc_char16 a,   vxc_char16 b, vxc_modifier modifier, vxc_512bits u) _RET0_

/* DP4x4: performs four dot-product of two 4-component values.
 *
 * Syntax:
 *      r = DP4x4(a, b);
 *
 * Semantics:
 *      r[0] = Sigma(a[i] * b[i]);  i E [0, 3]
 *      r[1] = Sigma(a[i] * b[i]);  i E [4, 7]
 *      r[2] = Sigma(a[i] * b[i]);  i E [8, 11]
 *      r[3] = Sigma(a[i] * b[i]);  i E [12, 15]
 */
_EXT_ vxc_uint4   viv_intrinsic_vxmc_DP4x4(vxc_uchar16 a,  vxc_uchar16 b, vxc_modifier modifier, vxc_512bits u) _RET0_
_EXT_ vxc_int4    viv_intrinsic_vxmc_DP4x4(vxc_char16 a,   vxc_char16 b, vxc_modifier modifier, vxc_512bits u) _RET0_

/* DP2x8: performs eight dot-product of two 2-component values.
 *
 * Syntax:
 *      r = DP2x8(a, b);
 *
 * Semantics:
 *      r[0] = Sigma(a[i] * b[i]);  i E [0, 1]
 *      r[1] = Sigma(a[i] * b[i]);  i E [2, 3]
 *      r[2] = Sigma(a[i] * b[i]);  i E [4, 5]
 *      r[3] = Sigma(a[i] * b[i]);  i E [6, 7]
 *      r[4] = Sigma(a[i] * b[i]);  i E [8, 9]
 *      r[5] = Sigma(a[i] * b[i]);  i E [10, 11]
 *      r[6] = Sigma(a[i] * b[i]);  i E [12, 13]
 *      r[7] = Sigma(a[i] * b[i]);  i E [14, 15]
 */
_EXT_ vxc_uchar8  viv_intrinsic_vxmc_DP2x8(vxc_uchar16 a,  vxc_uchar16 b, vxc_modifier modifier, vxc_512bits u) _RET0_
_EXT_ vxc_char8   viv_intrinsic_vxmc_DP2x8(vxc_char16 a,   vxc_char16 b, vxc_modifier modifier, vxc_512bits u) _RET0_

/* DP32 <a, b> dot constant */
_EXT_ vxc_uint    viv_intrinsic_vxmc_DP32x1(vxc_uchar16 a,  vxc_uchar16 b, vxc_modifier modifier, vxc_512bits u) _RET0_
_EXT_ vxc_int     viv_intrinsic_vxmc_DP32x1(vxc_char16 a,   vxc_char16 b,  vxc_modifier modifier, vxc_512bits u) _RET0_

_EXT_ vxc_uint2   viv_intrinsic_vxmc_DP16x2(vxc_uchar16 a,  vxc_uchar16 b, vxc_modifier modifier, vxc_512bits u) _RET0_
_EXT_ vxc_int2    viv_intrinsic_vxmc_DP16x2(vxc_char16 a,   vxc_char16 b,  vxc_modifier modifier, vxc_512bits u) _RET0_

_EXT_ vxc_uint4   viv_intrinsic_vxmc_DP8x4(vxc_uchar16 a,  vxc_uchar16 b, vxc_modifier modifier, vxc_512bits u) _RET0_
_EXT_ vxc_int4    viv_intrinsic_vxmc_DP8x4(vxc_char16 a,   vxc_char16 b,  vxc_modifier modifier, vxc_512bits u) _RET0_

_EXT_ vxc_ushort8 viv_intrinsic_vxmc_DP4x8(vxc_uchar16 a,  vxc_uchar16 b, vxc_modifier modifier, vxc_512bits u) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vxmc_DP4x8(vxc_char16 a,   vxc_char16 b,  vxc_modifier modifier, vxc_512bits u) _RET0_

_EXT_ vxc_uchar16 viv_intrinsic_vxmc_DP2x16(vxc_uchar16 a,  vxc_uchar16 b, vxc_modifier modifier, vxc_512bits u) _RET0_
_EXT_ vxc_char16  viv_intrinsic_vxmc_DP2x16(vxc_char16 a,   vxc_char16 b,  vxc_modifier modifier, vxc_512bits u) _RET0_

/* DP32 <a, b> dot c
 *  vxc_char32 a;
 *  vxc_char16 b;
 *  vxc_int result = viv_intrinsic_vxmc_DP32x1_b(a.hi, a.lo, b, modifier, u);
 */
_EXT_ vxc_uint    viv_intrinsic_vxmc_DP32x1_b(vxc_uchar16 a_hi,  vxc_uchar16 a_lo, vxc_uchar16 b, vxc_modifier modifier, vxc_512bits u) _RET0_
_EXT_ vxc_int     viv_intrinsic_vxmc_DP32x1_b(vxc_char16 a_hi,   vxc_char16 a_lo,  vxc_char16 b,  vxc_modifier modifier, vxc_512bits u) _RET0_

_EXT_ vxc_uint2   viv_intrinsic_vxmc_DP16x2_b(vxc_uchar16 a_hi,  vxc_uchar16 a_lo, vxc_uchar16 b, vxc_modifier modifier, vxc_512bits u) _RET0_
_EXT_ vxc_int2    viv_intrinsic_vxmc_DP16x2_b(vxc_char16 a_hi,   vxc_char16 a_lo,  vxc_char16 b,  vxc_modifier modifier, vxc_512bits u) _RET0_

_EXT_ vxc_uint4   viv_intrinsic_vxmc_DP8x4_b(vxc_uchar16 a_hi,  vxc_uchar16 a_lo, vxc_uchar16 b, vxc_modifier modifier, vxc_512bits u) _RET0_
_EXT_ vxc_int4    viv_intrinsic_vxmc_DP8x4_b(vxc_char16 a_hi,   vxc_char16 a_lo,  vxc_char16 b,  vxc_modifier modifier, vxc_512bits u) _RET0_

_EXT_ vxc_ushort8 viv_intrinsic_vxmc_DP4x8_b(vxc_uchar16 a_hi,  vxc_uchar16 a_lo, vxc_uchar16 b, vxc_modifier modifier, vxc_512bits u) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vxmc_DP4x8_b(vxc_char16 a_hi,   vxc_char16 a_lo,  vxc_char16 b,  vxc_modifier modifier, vxc_512bits u) _RET0_

_EXT_ vxc_uchar16 viv_intrinsic_vxmc_DP2x16_b(vxc_uchar16 a_hi,  vxc_uchar16 a_lo, vxc_uchar16 b, vxc_modifier modifier, vxc_512bits u) _RET0_
_EXT_ vxc_char16  viv_intrinsic_vxmc_DP2x16_b(vxc_char16 a_hi,   vxc_char16 a_lo,  vxc_char16 b,  vxc_modifier modifier, vxc_512bits u) _RET0_

/* Clamp */
_EXT_ vxc_uchar16 viv_intrinsic_vxmc_Clamp_uc(vxc_uchar16 a, vxc_uchar16 b, vxc_uchar16 c, vxc_modifier modifier) _RET0_
_EXT_ vxc_char16  viv_intrinsic_vxmc_Clamp_c(vxc_char16 a, vxc_char16 b, vxc_char16 c, vxc_modifier modifier) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vxmc_Clamp_s(vxc_short8 a, vxc_short8 b, vxc_short8 c, vxc_modifier modifier) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vxmc_Clamp_us(vxc_ushort8 a, vxc_ushort8 b, vxc_ushort8 c, vxc_modifier modifier) _RET0_

/* BiLinear */
_EXT_ vxc_uchar16 viv_intrinsic_vxmc_BiLinear_uc(vxc_uchar16 a, vxc_uchar16 b, float2 c, vxc_modifier modifier) _RET0_
_EXT_ vxc_char16  viv_intrinsic_vxmc_BiLinear_c(vxc_char16 a, vxc_char16 b, float2 c, vxc_modifier modifier) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vxmc_BiLinear_s(vxc_short8 a, vxc_short8 b, float2 c, vxc_modifier modifier) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vxmc_BiLinear_us(vxc_ushort8 a, vxc_ushort8 b, float2 c, vxc_modifier modifier) _RET0_

/* SelectAdd */
_EXT_ vxc_uchar16 viv_intrinsic_vxmc_SelectAdd_uc(vxc_uchar16 a, vxc_uchar16 b, vxc_512bits c, vxc_modifier modifier) _RET0_
_EXT_ vxc_char16  viv_intrinsic_vxmc_SelectAdd_c(vxc_char16 a,  vxc_char16 b,  vxc_512bits c, vxc_modifier modifier) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vxmc_SelectAdd_s(vxc_short8 a,  vxc_short8 b,  vxc_512bits c, vxc_modifier modifier) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vxmc_SelectAdd_us(vxc_ushort8 a, vxc_ushort8 b, vxc_512bits c, vxc_modifier modifier) _RET0_

/* AtomicAdd */
_EXT_ vxc_uchar16 viv_intrinsic_vxmc_AtomicAdd_uc(vxc_uchar16 * a, vxc_int offset, vxc_uchar16 c, vxc_modifier modifier) _RET0_
_EXT_ vxc_char16  viv_intrinsic_vxmc_AtomicAdd_c(vxc_char16 * a,  vxc_int offset, vxc_char16 c, vxc_modifier modifier) _RET0_
_EXT_ vxc_short8  viv_intrinsic_vxmc_AtomicAdd_s(vxc_short8 * a,  vxc_int offset,  vxc_short8 c, vxc_modifier modifier) _RET0_
_EXT_ vxc_ushort8 viv_intrinsic_vxmc_AtomicAdd_us(vxc_ushort8 * a, vxc_int offset, vxc_ushort8 c, vxc_modifier modifier) _RET0_

/* BitExtract */
_EXT_ vxc_ushort8 viv_intrinsic_vxmc_BitExtract_us(vxc_ushort8 a, vxc_ushort8 b, vxc_uchar16 c, vxc_modifier modifier) _RET0_
_EXT_ vxc_uchar8  viv_intrinsic_vxmc_BitExtract_uc(vxc_uchar16 a, vxc_uchar16 b, vxc_uchar16 c, vxc_modifier modifier) _RET0_

/* BitReplace */
_EXT_ vxc_ushort8 viv_intrinsic_vxmc_BitReplace_us(vxc_ushort8 a, vxc_ushort8 b, vxc_uchar16 c, vxc_modifier modifier) _RET0_
_EXT_ vxc_uchar16 viv_intrinsic_vxmc_BitReplace_uc(vxc_uchar16 a, vxc_uchar16 b, vxc_uchar16 c, vxc_modifier modifier) _RET0_

#undef _RET0_
#undef _RET_
#undef _EXT_


#ifdef __cplusplus
}
#endif

#endif /* _VIV_VX_EXTENSION */

#endif
