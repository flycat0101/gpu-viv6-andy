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

#include "gc_vxk_common.h"

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
#define VX_MAX_REF_COUNT                    1024
#define VX_MAX_NODE_COUNT                   VX_MAX_REF_COUNT
#define VX_MAX_MODULE_COUNT                 16
#define VX_MAX_TARGET_COUNT                 1
#define VX_MAX_KERNEL_COUNT                 1024
#define VX_MAX_USER_STRUCT_COUNT            1024
#define VX_MAX_CONVOLUTION_DIM              15
#define VX_MAX_OPTICAL_FLOW_WINDOW_DIM      9
#define VX_MAX_PARAMETERS                   10
#define VX_MAX_NODES_IN_GRAPH               VX_MAX_NODE_COUNT
#define VX_MAX_PLANES                       4


#define VX_HOST_CORE_COUNT                  1

/* Function macros */
#ifndef vxmLENGTH_OF
#define vxmLENGTH_OF(array)                 (sizeof(array) / sizeof((array)[0]))
#endif

#define vxmASSERT                           gcmASSERT

#define vxmBOOL_TO_STRING(b)                ((b) ? "true" : "false")

#define vxmIS_SCALAR(type)                  (VX_TYPE_INVALID < (type) && (type) < VX_TYPE_SCALAR_MAX)
#define vxmIS_STRUCT(type)                  ((type) >= VX_TYPE_RECTANGLE && (type) < VX_TYPE_STRUCT_MAX)
#define vxmIS_OBJECT(type)                  ((type) >= VX_TYPE_REFERENCE && (type) < VX_TYPE_OBJECT_MAX)

#define vxmIS_VALID_PARAMETERS(ptr, size, type, align) \
        ((size) == sizeof(type) && ((vx_size)(ptr) & (align)) == 0)

#define vxmVALIDATE_PARAMETERS(ptr, size, type, align) \
        do \
        { \
            if ((size) != sizeof(type) || ((vx_size)(ptr) & (align)) != 0) return VX_ERROR_INVALID_PARAMETERS; \
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


#ifndef VX_MAX_TARGET_NAME
#define VX_MAX_TARGET_NAME                  64
#endif

#define VX_LIBRARY_KHR_INTERNAL             0x1

enum vx_kernel_internal_e
{
    VX_KERNEL_INTERNAL_SOBEL_MxN                   = VX_KERNEL_BASE(VX_ID_KHRONOS, VX_LIBRARY_KHR_INTERNAL) + 0x0,

    VX_KERNEL_INTERNAL_HARRIS_SCORE                = VX_KERNEL_BASE(VX_ID_KHRONOS, VX_LIBRARY_KHR_INTERNAL) + 0x1,

    VX_KERNEL_INTERNAL_EUCLIDEAN_NONMAXSUPPRESSION = VX_KERNEL_BASE(VX_ID_KHRONOS, VX_LIBRARY_KHR_INTERNAL) + 0x2,

    VX_KERNEL_INTERNAL_IMAGE_LISTER                = VX_KERNEL_BASE(VX_ID_KHRONOS, VX_LIBRARY_KHR_INTERNAL) + 0x3,

    VX_KERNEL_INTERNAL_ELEMENTWISE_NORM            = VX_KERNEL_BASE(VX_ID_KHRONOS, VX_LIBRARY_KHR_INTERNAL) + 0x4,

    VX_KERNEL_INTERNAL_NONMAXSUPPRESSION           = VX_KERNEL_BASE(VX_ID_KHRONOS, VX_LIBRARY_KHR_INTERNAL) + 0x5,

    VX_KERNEL_INTERNAL_EDGE_TRACE                  = VX_KERNEL_BASE(VX_ID_KHRONOS, VX_LIBRARY_KHR_INTERNAL) + 0x6,
};

enum _vx_extra_df_image
{
    VX_DF_IMAGE_F32 = VX_DF_IMAGE('F','0','3','2'),
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

#define VX_DEFAULT_TARGET_NAME              "vivante"

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

    vx_delay                                delay;
    vx_int32                                delayIndex;

    vx_ptr                                  reserved;
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

    vx_scalar_data*                         value;

    void*                                   node;
    void*                                   physical;
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

typedef struct _vx_kernel_attribute_s
{
    vx_size                                 localDataSize;
    vx_ptr                                  localDataPtr;

    vx_size                                 globalDataSize;
    vx_ptr                                  globalDataPtr;

    vx_border_mode_t                        borderMode;

    vx_bool                                 isAllGPU;

#ifdef OPENVX_KHR_TILING
    vx_tile_block_size_t                    tileBlockSize;

    vx_neighborhood_size_t                  inputNeighborhoodSize;

    vx_size                                 tileMemorySize;
    vx_ptr                                  tileMemoryPtr;
#endif
    vx_reference                            stagings[10];
}
vx_kernel_attribute_s;

typedef struct _vx_program
{
    vx_reference_s                          base;
}
vx_program_s;

typedef vx_program_s *                      vx_program;

typedef vx_program *                        vx_program_ptr;

typedef struct _vx_kernel
{
    vx_reference_s                          base;

    vx_char                                 name[VX_MAX_KERNEL_NAME];

    vx_enum                                 enumeration;

    vx_program                              program;

    vx_kernel_f                             function;

    vx_signature_s                          signature;

    vx_bool                                 enabled;

    vx_kernel_input_validate_f              inputValidateFunction;

    vx_kernel_output_validate_f             outputValidateFunction;

    vx_kernel_initialize_f                  initializeFunction;

    vx_kernel_deinitialize_f                deinitializeFunction;

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

typedef vx_status (* vx_target_initialize_f)        (vx_target target);

typedef vx_status (* vx_target_deinitialize_f)      (vx_target target);

typedef vx_status (* vx_target_iskernelsupported_f) (
        vx_target target, vx_char targetName[VX_MAX_TARGET_NAME], vx_char kernelName[VX_MAX_TARGET_NAME],
#if defined(OPENVX_USE_VARIANTS)
        vx_char variantName[VX_MAX_VARIANT_NAME],
#endif
        vx_uint32_ptr indexPtr);

typedef vx_kernel (*vx_target_addkernel_f)          (
        vx_target target, vx_char name[VX_MAX_KERNEL_NAME], vx_enum enumeration,
        vx_program program, vx_kernel_f funcPtr, vx_uint32 paramCount,
        vx_kernel_input_validate_f inputValidator, vx_kernel_output_validate_f outputValidator,
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
}
vx_accessor_s;

typedef struct _vx_user_struct
{
    vx_enum                                 type;
    vx_size                                 size;
}
vx_user_struct_s;

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

    vx_user_struct_s                        userStructTable[VX_MAX_USER_STRUCT_COUNT];

#if VX_USE_THREADPOOL
    vx_threadpool                           threadPool;
#endif

    vx_border_mode_t                        immediateBorderMode;
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

    vx_cost_factors_s                       costFactors;

    void*                                   cmdBuffer;

    vx_size                                 cmdSizeBytes;
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

typedef struct _vx_graph
{
    vx_reference_s                          base;

    vx_uint32                               nodeCount;
    vx_node                                 nodeTable[VX_MAX_NODE_COUNT];

    vx_perf_t                               perf;

    vx_uint32                               headNodeCount;
    vx_uint32                               headNodeIndexTable[VX_MAX_NODE_COUNT];

    vx_status                               status;

    vx_bool                                 verified;
    vx_bool                                 dirty;

    vx_uint32                               paramCount;
    vx_graph_parameter_s                    paramTable[VX_MAX_PARAMETERS];

    vx_bool                                 serialize;

    vx_mutex                                scheduleLock;

    vx_value_set_s                          data;
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

typedef struct _vx_memory_s
{
    vx_uint32                               planeCount;

    vx_uint32                               dimCount;
    vx_int32                                dims[VX_MAX_PLANES][VX_MAX_DIMS];
    vx_int32                                strides[VX_MAX_PLANES][VX_MAX_DIMS];

    vx_bool                                 allocated;

    vx_uint8_ptr                            logicals[VX_MAX_PLANES];
    vx_uint32                               physicals[VX_MAX_PLANES];
    gcsSURF_NODE_PTR                        nodePtrs[VX_MAX_PLANES];

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
}
vx_image_s;

typedef vx_image_s *                        vx_image;
typedef vx_image *                          vx_image_ptr;

typedef struct _vx_array
{
    vx_reference_s                          base;

    vx_memory_s                             memory;

    vx_enum                                 itemType;

    vx_size                                 itemSize;

    vx_size                                 itemCount;

    vx_size                                 capacity;
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

typedef struct _vx_delay
{
    vx_reference_s                          base;

    vx_enum                                 type;

    vx_size                                 count;
    vx_uint32                               index;

    vx_delay_parameter                      paramListTable;

    vx_reference_ptr                        refTable;
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
}
vx_remap_s;

typedef struct _vx_distribution
{
    vx_reference_s                          base;

    vx_memory_s                             memory;

    vx_uint32                               windowX;
    vx_uint32                               windowY;

    vx_uint32                               offsetX;
    vx_uint32                               offsetY;
}
vx_distribution_s;

typedef struct _vx_threshold
{
    vx_reference_s                          base;

    vx_enum                                 type;

    vx_uint32                               value;

    vx_uint32                               lower;
    vx_uint32                               upper;

    vx_uint32                               trueValue;
    vx_uint32                               falseValue;
}
vx_threshold_s;

typedef struct _vx_matrix
{
    vx_reference_s                          base;

    vx_memory_s                             memory;

    vx_enum                                 dataType;

    vx_size                                 columns;
    vx_size                                 rows;
}
vx_matrix_s;

typedef struct _vx_convolution
{
    vx_matrix_s                             matrix;

    vx_uint32                               scale;
}
vx_convolution_s;

typedef struct _vx_pyramid
{
    vx_reference_s                          base;

    vx_uint32                               levelCount;
    vx_image *                              levels;

    vx_float32                              scale;

    vx_uint32                               width;
    vx_uint32                               height;
    vx_df_image                             format;
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
    }
    u;
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

    vx_kernel_input_validate_f              inputValidate;
    vx_kernel_output_validate_f             outputValidate;
    vx_kernel_initialize_f                  initialize;
    vx_kernel_deinitialize_f                deinitialize;
}
vx_kernel_description_s;

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

#include <gc_vx_program.h>

EXTERN_C_BEGIN

#include <gc_vx_inlines.c>

EXTERN_C_END

#endif /* __GC_VX_COMMON_H__ */
