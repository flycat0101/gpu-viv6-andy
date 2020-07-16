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


/***********************************************************************************
* Copyright:    Verisilicon
* FileName:        archSwType.c
* Author:        JinboHuang
* Data:            2019-05-27
* Version:        0.5.00
* Description:    Type definition for Arch Model Software Librart
*
***************************************************************************************/

#ifndef _ARCH_SW_TYPE_H_
#define _ARCH_SW_TYPE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>

#if defined(__linux__) || defined(__ANDROID__) || defined(__QNX__) || defined(__APPLE__) || defined(__CYGWIN__)
#include <dlfcn.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#elif defined(_WIN32) || defined(UNDER_CE)
#include <windows.h>
#endif

#ifdef __cplusplus

#ifndef EXTERN_C_BEGIN
#define EXTERN_C_BEGIN                      extern "C" {
#endif

#ifndef EXTERN_C_END
#define EXTERN_C_END                        }
#endif

#ifndef EXTERN_C
#define EXTERN_C                            extern "C"
#endif

#else

#ifndef EXTERN_C_BEGIN
#define EXTERN_C_BEGIN
#endif

#ifndef EXTERN_C_END
#define EXTERN_C_END
#endif

#ifndef EXTERN_C
#define EXTERN_C
#endif

#endif /* __cplusplus */


/*************************************** Basic data type definition ************************************/
typedef char     arch_char;
typedef unsigned char  arch_uint8;
typedef unsigned short arch_uint16;
typedef unsigned int arch_uint32;
typedef unsigned long long arch_uint64;

typedef char   arch_int8;
typedef short  arch_int16;
typedef int        arch_int32;
typedef long long  arch_int64;

typedef float    arch_float32;
typedef double   arch_float64;

typedef int arch_status; // refine me
typedef int arch_bool;

typedef void * archPOINTER;

typedef enum _arch_bool_e {
    arch_false_e = 0,
    arch_true_e  = 1,
} arch_bool_e;


/*************************************** MACRO definition ************************************/
/* SW tiling source */
#define SW_TILING_FROM_DDR          0
#define SW_TILING_FROM_AXI_SRAM     1
#define SW_TILING_FROM_VIP_SRAM     2
#define SW_TILING_PERM_AXI_SRAM     3
#define SW_TILING_PERM_VIP_SRAM     4

/* function type definition */
#define ARCH_PRIVATE_API                      static
#define ARCH_INTERNAL_API
#define ARCH_NULL                             NULL


#define archMIN(x, y)            (((x) <= (y)) ?  (x) :  (y))
#define archMAX(x, y)            (((x) >= (y)) ?  (x) :  (y))

#define archIS_ERROR(status)         (status < 0)
#define archNO_ERROR(status)         (status >= 0)
#define archIS_SUCCESS(status)       (status == archSTATUS_OK)


/******************************************************************************\
******************************* Alignment Macros *******************************
\******************************************************************************/

/* Alignment with a non-power of two value. */
#define archALIGN_NP2(n, align) (((n) + (align) - 1) - (((n) + (align) - 1) % (align)))

#define archALIGN_NP2_SAFE(n, align)                                        \
(                                                                          \
    (archALIGN_NP2((n) & ~0ULL, (align) & ~0ULL) ^ archALIGN_NP2(n, align)) ?   \
        (n) : archALIGN_NP2(n, align)                                       \
)

/* Alignment with a power of two value. */
#define archALIGN(n, align) (((n) + ((align) - 1)) & ~((align) - 1))

#define archALIGN_SAFE(n, align)                                        \
(                                                                      \
    (archALIGN((n) & ~0ULL, (align) & ~0ULL) ^ archALIGN(n, align)) ?    \
         (n) : archALIGN(n, align)                                      \
)

#define archALIGN_BASE(n, align) \
( \
    ((n) & ~((align) - 1)) \
)



/* TBD */
#define ARCH_VIP_SRAM_IMAGE_STREAM_SIZE  2048

/* SWTILING PHASE */
enum
{
    ARCH_SWTILING_OPTION_OFF    = 0,
    ARCH_SWTILING_OPTION_ALL    = 1,
    ARCH_SWTILING_OPTION_AB     = 2,
    ARCH_SWTILING_OPTION_TILING = 3,
};

/* Chip features. */
typedef enum _archFEATURE
{
    archFEATURE_PIPE_2D = 0,
    archFEATURE_PIPE_3D,
    archFEATURE_PIPE_VG,
    archFEATURE_DC,
    archFEATURE_HIGH_DYNAMIC_RANGE,
    archFEATURE_MODULE_CG,
    archFEATURE_MIN_AREA,
    archFEATURE_BUFFER_INTERLEAVING,
    archFEATURE_BYTE_WRITE_2D,
    archFEATURE_ENDIANNESS_CONFIG,
    archFEATURE_DUAL_RETURN_BUS,
    archFEATURE_DEBUG_MODE,
    archFEATURE_YUY2_RENDER_TARGET,
    archFEATURE_FRAGMENT_PROCESSOR,
    archFEATURE_2DPE20,
    archFEATURE_FAST_CLEAR,
    archFEATURE_YUV420_TILER,
    archFEATURE_YUY2_AVERAGING,
    archFEATURE_FLIP_Y,
    archFEATURE_EARLY_Z,
    archFEATURE_COMPRESSION,
    archFEATURE_MSAA,
    archFEATURE_SPECIAL_ANTI_ALIASING,
    archFEATURE_SPECIAL_MSAA_LOD,
    archFEATURE_422_TEXTURE_COMPRESSION,
    archFEATURE_DXT_TEXTURE_COMPRESSION,
    archFEATURE_ETC1_TEXTURE_COMPRESSION,
    archFEATURE_CORRECT_TEXTURE_CONVERTER,
    archFEATURE_TEXTURE_8K,
    archFEATURE_SCALER,
    archFEATURE_YUV420_SCALER,
    archFEATURE_SHADER_HAS_W,
    archFEATURE_SHADER_HAS_SIGN,
    archFEATURE_SHADER_HAS_FLOOR,
    archFEATURE_SHADER_HAS_CEIL,
    archFEATURE_SHADER_HAS_SQRT,
    archFEATURE_SHADER_HAS_TRIG,
    archFEATURE_HZ,
    archFEATURE_CORRECT_STENCIL,
    archFEATURE_VG20,
    archFEATURE_VG_FILTER,
    archFEATURE_VG21,
    archFEATURE_VG_DOUBLE_BUFFER,
    archFEATURE_VG_RESOLUTION_8K,
    archFEATURE_MC20,
    archFEATURE_SUPER_TILED,
    archFEATURE_FAST_CLEAR_FLUSH,
    archFEATURE_2D_FILTERBLIT_PLUS_ALPHABLEND,
    archFEATURE_2D_DITHER,
    archFEATURE_2D_A8_TARGET,
    archFEATURE_2D_A8_NO_ALPHA,
    archFEATURE_2D_FILTERBLIT_FULLROTATION,
    archFEATURE_2D_BITBLIT_FULLROTATION,
    archFEATURE_WIDE_LINE,
    archFEATURE_FC_FLUSH_STALL,
    archFEATURE_FULL_DIRECTFB,
    archFEATURE_HALF_FLOAT_PIPE,
    archFEATURE_LINE_LOOP,
    archFEATURE_2D_YUV_BLIT,
    archFEATURE_2D_TILING,
    archFEATURE_NON_POWER_OF_TWO,
    archFEATURE_3D_TEXTURE,
    archFEATURE_TEXTURE_ARRAY,
    archFEATURE_TILE_FILLER,
    archFEATURE_LOGIC_OP,
    archFEATURE_MIXED_STREAMS,
    archFEATURE_2D_MULTI_SOURCE_BLT,
    archFEATURE_END_EVENT,
    archFEATURE_VERTEX_10_10_10_2,
    archFEATURE_TEXTURE_10_10_10_2,
    archFEATURE_TEXTURE_ANISOTROPIC_FILTERING,
    archFEATURE_TEXTURE_FLOAT_HALF_FLOAT,
    archFEATURE_2D_ROTATION_STALL_FIX,
    archFEATURE_2D_MULTI_SOURCE_BLT_EX,
    archFEATURE_BUG_FIXES10,
    archFEATURE_2D_MINOR_TILING,
    archFEATURE_TEX_COMPRRESSION_SUPERTILED, /* Supertiled compressed textures are supported. */
    archFEATURE_FAST_MSAA,
    archFEATURE_BUG_FIXED_INDEXED_TRIANGLE_STRIP,
    archFEATURE_TEXTURE_TILE_STATUS_READ,
    archFEATURE_DEPTH_BIAS_FIX,
    archFEATURE_RECT_PRIMITIVE,
    archFEATURE_BUG_FIXES11,
    archFEATURE_SUPERTILED_TEXTURE,
    archFEATURE_2D_NO_COLORBRUSH_INDEX8,
    archFEATURE_RS_YUV_TARGET,
    archFEATURE_2D_FC_SOURCE, /* For tilestatus compression feature*/
    archFEATURE_2D_CC_NOAA_SOURCE,
    archFEATURE_PE_DITHER_FIX,
    archFEATURE_2D_YUV_SEPARATE_STRIDE,
    archFEATURE_FRUSTUM_CLIP_FIX,
    archFEATURE_TEXTURE_SWIZZLE,
    archFEATURE_PRIMITIVE_RESTART,
    archFEATURE_TEXTURE_LINEAR,
    archFEATURE_TEXTURE_YUV_ASSEMBLER,
    archFEATURE_LINEAR_RENDER_TARGET,
    archFEATURE_SHADER_HAS_ATOMIC,
    archFEATURE_SHADER_HAS_INSTRUCTION_CACHE,
    archFEATURE_SHADER_ENHANCEMENTS2,
    archFEATURE_BUG_FIXES7,
    archFEATURE_SHADER_HAS_RTNE,
    archFEATURE_SHADER_HAS_EXTRA_INSTRUCTIONS2,
    archFEATURE_SHADER_ENHANCEMENTS3,
    archFEATURE_DYNAMIC_FREQUENCY_SCALING,
    archFEATURE_SINGLE_BUFFER,
    archFEATURE_OCCLUSION_QUERY,
    archFEATURE_2D_GAMMA,
    archFEATURE_2D_COLOR_SPACE_CONVERSION,
    archFEATURE_2D_SUPER_TILE_VERSION,
    archFEATURE_HALTI0,
    archFEATURE_HALTI1,
    archFEATURE_HALTI2,
    archFEATURE_SUPPORT_GCREGTX,
    archFEATURE_2D_MIRROR_EXTENSION,
    archFEATURE_TEXTURE_ASTC,
    archFEATURE_TEXTURE_ASTC_DECODE_FIX,
    archFEATURE_TEXTURE_ASTC_BASE_LOD_FIX,
    archFEATURE_2D_SUPER_TILE_V1,
    archFEATURE_2D_SUPER_TILE_V2,
    archFEATURE_2D_SUPER_TILE_V3,
    archFEATURE_2D_MULTI_SOURCE_BLT_EX2,
    archFEATURE_NEW_RA,
    archFEATURE_BUG_FIXED_IMPLICIT_PRIMITIVE_RESTART,
    archFEATURE_PE_MULTI_RT_BLEND_ENABLE_CONTROL,
    archFEATURE_SMALL_MSAA, /* An upgraded version of Fast MSAA */
    archFEATURE_VERTEX_INST_ID_AS_ATTRIBUTE,
    archFEATURE_DUAL_16,
    archFEATURE_BRANCH_ON_IMMEDIATE_REG,
    archFEATURE_2D_COMPRESSION,
    archFEATURE_TPC_COMPRESSION,
    archFEATURE_TPCV11_COMPRESSION,
    archFEATURE_DEC_COMPRESSION,
    archFEATURE_DEC300_COMPRESSION,
    archFEATURE_DEC400_COMPRESSION,
    archFEATURE_DEC_TPC_COMPRESSION,
    archFEATURE_DEC_COMPRESSION_TILE_NV12_8BIT,
    archFEATURE_DEC_COMPRESSION_TILE_NV12_10BIT,
    archFEATURE_2D_OPF_YUV_OUTPUT,
    archFEATURE_2D_FILTERBLIT_A8_ALPHA,
    archFEATURE_2D_MULTI_SRC_BLT_TO_UNIFIED_DST_RECT,
    archFEATURE_2D_MULTI_SRC_BLT_BILINEAR_FILTER,
    archFEATURE_2D_MULTI_SRC_BLT_1_5_ENHANCEMENT,
    archFEATURE_V2_COMPRESSION_Z16_FIX,
    archFEATURE_VERTEX_INST_ID_AS_INTEGER,
    archFEATURE_2D_YUV_MODE,
    archFEATURE_2D_CACHE_128B256BPERLINE,
    archFEATURE_2D_SEPARATE_CACHE,
    archFEATURE_2D_MAJOR_SUPER_TILE,
    archFEATURE_2D_V4COMPRESSION,
    archFEATURE_2D_VMSAA,
    archFEATURE_2D_10BIT_OUTPUT_LINEAR,
    archFEATURE_2D_YUV420_OUTPUT_LINEAR,
    archFEATURE_ACE,
    archFEATURE_COLOR_COMPRESSION,
    archFEATURE_32BPP_COMPONENT_TEXTURE_CHANNEL_SWIZZLE,
    archFEATURE_64BPP_HW_CLEAR_SUPPORT,
    archFEATURE_TX_LERP_PRECISION_FIX,
    archFEATURE_COMPRESSION_V2,
    archFEATURE_MMU,
    archFEATURE_COMPRESSION_V3,
    archFEATURE_TX_DECOMPRESSOR,
    archFEATURE_MRT_TILE_STATUS_BUFFER,
    archFEATURE_COMPRESSION_V1,
    archFEATURE_V1_COMPRESSION_Z16_DECOMPRESS_FIX,
    archFEATURE_RTT,
    archFEATURE_GENERIC_ATTRIB,
    archFEATURE_2D_ONE_PASS_FILTER,
    archFEATURE_2D_ONE_PASS_FILTER_TAP,
    archFEATURE_2D_POST_FLIP,
    archFEATURE_2D_PIXEL_ALIGNMENT,
    archFEATURE_CORRECT_AUTO_DISABLE_COUNT,
    archFEATURE_CORRECT_AUTO_DISABLE_COUNT_WIDTH,
    archFEATURE_8K_RT,
    archFEATURE_HALTI3,
    archFEATURE_EEZ,
    archFEATURE_INTEGER_SIGNEXT_FIX,
    archFEATURE_PSOUTPUT_MAPPING,
    archFEATURE_8K_RT_FIX,
    archFEATURE_TX_TILE_STATUS_MAPPING,
    archFEATURE_SRGB_RT_SUPPORT,
    archFEATURE_TEXTURE_16K,
    archFEATURE_PA_FARZCLIPPING_FIX,
    archFEATURE_PE_DITHER_COLORMASK_FIX,
    archFEATURE_ZSCALE_FIX,
    archFEATURE_MULTI_PIXELPIPES,
    archFEATURE_PIPE_CL,
    archFEATURE_BUG_FIXES18,
    archFEATURE_UNIFIED_SAMPLERS,
    archFEATURE_CL_PS_WALKER,
    archFEATURE_NEW_HZ,
    archFEATURE_TX_FRAC_PRECISION_6BIT,
    archFEATURE_SH_INSTRUCTION_PREFETCH,
    archFEATURE_PROBE,
    archFEATURE_SINGLE_PIPE_HALTI1,
    archFEATURE_BUG_FIXES8, /* This HW feature is wrong, we can't use this to check integer branch!!!*/
    archFEATURE_2D_ALL_QUAD,
    archFEATURE_SEPARATE_SRC_DST,
    archFEATURE_TX_HOR_ALIGN_SEL,
    archFEATURE_HALTI4,
    archFEATURE_MRT_FC_FIX,
    archFEATURE_TESSELLATION,
    archFEATURE_DRAW_INDIRECT,
    archFEATURE_COMPUTE_INDIRECT,
    archFEATURE_MSAA_TEXTURE,
    archFEATURE_STENCIL_TEXTURE,
    archFEATURE_S8_ONLY_RENDERING,
    archFEATURE_D24S8_SAMPLE_STENCIL,
    archFEATURE_ADVANCED_BLEND_MODE_PART0,
    archFEATURE_RA_DEPTH_WRITE,
    archFEATURE_RS_DS_DOWNSAMPLE_NATIVE_SUPPORT,
    archFEATURE_S8_MSAA_COMPRESSION,
    archFEATURE_MSAA_FRAGMENT_OPERATION,
    archFEATURE_FE_START_VERTEX_SUPPORT,
    archFEATURE_DIVISOR_STREAM_ADDR_FIX,
    archFEATURE_ZERO_ATTRIB_SUPPORT,
    archFEATURE_DANGLING_VERTEX_FIX,
    archFEATURE_PE_DISABLE_COLOR_PIPE,
    archFEATURE_FE_12bit_stride,
    archFEATURE_TX_LOD_GUARDBAND,
    archFEATURE_HAS_PRODUCTID,
    archFEATURE_INTEGER32_FIX,
    archFEATURE_TEXTURE_GATHER,
    archFEATURE_IMG_INSTRUCTION,
    archFEATURE_HELPER_INVOCATION,
    archFEATURE_NO_USER_CSC,
    archFEATURE_ANDROID_ONLY,
    archFEATURE_V2_MSAA_COHERENCY_FIX,
    archFEATURE_BLOCK_SIZE_16x16,
    archFEATURE_TX_SUPPORT_DEC,
    archFEATURE_RSBLT_MSAA_DECOMPRESSION,
    archFEATURE_TILEFILLER_32TILE_ALIGNED,
    archFEATURE_GEOMETRY_SHADER,
    archFEATURE_HALTI5,
    archFEATURE_PIPELINE_32_ATTRIBUTES,
    archFEATURE_USC,
    archFEATURE_CUBEMAP_ARRAY,
    archFEATURE_TX_DESCRIPTOR,
    archFEATURE_SEPARATE_RT_CTRL,
    archFEATURE_RENDER_ARRAY,
    archFEATURE_BLT_ENGINE,
    archFEATURE_TEXTURE_BUFFER,
    archFEATURE_GS_SUPPORT_EMIT,
    archFEATURE_SAMPLER_BASE_OFFSET,
    archFEATURE_IMAGE_OUT_BOUNDARY_FIX,
    archFEATURE_TX_BORDER_CLAMP,
    archFEATURE_MSAA_SHADING,
    archFEATURE_ADVANCED_SH_INST,
    archFEATURE_LOD_FIX_FOR_BASELEVEL,
    archFEATURE_MULTIDRAW_INDIRECT,
    archFEATURE_DRAW_ELEMENTS_BASE_VERTEX,
    archFEATURE_NEW_STEERING_AND_ICACHE_FLUSH, /* Steering base on register base. Trigger-style Icache flush state. */
    archFEATURE_PE_DITHER_FIX2,
    archFEATURE_INDEX_FETCH_FIX,
    archFEATURE_TEX_BASELOD,
    archFEATURE_TEX_SEAMLESS_CUBE,
    archFEATURE_TEX_ETC2,
    archFEATURE_TEX_CUBE_BORDER_LOD,
    archFEATURE_FE_ALLOW_STALL_PREFETCH_ENG,
    archFEATURE_TX_8BPP_TS_FIX,
    archFEATURE_HW_TFB,
    archFEATURE_COMPRESSION_V4,
    archFEATURE_FENCE_32BIT,
    archFEATURE_FENCE_64BIT,
    archFEATURE_R8_UNORM,
    archFEATURE_TX_DEFAULT_VALUE_FIX,
    archFEATURE_TX_8bit_UVFrac,
    archFEATURE_TX_MIPFILTER_NONE_FIX,
    archFEATURE_MC_STENCIL_CTRL,
    archFEATURE_DEPTH_MATH_FIX,
    archFEATURE_PE_B2B_PIXEL_FIX,
    archFEATURE_TEXTURE_GATHER_OFFSETS,
    archFEATURE_TEX_CACHE_FLUSH_FIX,
    archFEATURE_WIDELINE_HELPER_FIX,
    archFEATURE_LINE_DIAMOND_RULE_FIX,
    archFEATURE_MULTIGPU_SYNC_V2,
    archFEATURE_DRAW_ID,
    archFEATURE_SNAPPAGE_CMD,
    archFEATURE_COMMAND_PREFETCH,
    archFEATURE_SAMPLEPOS_SWIZZLE_FIX,
    archFEATURE_SELECTMAP_SRC0_SWIZZLE_FIX,
    archFEATURE_LOADATTR_OOB_FIX,
    archFEATURE_RA_DEPTH_WRITE_MSAA1X_FIX,
    archFEATURE_MRT_8BIT_DUAL_PIPE_FIX,
    archFEATURE_BUG_FIXES1,
    archFEATURE_MULTI_SOURCE_BLT,
    archFEATURE_ZCOMPRESSION,
    archFEATURE_DITHER_AND_FILTER_PLUS_ALPHA_2D,
    archFEATURE_ONE_PASS_2D_FILTER,
    archFEATURE_TX_FILTER,
    archFEATURE_CHIPENABLE_LINK,
    archFEATURE_TEXTURE_BIAS_LOD_FIX,
    archFEATURE_USE_GL_Z,
    archFEATURE_SUPPORT_INTEGER,
    /* PARTLY_SUPPORT_INTEGER_BRANCH:
    **      chips can support all integer types for compare instructions, e.g, CMP, SELECT.
    ** FULLLY_SUPPORT_INTEGER_BRANCH:
    **      chips can support all integer types for JMP instruction.
    ** If PARTLY_SUPPORT_INTEGER_BRANCH is TRUE but FULLLY_SUPPORT_INTEGER_BRANCH is FALSE,
    ** then this chip can only support INT32/UINT32 JMP instruction.
    */
    archFEATURE_PARTLY_SUPPORT_INTEGER_BRANCH,
    archFEATURE_FULLLY_SUPPORT_INTEGER_BRANCH,
    archFEATURE_SUPPORT_INTEGER_ATTRIBUTE,
    archFEATURE_SUPPORT_MOVAI,
    archFEATURE_NEED_FIX_FOR_CL_X,
    archFEATURE_NEED_FIX_FOR_CL_XE,
    archFEATURE_HAS_OUTPUT_COUNT_FIX,
    archFEATURE_VARYING_PACKING_LIMITATION,
    archFEATURE_HIGHP_VARYING_SHIFT,
    archFEATURE_BUG_FIXES2,
    archFEATURE_64K_L2_CACHE,
    archFEATURE_128BTILE,
    archFEATURE_ADVANCED_BLEND_OPT,
    archFEATURE_SNAPPAGE_CMD_FIX,
    archFEATURE_L2_CACHE_FOR_2D_420,
    archFEATURE_TILE_STATUS_2BITS,
    archFEATURE_EXTRA_SHADER_INSTRUCTIONS0,
    archFEATURE_EXTRA_SHADER_INSTRUCTIONS1,
    archFEATURE_EXTRA_SHADER_INSTRUCTIONS2,
    archFEATURE_MEDIUM_PRECISION,
    archFEATURE_FE20_BIT_INDEX,
    archFEATURE_BUG_FIXES4,
    archFEATURE_BUG_FIXES12,
    archFEATURE_VMSAA,
    archFEATURE_ROBUST_ATOMIC,
    archFEATURE_32F_COLORMASK_FIX,
    archFEATURE_NEW_GPIPE,
    archFEATURE_RS_NEW_BASEADDR,
    archFEATURE_TX_DXT,
    archFEATURE_SH_FLAT_INTERPOLATION_DUAL16_FIX,
    archFEATURE_EVIS,
    archFEATURE_SH_SUPPORT_V4,
    archFEATURE_SH_SUPPORT_ALPHA_KILL,
    archFEATURE_PE_NO_ALPHA_TEST,
    archFEATURE_SH_SNAP2PAGE_MAXPAGES_FIX,
    archFEATURE_USC_FULLCACHE_FIX,
    archFEATURE_PE_64bit_FENCE_FIX,
    archFEATURE_BLT_8bit_256TILE_FC_FIX,
    archFEATURE_PE_RGBA16I_FIX,
    archFEATURE_BLT_64bpp_MASKED_CLEAR_FIX,
    archFEATURE_SH_PSO_MSAA1x_FIX,
    archFEATURE_USC_ATOMIC_FIX,
    archFEATURE_INDEX_CONST_ON_B0,
    archFEATURE_SH_NO_ONECONST_LIMIT,
    archFEATURE_EVIS_NO_ABSDIFF,
    archFEATURE_EVIS_NO_BITREPLACE,
    archFEATURE_EVIS_NO_BOXFILTER,
    archFEATURE_EVIS_NO_CORDIAC,
    archFEATURE_EVIS_NO_DP32,
    archFEATURE_EVIS_NO_FILTER,
    archFEATURE_EVIS_NO_IADD,
    archFEATURE_EVIS_NO_SELECTADD,
    archFEATURE_EVIS_LERP_7OUTPUT,
    archFEATURE_EVIS_ACCSQ_8OUTPUT,
    archFEATURE_ROBUSTNESS,
    archFEATURE_SECURITY,
    archFEATURE_TX_YUV_ASSEMBLER_10BIT,
    archFEATURE_USC_GOS_ADDR_FIX,
    archFEATURE_SUPPORT_MSAA2X,
    archFEATURE_TX_DESC_CACHE_CLOCKGATE_FIX,
    archFEATURE_TX_INTEGER_COORDINATE,
    archFEATURE_PSIO_SAMPLEMASK_IN_R0ZW_FIX,
    archFEATURE_MULTI_CORE_BLOCK_SET_CONFIG,
    archFEATURE_SH_IMG_LDST_ON_TEMP,
    archFEATURE_TX_INTEGER_COORDINATE_V2,
    archFEATURE_COMPUTE_ONLY,
    archFEATURE_SH_IMG_LDST_CLAMP,
    archFEATURE_SH_ICACHE_ALLOC_COUNT_FIX,
    archFEATURE_MSAA_OQ_FIX,
    archFEATURE_PE_ENHANCEMENTS2,
    archFEATURE_PSIO_MSAA_CL_FIX,
    archFEATURE_FE_NEED_DUMMYDRAW,
    archFEATURE_MULTI_CLUSTER,
    archFEATURE_PSIO_INTERLOCK,
    archFEATURE_BLIT_COMPRESS_DEST,
    archFEATURE_SH_MULTI_WG_PACK,
    archFEATURE_FE_ROBUST_FIX,
    archFEATURE_TX_ASTC_MULTISLICE_FIX,
    archFEATURE_PSIO_DUAL16_32bpc_FIX,
    archFEATURE_LS_SUPPORT_PER_COMP_DEPENDENCY,
    archFEATURE_COMPRESSION_DEC400,
    archFEATURE_SH_TEXLD_U_FIX,
    archFEATURE_TX_FLUSH_L1CACHE,
    archFEATURE_USC_DEFER_FILL_FIX,
    archFEATURE_MC_FCCACHE_BYTEMASK,
    archFEATURE_SH_MULTI_WG_PACK_FIX,
    archFEATURE_FE_PATCHLIST_FETCH_FIX,
    archFEATURE_RA_CG_FIX,
    archFEATURE_EVIS_VX2,
    archFEATURE_SH_HALF_DEPENDENCY_FIX,
    archFEATURE_FE_BASEINSTANCE,
    archFEATURE_FE_COMPUREINDIRECT_SKIP_UNIFORM,
    archFEATURE_SH_CLOCK_GATE_FIX,
    archFEATURE_GPIPE_CLOCK_GATE_FIX,
    archFEATURE_TP_ENGINE,
    archFEATURE_TX_BORDER_CLAMP_FIX,
    archFEATURE_SH_IMAGE_LD_LAST_PIXEL_FIX,
    archFEATURE_MULTI_CORE_BLOCK_SET_CONFIG2,
    archFEATURE_MULTIGPU_SYNC_V3,
    archFEATURE_PE_VMSAA_COVERAGE_CACHE_FIX,
    archFEATURE_SECURITY_AHB,
    archFEATURE_TX_LERP_LESS_BIT,
    archFEATURE_SMALL_BATCH,
    archFEATURE_SH_IDIV0_SWZL_EHS,
    archFEATURE_SH_CMPLX,
    archFEATURE_VIP_V7,
    archFEATURE_SH_GM_ENDIAN,
    archFEATURE_SH_GM_USC_UNALLOC,
    archFEATURE_SH_END_OF_BB,
    archFEATURE_ASYNC_BLIT,
    archFEATURE_ASYNC_FE_FENCE_FIX,
    archFEATURE_PSCS_THROTTLE,
    archFEATURE_SEPARATE_LS,
    archFEATURE_PA_VARYING_COMPONENT_TOGGLE_FIX,
    archFEATURE_TX_MULTISAMPLER_FC_FIX,
    archFEATURE_WIDELINE_TRIANGLE_EMU,
    archFEATURE_FENCE,
    archFEATURE_MCFE,
    archFEATURE_NN_INTERLEAVE8,
    archFEATURE_TP_REORDER,
    archFEATURE_TP_RTNE,
    archFEATURE_TP_LRN,
    archFEATURE_TP_ROI_POOLING,
    archFEATURE_TP_MAX_POOLING_STRIDE1,
    archFEATURE_NN_BRICK_MODE,
    archFEATURE_NN_BORDER_MODE,
    archFEATURE_NN_FP16_ALU,
    archFEATURE_NN_INT16_ALU,
    archFEATURE_NN_ZDP3,
    archFEATURE_NN_ZDP6,
    archFEATURE_PE_DEPTH_ONLY_OQFIX,
    archFEATURE_TX_SNORM_SUPPORT,
    archFEATURE_HWMANAGED_LS,
    archFEATURE_SH_SCATTER_GATHER,
    archFEATURE_NN_POWER_ISOLATION,
    archFEATURE_SWTILING_PHASE1,
    archFEATURE_SWTILING_PHASE2,
    archFEATURE_SWTILING_PHASE3,
    archFEATURE_TF_QUANTIZATION,
    archFEATURE_NN_XYDP9,
    archFEATURE_TP_SIMPLE_INT16,
    archFEATURE_TP_REAL_INT16,
    archFEATURE_NN_FIRST_PIXEL_POOLING,
    archFEATURE_NN_STRIDE_SUPPORT,
    archFEATURE_NN_XYDP6,
    archFEATURE_NN_XYDP0,
    archFEATURE_TP_REORDER_FIX,
    archFEATURE_NN_CONV1x1_PERF_FIX,
    archFEATURE_NN_CACHELINE_MODE_PERF_FIX,
    archFEATURE_NN_PER3DTILE_BUBBLE_FIX,
    archFEATURE_SH_IO_CG_FIX,
    archFEATURE_USC_STAY_LRU,
    archFEATURE_NN_NONZERO_MIRROR_BORDER,
    archFEATURE_NN_COEF_DECOMPRESS_PERF2X,
    archFEATURE_4BIT_INPUT,
    archFEATURE_COEF_COMPRESSION_ENHANCEMENT,
    archFEATURE_NN_ZDP3_NO_COMPRESS_FIX,
    archFEATURE_NN_ASYNC_COPY_PERF_FIX,
    archFEATURE_OCB_COUNTER,
    archFEATURE_NN_ZXDP3_KERNEL_READ_CONFLICT_FIX,
    archFEATURE_NN_FULLCACHE_KERNEL_INTERLEAVE_FIX,
    archFEATURE_OCB_REMAP_PHYSICAL_ADDRESS,

    archFEATURE_IMAGE_LS_NO_FULLMASK_FIX,
    archFEATURE_BLT_YUV_OUTPUT,
    archFEATURE_PE_TILE_CACHE_FLUSH_FIX,
    archFEATURE_SH_ROBUSTNESS_FIX,
    archFEATURE_USC_ATOMIC_FIX2,
    archFEATURE_MULTIVIEW_RENDER,
    archFEATURE_FE_DRAW_DIRECT,
    archFEATURE_TX_VKBORDER_MODE,
    archFEATURE_TX_UNNORMALIZED_COORD,
    archFEATURE_VG_IMAGE_16K,
    archFEATURE_MULTICORE_CONFIG,
    archFEATURE_PA_LINECLIP_FIX,
    archFEATURE_NN_ENGINE,
    archFEATURE_NN_ASYNC_COPY_MERGE_FIX,
    archFEATURE_NN_CONVOUT_FIFO_DEPTH_FIX,
    archFEATURE_NN_SMALLBATCH_PHASE1,
    archFEATURE_TP_SMALLBATCH_PHASE1,
    archFEATURE_VIP_SCALER,
    archFEATURE_TX_8bit_UVFrac_ROUNDING_FIX,
    archFEATURE_NN_REQ_SLOWARBITRATION_FIX,
    archFEATUER_IMAGE_PARTIAL_CACHE,
    archFEATURE_FULLCACHE_KERNELHEAD_FIX,
    archFEATURE_NN_SINGLEPORT_ACCUMBUFFER,
    archFEATURE_NN_SMALLBATCH,
    archFEATURE_TP_SMALLBATCH,
    archFEATURE_NN_ZDP_INIMAGE_SIZE_FIX,
    archFEATURE_HI_REORDER_FIX,
    archFEATURE_TP_COEF_COMPRESSION_ENHANCEMENT,
    archFEATURE_NN_DEPTHWISE_SUPPORT,
    archFEATURE_IMAGE_NOT_PACKED_IN_SRAM_FIX,
    archFEATURE_IDLE_BEFORE_FLUSH_COMPLETE_FIX,
    archFEATURE_NO_FLUSH_USC_FIX,
    archFEATURE_COEF_DELTA_CORD_OVERFLOW_ZRL_8BIT_FIX,
    archFEATURE_XY_OFFSET_LIMITATION_FIX,
    archFEATURE_USC_INVALIDATE_CACHE_LINE_FIX,
    archFEATURE_LOW_EFFICIENCY_OF_ID_WRITE_IMGBUF_FIX,
    archFEATURE_KERNEL_PER_CORE_LESS_THAN_THIRD_COEF_BUFF_DEPTH_FIX,
    archFEATURE_NN_PER_CHANNEL_POST_MULTIPLY,
    archFEATURE_NN_NO_Z_LOCATION_OFFSET,
    archFEATURE_NN_PRELU,
    archFEATURE_NN_KERNEL_SIZE_WASTE_IN_PARTIAL_MODE_FIX,
    archFEATURE_VIP_DEC400,
    archFEATURE_MAX_POINTSIZE_CLAMP,
    archFEATURE_2D_FAST_CLEAR, /* For tilestatus Fast Clear feature*/

    /* Insert features above this comment only. */
    archFEATURE_COUNT                /* Not a feature. */
}
archFEATURE;

/* Option Set*/
typedef enum _archOPTION
{
    /* HW setting. */
    archOPTION_PREFER_ZCONVERT_BYPASS = 0,
    archOPTION_PREFER_TILED_DISPLAY_BUFFER = 1,
    archOPTION_PREFER_GUARDBAND = 2,
    archOPTION_PREFER_TPG_TRIVIALMODEL = 3,
    archOPTION_PREFER_RA_DEPTH_WRITE = 4,
    archOPTION_PREFER_USC_RECONFIG = 5,
    archOPTION_PREFER_DISALBE_HZ = 6,

    /* SW options */
    archOPTION_HW_NULL = 50,
    archOPTION_PRINT_OPTION = 51,
    archOPTION_KERNEL_FENCE = 52,
    archOPTION_ASYNC_PIPE = 53,
    archOPTION_FBO_PREFER_MEM = 54,
    archOPTION_GPU_TEX_UPLOAD = 55,
    archOPTION_GPU_BUFOBJ_UPLOAD = 56,

    /* OCL option */
    archOPTION_OCL_ASYNC_BLT = 200,
    archOPTION_OCL_IN_THREAD,
    archOPTION_COMPRESSION_DEC400,
    archOPTION_OCL_VIR_SHADER,
    archOPTION_OCL_USE_MULTI_DEVICES,

    /* OVX options that HAL could access */
    archOPTION_OVX_ENABLE_NN_ZDP3 = 500,
    archOPTION_OVX_ENABLE_NN_ZDP6,
    archOPTION_OVX_ENABLE_NN_STRIDE,
    archOPTION_OVX_USE_MULTI_DEVICES,

    /* Insert option above this comment only */
    archOPTION_COUNT                     /* Not a OPTION*/
}
archOPTION;


/******************************************************************************\
********************************* Status Codes *********************************
\******************************************************************************/

// refine me
typedef enum _archSTATUS
{
    archSTATUS_OK                    =   0,
    archSTATUS_FALSE                 =   0,
    archSTATUS_TRUE                  =   1,
    archSTATUS_NO_MORE_DATA          =   2,
    archSTATUS_CACHED                =   3,
    archSTATUS_MIPMAP_TOO_LARGE      =   4,
    archSTATUS_NAME_NOT_FOUND        =   5,
    archSTATUS_NOT_OUR_INTERRUPT     =   6,
    archSTATUS_MISMATCH              =   7,
    archSTATUS_MIPMAP_TOO_SMALL      =   8,
    archSTATUS_LARGER                =   9,
    archSTATUS_SMALLER               =   10,
    archSTATUS_CHIP_NOT_READY        =   11,
    archSTATUS_NEED_CONVERSION       =   12,
    archSTATUS_SKIP                  =   13,
    archSTATUS_DATA_TOO_LARGE        =   14,
    archSTATUS_INVALID_CONFIG        =   15,
    archSTATUS_CHANGED               =   16,
    archSTATUS_NOT_SUPPORT_DITHER    =   17,
    archSTATUS_EXECUTED              =   18,
    archSTATUS_TERMINATE             =   19,

    archSTATUS_INVALID_ARGUMENT      =   -1,
    archSTATUS_INVALID_OBJECT        =   -2,
    archSTATUS_OUT_OF_MEMORY         =   -3,
    archSTATUS_MEMORY_LOCKED         =   -4,
    archSTATUS_MEMORY_UNLOCKED       =   -5,
    archSTATUS_HEAP_CORRUPTED        =   -6,
    archSTATUS_GENERIC_IO            =   -7,
    archSTATUS_INVALID_ADDRESS       =   -8,
    archSTATUS_CONTEXT_LOSSED        =   -9,
    archSTATUS_TOO_COMPLEX           =   -10,
    archSTATUS_BUFFER_TOO_SMALL      =   -11,
    archSTATUS_INTERFACE_ERROR       =   -12,
    archSTATUS_NOT_SUPPORTED         =   -13,
    archSTATUS_MORE_DATA             =   -14,
    archSTATUS_TIMEOUT               =   -15,
    archSTATUS_OUT_OF_RESOURCES      =   -16,
    archSTATUS_INVALID_DATA          =   -17,
    archSTATUS_INVALID_MIPMAP        =   -18,
    archSTATUS_NOT_FOUND             =   -19,
    archSTATUS_NOT_ALIGNED           =   -20,
    archSTATUS_INVALID_REQUEST       =   -21,
    archSTATUS_GPU_NOT_RESPONDING    =   -22,
    archSTATUS_TIMER_OVERFLOW        =   -23,
    archSTATUS_VERSION_MISMATCH      =   -24,
    archSTATUS_LOCKED                =   -25,
    archSTATUS_INTERRUPTED           =   -26,
    archSTATUS_DEVICE                =   -27,
    archSTATUS_NOT_MULTI_PIPE_ALIGNED =   -28,
    archSTATUS_OUT_OF_SAMPLER         =   -29,

    /* Linker errors. */
    archSTATUS_GLOBAL_TYPE_MISMATCH              =   -1000,
    archSTATUS_TOO_MANY_ATTRIBUTES               =   -1001,
    archSTATUS_TOO_MANY_UNIFORMS                 =   -1002,
    archSTATUS_TOO_MANY_VARYINGS                 =   -1003,
    archSTATUS_UNDECLARED_VARYING                =   -1004,
    archSTATUS_VARYING_TYPE_MISMATCH             =   -1005,
    archSTATUS_MISSING_MAIN                      =   -1006,
    archSTATUS_NAME_MISMATCH                     =   -1007,
    archSTATUS_INVALID_INDEX                     =   -1008,
    archSTATUS_UNIFORM_MISMATCH                  =   -1009,
    archSTATUS_UNSAT_LIB_SYMBOL                  =   -1010,
    archSTATUS_TOO_MANY_SHADERS                  =   -1011,
    archSTATUS_LINK_INVALID_SHADERS              =   -1012,
    archSTATUS_CS_NO_WORKGROUP_SIZE              =   -1013,
    archSTATUS_LINK_LIB_ERROR                    =   -1014,

    archSTATUS_SHADER_VERSION_MISMATCH           =   -1015,
    archSTATUS_TOO_MANY_INSTRUCTION              =   -1016,
    archSTATUS_SSBO_MISMATCH                     =   -1017,
    archSTATUS_TOO_MANY_OUTPUT                   =   -1018,
    archSTATUS_TOO_MANY_INPUT                    =   -1019,
    archSTATUS_NOT_SUPPORT_CL                    =   -1020,
    archSTATUS_NOT_SUPPORT_INTEGER               =   -1021,
    archSTATUS_UNIFORM_TYPE_MISMATCH             =   -1022,

    archSTATUS_MISSING_PRIMITIVE_TYPE            =   -1023,
    archSTATUS_MISSING_OUTPUT_VERTEX_COUNT       =   -1024,
    archSTATUS_NON_INVOCATION_ID_AS_INDEX        =   -1025,
    archSTATUS_INPUT_ARRAY_SIZE_MISMATCH         =   -1026,
    archSTATUS_OUTPUT_ARRAY_SIZE_MISMATCH        =   -1027,
    archSTATUS_LOCATION_ALIASED                  =   -1028,

    /* Compiler errors. */
    archSTATUS_COMPILER_FE_PREPROCESSOR_ERROR    =   -2000,
    archSTATUS_COMPILER_FE_PARSER_ERROR          =   -2001,

    /* Recompilation Errors */
    archSTATUS_RECOMPILER_CONVERT_UNIMPLEMENTED  =   -3000,
}
archSTATUS;


/*************************************** Return Status definition ************************************/
enum arch_status_e {
    ARCH_STATUS_MIN                       = -25,/*!< \brief Indicates the lower bound of status codes in VX. Used for bounds checks only. */
    /* add new codes here */
    ARCH_ERROR_REFERENCE_NONZERO          = -24,/*!< \brief Indicates that an operation did not complete due to a reference count being non-zero. */
    ARCH_ERROR_MULTIPLE_WRITERS           = -23,/*!< \brief Indicates that the graph has more than one node outputting to the same data object. This is an invalid graph structure. */
    ARCH_ERROR_GRAPH_ABANDONED            = -22,/*!< \brief Indicates that the graph is stopped due to an error or a callback that abandoned execution. */
    ARCH_ERROR_GRAPH_SCHEDULED            = -21,/*!< \brief Indicates that the supplied graph already has been scheduled and may be currently executing. */
    ARCH_ERROR_INVALID_SCOPE              = -20,/*!< \brief Indicates that the supplied parameter is from another scope and cannot be used in the current scope. */
    ARCH_ERROR_INVALID_NODE               = -19,/*!< \brief Indicates that the supplied node could not be created.*/
    ARCH_ERROR_INVALID_GRAPH              = -18,/*!< \brief Indicates that the supplied graph has invalid connections (cycles). */
    ARCH_ERROR_INVALID_TYPE               = -17,/*!< \brief Indicates that the supplied type parameter is incorrect. */
    ARCH_ERROR_INVALID_VALUE              = -16,/*!< \brief Indicates that the supplied parameter has an incorrect value. */
    ARCH_ERROR_INVALID_DIMENSION          = -15,/*!< \brief Indicates that the supplied parameter is too big or too small in dimension. */
    ARCH_ERROR_INVALID_FORMAT             = -14,/*!< \brief Indicates that the supplied parameter is in an invalid format. */
    ARCH_ERROR_INVALID_LINK               = -13,/*!< \brief Indicates that the link is not possible as specified. The parameters are incompatible. */
    ARCH_ERROR_INVALID_REFERENCE          = -12,/*!< \brief Indicates that the reference provided is not valid. */
    ARCH_ERROR_INVALID_MODULE             = -11,/*!< \brief This is returned from <tt>\ref vxLoadKernels</tt> when the module does not contain the entry point. */
    ARCH_ERROR_INVALID_PARAMETERS         = -10,/*!< \brief Indicates that the supplied parameter information does not match the kernel contract. */
    ARCH_ERROR_OPTIMIZED_AWAY             = -9,/*!< \brief Indicates that the object refered to has been optimized out of existence. */
    ARCH_ERROR_NO_MEMORY                  = -8,/*!< \brief Indicates that an internal or implicit allocation failed. Typically catastrophic. After detection, deconstruct the context. \see vxVerifyGraph. */
    ARCH_ERROR_NO_RESOURCES               = -7,/*!< \brief Indicates that an internal or implicit resource can not be acquired (not memory). This is typically catastrophic. After detection, deconstruct the context. \see vxVerifyGraph. */
    ARCH_ERROR_NOT_COMPATIBLE             = -6,/*!< \brief Indicates that the attempt to link two parameters together failed due to type incompatibilty. */
    ARCH_ERROR_NOT_ALLOCATED              = -5,/*!< \brief Indicates to the system that the parameter must be allocated by the system.  */
    ARCH_ERROR_NOT_SUFFICIENT             = -4,/*!< \brief Indicates that the given graph has failed verification due to an insufficient number of required parameters, which cannot be automatically created. Typically this indicates required atomic parameters. \see vxVerifyGraph. */
    ARCH_ERROR_NOT_SUPPORTED              = -3,/*!< \brief Indicates that the requested set of parameters produce a configuration that cannot be supported. Refer to the supplied documentation on the configured kernels. \see ARCH_kernel_e. This is also returned if a function to set an attribute is called on a Read-only attribute.*/
    ARCH_ERROR_NOT_IMPLEMENTED            = -2,/*!< \brief Indicates that the requested kernel is missing. \see ARCH_kernel_e vxGetKernelByName. */
    ARCH_FAILURE                          = -1,/*!< \brief Indicates a generic error code, used when no other describes the error. */
    ARCH_SUCCESS                          =  0,/*!< \brief No error. */
};

/*************************************** Feature definition ************************************/
typedef enum _arch_nn_feature_e
{
    ARCH_NN_FEATURE_TP,
    ARCH_NN_FEATURE_MULTI_TP,
    ARCH_NN_FEATURE_TP_RESHUFFLE,
    ARCH_NN_FEATURE_TP_SINGLE_FC,
    ARCH_NN_FEATURE_TP_MAX_POOLING,
    ARCH_NN_FEATURE_TP_ACTIVATION,
    ARCH_NN_FEATURE_TP_LRN,
    ARCH_NN_FEATURE_TP_TRANSPOSE,
    ARCH_NN_FEATURE_TP_ROI_POOLING,
    ARCH_NN_FEATURE_TP_BRICK_MODE,
    ARCH_NN_FEATURE_TP_REORG,
    ARCH_NN_FEATURE_TP_ADD,
    ARCH_NN_FEATURE_TP_REVERSE,
    ARCH_NN_FEATURE_TP_UPSAMPLE,
    ARCH_NN_FEATURE_TP_REORDER,
    ARCH_NN_FEATURE_TP_RTNE,
    ARCH_NN_FEATURE_BRICK_MODE,
    ARCH_NN_FEATURE_INTERLEVE8,
    ARCH_NN_FEATURE_BORDER_MODE,
    ARCH_NN_FEATURE_SRAM,
    ARCH_NN_FEATURE_ZDP3,
    ARCH_NN_FEATURE_ZDP6,
    ARCH_NN_FEATURE_XYDP9,
    ARCH_NN_FEATURE_XYDP6,
    ARCH_NN_FEATURE_XYDP0,
    ARCH_NN_FEATURE_SWTILING_PHASE1,
    ARCH_NN_FEATURE_SWTILING_PHASE2,
    ARCH_NN_FEATURE_SWTILING_PHASE3,
    ARCH_NN_FEATURE_TF_QUANT,
    ARCH_NN_FEATURE_FIRST_PIXEL_POOLING,
    ARCH_NN_FEATURE_NN_STRIDE_SUPPORT,
    ARCH_NN_FEATURE_COEF_COMPRESSION_ENHANCEMENT,
    ARCH_NN_FEATURE_TP_COMPRESSION_ENHANCEMENT,
    ARCH_NN_FEATURE_SCALER,
    ARCH_NN_FEATURE_NN_DEPTHWISE_SUPPORT,
    ARCH_NN_FEATURE_VIP_DEC400,
    ARCH_NN_FEATURE_SHADER,

    ARCH_NN_FEATURE_COUNT,
}
arch_nn_feature_e;

/*************************************** Operation Target definition ************************************/
typedef enum archnne_operation_target_e
{
    ARCHNNE_OPERATION_TARGET_NONE = 0,
    ARCHNNE_OPERATION_TARGET_SH,
    ARCHNNE_OPERATION_TARGET_NN,
    ARCHNNE_OPERATION_TARGET_TP,
    ARCHNNE_OPERATION_TARGET_SW,
    ARCHNNE_OPERATION_TARGET_SC, /* SCALER */
    ARCHNNE_OPERATION_TARGET_NBG,
}
archnne_operation_target_e;

/*************************************** Operation Type definition ************************************/
typedef enum archnne_operator_e
{
    ARCHNNE_OPERATOR_NONE = 0,
    ARCHNNE_OPERATOR_CONVOLUTION,
    ARCHNNE_OPERATOR_RESHUFFLE,
    ARCHNNE_OPERATOR_FULLYCONNECTED,
    ARCHNNE_OPERATOR_ACTIVATION,
    ARCHNNE_OPERATOR_POOLING,
    ARCHNNE_OPERATOR_RESIZE,
    ARCHNNE_OPERATOR_TENSOR_ADD,
    ARCHNNE_OPERATOR_TENSOR_SUB,
    ARCHNNE_OPERATOR_TENSOR_MUL,
    ARCHNNE_OPERATOR_TENSOR_DIV,
    ARCHNNE_OPERATOR_TENSOR_TRANS,
    ARCHNNE_OPERATOR_SOFTMAX,
    ARCHNNE_OPERATOR_NORMALIZATION,
    ARCHNNE_OPERATOR_BATCHNORM,
    ARCHNNE_OPERATOR_INPUT2WEIGHT,
    ARCHNNE_OPERATOR_RPN_SOFTMAX,
    ARCHNNE_OPERATOR_RPN_REGRESSION,
    ARCHNNE_OPERATOR_RPN_SORT,
    ARCHNNE_OPERATOR_RPN_NMS,
    ARCHNNE_OPERATOR_RPN_SORT_NMS,
    ARCHNNE_OPERATOR_RPN_RETRIEVE,
    ARCHNNE_OPERATOR_RPN,
    ARCHNNE_OPERATOR_ROIPOOL,
    ARCHNNE_OPERATOR_ROIPOOLRELU,
    ARCHNNE_OPERATOR_CONCAT2,
    ARCHNNE_OPERATOR_CONCATINDEFINITE,
    ARCHNNE_OPERATOR_REORG,
    ARCHNNE_OPERATOR_VERTMAXPOOL,
    ARCHNNE_OPERATOR_HORZMAXPOOL,
    ARCHNNE_OPERATOR_PRETREATEDRECT,
    ARCHNNE_OPERATOR_BRICK,
    ARCHNNE_OPERATOR_DECONVOLUTION,
    ARCHNNE_OPERATOR_L2NORMALIZE,
    ARCHNNE_OPERATOR_L2NORMALIZE_SUMSQRT,
    ARCHNNE_OPERATOR_L2NORMALIZE_SUMSCALE,
    ARCHNNE_OPERATOR_TENSOR_COPY,
    ARCHNNE_OPERATOR_CONVERT_FORMAT,
    ARCHNNE_OPERATOR_TENSOR_REDUCE_SUM,
    ARCHNNE_OPERATOR_TENSOR_PAD,
    ARCHNNE_OPERATOR_LSTM_UNIT,
    ARCHNNE_OPERATOR_LSTM_LAYER,
    ARCHNNE_OPERATOR_REORG2,
    ARCHNNE_OPERATOR_TENSOR_ROUNDING,
    ARCHNNE_OPERATOR_HASHLUT,
    ARCHNNE_OPERATOR_LSH_PROJECTION,
    ARCHNNE_OPERATOR_TENSOR_RESHAPE,
    ARCHNNE_OPERATOR_TENSOR_SCALE,
    ARCHNNE_OPERATOR_YUV2RGB_SCALE,
    ARCHNNE_OPERATOR_RNN,
    ARCHNNE_OPERATOR_SVDF,
    ARCHNNE_OPERATOR_LUT2,
    ARCHNNE_OPERATOR_UPSAMPLE,
    ARCHNNE_OPERATOR_DILATION_RESHUFFLE,
    ARCHNNE_OPERATOR_DILATION_UPSAMPLE,
    ARCHNNE_OPERATOR_DILATION_UPSAMPLE2,
    ARCHNNE_OPERATOR_ADAPTER,
    ARCHNNE_OPERATOR_INTERLEAVE,
    ARCHNNE_OPERATOR_DEPTHWISE_CONV,
    ARCHNNE_OPERATOR_LSTM_RESHUFFLE_INPUT,
    ARCHNNE_OPERATOR_LSTM_STATE_OUT,
    ARCHNNE_OPERATOR_TENSOR_REVERSE,
    ARCHNNE_OPERATOR_USER_VXC,
    ARCHNNE_OPERATOR_USER_CPU,
    ARCHNNE_OPERATOR_TENSOR_MEAN,
    ARCHNNE_OPERATOR_TENSOR_SQUEEZE,
    ARCHNNE_OPERATOR_TENSOR_STRIDE_SLICE,
    ARCHNNE_OPERATOR_PRELU,
    ARCHNNE_OPERATOR_GRU,
    ARCHNNE_OPERATOR_GRU_LAYER,
    ARCHNNE_OPERATOR_CONV_LSTM,
    ARCHNNE_OPERATOR_CONV_LSTM_LAYER,
    ARCHNNE_OPERATOR_DEPTH_WISE_CONV,
    ARCHNNE_OPERATOR_SVDF_MAP,
    ARCHNNE_OPERATOR_SVDF_ROTATION,
    ARCHNNE_OPERATOR_LAYERNORM,
    ARCHNNE_OPERATOR_NBG,
    ARCHNNE_OPERATOR_TENSOR_MAX,
    ARCHNNE_OPERATOR_TENSOR_ADD_MERGE,
    ARCHNNE_OPERATOR_FIRST_PIXEL_POOLING,
}
archnne_operator_e;


typedef enum archnne_operator_type
{
    ARCHNNE_LAYER_ZERO = 0,
    ARCHNNE_LAYER_CONV_RELU_POOLING,
    ARCHNNE_LAYER_CONV_RELU_POOLING2,
    ARCHNNE_LAYER_TENSOR_SCALE,
    ARCNNNE_LAYER_CONV_RELU,
    ARCNNNE_LAYER_SOFT_MAX,
    ARCHNNE_LAYER_SOFT_MAX2,
    ARCNNNE_LAYER_POOLING,
    ARCNNNE_LAYER_POOLING2,
    ARCNNNE_LYAER_DW_CONV,
    ARCNNNE_LYAER_CONV,
    ARCNNNE_LYAER_FC,
    ARCNNNE_LAYER_FC_RELU,
    ARCNNNE_LYAER_ACTIVATE,
    ARCNNNE_LYAER_LEAKY_RELU,
    ARCNNNE_LYAER_PRE_RELU,
    ARCHNNE_LAYER_RPN,
    ARCHNNE_LAYER_RIO_POOLING,
    ARCHNNE_LAYER_RIO_POOLING_RELU,
    ARCHNNE_LAYER_CONCAT,
    ARCHNNE_LAYER_REORG,
    ARCHNNE_LAYER_REORG2,
    ARCHNNE_LAYER_DE_CONV,
    ARCNNNE_LAYER_NORMALIZE,
    ARCNNNE_LYAER_L2_NORMALIZE,
    ARCNNNE_LYAER_BATCH_NORMALIZE,
    ARCHNNE_LAYER_TENSOR_ADD,
    ARCHNNE_LAYER_TENSOR_SUB,
    ARCHNNE_LAYER_TENSOR_MUL,
    ARCHNNE_LAYER_TENSOR_DIV,
    ARCHNNE_LAYER_TENSOR_TRANSPOSE,
    ARCHNNE_LAYER_TENSOR_REDUCE_SUM,
    ARCHNNE_LAYER_TENSOR_PAD,
    ARCHNNE_LAYER_TENSOR_PAD2,
    ARCHNNE_LAYER_TENSOR_COPY,
    ARCHNNE_LAYER_TENSOR_REVERSE,
    ARCHNNE_LAYER_TENSOR_MEAN,
    ARCHNNE_LAYER_TENSOR_SQUEEZE,
    ARCHNNE_LAYER_TENSOR_STRIDE_SLICE,
    ARCHNNE_LAYER_TENSOR_ROUNDING,
    ARCHNNE_LAYER_HASHLUT,
    ARCHNNE_LAYER_LSH_PROJECT,
    ARCHNNE_LAYER_RESHAPE,
    ARCHNNE_LAYER_LUT2,
    ARCHNNE_LAYER_NORMALIZE2,
    ARCHNNE_LAYER_ADAPTER,
    ARCHNNE_LAYER_YUV2RGB_SCALE,
    ARCHNNE_LAYER_LSTM
}
archnne_operator_type;

/*************************************** Data Size Type definition ************************************/
enum arch_type_e {
    ARCH_TYPE_INVALID         = 0x000,/*!< \brief An invalid type value. When passed an error must be returned. */
    ARCH_TYPE_CHAR            = 0x001,/*!< \brief A <tt>\ref arch_char</tt>. */
    ARCH_TYPE_INT8            = 0x002,/*!< \brief A <tt>\ref arch_int8</tt>. */
    ARCH_TYPE_UINT8           = 0x003,/*!< \brief A <tt>\ref arch_uint8</tt>. */
    ARCH_TYPE_INT16           = 0x004,/*!< \brief A <tt>\ref arch_int16</tt>. */
    ARCH_TYPE_UINT16          = 0x005,/*!< \brief A <tt>\ref arch_uint16</tt>. */
    ARCH_TYPE_INT32           = 0x006,/*!< \brief A <tt>\ref arch_int32</tt>. */
    ARCH_TYPE_UINT32          = 0x007,/*!< \brief A <tt>\ref arch_uint32</tt>. */
    ARCH_TYPE_INT64           = 0x008,/*!< \brief A <tt>\ref arch_int64</tt>. */
    ARCH_TYPE_UINT64          = 0x009,/*!< \brief A <tt>\ref arch_uint64</tt>. */
    ARCH_TYPE_FLOAT32         = 0x00A,/*!< \brief A <tt>\ref arch_float32</tt>. */
    ARCH_TYPE_FLOAT64         = 0x00B,/*!< \brief A <tt>\ref arch_float64</tt>. */
    ARCH_TYPE_ENUM            = 0x00C,/*!< \brief A <tt>\ref arch_enum</tt>. Equivalent in size to a <tt>\ref arch_int32</tt>. */
    ARCH_TYPE_SIZE            = 0x00D,/*!< \brief A <tt>\ref arch_size</tt>. */
    ARCH_TYPE_DF_IMAGE        = 0x00E,/*!< \brief A <tt>\ref arch_df_image</tt>. */
    ARCH_TYPE_FLOAT16         = 0x00F,/*!< \brief A <tt>\ref arch_float16</tt>. */
    ARCH_TYPE_BOOL            = 0x010,/*!< \brief A <tt>\ref arch_bool</tt>. */

    ARCH_TYPE_RECTANGLE       = 0x020,/*!< \brief A <tt>\ref arch_rectangle_t</tt>. */
    ARCH_TYPE_KEYPOINT        = 0x021,/*!< \brief A <tt>\ref arch_keypoint_t</tt>. */
    ARCH_TYPE_COORDINATES2D   = 0x022,/*!< \brief A <tt>\ref arch_coordinates2d_t</tt>. */
    ARCH_TYPE_COORDINATES3D   = 0x023,/*!< \brief A <tt>\ref arch_coordinates3d_t</tt>. */
    ARCH_TYPE_COORDINATES2DF  = 0x024,/*!< \brief A <tt>\ref arch_coordinates2df_t</tt>. */

    /* Reserve enums that are defined in khronos extensions
        NN extensions:
        ARCH_TYPE_NN_CONVOLUTION_PARAMS     = 0x025,
        ARCH_TYPE_NN_DECONVOLUTION_PARAMS   = 0x026,
        ARCH_TYPE_NN_ROI_POOL_PARAMS        = 0x027,
        Classifier extension:
        ARCH_TYPE_CLASSIFER_MODEL           = 0x02C,
    */
    ARCH_TYPE_HOG_PARAMS                       = 0x028, /*!< \brief A <tt>\ref arch_hog_t</tt>. */
    ARCH_TYPE_HOUGH_LINES_PARAMS               = 0x029, /*!< \brief A <tt>\ref arch_hough_lines_p_t</tt>. */
    ARCH_TYPE_LINE_2D                          = 0x02A, /*!< \brief A <tt>\ref arch_line2d_t</tt>. */
    ARCH_TYPE_TENSOR_MATRIX_MULTIPLY_PARAMS    = 0x02B, /*!< \brief A <tt>\ref arch_tensor_matrix_multiply_params_t</tt>. */


    ARCH_TYPE_USER_STRUCT_START    = 0x100,/*!< \brief A user-defined struct base index.*/
    ARCH_TYPE_VENDOR_STRUCT_START  = 0x400,/*!< \brief A vendor-defined struct base index.*/
    ARCH_TYPE_KHRONOS_OBJECT_START = 0x800,/*!< \brief A Khronos defined object base index. */
    ARCH_TYPE_VENDOR_OBJECT_START  = 0xC00,/*!< \brief A vendor defined object base index. */

    ARCH_TYPE_WEIGHTS_BIASES_PARAMETER = ARCH_TYPE_VENDOR_OBJECT_START,
    ARCH_TYPE_WEIGHTS_BIASES_PARAMETER_BASE = ARCH_TYPE_VENDOR_OBJECT_START+1,

    ARCH_TYPE_KHRONOS_STRUCT_MAX   = ARCH_TYPE_USER_STRUCT_START - 1,/*!< \brief A value for comparison between Khronos defined structs and user structs. */

    ARCH_TYPE_USER_STRUCT_END      = ARCH_TYPE_VENDOR_STRUCT_START - 1,/*!< \brief A value for comparison between user structs and vendor structs. */
    ARCH_TYPE_VENDOR_STRUCT_END    = ARCH_TYPE_KHRONOS_OBJECT_START - 1,/*!< \brief A value for comparison between vendor structs and Khronos defined objects. */
    ARCH_TYPE_KHRONOS_OBJECT_END   = ARCH_TYPE_VENDOR_OBJECT_START - 1,/*!< \brief A value for comparison between Khronos defined objects and vendor structs. */
    ARCH_TYPE_VENDOR_OBJECT_END    = 0xFFF,/*!< \brief A value used for bound checking of vendor objects */


    ARCH_TYPE_REFERENCE       = 0x800,/*!< \brief A <tt>\ref arch_reference</tt>. */
    ARCH_TYPE_CONTEXT         = 0x801,/*!< \brief A <tt>\ref arch_context</tt>. */
    ARCH_TYPE_GRAPH           = 0x802,/*!< \brief A <tt>\ref arch_graph</tt>. */
    ARCH_TYPE_NODE            = 0x803,/*!< \brief A <tt>\ref arch_node</tt>. */
    ARCH_TYPE_KERNEL          = 0x804,/*!< \brief A <tt>\ref arch_kernel</tt>. */
    ARCH_TYPE_PARAMETER       = 0x805,/*!< \brief A <tt>\ref arch_parameter</tt>. */
    ARCH_TYPE_DELAY           = 0x806,/*!< \brief A <tt>\ref arch_delay</tt>. */
    ARCH_TYPE_LUT             = 0x807,/*!< \brief A <tt>\ref arch_lut</tt>. */
    ARCH_TYPE_DISTRIBUTION    = 0x808,/*!< \brief A <tt>\ref arch_distribution</tt>. */
    ARCH_TYPE_PYRAMID         = 0x809,/*!< \brief A <tt>\ref arch_pyramid</tt>. */
    ARCH_TYPE_THRESHOLD       = 0x80A,/*!< \brief A <tt>\ref arch_threshold</tt>. */
    ARCH_TYPE_MATRIX          = 0x80B,/*!< \brief A <tt>\ref arch_matrix</tt>. */
    ARCH_TYPE_CONVOLUTION     = 0x80C,/*!< \brief A <tt>\ref arch_convolution</tt>. */
    ARCH_TYPE_SCALAR          = 0x80D,/*!< \brief A <tt>\ref arch_scalar</tt>. when needed to be completely generic for kernel validation. */
    ARCH_TYPE_ARRAY           = 0x80E,/*!< \brief A <tt>\ref arch_array</tt>. */
    ARCH_TYPE_IMAGE           = 0x80F,/*!< \brief A <tt>\ref arch_image</tt>. */
    ARCH_TYPE_REMAP           = 0x810,/*!< \brief A <tt>\ref arch_remap</tt>. */
    ARCH_TYPE_ERROR           = 0x811,/*!< \brief An error object which has no type. */
    ARCH_TYPE_META_FORMAT     = 0x812,/*!< \brief A <tt>\ref arch_meta_format</tt>. */
    ARCH_TYPE_OBJECT_ARRAY    = 0x813,/*!< \brief A <tt>\ref arch_object_array</tt>. */
    /* Reserved for IX and XML extensions */
    /* ARCH_TYPE_IMPORT          = 0x814, !< \brief A <tt>\ref arch_import</tt>. */
    ARCH_TYPE_TENSOR          = 0x815,/*!< \brief A <tt>\ref arch_tensor</tt>. */
    /* Reserved for ARCH_TYPE_TARGET extensions*/
    ARCH_TYPE_TARGET          = 0x816,/*!< \brief A <tt>\ref arch_target</tt> */
    ARCH_TYPE_TENSOR_VIEW     = 0x817,/*!< \brief A <tt>\ref arch_tensor_view</tt>. */
    ARCH_TYPE_TENSOR_ADDRESS  = 0x818,/*!< \brief A <tt>\ref arch_tensor_addressing</tt>. */
    ARCH_TYPE_TENSOR_MEM      = 0x819,/*!< \brief A <tt>\ref arch_tensor_alloc_info</tt>. */

    /* \todo add new object types here */

};

/*************************************** typedef definition ************************************/
typedef enum arch_type_e                      arch_type_e;

#endif /* ARCH_SW_TYPE_H_ */
