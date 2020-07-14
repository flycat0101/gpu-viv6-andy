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


#ifndef __GC_VX_OP_TENSOR_COPY_H__
#define __GC_VX_OP_TENSOR_COPY_H__

#include <gc_vx_common.h>
#include <gc_vx_layer.h>

/* Moved to gc_vx_layer.h. */

vx_status vxoTensorCopyOperation_Initialize(
    vxnne_tensor_copy_operation operation,
    vxnne_layer layer,
    vx_tensor inputs,
    vx_tensor outputs,
    vx_uint32_ptr op_index
    );

vx_status vxoTensorCopyOperation_Deinitialize(
    vxnne_tensor_copy_operation operation
    );

vx_status vxoTensorCopyOperationTP_Initialize(
    vxnne_tp_operation operation,
    vxnne_layer layer,
    vx_tensor inputs,
    vx_uint32 batch_count,
    vx_tensor outputs,
    vx_uint32_ptr op_index
    );

vx_status vxoTensorCopyOperationSH_Initialize(
    vxnne_shader_operation operation,
    vxnne_layer layer,
    vx_tensor inputs,
    vx_tensor outputs,
    vx_uint32_ptr op_index
    );

vx_status vxoTensorCopyOperationSW_Initialize(
    vxnne_tensor_copy_sw_operation operation,
    vxnne_layer layer,
    vx_tensor inputs,
    vx_uint32 batch_count,
    vx_tensor outputs,
    vx_uint32_ptr op_index
    );

#endif

