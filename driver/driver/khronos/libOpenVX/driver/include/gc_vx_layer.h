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


#ifndef __GC_VX_LAYER_H__
#define __GC_VX_LAYER_H__

#ifdef __cplusplus
extern "C" {
#endif

#define VX_MAX_LAYER_NAME                   128
#define VX_MAX_TEMP_ARRAYS                  64
#define VX_MAX_TEMP_TENSORS                 4096
#define VX_MAX_UNIFORMS                     128
#define VX_MAX_SHADER_PARAMETERS            20
#define VX_MAX_OPERTAION_PARAMETERS         64
#define VX_MAX_OPERTAION_INPUTS_OUTPUTS     30
#define VX_MAX_OPERTAION_GENERICS           (VX_MAX_OPERTAION_PARAMETERS - (VX_MAX_OPERTAION_INPUTS_OUTPUTS<<1))
#define VX_MAX_OPERATION_COUNT              8192
#define VX_MAX_BLOCK_COUNT                  1024
#define VX_MAX_SEGMENT_COUNT                4 * 1024
#define VX_MAX_BLOCK_SEGMENT_COUNT          64

#define VX_VIP_SRAM_IMAGE_STREAM_SIZE \
    (context->nnConfig.fixedFeature.latencyHidingAtFullAxiBw * context->nnConfig.fixedFeature.axiBusWidth)

#define SW_TILING_DEBUG                     1

struct _vxnne_operation_s;
struct _vxnne_layer_s;
struct _vx_shader_s;
struct _vx_node_block_s;
struct _vxnne_tiling_rect_s;
struct _vxnne_command_buffer_s;
struct _vxnne_tiling_block_info_s;

typedef struct _vxnne_mem_param_s *vxnne_mem_param;
typedef struct _vxnne_mem_request_s *vxnne_mem_request;
typedef struct _vx_arch_perf_s* vx_arch_perf;
typedef struct _vx_weights_biases_parameter_base_s *vx_weights_biases_parameter_base;

typedef vx_status (*vxnne_layer_dump_f)(struct _vxnne_layer_s* layer, int verbose, vx_uint32 format);
typedef vx_status (*vxnne_layer_claim_f)(struct _vxnne_layer_s* layer, vx_uint32 opindex);
typedef vx_status (*vxnne_layer_reclaim_f)(struct _vxnne_layer_s* layer, vx_uint32 opindex);
typedef vx_status (*vxnne_layer_deinitialize_f)(struct _vxnne_layer_s* layer);
typedef vx_status (*vxnne_layer_execute_f)(struct _vxnne_layer_s* layer);


typedef struct _vxnne_sram_s
{
    gctPOINTER                              logical;
    gctUINT32                               physical;
    gctUINT32                               physBase;
    gctUINT32                               size;
    gctUINT32                               used;
    gctUINT32                               tailUsed;
    gcsSURF_NODE_PTR                        node; /* video memory node to emulate SRAM */
}vxnne_sram_s, *vxnne_sram;


typedef struct _vxnne_layer_s
{
    vx_char                     *name;
    struct _vx_node             *node;
    vx_uint32                   num_operations;
    vx_uint32                   max_num_operations;
    struct _vxnne_operation_s**  operations;

    vx_uint32                   num_temp_tensors;
    vx_tensor                   temp_tensors[VX_MAX_TEMP_TENSORS];

    vx_uint32                   num_temp_arrays;
    vx_array                    temp_arrays[VX_MAX_TEMP_ARRAYS];

    vxnne_layer_dump_f           dump;
    vxnne_layer_claim_f          claim;
    vxnne_layer_reclaim_f        reclaim;
    vxnne_layer_execute_f        execute;
    vxnne_layer_deinitialize_f   deinitialize;

    vx_bool                      hasCPUFunction;
}
vxnne_layer_s, *vxnne_layer;


typedef enum vxnne_operation_target_e
{
    VXNNE_OPERATION_TARGET_NONE = 0,
    VXNNE_OPERATION_TARGET_SH,
    VXNNE_OPERATION_TARGET_NN,
    VXNNE_OPERATION_TARGET_TP,
    VXNNE_OPERATION_TARGET_SW,
    VXNNE_OPERATION_TARGET_SC, /* SCALER */
}
vxnne_operation_target_e;

typedef enum vxnne_sync_mode_e
{
    VXNNE_SYNC_MODE_NONE = 0,
    VXNNE_SYNC_MODE_HW_WAKE,
    VXNNE_SYNC_MODE_SW_WAKE,
    VXNNE_SYNC_MODE_HW_WAIT,
    VXNNE_SYNC_MODE_SW_WAIT
}
vxnne_sync_mode_e;

typedef enum _vxnne_adapter_type_e
{
    /*! \brief  Reorgnization from cwhn(ann) to whcn(ovx). */
    VX_ADAPTER_CWHN_TO_WHCN = 10,

    /*! \brief  Reorgnization from whcn(ovx) to cwhn(ann). */
    VX_ADAPTER_WHCN_TO_CWHN,

    /*! \brief  Reorgnization from float32 to float16. */
    VX_ADAPTER_F32_TO_F16,

    /*! \brief  Reorgnization from float32 to float16. */
    VX_ADAPTER_F16_TO_F32,
}
vxnne_adapter_type_e;

#define VXNNE_MAKE_SYNC_PATTERN(OperationTarget1, OperationTarget2)  (OperationTarget1 << 16 | OperationTarget2)

#define VXNNE_OPERATION_SYNC_PATTERN_SW_SW  VXNNE_MAKE_SYNC_PATTERN(VXNNE_OPERATION_TARGET_SW, VXNNE_OPERATION_TARGET_SW)
#define VXNNE_OPERATION_SYNC_PATTERN_SW_NN  VXNNE_MAKE_SYNC_PATTERN(VXNNE_OPERATION_TARGET_SW, VXNNE_OPERATION_TARGET_NN)
#define VXNNE_OPERATION_SYNC_PATTERN_SW_SH  VXNNE_MAKE_SYNC_PATTERN(VXNNE_OPERATION_TARGET_SW, VXNNE_OPERATION_TARGET_SH)
#define VXNNE_OPERATION_SYNC_PATTERN_SW_TP  VXNNE_MAKE_SYNC_PATTERN(VXNNE_OPERATION_TARGET_SW, VXNNE_OPERATION_TARGET_TP)
#define VXNNE_OPERATION_SYNC_PATTERN_SW_SC  VXNNE_MAKE_SYNC_PATTERN(VXNNE_OPERATION_TARGET_SW, VXNNE_OPERATION_TARGET_SC)

#define VXNNE_OPERATION_SYNC_PATTERN_NN_SW  VXNNE_MAKE_SYNC_PATTERN(VXNNE_OPERATION_TARGET_NN, VXNNE_OPERATION_TARGET_SW)
#define VXNNE_OPERATION_SYNC_PATTERN_NN_NN  VXNNE_MAKE_SYNC_PATTERN(VXNNE_OPERATION_TARGET_NN, VXNNE_OPERATION_TARGET_NN)
#define VXNNE_OPERATION_SYNC_PATTERN_NN_SH  VXNNE_MAKE_SYNC_PATTERN(VXNNE_OPERATION_TARGET_NN, VXNNE_OPERATION_TARGET_SH)
#define VXNNE_OPERATION_SYNC_PATTERN_NN_TP  VXNNE_MAKE_SYNC_PATTERN(VXNNE_OPERATION_TARGET_NN, VXNNE_OPERATION_TARGET_TP)
#define VXNNE_OPERATION_SYNC_PATTERN_NN_SC  VXNNE_MAKE_SYNC_PATTERN(VXNNE_OPERATION_TARGET_NN, VXNNE_OPERATION_TARGET_SC)

#define VXNNE_OPERATION_SYNC_PATTERN_SH_SW  VXNNE_MAKE_SYNC_PATTERN(VXNNE_OPERATION_TARGET_SH, VXNNE_OPERATION_TARGET_SW)
#define VXNNE_OPERATION_SYNC_PATTERN_SH_NN  VXNNE_MAKE_SYNC_PATTERN(VXNNE_OPERATION_TARGET_SH, VXNNE_OPERATION_TARGET_NN)
#define VXNNE_OPERATION_SYNC_PATTERN_SH_SH  VXNNE_MAKE_SYNC_PATTERN(VXNNE_OPERATION_TARGET_SH, VXNNE_OPERATION_TARGET_SH)
#define VXNNE_OPERATION_SYNC_PATTERN_SH_TP  VXNNE_MAKE_SYNC_PATTERN(VXNNE_OPERATION_TARGET_SH, VXNNE_OPERATION_TARGET_TP)
#define VXNNE_OPERATION_SYNC_PATTERN_SH_SC  VXNNE_MAKE_SYNC_PATTERN(VXNNE_OPERATION_TARGET_SH, VXNNE_OPERATION_TARGET_SC)

#define VXNNE_OPERATION_SYNC_PATTERN_TP_SW  VXNNE_MAKE_SYNC_PATTERN(VXNNE_OPERATION_TARGET_TP, VXNNE_OPERATION_TARGET_SW)
#define VXNNE_OPERATION_SYNC_PATTERN_TP_NN  VXNNE_MAKE_SYNC_PATTERN(VXNNE_OPERATION_TARGET_TP, VXNNE_OPERATION_TARGET_NN)
#define VXNNE_OPERATION_SYNC_PATTERN_TP_SH  VXNNE_MAKE_SYNC_PATTERN(VXNNE_OPERATION_TARGET_TP, VXNNE_OPERATION_TARGET_SH)
#define VXNNE_OPERATION_SYNC_PATTERN_TP_TP  VXNNE_MAKE_SYNC_PATTERN(VXNNE_OPERATION_TARGET_TP, VXNNE_OPERATION_TARGET_TP)
#define VXNNE_OPERATION_SYNC_PATTERN_TP_SC  VXNNE_MAKE_SYNC_PATTERN(VXNNE_OPERATION_TARGET_TP, VXNNE_OPERATION_TARGET_SC)

#define VXNNE_OPERATION_SYNC_PATTERN_SC_SW  VXNNE_MAKE_SYNC_PATTERN(VXNNE_OPERATION_TARGET_SC, VXNNE_OPERATION_TARGET_SW)
#define VXNNE_OPERATION_SYNC_PATTERN_SC_NN  VXNNE_MAKE_SYNC_PATTERN(VXNNE_OPERATION_TARGET_SC, VXNNE_OPERATION_TARGET_NN)
#define VXNNE_OPERATION_SYNC_PATTERN_SC_SH  VXNNE_MAKE_SYNC_PATTERN(VXNNE_OPERATION_TARGET_SC, VXNNE_OPERATION_TARGET_SH)
#define VXNNE_OPERATION_SYNC_PATTERN_SC_TP  VXNNE_MAKE_SYNC_PATTERN(VXNNE_OPERATION_TARGET_SC, VXNNE_OPERATION_TARGET_TP)
#define VXNNE_OPERATION_SYNC_PATTERN_SC_SC  VXNNE_MAKE_SYNC_PATTERN(VXNNE_OPERATION_TARGET_SC, VXNNE_OPERATION_TARGET_SC)

enum
{
    VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NONE_BIT          = 0,
    VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NO_BATCH_BIT      = 1 << 0,
    VXNNE_SHADER_PARAMETERS_ATTRIBUTE_INPUT_BIT         = 1 << 1, /* This parameter is an input of the shader operation */
    VXNNE_SHADER_PARAMETERS_ATTRIBUTE_OUTPUT_BIT        = 1 << 2, /* This parameter is an output of the shader operation */
    VXNNE_SHADER_PARAMETERS_ATTRIBUTE_ONE_COMPONENTS    = 1 << 3, /* This parameter is an tensor's components of the shader operation */
    VXNNE_SHADER_PARAMETERS_ATTRIBUTE_TWO_COMPONENTS    = 1 << 4, /* This parameter is an tensor's components of the shader operation */
    VXNNE_SHADER_PARAMETERS_ATTRIBUTE_THREE_COMPONENTS  = 1 << 5, /* This parameter is an tensor's components of the shader operation */
    VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS   = 1 << 6, /* This parameter is an tensor's components of the shader operation */
};

typedef enum vxnne_operator_e
{
    VXNNE_OPERATOR_NONE = 0,
    VXNNE_OPERATOR_CONVOLUTION,
    VXNNE_OPERATOR_RESHUFFLE,
    VXNNE_OPERATOR_FULLYCONNECTED,
    VXNNE_OPERATOR_ACTIVATION,
    VXNNE_OPERATOR_POOLING,
    VXNNE_OPERATOR_RESIZE,
    VXNNE_OPERATOR_TENSOR_ADD,
    VXNNE_OPERATOR_TENSOR_SUB,
    VXNNE_OPERATOR_TENSOR_MUL,
    VXNNE_OPERATOR_TENSOR_DIV,
    VXNNE_OPERATOR_TENSOR_TRANS,
    VXNNE_OPERATOR_SOFTMAX,
    VXNNE_OPERATOR_NORMALIZATION,
    VXNNE_OPERATOR_BATCHNORM,
    VXNNE_OPERATOR_INPUT2WEIGHT,
    VXNNE_OPERATOR_RPN_SOFTMAX,
    VXNNE_OPERATOR_RPN_REGRESSION,
    VXNNE_OPERATOR_RPN_SORT,
    VXNNE_OPERATOR_RPN_NMS,
    VXNNE_OPERATOR_RPN_SORT_NMS,
    VXNNE_OPERATOR_RPN_RETRIEVE,
    VXNNE_OPERATOR_RPN,
    VXNNE_OPERATOR_ROIPOOL,
    VXNNE_OPERATOR_ROIPOOLRELU,
    VXNNE_OPERATOR_CONCAT2,
    VXNNE_OPERATOR_CONCATINDEFINITE,
    VXNNE_OPERATOR_REORG,
    VXNNE_OPERATOR_VERTMAXPOOL,
    VXNNE_OPERATOR_HORZMAXPOOL,
    VXNNE_OPERATOR_PRETREATEDRECT,
    VXNNE_OPERATOR_BRICK,
    VXNNE_OPERATOR_DECONVOLUTION,
    VXNNE_OPERATOR_L2NORMALIZE,
    VXNNE_OPERATOR_L2NORMALIZE_SUMSQRT,
    VXNNE_OPERATOR_L2NORMALIZE_SUMSCALE,
    VXNNE_OPERATOR_TENSOR_COPY,
    VXNNE_OPERATOR_CONVERT_FORMAT,
    VXNNE_OPERATOR_TENSOR_REDUCE_SUM,
    VXNNE_OPERATOR_TENSOR_PAD,
    VXNNE_OPERATOR_LSTM_UNIT,
    VXNNE_OPERATOR_LSTM_LAYER,
    VXNNE_OPERATOR_REORG2,
    VXNNE_OPERATOR_TENSOR_ROUNDING,
    VXNNE_OPERATOR_HASHLUT,
    VXNNE_OPERATOR_LSH_PROJECTION,
    VXNNE_OPERATOR_TENSOR_RESHAPE,
    VXNNE_OPERATOR_TENSOR_SCALE,
    VXNNE_OPERATOR_YUV2RGB_SCALE,
    VXNNE_OPERATOR_RNN,
    VXNNE_OPERATOR_SVDF,
    VXNNE_OPERATOR_LUT2,
    VXNNE_OPERATOR_UPSAMPLE,
    VXNNE_OPERATOR_DILATION_RESHUFFLE,
    VXNNE_OPERATOR_DILATION_UPSAMPLE,
    VXNNE_OPERATOR_DILATION_UPSAMPLE2,
    VXNNE_OPERATOR_ADAPTER,
    VXNNE_OPERATOR_INTERLEAVE,
    VXNNE_OPERATOR_DEPTHWISE_CONV,
    VXNNE_OPERATOR_LSTM_RESHUFFLE_INPUT,
    VXNNE_OPERATOR_LSTM_STATE_OUT,
    VXNNE_OPERATOR_TENSOR_REVERSE,
    VXNNE_OPERATOR_USER_VXC,
    VXNNE_OPERATOR_USER_CPU,
    VXNNE_OPERATOR_TENSOR_MEAN,
    VXNNE_OPERATOR_TENSOR_SQUEEZE,
    VXNNE_OPERATOR_TENSOR_STRIDE_SLICE,
    VXNNE_OPERATOR_PRELU,
    VXNNE_OPERATOR_GRU,
    VXNNE_OPERATOR_GRU_LAYER,
    VXNNE_OPERATOR_CONV_LSTM,
    VXNNE_OPERATOR_CONV_LSTM_LAYER,
    VXNNE_OPERATOR_DEPTH_WISE_CONV,
    VXNNE_OPERATOR_SVDF_MAP,
    VXNNE_OPERATOR_SVDF_ROTATION,
    VXNNE_OPERATOR_LAYERNORM,
    VXNNE_OPERATOR_NBG,
    VXNNE_OPERATOR_TENSOR_MAX,
}
vxnne_operator_e;

enum vxnne_kernel_e
{
    VXNNE_KERNEL_TENSOR_ELTWISE = 0,
    VXNNE_KERNEL_GPU_TENSOR_ELTWISE,
    VXNNE_KERNEL_NN_LEAKY,
    VXNNE_KERNEL_GPU_NN_LEAKY,
    VXNNE_KERNEL_MAXPOOLING,
    VXNNE_KERNEL_GPU_MAXPOOLING,
    VXNNE_KERNEL_AVGPOOLING,
    VXNNE_KERNEL_GPU_AVGPOOLING,
    VXNNE_KERNEL_NORMALIZATION,
    VXNNE_KERNEL_GPU_NORMALIZATION,
    VXNNE_KERNEL_FULLYCONNECTED,
    VXNNE_KERNEL_GPU_FULLYCONNECTED,
    VXNNE_KERNEL_ACTIVATION,
    VXNNE_KERNEL_GPU_ACTIVATION,
    VXNNE_KERNEL_SOFTMAX,
    VXNNE_KERNEL_GPU_SOFTMAX,
    VXNNE_KERNEL_REORG,
    VXNNE_KERNEL_GPU_REORG,
    VXNNE_KERNEL_BATCHNORM,
    VXNNE_KERNEL_GPU_BATCHNORM,
    VXNNE_KERNEL_TENSOR_VERTMAXPOOL,
    VXNNE_KERNEL_TENSOR_HORZMAXPOOL,
    VXNNE_KERNEL_TENSOR_PRETREATEDRECT,
    VXNNE_KERNEL_DECONVOLUTION,
    VXNNE_KERNEL_GPU_DECONVOLUTION,
    VXNNE_KERNEL_TENSOR_COPY,
    VXNNE_KERNEL_GPU_TENSOR_COPY,
    VXNNE_KERNEL_TENSOR_RESHUFFLE,
    VXNNE_KERNEL_TENSOR_TRANSPOSE,
    VXNNE_KERNEL_GPU_TENSOR_TRANSPOSE,
    VXNNE_KERNEL_RPN_SOFTMAX,
    VXNNE_KERNEL_RPN_REGRESSION,
    VXNNE_KERNEL_RPN_NMS,
    VXNNE_KERNEL_TENSOR_CONVFORMAT,
    VXNNE_KERNEL_ACTIVATION_SOFTRELU,
    VXNNE_KERNEL_RPN_RETRIEVE,
    VXNNE_KERNEL_RPN_SORT,
    VXNNE_KERNEL_TENSOR2ROW,
    VXNNE_KERNEL_GPU_TENSOR2ROW,
    VXNNE_KERNEL_GEMM,
    VXNNE_KERNEL_GPU_GEMM,
    VXNNE_KERNEL_L2NORM_SUMSQRT,
    VXNNE_KERNEL_GPU_L2NORM_SUMSQRT,
    VXNNE_KERNEL_L2NORM_SUMSCALE,
    VXNNE_KERNEL_GPU_L2NORM_SUMSCALE,
    VXNNE_KERNEL_TENSOR_LSTMLAYER,
    VXNNE_KERNEL_TENSOR_LSTMUNIT,
    VXNNE_KERNEL_GPU_TENSOR_LSTMUNIT,
    VXNNE_KERNEL_AVGPOOLING_INT16,
    VXNNE_KERNEL_AVGPOOLING_UINT8,
    VXNNE_KERNEL_ACTIVATION_UINT8,
    VXNNE_KERNEL_TF_AVGPOOLING,
    VXNNE_KERNEL_ROIPOOL,
    VXNNE_KERNEL_GPU_ROIPOOL,
    VXNNE_KERNEL_L2POOLING,
    VXNNE_KERNEL_GPU_L2POOLING,
    VXNNE_KERNEL_DEPTH2SPACE,
    VXNNE_KERNEL_GPU_DEPTH2SPACE,
    VXNNE_KERNEL_RNN,
    VXNNE_KERNEL_GPU_RNN,
    VXNNE_KERNEL_TENSOR_SCALE,
    VXNNE_KERNEL_GPU_TENSOR_SCALE,
    VXNNE_KERNEL_ROIRECT2ROILIST,
    VXNNE_KERNEL_SVDF,
    VXNNE_KERNEL_GPU_SVDF,
    VXNNE_KERNEL_FLOOR,
    VXNNE_KERNEL_GPU_FLOOR,
    VXNNE_KERNEL_DEPTHWISE_CONV,
    VXNNE_KERNEL_GPU_DEPTHWISE_CONV,
    VXNNE_KERNEL_TENSOR_REVERSE,
    VXNNE_KERNEL_GPU_TENSOR_REVERSE,
    VXNNE_KERNEL_TENSOR_LSTMUNIT_PROJECTION,
    VXNNE_KERNEL_GPU_TENSOR_LSTMUNIT_PROJECTION,
    VXNNE_KERNEL_TENSOR_LSTMUNIT_HIDDENOUT,
    VXNNE_KERNEL_TENSOR_PAD,
    VXNNE_KERNEL_GPU_TENSOR_PAD,
    VXNNE_KERNEL_TENSOR_PAD2,
    VXNNE_KERNEL_GPU_TENSOR_PAD2,
    VXNNE_KERNEL_EMBEDDINGLUT,
    VXNNE_KERNEL_GPU_EMBEDDINGLUT,
    VXNNE_KERNEL_HASHLUT,
    VXNNE_KERNEL_GPU_HASHLUT,
    VXNNE_KERNEL_GPU_TENSOR_ADD,
    VXNNE_KERNEL_TENSOR_MUL,
    VXNNE_KERNEL_GPU_TENSOR_MUL,
    VXNNE_KERNEL_TENSOR_DIV,
    VXNNE_KERNEL_GPU_TENSOR_DIV,
    VXNNE_KERNEL_RESIZE_NEAREST_NEIGHBOR,
    VXNNE_KERNEL_GPU_RESIZE_NEAREST_NEIGHBOR,
    VXNNE_KERNEL_GEMM_NOBIAS,
    VXNNE_KERNEL_GPU_GEMM_NOBIAS,
    VXNNE_KERNEL_TENSOR_MEAN_AXIS,
    VXNNE_KERNEL_GPU_TENSOR_MEAN_AXIS,
    VXNNE_KERNEL_TENSOR_CROP,
    VXNNE_KERNEL_GPU_TENSOR_CROP,
    VXNNE_KERNEL_TENSOR_STRIDE_SLICE,
    VXNNE_KERNEL_GPU_TENSOR_STRIDE_SLICE,
    VXNNE_KERNEL_PRELU,
    VXNNE_KERNEL_GPU_PRELU,
    VXNNE_KERNEL_TENSOR_LSTMUNIT_HIDDENOUT_EXT,
    VXNNE_KERNEL_TENSOR_2D_ADD,
    VXNNE_KERNEL_TENSOR_ABS,
    VXNNE_KERNEL_TENSOR_TRANSCENDENTAL,
    VXNNE_KERNEL_GPU_TENSOR_TRANSCENDENTAL,
    VXNNE_KERNEL_TENSOR_MUL_SAT_RTE,
    VXNNE_KERNEL_GPU_CONVOLUTION_1X1,
    VXNNE_KERNEL_GPU_TENSOR_COPYROI,
    VXNNE_KERNEL_SPACE2DEPTH,
    VXNNE_KERNEL_GPU_SPACE2DEPTH,
    VXNNE_KERNEL_SPACE2BATCH,
    VXNNE_KERNEL_GPU_SPACE2BATCH,
    VXNNE_KERNEL_BATCH2SPACE,
    VXNNE_KERNEL_GPU_BATCH2SPACE,
    VXNNE_KERNEL_SHUFFLECHANNEL,
    VXNNE_KERNEL_GPU_SHUFFLECHANNEL,
    VXNNE_KERNEL_FC_TP_CHECK,
    VXNNE_KERNEL_FIXED_COUNT,
};

#define VXNNE_KERNEL_DYNAMIC_COUNT 1024

typedef enum _vxnne_user_node_type_e
{
    VXNNE_USER_NODE_TYPE_NONE,
    VXNNE_USER_NODE_TYPE_VXC,
    VXNNE_USER_NODE_TYPE_CPU
}
vxnne_user_node_type_e;

typedef vx_status (*vxnne_operation_execute_f)(struct _vxnne_operation_s *operation);
typedef vx_status (*vxnne_operation_dump_f)(struct _vxnne_operation_s *operation, int verbose);
typedef vx_status (*vxnne_operation_deinitialize_f)(struct _vxnne_operation_s *operation);
typedef vx_status (*vxnne_operation_initialize_f)(struct _vxnne_operation_s *operation);

typedef vx_status (*vxnne_operation_commands_f)(
    vx_context                         context,
    struct _vxnne_operation_s*         operation,
    struct _vxnne_tiling_rect_s*       input,
    struct _vxnne_tiling_rect_s*       output,
    vx_uint32                          kernelMode,
    vx_uint32                          opID,
    struct _vxnne_tiling_block_info_s* tilingBlock,
    struct _vxnne_command_buffer_s*    commandBuffer);

typedef vx_status (*vxnne_operation_calDimSize_f)(
    vx_int32                        inDimSize,
    struct _vxnne_operation_s*      operation,
    vx_int32*                       outDimSize,
    vx_int32                        calcuteType);

typedef struct _vx_tp_value_cmd_s
{
    vx_float32    f32[3];
    vx_uint32     u32[7];
    vx_enum       e32[1];
    vx_uint8*     p8[1];
}
vx_tp_value_cmd_s, *vx_tp_value_cmd;

typedef struct _vx_op_param_s
{
    vx_enum      pool_type;
    vx_uint32    pool_size_x;
    vx_uint32    pool_size_y;
    vx_uint32    pool_stride;
    vx_bool      enable_relu;
    vx_enum      conv_rounding_type;
    vx_uint32    pad_x_left;
    vx_uint32    pad_x_right;
    vx_uint32    pad_y_top;
    vx_uint32    pad_y_bottom;
    vx_uint32    pad_z_front;
    vx_uint32    pad_z_back;
    vx_uint32    pad_w_front;
    vx_uint32    pad_w_back;
    vx_enum      pad_mode;
    vx_int32     pad_const;
    vx_size      dilationX;
    vx_size      dilationY;
    /* for NN */
    vx_uint32    kernel_x;
    vx_uint32    kernel_y;
    vx_uint32    kernel_z;
    vx_uint32    out_image_tile_x;
    vx_uint32    out_image_tile_y;
    vx_uint32    kernels_per_core;
    vx_uint32    interleave_mode;
    vx_uint32    nnCoreCount;
    vx_uint32    kernelCacheStart;
    vx_uint32    kernelCacheSize;
    vx_enum      kernelCacheMode;
    vx_uint32    imageCacheStart;
    vx_uint32    imageCacheSize;
    vx_enum      imageCacheMode;

    vx_uint32    transposeInStart;
    vx_uint32    transposeInSize;
    vx_enum      transposeInMode;
    vx_uint8     transposeInChannel;

    vx_uint32    transposeOutStart;
    vx_uint32    transposeOutSize;
    vx_enum      transposeOutMode;
    vx_uint8     transposeOutChannel;

    /* for TP */
    vx_enum          tpType;
    vx_tp_value_cmd  tp_value;
    vx_reference     other_ref;
    vx_tensor        data_buff; /* TP lut or roi list */
    vx_bool          orig_no_pad;
}
vx_op_param_s, *vx_op_param;

typedef struct _vx_sw_tiling_param_s
{
    vx_uint32    sub_wb_phyical;
    vx_uint32    kernelStreamSize;
    vx_uint32    kernelStreamAlignSramSize;
}
vx_sw_tiling_param_s, *vx_sw_tiling_param;

typedef struct _vx_engines_sync_s
{
    vx_uint32 eventId[32];
    vx_uint32 eventCnt;
    vx_uint32 waitId[32];
    vx_uint32 waitTarget[32];
    vx_uint32 waitCnt;
}
vx_engines_sync_s, *vx_engines_sync;

typedef struct _vxnne_operation_s
{
    vxnne_layer                     layer;
    vx_uint32                       id; /* index within its parent layer */
    vxnne_operation_target_e        target;
    vxnne_operator_e                operatorType;
    vxnne_operation_execute_f       execute;
    vxnne_operation_dump_f          dump;
    vxnne_operation_initialize_f    initialize;
    vxnne_operation_deinitialize_f  deinitialize;
    vxnne_operation_commands_f      generateCommands;
    vxnne_operation_calDimSize_f    caculateDimSize;
    vxnne_sync_mode_e               waitMode;
    vxnne_sync_mode_e               wakeMode;
    gctUINT32                       semaWaitHandle;
    gctUINT32                       semaWakeHandle;

    vx_uint32                       circuleBufferHeight;
    vx_uint32                       imageCacheStart;
    vx_uint32                       imageCacheSize;
    vx_enum                         imageCacheMode;
    vx_uint32                       kernelCacheStart;
    vx_uint32                       kernelCacheSize;
    vx_enum                         kernelCacheMode;
    vx_uint32                       transposeInSize;
    vx_uint32                       transposeInStart;
    vx_uint32                       transposeInMode;
    vx_uint8                        transposeInChannel;
    vx_uint32                       transposeOutSize;
    vx_uint32                       transposeOutStart;
    vx_uint32                       transposeOutMode;
    vx_uint8                        transposeOutChannel;
    vx_uint32                       transposeKernelSize;
    vx_bool                         bTransposeIn;
    vx_bool                         bTransposeOut;

    vx_uint32                       esitimateKernelCacheSize;
    vx_uint32                       esitimateImageCacheSize;

    vx_uint32                       estimateInTransposeSize;
    vx_uint32                       estimateOutTransposeSize;

    vx_uint32                       perCmdSize;

    /* all references pass through operations */
    vx_reference                    references[VX_MAX_OPERTAION_PARAMETERS];
    vx_uint32                       refNum;
    /* subset of parameters which are only for easy access */
    vx_reference                    *inputs;
    vx_uint32                       inputsNum;
    vx_reference                    *outputs;
    vx_uint32                       outputsNum;
    vx_reference                    *generics;
    vx_uint32                       genericNum;
    /* different from parameters, they are only valid in one operation life period */
    vx_reference                    onetimeRefs[VX_MAX_OPERTAION_GENERICS];
    vx_uint32                       onetimeRefsNum;

    vx_uint32                       currBatchIndex;
    vx_uint32                       batchCount;

    vx_op_param_s                   parameter;

#define MAX_PARENT_CHILD_OP_NUM 400
    struct _vxnne_operation_s*      parentOps[MAX_PARENT_CHILD_OP_NUM];
    vx_uint32                       parentOpNum;
    vx_uint32                       parentLayerNum;
    struct _vxnne_operation_s*      childOps[MAX_PARENT_CHILD_OP_NUM];
    vx_uint32                       childOpNum;
    vx_uint32                       childLayerNum;
    vx_uint32                       segIndex;

    vx_uint32                       absoluteOperationID;
    vx_float64                      imgNonZeroRatio;
    vx_uint32                       gpuId;
    vx_bool                         mGpuSync;
    struct _vxnne_operation_s*      mGpuNext;
    struct _vxnne_operation_s*      mGpuHead;

    vx_uint32                       opDepth;
    vx_uint32                       opSequence;
    vx_engines_sync_s               engineSync;
}
vxnne_operation_s, *vxnne_operation;

typedef struct _vxnne_operation_info
{
    vx_enum                         target;
    vx_enum                         opType;
    vx_enum                         tpType;
    vx_tensor                       input;
    vx_tensor                       output;
    vx_weights_biases_parameter     weightsBiases;
    vx_size                         dilationX;
    vx_size                         dilationY;

    struct
    {
        vx_uint32                   left;
        vx_uint32                   top;
        vx_uint32                   right;
        vx_uint32                   bottom;
    }pad;

    vx_enum                         convRoundingType;
    vx_bool                         enableRelu;
    vx_bool                         enablePooling;
    vx_uint32                       kernelX;
    vx_uint32                       kernelY;
    vx_uint32                       kernelZ;
    vx_enum                         poolType;
    vx_uint32                       poolSizeX;
    vx_uint32                       poolSizeY;
    vx_uint32                       poolStrideX;
    vx_uint32                       poolStrideY;
    vx_uint32                       reshuffStrideX;
    vx_uint32                       reshuffStrideY;
    vx_enum                         padMode;
    vx_int32                        padConst;
    vx_int32                        normStrideX;
    vx_int32                        normStrideY;
}vxnne_operation_info_s, *vxnne_operation_info;


typedef enum vxnne_operation_reference_e
{
    VXNNE_OPERATION_REFENRENCE_NONE = 0,
    VXNNE_OPERATION_REFENRENCE_INPUT,
    VXNNE_OPERATION_REFENRENCE_OUTPUT,
    VXNNE_OPERATION_REFENRENCE_GENERIC,
    VXNNE_OPERATION_REFENRENCE_ONETIME,
    VXNNE_OPERATION_REFENRENCE_ALL_SIZE
}
vxnne_operation_reference_e;

typedef struct _vxnne_tiling_rect_s
{
    vx_uint32   x;
    vx_uint32   y;
    vx_uint32   width;
    vx_uint32   height;
    vx_uint32   circleBufferSize;
    vx_uint32   circularBufEndAddrPlus1;
    vx_uint32   xStride;
    vx_uint32   yStride;
    vx_uint32   zStride;
    vx_uint32   sRAM;
    vx_uint32   physical;
    gctPOINTER  logical;
    gctPOINTER  logicalBase;
    vx_uint32   memoryPhysicalBase;
    vx_uint32   memorySize;
    gctPOINTER  memoryLogicalBase;
}vxnne_tiling_rect_s, *vxnne_tiling_rect;

typedef struct _vxnne_operation_command_s *vxnne_operation_command;

typedef vx_status (*vxnne_operation_command_dump_f)(vxnne_operation_command opCommand, vxnne_operation operation, vx_enum dumpStage);

enum
{
    VXNNE_SRAM_CACHE_MODE_NONE = 0,
    VXNNE_SRAM_CACHE_MODE_PARTIAL_CACHE,
    VXNNE_SRAM_CACHE_MODE_FULL_CACHE,
    VXNNE_SRAM_CACHE_MODE_STREAM_CACHE
};

typedef struct _vxnne_tiling_rect_info_s
{
    vx_uint32                    start;
    vx_uint32                    end;
    vx_uint32                    x;
    vx_uint32                    y;
    vx_uint32                    width;
    vx_uint32                    height;
}vxnne_tiling_rect_info_s, *vxnne_tiling_rect_info;

typedef struct _vxnne_tiling_perf_param_s
{
    vx_uint32                    interleaveMode;
    vx_uint32                    kernelsPerCore;
    vx_uint32                    outImageTileXSize;
    vx_uint32                    outImageTileYSize;
    vx_uint32                    nnCoreCount;
}vxnne_tiling_perf_param_s, *vxnne_tiling_perf_param;


typedef struct _vxnne_tiling_info_s
{
    /*1st level data*/
    vxnne_tiling_rect_info_s        input;
    vxnne_tiling_rect_info_s        output;
    vx_uint32                       convWidth;
    vx_uint32                       convHeight;
    vx_uint32                       padLeft;
    vx_uint32                       padTop;

    vx_enum                         kernelMode;
    /*2nd level data*/
    vxnne_tiling_perf_param_s       tilingParam;

    /*3rd level data*/
    vx_bool                         flush;
    vx_bool                         walked;

}vxnne_tiling_info_s, *vxnne_tiling_info;

typedef struct _vxnne_tiling_order_info_s
{
    vx_uint32                   opID;
    vx_uint32                   subID;
    vxnne_tiling_info           tilingInfo;
}vxnne_tiling_order_info_s, *vxnne_tiling_order_info;

typedef struct _vxnne_segment_tiling_info_s
{
    vx_uint32                    start;
    vx_uint32                    count;
    vx_uint32                    M;
    vx_uint32                    N;
    vx_uint32                    estimateAxiSRAMUsed;
    vx_uint32                    estimateVipSRAMUsed;
    vx_uint32                    tileXCount;
    vx_uint32                    tileYCount;
    vxnne_tiling_info            tilingInfo;
    vx_uint32                    tilingOrderCount;
    vxnne_tiling_order_info      tilingOrderInfo;
}vxnne_segment_tiling_info_s, *vxnne_segment_tiling_info;

enum
{
    VXNNE_SEGMENT_TYPE_AB = 0,
    VXNNE_SEGMENT_TYPE_TILING
};

typedef struct _vxnne_segment_ab_info_s
{
    vx_uint32 mergeInputType;
    vx_uint32 mergeOutputType;
}vxnne_segment_ab_info_s, *vxnne_segment_ab_info;

typedef struct _vxnne_segment_s
{
    vx_enum        type;
    vx_enum        memType;
    vx_uint32      start;
    vx_uint32      end;
    vx_uint32      count;
    union
    {
        vxnne_segment_tiling_info_s tiling;
        vxnne_segment_ab_info_s     ab;
    }segmentInfo;

}vxnne_segment_s, *vxnne_segment;

typedef struct _vxnne_segment_collection_s
{
    vx_uint32           segmentNum;
    vxnne_segment       segments[VX_MAX_SEGMENT_COUNT];

}vxnne_segment_collection_s, *vxnne_segment_collection;

typedef struct _vxnne_block_s
{
    vx_uint32           start;
    vx_uint32           count;
    vx_uint32           segmentNum;
    vxnne_segment       segments[VX_MAX_BLOCK_SEGMENT_COUNT];
    vxnne_mem_param     memParam;
    vx_uint32           totalCommandCount;


}vxnne_block_s, *vxnne_block;

typedef struct _vxnne_command_buffer_s
{
    gctUINT32        commandCount;
    gctUINT          *eventID;
    gctUINT32        physical;
    gctPOINTER       logical;
    gcsSURF_NODE_PTR node;
}vxnne_command_buffer_s, *vxnne_command_buffer;

typedef struct _vxnne_tensor_info_s
{
    struct
    {
        gctUINT32        start;
        gctUINT32        circularBufEndAddrPlus1;
    }physical;

    gctUINT32        width;
    gctUINT32        height;
    gctUINT32        depth;

    gctUINT32        yStride;
    gctUINT32        zStride;
    gctUINT32        circleBufferSize;
    gctUINT32        sRAM;
    vx_uint32        memoryPhysicalBase;
    vx_uint32        memorySize;
    gctPOINTER       memoryLogicalBase;

    vx_enum          dataFormat;
    vx_enum          roundingMode;
    vx_int8          fixedPointPos;
    vx_float32       scale;
    vx_int32         zeroPoint;
    vx_enum          quantFormat;
    vx_int32         padZeorValue;
    vx_bool          brickMode;

    vx_bool          flush;
}vxnne_tensor_info_s, *vxnne_tensor_info;

enum
{
    VXNNE_BLOCK_FLAG_NONE = 0,
    VXNNE_BLOCK_FLAG_START,
    VXNNE_BLOCK_FLAG_END,
    VXNNE_BLOCK_FLAG_INTERNAL
};

enum
{
    VXNNE_SEGMENT_FLAG_NONE = 0,
    VXNNE_SEGMENT_FLAG_START,
    VXNNE_SEGMENT_FLAG_END,
    VXNNE_SEGMENT_FLAG_INTERNAL
};

enum
{
    VXNNE_DUMP_PRE_EXECUTION = 0,
    VXNNE_DUMP_POST_EXECUTION
};

typedef struct _vxnne_operation_command_s
{
    vx_uint32                    operationID;
    vx_uint32                    batchID;
    vx_uint32                    tileID;
    vxnne_operation              operation;
    vxnne_sync_mode_e            waitMode;
    vxnne_sync_mode_e            wakeMode;
    gctUINT32                    semaWaitHandle;
    gctUINT32                    semaWakeHandle;


    vxnne_tiling_rect_s          inputTile;
    vxnne_tiling_rect_s          outputTile;

    struct
    {
        vx_uint32 padLeft;
        vx_uint32 padTop;
        vx_uint32 convWidth;
        vx_uint32 convHeight;
        vx_bool   flush;
        vx_uint32 kernelCacheStart;
        vx_uint32 kernelCacheSize;
        vx_enum   kernelCacheMode;
        vx_uint32 imageCacheStart;
        vx_uint32 imageCacheSize;
        vx_enum   imageCacheMode;
        vx_uint32 transposeInSize;
        vx_uint32 transposeInStart;
        vx_uint32 transposeInMode;
        vx_uint8  transposeInChannel;
        vx_uint32 transposeOutSize;
        vx_uint32 transposeOutStart;
        vx_uint32 transposeOutMode;
        vx_uint8  transposeOutChannel;
        vxnne_tiling_perf_param_s   tilingParam;
        vx_weights_biases_parameter wb;
    }cmdInfo;

    vxnne_command_buffer_s       commandBuffer;
    vx_enum                      blockFlag;
    vx_enum                      segmentFlag;
    vxnne_operation_command_dump_f dump;
    vx_op_param_s                parameter;
}vxnne_operation_command_s;

#define MAX_SWAP_HANDLE 128

typedef struct _vx_swapHandel
{
    vx_reference     ref;
    vx_uint32        orgAddress;             /*original address of tensor or image*/
    vx_uint32*       cmdAddr[MAX_SWAP_HANDLE];/* NN or TP cmd address of tensor*/

    union
    {
        vx_uint32    offset[MAX_SWAP_HANDLE];/*tensor or image input/output offset*/
        vx_uint32    nodeTable[MAX_SWAP_HANDLE];
    }u;

    vx_uint32        cmdCount;
    vx_bool          isSH;
}
vx_swapHandel;

#define MAX_HANDLE 2048

typedef struct _vxnne_execution_layer_s
{

    vxnne_layer_s                base;
    vx_graph                     graph;
    vxnne_operation              *operations;
    vxnne_mem_request            memRequestList;
    vxnne_mem_param              initialReqList;

    vx_uint32                    blockNum;
    vxnne_block                  blocks;

    vx_uint32                    opIndicesNum;
    vxnne_operation_command      opIndices;
    vx_swapHandel*               swapHandle[MAX_HANDLE];
    vx_uint32                    swapcount;
}
vxnne_execution_layer_s, *vxnne_execution_layer;


typedef struct _vxnne_kernel_shaders_s
{
    vx_enum                             kernelEnum;
    vx_char                             *kernelName;
    struct _vx_shader_s                 **kernelShader;
    vx_uint32                           kernelShaderCount;
    vx_uint32                           paramNum;

}vxnne_kernel_shaders_s, *vxnne_kernel_shaders;

typedef struct _vx_uniform_s
{
    vx_char          *name;
    vx_uint8         *data;
    vx_uint32        size;
    vx_uint32        count;
}vx_uniform_s, *vx_uniform;

typedef struct _vxnne_shader_executable_s
{
    struct _vx_shader_s                 *kernelShader;

    vx_kernel_execution_parameters_t    shaderParam;
    vx_border_mode_t                    borderMode;
    vx_reference                        param[VX_MAX_SHADER_PARAMETERS];
    vx_reference                        *params;
    vx_enum                             *datatypes;
    vx_bitfield                         attribue[VX_MAX_SHADER_PARAMETERS];
    vx_uint32                           paramNum;
    vx_uint32                           inputNum;
    vx_uint32                           outputNum;
    vx_uniform                          uniforms;
    vx_uint32                           uniformCount;
}vxnne_shader_executable_s, *vxnne_shader_executable;


typedef struct _vxnne_shader_operation_s
{
    vxnne_operation_s                   base;
    vxnne_shader_executable             shaderExecutable;
}
vxnne_shader_operation_s, *vxnne_shader_operation;

typedef struct _vxnne_tp_operation_s
{
    vxnne_operation_s               base;
    vx_tensor                       input;
    vx_tensor                       input_ex;
    vx_weights_biases_parameter     weights_biases;
    vx_tensor                       output;
    vx_uint32                       slice_num;
}
vxnne_tp_operation_s, *vxnne_tp_operation;

/* customized declaration*/
typedef struct _vxnne_reshuffle_operation_s
{
    vxnne_operation_s               base;
    vx_tensor                       inputs;
    vx_weights_biases_parameter     weights_biases;
    vx_uint32                       pad_x_left;
    vx_uint32                       pad_x_right;
    vx_uint32                       pad_y_top;
    vx_uint32                       pad_y_bottom;
    vx_enum                         pad_mode;
    vx_scalar                       pad_const;
    vx_tensor                       outputs;
}
vxnne_reshuffle_operation_s, *vxnne_reshuffle_operation;

typedef struct _vxnne_convert_format_operation_s
{
    vxnne_operation_s               base;
    vx_tensor                       inputs;
    vx_tensor                       outputs;
}
vxnne_convert_format_operation_s, *vxnne_convert_format_operation;

typedef struct _vxnne_brick_operation_s
{
    vxnne_operation_s               base;
    vx_tensor                       inputs;
    vx_uint32                       outTileX;
    vx_uint32                       outTileY;
    vx_uint32                       num_tile_x;
    vx_uint32                       num_tile_y;
    vx_uint32                       kernel_x;
    vx_uint32                       kernel_y;
    vx_uint32                       pad_x_left;
    vx_uint32                       pad_x_right;
    vx_uint32                       pad_y_top;
    vx_uint32                       pad_y_bottom;
    vx_uint32                       stride;
    vx_tensor                       outputs;
}
vxnne_brick_operation_s, *vxnne_brick_operation;

typedef struct _vxnne_resize_operation_s
{
    vxnne_operation_s               base;
    vx_tensor                       inputs;
    vx_tensor                       outputs;
}
vxnne_resize_operation_s, *vxnne_resize_operation;

typedef struct _vx_performance_info_s
{
#define HALF_DONE 0x5
    vx_uint8                                 calculated;
    vx_uint32                                kernelsPerCore;
    vx_uint32                                outImageTileXSize;
    vx_uint32                                outImageTileYSize;
    vx_uint32                                interleaveMode;
    vx_uint32                                nnCoreCount;
    vx_float64                               perfCycleCount;
    vx_float64                               perfReadBandWidth;
    vx_float64                               perfWriteBandWidth;
    vx_float64                               perfAXIReadBandWidth;
    vx_float64                               perfAXIWriteBandWidth;
    vx_float64                               perfKernelReadBandWidth;
    vx_float64                               perfInImageReadBandWidth;
    vx_bool                                  isFirstComputeBottleNeck;
}
vx_performance_info_s, *vx_performance_info;

typedef struct _vxnne_convolution_relu_pooling_operation_s
{
    vxnne_operation_s               base;
    vx_tensor                       inputs;
    vx_tensor                       orig_inputs;
    vx_tensor                       reshape_inputs;
    vx_tensor                       reshape_outputs;
    vx_weights_biases_parameter     weights_biases;
    vx_weights_biases_parameter     reshape_weights_biases;
    vx_size                         dilationX;
    vx_size                         dilationY;
    vx_uint32                       pad_x_left;
    vx_uint32                       pad_x_right;
    vx_uint32                       pad_y_top;
    vx_uint32                       pad_y_bottom;
    vx_enum                         conv_rounding_type;
    vx_bool                         enable_relu;
    vx_bool                         enable_pooling;
    vx_enum                         pool_type;
    vx_uint32                       pool_size_x;
    vx_uint32                       pool_size_y;
    vx_enum                         padMode;
    vx_scalar                       padConst;
    vx_enum                         down_scale_size_rounding;
    vx_tensor                       outputs;
    vx_weights_biases_parameter     sub_wb;
    /* temp fix, need to remove later */
    vx_weights_biases_parameter     swtWeightBiases;

    vx_performance_info_s           resultInfo;
}
vxnne_convolution_relu_pooling_operation_s, *vxnne_convolution_relu_pooling_operation;


typedef struct _vxnne_covolution_relu_pooling_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[3];
    vxnne_reshuffle_operation_s                     reshuffle_operation;
    vxnne_shader_operation_s                        reshuffle_shader_operation;
    vxnne_resize_operation_s                        resize_operation;
    vxnne_brick_operation_s                         brick_operation;
    vxnne_convert_format_operation_s                convert_format_operation;
    vxnne_shader_operation_s                        convert_format_shader_operation;
    vxnne_convolution_relu_pooling_operation_s      convolution_operation;
    vxnne_tp_operation_s                            reshuffle_tp_operation;
    vxnne_tp_operation_s                            brick_tp_operation;
}
vxnne_convolution_relu_pooling_layer_s, *vxnne_convolution_relu_pooling_layer;

typedef struct _vxnne_fully_connected_sw_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs;
    vx_tensor                        weights;
    vx_tensor                        biases;
    vx_scalar                        pad;
    vx_scalar                        accumulator_bits;
    vx_scalar                        overflow_policy;
    vx_scalar                        rounding_policy;
    vx_scalar                        down_scale_size_rounding;
    vx_tensor                        outputs;
}
vxnne_fully_connected_sw_operation_s, *vxnne_fully_connected_sw_operation;

typedef struct _vxnne_fully_connected_sw_operation_fp16_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs;
    vx_tensor                        outputs;
}
vxnne_fully_connected_sw_operation_fp16_s, *vxnne_fully_connected_sw_operation_fp16;

typedef struct _vxnne_intput_convert_weight_operation_s
{
    vxnne_operation_s               base;
    vx_tensor                       inputs;
    vx_weights_biases_parameter     weights_biases;
    vx_bool                         enable_relu;
    vx_int8                         output_fp_pos;
    vx_tensor                       outputs;
}
vxnne_intput_convert_weight_operation_s, *vxnne_intput_convert_weight_operation;

typedef struct _vxnne_fully_connected_relu_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[3];
    vxnne_intput_convert_weight_operation_s         input_convert_weight_operation;
    vxnne_fully_connected_sw_operation_s            fully_connected_operation;
    vxnne_shader_operation_s                        fully_connected_SHoperation;
    vxnne_convolution_relu_pooling_operation_s      fully_connected_relu_operation;
    vxnne_tp_operation_s                            fully_connected_TPoperation[2];
    vxnne_fully_connected_sw_operation_fp16_s       fully_connected_sw_operation_fp16;
    vxnne_shader_operation_s                        fully_connected_sh_operation_fp16;
}
vxnne_fully_connected_relu_layer_s, *vxnne_fully_connected_relu_layer;

typedef struct _vxnne_activation_sw_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs;
    vx_scalar                        func;
    vx_scalar                        a;
    vx_scalar                        b;
    vx_tensor                        outputs;
}
vxnne_activation_sw_operation_s, *vxnne_activation_sw_operation;

typedef struct _vxnne_activation_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_activation_sw_operation_s                 activation_operation;
    vxnne_shader_operation_s                        activation_SHoperation;
    vxnne_tp_operation_s                            activation_tp_operation;
}
vxnne_activation_layer_s, *vxnne_activation_layer;

typedef struct _vxnne_prelu_sw_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs;
    vx_tensor                        alpha;
    vx_tensor                        outputs;
}
vxnne_prelu_sw_operation_s, *vxnne_prelu_sw_operation;

typedef struct _vxnne_prelu_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_prelu_sw_operation_s                      prelu_operation;
    vxnne_shader_operation_s                        prelu_sh_operation;
}
vxnne_prelu_layer_s, *vxnne_prelu_layer;

typedef struct _vxnne_batchnorm_sw_operation_s
{
    vxnne_operation_s                base;

    vx_scalar                        global;
    vx_scalar                        moving_average_fraction;
    vx_scalar                        eps;

    vx_tensor                        mean;
    vx_tensor                        variance;
    vx_tensor                        gamma;
    vx_tensor                        beta;

    vx_tensor                        input;
    vx_tensor                        output;
}
vxnne_batchnorm_sw_operation_s, *vxnne_batchnorm_sw_operation;

typedef struct _vxnne_batchnorm_layer_s
{
    vxnne_layer_s                                  base;
    vxnne_operation                                operations[1];
    vxnne_batchnorm_sw_operation_s                 batchnorm_operation;
    vxnne_shader_operation_s                       batchnorm_sh_operation;
}
vxnne_batchnorm_layer_s, *vxnne_batchnorm_layer;

typedef struct _vxnne_layernorm_sw_operation_s
{
    vxnne_operation_s                base;

    vx_scalar                        eps;

    vx_tensor                        bias;
    vx_tensor                        scale;

    vx_tensor                        input;
    vx_tensor                        output;
}
vxnne_layernorm_sw_operation_s, *vxnne_layernorm_sw_operation;

typedef struct _vxnne_layernorm_layer_s
{
    vxnne_layer_s                                  base;
    vxnne_operation                                operations[1];
    vxnne_layernorm_sw_operation_s                 layernorm_operation;
    vxnne_shader_operation_s                       layernorm_sh_operation;
}
vxnne_layernorm_layer_s, *vxnne_layernorm_layer;

typedef struct _vxnne_eltwise_sw_operation_s
{
    vxnne_operation_s                base;
    vx_enum                          kernel;
    vx_tensor                        input1;
    vx_tensor                        input2;
    vx_scalar                        scale;
    vx_scalar                        overflow;
    vx_scalar                        rounding;
    vx_tensor                        output;
}
vxnne_eltwise_sw_operation_s, *vxnne_eltwise_sw_operation;

typedef struct _vxnne_eltwise_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_eltwise_sw_operation_s                    eltwise_operation;
}
vxnne_eltwise_layer_s, *vxnne_eltwise_layer;

typedef struct _vxnne_pooling_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs;
    vx_enum                          pool_type;
    vx_uint32                        pool_size_x;
    vx_uint32                        pool_size_y;
    vx_uint32                        pool_pad_x_left;
    vx_uint32                        pool_pad_x_right;
    vx_uint32                        pool_pad_y_top;
    vx_uint32                        pool_pad_y_bottom;
    vx_enum                          rounding;
    vx_uint32                        stride_x;
    vx_uint32                        stride_y;
    vx_tensor                        outputs;
    vx_weights_biases_parameter      weights_biases;
}
vxnne_pooling_operation_s, * vxnne_pooling_operation;

typedef struct _vxnne_pooling_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[2];
    vxnne_pooling_operation_s                       pooling_sw_operation;
    vxnne_pooling_operation_s                       pooling_nne_operation;
    vxnne_tp_operation_s                            pooling_tp_operation;
    vxnne_shader_operation_s                        pooling_tensorcopy_sh_operation;
    vxnne_shader_operation_s                        pooling_sh_operation;
}
vxnne_pooling_layer_s, *vxnne_pooling_layer;

typedef struct _vxnne_normalization_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs;
    vx_enum                          norm_type;
    vx_uint32                        norm_size;
    vx_uint32                        div;
    vx_float32                       alpha;
    vx_float32                       beta;
    vx_float32                       bias;
    vx_tensor                        outputs;
}
vxnne_normalization_operation_s, * vxnne_normalization_operation;

typedef struct _vxnne_normalization_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_normalization_operation_s                 lrn_sw_operation;
    vxnne_shader_operation_s                        lrn_sh_operation;
    vxnne_tp_operation_s                            lrn_tp_operation;
}
vxnne_normalization_layer_s, *vxnne_normalization_layer;

typedef struct _vxnne_softmax_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs;
    vx_tensor                        outputs;
    vx_scalar                        beta;
}
vxnne_softmax_operation_s, * vxnne_softmax_operation;

typedef struct _vxnne_softmax_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_softmax_operation_s                       softmax_sw_operation;
    vxnne_shader_operation_s                        softmax_SHoperation;
}
vxnne_softmax_layer_s, *vxnne_softmax_layer;

typedef struct _vxnne_concat_Indefinite_sw_operation_s
{
    vxnne_operation_s                base;
    vx_object_array                  inputs;
    vx_scalar                        axis;
    vx_tensor                        outputs;
}
vxnne_concatIndefinite_sw_operation_s, *vxnne_concatIndefinite_sw_operation;

typedef struct _vxnne_concatIndefinite_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_concatIndefinite_sw_operation_s           concatIndefinite_operation;

    vxnne_operation*                                operations2;
    vxnne_shader_operation_s                        *concat_sh_unit_operation;
    vxnne_tp_operation                              tp_operation;
}
vxnne_concatIndefinite_layer_s, *vxnne_concatIndefinite_layer;

typedef struct _vxnne_concat2_sw_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs0;
    vx_tensor                        inputs1;
    vx_tensor                        outputs;
}
vxnne_concat2_sw_operation_s, *vxnne_concat2_sw_operation;

typedef struct _vxnne_concat2_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[2];
    vxnne_concat2_sw_operation_s                    concat2_operation;
    vxnne_shader_operation_s                        concat_sh_unit_operation[2];
}
vxnne_concat2_layer_s, *vxnne_concat2_layer;

typedef struct _vxnne_tensor_copy_sw_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        src;
    vx_tensor                        dst;
}
vxnne_tensor_copy_sw_operation_s, *vxnne_tensor_copy_sw_operation;

typedef struct _vxnne_tensor_copy_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_tensor_copy_sw_operation_s                tensor_copy_operation;
    vxnne_shader_operation_s                        tensor_copy_sh_operation;
    vxnne_tp_operation_s                            tensor_copy_tp_operation;
}
vxnne_tensor_copy_s, *vxnne_tensor_copy;

typedef struct _vxnne_tensor_copy_operation_s {
    vxnne_tp_operation_s tp_operation;
    vxnne_shader_operation_s sh_operation;
    vxnne_tensor_copy_sw_operation_s sw_operation;
}
vxnne_tensor_copy_operation_s, *vxnne_tensor_copy_operation;

typedef struct _vxnne_tensor_reverse_sw_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        input;
    vx_tensor                        output;
    vx_scalar                        axis[4];
    vx_uint32                        numOfAxis;
}
vxnne_tensor_reverse_sw_operation_s, *vxnne_tensor_reverse_sw_operation;

typedef struct _vxnne_tensor_reverse_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_tensor_reverse_sw_operation_s             tensor_reverse_sw_operation;
    vxnne_shader_operation_s                        tensor_reverse_sh_operation;
    vxnne_tp_operation_s                            tensor_reverse_tp_operation;
}
vxnne_tensor_reverse_s, *vxnne_tensor_reverse;

typedef struct _vxnne_tensor_reduce_sum_sw_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        src;
    vx_tensor                        dst;
    vx_scalar                        reduceDim;
    vx_scalar                        keepDim;
}
vxnne_tensor_reduce_sum_sw_operation_s, *vxnne_tensor_reduce_sum_sw_operation;

typedef struct _vxnne_tensor_trans_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        input;
    vx_array                         perm;
    vx_scalar                        pnum;
    vx_tensor                        output;
}
vxnne_tensor_trans_operation_s, *vxnne_tensor_trans_operation;

typedef struct _vxnne_tensor_reduce_sum_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[2];
    vxnne_tensor_reduce_sum_sw_operation_s          tensor_reduce_sum_operation;
    vxnne_tp_operation_s                            tensor_reduce_sum_trans_tp_operation;
    vxnne_shader_operation_s                        tensor_reduce_sum_trans_sh_operation;
    vxnne_shader_operation_s                        tensor_reduce_sum_sh_operation;
    vxnne_tensor_trans_operation_s                  tensor_trans_sw_operation;
}
vxnne_tensor_reduce_sum_s, *vxnne_tensor_reduce_sum;

typedef struct _vxnne_tensor_pad_sw_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        src;
    vx_tensor                        dst;
    vx_scalar                        padLeft;
    vx_scalar                        padRight;
    vx_scalar                        padTop;
    vx_scalar                        padBottom;
    vx_tensor                        pad_dims;
    vx_scalar                        padMode;
    vx_scalar                        padConst;
}
vxnne_tensor_pad_sw_operation_s, *vxnne_tensor_pad_sw_operation;

typedef struct _vxnne_tensor_pad_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[2];
    vxnne_tensor_pad_sw_operation_s                 tensor_pad_operation;
    vxnne_shader_operation_s                        tensor_pad_sh_operation;
    vxnne_tp_operation_s                            tensor_pad_tp_operation;
    vxnne_tp_operation_s                            tensor_pad_nc_tp_operation;
}
vxnne_tensor_pad_s, *vxnne_tensor_pad;

typedef struct _vxnne_tensor_add_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        input0;
    vx_tensor                        input1;
    vx_scalar                        policy;
    vx_tensor                        output;
}
vxnne_tensor_add_operation_s, *vxnne_tensor_add_operation;

typedef struct _vxnne_tensor_add_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_tensor_add_operation_s                    tensorAddSW;
    vxnne_shader_operation_s                        tensorAddSH;
    vxnne_tp_operation_s                            tensorAddTP;
}
vxnne_tensor_add_layer_s, *vxnne_tensor_add_layer;

typedef struct _vxnne_tensor_sub_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        input0;
    vx_tensor                        input1;
    vx_scalar                        policy;
    vx_tensor                        output;
}
vxnne_tensor_sub_operation_s, *vxnne_tensor_sub_operation;

typedef struct _vxnne_tensor_sub_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_tensor_sub_operation_s                    tensorSubSW;
    vxnne_shader_operation_s                        tensorSubSH;
}
vxnne_tensor_sub_layer_s, *vxnne_tensor_sub_layer;

typedef struct _vxnne_tensor_mul_operation_s
{
    vxnne_operation_s                base;
    vx_enum                          kernel;
    vx_tensor                        input0;
    vx_tensor                        input1;
    vx_scalar                        scale;
    vx_scalar                        overflow;
    vx_scalar                        rounding;
    vx_tensor                        output;
}
vxnne_tensor_mul_operation_s, *vxnne_tensor_mul_operation;

typedef struct _vxnne_tensor_mul_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_tensor_mul_operation_s                    tensorMulSW;
    vxnne_shader_operation_s                        tensorMulSH;
}
vxnne_tensor_mul_layer_s, *vxnne_tensor_mul_layer;

typedef struct _vxnne_tensor_div_operation_s
{
    vxnne_operation_s                base;
    vx_enum                          kernel;
    vx_tensor                        input0;
    vx_tensor                        input1;
    vx_scalar                        scale;
    vx_scalar                        overflow;
    vx_scalar                        rounding;
    vx_tensor                        output;
}
vxnne_tensor_div_operation_s, *vxnne_tensor_div_operation;

typedef struct _vxnne_tensor_div_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_tensor_div_operation_s                    tensorDivSW;
    vxnne_shader_operation_s                        tensorDivSH;
}
vxnne_tensor_div_layer_s, *vxnne_tensor_div_layer;


typedef struct _vxnne_tensor_trans_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[2];
    vxnne_tensor_trans_operation_s                  tensor_trans_sw_operation;
    vxnne_shader_operation_s                        tensor_trans_shader_operation;
    vxnne_shader_operation_s                        tensor_trans_shader_operation2;
    vxnne_shader_operation_s                        tensor_copy_sh_operation;
    vxnne_tp_operation_s                            tensor_trans_tp_operation;
}
vxnne_tensor_trans_layer_s, *vxnne_tensor_trans_layer;

typedef struct _vxnne_tensor_rpn_softmax_operation_s
{
    vxnne_operation_s               base;
    vx_tensor                       input;
    vx_tensor                       output;

    vx_bool                         input_stage;
    vx_bool                         output_stage;
}
vxnne_tensor_rpn_softmax_operation_s, *vxnne_tensor_rpn_softmax_operation;

typedef struct _vxnne_tensor_rpn_regression_operation_s
{
    vxnne_operation_s               base;
    vx_scalar                       feature_stride;
    vx_scalar                       min_size;
    vx_tensor                       score_buffer;
    vx_tensor                       bbox;
    vx_tensor                       anchors;
    vx_tensor                       img_info;
    vx_tensor                       output;

    vx_bool                         input_stage;
    vx_bool                         output_stage;
}
vxnne_tensor_rpn_regression_operation_s, *vxnne_tensor_rpn_regression_operation;

typedef struct _vxnne_tensor_rpn_sort_operation_s
{
    vxnne_operation_s               base;
    vx_scalar                       pre_nms_topn;
    vx_tensor                       proposal;

    vx_bool                         input_stage;
    vx_bool                         output_stage;
}
vxnne_tensor_rpn_sort_operation_s, *vxnne_tensor_rpn_sort_operation;

typedef struct _vxnne_tensor_rpn_nms_operation_s
{
    vxnne_operation_s               base;
    vx_scalar                       pre_nms_topn;
    vx_scalar                       post_nms_topn;
    vx_scalar                       nms_thresh;
    vx_tensor                       proposal;
    vx_tensor                       roi_indices;
    vx_scalar                       real_roi_t;

    vx_bool                         input_stage;
    vx_bool                         output_stage;
}
vxnne_tensor_rpn_nms_operation_s, *vxnne_tensor_rpn_nms_operation;
typedef struct _vxnne_tensor_rpn_sort_nms_operation_s
{
    vxnne_operation_s               base;
    vx_scalar                       pre_nms_topn;
    vx_scalar                       post_nms_topn;
    vx_scalar                       nms_thresh;
    vx_tensor                       proposal;
    vx_tensor                       roi_output;
    vx_tensor                       score_output;

    vx_bool                         input_stage;
    vx_bool                         output_stage;
}
vxnne_tensor_rpn_sort_nms_operation_s, *vxnne_tensor_rpn_sort_nms_operation;
typedef struct _vxnne_tensor_rpn_retrieve_operation_s
{
    vxnne_operation_s               base;

    vx_tensor                       proposal;
    vx_tensor                       roi_indices;
    vx_scalar                       real_roi_t;
    vx_tensor                       roi_output;
    vx_tensor                       score_output;

    vx_bool                         input_stage;
    vx_bool                         output_stage;
}
vxnne_tensor_rpn_retrieve_operation_s, *vxnne_tensor_rpn_retrieve_operation;


typedef struct _vxnne_tensor_rpn_operation_s
{
    vxnne_operation_s               base;
    vx_tensor                       score;
    vx_tensor                       bbox;
    vx_tensor                       anchors;
    vx_tensor                       img_info;
    vx_scalar                       feature_stride;
    vx_scalar                       min_size;
    vx_scalar                       pre_nms_topn;
    vx_scalar                       post_nms_topn;
    vx_scalar                       nms_thresh;
    vx_tensor                       roi_output;
    vx_tensor                       score_output;
}
vxnne_tensor_rpn_operation_s, *vxnne_tensor_rpn_operation;

typedef struct _vxnne_tensor_mean_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        input;
    vx_tensor                        axis;
    vx_scalar                        keep_dims;
    vx_tensor                        output;
}
vxnne_tensor_mean_operation_s, *vxnne_tensor_mean_operation;

typedef struct _vxnne_tensor_mean_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[2];
    vxnne_tensor_mean_operation_s                   tensor_mean_sw_operation;
    vxnne_tp_operation_s                            tensor_mean_trans_tp_operation;
    vxnne_shader_operation_s                        tensor_mean_pool_sh_operation;
    vxnne_shader_operation_s                        tensor_mean_axis0_sh_operation;
    vxnne_shader_operation_s                        tensor_mean_trans_sh_operation;
    vxnne_tensor_trans_operation_s                  tensor_trans_sw_operation;
}
vxnne_tensor_mean_layer_s, *vxnne_tensor_mean_layer;

typedef struct _vxnne_tensor_squeeze_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        input;
    vx_tensor                        squeeze_dims;
    vx_tensor                        output;
}
vxnne_tensor_squeeze_operation_s, *vxnne_tensor_squeeze_operation;

typedef struct _vxnne_tensor_squeezelayer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_tensor_squeeze_operation_s                tensor_squeeze_sw_operation;
    vxnne_shader_operation_s                        tensor_squeeze_sh_operation;
    vxnne_tp_operation_s                            tensor_squeeze_tp_operation;
}
vxnne_tensor_squeeze_layer_s, *vxnne_tensor_squeeze_layer;

typedef struct _vxnne_tensor_stride_slice_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        input;
    vx_tensor                        begin_dims;
    vx_tensor                        end_dims;
    vx_tensor                        strides;
    vx_scalar                        begin_mask;
    vx_scalar                        end_mask;
    vx_scalar                        shrink_axis_mask;
    vx_tensor                        output;
}
vxnne_tensor_stride_slice_operation_s, *vxnne_tensor_stride_slice_operation;

typedef struct _vxnne_tensor_stride_slicelayer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[2];
    vxnne_tensor_stride_slice_operation_s           tensor_stride_slice_sw_operation;
    vxnne_shader_operation_s                        tensor_stride_slice_sh_operation;
    vxnne_shader_operation_s                        tensor_crop_sh_operation;
    vxnne_shader_operation_s                        tensor_reverse_sh_operation;
    vxnne_tp_operation_s                            tensor_reverse_tp_operation;

    vxnne_tp_operation_s                            tensor_stride_slice_tp_operation;
}
vxnne_tensor_stride_slice_layer_s, *vxnne_tensor_stride_slice_layer;

typedef struct _vxnne_tensor_rpn_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[6];
    vxnne_tensor_rpn_operation_s                    tensorRpnSW;
    vxnne_tensor_rpn_softmax_operation_s            tensorRpnSoftmaxSW;
    vxnne_tensor_rpn_regression_operation_s         tensorRpnRegressionSW;
    vxnne_tensor_rpn_sort_operation_s               tensorRpnSortSW;
    vxnne_tensor_rpn_nms_operation_s                tensorRpnNmsSW;
    vxnne_tensor_rpn_sort_nms_operation_s           tensorRpnSortNmsSW;
    vxnne_tensor_rpn_retrieve_operation_s           tensorRpnRetrieveSW;
    vxnne_shader_operation_s                        tensorRpnSH;
    vxnne_shader_operation_s                        tensorRpnSoftmaxSH;
    vxnne_shader_operation_s                        tensorRpnRegressionSH;
    vxnne_shader_operation_s                        tensorRpnNmsSH;
    vxnne_shader_operation_s                        tensorRpnRetrieveSH;
    vxnne_shader_operation_s                        tensorRpnSortSH;
}
vxnne_tensor_rpn_layer_s, *vxnne_tensor_rpn_layer;


typedef struct _vxnne_vertmaxpool_operation_s
{
    vxnne_operation_s               base;
    vx_tensor                       inputs;
    vx_tensor                       outputs;
    vxnne_shader_operation_s        vertmaxpool_SHoperation;
}
vxnne_vertmaxpool_operation_s, *vxnne_vertmaxpool_operation;

typedef struct _vxnne_horzmaxpool_operation_s
{
    vxnne_operation_s               base;
    vx_tensor                       inputs;
    vx_tensor                       outputs;
    vxnne_shader_operation_s        horzmaxpool_SHoperation;
}
vxnne_horzmaxpool_operation_s, *vxnne_horzmaxpool_operation;

typedef struct _vxnne_pretreatedrect_operation_s
{
    vxnne_operation_s               base;
    vx_tensor                       inputs;
    vx_tensor                       outputs;
    vxnne_shader_operation_s        pretreatedrect_SHoperation;
}
vxnne_pretreatedrect_operation_s, *vxnne_pretreatedrect_operation;

typedef struct _vxnne_tensor_roipool_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        input_data;
    vx_tensor                        input_rois;
    vx_scalar                        pool_type;
    vx_scalar                        spatial_scale;
    vx_scalar                        pooled_height;
    vx_scalar                        pooled_width;
    vx_tensor                        output;
    vx_scalar                        relu;
}
vxnne_tensor_roipool_operation_s, *vxnne_tensor_roipool_operation;

typedef struct _vxnne_tensor_roipool_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[3];
    vxnne_vertmaxpool_operation_s                   vertmaxpool_operation;
    vxnne_horzmaxpool_operation_s                   horzmaxpool_operation;
    vxnne_pretreatedrect_operation_s                pretreatedrect_operation;
    vxnne_tensor_roipool_operation_s                tensorROIPoolSW;
    vxnne_shader_operation_s                        tensorROIPoolSH;
    vxnne_tp_operation_s                            roipool_tp_operation[2];
}
vxnne_tensor_roipool_layer_s, *vxnne_tensor_roipool_layer;

typedef struct _vxnne_convolution_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs;
    vx_tensor                        weights;
    vx_tensor                        biases;
    vx_scalar                        padX;
    vx_scalar                        padY;
    vx_scalar                        padXRight;
    vx_scalar                        padYBottom;
    vx_scalar                        dilationX;
    vx_scalar                        dilationY;
    vx_scalar                        strideX;
    vx_scalar                        strideY;
    vx_scalar                        accumulatorBits;
    vx_scalar                        overflowPolicy;
    vx_scalar                        roundingPolicy;
    vx_scalar                        relu;
    vx_scalar                        downScaleSizeRounding;
    vx_tensor                        outputs;
}
vxnne_convolution_operation_s, *vxnne_convolution_operation;

typedef struct _vxnne_deconvolution_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs;
    vx_tensor                        weights;
    vx_tensor                        biases;
    vx_scalar                        padding_x_left;
    vx_scalar                        padding_x_right;
    vx_scalar                        padding_y_top;
    vx_scalar                        padding_y_bottom;
    vx_scalar                        overflow_policy;
    vx_scalar                        rounding_policy;
    vx_scalar                        a_x;
    vx_scalar                        a_y;
    vx_scalar                        group;
    vx_scalar                        stride_x;
    vx_scalar                        stride_y;
    vx_tensor                        outputs;
}
vxnne_deconvolution_operation_s, * vxnne_deconvolution_operation;

typedef struct _vxnne_deconvolution_reshuffle_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        weights;
    vx_scalar                        padding_x_left;
    vx_scalar                        padding_x_right;
    vx_scalar                        padding_y_top;
    vx_scalar                        padding_y_bottom;
    vx_scalar                        a_x;
    vx_scalar                        a_y;
    vx_scalar                        group;

    vx_scalar                        stride_x;
    vx_scalar                        stride_y;

    vx_tensor                        reshuffled_inputs;
    vx_tensor                        reshuffled_weights;
    vx_tensor                        reshuffled_biases;

    vx_bool                          reshuffled;

    vx_bool                          create_wbp;
    vx_tensor                        inputs;
    vx_tensor                        outputs;
    vx_tensor                        bias;

    vx_weights_biases_parameter      weights_biaes;
    vx_weights_biases_parameter_optimizations_t* opt;
}
vxnne_deconvolution_reshuffle_operation_s, * vxnne_deconvolution_reshuffle_operation;

typedef struct _vxnne_deconvolution_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[4];
    vxnne_deconvolution_operation_s                 deconvolution_sw_operation;

    vxnne_deconvolution_reshuffle_operation_s       deconvolution_sw1_reshuffle_operation;
    vxnne_convolution_operation_s                   deconvolution_sw1_convolution_operation;
    vxnne_deconvolution_operation_s                 deconvolution_sw1_upsample_operation;

    vxnne_convolution_relu_pooling_operation_s      convolution_operation;

    vxnne_shader_operation_s                        deconvolution_sh_operation;

    vxnne_tp_operation_s                            upsample_tp_operation;
    vxnne_tp_operation_s                            upsample_tp_operation_clip;
}
vxnne_deconvolution_layer_s, *vxnne_deconvolution_layer;

typedef struct _vxnne_depthwise_convolution_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs;
    vx_tensor                        weights;
    vx_tensor                        biases;
    vx_scalar                        padXLeft;
    vx_scalar                        padXRight;
    vx_scalar                        padYTop;
    vx_scalar                        padYBottom;
    vx_scalar                        dilationX;
    vx_scalar                        dilationY;

    vx_scalar                        group;

    vx_scalar                        stride_x;
    vx_scalar                        stride_y;

    vx_scalar                        accumulatorBits;
    vx_scalar                        overflowPolicy;
    vx_scalar                        roundingPolicy;
    vx_scalar                        downScaleSizeRounding;
    vx_tensor                        outputs;

    vx_scalar                        depth_multiplier;
}
vxnne_depthwise_convolution_operation_s, * vxnne_depthwise_convolution_operation;

typedef struct _vxnne_convolution_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[4];
    vxnne_operation                                 *dynamic_operations;
    vxnne_reshuffle_operation_s                     reshuffleSW;
    vxnne_convolution_operation_s                   convolutionSW;

    vxnne_deconvolution_reshuffle_operation_s       convolution_sw1_reshuffle_operation;
    vxnne_deconvolution_reshuffle_operation_s       convolution_sw1_reshuffle_operation2;
    vxnne_convolution_operation_s                   convolution_sw1_convolution_operation;
    vxnne_convolution_operation_s                   convolution_sw1_upsample_operation;

    vxnne_tp_operation_s                            convolution_tp_reshuffle_operation;
    vxnne_convolution_relu_pooling_operation_s      convolution_nn_convolution_operation;
    vxnne_tp_operation_s                            convolution_tp_upsample_operation;
    vxnne_tp_operation_s                            convolution_tp_upsample_operation2;

    vxnne_shader_operation_s                        convolutionTensor2Row_sh_operation;
    vxnne_shader_operation_s                        convolutionGemm_sh_operation;

    vxnne_convolution_relu_pooling_operation        convolution_nn_convolution_dynamic_operation;
}
vxnne_convolution_layer_s, *vxnne_convolution_layer;


typedef struct _vxnne_depthwise_convolution_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[2];

    vxnne_depthwise_convolution_operation_s         convolution_sw1_depthwise_operation;
    vxnne_convolution_operation_s                   convolution_nn_convolution_sw;
    vxnne_convolution_relu_pooling_operation_s      convolution_nn_convolution_nne;
    vxnne_shader_operation_s                        depthwise_tensorcopy_sh_operation;
    vxnne_shader_operation_s                        depthwise_convolution_sh_operation;
}
vxnne_depthwise_convolution_layer_s, *vxnne_depthwise_convolution_layer;


typedef struct _vxnne_reorg_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs;
    vx_reference                     stride;
    vx_scalar                        type;
    vx_tensor                        pad;
    vx_tensor                        outputs;
    vx_scalar                        num_group;
    vx_scalar                        axis;
}
vxnne_reorg_operation_s, * vxnne_reorg_operation;

typedef struct _vxnne_reorg_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_reorg_operation_s                         reorg_sw_operation;
    vxnne_shader_operation_s                        reorg_sh_operation;
    vxnne_tp_operation_s                            reorg_tp_operation;
}
vxnne_reorg_layer_s, *vxnne_reorg_layer;

typedef struct _vxnne_l2normalize_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs;
    vx_tensor                        outputs;
}
vxnne_l2normalize_operation_s, * vxnne_l2normalize_operation;

typedef struct _vxnne_l2normalize_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[2];
    vxnne_l2normalize_operation_s                   l2normalize_sw_operation;
    vxnne_shader_operation_s                        l2normalize_SumSqrt_sh_operation;
    vxnne_shader_operation_s                        l2normalize_sumScale_sh_operation;
}
vxnne_l2normalize_layer_s, *vxnne_l2normalize_layer;

typedef struct _vxnne_tensor_rounding_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs;
    vx_scalar                        mode;
    vx_tensor                        outputs;
}
vxnne_tensor_rounding_operation_s, * vxnne_tensor_rounding_operation;

typedef struct _vxnne_tensor_rounding_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_tensor_rounding_operation_s               tensor_rounding_sw_operation;
    vxnne_shader_operation_s                        tensor_rounding_sh_operation;
}
vxnne_tensor_rounding_layer_s, *vxnne_tensor_rounding_layer;

typedef struct _vxnne_hashlut_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs;
    vx_tensor                        keys;
    vx_tensor                        values;
    vx_tensor                        hits;
    vx_tensor                        outputs;
}
vxnne_hashlut_operation_s, * vxnne_hashlut_operation;

typedef struct _vxnne_hashlut_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_hashlut_operation_s                       hashlut_sw_operation;
    vxnne_shader_operation_s                        hashlut_sh_operation;
}
vxnne_hashlut_layer_s, *vxnne_hashlut_layer;

typedef struct _vxnne_lshprojection_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs;
    vx_tensor                        type;
    vx_tensor                        hash_func;
    vx_tensor                        weight;
    vx_tensor                        outputs;
}
vxnne_lshprojection_operation_s, * vxnne_lshprojection_operation;

typedef struct _vxnne_lshprojection_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_lshprojection_operation_s                 lshprojection_sw_operation;
}
vxnne_lshprojection_layer_s, *vxnne_lshprojection_layer;

typedef struct _vxnne_reshape_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs;
    vx_tensor                        dims;
    vx_tensor                        outputs;
}
vxnne_reshape_operation_s, * vxnne_reshape_operation;

typedef struct _vxnne_reshape_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_reshape_operation_s                       reshape_sw_operation;
    vxnne_shader_operation_s                        tensor_copy_sh_operation;
    vxnne_tp_operation_s                            tensor_copy_tp_operation;
}
vxnne_reshape_layer_s, *vxnne_reshape_layer;

typedef struct _vxnne_tensor_scale_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs;
    vx_scalar                        type;
    vx_tensor                        outputs;
}
vxnne_tensor_scale_operation_s, * vxnne_tensor_scale_operation;

typedef struct _vxnne_tensor_scale_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_tensor_scale_operation_s                  tensor_scale_sw_operation;
    vxnne_shader_operation_s                        tensor_scale_sh_operation;
}
vxnne_tensor_scale_layer_s, *vxnne_tensor_scale_layer;

typedef struct _vxnne_yuv2rgb_scale_operation_s
{
    vxnne_operation_s                base;
    vx_image                         inputs;
    vx_scalar                        r_mean;
    vx_scalar                        g_mean;
    vx_scalar                        b_mean;
    vx_scalar                        rgb_scale;
    vx_scalar                        y_only;
    vx_scalar                        output_rgb;
    vx_rectangle_t                   rect;
    vx_uint32                        x_scale;
    vx_uint32                        y_scale;
    vx_uint16                        x_init_error;
    vx_uint16                        y_init_error;
    vx_uint16                        x_init_int_error;
    vx_uint16                        y_init_int_error;
    vx_uint32                        output_y_start;
    vx_uint32                        output_y_end;
    vx_tensor                        outputs;
}
vxnne_yuv2rgb_scale_operation_s, * vxnne_yuv2rgb_scale_operation;

typedef struct _vxnne_uiv2rgb_scale_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_yuv2rgb_scale_operation_s                 yuv2rgb_scale_sc_operation;
    vxnne_yuv2rgb_scale_operation_s                 yuv2rgb_scale_sw_operation;
    vxnne_shader_operation_s                        yuv2rgb_scale_sh_operation;
}
vxnne_yuv2rgb_scale_layer_s, *vxnne_yuv2rgb_scale_layer;

typedef struct _vxnne_rnn_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs;
    vx_tensor                        weights;
    vx_tensor                        recurrent_weights;
    vx_tensor                        bias;
    vx_tensor                        state_in;
    vx_tensor                        activation;
    vx_tensor                        state_out;
    vx_tensor                        outputs;
}
vxnne_rnn_operation_s, * vxnne_rnn_operation;

typedef struct _vxnne_rnn_input_reshuffle_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        input0;
    vx_tensor                        input1;
    vx_tensor                        new_input;

    vx_tensor                        input2;
    vx_tensor                        new_input2;
}
vxnne_rnn_input_reshuffle_operation_s, * vxnne_rnn_input_reshuffle_operation;

typedef struct _vxnne_rnn_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[4];
    vxnne_rnn_operation_s                           rnn_sw_operation;
    vxnne_shader_operation_s                        rnn_sh_operation;

    vxnne_convolution_relu_pooling_operation_s      convolution_nn_convolution_operation0;
    vxnne_convolution_relu_pooling_operation_s      convolution_nn_convolution_operation1;

    vxnne_tp_operation_s                            rnn_input_reshuffle_tp_operation;
    vxnne_tp_operation_s                            rnn_input_reshuffle_tp_operation2;

    vxnne_rnn_input_reshuffle_operation_s           rnn_input_reshuffle_operation;
    vxnne_convolution_operation_s                   rnn_sw_operation2;

    vxnne_tp_operation_s                            rnn_tensor_copy;
    vxnne_tensor_copy_sw_operation_s                rnn_sw_tensor_copy;
}
vxnne_rnn_layer_s, *vxnne_rnn_layer;

typedef struct _vxnne_softmax2_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[3];
    vxnne_softmax_operation_s                      softmax2_sw_operation;
    vxnne_shader_operation_s                        softmax2_SHoperation;
    vxnne_shader_operation_s                        softmax2_SHTensorMaxValueoperation;
    vxnne_shader_operation_s                        softmax2_SHTensorDivoperation;
}
vxnne_softmax2_layer_s, *vxnne_softmax2_layer;

typedef struct _vxnne_svdf_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs;
    vx_tensor                        weights_feature;
    vx_tensor                        recurrent_time;
    vx_tensor                        bias;
    vx_tensor                        state_in;
    vx_tensor                        rank;
    vx_tensor                        activation;
    vx_tensor                        state_out;
    vx_tensor                        outputs;
}
vxnne_svdf_operation_s, * vxnne_svdf_operation;

typedef struct _vxnne_svdf_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[4];
    vxnne_svdf_operation_s                          svdf_sw_operation;
    vxnne_shader_operation_s                        svdf_sh_operation;
    vxnne_tp_operation_s                            svdf_tp_operation[4];

    vxnne_fully_connected_sw_operation_s            svdf_sw_fc_operations[2];
    vxnne_svdf_operation_s                          svdf_sw_operations[2];

    vxnne_convolution_relu_pooling_operation_s      svdf_nn_operation[2];
    vxnne_fully_connected_sw_operation_fp16_s       svdf_sw_operation_fp16;
    vxnne_shader_operation_s                        svdf_sh_operation_fp16;
}
vxnne_svdf_layer_s, *vxnne_svdf_layer;


typedef struct _vxnne_lut2_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs;
    vx_tensor                        lut;
    vx_tensor                        outputs;
}
vxnne_lut2_operation_s, * vxnne_lut2_operation;

typedef struct _vxnne_lut2_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_lut2_operation_s                          lut2_sw_operation;
    vxnne_shader_operation_s                        lut2_sh_operation;
}
vxnne_lut2_layer_s, *vxnne_lut2_layer;

typedef struct _vxnne_adapter_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs;
    vx_scalar                        type;
    vx_tensor                        outputs;
}
vxnne_adapter_operation_s, * vxnne_adapter_operation;

typedef struct _vxnne_adapter_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[2];
    vxnne_adapter_operation_s                       adapter_sw_operation;
    vxnne_shader_operation_s                        adapter_sh_operation;
    vxnne_shader_operation_s                        adapter_convert_sh_operation;
    vxnne_tp_operation_s                            adapter_tp_operation;
}
vxnne_adapter_layer_s, *vxnne_adapter_layer;

typedef struct _vxnne_lstm_resuffle_input_operation_s
{
    vxnne_operation_s                base;

    vx_tensor                        input;

    vx_tensor                        output_state_in;
    vx_tensor                        cell_state_in;

    vx_tensor                        reshuffled_input;
}
vxnne_lstm_resuffle_input_operation_s, * vxnne_lstm_resuffle_input_operation;

typedef struct _vxnne_lstm_unit_sw_operation_s
{
    vxnne_operation_s                base;

    vx_tensor                        input;

    /* Input weight tensors of size: {n_cell, n_input}*/
    vx_tensor                        input2input_weight;                  /*!< \brief Optional */
    vx_tensor                        input2forget_weight;                 /*!< \brief  */
    vx_tensor                        input2cell_weight;                   /*!< \brief  */
    vx_tensor                        input2output_weight;                 /*!< \brief  */

    /* Recurrent weight tensors of size: {n_cell, n_output}*/
    vx_tensor                        recurrent2input_weight;              /*!< \brief Optional */
    vx_tensor                        recurrent2forget_weight;             /*!< \brief  */
    vx_tensor                        recurrent2cell_weight;               /*!< \brief  */
    vx_tensor                        recurrent2output_weight;             /*!< \brief  */

    /* Peephole weight tensors of size: {n_cell}*/
    vx_tensor                        cell2input_weight;                   /*!< \brief Optional */
    vx_tensor                        cell2forget_weight;                  /*!< \brief Optional */
    vx_tensor                        cell2output_weight;                  /*!< \brief Optional */

    /* Layer norm weight tensors of size: {n_cell}*/
    vx_tensor                        layernorm2input_weight;              /*!< \brief Optional */
    vx_tensor                        layernorm2forget_weight;             /*!< \brief  */
    vx_tensor                        layernorm2cell_weight;               /*!< \brief  */
    vx_tensor                        layernorm2output_weight;             /*!< \brief  */

    /* Gates bias tensors of size: {n_cell}*/
    vx_tensor                        input_gate_bias;                     /*!< \brief Optional */
    vx_tensor                        forget_gate_bias;                    /*!< \brief  */
    vx_tensor                        cell_bias;                           /*!< \brief  */
    vx_tensor                        output_gate_bias;                    /*!< \brief  */

    vx_tensor                        output_state_in;
    vx_tensor                        cell_state_in;

    /* Projection weight tensors of size: {n_output, n_cell}*/
    vx_tensor                        projection_weight;                   /*!< \brief Optional */
    /* Projection bias tensors of size: {n_output}*/
    vx_tensor                        projection_bias;                     /*!< \brief Optional */

    vx_tensor                        scratch;
    vx_tensor                        output_state_out;
    vx_tensor                        cell_state_out;
    vx_tensor                        output;

    vx_tensor                        activation;
    vx_tensor                        forget_bias;
    vx_tensor                        cell_clip;
    vx_tensor                        proj_clip;
}
vxnne_lstm_unit_sw_operation_s, *vxnne_lstm_unit_sw_operation;

typedef struct _vxnne_lstm_hidden_unit_sw_operation_s
{
    vxnne_operation_s               base;

    vx_tensor                       input_fc_in;
    vx_tensor                       hidden_fc_in;


    vx_tensor                       activation;
    vx_tensor                       forget_bias;
    vx_tensor                       cell_state;

    vx_uint32                       batch;
    vx_bool                         enable_cell_in;

    vx_bool                         enable_packed_mode;

    /* Output tensor. */
    vx_tensor                       output_state_out;
    vx_tensor                       cell_state_out;
    vx_tensor                       output;
    vx_uint32                       output_num;
}
vxnne_lstm_hidden_unit_sw_operation_s, *vxnne_lstm_hidden_unit_sw_operation;

typedef struct _vxnne_fc_operation_s {
    vxnne_tp_operation_s tp_operation;
    vxnne_tp_operation_s aux_tp_operation;
    vxnne_tp_operation_s copy_tp_operation;
    vxnne_tensor_copy_sw_operation_s copy_sw_operation;

    vxnne_tp_operation_s nn_trans_tp_operation[2];                /* transpose tp operation for NN FC */
    vxnne_shader_operation_s nn_trans_sh_operation[2];            /* transpose sh operation for NN FC */
    vxnne_tensor_trans_operation_s nn_trans_sw_operation[2];      /* transpose sh operation for NN FC */

    vxnne_convolution_relu_pooling_operation_s nn_operation;
    vxnne_convolution_operation_s nn_operation_sw;                /* NN fc sw operation */

    vxnne_tp_operation_s                            nn_operation_cp_tp_operation;
    vxnne_tensor_copy_sw_operation_s                nn_operation_cp_sw_operation;

    vxnne_shader_operation_s sh_operation;
    vxnne_fully_connected_sw_operation_s sw_operation;
    vx_weights_biases_parameter weights_biases;
}
vxnne_fc_operation_s, *vxnne_fc_operation;

typedef struct _vxnne_lstm_unit_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[16];
    vxnne_lstm_unit_sw_operation_s                  lstm_unit_operation;
    vxnne_shader_operation_s                        lstm_fc_sh_operation;
    vxnne_shader_operation_s                        lstm_unit_sh_operation;
    vxnne_shader_operation_s                        lstm_unit_prj_sh_operation;
    vxnne_shader_operation_s                        ts_cp_sh_operation[12];

    vxnne_tp_operation_s                            lstm_resuffle_input_tp_operation;
    vxnne_tp_operation_s                            lstm_resuffle_input_tp_operation2;

    vxnne_lstm_resuffle_input_operation_s           lstm_resuffle_input_operation;

    vxnne_fully_connected_sw_operation_s            lstm_sw_operation;
    vxnne_convolution_relu_pooling_operation_s      lstm_nn_operation;

    vxnne_tp_operation_s                            lstm_tp_fc_operation;
    vxnne_fc_operation_s                            fc_operation;
    vxnne_fully_connected_sw_operation_fp16_s       fully_connected_sw_operation_fp16;
    vxnne_shader_operation_s                        fully_connected_sh_operation_fp16;
}
vxnne_lstm_unit_s, *vxnne_lstm_unit;

typedef struct _vxnne_lstm_layer_sw_operation_s
{
    vxnne_operation_s               base;

    vx_tensor                       input;
    vx_uint32                       input_num;
    vx_tensor                       static_input;
    vx_tensor                       cont;

    /* Concated input weight tensor of size: {n_cell * 4, n_input}. */
    vx_tensor                       w_x;

    /* Concated recurrent weight tensor of size: {n_cell * 4, n_output}*/
    vx_tensor                       w_h;

    /* Concated recurrent weight tensor of size: {n_cell * 4, n_output}*/
    vx_tensor                       w_c;

    /* Concated gates bias tensor of size: {n_cell * 4}*/
    vx_tensor                       bias;

    /* Projection weight tensors of size: {n_output, n_cell}*/
    vx_tensor                       projection_weight;                   /*!< \brief Optional */
    /* Projection bias tensors of size: {n_output}*/
    vx_tensor                       projection_bias;                     /*!< \brief Optional */

    vx_tensor                       activation;
    vx_tensor                       forget_bias;
    vx_tensor                       cell_clip;
    vx_tensor                       proj_clip;
    vx_enum                         lstm_layer_type;

    vx_uint32                       time_step;
    vx_uint32                       batch;

    /* Intermidiate tensors. */
    vx_tensor                       w_x_x;
    vx_tensor                       gate_input;

    /* Output tensor. */
    vx_tensor                       output;
    vx_uint32                       output_num;
}
vxnne_lstm_layer_sw_operation_s, *vxnne_lstm_layer_sw_operation;

typedef struct _vxnne_lstm_hidden_unit_s
{
    vxnne_lstm_hidden_unit_sw_operation_s           lstm_hidden_unit_operation;
    vxnne_shader_operation_s                        lstm_hidden_unit_sh_operation;

    vxnne_tp_operation_s                            lstm_tp_fc_operation;
    vxnne_tp_operation_s                            lstm_aux_tp_fc_operation;
    vxnne_convolution_relu_pooling_operation_s      lstm_nn_operation;

    vxnne_fully_connected_sw_operation_s            lstm_sw_operation;
    vxnne_fc_operation_s                            recurrent_fc_operation;
    vxnne_fully_connected_sw_operation_fp16_s       fully_connected_sw_operation_fp16;
    vxnne_shader_operation_s                        fully_connected_sh_operation_fp16;
}
vxnne_lstm_hidden_unit_s, *vxnne_lstm_hidden_unit;

typedef struct _vxnne_lstm_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[4];
    vx_tensor                                       *cell_states;
    vx_tensor                                       *hidden_states;
    vxnne_lstm_layer_sw_operation_s                 lstm_layer_sw_operation;
    vxnne_convolution_operation_s                   lstm_conv_sw_operation;
    vxnne_shader_operation_s                        fc2_sh_operation;
    vxnne_shader_operation_s                        lstm_sh_operation;
    vxnne_shader_operation_s                        tensor_trans0_shader_operation;
    vxnne_shader_operation_s                        tensor_trans1_shader_operation;
    vxnne_convolution_relu_pooling_operation_s      lstm_nn_operation;
    vxnne_tp_operation_s                            tensor_trans0_tp_operation;
    vxnne_tp_operation_s                            tensor_trans1_tp_operation;

    /* lstm layer, operations point */
    vxnne_operation*                                operations2;
    /* lstm layer, unit */
    vxnne_lstm_unit                                 units;

    vxnne_operation*                                operations3;
    vxnne_fc_operation_s                            input_fc_operation;
    vxnne_lstm_hidden_unit                          hidden_units;
    vx_uint32                                       hidden_unit_num;
    vx_weights_biases_parameter                     sub_wb;
}
vxnne_lstm_layer_s, *vxnne_lstm_layer;

typedef struct _vxnne_gru_unit_sw_operation_s
{
    vxnne_operation_s                base;

    vx_tensor                        input;

    /* Input weight tensors of size: {n_cell, n_input}*/
    vx_tensor                        reset2input_weights;                  /*!< \brief Optional */
    vx_tensor                        update2input_weights;                 /*!< \brief  */
    vx_tensor                        connection2input_weights;             /*!< \brief  */

    /* Recurrent weight tensors of size: {n_cell, n_output}*/
    vx_tensor                        reset2recurrent_weights;              /*!< \brief Optional */
    vx_tensor                        update2recurrent_weights;             /*!< \brief  */
    vx_tensor                        connection2recurrent_weights;         /*!< \brief  */

    /* Gates bias tensors of size: {n_cell}*/
    vx_tensor                        gate_input_bias;                     /*!< \brief Optional */
    vx_tensor                        gate_recurrent_bias;                 /*!< \brief  */
    vx_tensor                        connection_bias;                     /*!< \brief  */

    vx_tensor                        state_in;

    vx_tensor                        state_out;
    vx_tensor                        output;
}
vxnne_gru_unit_sw_operation_s, *vxnne_gru_unit_sw_operation;

typedef struct _vxnne_gru_unit_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[16];
    vxnne_gru_unit_sw_operation_s                   gru_unit_operation;
}
vxnne_gru_unit_s, *vxnne_gru_unit;

typedef struct _vxnne_gru_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 *operations;
    vxnne_gru_unit                                  gru_layer_operation;
}
vxnne_gru_layer_s, *vxnne_gru_layer;
typedef struct _vxnne_convlstm_unit_sw_operation_s
{
    vxnne_operation_s                base;

    vx_tensor                        input;

    /* Input weight tensors of size: {n_cell, n_input}*/
    vx_tensor                        input2input_weight;                  /*!< \brief Optional */
    vx_tensor                        input2forget_weight;                 /*!< \brief  */
    vx_tensor                        input2cell_weight;                   /*!< \brief  */
    vx_tensor                        input2output_weight;                 /*!< \brief  */

    /* Recurrent weight tensors of size: {n_cell, n_output}*/
    vx_tensor                        recurrent2input_weight;              /*!< \brief Optional */
    vx_tensor                        recurrent2forget_weight;             /*!< \brief  */
    vx_tensor                        recurrent2cell_weight;               /*!< \brief  */
    vx_tensor                        recurrent2output_weight;             /*!< \brief  */

    /* Gates bias tensors of size: {n_cell}*/
    vx_tensor                        input_gate_bias;                     /*!< \brief Optional */
    vx_tensor                        forget_gate_bias;                    /*!< \brief  */
    vx_tensor                        cell_bias;                           /*!< \brief  */
    vx_tensor                        output_gate_bias;                    /*!< \brief  */

    vx_tensor                        state_in;

    vx_tensor                        state_out;
    vx_tensor                        output;
}
vxnne_convlstm_unit_sw_operation_s, *vxnne_convlstm_unit_sw_operation;

typedef struct _vxnne_convlstm_unit_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[16];
    vxnne_convlstm_unit_sw_operation_s              convlstm_unit_operation;
}
vxnne_convlstm_unit_s, *vxnne_convlstm_unit;

typedef struct _vxnne_convlstm_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 *operations;
    vxnne_convlstm_unit                             convlstm_layer_operation;
}
vxnne_convlstm_layer_s, *vxnne_convlstm_layer;

typedef struct _vxnne_user_operation_s
{
    vxnne_operation_s base;
    vxnne_user_node_type_e nodeType;
}
vxnne_user_operation_s, *vxnne_user_operation;

typedef struct _vxnne_user_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_user_operation_s                          user_cpu_operation;
    vxnne_shader_operation_s                        user_shader_operation;
}
vxnne_user_layer_s, *vxnne_user_layer;

/* export functions */
vx_status vxnneLayer_Free(struct _vxnne_layer_s* layer);

vx_status vxnneLayer_Initialize(
    vxnne_layer                 layer,
    vx_char                     *name,
    vx_node                     node,
    vx_uint32                   max_num_operations,
    vxnne_operation             *operations,
    vxnne_layer_deinitialize_f  deinitialize
    );

vx_status vxnneLayer_SetOperation(
    vxnne_layer layer,
    vxnne_operation operation,
    vx_uint32 index
    );


vx_status vxnneLayer_Deinitialize(
    struct _vxnne_layer_s* layer);

vx_status vxnneExecutionLayer_Create(
    vx_graph graph,
    vxnne_execution_layer* layer);

vx_status vxnneExecutionLayer_Execute(
    vxnne_layer layer
    );

vx_status vxnneExecutionLayer_Deinitialize(
    vxnne_layer layer
    );

vx_status vxnneExecutionLayer_GenerateCommands(
    vx_context context,
    vxnne_layer layer);

vx_status vxnneCommandBuffer_ExecuteCommands(
    vx_node                 node,
    vxnne_command_buffer    commandBuffer,
    gceVX_ACCELERATOR_TYPE  type,
    vx_uint32               gpuId,
    vx_bool                 sync,
    vx_uint32               syncEventID[]
    );

vx_status vxnneSRAM_Allocate(
    vxnne_sram              sRam,
    vx_uint32               size,
    gctPOINTER*             logical,
    vx_uint32*              physical
    );

vx_status vxnneSRAM_AllocateRest(
    vxnne_sram              sRam,
    vx_uint32*              restSize,
    gctPOINTER*             logical,
    vx_uint32*              physical
    );

vx_status vxnneSRAM_AllocateEx(
    vxnne_sram              sRam,
    vx_uint32               size,
    gctPOINTER*             logical,
    vx_uint32*              physical,
    vx_enum                 allocateFlag
    );

vx_status vxnneSRAM_Free(
    vxnne_sram              sRam,
    vx_uint32               size
    );

vx_status vxnneSRAM_Reset(
    vxnne_sram              sRam
    );


vx_uint32 ComputeInputSize(
    vx_uint32               outputSize,
    vx_uint32               kernelSize,
    vx_uint32               padTop,
    vx_uint32               padBottom,
    vx_uint32               poolingSize,
    vx_uint32               poolingStride,
    vx_uint32*              convOut,
    vx_uint32               type);

vx_status vxnneOperation_GetInfo(
    vxnne_operation operation,
    vxnne_operation_info info
    );

vx_status vxnneOperation_ExecuteCommands(
    vxnne_operation operation,
    vxnne_command_buffer commandBuffer
    );

vx_status vxnneOperation_NodeDump(
    vxnne_operation_command opCommand
    );

vx_status vxnneOperationCommand_GenerateCommands(
    vx_context               context,
    vxnne_operation_command  operationCommand
    );

vx_status vxnneOperation_CalculateDimSize(
    vx_int32                inDimSize,
    vxnne_operation         operation,
    vx_int32*               outDimSize,
    vx_int32                calculateType
    );

vx_status vxnneCalculateConvTilingParam(
    vx_context                                context,
    vxnne_convolution_relu_pooling_operation  conv_op,
    vxnne_tiling_info                         info,
    vx_uint8                                  inputSRAM,
    vx_uint8                                  outputSRAM,
    vx_bool_e                                 swtilingSubImage,
    vx_uint32                                 count,
    vx_uint32                                 vipSize
    );

vx_status vxnneOperation_InitializeCommand(
    vx_context context,
    vx_graph graph,
    vxnne_operation operation,
    vxnne_operation_command_s * command
    );

vx_uint32 caculate3DTileSize(
    vx_context context,
    vx_uint32 outputTileXSize,
    vx_uint32 outputTileYSize,
    vx_uint32 kernelX,
    vx_uint32 kernelY,
    vx_uint32 kernelZ,
    vx_uint32 dataFormat,
    vx_uint32 interleaveMode
    );


vx_bool checkImageCacheMode(
    vx_uint32 outImageZSize,
    vx_uint32 kernelsPerCore,
    vx_uint32 nnCoreCount
    );

vx_status vxnneOperation_CaculateSRAMCache(
    vx_context      context,
    vxnne_operation operation,
    vx_bool         enableImageCache,
    vx_uint32*      kernelCacheSize,
    vx_uint32*      kernelCacheStart,
    vx_enum*        kernelCacheMode,
    vx_uint32*      imageCacheSize,
    vx_uint32*      imageCacheStart,
    vx_enum*        imageCacheMode
    );

vx_weights_biases_parameter vxoWeightsBiases_Create(
    vx_context                       context,
    vx_weights_biases_parameter_base wb_base,
    vx_uint32 *                      weight_dims,
    vx_enum                          layer_type,
    vx_bool                          first_time
    );

vx_status vxoWeightsBiases_Compress(
    vx_context                       context,
    vx_weights_biases_parameter      wb,
    vx_uint32                        kernel_per_core,
    vx_uint32 *                      pooling_output_dims,
    vx_enum                          output_format,
    vx_int32                         z_offset
    );

vx_status vxoWeightsBiases_Decompress(
    vx_context                       context,
    vx_weights_biases_parameter      wb
    );

vx_status vxnneOperation_Initialize(
    vxnne_operation_s               *operation,
    vxnne_layer                     layer,
    vxnne_operation_target_e        target,
    vxnne_operator_e                operatorType,
    vxnne_operation_execute_f       execute,
    vxnne_operation_deinitialize_f  deinitialize,
    vx_uint32                       batchCount,
    vx_uint32                       cmdBuffSize
    );

vx_status vxnneShaderOperation_Initialize(
    vxnne_shader_operation_s            *operation,
    vxnne_layer                         layer,
    vxnne_operator_e                    operatorType,
    vx_uint32                           batchCount,
    vxnne_shader_executable             shaderExecutable
    );

vx_status vxnneShaderExecutable_SetUniform(
    vxnne_shader_executable shaderExecutable,
    vx_char     *name,
    vx_uint32   count,
    void        *value
    );

vxnne_kernel_shaders vxnneGetKernelShadersByEnum(
    vx_context context,
    vx_enum kernelEnum);

vxnne_kernel_shaders vxnneAddKernelShadersInProgram(
    vx_context context,
    vx_char* kernelName,
    vx_program program,
    vx_uint32  paramNum,
    vx_enum kernelEnum
    );

vxnne_shader_executable vxnneKernelShaders_CreateShaderExecutable(
    vxnne_kernel_shaders kernel,
    vx_char * subName,
    vx_border_mode_t *borderMode
    );

vx_status vxnneShaderExecutable_SetParameters(
    vxnne_shader_executable shaderExecutable,
    vx_reference parameters[],
    vx_uint32 paramNum);

vx_status vxnneShaderExecutable_SetParametersEx(
    vxnne_shader_executable shaderExecutable,
    vx_reference parameters[],
    vx_enum datatypes[],
    vx_uint32 paramNum);

vx_status vxnneShaderExecutable_SetParametersAttribute(
    vxnne_shader_executable shaderExecutable,
    vx_uint32 index,
    vx_bitfield attrib);

vx_status vxnneShaderExecutable_SetParametersAttributes(
    vxnne_shader_executable shaderExecutable,
    vx_uint32 index[],
    vx_uint32 count,
    vx_bitfield attrib);

vx_status vxnneShaderExecutable_SetExecutionParameters(
    vxnne_shader_executable shaderExecutable,
    vx_kernel_execution_parameters_t *shaderParam
    );

vx_status vxnneShaderExecutable_Destroy(
    vxnne_shader_executable shaderExecutable
    );

vx_status vxnneShaderExecutable_GetMaxWorkGroupSize(
    vxnne_shader_executable shaderExecutable,
    vx_uint32               *maxWorkGroupSize
    );

vxnne_shader_executable vxnneGetLeakyReluShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               alpha,
    vx_tensor               output
    );

vxnne_shader_executable vxnneGetFC_TPCheckShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_uint32               tp_fc_ksize);

vxnne_shader_executable vxnneGetFullyConnectedShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               weights,
    vx_tensor               bias,
    vx_int32                activation,
    vx_tensor               output);
vxnne_shader_executable vxnneGetTensorDivShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input0,
    vx_tensor               input1,
    vx_scalar               scale_s,
    vx_scalar               convertPolicy,
    vx_int32                activation,
    vx_enum                 operation,
    vx_tensor               output
    );
vxnne_shader_executable vxnneGetTensorAddShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input0,
    vx_tensor               input1,
    vx_scalar               scale_s,
    vx_scalar               convertPolicy,
    vx_int32                activation,
    vx_enum                 operation,
    vx_tensor               output);
vxnne_shader_executable vxnneGetTensorMulShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input0,
    vx_tensor               input1,
    vx_scalar               scale_s,
    vx_scalar               convertPolicy,
    vx_scalar               round,
    vx_int32                activation,
    vx_enum                 operation,
    vx_tensor               output);
vxnne_shader_executable vxnneGetAvgPoolingShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               poolType,
    vx_scalar               stride_x_s,
    vx_scalar               stride_y_s,
    vx_scalar               poolSizeX,
    vx_scalar               poolSizeY,
    vx_uint32               pool_pad_x_left,
    vx_uint32               pool_pad_y_top,
    vx_scalar               rounding,
    vx_tensor               output
    );
vxnne_shader_executable vxnneGetAvgPooling_Int16ShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               poolType,
    vx_scalar               stride_x_s,
    vx_scalar               stride_y_s,
    vx_scalar               poolSizeX,
    vx_scalar               poolSizeY,
    vx_uint32               pool_pad_x_left,
    vx_uint32               pool_pad_y_top,
    vx_scalar               rounding,
    vx_int32                activation,
    vx_tensor               output
    );
vxnne_shader_executable vxnneGetAvgPooling_UInt8ShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               poolType,
    vx_scalar               stride_x_s,
    vx_scalar               stride_y_s,
    vx_scalar               poolSizeX,
    vx_scalar               poolSizeY,
    vx_uint32               pool_pad_x_left,
    vx_uint32               pool_pad_y_top,
    vx_scalar               rounding,
    vx_int32                activation,
    vx_tensor               output
    );
vxnne_shader_executable vxnneGetMaxPoolingShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               poolType,
    vx_scalar               stride_x_s,
    vx_scalar               stride_y_s,
    vx_scalar               poolSizeX,
    vx_scalar               poolSizeY,
    vx_uint32               pool_pad_x_left,
    vx_uint32               pool_pad_y_top,
    vx_scalar               rounding,
    vx_int32                activation,
    vx_tensor               output
    );
vxnne_shader_executable vxnneGetActivationShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_enum                 funcType,
    vx_tensor               input,
    vx_float32              minVal,
    vx_float32              maxVal,
    vx_tensor               output);

vxnne_shader_executable vxnneGetActivation_UInt8ShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_enum                 funcType,
    vx_tensor               input,
    vx_float32              minVal,
    vx_float32              maxVal,
    vx_tensor               output);

vxnne_shader_executable vxnneGetSoftmaxShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_uint32               dims,
    vx_tensor               input,
    vx_float32              beta,
    vx_tensor               output);

vxnne_shader_executable vxnneGetNormalizationShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               type_s,
    vx_scalar               norm_size_s,
    vx_scalar               alpha_s,
    vx_scalar               beta_s,
    vx_float32              bias,
    vx_tensor               output
    );
vxnne_shader_executable vxnneGetNormalizationUint8ShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               type_s,
    vx_scalar               norm_size_s,
    vx_scalar               alpha_s,
    vx_scalar               beta_s,
    vx_float32              bias,
    vx_tensor               output
    );
vxnne_shader_executable vxnneGetReorgShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               stride,
    vx_scalar               outc,
    vx_tensor               output
    );
vxnne_shader_executable vxnneGetSpace2DepthShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               stride,
    vx_scalar               outc,
    vx_tensor               output
    );
vxnne_shader_executable vxnneGetDepth2SpaceShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               block_sizes,
    vx_tensor               output
    );
vxnne_shader_executable vxnneGetSpace2BatchShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               stride,
    vx_tensor               pad,
    vx_scalar               outc,
    vx_tensor               output,
    vx_uint32*              padList
    );
vxnne_shader_executable vxnneGetBatch2SpaceShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_uint32               crop_left,
    vx_uint32               crop_top,
    vx_tensor               stride,
    vx_scalar               outc,
    vx_tensor               output
    );

vxnne_shader_executable vxnneGetShuffleChannelShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               num_group_s,
    vx_scalar               axis_s,
    vx_tensor               output
    );

vxnne_shader_executable vxnneGetBatchNormShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_uint32               axis,
    vx_tensor               input,
    vx_tensor               weights,
    vx_tensor               biases,
    vx_tensor               output
    );

vxnne_shader_executable vxnneGetRnnShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               bias,
    vx_tensor               weight,
    vx_tensor               hidden,
    vx_tensor               recurrent,
    vx_tensor               activation,
    vx_tensor               state_out,
    vx_tensor               output
    );

vxnne_shader_executable vxnneGetSvdfShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               bias,
    vx_tensor               weight,
    vx_tensor               recurrent,
    vx_tensor               activation,
    vx_tensor               rank,
    vx_tensor               state_in,
    vx_tensor               state_out,
    vx_tensor               output
    );

vxnne_shader_executable vxnneGetReverseShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               output,
    vx_uint32               axsisNum,
    vx_uint32*              axsis
    );

vxnne_shader_executable vxnneGetFloorShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               mode,
    vx_tensor               output
    );

vxnne_shader_executable vxnneGetEmbeddingLUTShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               value,
    vx_tensor               output
    );

vxnne_shader_executable vxnneGetHashLUTShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               key,
    vx_tensor               value,
    vx_tensor               hit,
    vx_tensor               output
    );

vxnne_shader_executable vxnneGetMeanStddevNormalizationShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_float32              eps,
    vx_tensor               output
    );

vxnne_shader_executable vxnneGetTensorAddMeanStdNormalizationShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               input1,
    vx_float32              eps,
    vx_tensor               output
    );

vxnne_shader_executable vxnneVertMaxPoolShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_uint32               pool_width,
    vx_uint32               pool_height,
    vx_bool                 enable_relu,
    vx_tensor               output
    );

vxnne_shader_executable vxnnePreTreatedRectShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_uint32                rois_stride,
    vx_uint32                rois_num,
    vx_uint32               imgWid,
    vx_uint32               imgHeight,
    vx_float32              spatial_scale,
    vx_tensor               output);

vxnne_shader_executable vxnneHorzMaxPoolShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               rois,
    vx_tensor               output
    );

vxnne_shader_executable vxnneDeConvolutionShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor  inputs,
    vx_tensor  weights,
    vx_tensor  bias,
    vx_scalar  padding_x,
    vx_scalar  padding_y,
    vx_scalar  overflow_policy,
    vx_scalar  rounding_policy,
    vx_scalar  a_x,
    vx_scalar  a_y,
    vx_scalar  group,
    vx_tensor  outputs
    );

vxnne_shader_executable vxnneReshuffleShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_uint32               stride_x,
    vx_uint32               stride_y,
    vx_enum                 padMode,
    vx_uint32               padConstValue,
    vx_uint32               padXLeft,
    vx_uint32               padXRight,
    vx_uint32               padYTop,
    vx_uint32               padYBottom,
    vx_tensor               output
    );

vxnne_shader_executable vxnneTensorTransposeShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_uint32               *perm,
    vx_uint32               pnum,
    vx_tensor               output
    );
vxnne_shader_executable vxnneRPNSoftMaxShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor  inputs,
    vx_tensor  outputs
    );
vxnne_shader_executable vxnneRPNRegressionShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor  scores,
    vx_tensor  bboxs,
    vx_tensor  anchors,
    vx_tensor  output,
    vx_scalar feat_stride,
    vx_scalar img_W,
    vx_scalar img_H,
    vx_scalar min_box_W,
    vx_scalar min_box_H
    );

vxnne_shader_executable vxnneRPNNmsShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor proposal,
    vx_tensor roi_indices,
    vx_scalar real_roi_t,
    vx_scalar pre_nms_topn,
    vx_scalar post_nms_topn,
    vx_scalar nms_thresh);

vxnne_shader_executable vxnneTensorConvFormatShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               output);


vxnne_shader_executable vxnneRPNRetrieveShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               rs_proposalTensor,
    vx_tensor               roiIndicesTensor,
    vx_scalar               realRoiScalar,
    vx_tensor               rs_roi_output,
    vx_tensor               rs_score_output);
vxnne_shader_executable vxnneRPNSortShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               rs_proposalTensor);

vxnne_shader_executable vxnneTensor2RowShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_int32                kernelSize_x,
    vx_int32                kernelSize_y,
    vx_int32                dilation_x,
    vx_int32                dilation_y,
    vx_int32                stride_x,
    vx_int32                stride_y,
    vx_int32                padding_x,
    vx_int32                padding_y,
    vx_int32                outputWidth,
    vx_int32                outputHeight,
    vx_tensor               output);

vxnne_shader_executable vxnneGemmShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               weight,
    vx_tensor               bias,
    vx_int32                fuseCode,
    vx_bool                 enable_2dTensor,
    vx_tensor               output);

vxnne_shader_executable vxnneGemm_noBiasShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               weight,
    vx_int32                fuseCode,
    vx_bool                 enable_2dTensor,
    vx_tensor               output);

vxnne_shader_executable vxnneL2NormSumSqrtShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               output);

vxnne_shader_executable vxnneL2NormSumScaleShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               sumTmp,
    vx_tensor               output);

vxnne_shader_executable vxnneLSTMUnitShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               w_h,
    vx_tensor               h_state_in,
    vx_tensor               c_state,
    vx_tensor               cell_clip,
    vx_bool                 enable_cifg,
    vx_bool                 enable_peephole,
    vx_bool                 enable_projection,
    vx_tensor               cell2input_weight,
    vx_tensor               cell2forget_weight,
    vx_tensor               cell2output_weight,
    vx_tensor               c_state_out,
    vx_tensor               h_state_out,
    vx_tensor               activation,
    vx_tensor               output);

vxnne_shader_executable vxnneLSTMLayerShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_float32              forget_bias,
    vx_tensor               w_h,
    vx_int32                time_step,
    vx_tensor               output,
    vx_uint32               maxComputeUnits);

vxnne_shader_executable vxnneTensorCopyShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               output);

vxnne_shader_executable vxnneROIPoolShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               input_rois,
    vx_uint32               pool_width,
    vx_uint32               pool_height,
    vx_float32              spatial_scale,
    vx_bool                 enable_relu,
    vx_tensor               output);

vxnne_shader_executable vxnneGetL2PoolingShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               poolType,
    vx_scalar               stride_x_s,
    vx_scalar               stride_y_s,
    vx_scalar               poolSizeX,
    vx_scalar               poolSizeY,
    vx_uint32               pad_left,
    vx_uint32               pad_top,
    vx_scalar               rounding,
    vx_int32                activation,
    vx_tensor               output
    );

vxnne_shader_executable vxnneGetTensorScaleShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_enum                 type,
    vx_tensor               output
    );

vxnne_shader_executable vxnneGetResizeNearestNeighborShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_enum                 type,
    vx_tensor               output
    );

vxnne_shader_executable vxnneROIRect2ROIListShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_uint32               rois_stride,
    vx_uint32               rois_num,
    vx_uint32               poolWidth,
    vx_uint32               poolHeight,
    vx_float32              spatial_scale,
    vx_uint32               slice,
    vx_tensor               split_end,
    vx_tensor               roiList);

vxnne_shader_executable vxnneGetDepthwiseConvShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               inputs,
    vx_tensor               weights,
    vx_tensor               biases,
    vx_scalar               padXLeft,
    vx_scalar               padXRight,
    vx_scalar               padYTop,
    vx_scalar               padYBottom,
    vx_enum                 padMode,
    vx_scalar               padConstant,
    vx_scalar               dilationX,
    vx_scalar               dilationY,
    vx_scalar               channel_multiplier,
    vx_scalar               relu_s,
    vx_scalar               pooling_s,
    vx_scalar               poolingX,
    vx_scalar               poolingY,
    vx_scalar               downScaleSizeRounding,
    vx_int32                strideXvalue,
    vx_int32                strideYvalue,
    vx_tensor               outputs);

vxnne_shader_executable vxnneGetLSTMUnitProjectionShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               weights,
    vx_tensor               bias,
    vx_tensor               proj_clip,
    vx_tensor               output_state_out,
    vx_tensor               output);

vxnne_shader_executable vxnneGetLSTMUnitHiddenOutShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_bool                 enable_cell_in,
    vx_tensor               input_conv,
    vx_float32              forget_bias,
    vx_tensor               cell_state_in,
    vx_tensor               output,
    vx_tensor               cell_state_out,
    vx_tensor               hidden_state_out,
    vx_tensor               hidden_conv);

vxnne_shader_executable vxnneGetLSTMUnitHiddenOutExtShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_bool                 enable_cell_in,
    vx_tensor               input_conv,
    vx_float32              forget_bias,
    vx_tensor               cell_state_in,
    vx_tensor               output,
    vx_tensor               cell_state_out,
    vx_tensor               hidden_state_out,
    vx_tensor               hidden_conv);

vxnne_shader_executable vxnneGetLSTMUnitHiddenOut_PackedShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_bool                 enable_cell_in,
    vx_tensor               input_conv,
    vx_float32              forget_bias,
    vx_tensor               cell_state_in,
    vx_tensor               output,
    vx_tensor               cell_state_out,
    vx_tensor               hidden_state_out,
    vx_tensor               hidden_conv);

vxnne_shader_executable vxnneGetLSTMUnitStateOutExtShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               inputs_conv,
    vx_float32              forget_bias,
    vx_tensor               cell_state_in,
    vx_tensor               biases,
    vx_tensor               output,
    vx_tensor               cell_state_out,
    vx_tensor               hidden_state_out,
    vx_tensor               recurrents_conv);

vxnne_shader_executable vxnneGetTensorPadShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               inputs,
    vx_scalar               padLeft,
    vx_scalar               padRight,
    vx_scalar               padTop,
    vx_scalar               padBottom,
    vx_scalar               padMode,
    vx_scalar               padConst,
    vx_tensor               outputs);

vxnne_shader_executable vxnneGetTensorPad2ShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               inputs,
    vx_scalar               padConst,
    vx_tensor               outputs,
    vx_int32                *pad_dims);

vxnne_shader_executable vxnneGetTFAvgPoolingShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               mask,
    vx_scalar               stride_x_s,
    vx_scalar               stride_y_s,
    vx_scalar               poolSizeX,
    vx_scalar               poolSizeY,
    vx_uint32               pad_x_left,
    vx_uint32               pad_y_top,
    vx_int32                activation,
    vx_tensor               output
    );

vxnne_shader_executable vxnneGetTensorMeanAxisShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_float32              axis_coef,
    vx_tensor               input,
    vx_tensor               output,
    vx_uint32               axis,
    vx_bool                 is_sum_op);

vxnne_shader_executable vxnneGetTensorCropShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_int32                start[4],
    vx_int32                stop[4],
    vx_tensor               input,
    vx_tensor               output);

vxnne_shader_executable vxnneGetTensorStridedSliceShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_int32                start[4],
    vx_int32                stop[4],
    vx_int32                stride[4],
    vx_tensor               input,
    vx_tensor               output);

vxnne_shader_executable vxnneGetPReluShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               alpha,
    vx_tensor               output);

vxnne_shader_executable vxnneGetUserShaderExecutable(
    vx_context                           context,
    vx_kernel                            kernel,
    vx_reference                         parameters[],
    vx_enum                              datatypes[],
    vx_uint32                            param_num,
    vx_uniform_s                         uniforms[],
    vx_uint32                            uniform_num,
    vx_border_mode_t *                   border_mode,
    vx_kernel_execution_parameters_t *   execution_params
    );

vxnne_shader_executable vxnneGetLayerNormShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               biases,
    vx_tensor               scale,
    vx_tensor               output,
    vx_scalar               eps);

vxnne_shader_executable vxnneGetLSTMUnitLayerNormStateOutShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               inputs_conv,
    vx_float32              forget_bias,
    vx_tensor               cell_state_in,
    vx_tensor               biases,
    vx_tensor               layer_norm_weights,
    vx_tensor               output,
    vx_tensor               cell_state_out,
    vx_tensor               hidden_state_out,
    vx_bool                 enable_cifg,
    vx_float32              cell_clip
    );

vxnne_shader_executable vxnneGetTensor2DAddShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input0,
    vx_tensor               input1,
    vx_int32                activation,
    vx_enum                 operation,
    vx_tensor               output);

vxnne_shader_executable vxnneGetTensorAbsShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               output);

vxnne_shader_executable vxnneGetTensorTRShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               inputs,
    vx_float32              a_val,
    vx_float32              b_val,
    vx_enum                 funcType,
    vx_tensor               outputs);

vxnne_shader_executable vxnneGetTensorMulSatRTEShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input0,
    vx_tensor               input1,
    vx_int32                activation,
    vx_enum                 operation,
    vx_tensor               output);

vx_status vxnneWrapUserNode(
    vx_context context,
    vx_node node,
    vxnne_user_node_type_e userNodeType
    );

vx_status vxnneComputeYUV2RGBInputParameter(
    vx_uint32 outputSize,
    vx_uint32 scale,
    vx_uint32 inputStart,
    vx_uint32 * splitNum,
    vx_uint32 * outputStarts,
    vx_uint32 * outputSizes,
    vx_uint32 * inputStarts,
    vx_uint32 * inputSizes,
    vx_uint16 * inputInitErrors,
    vx_uint16 * inputInitIntErrors
);
/*nn gpu function declair*/
vxnne_shader_executable vxnneGetGPUAvgPoolingShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               poolType,
    vx_scalar               stride_x,
    vx_scalar               stride_y,
    vx_scalar               poolSizeX,
    vx_scalar               poolSizeY,
    vx_uint32               pool_pad_x_left,
    vx_uint32               pool_pad_y_top,
    vx_uint32               pool_pad_x_right,
    vx_uint32               pool_pad_y_bottom,
    vx_scalar               rounding,
    vx_bool                 is_roi_copy,
    vx_uint32               input_width,
    vx_uint32               input_height,
    vx_bool                 enable_tf_avgPool,
    vx_tensor               output);

vxnne_shader_executable vxnneGPUTensorCopyShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               output);

vxnne_shader_executable vxnneGPUROIPoolShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               input_rois,
    vx_uint32               pool_width,
    vx_uint32               pool_height,
    vx_float32              spatial_scale,
    vx_bool                 enable_relu,
    vx_tensor               output);

vxnne_shader_executable vxnneGPUTensorTransposeShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_uint32               *perm,
    vx_uint32               pnum,
    vx_tensor               output
    );

vxnne_shader_executable vxnneGetGPUEmbeddingLUTShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               value,
    vx_tensor               output
    );

vxnne_shader_executable vxnneGPUTensor2RowShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_int32                kernelSize_x,
    vx_int32                kernelSize_y,
    vx_int32                dilation_x,
    vx_int32                dilation_y,
    vx_int32                stride_x,
    vx_int32                stride_y,
    vx_int32                padding_x,
    vx_int32                padding_y,
    vx_int32                outputWidth,
    vx_int32                outputHeight,
    vx_tensor               output);

vxnne_shader_executable vxnneGPUGemm_noBiasShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               weight,
    vx_tensor               output);

vxnne_shader_executable vxnneGPUGemmShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_bool                 enable_reorgWeights,
    vx_tensor               input,
    vx_tensor               weight,
    vx_tensor               bias,
    vx_tensor               output);

vxnne_shader_executable vxnneGetGPUDepthwiseConvShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               inputs,
    vx_tensor               weights,
    vx_tensor               biases,
    vx_scalar               padXLeft,
    vx_scalar               padXRight,
    vx_scalar               padYTop,
    vx_scalar               padYBottom,
    vx_scalar               dilationX,
    vx_scalar               dilationY,
    vx_scalar               channel_multiplier,
    vx_scalar               downScaleSizeRounding,
    vx_int32                strideXvalue,
    vx_int32                strideYvalue,
    vx_tensor               outputs);

vxnne_shader_executable vxnneGetGPUDepth2SpaceShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               block_sizes,
    vx_tensor               output);

vxnne_shader_executable vxnneGetGPUSpace2DepthShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               stride,
    vx_tensor               output);

vxnne_shader_executable vxnneGetGPUFloorShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               mode,
    vx_tensor               output
    );

vxnne_shader_executable vxnneGetGPUFullyConnectedShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_bool                 enable_cast_format,
    vx_tensor               input,
    vx_tensor               weights,
    vx_tensor               bias,
    vx_int32                activation,
    vx_tensor               output);

vxnne_shader_executable vxnneGetGPUHashLUTShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               key,
    vx_tensor               value,
    vx_tensor               hit,
    vx_tensor               output
    );

vxnne_shader_executable vxnneGPUL2NormSumSqrtShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               output);

vxnne_shader_executable vxnneGPUL2NormSumScaleShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               sumTmp,
    vx_tensor               output);

vxnne_shader_executable vxnneGetGPUReorgShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               stride,
    vx_scalar               outc,
    vx_tensor               output);

vxnne_shader_executable vxnneGetGPUL2PoolingShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               poolType,
    vx_scalar               stride_x,
    vx_scalar               stride_y,
    vx_scalar               poolSizeX,
    vx_scalar               poolSizeY,
    vx_uint32               pad_left,
    vx_uint32               pad_top,
    vx_uint32               pad_right,
    vx_uint32               pad_bottom,
    vx_scalar               rounding,
    vx_tensor               output
    );

vxnne_shader_executable vxnneGetGPUNormalizationShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               type_s,
    vx_scalar               norm_size_s,
    vx_scalar               alpha_s,
    vx_scalar               beta_s,
    vx_scalar               bias,
    vx_tensor               output);

vxnne_shader_executable vxnneGetGPUActivationShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_enum                 funcType,
    vx_tensor               input,
    vx_float32              minVal,
    vx_float32              maxVal,
    vx_tensor               output);

vxnne_shader_executable vxnneGetGPUMaxPoolingShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               poolType,
    vx_scalar               stride_x,
    vx_uint32               stride_h,
    vx_scalar               poolSizeX,
    vx_scalar               poolSizeY,
    vx_uint32               pad_left,
    vx_uint32               pad_top,
    vx_uint32               pad_right,
    vx_uint32               pad_bottom,
    vx_scalar               rounding,
    vx_tensor               output);

vxnne_shader_executable vxnneGetGPUTensorScaleShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_enum                 type,
    vx_tensor               output
    );

vxnne_shader_executable vxnneGetGPUResizeNearestNeighborShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_enum                 type,
    vx_tensor               output
    );

vxnne_shader_executable vxnneGetGPUTensorMaxValueShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_array                output
    );

vxnne_shader_executable vxnneGetGPUSoftmaxShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_scalar               beta,
    vx_tensor               input,
    vx_tensor               output
    );

vxnne_shader_executable vxnneGetGPUTensorReduceDivShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input0,
    vx_tensor               output
    );

vxnne_shader_executable vxnneGetGPURnnShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               bias,
    vx_tensor               weight,
    vx_tensor               hidden,
    vx_tensor               recurrent,
    vx_tensor               activation,
    vx_tensor               state_out,
    vx_tensor               output
    );

vxnne_shader_executable vxnneGetGPUTensorEltwiseShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input0,
    vx_tensor               input1,
    vx_int32                activation,
    vx_enum                 operation,
    vx_tensor               output);

vxnne_shader_executable vxnneGPULSTMUnitShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               w_h,
    vx_tensor               h_state_in,
    vx_tensor               c_state,
    vx_float32              cellClipValue,
    vx_bool                 enable_cifg,
    vx_bool                 enable_peephole,
    vx_bool                 enable_projection,
    vx_tensor               cell2input_weight,
    vx_tensor               cell2forget_weight,
    vx_tensor               cell2output_weight,
    vx_tensor               c_state_out,
    vx_tensor               h_state_out,
    vx_enum                 activation,
    vx_tensor               output);

vxnne_shader_executable vxnneGetGPULSTMUnitProjectionShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               weights,
    vx_tensor               bias,
    vx_float32              projClipValue,
    vx_tensor               output_state_out,
    vx_tensor               output);

vxnne_shader_executable vxnneGetGPUBatch2SpaceShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_uint32               crop_left,
    vx_uint32               crop_top,
    vx_tensor               stride,
    vx_tensor               output);

vxnne_shader_executable vxnneGetGPUSpace2BatchShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               stride,
    vx_scalar               outc,
    vx_tensor               output,
    vx_uint32*              padList);

vxnne_shader_executable vxnneGetGPUShuffleChannelShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               num_group_s,
    vx_scalar               axis_s,
    vx_tensor               output);

vxnne_shader_executable vxnneGetGPUTensorMeanAxisShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_float32              axis_coef,
    vx_tensor               input,
    vx_tensor               output,
    vx_uint32               axis);

vxnne_shader_executable vxnneGetGPUSvdfShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               bias,
    vx_tensor               weight,
    vx_tensor               recurrent,
    vx_enum                 activation,
    vx_int32                rank,
    vx_tensor               state_in,
    vx_tensor               state_out,
    vx_tensor               output
    );

vxnne_shader_executable vxnneGetGPUTensorPadShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               inputs,
    vx_scalar               padLeft,
    vx_scalar               padRight,
    vx_scalar               padTop,
    vx_scalar               padBottom,
    vx_scalar               padMode,
    vx_scalar               padConst,
    vx_tensor               outputs);

vxnne_shader_executable vxnneGetGPUTensorPad2ShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               inputs,
    vx_scalar               padConst,
    vx_tensor               outputs,
    vx_int32                *pad_dims);

vxnne_shader_executable vxnneGetGPUTensorCropShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_int32                start[4],
    vx_int32                stop[4],
    vx_tensor               input,
    vx_tensor               output);

vxnne_shader_executable vxnneGetGPUTensorStridedSliceShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_int32                start[4],
    vx_int32                stop[4],
    vx_int32                stride[4],
    vx_tensor               input,
    vx_tensor               output);

vxnne_shader_executable vxnneGetGPUPReluShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               alpha,
    vx_tensor               output);

vxnne_shader_executable vxnneGetGPUReverseShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               output,
    vx_uint32               axsisNum,
    vx_uint32*              axsis
    );

vxnne_shader_executable vxnneGPUConv2D_1x1ShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_bool                 enable_cast_format,
    vx_bool                 enable_packed_weights,
    vx_tensor               input,
    vx_tensor               weight,
    vx_tensor               bias,
    vx_tensor               output
    );

vxnne_shader_executable vxnneGPUTensorCopyROIShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_scalar               inputXOffset,
    vx_scalar               inputYOffset,
    vx_scalar               outputXOffset,
    vx_scalar               outputYOffset,
    vx_tensor               input,
    vx_tensor               output);

vxnne_shader_executable vxnneGetGPULeakyReluShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               alpha,
    vx_tensor               output);

vxnne_shader_executable vxnneGetGPUBatchNormShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_uint32               axis,
    vx_tensor               input,
    vx_tensor               weights,
    vx_tensor               biases,
    vx_tensor               output);

vxnne_shader_executable vxnneGetGPUTensorTRShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               inputs,
    vx_float32              a_val,
    vx_float32              b_val,
    vx_enum                 funcType,
    vx_tensor               outputs);

#ifdef __cplusplus
}
#endif

#endif /* __GC_VX_LAYER_H__ */

