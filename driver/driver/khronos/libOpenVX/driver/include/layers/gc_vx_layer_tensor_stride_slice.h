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


#ifndef _GC_VX_LAYER_TENSOR_STRIDE_SLICE_H_
#define _GC_VX_LAYER_TENSOR_STRIDE_SLICE_H_

#include <gc_vxk_common.h>
#include <gc_vx_nn_util.h>


VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorStrideSlice(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorStrideSlice_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorStrideSlice_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorStrideSlice_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorStrideSlice_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

static vx_param_description_s nn_TensorStrideSlice_params[] = {
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e }
};

vx_kernel_description_s internalkernel_NNTensorStrideSlice = {
    VX_KERNEL_NN_TENSOR_STRIDE_SLICE,
    "vivante.nn.tensor.stride_slice",
    vxoBaseKernel_NNTensorStrideSlice,
    nn_TensorStrideSlice_params, vxmLENGTH_OF(nn_TensorStrideSlice_params),
    VX_NULL,
    vxoNNTensorStrideSlice_ValidateInput,
    vxoNNTensorStrideSlice_ValidateOutput,
    vxoNNTensorStrideSlice_Initializer,
    vxoNNTensorStrideSlice_Deinitializer
};

#endif

