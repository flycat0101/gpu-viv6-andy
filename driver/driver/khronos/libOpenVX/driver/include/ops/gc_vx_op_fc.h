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


#ifndef __GC_VX_OP_FC_H__
#define __GC_VX_OP_FC_H__

#include <gc_vx_common.h>
#include <gc_vx_layer.h>

/* Moved to gc_vx_layer.h. */

vx_status vxoFCOperation_Initialize(
    vxnne_fc_operation operation,
    vxnne_layer layer,
    vx_tensor inputs,
    vx_weights_biases_parameter *weights_biases,
    vx_tensor weights,
    vx_tensor biases,
    vx_enum overflow_policy, /* type of vx_convert_policy_e */
    vx_enum rounding_policy, /* type of vx_round_policy_e */
    vx_bool enable_relu,
    vx_tensor outputs,
    vx_uint32_ptr op_index
    );

vx_status vxoFCOperation_Deinitialize(
    vxnne_fc_operation operation
    );

/*
 * TP Fullyconnected operation.
 *
 * If kz exceeds the maximum value of TP KZ size (2^16 - 1), it
 * will be broken down into 2 seperated operations:
 * 1. One FC operation which generates 2 half-kz-size of FC commands.
 * 2. One Sum filter.
 */
vx_status vxoFCOperationTP_Initialize(
    vxnne_tp_operation operation,
    vxnne_tp_operation aux_operation,
    vxnne_layer layer,
    vx_tensor inputs,
    vx_uint32 batch_count,
    vx_weights_biases_parameter weights_biases,
    vx_enum overflow_policy,
    vx_enum rounding_policy,
    vx_bool enable_relu,
    vx_tensor outputs,
    vx_uint32_ptr op_index
    );

vx_status vxoFCOperationNN_Initialize(
    vxnne_convolution_relu_pooling_operation operation,
    vxnne_convolution_operation operation_sw,
    vxnne_layer layer,
    vx_tensor inputs,
    vx_uint32 batch_count,
    vx_weights_biases_parameter weights_biases,
    vx_enum overflow_policy,
    vx_enum rounding_policy,
    vx_bool enable_relu,
    vx_tensor outputs,
    vx_uint32_ptr op_index
    );

vx_status vxoFCOperationSH_Initialize(
    vxnne_shader_operation operation,
    vxnne_layer layer,
    vx_tensor inputs,
    vx_uint32 batch_count,
    vx_tensor weights,
    vx_tensor biases,
    vx_enum overflow_policy,
    vx_enum rounding_policy,
    vx_bool enable_relu,
    vx_tensor outputs,
    vx_uint32_ptr op_index
    );

vx_status vxoFCOperationSW_Initialize(
    vxnne_fully_connected_sw_operation operation,
    vxnne_layer layer,
    vx_tensor inputs,
    vx_uint32 batch_count,
    vx_tensor weights,
    vx_tensor biases,
    vx_enum overflow_policy,
    vx_enum rounding_policy,
    vx_bool enable_relu,
    vx_tensor outputs,
    vx_uint32_ptr op_index
    );

vx_status vxoFCOperationTP(
    vxnne_fc_operation operation,
    vxnne_layer layer,
    vx_tensor inputs,
    vx_weights_biases_parameter *weights_biases,
    vx_tensor weights,
    vx_tensor biases,
    vx_enum overflow_policy, /* type of vx_convert_policy_e */
    vx_enum rounding_policy, /* type of vx_round_policy_e */
    vx_bool enable_relu,
    vx_tensor outputs,
    vx_uint32_ptr op_index,
    vx_uint32 batch_count
    );

vx_status vxoFCOperationNN(
    vxnne_fc_operation operation,
    vxnne_layer layer,
    vx_tensor inputs,
    vx_weights_biases_parameter *weights_biases,
    vx_tensor weights,
    vx_tensor biases,
    vx_enum overflow_policy, /* type of vx_convert_policy_e */
    vx_enum rounding_policy, /* type of vx_round_policy_e */
    vx_bool enable_relu,
    vx_tensor outputs,
    vx_uint32_ptr op_index,
    vx_uint32 batch_count
    );

#endif

