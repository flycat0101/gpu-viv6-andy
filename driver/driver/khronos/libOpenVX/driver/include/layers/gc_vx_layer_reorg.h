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


#ifndef _GC_VX_LAYER_REORG_H_
#define _GC_VX_LAYER_REORG_H_

#include <gc_vxk_common.h>
#include <gc_vx_nn_util.h>


static vx_param_description_s nn_ReorgLayer_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e}
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


static vx_param_description_s nn_reorg2_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_true_e },
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e}
};
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNReOrg2(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoReOrg2_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoReOrg2_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoReOrg2_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoReOrg2_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s internalkernel_NNReOrg2 = {
    VX_KERNEL_NN_REORG2_LAYER,
    "vivante.nn.reorg2.layer",
    vxoNNReOrg2,
    nn_reorg2_params, vxmLENGTH_OF(nn_reorg2_params),
    VX_NULL,
    vxoReOrg2_ValidateInput,
    vxoReOrg2_ValidateOutput,
    vxoReOrg2_Initializer,
    vxoReOrg2_Deinitializer
};

#endif
