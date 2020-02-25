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
#include <layers/gc_vx_layer_eltwise.h>


vx_status vxnneExecuteSWEltwise(struct _vxnne_operation_s *operation)
{
    vxnne_eltwise_sw_operation eltwiseOperation   = (vxnne_eltwise_sw_operation)operation;

    vx_tensor input1 = eltwiseOperation->input1;
    vx_tensor input2 = eltwiseOperation->input2;
    vx_tensor output = eltwiseOperation->output;

    vx_enum kernel = eltwiseOperation->kernel;
    vx_int32 dim1 = input1->viewRegion.dimCount;
    vx_int32 dim2 = input2->viewRegion.dimCount;
    vx_enum overflow = eltwiseOperation->overflow->value->e;

    if (dim1 == dim2)
    {
        switch(kernel)
        {
        case VX_KERNEL_TENSOR_ADD:
            eltwise(input1, input2, 1.f, overflow, VX_ROUND_POLICY_TO_ZERO, VX_TENSOR_OP_ADD, output);
            break;
        case VX_KERNEL_TENSOR_SUBTRACT:
            eltwise(input1, input2, 1.f, overflow, VX_ROUND_POLICY_TO_ZERO, VX_TENSOR_OP_SUB, output);
            break;
        case VX_KERNEL_TENSOR_MULTIPLY:
            {
                vx_enum rounding = eltwiseOperation->rounding->value->e;
                vx_float32 scale = eltwiseOperation->scale->value->f32;
                eltwise(input1, input2, scale, overflow, rounding, VX_TENSOR_OP_MUL, output);
            }
            break;
        default:
            vxError("Not support kenrel: %d\n", kernel);
            break;
        }
    }
    else
        vxError("Difference dim\n");
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorAdd(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorAdd_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorAdd_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_uint32 getTensorOffset(vx_uint32 index, vx_tensor inputTensor, vx_uint32 * out_dims)
{
    vx_uint32 offset = 0;
    vx_uint32 i;

    for(i = 0; i < inputTensor->dimCount; i++)
    {
        offset += inputTensor->strides[i] * (index % out_dims[i]);

        index /= out_dims[i];
    }

    return offset;
}

VX_PRIVATE_API vx_uint32 getExpandTensorOffset(vx_uint32 index, vx_tensor inputTensor, vx_uint32 * out_dims)
{
    vx_uint32 offset = 0;
    vx_uint32 i;

    for(i = 0; i < inputTensor->dimCount; i++)
    {
        if(inputTensor->dims[i] == out_dims[i])
            offset += inputTensor->strides[i] * (index % out_dims[i]);

        index /= out_dims[i];
    }

    return offset;
}

vx_status vxnneExecuteSWTensorAdd(vxnne_operation operation)
{
    vxnne_tensor_add_operation eltwiseOperation   = (vxnne_tensor_add_operation)operation;

    vx_tensor input1 = eltwiseOperation->input0;
    vx_tensor input2 = eltwiseOperation->input1;
    vx_tensor output = eltwiseOperation->output;
    vx_uint32 dims   = TENSOR_DIM_NUM(output);

    {
        vx_uint8_ptr input1base, input2base, outputbase;
        vx_uint32 i, numCount = 1;

        vxoTensor_GetTensorViewMemory(input1, (gctPOINTER*)&input1base, VX_NULL);
        vxoTensor_GetTensorViewMemory(input2, (gctPOINTER*)&input2base, VX_NULL);
        vxoTensor_GetTensorViewMemory(output, (gctPOINTER*)&outputbase, VX_NULL);

        for (i = 0; i < dims; i++)
        {
            numCount *= output->dims[i];
        }

        for (i = 0; i < numCount; i++)
        {
            vx_uint32 in1offset, in2offset, outoffset;
            vx_int8_ptr in1, in2, out;
            vx_status status = VX_SUCCESS;
            vx_float32 in1Data_fl32, in2Data_fl32;

            in1offset = getExpandTensorOffset(i, input1, output->dims);
            in2offset = getExpandTensorOffset(i, input2, output->dims);
            outoffset = getTensorOffset(i, output, output->dims);

            in1 = (vx_int8_ptr)input1base + in1offset;
            in2 = (vx_int8_ptr)input2base + in2offset;
            out = (vx_int8_ptr)outputbase + outoffset;

            in1Data_fl32 = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(input1), TENSOR_QUANT_TYPE(input1), 0, (vx_uint8_ptr)in1,
                TENSOR_POS(input1), TENSOR_TF_ZEROPOINT(input1), TENSOR_TF_SCALE(input1));
            in2Data_fl32 = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(input2), TENSOR_QUANT_TYPE(input2), 0, (vx_uint8_ptr)in2,
                TENSOR_POS(input2), TENSOR_TF_ZEROPOINT(input2), TENSOR_TF_SCALE(input2));
            status = vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(output), TENSOR_QUANT_TYPE(output), 0, in1Data_fl32 + in2Data_fl32, (vx_uint8_ptr)out,
                TENSOR_POS(output), TENSOR_TF_ZEROPOINT(output), TENSOR_TF_SCALE(output), TENSOR_ROUNDING_MODE(output));
        }
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorAdd_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    vx_tensor input0   = (vx_tensor)parameters[0];
    vx_tensor input1   = (vx_tensor)parameters[1];
    vx_scalar policy   = (vx_scalar)parameters[2];
    vx_tensor output   = (vx_tensor)parameters[3];
    vx_context context = vxGetContext((vx_reference)node);

    vx_type_e input0Format          = TENSOR_DATA_TYPE(input0);
    vx_type_e input1Format          = TENSOR_DATA_TYPE(input1);
    vx_type_e outputFormat          = TENSOR_DATA_TYPE(output);

    vx_bool   format_flag           = vx_false_e;
    vx_bool   shExe_flag            = vx_true_e;
    vx_bool   swExe_flag            = vx_false_e;
    vx_bool   enable_2d_tensor      = vx_false_e;
    vx_enum   policyEnum            = policy->value->e;
    vx_uint32 depth                 = (TENSOR_DIM_NUM(output) > 2) ? TENSOR_VIEW_SIZE_INDEX(output, 2) : 1;
    vx_uint32 batchCount0           = (TENSOR_SIZE_INDEX(input0, 3) == 0) ? 1 : TENSOR_SIZE_INDEX(input0, 3);
    vx_uint32 batchCount1           = (TENSOR_SIZE_INDEX(input1, 3) == 0) ? 1 : TENSOR_SIZE_INDEX(input1, 3);
    vx_uint32 batchCount            = (TENSOR_SIZE_INDEX(output, 3) == 0) ? 1 : TENSOR_SIZE_INDEX(output, 3);

    vxnne_tensor_add_layer tensor_add_layer = VX_NULL;
    swExe_flag = (TENSOR_DIM_NUM(output) > 4);
    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_tensor_add_layer_s), (gctPOINTER*)&tensor_add_layer);
    if (!tensor_add_layer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(tensor_add_layer, sizeof(vxnne_tensor_add_layer_s));

    vxnneLayer_Initialize(&tensor_add_layer->base,
                          "TensorAdd",
                          node,
                          vxmOPERATION_COUNT(tensor_add_layer),
                          tensor_add_layer->operations,
                          VX_NULL);

    if(context->evisNoInst.supportEVIS)
    {
        format_flag = (vx_bool)((input0Format != VX_TYPE_FLOAT32) && (input1Format != VX_TYPE_FLOAT32) && (outputFormat != VX_TYPE_FLOAT32));
        enable_2d_tensor = (vx_bool)(depth == 1 && ((input0Format == VX_TYPE_FLOAT16 && (input1Format == VX_TYPE_FLOAT16 || input1Format == VX_TYPE_FLOAT32) && outputFormat == VX_TYPE_FLOAT16) || format_flag) && policyEnum == VX_CONVERT_POLICY_SATURATE);
    }
    else
    {
        format_flag = vx_true_e;
    }

    shExe_flag = format_flag  || enable_2d_tensor;

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_ADD) &&
        vxnneIsTPSupportFormat(context, input0, VX_NULL, output) &&
        (input0Format == input1Format) &&
        TENSOR_POS(input0) == TENSOR_POS(input1) &&
        TENSOR_QUANT_TYPE(input0) == TENSOR_QUANT_TYPE(input1) &&
        TENSOR_TF_SCALE(input0) == TENSOR_TF_SCALE(input1) &&
        TENSOR_TF_ZEROPOINT(input0) == TENSOR_TF_ZEROPOINT(input1) &&
        !swExe_flag)
    {
        vx_op_param_s conv = {0};

        status = vxnneOperation_Initialize(&tensor_add_layer->tensorAddTP.base,
                                           &tensor_add_layer->base,
                                           VXNNE_OPERATION_TARGET_TP,
                                           VXNNE_OPERATOR_TENSOR_ADD,
                                           VX_NULL,
                                           VX_NULL,
                                           batchCount,
                                           0);
        if (status != VX_SUCCESS) goto exit;

        vxnneLayer_SetOperation(
            &tensor_add_layer->base,
            &tensor_add_layer->tensorAddTP.base,
            0);

        tensor_add_layer->tensorAddTP.input    = input0;
        tensor_add_layer->tensorAddTP.input_ex = input1;
        tensor_add_layer->tensorAddTP.output   = output;

        memset(&conv, 0, sizeof(vx_op_param_s));
        conv.pad_x_left = 0;
        conv.pad_y_top = 0;
        conv.pool_size_x = 0;
        conv.pool_size_y = 0;
        conv.pool_stride = 1;
        conv.enable_relu = vx_false_e;
        conv.conv_rounding_type = 0;
        conv.pad_mode = VX_PAD_CONSTANT;
        conv.pad_const = 0;
        conv.tpType = TP_ADD;
        conv.other_ref = (vx_reference)input1;
        conv.data_buff = gcvNULL;

        memcpy(&tensor_add_layer->tensorAddTP.base.parameter, &conv, sizeof(vx_op_param_s));

        vxnneOperation_AddReference(&tensor_add_layer->tensorAddTP.base, (vx_reference)input0, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&tensor_add_layer->tensorAddTP.base, (vx_reference)input1, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&tensor_add_layer->tensorAddTP.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }
    else if (shExe_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)) &&  !swExe_flag)
    {
        vxnne_shader_executable shaderExecutable = VX_NULL;

        if(node->base.context->evisNoInst.supportEVIS)
        {
            if (enable_2d_tensor)
            {
                shaderExecutable = vxnneGetTensor2DAddShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_2D_ADD, &node->kernelAttributes.borderMode, input0, input1, VX_NN_ACTIVATION_NONE, VX_TENSOR_OP_ADD, output);
            }
            else
            {
                if (input0Format != input1Format && input0Format == VX_TYPE_FLOAT16)
                    shaderExecutable = vxnneGetTensorAddShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_ELTWISE, &node->kernelAttributes.borderMode, input1, input0, NULL, policy, VX_NN_ACTIVATION_NONE, VX_TENSOR_OP_ADD, output);
                else
                    shaderExecutable = vxnneGetTensorAddShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_ELTWISE, &node->kernelAttributes.borderMode, input0, input1, NULL, policy, VX_NN_ACTIVATION_NONE, VX_TENSOR_OP_ADD, output);
            }

        }
        else
        {
                shaderExecutable = vxnneGetGPUTensorEltwiseShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_ADD, &node->kernelAttributes.borderMode, input0, input1, VX_NN_ACTIVATION_NONE, VX_TENSOR_OP_ADD, output);
        }

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&tensor_add_layer->tensorAddSH,
                                        &tensor_add_layer->base,
                                        VXNNE_OPERATOR_TENSOR_ADD,
                                        batchCount,
                                        shaderExecutable);
        if (status != VX_SUCCESS)
            goto exit;

        if (batchCount != 1 && batchCount0 != batchCount1)
        {
            vx_tensor src0 = (vx_tensor)shaderExecutable->param[0];
            vx_uint32 batch0 = (TENSOR_SIZE_INDEX(src0, 3) == 0) ? 1 : TENSOR_SIZE_INDEX(src0, 3);

            if (batch0 == 1)
                vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NO_BATCH_BIT);
            else
                vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NO_BATCH_BIT);
        }

        vxnneOperation_AddReference(&tensor_add_layer->tensorAddSH.base, (vx_reference)input0, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&tensor_add_layer->tensorAddSH.base, (vx_reference)input1, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&tensor_add_layer->tensorAddSH.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        vxnneLayer_SetOperation(
            &tensor_add_layer->base,
            &tensor_add_layer->tensorAddSH.base,
            0);
    }
    else
    {
        vxnneOperation_Initialize(&tensor_add_layer->tensorAddSW.base,
                                &tensor_add_layer->base,
                                VXNNE_OPERATION_TARGET_SW,
                                VXNNE_OPERATOR_TENSOR_ADD,
                                /*vxnneExecuteSWEltwise,*/
                                vxnneExecuteSWTensorAdd,
                                VX_NULL,
                                batchCount,
                                0);

        vxnneLayer_SetOperation(
            &tensor_add_layer->base,
            &tensor_add_layer->tensorAddSW.base,
            0);

        tensor_add_layer->tensorAddSW.input0    = input0;
        tensor_add_layer->tensorAddSW.input1    = input1;
        tensor_add_layer->tensorAddSW.policy    = policy;
        tensor_add_layer->tensorAddSW.output    = output;

        vxnneOperation_AddReference(&tensor_add_layer->tensorAddSW.base, (vx_reference)input0, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&tensor_add_layer->tensorAddSW.base, (vx_reference)input1, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&tensor_add_layer->tensorAddSW.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }

/*    tensor_add_layer->base.num_temp_tensors = 0;
    if(input0 && input0_reshape_flag == vx_true_e)
        tensor_add_layer->base.temp_tensors[tensor_add_layer->base.num_temp_tensors++] = input0;
    if(input1 && input1_reshape_flag == vx_true_e)
        tensor_add_layer->base.temp_tensors[tensor_add_layer->base.num_temp_tensors++] = input1;
    if(output && output_reshape_flag == vx_true_e)
        tensor_add_layer->base.temp_tensors[tensor_add_layer->base.num_temp_tensors++] = output;*/

    node->layer = &tensor_add_layer->base;

    return status;

exit:
    if (tensor_add_layer)
        gcoOS_Free(NULL, (gctPOINTER)tensor_add_layer);

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorAdd_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

/*Tensor sub*/

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorSub(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorSub_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorSub_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

vx_status vxnneExecuteSWTensorSub(vxnne_operation operation)
{
    vxnne_tensor_sub_operation eltwiseOperation   = (vxnne_tensor_sub_operation)operation;

    vx_tensor input1 = eltwiseOperation->input0;
    vx_tensor input2 = eltwiseOperation->input1;
    vx_tensor output = eltwiseOperation->output;
    vx_uint32 dims   = TENSOR_DIM_NUM(output);

    {
        vx_uint8_ptr input1base, input2base, outputbase;
        vx_uint32 i, numCount=1;

        vxoTensor_GetTensorViewMemory(input1, (gctPOINTER*)&input1base, VX_NULL);
        vxoTensor_GetTensorViewMemory(input2, (gctPOINTER*)&input2base, VX_NULL);
        vxoTensor_GetTensorViewMemory(output, (gctPOINTER*)&outputbase, VX_NULL);

        for(i = 0; i < dims; i++)
        {
            numCount *= output->dims[i];
        }

        for(i = 0; i < numCount; i++)
        {
            vx_uint32 in1offset, in2offset, outoffset;
            vx_int8_ptr in1, in2, out;

            in1offset = getExpandTensorOffset(i, input1, output->dims);
            in2offset = getExpandTensorOffset(i, input2, output->dims);
            outoffset = getTensorOffset(i, output, output->dims);

            in1 = (vx_int8_ptr)input1base + in1offset;
            in2 = (vx_int8_ptr)input2base + in2offset;
            out = (vx_int8_ptr)outputbase + outoffset;

            {
                vx_status status = VX_SUCCESS;
                vx_float32 in1Data_fl32 = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(input1), TENSOR_QUANT_TYPE(input1), 0, (vx_uint8_ptr)in1,
                    TENSOR_POS(input1), TENSOR_TF_ZEROPOINT(input1), TENSOR_TF_SCALE(input1));
                vx_float32 in2Data_fl32 = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(input2), TENSOR_QUANT_TYPE(input2), 0, (vx_uint8_ptr)in2,
                    TENSOR_POS(input2), TENSOR_TF_ZEROPOINT(input2), TENSOR_TF_SCALE(input2));
                status = vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(output), TENSOR_QUANT_TYPE(output), 0, in1Data_fl32 - in2Data_fl32, (vx_uint8_ptr)out,
                    TENSOR_POS(output), TENSOR_TF_ZEROPOINT(output), TENSOR_TF_SCALE(output), TENSOR_ROUNDING_MODE(output));
            }
        }
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorSub_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    vx_tensor input0   = (vx_tensor)parameters[0];
    vx_tensor input1   = (vx_tensor)parameters[1];
    vx_scalar policy   = (vx_scalar)parameters[2];
    vx_tensor output   = (vx_tensor)parameters[3];

    vx_type_e input0Format = TENSOR_DATA_TYPE(input0);
    vx_type_e input1Format = TENSOR_DATA_TYPE(input1);
    vx_type_e outputFormat = TENSOR_DATA_TYPE(output);

    vx_bool   format_flag           = vx_false_e;
    vx_bool   shExe_flag            = vx_true_e;
    vx_bool   swExe_flag            = vx_false_e;
    vx_bool   enable_2d_tensor      = vx_false_e;
    vx_enum   policyEnum            = policy->value->e;
    vx_uint32 depth                 = (TENSOR_DIM_NUM(output) > 2) ? TENSOR_VIEW_SIZE_INDEX(output, 2) : 1;
    vx_uint32 batchCount0           = (TENSOR_SIZE_INDEX(input0, 3) == 0) ? 1 : TENSOR_SIZE_INDEX(input0, 3);
    vx_uint32 batchCount1 = (TENSOR_SIZE_INDEX(input1, 3) == 0) ? 1 : TENSOR_SIZE_INDEX(input1, 3);
    vx_uint32 batchCount = (TENSOR_SIZE_INDEX(output, 3) == 0) ? 1 : TENSOR_SIZE_INDEX(output, 3);

    vxnne_tensor_sub_layer tensor_sub_layer = VX_NULL;

    swExe_flag = (TENSOR_DIM_NUM(output) > 4);

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_tensor_sub_layer_s), (gctPOINTER*)&tensor_sub_layer);
    if (!tensor_sub_layer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(tensor_sub_layer, sizeof(vxnne_tensor_sub_layer_s));

    vxnneLayer_Initialize(&tensor_sub_layer->base,
                          "TensorSub",
                          node,
                          vxmOPERATION_COUNT(tensor_sub_layer),
                          tensor_sub_layer->operations,
                          VX_NULL);

    if(node->base.context->evisNoInst.supportEVIS)
    {
        format_flag = (vx_bool)((input0Format != VX_TYPE_FLOAT32) && (input1Format != VX_TYPE_FLOAT32) && (outputFormat != VX_TYPE_FLOAT32));
        enable_2d_tensor = (vx_bool)(depth == 1 && format_flag && policyEnum == VX_CONVERT_POLICY_SATURATE);
        shExe_flag = format_flag  || enable_2d_tensor;
    }
    else
        shExe_flag = vx_true_e;

    if (shExe_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)) && !swExe_flag)
    {
        vxnne_shader_executable shaderExecutable = VX_NULL;
        if(node->base.context->evisNoInst.supportEVIS)
        {
            if (enable_2d_tensor)
                shaderExecutable = vxnneGetTensor2DAddShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_2D_ADD, &node->kernelAttributes.borderMode, input0, input1, VX_NN_ACTIVATION_NONE, VX_TENSOR_OP_SUB, output);
            else
                shaderExecutable = vxnneGetTensorAddShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_ELTWISE, &node->kernelAttributes.borderMode, input0, input1, NULL, policy, VX_NN_ACTIVATION_NONE, VX_TENSOR_OP_SUB, output);
        }
        else
        {
            shaderExecutable = vxnneGetGPUTensorEltwiseShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_ELTWISE, &node->kernelAttributes.borderMode, input0, input1, VX_NN_ACTIVATION_NONE, VX_TENSOR_OP_SUB, output);
        }

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&tensor_sub_layer->tensorSubSH,
            &tensor_sub_layer->base,
            VXNNE_OPERATOR_TENSOR_SUB,
            batchCount,
            shaderExecutable);

        if (status != VX_SUCCESS)
            goto exit;

        if (batchCount != 1 && batchCount0 != batchCount1)
        {
            vx_tensor src0 = (vx_tensor)shaderExecutable->param[0];
            vx_uint32 batch0 = (TENSOR_SIZE_INDEX(src0, 3) == 0) ? 1 : TENSOR_SIZE_INDEX(src0, 3);

            if (batch0 == 1)
                vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NO_BATCH_BIT);
            else
                vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NO_BATCH_BIT);
        }

        vxnneOperation_AddReference(&tensor_sub_layer->tensorSubSH.base, (vx_reference)input0, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&tensor_sub_layer->tensorSubSH.base, (vx_reference)input1, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&tensor_sub_layer->tensorSubSH.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        vxnneLayer_SetOperation(
            &tensor_sub_layer->base,
            &tensor_sub_layer->tensorSubSH.base,
            0);
    }
    else
    {
        vxnneOperation_Initialize(&tensor_sub_layer->tensorSubSW.base,
                                &tensor_sub_layer->base,
                                VXNNE_OPERATION_TARGET_SW,
                                VXNNE_OPERATOR_TENSOR_SUB,
                                /*vxnneExecuteSWEltwise,*/
                                vxnneExecuteSWTensorSub,
                                VX_NULL,
                                batchCount,
                                0);
        vxnneLayer_SetOperation(
            &tensor_sub_layer->base,
            &tensor_sub_layer->tensorSubSW.base,
            0);

        tensor_sub_layer->tensorSubSW.input0    = input0;
        tensor_sub_layer->tensorSubSW.input1    = input1;
        tensor_sub_layer->tensorSubSW.policy    = policy;
        tensor_sub_layer->tensorSubSW.output    = output;

        vxnneOperation_AddReference(&tensor_sub_layer->tensorSubSW.base, (vx_reference)input0, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&tensor_sub_layer->tensorSubSW.base, (vx_reference)input1, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&tensor_sub_layer->tensorSubSW.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }

    node->layer = &tensor_sub_layer->base;
    return status;

exit:
    if (tensor_sub_layer)
        gcoOS_Free(NULL, (gctPOINTER)tensor_sub_layer);

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorSub_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}
//end tensor sub

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorMul(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorMul_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorMul_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

//Tensor mul
vx_status vxnneExecuteSWTensorMul(vxnne_operation operation)
{
    vxnne_tensor_mul_operation eltwiseOperation   = (vxnne_tensor_mul_operation)operation;

    vx_tensor input1 = eltwiseOperation->input0;
    vx_tensor input2 = eltwiseOperation->input1;
    vx_tensor output = eltwiseOperation->output;
    vx_uint32 dims   = TENSOR_DIM_NUM(output);

    {
        vx_uint8_ptr input1base, input2base, outputbase;
        vx_uint32 i, numCount = 1;

        vxoTensor_GetTensorViewMemory(input1, (gctPOINTER*)&input1base, VX_NULL);
        vxoTensor_GetTensorViewMemory(input2, (gctPOINTER*)&input2base, VX_NULL);
        vxoTensor_GetTensorViewMemory(output, (gctPOINTER*)&outputbase, VX_NULL);

        for (i = 0; i < dims; i++)
        {
            numCount *= output->dims[i];
        }

        for (i = 0; i < numCount; i++)
        {
            vx_uint32 in1offset, in2offset, outoffset;
            vx_int8_ptr in1, in2, out;
            vx_status status = VX_SUCCESS;
            vx_float32 in1Data_fl32, in2Data_fl32;

            in1offset = getExpandTensorOffset(i, input1, output->dims);
            in2offset = getExpandTensorOffset(i, input2, output->dims);
            outoffset = getTensorOffset(i, output, output->dims);

            in1 = (vx_int8_ptr)input1base + in1offset;
            in2 = (vx_int8_ptr)input2base + in2offset;
            out = (vx_int8_ptr)outputbase + outoffset;

            in1Data_fl32 = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(input1), TENSOR_QUANT_TYPE(input1), 0, (vx_uint8_ptr)in1,
                TENSOR_POS(input1), TENSOR_TF_ZEROPOINT(input1), TENSOR_TF_SCALE(input1));
            in2Data_fl32 = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(input2), TENSOR_QUANT_TYPE(input2), 0, (vx_uint8_ptr)in2,
                TENSOR_POS(input2), TENSOR_TF_ZEROPOINT(input2), TENSOR_TF_SCALE(input2));
            status = vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(output), TENSOR_QUANT_TYPE(output), 0, in1Data_fl32 * in2Data_fl32, (vx_uint8_ptr)out,
                TENSOR_POS(output), TENSOR_TF_ZEROPOINT(output), TENSOR_TF_SCALE(output), TENSOR_ROUNDING_MODE(output));
        }
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorMul_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    vx_tensor input0   = (vx_tensor)parameters[0];
    vx_tensor input1   = (vx_tensor)parameters[1];
    vx_scalar scale    = (vx_scalar)parameters[2];
    vx_scalar policy   = (vx_scalar)parameters[3];
    vx_scalar rounding = (vx_scalar)parameters[4];
    vx_tensor output   = (vx_tensor)parameters[5];

    vx_type_e input0Format = TENSOR_DATA_TYPE(input0);
    vx_type_e input1Format = TENSOR_DATA_TYPE(input1);
    vx_type_e outputFormat = TENSOR_DATA_TYPE(output);

    vx_bool shExe_flag   = vx_true_e;
    vx_bool swExe_flag   = vx_false_e;
    vx_uint32 batchCount0 = (TENSOR_SIZE_INDEX(input0, 3) == 0) ? 1 : TENSOR_SIZE_INDEX(input0, 3);
    vx_uint32 batchCount1 = (TENSOR_SIZE_INDEX(input1, 3) == 0) ? 1 : TENSOR_SIZE_INDEX(input1, 3);
    vx_uint32 batchCount = (TENSOR_SIZE_INDEX(output, 3) == 0) ? 1 : TENSOR_SIZE_INDEX(output, 3);

    vxnne_tensor_mul_layer tensor_mul_layer = VX_NULL;

    swExe_flag = (TENSOR_DIM_NUM(output) > 4);

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_tensor_mul_layer_s), (gctPOINTER*)&tensor_mul_layer);
    if (!tensor_mul_layer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(tensor_mul_layer, sizeof(vxnne_tensor_mul_layer_s));

    vxnneLayer_Initialize(&tensor_mul_layer->base,
                          "TensorMul",
                          node,
                          vxmOPERATION_COUNT(tensor_mul_layer),
                          tensor_mul_layer->operations,
                          VX_NULL);

    if(node->base.context->evisNoInst.supportEVIS)
        shExe_flag = (vx_bool)((input0Format != VX_TYPE_FLOAT32) && (input1Format != VX_TYPE_FLOAT32) && (outputFormat != VX_TYPE_FLOAT32));
    else
        shExe_flag = vx_true_e;

    if (shExe_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)) && !swExe_flag)
    {
        vxnne_shader_executable shaderExecutable = VX_NULL;

        if(node->base.context->evisNoInst.supportEVIS)
        {
            vx_enum    policyEnum   = policy->value->e;
            vx_enum    roundingEnum = rounding->value->e;
            vx_float32 scale_val    = scale->value->f32;

            if ((vxDataType_GetSize(input1Format) == 1 && vxDataType_GetSize(input0Format) == 2) || (input1Format == VX_TYPE_INT16 && input0Format == VX_TYPE_FLOAT16))
            {
                if (roundingEnum == VX_ROUND_POLICY_TO_NEAREST_EVEN && policyEnum == VX_CONVERT_POLICY_SATURATE && scale_val == 1.0f)
                    shaderExecutable = vxnneGetTensorMulSatRTEShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_MUL_SAT_RTE, &node->kernelAttributes.borderMode, input1, input0, VX_NN_ACTIVATION_NONE, VX_TENSOR_OP_MUL, output);
                else
                    shaderExecutable = vxnneGetTensorMulShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_MUL, &node->kernelAttributes.borderMode, input1, input0, scale, policy, rounding, VX_NN_ACTIVATION_NONE, VX_TENSOR_OP_MUL, output);
            }
            else
            {
                if (roundingEnum == VX_ROUND_POLICY_TO_NEAREST_EVEN && policyEnum == VX_CONVERT_POLICY_SATURATE && scale_val == 1.0f)
                    shaderExecutable = vxnneGetTensorMulSatRTEShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_MUL_SAT_RTE, &node->kernelAttributes.borderMode, input0, input1, VX_NN_ACTIVATION_NONE, VX_TENSOR_OP_MUL, output);
                else
                    shaderExecutable = vxnneGetTensorMulShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_MUL, &node->kernelAttributes.borderMode, input0, input1, scale, policy, rounding, VX_NN_ACTIVATION_NONE, VX_TENSOR_OP_MUL, output);
            }
        }
        else
        {
            shaderExecutable = vxnneGetGPUTensorEltwiseShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_MUL, &node->kernelAttributes.borderMode, input0, input1, VX_NN_ACTIVATION_NONE, VX_TENSOR_OP_MUL, output);
        }

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&tensor_mul_layer->tensorMulSH,
                                        &tensor_mul_layer->base,
                                        VXNNE_OPERATOR_TENSOR_MUL,
                                        batchCount,
                                        shaderExecutable);

        if (status != VX_SUCCESS)
            goto exit;

        if (batchCount != 1 && batchCount0 != batchCount1)
        {
            vx_tensor src0 = (vx_tensor)shaderExecutable->param[0];
            vx_uint32 batch0 = (TENSOR_SIZE_INDEX(src0, 3) == 0) ? 1 : TENSOR_SIZE_INDEX(src0, 3);

            if (batch0 == 1)
                vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NO_BATCH_BIT);
            else
                vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NO_BATCH_BIT);
        }

        vxnneOperation_AddReference(&tensor_mul_layer->tensorMulSH.base, (vx_reference)input0, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&tensor_mul_layer->tensorMulSH.base, (vx_reference)input1, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&tensor_mul_layer->tensorMulSH.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        vxnneLayer_SetOperation(
            &tensor_mul_layer->base,
            &tensor_mul_layer->tensorMulSH.base,
            0);
    }
    else
    {
        vxnneOperation_Initialize(&tensor_mul_layer->tensorMulSW.base,
                                &tensor_mul_layer->base,
                                VXNNE_OPERATION_TARGET_SW,
                                VXNNE_OPERATOR_TENSOR_MUL,
                                /*vxnneExecuteSWEltwise,*/
                                vxnneExecuteSWTensorMul,
                                VX_NULL,
                                batchCount,
                                0);

        vxnneLayer_SetOperation(
            &tensor_mul_layer->base,
            &tensor_mul_layer->tensorMulSW.base,
            0);

        tensor_mul_layer->tensorMulSW.input0    = input0;
        tensor_mul_layer->tensorMulSW.input1    = input1;
        tensor_mul_layer->tensorMulSW.scale     = scale;
        tensor_mul_layer->tensorMulSW.overflow  = policy;
        tensor_mul_layer->tensorMulSW.rounding  = rounding;
        tensor_mul_layer->tensorMulSW.output    = output;

        vxnneOperation_AddReference(&tensor_mul_layer->tensorMulSW.base, (vx_reference)input0, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&tensor_mul_layer->tensorMulSW.base, (vx_reference)input1, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&tensor_mul_layer->tensorMulSW.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }

    node->layer = &tensor_mul_layer->base;
    return status;

exit:
    if (tensor_mul_layer)
        gcoOS_Free(NULL, (gctPOINTER)tensor_mul_layer);

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorMul_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}
//end tensor mul

