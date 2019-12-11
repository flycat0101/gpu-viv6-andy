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



#include <gc_vx_common.h>
#include <layers/gc_vx_layer_tensor_squeeze.h>

/***************************************************************************************************************************
*                                                 TENSOR SQUEEZE
***************************************************************************************************************************/

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorSqueeze(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorSqueeze_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorSqueeze_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxnneExecuteSWTensorSqueeze(struct _vxnne_operation_s *operation)
{
    vxnne_tensor_squeeze_operation transOperation = (vxnne_tensor_squeeze_operation)operation;

    vx_tensor input        = (vx_tensor)transOperation->input;
    /*vx_tensor squeeze_dims = (vx_tensor)transOperation->squeeze_dims;*/
    vx_tensor output       = (vx_tensor)transOperation->output;
    vx_size size = 0;
    vx_uint8_ptr input_base = VX_NULL, output_base = VX_NULL;

    vxoTensor_GetTensorWholeSize(input, &size);
    vxoTensor_GetTensorViewMemory(input, (gctPOINTER*)&input_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(output, (gctPOINTER*)&output_base, VX_NULL);

    memcpy(output_base, input_base, (vx_uint32)size);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorSqueeze_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vx_context context = vxGetContext((vx_reference)node);

    vx_tensor input         = (vx_tensor)parameters[0];
    vx_tensor squeeze_dims  = (vx_tensor)parameters[1];
    vx_tensor output        = (vx_tensor)parameters[2];
    vx_uint32 batchCount = TENSOR_SIZE_INDEX(input, 3);
    vx_enum   inputFormat   = TENSOR_DATA_TYPE(input);
    vx_enum   outputFormat  = TENSOR_DATA_TYPE(output);
    vx_bool   shExe_flag    = vx_false_e;

    vxnne_tensor_squeeze_layer tensor_squeeze_layer = VX_NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_tensor_squeeze_layer_s), (gctPOINTER*)&tensor_squeeze_layer);
    if (!tensor_squeeze_layer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(tensor_squeeze_layer, sizeof(vxnne_tensor_squeeze_layer_s));

    vxnneLayer_Initialize(&tensor_squeeze_layer->base,
        "TensorSqueeze",
        node,
        vxmOPERATION_COUNT(tensor_squeeze_layer),
        tensor_squeeze_layer->operations,
        VX_NULL);

    if(context->evisNoInst.supportEVIS)
    {
        shExe_flag = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
                            || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16)
                            || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8)
                            || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
                            || (inputFormat == VX_TYPE_BFLOAT16 && outputFormat == VX_TYPE_BFLOAT16)
                            || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT16));
    }
    else
    {
        shExe_flag = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
                            || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32)
                            || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
                            || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT32)
                            || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT16));
    }

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP) &&
        vxnneIsTPSupportFormat(context, input, VX_NULL, output))
    {
        vx_op_param_s conv = { 0 };

        conv.pad_x_left = 0;
        conv.pad_y_top = 0;
        conv.pool_size_x = 0;
        conv.pool_size_y = 0;
        conv.pool_stride = 1;
        conv.enable_relu = vx_false_e;
        conv.pad_mode = VX_PAD_CONSTANT;
        conv.pad_const = 0;

        vxnneOperation_Initialize(&tensor_squeeze_layer->tensor_squeeze_tp_operation.base,
            &tensor_squeeze_layer->base,
            VXNNE_OPERATION_TARGET_TP,
            VXNNE_OPERATOR_TENSOR_SQUEEZE,
            VX_NULL,
            vxnneOperation_TP_Deinitialize,
            batchCount,
            0);

        vxnneLayer_SetOperation(
            &tensor_squeeze_layer->base,
            &tensor_squeeze_layer->tensor_squeeze_tp_operation.base,
            0);

        tensor_squeeze_layer->tensor_squeeze_tp_operation.input = input;
        tensor_squeeze_layer->tensor_squeeze_tp_operation.output = output;

        vxnneOperation_AddReference(&tensor_squeeze_layer->tensor_squeeze_tp_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&tensor_squeeze_layer->tensor_squeeze_tp_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        conv.tpType = TP_TENSOR_SQUEEZE;
        conv.other_ref = (vx_reference)input;
        conv.data_buff = gcvNULL;

        memcpy(&tensor_squeeze_layer->tensor_squeeze_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));
    }
    else
    {
        if (shExe_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
        {
            vxnne_shader_executable shaderExecutable = VX_NULL;
            vx_tensor  src          = NULL;
            vx_tensor  dst          = NULL;
            vx_uint32 sizes[VX_CONTEXT_TENSOR_MAX_DIMENSION];
            vx_uint32 dims = 0;

            vxoElementOptimization_GetTensorShape(input, sizes, &dims);

            src        = vxoTensor_ReshapeTensor(input, (vx_int32*)sizes, dims);
            dst        = vxoTensor_ReshapeTensor(output, (vx_int32*)sizes, dims);

            tensor_squeeze_layer->base.temp_tensors[0]  = src;
            tensor_squeeze_layer->base.temp_tensors[1]  = dst;
            tensor_squeeze_layer->base.num_temp_tensors = 2;

            if(node->base.context->evisNoInst.supportEVIS)
            {
                shaderExecutable = vxnneTensorCopyShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_COPY, &node->kernelAttributes.borderMode, src, dst);
            }
            else
            {
                shaderExecutable = vxnneGPUTensorCopyShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_COPY, &node->kernelAttributes.borderMode, src, dst);
            }
            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto exit;
            }

            status = vxnneShaderOperation_Initialize(&tensor_squeeze_layer->tensor_squeeze_sh_operation,
                &tensor_squeeze_layer->base,
                VXNNE_OPERATOR_TENSOR_COPY,
                1,
                shaderExecutable);

            if (status != VX_SUCCESS)
                goto exit;

            vxnneOperation_AddReference(&tensor_squeeze_layer->tensor_squeeze_sh_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&tensor_squeeze_layer->tensor_squeeze_sh_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            vxnneLayer_SetOperation(&tensor_squeeze_layer->base, &tensor_squeeze_layer->tensor_squeeze_sh_operation.base, 0);
        }
        else
        {
            vxnneOperation_Initialize(&tensor_squeeze_layer->tensor_squeeze_sw_operation.base,
                &tensor_squeeze_layer->base,
                VXNNE_OPERATION_TARGET_SW,
                VXNNE_OPERATOR_TENSOR_SQUEEZE,
                vxnneExecuteSWTensorSqueeze,
                VX_NULL,
                batchCount,
                0);

            vxnneLayer_SetOperation(
                &tensor_squeeze_layer->base,
                &tensor_squeeze_layer->tensor_squeeze_sw_operation.base,
                0);

            tensor_squeeze_layer->tensor_squeeze_sw_operation.input         = input;
            tensor_squeeze_layer->tensor_squeeze_sw_operation.squeeze_dims  = squeeze_dims;
            tensor_squeeze_layer->tensor_squeeze_sw_operation.output        = output;

            vxnneOperation_AddReference(&tensor_squeeze_layer->tensor_squeeze_sw_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&tensor_squeeze_layer->tensor_squeeze_sw_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }
    }

    node->layer = &tensor_squeeze_layer->base;
    return status;

exit:
    if (tensor_squeeze_layer)
        gcoOS_Free(NULL, (gctPOINTER)tensor_squeeze_layer);

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorSqueeze_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

