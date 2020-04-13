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
#include <layers/gc_vx_layer_reorg.h>


/***************************************************************************************************************************
 *                                                 Reorg
 ***************************************************************************************************************************/

vx_status vxnneExecuteSWReorg(struct _vxnne_operation_s *operation)
{
    vxnne_reorg_operation reorgOperation   = (vxnne_reorg_operation)operation;

    vx_tensor  inputs           = (vx_tensor)reorgOperation->inputs;
    vx_scalar  strides          = (vx_scalar)reorgOperation->stride;
    vx_tensor  outputs          = (vx_tensor)reorgOperation->outputs;

    vx_uint32  stride           = strides->value->u32;
    vx_uint32  input_width      = TENSOR_SIZE_INDEX(inputs, 0);
    vx_uint32  input_height     = TENSOR_SIZE_INDEX(inputs, 1);
    vx_uint32  input_depth      = TENSOR_SIZE_INDEX(inputs, 2);
    vx_uint32  input_batch      = TENSOR_SIZE_INDEX(inputs, 3);
    vx_type_e  inputFormat      = (vx_type_e)TENSOR_DATA_TYPE(inputs);
    vx_type_e  outputFormat     = (vx_type_e)TENSOR_DATA_TYPE(outputs);
    vx_int8   inputFixedPoint   = TENSOR_POS(inputs);
    vx_int8   outputFixedPoint  = TENSOR_POS(outputs);

    vx_uint32  out_c            = input_depth / (stride * stride);
    vx_uint32  i,j,k,b;
    vx_float32 data = 0.0f;
    gctPOINTER inputBase;
    gctPOINTER outputBase;

    vx_status status = VX_SUCCESS;

    if (input_batch == 0)
    {
        input_batch = 1;
    }

    vxoTensor_GetTensorViewMemory(inputs, &inputBase, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, &outputBase, VX_NULL);

    if ((TENSOR_DATA_TYPE(inputs) != VX_TYPE_BFLOAT16 && TENSOR_DATA_TYPE(inputs) != VX_TYPE_FLOAT16 && TENSOR_DATA_TYPE(inputs) != VX_TYPE_FLOAT32 && TENSOR_DATA_TYPE(inputs) != VX_TYPE_INT8 && TENSOR_DATA_TYPE(inputs) != VX_TYPE_UINT8)
        || (TENSOR_DATA_TYPE(outputs) != VX_TYPE_BFLOAT16 && TENSOR_DATA_TYPE(outputs) != VX_TYPE_FLOAT16 && TENSOR_DATA_TYPE(outputs) != VX_TYPE_FLOAT32 && TENSOR_DATA_TYPE(outputs) != VX_TYPE_INT8 && TENSOR_DATA_TYPE(outputs) != VX_TYPE_UINT8))
    {
        vxError("input or outputs format is not support");
        status = VX_ERROR_NOT_SUPPORTED;
        return status;
    }
    for(b = 0; b < input_batch; ++b)
    {
        for(k = 0; k < input_depth; ++k)
        {
            for(j = 0; j < input_height; ++j)
            {
                for(i = 0; i < input_width; ++i)
                {
                    vx_int32 in_index  = i + input_width * (j + input_height * (k + input_depth * b));
                    vx_int32 c2 = k % out_c;
                    vx_int32 offset = k / out_c;
                    vx_int32 w2 = i * stride + offset % stride;
                    vx_int32 h2 = j * stride + offset / stride;
                    vx_int32 out_index = w2 + input_width * stride * (h2 + input_height * stride * (c2 + out_c * b));

                    data = vxnneGetDataExt(inputFormat, TENSOR_QUANT_TYPE(inputs), out_index, (vx_uint8_ptr)inputBase, inputFixedPoint, TENSOR_TF_ZEROPOINT(inputs), TENSOR_TF_SCALE(inputs));
                    vxnneSaveDataExt(outputFormat, TENSOR_QUANT_TYPE(outputs), in_index, data, (vx_uint8_ptr)outputBase, outputFixedPoint, TENSOR_TF_ZEROPOINT(outputs), TENSOR_TF_SCALE(outputs), TENSOR_ROUNDING_MODE(outputs));
                }
            }
        }
    }

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNReorgLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNReorgLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNReorgLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}
#if REGISTER_FRAME

VX_PRIVATE_API vx_status vxoNNReorgLayer_SW_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vx_tensor  inputs = (vx_tensor)parameters[0];
    vx_scalar  stride_s = (vx_scalar)parameters[1];
    vx_tensor  outputs = (vx_tensor)parameters[2];
    vx_uint32  batchCount = TENSOR_SIZE_INDEX(inputs, 3);

    vxnne_reorg_layer  reorgLayer = (vxnne_reorg_layer)ops_layer;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);
    vxmONERROR(vxnneOperation_Initialize(&reorgLayer->reorg_sw_operation.base,
        &reorgLayer->base,
        VXNNE_OPERATION_TARGET_SW,
        VXNNE_OPERATOR_REORG,
        vxnneExecuteSWReorg,
        VX_NULL,
        batchCount,
        0));

    vxmONERROR(vxnneLayer_SetOperation(
        &reorgLayer->base,
        &reorgLayer->reorg_sw_operation.base,
        0));

    reorgLayer->reorg_sw_operation.inputs = inputs;
    reorgLayer->reorg_sw_operation.stride = (vx_reference)stride_s;
    reorgLayer->reorg_sw_operation.outputs = outputs;

    vxmONERROR(vxnneOperation_AddReference(&reorgLayer->reorg_sw_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&reorgLayer->reorg_sw_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);
OnError:
    return status;
}

VX_PRIVATE_API vx_bool vxoNNReorgLayer_SH_Support_Ext(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_tensor  inputs = (vx_tensor)parameters[0];
    vx_scalar  stride_s = (vx_scalar)parameters[1];
    vx_tensor  outputs = (vx_tensor)parameters[2];
    vx_uint32  stride = stride_s->value->u32;
    vx_enum    inputFormat = TENSOR_DATA_TYPE(inputs);
    vx_enum    outputFormat = TENSOR_DATA_TYPE(outputs);

    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (evis)
    {
        support = support && (((inputFormat == VX_TYPE_FLOAT16 || inputFormat == VX_TYPE_INT8) && (outputFormat == VX_TYPE_FLOAT16 || outputFormat == VX_TYPE_INT8))
                            || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
                            || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16));
    }
    else
    {
        support = support && ((inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
                             || ((inputFormat == VX_TYPE_FLOAT16 || inputFormat == VX_TYPE_FLOAT32) && (outputFormat == VX_TYPE_FLOAT16 || outputFormat == VX_TYPE_FLOAT32)));
    }
    support = support && (stride == 2);
    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}


VX_PRIVATE_API vx_bool vxoNNReorgLayer_SH_Evis_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && node->base.context->evisNoInst.supportEVIS;

    if (!support)return support;

    support = support && vxoNNReorgLayer_SH_Support_Ext(node, parameters, num, reg_param, vx_true_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_bool vxoNNReorgLayer_SH_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && vxoNNReorgLayer_SH_Support_Ext(node, parameters, num, reg_param, vx_false_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNReorgLayer_SH_Initialize_Ext(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_status status = VX_SUCCESS;

    vx_tensor  inputs = (vx_tensor)parameters[0];
    vx_scalar  stride_s = (vx_scalar)parameters[1];
    vx_tensor  outputs = (vx_tensor)parameters[2];
    vx_uint32  input_depth = TENSOR_SIZE_INDEX(inputs, 2);
    vx_uint32  batchCount = TENSOR_SIZE_INDEX(inputs, 3);
    vx_context context = vxGetContext((vx_reference)ops_layer->node);

    vxnne_reorg_layer  reorgLayer = (vxnne_reorg_layer)ops_layer;

    vx_scalar outc_s = vxCreateScalar(context, VX_TYPE_UINT32, &input_depth);

    vxnne_shader_executable shaderExecutable = VX_NULL;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    if (evis)
    {
        shaderExecutable = vxnneGetReorgShaderExecutable(context, VXNNE_KERNEL_REORG, &ops_layer->node->kernelAttributes.borderMode,
            inputs, stride_s, outc_s, outputs);
    }
    else
    {
        shaderExecutable = vxnneGetGPUReorgShaderExecutable(context, VXNNE_KERNEL_GPU_REORG, &ops_layer->node->kernelAttributes.borderMode,
            inputs, stride_s, outc_s, outputs);
    }

    if (!shaderExecutable)
    {
        status = VX_FAILURE;
        if (!outc_s) vxReleaseScalar(&outc_s);
        goto OnError;
    }

    vxmONERROR(vxnneShaderOperation_Initialize(&reorgLayer->reorg_sh_operation,
        &reorgLayer->base,
        VXNNE_OPERATOR_REORG,
        batchCount,
        shaderExecutable));

    vxmONERROR(vxnneOperation_AddReference(&reorgLayer->reorg_sh_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&reorgLayer->reorg_sh_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    vxmONERROR(vxnneLayer_SetOperation(
        &reorgLayer->base,
        &reorgLayer->reorg_sh_operation.base,
        0));

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

OnError:
    if (!outc_s) vxReleaseScalar(&outc_s);
    return status;
}

VX_PRIVATE_API vx_status vxoNNReorgLayer_SH_EVIS_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxoNNReorgLayer_SH_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_true_e));

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

OnError:
    return status;
}

VX_PRIVATE_API vx_status vxoNNReorgLayer_SH_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxoNNReorgLayer_SH_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_false_e));

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

OnError:
    return status;
}

VX_PRIVATE_API vx_bool vxoNNReorgLayer_TP_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_tensor  inputs = (vx_tensor)parameters[0];
    vx_tensor  outputs = (vx_tensor)parameters[2];
    vx_context context = vxGetContext((vx_reference)node);

    vx_bool support = vxoLayer_CheckSupport(context, VX_NN_QUERY_TP, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    support = support && vxnneIsTPSupportFormat(context, inputs, VX_NULL, outputs);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNReorgLayer_TP_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vx_tensor  inputs = (vx_tensor)parameters[0];
    vx_scalar  stride_s = (vx_scalar)parameters[1];
    vx_tensor  outputs = (vx_tensor)parameters[2];
    vx_uint32  stride = stride_s->value->u32;
    vx_uint32  batchCount = TENSOR_SIZE_INDEX(inputs, 3);

    vxnne_reorg_layer  reorgLayer = (vxnne_reorg_layer)ops_layer;

    vx_op_param_s conv = { 0 };

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxnneOperation_Initialize(&reorgLayer->reorg_tp_operation.base,
        &reorgLayer->base,
        VXNNE_OPERATION_TARGET_TP,
        VXNNE_OPERATOR_REORG,
        VX_NULL,
        vxnneOperation_TP_Deinitialize,
        batchCount,
        0));

    conv.pad_x_left = 0;
    conv.pad_y_top = 0;
    conv.pool_size_x = 0;
    conv.pool_size_y = 0;
    conv.pool_stride = 1;
    conv.enable_relu = vx_false_e;
    conv.pad_mode = VX_PAD_CONSTANT;
    conv.pad_const = 0;
    conv.tpType = TP_REORG;
    conv.other_ref = gcvNULL;
    conv.data_buff = gcvNULL;
    conv.tp_value = (vx_tp_value_cmd)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
    conv.tp_value->u32[0] = stride;

    vxMemCopy(&reorgLayer->reorg_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));

    vxmONERROR(vxnneLayer_SetOperation(
        &reorgLayer->base,
        &reorgLayer->reorg_tp_operation.base,
        0));
    reorgLayer->reorg_tp_operation.input = inputs;
    reorgLayer->reorg_tp_operation.output = outputs;

    vxmONERROR(vxnneOperation_AddReference(&reorgLayer->reorg_tp_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&reorgLayer->reorg_tp_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

OnError:
    return status;
}

VX_PRIVATE_API vx_status vxoNNLayer_GetOperations(vxnne_layer ops_layer, vx_uint32_ptr max_num_operations, vxnne_operation **operations)
{
    vx_status  status = VX_SUCCESS;
    vxnne_reorg_layer  reorgLayer = (vxnne_reorg_layer)ops_layer;

    *max_num_operations = gcmCOUNTOF(reorgLayer->operations);

    *operations = reorgLayer->operations;

    return status;
}
#endif



VX_PRIVATE_API vx_status VX_CALLBACK vxoNNReorgLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
#if REGISTER_FRAME

    vxnne_layer_imp_s registerReorgLayers[] = {/* Please DON'T adjust the order, it's importent */
        { "ReorgLayer NN", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "ReorgLayer TP", vxoNNReorgLayer_TP_Support, vxoNNReorgLayer_TP_Initialize, vxoNNCommon_Deinitialize },
        { "ReorgLayer SH EVIS", vxoNNReorgLayer_SH_Evis_Support, vxoNNReorgLayer_SH_EVIS_Initialize, VX_NULL },
        { "ReorgLayer SH F32", vxoNNReorgLayer_SH_Support, vxoNNReorgLayer_SH_Initialize, VX_NULL },
        { "ReorgLayer SW ", vxoNNCommon_Support, vxoNNReorgLayer_SW_Initialize, VX_NULL },
    };

    REGISTER_LAYERS(registerReorgLayers, vxnne_reorg_layer_s, "ReorgLayer", vxoNNLayer_GetOperations);

OnError:
#else

    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_scalar  stride_s                   = (vx_scalar)parameters[1];
    vx_tensor  outputs                    = (vx_tensor)parameters[2];
    vx_uint32  stride                     = stride_s->value->u32;
    vx_enum    inputFormat                = TENSOR_DATA_TYPE(inputs);
    vx_enum    outputFormat               = TENSOR_DATA_TYPE(outputs);
    vx_uint32  input_depth                = TENSOR_SIZE_INDEX(inputs, 2);
    vx_uint32  batchCount                 = TENSOR_SIZE_INDEX(inputs, 3);
    vx_context context                    = vxGetContext((vx_reference)node);

    vxnne_reorg_layer  reorgLayer         = VX_NULL;
    vx_bool            dataFormat_flag    = (vx_bool)(((inputFormat == VX_TYPE_FLOAT16 || inputFormat == VX_TYPE_INT8) && (outputFormat == VX_TYPE_FLOAT16 || outputFormat == VX_TYPE_INT8))
                                                    || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
                                                    || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16));
    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }
    gcoOS_Allocate(gcvNULL, sizeof(vxnne_reorg_layer_s), (gctPOINTER*)&reorgLayer);
    if (!reorgLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(reorgLayer, sizeof(vxnne_reorg_layer_s));

    vxnneLayer_Initialize(&reorgLayer->base,
                          "ReorgLayer",
                          node,
                          vxmOPERATION_COUNT(reorgLayer),
                          reorgLayer->operations,
                          VX_NULL);

    if (vxnneIsTPSupportFormat(context, inputs, VX_NULL, outputs) &&
        vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_REORG))
    {
        vx_op_param_s conv = {0};

        status = vxnneOperation_Initialize(&reorgLayer->reorg_tp_operation.base,
                                           &reorgLayer->base,
                                           VXNNE_OPERATION_TARGET_TP,
                                           VXNNE_OPERATOR_REORG,
                                           VX_NULL,
                                           vxnneOperation_TP_Deinitialize,
                                           batchCount,
                                           0);
        if (status != VX_SUCCESS) goto exit;

        conv.pad_x_left = 0;
        conv.pad_y_top = 0;
        conv.pool_size_x = 0;
        conv.pool_size_y = 0;
        conv.pool_stride = 1;
        conv.enable_relu = vx_false_e;
        conv.pad_mode = VX_PAD_CONSTANT;
        conv.pad_const = 0;
        conv.tpType = TP_REORG;
        conv.other_ref = gcvNULL;
        conv.data_buff = gcvNULL;
        conv.tp_value = (vx_tp_value_cmd)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
        conv.tp_value->u32[0] = stride;

        vxMemCopy(&reorgLayer->reorg_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));

        vxnneLayer_SetOperation(
            &reorgLayer->base,
            &reorgLayer->reorg_tp_operation.base,
            0);
        reorgLayer->reorg_tp_operation.input  = inputs;
        reorgLayer->reorg_tp_operation.output = outputs;

        vxnneOperation_AddReference(&reorgLayer->reorg_tp_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&reorgLayer->reorg_tp_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }
    else
    {
        if (stride == 2 && dataFormat_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
        {
            vx_scalar outc_s   = vxCreateScalar(node->base.context, VX_TYPE_UINT32, &input_depth);

            vxnne_shader_executable shaderExecutable = VX_NULL;

            shaderExecutable = vxnneGetReorgShaderExecutable(node->base.context, VXNNE_KERNEL_REORG, &node->kernelAttributes.borderMode,
                                                                 inputs, stride_s, outc_s, outputs);

            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                if (!outc_s) vxReleaseScalar(&outc_s);
                goto exit;
            }

            status = vxnneShaderOperation_Initialize(&reorgLayer->reorg_sh_operation,
                                            &reorgLayer->base,
                                            VXNNE_OPERATOR_REORG,
                                            batchCount,
                                            shaderExecutable);

            if (status != VX_SUCCESS)
            {
                if (!outc_s) vxReleaseScalar(&outc_s);
                goto exit;
            }

            vxnneOperation_AddReference(&reorgLayer->reorg_sh_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&reorgLayer->reorg_sh_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            vxnneLayer_SetOperation(
                &reorgLayer->base,
                &reorgLayer->reorg_sh_operation.base,
                0);
        }
        else
        {
            vxnneOperation_Initialize(&reorgLayer->reorg_sw_operation.base,
                                      &reorgLayer->base,
                                      VXNNE_OPERATION_TARGET_SW,
                                      VXNNE_OPERATOR_REORG,
                                      vxnneExecuteSWReorg,
                                      VX_NULL,
                                      batchCount,
                                      0);

            vxnneLayer_SetOperation(
                &reorgLayer->base,
                &reorgLayer->reorg_sw_operation.base,
                0);

            reorgLayer->reorg_sw_operation.inputs           = inputs;
            reorgLayer->reorg_sw_operation.stride           = (vx_reference)stride_s;
            reorgLayer->reorg_sw_operation.outputs          = outputs;

            vxnneOperation_AddReference(&reorgLayer->reorg_sw_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&reorgLayer->reorg_sw_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }
    }

    node->layer = &reorgLayer->base;
    return status;

exit:
    if (reorgLayer) gcoOS_Free(gcvNULL, (gctPOINTER)reorgLayer);
#endif

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNReorgLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}


/***************************************************************************************************************************
 *                                                 Reorg2
 ***************************************************************************************************************************/
vx_status vxnneReorg2_Space2Depth(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;

    vxnne_reorg_operation reorgOperation   = (vxnne_reorg_operation)operation;

    vx_tensor  inputs           = (vx_tensor)reorgOperation->inputs;
    vx_tensor  strides          = (vx_tensor)reorgOperation->stride;
    vx_tensor  outputs          = (vx_tensor)reorgOperation->outputs;

    vx_uint32  block_size       = (vx_uint32)VX_GET_DATA_FROM_TENSOR(strides, 0);

    vx_type_e  inputFormat      = (vx_type_e)TENSOR_DATA_TYPE(inputs);
    vx_type_e  outputFormat     = (vx_type_e)TENSOR_DATA_TYPE(outputs);

    vx_uint8_ptr inputBase;
    vx_uint8_ptr outputBase;

    const vx_uint32 input_width = TENSOR_SIZE_INDEX(inputs, 0);  /* W */
    const vx_uint32 input_height = TENSOR_SIZE_INDEX(inputs, 1); /* H */
    const vx_uint32 input_depth = TENSOR_SIZE_INDEX(inputs, 2);  /* C */
    const vx_uint32 input_batch = TENSOR_SIZE_INDEX(inputs, 3);  /* N */

    const vx_uint32 output_width = TENSOR_SIZE_INDEX(outputs, 0);  /* W */
    const vx_uint32 output_height = TENSOR_SIZE_INDEX(outputs, 1); /* H */
    const vx_uint32 output_depth = TENSOR_SIZE_INDEX(outputs, 2);  /* C */
    const vx_uint32 output_batch = TENSOR_SIZE_INDEX(outputs, 3);  /* N */

    const vx_int32 item_size = vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(outputs));

    vx_uint32 batch = 0, in_h = 0, in_w = 0;
    vx_float32 data = .0f;

    vxoTensor_GetTensorViewMemory(inputs, (gctPOINTER*)&inputBase, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, (gctPOINTER*)&outputBase, VX_NULL);

    gcmASSERT(input_width == output_width * block_size);
    gcmASSERT(input_height == output_height * block_size);
    gcmASSERT(output_depth == input_depth * block_size * block_size);
    if (output_batch != input_batch)
    {
        gcmASSERT(0);
    }

    outputBase = TENSOR_LOGICAL_ADDR(outputs);
    inputBase = TENSOR_LOGICAL_ADDR(inputs);

    /**************************************************************************************************
     *
     *                 W H C N                                                            W H C N
     *                 4 4 2 1                   =>                                       2 2 8 1
     *   _______________     _______________
     *  | 10| 11| 12| 13|   | 20| 21| 22| 23|
     *  |___|___|___|___|   |___|___|___|___|          _______   _______   _______   _______   _______   _______   _______   _______
     *  | 14| 15| 16| 17|   | 24| 25| 26| 27|         | 10| 12| | 20| 22| | 11| 13| | 21| 23| | 14| 16| | 24| 26| | 15| 17| | 25| 27|
     *  |___|___|___|___|   |___|___|___|___|    =>   |___|___| |___|___| |___|___| |___|___| |___|___| |___|___| |___|___| |___|___|
     *  | 18| 19|110|111|   | 28| 29|210|211|         | 18|110| | 28|210| | 19|111| | 29|211| |112|114| |212|214| |113|115| |213|215|
     *  |___|___|___|___|   |___|___|___|___|         |___|___| |___|___| |___|___| |___|___| |___|___| |___|___| |___|___| |___|___|
     *  |112|113|114|115|   |212|213|214|215|
     *  |___|___|___|___|   |___|___|___|___|
     *
     **************************************************************************************************/

    for (batch = 0; batch < input_batch; ++ batch)
    {
        vx_uint32 output_batch_index = batch * output_height * output_width * output_depth;
        vx_uint32 input_batch_index = batch * input_height * input_width * input_depth;
        vx_uint32 in_d = 0;

        for (in_d = 0; in_d < input_depth; in_d ++)
        {
            for (in_h = 0; in_h < input_height; ++ in_h)
            {
                for (in_w = 0; in_w < input_width; in_w ++)
                {
                    vx_int32 out_w = in_w / block_size;
                    vx_int32 out_h = in_h / block_size;
                    vx_int32 out_d = (in_w  % block_size) * input_depth + (in_h % block_size) * block_size * input_depth + in_d;

                    vx_int32 in_index = in_w + in_h * input_width +in_d * input_height * input_width + input_batch_index;
                    vx_int32 out_index = out_w + out_h * output_width +  out_d * output_width * output_height + output_batch_index;

                    if (item_size == vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs)))
                    {
                        memcpy(outputBase + out_index * item_size, inputBase + in_index * item_size, item_size);
                    }
                    else
                    {
                        data = vxnneGetDataExt(inputFormat, TENSOR_QUANT_TYPE(inputs), in_index, inputBase, TENSOR_POS(inputs), TENSOR_TF_ZEROPOINT(inputs), TENSOR_TF_SCALE(inputs));

                        status |= vxnneSaveDataExt(outputFormat, TENSOR_QUANT_TYPE(outputs), out_index, data, outputBase, TENSOR_POS(outputs), TENSOR_TF_ZEROPOINT(outputs), TENSOR_TF_SCALE(outputs), TENSOR_ROUNDING_MODE(outputs));
                    }
                }
            }
        }
    }

    return status;
}

vx_status vxnneReorg2_Depth2Space(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;

    vxnne_reorg_operation reorgOperation   = (vxnne_reorg_operation)operation;

    vx_tensor  inputs           = (vx_tensor)reorgOperation->inputs;
    vx_tensor  strides          = (vx_tensor)reorgOperation->stride;
    vx_tensor  outputs          = (vx_tensor)reorgOperation->outputs;

    vx_uint32  block_size       = (vx_uint32)VX_GET_DATA_FROM_TENSOR(strides, 0);

    vx_type_e  inputFormat      = (vx_type_e)TENSOR_DATA_TYPE(inputs);
    vx_type_e  outputFormat     = (vx_type_e)TENSOR_DATA_TYPE(outputs);

    vx_uint8_ptr inputBase;
    vx_uint8_ptr outputBase;

    const vx_uint32 input_width = TENSOR_SIZE_INDEX(inputs, 0);  /* W */
    const vx_uint32 input_height = TENSOR_SIZE_INDEX(inputs, 1); /* H */
    const vx_uint32 input_depth = TENSOR_SIZE_INDEX(inputs, 2);  /* C */
    const vx_uint32 input_batch = TENSOR_SIZE_INDEX(inputs, 3);  /* N */

    const vx_uint32 output_width = TENSOR_SIZE_INDEX(outputs, 0);  /* W */
    const vx_uint32 output_height = TENSOR_SIZE_INDEX(outputs, 1); /* H */
    const vx_uint32 output_depth = TENSOR_SIZE_INDEX(outputs, 2);  /* C */
    const vx_uint32 output_batch = TENSOR_SIZE_INDEX(outputs, 3);  /* N */

    const vx_int32 item_size = vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(outputs));

    vx_uint32 batch = 0, out_h = 0, out_w = 0;
    vx_float32 data = .0f;

    vxoTensor_GetTensorViewMemory(inputs, (gctPOINTER*)&inputBase, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, (gctPOINTER*)&outputBase, VX_NULL);

    gcmASSERT(input_width * block_size == output_width);
    gcmASSERT(input_height * block_size == output_height);
    gcmASSERT(output_depth * block_size * block_size == input_depth);
    if (output_batch != input_batch)
    {
        gcmASSERT(0);
    }

    outputBase = TENSOR_LOGICAL_ADDR(outputs);
    inputBase = TENSOR_LOGICAL_ADDR(inputs);

    /**************************************************************************************************
     *                 output                                                              input
     *                 W H C N                                                            W H C N
     *                 4 4 2 1                   <=                                       2 2 8 1
     *   _______________     _______________
     *  | 10| 11| 12| 13|   | 20| 21| 22| 23|
     *  |___|___|___|___|   |___|___|___|___|          _______   _______   _______   _______   _______   _______   _______   _______
     *  | 14| 15| 16| 17|   | 24| 25| 26| 27|         | 10| 12| | 20| 22| | 11| 13| | 21| 23| | 14| 16| | 24| 26| | 15| 17| | 25| 27|
     *  |___|___|___|___|   |___|___|___|___|    <=   |___|___| |___|___| |___|___| |___|___| |___|___| |___|___| |___|___| |___|___|
     *  | 18| 19|110|111|   | 28| 29|210|211|         | 18|110| | 28|210| | 19|111| | 29|211| |112|114| |212|214| |113|115| |213|215|
     *  |___|___|___|___|   |___|___|___|___|         |___|___| |___|___| |___|___| |___|___| |___|___| |___|___| |___|___| |___|___|
     *  |112|113|114|115|   |212|213|214|215|
     *  |___|___|___|___|   |___|___|___|___|
     *

     **************************************************************************************************/

    for (batch = 0; batch < output_batch; ++ batch)
    {
        vx_uint32 output_batch_index = batch * output_height * output_width * output_depth;
        vx_uint32 input_batch_index = batch * input_height * input_width * input_depth;
        vx_uint32 out_d = 0;

        for (out_d = 0; out_d < output_depth; out_d ++)
        {
            for (out_h = 0; out_h < output_height; ++ out_h)
            {
                for (out_w = 0; out_w < output_width; out_w ++)
                {
                    vx_int32 in_w = out_w / block_size;
                    vx_int32 in_h = out_h / block_size;
                    vx_int32 in_d = ((out_w  % block_size) + (out_h % block_size) * block_size) * output_depth + out_d;

                    vx_int32 in_index = in_w + in_h * input_width +  in_d * input_width * input_height + input_batch_index;
                    vx_int32 out_index = out_w + out_h * output_width + out_d * output_height * output_width + output_batch_index;

                    if (item_size == vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs)))
                    {
                        memcpy(outputBase + out_index * item_size, inputBase + in_index * item_size, item_size);
                    }
                    else
                    {
                        data = vxnneGetDataExt(inputFormat, TENSOR_QUANT_TYPE(inputs), in_index, inputBase, TENSOR_POS(inputs), TENSOR_TF_ZEROPOINT(inputs), TENSOR_TF_SCALE(inputs));

                        status |= vxnneSaveDataExt(outputFormat, TENSOR_QUANT_TYPE(outputs), out_index, data, outputBase, TENSOR_POS(outputs), TENSOR_TF_ZEROPOINT(outputs), TENSOR_TF_SCALE(outputs), TENSOR_ROUNDING_MODE(outputs));
                    }
                }
            }
        }
    }

    return status;
}

vx_status vxnneReorg2_Batch2SpaceND(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_reorg_operation reorgOperation = (vxnne_reorg_operation)operation;

    vx_tensor  inputs = (vx_tensor)reorgOperation->inputs;
    vx_tensor  strides = (vx_tensor)reorgOperation->stride;
    vx_tensor  outputs = (vx_tensor)reorgOperation->outputs;

    vx_type_e  inputFormat = (vx_type_e)TENSOR_DATA_TYPE(inputs);
    vx_type_e  outputFormat = (vx_type_e)TENSOR_DATA_TYPE(outputs);

    vx_uint8_ptr inputBase = VX_NULL, outputBase = VX_NULL;
    vx_int32_ptr block_size = VX_NULL;

    const vx_int32 input_width = TENSOR_SIZE_INDEX(inputs, 0);  /* W */
    const vx_int32 input_height = TENSOR_SIZE_INDEX(inputs, 1); /* H */
    const vx_int32 input_depth = TENSOR_SIZE_INDEX(inputs, 2);  /* C */
    const vx_int32 input_batch = TENSOR_SIZE_INDEX(inputs, 3);  /* N */

    const vx_int32 output_width = TENSOR_SIZE_INDEX(outputs, 0);  /* W */
    const vx_int32 output_height = TENSOR_SIZE_INDEX(outputs, 1); /* H */
    const vx_int32 output_depth = TENSOR_SIZE_INDEX(outputs, 2);  /* C */
    const vx_int32 output_batch = TENSOR_SIZE_INDEX(outputs, 3);  /* N */
    const vx_int32 item_size = vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(outputs));
    /* need get from parameter*/
    const vx_int32 crops_top = 0;
    const vx_int32 crops_bottom = 0;
    const vx_int32 crops_left = 0;
    const vx_int32 crops_right = 0;

    vx_int32 in_h = 0, in_w = 0, in_d = 0, in_b = 0;
    vx_float32 data = .0f;
    vx_int32 block_w = 0, block_h = 0;

    vxoTensor_GetTensorViewMemory(inputs, (gctPOINTER*)&inputBase, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, (gctPOINTER*)&outputBase, VX_NULL);
    vxoTensor_GetTensorViewMemory(strides, (gctPOINTER*)&block_size, VX_NULL);

    block_w = block_size[0];
    block_h = block_size[1];

    if(output_width + crops_left + crops_right != input_width * block_w)
    {
        gcmASSERT(0);
    }
    if(output_height + crops_bottom + crops_top != input_height * block_h)
    {
        gcmASSERT(0);
    }
    /*gcmASSERT(input_batch == output_batch * block_w * block_h);*/
    if (output_depth != input_depth)
    {
        gcmASSERT(0);
    }

    /**********************************************************************************************************
    *             intput          block_size      pad(t, b, l, r)    output
    *             W H C N         (w x h: 2x2)     (0, 0, 0, 0)      W H C N
    *             2 2 2 1                  =>                        1 1 2 4
    *
    *
    *                                                                 ___   ___
    *                                                                |1.4| |2.3|
    *                                                                |___| |___|
    *        _______   _______                                        ___   ___
    *       |1.4|3.2| |2.3|4.1|                                      |3.2| |4.1|
    *       |___|___| |___|___|             <=                       |___| |___|
    *       |5.4|7.2| |7.2|8.1|                                       ___   ___
    *       |___|___| |___|___|                                      |5.4| |6.3|
    *                                                                |___| |___|
    *                                                                 ___   ___
    *                                                                |7.2| |8.1|
    *                                                                |___| |___|
    *
    *
    *           intput         block_size     pad(t, b, l, r)       output
    *           C W H N       (w x h: 2x2)    (0, 0, 0, 0)          C W H N
    *           2 2 2 1                 =>                          2 1 1 4
    *
    *                                                                _______
    *                                                               |1.4 2.3|
    *                                                               |_______|
    *           _______________                                     |3.2 4.1|
    *          |1.4 2.3|3.2 4.1|                                    |_______|
    *          |_______|_______|             <=                     |5.4 6.3|
    *          |5.4 6.3|7.2 8.1|                                    |_______|
    *          |_______|_______|                                    |7.2 8.1|
    *                                                               |_______|
    *
    *
    **********************************************************************************************************/

    for (in_b = 0; in_b < input_batch; in_b++)
    {
        vx_int32 input_batch_index = in_b * input_height * input_width * input_depth;
        for (in_d = 0; in_d < input_depth; in_d++)
        {
            for (in_h = 0; in_h < input_height; ++in_h)
            {
                for (in_w = 0; in_w < input_width; in_w++)
                {
                    vx_int32 out_b = in_b % output_batch;
                    vx_int32 spatial_offset= in_b / output_batch;
                    vx_int32 out_h = in_h * block_h + spatial_offset / block_w - crops_top;
                    vx_int32 out_w = in_w * block_w + spatial_offset % block_w - crops_left;
                    vx_int32 output_batch_index = out_b * output_height * output_width * output_depth;
                    vx_int32 out_index = out_w + out_h * output_width + in_d * output_height * output_width + output_batch_index;

                    if (out_w < 0 || out_w >= output_width || out_h < 0 || out_h >= output_height)
                    {
                        continue;
                    }

                    if (in_w < 0 || in_w >= input_width || in_h < 0 || in_h >= input_height)
                    {
                        memset(outputBase + out_index * item_size, 0, item_size);
                    }
                    else
                    {
                        vx_int32 in_index = in_w + in_h * input_width + in_d * input_width * input_height + input_batch_index;

                        if (item_size == vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs)))
                        {
                            memcpy(outputBase + out_index * item_size, inputBase + in_index * item_size, item_size);
                        }
                        else
                        {
                            data = vxnneGetDataExt(inputFormat, TENSOR_QUANT_TYPE(inputs), in_index, inputBase, TENSOR_POS(inputs), TENSOR_TF_ZEROPOINT(inputs), TENSOR_TF_SCALE(inputs));

                            status |= vxnneSaveDataExt(outputFormat, TENSOR_QUANT_TYPE(outputs), out_index, data, outputBase, TENSOR_POS(outputs), TENSOR_TF_ZEROPOINT(outputs), TENSOR_TF_SCALE(outputs), TENSOR_ROUNDING_MODE(outputs));
                        }
                    }

                }
            }
        }
    }



    return status;
}

vx_status vxnneReorg2_Space2BatchND(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_reorg_operation reorgOperation = (vxnne_reorg_operation)operation;

    vx_tensor  inputs  = (vx_tensor)reorgOperation->inputs;
    vx_tensor  strides = (vx_tensor)reorgOperation->stride;
    vx_tensor  pad     = (vx_tensor)reorgOperation->pad;
    vx_tensor  outputs = (vx_tensor)reorgOperation->outputs;

    vx_type_e  inputFormat = (vx_type_e)TENSOR_DATA_TYPE(inputs);
    vx_type_e  outputFormat = (vx_type_e)TENSOR_DATA_TYPE(outputs);

    vx_uint8_ptr inputBase = VX_NULL, outputBase = VX_NULL;
    vx_int32_ptr block_size = VX_NULL, pads = VX_NULL;

    const vx_int32 input_width = TENSOR_SIZE_INDEX(inputs, 0);  /* W */
    const vx_int32 input_height = TENSOR_SIZE_INDEX(inputs, 1); /* H */
    const vx_int32 input_depth = TENSOR_SIZE_INDEX(inputs, 2);  /* C */
    const vx_int32 input_batch = TENSOR_SIZE_INDEX(inputs, 3);  /* N */

    const vx_int32 output_width = TENSOR_SIZE_INDEX(outputs, 0);  /* W */
    const vx_int32 output_height = TENSOR_SIZE_INDEX(outputs, 1); /* H */
    const vx_int32 output_depth = TENSOR_SIZE_INDEX(outputs, 2);  /* C */
    const vx_int32 item_size = vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(outputs));

    vx_int32 in_h = 0, in_w = 0, in_d = 0, in_b = 0;
    vx_float32 data = .0f;
    vx_int32 pad_t = 0, pad_b = 0, pad_l = 0, pad_r = 0;
    vx_int32 block_w = 0, block_h = 0;

    vxoTensor_GetTensorViewMemory(inputs, (gctPOINTER*)&inputBase, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, (gctPOINTER*)&outputBase, VX_NULL);
    vxoTensor_GetTensorViewMemory(strides, (gctPOINTER*)&block_size, VX_NULL);
    vxoTensor_GetTensorViewMemory(pad, (gctPOINTER*)&pads, VX_NULL);

    block_w = block_size[0];
    block_h = block_size[1];

    /* fetch pad height info from pad[1] and weight info from pad[0] */
    pad_l = pads[0];
    pad_r = pads[1];
    pad_t = pads[2];
    pad_b = pads[3];

    gcmASSERT(output_width * block_w == pad_l + input_width + pad_r);
    gcmASSERT(output_height * block_h == pad_t + input_height + pad_b);
    /*gcmASSERT(input_batch * block_w * block_h == output_batch);*/
    if (output_depth != input_depth)
    {
        gcmASSERT(0);
    }

    /******************************************************************************************************************************************************************************************************
    *                 intput               block_size         pad(t, b, l, r)            output       *            intput          block_size      pad(t, b, l, r)    output
    *                 W H C N              (w x h: 2x3)        (1, 1, 2, 4)              W H C N      *            W H C N         (w x h: 2x2)     (0, 0, 0, 0)      W H C N
    *                 2 4 1 1                       =>                                   4 2 1 6      *            2 2 2 1                  =>                        1 1 2 4
    *                                                                                                 *
    *                                                            _______________                      *
    *                                                           | 0 | 0 | 0 | 0 |                     *                                                                ___   ___
    *                                                           |___|___|___|___|                     *                                                               |1.4| |2.3|
    *                                                           | 0 | 5 | 0 | 0 |                     *                                                               |___| |___|
    *                                                           |___|___|___|___|                     *       _______   _______                                        ___   ___
    *        _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _                     _______________                      *      |1.4|3.2| |2.3|4.1|                                      |3.2| |4.1|
    *       | 0 | 0 |  0|  0| 0 | 0 | 0 | 0 |                   | 0 | 0 | 0 | 0 |                     *      |___|___| |___|___|             =>                       |___| |___|
    *       |_ _|_ _|___|___|___|___|___|___|                   |___|___|___|___|                     *      |5.4|7.2| |7.2|8.1|                                       ___   ___
    *       | 0 | 0 |  1|  2| 0 | 0 | 0 | 0 |                   | 0 | 6 | 0 | 0 |                     *      |___|___| |___|___|                                      |5.4| |6.3|
    *       |___|___|___|___|___|___|___|___|                   |___|___|___|___|                     *                                                               |___| |___|
    *       | 0 | 0 |  3|  4| 0 | 0 | 0 | 0 |                    _______________                      *                                                                ___   ___
    *       |___|___|___|___|___|___|___|___|       =>          | 0 | 1 | 0 | 0 |                     *                                                               |7.2| |8.1|
    *       | 0 | 0 |  5|  6| 0 | 0 | 0 | 0 |                   |___|___|___|___|                     *                                                               |___| |___|
    *       |___|___|___|___|___|___|___|___|                   | 0 | 7 | 0 | 0 |                     *
    *       | 0 | 0 |  7|  8| 0 | 0 | 0 | 0 |                   |___|___|___|___|                     *
    *       |___|___|___|___|___|___|___|___|                    _______________                      *          intput         block_size     pad(t, b, l, r)       output
    *       | 0 | 0 |  0|  0| 0 | 0 | 0 | 0 |                   | 0 | 2 | 0 | 0 |                     *          C W H N       (w x h: 2x2)    (0, 0, 0, 0)          C W H N
    *       |_ _|_ _|_ _|_ _|_ _|_ _|_ _|_ _|                   |___|___|___|___|                     *          2 2 2 1                 =>                          2 1 1 4
    *                                                           | 0 | 8 | 0 | 0 |                     *
    *                                                           |___|___|___|___|                     *                                                               _______
    *                                                            _______________                      *                                                              |1.4 2.3|
    *                                                           | 0 | 3 | 0 | 0 |                     *                                                              |_______|
    *                                                           |___|___|___|___|                     *          _______________                                     |3.2 4.1|
    *                                                           | 0 | 0 | 0 | 0 |                     *         |1.4 2.3|3.2 4.1|                                    |_______|
    *                                                           |___|___|___|___|                     *         |_______|_______|             =>                     |5.4 6.3|
    *                                                            _______________                      *         |5.4 6.3|7.2 8.1|                                    |_______|
    *                                                           | 0 | 4 | 0 | 0 |                     *         |_______|_______|                                    |7.2 8.1|
    *                                                           |___|___|___|___|                     *                                                              |_______|
    *                                                           | 0 | 0 | 0 | 0 |                     *
    *                                                           |___|___|___|___|                     *
    *                                                                                                 *
    ******************************************************************************************************************************************************************************************************/

    for (in_b = 0; in_b < input_batch; in_b++)
    {
        for (in_d = 0; in_d < input_depth; in_d++)
        {
            for (in_h = -pad_t; in_h < input_height + pad_b; ++in_h)
            {
                for (in_w = -pad_l; in_w < input_width + pad_r; in_w++)
                {
                    vx_int32 out_w = (in_w + pad_l) / block_w;
                    vx_int32 out_h = (in_h + pad_t) / block_h;
                    vx_int32 out_b = (in_w + pad_l) % block_w + ((in_h + pad_t) % block_h) * block_w;
                    vx_int32 output_batch_index = out_b * output_height * output_width * output_depth * input_batch + in_b * output_height * output_width * output_depth;

                    vx_int32 out_index = out_w + out_h * output_width + in_d * output_height * output_width + output_batch_index;

                    if (in_w < 0 || in_w >= input_width || in_h < 0 || in_h >= input_height)
                    {
                        memset(outputBase + out_index * item_size, 0, item_size);
                    }
                    else
                    {
                        vx_int32 in_index = in_b * input_width * input_height * input_depth + in_w + in_h * input_width + in_d * input_width * input_height;

                        if (item_size == vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs)))
                        {
                            memcpy(outputBase + out_index * item_size, inputBase + in_index * item_size, item_size);
                        }
                        else
                        {
                            data = vxnneGetDataExt(inputFormat, TENSOR_QUANT_TYPE(inputs), in_index, inputBase, TENSOR_POS(inputs), TENSOR_TF_ZEROPOINT(inputs), TENSOR_TF_SCALE(inputs));

                            status |= vxnneSaveDataExt(outputFormat, TENSOR_QUANT_TYPE(outputs), out_index, data, outputBase, TENSOR_POS(outputs), TENSOR_TF_ZEROPOINT(outputs), TENSOR_TF_SCALE(outputs), TENSOR_ROUNDING_MODE(outputs));
                        }
                    }

                }
            }
        }
    }


    return status;
}

vx_status vxnneReorg2_ShuffleChannel(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_reorg_operation reorgOperation = (vxnne_reorg_operation)operation;

    vx_tensor  inputs  = (vx_tensor)reorgOperation->inputs;
    vx_tensor  outputs = (vx_tensor)reorgOperation->outputs;

    vx_uint8_ptr inputBase = VX_NULL, outputBase = VX_NULL;
    vx_int32 i, j, n, len = 1, num = 1, feature_map_size = 1;

    const vx_int32 item_size = vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(outputs));

    vx_int32 num_group = 0, axis = 0, group_size = 0, size = 0, fms_bytes = 0;

    vxoTensor_GetTensorViewMemory(inputs, (gctPOINTER*)&inputBase, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, (gctPOINTER*)&outputBase, VX_NULL);

    num_group = reorgOperation->num_group->value->e;
    axis = reorgOperation->axis->value->e;

    gcmASSERT(axis < 4);

    group_size = inputs->dims[axis] / num_group;


    for (i = 0; i < axis; i++)
    {
        len *= inputs->dims[i];
    }

    for (i = axis + 1; i < (vx_int32)inputs->dimCount; i++)
    {
        num *= inputs->dims[i];
    }

    for (i = 0; i <= axis; i++)
    {
        feature_map_size *= inputs->dims[i];
    }

    /* Shuffle Channel CPU Implement, the shape and dtype of output must same as input */
    size = len * item_size;
    fms_bytes = feature_map_size * item_size;
    for (n = 0; n < num; n++)
    {
        for (i = 0; i < num_group; i++)
        {
            for (j = 0; j < group_size; j++)
            {
                memcpy(outputBase + n * fms_bytes + (j * num_group + i) * size, inputBase + n * fms_bytes + (i * group_size + j) * size, size);
            }
        }
    }

    return status;
}

vx_status vxnneExecuteSWReorg2(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_reorg_operation reorgOperation   = (vxnne_reorg_operation)operation;

    vx_scalar  types            = (vx_scalar)reorgOperation->type;

    vx_enum    type             = types->value->e;

    switch (type)
    {
        case VX_REORG_DEPTH_TO_SPACE:
            vxnneReorg2_Depth2Space(operation);
            break;
        case VX_REORG_SPACE_TO_DEPTH:

            vxnneReorg2_Space2Depth(operation);
            break;

        case VX_REORG_BATCH_TO_SPACE_ND:
            vxnneReorg2_Batch2SpaceND(operation);
            break;

        case VX_REORG_SPACE_TO_BATCH_ND:
            vxnneReorg2_Space2BatchND(operation);
            break;

        case VX_REORG_SHUFFLE_CHANNEL:
            vxnneReorg2_ShuffleChannel(operation);
            break;

        default:
            break;
    }

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNReOrg2(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoReOrg2_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxoReOrg2_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status _GetTPReorgCmdType(vx_enum reorg_type,
                                            vx_tp_cmd_type_e *cmd_type)
{
    vx_status status = VX_SUCCESS;

    if (!cmd_type)
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    switch (reorg_type)
    {
    case VX_REORG_DEPTH_TO_SPACE:
        *cmd_type = TP_REORG_DEPTH2SPACE;
        break;

    case VX_REORG_SPACE_TO_DEPTH:
        *cmd_type = TP_REORG_SPACE2DEPTH;
        break;

    case VX_REORG_SPACE_TO_BATCH_ND:
        *cmd_type = TP_REORG_SPACE2BATCH;
        break;

    case VX_REORG_BATCH_TO_SPACE_ND:
        *cmd_type = TP_REORG_BATCH2SPACE;
        break;

    case VX_REORG_SHUFFLE_CHANNEL:
        *cmd_type = TP_REORG_SHUFFLECHANNEL;
        break;

    default:
        status = VX_ERROR_NOT_SUPPORTED;
        break;
    }

    return status;
}

VX_PRIVATE_API vx_status _InitializeReorg2OperationTP(
    vxnne_reorg_layer layer,
    vx_tensor input,
    vx_tensor output,
    vx_uint32 batch_count,
    vx_tensor block_size,
    vx_scalar type_s,
    vx_tensor pad,
    vx_scalar num_group_s,
    vx_scalar axis_s,
    vx_uint32 *op_index)
{
    vx_status status = VX_SUCCESS;

    vx_op_param op_param = VX_NULL;
    vx_tp_cmd_type_e tp_cmd_type = TP_NONE;
    vx_uint32 block_size_width = (vx_uint32)VX_GET_DATA_FROM_TENSOR(block_size, 0);
    vx_uint32 block_size_height = (vx_uint32)VX_GET_DATA_FROM_TENSOR(block_size, 1);
    vx_enum type = type_s->value->e;

    if (!op_index)
    {
        vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
    }

    vxmONERROR(_GetTPReorgCmdType(type, &tp_cmd_type));

    vxmONERROR(vxnneOperation_Initialize(&layer->reorg_tp_operation.base,
                                         &layer->base,
                                         VXNNE_OPERATION_TARGET_TP,
                                         VXNNE_OPERATOR_REORG2,
                                         VX_NULL,
                                         vxnneOperation_TP_Deinitialize,
                                         batch_count,
                                         0));

    op_param = &layer->reorg_tp_operation.base.parameter;

    /* fetch pad height info from pad[1] and weight info from pad[0] */
    if (pad)
    {
        op_param->pad_x_left   = (vx_uint32)VX_GET_DATA_FROM_TENSOR(pad, 0);
        op_param->pad_x_right  = (vx_uint32)VX_GET_DATA_FROM_TENSOR(pad, 1);
        op_param->pad_y_top    = (vx_uint32)VX_GET_DATA_FROM_TENSOR(pad, 2);
        op_param->pad_y_bottom = (vx_uint32)VX_GET_DATA_FROM_TENSOR(pad, 3);
    }

    op_param->pool_size_x = 0;
    op_param->pool_size_y = 0;
    op_param->pool_stride = 1;
    op_param->enable_relu = vx_false_e;
    op_param->pad_mode    = VX_PAD_CONSTANT;
    op_param->pad_const   = TENSOR_PAD_ZERO_VALUE(input);
    op_param->tpType      = tp_cmd_type;
    op_param->other_ref   = gcvNULL;
    op_param->data_buff   = gcvNULL;

    op_param->tp_value = (vx_tp_value_cmd)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
    if (!op_param->tp_value)
    {
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }
    op_param->tp_value->u32[0] = block_size_width;
    op_param->tp_value->u32[1] = block_size_height;
    op_param->tp_value->u32[2] = TENSOR_VIEW_SIZE_INDEX(input, 3);

    if (num_group_s)
    {
        op_param->tp_value->u32[3] = num_group_s->value->e;
    }

    if (axis_s)
    {
        op_param->tp_value->u32[4] = axis_s->value->e;
    }
    vxmONERROR(vxnneLayer_SetOperation(&layer->base,
                                       &layer->reorg_tp_operation.base,
                                       (*op_index)++));

    layer->reorg_tp_operation.input  = input;
    layer->reorg_tp_operation.output = output;

    vxnneOperation_AddReference(&layer->reorg_tp_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
    vxnneOperation_AddReference(&layer->reorg_tp_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

OnError:
    return status;
}

VX_PRIVATE_API vx_status _InitializeReorg2OperationSH(
    vxnne_reorg_layer reorgLayer,
    vx_tensor inputs,
    vx_tensor outputs,
    vx_uint32 batchCount,
    vx_tensor block_size_s,
    vx_scalar type_s,
    vx_tensor pad,
    vx_uint32 *op_index,
    vx_scalar num_group_s,
    vx_scalar axis_s,
    vx_bool evis)
{
    vx_status status = VX_SUCCESS;

    vx_scalar  stride;
    vx_uint32  block_size  = (vx_uint32)VX_GET_DATA_FROM_TENSOR(block_size_s, 0);
    vx_uint32 pad_list[4]  = {0};
    vx_uint32  input_depth = TENSOR_SIZE_INDEX(inputs, 2);
    vx_scalar outc_s       = vxCreateScalar(reorgLayer->base.node->base.context, VX_TYPE_UINT32, &input_depth);
    vx_enum type           = type_s->value->e;
    vx_node node           = reorgLayer->base.node;
    vx_context context     = vxGetContext((vx_reference)node);
    vxnne_shader_executable shaderExecutable = NULL;

    if(pad)
    {
        pad_list[0]  = (vx_uint32)VX_GET_DATA_FROM_TENSOR(pad, 0);
        pad_list[1]  = (vx_uint32)VX_GET_DATA_FROM_TENSOR(pad, 1);
        pad_list[2]  = (vx_uint32)VX_GET_DATA_FROM_TENSOR(pad, 2);
        pad_list[3]  = (vx_uint32)VX_GET_DATA_FROM_TENSOR(pad, 3);
    }

    stride = vxCreateScalar(context, VX_TYPE_UINT32, &block_size);

    if(evis)
    {
        if (type == VX_REORG_DEPTH_TO_SPACE)
            shaderExecutable = vxnneGetDepth2SpaceShaderExecutable(context, VXNNE_KERNEL_DEPTH2SPACE, &node->kernelAttributes.borderMode,
                inputs, stride, outputs);
        else if(type == VX_REORG_SPACE_TO_DEPTH)
            shaderExecutable = vxnneGetSpace2DepthShaderExecutable(context, VXNNE_KERNEL_SPACE2DEPTH, &node->kernelAttributes.borderMode,
                inputs, stride, outc_s, outputs);
        else if(type == VX_REORG_SPACE_TO_BATCH_ND && pad)
            shaderExecutable = vxnneGetSpace2BatchShaderExecutable(context, VXNNE_KERNEL_SPACE2BATCH, &node->kernelAttributes.borderMode,
                inputs, block_size_s, pad, outc_s, outputs, pad_list);
        else if(type == VX_REORG_BATCH_TO_SPACE_ND)
            shaderExecutable = vxnneGetBatch2SpaceShaderExecutable(context, VXNNE_KERNEL_BATCH2SPACE, &node->kernelAttributes.borderMode,
                inputs, pad_list[0], pad_list[2], block_size_s, outc_s, outputs);
        else if(type == VX_REORG_SHUFFLE_CHANNEL)
            shaderExecutable = vxnneGetShuffleChannelShaderExecutable(context, VXNNE_KERNEL_SHUFFLECHANNEL, &node->kernelAttributes.borderMode,
                inputs, num_group_s, axis_s, outputs);
    }
    else
    {
        if (type == VX_REORG_DEPTH_TO_SPACE)
            shaderExecutable = vxnneGetGPUDepth2SpaceShaderExecutable(context, VXNNE_KERNEL_GPU_DEPTH2SPACE, &node->kernelAttributes.borderMode,
                inputs, stride, outputs);
        else if(type == VX_REORG_SPACE_TO_DEPTH)
            shaderExecutable = vxnneGetGPUSpace2DepthShaderExecutable(context, VXNNE_KERNEL_GPU_SPACE2DEPTH, &node->kernelAttributes.borderMode,
                inputs, stride, outputs);
        else if(type == VX_REORG_BATCH_TO_SPACE_ND)
            shaderExecutable = vxnneGetGPUBatch2SpaceShaderExecutable(context, VXNNE_KERNEL_GPU_BATCH2SPACE, &node->kernelAttributes.borderMode,
                inputs, pad_list[0], pad_list[2], block_size_s, outputs);
        else if(type == VX_REORG_SPACE_TO_BATCH_ND)
            shaderExecutable = vxnneGetGPUSpace2BatchShaderExecutable(context, VXNNE_KERNEL_GPU_SPACE2BATCH, &node->kernelAttributes.borderMode,
                inputs, block_size_s, outc_s, outputs, pad_list);
        else if(type == VX_REORG_SHUFFLE_CHANNEL)
            shaderExecutable = vxnneGetGPUShuffleChannelShaderExecutable(context, VXNNE_KERNEL_GPU_SHUFFLECHANNEL, &node->kernelAttributes.borderMode,
                inputs, num_group_s, axis_s, outputs);

        if(type == VX_REORG_BATCH_TO_SPACE_ND || type == VX_REORG_SPACE_TO_BATCH_ND)
            batchCount = 1;
    }

    if (!shaderExecutable)
    {
        status = VX_FAILURE;
        if (outc_s) vxReleaseScalar(&outc_s);
        if (stride) vxReleaseScalar(&stride);
        goto OnError;
    }

    status = vxnneShaderOperation_Initialize(&reorgLayer->reorg_sh_operation,
        &reorgLayer->base,
        VXNNE_OPERATOR_REORG2,
        batchCount,
        shaderExecutable);

    if (status != VX_SUCCESS)
    {
        if (outc_s) vxReleaseScalar(&outc_s);
        if (stride) vxReleaseScalar(&stride);
        if (outc_s) vxReleaseScalar(&outc_s);
        goto OnError;
    }

    vxnneOperation_AddReference(&reorgLayer->reorg_sh_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
    vxnneOperation_AddReference(&reorgLayer->reorg_sh_operation.base, (vx_reference)block_size_s, VXNNE_OPERATION_REFENRENCE_INPUT);
    if(pad)
        vxnneOperation_AddReference(&reorgLayer->reorg_sh_operation.base, (vx_reference)pad, VXNNE_OPERATION_REFENRENCE_INPUT);
    vxnneOperation_AddReference(&reorgLayer->reorg_sh_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

    vxnneLayer_SetOperation(
        &reorgLayer->base,
        &reorgLayer->reorg_sh_operation.base,
        0);

    if (outc_s) vxReleaseScalar(&outc_s);
    if (stride) vxReleaseScalar(&stride);

OnError:
    return status;
}

VX_PRIVATE_API vx_status _InitializeReorg2OperationSW(
    vxnne_reorg_layer layer,
    vx_tensor input,
    vx_tensor output,
    vx_uint32 batch_count,
    vx_tensor block_size,
    vx_scalar type_s,
    vx_tensor pad,
    vx_scalar num_group_s,
    vx_scalar axis_s,
    vx_uint32 *op_index)
{
    vx_status status = VX_SUCCESS;

    if (!op_index)
    {
        vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
    }

    vxmONERROR(vxnneOperation_Initialize(&layer->reorg_sw_operation.base,
                                         &layer->base,
                                         VXNNE_OPERATION_TARGET_SW,
                                         VXNNE_OPERATOR_REORG2,
                                         vxnneExecuteSWReorg2,
                                         VX_NULL,
                                         batch_count,
                                         0));

    vxmONERROR(vxnneLayer_SetOperation(&layer->base,
                                       &layer->reorg_sw_operation.base,
                                       (*op_index)++));

    layer->reorg_sw_operation.inputs  = input;
    layer->reorg_sw_operation.stride  = (vx_reference)block_size;
    layer->reorg_sw_operation.type    = type_s;
    layer->reorg_sw_operation.pad     = pad;
    layer->reorg_sw_operation.outputs = output;
    layer->reorg_sw_operation.num_group = num_group_s;
    layer->reorg_sw_operation.axis = axis_s;


    vxnneOperation_AddReference(&layer->reorg_sw_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
    vxnneOperation_AddReference(&layer->reorg_sw_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

OnError:
    return status;
}
#if REGISTER_FRAME

VX_PRIVATE_API vx_status vxoNNReorgLayer2_SW_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vx_tensor inputs = (vx_tensor)parameters[0];
    vx_tensor block_size = (vx_tensor)parameters[1];
    vx_scalar type_s = (vx_scalar)parameters[2];
    vx_tensor pad = (vx_tensor)parameters[3];
    vx_tensor outputs = (vx_tensor)parameters[4];
    vx_scalar num_group_s = (vx_scalar)parameters[5];
    vx_scalar axis_s = (vx_scalar)parameters[6];
    vx_enum type = type_s->value->e;
    vx_uint32 batch_count = (type == VX_REORG_BATCH_TO_SPACE_ND || type == VX_REORG_SPACE_TO_BATCH_ND) ? 1 : TENSOR_SIZE_INDEX(inputs, 3);
    vx_uint32 op_index = 0;
    vxnne_reorg_layer  reorg_layer = (vxnne_reorg_layer)ops_layer;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(_InitializeReorg2OperationSW(reorg_layer,
        inputs,
        outputs,
        batch_count,
        block_size,
        type_s,
        pad,
        num_group_s,
        axis_s,
        &op_index));

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);
OnError:
    return status;
}

VX_PRIVATE_API vx_bool vxoNNReorgLayer2_SH_EVIS_Support_Ext(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_tensor inputs = (vx_tensor)parameters[0];
    vx_scalar type_s = (vx_scalar)parameters[2];
    vx_tensor outputs = (vx_tensor)parameters[4];
    vx_scalar axis_s = (vx_scalar)parameters[6];
    vx_enum    type = type_s->value->e;
    vx_bool    dataFormat_flag[5] = { vx_false_e };
    vx_bool    depth2Space_flag   = vx_false_e;
    vx_bool    space2Depth_flag   = vx_false_e;
    vx_bool    space2Batch_flag   = vx_false_e;
    vx_bool    batch2Space_flag   = vx_false_e;
    vx_bool    shuffle_flag       = vx_false_e;
    vx_int8    in_fixpoint  = TENSOR_POS(inputs);
    vx_int8    out_fixpoint = TENSOR_POS(outputs);
    vx_enum    inputFormat  = TENSOR_DATA_TYPE(inputs);
    vx_enum    outputFormat = TENSOR_DATA_TYPE(outputs);

    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    dataFormat_flag[0] = (vx_bool)((inputFormat == VX_TYPE_UINT8) && (outputFormat == VX_TYPE_UINT8));
    dataFormat_flag[1] = (vx_bool)((inputFormat == VX_TYPE_FLOAT16) && (outputFormat == VX_TYPE_FLOAT16));
    dataFormat_flag[2] = (vx_bool)((inputFormat == VX_TYPE_INT16) && (outputFormat == VX_TYPE_INT16) && (in_fixpoint == out_fixpoint));
    dataFormat_flag[3] = (vx_bool)((inputFormat == VX_TYPE_INT8) && (outputFormat == VX_TYPE_INT8) && (in_fixpoint == out_fixpoint));
    dataFormat_flag[4] = (vx_bool)((inputFormat == VX_TYPE_FLOAT32) && (outputFormat == VX_TYPE_FLOAT32) && (in_fixpoint == out_fixpoint));
    depth2Space_flag = (vx_bool)(type == VX_REORG_DEPTH_TO_SPACE && (dataFormat_flag[0] || dataFormat_flag[1]));
    space2Depth_flag = (vx_bool)(type == VX_REORG_SPACE_TO_DEPTH && (dataFormat_flag[0] || dataFormat_flag[1] || dataFormat_flag[2] || dataFormat_flag[3]));
    space2Batch_flag = (vx_bool)(type == VX_REORG_SPACE_TO_BATCH_ND && (dataFormat_flag[0] || dataFormat_flag[1]));
    batch2Space_flag = (vx_bool)(type == VX_REORG_BATCH_TO_SPACE_ND && (dataFormat_flag[0] || dataFormat_flag[1]));
    shuffle_flag     = (vx_bool)(type == VX_REORG_SHUFFLE_CHANNEL && (axis_s->value->n32 <= 2) && _IsSameType(inputs, outputs)
                                && (dataFormat_flag[0] || dataFormat_flag[1] || dataFormat_flag[2] || dataFormat_flag[3]));

    if (!evis)
    {
        depth2Space_flag = depth2Space_flag || (type == VX_REORG_DEPTH_TO_SPACE && dataFormat_flag[4]);
        space2Depth_flag = space2Depth_flag || (type == VX_REORG_SPACE_TO_DEPTH && dataFormat_flag[4]);
        space2Batch_flag = space2Batch_flag || (type == VX_REORG_SPACE_TO_BATCH_ND && dataFormat_flag[4]);
        batch2Space_flag = batch2Space_flag || (type == VX_REORG_BATCH_TO_SPACE_ND && dataFormat_flag[4]);
        shuffle_flag     = shuffle_flag     || (type == VX_REORG_SHUFFLE_CHANNEL && dataFormat_flag[4]);
    }

    support = support && (depth2Space_flag || space2Depth_flag || space2Batch_flag || batch2Space_flag || shuffle_flag);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNReorgLayer2_SH_EVIS_Initialize_Ext(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_status status = VX_SUCCESS;

    vx_tensor inputs = (vx_tensor)parameters[0];
    vx_tensor block_size = (vx_tensor)parameters[1];
    vx_scalar type_s = (vx_scalar)parameters[2];
    vx_tensor pad = (vx_tensor)parameters[3];
    vx_tensor outputs = (vx_tensor)parameters[4];
    vx_scalar num_group_s = (vx_scalar)parameters[5];
    vx_scalar axis_s = (vx_scalar)parameters[6];
    vx_enum type = type_s->value->e;
    vx_uint32 batch_count = (type == VX_REORG_BATCH_TO_SPACE_ND || type == VX_REORG_SPACE_TO_BATCH_ND) ? 1 : TENSOR_SIZE_INDEX(inputs, 3);
    vx_uint32 op_index = 0;

    vxnne_reorg_layer  reorg_layer = (vxnne_reorg_layer)ops_layer;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(_InitializeReorg2OperationSH(reorg_layer,
        inputs,
        outputs,
        batch_count,
        block_size,
        type_s,
        pad,
        &op_index,
        num_group_s,
        axis_s,
        evis));

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

OnError:
    return status;
}


VX_PRIVATE_API vx_bool vxoNNReorgLayer2_SH_EVIS_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && node->base.context->evisNoInst.supportEVIS;

    if (!support)return support;

    support = support && vxoNNReorgLayer2_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_true_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNReorgLayer2_SH_EVIS_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxoNNReorgLayer2_SH_EVIS_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_true_e));

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

OnError:
    return status;
}
VX_PRIVATE_API vx_bool vxoNNReorgLayer2_SH_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    support = support && vxoNNReorgLayer2_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_false_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNReorgLayer2_SH_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxoNNReorgLayer2_SH_EVIS_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_false_e));

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

OnError:
    return status;
}

VX_PRIVATE_API vx_bool vxoNNReorgLayer2_TP_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_tensor inputs = (vx_tensor)parameters[0];
    vx_scalar type_s = (vx_scalar)parameters[2];
    vx_tensor outputs = (vx_tensor)parameters[4];
    vx_tensor pad = (vx_tensor)parameters[3];
    vx_enum type = type_s->value->e;
    vx_tp_cmd_type_e tp_cmd_type = TP_NONE;

    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_TP, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    support = support && (vxnneIsTPSupportFormat(node->base.context, inputs, VX_NULL, outputs) &&
        !vxmIS_ERROR(_GetTPReorgCmdType(type, &tp_cmd_type)));

    if (type == VX_REORG_BATCH_TO_SPACE_ND)
    {
        const vx_int32 left = TENSOR_SIZE_INDEX(pad, 0);
        const vx_int32 right = TENSOR_SIZE_INDEX(pad, 1);

        const vx_int32 top = TENSOR_SIZE_INDEX(pad, 2);
        const vx_int32 bottom = TENSOR_SIZE_INDEX(pad, 3);
        support = support && (left == 0) && (right == 0) && (top == 0) && (bottom == 0);
    }


    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNReorgLayer2_TP_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vx_tensor inputs = (vx_tensor)parameters[0];
    vx_tensor block_size = (vx_tensor)parameters[1];
    vx_scalar type_s = (vx_scalar)parameters[2];
    vx_tensor pad = (vx_tensor)parameters[3];
    vx_tensor outputs = (vx_tensor)parameters[4];
    vx_scalar num_group_s = (vx_scalar)parameters[5];
    vx_scalar axis_s = (vx_scalar)parameters[6];
    vx_enum type = type_s->value->e;
    vx_uint32 batch_count = (type == VX_REORG_BATCH_TO_SPACE_ND || type == VX_REORG_SPACE_TO_BATCH_ND) ? 1 : TENSOR_SIZE_INDEX(inputs, 3);

    vx_uint32 op_index = 0;
    vxnne_reorg_layer  reorg_layer = (vxnne_reorg_layer)ops_layer;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(_InitializeReorg2OperationTP(reorg_layer,
        inputs,
        outputs,
        batch_count,
        block_size,
        type_s,
        pad,
        num_group_s,
        axis_s,
        &op_index));

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

OnError:
    return status;
}

VX_PRIVATE_API vx_status vxoNNLayer_GetOperations2(vxnne_layer ops_layer, vx_uint32_ptr max_num_operations, vxnne_operation **operations)
{
    vx_status  status = VX_SUCCESS;
    vxnne_reorg_layer  ReorgLayer2 = (vxnne_reorg_layer)ops_layer;

    *max_num_operations = gcmCOUNTOF(ReorgLayer2->operations);

    *operations = ReorgLayer2->operations;

    return status;
}
#else

VX_PRIVATE_API vx_status _InitializeReorg2Operation(
    vxnne_reorg_layer layer,
    vxnne_operation_target_e target,
    vx_tensor input,
    vx_tensor output,
    vx_uint32 batch_count,
    vx_tensor block_size,
    vx_scalar type_s,
    vx_tensor pad,
    vx_uint32 *op_index,
    vx_scalar num_group_s,
    vx_scalar axis_s)
{
    vx_status status = VX_SUCCESS;

    if (!op_index)
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    switch (target)
    {
    case VXNNE_OPERATION_TARGET_TP:
        vxmONERROR(_InitializeReorg2OperationTP(layer,
                                                input,
                                                output,
                                                batch_count,
                                                block_size,
                                                type_s,
                                                pad,
                                                op_index));
        break;

    case VXNNE_OPERATION_TARGET_SH:
        vxmONERROR(_InitializeReorg2OperationSH(layer,
                                                input,
                                                output,
                                                batch_count,
                                                block_size,
                                                type_s,
                                                pad,
                                                op_index,
                                                num_group_s,
                                                axis_s,
                                                layer->base.node->base.context->evisNoInst.supportEVIS));
        break;

    case VXNNE_OPERATION_TARGET_SW:
        vxmONERROR(_InitializeReorg2OperationSW(layer,
                                                input,
                                                output,
                                                batch_count,
                                                block_size,
                                                type_s,
                                                pad,
                                                op_index));
        break;

    default:
        status = VX_ERROR_NOT_SUPPORTED;
        break;
    }

OnError:
    return status;
}
#endif



VX_PRIVATE_API vx_status VX_CALLBACK vxoReOrg2_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status                = VX_SUCCESS;
#if REGISTER_FRAME

    vxnne_layer_imp_s registerReorgLayer2s[] = {/* Please DON'T adjust the order, it's importent */
        { "ReorgLayer2 NN", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "ReorgLayer2 TP", vxoNNReorgLayer2_TP_Support, vxoNNReorgLayer2_TP_Initialize, VX_NULL },
        { "ReorgLayer2 SH EVIS", vxoNNReorgLayer2_SH_EVIS_Support, vxoNNReorgLayer2_SH_EVIS_Initialize, VX_NULL },
        { "ReorgLayer2 SH", vxoNNReorgLayer2_SH_Support, vxoNNReorgLayer2_SH_Initialize, VX_NULL },
        { "ReorgLayer2 SW ", vxoNNCommon_Support, vxoNNReorgLayer2_SW_Initialize, VX_NULL },
    };

    REGISTER_LAYERS(registerReorgLayer2s, vxnne_reorg_layer_s, "ReorgLayer2", vxoNNLayer_GetOperations2);

OnError:
#else
    vx_tensor inputs                = (vx_tensor)parameters[0];
    vx_tensor block_size            = (vx_tensor)parameters[1];
    vx_scalar type_s                = (vx_scalar)parameters[2];
    vx_tensor pad                   = (vx_tensor)parameters[3];
    vx_tensor outputs               = (vx_tensor)parameters[4];
    vx_scalar num_group_s           = (vx_scalar)parameters[5];
    vx_scalar axis_s                = (vx_scalar)parameters[6];
    vx_context context              = vxGetContext((vx_reference)node);
    vx_enum type                    = type_s->value->e;
    vx_uint32 batch_count           = (type == VX_REORG_BATCH_TO_SPACE_ND || type == VX_REORG_SPACE_TO_BATCH_ND) ? 1 : TENSOR_SIZE_INDEX(inputs, 3);
    vxnne_operation_target_e target = VXNNE_OPERATION_TARGET_NONE;
    vx_tp_cmd_type_e tp_cmd_type    = TP_NONE;
    vxnne_reorg_layer reorg_layer   = VX_NULL;
    vx_uint32 op_index              = 0;
    vx_bool    dataFormat_flag[5]   = {vx_false_e};
    vx_bool    depth2Space_flag     = vx_false_e;
    vx_bool    space2Depth_flag     = vx_false_e;
    vx_bool    space2Batch_flag     = vx_false_e;
    vx_bool    batch2Space_flag     = vx_false_e;
    vx_bool    shuffle_flag         = vx_false_e;
    vx_int8    in_fixpoint          = TENSOR_POS(inputs);
    vx_int8    out_fixpoint         = TENSOR_POS(outputs);
    vx_enum    inputFormat          = TENSOR_DATA_TYPE(inputs);
    vx_enum    outputFormat         = TENSOR_DATA_TYPE(outputs);

    dataFormat_flag[0] = (vx_bool)((inputFormat == VX_TYPE_UINT8) && (outputFormat == VX_TYPE_UINT8));
    dataFormat_flag[1] = (vx_bool)((inputFormat == VX_TYPE_FLOAT16) && (outputFormat == VX_TYPE_FLOAT16));
    dataFormat_flag[2] = (vx_bool)((inputFormat == VX_TYPE_INT16) && (outputFormat == VX_TYPE_INT16) && (in_fixpoint == out_fixpoint));
    dataFormat_flag[3] = (vx_bool)((inputFormat == VX_TYPE_INT8) && (outputFormat == VX_TYPE_INT8) && (in_fixpoint == out_fixpoint));
    dataFormat_flag[4] = (vx_bool)((inputFormat == VX_TYPE_FLOAT32) && (outputFormat == VX_TYPE_FLOAT32) && (in_fixpoint == out_fixpoint));
    depth2Space_flag   = (vx_bool)(type == VX_REORG_DEPTH_TO_SPACE && (dataFormat_flag[0] || dataFormat_flag[1] || dataFormat_flag[4]));
    space2Depth_flag   = (vx_bool)(type == VX_REORG_SPACE_TO_DEPTH && (dataFormat_flag[0] || dataFormat_flag[1] || dataFormat_flag[2] || dataFormat_flag[3] || dataFormat_flag[4]));
    space2Batch_flag   = (vx_bool)(type == VX_REORG_SPACE_TO_BATCH_ND && (dataFormat_flag[0] || dataFormat_flag[1] || dataFormat_flag[4]));
    batch2Space_flag   = (vx_bool)(type == VX_REORG_BATCH_TO_SPACE_ND && (dataFormat_flag[0] || dataFormat_flag[1] || dataFormat_flag[4]));
    shuffle_flag       = (vx_bool)(type == VX_REORG_SHUFFLE_CHANNEL && (dataFormat_flag[0] || dataFormat_flag[1] || dataFormat_flag[2] || dataFormat_flag[3]));
    /* Destroy the existing layer. */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    /* Create the layer. */
    reorg_layer = (vxnne_reorg_layer)vxAllocate(sizeof(vxnne_reorg_layer_s));
    if (!reorg_layer)
    {
        vxError("Allocate memory fail at function %s line %d.", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }

    vxmONERROR(vxnneLayer_Initialize(&reorg_layer->base,
                                     "ReorgLayer2",
                                     node,
                                     vxmOPERATION_COUNT(reorg_layer),
                                     reorg_layer->operations,
                                     VX_NULL));

    /* Choose acceleration path. */
    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP) &&
        vxnneIsTPSupportFormat(context, inputs, VX_NULL, outputs) &&
        !vxmIS_ERROR(_GetTPReorgCmdType(type, &tp_cmd_type)))
    {
        target = VXNNE_OPERATION_TARGET_TP;
    }
    else if ((depth2Space_flag || space2Depth_flag || space2Batch_flag || batch2Space_flag || shuffle_flag) && vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER))
    {
        target = VXNNE_OPERATION_TARGET_SH;
    }
    else
    {
        target = VXNNE_OPERATION_TARGET_SW;
    }

    /* Initialize the operation. */
    vxmONERROR(_InitializeReorg2Operation(reorg_layer,
                                          target,
                                          inputs,
                                          outputs,
                                          batch_count,
                                          block_size,
                                          type_s,
                                          pad,
                                          &op_index,
                                          num_group_s,
                                          axis_s));

    node->layer = &reorg_layer->base;

    return status;

OnError:
    if (reorg_layer)
    {
        if (reorg_layer->reorg_tp_operation.base.parameter.tp_value)
        {
            gcoOS_Free(gcvNULL, (gctPOINTER)reorg_layer->reorg_tp_operation.base.parameter.tp_value);
        }
        gcoOS_Free(gcvNULL, (gctPOINTER)reorg_layer);
    }
#endif

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoReOrg2_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

