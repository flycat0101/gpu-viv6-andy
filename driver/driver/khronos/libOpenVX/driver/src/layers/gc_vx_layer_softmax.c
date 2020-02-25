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
#include <layers/gc_vx_layer_softmax.h>

vx_status vxnneExecuteSWSoftmax(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_softmax_operation softmaxOperation   = (vxnne_softmax_operation)operation;

    vx_tensor  input           = softmaxOperation->inputs;
    vx_tensor  output          = softmaxOperation->outputs;
    vx_scalar  betas           = softmaxOperation->beta;

    vx_type_e  input_format    = (vx_type_e)TENSOR_DATA_TYPE(input);
    vx_type_e  output_format   = (vx_type_e)TENSOR_DATA_TYPE(output);

    vx_int8    input_fp        = TENSOR_POS(input);
    vx_int8    output_fp       = TENSOR_POS(output);
    vx_enum    outputRMode     = TENSOR_ROUNDING_MODE(output);
    vx_float32 beta;

    vx_uint32 width,height,channel,dims;
    vx_uint32 i,c,index,Dim,ItemCount;
    vx_float32_ptr p_pfProSum, pfProSum = NULL;
    vx_float32_ptr p_pfMax, pfMax = NULL;
    vx_float32_ptr p_pfProbFP32, pfProbFP32 = VX_NULL;
    vx_float32_ptr p_pfInput, pfInput = VX_NULL;
    vx_uint8_ptr input_data_ptr = NULL;
    vx_uint8_ptr output_data_ptr = NULL;

    vxoTensor_GetTensorBatchArrayViewMemory(input, operation->currBatchIndex, (gctPOINTER *)&input_data_ptr, VX_NULL);
    vxoTensor_GetTensorBatchArrayViewMemory(output, operation->currBatchIndex, (gctPOINTER *)&output_data_ptr, VX_NULL);

    dims = TENSOR_DIM_NUM(input);
    switch(dims)
    {
        case 1:
            channel = TENSOR_VIEW_SIZE_INDEX(input, 0);
            width   = 1;
            height  = 1;
            break;
        case 2:
            channel = TENSOR_VIEW_SIZE_INDEX(input, 0);
            width   = 1;
            height  = 1;
            break;
        case 3:
            width   = TENSOR_VIEW_SIZE_INDEX(input, 0);
            height  = TENSOR_VIEW_SIZE_INDEX(input, 1);
            channel = TENSOR_VIEW_SIZE_INDEX(input, 2);
            break;
        case 4:
            width   = TENSOR_VIEW_SIZE_INDEX(input, 0);
            height  = TENSOR_VIEW_SIZE_INDEX(input, 1);
            channel = TENSOR_VIEW_SIZE_INDEX(input, 2);
            break;
        default:
            vxError("Input tensor error dimension[%u]\n", dims);
            return VX_ERROR_INVALID_DIMENSION;
    }

    ItemCount = width * height;
    Dim       = channel * width * height; /* default axis = 3, so softmax it in channel */
    pfMax       = (vx_float32_ptr)vxAllocateAndZeroMemory(ItemCount * sizeof(vx_float32));
    pfProSum    = (vx_float32_ptr)vxAllocateAndZeroMemory(ItemCount * sizeof(vx_float32));
    pfInput     = (vx_float32_ptr)vxAllocateAndZeroMemory(channel * ItemCount * sizeof(vx_float32));
    pfProbFP32  = (vx_float32_ptr)vxAllocateAndZeroMemory(channel * ItemCount * sizeof(vx_float32));

    index = 0;
    p_pfInput = pfInput;
    p_pfMax = pfMax;

    for(c = 0; c < channel; c++)
    {
        for(i = 0; i < ItemCount; i++)
        {
            index = c * ItemCount + i;
            p_pfInput[i] = vxnneGetDataExt(input_format, TENSOR_QUANT_TYPE(input), index, (vx_uint8_ptr)input_data_ptr, input_fp, TENSOR_TF_ZEROPOINT(input), TENSOR_TF_SCALE(input));
            p_pfMax[i]   = gcmMAX(p_pfMax[i], p_pfInput[i]);
        }
        p_pfInput += ItemCount;
    }
    p_pfMax += ItemCount;


    p_pfProbFP32 = pfProbFP32;
    p_pfInput = pfInput;
    p_pfMax = pfMax;
    p_pfProSum = pfProSum;

    for(c = 0; c < channel; c++)
    {
        for(i = 0; i < ItemCount; i++)
        {
            if (betas != VX_NULL)
            {
                beta = betas->value->f32;
                p_pfProbFP32[i] = gcoMATH_Exp((p_pfInput[i] - p_pfMax[i]) * beta);
            }
            else
                p_pfProbFP32[i] = gcoMATH_Exp(p_pfInput[i] - p_pfMax[i]);
        }
        p_pfProbFP32 += ItemCount;
        p_pfInput += ItemCount;
    }

    p_pfProbFP32 = pfProbFP32;
    p_pfProSum = pfProSum;

    for(c = 0; c < channel; c++)
    {
        for(i = 0; i < ItemCount; i++)
        {
            index = c * ItemCount + i;
            p_pfProSum[i] += pfProbFP32[index];
        }
    }
    p_pfProSum += ItemCount;


    p_pfProbFP32 = pfProbFP32;
    p_pfProSum = pfProSum;
    index = 0;

    for(c = 0; c < channel; c++)
    {
        for(i = 0; i < ItemCount; i++)
        {
            vx_float32 div = p_pfProbFP32[i] / p_pfProSum[i];


            vxnneSaveDataExt(output_format, TENSOR_QUANT_TYPE(output), index++, div, (vx_uint8_ptr)output_data_ptr, output_fp, TENSOR_TF_ZEROPOINT(output), TENSOR_TF_SCALE(output), outputRMode);
        }
        p_pfProbFP32 += ItemCount;
    }
    p_pfProSum += ItemCount;

    if(pfMax)
    {
        vxFree(pfMax);
    }

    if(pfProSum)
    {
        vxFree(pfProSum);
    }

    if(pfInput)
    {
        vxFree(pfInput);
    }

    if(pfProbFP32)
    {
        vxFree(pfProbFP32);
    }
    return status;
}


vx_status VX_CALLBACK vxoBaseKernel_NNSoftmaxLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoSoftmaxLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoSoftmaxLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    if (index != vxmLENGTH_OF(nn_Softmax_params) - 1) return VX_ERROR_INVALID_PARAMETERS;

    ptr->type                 = VX_TYPE_TENSOR;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoSoftmaxLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status  status                     = VX_SUCCESS;

    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_tensor  outputs                    = (vx_tensor)parameters[1];
    vx_enum    srcFormat                  = TENSOR_DATA_TYPE(inputs);
    vx_enum    dstFormat                  = TENSOR_DATA_TYPE(outputs);
    vx_uint32  dims                       = TENSOR_DIM_NUM(inputs);
    vx_uint32  width                      = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
    vx_uint32  height                     = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
    vx_bool    useShadeExe                = vx_false_e;
    vx_bool    enable_format              = vx_false_e;
    vx_bool    enable_tf_quantize         = vx_false_e;
    vx_bool    enable_float32             = vx_false_e;
    vx_uint32  batchCount                 = TENSOR_SIZE_INDEX(inputs, 3);
    vx_float32 beta                       = 1.0;
    vxnne_softmax_layer  softmaxLayer = VX_NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_softmax_layer_s), (gctPOINTER*)&softmaxLayer);
    if (!softmaxLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(softmaxLayer, sizeof(vxnne_softmax_layer_s));

    vxnneLayer_Initialize(&softmaxLayer->base,
                          "SoftmaxLayer",
                          node,
                          vxmOPERATION_COUNT(softmaxLayer),
                          softmaxLayer->operations,
                          VX_NULL);

    switch(dims)
    {
        case 1:
            batchCount   = 1;
            break;
        case 2:
            batchCount   = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
            break;
        case 3:
            batchCount   = 1;
            break;
        case 4:
            batchCount   = TENSOR_VIEW_SIZE_INDEX(inputs, 3);
            break;
        default:
            vxError("Input tensor error dimension[%u]\n", dims);
            status = VX_ERROR_INVALID_DIMENSION;
            goto exit;
    }

    if(node->base.context->evisNoInst.supportEVIS)
    {
        enable_float32 = (vx_bool)(dstFormat == VX_TYPE_FLOAT32 && ((width % 4 == 0) || ((width * height < IMG_MAX_WIDTH) && ((width * height % 4 == 0) || dims < 3)) || dims == 1));
        enable_format = (((srcFormat == VX_TYPE_INT8 ||  srcFormat == VX_TYPE_FLOAT16) && (dstFormat == VX_TYPE_FLOAT16 || enable_float32))
                         || ((srcFormat == VX_TYPE_BFLOAT16) && (dstFormat == VX_TYPE_BFLOAT16 || dstFormat == VX_TYPE_FLOAT16 || enable_float32))
                         || (srcFormat == VX_TYPE_INT16 && (dstFormat == VX_TYPE_INT16 || dstFormat == VX_TYPE_FLOAT16))
                         || (srcFormat == VX_TYPE_INT8 &&  dstFormat == VX_TYPE_INT8));
        enable_tf_quantize = ((srcFormat == VX_TYPE_UINT8) && (dstFormat == VX_TYPE_FLOAT16 || enable_float32 || dstFormat == VX_TYPE_UINT8));
    }
    else
    {
        enable_format = ((srcFormat == VX_TYPE_FLOAT32 ||  srcFormat == VX_TYPE_FLOAT16) && (dstFormat == VX_TYPE_FLOAT16 || dstFormat == VX_TYPE_FLOAT32));
        enable_tf_quantize = ((srcFormat == VX_TYPE_UINT8) && (dstFormat == VX_TYPE_FLOAT16 || dstFormat == VX_TYPE_FLOAT32 || dstFormat == VX_TYPE_UINT8));
    }
    /* Current the SH layer only process 3D tensor*/
    useShadeExe  = (enable_format || enable_tf_quantize);

   if(useShadeExe && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
   {
        vxnne_shader_executable shaderExecutable = VX_NULL;

        if (dims == 2) batchCount = 1;

        if(node->base.context->evisNoInst.supportEVIS)
        {
            shaderExecutable = vxnneGetSoftmaxShaderExecutable(node->base.context, VXNNE_KERNEL_SOFTMAX, &node->kernelAttributes.borderMode, dims, inputs, beta, outputs);

            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto exit;
            }
        }
        else
        {
            vx_scalar beta_s = vxCreateScalar(node->base.context, VX_TYPE_FLOAT32, &beta);

            shaderExecutable = vxnneGetGPUSoftmaxShaderExecutable(node->base.context, VXNNE_KERNEL_SOFTMAX, &node->kernelAttributes.borderMode, beta_s, inputs, outputs);

            vxReleaseScalar(&beta_s);

            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto exit;
            }
        }

        status = vxnneShaderOperation_Initialize(&softmaxLayer->softmax_SHoperation,
            &softmaxLayer->base,
            VXNNE_OPERATOR_SOFTMAX,
            batchCount,
            shaderExecutable);

        if (status != VX_SUCCESS) {
            goto exit;
        }

        vxnneOperation_AddReference(&softmaxLayer->softmax_SHoperation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&softmaxLayer->softmax_SHoperation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        vxnneLayer_SetOperation(&softmaxLayer->base, &softmaxLayer->softmax_SHoperation.base, 0);
    }
    else
    {
        vxnneOperation_Initialize(&softmaxLayer->softmax_sw_operation.base,
            &softmaxLayer->base,
            VXNNE_OPERATION_TARGET_SW,
            VXNNE_OPERATOR_SOFTMAX,
            vxnneExecuteSWSoftmax,
            VX_NULL,
            batchCount,
            0);

        vxnneLayer_SetOperation(&softmaxLayer->base, &softmaxLayer->softmax_sw_operation.base, 0);
        softmaxLayer->softmax_sw_operation.inputs           = inputs;
        softmaxLayer->softmax_sw_operation.outputs          = outputs;

        vxnneOperation_AddReference(&softmaxLayer->softmax_sw_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&softmaxLayer->softmax_sw_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }

    node->layer = &softmaxLayer->base;
    return status;

exit:
    if (softmaxLayer) {
        gcoOS_Free(gcvNULL, (gctPOINTER)softmaxLayer);
        softmaxLayer = VX_NULL;
    }
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoSoftmaxLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNSoftmaxLayer2(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoSoftmaxLayer2_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxoSoftmaxLayer2_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoSoftmaxLayer2_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_scalar  beta                       = (vx_scalar)parameters[1];
    vx_tensor  outputs                    = (vx_tensor)parameters[2];
    vx_float32 betaVal                    = beta->value->f32;
    vx_bool    useShadeExe                = vx_false_e;
    vx_bool    enable_format              = vx_false_e;
    vx_bool    enable_tf_quantize         = vx_false_e;
    vx_bool    enable_float32             = vx_false_e;
    vx_uint32  batchCount                 = TENSOR_SIZE_INDEX(inputs, 3);
    vx_enum    srcFormat                  = TENSOR_DATA_TYPE(inputs);
    vx_enum    dstFormat                  = TENSOR_DATA_TYPE(outputs);
    vx_uint32  dims                       = TENSOR_DIM_NUM(inputs);
    vx_uint32  width                      = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
    vx_uint32  height                     = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
    vx_uint32  idx                        = 0;
    vx_uint32  numTmpTensor               = 0;
    vxnne_softmax2_layer  softmax2Layer   = VX_NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }
    gcoOS_Allocate(gcvNULL, sizeof(vxnne_softmax2_layer_s), (gctPOINTER*)&softmax2Layer);
    if (!softmax2Layer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("Out of Memory at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(softmax2Layer, sizeof(vxnne_softmax2_layer_s));

    vxnneLayer_Initialize(&softmax2Layer->base,
                          "SoftMax2",
                          node,
                          vxmOPERATION_COUNT(softmax2Layer),
                          softmax2Layer->operations,
                          VX_NULL);

    if(node->base.context->evisNoInst.supportEVIS)
    {
        enable_float32 = (vx_bool)(dstFormat == VX_TYPE_FLOAT32 && ((width % 4 == 0) || ((width * height < IMG_MAX_WIDTH) && ((width * height % 4 == 0) || dims < 3)) || dims == 1));
        enable_format = (((srcFormat == VX_TYPE_INT8 ||  srcFormat == VX_TYPE_FLOAT16) && (dstFormat == VX_TYPE_FLOAT16 || enable_float32))
                         || ((srcFormat == VX_TYPE_BFLOAT16) && (dstFormat == VX_TYPE_BFLOAT16 || dstFormat == VX_TYPE_FLOAT16 || enable_float32))
                         || (srcFormat == VX_TYPE_INT16 && (dstFormat == VX_TYPE_INT16 || dstFormat == VX_TYPE_FLOAT16))
                         || (srcFormat == VX_TYPE_INT8 &&  dstFormat == VX_TYPE_INT8));
        enable_tf_quantize = ((srcFormat == VX_TYPE_UINT8) && (dstFormat == VX_TYPE_FLOAT16 || enable_float32 || dstFormat == VX_TYPE_UINT8));
    }
    else
    {
        enable_format = ((srcFormat == VX_TYPE_FLOAT32 ||  srcFormat == VX_TYPE_FLOAT16) && (dstFormat == VX_TYPE_FLOAT16 || dstFormat == VX_TYPE_FLOAT32));
        enable_tf_quantize = ((srcFormat == VX_TYPE_UINT8) && (dstFormat == VX_TYPE_FLOAT16 || dstFormat == VX_TYPE_FLOAT32 || dstFormat == VX_TYPE_UINT8));
    }

    /* Current the SH layer only process 3D tensor*/
    useShadeExe  = (enable_format || enable_tf_quantize);

    if(useShadeExe && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
    {
        vxnne_shader_executable shaderExecutable = VX_NULL;

        if(node->base.context->evisNoInst.supportEVIS)
        {
            shaderExecutable = vxnneGetSoftmaxShaderExecutable(node->base.context, VXNNE_KERNEL_SOFTMAX, &node->kernelAttributes.borderMode, dims, inputs, betaVal, outputs);
            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto exit;
            }

            status = vxnneShaderOperation_Initialize(&softmax2Layer->softmax2_SHoperation,
                &softmax2Layer->base,
                VXNNE_OPERATOR_SOFTMAX,
                batchCount,
                shaderExecutable);
            if (status != VX_SUCCESS) goto exit;

            vxnneOperation_AddReference(&softmax2Layer->softmax2_SHoperation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&softmax2Layer->softmax2_SHoperation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            vxnneLayer_SetOperation(
                &softmax2Layer->base,
                &softmax2Layer->softmax2_SHoperation.base,
                0);
        }
        else
        {
            shaderExecutable = vxnneGetGPUSoftmaxShaderExecutable(node->base.context, VXNNE_KERNEL_SOFTMAX, &node->kernelAttributes.borderMode, beta, inputs, outputs);
            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto exit;
            }

            status = vxnneShaderOperation_Initialize(&softmax2Layer->softmax2_SHoperation,
                &softmax2Layer->base,
                VXNNE_OPERATOR_SOFTMAX,
                batchCount,
                shaderExecutable);
            if (status != VX_SUCCESS) goto exit;

            vxnneOperation_AddReference(&softmax2Layer->softmax2_SHoperation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&softmax2Layer->softmax2_SHoperation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            vxnneLayer_SetOperation(
                &softmax2Layer->base,
                &softmax2Layer->softmax2_SHoperation.base,
                idx++);
        }
    }
    else
    {
        vxnneOperation_Initialize(&softmax2Layer->softmax2_sw_operation.base,
            &softmax2Layer->base,
            VXNNE_OPERATION_TARGET_SW,
            VXNNE_OPERATOR_SOFTMAX,
            vxnneExecuteSWSoftmax,
            VX_NULL,
            batchCount,
            0);

        vxnneLayer_SetOperation(
            &softmax2Layer->base,
            &softmax2Layer->softmax2_sw_operation.base,
            0);

        softmax2Layer->softmax2_sw_operation.inputs           = inputs;
        softmax2Layer->softmax2_sw_operation.beta             = beta;
        softmax2Layer->softmax2_sw_operation.outputs          = outputs;

        vxnneOperation_AddReference(&softmax2Layer->softmax2_sw_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&softmax2Layer->softmax2_sw_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }

    softmax2Layer->base.num_temp_tensors = numTmpTensor;
    node->layer = &softmax2Layer->base;

    return status;
exit:
    if (softmax2Layer) gcoOS_Free(gcvNULL, (gctPOINTER)softmax2Layer);
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoSoftmaxLayer2_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

