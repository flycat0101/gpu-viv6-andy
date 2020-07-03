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


#ifndef _GC_VX_LAYER_SOFTMAX_H_
#define _GC_VX_LAYER_SOFTMAX_H_

#include <gc_vxk_common.h>
#include <gc_vx_nn_util.h>


static vx_param_description_s nn_Softmax_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNSoftmaxLayer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSoftmaxLayer_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSoftmaxLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSoftmaxLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSoftmaxLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s internalkernel_NNSoftmaxLayer = {
    VX_KERNEL_SOFTMAX_LAYER,
    "vivante.nn.softmax.layer",
    vxoBaseKernel_NNSoftmaxLayer,
    nn_Softmax_params, vxmLENGTH_OF(nn_Softmax_params),
    VX_NULL,
    vxoSoftmaxLayer_ValidateInput,
    vxoSoftmaxLayer_ValidateOutput,
    vxoSoftmaxLayer_Initializer,
    vxoSoftmaxLayer_Deinitializer
};



static vx_param_description_s nn_softmaxlayer2_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_true_e},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e}
};
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNSoftmaxLayer2(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSoftmaxLayer2_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSoftmaxLayer2_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSoftmaxLayer2_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSoftmaxLayer2_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s internalkernel_NNSoftmaxLayer2 = {
    VX_KERNEL_NN_SOFTMAX2_LAYER,
    "vivante.nn.softmaxlayer2.layer",
    vxoNNSoftmaxLayer2,
    nn_softmaxlayer2_params, vxmLENGTH_OF(nn_softmaxlayer2_params),
    VX_NULL,
    vxoSoftmaxLayer2_ValidateInput,
    vxoSoftmaxLayer2_ValidateOutput,
    vxoSoftmaxLayer2_Initializer,
    vxoSoftmaxLayer2_Deinitializer
};



#endif

