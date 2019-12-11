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
#include <layers/gc_vx_layer_div.h>


//Tensor div
vx_status vxnneExecuteSWTensorDiv(vxnne_operation operation)
{
    vxnne_tensor_div_operation eltwiseOperation   = (vxnne_tensor_div_operation)operation;

    vx_tensor input1 = eltwiseOperation->input0;
    vx_tensor input2 = eltwiseOperation->input1;
    vx_tensor output = eltwiseOperation->output;

    vx_int32 dim1 = input1->viewRegion.dimCount;
    vx_int32 dim2 = input2->viewRegion.dimCount;
    vx_enum overflow = eltwiseOperation->overflow->value->e;

    if (dim1 == dim2)
    {
        vx_enum rounding = eltwiseOperation->rounding->value->e;
        vx_float32 scale = eltwiseOperation->scale->value->f32;
        eltwise(input1, input2, scale, overflow, rounding, VX_TENSOR_OP_DIV, output);
    }
    else
        vxError("Difference dim\n");

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorDiv(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorDiv_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorDiv_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorDiv_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    vx_tensor input0   = (vx_tensor)parameters[0];
    vx_tensor input1   = (vx_tensor)parameters[1];
    vx_scalar scale    = (vx_scalar)parameters[2];
    vx_scalar overflow = (vx_scalar)parameters[3];
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

    vxnne_tensor_div_layer tensor_div_layer = VX_NULL;

    swExe_flag = (TENSOR_DIM_NUM(output) > 4);

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_tensor_div_layer_s), (gctPOINTER*)&tensor_div_layer);
    if (!tensor_div_layer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(tensor_div_layer, sizeof(vxnne_tensor_div_layer_s));

    vxnneLayer_Initialize(&tensor_div_layer->base,
                          "TensorDiv",
                          node,
                          vxmOPERATION_COUNT(tensor_div_layer),
                          tensor_div_layer->operations,
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
            shaderExecutable = vxnneGetTensorDivShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_DIV, &node->kernelAttributes.borderMode, input0, input1, scale, overflow, VX_NN_ACTIVATION_NONE, VX_TENSOR_OP_DIV, output);
        }
        else
        {
            shaderExecutable = vxnneGetGPUTensorEltwiseShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_DIV, &node->kernelAttributes.borderMode, input0, input1, VX_NN_ACTIVATION_NONE, VX_TENSOR_OP_DIV, output);
        }
        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&tensor_div_layer->tensorDivSH,
                                        &tensor_div_layer->base,
                                        VXNNE_OPERATOR_TENSOR_DIV,
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

        vxnneOperation_AddReference(&tensor_div_layer->tensorDivSH.base, (vx_reference)input0, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&tensor_div_layer->tensorDivSH.base, (vx_reference)input1, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&tensor_div_layer->tensorDivSH.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        vxnneLayer_SetOperation(
            &tensor_div_layer->base,
            &tensor_div_layer->tensorDivSH.base,
            0);
    }
    else
    {
        vxnneOperation_Initialize(&tensor_div_layer->tensorDivSW.base,
                                &tensor_div_layer->base,
                                VXNNE_OPERATION_TARGET_SW,
                                VXNNE_OPERATOR_TENSOR_DIV,
                                vxnneExecuteSWTensorDiv,
                                VX_NULL,
                                batchCount,
                                0);

        vxnneLayer_SetOperation(
            &tensor_div_layer->base,
            &tensor_div_layer->tensorDivSW.base,
            0);

        tensor_div_layer->tensorDivSW.input0    = input0;
        tensor_div_layer->tensorDivSW.input1    = input1;
        tensor_div_layer->tensorDivSW.scale = scale;
        tensor_div_layer->tensorDivSW.overflow  = overflow;
        tensor_div_layer->tensorDivSW.rounding = rounding;
        tensor_div_layer->tensorDivSW.output    = output;

        vxnneOperation_AddReference(&tensor_div_layer->tensorDivSW.base, (vx_reference)input0, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&tensor_div_layer->tensorDivSW.base, (vx_reference)input1, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&tensor_div_layer->tensorDivSW.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }

    node->layer = &tensor_div_layer->base;
    return status;

exit:
    if (tensor_div_layer)
        gcoOS_Free(NULL, (gctPOINTER)tensor_div_layer);

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorDiv_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}
//end tensor div

