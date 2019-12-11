/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef _GC_VX_LAYER_TENSOR_SQUEEZE_H_
#define _GC_VX_LAYER_TENSOR_SQUEEZE_H_

#include <gc_vxk_common.h>
#include <gc_vx_nn_util.h>


VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorSqueeze(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorSqueeze_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorSqueeze_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorSqueeze_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorSqueeze_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

static vx_param_description_s nn_TensorSqueeze_params[] = {
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_true_e },
    { VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e }
};

vx_kernel_description_s internalkernel_NNTensorSqueeze = {
    VX_KERNEL_NN_TENSOR_SQUEEZE,
    "vivante.nn.tensor.squeeze",
    vxoBaseKernel_NNTensorSqueeze,
    nn_TensorSqueeze_params, vxmLENGTH_OF(nn_TensorSqueeze_params),
    VX_NULL,
    vxoNNTensorSqueeze_ValidateInput,
    vxoNNTensorSqueeze_ValidateOutput,
    vxoNNTensorSqueeze_Initializer,
    vxoNNTensorSqueeze_Deinitializer
};


#endif

