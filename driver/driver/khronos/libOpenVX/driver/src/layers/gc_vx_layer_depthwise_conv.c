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
#include <gc_vx_nn_wb.h>
#include <layers/gc_vx_layer_depthwise_conv.h>

extern vx_status vxnneExecuteSWConvolution(vxnne_operation operation);

extern vx_status vxnneConvolutionReluPoolingInitializer(
    vx_node node,
    char* name,
    vx_tensor inputs,
    vx_weights_biases_parameter weights_biases,
    vx_size dilationX,
    vx_size dilationY,
    vx_uint32 pad_x_left,
    vx_uint32 pad_x_right,
    vx_uint32 pad_y_top,
    vx_uint32 pad_y_bottom,
    vx_enum conv_rounding_type,
    vx_bool enable_relu,
    vx_bool enable_pooling,
    vx_enum pool_type,
    vx_uint32 pool_size_x,
    vx_uint32 pool_size_y,
    vx_enum padMode,
    vx_scalar padConst,
    vx_array  internalScale,
    vx_array  internalZp,
    vx_array  internal_data_type,
    vx_tensor outputs
    );

VX_PRIVATE_API vx_status vxnneExecuteSW_Depthwise_Convolution_DeInilition(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_convolution_operation convOperation   = (vxnne_convolution_operation)operation;

    if (convOperation->weights)
        vxoTensor_ReleaseTensor(&convOperation->weights);

    if (convOperation->downScaleSizeRounding)
        vxReleaseScalar(&convOperation->downScaleSizeRounding);

    return status;
}

VX_PRIVATE_API vx_status vxoNNSWDepthwiseConvolution(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_depthwise_convolution_operation dcOperation = (vxnne_depthwise_convolution_operation)operation;


    vx_tensor inputs  = dcOperation->inputs;
    vx_tensor outputs = dcOperation->outputs;
    vx_tensor weights = dcOperation->weights;
    vx_tensor biases  = dcOperation->biases;
    vx_scalar padXLeft = dcOperation->padXLeft;
    vx_scalar padXRight = dcOperation->padXRight;
    vx_scalar padYTop = dcOperation->padYTop;
    vx_scalar padYBottom = dcOperation->padYBottom;
    vx_scalar downScaleSizeRounding = dcOperation->downScaleSizeRounding;
    vx_scalar strideX   = dcOperation->stride_x;
    vx_scalar strideY   = dcOperation->stride_y;
    vx_scalar dilationX = dcOperation->dilationX;
    vx_scalar dilationY = dcOperation->dilationY;

    vx_int32 channel_multiplier;

    vx_int32 input_width = TENSOR_SIZE_INDEX(inputs, 0);    /* W */
    vx_int32 input_height = TENSOR_SIZE_INDEX(inputs, 1);   /* H */
    vx_int32 input_depth = TENSOR_SIZE_INDEX(inputs, 2);    /* D */
    /*vx_int32 input_batch = TENSOR_SIZE_INDEX(inputs, 3);*/    /* N */

    vx_int32 output_width = TENSOR_SIZE_INDEX(outputs, 0);   /* W */
    vx_int32 output_height = TENSOR_SIZE_INDEX(outputs, 1);  /* H */
    vx_int32 output_depth = TENSOR_SIZE_INDEX(outputs, 2);   /* D */
    vx_int32 output_batch = TENSOR_SIZE_INDEX(outputs, 3);   /* N */

    vx_int32 kernel_width = TENSOR_SIZE_INDEX(weights, 0);      /* W */
    vx_int32 kernel_height = TENSOR_SIZE_INDEX(weights, 1);     /* H */
    /*vx_int32 kernel_in_depth = TENSOR_SIZE_INDEX(weights, 2);*/   /* D */
    /*vx_int32 kernel_out_depth = TENSOR_SIZE_INDEX(weights, 3);*/  /* N */
    vx_int32 pad_left = padXLeft->value->n32, pad_right = padXRight->value->n32;
    vx_int32 pad_top = padYTop->value->n32, pad_bottom = padYBottom->value->n32;
    vx_float64 sum = .0f, in = .0f, weight = .0f;
    vx_int32 strideXvalue = 1, strideYvalue = 1;
    vx_int32 b = 0, d = 0, h = 0, w = 0, dm = 0, m = 0, n = 0, kstart_x = 0, kstart_y = 0;
    vx_int32 dilation_x = (dilationX) ? dilationX->value->n32 + 1 : 1;
    vx_int32 dilation_y = (dilationY) ? dilationY->value->n32 + 1 : 1;


    if (strideX)
    {
        strideXvalue = strideX->value->n32;
    }
    else
    {
        if ((input_width == 1) || (output_width == 1))
        {
            strideXvalue = 1;
        }
        else
        {
            strideXvalue = vxoNNExternsionConvlutionRound((vx_float32)(input_width + pad_left + pad_right - kernel_width) / (output_width - 1), downScaleSizeRounding->value->e);
        }
    }

    if (strideY)
    {
        strideYvalue = strideY->value->n32;
    }
    else
    {
        if ((input_height == 1) || (output_height == 1))
        {
            strideYvalue = 1;
        }
        else
        {
            strideYvalue = vxoNNExternsionConvlutionRound((vx_float32)(input_height + pad_top + pad_bottom - kernel_height) / (output_height - 1), downScaleSizeRounding->value->e);
        }
    }

    gcmASSERT(strideXvalue > 0);
    gcmASSERT(strideYvalue > 0);

    channel_multiplier = (dcOperation->depth_multiplier != VX_NULL) ? dcOperation->depth_multiplier->value->n32 : 1;

    for (b = 0; b < output_batch; b ++)
    {
        for (d = 0; d < output_depth; d ++)
        {
            vx_int32 output_base_offset = d * output_width * output_height + b * output_depth * output_width * output_height;
            vx_int32 input_slice_index = d / channel_multiplier;
            vx_int32 input_base_offset;

            if (input_slice_index < input_depth)
            {
                input_base_offset = input_slice_index * input_width * input_height + b * input_depth * input_width * input_height;
            }
            else
            {
                input_base_offset = -1;
            }

            for (h = 0; h < output_height; h ++)
            {
                vx_int32 y_start = h * strideYvalue - pad_top;
                vx_int32 y_end = gcmMIN((y_start + kernel_height * dilation_y), input_height);

                kstart_y = (y_start < 0) ? (gcmALIGN_NP2_SAFE(-y_start, dilation_y) / dilation_y) : 0;
                y_start = gcmMAX(0, y_start);
                if (y_start < 0 && dilation_y > 1)
                {
                    y_start = gcmMAX(y_start, (h * strideYvalue) % dilation_y);
                }
                else
                {
                    y_start = gcmMAX(y_start, 0);
                }

                for (w = 0; w < output_width; w ++)
                {
                    vx_int32 x = 0, y = 0;
                    vx_int32 x_start = w * strideXvalue - pad_left;
                    vx_int32 x_end = gcmMIN((x_start + kernel_width * dilation_x), input_width);

                    kstart_x = (x_start < 0) ? (gcmALIGN_NP2_SAFE(-x_start, dilation_x) / dilation_x) : 0;
                    x_start = gcmMAX(0, x_start);
                    if (x_start < 0 && dilation_x > 1)
                    {
                        x_start = gcmMAX(x_start, (w * strideXvalue) % dilation_x);
                    }
                    else
                    {
                        x_start = gcmMAX(x_start, 0);
                    }

                    sum = 0.f;

                    if (input_base_offset != -1)
                    {
                        for (y = y_start, n = kstart_y; y < y_end; y += dilation_y, n ++)
                        {
                            for (x = x_start, m = kstart_x; x < x_end; x += dilation_x, m ++)
                            {
                                in = VX_GET_DATA_FROM_TENSOR(inputs, y * input_width + x + input_base_offset);
                                weight = VX_GET_DATA_FROM_TENSOR(weights, n * kernel_width + m + (d + dm) * kernel_width * kernel_height);

                                sum += in * weight;
                            }
                        }
                    }

                    if (biases)
                    {
                        sum += VX_GET_DATA_FROM_TENSOR(biases, d + dm);
                    }

                    VX_SAVE_DATA_TO_TENSOR(outputs, sum, h * output_width + w + output_base_offset);
                }
            }
        }
    }

    return status;

}

VX_PRIVATE_API vx_status vxoNNSWDepthwiseConv_DilationWeight(vx_tensor weights, vx_tensor weight_dilation, vx_int32 channels_multiplier)
{
    vx_status status = VX_SUCCESS;

    vx_int32 weights_width = TENSOR_SIZE_INDEX(weight_dilation, 0), weights_height = TENSOR_SIZE_INDEX(weight_dilation, 1), weights_input_depth = TENSOR_SIZE_INDEX(weight_dilation, 2), weights_output_depth = TENSOR_SIZE_INDEX(weight_dilation, 3);
    vx_int32 d = 0, b = 0, item_size = TENSOR_DATA_SIZE(weights), slice = weights_width * weights_height, multiplier = weights_output_depth/weights_input_depth;
    vx_uint8_ptr input_base = TENSOR_LOGICAL_ADDR(weights);
    vx_uint8_ptr output_base = VX_NULL;

    gcmASSERT(channels_multiplier == multiplier);

    if (TENSOR_LOGICAL_ADDR(weight_dilation) == VX_NULL && vxoTensor_AllocateMemory(weight_dilation) != VX_SUCCESS)
        return VX_ERROR_NO_MEMORY;

    output_base = TENSOR_LOGICAL_ADDR(weight_dilation);

    for (b = 0; b < weights_output_depth; b ++)
    {
        vx_int32 output_offset = b * weights_input_depth * slice;
        for (d = 0; d < weights_input_depth; d ++)
        {
            if (TENSOR_DATA_TYPE(weights) == VX_TYPE_UINT8 && TENSOR_QUANT_TYPE(weights) == VX_QUANT_AFFINE_SCALE && (TENSOR_DATA_TYPE(weights) == (TENSOR_DATA_TYPE(weight_dilation))))
            {
                vx_int32 s = 0;
                for (s = 0; s < slice; s++)
                {
                    if (d == (b/multiplier))
                        output_base[output_offset + d * slice + s] = input_base[b * slice + s];
                    else
                        output_base[output_offset + d * slice + s] = (vx_uint8)TENSOR_TF_ZEROPOINT(weights);
                }
            }
            else
            {
                if (d == (b/multiplier))
                    memcpy(output_base + output_offset * item_size + d * slice * item_size, input_base + b * slice * item_size, slice * item_size);
                else
                    memset(output_base + output_offset * item_size + d * slice * item_size, 0, slice * item_size);
            }
        }
    }

    return status;
}

VX_PRIVATE_API vx_tensor _mergeInputZeroPoint2Bias(vx_graph graph, vx_tensor input, vx_tensor weight, vx_tensor bias)
{
    vx_tensor new_bias = VX_NULL;
    vx_tensor_create_params_t params;
    vx_int32_ptr bias_base_ptr = VX_NULL;
    vx_int32_ptr bias_base_prev_ptr = VX_NULL;
    vx_uint8_ptr kernelDataPtr = VX_NULL;
    vx_uint32 channelCount = TENSOR_VIEW_SIZE_INDEX(weight, 2);
    vx_uint32 sizes[2] = {channelCount, 1};
    vx_enum  weight_quant_format = TENSOR_QUANT_TYPE(weight);

    gcoOS_MemFill(&params, 0, sizeof(vx_tensor_create_params_t));

    params.num_of_dims = 2;
    params.sizes = sizes;
    params.data_format = TENSOR_DATA_TYPE(weight);
    params.quant_format = weight_quant_format;
    if (params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
    {
        params.quant_data.dfp.fixed_point_pos = TENSOR_POS(input) * TENSOR_POS(weight);
        params.data_format = VX_TYPE_INT32;
    }
    else if (params.quant_format == VX_QUANT_AFFINE_SCALE_PER_CHANNEL)
    {
        params.quant_data.affinePerChannel.channelDim = 0;
        params.quant_data.affinePerChannel.scaleCount = channelCount;
        params.quant_data.affinePerChannel.zeroPointCount = channelCount;
        params.quant_data.affinePerChannel.scales = (vx_float32 *)vxAllocateAndZeroMemory(channelCount * sizeof(vx_float32));
        if (params.quant_data.affinePerChannel.scales == VX_NULL)
        {
            vxError("vxoTensor_Create: Out of memory");
            goto OnError;
        }

        gcoOS_MemFill(params.quant_data.affinePerChannel.scales, 0, channelCount * sizeof(vx_float32));

        params.quant_data.affinePerChannel.zeroPoint = (vx_int32 *)vxAllocateAndZeroMemory(channelCount * sizeof(vx_int32));
        if (params.quant_data.affinePerChannel.zeroPoint == VX_NULL)
        {
            vxError("vxoTensor_Create: Out of memory");
            goto OnError;
        }

        gcoOS_MemFill(params.quant_data.affinePerChannel.zeroPoint, 0, channelCount * sizeof(vx_int32));

        params.data_format = VX_TYPE_INT32;
    }
    else
    {
        params.quant_data.affine.scale = TENSOR_TF_SCALE(input) * TENSOR_TF_SCALE(weight);
        params.quant_data.affine.zeroPoint = 0;
        params.data_format = VX_TYPE_INT32;
    }

    if (bias && params.quant_format == VX_QUANT_AFFINE_SCALE_PER_CHANNEL)
    {
        gcoOS_MemCopy(params.quant_data.affinePerChannel.scales, TENSOR_TF_SCALE_POINTER(bias), channelCount * sizeof(vx_float32));
    }
    else  if (params.quant_format == VX_QUANT_AFFINE_SCALE_PER_CHANNEL)
    {
        vx_uint32 i = 0;
        vx_float32_ptr bias_scales = params.quant_data.affinePerChannel.scales;
        vx_float32 input_scales = TENSOR_TF_SCALE(input);
        vx_float32_ptr weight_scales = TENSOR_TF_SCALE_POINTER(weight);

        for (i = 0; i < channelCount; i++)
        {
            bias_scales[i] = input_scales * weight_scales[i];
        }
    }

    new_bias = vxoTensor_CreateTensor2(vxGetContext((vx_reference)graph), &params, sizeof(vx_tensor_create_params_t));
    if (vxoTensor_AllocateMemory(new_bias) != VX_SUCCESS)
    {
        vxError("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
        goto OnError;
    }

    vxoTensor_GetTensorViewMemory(new_bias, (gctPOINTER*)&bias_base_ptr, VX_NULL);
    vxoTensor_GetTensorViewMemory(weight, (gctPOINTER*)&kernelDataPtr, VX_NULL);

    if (bias)
    {
        vxoTensor_GetTensorViewMemory(bias, (gctPOINTER*)&bias_base_prev_ptr, VX_NULL);
        gcoOS_MemCopy(bias_base_ptr, bias_base_prev_ptr, channelCount * sizeof(vx_int32));
    }

    if (TENSOR_TF_ZEROPOINT(input) != 0)
    {
        vx_uint32 filterIndex   = 0;
        vx_uint32 x             = 0;
        vx_uint32 y             = 0;
        vx_int32  weightZp      = 0;
        vx_int32  input_zp      = TENSOR_TF_ZEROPOINT(input);
        vx_uint32 weight_x      = TENSOR_VIEW_SIZE_INDEX(weight, 0);
        vx_uint32 weight_y      = TENSOR_VIEW_SIZE_INDEX(weight, 1);
        vx_enum   weight_format = TENSOR_DATA_TYPE(weight);
        vx_uint32 weightSize    = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);

        for (filterIndex = 0; filterIndex < channelCount; filterIndex++)
        {
            if (weight_quant_format == VX_QUANT_AFFINE_SCALE)
            {
                weightZp = TENSOR_TF_ZEROPOINT(weight);
            }

            for (y = 0; y < weight_y; y++)
            {
                for (x = 0; x < weight_x; x++)
                {
                    vx_int32 weight = 0;

                    if(weight_format == VX_TYPE_UINT8)
                    {
                        weight = *((vx_uint8 *)kernelDataPtr);
                    }
                    else if(weight_format == VX_TYPE_INT8)
                    {
                        weight = *((vx_int8 *)kernelDataPtr);
                    }

                    kernelDataPtr = kernelDataPtr + weightSize;

                    /*Calc sum((coef[i] - coefZP) * InZP) */
                    bias_base_ptr[filterIndex] -= (weight - weightZp) * input_zp;
                }
            }
        }
    }

    if (params.quant_data.affinePerChannel.scales)
    {
        vxFree(params.quant_data.affinePerChannel.scales);
        params.quant_data.affinePerChannel.scales = VX_NULL;
    }

    if (params.quant_data.affinePerChannel.zeroPoint != VX_NULL)
    {
        vxFree(params.quant_data.affinePerChannel.zeroPoint);
        params.quant_data.affinePerChannel.zeroPoint = VX_NULL;
    }

    TENSOR_DATA_LIFETIME(new_bias) = VX_TENSOR_LIFE_TIME_STATIC;

    return new_bias;

OnError:
    if (params.quant_data.affinePerChannel.scales)
    {
        vxFree(params.quant_data.affinePerChannel.scales);
        params.quant_data.affinePerChannel.scales = VX_NULL;
    }

    if (params.quant_data.affinePerChannel.zeroPoint != VX_NULL)
    {
        vxFree(params.quant_data.affinePerChannel.zeroPoint);
        params.quant_data.affinePerChannel.zeroPoint = VX_NULL;
    }

    if (new_bias)
    {
        vxoTensor_ReleaseTensor(&new_bias);
    }

    return VX_NULL;
}

#if REGISTER_FRAME

VX_PRIVATE_API vx_status vxoNNDepthwiseConvLayer_SW_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vxnne_depthwise_convolution_layer  depthwiseConvolutionLayer = (vxnne_depthwise_convolution_layer)ops_layer;

    vx_tensor inputs = (vx_tensor)parameters[0];
    vx_tensor weights = (vx_tensor)parameters[1];
    vx_tensor biases = (vx_tensor)parameters[2];
    vx_scalar padXLeft = (vx_scalar)parameters[3];
    vx_scalar padXRight = (vx_scalar)parameters[4];
    vx_scalar padYTop = (vx_scalar)parameters[5];
    vx_scalar padYBottom = (vx_scalar)parameters[6];
    vx_scalar dilationX = (vx_scalar)parameters[9];
    vx_scalar dilationY = (vx_scalar)parameters[10];
    vx_scalar strideX   = (vx_scalar)parameters[11];
    vx_scalar strideY   = (vx_scalar)parameters[12];
    vx_scalar depth_multiplier = (vx_scalar)parameters[13];
    vx_scalar downScaleSizeRounding = (vx_scalar)parameters[18];
    vx_tensor outputs = (vx_tensor)parameters[19];

    vx_uint32  batchCount = TENSOR_SIZE_INDEX(inputs, 3);

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxnneOperation_Initialize(&depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.base,
        &depthwiseConvolutionLayer->base,
        VXNNE_OPERATION_TARGET_SW,
        VXNNE_OPERATOR_DEPTHWISE_CONV,
        vxoNNSWDepthwiseConvolution,
        VX_NULL,
        batchCount,
        0));

    vxmONERROR(vxnneLayer_SetOperation(
        &depthwiseConvolutionLayer->base,
        &depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.base,
        0));

    depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.inputs = inputs;
    depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.weights = weights;
    depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.biases = biases;
    depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.padXLeft = padXLeft;
    depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.padXRight = padXRight;
    depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.padYTop = padYTop;
    depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.padYBottom = padYBottom;
    depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.dilationX = dilationX;
    depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.dilationY = dilationY;
    depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.depth_multiplier = depth_multiplier;
    depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.stride_x  = strideX;
    depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.stride_y  = strideY;
    depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.downScaleSizeRounding = downScaleSizeRounding;
    depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.outputs = outputs;

    vxmONERROR(vxnneOperation_AddReference(&depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.base, (vx_reference)weights, VXNNE_OPERATION_REFENRENCE_INPUT));
    if (biases)
    {
        vxmONERROR(vxnneOperation_AddReference(&depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.base, (vx_reference)biases, VXNNE_OPERATION_REFENRENCE_INPUT));
    }
    vxmONERROR(vxnneOperation_AddReference(&depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNDepthwiseConvLayer_SH_EVIS_Support_Ext(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_tensor inputs = (vx_tensor)parameters[0];
    vx_tensor weights = (vx_tensor)parameters[1];
    vx_tensor biases = (vx_tensor)parameters[2];
    vx_tensor outputs = (vx_tensor)parameters[19];

    vx_enum  inputFormat = TENSOR_DATA_TYPE(inputs);
    vx_enum  weightFormat = TENSOR_DATA_TYPE(weights);
    vx_enum  biasFormat = VX_TYPE_INVALID;
    vx_enum  outputFormat = TENSOR_DATA_TYPE(outputs);
    vx_bool  dataformat_flag[5] = { vx_false_e };
    vx_bool  is_w_sym_per_channel_quant = (vx_bool)(TENSOR_QUANT_TYPE(weights) == VX_QUANT_AFFINE_SCALE_PER_CHANNEL);
    vx_bool  is_b_sym_per_channel_quant = biases == NULL? vx_false_e : (vx_bool)(TENSOR_QUANT_TYPE(biases) == VX_QUANT_AFFINE_SCALE_PER_CHANNEL);
    vx_bool  is_sym_per_channel_quant   = is_w_sym_per_channel_quant && is_b_sym_per_channel_quant;
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);
    vx_scalar dilationX  = (vx_scalar)parameters[9];
    vx_scalar dilationY  = (vx_scalar)parameters[10];
    vx_int32  dilation_x = dilationX->value->n32 + 1;
    vx_int32  dilation_y = dilationY->value->n32 + 1;
    vx_bool   is_support_dilation = vx_true_e;

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (biases != VX_NULL)
        biasFormat = TENSOR_DATA_TYPE(biases);

    if (evis)
    {
        dataformat_flag[0] = (vx_bool)(inputFormat == VX_TYPE_FLOAT16 && weightFormat == VX_TYPE_FLOAT16 && (biasFormat == VX_TYPE_INVALID || biasFormat == VX_TYPE_FLOAT32) && outputFormat == VX_TYPE_FLOAT16);
        dataformat_flag[3] = (vx_bool)(inputFormat == VX_TYPE_INT16 && weightFormat == VX_TYPE_INT16
            && (biasFormat == VX_TYPE_INT32 || biasFormat == VX_TYPE_INT64 || biasFormat == VX_TYPE_INVALID)
            && outputFormat == VX_TYPE_INT16);
        dataformat_flag[2] = (vx_bool)(inputFormat == VX_TYPE_INT8 && weightFormat == VX_TYPE_INT8 && (biasFormat == VX_TYPE_INVALID || biasFormat == VX_TYPE_INT32) && outputFormat == VX_TYPE_INT8);
        if ((biases == VX_NULL) && ((1 != dilation_x) || (1 != dilation_y)))
        {
            is_support_dilation = vx_false_e;
        }
    }
    else
    {
        dataformat_flag[0] = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && weightFormat == VX_TYPE_FLOAT16 && (biasFormat == VX_TYPE_INVALID || biasFormat == VX_TYPE_FLOAT32) && outputFormat == VX_TYPE_FLOAT16) ||
            (inputFormat == VX_TYPE_FLOAT16 && weightFormat == VX_TYPE_FLOAT16 && (biasFormat == VX_TYPE_INVALID || biasFormat == VX_TYPE_FLOAT16) && outputFormat == VX_TYPE_FLOAT16) ||
            (inputFormat == VX_TYPE_FLOAT32 && weightFormat == VX_TYPE_FLOAT32 && (biasFormat == VX_TYPE_INVALID || biasFormat == VX_TYPE_FLOAT32) && outputFormat == VX_TYPE_FLOAT32));
    }
    dataformat_flag[4] = (vx_bool)(inputFormat == VX_TYPE_UINT8 && weightFormat == VX_TYPE_INT8 && biasFormat == VX_TYPE_INT32 && outputFormat == VX_TYPE_UINT8 && is_sym_per_channel_quant);
    dataformat_flag[1] = (vx_bool)(inputFormat == VX_TYPE_UINT8 && weightFormat == VX_TYPE_UINT8 && (biasFormat == VX_TYPE_INVALID || biasFormat == VX_TYPE_INT32) && outputFormat == VX_TYPE_UINT8);

    support = support && (vx_bool)(dataformat_flag[0] || dataformat_flag[1] || dataformat_flag[2] || dataformat_flag[3] || dataformat_flag[4]);
    support = support && is_support_dilation;
    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_bool vxoNNDepthwiseConvLayer_SH_EVIS_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && node->base.context->evisNoInst.supportEVIS;

    if (!support)return support;

    support = support && vxoNNDepthwiseConvLayer_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_true_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_tensor _createTensorFromData(vx_graph graph, vx_tensor_create_params_t params, vx_uint8_ptr data)
{
    vx_tensor tensor = VX_NULL;
    vx_uint8_ptr input_base_ptr = VX_NULL;
    vx_uint32 tensor_size = 0;

    tensor = vxoTensor_CreateTensor2(vxGetContext((vx_reference)graph), &params, sizeof(vx_tensor_create_params_t));
    if (vxoTensor_AllocateMemory(tensor) != VX_SUCCESS)
    {
        vxError("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
        return VX_NULL;
    }

    vxoTensor_GetTensorSize(tensor, &tensor_size);

    input_base_ptr = TENSOR_LOGICAL_ADDR(tensor);
    memcpy(input_base_ptr, data, tensor_size);

    return tensor;
}

VX_PRIVATE_API vx_status vxoNNDepthwiseConvLayer_SH_EVIS_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxnne_depthwise_convolution_layer  depthwiseConvolutionLayer = (vxnne_depthwise_convolution_layer)ops_layer;

    vx_tensor inputs = (vx_tensor)parameters[0];
    vx_tensor weights = (vx_tensor)parameters[1];
    vx_tensor biases = (vx_tensor)parameters[2];
    vx_scalar padXLeft = (vx_scalar)parameters[3];
    vx_scalar padXRight = (vx_scalar)parameters[4];
    vx_scalar padYTop = (vx_scalar)parameters[5];
    vx_scalar padYBottom = (vx_scalar)parameters[6];
    vx_scalar dilationX = (vx_scalar)parameters[9];
    vx_scalar dilationY = (vx_scalar)parameters[10];
    vx_scalar strideX   = (vx_scalar)parameters[11];
    vx_scalar strideY   = (vx_scalar)parameters[12];
    vx_scalar depth_multiplier = (vx_scalar)parameters[13];
    vx_scalar downScaleSizeRounding = (vx_scalar)parameters[18];
    vx_tensor outputs = (vx_tensor)parameters[19];

    vx_uint32  batchCount = TENSOR_SIZE_INDEX(inputs, 3);

    /*vx_enum  biasFormat = VX_TYPE_INVALID;*/
    vx_uint32  operation_idx = 0;
    vx_int32   strideXvalue  = 0;
    vx_int32   strideYvalue  = 0;
    vxnne_shader_executable shaderExecutable = NULL;
    vx_tensor               newBiases = NULL;
    vx_tensor               tensorCopy = NULL;
    vx_uint32     input_width   = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
    vx_uint32     input_height  = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
    vx_uint32     output_width  = TENSOR_VIEW_SIZE_INDEX(outputs, 0);
    vx_uint32     output_height = TENSOR_VIEW_SIZE_INDEX(outputs, 1);
    vx_int32      kernel_width  = TENSOR_VIEW_SIZE_INDEX(weights, 0);
    vx_int32      kernel_height = TENSOR_VIEW_SIZE_INDEX(weights, 1);
    vx_int32      padLeftv      = padXLeft->value->n32;
    vx_int32      padRightv     = padXRight->value->n32;
    vx_int32      padTopv       = padYTop->value->n32;
    vx_int32      padBottomv    = padYBottom->value->n32;
    vx_tensor     scales        = NULL;
    vx_uint32     numTmpTensor  = 0;

    if (strideX)
    {
        strideXvalue = strideX->value->n32;
    }
    else
    {
        if ((input_width == 1) || (output_width == 1))
        {
            strideXvalue = 1;
        }
        else
        {
            strideXvalue = vxoNNExternsionConvlutionRound((vx_float32)(input_width + padLeftv + padRightv - kernel_width) / (output_width - 1), downScaleSizeRounding->value->e);
        }
    }

    if (strideY)
    {
        strideYvalue = strideY->value->n32;
    }
    else
    {
        if ((input_height == 1) || (output_height == 1))
        {
            strideYvalue = 1;
        }
        else
        {
            strideYvalue = vxoNNExternsionConvlutionRound((vx_float32)(input_height + padTopv + padBottomv - kernel_height) / (output_height - 1), downScaleSizeRounding->value->e);
        }
    }

    if (TENSOR_QUANT_TYPE(weights) == VX_QUANT_AFFINE_SCALE_PER_CHANNEL)
    {
        vx_tensor_create_params_t params;
        vx_enum        input_quant      = TENSOR_QUANT_TYPE(inputs);
        vx_uint32      channelCount     = TENSOR_TF_SCALE_COUNT(weights);
        vx_uint32      sizes[2]         = {channelCount, 1};
        vx_float32_ptr scales_data_ptr  = VX_NULL;
        vx_enum        ouput_quant_type = TENSOR_QUANT_TYPE(outputs);
        vx_float32     input_scales     = 1;
        vx_uint32      idx              = 0;

        if (input_quant == VX_QUANT_AFFINE_SCALE)
            input_scales = TENSOR_TF_SCALE(inputs);

        scales_data_ptr = (vx_float32 *)vxAllocateAndZeroMemory(channelCount * sizeof(vx_float32));
        if (scales_data_ptr == VX_NULL)
        {
            status = VX_ERROR_NO_MEMORY;
            vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            goto OnError;
        }
        gcoOS_MemCopy(scales_data_ptr, TENSOR_TF_SCALE_POINTER(weights), channelCount * sizeof(vx_float32));

        for (idx = 0; idx < channelCount; idx++)
        {
            scales_data_ptr[idx] *= input_scales;
            if (ouput_quant_type == VX_QUANT_AFFINE_SCALE)
                scales_data_ptr[idx] /= TENSOR_TF_SCALE(outputs);
        }

        gcoOS_MemFill(&params, 0, sizeof(vx_tensor_create_params_t));

        params.num_of_dims = 2;
        params.sizes = sizes;
        params.data_format = VX_TYPE_FLOAT32;

        scales = _createTensorFromData(ops_layer->node->graph, params, (vx_uint8_ptr)scales_data_ptr);
        depthwiseConvolutionLayer->base.temp_tensors[numTmpTensor++] = scales;
        if (scales_data_ptr)
        {
            vxFree(scales_data_ptr);
            scales_data_ptr = VX_NULL;
        }
    }

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    newBiases = biases;
    tensorCopy = inputs;
    if (newBiases)
    {
        shaderExecutable = vxnneGetDepthwiseConvShaderExecutable(ops_layer->node->base.context,
            VXNNE_KERNEL_DEPTHWISE_CONV,
            &ops_layer->node->kernelAttributes.borderMode,
            tensorCopy,
            weights,
            newBiases,
            padXLeft,
            padXRight,
            padYTop,
            padYBottom,
            VX_PAD_CONSTANT,
            VX_NULL,
            dilationX,
            dilationY,
            depth_multiplier,
            VX_NULL,
            VX_NULL,
            VX_NULL,
            VX_NULL,
            downScaleSizeRounding,
            strideXvalue,
            strideYvalue,
            scales,
            outputs);
     }
     else
     {
         shaderExecutable = vxnneGetDepthwiseConvNoBiasShaderExecutable(ops_layer->node->base.context,
            VXNNE_KERNEL_DEPTHWISE_CONV,
            &ops_layer->node->kernelAttributes.borderMode,
            tensorCopy,
            weights,
            padXLeft,
            padXRight,
            padYTop,
            padYBottom,
            VX_PAD_CONSTANT,
            VX_NULL,
            dilationX,
            dilationY,
            depth_multiplier,
            VX_NULL,
            VX_NULL,
            VX_NULL,
            VX_NULL,
            downScaleSizeRounding,
            strideXvalue,
            strideYvalue,
            outputs);
     }

    if (!shaderExecutable)
    {
        status = VX_FAILURE;
        goto OnError;
    }

    vxmONERROR(vxnneShaderOperation_Initialize(&depthwiseConvolutionLayer->depthwise_convolution_sh_operation,
        &depthwiseConvolutionLayer->base,
        VXNNE_OPERATOR_DEPTHWISE_CONV,
        batchCount,
        shaderExecutable));

    if (batchCount > 1)
    {
        vxmONERROR(vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NO_BATCH_BIT));
        vxmONERROR(vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 2, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NO_BATCH_BIT));
    }

    vxmONERROR(vxnneOperation_AddReference(&depthwiseConvolutionLayer->depthwise_convolution_sh_operation.base, (vx_reference)tensorCopy, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&depthwiseConvolutionLayer->depthwise_convolution_sh_operation.base, (vx_reference)weights, VXNNE_OPERATION_REFENRENCE_INPUT));
    if (newBiases)
    {
        vxmONERROR(vxnneOperation_AddReference(&depthwiseConvolutionLayer->depthwise_convolution_sh_operation.base, (vx_reference)newBiases, VXNNE_OPERATION_REFENRENCE_INPUT));
    }
    vxmONERROR(vxnneOperation_AddReference(&depthwiseConvolutionLayer->depthwise_convolution_sh_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    vxmONERROR(vxnneLayer_SetOperation(
        &depthwiseConvolutionLayer->base,
        &depthwiseConvolutionLayer->depthwise_convolution_sh_operation.base,
        operation_idx++));

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

OnError:
    return status;
}
VX_PRIVATE_API vx_bool vxoNNDepthwiseConvLayer_SH_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    support = support && vxoNNDepthwiseConvLayer_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_false_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNDepthwiseConvLayer_SH_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxnne_depthwise_convolution_layer  depthwiseConvolutionLayer = (vxnne_depthwise_convolution_layer)ops_layer;

    vx_tensor inputs = (vx_tensor)parameters[0];
    vx_tensor weights = (vx_tensor)parameters[1];
    vx_tensor biases = (vx_tensor)parameters[2];
    vx_scalar padXLeft = (vx_scalar)parameters[3];
    vx_scalar padXRight = (vx_scalar)parameters[4];
    vx_scalar padYTop = (vx_scalar)parameters[5];
    vx_scalar padYBottom = (vx_scalar)parameters[6];
    vx_scalar dilationX = (vx_scalar)parameters[9];
    vx_scalar dilationY = (vx_scalar)parameters[10];
    vx_scalar strideX   = (vx_scalar)parameters[11];
    vx_scalar strideY   = (vx_scalar)parameters[12];
    vx_scalar depth_multiplier = (vx_scalar)parameters[13];
    vx_scalar downScaleSizeRounding = (vx_scalar)parameters[18];
    vx_tensor outputs = (vx_tensor)parameters[19];
    vx_tensor scales  = NULL;
    vx_enum  inputFormat = TENSOR_DATA_TYPE(inputs);
    /*vx_enum  biasFormat = VX_TYPE_INT32;*/
    vx_context context = vxGetContext((vx_reference)inputs);
    vx_uint32  operation_idx = 0;
    vx_uint32  numTmpTensor = 0;
    vx_int32   dilation_x  = dilationX->value->n32 + 1;
    vx_int32   dilation_y  = dilationY->value->n32 + 1;
    vx_uint32  batchCount  = TENSOR_SIZE_INDEX(inputs, 3);

    vxnne_shader_executable shaderExecutable = NULL;
    vx_tensor               newBiases = NULL;
    vx_tensor               tensorCopy = NULL;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    {
#define DWCONV_ALIGN_SIZE4 (4)
        vx_bool       enable_adjust_biases = vx_false_e;
        vx_int32      kernel_width = TENSOR_VIEW_SIZE_INDEX(weights, 0);
        vx_int32      kernel_height = TENSOR_VIEW_SIZE_INDEX(weights, 1);
        vx_bool       is_static_weights_biases = vx_false_e;
        vx_int32      padLeftv = padXLeft->value->n32;
        vx_int32      padRightv = padXRight->value->n32;
        vx_int32      padTopv = padYTop->value->n32;
        vx_int32      padBottomv = padYBottom->value->n32;
        vx_bool       is_copy_tensor = vx_false_e;
        vx_scalar     padLeft = VX_NULL;
        vx_scalar     padRight = VX_NULL;
        vx_scalar     padTop = VX_NULL;
        vx_scalar     padBottom = VX_NULL;
        vx_int32      strideXvalue = 0;
        vx_int32      strideYvalue = 0;
        vx_uint32     input_width = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
        vx_uint32     input_height = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
        vx_uint32     output_width = TENSOR_VIEW_SIZE_INDEX(outputs, 0);
        vx_uint32     output_height = TENSOR_VIEW_SIZE_INDEX(outputs, 1);

        if (strideX)
        {
            strideXvalue = strideX->value->n32;
        }
        else
        {
            if ((input_width == 1) || (output_width == 1))
            {
                strideXvalue = 1;
            }
            else
            {
                strideXvalue = vxoNNExternsionConvlutionRound((vx_float32)(input_width + padLeftv + padRightv - kernel_width) / (output_width - 1), downScaleSizeRounding->value->e);
            }
        }

        if (strideY)
        {
            strideYvalue = strideY->value->n32;
        }
        else
        {
            if ((input_height == 1) || (output_height == 1))
            {
                strideYvalue = 1;
            }
            else
            {
                strideYvalue = vxoNNExternsionConvlutionRound((vx_float32)(input_height + padTopv + padBottomv - kernel_height) / (output_height - 1), downScaleSizeRounding->value->e);
            }
        }

        if (TENSOR_QUANT_TYPE(weights) == VX_QUANT_AFFINE_SCALE_PER_CHANNEL)
        {
            vx_tensor_create_params_t params;
            vx_enum        input_quant      = TENSOR_QUANT_TYPE(inputs);
            vx_uint32      channelCount     = TENSOR_TF_SCALE_COUNT(weights);
            vx_uint32      sizes[2]         = {channelCount, 1};
            vx_float32_ptr scales_data_ptr  = VX_NULL;
            vx_enum        ouput_quant_type = TENSOR_QUANT_TYPE(outputs);
            vx_float32     input_scales     = 1;
            vx_uint32      idx              = 0;

            if (input_quant == VX_QUANT_AFFINE_SCALE)
                input_scales = TENSOR_TF_SCALE(inputs);

            scales_data_ptr = (vx_float32 *)vxAllocateAndZeroMemory(channelCount * sizeof(vx_float32));
            if (scales_data_ptr == VX_NULL)
            {
                status = VX_ERROR_NO_MEMORY;
                vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
                goto OnError;
            }
            gcoOS_MemCopy(scales_data_ptr, TENSOR_TF_SCALE_POINTER(weights), channelCount * sizeof(vx_float32));

            for (idx = 0; idx < channelCount; idx++)
            {
                scales_data_ptr[idx] *= input_scales;
                if (ouput_quant_type == VX_QUANT_AFFINE_SCALE)
                    scales_data_ptr[idx] /= TENSOR_TF_SCALE(outputs);
            }

            gcoOS_MemFill(&params, 0, sizeof(vx_tensor_create_params_t));

            params.num_of_dims = 2;
            params.sizes = sizes;
            params.data_format = VX_TYPE_FLOAT32;

            scales = _createTensorFromData(ops_layer->node->graph, params, (vx_uint8_ptr)scales_data_ptr);
            depthwiseConvolutionLayer->base.temp_tensors[numTmpTensor++] = scales;
            if (scales_data_ptr)
            {
                vxFree(scales_data_ptr);
                scales_data_ptr = VX_NULL;
            }
        }

        is_copy_tensor = ((inputFormat == VX_TYPE_UINT8) && (3 == kernel_width) && (3 == kernel_height) && (strideXvalue == 1 || strideXvalue == 2)
            && (padLeftv == 1 || padRightv == 1 || padTopv == 1 || padBottomv == 1));
        if (is_copy_tensor)
        {
            vx_tensor_create_params_t tensor_create_params;
            vx_uint32 sizes[] = { 1, 1, 1, 1 };
            /*vx_uint32 inputWidth   = 0;*/
            vx_uint32 copy_dims = TENSOR_DIM_NUM(inputs);
            gctPOINTER inputLogical = VX_NULL;
            vx_uint8   inputZP = (vx_uint8)TENSOR_TF_ZEROPOINT(inputs);
            vx_uint32  copy_size = 0;


            sizes[0] = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
            sizes[1] = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
            sizes[2] = copy_dims > 2 ? TENSOR_VIEW_SIZE_INDEX(inputs, 2) : 1;
            sizes[3] = copy_dims > 3 ? TENSOR_VIEW_SIZE_INDEX(inputs, 3) : 1;
            sizes[0] = sizes[0] + padLeftv + padRightv;
            /*inputWidth = sizes[0];*/
            sizes[1] = sizes[1] + padTopv + padBottomv;
            if (sizes[0] % DWCONV_ALIGN_SIZE4 != 0)
            {
                sizes[0] = gcmALIGN(sizes[0], DWCONV_ALIGN_SIZE4);
            }
            gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
            tensor_create_params.num_of_dims = copy_dims;
            tensor_create_params.sizes = sizes;
            tensor_create_params.data_format = TENSOR_DATA_TYPE(inputs);
            tensor_create_params.quant_format = TENSOR_QUANT_TYPE(inputs);
            if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
            {
                tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(inputs);
            }
            else
            {
                tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(inputs);
                tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(inputs);
            }
            tensorCopy = vxoTensor_CreateTensor(ops_layer->node->base.context, ops_layer->node->graph, &tensor_create_params, vx_false_e);
            if (tensorCopy == VX_NULL || vxoTensor_AllocateMemory(tensorCopy) != VX_SUCCESS)
            {
                vxError("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
                status = VX_ERROR_NO_MEMORY;
                goto OnError;
            }
            vxmONERROR(vxoTensor_GetTensorViewMemory(tensorCopy, &inputLogical, VX_NULL));
            copy_size = sizes[0] * sizes[1] * sizes[2] * sizes[3];
            gcoOS_MemFill(inputLogical, inputZP, copy_size);
            padLeftv = 0;
            padRightv = 0;
            padTopv = 0;
            padBottomv = 0;
            padLeft = vxCreateScalar(context, VX_TYPE_INT32, &padLeftv);
            padRight = vxCreateScalar(context, VX_TYPE_INT32, &padRightv);
            padTop = vxCreateScalar(context, VX_TYPE_INT32, &padTopv);
            padBottom = vxCreateScalar(context, VX_TYPE_INT32, &padBottomv);

            shaderExecutable = vxnneGPUTensorCopyROIShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_GPU_TENSOR_COPYROI, &ops_layer->node->kernelAttributes.borderMode,
                padLeft, padTop, padXLeft, padYTop, inputs, tensorCopy);
            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto OnError;
            }

            vxmONERROR(vxnneShaderOperation_Initialize(&depthwiseConvolutionLayer->depthwise_tensorcopy_sh_operation,
                &depthwiseConvolutionLayer->base,
                VXNNE_OPERATOR_DEPTHWISE_CONV,
                batchCount,
                shaderExecutable));

            vxmONERROR(vxnneLayer_SetOperation(
                &depthwiseConvolutionLayer->base,
                &depthwiseConvolutionLayer->depthwise_tensorcopy_sh_operation.base,
                operation_idx++));

            vxmONERROR(vxnneOperation_AddReference(&depthwiseConvolutionLayer->depthwise_tensorcopy_sh_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT));
            vxmONERROR(vxnneOperation_AddReference(&depthwiseConvolutionLayer->depthwise_tensorcopy_sh_operation.base, (vx_reference)tensorCopy, VXNNE_OPERATION_REFENRENCE_OUTPUT));

            depthwiseConvolutionLayer->base.temp_tensors[numTmpTensor++] = tensorCopy;
            depthwiseConvolutionLayer->base.num_temp_tensors = numTmpTensor;
        }
        else
        {
            tensorCopy = inputs;
            padLeft = vxCreateScalar(context, VX_TYPE_INT32, &padLeftv);
            padRight = vxCreateScalar(context, VX_TYPE_INT32, &padRightv);
            padTop = vxCreateScalar(context, VX_TYPE_INT32, &padTopv);
            padBottom = vxCreateScalar(context, VX_TYPE_INT32, &padBottomv);
        }

        if (biases)
        {
            is_static_weights_biases = (vx_bool)(CHECK_LIFETIME_IS_STATIC(weights) && CHECK_LIFETIME_IS_STATIC(biases));
        }
        else
        {
            is_static_weights_biases = (vx_bool)(CHECK_LIFETIME_IS_STATIC(weights));
        }

        enable_adjust_biases = (is_static_weights_biases && (inputFormat == VX_TYPE_UINT8) && (3 == kernel_width) && (3 == kernel_height));
        enable_adjust_biases = enable_adjust_biases && (1 == dilation_x) && (1 == dilation_y);

        if (enable_adjust_biases)
        {
            newBiases = _mergeInputZeroPoint2Bias(ops_layer->node->graph, inputs, weights, biases);
            depthwiseConvolutionLayer->base.temp_tensors[numTmpTensor++] = newBiases;
            depthwiseConvolutionLayer->base.num_temp_tensors = numTmpTensor;
        }
        else
        {
            newBiases = biases;
        }
        shaderExecutable = vxnneGetGPUDepthwiseConvShaderExecutable(ops_layer->node->base.context,
            VXNNE_KERNEL_GPU_DEPTHWISE_CONV,
            &ops_layer->node->kernelAttributes.borderMode,
            tensorCopy,
            weights,
            newBiases,
            padLeft,
            padRight,
            padTop,
            padBottom,
            dilationX,
            dilationY,
            depth_multiplier,
            downScaleSizeRounding,
            strideXvalue,
            strideYvalue,
            scales,
            outputs);

        if (padLeft)    vxReleaseScalar(&padLeft);
        if (padRight)   vxReleaseScalar(&padRight);
        if (padTop)     vxReleaseScalar(&padTop);
        if (padBottom)  vxReleaseScalar(&padBottom);
#undef DWCONV_ALIGN_SIZE4
    }

    if (!shaderExecutable)
    {
        status = VX_FAILURE;
        goto OnError;
    }

    vxmONERROR(vxnneShaderOperation_Initialize(&depthwiseConvolutionLayer->depthwise_convolution_sh_operation,
        &depthwiseConvolutionLayer->base,
        VXNNE_OPERATOR_DEPTHWISE_CONV,
        batchCount,
        shaderExecutable));

    if (batchCount > 1)
    {
        vxmONERROR(vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NO_BATCH_BIT));
        vxmONERROR(vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 2, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NO_BATCH_BIT));
    }

    vxmONERROR(vxnneOperation_AddReference(&depthwiseConvolutionLayer->depthwise_convolution_sh_operation.base, (vx_reference)tensorCopy, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&depthwiseConvolutionLayer->depthwise_convolution_sh_operation.base, (vx_reference)weights, VXNNE_OPERATION_REFENRENCE_INPUT));
    if (newBiases)
    {
        vxmONERROR(vxnneOperation_AddReference(&depthwiseConvolutionLayer->depthwise_convolution_sh_operation.base, (vx_reference)newBiases, VXNNE_OPERATION_REFENRENCE_INPUT));
    }
    vxmONERROR(vxnneOperation_AddReference(&depthwiseConvolutionLayer->depthwise_convolution_sh_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    vxmONERROR(vxnneLayer_SetOperation(
        &depthwiseConvolutionLayer->base,
        &depthwiseConvolutionLayer->depthwise_convolution_sh_operation.base,
        operation_idx++));

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);
OnError:
    return status;
}

VX_PRIVATE_API vx_bool vxoNNDepthwiseConvLayer_NN_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_tensor inputs = (vx_tensor)parameters[0];
    vx_tensor weights = (vx_tensor)parameters[1];
    vx_tensor biases = (vx_tensor)parameters[2];
    vx_scalar dilationX = (vx_scalar)parameters[9];
    vx_scalar dilationY = (vx_scalar)parameters[10];
    vx_scalar depth_multiplier = (vx_scalar)parameters[13];
    vx_tensor outputs = (vx_tensor)parameters[19];
    vx_uint32 origWeightX = TENSOR_SIZE_INDEX(weights, 0);
    vx_uint32 origWeightY = TENSOR_SIZE_INDEX(weights, 1);
    vx_scalar strideX = (vx_scalar)parameters[11];
    vx_scalar strideY = (vx_scalar)parameters[12];
    vx_uint32 weightX = (((origWeightX % strideX->value->u32 == 0) ? origWeightX : (origWeightX + (strideX->value->u32 - origWeightX % strideX->value->u32))) / strideX->value->u32);
    vx_uint32 weightY = (((origWeightY % strideY->value->u32 == 0) ? origWeightY : (origWeightY + (strideY->value->u32 - origWeightY % strideY->value->u32))) / strideY->value->u32);
    vx_context context = vxGetContext((vx_reference)inputs);
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_NN, VX_TYPE_INVALID, VX_NULL);
    vx_int32   dilation_x  = dilationX->value->n32 + 1;
    vx_int32   dilation_y  = dilationY->value->n32 + 1;

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    support = support && ((depth_multiplier != NULL) && (depth_multiplier->value->n32 > 0));
    support = support && (!(vx_bool)(TENSOR_QUANT_TYPE(weights) == VX_QUANT_AFFINE_SCALE_PER_CHANNEL));
    support = support && (1 == dilation_x) && (1 == dilation_y);

    if ((TENSOR_DATA_LIFETIME(weights) == VX_TENSOR_LIFE_TIME_STATIC) &&
        (biases == VX_NULL) &&
        vxnneIsNNSupportFormat(context, node->graph, inputs, VX_NULL, outputs))
    {
        support = support && vx_true_e;
    }
    else if ((TENSOR_DATA_LIFETIME(weights) == VX_TENSOR_LIFE_TIME_STATIC) &&
        vxnneIsNNSupportFormat(context, node->graph, inputs, VX_NULL, outputs) &&
        (TENSOR_DATA_LIFETIME(biases) == VX_TENSOR_LIFE_TIME_STATIC)
        )
    {
        support = support && vx_true_e;
    }
    else
        support = vx_false_e;

    /* Check if nn support fp16 */
    if (node->base.context->nnConfig.fixedFeature.nnCoreCountFloat16 == 0 &&
        (TENSOR_DATA_TYPE(inputs) == VX_TYPE_FLOAT16 || TENSOR_DATA_TYPE(weights) == VX_TYPE_FLOAT16 || TENSOR_DATA_TYPE(outputs) == VX_TYPE_FLOAT16))
        support = vx_false_e;
    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);
    if (weightX > context->nnConfig.fixedFeature.nnMaxKXSize || weightY > context->nnConfig.fixedFeature.nnMaxKYSize)
    {
        support = vx_false_e;
    }

    return support;
}

VX_PRIVATE_API vx_status vxoNNDepthwiseConvLayer_NN_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vxnne_depthwise_convolution_layer  depthwiseConvolutionLayer = (vxnne_depthwise_convolution_layer)ops_layer;

    vx_tensor inputs = (vx_tensor)parameters[0];
    vx_tensor weights = (vx_tensor)parameters[1];
    vx_tensor biases = (vx_tensor)parameters[2];
    vx_scalar padXLeft = (vx_scalar)parameters[3];
    vx_scalar padXRight = (vx_scalar)parameters[4];
    vx_scalar padYTop = (vx_scalar)parameters[5];
    vx_scalar padYBottom = (vx_scalar)parameters[6];
    vx_scalar dilationX = (vx_scalar)parameters[9];
    vx_scalar dilationY = (vx_scalar)parameters[10];
    vx_scalar strideX = (vx_scalar)parameters[11];
    vx_scalar strideY = (vx_scalar)parameters[12];
    vx_scalar depth_multiplier = (vx_scalar)parameters[13];
    vx_scalar downScaleSizeRounding = (vx_scalar)parameters[18];
    vx_tensor outputs = (vx_tensor)parameters[19];

    vx_int32 channels_multiplier = depth_multiplier->value->n32;
    vx_uint32 size[][4] = {
        { TENSOR_SIZE_INDEX(weights, 0), TENSOR_SIZE_INDEX(weights, 1), TENSOR_SIZE_INDEX(inputs, 2), TENSOR_SIZE_INDEX(weights, 2) },
    };

    vx_bool sw_convolution = vx_false_e;

    vx_tensor_create_params_t tensor_create_params;
    vx_tensor weight_dilation;

    vx_uint32  batchCount = TENSOR_SIZE_INDEX(inputs, 3);

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
    tensor_create_params.num_of_dims = TENSOR_DIM_NUM(weights);
    tensor_create_params.sizes = size[0];
    tensor_create_params.data_format = TENSOR_DATA_TYPE(weights);
    tensor_create_params.quant_format = TENSOR_QUANT_TYPE(weights);
    if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
    {
        tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(weights);
    }
    else
    {
        tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(weights);
        tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(weights);
    }

    weight_dilation = vxoTensor_CreateTensor(vxGetContext((vx_reference)ops_layer->node), ops_layer->node->graph, &tensor_create_params, vx_false_e);

    vxmONERROR(vxoNNSWDepthwiseConv_DilationWeight(weights, weight_dilation, channels_multiplier));

    if (sw_convolution)
    {
        /* Initialize covolution operation */
        vxmONERROR(vxnneOperation_Initialize(&depthwiseConvolutionLayer->convolution_nn_convolution_sw.base,
            &depthwiseConvolutionLayer->base,
            VXNNE_OPERATION_TARGET_SW,
            VXNNE_OPERATOR_CONVOLUTION,
            vxnneExecuteSWConvolution,
            vxnneExecuteSW_Depthwise_Convolution_DeInilition,
            batchCount,
            0));

        vxmONERROR(vxnneLayer_SetOperation(
            &depthwiseConvolutionLayer->base,
            &depthwiseConvolutionLayer->convolution_nn_convolution_sw.base,
            0));

        depthwiseConvolutionLayer->convolution_nn_convolution_sw.inputs = inputs;
        depthwiseConvolutionLayer->convolution_nn_convolution_sw.weights = weight_dilation;
        depthwiseConvolutionLayer->convolution_nn_convolution_sw.biases = biases;
        depthwiseConvolutionLayer->convolution_nn_convolution_sw.padX = padXLeft;
        depthwiseConvolutionLayer->convolution_nn_convolution_sw.padY = padYTop;
        depthwiseConvolutionLayer->convolution_nn_convolution_sw.downScaleSizeRounding = downScaleSizeRounding;
        depthwiseConvolutionLayer->convolution_nn_convolution_sw.outputs = outputs;

        vxmONERROR(vxnneOperation_AddReference(&depthwiseConvolutionLayer->convolution_nn_convolution_sw.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT));
        vxmONERROR(vxnneOperation_AddReference(&depthwiseConvolutionLayer->convolution_nn_convolution_sw.base, (vx_reference)weight_dilation, VXNNE_OPERATION_REFENRENCE_INPUT));
        vxmONERROR(vxnneOperation_AddReference(&depthwiseConvolutionLayer->convolution_nn_convolution_sw.base, (vx_reference)biases, VXNNE_OPERATION_REFENRENCE_INPUT));
        vxmONERROR(vxnneOperation_AddReference(&depthwiseConvolutionLayer->convolution_nn_convolution_sw.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    }
    else
    {
        vx_weights_biases_parameter_optimizations_t opt = { -1, TENSOR_DATA_TYPE(outputs), TENSOR_TF_ZEROPOINT(inputs) };
        vx_weights_biases_parameter weights_biases = vxoCreateWeightsBiasesParameterFromTensors(
            vxGetContext((vx_reference)ops_layer->node),
            VX_NN_CONVOLUTION_LAYER,
            inputs->dims,/*inputs_dims,*/
            inputs->dimCount,
            inputs->dimCount,
            padXLeft->value->n32,
            padXRight->value->n32,
            padYTop->value->n32,
            padYBottom->value->n32,
            0,/*pooling_size_x,*/
            0,/*pooling_size_y,*/
            strideX->value->n32,
            strideY->value->n32,
            VX_NN_DS_SIZE_ROUNDING_FLOOR,
            outputs->dims,/*convolution_outputs_dims,*/
            outputs->dims,/*pool_outputs_dims,*/
            &opt, /*optimizations,*/
            0,
            TENSOR_DATA_TYPE(weight_dilation),
            0,
            VX_TENSOR_RANK_WHCN,
            weight_dilation,
            biases,
            VX_NULL,
            VX_NN_CONV_ONLY,
            vx_false_e
        );

        vxmONERROR(vxnneConvolutionReluPoolingInitializer(ops_layer->node,
            "ConvolutionReluLayer",
            inputs,
            weights_biases,
            dilationX->value->n32,
            dilationY->value->n32,
            padXLeft->value->n32,
            padXRight->value->n32,
            padYTop->value->n32,
            padYBottom->value->n32,
            VX_NN_DS_SIZE_ROUNDING_FLOOR,
            vx_false_e,
            vx_false_e,
            VIV_NN_POOLING_NON,
            0,
            0,
            VX_PAD_CONSTANT,
            VX_NULL,
            VX_NULL,
            VX_NULL,
            VX_NULL,
            outputs));
    }
OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_status vxoNNLayer_GetOperations(vxnne_layer ops_layer, vx_uint32_ptr max_num_operations, vxnne_operation **operations)
{
    vx_status  status = VX_SUCCESS;
    vxnne_depthwise_convolution_layer  depthwiseconvLayer = (vxnne_depthwise_convolution_layer)ops_layer;

    *max_num_operations = gcmCOUNTOF(depthwiseconvLayer->operations);

    *operations = depthwiseconvLayer->operations;

    return status;
}

vx_status vxoNNDepthwiseConvolutionLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vxnne_layer_imp_s registerDepthwiseConvLayers[] = {/* Please DON'T adjust the order, it's importent */
        { "DepthwiseConvLayer NN", vxoNNDepthwiseConvLayer_NN_Support, vxoNNDepthwiseConvLayer_NN_Initialize, VX_NULL },
        { "DepthwiseConvLayer TP", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "DepthwiseConvLayer SH EVIS", vxoNNDepthwiseConvLayer_SH_EVIS_Support, vxoNNDepthwiseConvLayer_SH_EVIS_Initialize, VX_NULL },
        { "DepthwiseConvLayer SH F32", vxoNNDepthwiseConvLayer_SH_Support, vxoNNDepthwiseConvLayer_SH_Initialize, VX_NULL },
        { "DepthwiseConvLayer SW", vxoNNCommon_Support, vxoNNDepthwiseConvLayer_SW_Initialize, VX_NULL },
    };

    REGISTER_LAYERS(registerDepthwiseConvLayers, vxnne_depthwise_convolution_layer_s, "DepthwiseConvLayer", vxoNNLayer_GetOperations);

OnError:
    return status;
}

vx_status VX_CALLBACK vxoNNDepthwiseConvolutionLayerInitializer(vx_node node,
        vx_tensor inputs,
        vx_weights_biases_parameter weights_biases,
        vx_tensor weights,
        vx_tensor biases,
        vx_scalar padXLeft,
        vx_scalar padXRight,
        vx_scalar padYTop,
        vx_scalar padYBottom,
        vx_enum   padMode,
        vx_scalar padConstant,
        vx_scalar dilationX,
        vx_scalar dilationY,
        vx_scalar strideX,
        vx_scalar strideY,
        vx_scalar depth_multiplier,
        vx_scalar relu_s,
        vx_scalar pooling_s,
        vx_scalar poolingX,
        vx_scalar poolingY,
        vx_scalar downScaleSizeRounding,
        vx_tensor outputs)
{
    vx_status status = VX_SUCCESS;
    vx_reference params[] = {
        (vx_reference)inputs, (vx_reference)weights, (vx_reference)biases,
        (vx_reference)padXLeft, (vx_reference)padXRight, (vx_reference)padYTop, (vx_reference)padYBottom,
        (vx_reference)vxCreateScalar(node->base.context, VX_TYPE_ENUM, &padMode), (vx_reference)padConstant, (vx_reference)dilationX, (vx_reference)dilationY,
        (vx_reference)strideX, (vx_reference)strideY, (vx_reference)depth_multiplier,
        (vx_reference)relu_s, (vx_reference)pooling_s, (vx_reference)poolingX, (vx_reference)poolingY,
        (vx_reference)downScaleSizeRounding, (vx_reference)outputs
    };

    status = vxoNNDepthwiseConvolutionLayer_Initializer(node, params, gcmCOUNTOF(params));

    vxReleaseScalar((vx_scalar*)&params[7]);
    return status;
}
#else


vx_status VX_CALLBACK vxoNNDepthwiseConvolutionLayerInitializer(vx_node node,
        vx_tensor inputs,
        vx_weights_biases_parameter weights_biases,
        vx_tensor weights,
        vx_tensor biases,
        vx_scalar padXLeft,
        vx_scalar padXRight,
        vx_scalar padYTop,
        vx_scalar padYBottom,
        vx_enum   padMode,
        vx_scalar padConstant,
        vx_scalar dilationX,
        vx_scalar dilationY,
        vx_scalar strideX,
        vx_scalar strideY,
        vx_scalar depth_multiplier,
        vx_scalar relu_s,
        vx_scalar pooling_s,
        vx_scalar poolingX,
        vx_scalar poolingY,
        vx_scalar downScaleSizeRounding,
        vx_tensor outputs)
{

    vx_status status = VX_SUCCESS;

    vx_uint32  batchCount                 = TENSOR_SIZE_INDEX(inputs, 3);

    vxnne_depthwise_convolution_layer  depthwiseConvolutionLayer         = VX_NULL;

    vx_bool enable_nne = vx_false_e;
    vx_context context = vxGetContext((vx_reference)inputs);
    vx_uint32  operation_idx = 0;
    vx_uint32  numTmpTensor = 0;
    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }
    gcoOS_Allocate(gcvNULL, sizeof(vxnne_depthwise_convolution_layer_s), (gctPOINTER*)&depthwiseConvolutionLayer);
    if (!depthwiseConvolutionLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(depthwiseConvolutionLayer, sizeof(vxnne_depthwise_convolution_layer_s));

    vxnneLayer_Initialize(&depthwiseConvolutionLayer->base,
                          "DepthwiseConvolutionLayer",
                          node,
                          vxmOPERATION_COUNT(depthwiseConvolutionLayer),
                          depthwiseConvolutionLayer->operations,
                          VX_NULL);

    if ((TENSOR_DATA_LIFETIME(weights) == VX_TENSOR_LIFE_TIME_STATIC) &&
        (biases == VX_NULL) &&
        vxnneIsNNSupportFormat(context, node->graph, inputs, VX_NULL, outputs))
    {
        enable_nne = vx_true_e;
    }
    else if ((TENSOR_DATA_LIFETIME(weights) == VX_TENSOR_LIFE_TIME_STATIC) &&
        (TENSOR_DATA_LIFETIME(biases) == VX_TENSOR_LIFE_TIME_STATIC) &&
        vxnneIsNNSupportFormat(context, node->graph, inputs, VX_NULL, outputs))
    {
        enable_nne = vx_true_e;
    }

    /* Check if nn support fp16 */
    if (node->base.context->nnConfig.fixedFeature.nnCoreCountFloat16 == 0 &&
        (TENSOR_DATA_TYPE(inputs) == VX_TYPE_FLOAT16 || TENSOR_DATA_TYPE(weights) == VX_TYPE_FLOAT16 || TENSOR_DATA_TYPE(outputs) == VX_TYPE_FLOAT16))
        enable_nne = vx_false_e;

    if (enable_nne)
    {
        {
            vx_int32 channels_multiplier = depth_multiplier->value->n32;
            vx_uint32 size[][4] = {
                {TENSOR_SIZE_INDEX(weights, 0), TENSOR_SIZE_INDEX(weights, 1), TENSOR_SIZE_INDEX(inputs, 2), TENSOR_SIZE_INDEX(weights, 2)},
            };

            vx_bool sw_convolution = vx_false_e;

            vx_tensor_create_params_t tensor_create_params;
            vx_tensor weight_dilation;

            gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
            tensor_create_params.num_of_dims = TENSOR_DIM_NUM(weights);
            tensor_create_params.sizes = size[0];
            tensor_create_params.data_format = TENSOR_DATA_TYPE(weights);
            tensor_create_params.quant_format = TENSOR_QUANT_TYPE(weights);
            if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
            {
                tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(weights);
            }
            else
            {
                tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(weights);
                tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(weights);
            }

            weight_dilation = vxoTensor_CreateTensor(vxGetContext((vx_reference)node), node->graph, &tensor_create_params, vx_false_e);

            vxoNNSWDepthwiseConv_DilationWeight(weights, weight_dilation, channels_multiplier);

            if (sw_convolution)
            {
                vx_enum downScaleSizeRounding = VX_NN_DS_SIZE_ROUNDING_FLOOR;
                vx_scalar down_scale_s = vxCreateScalar(vxGetContext((vx_reference)node), VX_TYPE_ENUM, &downScaleSizeRounding);

                /* Initialize covolution operation */
                vxnneOperation_Initialize(&depthwiseConvolutionLayer->convolution_nn_convolution_sw.base,
                                          &depthwiseConvolutionLayer->base,
                                          VXNNE_OPERATION_TARGET_SW,
                                          VXNNE_OPERATOR_CONVOLUTION,
                                          vxnneExecuteSWConvolution,
                                          vxnneExecuteSW_Depthwise_Convolution_DeInilition,
                                          batchCount,
                                          0);

                vxnneLayer_SetOperation(
                    &depthwiseConvolutionLayer->base,
                    &depthwiseConvolutionLayer->convolution_nn_convolution_sw.base,
                    0);

                depthwiseConvolutionLayer->convolution_nn_convolution_sw.inputs                  = inputs;
                depthwiseConvolutionLayer->convolution_nn_convolution_sw.weights                 = weight_dilation;
                depthwiseConvolutionLayer->convolution_nn_convolution_sw.biases                  = biases;
                depthwiseConvolutionLayer->convolution_nn_convolution_sw.padX                    = padXLeft;
                depthwiseConvolutionLayer->convolution_nn_convolution_sw.padY                    = padYTop;
                depthwiseConvolutionLayer->convolution_nn_convolution_sw.downScaleSizeRounding   = down_scale_s;
                depthwiseConvolutionLayer->convolution_nn_convolution_sw.outputs                 = outputs;

                vxnneOperation_AddReference(&depthwiseConvolutionLayer->convolution_nn_convolution_sw.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&depthwiseConvolutionLayer->convolution_nn_convolution_sw.base, (vx_reference)weight_dilation, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&depthwiseConvolutionLayer->convolution_nn_convolution_sw.base, (vx_reference)biases, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&depthwiseConvolutionLayer->convolution_nn_convolution_sw.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            }
            else
            {
                vx_weights_biases_parameter_optimizations_t opt = { -1, TENSOR_DATA_TYPE(outputs), TENSOR_TF_ZEROPOINT(inputs)};
                vx_weights_biases_parameter weights_biases = vxoCreateWeightsBiasesParameterFromTensors(
                                    vxGetContext((vx_reference)node),
                                    VX_NN_CONVOLUTION_LAYER,
                                    inputs->dims,/*inputs_dims,*/
                                    inputs->dimCount,
                                    inputs->dimCount,
                                    padXLeft->value->n32,
                                    padXRight->value->n32,
                                    padYTop->value->n32,
                                    padYBottom->value->n32,
                                    0,/*pooling_size_x,*/
                                    0,/*pooling_size_y,*/
                                    strideX->value->n32,
                                    strideY->value->n32,
                                    VX_NN_DS_SIZE_ROUNDING_FLOOR,
                                    outputs->dims,/*convolution_outputs_dims,*/
                                    outputs->dims,/*pool_outputs_dims,*/
                                    &opt, /*optimizations,*/
                                    TENSOR_DATA_TYPE(weight_dilation),
                                    0,
                                    VX_TENSOR_RANK_WHCN,
                                    weight_dilation,
                                    biases,
                                    VX_NULL,
                                    vx_false_e,
                                    vx_false_e
                                    );

                status = vxnneConvolutionReluPoolingInitializer(node,
                    "ConvolutionReluLayer",
                    inputs,
                    weights_biases,
                    dilationX->value->n32,
                    dilationY->value->n32,
                    padXLeft->value->n32,
                    padXRight->value->n32,
                    padYTop->value->n32,
                    padYBottom->value->n32,
                    VX_NN_DS_SIZE_ROUNDING_FLOOR,
                    vx_false_e,
                    vx_false_e,
                    VIV_NN_POOLING_NON,
                    0,
                    0,
                    VX_PAD_CONSTANT,
                    VX_NULL,
                    outputs);

                goto exit;
            }
        }
    }
    else
    {
        vx_enum  inputFormat                       = TENSOR_DATA_TYPE(inputs);
        vx_enum  weightFormat                      = TENSOR_DATA_TYPE(weights);
        vx_enum  biasFormat                        = VX_TYPE_INT32;
        vx_enum  outputFormat                      = TENSOR_DATA_TYPE(outputs);
        vx_bool  dataformat_flag[5]                = {vx_false_e};
        vx_bool  depthwiseConv_shader_flag         = vx_false_e;
        vx_bool  is_w_sym_per_channel_quant        = (vx_bool)(TENSOR_QUANT_TYPE(weights) == VX_QUANT_AFFINE_SCALE_PER_CHANNEL);
        vx_bool  is_b_sym_per_channel_quant        = biases == NULL? vx_false_e : (vx_bool)(TENSOR_QUANT_TYPE(weights) == VX_QUANT_AFFINE_SCALE_PER_CHANNEL);
        vx_bool  is_sym_per_channel_quant          = is_w_sym_per_channel_quant && is_b_sym_per_channel_quant;

        if (biases != VX_NULL)
            biasFormat = TENSOR_DATA_TYPE(biases);

        if(context->evisNoInst.supportEVIS)
        {
            dataformat_flag[0]        = (vx_bool)(inputFormat == VX_TYPE_FLOAT16 && weightFormat == VX_TYPE_FLOAT16 && ((biasFormat == VX_TYPE_INVALID || biasFormat == VX_TYPE_FLOAT32) && outputFormat == VX_TYPE_FLOAT16);
            dataformat_flag[3]        = (vx_bool)(inputFormat == VX_TYPE_INT16 && weightFormat == VX_TYPE_INT16
                                                && ((biasFormat == VX_TYPE_INVALID || biasFormat == VX_TYPE_INT32 || biasFormat == VX_TYPE_INT64)
                                                && outputFormat == VX_TYPE_INT16);
            dataformat_flag[2]        = (vx_bool)(inputFormat == VX_TYPE_INT8 && weightFormat == VX_TYPE_INT8 && ((biasFormat == VX_TYPE_INVALID || biasFormat == VX_TYPE_INT32) && outputFormat == VX_TYPE_INT8);
        }
        else
        {
            dataformat_flag[0]        = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && weightFormat == VX_TYPE_FLOAT16 && (biasFormat == VX_TYPE_INVALID || biasFormat == VX_TYPE_FLOAT32) && outputFormat == VX_TYPE_FLOAT16) ||
                                                  (inputFormat == VX_TYPE_FLOAT16 && weightFormat == VX_TYPE_FLOAT16 && (biasFormat == VX_TYPE_INVALID || biasFormat == VX_TYPE_FLOAT16) && outputFormat == VX_TYPE_FLOAT16) ||
                                                  (inputFormat == VX_TYPE_FLOAT32 && weightFormat == VX_TYPE_FLOAT32 && (biasFormat == VX_TYPE_INVALID || biasFormat == VX_TYPE_FLOAT32) && outputFormat == VX_TYPE_FLOAT32));
            dataformat_flag[3]        = (vx_bool)(inputFormat == VX_TYPE_INT16 && weightFormat == VX_TYPE_INT16 && (biasFormat == VX_TYPE_INVALID || biasFormat == VX_TYPE_INT32) && outputFormat == VX_TYPE_INT16);
        }
        dataformat_flag[1]        = (vx_bool)(inputFormat == VX_TYPE_UINT8 && weightFormat == VX_TYPE_UINT8 && (biasFormat == VX_TYPE_INVALID || biasFormat == VX_TYPE_INT32) && outputFormat == VX_TYPE_UINT8);
        dataformat_flag[4]        = (vx_bool)(inputFormat == VX_TYPE_UINT8 && weightFormat == VX_TYPE_INT8 && biasFormat == VX_TYPE_INT32 && outputFormat == VX_TYPE_UINT8 && is_sym_per_channel_quant);
        depthwiseConv_shader_flag = (vx_bool) (dataformat_flag[0] || dataformat_flag[1] || dataformat_flag[2] || dataformat_flag[3] || dataformat_flag[4]);
        if (depthwiseConv_shader_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
        {
            vxnne_shader_executable shaderExecutable = NULL;
            vx_tensor               newBiases        = NULL;
            vx_tensor               tensorCopy       = NULL;
            vx_tensor               scales        = NULL;

            if (TENSOR_QUANT_TYPE(weights) == VX_QUANT_AFFINE_SCALE_PER_CHANNEL)
            {
                vx_tensor_create_params_t params;
                vx_enum        input_quant      = TENSOR_QUANT_TYPE(inputs);
                vx_uint32      channelCount     = TENSOR_TF_SCALE_COUNT(weights);
                vx_uint32      sizes[2]         = {channelCount, 1};
                vx_float32_ptr scales_data_ptr  = VX_NULL;
                vx_enum        ouput_quant_type = TENSOR_QUANT_TYPE(outputs);
                vx_float32     input_scales     = 1;
                vx_uint32      idx              = 0;

                if (input_quant == VX_QUANT_AFFINE_SCALE)
                    input_scales = TENSOR_TF_SCALE(inputs);

                scales_data_ptr = (vx_float32 *)vxAllocateAndZeroMemory(channelCount * sizeof(vx_float32));
                if (scales_data_ptr == VX_NULL)
                {
                    status = VX_ERROR_NO_MEMORY;
                    vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
                    goto OnError;
                }
                gcoOS_MemCopy(scales_data_ptr, TENSOR_TF_SCALE_POINTER(weights), channelCount * sizeof(vx_float32));

                for (idx = 0; idx < channelCount; idx++)
                {
                    scales_data_ptr[idx] *= input_scales;
                    if (ouput_quant_type == VX_QUANT_AFFINE_SCALE)
                        scales_data_ptr[idx] /= TENSOR_TF_SCALE(outputs);
                }

                gcoOS_MemFill(&params, 0, sizeof(vx_tensor_create_params_t));

                params.num_of_dims = 2;
                params.sizes = sizes;
                params.data_format = VX_TYPE_FLOAT32;

                scales = _createTensorFromData(ops_layer->node->graph, params, (vx_uint8_ptr)scales_data_ptr);
                depthwiseConvolutionLayer->base.temp_tensors[numTmpTensor++] = scales;
                if (scales_data_ptr)
                {
                    vxFree(scales_data_ptr);
                    scales_data_ptr = VX_NULL;
                }
            }

            if(node->base.context->evisNoInst.supportEVIS)
            {
                vx_int32      kernel_width                   = TENSOR_VIEW_SIZE_INDEX(weights, 0);
                vx_int32      kernel_height                  = TENSOR_VIEW_SIZE_INDEX(weights, 1);
                vx_int32      padLeftv                       = padXLeft->value->n32;
                vx_int32      padRightv                      = padXRight->value->n32;
                vx_int32      padTopv                        = padYTop->value->n32;
                vx_int32      padBottomv                     = padYBottom->value->n32;
                vx_int32      strideXvalue                   = 0;
                vx_int32      strideYvalue                   = 0;
                vx_uint32     input_width                    = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
                vx_uint32     input_height                   = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
                vx_uint32     output_width                   = TENSOR_VIEW_SIZE_INDEX(outputs, 0);
                vx_uint32     output_height                  = TENSOR_VIEW_SIZE_INDEX(outputs, 1);

                if (strideX)
                {
                    strideXvalue = strideX->value->n32;
                }
                else
                {
                    if ((input_width == 1) || (output_width == 1))
                    {
                        strideXvalue = 1;
                    }
                    else
                    {
                        strideXvalue = vxoNNExternsionConvlutionRound((vx_float32)(input_width + padLeftv + padRightv - kernel_width) / (output_width - 1), downScaleSizeRounding->value->e);
                    }
                }

                if (strideY)
                {
                    strideYvalue = strideY->value->n32;
                }
                else
                {
                    if ((input_height == 1) || (output_height == 1))
                    {
                        strideYvalue = 1;
                    }
                    else
                    {
                        strideYvalue = vxoNNExternsionConvlutionRound((vx_float32)(input_height + padTopv + padBottomv - kernel_height) / (output_height - 1), downScaleSizeRounding->value->e);
                    }
                }

                newBiases  = biases;
                tensorCopy = inputs;
                shaderExecutable = vxnneGetDepthwiseConvShaderExecutable(node->base.context,
                                                                         VXNNE_KERNEL_DEPTHWISE_CONV,
                                                                         &node->kernelAttributes.borderMode,
                                                                         tensorCopy,
                                                                         weights,
                                                                         newBiases,
                                                                         padXLeft,
                                                                         padXRight,
                                                                         padYTop,
                                                                         padYBottom,
                                                                         padMode,
                                                                         padConstant,
                                                                         dilationX,
                                                                         dilationY,
                                                                         depth_multiplier,
                                                                         relu_s,
                                                                         pooling_s,
                                                                         poolingX,
                                                                         poolingY,
                                                                         downScaleSizeRounding,
                                                                         strideXvalue,
                                                                         strideYvalue,
                                                                         scales,
                                                                         outputs);
            }
            else
            {
            #define DWCONV_ALIGN_SIZE4 (4)
                vx_bool       enable_adjust_biases           = vx_false_e;
                vx_int32      kernel_width                   = TENSOR_VIEW_SIZE_INDEX(weights, 0);
                vx_int32      kernel_height                  = TENSOR_VIEW_SIZE_INDEX(weights, 1);
                vx_bool       is_static_weights_biases       = vx_false_e;
                vx_int32      padLeftv                       = padXLeft->value->n32;
                vx_int32      padRightv                      = padXRight->value->n32;
                vx_int32      padTopv                        = padYTop->value->n32;
                vx_int32      padBottomv                     = padYBottom->value->n32;
                vx_bool       is_copy_tensor                 = vx_false_e;
                vx_scalar     padLeft                        = VX_NULL;
                vx_scalar     padRight                       = VX_NULL;
                vx_scalar     padTop                         = VX_NULL;
                vx_scalar     padBottom                      = VX_NULL;
                vx_int32      strideXvalue                   = 0;
                vx_int32      strideYvalue                   = 0;
                vx_uint32     input_width                    = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
                vx_uint32     input_height                   = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
                vx_uint32     output_width                   = TENSOR_VIEW_SIZE_INDEX(outputs, 0);
                vx_uint32     output_height                  = TENSOR_VIEW_SIZE_INDEX(outputs, 1);

                if (strideX)
                {
                    strideXvalue = strideX->value->n32;
                }
                else
                {
                    if ((input_width == 1) || (output_width == 1))
                    {
                        strideXvalue = 1;
                    }
                    else
                    {
                        strideXvalue = vxoNNExternsionConvlutionRound((vx_float32)(input_width + padLeftv + padRightv - kernel_width) / (output_width - 1), downScaleSizeRounding->value->e);
                    }
                }

                if (strideY)
                {
                    strideYvalue = strideY->value->n32;
                }
                else
                {
                    if ((input_height == 1) || (output_height == 1))
                    {
                        strideYvalue = 1;
                    }
                    else
                    {
                        strideYvalue = vxoNNExternsionConvlutionRound((vx_float32)(input_height + padTopv + padBottomv - kernel_height) / (output_height - 1), downScaleSizeRounding->value->e);
                    }
                }

                is_copy_tensor = ((inputFormat == VX_TYPE_UINT8) && (3 == kernel_width) && (3 == kernel_height) && (strideXvalue == 1 || strideXvalue == 2)
                                 && (padLeftv == 1 || padRightv == 1 || padTopv == 1 || padBottomv == 1));
                if (is_copy_tensor)
                {
                    vx_tensor_create_params_t tensor_create_params;
                    vx_uint32 sizes[]       = {1, 1, 1, 1};
                    vx_uint32 inputWidth   = 0;
                    vx_uint32 copy_dims     = TENSOR_DIM_NUM(inputs);
                    gctPOINTER inputLogical = VX_NULL;
                    vx_uint8   inputZP      = (vx_uint8)TENSOR_TF_ZEROPOINT(inputs);
                    vx_uint32  copy_size     = 0;


                    sizes[0] = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
                    sizes[1] = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
                    sizes[2] = copy_dims > 2 ? TENSOR_VIEW_SIZE_INDEX(inputs, 2) : 1;
                    sizes[3] = copy_dims > 3 ? TENSOR_VIEW_SIZE_INDEX(inputs, 3) : 1;
                    sizes[0] = sizes[0] + padLeftv + padRightv;
                    inputWidth = sizes[0];
                    sizes[1] = sizes[1] + padTopv + padBottomv;
                    if (sizes[0] % DWCONV_ALIGN_SIZE4 != 0)
                    {
                         sizes[0] = gcmALIGN(sizes[0], DWCONV_ALIGN_SIZE4);
                    }
                    gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
                    tensor_create_params.num_of_dims = copy_dims;
                    tensor_create_params.sizes = sizes;
                    tensor_create_params.data_format = TENSOR_DATA_TYPE(inputs);
                    tensor_create_params.quant_format = TENSOR_QUANT_TYPE(inputs);
                    if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
                    {
                        tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(inputs);
                    }
                    else
                    {
                        tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(inputs);
                        tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(inputs);
                    }
                    tensorCopy = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_false_e);
                    if (tensorCopy == VX_NULL || vxoTensor_AllocateMemory(tensorCopy) != VX_SUCCESS)
                    {
                        vxError("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
                        status = VX_ERROR_NO_MEMORY;
                        goto exit;
                    }
                    vxoTensor_GetTensorViewMemory(tensorCopy, &inputLogical, VX_NULL);
                    copy_size = sizes[0] * sizes[1] * sizes[2] * sizes[3];
                    gcoOS_MemFill(inputLogical, inputZP, copy_size);
                    padLeftv   = 0;
                    padRightv  = 0;
                    padTopv    = 0;
                    padBottomv = 0;
                    padLeft   = vxCreateScalar(context, VX_TYPE_INT32, &padLeftv);
                    padRight  = vxCreateScalar(context, VX_TYPE_INT32, &padRightv);
                    padTop    = vxCreateScalar(context, VX_TYPE_INT32, &padTopv);
                    padBottom = vxCreateScalar(context, VX_TYPE_INT32, &padBottomv);

                    shaderExecutable = vxnneGPUTensorCopyROIShaderExecutable(node->base.context, VXNNE_KERNEL_GPU_TENSOR_COPYROI, &node->kernelAttributes.borderMode,
                                                         padLeft, padTop, padXLeft, padYTop, inputs, tensorCopy);
                    if (!shaderExecutable)
                    {
                        status = VX_FAILURE;
                        goto exit;
                    }

                    status = vxnneShaderOperation_Initialize(&depthwiseConvolutionLayer->depthwise_tensorcopy_sh_operation,
                        &depthwiseConvolutionLayer->base,
                        VXNNE_OPERATOR_DEPTHWISE_CONV,
                        batchCount,
                        shaderExecutable);

                    if (status != VX_SUCCESS) goto exit;

                    vxnneLayer_SetOperation(
                        &depthwiseConvolutionLayer->base,
                        &depthwiseConvolutionLayer->depthwise_tensorcopy_sh_operation.base,
                        operation_idx++);

                    vxnneOperation_AddReference(&depthwiseConvolutionLayer->depthwise_tensorcopy_sh_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
                    vxnneOperation_AddReference(&depthwiseConvolutionLayer->depthwise_tensorcopy_sh_operation.base, (vx_reference)tensorCopy, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                    depthwiseConvolutionLayer->base.temp_tensors[numTmpTensor++] = tensorCopy;
                    depthwiseConvolutionLayer->base.num_temp_tensors = numTmpTensor;
                }
                else
                {
                    tensorCopy = inputs;
                    padLeft   = vxCreateScalar(context, VX_TYPE_INT32, &padLeftv);
                    padRight  = vxCreateScalar(context, VX_TYPE_INT32, &padRightv);
                    padTop    = vxCreateScalar(context, VX_TYPE_INT32, &padTopv);
                    padBottom = vxCreateScalar(context, VX_TYPE_INT32, &padBottomv);
                }

                if (biases)
                {
                    is_static_weights_biases = (vx_bool)(CHECK_LIFETIME_IS_STATIC(weights) && CHECK_LIFETIME_IS_STATIC(biases));
                }
                else
                {
                    is_static_weights_biases = (vx_bool)(CHECK_LIFETIME_IS_STATIC(weights));
                }
                enable_adjust_biases = (is_static_weights_biases && (inputFormat == VX_TYPE_UINT8) && (3 == kernel_width) && (3 == kernel_height));

                if (enable_adjust_biases)
                {
                    newBiases = _mergeInputZeroPoint2Bias(node->graph, inputs, weights, biases);
                    depthwiseConvolutionLayer->base.temp_tensors[numTmpTensor++] = newBiases;
                    depthwiseConvolutionLayer->base.num_temp_tensors = numTmpTensor;
                }
                else
                {
                    newBiases = biases;
                }
                shaderExecutable = vxnneGetGPUDepthwiseConvShaderExecutable(node->base.context,
                                                                             VXNNE_KERNEL_GPU_DEPTHWISE_CONV,
                                                                             &node->kernelAttributes.borderMode,
                                                                             tensorCopy,
                                                                             weights,
                                                                             newBiases,
                                                                             padLeft,
                                                                             padRight,
                                                                             padTop,
                                                                             padBottom,
                                                                             dilationX,
                                                                             dilationY,
                                                                             depth_multiplier,
                                                                             downScaleSizeRounding,
                                                                             strideXvalue,
                                                                             strideYvalue,
                                                                             scales,
                                                                             outputs);

                if(padLeft)    vxReleaseScalar(&padLeft);
                if(padRight)   vxReleaseScalar(&padRight);
                if(padTop)     vxReleaseScalar(&padTop);
                if(padBottom)  vxReleaseScalar(&padBottom);
            #undef DWCONV_ALIGN_SIZE4
            }

            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto exit;
            }

            status = vxnneShaderOperation_Initialize(&depthwiseConvolutionLayer->depthwise_convolution_sh_operation,
                                                     &depthwiseConvolutionLayer->base,
                                                     VXNNE_OPERATOR_DEPTHWISE_CONV,
                                                     batchCount,
                                                     shaderExecutable);
            if (status != VX_SUCCESS) goto exit;

            if (batchCount > 1)
            {
                vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NO_BATCH_BIT);
                vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 2, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NO_BATCH_BIT);
            }

            vxnneOperation_AddReference(&depthwiseConvolutionLayer->depthwise_convolution_sh_operation.base, (vx_reference)tensorCopy, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&depthwiseConvolutionLayer->depthwise_convolution_sh_operation.base, (vx_reference)weights, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&depthwiseConvolutionLayer->depthwise_convolution_sh_operation.base, (vx_reference)newBiases, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&depthwiseConvolutionLayer->depthwise_convolution_sh_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            vxnneLayer_SetOperation(
                &depthwiseConvolutionLayer->base,
                &depthwiseConvolutionLayer->depthwise_convolution_sh_operation.base,
                operation_idx++);
        }
        else
        {
            vxnneOperation_Initialize(&depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.base,
                                      &depthwiseConvolutionLayer->base,
                                      VXNNE_OPERATION_TARGET_SW,
                                      VXNNE_OPERATOR_DEPTHWISE_CONV,
                                      vxoNNSWDepthwiseConvolution,
                                      VX_NULL,
                                      batchCount,
                                      0);

            vxnneLayer_SetOperation(
                &depthwiseConvolutionLayer->base,
                &depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.base,
                0);

            depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.inputs                = inputs;
            depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.weights               = weights;
            depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.biases                = biases;
            depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.padXLeft              = padXLeft;
            depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.padXRight             = padXRight;
            depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.padYTop               = padYTop;
            depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.padYBottom            = padYBottom;
            depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.dilationX             = dilationX;
            depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.dilationY             = dilationY;
            depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.depth_multiplier      = depth_multiplier;
            depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.downScaleSizeRounding = downScaleSizeRounding;
            depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.stride_x              = strideX;
            depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.stride_y              = strideY;
            depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.outputs               = outputs;

            vxnneOperation_AddReference(&depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.base, (vx_reference)weights, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.base, (vx_reference)biases, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&depthwiseConvolutionLayer->convolution_sw1_depthwise_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }
    }

    node->layer = &depthwiseConvolutionLayer->base;
    return status;

exit:
    if (depthwiseConvolutionLayer != NULL) gcoOS_Free(NULL, depthwiseConvolutionLayer);
    return status;
}
#endif



