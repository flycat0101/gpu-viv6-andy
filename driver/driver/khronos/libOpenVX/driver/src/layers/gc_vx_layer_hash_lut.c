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
#include <layers/gc_vx_layer_hash_lut.h>


/***************************************************************************************************************************
 *                                                 Hash Lookup Table
 ***************************************************************************************************************************/
vx_status vxnneExecuteSWHashLUT(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_hashlut_operation hashlutOperation = (vxnne_hashlut_operation)operation;

    vx_tensor input  = hashlutOperation->inputs;
    vx_tensor keys   = hashlutOperation->keys;
    vx_tensor values = hashlutOperation->values;
    vx_tensor hits   = hashlutOperation->hits;
    vx_tensor output = hashlutOperation->outputs;

    vx_int32 input_count = TENSOR_SIZE_INDEX(input, 0);

    vx_int32 key_count = TENSOR_SIZE_INDEX(keys, 0);

    vx_int32 value_count = TENSOR_SIZE_INDEX(values, 1);
    vx_int32 value_stride = TENSOR_SIZE_INDEX(values, 0);

    vx_int32 i = 0, j = 0, key = 0, index = -1, stride = value_stride * TENSOR_DATA_SIZE(values);
    vx_int32_ptr inPtr = (vx_int32_ptr)TENSOR_LOGICAL_ADDR(input);
    vx_int32_ptr keyPtr = (vx_int32_ptr)TENSOR_LOGICAL_ADDR(keys);
    vx_uint8_ptr hitPtr = (vx_uint8_ptr)TENSOR_LOGICAL_ADDR(hits);
    for (i = 0; i < input_count; i ++)
    {
        //key = (vx_int32)VX_GET_DATA_FROM_TENSOR(input, i);
        key = inPtr[i];

        index = -1;

        /* find the index of key from keys*/
        for (j = 0; j < key_count; j ++)
        {
            //if (key == VX_GET_DATA_FROM_TENSOR(keys, j))
            if (key == keyPtr[j])
            {
                index = j;
                break;
            }
        }

        if ((index < key_count) && (index >= 0 && index < value_count))
        {
            hitPtr[i] = 1;
            memcpy(TENSOR_LOGICAL_ADDR(output) + i * stride, TENSOR_LOGICAL_ADDR(values) + index * stride, stride);
        }
        else
        {
            hitPtr[i] = 0;
            memset(TENSOR_LOGICAL_ADDR(output) + i * stride, 0, stride);
        }
    }

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNHashLUT(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    status = VX_SUCCESS;

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoHashLUT_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxoHashLUT_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoHashLUT_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_tensor  keys                       = (vx_tensor)parameters[1];
    vx_tensor  values                     = (vx_tensor)parameters[2];
    vx_tensor  hits                       = (vx_tensor)parameters[3];
    vx_tensor  outputs                    = (vx_tensor)parameters[4];

    vx_uint32  batchCount                 = TENSOR_SIZE_INDEX(inputs, 3);

    vxnne_hashlut_layer  hashlutLayer     = VX_NULL;
    vx_enum    outputFormat               = TENSOR_DATA_TYPE(outputs);
    vx_enum    valueFormat                = TENSOR_DATA_TYPE(values);
    vx_bool    dataFormat_flag            = vx_false_e;
    vx_float32    input_scale             = TENSOR_TF_SCALE(values);
    vx_float32    output_scale            = TENSOR_TF_SCALE(outputs);
    vx_int32      inputZP                 = TENSOR_TF_ZEROPOINT(values);
    vx_int32      outputZP                = TENSOR_TF_ZEROPOINT(outputs);

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }
    gcoOS_Allocate(gcvNULL, sizeof(vxnne_hashlut_layer_s), (gctPOINTER*)&hashlutLayer);
    if (!hashlutLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("Out of Memory at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(hashlutLayer, sizeof(vxnne_hashlut_layer_s));

    if(node->base.context->evisNoInst.supportEVIS)
    {
        if (((valueFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
            && (input_scale == output_scale && inputZP == outputZP))
            || (valueFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16))
            dataFormat_flag = vx_true_e;
    }
    else
    {
        if (((valueFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
            && (input_scale == output_scale && inputZP == outputZP))
            || (valueFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
            || (valueFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32))
            dataFormat_flag = vx_true_e;
    }

    vxnneLayer_Initialize(&hashlutLayer->base,
                          "HashLUT",
                          node,
                          vxmOPERATION_COUNT(hashlutLayer),
                          hashlutLayer->operations,
                          VX_NULL);

    if (dataFormat_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
    {
        vxnne_shader_executable shaderExecutable = VX_NULL;

        if(node->base.context->evisNoInst.supportEVIS)
        {
            shaderExecutable = vxnneGetHashLUTShaderExecutable(node->base.context, VXNNE_KERNEL_HASHLUT,
                &node->kernelAttributes.borderMode, inputs, keys, values, hits, outputs);
        }
        else
        {
            shaderExecutable = vxnneGetGPUHashLUTShaderExecutable(node->base.context, VXNNE_KERNEL_HASHLUT,
                &node->kernelAttributes.borderMode, inputs, keys, values, hits, outputs);
        }

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto OnError;
        }

        status = vxnneShaderOperation_Initialize(&hashlutLayer->hashlut_sh_operation,
            &hashlutLayer->base,
            VXNNE_OPERATOR_HASHLUT,
            batchCount,
            shaderExecutable);
        if (status != VX_SUCCESS)
            goto OnError;

        vxnneOperation_AddReference(&hashlutLayer->hashlut_sh_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&hashlutLayer->hashlut_sh_operation.base, (vx_reference)keys, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&hashlutLayer->hashlut_sh_operation.base, (vx_reference)values, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&hashlutLayer->hashlut_sh_operation.base, (vx_reference)hits, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        vxnneOperation_AddReference(&hashlutLayer->hashlut_sh_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        vxnneLayer_SetOperation(
            &hashlutLayer->base,
            &hashlutLayer->hashlut_sh_operation.base,
            0);
    }
    else
    {
        vxnneOperation_Initialize(&hashlutLayer->hashlut_sw_operation.base,
                                  &hashlutLayer->base,
                                  VXNNE_OPERATION_TARGET_SW,
                                  VXNNE_OPERATOR_HASHLUT,
                                  vxnneExecuteSWHashLUT,
                                  VX_NULL,
                                  batchCount,
                                  0);

        vxnneLayer_SetOperation(
            &hashlutLayer->base,
            &hashlutLayer->hashlut_sw_operation.base,
            0);

        hashlutLayer->hashlut_sw_operation.inputs           = inputs;
        hashlutLayer->hashlut_sw_operation.keys             = keys;
        hashlutLayer->hashlut_sw_operation.values           = values;
        hashlutLayer->hashlut_sw_operation.hits             = hits;
        hashlutLayer->hashlut_sw_operation.outputs          = outputs;

        vxnneOperation_AddReference(&hashlutLayer->hashlut_sw_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&hashlutLayer->hashlut_sw_operation.base, (vx_reference)keys, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&hashlutLayer->hashlut_sw_operation.base, (vx_reference)values, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&hashlutLayer->hashlut_sw_operation.base, (vx_reference)hits, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        vxnneOperation_AddReference(&hashlutLayer->hashlut_sw_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }

    node->layer = &hashlutLayer->base;

    return status;

OnError:
    if (hashlutLayer) {
        gcoOS_Free(VX_NULL, (gctPOINTER)hashlutLayer);
        hashlutLayer = VX_NULL;
    }
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoHashLUT_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

