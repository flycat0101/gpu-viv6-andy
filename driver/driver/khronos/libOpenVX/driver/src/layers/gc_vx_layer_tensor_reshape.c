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
#include <layers/gc_vx_layer_tensor_reshape.h>


/***************************************************************************************************************************
 *                                                 Tensor Reshape
 ***************************************************************************************************************************/
vx_status vxnneExecuteSWReshape(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_reshape_operation reshapeOperation = (vxnne_reshape_operation)operation;

    vx_tensor input  = reshapeOperation->inputs;
    vx_tensor output = reshapeOperation->outputs;
    vx_tensor dims   = reshapeOperation->dims;

    vx_int32 in_size = (vx_int32)vxoMemory_ComputeElementCount(&input->tensorBuffer->memory, 0);
    vx_int32 out_size = (vx_int32)vxoMemory_ComputeElementCount(&output->tensorBuffer->memory, 0);
    vx_int32 i = 0;

    vx_type_e input_format  = (vx_type_e)TENSOR_DATA_TYPE(input);
    vx_type_e output_format  = (vx_type_e)TENSOR_DATA_TYPE(output);

    vx_uint8_ptr input_ptr  = TENSOR_LOGICAL_ADDR(input);
    vx_uint8_ptr output_ptr = TENSOR_LOGICAL_ADDR(output);

    vx_enum count = dims->dimCount;
    vx_int32 reshape_size = 1;
    vx_int32_ptr dim = (dims != VX_NULL)?(vx_int32_ptr)TENSOR_LOGICAL_ADDR(dims):VX_NULL;

    if ((dim == VX_NULL) || ((dim != VX_NULL) && (count == 1) && (dim[i] == -1)))
    {
        reshape_size = in_size;
    }
    else
    {
        if (dim != VX_NULL)
        {
            for (i = 0; i < count; i ++)
            {
                reshape_size *= dim[i];
            }
        }
    }

    if ((reshape_size != in_size) || (reshape_size > out_size) || (output_format != input_format))
       vxError("Invalid parament! reshape_size = %d, in_size = %d, out_size = %d, output_format = %d, input_format = %d", reshape_size, in_size, out_size, output_format, input_format);

    memcpy(output_ptr, input_ptr, reshape_size * vxDataType_GetSize(input_format));

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNReshape(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoReshape_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxoReshape_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoReshape_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{

    vx_status status = VX_SUCCESS;
    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_tensor  dims                       = (vx_tensor)parameters[1];
    vx_tensor  outputs                    = (vx_tensor)parameters[2];
    vx_enum    inputFormat                = TENSOR_DATA_TYPE(inputs);
    vx_enum    outputFormat               = TENSOR_DATA_TYPE(outputs);
    vx_bool    shExe_flag                 = vx_false_e;
    vx_uint32  src_elementCount           = 0;
    vx_uint32  dst_elementCount           = 0;
    vx_uint32  dimCount0                  = 0;
    vx_uint32  width0                     = 0;
    vx_uint32  height0                    = 0;
    vx_uint32  depth0                     = 0;
    vx_uint32  batch0                     = 0;
    vx_uint32  dimCount1                  = 0;
    vx_uint32  width1                     = 0;
    vx_uint32  height1                    = 0;
    vx_uint32  depth1                     = 0;
    vx_uint32  batch1                     = 0;
    vx_uint32  batchCount                 = 1;


    vxnne_reshape_layer  reshapeLayer     = VX_NULL;
    vx_context           context          = vxGetContext((vx_reference)node);

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }
    gcoOS_Allocate(gcvNULL, sizeof(vxnne_reshape_layer_s), (gctPOINTER*)&reshapeLayer);
    if (!reshapeLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("Out of Memory at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(reshapeLayer, sizeof(vxnne_reshape_layer_s));

    vxnneLayer_Initialize(&reshapeLayer->base,
                          "Reshape",
                          node,
                          vxmOPERATION_COUNT(reshapeLayer),
                          reshapeLayer->operations,
                          VX_NULL);

    dimCount0    = TENSOR_VIEW_DIM_NUM(inputs);
    width0       = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
    height0      = (dimCount0 > 1) ? TENSOR_VIEW_SIZE_INDEX(inputs, 1) : 1;
    depth0       = (dimCount0 > 2) ? TENSOR_VIEW_SIZE_INDEX(inputs, 2) : 1;
    batch0       = (dimCount0 > 3) ? TENSOR_VIEW_SIZE_INDEX(inputs, 3) : 1;
    src_elementCount = width0 * height0 * depth0 * batch0;

    dimCount1    = TENSOR_VIEW_DIM_NUM(outputs);
    width1       = TENSOR_VIEW_SIZE_INDEX(outputs, 0);
    height1      = (dimCount1 > 1) ? TENSOR_VIEW_SIZE_INDEX(outputs, 1) : 1;
    depth1       = (dimCount1 > 2) ? TENSOR_VIEW_SIZE_INDEX(outputs, 2) : 1;
    batch1       = (dimCount1 > 3) ? TENSOR_VIEW_SIZE_INDEX(outputs, 3) : 1;
    dst_elementCount = width1 * height1 * depth1 * batch1;

    if(context->evisNoInst.supportEVIS)
    {
        shExe_flag = (vx_bool)(((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
            || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16)
            || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8)
            || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
            || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT16)
            || (inputFormat == VX_TYPE_BFLOAT16 && outputFormat == VX_TYPE_BFLOAT16))
            && src_elementCount == dst_elementCount);
    }
    else
    {
        shExe_flag = (vx_bool)(((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
            || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32)
            || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
            || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT32)
            || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT16))
            && src_elementCount == dst_elementCount);
    }

    if (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_TP) &&
        (src_elementCount == dst_elementCount) &&
        vxnneIsTPSupportFormat(context, inputs, VX_NULL, outputs))
    {
        vx_tensor     src       = NULL;
        vx_tensor     dst       = NULL;
        vx_int32      sizes[4]  = {0};
        vx_op_param_s conv      = {0};

        sizes[0]   = gcmMAX(gcmMAX(width0, height0), depth0);
        sizes[1]   = gcmMAX(gcmMIN(width0, height0), gcmMIN(gcmMAX(width0, height0), depth0));
        sizes[2]   = gcmMIN(gcmMIN(width0, height0), depth0) * batch0;
        sizes[3]   = 1;

        src     = vxoTensor_ReshapeTensor(inputs, sizes, 3);
        dst     = vxoTensor_ReshapeTensor(outputs, sizes, 3);
        reshapeLayer->base.temp_tensors[0]      = src;
        reshapeLayer->base.temp_tensors[1]      = dst;
        reshapeLayer->base.num_temp_tensors     = 2;


        status = vxnneOperation_Initialize(&reshapeLayer->tensor_copy_tp_operation.base,
            &reshapeLayer->base,
            VXNNE_OPERATION_TARGET_TP,
            VXNNE_OPERATOR_TENSOR_COPY,
            VX_NULL,
            VX_NULL,
            batchCount,
            0);

        if (status != VX_SUCCESS) goto exit;

        memset(&conv, 0, sizeof(vx_op_param_s));

        conv.enable_relu = vx_false_e;
        conv.pool_stride = 1;
        conv.tpType = TP_TENSOR_COPY;

        memcpy(&reshapeLayer->tensor_copy_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));

        vxnneOperation_AddReference(&reshapeLayer->tensor_copy_tp_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&reshapeLayer->tensor_copy_tp_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        reshapeLayer->tensor_copy_tp_operation.input = src;
        reshapeLayer->tensor_copy_tp_operation.output = dst;

        vxnneLayer_SetOperation(
            &reshapeLayer->base,
            &reshapeLayer->tensor_copy_tp_operation.base,
            0);
    }
    else if (shExe_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)) )
    {
        vxnne_shader_executable shaderExecutable = VX_NULL;
        vx_tensor input      = NULL;
        vx_tensor output     = NULL;
        vx_uint32 sizes[VX_CONTEXT_TENSOR_MAX_DIMENSION];
        vx_uint32 dims = 0;

        vxoElementOptimization_GetTensorShape(inputs, sizes, &dims);

        input     = vxoTensor_ReshapeTensor(inputs, (vx_int32*)sizes, dims);
        output     = vxoTensor_ReshapeTensor(outputs, (vx_int32*)sizes, dims);

        reshapeLayer->base.temp_tensors[0] = input;
        reshapeLayer->base.temp_tensors[1] = output;
        reshapeLayer->base.num_temp_tensors = 2;

        if(node->base.context->evisNoInst.supportEVIS)
        {
            if (input && output)
                shaderExecutable = vxnneTensorCopyShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_COPY, &node->kernelAttributes.borderMode, input, output);
        }
        else
        {
            if (input && output)
                shaderExecutable = vxnneGPUTensorCopyShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_COPY, &node->kernelAttributes.borderMode, input, output);
        }

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }
        status = vxnneShaderOperation_Initialize(&reshapeLayer->tensor_copy_sh_operation,
            &reshapeLayer->base,
            VXNNE_OPERATOR_CONVERT_FORMAT,
            batchCount,
            shaderExecutable);

        if (status != VX_SUCCESS)
            goto exit;

        vxnneOperation_AddReference(&reshapeLayer->tensor_copy_sh_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&reshapeLayer->tensor_copy_sh_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        vxnneLayer_SetOperation(
            &reshapeLayer->base,
            &reshapeLayer->tensor_copy_sh_operation.base,
            0);
    }
    else
    {
        vxnneOperation_Initialize(&reshapeLayer->reshape_sw_operation.base,
            &reshapeLayer->base,
            VXNNE_OPERATION_TARGET_SW,
            VXNNE_OPERATOR_TENSOR_RESHAPE,
            vxnneExecuteSWReshape,
            VX_NULL,
            batchCount,
            0);

        vxnneLayer_SetOperation(
            &reshapeLayer->base,
            &reshapeLayer->reshape_sw_operation.base,
            0);

        reshapeLayer->reshape_sw_operation.inputs           = inputs;
        reshapeLayer->reshape_sw_operation.dims             = dims;
        reshapeLayer->reshape_sw_operation.outputs          = outputs;

        vxnneOperation_AddReference(&reshapeLayer->reshape_sw_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&reshapeLayer->reshape_sw_operation.base, (vx_reference)dims, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&reshapeLayer->reshape_sw_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }

    node->layer = &reshapeLayer->base;


    return status;
exit:
    if (reshapeLayer != NULL)
        gcoOS_Free(NULL, reshapeLayer);

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoReshape_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

