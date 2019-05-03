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


#include <ops/gc_vx_op_tensor_copy.h>
#include <gc_vx_nn_util.h>

extern vx_status vxnneExecuteSWTensorCopy(struct _vxnne_operation_s *operation);
extern vx_status vxnneOperation_TP_Deinitialize(vxnne_operation_s *operation);
extern vx_status vxnneOperation_AddReference(
    vxnne_operation_s*            operation,
    vx_reference                  reference,
    vxnne_operation_reference_e   refType
    );

vx_status vxoTensorCopyOperation_Initialize(
    vxnne_tensor_copy_operation operation,
    vxnne_layer layer,
    vx_tensor inputs,
    vx_tensor outputs,
    vx_uint32_ptr op_index
    )
{
    vx_status status = VX_SUCCESS;

    vx_node node = layer->node;
    vx_context context = vxGetContext((vx_reference)node);
    vx_uint32 dim_num = TENSOR_VIEW_DIM_NUM(inputs);
    vxnne_operation_target_e target = VXNNE_OPERATION_TARGET_NONE;
    vx_uint32 batch_count = (dim_num > 3) ? TENSOR_VIEW_SIZE_INDEX(inputs, dim_num - 1) : 1;

    /* Choose the accelerator. */
    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP) &&
        vxnneIsTPSupportFormat(context, inputs, VX_NULL, outputs))
    {
        target = VXNNE_OPERATION_TARGET_TP;
    }
    else if (0 && vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_SHADER))
    {
        target = VXNNE_OPERATION_TARGET_SH;
    }
    else
    {
        target = VXNNE_OPERATION_TARGET_SW;
    }

    target = VXNNE_OPERATION_TARGET_SW;

    switch (target)
    {
    case VXNNE_OPERATION_TARGET_TP:
        vxmONERROR(vxoTensorCopyOperationTP_Initialize(&operation->tp_operation,
                                                       layer,
                                                       inputs,
                                                       batch_count,
                                                       outputs,
                                                       op_index));
        break;

    case VXNNE_OPERATION_TARGET_SH:
        vxmONERROR(vxoTensorCopyOperationSH_Initialize(&operation->sh_operation,
                                                       layer,
                                                       inputs,
                                                       outputs,
                                                       op_index));
        break;

    case VXNNE_OPERATION_TARGET_SW:
        vxmONERROR(vxoTensorCopyOperationSW_Initialize(&operation->sw_operation,
                                                       layer,
                                                       inputs,
                                                       batch_count,
                                                       outputs,
                                                       op_index));
        break;

    default:
        vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
        break;
    }

OnError:
    return status;
}

vx_status vxoTensorCopyOperation_Deinitialize(
    vxnne_tensor_copy_operation operation
    )
{
    vx_status status = VX_SUCCESS;

    return status;
}

vx_status vxoTensorCopyOperationTP_Initialize(
    vxnne_tp_operation operation,
    vxnne_layer layer,
    vx_tensor inputs,
    vx_uint32 batch_count,
    vx_tensor outputs,
    vx_uint32_ptr op_index
    )
{
    vx_status status = VX_SUCCESS;

    vx_op_param op_param = VX_NULL;

    if (!op_index)
    {
        vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
    }

    vxmONERROR(vxnneOperation_Initialize(&operation->base,
                                         layer,
                                         VXNNE_OPERATION_TARGET_TP,
                                         VXNNE_OPERATOR_TENSOR_COPY,
                                         VX_NULL,
                                         vxnneOperation_TP_Deinitialize,
                                         1,
                                         0));

    op_param = &operation->base.parameter;

    op_param->enable_relu = vx_false_e;
    op_param->tpType = TP_TENSOR_COPY;
    op_param->pool_stride = 1;

    operation->input = inputs;
    operation->output = outputs;

    vxmONERROR(vxnneLayer_SetOperation(layer,
                                       &operation->base,
                                       (*op_index)++));

    vxnneOperation_AddReference(&operation->base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
    vxnneOperation_AddReference(&operation->base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

OnError:
    return status;
}

vx_status vxoTensorCopyOperationSH_Initialize(
    vxnne_shader_operation operation,
    vxnne_layer layer,
    vx_tensor src,
    vx_tensor dst,
    vx_uint32_ptr op_index
    )
{
    vx_status status = VX_SUCCESS;

    vx_node node = layer->node;
    vxnne_shader_executable shaderExecutable;
    vx_tensor input      = NULL;
    vx_tensor output     = NULL;
    vx_int32   sizes[4]  = {0};
    vx_enum    inputFormat             = TENSOR_DATA_TYPE(src);
    vx_enum    outputFormat            = TENSOR_DATA_TYPE(dst);
    vx_bool    shExe_flag              = vx_false_e;
    vx_uint32 src_elementCount = 0;
    vx_uint32 dst_elementCount = 0;
    vx_uint32 dimCount0        = 0;
    vx_uint32 width0           = 0;
    vx_uint32 height0          = 0;
    vx_uint32 depth0           = 0;
    vx_uint32 batch0           = 0;
    vx_uint32 dimCount1        = 0;
    vx_uint32 width1           = 0;
    vx_uint32 height1          = 0;
    vx_uint32 depth1           = 0;
    vx_uint32 batch1           = 0;

    dimCount0    = TENSOR_VIEW_DIM_NUM(src);
    width0       = TENSOR_VIEW_SIZE_INDEX(src, 0);
    height0      = (dimCount0 > 1) ? TENSOR_VIEW_SIZE_INDEX(src, 1) : 1;
    depth0       = (dimCount0 > 2) ? TENSOR_VIEW_SIZE_INDEX(src, 2) : 1;
    batch0       = (dimCount0 > 3) ? TENSOR_VIEW_SIZE_INDEX(src, 3) : 1;
    src_elementCount = width0 * height0 * depth0 * batch0;

    dimCount1    = TENSOR_VIEW_DIM_NUM(dst);
    width1       = TENSOR_VIEW_SIZE_INDEX(dst, 0);
    height1      = (dimCount1 > 1) ? TENSOR_VIEW_SIZE_INDEX(dst, 1) : 1;
    depth1       = (dimCount1 > 2) ? TENSOR_VIEW_SIZE_INDEX(dst, 2) : 1;
    batch1       = (dimCount1 > 3) ? TENSOR_VIEW_SIZE_INDEX(dst, 3) : 1;
    dst_elementCount = width1 * height1 * depth1 * batch1;

    shExe_flag = (vx_bool)(((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
                         || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16)
                         || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8)
                         || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
                         || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT16))
                         && src_elementCount == dst_elementCount);

    sizes[0]   = gcmMAX(gcmMAX(width0, height0), depth0);
    sizes[1]   = gcmMAX(gcmMIN(width0, height0), gcmMIN(gcmMAX(width0, height0), depth0));
    sizes[2]   = gcmMIN(gcmMIN(width0, height0), depth0) * batch0;
    sizes[3]   = 1;

    input     = vxoTensor_ReshapeTensor(src, sizes, dimCount0);

    sizes[0]   = gcmMAX(gcmMAX(width1, height1), depth1);
    sizes[1]   = gcmMAX(gcmMIN(width1, height1), gcmMIN(gcmMAX(width1, height1), depth1));
    sizes[2]   = gcmMIN(gcmMIN(width1, height1), depth1) * batch1;
    sizes[3]   = 1;

    output     = vxoTensor_ReshapeTensor(dst, sizes, dimCount1);

    shaderExecutable = vxnneTensorCopyShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_COPY, &node->kernelAttributes.borderMode, input, output);

    if (input) vxoTensor_ReleaseTensor(&input);
    if (output) vxoTensor_ReleaseTensor(&output);

    if (!shaderExecutable)
    {
        vxmONERROR(VX_FAILURE);
    }

    vxmONERROR(vxnneShaderOperation_Initialize(operation,
                                               layer,
                                               VXNNE_OPERATOR_CONVERT_FORMAT,
                                               1, /*batchCount is 1 after reshape tensor object*/
                                               shaderExecutable));


    vxmONERROR(vxnneLayer_SetOperation(layer,
                                       &operation->base,
                                       (*op_index)++));

    vxnneOperation_AddReference(&operation->base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT);
    vxnneOperation_AddReference(&operation->base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT);

OnError:
    return status;
}

vx_status vxoTensorCopyOperationSW_Initialize(
    vxnne_tensor_copy_sw_operation operation,
    vxnne_layer layer,
    vx_tensor inputs,
    vx_uint32 batch_count,
    vx_tensor outputs,
    vx_uint32_ptr op_index
    )
{
    vx_status status = VX_SUCCESS;

    if (!op_index)
    {
        vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
    }

    vxmONERROR(vxnneOperation_Initialize(&operation->base,
                                         layer,
                                         VXNNE_OPERATION_TARGET_SW,
                                         VXNNE_OPERATOR_TENSOR_COPY,
                                         vxnneExecuteSWTensorCopy,
                                         VX_NULL,
                                         batch_count,
                                         0));

    operation->src = inputs;
    operation->dst = outputs;

    vxmONERROR(vxnneLayer_SetOperation(layer,
                                       &operation->base,
                                       (*op_index)++));

    vxnneOperation_AddReference(&operation->base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
    vxnneOperation_AddReference(&operation->base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

OnError:
    return status;
}

