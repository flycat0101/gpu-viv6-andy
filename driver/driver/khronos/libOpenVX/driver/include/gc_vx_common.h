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


#ifndef __GC_VX_COMMON_H__
#define __GC_VX_COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <time.h>

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

#include <VX/vx_ext_program.h>
#include <VX/vx_viv_cnn.h>
#include <VX/vx_khr_cnn.h>

#include "gc_vxk_common.h"

#include <gc_vx_profiler.h>
#include <gc_vx_layer.h>
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
#define VX_C_MEMORY_MANAGE  1

#define VX_SHADER_TP        1

#define VX_NN_SH_PARALLEL   1

#define VX_NN_FC_ACCEL      0

#define NN_LAYER_C          0

#define ENABLE_SPLIT_WB     1


#define TP_FC_Z_MAX         512

#define NN_INTEGER_BIAS_BITS                27
#define NN_INTEGER_BIAS_BITS_VIP_V7         32
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
#define VX_MAX_REF_COUNT                    4096
#define VX_MAX_NODE_COUNT                   1024
#define VX_MAX_MODULE_COUNT                 16
#define VX_MAX_TARGET_COUNT                 1
#define VX_MAX_KERNEL_COUNT                 1024
#define VX_MAX_USER_STRUCT_COUNT            1024
#define VX_MAX_CONVOLUTION_DIM              15
#define VX_INT_MAX_NONLINEAR_DIM            9
#define VX_MAX_OPTICAL_FLOW_WINDOW_DIM      9
#define VX_MAX_PARAMETERS                   20
#define VX_MAX_NODES_IN_GRAPH               VX_MAX_NODE_COUNT
#define VX_MAX_PLANES                       4

#define VX_HOST_CORE_COUNT                  1
#define VX_MAX_DEVICES                      4
#define VX_MAX_PRINTF_BUFFER_SIZE           (1024*1024)

#define MAP_UNMAP_REFERENCE                 0

/* Function macros */
#ifndef vxmLENGTH_OF
#define vxmLENGTH_OF(array)                 (sizeof(array) / sizeof((array)[0]))
#endif

#define vxmASSERT                           gcmASSERT

#define vxWarning(...) gcmTRACE(gcvLEVEL_WARNING, __VA_ARGS__)

#define vxError(...) gcmTRACE(gcvLEVEL_ERROR, __VA_ARGS__)


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

    VX_KERNEL_INTERNAL_NONMAXSUPPRESSION           = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x5,

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

    VX_KERNEL_INTERNAL_CNN_SOFTMAX                 = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x22,

    VX_KERNEL_INTERNAL_CNN_INTERLEAVE_BUFFERS      = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x23,

    VX_KERNEL_INTERNAL_CNN_LAYER                   = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x24,

    VX_KERNEL_INTERNAL_CNN_DATACONVERT             = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x25,

    VX_KERNEL_INTERNAL_CNN_RESHUFFLE_IMAGE         = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x26,

    VX_KERNEL_INTERNAL_FASTERRCNN_SOFTMAX          = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x27,

    VX_KERNEL_INTERNAL_MAX_POOL3x3                 = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x28,
    VX_KERNEL_INTERNAL_LRN                         = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x29,
    VX_KERNEL_INTERNAL_ROI_POOLING                 = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x2A,

    VX_KERNEL_INTERNAL_POOLING                     = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x2B,

    VX_KERNEL_INTERNAL_FASTERRCNN_RESHUFFLE_DATA   = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x2C,

    VX_KERNEL_INTERNAL_RPN                         = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x2D,

    VX_KERNEL_INTERNAL_CONVERT                     = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x2E,

    VX_KERNEL_INTERNAL_FASTERRCNN_WAIT             = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x2F,

    VX_KERNEL_INTERNAL_SOBEL_MxN_F16               = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x30,

    VX_KERNEL_INTERNAL_ELEMENTWISE_NORM_F16        = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x31,

    VX_KERNEL_INTERNAL_PHASE_F16                   = VX_KERNEL_BASE(VX_ID_VIVANTE, VX_LIBRARY_KHR_INTERNAL) + 0x32,
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

enum vx_kernel_invalid_e
{
    VX_KERNEL_INVALID = VX_KERNEL_BASE(VX_ID_KHRONOS, VX_LIBRARY_KHR_BASE)
};

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

typedef enum _vx_cnn_net_name_e
{
    CNN_INVALID_NET,
    CNN_ALEX_NET,
    CNN_DAHUA_NET,
    CNN_FASTCNN_NET,
    CNN_VGG_NET,
    CNN_NIN_NET,
    CNN_TOTAL_NETS,
}
vx_cnn_net_name_e;

typedef struct _vx_cnn_kernelStreamInfo
{
    /* parameters from input */
    const vx_float32 *orgWeightData;
    const vx_float32 *orgBiasData;
    vx_uint32 orgWeightWidth;
    vx_uint32 orgWeightHeight;
    vx_uint32 orgWeightDepth;
    vx_uint32 alignedWeightWidth;
    vx_uint32 alignedWeightHeight;
    vx_uint32 orgInImageXSize;
    vx_uint32 orgInImageYSize;
    vx_uint32 orgInImageZSize;
    vx_uint32 alignedInImageXSize;
    vx_uint32 alignedInImageYSize;

    /* parameters for kernel stream */
    void *weightData;
    void *biasData;
    vx_uint32 repeats;
    vx_uint32 filterTotalCount;
    vx_uint32 filtersPerCore;      /* filter count every group */
    vx_uint32 sliceCount;       /* slice count every filter */
    vx_uint32 weightWidth;
    vx_uint32 weightHeight;
    vx_uint32 inImageXSize;
    vx_uint32 inImageYSize;
    vx_int32  inImageXOffset;
    vx_int32  inImageYOffset;
    vx_uint32 outImageXSize;
    vx_uint32 outImageYSize;
    vx_uint32 outImageZSize;
    vx_uint32 outImageTileXSize;
    vx_uint32 outImageTileYSize;
    vx_uint32 outFinalImageXSize; /*if no pooling, outImageXSize = outFinalImageXSize*/
    vx_uint32 outFinalImageYSize;
    vx_uint32 strideX;
    vx_uint32 strideY;
    vx_uint32 outItemCount;
    vx_uint32 roundingMode;
    viv_nn_non_linear_func_e nonlinearFunc;
    vx_uint32 inImageBitWidth;
    vx_uint32 outImageBitWidth;
    vx_uint32 weightBitWidth;
    vx_int32  inImageFractionLength;
    vx_int32  outImageFractionLength;
    vx_int32  weightFractionLength;
    vx_uint32 postMultiplier;
    vx_uint32 postShift;
    vx_uint8  kernelDataType;
    vx_uint8  inImageDataType;
    vx_uint8  outImageDataType;
#if ENABLE_LRN_LAYER
    viv_nn_norm_type_e      lrnType;
    unsigned int            lrnKernelSize;
    float                   lrnAlpha;
    float                   lrnBeta;
#endif
    viv_nn_pooling_type_e   poolingType;
    unsigned int            poolingSize;
    unsigned int            poolingStride;
    vx_uint8 zeroRunLen;
    vx_uint8 zeroRunLen2;
    unsigned int layerType;
#if VX_NN_FC_ACCEL
    void *tranposedWeightBiasData;
#endif
}
vx_cnn_kernelStreamInfo_s;

typedef struct _vx_cnn_networkDataInfo
{
    vx_uint8 networkTypeValue;
    vx_type_e weightType;
    vx_size  weightItemSize;
    vx_uint32 weightBitSize;
    vx_type_e biasType;
    vx_size biasItemSize;
    vx_size  biasBitSize;
}
vx_cnn_networkDataInfo_s;

typedef struct _vx_cnn_attributes_s
{
    vx_array                                kernelBuffer[20][2];
    vx_array                                cmmdBuffer[20];
    vx_cnn_kernelStreamInfo_s               cnnkernelStreamInfo[20];
    vx_uint32                               layerCount;
    vx_uint32                               batchSizeValue;
    vx_cnn_networkDataInfo_s                networkDataInfo;
    vx_uint32                               batchLevelIndex;
    vx_cnn_net_name_e                       netFlag;
    unsigned int                            layerType[20];
}
vx_cnn_attributes_s;

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
    vx_uint32 aluZFilterStartOverfetch;
    vx_uint32 aluZFilterEndOverfetch;
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

    vx_kernel_attribute_s                   attributes;

    vx_cnn_attributes_s                     cnnAttributes;

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

typedef struct _vx_target_funcs_s
{
    vx_target_initialize_f                  initialize;
    vx_target_deinitialize_f                deinitialize;

    vx_target_iskernelsupported_f           iskernelsupported;
    vx_target_addkernel_f                   addkernel;
    vx_target_addtilingkernel_f             addtilingkernel;

    vx_target_verifynode_f                  verifynode;
    vx_target_processnodes_f                processnodes;
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

typedef struct _vx_context
{
    vx_reference_s                          base;

    vx_uint32                               refCount;
    vx_reference                            refTable[VX_MAX_REF_COUNT];

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

    vx_border_t                                immediateBorderMode;
    vx_enum                                 immediateBorderModePolicy;

    vx_enum                                 immediateTargetEnum;

    vx_char                                 immediateTargetString[VX_MAX_TARGET_NAME];

    vx_evis_no_inst_s                       evisNoInst;

    /* Profiler */
#if VIVANTE_PROFILER
    vx_profiler_s                           profiler;
    gcoPROFILER                             halProfile;
#endif

    gctPOINTER                              devices[VX_MAX_DEVICES];
    vx_uint32                               deviceCount;

    vx_uint32                               memoryCount;

    vx_nn_config                            nnConfig;

    vx_uint32                               cnnAvailableEventID;

    vxnne_kernel_shaders_s                  kernels[VXNNE_KERNEL_COUNT];

    /* gcCompileKernel */
    gctCLCompiler                           compileKernel;
    gctHANDLE                               libCLC;
    gceSTATUS                               (*loadCompiler)(IN gcsHWCaps *HWCaps, IN gcePATCH_ID PatchId);
    gceSTATUS                               (*unloadCompiler)(void);

    vx_drv_option                           options;

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

    vx_bool                                    isReplicated;

    vx_bool                                    replicated_flags[VX_MAX_PARAMETERS];

    vx_bool                                 localDataChangeIsEnabled;

    vx_bool                                 localDataSetByImplementation;
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

    vx_bool                                 dirty;

    vx_bool                                 Initilized;

    vx_uint32                               paramCount;
    vx_graph_parameter_s                    paramTable[VX_MAX_PARAMETERS];

    vx_bool                                 serialize;

    vx_mutex                                scheduleLock;

    vx_value_set_s                          data;

    vx_graph                                parentGraph;

    vx_delay                                delays[VX_MAX_REF_COUNT];

    vx_bool                                 isChildGraph;

    /* only for AlexNet node*/
    vx_bool                                 isNNKernelBufReady;
    vx_array                                nnInputKernelBuffer[8][2];
    vx_array                                nnCmds;

    /* Pre-allocated staging buffers for CPU nodes. Used by RCNN*/
    vx_array                                wStageA[2];
    vx_array                                wStageB[2];
    vx_array                                wStageC[2];
    vx_array                                wStageD[2];

#if gcdVX_OPTIMIZER
    vx_bool                                 isSubGraph;
    vx_bool                                 optimized;
    vx_uint32                               optimizedNodeCount;
    vx_node                                 optimizedNodeTable[VX_MAX_NODE_COUNT];

    vx_uint32                               nodeBatchCount;
    vx_node_batch_s                         nodeBatch[VX_MAX_NODE_COUNT];
#endif

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

#define VX_CONTEXT_TENSOR_MAX_DIMENSION     6

typedef enum vx_bounds_e
{
    VX_BOUND_START = 0,
    VX_BOUND_END,

    VX_MAX_BOUNDS
}
vx_bounds_e;

typedef struct _vx_memory_s
{
    vx_uint32                               planeCount;

    vx_uint32                               dimCount;
    vx_int32                                dims[VX_MAX_PLANES][VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_int32                                strides[VX_MAX_PLANES][VX_CONTEXT_TENSOR_MAX_DIMENSION];

    vx_bool                                 allocated;

    vx_uint32                                offset[VX_MAX_PLANES];

    vx_uint8_ptr                            logicals[VX_MAX_PLANES];
    vx_uint32                               physicals[VX_MAX_PLANES];
    gcsSURF_NODE_PTR                        nodePtrs[VX_MAX_PLANES];

    gctUINT32                               wrappedNode[VX_MAX_PLANES];
    gctUINT32                               wrappedSize[VX_MAX_PLANES];
    gctUINT32                               wrapFlag;

    vx_mutex                                writeLocks[VX_MAX_PLANES];
}
vx_memory_s;

typedef vx_memory_s *                       vx_memory;

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
    vx_int8                                 fixedPointPos;
    vx_enum                                 roundingMode;
    vx_uint32                               elementSize;

    vx_memory_s                             memory;

    vx_int32                                bufRefCount;
    vx_int32                                memRefCount;
}
vx_tensor_buffer_s;

typedef struct _vx_tensor_t
{
    vx_reference_s                          base;

    vx_view_region_s                        viewRegion; /* default is whole size */
    vx_bool                                 isViewed;

    vx_tensor_buffer_s *                    tensorBuffer; /* shared by all related tensors */
    vx_bool                                 isVirtual;

    /* they are same as those in memory structure in most time unless reshaped */
    vx_uint32                               dimCount;
    vx_uint32                               dims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32                               strides[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32                               baseAddressOffset;

    vx_uint32                               insideDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32                               finalDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
}
vx_tensor_s;

enum _vx_tp_cmd_type_e
{
    TP_RESHUFFLE,
    TP_SINGLE_FC,
    TP_MAX_POOLING,
    TP_LEAKY_RELU,
    TP_LEAKY_RELU_MAX_POOLING,
    TP_LRN,
    TP_TRANSPOSE,
    TP_ROI_POOLING,
    TP_CMD_NUM,
};

typedef struct  _vx_weights_biases_parameter
{
    vx_reference_s                           base;

    vx_uint32                                inout_num_of_dims;
    vx_uint32                                weights_num_of_dims;
    vx_uint32 *                              weights_sizes;
    vx_uint32 *                              org_weights_sizes;
    vx_enum                                  weights_data_format;
    vx_uint8                                 weights_fixed_point_pos;
    vx_uint32                                biases_num_of_dims;
    vx_uint32 *                              biases_sizes;
    vx_enum                                  biases_data_format;
    vx_uint8                                 biases_fixed_point_pos;

    vx_uint32                                stride;
    vx_uint32                                pooling_size_x;
    vx_uint32                                pooling_size_y;
    vx_uint32                                pad_x_left;
    vx_uint32                                pad_x_right;
    vx_uint32                                pad_y_bottom;
    vx_uint32                                pad_y_top;
    vx_uint32                                pooling_stride;
    vx_enum                                  down_scale_size_rounding;
    vx_enum                                  layer_type;

#define MAX_WEIGHT_BIAS_GROUPS  4
#define MAX_ZGROUP_COUNT     64

    vx_uint32                                kernelsPerCore[2][MAX_WEIGHT_BIAS_GROUPS];
    vx_uint32                                outImageTileXSize[2][MAX_WEIGHT_BIAS_GROUPS];
    vx_uint32                                outImageTileYSize[2][MAX_WEIGHT_BIAS_GROUPS];

    vx_size                                  perfCycleCount[2][MAX_WEIGHT_BIAS_GROUPS];
    vx_size                                  perfReadBandWidth[2][MAX_WEIGHT_BIAS_GROUPS];
    vx_size                                  perfNCReadBandWidth[2][MAX_WEIGHT_BIAS_GROUPS];
    vx_size                                  perfWriteBandWidth[2][MAX_WEIGHT_BIAS_GROUPS];

    vx_size                                  perfTPCycleCount[MAX_ZGROUP_COUNT];
    vx_size                                  perfTPReadBandWidth[MAX_ZGROUP_COUNT];
    vx_size                                  perfTPWriteBandWidth[MAX_ZGROUP_COUNT];

    vx_uint32                                zgroup_num;
    vx_uint32                                zgroup_array[MAX_ZGROUP_COUNT];

    vx_memory_s                              memory;
    vx_size                                  memory_size;
    vx_size                                  memory_offset_array[MAX_ZGROUP_COUNT];
    vx_size                                  memory_sizes_array[MAX_ZGROUP_COUNT];
    vx_uint32                                memory_pad;
    vx_uint32                                memroy_head_offset;

    vx_uint32 *                              input_sizes;
    vx_uint32 *                              output_sizes;
    vx_uint32                                all_count[MAX_ZGROUP_COUNT];
    vx_uint32                                zero_count[MAX_ZGROUP_COUNT];
    vx_uint32                                orig_size[MAX_ZGROUP_COUNT];
    vx_uint32                                compressed_size[MAX_ZGROUP_COUNT];
    vx_float32                               sustained_bandwidth;
    vx_uint32                                current_mad_per_core;

    vx_bool                                  use_tp_fc;
    vx_bool                                  use_fc_accel;
    vx_bool                                  fc_accel_large_size;
    vx_uint32                                input_nonzero_count;
    void *                                   tmp_fcaccel_input_ptr;
    void *                                   tmp_fcaccel_wb_ptr;

    vx_int32                                 inImageFractionLength;
    vx_int32                                 outImageFractionLength;
    vx_int32                                 weightFractionLength;

    vx_int8                                  weightFixedPointPos;

    vx_uint32                                postMultiplier;
    vx_uint32                                postShift;

    /* for SRAM */
    vx_uint32                                kernelStreamSize;
}
vx_weights_biases_parameter_s;

typedef struct _vx_array
{
    vx_reference_s                          base;

    vx_memory_s                             memory;

    vx_enum                                 itemType;

    vx_size                                 itemSize;

    vx_size                                 itemCount;

    vx_uint32                                offset;

    vx_size                                 capacity;

    vx_mem_alloc_info_s                     memAllocInfo;
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

typedef struct _vx_threshold
{
    vx_reference_s                          base;

    vx_enum                                 thresholdType;

    vx_enum                                 dataType;

    vx_uint32                               value;

    vx_uint32                               lower;
    vx_uint32                               upper;

    vx_uint32                               trueValue;
    vx_uint32                               falseValue;

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
        } thresholdInfo;

        struct object_array {
            vx_enum item_type;
            vx_size item_count;
        } objectArrayInfo;

        struct tensor {
            vx_enum                         dataFormat;
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
    }
    u;
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

#if defined(__linux__)

VX_INTERNAL_API struct timeval gcfVX_PerfStart(vx_reference ref);
VX_INTERNAL_API vx_uint32 gcfVX_PerfEnd(vx_reference ref, struct timeval start);

#endif

#include <gc_vx_context.h>
#include <gc_vx_delay.h>
#include <gc_vx_graph.h>
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

#endif /* __GC_VX_COMMON_H__ */

