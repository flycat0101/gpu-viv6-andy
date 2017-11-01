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


#ifndef _GC_VX_NN_EXTENSION_CONCAT_H_
#define _GC_VX_NN_EXTENSION_CONCAT_H_

#include <gc_vxk_common.h>

static vx_param_description_s nn_ConcatIndefiniteLayer_params[] = {
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
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

