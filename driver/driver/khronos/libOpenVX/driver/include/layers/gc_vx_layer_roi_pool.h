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


#ifndef _GC_VX_LAYER_ROI_POOL_H_
#define _GC_VX_LAYER_ROI_POOL_H_

#include <gc_vxk_common.h>
#include <gc_vx_nn_util.h>


static vx_param_description_s nn_ROIPoolLayer_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNROIPoolLayer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolLayer_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NNROIPoolLayer = {
    VX_KERNEL_ROI_POOLING_LAYER,
    "vivante.nn.roipool.layer",
    vxoBaseKernel_NNROIPoolLayer,
    nn_ROIPoolLayer_params, vxmLENGTH_OF(nn_ROIPoolLayer_params),
    VX_NULL,
    vxoNNROIPoolLayer_ValidateInput,
    vxoNNROIPoolLayer_ValidateOutput,
    vxoNNROIPoolLayer_Initializer,
    vxoNNROIPoolLayer_Deinitializer
};

static vx_param_description_s nn_ROIPoolReluLayer_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_NNROIPoolReluLayer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolReluLayer_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolReluLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolReluLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolReluLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NNROIPoolReluLayer = {
    VX_KERNEL_INTERNAL_ROI_POOLING_RELU_LAYER,
    "vivante.nn.roipoolrelu.layer",
    vxoInternalKernel_NNROIPoolReluLayer,
    nn_ROIPoolReluLayer_params, vxmLENGTH_OF(nn_ROIPoolReluLayer_params),
    VX_NULL,
    vxoNNROIPoolReluLayer_ValidateInput,
    vxoNNROIPoolReluLayer_ValidateOutput,
    vxoNNROIPoolReluLayer_Initializer,
    vxoNNROIPoolReluLayer_Deinitializer
};


#endif
