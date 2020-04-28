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
#include <layers/gc_vx_layer_tensor_reverse.h>

vx_status CPUTensorReverse(vx_tensor src, vx_tensor dst, vx_uint32 axis)
{
    vx_type_e srcFormat = (vx_type_e)TENSOR_DATA_TYPE(src);
    vx_enum srcQuantFormat = TENSOR_QUANT_TYPE(src);
    vx_int8 srcFp = TENSOR_POS(src);
    vx_int32 srcZp = TENSOR_TF_ZEROPOINT(src);
    vx_float32 srcScale = TENSOR_TF_SCALE(src);

    vx_type_e dstFormat = (vx_type_e)TENSOR_DATA_TYPE(dst);
    vx_enum dstQuantFormat = TENSOR_QUANT_TYPE(dst);
    vx_int8 dstFp = TENSOR_POS(dst);
    vx_int32 dstZp = TENSOR_TF_ZEROPOINT(dst);
    vx_float32 dstScale = TENSOR_TF_SCALE(dst);
    vx_enum   dstRoundingMode = TENSOR_ROUNDING_MODE(dst);

    gctPOINTER srcLogical = VX_NULL;
    gctPOINTER dstLogical = VX_NULL;

    vx_uint32 batch = src->dimCount < 4 ? 1 : TENSOR_VIEW_SIZE_INDEX(src, 3);
    vx_uint32 channel = src->dimCount < 3 ? 1 : TENSOR_VIEW_SIZE_INDEX(src, 2);
    vx_uint32 height = src->dimCount < 2 ? 1 : TENSOR_VIEW_SIZE_INDEX(src, 1);
    vx_uint32 width = TENSOR_VIEW_SIZE_INDEX(src, 0);

    vx_uint32 i, j, m, n;

    vxmASSERT(TENSOR_VIEW_SIZE_INDEX(src, 0) == TENSOR_VIEW_SIZE_INDEX(dst, 0));
    vxmASSERT(axis < src->dimCount);

    vxoTensor_GetTensorViewMemory(src, &srcLogical, VX_NULL);
    vxoTensor_GetTensorViewMemory(dst, &dstLogical, VX_NULL);

    switch (axis)
    {
    case 0:
        for (n = 0; n < batch; n++)
        {
            for (m = 0; m < channel; m++)
            {
                for (j = 0; j < height; j++)
                {
                    for (i = 0; i < width; i++)
                    {
                        vx_uint32 srcIdx = n * channel * height * width + m * height * width + j * width + i;
                        vx_uint32 dstIdx = n * channel * height * width + m * height * width + j * width + width - i - 1;

                        vx_float32 value = vxnneGetDataExt(srcFormat, srcQuantFormat, srcIdx, (vx_uint8_ptr)srcLogical, srcFp, srcZp, srcScale);
                        vxnneSaveDataExt(dstFormat, dstQuantFormat, dstIdx, value, (vx_uint8_ptr)dstLogical, dstFp, dstZp, dstScale, dstRoundingMode);
                    }
                }
            }
        }
        break;

    case 1:
        for (n = 0; n < batch; n++)
        {
            for (m = 0; m < channel; m++)
            {
                for (j = 0; j < height; j++)
                {
                    for (i = 0; i < width; i++)
                    {
                        vx_uint32 srcIdx = n * channel * height * width + m * height * width + j * width + i;
                        vx_uint32 dstIdx = n * channel * height * width + m * height * width + (height - j - 1) * width + i;

                        vx_float32 value = vxnneGetDataExt(srcFormat, srcQuantFormat, srcIdx, (vx_uint8_ptr)srcLogical, srcFp, srcZp, srcScale);
                        vxnneSaveDataExt(dstFormat, dstQuantFormat, dstIdx, value, (vx_uint8_ptr)dstLogical, dstFp, dstZp, dstScale, dstRoundingMode);
                    }
                }
            }
        }
        break;

    case 2:
        for (n = 0; n < batch; n++)
        {
            for (m = 0; m < channel; m++)
            {
                for (j = 0; j < height; j++)
                {
                    for (i = 0; i < width; i++)
                    {
                        vx_uint32 srcIdx = n * channel * height * width + m * height * width + j * width + i;
                        vx_uint32 dstIdx = n * channel * height * width + (channel - m - 1) * height * width + j * width + i;

                        vx_float32 value = vxnneGetDataExt(srcFormat, srcQuantFormat, srcIdx, (vx_uint8_ptr)srcLogical, srcFp, srcZp, srcScale);
                        vxnneSaveDataExt(dstFormat, dstQuantFormat, dstIdx, value, (vx_uint8_ptr)dstLogical, dstFp, dstZp, dstScale, dstRoundingMode);
                    }
                }
            }
        }
        break;

    case 3:
        for (n = 0; n < batch; n++)
        {
            for (m = 0; m < channel; m++)
            {
                for (j = 0; j < height; j++)
                {
                    for (i = 0; i < width; i++)
                    {
                        vx_uint32 srcIdx = n * channel * height * width + m * height * width + j * width + i;
                        vx_uint32 dstIdx = (batch - n - 1) * channel * height * width + m  * height * width + j * width + i;

                        vx_float32 value = vxnneGetDataExt(srcFormat, srcQuantFormat, srcIdx, (vx_uint8_ptr)srcLogical, srcFp, srcZp, srcScale);
                        vxnneSaveDataExt(dstFormat, dstQuantFormat, dstIdx, value, (vx_uint8_ptr)dstLogical, dstFp, dstZp, dstScale, dstRoundingMode);
                    }
                }
            }
        }
        break;

    default:
        vxError("Tensor Reverse: invalid axis");
        return VX_ERROR_INVALID_VALUE;
    }

    return VX_SUCCESS;
}

vx_status vxnneExecuteSWTensorReverse(struct _vxnne_operation_s *operation)
{
    vxnne_tensor_reverse_sw_operation reverseOperation = (vxnne_tensor_reverse_sw_operation)operation;

    vx_tensor input = reverseOperation->input;
    vx_tensor output = reverseOperation->output;
    vx_uint32 numOfAxis = reverseOperation->numOfAxis;

    vx_context ctx = vxGetContext((vx_reference)input);
    vx_graph graph = input->tensorBuffer->memory.graph;

    vx_tensor tmp[2] = {VX_NULL};
    vx_uint32 sizes[4] = {0};
    vx_status status = VX_SUCCESS;
    vx_uint32 i;
    vx_tensor_create_params_t tensor_create_params;

    for (i = 0; i < 4; i++)
    {
        sizes[i] = TENSOR_VIEW_SIZE_INDEX(input, i);
    }

    gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
    tensor_create_params.num_of_dims = TENSOR_DIM_NUM(input);
    tensor_create_params.sizes = sizes;
    tensor_create_params.data_format = TENSOR_DATA_TYPE(input);
    tensor_create_params.quant_format = TENSOR_QUANT_TYPE(input);
    if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
    {
        tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(input);
    }
    else
    {
        tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(input);
        tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(input);
    }

    switch (numOfAxis)
    {
    case 1:
        status = CPUTensorReverse(input, output, ((vx_scalar)reverseOperation->axis[0])->value->n32);
        if (status != VX_SUCCESS)
            goto OnError;

        break;

    case 2:
        tmp[0] = vxoTensor_CreateTensor(ctx, graph, &tensor_create_params, vx_false_e);
        vxoTensor_AllocateMemory(tmp[0]);

        status = CPUTensorReverse(input, tmp[0], ((vx_scalar)reverseOperation->axis[0])->value->n32);
        if (status != VX_SUCCESS)
            goto OnError;

        status = CPUTensorReverse(tmp[0], output, ((vx_scalar)reverseOperation->axis[1])->value->n32);
        if (status != VX_SUCCESS)
            goto OnError;

        break;

    case 3:
        tmp[0] = vxoTensor_CreateTensor(ctx, graph, &tensor_create_params, vx_false_e);
        vxoTensor_AllocateMemory(tmp[0]);
        tmp[1] = vxoTensor_CreateTensor(ctx, graph, &tensor_create_params, vx_false_e);
        vxoTensor_AllocateMemory(tmp[1]);

        status = CPUTensorReverse(input, tmp[0], ((vx_scalar)reverseOperation->axis[0])->value->n32);
        if (status != VX_SUCCESS)
            goto OnError;

        status = CPUTensorReverse(tmp[0], tmp[1], ((vx_scalar)reverseOperation->axis[1])->value->n32);
        if (status != VX_SUCCESS)
            goto OnError;

        status = CPUTensorReverse(tmp[1], output, ((vx_scalar)reverseOperation->axis[2])->value->n32);
        if (status != VX_SUCCESS)
            goto OnError;

        break;

    case 4:
        tmp[0] = vxoTensor_CreateTensor(ctx, graph, &tensor_create_params, vx_false_e);
        vxoTensor_AllocateMemory(tmp[0]);
        tmp[1] = vxoTensor_CreateTensor(ctx, graph, &tensor_create_params, vx_false_e);
        vxoTensor_AllocateMemory(tmp[1]);

        status = CPUTensorReverse(input, tmp[0], ((vx_scalar)reverseOperation->axis[0])->value->n32);
        if (status != VX_SUCCESS)
            goto OnError;

        status = CPUTensorReverse(tmp[0], tmp[1], ((vx_scalar)reverseOperation->axis[1])->value->n32);
        if (status != VX_SUCCESS)
            goto OnError;

        status = CPUTensorReverse(tmp[1], tmp[0], ((vx_scalar)reverseOperation->axis[2])->value->n32);
        if (status != VX_SUCCESS)
            goto OnError;

        status = CPUTensorReverse(tmp[0], output, ((vx_scalar)reverseOperation->axis[3])->value->n32);
        if (status != VX_SUCCESS)
            goto OnError;

        break;
    default:
        vxError("Tensor Reverse: invalid axis");
        return VX_ERROR_INVALID_VALUE;
    }

OnError:
    if (tmp[0] != VX_NULL)
        vxoTensor_ReleaseTensor(&tmp[0]);
    if (tmp[1] != VX_NULL)
        vxoTensor_ReleaseTensor(&tmp[1]);
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorReverse(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorReverse_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorReverse_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}
#if REGISTER_FRAME
VX_PRIVATE_API vx_status vxoNNTensorReverse_SW_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vxnne_tensor_reverse  reverseNode = (vxnne_tensor_reverse)ops_layer;
    vx_tensor  input      = (vx_tensor)parameters[0];
    vx_tensor  output     = (vx_tensor)parameters[6];
    vx_uint32  numOfAxis  = ((vx_scalar)parameters[1])->value->u32;
    vx_uint32  batchCount = (TENSOR_SIZE_INDEX(input, TENSOR_VIEW_DIM_NUM(input)) == 0) ? 1 : TENSOR_SIZE_INDEX(input, TENSOR_VIEW_DIM_NUM(input));
    vx_uint32 i;
    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);
    vxmONERROR(vxnneOperation_Initialize(&reverseNode->tensor_reverse_sw_operation.base,
        &reverseNode->base,
        VXNNE_OPERATION_TARGET_SW,
        VXNNE_OPERATOR_TENSOR_REVERSE,
        vxnneExecuteSWTensorReverse,
        VX_NULL,
        batchCount,
        0));

    vxmONERROR(vxnneLayer_SetOperation(&reverseNode->base, &reverseNode->tensor_reverse_sw_operation.base, 0));
    reverseNode->tensor_reverse_sw_operation.input           = input;
    reverseNode->tensor_reverse_sw_operation.output          = output;
    reverseNode->tensor_reverse_sw_operation.numOfAxis       = numOfAxis;
    for (i = 0; i <numOfAxis; i++)
    {
        reverseNode->tensor_reverse_sw_operation.axis[i] = (vx_scalar)parameters[i + 2];
    }

    vxmONERROR(vxnneOperation_AddReference(&reverseNode->tensor_reverse_sw_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&reverseNode->tensor_reverse_sw_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNTensorReverse_SH_Support_Ext(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_tensor  input      = (vx_tensor)parameters[0];
    vx_tensor  output     = (vx_tensor)parameters[6];
    vx_uint32  numOfAxis  = ((vx_scalar)parameters[1])->value->u32;
    vx_uint32 i;
    vx_bool dataFormat_flag, axFlag;
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    axFlag = vx_true_e;

    dataFormat_flag = (vx_bool)(!checkOutputTensorDoAlu(input, output));
    for (i = 0; i <numOfAxis; i++)
    {
       if(((vx_scalar)parameters[i + 2])->value->n32 == 3)
       {
           axFlag = vx_false_e;
           break;
       }
    }

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    support = dataFormat_flag && support;
    support = support && (numOfAxis < 4);
    support = support && axFlag;

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}


VX_PRIVATE_API vx_bool vxoNNTensorReverse_SH_Evis_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && node->base.context->evisNoInst.supportEVIS;

    if (!support)return support;

    support = support && vxoNNTensorReverse_SH_Support_Ext(node, parameters, num, reg_param, vx_true_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_bool vxoNNTensorReverse_SH_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && vxoNNTensorReverse_SH_Support_Ext(node, parameters, num, reg_param, vx_false_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNTensorReverse_SH_Initialize_Ext(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_status status = VX_SUCCESS;
    vx_tensor  input      = (vx_tensor)parameters[0];
    vx_tensor  output     = (vx_tensor)parameters[6];
    vx_uint32  numOfAxis  = ((vx_scalar)parameters[1])->value->u32;
    vx_uint32  batchCount = TENSOR_VIEW_DIM_NUM(input) < 4 ? 1 : TENSOR_SIZE_INDEX(input, 3);
    vxnne_tensor_reverse  reverseNode = (vxnne_tensor_reverse)ops_layer;
    vx_uint32 i;

    vxnne_shader_executable shaderExecutable = VX_NULL;
    vx_uint32 axis[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {8, 8, 8, 8, 8, 8};
    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    for (i = 0; i <numOfAxis; i++)
    {
        axis[i] = ((vx_scalar)parameters[i + 2])->value->n32;
    }

    if (evis)
    {
        shaderExecutable = vxnneGetReverseShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_REVERSE,
                           &ops_layer->node->kernelAttributes.borderMode, input, output, numOfAxis, axis);
    }
    else
    {
        shaderExecutable = vxnneGetGPUReverseShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_GPU_TENSOR_REVERSE,
                           &ops_layer->node->kernelAttributes.borderMode, input, output, numOfAxis, axis);
    }

    if (!shaderExecutable)
    {
        status = VX_FAILURE;
        goto OnError;
    }

    vxmONERROR(vxnneShaderOperation_Initialize(&reverseNode->tensor_reverse_sh_operation,
        &reverseNode->base,
        VXNNE_OPERATOR_TENSOR_REVERSE,
        batchCount,
        shaderExecutable));

    vxmONERROR(vxnneOperation_AddReference(&reverseNode->tensor_reverse_sh_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&reverseNode->tensor_reverse_sh_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT));
    vxmONERROR(vxnneLayer_SetOperation(
        &reverseNode->base,
        &reverseNode->tensor_reverse_sh_operation.base,
        0));
OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_status vxoNNTensorReverse_SH_EVIS_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxoNNTensorReverse_SH_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_true_e));

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

OnError:
    return status;
}

VX_PRIVATE_API vx_status vxoNNTensorReverse_SH_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxoNNTensorReverse_SH_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_false_e));

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

OnError:
    return status;
}

VX_PRIVATE_API vx_bool vxoNNTensorReverse_TP_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_tensor  input      = (vx_tensor)parameters[0];
    vx_tensor  output     = (vx_tensor)parameters[6];
    vx_context context = vxGetContext((vx_reference)node);
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_TP, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    support = support && vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_REVERSE);
    support = support && vxnneIsTPSupportFormat(context, input, VX_NULL, output);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNTensorReverse_TP_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vx_tensor  input      = (vx_tensor)parameters[0];
    vx_tensor  output     = (vx_tensor)parameters[6];
    vx_uint32  numOfAxis  = ((vx_scalar)parameters[1])->value->u32;
    vx_uint32  batchCount = (TENSOR_SIZE_INDEX(input, TENSOR_VIEW_DIM_NUM(input)) == 0) ? 1 : TENSOR_SIZE_INDEX(input, TENSOR_VIEW_DIM_NUM(input));
    vxnne_tensor_reverse  reverseNode = (vxnne_tensor_reverse)ops_layer;
    vx_uint32 i;
    vx_op_param_s conv = {0};
    vx_int32 axis[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    for (i = 0; i <numOfAxis; i++)
    {
        axis[i] = ((vx_scalar)parameters[i + 2])->value->n32;
    }

    vxmONERROR(vxnneOperation_Initialize(&reverseNode->tensor_reverse_tp_operation.base,
                                        &reverseNode->base,
                                        VXNNE_OPERATION_TARGET_TP,
                                        VXNNE_OPERATOR_TENSOR_REVERSE,
                                        VX_NULL,
                                        vxnneOperation_TP_Deinitialize,
                                        batchCount,
                                        0));

    vxmONERROR(vxnneLayer_SetOperation(&reverseNode->base, &reverseNode->tensor_reverse_tp_operation.base, 0));
    reverseNode->tensor_reverse_tp_operation.input  = input;
    reverseNode->tensor_reverse_tp_operation.output = output;

    vxmONERROR(vxnneOperation_AddReference(&reverseNode->tensor_reverse_tp_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&reverseNode->tensor_reverse_tp_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    conv.pad_x_left = 0;
    conv.pad_y_top = 0;
    conv.pool_size_x = 0;
    conv.pool_size_y = 0;
    conv.pool_stride = 1;
    conv.enable_relu = vx_false_e;
    conv.conv_rounding_type = 0;
    conv.pad_mode = VX_PAD_CONSTANT;
    conv.pad_const = 0;
    conv.tpType = TP_REVERSE;
    conv.data_buff = gcvNULL;
    conv.other_ref = (vx_reference)input;
    conv.tp_value = (vx_tp_value_cmd)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
    conv.tp_value->u32[0] = numOfAxis;
    conv.tp_value->p8[0] = (vx_uint8_ptr)vxAllocateAndZeroMemory(sizeof(axis));
    vxMemCopy(conv.tp_value->p8[0], axis, sizeof(axis));

    vxMemCopy(&reverseNode->tensor_reverse_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));
OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_status vxoNNLayer_GetOperations(vxnne_layer ops_layer, vx_uint32_ptr max_num_operations, vxnne_operation **operations)
{
    vx_status  status = VX_SUCCESS;
    vxnne_tensor_reverse  reverseNode = (vxnne_tensor_reverse)ops_layer;

    *max_num_operations = gcmCOUNTOF(reverseNode->operations);

    *operations = reverseNode->operations;

    return status;
}
#endif

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorReverse_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status  status     = VX_SUCCESS;
#if REGISTER_FRAME
    vxnne_layer_imp_s registerTensorReverse[] = {/* Please DON'T adjust the order, it's importent */
        { "TensorReverse NN", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "TensorReverse TP", vxoNNTensorReverse_TP_Support, vxoNNTensorReverse_TP_Initialize, VX_NULL },
        { "TensorReverse SH EVIS", vxoNNTensorReverse_SH_Evis_Support, vxoNNTensorReverse_SH_EVIS_Initialize, VX_NULL },
        { "TensorReverse SH F32", vxoNNTensorReverse_SH_Support, vxoNNTensorReverse_SH_Initialize, VX_NULL },
        { "TensorReverse SW ", vxoNNCommon_Support, vxoNNTensorReverse_SW_Initialize, VX_NULL },
    };

    REGISTER_LAYERS(registerTensorReverse, vxnne_tensor_reverse_s, "TensorReverse", vxoNNLayer_GetOperations);

OnError:
#else
    vx_tensor  input      = (vx_tensor)parameters[0];
    vx_tensor  output     = (vx_tensor)parameters[6];
    vx_uint32  numOfAxis  = ((vx_scalar)parameters[1])->value->u32;
    vx_uint32  batchCount = (TENSOR_SIZE_INDEX(input, TENSOR_VIEW_DIM_NUM(input)) == 0) ? 1 : TENSOR_SIZE_INDEX(input, TENSOR_VIEW_DIM_NUM(input));
    vx_context context = vxGetContext((vx_reference)node);
    vxnne_tensor_reverse  reverseNode = VX_NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    {
        vx_uint32 i;
        vx_bool dataFormat_flag, axFlag;

        axFlag = vx_true_e;

        dataFormat_flag = (vx_bool)(!checkOutputTensorDoAlu(input, output));
        for (i = 0; i <numOfAxis; i++)
        {
           if(((vx_scalar)parameters[i + 2])->value->n32 == 3)
           {
               axFlag = vx_false_e;
               break;
           }
        }

        gcoOS_Allocate(gcvNULL, sizeof(vxnne_tensor_reverse_s), (gctPOINTER*)&reverseNode);
        if (!reverseNode)
        {
            status = VX_ERROR_NO_MEMORY;
            vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            goto exit;
        }

        gcoOS_ZeroMemory(reverseNode, sizeof(vxnne_tensor_reverse_s));

        vxnneLayer_Initialize(&reverseNode->base,
                              "TensorReverse",
                              node,
                              vxmOPERATION_COUNT(reverseNode),
                              reverseNode->operations,
                              VX_NULL);

        if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_REVERSE) &&
            vxnneIsTPSupportFormat(context, input, VX_NULL, output))
        {
            vx_op_param_s conv = {0};
            vx_int32 axis[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};

            for (i = 0; i <numOfAxis; i++)
            {
                axis[i] = ((vx_scalar)parameters[i + 2])->value->n32;
            }

            status = vxnneOperation_Initialize(&reverseNode->tensor_reverse_tp_operation.base,
                                               &reverseNode->base,
                                               VXNNE_OPERATION_TARGET_TP,
                                               VXNNE_OPERATOR_TENSOR_REVERSE,
                                               VX_NULL,
                                               vxnneOperation_TP_Deinitialize,
                                               batchCount,
                                               0);
            if (status != VX_SUCCESS) goto exit;
            vxnneLayer_SetOperation(&reverseNode->base, &reverseNode->tensor_reverse_tp_operation.base, 0);
            reverseNode->tensor_reverse_tp_operation.input  = input;
            reverseNode->tensor_reverse_tp_operation.output = output;

            vxnneOperation_AddReference(&reverseNode->tensor_reverse_tp_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&reverseNode->tensor_reverse_tp_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            conv.pad_x_left = 0;
            conv.pad_y_top = 0;
            conv.pool_size_x = 0;
            conv.pool_size_y = 0;
            conv.pool_stride = 1;
            conv.enable_relu = vx_false_e;
            conv.conv_rounding_type = 0;
            conv.pad_mode = VX_PAD_CONSTANT;
            conv.pad_const = 0;
            conv.tpType = TP_REVERSE;
            conv.data_buff = gcvNULL;
            conv.other_ref = (vx_reference)input;
            conv.tp_value = (vx_tp_value_cmd)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
            conv.tp_value->u32[0] = numOfAxis;
            conv.tp_value->p8[0] = (vx_uint8_ptr)vxAllocateAndZeroMemory(sizeof(axis));
            vxMemCopy(conv.tp_value->p8[0], axis, sizeof(axis));

            vxMemCopy(&reverseNode->tensor_reverse_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));
        }
        else if(dataFormat_flag && vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)
            && numOfAxis < 4 && axFlag)
        {
            vxnne_shader_executable shaderExecutable = VX_NULL;
            vx_uint32 axis[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {8, 8, 8, 8, 8, 8};
            for (i = 0; i <numOfAxis; i++)
            {
                axis[i] = ((vx_scalar)parameters[i + 2])->value->n32;
            }

            shaderExecutable = vxnneGetReverseShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_REVERSE, &node->kernelAttributes.borderMode,
                input, output, numOfAxis, axis);

            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto exit;
            }

            status = vxnneShaderOperation_Initialize(&reverseNode->tensor_reverse_sh_operation,
                &reverseNode->base,
                VXNNE_OPERATOR_TENSOR_REVERSE,
                batchCount,
                shaderExecutable);

            if (status != VX_SUCCESS) goto exit;

            vxnneOperation_AddReference(&reverseNode->tensor_reverse_sh_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&reverseNode->tensor_reverse_sh_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            vxnneLayer_SetOperation(
                &reverseNode->base,
                &reverseNode->tensor_reverse_sh_operation.base,
                0);
        }
        else
        {
            vxnneOperation_Initialize(&reverseNode->tensor_reverse_sw_operation.base,
                &reverseNode->base,
                VXNNE_OPERATION_TARGET_SW,
                VXNNE_OPERATOR_TENSOR_REVERSE,
                vxnneExecuteSWTensorReverse,
                VX_NULL,
                batchCount,
                0);

            vxnneLayer_SetOperation(&reverseNode->base, &reverseNode->tensor_reverse_sw_operation.base, 0);
            reverseNode->tensor_reverse_sw_operation.input           = input;
            reverseNode->tensor_reverse_sw_operation.output          = output;
            reverseNode->tensor_reverse_sw_operation.numOfAxis       = numOfAxis;
            for (i = 0; i <numOfAxis; i++)
            {
                reverseNode->tensor_reverse_sw_operation.axis[i] = (vx_scalar)parameters[i + 2];
            }

            vxnneOperation_AddReference(&reverseNode->tensor_reverse_sw_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&reverseNode->tensor_reverse_sw_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }
        node->layer = &reverseNode->base;
    }

    return status;

exit:
    if(reverseNode) gcoOS_Free(gcvNULL, reverseNode);
#endif
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorReverse_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

