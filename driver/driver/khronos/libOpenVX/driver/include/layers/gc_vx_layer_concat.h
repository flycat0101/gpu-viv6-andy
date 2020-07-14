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


#ifndef _GC_VX_LAYER_CONCAT_H_
#define _GC_VX_LAYER_CONCAT_H_

#include <gc_vxk_common.h>
#include <gc_vx_nn_util.h>

static vx_param_description_s nn_Concat2Layer_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e}
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

static vx_param_description_s nn_ConcatIndefiniteLayer_params[] = {
    {VX_INPUT, VX_TYPE_OBJECT_ARRAY, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNConcatIndefiniteLayer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcatIndefiniteLayer_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcatIndefiniteLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcatIndefiniteLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcatIndefiniteLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NNConcatIndefiniteLayer = {
    VX_KERNEL_NN_CONCATINDEFINITE_LAYER,
    "vivante.nn.concat.indefinite.layer",
    vxoBaseKernel_NNConcatIndefiniteLayer,
    nn_ConcatIndefiniteLayer_params, vxmLENGTH_OF(nn_ConcatIndefiniteLayer_params),
    VX_NULL,
    vxoNNConcatIndefiniteLayer_ValidateInput,
    vxoNNConcatIndefiniteLayer_ValidateOutput,
    vxoNNConcatIndefiniteLayer_Initializer,
    vxoNNConcatIndefiniteLayer_Deinitializer
};

#endif

