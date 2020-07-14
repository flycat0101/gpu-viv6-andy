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


#ifndef _GC_VX_LAYER_TENSOR_ELTWISE_H_
#define _GC_VX_LAYER_TENSOR_ELTWISE_H_

#include <gc_vxk_common.h>
#include <gc_vx_nn_util.h>


static vx_param_description_s nn_TensorAdd_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e}
};

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

vx_kernel_description_s internalkernel_NNTensorAdd = {
    VX_KERNEL_TENSOR_ADD,
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
    VX_KERNEL_TENSOR_SUBTRACT,
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
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e}
};

vx_kernel_description_s internalkernel_NNTensorMul = {
    VX_KERNEL_TENSOR_MULTIPLY,
    "vivante.nn.tensor.mul",
    vxoBaseKernel_NNTensorMul,
    nn_TensorMul_params, vxmLENGTH_OF(nn_TensorMul_params),
    VX_NULL,
    vxoNNTensorMul_ValidateInput,
    vxoNNTensorMul_ValidateOutput,
    vxoNNTensorMul_Initializer,
    vxoNNTensorMul_Deinitializer
};


#endif

