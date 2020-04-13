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


#ifndef __GC_VX_COMMON_H__
#define __GC_VX_COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <time.h>
#include <ctype.h>

#if defined(__linux__) || defined(__ANDROID__) || defined(__QNX__) || defined(__APPLE__) || defined(__CYGWIN__)
#include <dlfcn.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#elif defined(_WIN32) || defined(UNDER_CE)
#include <windows.h>
#endif

#define OPENVX_TILING_1_0

#include <VX/vx.h>
#include <VX/vx_khr_tiling.h>
#include <VX/vx_lib_extras.h>
#include <VX/vx_compatibility.h>
#include <VX/vx_khr_import_kernel.h>
#include <VX/vx_ext_target.h>

#if defined(OPENVX_USE_NODE_MEMORY)
#include <VX/vx_khr_node_memory.h>
#endif
#if defined(OPENVX_USE_OPENCL)
#include <VX/vx_khr_opencl.h>
#endif
#if defined(OPENVX_USE_DOT)
#include <VX/vx_khr_dot.h>
#endif
#if defined(OPENVX_USE_XML)
#include <VX/vx_khr_xml.h>
#endif
#if defined(OPENVX_USE_TARGET)
#include <VX/vx_ext_target.h>
#endif
#if defined(OPENVX_USE_VARIANTS)
#include <VX/vx_khr_variants.h>
#endif

#include <gc_hal.h>
#include <gc_hal_user.h>


#include <VX/vx_ext_program.h>
#include <VX/vx_khr_cnn.h>

#include <gc_vx_layer.h>
#include "gc_vxk_common.h"

#include <gc_vx_profiler.h>
#ifdef ORI_NNARCHPERF
#include "nnArchPerfOri.h"
#else
#include "nnArchPerf.h"
#endif


/*
** Macro definitions
*/
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

/*#define USE_REF_INPUT*/
#define NUM_ROI             256

#define NN_MULTI_THREAD     1
#define NN_MULTI_THREAD2    1
#define NN_WSZIE_REG        0   /* to match old register. 05/18 FPGA result is not stable without this */
#define NN_LAYER_FLUSH      1
#define ENABLE_LRN_LAYER    0
#define ENABLE_COV_BATCH    1

#define VX_SHADER_TP        1

#define VX_NN_SH_PARALLEL   1

#define VX_NN_FC_ACCEL      0

#define NN_LAYER_C          0

#define ENABLE_SPLIT_WB     1



#define VXM_FORCE_PER_OPERATION_IDLE      0

#define TP_FC_Z_MAX         512
#define MAX_TP_FC_KZ_SIZE  ((0x1 <<16) -64)
#define NN_INTEGER_BIAS_BITS                27
#define NN_INTEGER_BIAS_BITS_VIP_V7         32
#define NN_INTEGER_BIAS_BITS_VIP_V7_INT16   48
#define NN_Z_POSITION_OFFSET_BITS           24
#define NN_Z_POSITION_OFFSET_BITS_VIP_V7    32

#define CONST_FILTER_PERCORE                0

#define VX_PRIVATE_API                      static
#define VX_INTERNAL_API
#define VX_INTERNAL_CALLBACK_API
#define VX_PUBLIC_API                       VX_API

#if defined(_WIN32) && !defined(__GNUC__)
#define VX_INLINE_API                       static _inline
#else
#define VX_INLINE_API                       static inline
#endif

#define VX_NULL                             gcvNULL

#define VX_FORMAT_HEX                       "0x%08X"
#define VX_FORMAT_SIZE                      "%lu"
#define VX_FORMAT_MS                        "%.3lfms"

#define VX_REF_SIGNATURE_ALIVE              0xCD9CCD9C
#define VX_REF_SIGNATURE_RELEASED           0xDEADDEAD

#define VX_MAX_PATH                         1024
#define VX_MAX_REF_COUNT                    8192
#define VX_MAX_NODE_COUNT                   2048
#define VX_MAX_MODULE_COUNT                 16
#define VX_MAX_TARGET_COUNT                 1
#define VX_MAX_KERNEL_COUNT                 1024
#define VX_MAX_USER_STRUCT_COUNT            1024
#define VX_MAX_CONVOLUTION_DIM              15
#define VX_INT_MAX_NONLINEAR_DIM            9
#define VX_MAX_OPTICAL_FLOW_WINDOW_DIM      9
#define VX_MAX_PARAMETERS                   48
#define VX_MAX_NODES_IN_GRAPH               VX_MAX_NODE_COUNT
#define VX_MAX_PLANES                       4
#define VX_MAX_NODE_COUNT_ONE_BLOCK         16
#define VX_MAX_NODE_PARENTS                 64
#define VX_MAX_NODE_CHILDREN                64

#define VX_HOST_CORE_COUNT                  1
#define VX_MAX_DEVICES                      gcdMAX_3DGPU_COUNT
#define VX_MAX_PRINTF_BUFFER_SIZE           (1024*1024)

#define VX_MAX_MCFE_SEMAPHORE               (128 * 8)

#define VX_CONTEXT_TENSOR_MAX_DIMENSION     6

#define MAP_UNMAP_REFERENCE                 0

#define NNE_COMMAND_SIZE                    128
#define TP_COMMAND_SIZE                     128

#define NN_IMAGE_XSIZE_MAX                  8191
#define NN_IMAGE_YSIZE_MAX                  8191
#define NN_IMAGE_ZSIZE_MAX                  16383

#define NN_KS_PARTIAL_CACHE_DATA_UNIT          0
#define CACHE_ALIGNMENT_SIZE                128
#define PRELOAD_WB_ALIGNMENT                64
#define VX_MAX_MEM_REQUEST_INPUT            64
#define VX_MAX_MEM_REQUEST_OUTPUT           32
#define VX_MAX_MEM_PARAM_INPUT              16
#define VX_MAX_MEM_PARAM_OUTPUT             8
#define VX_KERNEL_PATTERN_BIT_SIZE          64

#define VX_TRANSPOSE_MAX_INTERLEAVE_CH      16
#define VX_TRANSPOSE_MAX_INTERLEAVE_1MULTI1_CH      9

#define VX_GRAPH_COMMAND_BUFFER_SIZE        gcdCMD_BUFFER_SIZE
#define IMG_MAX_WIDTH (65536)

#define REGISTER_FRAME 1
#define REGISTER_FRAME_CL_RELAX 0
#define REGISTER_FRAME_LAYER_DEBUG 0

/* Function macros */
#ifndef vxmLENGTH_OF
#define vxmLENGTH_OF(array)                 (sizeof(array) / sizeof((array)[0]))
#endif

#define vxmASSERT                           gcmASSERT

#define vxInfo(...)  vxPRINT(VX_DEBUG_LEVEL_INFO, __VA_ARGS__)
#define vxWarning(...) vxPRINT(VX_DEBUG_LEVEL_INFO, __VA_ARGS__)
#define vxError(...) vxPRINT(VX_DEBUG_LEVEL_INFO, __VA_ARGS__)

#define vxmBOOL_TO_STRING(b)                ((b) ? "true" : "false")

#define vxmIS_SCALAR(type)                  (VX_TYPE_INVALID < (type) && (type) < VX_TYPE_SCALAR_MAX)
#define vxmIS_STRUCT(type)                  ((type) >= VX_TYPE_RECTANGLE && (type) < VX_TYPE_STRUCT_MAX)
#define vxmIS_OBJECT(type)                  (((type) >= VX_TYPE_REFERENCE && (type) < VX_TYPE_OBJECT_MAX) || ((type) == VX_TYPE_PROGRAM))

#define vxmIS_VALID_PARAMETERS(ptr, size, type, align) \
        ((size) == sizeof(type) && ((vx_size)(ptr) & (align)) == 0)

#define vxmVALIDATE_PARAMETERS(ptr, size, type, align) \
    vxmVALIDATE_PARAMETER(ptr, size, type, 1, align)
#define vxmVALIDATE_PARAMETER(ptr, size, type, num, align) \
        do \
        { \
            if ((size) != sizeof(type) * (num) || ((vx_size)(ptr) & (align)) != 0) return VX_ERROR_INVALID_PARAMETERS; \
        } \
        while (vx_false_e)

#define vxmVALIDATE_PARAMETERS_EX(ptr, size, type) \
        do \
        { \
            if ((size) != sizeof(type) || (vx_size)(ptr) == 0) return VX_ERROR_INVALID_PARAMETERS; \
        } \
        while (vx_false_e)

#define vxmIS_VALID_DIRECTION(d)            ((d) >= VX_INPUT && (d) <= VX_BIDIRECTIONAL)
#define vxmIS_VALID_DIRECTION_FOR_USER_KERNEL(d) \
        ((d) == VX_INPUT || (d) == VX_OUTPUT)

#define vxmIS_INPUT_OR_BIDIRECTION(d)       ((d) == VX_INPUT || (d) == VX_BIDIRECTIONAL)
#define vxmIS_OUTPUT_OR_BIDIRECTION(d)      ((d) == VX_OUTPUT || (d) == VX_BIDIRECTIONAL)

#define vxmIS_VALID_STATE(s)                ((s) == VX_PARAMETER_STATE_REQUIRED || (s) == VX_PARAMETER_STATE_OPTIONAL)

/* Is d aligned to align? */
#define vxmIS_ALIGNED(d, align)                    (!((d) & ((align) - 1)))

#define vxmCHECK_PRECISION(tensor, precisions) \
        if ((tensor != VX_NULL) && (tensor->tensorBuffer->precision != precisions)) \
            tensor->tensorBuffer->precision = precisions;

#define vxmCHECK_LIFETIME(tensor, lifetime) \
        if ((tensor != VX_NULL) && (tensor->tensorBuffer->data_lifetime != lifetime)) \
            tensor->tensorBuffer->data_lifetime = lifetime;

#define vxmIS_ERROR(status)  (status != VX_SUCCESS)
#define vxmONERROR(func) \
    do \
    { \
        status = func; \
        if (vxmIS_ERROR(status)) \
        { \
            goto OnError; \
        } \
    } \
    while (gcvFALSE)

#define vxmONERROR_STATUS(status) \
        do \
        { \
            if (vxmIS_ERROR((status))) \
            { \
                goto OnError; \
            } \
        } \
        while (gcvFALSE)


#define vxmONERROR_FALSE(func) \
        do \
        { \
            if (!func) \
            { \
                status = VX_FAILURE; \
                goto OnError; \
            } \
        } \
        while (gcvFALSE)

#define vxmONERROR_NULLPTR(ptr) \
                do \
                { \
                    if (VX_NULL == (ptr)) \
                    { \
                        status = VX_ERROR_NO_MEMORY; \
                        goto OnError; \
                    } \
                } \
                while (gcvFALSE)
#define vxmOPERATION_COUNT(layer)       gcmCOUNTOF(layer->operations)

#define VX_GET_DATA_FROM_TENSOR(tensor, index) \
    vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(tensor), TENSOR_QUANT_TYPE(tensor), index, TENSOR_LOGICAL_ADDR(tensor), TENSOR_POS(tensor), TENSOR_TF_ZEROPOINT(tensor), TENSOR_TF_SCALE(tensor))

#define VX_SAVE_DATA_TO_TENSOR(tensor, data, index) \
    vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(tensor), TENSOR_QUANT_TYPE(tensor), index, data, TENSOR_LOGICAL_ADDR(tensor), TENSOR_POS(tensor), TENSOR_TF_ZEROPOINT(tensor), TENSOR_TF_SCALE(tensor), TENSOR_ROUNDING_MODE(tensor))

#define CHECK_LIFETIME_IS_STATIC(tensor) \
    (((tensor != VX_NULL) && TENSOR_DATA_LIFETIME(tensor) == VX_TENSOR_LIFE_TIME_STATIC) ? vx_true_e : vx_false_e)


#define VX_INVALID_VALUE  0xDEADDEAD

#define vxmARCH_VIP_SRAM_SIZE \
    ((context->nnConfig.customizedFeature.vipSRAMSize <= VX_VIP_SRAM_IMAGE_STREAM_SIZE) ? context->nnConfig.customizedFeature.vipSRAMSize : \
    (context->nnConfig.customizedFeature.vipSRAMSize - VX_VIP_SRAM_IMAGE_STREAM_SIZE))

#define vxmARCH_AXI_SRAM_SIZE \
    (context->nnConfig.customizedFeature.axiSRAMSize)

/*
** typedefs
*/
typedef gctPOINTER                          vx_ptr;
typedef gctCONST_POINTER                    vx_const_ptr;
typedef gctPOINTER *                        vx_ptr_ptr;

typedef gctPOINTER                          vx_thread;

typedef gctSIGNAL                           vx_event;

typedef gctPOINTER                          vx_mutex;
typedef gctPOINTER *                        vx_mutex_ptr;

typedef vx_char *                           vx_char_ptr;

typedef vx_int8 *                           vx_int8_ptr;
typedef vx_int16 *                          vx_int16_ptr;
typedef vx_int32 *                          vx_int32_ptr;

typedef vx_uint8 *                          vx_uint8_ptr;
typedef vx_uint16 *                         vx_uint16_ptr;
typedef vx_uint32 *                         vx_uint32_ptr;

typedef vx_float32 *                        vx_float32_ptr;
typedef vx_float64 *                        vx_float64_ptr;

typedef vx_int64 *                          vx_int64_ptr;

typedef const vx_uint8 *                    vx_const_uint8_ptr;

typedef vx_char *                           vx_string;
typedef const vx_char *                     vx_const_string;

typedef vx_size                             vx_return_value;

typedef gcTHREAD_ROUTINE                    vx_thread_routine_f;

typedef vx_int32                            vx_build_status;

#ifndef VX_MAX_TARGET_NAME
#define VX_MAX_TARGET_NAME                  64
#endif

#define VX_LIBRARY_KHR_INTERNAL             0x1

enum vx_kernel_internal_e
{
    VX_KERNEL_INTERNAL_SOBEL_MxN                   = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x0,

    VX_KERNEL_INTERNAL_HARRIS_SCORE                = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x1,

    VX_KERNEL_INTERNAL_EUCLIDEAN_NONMAXSUPPRESSION = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x2,

    VX_KERNEL_INTERNAL_IMAGE_LISTER                = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x3,

    VX_KERNEL_INTERNAL_ELEMENTWISE_NORM            = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x4,

    VX_KERNEL_INTERNAL_NONMAXSUPPRESSION_CANNY     = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x5,

    VX_KERNEL_INTERNAL_EDGE_TRACE                  = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x6,

    VX_KERNEL_INTERNAL_COPY_IMAGE                  = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x7,

    VX_KERNEL_INTERNAL_FAST9CORNERS_STRENGTH       = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x8,

    VX_KERNEL_INTERNAL_FAST9CORNERS_NONMAX         = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x9,

    VX_KERNEL_INTERNAL_CREATE_LISTER               = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0xA,

    VX_KERNEL_INTERNAL_PACK_ARRAYS               = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0xB,

    VX_KERNEL_INTERNAL_MINMAXLOC_FILTER            = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0xC,

    VX_KERNEL_INTERNAL_GET_LOCATION                = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0xD,

    VX_KERNEL_INTERNAL_MIN_MAX_LOC_PACK_ARRAYS     = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0xE,

    VX_KERNEL_INTERNAL_EDGE_TRACE_THRESHOLD        = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0xF,

    VX_KERNEL_INTERNAL_EDGE_TRACE_HYSTERESIS       = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x10,

    VX_KERNEL_INTERNAL_EDGE_TRACE_CLAMP            = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x11,

    VX_KERNEL_INTERNAL_INTEGRAL_IMAGE_STEP         = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x12,

    VX_KERNEL_INTERNAL_SCHARR3x3                   = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x13,

    VX_KERNEL_INTERNAL_VLK_TRACKER                 = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x14,

    VX_KERNEL_INTERNAL_EQUALIZE_HISTOGRAM_HIST     = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x15,

    VX_KERNEL_INTERNAL_EQUALIZE_HISTOGRAM_GCDF     = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x16,

    VX_KERNEL_INTERNAL_EQUALIZE_HISTOGRAM_CDF      = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x17,

    VX_KERNEL_INTERNAL_EQUALIZE_HISTOGRAM_LUT      = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x18,

    VX_KERNEL_INTERNAL_SGM                         = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x19,

    VX_KERNEL_INTERNAL_SGM_COST                    = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x1A,

    VX_KERNEL_INTERNAL_SGM_PATH90                  = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x1B,

    VX_KERNEL_INTERNAL_SGM_PATH45                  = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x1C,

    VX_KERNEL_INTERNAL_SGM_PATH135                 = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x1D,

    VX_KERNEL_INTERNAL_SGM_PATH0                   = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x1E,

    VX_KERNEL_INTERNAL_SGM_DISP                    = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x1F,

    VX_KERNEL_INTERNAL_LAPLACIAN3x3                = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x20,

    VX_KERNEL_INTERNAL_CENSUS3x3                   = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x21,

    VX_KERNEL_INTERNAL_SOBEL_MxN_F16               = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x22,

    VX_KERNEL_INTERNAL_ELEMENTWISE_NORM_F16        = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x23,

    VX_KERNEL_INTERNAL_PHASE_F16                   = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x24,

    VX_KERNEL_INTERNAL_ADAPTER                     = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x25,

    VX_KERNEL_INTERNAL_IMAGE_COPY                  = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x26,

    VX_KERNEL_INTERNAL_ARRAY_COPY                  = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x27,

    VX_KERNEL_INTERNAL_SCALAR_COPY                 = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x28,

    VX_KERNEL_INTERNAL_LUT_COPY                    = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x29,

    VX_KERNEL_INTERNAL_MATRIX_COPY                 = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x2A,

    VX_KERNEL_INTERNAL_CONVOLUTION_COPY            = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x2B,

    VX_KERNEL_INTERNAL_DISTRIBUTION_COPY           = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x2C,

    VX_KERNEL_INTERNAL_TENSOR_COPY                 = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x2D,

    VX_KERNEL_INTERNAL_THRESHOLD_COPY              = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x2E,

    VX_KERNEL_INTERNAL_REMAP_COPY                  = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x2F,

    VX_KERNEL_INTERNAL_HOUGH_MAKEPOINTS            = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x30,

    VX_KERNEL_INTERNAL_HOUGH_FILLACCUM             = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x31,

    VX_KERNEL_INTERNAL_BILATERAL_FILTER            = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x32,

    VX_KERNEL_INTERNAL_HOUGH_GETLINES              = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x33,

    VX_KERNEL_INTERNAL_UPSAMPLE_PADDING            = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x34,

    VX_KERNEL_INTERNAL_UPSAMPLE_CONVERT            = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x35,

    VX_KERNEL_INTERNAL_PYRAMID_COPY_IMAGE          = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x36,

    VX_KERNEL_INTERNAL_SCALAR_OPERATION            = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x37,

    VX_KERNEL_INTERNAL_TRANSPOSE_2D_TENSOR         = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x38,

    VX_KERNEL_INTERNAL_MULTIPLY_2D_MATRIXES        = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x39,

    VX_KERNEL_INTERNAL_ROI_POOLING_RELU_LAYER      = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x3A,
};


enum vx_convolutional_network_pooling_internal_type_e
{
    /*! \brief first pixel pooling, stride 2*/
    VX_NN_POOLING_FPP = VX_ENUM_BASE(VX_ID_VIVANTE, VX_ENUM_NN_POOLING_TYPE) + 0x2,
};

enum vx_convolutional_network_layer_internal_type_e
{
    /*! \brief depth-wise convolution layer */
    VX_NN_DEPTH_WISE_CONVOLUTION_LAYER = VX_ENUM_BASE(VX_ID_VIVANTE, VX_ENUM_NN_LAYER_TYPE) + 0x0,
};

#if defined(__linux__) || defined(__ANDROID__) || defined(__QNX__)

#define VX_MODULE_NAME(name)                "lib"name".so"

#define VX_MODULE_POSTFIX_NAME              ".so"

#elif defined(_WIN32) || defined(UNDER_CE)

#define VX_MODULE_NAME(name)                ""name".dll"

#define VX_MODULE_POSTFIX_NAME              ".dll"

#else

#error Unknown platform

#endif

typedef gctHANDLE                           vx_module_handle;

typedef gctHANDLE                           vx_symbol_handle;

#define VX_NULL_MODULE_HANDLE               ((vx_module_handle)VX_NULL)

#define VX_DEFAULT_TARGET_NAME              "vivante.any"

typedef struct _vx_value_set_s
{
    vx_return_value                         v1;
    vx_return_value                         v2;
    vx_return_value                         v3;
}
vx_value_set_s;

typedef vx_value_set_s *                    vx_value_set;
typedef vx_value_set *                      vx_value_set_ptr;

#define VX_QUEUE_DEPTH                      64

typedef struct _vx_queue_s
{
    vx_value_set                            data[VX_QUEUE_DEPTH];

    vx_mutex                                lock;

    vx_bool                                 stopped;

    vx_int32                                beginIndex;
    vx_int32                                endIndex;

    vx_event                                readEvent;
    vx_event                                writeEvent;
}
vx_queue_s;

typedef vx_queue_s *                        vx_queue;
typedef vx_queue *                          vx_queue_ptr;

typedef struct _vx_processor_s
{
    vx_queue_s                              input;
    vx_queue_s                              output;

    vx_thread                               thread;

    vx_bool                                 running;
}
vx_processor_s;

typedef vx_processor_s *                    vx_processor;

#define VX_USE_THREADPOOL                   0

#if VX_USE_THREADPOOL
struct _vx_threadpool_s;
struct _vx_threadpool_worker_s;
typedef struct _vx_threadpool_worker_s      vx_threadpool_worker_s;
typedef vx_threadpool_worker_s *            vx_threadpool_worker;

typedef vx_bool (*vx_threadpool_worker_callback_f)(vx_threadpool_worker worker);

typedef struct _vx_threadpool_s             vx_threadpool_s;
typedef vx_threadpool_s *                   vx_threadpool;
typedef vx_threadpool *                     vx_threadpool_ptr;

struct _vx_threadpool_worker_s
{
    vx_threadpool                           threadPool;

    vx_queue                                queue;

    vx_uint32                               index;

    vx_bool                                 active;

    vx_threadpool_worker_callback_f         callback;

    vx_ptr                                  arg;

    vx_value_set                            data;

    vx_perf_t                               perf;

    vx_thread                               thread;
};

typedef struct _vx_work_s
{
    vx_target                               target;

    vx_node                                 node;

    vx_enum                                 action;
}
vx_work_s;
#endif

typedef enum vx_reference_kind_e
{
    VX_REF_EXTERNAL                         = 0x0000,
    VX_REF_INTERNAL                         = 0x0001
}
vx_reference_kind_e;

typedef enum vx_type_e                      vx_type_e;

typedef struct _vx_reference
{
    vx_uint32                               signature;

    vx_context                              context;

    vx_type_e                               type;

    vx_reference                            scope;

    vx_mutex                                lock;

    vx_uint32                               externalCount;
    vx_uint32                               internalCount;

    vx_uint32                               readCount;
    vx_uint32                               writeCount;

    vx_bool                                 extracted;
    vx_bool                                 isVirtual;
    vx_bool                                 accessible;

    vx_char                                 name[VX_MAX_REFERENCE_NAME];

    vx_delay                                delay;
    vx_int32                                delayIndex;

    vx_ptr                                  reserved;

    vx_bool                                 isStage;/*if enable, skip write dependency checking*/
}
vx_reference_s;

typedef vx_reference *                      vx_reference_ptr;

typedef struct _vx_error
{
    vx_reference_s                          base;

    vx_status                               status;
}
vx_error_s;

typedef vx_error_s *                        vx_error;
typedef vx_error *                          vx_error_ptr;

typedef union
{
    vx_char                                 c;

    vx_int8                                 n8;
    vx_uint8                                u8;
    vx_int16                                n16;
    vx_uint16                               u16;
    vx_int32                                n32;
    vx_uint32                               u32;
    vx_int64                                n64;
    vx_int64                                u64;

#if defined(OPENVX_PLATFORM_SUPPORTS_16_FLOAT)
    vx_float16                              f16;
#endif
    vx_float32                              f32;
    vx_float64                              f64;

    vx_df_image                             imageFormat;

    vx_enum                                 e;

    vx_size                                 s;

    vx_bool                                 b;
}vx_scalar_data;

typedef struct _vx_scalar
{
    vx_reference_s                          base;

    vx_enum                                 dataType;

    vx_scalar_data *                        value;

    vx_ptr                                  userValue;

    void *                                  node;
    vx_uint32                                physical;
}
vx_scalar_s;

typedef vx_scalar *                         vx_scalar_ptr;

typedef struct _vx_signature_s
{
    vx_uint32                               paramCount;

    vx_enum                                 directionTable[VX_MAX_PARAMETERS];

    vx_enum                                 dataTypeTable[VX_MAX_PARAMETERS];

    vx_enum                                 stateTable[VX_MAX_PARAMETERS];

    vx_bool                                 isStaticTable[VX_MAX_PARAMETERS];
}
vx_signature_s;

typedef vx_signature_s *                    vx_signature;

typedef struct _vx_parameter
{
    vx_reference_s                          base;

    vx_uint32                               index;

    vx_node                                 node;

    vx_kernel                               kernel;
}
vx_parameter_s;

#if gcdVX_OPTIMIZER
typedef struct _vx_kernel_optimization_attribute_s
{
    vx_bool                                 hwModule;
    vx_uint8                                moduleClass;
    vx_uint32                               outputSize;
    vx_uint32                               outputStride;
    vx_uint32                               inputSize;
    vx_uint32                               inputStride;
    vx_uint32                               inputOffset;
    vx_uint32                               tileWalkingPattern;
    vx_uint32                               bufferAttribute;

    vx_uint32                               outputTileSize;
    vx_uint32                               outputTileStride;
    vx_uint32                               inputTileSize;
    vx_uint32                               inputTileStride;

    vx_bool                                 oneKernelModule;
    vx_bool                                 tileOptimizable;
    vx_uint32                               dim;
    vx_uint32                               xDependency;
    vx_uint32                               yDependency;
    vx_uint32                               zDependency;
    vx_uint32                               inputImageCount;
    vx_uint32                               outputImageCount;
}
vx_kernel_optimization_attribute_s;
#endif

typedef struct _vx_kernel_attribute_s
{
    vx_size                                 localDataSize;
    vx_ptr                                  localDataPtr;

    vx_size                                 globalDataSize;
    vx_ptr                                  globalDataPtr;

    vx_border_t                             borderMode;

    vx_bool                                 isAllGPU;

    vx_bool                                 isGPUKernel;

#ifdef OPENVX_KHR_TILING
    vx_tile_block_size_t                    tileBlockSize;

    vx_neighborhood_size_t                  inputNeighborhoodSize;

    vx_size                                 tileMemorySize;
    vx_ptr                                  tileMemoryPtr;
#endif
    vx_reference                            stagings[10];

    void *                                  stageMemory[10];

    vx_kernel_execution_parameters_t        shaderParameter;

#if gcdVX_OPTIMIZER
    /* User-input attributes. */
    vx_kernel_optimization_attribute_s      optAttributes;
#endif

    vx_bool                                 validRectReset;
}
vx_kernel_attribute_s;

typedef enum
{
    VX_MULTIVIP_REFERENCE_START = 10,
    VX_MULTIVIP_INPUT_TENSOR_REFERENCE = VX_MULTIVIP_REFERENCE_START,
    VX_MULTIVIP_OUTPUT_TENSOR_REFERENCE,
    VX_MULTIVIP_WEIGHT_TENSOR_REFERENCE,
    VX_MULTIVIP_BIAS_TENSOR_REFERENCE,
    VX_MULTIVIP_WEIGHT_BIAS_PARAM_REFERENCE,
    VX_MULTIVIP_REFERENCE_END,
}
vx_multiVIP_reference_type_t;

typedef enum
{
    VX_MULTIVIP_CONV_SPLIT_NONE = 0,
    VX_MULTIVIP_CONV_SPLIT_Y_AXIS,
    VX_MULTIVIP_CONV_SPLIT_Z_AXIS,
    VX_MULTIVIP_CONV_SPLIT_END,
}
vx_multiVIP_conv_split_axis_t;

typedef struct _vx_tp_coomandInfo_s
{
    vx_uint32 inImageXSize;
    vx_uint32 inImageYSize;
    vx_uint32 inImageZSize;
    vx_uint32 inImageStride;
    vx_uint32 inImageSlice;
    vx_int32  inWindowXStart;
    vx_int32  inWindowYStart;
    vx_uint32 inWindowXEnd;
    vx_uint32 inWindowYEnd;
    vx_uint32 inTileSequence;
    vx_uint32 inTileListGlobalMem;
    vx_uint32 inImageGlobalMem;
    vx_uint32 aluI2FEnable;
    vx_uint32 aluSquareEnable;
    vx_uint32 aluHorzProcessing;
    vx_uint32 aluHorzProcCount;
    vx_uint32 aluHorzProcStride;
    vx_uint32 aluVertProcessing;
    vx_uint32 aluVertProcCount;
    vx_uint32 aluVertProcStride;
    vx_uint32 aluNmsEnable;
    vx_uint32 aluPwlEnable;
    vx_uint32 aluMultEnable;
    vx_uint32 aluF2IEnable;
    vx_uint32 aluLoadPwlLUT;
    vx_uint32 aluLoadPwlLUTGlobalMem;
    vx_uint32 inImageBaseAddress;
    vx_uint32 inTileListAddress;
    vx_uint32 inTileXSize;
    vx_uint32 inTileYSize;
    vx_uint32 inTileXInc;
    vx_uint32 inTileYInc;
    vx_uint32 aluLoadPwlLUTAddress;
    vx_uint32 outTileSkipAtborder;
    vx_uint32 outGlobalMem;
    vx_uint32 outLoop1Reset;
    vx_uint32 outLoop2Reset;
    vx_uint32 outLoop3Reset;
    vx_uint32 outBrickMode;
    vx_uint32 aluZFilterMode;
    vx_uint32 inWindowZStartOverfetch;
    vx_uint32 inWindowZEndOverfetch;
    vx_uint32 aluSquarePreshift;
    vx_uint32 last;
    vx_uint32 outBaseAddress;
    vx_uint32 outLoop0Inc;
    vx_uint32 outLoop1Inc;
    vx_uint32 outLoop2Inc;
    vx_uint32 outLoop3Inc;
    vx_uint32 outLoop4Inc;
    vx_uint32 outLoop5Inc;
    vx_uint32 outLoop6Inc;
    vx_uint32 outLoop0Count;
    vx_uint32 outLoop1Count;
    vx_uint32 outLoop2Count;
    vx_uint32 outLoop3Count;
    vx_uint32 outLoop4Count;
    vx_uint32 outLoop5Count;
    vx_uint32 inImageDataType;
    vx_uint32 outImageDataType;
    vx_uint32 kernelDataType;
    vx_uint32 aluFilterPwlSwap;
    vx_uint32 aluPwlSignSupport;
    vx_uint32 aluReluEnable;
    vx_uint32 floatRoundingMode;
    vx_uint32 integeroundingMode;
    vx_uint32 aluReorderBitsUsed;
    vx_uint32 aluReorderLoop2Mode;
    vx_uint32 inImageBorderMode;
    vx_int32  inImageBorderConst;
    vx_int32  aluInputPreshift;
    vx_int32  aluOutputPostshift;
    vx_int32  noFlush;
}
vx_tp_coomandInfo_s;

typedef struct _vx_tp_roi_coomandInfo_s
{
    vx_uint32 poolingHsize;
    vx_uint32 hstride;
    vx_uint32 poolingVsize;
    vx_uint32 vstride;
    vx_uint32 last;
    vx_uint32 xcoord;
    vx_uint32 ycoord;
}
vx_tp_roi_coomandInfo_s;

typedef struct _vx_tp_convLayerDesc_s
{
    vx_uint32 outImageZsize;
    vx_uint32 kernelXsize;
    vx_uint32 kernelYsize;
    vx_uint32 kernelZsize;
    vx_uint32 xStride;
    vx_uint32 yStride;
}
vx_tp_convLayerDesc_s;

typedef struct _vx_tp_poolLayerDesc_s
{
    vx_uint32 poolXsize;
    vx_uint32 poolYsize;
    vx_uint32 xStride;
    vx_uint32 yStride;
}
vx_tp_poolLayerDesc_s;

typedef struct _vx_tp_imageDesc_s
{
    vx_uint32 baseAddr;
    vx_uint32 xsize;
    vx_uint32 ysize;
    vx_uint32 zsize;
    vx_uint32 stride;
    vx_uint32 slice;
}
vx_tp_imageDesc_s;

typedef struct _vx_tp_packedImageDesc_s
{
    vx_uint32 baseAddr;
    vx_uint32 xsize;
    vx_uint32 ysize;
    vx_uint32 zsize;
}
vx_tp_packedImageDesc_s;

typedef struct _vx_program
{
    vx_reference_s                          base;
    gctSTRING                               source;
    gctUINT                                 binarySize;
    gctUINT8_PTR                            binary;
    gctSTRING                               buildOptions;
    gctSTRING                               buildLog;
    vx_build_status                         buildStatus;
    gctBOOL                                 linked;
}
vx_program_s;

typedef vx_program *                        vx_program_ptr;

typedef struct _vx_mem
{
    union {
        struct {
            void *                  image;
            vx_df_image             format;
            size_t                  width;
            size_t                  height;
            size_t                  depth;
            size_t                  rowPitch;
            size_t                  slicePitch;
            gctSIZE_T               elementSize;
            gctSIZE_T               size;
            gctUINT                 allocatedSize;
            gctUINT32               physical;           /* Image header. */
            gctPOINTER              logical;            /* Image header. */
            gcsSURF_NODE_PTR        node;
            gceSURF_FORMAT          internalFormat;
            gcoTEXTURE              texture;
            gcoSURF                 surface;
            gctBOOL                 surfaceMapped;
            gctUINT32               texturePhysical;    /* Texture data. */
            gctPOINTER              textureLogical;     /* Texture data. */
            gctUINT                 textureStride;      /* Texture data. */
        } image;
    } u;
}
vx_mem_s;

typedef vx_mem_s *                      vx_mem;

typedef struct _vx_mem_alloc_info
{
    gctUINT                 allocatedSize;
    gctUINT32               physical; /* gpu virtual address */
    gctPOINTER              logical;
    gcsSURF_NODE_PTR        node;
    gctPOINTER              data;
}
vx_mem_alloc_info_s;

typedef vx_mem_alloc_info_s *               vx_mem_alloc_info;

typedef struct _vx_argument
{
    gcUNIFORM               uniform;
    gctUINT32               size;
    gctPOINTER              data;
    gctBOOL                 set;
    gctBOOL                 isMemAlloc;
    gctBOOL                 isPointer;
    gctBOOL                 isMemObj;
    gctBOOL                 noBatch;
    gctBOOL                 isVivArray;
    gctUINT32               components;
}
vx_argument_s;

typedef vx_argument_s *                     vx_argument;

typedef struct _vx_shader_states
{
    gctUINT8_PTR            binary;

    /* States info. */
    gcsPROGRAM_STATE        programState;
}
vx_shader_states_s;

typedef struct _vx_shader_s
{
    gctSTRING               name;
    size_t                  maxWorkGroupSize;
    size_t                  maxWorkItemSizes[3];
    size_t                  compileWorkGroupSize[3];
    size_t                  preferredWorkGroupSizeMultiple;

    gctUINT64               maxGlobalWorkSize;
    vx_uint64               localMemSize;
    vx_uint64               privateMemSize;
    gctSIZE_T               constantMemSize;
    gctCHAR *               constantMemBuffer;

    gctUINT                 numArgs;
    vx_argument             args;

    vx_shader_states_s      states;
    gctUINT                 attributeCount;

    gctBOOL                 hasPrintf;

}
vx_shader_s;

typedef vx_shader_s *                vx_shader;

typedef struct _vx_kernel
{
    vx_reference_s                          base;

    vx_char                                 name[VX_MAX_KERNEL_NAME];

    vx_enum                                 enumeration;

    vx_program                              program;

    vx_kernel_f                             function;

    vx_shader                               *kernelShader;
    vx_uint32                               kernelShaderCount;
    vx_uint32                               currShaderID;

    vx_char                                 subname[VX_MAX_KERNEL_NAME];

    vx_signature_s                          signature;

    vx_bool                                 enabled;

    vx_bool                                 isUserkernel;

    vx_kernel_validate_f                    validateFunction;

    vx_kernel_input_validate_f              inputValidateFunction;

    vx_kernel_output_validate_f             outputValidateFunction;

    vx_kernel_initialize_f                  initializeFunction;

    vx_kernel_deinitialize_f                deinitializeFunction;

    vx_kernel_deinitialize_f                deinitializeWrapFunction; /* for user node */

    vx_kernel_attribute_s                   attributes;

    vx_uint32                               targetIndex;

#ifdef OPENVX_TILING_1_0
    vx_tiling_kernel_f                      tilingFunction;
#endif
}
vx_kernel_s;

typedef vx_kernel *                         vx_kernel_ptr;

typedef struct _vx_module_s
{
    vx_char                                 name[VX_MAX_PATH];

    vx_module_handle                        handle;
}
vx_module_s;

typedef struct _vx_node_block_s *vx_node_block;
typedef struct _vx_memory_pool_s *vx_memory_pool;

typedef vx_status (* vx_target_initialize_f)        (vx_target target);

typedef vx_status (* vx_target_deinitialize_f)      (vx_target target);

typedef vx_status (* vx_target_iskernelsupported_f) (
        vx_target target, vx_char targetName[VX_MAX_TARGET_NAME], vx_char kernelName[VX_MAX_TARGET_NAME],
#if defined(OPENVX_USE_VARIANTS)
        vx_char variantName[VX_MAX_VARIANT_NAME],
#endif
        vx_uint32_ptr indexPtr);

typedef vx_kernel (*vx_target_addkernel_f)          (
        vx_target target, const vx_char name[VX_MAX_KERNEL_NAME], vx_enum enumeration,
        vx_program program, vx_kernel_f funcPtr, vx_uint32 paramCount,
        vx_kernel_validate_f validate, vx_kernel_input_validate_f inputValidator, vx_kernel_output_validate_f outputValidator,
        vx_kernel_initialize_f initializer, vx_kernel_deinitialize_f deinitializer);

typedef vx_kernel (* vx_target_addtilingkernel_f)   (
        vx_target target, vx_char name[VX_MAX_KERNEL_NAME], vx_enum enumeration,
        vx_tiling_kernel_f flexibleFuncPtr, vx_tiling_kernel_f fastFuncPtr, vx_uint32 paramCount,
        vx_kernel_input_validate_f inputValidator, vx_kernel_output_validate_f outputValidator);

typedef vx_status (* vx_target_verifynode_f)        (
        vx_target target, vx_node node);

typedef vx_action (* vx_target_processnodes_f)      (
        vx_target target, vx_node nodes[], vx_size startIndex, vx_size nodeCount);

typedef vx_action (* vx_target_processlayer_f)      (
        vx_target target, vxnne_layer layer);

typedef vx_action (* vx_target_processblock_f)      (
        vx_target target, vx_node_block nodeBlock);

typedef struct _vx_target_funcs_s
{
    vx_target_initialize_f                  initialize;
    vx_target_deinitialize_f                deinitialize;

    vx_target_iskernelsupported_f           iskernelsupported;
    vx_target_addkernel_f                   addkernel;
    vx_target_addtilingkernel_f             addtilingkernel;

    vx_target_verifynode_f                  verifynode;
    vx_target_processnodes_f                processnodes;
    vx_target_processblock_f                processnodesBlock;
    vx_target_processlayer_f                processLayer;
}
vx_target_funcs_s;

typedef struct _vx_target
{
    vx_reference_s                          base;

    vx_bool                                 enabled;

    vx_char                                 name[VX_MAX_TARGET_NAME];

    vx_module_s                             module;

    vx_target_funcs_s                       funcs;

    vx_uint32                               priority;

    vx_uint32                               kernelCount;
    vx_kernel_s                             kernelTable[VX_MAX_KERNEL_COUNT];

    vx_ptr                                  reserved;

    vx_context                              context;
}
vx_target_s;

typedef vx_target *                         vx_target_ptr;

typedef struct _vx_accessor
{
    vx_bool                                 used;

    vx_enum                                 usage;

    vx_ptr                                  ptr;

    vx_bool                                 allocated;

    vx_reference                            ref;

    vx_ptr                                  extraDataPtr;
}
vx_accessor_s;

typedef struct _vx_user_struct
{
    vx_enum                                 type;
    vx_size                                 size;
}
vx_user_struct_s;

typedef union _vx_memory_map_extra_s
{
    struct
    {
        vx_rectangle_t rect;
        vx_uint32 plane_index;
    } image_data;

    struct
    {
        vx_size start;
        vx_size end;
    } array_data;

} vx_memory_map_extra_s;

typedef struct _vx_memory_map_s
{
    vx_bool used;

    vx_reference ref;

    vx_memory_map_extra_s extra;

    vx_enum usage;

    vx_enum mem_type;

    vx_uint32 flags;

    void* logical;

} vx_memory_map_s;

typedef struct _vx_reference_item
{
    vx_reference                            ref;
    struct _vx_reference_item *             prev;
    struct _vx_reference_item *             next;
} vx_reference_item_s, *vx_reference_item;

typedef struct _vx_global_data_s
{
    vx_uint32                               refGlobalDataCount;

    vxnne_kernel_shaders_s                  kernels[VXNNE_KERNEL_FIXED_COUNT + VXNNE_KERNEL_DYNAMIC_COUNT + 1];

    /* gcCompileKernel */
    gctCLCompiler                           compileKernel;
    gctHANDLE                               libCLC;
    gceSTATUS                               (*loadCompiler)(IN gcsHWCaps *HWCaps, IN gcePATCH_ID PatchId);
    gceSTATUS                               (*unloadCompiler)(void);

    APMHandle                               apm;

    vxnne_sram_s                            axiSRAM[MAX_GPU_CORE_COUNT];
    vxnne_sram_s                            vipSRAM;
#if gcdUSE_VXC_BINARY
    void *                                  libNNVXCKernelHandle;
    void *                                  libOvx12VXCBinaryHandle;
    void *                                  libNNGPUKernelHandle;
#endif


    vx_drv_option                           options;
    vx_nn_config                            nnConfig;
} vx_global_data_s, *vx_global_data;

typedef struct _vx_wb_list
{
    vx_weights_biases_parameter             wb;
    struct _vx_wb_list                      *next;
} vx_wb_list_s, *vx_wb_list;

typedef struct _vx_context
{
    vx_reference_s                          base;
    vx_global_data                          globalData;
    vx_uint32                               refTotalCount;
    vx_uint32                               refFreeCount;
    vx_reference_item                       refListHead;
    vx_reference_item                       refListTail;

    vx_uint32                               moduleCount;
    vx_module_s                             moduleTable[VX_MAX_MODULE_COUNT];

    vx_processor_s                          processor;

    vx_uint32                               kernelCount;
    vx_uint32                               uniqueKernelCount;

    vx_uint32                               targetCount;
    vx_target_s                             targetTable[VX_MAX_TARGET_COUNT];
    vx_uint32                               targetPriorityTable[VX_MAX_TARGET_COUNT];

    vx_log_callback_f                       logCallback;
    vx_mutex                                logLock;
    vx_bool                                 logEnabled;
    vx_bool                                 logCallbackReentrant;

    vx_accessor_s                           accessorTable[VX_MAX_REF_COUNT];

    vx_mutex                                memoryMapsLock;
    vx_memory_map_s                         memoryMaps[VX_MAX_REF_COUNT];

    vx_user_struct_s                        userStructTable[VX_MAX_USER_STRUCT_COUNT];

#if (NN_MULTI_THREAD || NN_MULTI_THREAD2)
    vx_event                                cnnEvent;
    vx_event                                cnnEvent2;
    vx_thread                               cnnThread;
#endif

#if VX_USE_THREADPOOL
    vx_threadpool                           threadPool;
#endif

    vx_uint32                               NextDynamicUserKernelID;

    vx_uint32                               NextDynamicUserLibraryID;

    vx_border_t                             immediateBorderMode;
    vx_enum                                 immediateBorderModePolicy;

    vx_enum                                 immediateTargetEnum;

    vx_char                                 immediateTargetString[VX_MAX_TARGET_NAME];

    vx_evis_no_inst_s                       evisNoInst;

    /* Profiler */
#if VIVANTE_PROFILER
    vx_profiler_s                           profiler;
    gcoPROFILER                             halProfile;
#endif

    vx_uint32                               deviceCount;

    vx_uint32                               memoryCount;

    /* hw chip info */
    vx_hw_chip_info                         hwChipInfo;
    vx_uint32                               cnnAvailableEventID;

    vx_uint32                               allTensorNum;

    vx_char                                 productName[32];
    vx_uint32                               pid;
    vx_uint32                               graphCount;
    vx_ptr_ptr                              binaryGraphInitBuffer;
    vx_uint32_ptr                           binaryGraphInitSize;
    vx_uint32                               SumTotalKernelBufferSize;
    vx_uint32                               CurrentContigousSize;
    vx_uint8_ptr*                           Logical;
    vx_uint32_ptr                           Physical;
    gcsSURF_NODE_PTR*                       Node;

    /*COPY_FROM_GLOBAL_DATA*/
    vxnne_sram_s                            axiSRAM[MAX_GPU_CORE_COUNT];
    vxnne_sram_s                            vipSRAM;
    vx_drv_option                           options;
    vx_nn_config                            nnConfig;
    /*end COPY_FROM_GLOBAL_DATA*/

    vx_wb_list                              wbList;
}
vx_context_s;

typedef vx_context *                        vx_context_ptr;

typedef struct _vx_cost_factors_s
{
    vx_size                                 bandwidth;

    vx_float32                              power;

    vx_float32                              cyclesPerUnit;

    vx_uint64                               overhead;
}
vx_cost_factors_s;

#if gcdVX_OPTIMIZER
typedef struct _vx_node_optimization_attribute_s
{
    vx_uint32                               registerCount;
    vx_uint32                               inputImageCount;
    vx_uint32                               outputImageCount;
    vx_uint32                               maxWorkGroupSize;
    vx_uint32                               workingSetSize;
    vx_uint32                               tileSize[3];
}
vx_node_optimization_attribute_s;
#endif

typedef vx_status (*vx_node_block_initialize_f)(vx_node_block nodeBlock);
typedef vx_status (*vx_node_block_deinitialize_f)(vx_node_block nodeBlock);
typedef vx_status (*vx_node_block_execute_f)(vx_node_block nodeBlock);

typedef struct _vx_node_block_s
{
    vx_graph                                graph;

    vx_size                                 nodeNum;
    vx_node                                 nodes[VX_MAX_NODE_COUNT];

    vx_status                               status;
    vx_bool                                 executed;

    vxnne_layer                             layer;

    vx_node_block_deinitialize_f            deinitialize;
    vx_node_block_execute_f                 execute;
}
vx_node_block_s;


#include <gc_vx_binary.h>

typedef struct _vx_node
{
    vx_reference_s                          base;

    vx_graph                                graph;

    vx_kernel                               kernel;

    vx_reference                            paramTable[VX_MAX_PARAMETERS];

    vx_status                               status;

    vx_perf_t                               perf;

    vx_nodecomplete_f                       completeCallback;

    vx_bool                                 visited;

    vx_bool                                 executed;


    vx_kernel_attribute_s                   kernelAttributes;

    vx_uint32                               targetIndex;

    vx_graph                                childGraph;

#if (NN_MULTI_THREAD || NN_MULTI_THREAD2)
    vx_graph                                workGraph;
#endif

    vx_cost_factors_s                       costFactors;

    void *                                  cmdBuffer;
    vx_size                                 cmdSizeBytes;

#if gcdVX_OPTIMIZER
    /* Derived attributes. */
    vx_node_optimization_attribute_s        optAttributes;

    void *                                  kernelContext;

    void *                                  instructionNode;
    void *                                  optimizedCmdBuffer;
    vx_size                                 optimizedCmdSizeBytes;
#else
    void *                                  kernelContext;
#endif

    vx_bool                                 forceWaitForEvent;

    vx_uint32                               cnnTriggerEventID;
    vx_uint32                               cnnWaitEventID0;
    vx_uint32                               cnnWaitEventID1;

    vxnne_layer                             layer;

    vx_uniform                              uniforms;
    vx_uint32                               uniformCount;

    vx_bool                                 isReplicated;

    vx_bool                                 replicated_flags[VX_MAX_PARAMETERS];

    vx_bool                                 localDataChangeIsEnabled;

    vx_bool                                 localDataSetByImplementation;

    vx_uint32                               patchLocation[VX_MAX_PARAMETERS][VX_MAX_PLANES];
    vxnne_sync_mode_e                       waitMode;
    vxnne_sync_mode_e                       wakeMode;
    gctUINT32                               semaWaitHandle;
    gctUINT32                               semaWakeHandle;

    vx_uint32                               id;

    vx_uint32                               numChildren;
    vx_uint32                               childNodes[VX_MAX_NODE_CHILDREN];
    vx_uint32                               numParents;
    vx_uint32                               parentNodes[VX_MAX_NODE_PARENTS];

    vx_bool                                 merged;
    vx_node                                 replacedBy; /*record the nodeId that replace current node*/
    vx_uint32                               numParameters;

    vx_uint32                               nodeID;

    vx_binaryLoad_memory_s                  *binLoadMem; /* define for every node loading graph binary memory */

    vx_bool                                 isTraversal; /* indicate that whether the node has been travesal*/

    vxnne_tp_operation                      mGpuTpOperation; /* multi gpu split tp operations*/
    vx_uint32                               mGpuTpOpCnt;
    vxnne_convolution_relu_pooling_operation mGpuNNOperation; /* multi gpu split nn operations*/
    vx_uint32                               mGpuNNOpCnt;
    vxnne_yuv2rgb_scale_operation           mGpuSCOperation;
    vx_uint32                               mGpuSCOpCnt;

    vx_bool                                 tensorVxcOptimize;
}
vx_node_s;

typedef vx_node *                           vx_node_ptr;

typedef struct vx_graph_parameter_s
{
    vx_node_s *                             node;

    vx_uint32                               index;
}
vx_graph_parameter_s;

typedef enum vx_direction_e                 vx_direction_e;

typedef vx_perf_t *                         vx_perf;


#if gcdVX_OPTIMIZER
typedef struct _vx_node_batch_s
{
    vx_uint32                               startIndex;
    vx_uint32                               endIndex;
    vx_bool                                 cpuInvolved;
    vx_bool                                 endHasCallback;

    vx_bool                                 tiled;
    vx_coordinates2d_t                      tileSize;
    vx_coordinates2d_t                      tileCount;
    vx_coordinates2d_t                      globalSize;
}
vx_node_batch_s;

typedef vx_node_batch_s *                   vx_node_batch;
#endif

typedef struct _vx_graph
{
    vx_reference_s                          base;

    vx_uint32                               nodeCount;
    vx_node                                 nodeTable[VX_MAX_NODE_COUNT];

    vx_perf_t                               perf;

    vx_uint32                               headNodeCount;
    vx_uint32                               headNodeIndexTable[VX_MAX_NODE_COUNT];
    vx_uint32                               allNodeIndexTable[VX_MAX_NODE_COUNT];

    vx_status                               status;

    vx_bool                                 verified;

    vx_bool                                 reverify;

    vx_bool                                 scalerHead;

    vx_bool                                 dirty;

    vx_bool                                 Initilized;

    vx_uint32                               paramCount;
    vx_graph_parameter_s                    paramTable[VX_MAX_PARAMETERS];

    vx_uint32                               headTensorCount;
    vx_graph_parameter_s*                   headTensorCountTable;

    vx_uint32                               inputCount;
    vx_reference                            *inputs;
    vx_uint32                               outputCount;
    vx_reference                            *outputs;

    vx_bool                                 serialize;

    vx_mutex                                scheduleLock;

    vx_value_set_s                          data;

    vx_graph                                parentGraph;

    vx_delay                                delays[VX_MAX_REF_COUNT];
    vx_bool                                 hasAutoAgingDelay;

    vx_bool                                 isChildGraph;

    gctUINT32                               semaNum;
    gctUINT32                               mcfeSema[VX_MAX_MCFE_SEMAPHORE];
#if gcdVX_OPTIMIZER
    vx_bool                                 isSubGraph;
    vx_bool                                 optimized;
    vx_uint32                               optimizedNodeCount;
    vx_node                                 optimizedNodeTable[VX_MAX_NODE_COUNT];

    vx_uint32                               nodeBatchCount;
    vx_node_batch_s                         nodeBatch[VX_MAX_NODE_COUNT];
#endif

    vx_bool                                 hasCPUFunction;
    vx_uint32                              *commandBuffer;
    vx_uint32                               commandBufferSizeInByte;

    vxnne_execution_layer                   layer;

    vx_binary_save                          binarySave;

    vx_uint32                               tailNodeCount;
    vx_uint32                               tailNodeIndexTable[VX_MAX_NODE_COUNT];

    vx_memory_pool                          memoryPool;
    vx_uint32                               virtTensorNum;

    vx_uint32                               deviceID; /*the Device CoreId to run this graph*/
    vx_uint32                               graphID; /*graph ID in this owner context */
    vx_uint32                               peakAxiSramUsedSize;
}
vx_graph_s;

typedef enum vx_dim_e
{
    VX_DIM_CHANNEL = 0,

    VX_DIM_X,

    VX_DIM_Y,

    VX_MAX_DIMS
}
vx_dim_e;

typedef enum vx_bounds_e
{
    VX_BOUND_START = 0,
    VX_BOUND_END,

    VX_MAX_BOUNDS
}
vx_bounds_e;

enum
{
    VXNNE_MEM_POOL_TYPE_ORIG_DDR     = 0x0,
    VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR  = 0x1,
    VXNNE_MEM_POOL_TYPE_VIP_SRAM     = 0x2,
    VXNNE_MEM_POOL_TYPE_AXI_SRAM     = 0x4,
    VXNNE_MEM_POOL_TYPE_END          = 0x5,
    VXNNE_MEM_POOL_TYPE_SRAM         = 0x2 | 0x4,
    VXNNE_MEM_POOL_TYPE_ALL          = 0x7,
};

typedef struct _vx_memory_s
{
    vx_uint32                               planeCount;

    vx_uint32                               dimCount;
    vx_int32                                dims[VX_MAX_PLANES][VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_int32                                strides[VX_MAX_PLANES][VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_size                                 sizes[VX_MAX_PLANES];

    vx_bool                                 allocated;
#define VXNNE_MEM_POOL_TYPE_SET_CACHE(t)       (t | 0x8000)
#define VXNNE_MEM_POOL_TYPE_IS_CACHE(t)        ((t & 0x8000) ? vx_true_e : vx_false_e)
#define VXNNE_MEM_POOL_TYPE_WITHOUT_CACHE(t)   (t & 0x3FFF)
    vx_enum                                 allocType;
    vx_enum                                 allocTypeTmp;
#define  VXNNE_MEM_ALLOC_TYPE_SET_MUST_HAVE(t)     (t | 0x8000)
#define  VXNNE_MEM_ALLOC_TYPE_IS_MUST_HAVE(t)      ((t & 0x8000) ? vx_true_e : vx_false_e)
#define  VXNNE_MEM_ALLOC_TYPE_WITHOUT_MUST_HAVE(t) (t & 0x3FFF)
#define  VXNNE_MEM_ALLOC_OPTIONAL_PRIORITY_1        0
#define  VXNNE_MEM_ALLOC_OPTIONAL_PRIORITY_2        1
#define  VXNNE_MEM_ALLOC_OPTIONAL_PRIORITY_3        2
#define  VXNNE_MEM_ALLOC_PRIORITY_TYPE_KIND         3
#define  VXNNE_MEM_ALLOC_HIGHEST_PRIORITY           VXNNE_MEM_ALLOC_OPTIONAL_PRIORITY_1
#define  VXNNE_MEM_ALLOC_LOWEST_PRIORITY            VXNNE_MEM_ALLOC_OPTIONAL_PRIORITY_3
#define  MEM_PRIORITY_LEFT_LOWER_RIGHT(left, right)  (VXNNE_MEM_ALLOC_TYPE_WITHOUT_MUST_HAVE(left) > VXNNE_MEM_ALLOC_TYPE_WITHOUT_MUST_HAVE(right) ? vx_true_e : vx_false_e)
#define  MEM_PRIORITY_LEFT_HIGHER_RIGHT(left, right)  (VXNNE_MEM_ALLOC_TYPE_WITHOUT_MUST_HAVE(left) < VXNNE_MEM_ALLOC_TYPE_WITHOUT_MUST_HAVE(right) ? vx_true_e : vx_false_e)
#define  MEM_PRIORITY_DOWN_LEVEL(_p) ((_p) == VXNNE_MEM_ALLOC_LOWEST_PRIORITY ? (_p) : (_p+1))
#define  MEM_PRIORITY_UP_LEVEL(_p)   ((_p) == VXNNE_MEM_ALLOC_HIGHEST_PRIORITY ? (_p) : (_p-1))
#define  LOOP_MEM_PRIORITY_HIGH2LOWEST(_p, from) \
             for (_p = from; _p <= VXNNE_MEM_ALLOC_LOWEST_PRIORITY; _p = MEM_PRIORITY_DOWN_LEVEL(_p))
#define  LOOP_MEM_PRIORITY_LOWEST2HIGH(_p, to) \
             for (_p = VXNNE_MEM_ALLOC_LOWEST_PRIORITY; _p >= to; _p = MEM_PRIORITY_UP_LEVEL(_p))
    vx_enum                                 allocPriority;
    vx_bool                                 allocPartial;

#define VXNNE_MEM_ID_INIT_VALUE   0xFFFFFFFF
    vx_uint32                               lastUseId;
    vx_uint32                               firstUseId;
    vx_bool                                 ignoreLastUseId;

    vx_size                                 memOffset;
    vx_bool                                 memReverse;
    vx_bool                                 circular;
    vx_bool                                 isDirty;

    vx_uint32                               offset[VX_MAX_PLANES];

    vx_uint8_ptr                            logicals[VX_MAX_PLANES];
    vx_uint32                               physicals[VX_MAX_PLANES];
    gcsSURF_NODE_PTR                        nodePtrs[VX_MAX_PLANES];

    gctUINT32                               wrappedNode[VX_MAX_PLANES];
    vx_size                                 wrappedSize[VX_MAX_PLANES];
    gctUINT32                               wrapFlag;

    vx_mutex                                writeLocks[VX_MAX_PLANES];

    /* currently only for virtual buffer and virtual tensor memory allocation */
    vx_graph                                graph;

    vx_bool                                 transposed;
    vx_uint8                                transposeChannel;
}
vx_memory_s;

typedef vx_memory_s *                       vx_memory;

typedef struct _vxnne_mem_request_s
{
    vx_uint32   inputCount;
    vx_memory   inputMemory[VX_MAX_MEM_REQUEST_INPUT];
    vx_uint32   outputCount;
    vx_memory   outputMemory[VX_MAX_MEM_REQUEST_OUTPUT];
    vx_memory_s kernelCache;
    vx_memory_s imageCache;
    vx_memory_s tpTransposeBuffer;
    vx_memory_s transposeIn;
    vx_memory_s transposeOut;
}
vxnne_mem_request_s;

typedef struct _vxnne_mem_param_s
{
    vx_uint32   inputCount;
    vx_memory_s inputMemory[VX_MAX_MEM_PARAM_INPUT];
    vx_uint32   outputCount;
    vx_memory_s outputMemory[VX_MAX_MEM_PARAM_OUTPUT];
}
vxnne_mem_param_s;

enum
{
    VX_MEMPOOL_STACK_EMPTY          = 0x0,
    VX_MEMPOOL_STACK_HAS_HEAD       = 0x1,
    VX_MEMPOOL_STACK_HAS_TAIL       = 0x2,
    VX_MEMPOOL_STACK_HAS_HEADTAIL   = 0x3,
};

typedef struct _vx_mempool_stack_item_s
{
    vx_memory   memory;
    vx_size     size;
    vx_size     offset;
}
vx_mempool_stack_item_s, *vx_mempool_stack_item;

typedef struct _vx_mempool_stack_s
{
    vx_mempool_stack_item buffer;
    vx_uint32 count;

    vx_uint32 first;
    vx_uint32 last;
}
vx_mempool_stack_s, *vx_mempool_stack;

typedef struct _vx_memory_pool_item_s
{
    vx_size                                 offset;
    vx_size                                 size;
    vx_uint32                               index;
    vx_bool                                 allocated;
    struct _vx_memory_pool_item_s *         prev;
    struct _vx_memory_pool_item_s *         next;
}
vx_memory_pool_item_s;

typedef vx_memory_pool_item_s *             vx_memory_pool_item;

#define VX_MEMPOOL_RESERVE_MEM_SIZE  1024
#define VX_MAX_MEMPOOL_ITEM_NUM      512
#define VX_MEMPOOL_ITEM_INVALID      (VX_MAX_MEMPOOL_ITEM_NUM + 1)


typedef struct _vx_memory_pool_s
{
    vx_size                                 size;
    vx_uint32                               count;
    vx_memory_pool_item_s                   pool[VX_MAX_MEMPOOL_ITEM_NUM];
    vx_bool                                 memExpandMode;
    vx_bool                                 locked;

    vx_uint8_ptr                            logical;
    vx_uint32                               physical;
    gcsSURF_NODE_PTR                        nodePtr;

    vx_memory_pool_item                     allocHead;
    vx_memory_pool_item                     freeHead;
}
vx_memory_pool_s;

typedef struct _vx_image
{
    vx_reference_s                          base;

    vx_memory_s                             memory;

    vx_uint32                               width;
    vx_uint32                               height;

    /*arraySize,sliceSize is for imageArray / image3D */
    gctUINT32                               arraySize;
    gctUINT32                               sliceSize;

    vx_df_image                             format;

    vx_uint32                               planeCount;

    vx_enum                                 colorSpace;

    vx_enum                                 channelRange;

    vx_uint32                               scales[VX_MAX_PLANES][VX_MAX_DIMS];

    vx_uint32                               bounds[VX_MAX_PLANES][VX_MAX_DIMS][VX_MAX_BOUNDS];

    vx_image                                parent;

    vx_bool                                 isUniform;

    vx_rectangle_t                          region;

    vx_enum                                 importType;

    vx_image                                subimages[VX_MAX_REF_COUNT];

    vx_bool                                 useInternalMem;
}
vx_image_s;

typedef vx_image *                          vx_image_ptr;

typedef struct _vx_tensor_addressing_t
{
    vx_reference_s                          base;

    vx_uint32                               dimCount;
    vx_uint32                               dimSizesUser[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32                               dimStridesUser[VX_CONTEXT_TENSOR_MAX_DIMENSION];
}
vx_tensor_addressing_s;

typedef struct _vx_view_region_s
{
    vx_uint32                               dimCount;
    vx_uint32                               viewStarts[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32                               viewEnds[VX_CONTEXT_TENSOR_MAX_DIMENSION];
}
vx_view_region_s;

typedef struct _vx_tensor_view_t
{
    vx_reference_s                          base;

    vx_view_region_s                        viewRegion;
}
vx_tensor_view_s;

typedef struct _vx_tensor_buffer_t
{
    vx_enum                                 dataFormat;
    vx_uint32                               elementSize;
    vx_enum                                 roundingMode;

    vx_memory_s                             memory;

    vx_int32                                bufRefCount;

    vx_int32                                padZeorValue;

    vx_bool                                 valued;
    vx_enum                                 precision;
    vx_enum                                 data_lifetime;

    vx_uint32                               refNum;
}
vx_tensor_buffer_s;

typedef struct _vx_tensor_t
{
    vx_reference_s                          base;

    vx_view_region_s                        viewRegion; /* default is whole size */
    vx_bool                                 isViewed;
    vx_bool                                 isReshaped;
    vx_uint32                               viewOffset;

    vx_tensor_buffer_s *                    tensorBuffer; /* shared by all related tensors */

    /* they are same as those in memory structure in most time unless reshaped */
    vx_uint32                               dimCount;
    vx_uint32                               dims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32                               strides[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32                               baseAddressOffset;

    vx_uint32                               memorySize;

    vx_bool                                 alloced;
    vx_bool                                 brickMode;
    vx_tensor_quant_param                   quantData;
    vx_enum                                 dataFormat;
    vx_uint32                               elementSize;

    vx_enum                                 quantFormat;

    vx_enum                                 rank;
    vx_tensor                               reshape;
    vx_bool                                 useInternalMem;

}
vx_tensor_s;

typedef enum _vx_tp_cmd_type_e
{
    TP_NONE = 0,
    TP_RESHUFFLE,
    TP_SINGLE_FC,
    TP_MAX_POOLING,
    TP_ACTIVATION,
    TP_LRN,
    TP_TRANSPOSE,
    TP_ROI_POOLING,
    TP_REORG,
    TP_REORG_DEPTH2SPACE,
    TP_REORG_SPACE2DEPTH,
    TP_REORG_SPACE2BATCH,
    TP_REORG_BATCH2SPACE,
    TP_REORG_SHUFFLECHANNEL,
    TP_ADD,
    TP_REVERSE,
    TP_UPSAMPLE,
    TP_DILATE_UPSAMPLE,
    TP_DILATE_UPSAMPLE2,
    TP_DILATE_RESHUFFLE,
    TP_BRICK,
    TP_RNN_INTERLEAVE,
    TP_TENSOR_COPY,
    TP_TENSOR_PAD,
    TP_LSTM_RESHUFFLE_INPUT,
    TP_LSTM_STATE_OUT,
    TP_LSTM_RESHUFFLE_STATE_O,
    TP_CMD_NUM,
    TP_ROI_POOLING_STEP_1,
    TP_ROI_POOLING_STEP_2,
    TP_UPSAMPLE_CLIP,
    TP_TENSOR_SQUEEZE,
    TP_TENSOR_PAD_CN,
    TP_TENSOR_COPY4CONCAT,
    TP_TENSOR_STRIDED_SLICE,
    TP_TENSOR_SVDF_MAP,

    TP_TENSOR_COUNT,
}
vx_tp_cmd_type_e;

typedef enum _vx_nn_feature_e
{
    VX_NN_FEATURE_TP,
    VX_NN_FEATURE_MULTI_TP,
    VX_NN_FEATURE_TP_RESHUFFLE,
    VX_NN_FEATURE_TP_SINGLE_FC,
    VX_NN_FEATURE_TP_MAX_POOLING,
    VX_NN_FEATURE_TP_ACTIVATION,
    VX_NN_FEATURE_TP_LRN,
    VX_NN_FEATURE_TP_TRANSPOSE,
    VX_NN_FEATURE_TP_ROI_POOLING,
    VX_NN_FEATURE_TP_BRICK_MODE,
    VX_NN_FEATURE_TP_REORG,
    VX_NN_FEATURE_TP_ADD,
    VX_NN_FEATURE_TP_REVERSE,
    VX_NN_FEATURE_TP_UPSAMPLE,
    VX_NN_FEATURE_TP_REORDER,
    VX_NN_FEATURE_TP_RTNE,
    VX_NN_FEATURE_BRICK_MODE,
    VX_NN_FEATURE_INTERLEVE8,
    VX_NN_FEATURE_BORDER_MODE,
    VX_NN_FEATURE_SRAM,
    VX_NN_FEATURE_ZDP3,
    VX_NN_FEATURE_ZDP6,
    VX_NN_FEATURE_XYDP9,
    VX_NN_FEATURE_XYDP6,
    VX_NN_FEATURE_XYDP0,
    VX_NN_FEATURE_SWTILING_PHASE1,
    VX_NN_FEATURE_SWTILING_PHASE2,
    VX_NN_FEATURE_SWTILING_PHASE3,
    VX_NN_FEATURE_TF_QUANT,
    VX_NN_FEATURE_FIRST_PIXEL_POOLING,
    VX_NN_FEATURE_NN_STRIDE_SUPPORT,
    VX_NN_FEATURE_COEF_COMPRESSION_ENHANCEMENT,
    VX_NN_FEATURE_TP_COMPRESSION_ENHANCEMENT,
    VX_NN_FEATURE_SCALER,
    VX_NN_FEATURE_SCALER_4K,
    VX_NN_FEATURE_NN_DEPTHWISE_SUPPORT,
    VX_NN_FEATURE_VIP_DEC400,
    VX_NN_FEATURE_SHADER,
    VX_NN_TP_PARALLEL,
    VX_TP_FEATURE_FP32_BIAS,
    VX_NN_FEATURE_NN_TRANSPOSE,
    VX_NN_FEATURE_FAST_FIRST_PIXEL_POOLING,
    VX_NN_FEATURE_COUNT,
}
vx_nn_feature_e;


typedef enum _vx_nn_calculate_type_e
{
    VX_DIM_WIDTH = 0x1,
    VX_DIM_HEIGHT = 0x2,
    VX_DIM_IN_TO_OUT = 0x4,
    VX_DIM_OUT_TO_IN = 0x8,
}
vx_nn_calculate_type_e;


#define MAX_WEIGHT_BIAS_GROUPS  4
#define MAX_ZGROUP_COUNT     128
#define MAX_KZGROUP_COUNT  16

#define SW_TILING_FROM_DDR          0
#define SW_TILING_FROM_AXI_SRAM     1
#define SW_TILING_FROM_VIP_SRAM     2
#define SW_TILING_PERM_AXI_SRAM     3
#define SW_TILING_PERM_VIP_SRAM     4

enum
{
    VX_SWTILING_OPTION_OFF    = 0,
    VX_SWTILING_OPTION_ALL    = 1,
    VX_SWTILING_OPTION_AB     = 2,
    VX_SWTILING_OPTION_TILING = 3,
};

typedef struct _vx_array
{
    vx_reference_s                          base;

    vx_memory_s                             memory;

    vx_enum                                 itemType;

    vx_size                                 itemSize;

    vx_size                                 itemCount;

    vx_uint32                               offset;

    vx_size                                 capacity;

    vx_mem_alloc_info_s                     memAllocInfo;

    vx_bool                                 isVivArray;
}
vx_array_s;

typedef struct _vx_delay_parameter_s
{
    vx_node                                 node;

    vx_uint32                               index;

    struct _vx_delay_parameter_s *          next;
}
vx_delay_parameter_s;

typedef vx_delay_parameter_s *              vx_delay_parameter;

typedef struct _vx_object_array {
    vx_reference_s                          base;

    vx_reference                            itemsTable[VX_MAX_REF_COUNT];
    vx_size                                 itemCount;

    vx_enum                                 itemType;
} vx_object_array_s;

typedef struct _vx_delay
{
    vx_reference_s                          base;

    vx_enum                                 type;

    vx_size                                 count;
    vx_uint32                               index;

    vx_delay_parameter                      paramListTable;

    vx_reference_ptr                        refTable;

    vx_delay                                *pyramidTable;
}
vx_delay_s;

typedef vx_array_s                          vx_lut_s;

typedef struct _vx_remap
{
    vx_reference_s                          base;

    vx_memory_s                             memory;

    vx_uint32                               srcWidth;
    vx_uint32                               srcHeight;

    vx_uint32                               destWidth;
    vx_uint32                               destHeight;

    vx_mem_alloc_info_s                     memAllocInfo;
}
vx_remap_s;

typedef struct _vx_distribution
{
    vx_reference_s                          base;

    vx_memory_s                             memory;

    vx_uint32                               windowX;
    vx_uint32                               windowY;

    vx_int32                                offsetX;
    vx_int32                                offsetY;

    vx_uint32                               rangeX;
    vx_uint32                               rangeY;

    vx_mem_alloc_info_s                     memAllocInfo;
}
vx_distribution_s;

#define VX_DEFAULT_THRESHOLD_FALSE_VALUE 0
#define VX_DEFAULT_THRESHOLD_TRUE_VALUE  255

#define VX_S16_THRESHOLD_FALSE_VALUE 0
#define VX_S16_THRESHOLD_TRUE_VALUE  (-1)
#define VX_U16_THRESHOLD_FALSE_VALUE 0
#define VX_U16_THRESHOLD_TRUE_VALUE  0xFFFF
#define VX_S32_THRESHOLD_FALSE_VALUE 0
#define VX_S32_THRESHOLD_TRUE_VALUE  (-1)
#define VX_U32_THRESHOLD_FALSE_VALUE 0
#define VX_U32_THRESHOLD_TRUE_VALUE  0xFFFFFFFF

typedef struct _vx_threshold
{
    vx_reference_s                          base;

    vx_enum                                 thresholdType;

    vx_enum                                 dataType;

    vx_pixel_value_t                        value;

    vx_pixel_value_t                        lower;
    vx_pixel_value_t                        upper;

    vx_pixel_value_t                        trueValue;
    vx_pixel_value_t                        falseValue;
    vx_df_image                             input_format;
    vx_df_image                             output_format;

    vx_mem_alloc_info_s                     memAllocInfo;
}
vx_threshold_s;

typedef struct _vx_matrix
{
    vx_reference_s                          base;

    vx_memory_s                             memory;

    vx_enum                                 dataType;

    vx_size                                 columns;
    vx_size                                 rows;

    vx_mem_alloc_info_s                     memAllocInfo;

    vx_coordinates2d_t                      origin;
    vx_enum                                 pattern;

}
vx_matrix_s;

typedef struct _vx_convolution
{
    vx_matrix_s                             matrix;

    vx_uint32                               scale;

    vx_mem_alloc_info_s                     memAllocInfo;
}
vx_convolution_s;

typedef struct _vx_pyramid
{
    vx_reference_s                          base;

    vx_size                                 levelCount;
    vx_image *                              levels;

    vx_float32                              scale;

    vx_uint32                               width;
    vx_uint32                               height;
    vx_df_image                             format;

    vx_mem_alloc_info_s                     memAllocInfo;
}
vx_pyramid_s;

typedef vx_pyramid_s *                      vx_pyramid_ptr;

typedef struct _vx_meta_format
{
    vx_reference_s                          base;

    vx_size                                 size;
    vx_enum                                 type;

    union
    {
        struct
        {
            vx_uint32                       width;
            vx_uint32                       height;
            vx_df_image                     format;

            vx_delta_rectangle_t            delta;
        }
        imageInfo;

        struct
        {
            vx_uint32                       width;
            vx_uint32                       height;
            vx_df_image                     format;

            vx_size                         levelCount;

            vx_float32                      scale;
        }
        pyramidInfo;

        struct
        {
            vx_enum                         type;
        }
        scalarInfo;

        struct
        {
            vx_enum                         itemType;
            vx_size                         capacity;
        }
        arrayInfo;

        struct matrix {
            vx_enum                         type;
            vx_size                         rows;
            vx_size                         cols;
        } matrixInfo;

        struct distribution {
            vx_size                         bins;
            vx_int32                        offset;
            vx_uint32                       range;
        } distributionInfo;

        struct remap {
            vx_uint32                       src_width;
            vx_uint32                       src_height;
            vx_uint32                       dst_width;
            vx_uint32                       dst_height;
        } remapInfo;

        struct lut {
            vx_enum                         type;
            vx_size                         count;
        } lutInfo;

        struct threshold {
            vx_enum                         type;
            vx_df_image                     input_format;
        } thresholdInfo;

        struct object_array {
            vx_enum item_type;
            vx_size item_count;
        } objectArrayInfo;

        struct tensor {
            vx_enum                         dataFormat;
            vx_int8                         fixedPointPosition;
            vx_uint32                       numOfDims;
            vx_uint32                       dimensions[VX_CONTEXT_TENSOR_MAX_DIMENSION];
        }tensorInfo;
    }
    u;

    vx_kernel_image_valid_rectangle_f setValidRectangleCallback;
}
vx_meta_format_s;

typedef vx_meta_format *                    vx_meta_format_ptr;

typedef struct _vx_keypoint_t_optpyrlk_internal
{
    vx_float32                              x;
    vx_float32                              y;

    vx_float32                              strength;
    vx_float32                              scale;
    vx_float32                              orientation;

    vx_int32                                tracking_status;
    vx_float32                              error;
}
vx_keypoint_t_optpyrlk_internal;

typedef struct _vx_param_description_s
{
    vx_enum                                 direction;

    vx_enum                                 dataType;

    vx_enum                                 state;

    vx_bool                                 isStatic;
}
vx_param_description_s;

typedef struct _vx_kernel_description_s
{
    vx_enum                                 enumeration;

    vx_char                                 name[VX_MAX_KERNEL_NAME];

    vx_kernel_f                             function;

    vx_param_description_s *                parameters;
    vx_uint32                               numParams;

    vx_kernel_validate_f                    validate;

    vx_kernel_input_validate_f              inputValidate;
    vx_kernel_output_validate_f             outputValidate;
    vx_kernel_initialize_f                  initialize;
    vx_kernel_deinitialize_f                deinitialize;

#if gcdVX_OPTIMIZER
    vx_kernel_optimization_attribute_s      optAttributes;
#endif

    struct{
        vx_char*                            source;
    }extension;
}
vx_kernel_description_s;

typedef struct _vx_object_data
{
    vx_enum                                 objType;

    union
    {
        struct
        {
            vx_uint32                       width;
            vx_uint32                       height;
            vx_df_image                     format;
        }
        imageInfo;

        struct
        {
            vx_enum                         dataType;
            void *                          scalarValuePtr;
        }
        scalarInfo;

        struct
        {
            vx_enum                         dataType;
        }
        lutArrayInfo;

        struct
        {
            vx_size                         numBins;
            vx_int32                        offset;
            vx_uint32                       range;
        }
        distributionInfo;

        struct
        {
            vx_enum                         dataType;
        }
        thresholdInfo;

        struct
        {
            vx_size                         rows;
            vx_size                         columns;
        }
        convolutionInfo;

        struct
        {
            vx_enum                         dataType;
            vx_size                         rows;
            vx_size                         columns;
        }
        matrixInfo;

        struct
        {
            vx_uint32                       srcWidth;
            vx_uint32                       srcHeight;
            vx_uint32                       dstWidth;
            vx_uint32                       dstHeight;
        }
        remapInfo;

        struct
        {
            vx_size                         numLevels;
            vx_float32                      scale;
            vx_uint32                       width;
            vx_uint32                       height;
            vx_df_image                     format;
        }
        pyramidInfo;

        struct
        {
            vx_enum                         dataType;
            vx_size                         capacity;
        }
        arrayInfo;

        struct
        {
            vx_enum                         dataType;
        }
        objArrayInfo;
    }
    u;

    vx_bool isVirtual;
}
vx_object_data_s;

typedef enum _vx_nn_round_mode_e
{
    VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING = 0,
    VX_NN_ROUNDING_MODE_RTNE,
    VX_NN_ROUNDING_MODE_RTZ,
    VX_NN_ROUNDING_MODE_RTNI,
}
vx_nn_round_mode_e;

VX_INTERNAL_API gctUINT64 gcfVX_PerfStart(vx_reference ref);
VX_INTERNAL_API vx_uint64 gcfVX_PerfEnd(vx_reference ref, gctUINT64 start);
VX_INTERNAL_API vx_int32 vxoContext_GetUserStructIndex(vx_context context, vx_enum dataType);
enum {
    VX_DEBUG_LEVEL_NONE = 0,
    VX_DEBUG_LEVEL_INFO = 1,
    /*VX_DEBUG_LEVEL_ERROR = 2,*/
};

void vxPRINT(vx_uint32 level, const char *msg, ...);

typedef enum viv_nn_pooling_type_e
{
    VIV_NN_POOLING_NON         = 0,
    VIV_NN_POOLING_MAX         = 1,
    VIV_NN_POOLING_AVG         = 2,
    VIV_NN_POOLING_FIRST_PIXEL = 3
}viv_nn_pooling_type_e;
vx_status vxnneOperation_TP_Deinitialize(vxnne_operation_s *operation);

vx_status vxnneOperation_AddReference(
    vxnne_operation_s*            operation,
    vx_reference                  reference,
    vxnne_operation_reference_e   refType
    );

vx_status vxnneGetTensorMemeory(vx_tensor tensor, vx_ptr_ptr ptr, vx_bool stage, vx_bool zero);

vx_bool vxoElementOptimization_GetTensorShape(vx_tensor input, vx_uint32 sizes[VX_CONTEXT_TENSOR_MAX_DIMENSION], vx_uint32 * num_of_dims);

vx_status vxnneOperation_Deinitialize(vxnne_operation_s *operation);

#if REGISTER_FRAME

typedef enum _vx_nn_support_type_e
{
    VX_NN_QUERY_NN,
    VX_NN_QUERY_TP,
    VX_NN_QUERY_SHADER,

    VX_NN_QUERY_COUNT
}
vx_nn_support_type_e;

vx_bool vxoLayer_CheckSupport(vx_context context, vx_enum type, vx_enum format, vx_uint32_ptr flag);

typedef struct _vxnne_register_param_s
{
    vx_uint32 flag;
    vx_int32 index;
    vx_bool support;
    vx_uint32_ptr ptr;
}
vxnne_register_param_s, *vxnne_register_param;

typedef vx_status (*vxnne_layer_ops_verification_f)(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param);
typedef vx_status (*vxnne_layer_ops_initialize_f)(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param);
typedef vx_status(*vxnne_layer_ops_get_ops_f)(vxnne_layer ops_layer, vx_uint32_ptr max_num_operations, vxnne_operation **operations);

typedef struct _vxnne_layer_imp_s
{
    vx_char                         description[32];
    vxnne_layer_ops_verification_f  verification;
    vxnne_layer_ops_initialize_f    initialize;
    vxnne_layer_deinitialize_f      deinitialize;
}
vxnne_layer_imp_s, *vxnne_layer_imp;

typedef struct _vxnne_layer_ops_s
{
    vxnne_layer_imp imps;
    vx_uint32       imp_count;
    vx_enum         imp_type;
    vxnne_layer_ops_get_ops_f get_operations;
}
vxnne_layer_ops_s, *vxnne_layer_ops;

vx_status vxnneLayer_Ops_Initialize(
    vx_node                     node,
    vxnne_layer_ops             ops,
    vx_char*                    name,
    const vx_uint32             size_of_layer,
    const vx_reference          parameters[],
    vx_uint32                   num
    );

vx_status vxoNNLayer_NotSupport_Initializer(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param);

vx_bool vxoNNCommon_NotSupport(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param);

vx_bool vxoNNCommon_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param);

vx_status vxoNNCommon_Deinitialize(vxnne_layer layer);

vx_status vxoLayer_VerificationHead(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param);
vx_status vxoLayer_VerificationFoot(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool* support);

vx_status vxoLayer_InitializeHead(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param);
vx_status vxoLayer_InitializeFoot(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param);

#if REGISTER_FRAME_CL_RELAX
#define REGISTER_ADDON_DEBUG(list) \
    { \
        vxnne_layer_imp_s _register; \
        gcoOS_MemCopy(&_register, &list[2], sizeof(vxnne_layer_imp_s)); \
        gcoOS_MemCopy(&list[2], &list[3], sizeof(vxnne_layer_imp_s)); \
        gcoOS_MemCopy(&list[3], &_register, sizeof(vxnne_layer_imp_s)); \
    }

#else
#define REGISTER_ADDON_DEBUG(list)
#endif

#define REGISTER_LAYERS(list, layer_type, name, callback) \
    { \
        vxnne_layer_ops_s ops = { VX_NULL, gcmCOUNTOF(list) };   \
        REGISTER_ADDON_DEBUG(registerSoftmax2s) \
        ops.imps = list;  \
        ops.get_operations = callback;  \
        vxmONERROR(vxnneLayer_Ops_Initialize(node, &ops, name, sizeof(layer_type), parameters, num)); \
    }

#define SETBIT(target, value, offset) (target) |= ((value) << (offset))
#define GETBIT(target, offset) (((target) & (1 << (offset))) >> (offset))

#endif



#include <gc_vx_context.h>
#include <gc_vx_delay.h>
#include <gc_vx_graph.h>
#include <gc_vx_json.h>
#include <gc_vx_graph_optimization.h>
#include <gc_vx_image.h>
#include <gc_vx_kernel.h>
#include <gc_vx_node.h>
#include <gc_vx_runtime.h>
#include <gc_vx_parameter.h>
#include <gc_vx_reference.h>
#include <gc_vx_scalar.h>
#include <gc_vx_target.h>
#include <gc_vx_memory.h>
#include <gc_vx_convolution.h>
#include <gc_vx_distribution.h>
#include <gc_vx_lut.h>
#include <gc_vx_matrix.h>
#include <gc_vx_pyramid.h>
#include <gc_vx_threshold.h>
#include <gc_vx_remap.h>
#include <gc_vx_array.h>
#include <gc_vx_error.h>
#include <gc_vx_meta_format.h>
#include <gc_vx_object_array.h>
#include <gc_vx_tensor.h>


#include <gc_vx_program.h>
EXTERN_C_BEGIN

#include <gc_vx_inlines.c>

EXTERN_C_END

#include <string.h>

#define INITIALIZE_STRUCT(values) {\
    memset((void*)&(values), 0, sizeof((values)));\
}
#endif /* __GC_VX_COMMON_H__ */

