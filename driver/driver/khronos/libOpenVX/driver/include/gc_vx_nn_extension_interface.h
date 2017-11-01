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


#ifndef _GC_VX_NN_EXTENSION_INTERFACE_H_
#define _GC_VX_NN_EXTENSION_INTERFACE_H_

#include <gc_vxk_common.h>

/**************************************************nn extension*************************************************************/
static vx_param_description_s nn_ConvolutionReluLayer_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_WEIGHTS_BIASES_PARAMETER, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNConvolutionReluLayer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluLayer_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NNConvolutionReluCnnLayer = {
    VX_KERNEL_NN_CONVOLUTION_RELU_LAYER,
    "vivante.nn.convolution.relu.layer",
    vxoBaseKernel_NNConvolutionReluLayer,
    nn_ConvolutionReluLayer_params, vxmLENGTH_OF(nn_ConvolutionReluLayer_params),
    VX_NULL,
    vxoNNConvolutionReluLayer_ValidateInput,
    vxoNNConvolutionReluLayer_ValidateOutput,
    vxoNNConvolutionReluLayer_Initializer,
    vxoNNConvolutionReluLayer_Deinitializer
};

static vx_param_description_s nn_ConvolutionReluPoolingLayer_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_WEIGHTS_BIASES_PARAMETER, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNConvolutionReluPoolingLayer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NNConvolutionReluPoolingCnnLayer = {
    VX_KERNEL_NN_CONVOLUTION_RELU_POOLING_LAYER,
    "vivante.nn.convolution.relu.pooling.layer",
    vxoBaseKernel_NNConvolutionReluPoolingLayer,
    nn_ConvolutionReluPoolingLayer_params, vxmLENGTH_OF(nn_ConvolutionReluPoolingLayer_params),
    VX_NULL,
    vxoNNConvolutionReluPoolingLayer_ValidateInput,
    vxoNNConvolutionReluPoolingLayer_ValidateOutput,
    vxoNNConvolutionReluPoolingLayer_Initializer,
    vxoNNConvolutionReluPoolingLayer_Deinitializer
};

static vx_param_description_s nn_ConvolutionReluPoolingLayer2_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_WEIGHTS_BIASES_PARAMETER, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNConvolutionReluPoolingLayer2(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer2_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer2_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer2_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer2_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NNConvolutionReluPoolingCnnLayer2 = {
    VX_KERNEL_NN_CONVOLUTION_RELU_POOLING_LAYER2,
    "vivante.nn.convolution.relu.pooling.layer2",
    vxoBaseKernel_NNConvolutionReluPoolingLayer2,
    nn_ConvolutionReluPoolingLayer2_params, vxmLENGTH_OF(nn_ConvolutionReluPoolingLayer2_params),
    VX_NULL,
    vxoNNConvolutionReluPoolingLayer2_ValidateInput,
    vxoNNConvolutionReluPoolingLayer2_ValidateOutput,
    vxoNNConvolutionReluPoolingLayer2_Initializer,
    vxoNNConvolutionReluPoolingLayer2_Deinitializer
};

static vx_param_description_s nn_FullyConnectedReluLayer_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_WEIGHTS_BIASES_PARAMETER, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNFullyConnectedReluLayer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedReluLayer_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedReluLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedReluLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedReluLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NNFullyConnectedReluLayer = {
    VX_KERNEL_NN_FULLY_CONNECTED_RELU_LAYER,
    "vivante.nn.fullyconnected.relu.layer",
    vxoBaseKernel_NNFullyConnectedReluLayer,
    nn_FullyConnectedReluLayer_params, vxmLENGTH_OF(nn_FullyConnectedReluLayer_params),
    VX_NULL,
    vxoNNFullyConnectedReluLayer_ValidateInput,
    vxoNNFullyConnectedReluLayer_ValidateOutput,
    vxoNNFullyConnectedReluLayer_Initializer,
    vxoNNFullyConnectedReluLayer_Deinitializer
};

static vx_param_description_s nn_Softmax_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNSoftmaxLayer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSoftmaxLayer_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSoftmaxLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSoftmaxLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSoftmaxLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s internalkernel_NNSoftmaxLayer = {
    VX_KERNEL_NN_SOFTMAX_LAYER,
    "vivante.nn.softmax.layer",
    vxoBaseKernel_NNSoftmaxLayer,
    nn_Softmax_params, vxmLENGTH_OF(nn_Softmax_params),
    VX_NULL,
    vxoSoftmaxLayer_ValidateInput,
    vxoSoftmaxLayer_ValidateOutput,
    vxoSoftmaxLayer_Initializer,
    vxoSoftmaxLayer_Deinitializer
};

static vx_param_description_s nn_Normalization_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_ENUM, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNNormalization(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNormalization_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNormalization_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNormalization_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNormalization_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s internalkernel_NNNormalization = {
    VX_KERNEL_NN_NORMALIZATION_LAYER,
    "vivante.nn.normalization.layer",
    vxoNNNormalization,
    nn_Normalization_params, vxmLENGTH_OF(nn_Normalization_params),
    VX_NULL,
    vxoNormalization_ValidateInput,
    vxoNormalization_ValidateOutput,
    vxoNormalization_Initializer,
    vxoNormalization_Deinitializer
};

static vx_param_description_s nn_NormalizeImage_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNNormalizeImage(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNormalizeImage_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNormalizeImage_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalkernel_NNNormalizeImage = {
    VX_KERNEL_NN_NORMALIZE_IMAGE_LAYER,
    "vivante.nn.cnnnormalizeimage",
    vxoNNNormalizeImage,
    nn_NormalizeImage_params, vxmLENGTH_OF(nn_NormalizeImage_params),
    VX_NULL,
    vxoNormalizeImage_ValidateInput,
    vxoNormalizeImage_ValidateOutput
};

static vx_param_description_s nn_PoolingLayer_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNPoolingLayer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NNPoolingLayer = {
    VX_KERNEL_NN_POOLING_LAYER,
    "vivante.nn.pooling.layer",
    vxoBaseKernel_NNPoolingLayer,
    nn_PoolingLayer_params, vxmLENGTH_OF(nn_PoolingLayer_params),
    VX_NULL,
    vxoNNPoolingLayer_ValidateInput,
    vxoNNPoolingLayer_ValidateOutput,
    vxoNNPoolingLayer_Initializer,
    vxoNNPoolingLayer_Deinitializer
};

static vx_param_description_s nn_ConvolutionLayer_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNConvolutionLayer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionLayer_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NNConvolutionLayer = {
    VX_KERNEL_NN_CONVOLUTION_LAYER,
    "vivante.nn.convolution.layer",
    vxoBaseKernel_NNConvolutionLayer,
    nn_ConvolutionLayer_params, vxmLENGTH_OF(nn_ConvolutionLayer_params),
    VX_NULL,
    vxoNNConvolutionLayer_ValidateInput,
    vxoNNConvolutionLayer_ValidateOutput,
    vxoNNConvolutionLayer_Initializer,
    vxoNNConvolutionLayer_Deinitializer
};
static vx_param_description_s nn_FullyConnectedLayer_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNFullyConnectedLayer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedLayer_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NNFullyConnectedLayer = {
    VX_KERNEL_NN_FULLY_CONNECTED_LAYER,
    "vivante.nn.fullyconnected.layer",
    vxoBaseKernel_NNFullyConnectedLayer,
    nn_FullyConnectedLayer_params, vxmLENGTH_OF(nn_FullyConnectedLayer_params),
    VX_NULL,
    vxoNNFullyConnectedLayer_ValidateInput,
    vxoNNFullyConnectedLayer_ValidateOutput,
    vxoNNFullyConnectedLayer_Initializer,
    vxoNNFullyConnectedLayer_Deinitializer
};

static vx_param_description_s nn_ActivationLayer_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNActivationLayer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNActivationLayer_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNActivationLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNActivationLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNActivationLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NNActivationLayer = {
    VX_KERNEL_NN_ACTIVATION_LAYER,
    "vivante.nn.activation.layer",
    vxoBaseKernel_NNActivationLayer,
    nn_ActivationLayer_params, vxmLENGTH_OF(nn_ActivationLayer_params),
    VX_NULL,
    vxoNNActivationLayer_ValidateInput,
    vxoNNActivationLayer_ValidateOutput,
    vxoNNActivationLayer_Initializer,
    vxoNNActivationLayer_Deinitializer
};

static vx_param_description_s nn_LeakyReluLayer_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNLeakyReluLayer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNLeakyReluLayer_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNLeakyReluLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNLeakyReluLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNLeakyReluLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NNLeakyReluLayer = {
    VX_KERNEL_NN_LEAKY,
    "vivante.nn.leaky.layer",
    vxoBaseKernel_NNLeakyReluLayer,
    nn_LeakyReluLayer_params, vxmLENGTH_OF(nn_LeakyReluLayer_params),
    VX_NULL,
    vxoNNLeakyReluLayer_ValidateInput,
    vxoNNLeakyReluLayer_ValidateOutput,
    vxoNNLeakyReluLayer_Initializer,
    vxoNNLeakyReluLayer_Deinitializer
};

static vx_param_description_s nn_BatchNormLayer_params[] = {
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNBatchNormalizationLayer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNBatchNormalizationLayer_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNBatchNormalizationLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNBatchNormalizationLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNBatchNormalizationLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NNBatchNormLayer = {
    VX_KERNEL_NN_BATCH_NORM,
    "vivante.nn.batchnormalization.layer",
    vxoBaseKernel_NNBatchNormalizationLayer,
    nn_BatchNormLayer_params, vxmLENGTH_OF(nn_BatchNormLayer_params),
    VX_NULL,
    vxoNNBatchNormalizationLayer_ValidateInput,
    vxoNNBatchNormalizationLayer_ValidateOutput,
    vxoNNBatchNormalizationLayer_Initializer,
    vxoNNBatchNormalizationLayer_Deinitializer
};

static vx_param_description_s nn_TensorAdd_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorEltwise(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorEltwise_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorEltwise_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorEltwise_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorEltwise_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorAdd(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorAdd_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorAdd_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorAdd_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorAdd_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorSub(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorSub_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorSub_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorSub_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorSub_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorMul(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorMul_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorMul_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorMul_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorMul_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorDiv(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorDiv_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorDiv_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorDiv_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorDiv_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NNTensorAdd = {
    VX_KERNEL_NN_TENSOR_ADD,
    "vivante.nn.tensor.add",
    vxoBaseKernel_NNTensorAdd,
    nn_TensorAdd_params, vxmLENGTH_OF(nn_TensorAdd_params),
    VX_NULL,
    vxoNNTensorAdd_ValidateInput,
    vxoNNTensorAdd_ValidateOutput,
    vxoNNTensorAdd_Initializer,
    vxoNNTensorAdd_Deinitializer
};

vx_kernel_description_s internalkernel_NNTensorSub = {
    VX_KERNEL_NN_TENSOR_SUB,
    "vivante.nn.tensor.sub",
    vxoBaseKernel_NNTensorSub,
    nn_TensorAdd_params, vxmLENGTH_OF(nn_TensorAdd_params),
    VX_NULL,
    vxoNNTensorSub_ValidateInput,
    vxoNNTensorSub_ValidateOutput,
    vxoNNTensorSub_Initializer,
    vxoNNTensorSub_Deinitializer
};

static vx_param_description_s nn_TensorMul_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};

vx_kernel_description_s internalkernel_NNTensorMul = {
    VX_KERNEL_NN_TENSOR_MUL,
    "vivante.nn.tensor.mul",
    vxoBaseKernel_NNTensorMul,
    nn_TensorMul_params, vxmLENGTH_OF(nn_TensorMul_params),
    VX_NULL,
    vxoNNTensorMul_ValidateInput,
    vxoNNTensorMul_ValidateOutput,
    vxoNNTensorMul_Initializer,
    vxoNNTensorMul_Deinitializer
};

static vx_param_description_s nn_TensorDiv_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};

vx_kernel_description_s internalkernel_NNTensorDiv = {
    VX_KERNEL_NN_TENSOR_DIV,
    "vivante.nn.tensor.div",
    vxoBaseKernel_NNTensorDiv,
    nn_TensorDiv_params, vxmLENGTH_OF(nn_TensorDiv_params),
    VX_NULL,
    vxoNNTensorDiv_ValidateInput,
    vxoNNTensorDiv_ValidateOutput,
    vxoNNTensorDiv_Initializer,
    vxoNNTensorDiv_Deinitializer
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorTrans(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorTrans_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorTrans_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorTrans_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorTrans_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

static vx_param_description_s nn_TensorTrans_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};

vx_kernel_description_s internalkernel_NNTensorTrans = {
    VX_KERNEL_NN_TENSOR_TRANS,
    "vivante.nn.tensor.transpose",
    vxoBaseKernel_NNTensorTrans,
    nn_TensorTrans_params, vxmLENGTH_OF(nn_TensorTrans_params),
    VX_NULL,
    vxoNNTensorTrans_ValidateInput,
    vxoNNTensorTrans_ValidateOutput,
    vxoNNTensorTrans_Initializer,
    vxoNNTensorTrans_Deinitializer
};

static vx_param_description_s nn_RpnLayer_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNRPNLayer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNRPNLayer_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNRPNLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNRPNLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNRPNLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NNRPNLayer = {
    VX_KERNEL_NN_RPN,
    "vivante.nn.rpn.layer",
    vxoBaseKernel_NNRPNLayer,
    nn_RpnLayer_params, vxmLENGTH_OF(nn_RpnLayer_params),
    VX_NULL,
    vxoNNRPNLayer_ValidateInput,
    vxoNNRPNLayer_ValidateOutput,
    vxoNNRPNLayer_Initializer,
    vxoNNRPNLayer_Deinitializer
};

static vx_param_description_s nn_ROIPoolLayer_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNROIPoolLayer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolLayer_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NNROIPoolLayer = {
    VX_KERNEL_NN_ROIPOOL,
    "vivante.nn.roipool.layer",
    vxoBaseKernel_NNROIPoolLayer,
    nn_ROIPoolLayer_params, vxmLENGTH_OF(nn_ROIPoolLayer_params),
    VX_NULL,
    vxoNNROIPoolLayer_ValidateInput,
    vxoNNROIPoolLayer_ValidateOutput,
    vxoNNROIPoolLayer_Initializer,
    vxoNNROIPoolLayer_Deinitializer
};

static vx_param_description_s nn_Concat2Layer_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNConcat2Layer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcat2Layer_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcat2Layer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcat2Layer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcat2Layer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NNConcat2Layer = {
    VX_KERNEL_NN_CONCAT2_LAYER,
    "vivante.nn.concat.layer",
    vxoBaseKernel_NNConcat2Layer,
    nn_Concat2Layer_params, vxmLENGTH_OF(nn_Concat2Layer_params),
    VX_NULL,
    vxoNNConcat2Layer_ValidateInput,
    vxoNNConcat2Layer_ValidateOutput,
    vxoNNConcat2Layer_Initializer,
    vxoNNConcat2Layer_Deinitializer
};

static vx_param_description_s nn_ReorgLayer_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNReorgLayer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNReorgLayer_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNReorgLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNReorgLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNReorgLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NNReorgLayer = {
    VX_KERNEL_NN_REORG_LAYER,
    "vivante.nn.reorg.layer",
    vxoBaseKernel_NNReorgLayer,
    nn_ReorgLayer_params, vxmLENGTH_OF(nn_ReorgLayer_params),
    VX_NULL,
    vxoNNReorgLayer_ValidateInput,
    vxoNNReorgLayer_ValidateOutput,
    vxoNNReorgLayer_Initializer,
    vxoNNReorgLayer_Deinitializer
};

static vx_param_description_s nn_DeConvolutionLayer_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNDeConvolutionLayer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNDeConvolutionLayer_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNDeConvolutionLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNDeConvolutionLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNDeConvolutionLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NNDeConvolutionLayer = {
    VX_KERNEL_NN_DECONVOLUTION_LAYER,
    "vivante.nn.deconvolution.layer",
    vxoBaseKernel_NNDeConvolutionLayer,
    nn_DeConvolutionLayer_params, vxmLENGTH_OF(nn_DeConvolutionLayer_params),
    VX_NULL,
    vxoNNDeConvolutionLayer_ValidateInput,
    vxoNNDeConvolutionLayer_ValidateOutput,
    vxoNNDeConvolutionLayer_Initializer,
    vxoNNDeConvolutionLayer_Deinitializer
};

static vx_param_description_s nn_L2NormalizeLayer_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNL2NormalizeLayer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNL2NormalizeLayer_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNL2NormalizeLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNL2NormalizeLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNL2NormalizeLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NNL2NormalizeLayer = {
    VX_KERNEL_NN_L2NORMALIZE_LAYER,
    "vivante.nn.L2Normalize.layer",
    vxoBaseKernel_NNL2NormalizeLayer,
    nn_L2NormalizeLayer_params, vxmLENGTH_OF(nn_L2NormalizeLayer_params),
    VX_NULL,
    vxoNNL2NormalizeLayer_ValidateInput,
    vxoNNL2NormalizeLayer_ValidateOutput,
    vxoNNL2NormalizeLayer_Initializer,
    vxoNNL2NormalizeLayer_Deinitializer
};

static vx_param_description_s nn_TensorCopy_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorCopy(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorCopy_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorCopy_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorCopy_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorCopy_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NNTensorCopy = {
    VX_KERNEL_NN_TENSOR_COPY,
    "vivante.nn.tensorcopy",
    vxoBaseKernel_NNTensorCopy,
    nn_TensorCopy_params, vxmLENGTH_OF(nn_TensorCopy_params),
    VX_NULL,
    vxoNNTensorCopy_ValidateInput,
    vxoNNTensorCopy_ValidateOutput,
    vxoNNTensorCopy_Initializer,
    vxoNNTensorCopy_Deinitializer
};


#endif

