/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
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
#define VX_MAX_TEMP_TENSORS                 8
#define VX_MAX_SHADER_PARAMETERS            16
#define VX_MAX_UNIFORMS                     128

struct _vxnne_operation_s;
struct _vxnne_layer_s;
struct _vx_shader_s;

typedef vx_status (*vxnne_layer_dump_f)(struct _vxnne_layer_s* layer, int verbose);
typedef vx_status (*vxnne_layer_claim_f)(struct _vxnne_layer_s* layer, vx_uint32 opindex);
typedef vx_status (*vxnne_layer_reclaim_f)(struct _vxnne_layer_s* layer, vx_uint32 opindex);
typedef vx_status (*vxnne_layer_deinitialize_f)(struct _vxnne_layer_s* layer);

typedef struct _vxnne_layer_s
{
    vx_char                     *name;
    struct _vx_node             *node;
    vx_uint32                   num_operations;
    struct _vxnne_operation_s**  operations;

    vx_uint32                   num_temp_tensors;
    vx_tensor                   temp_tensors[VX_MAX_TEMP_TENSORS];
    vx_array                    cmdNNBuff;
    vx_array                    cmdTPBuff;

    vxnne_layer_dump_f           dump;
    vxnne_layer_claim_f          claim;
    vxnne_layer_reclaim_f        reclaim;
    vxnne_layer_deinitialize_f   deinitialize;
}
vxnne_layer_s, *vxnne_layer;


typedef enum vxnne_operation_target_e
{
    VXNNE_OPERATION_TARGET_SW = 0,
    VXNNE_OPERATION_TARGET_NN,
    VXNNE_OPERATION_TARGET_SH,
    VXNNE_OPERATION_TARGET_TP
}
vxnne_operation_target_e;

typedef enum vxnne_operator_e
{
    VXNNE_OPERATOR_CONVOLUTION = 0,
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
    VXNNE_OPERATOR_RPN_RETRIEVE,
    VXNNE_OPERATOR_RPN,
    VXNNE_OPERATOR_ROIPOOL,
    VXNNE_OPERATOR_CONCAT2,
    VXNNE_OPERATOR_CONCATINDEFINITE,
    VXNNE_OPERATOR_REORG,
    VXNNE_OPERATOR_VERTMAXPOOL,
    VXNNE_OPERATOR_HORZMAXPOOL,
    VXNNE_OPERATOR_PRETREATEDRECT,
    VXNNE_OPERATOR_BRICK,
    VXNNE_OPERATOR_DECONVOLUTION,
    VXNNE_OPERATOR_L2NORMALIZE,
    VXNNE_OPERATOR_TENSOR_COPY,
}
vxnne_operator_e;

enum vxnne_kernel_e
{
    VXNNE_KERNEL_TENSOR_ADD = 0,
    VXNNE_KERNEL_NN_LEAKY = 1,
    VXNNE_KERNEL_MAXPOOLING = 2,
    VXNNE_KERNEL_AVGPOOLING = 3,
    VXNNE_KERNEL_NORMALIZATION = 4,
    VXNNE_KERNEL_FULLYCONNECTED = 5,
    VXNNE_KERNEL_ACTIVATION_RELU = 6,
    VXNNE_KERNEL_SOFTMAX = 7,
    VXNNE_KERNEL_REORG = 8,
    VXNNE_KERNEL_BATCHNORM = 9,
    VXNNE_KERNEL_TENSOR_VERTMAXPOOL = 10,
    VXNNE_KERNEL_TENSOR_HORZMAXPOOL = 11,
    VXNNE_KERNEL_TENSOR_PRETREATEDRECT = 12,
    VXNNE_KERNEL_DECONVOLUTION = 13,
    VXNNE_KERNEL_TENSOR_SUB = 14,
    VXNNE_KERNEL_TENSOR_MUL = 15,
    VXNNE_KERNEL_TENSOR_DIV = 16,
    VXNNE_KERNEL_TENSOR_COPY = 17,
    VXNNE_KERNEL_TENSOR_RESHUFFLE = 18,
    VXNNE_KERNEL_TENSOR_TRANSPOSE = 19,
    VXNNE_KERNEL_COUNT,
};

typedef vx_status (*vxnne_operation_execute_f)(struct _vxnne_operation_s *operation);
typedef vx_status (*vxnne_operation_dump_f)(struct _vxnne_operation_s *operation, int verbose);
typedef vx_status (*vxnne_operation_deinitialize_f)(struct _vxnne_operation_s *operation);
typedef vx_status (*vxnne_operation_initialize_f)(struct _vxnne_operation_s *operation);

typedef struct _vxnne_operation_s
{
    vxnne_layer                   layer;
    vxnne_operation_target_e      target;
    vxnne_operator_e              operatorType;
    vxnne_operation_execute_f     execute;
    vxnne_operation_dump_f        dump;
    vxnne_operation_initialize_f    initialize;
    vxnne_operation_deinitialize_f  deinitialize;

}
vxnne_operation_s, *vxnne_operation;

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
    vx_uint32                           paramNum;
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
    vx_tensor                       inputs[2];
    vx_weights_biases_parameter     weights_biases;
    vx_tensor                       outputs[2];
    vx_array                        buffer;
    vx_uint32                       op_num;
    vx_bool                         multi_tensor;
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


typedef struct _vxnne_convolution_relu_pooling_operation_s
{
    vxnne_operation_s               base;
    vx_tensor                       inputs;
    vx_weights_biases_parameter     weights_biases;
    vx_tensor                       outputs;
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
    vxnne_convolution_relu_pooling_operation_s      convolution_operation;
    vxnne_tp_operation_s                            reshuffle_tp_operation;
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

typedef struct _vxnne_intput_convert_weight_operation_s
{
    vxnne_operation_s               base;
    vx_tensor                       inputs;
    vx_weights_biases_parameter     weights_biases;
    vx_bool                         enable_relu;
    vx_uint8                        output_fp_pos;
    vx_tensor                       outputs;
}
vxnne_intput_convert_weight_operation_s, *vxnne_intput_convert_weight_operation;

typedef struct _vxnne_fully_connected_relu_nne_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs;
    vx_weights_biases_parameter      weights_biases;
    vx_uint32                        pad;
    vx_uint8                         accumulator_bits;
    vx_enum                          overflow_policy;
    vx_enum                          rounding_policy;
    vx_enum                          down_scale_size_rounding;
    vx_bool                          enable_relu;
    vx_tensor                        outputs;
}
vxnne_fully_connected_relu_nne_operation_s, *vxnne_fully_connected_relu_nne_operation;

typedef struct _vxnne_fully_connected_relu_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[2];
    vxnne_intput_convert_weight_operation_s         input_convert_weight_operation;
    vxnne_fully_connected_sw_operation_s            fully_connected_operation;
    vxnne_shader_operation_s                        fully_connected_SHoperation;
    vxnne_fully_connected_relu_nne_operation_s      fully_connected_relu_operation;
    vxnne_tp_operation_s                            fully_connected_TPoperation;
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
    vxnne_shader_operation_s                        pooling_sh_operation;
}
vxnne_pooling_layer_s, *vxnne_pooling_layer;

typedef struct _vxnne_normalization_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs;
    vx_enum                          type;
    vx_uint32                        norm_size;
    vx_float32                       alpha;
    vx_float32                       beta;
    vx_tensor                        outputs;
}
vxnne_normalization_operation_s, * vxnne_normalization_operation;

typedef struct _vxnne_normalization_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_normalization_operation_s                 normalization_sw_operation;
    vxnne_shader_operation_s                        normalization_sh_operation;
    vxnne_tp_operation_s                            normalization_tp_operation;
}
vxnne_normalization_layer_s, *vxnne_normalization_layer;

typedef struct _vxnne_softmax_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs;
    vx_tensor                        outputs;
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
    vx_tensor                        *inputs;
    vx_uint32                        num;
    vx_uint32                        axis;
    vx_tensor                        outputs;
}
vxnne_concatIndefinite_sw_operation_s, *vxnne_concatIndefinite_sw_operation;

typedef struct _vxnne_concatIndefinite_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_concatIndefinite_sw_operation_s           concatIndefinite_operation;
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
    vxnne_operation                                 operations[1];
    vxnne_concat2_sw_operation_s                    concat2_operation;
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
}
vxnne_tensor_copy_s, *vxnne_tensor_copy;

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

typedef struct _vxnne_tensor_trans_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        input;
    vx_array                         perm;
    vx_scalar                        pnum;
    vx_tensor                        output;
}
vxnne_tensor_trans_operation_s, *vxnne_tensor_trans_operation;

typedef struct _vxnne_tensor_trans_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_tensor_trans_operation_s                  tensor_trans_sw_operation;
    vxnne_shader_operation_s                        tensor_trans_shader_operation;
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
    vx_tensor                       real_roi_t;

    vx_bool                         input_stage;
    vx_bool                         output_stage;
}
vxnne_tensor_rpn_nms_operation_s, *vxnne_tensor_rpn_nms_operation;

typedef struct _vxnne_tensor_rpn_retrieve_operation_s
{
    vxnne_operation_s               base;

    vx_tensor                       proposal;
    vx_tensor                       roi_indices;
    vx_tensor                       real_roi_t;
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

typedef struct _vxnne_tensor_rpn_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[4];
    vxnne_tensor_rpn_operation_s                    tensorRpnSW;
    vxnne_tensor_rpn_softmax_operation_s            tensorRpnSoftmaxSW;
    vxnne_tensor_rpn_regression_operation_s         tensorRpnRegressionSW;
    vxnne_tensor_rpn_sort_operation_s               tensorRpnSortSW;
    vxnne_tensor_rpn_nms_operation_s                tensorRpnNmsSW;
    vxnne_tensor_rpn_retrieve_operation_s           tensorRpnRetrieveSW;
    vxnne_shader_operation_s                        tensorRpnSH;
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
    vxnne_tp_operation_s                            roipool_tp_operation;
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
    vx_scalar                        accumulatorBits;
    vx_scalar                        overflowPolicy;
    vx_scalar                        roundingPolicy;
    vx_scalar                        downScaleSizeRounding;
    vx_tensor                        outputs;
}
vxnne_convolution_operation_s, *vxnne_convolution_operation;

typedef struct _vxnne_convolution_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_reshuffle_operation_s                     reshuffleSW;
    vxnne_convolution_operation_s                   convolutionSW;
}
vxnne_convolution_layer_s, *vxnne_convolution_layer;

typedef struct _vxnne_deconvolution_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs;
    vx_tensor                        weights;
    vx_tensor                        biases;
    vx_scalar                        padding_x;
    vx_scalar                        padding_y;
    vx_scalar                        overflow_policy;
    vx_scalar                        rounding_policy;
    vx_scalar                        a_x;
    vx_scalar                        a_y;
    vx_scalar                         group;
    vx_tensor                        outputs;
}
vxnne_deconvolution_operation_s, * vxnne_deconvolution_operation;

typedef struct _vxnne_deconvolution_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_deconvolution_operation_s                 deconvolution_sw_operation;
    vxnne_shader_operation_s                        deconvolution_sh_operation;
}
vxnne_deconvolution_layer_s, *vxnne_deconvolution_layer;

typedef struct _vxnne_reorg_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        inputs;
    vx_scalar                        stride;
    vx_tensor                        outputs;
}
vxnne_reorg_operation_s, * vxnne_reorg_operation;

typedef struct _vxnne_reorg_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_reorg_operation_s                         reorg_sw_operation;
    vxnne_shader_operation_s                        reorg_sh_operation;
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
    vxnne_operation                                 operations[1];
    vxnne_l2normalize_operation_s                   l2normalize_sw_operation;
    vxnne_shader_operation_s                        l2normalize_sh_operation;
}
vxnne_l2normalize_layer_s, *vxnne_l2normalize_layer;

/* export functions */
vx_status vxnneLayer_Free(struct _vxnne_layer_s* layer);

vx_status vxnneLayer_Initialize(
    vxnne_layer                 layer,
    vx_char                     *name,
    vx_node                     node,
    vxnne_operation             *operation,
    vxnne_layer_deinitialize_f  deinitialize
    );

vx_status vxnneOperation_Initialize(
    vxnne_operation_s               *operation,
    vxnne_layer                     layer,
    vxnne_operation_target_e        target,
    vxnne_operator_e                operatorType,
    vxnne_operation_execute_f       execute,
    vxnne_operation_deinitialize_f  deinitialize
    );

vx_status vxnneShaderOperation_Initialize(
    vxnne_shader_operation_s            *operation,
    vxnne_layer                         layer,
    vxnne_operator_e                    operatorType,
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

vx_status vxnneShaderExecutable_SetExecutionParameters(
    vxnne_shader_executable shaderExecutable,
    vx_kernel_execution_parameters_t *shaderParam
    );

vx_status vxnneShaderExecutable_Destroy(
    vxnne_shader_executable shaderExecutable
    );

vxnne_shader_executable vxnneGetLeakyReluShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               alpha,
    vx_tensor               output
    );
vxnne_shader_executable vxnneGetFullyConnectedShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               weights,
    vx_tensor               bias,
    vx_tensor               output);
vxnne_shader_executable vxnneGetTensorAddShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input0,
    vx_tensor               input1,
    vx_scalar               convertPolicy,
    vx_tensor               output
    );
vxnne_shader_executable vxnneGetTensorSubShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input0,
    vx_tensor               input1,
    vx_scalar               convertPolicy,
    vx_tensor               output
    );
vxnne_shader_executable vxnneGetTensorMulShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input0,
    vx_tensor               input1,
    vx_scalar               convertPolicy,
    vx_scalar               scale,
    vx_scalar               rounding,
    vx_tensor               output
    );
vxnne_shader_executable vxnneGetTensorDivShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input0,
    vx_tensor               input1,
    vx_scalar               convertPolicy,
    vx_scalar               scale,
    vx_scalar               rounding,
    vx_tensor               output
    );
vxnne_shader_executable vxnneGetAvgPoolingShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               poolType,
    vx_scalar               stride_s,
    vx_scalar               poolSizeX,
    vx_scalar               poolSizeY,
    vx_scalar               poolPadX,
    vx_scalar               poolPadY,
    vx_scalar               rounding,
    vx_tensor               output
    );
vxnne_shader_executable vxnneGetMaxPoolingShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               poolType,
    vx_scalar               stride_s,
    vx_scalar               poolSizeX,
    vx_scalar               poolSizeY,
    vx_scalar               poolPadX,
    vx_scalar               poolPadY,
    vx_scalar               rounding,
    vx_tensor               output
    );
vxnne_shader_executable vxnneGetActivationReluShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               output);
vxnne_shader_executable vxnneGetSoftmaxShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
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
vxnne_shader_executable vxnneGetBatchNormShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               eps,
    vx_tensor               mean,
    vx_tensor               variance,
    vx_tensor               gamma,
    vx_tensor               beta,
    vx_tensor               output
    );

vxnne_shader_executable vxnneVertMaxPoolShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_uint32                pool_width,
    vx_uint32                pool_height,
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
#ifdef __cplusplus
}
#endif

#endif /* __GC_VX_LAYER_H__ */

