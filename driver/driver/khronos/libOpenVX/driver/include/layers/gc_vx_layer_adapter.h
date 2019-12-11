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


#ifndef _GC_VX_LAYER_ADAPTER_H_
#define _GC_VX_LAYER_ADAPTER_H_

#include <gc_vxk_common.h>

static vx_param_description_s nn_adapter_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e}
};
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNAdapter(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoAdapter_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoAdapter_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoAdapter_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoAdapter_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s internalkernel_NNAdapter = {
    VX_KERNEL_INTERNAL_ADAPTER,
    "vivante.nn.adapter.layer",
    vxoNNAdapter,
    nn_adapter_params, vxmLENGTH_OF(nn_adapter_params),
    VX_NULL,
    vxoAdapter_ValidateInput,
    vxoAdapter_ValidateOutput,
    vxoAdapter_Initializer,
    vxoAdapter_Deinitializer
};

#endif

