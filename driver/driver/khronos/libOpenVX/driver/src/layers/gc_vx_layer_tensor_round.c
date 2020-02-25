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
#include <layers/gc_vx_layer_tensor_round.h>

/***************************************************************************************************************************
 *                                                 TensorRounding
 ***************************************************************************************************************************/
vx_status vxnneExecuteSWTensorRounding(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_tensor_rounding_operation roundingOperation = (vxnne_tensor_rounding_operation)operation;

    vx_tensor input  = roundingOperation->inputs;
    vx_tensor output = roundingOperation->outputs;
    vx_scalar mode   = roundingOperation->mode;

    vx_int32 size = (vx_int32)vxoMemory_ComputeElementCount(&input->tensorBuffer->memory, 0);
    vx_int32 i = 0;

    vx_type_e input_format  = (vx_type_e)TENSOR_DATA_TYPE(input);
    vx_type_e output_format = (vx_type_e)TENSOR_DATA_TYPE(output);

    vx_uint8_ptr input_ptr  = TENSOR_LOGICAL_ADDR(input);
    vx_uint8_ptr output_ptr = TENSOR_LOGICAL_ADDR(output);

    vx_float32 data         = .0f;

    vx_int8 in_fixpoint    = TENSOR_POS(input);
    vx_int8 out_fixpoint   = TENSOR_POS(output);

    vx_enum out_rounding_mode = TENSOR_ROUNDING_MODE(output);

    vx_enum rounding = mode->value->e;

    for (i = 0; i < size; i ++)
    {
        data = vxnneGetDataExt(input_format, TENSOR_QUANT_TYPE(input), i, input_ptr, in_fixpoint, TENSOR_TF_ZEROPOINT(input), TENSOR_TF_SCALE(input));
        status |= vxnneSaveDataExt(output_format, TENSOR_QUANT_TYPE(output), i, vxoNNExternsionConvlutionRound(data, rounding), output_ptr, out_fixpoint, TENSOR_TF_ZEROPOINT(output), TENSOR_TF_SCALE(output), out_rounding_mode);
    }

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorRounding(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoTensorRounding_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxoTensorRounding_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoTensorRounding_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_scalar  mode_s                     = (vx_scalar)parameters[1];
    vx_tensor  outputs                    = (vx_tensor)parameters[2];

    vx_uint32  batchCount                 = TENSOR_SIZE_INDEX(inputs, 3);
    vx_enum    inputFormat                = TENSOR_DATA_TYPE(inputs);
    vx_enum    outputFormat               = TENSOR_DATA_TYPE(outputs);
    vx_bool    dataFormat_flag            = vx_false_e;
    vx_enum    rounding                   = mode_s->value->e;

    vxnne_tensor_rounding_layer  roundingLayer = VX_NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }
    gcoOS_Allocate(gcvNULL, sizeof(vxnne_tensor_rounding_layer_s), (gctPOINTER*)&roundingLayer);
    if (!roundingLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("Out of Memory at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    if(node->base.context->evisNoInst.supportEVIS)
    {
        dataFormat_flag = (vx_bool)((inputFormat == VX_TYPE_FLOAT16  && outputFormat == VX_TYPE_FLOAT16) ||
                                    (inputFormat == VX_TYPE_BFLOAT16 && outputFormat == VX_TYPE_BFLOAT16) ||
                                    (inputFormat == VX_TYPE_FLOAT16  && outputFormat == VX_TYPE_INT8) ||
                                    (inputFormat == VX_TYPE_FLOAT16  && outputFormat == VX_TYPE_INT16) ||
                                    (inputFormat == VX_TYPE_FLOAT16  && outputFormat == VX_TYPE_UINT8) ||
                                    (inputFormat == VX_TYPE_INT8     && outputFormat == VX_TYPE_FLOAT16) ||
                                    (inputFormat == VX_TYPE_INT16    && outputFormat == VX_TYPE_FLOAT16) ||
                                    (inputFormat == VX_TYPE_UINT8    && outputFormat == VX_TYPE_FLOAT16) ||
                                    (inputFormat == VX_TYPE_UINT8    && outputFormat == VX_TYPE_UINT8  ) ||
                                    (inputFormat == VX_TYPE_INT8     && outputFormat == VX_TYPE_INT8   ) ||
                                    (inputFormat == VX_TYPE_INT16    && outputFormat == VX_TYPE_INT16  ) );
    }
    else
    {
        dataFormat_flag = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) ||
                                    (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32));
    }
    gcoOS_ZeroMemory(roundingLayer, sizeof(vxnne_tensor_rounding_layer_s));

    vxnneLayer_Initialize(&roundingLayer->base,
                          "TensorRounding",
                          node,
                          vxmOPERATION_COUNT(roundingLayer),
                          roundingLayer->operations,
                          VX_NULL);

    if (dataFormat_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER))
        && rounding == VX_NN_DS_SIZE_ROUNDING_FLOOR)
    {
        vxnne_shader_executable shaderExecutable = VX_NULL;

        if(node->base.context->evisNoInst.supportEVIS)
        {
            shaderExecutable = vxnneGetFloorShaderExecutable(node->base.context, VXNNE_KERNEL_FLOOR, &node->kernelAttributes.borderMode,
                inputs, mode_s, outputs);
        }
        else
        {
            shaderExecutable = vxnneGetGPUFloorShaderExecutable(node->base.context, VXNNE_KERNEL_FLOOR, &node->kernelAttributes.borderMode,
                inputs, mode_s, outputs);
        }

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&roundingLayer->tensor_rounding_sh_operation,
            &roundingLayer->base,
            VXNNE_OPERATOR_TENSOR_ROUNDING,
            batchCount,
            shaderExecutable);
        if (status != VX_SUCCESS) goto exit;

        vxnneOperation_AddReference(&roundingLayer->tensor_rounding_sh_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&roundingLayer->tensor_rounding_sh_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        vxnneLayer_SetOperation(
            &roundingLayer->base,
            &roundingLayer->tensor_rounding_sh_operation.base,
            0);
    }
    else
    {
        vxnneOperation_Initialize(&roundingLayer->tensor_rounding_sw_operation.base,
            &roundingLayer->base,
            VXNNE_OPERATION_TARGET_SW,
            VXNNE_OPERATOR_TENSOR_ROUNDING,
            vxnneExecuteSWTensorRounding,
            VX_NULL,
            batchCount,
            0);

        vxnneLayer_SetOperation(
            &roundingLayer->base,
            &roundingLayer->tensor_rounding_sw_operation.base,
            0);

        roundingLayer->tensor_rounding_sw_operation.inputs           = inputs;
        roundingLayer->tensor_rounding_sw_operation.mode             = mode_s;
        roundingLayer->tensor_rounding_sw_operation.outputs          = outputs;

        vxnneOperation_AddReference(&roundingLayer->tensor_rounding_sw_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&roundingLayer->tensor_rounding_sw_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }
    node->layer = &roundingLayer->base;

    return status;

exit:
    if(roundingLayer) gcoOS_Free(NULL, (gctPOINTER)roundingLayer);
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoTensorRounding_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}
