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
#include <layers/gc_vx_layer_lut.h>

/***************************************************************************************************************************
 *                                                 LUT2
 ***************************************************************************************************************************/
vx_status vxnneExecuteSWLUT2(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_lut2_operation lutOperation = (vxnne_lut2_operation)operation;

    vx_tensor input  = lutOperation->inputs;
    vx_tensor lut  = lutOperation->lut;
    vx_tensor output = lutOperation->outputs;

    vx_int32 input_index = TENSOR_SIZE_INDEX(input, 0);

    vx_int32 lut_index = TENSOR_SIZE_INDEX(lut, 2);
    vx_int32 lut_width = TENSOR_SIZE_INDEX(lut, 0);
    vx_int32 lut_height = TENSOR_SIZE_INDEX(lut, 1);

    vx_int32 i = 0, index = 0, stride = lut_width * lut_height * TENSOR_DATA_SIZE(lut);

    for (i = 0; i < input_index; i ++)
    {
        index = (vx_int32)VX_GET_DATA_FROM_TENSOR(input, i);

        if (index >= 0 && index < lut_index)
        {
            memcpy(TENSOR_LOGICAL_ADDR(output) + i * stride, TENSOR_LOGICAL_ADDR(lut) + index * stride, stride);
        }
    }

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNLUT2(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoLUT2_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxoLUT2_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoLUT2_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_tensor  lut                        = (vx_tensor)parameters[1];
    vx_tensor  outputs                    = (vx_tensor)parameters[2];

    vx_uint32  batchCount                 = TENSOR_SIZE_INDEX(inputs, 3);

    vxnne_lut2_layer  lut2Layer           = VX_NULL;
    vx_enum    outputFormat               = TENSOR_DATA_TYPE(outputs);
    vx_enum    valueFormat                = TENSOR_DATA_TYPE(lut);
    vx_bool    dataFormat_flag            = vx_false_e;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }
    gcoOS_Allocate(gcvNULL, sizeof(vxnne_lut2_layer_s), (gctPOINTER*)&lut2Layer);
    if (!lut2Layer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("Out of Memory at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    if(node->base.context->evisNoInst.supportEVIS)
    {
        if ((valueFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
            || (valueFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16))
            dataFormat_flag = vx_true_e;
    }
    else
    {
        if ((valueFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
            || (valueFormat == VX_TYPE_INT32 && outputFormat == VX_TYPE_INT32)
            || (valueFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
            || (valueFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32))
            dataFormat_flag = vx_true_e;
    }

    gcoOS_ZeroMemory(lut2Layer, sizeof(vxnne_lut2_layer_s));

    vxnneLayer_Initialize(&lut2Layer->base,
                          "LUT2",
                          node,
                          vxmOPERATION_COUNT(lut2Layer),
                          lut2Layer->operations,
                          VX_NULL);

    if (dataFormat_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
    {
        vxnne_shader_executable shaderExecutable = VX_NULL;
        if(node->base.context->evisNoInst.supportEVIS)
        {
            shaderExecutable = vxnneGetEmbeddingLUTShaderExecutable(node->base.context, VXNNE_KERNEL_EMBEDDINGLUT,
                &node->kernelAttributes.borderMode, inputs, lut, outputs);
        }
        else
        {
            shaderExecutable = vxnneGetGPUEmbeddingLUTShaderExecutable(node->base.context, VXNNE_KERNEL_EMBEDDINGLUT,
                &node->kernelAttributes.borderMode, inputs, lut, outputs);
        }

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&lut2Layer->lut2_sh_operation,
            &lut2Layer->base,
            VXNNE_OPERATOR_LUT2,
            batchCount,
            shaderExecutable);
        if (status != VX_SUCCESS)
            goto exit;

        vxnneOperation_AddReference(&lut2Layer->lut2_sh_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&lut2Layer->lut2_sh_operation.base, (vx_reference)lut, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&lut2Layer->lut2_sh_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        vxnneLayer_SetOperation(
            &lut2Layer->base,
            &lut2Layer->lut2_sh_operation.base,
            0);
    }
    else
    {
        vxnneOperation_Initialize(&lut2Layer->lut2_sw_operation.base,
                                  &lut2Layer->base,
                                  VXNNE_OPERATION_TARGET_SW,
                                  VXNNE_OPERATOR_LUT2,
                                  vxnneExecuteSWLUT2,
                                  VX_NULL,
                                  batchCount,
                                  0);

        vxnneLayer_SetOperation(
            &lut2Layer->base,
            &lut2Layer->lut2_sw_operation.base,
            0);

        lut2Layer->lut2_sw_operation.inputs           = inputs;
        lut2Layer->lut2_sw_operation.lut              = lut;
        lut2Layer->lut2_sw_operation.outputs          = outputs;

        vxnneOperation_AddReference(&lut2Layer->lut2_sw_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&lut2Layer->lut2_sw_operation.base, (vx_reference)lut, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&lut2Layer->lut2_sw_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }
    node->layer = &lut2Layer->base;

    return status;

exit:
    if (lut2Layer) {
        gcoOS_Free(VX_NULL, (gctPOINTER)lut2Layer);
        lut2Layer = VX_NULL;
    }
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoLUT2_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

