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


#ifndef _GC_VX_LAYER_POOL_H_
#define _GC_VX_LAYER_POOL_H_

#include <gc_vxk_common.h>
#include <gc_vx_nn_util.h>


static vx_param_description_s nn_PoolingLayer_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNPoolingLayer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NNPoolingLayer = {
    VX_KERNEL_POOLING_LAYER,
    "vivante.nn.pooling.layer",
    vxoBaseKernel_NNPoolingLayer,
    nn_PoolingLayer_params, vxmLENGTH_OF(nn_PoolingLayer_params),
    VX_NULL,
    vxoNNPoolingLayer_ValidateInput,
    vxoNNPoolingLayer_ValidateOutput,
    vxoNNPoolingLayer_Initializer,
    vxoNNPoolingLayer_Deinitializer
};


static vx_param_description_s nn_PoolingLayer_params2[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_true_e},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNPoolingLayer2(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer2_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer2_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer2_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer2_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NNPoolingLayer2 = {
    VX_KERNEL_NN_POOLING_LAYER2,
    "vivante.nn.pooling.layer2",
    vxoBaseKernel_NNPoolingLayer2,
    nn_PoolingLayer_params2, vxmLENGTH_OF(nn_PoolingLayer_params2),
    VX_NULL,
    vxoNNPoolingLayer2_ValidateInput,
    vxoNNPoolingLayer2_ValidateOutput,
    vxoNNPoolingLayer2_Initializer,
    vxoNNPoolingLayer2_Deinitializer
};



#endif

