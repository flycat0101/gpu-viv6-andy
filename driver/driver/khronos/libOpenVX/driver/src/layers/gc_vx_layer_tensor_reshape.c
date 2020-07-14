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
#if REGISTER_FRAME
VX_PRIVATE_API vx_status vxoNNReshape_SW_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_tensor  dims                       = (vx_tensor)parameters[1];
    vx_tensor  outputs                    = (vx_tensor)parameters[2];
    vx_uint32  batchCount                 = 1;
    vxnne_reshape_layer reshapeLayer = (vxnne_reshape_layer)ops_layer;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxnneOperation_Initialize(&reshapeLayer->reshape_sw_operation.base,
        &reshapeLayer->base,
        VXNNE_OPERATION_TARGET_SW,
        VXNNE_OPERATOR_TENSOR_RESHAPE,
        vxnneExecuteSWReshape,
        VX_NULL,
        batchCount,
        0));

    vxmONERROR(vxnneLayer_SetOperation(
        &reshapeLayer->base,
        &reshapeLayer->reshape_sw_operation.base,
        0));

    reshapeLayer->reshape_sw_operation.inputs           = inputs;
    reshapeLayer->reshape_sw_operation.dims             = dims;
    reshapeLayer->reshape_sw_operation.outputs          = outputs;

    vxmONERROR(vxnneOperation_AddReference(&reshapeLayer->reshape_sw_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&reshapeLayer->reshape_sw_operation.base, (vx_reference)dims, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&reshapeLayer->reshape_sw_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNReshape_SH_EVIS_Support_Ext(vx_node node, const vx_reference parameters[], vx_uint32 _num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_tensor  inputs                     = (vx_tensor)parameters[0];
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
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, _num, reg_param);

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

    if(evis)
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

    support = shExe_flag && support;

    vxoLayer_VerificationFoot(node, parameters, _num, reg_param, &support);
    return support;
}

VX_PRIVATE_API vx_status vxoNNReshape_SH_Initialize_Ext(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 _num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_status status = VX_SUCCESS;
    vxnne_reshape_layer reshapeLayer = (vxnne_reshape_layer)ops_layer;
    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_tensor  outputs                    = (vx_tensor)parameters[2];
    vx_uint32  batchCount                 = 1;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vx_tensor input      = NULL;
    vx_tensor output     = NULL;
    vx_uint32 sizes[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32 dims = 0;
    vxoLayer_InitializeHead(ops_layer, parameters, _num, reg_param);

    vxoElementOptimization_GetTensorShape(inputs, sizes, &dims);

    input     = vxoTensor_ReshapeTensor(inputs, (vx_int32*)sizes, dims);
    output     = vxoTensor_ReshapeTensor(outputs, (vx_int32*)sizes, dims);

    reshapeLayer->base.temp_tensors[0] = input;
    reshapeLayer->base.temp_tensors[1] = output;
    reshapeLayer->base.num_temp_tensors = 2;

    if(evis)
    {
        if (input && output)
            shaderExecutable = vxnneTensorCopyShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_COPY, &ops_layer->node->kernelAttributes.borderMode, input, output);
    }
    else
    {
        if (input && output)
            shaderExecutable = vxnneGPUTensorCopyShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_COPY, &ops_layer->node->kernelAttributes.borderMode, input, output);
    }

    if (!shaderExecutable)
    {
        status = VX_FAILURE;
        goto OnError;
    }

    vxmONERROR(vxnneShaderOperation_Initialize(&reshapeLayer->tensor_copy_sh_operation,
        &reshapeLayer->base,
        VXNNE_OPERATOR_CONVERT_FORMAT,
        batchCount,
        shaderExecutable));

    vxmONERROR(vxnneOperation_AddReference(&reshapeLayer->tensor_copy_sh_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&reshapeLayer->tensor_copy_sh_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    vxmONERROR(vxnneLayer_SetOperation(
        &reshapeLayer->base,
        &reshapeLayer->tensor_copy_sh_operation.base,
        0));
OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, _num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNReshape_SH_EVIS_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && node->base.context->evisNoInst.supportEVIS;

    if (!support)return support;

    support = support && vxoNNReshape_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_true_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNReshape_SH_EVIS_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    status = vxoNNReshape_SH_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_true_e);

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNReshape_SH_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    support = support && vxoNNReshape_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_false_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNReshape_SH_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    status = vxoNNReshape_SH_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_false_e);

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNReshape_TP_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_tensor  outputs                    = (vx_tensor)parameters[2];
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

    vx_context           context          = vxGetContext((vx_reference)node);

    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_TP, VX_TYPE_INVALID, VX_NULL);
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
    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    support = support && vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_TP);
    support = support && (src_elementCount == dst_elementCount);
    support = support && vxnneIsTPSupportFormat(context, inputs, VX_NULL, outputs);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNReshape_TP_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_tensor  outputs                    = (vx_tensor)parameters[2];
    vx_uint32  dimCount0                  = 0;
    vx_uint32  width0                     = 0;
    vx_uint32  height0                    = 0;
    vx_uint32  depth0                     = 0;
    vx_uint32  batch0                     = 0;
    vx_uint32  batchCount                 = 1;
    vx_tensor     src       = NULL;
    vx_tensor     dst       = NULL;
    vx_int32      sizes[4]  = {0};
    vx_op_param_s conv      = {0};
    vxnne_reshape_layer reshapeLayer = (vxnne_reshape_layer)ops_layer;

    dimCount0    = TENSOR_VIEW_DIM_NUM(inputs);
    width0       = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
    height0      = (dimCount0 > 1) ? TENSOR_VIEW_SIZE_INDEX(inputs, 1) : 1;
    depth0       = (dimCount0 > 2) ? TENSOR_VIEW_SIZE_INDEX(inputs, 2) : 1;
    batch0       = (dimCount0 > 3) ? TENSOR_VIEW_SIZE_INDEX(inputs, 3) : 1;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    sizes[0]   = gcmMAX(gcmMAX(width0, height0), depth0);
    sizes[1]   = gcmMAX(gcmMIN(width0, height0), gcmMIN(gcmMAX(width0, height0), depth0));
    sizes[2]   = gcmMIN(gcmMIN(width0, height0), depth0) * batch0;
    sizes[3]   = 1;

    src     = vxoTensor_ReshapeTensor(inputs, sizes, 3);
    dst     = vxoTensor_ReshapeTensor(outputs, sizes, 3);
    reshapeLayer->base.temp_tensors[0]      = src;
    reshapeLayer->base.temp_tensors[1]      = dst;
    reshapeLayer->base.num_temp_tensors     = 2;


    vxmONERROR(vxnneOperation_Initialize(&reshapeLayer->tensor_copy_tp_operation.base,
        &reshapeLayer->base,
        VXNNE_OPERATION_TARGET_TP,
        VXNNE_OPERATOR_TENSOR_COPY,
        VX_NULL,
        VX_NULL,
        batchCount,
        0));

    memset(&conv, 0, sizeof(vx_op_param_s));

    conv.enable_relu = vx_false_e;
    conv.pool_stride = 1;
    conv.tpType = TP_TENSOR_COPY;

    memcpy(&reshapeLayer->tensor_copy_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));

    vxmONERROR(vxnneOperation_AddReference(&reshapeLayer->tensor_copy_tp_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&reshapeLayer->tensor_copy_tp_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    reshapeLayer->tensor_copy_tp_operation.input = src;
    reshapeLayer->tensor_copy_tp_operation.output = dst;

    vxmONERROR(vxnneLayer_SetOperation(
        &reshapeLayer->base,
        &reshapeLayer->tensor_copy_tp_operation.base,
        0));
OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_status vxoNNLayer_GetOperations(vxnne_layer ops_layer, vx_uint32_ptr max_num_operations, vxnne_operation **operations)
{
    vx_status  status = VX_SUCCESS;

    vxnne_reshape_layer reshapeLayer = (vxnne_reshape_layer)ops_layer;

    *max_num_operations = gcmCOUNTOF(reshapeLayer->operations);

    *operations = reshapeLayer->operations;

    return status;
}
#endif

VX_PRIVATE_API vx_status VX_CALLBACK vxoReshape_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{

    vx_status status = VX_SUCCESS;
#if REGISTER_FRAME
    vxnne_layer_imp_s registerReshape[] = {/* Please DON'T adjust the order, it's importent */
        { "Reshape NN", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "Reshape TP", vxoNNReshape_TP_Support, vxoNNReshape_TP_Initialize, VX_NULL },
        { "Reshape SH EVIS", vxoNNReshape_SH_EVIS_Support, vxoNNReshape_SH_EVIS_Initialize, VX_NULL },
        { "Reshape SH F32", vxoNNReshape_SH_Support, vxoNNReshape_SH_Initialize, VX_NULL },
        { "Reshape SW ", vxoNNCommon_Support, vxoNNReshape_SW_Initialize, VX_NULL },
    };

    REGISTER_LAYERS(registerReshape, vxnne_reshape_layer_s, "Reshape", vxoNNLayer_GetOperations);

OnError:
#else
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
#endif

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

