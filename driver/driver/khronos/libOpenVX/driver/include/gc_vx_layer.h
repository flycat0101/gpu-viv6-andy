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
    VXNNE_OPERATOR_SOFTMAX,
    VXNNE_OPERATOR_NORMALIZATION,
    VXNNE_OPERATOR_BATCHNORM,
    VXNNE_OPERATOR_INPUT2WEIGHT,
    VXNNE_OPERATOR_RPN,
    VXNNE_OPERATOR_ROIPOOL,
    VXNNE_OPERATOR_CONCAT2,
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



/* customized declaration*/
typedef struct _vxnne_reshuffle_operation_s
{
    vxnne_operation_s               base;
    vx_tensor                       inputs;
    vx_weights_biases_parameter     weights_biases;
    vx_scalar                       pad_x_s;
    vx_scalar                       pad_y_s;
    vx_tensor                       outputs;
}
vxnne_reshuffle_operation_s, *vxnne_reshuffle_operation;

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
    vxnne_operation                                 operations[2];
    vxnne_reshuffle_operation_s                     reshuffle_operation;
    vxnne_shader_operation_s                        reshuffle_shader_operation;
    vxnne_resize_operation_s                        resize_operation;
    vxnne_convolution_relu_pooling_operation_s      convolution_operation;
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

    vx_tensor                        outputs;
}
vxnne_batchnorm_sw_operation_s, *vxnne_batchnorm_sw_operation;

typedef struct _vxnne_batchnorm_layer_s
{
    vxnne_layer_s                                  base;
    vxnne_operation                                operations[1];
    vxnne_batchnorm_sw_operation_s                 batchnorm_operation;
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
    vx_uint32                        pool_pad_x;
    vx_uint32                        pool_pad_y;
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
    vxnne_pooling_operation_s                       pooling_tp_operation;
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
    vxnne_shader_operation_s                        normalization_sh_operation;  //#zk
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
}
vxnne_softmax_layer_s, *vxnne_softmax_layer;

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

typedef struct _vxnne_tensor_rpn_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        score;
    vx_tensor                        bbox;
    vx_tensor                        output;
}
vxnne_tensor_rpn_operation_s, *vxnne_tensor_rpn_operation;

typedef struct _vxnne_tensor_rpn_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_tensor_rpn_operation_s                    tensorRpnSW;
    vxnne_shader_operation_s                        tensorRpnSH;
}
vxnne_tensor_rpn_layer_s, *vxnne_tensor_rpn_layer;

typedef struct _vxnne_tensor_roipool_operation_s
{
    vxnne_operation_s                base;
    vx_tensor                        input_data;
    vx_tensor                        input_rois;
    vx_scalar                        pool_types;
    vx_scalar                        spatial_scales;
    vx_tensor                        output;
}
vxnne_tensor_roipool_operation_s, *vxnne_tensor_roipool_operation;

typedef struct _vxnne_tensor_roipool_layer_s
{
    vxnne_layer_s                                   base;
    vxnne_operation                                 operations[1];
    vxnne_tensor_roipool_operation_s                tensorROIPoolSW;
    vxnne_shader_operation_s                        tensorROIPoolSH;
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
    );  //#zk
#ifdef __cplusplus
}
#endif

#endif /* __GC_VX_LAYER_H__ */

