/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <VX/vx.h>
#ifdef WIN32
#include <dirent_win.h>
#else
#include <dirent.h>
#endif

#include <gc_vx_common.h>
#include <gc_vx_nn_extension_interface.h>
#include <gc_vx_internal_node_api.h>
#include <gc_vx_nn_util.h>
#include "gc_hal_types.h"
#include "anchors.h"

#define TP_FC_DUMP_WEIGHTS_BIASES   0
#define TP_FC_DUMP_INPUTS           0

#if TP_FC_DUMP_WEIGHTS_BIASES || TP_FC_DUMP_INPUTS
#include <stdio.h>
#endif

VX_PRIVATE_API vx_status vxnneGetTensorMemeory(vx_tensor tensor, vx_ptr_ptr ptr, vx_bool stage, vx_bool zero);

VX_API_ENTRY vx_status VX_API_CALL vxReleaseWeightsBiasesParameter(vx_weights_biases_parameter *weights_bias)
{
    if ((*weights_bias) == VX_NULL) return VX_ERROR_INVALID_PARAMETERS;

    if ((*weights_bias)->weights_sizes)
    {
        vxFree((*weights_bias)->weights_sizes);
        (*weights_bias)->weights_sizes = VX_NULL;
    }
    if ((*weights_bias)->org_weights_sizes)
    {
        vxFree((*weights_bias)->org_weights_sizes);
        (*weights_bias)->org_weights_sizes = VX_NULL;
    }
    if ((*weights_bias)->biases_sizes)
    {
        vxFree((*weights_bias)->biases_sizes);
        (*weights_bias)->biases_sizes = VX_NULL;
    }
    if ((*weights_bias)->input_sizes)
    {
        vxFree((*weights_bias)->input_sizes);
        (*weights_bias)->input_sizes = VX_NULL;
    }
    if ((*weights_bias)->output_sizes)
    {
        vxFree((*weights_bias)->output_sizes);
        (*weights_bias)->output_sizes = VX_NULL;
    }

    if ((*weights_bias)->tmp_fcaccel_input_ptr)
    {
        vxFree((*weights_bias)->tmp_fcaccel_input_ptr);
        (*weights_bias)->tmp_fcaccel_input_ptr = VX_NULL;
    }
    if ((*weights_bias)->tmp_fcaccel_wb_ptr)
    {
        vxFree((*weights_bias)->tmp_fcaccel_wb_ptr);
        (*weights_bias)->tmp_fcaccel_wb_ptr = VX_NULL;
    }

    if ((*weights_bias)->memory.logicals[0] != VX_NULL)
    {
        gcoVX_FreeMemory((gcsSURF_NODE_PTR)(*weights_bias)->memory.nodePtrs[0]);
        (*weights_bias)->memory.logicals[0]    = VX_NULL;
        (*weights_bias)->memory.nodePtrs[0]    = VX_NULL;
    }
    if ((*weights_bias)->memory.writeLocks[0] != VX_NULL)
    {
        vxDestroyMutex((*weights_bias)->memory.writeLocks[0]);
        (*weights_bias)->memory.writeLocks[0]  = VX_NULL;
    }

    return vxoReference_Release((vx_reference_ptr)weights_bias, VX_TYPE_WEIGHTS_BIASES_PARAMETER, VX_REF_EXTERNAL);
}

VX_API_ENTRY vx_weights_biases_parameter VX_API_CALL
vxCreateWeightsBiasesParameter(
    vx_context  context,
    vx_enum     layer_type,
    vx_uint32   num_of_dims,
    vx_uint32 * inputs_dims,
    vx_uint32   pad_x,
    vx_uint32   pad_y,
    vx_uint32   pooling_size_x,
    vx_uint32   pooling_size_y,
    vx_enum     down_scale_size_rounding,
    vx_uint32 * convolution_outputs_dims,
    vx_uint32 * pool_outputs_dims,
    vx_uint32   weights_num_of_dims,
    vx_uint32 * weights_dims,
    vx_enum     weights_data_format,
    vx_uint8    weights_fixed_point_pos,
    vx_uint32   biases_num_of_dims,
    vx_uint32 * biases_dims,
    vx_enum     biases_data_format,
    vx_uint8    biases_fixed_point_pos,
    vx_uint32   raw_data_size
    )
{
    vx_weights_biases_parameter weight_bias = VX_NULL;
    vx_uint32 i;
    vx_uint32 padXLeft, padXRight, padYTop, padYBottom;

    if (!vxoContext_IsValid(context)) return VX_NULL;

    vxnneGetPadValue((vx_int32)pad_x, (vx_int32)pad_y, &padXLeft, &padXRight, &padYTop, &padYBottom);

    weight_bias = (vx_weights_biases_parameter)vxoWeightsBiases_Create(context,
                                                                       layer_type,
                                                                       num_of_dims,
                                                                       inputs_dims,
                                                                       padXLeft,
                                                                       padXRight,
                                                                       padYTop,
                                                                       padYBottom,
                                                                       pooling_size_x,
                                                                       pooling_size_y,
                                                                       down_scale_size_rounding,
                                                                       convolution_outputs_dims,
                                                                       weights_num_of_dims,
                                                                       weights_dims,
                                                                       weights_data_format,
                                                                       weights_fixed_point_pos,
                                                                       biases_num_of_dims,biases_dims,
                                                                       biases_data_format,
                                                                       biases_fixed_point_pos);

    if (!WeightBiasBufferAllocate(context, weight_bias, raw_data_size, vx_true_e)) return VX_NULL;

    if (!vxoNNExternsionAdjustWeightsBiases(weight_bias, vx_true_e, vx_true_e, raw_data_size - weight_bias->memroy_head_offset)) return VX_NULL;

    for (i = 0; i < weight_bias->zgroup_num; i++)
    {
        /* Calculate filters per core */
        calculateFilterPerCore(context, weight_bias, i, vx_false_e, 0);
    }

    if (vxoReference_GetStatus((vx_reference)weight_bias) != VX_SUCCESS) return VX_NULL;

    return weight_bias;
}

VX_API_ENTRY vx_weights_biases_parameter VX_API_CALL vxCreateWeightsBiasesParameterFromTensors(
    vx_enum     layer_type,
    vx_uint32   num_of_dims,
    vx_uint32 * inputs_dims,
    vx_uint32   pad_x,
    vx_uint32   pad_y,
    vx_uint32   pooling_size_x,
    vx_uint32   pooling_size_y,
    vx_enum     down_scale_size_rounding,
    vx_uint32 * convolution_outputs_dims,
    vx_uint32 * pool_outputs_dims,
    vx_weights_biases_parameter_optimizations_t *optimizations,
    vx_tensor   weights,
    vx_tensor   biases
    )
{
    vx_weights_biases_parameter wb;
    vx_context context = vxGetContext((vx_reference)weights);
    vx_uint32 padXLeft, padXRight, padYTop, padYBottom;

    if (!vxoContext_IsValid(context)) return VX_NULL;

    vxnneGetPadValue((vx_int32)pad_x, (vx_int32)pad_y, &padXLeft, &padXRight, &padYTop, &padYBottom);

    wb = _createWeightsBiasesParameterFromTensors(
            context,
            layer_type,
            num_of_dims,
            inputs_dims,
            padXLeft,
            padXRight,
            padYTop,
            padYBottom,
            pooling_size_x,
            pooling_size_y,
            down_scale_size_rounding,
            convolution_outputs_dims,
            pool_outputs_dims,
            optimizations,
            weights,
            biases);

    if (wb != VX_NULL && context->options.enableNNArchPerfPrint == 2)
    {
        vxoWeightsBiasesParameter_ShowPerformance(context, wb);
    }

    return wb;
}

VX_API_ENTRY vx_weights_biases_parameter vxCreateWeightsBiasesParameterFromTensors2(
    vx_enum     layer_type,
    vx_uint32   num_of_dims,
    vx_uint32 * inputs_dims,
    vx_uint32 * convolution_outputs_dims,
    vx_uint32 * pool_outputs_dims,
    vx_enum     output_format,
    const vx_nn_convolution_relu_pooling_params convolution_relu_pooling_params,
    vx_size size_of_convlution_relu_pooling_params,
    vx_weights_biases_parameter_optimizations_t *optimizations,
    vx_tensor   weights,
    vx_tensor   biases)
{
    vx_weights_biases_parameter wb;
    vx_context context = vxGetContext((vx_reference)weights);

    if (!vxoContext_IsValid(context)) return VX_NULL;

    wb = _createWeightsBiasesParameterFromTensors(
            context,
            layer_type,
            num_of_dims,
            inputs_dims,
            convolution_relu_pooling_params->pad_x_left,
            convolution_relu_pooling_params->pad_x_right,
            convolution_relu_pooling_params->pad_y_top,
            convolution_relu_pooling_params->pad_y_bottom,
            convolution_relu_pooling_params->pool_size_x,
            convolution_relu_pooling_params->pool_size_y,
            convolution_relu_pooling_params->down_scale_size_rounding,
            convolution_outputs_dims,
            pool_outputs_dims,
            optimizations,
            weights,
            biases);

    if (wb != VX_NULL && context->options.enableNNArchPerfPrint)
    {
        vxoWeightsBiasesParameter_ShowPerformance(context, wb);
    }

    return wb;
}


VX_API_ENTRY vx_status VX_API_CALL
vxMapWeightsBiasesParameter(
    vx_weights_biases_parameter     weights_biases,
    vx_map_id *                     map_id,
    vx_uint32 *                     stride,
    void **                         ptr,
    vx_enum                         usage,
    vx_enum                         mem_type,
    vx_uint32                       flags
    )
{
    if (!vxoWeightsBiasesParameter_IsValid(weights_biases)) return VX_ERROR_INVALID_REFERENCE;

    return vxoWeightsBiasesParameter_Map(weights_biases, map_id, stride, ptr, usage, mem_type, flags);
}

VX_API_ENTRY vx_status VX_API_CALL
vxUnmapWeightsBiasesParameter(
    vx_weights_biases_parameter     weights_biases,
    vx_map_id                       map_id
    )
{
    if (!vxoWeightsBiasesParameter_IsValid(weights_biases)) return VX_ERROR_INVALID_REFERENCE;

    return vxoWeightsBiasesParameter_Unmap(weights_biases, map_id);
}

VX_API_ENTRY vx_status VX_API_CALL vxConfigTarget(
    vx_context context,
    vx_int32 dp_amount,
    vx_int32 mac_per_core,
    vx_int32 conv_cores,
    vx_int32 in_buffer_depth,
    vx_int32 accum_buffer_height,
    vx_int32 l2_cache_size,
    vx_int32 tp_cores
)
{
    if (!vxoContext_IsValid(context)) return VX_ERROR_INVALID_REFERENCE;

    context->nnConfig.nnMadPerCore = mac_per_core;
    context->nnConfig.nnCoreCount = conv_cores;
    context->nnConfig.nnInputBufferDepth = in_buffer_depth;
    context->nnConfig.nnAccumBufferDepth = accum_buffer_height;
    context->nnConfig.nnDPAmount = dp_amount;
    context->nnConfig.nnL2CacheSize = l2_cache_size;
    context->nnConfig.tpCoreCount = tp_cores;
    context->nnConfig.isSet = gcvTRUE;
    return VX_SUCCESS;
}

VX_API_ENTRY vx_uint32* VX_API_CALL vxWeightsBiasesParameterToStream(
    vx_context context,
    vx_weights_biases_parameter weights_biases_parameter,
    vx_uint32 *weights_biases_stream_size,
    vx_bool onlyHeaderStream
)
{
    vx_uint32 bufferSize;
    vx_uint32 checkSize;
    vx_uint32 bitOffset;
    vx_uint32* kernelBufferPtr = NULL;
    vx_uint32* base = NULL;
    vx_uint32 i;

    gcmASSERT(context);

    /*calculate stream buffer size*/
    if (onlyHeaderStream)
        bufferSize = sizeof(vx_uint32) * 22 + sizeof(vx_uint32) * VX_CONTEXT_TENSOR_MAX_DIMENSION * 6 + sizeof(vx_uint32) * MAX_WEIGHT_BIAS_GROUPS * 4;
    else if (weights_biases_parameter->use_fc_accel && weights_biases_parameter->layer_type == VX_CONVOLUTIONAL_NETWORK_FULLYCONNECTED_LAYER)
        bufferSize = (vx_uint32)weights_biases_parameter->memory_size - weights_biases_parameter->memroy_head_offset + sizeof(vx_uint32) * 22 + sizeof(vx_uint32) * VX_CONTEXT_TENSOR_MAX_DIMENSION * 6 + sizeof(vx_uint32) * MAX_WEIGHT_BIAS_GROUPS * 4;
    else
        bufferSize = (vx_uint32)weights_biases_parameter->memory_size + sizeof(vx_uint32) * 22 + sizeof(vx_uint32) * VX_CONTEXT_TENSOR_MAX_DIMENSION * 6 + sizeof(vx_uint32) * MAX_WEIGHT_BIAS_GROUPS * 4;

    kernelBufferPtr = (vx_uint32*)malloc(bufferSize);

    if (kernelBufferPtr == VX_NULL)
    {
        vxError("vxWeightsBiasesParameterToStream: OUT OF MEMORY");
        *weights_biases_stream_size = 0;
        return VX_NULL;
    }

    base = kernelBufferPtr;

    /* Write weights biases parameter to the head of data*/
    bitOffset = 0;
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->layer_type, 32);
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->inout_num_of_dims, 32);
    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        if (i < weights_biases_parameter->inout_num_of_dims)
        {
            writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->input_sizes[i], 32);
        }
        else
        {
            writeBits(&kernelBufferPtr, &bitOffset, 0, 32);
        }
    }
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->pad_x_left, 32);
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->pad_x_right, 32);
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->pad_y_top, 32);
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->pad_y_bottom, 32);
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->pooling_size_x, 32);
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->pooling_size_y, 32);
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->down_scale_size_rounding, 32);

    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        if (i < weights_biases_parameter->inout_num_of_dims)
        {
            writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->output_sizes[i], 32);
        }
        else
        {
            writeBits(&kernelBufferPtr, &bitOffset, 0, 32);
        }
    }

    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        if (i < 2)
        {
            writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->output_sizes[i] / weights_biases_parameter->pooling_stride, 32);
        }
        else if (i < weights_biases_parameter->inout_num_of_dims)
        {
            writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->output_sizes[i], 32);
        }
        else
        {
            writeBits(&kernelBufferPtr, &bitOffset, 0, 32);
        }
    }

    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->weights_num_of_dims, 32);
    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        if (i < weights_biases_parameter->weights_num_of_dims)
        {
            writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->weights_sizes[i], 32);
            writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->org_weights_sizes[i], 32);
        }
        else
        {
            writeBits(&kernelBufferPtr, &bitOffset, 0, 32);
            writeBits(&kernelBufferPtr, &bitOffset, 0, 32);
        }
    }
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->weights_data_format, 32);
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->weights_fixed_point_pos, 32);

    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->biases_num_of_dims, 32);
    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        if (i < weights_biases_parameter->biases_num_of_dims)
        {
            writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->biases_sizes[i], 32);
        }
        else
        {
            writeBits(&kernelBufferPtr, &bitOffset, 0, 32);
        }
    }
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->biases_data_format, 32);
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->biases_fixed_point_pos, 32);
    /* Save kernel stream buffer size*/
    writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)weights_biases_parameter->memory_size, 32);

    /* Save kernel per core*/
    for (i = 0; i < MAX_WEIGHT_BIAS_GROUPS; i++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->zgroup_array[i], 32);
        writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->outImageTileXSize[0][i], 32);
        writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->outImageTileYSize[0][i], 32);
        writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->kernelsPerCore[0][i], 32);
    }
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->current_mad_per_core, 32);
    writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)weights_biases_parameter->use_fc_accel, 32);
    writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)weights_biases_parameter->fc_accel_large_size, 32);
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->input_nonzero_count, 32);
    writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)weights_biases_parameter->weightFixedPointPos, 32);
    writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)weights_biases_parameter->kernelStreamSize, 32);

    /* Copy kernel stream data*/
    kernelBufferPtr++;
    if (onlyHeaderStream)
        checkSize = (vx_uint32)((vx_uint8*)kernelBufferPtr - (vx_uint8*)base);
    else if (weights_biases_parameter->use_fc_accel && weights_biases_parameter->layer_type == VX_CONVOLUTIONAL_NETWORK_FULLYCONNECTED_LAYER)
    {
        memcpy(kernelBufferPtr, weights_biases_parameter->tmp_fcaccel_wb_ptr, weights_biases_parameter->memory_size - weights_biases_parameter->memroy_head_offset);
        checkSize = (vx_uint32)((vx_uint8*)kernelBufferPtr - (vx_uint8*)base) + (vx_uint32)weights_biases_parameter->memory_size - weights_biases_parameter->memroy_head_offset;
    }
    else
    {
        memcpy(kernelBufferPtr, weights_biases_parameter->memory.logicals[0] - weights_biases_parameter->memroy_head_offset, weights_biases_parameter->memory_size);
        checkSize = (vx_uint32)((vx_uint8*)kernelBufferPtr - (vx_uint8*)base) + (vx_uint32)weights_biases_parameter->memory_size;
    }

    vxmASSERT(checkSize == bufferSize);
    *weights_biases_stream_size = bufferSize;
    return base;
}

VX_API_ENTRY vx_status VX_API_CALL vxFreeWeightsBiasesParameterStream(
    vx_uint32 *weights_biases_stream
)
{
    if (weights_biases_stream != VX_NULL)
        free(weights_biases_stream);
    weights_biases_stream = VX_NULL;

    return VX_SUCCESS;
}

#define DUMP_OFFLINE 0
VX_API_ENTRY vx_weights_biases_parameter VX_API_CALL vxCreateWeightsBiasesParameterFromStream(
    vx_context context,
    vx_uint32 * weights_biases_stream
)
{
    vx_weights_biases_parameter weights_bias;
    vx_uint32 bitOffset = 0;
    vx_uint32* streamBufferPtr = weights_biases_stream;
    vx_uint32 i;
    vx_status status = VX_SUCCESS;

    vx_enum     layerType;
    vx_uint32   numOfDims;
    vx_uint32   inputsDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32   poolingSizeX;
    vx_uint32   poolingSizeY;
    vx_enum     downScaleSizeRounding;
    vx_uint32   convolutionOutputsDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32   weightsNumOfDims;
    vx_uint32   weightsDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32   weightsOrgDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_enum     weightsFormat;
    vx_uint8    weightsFixedPointPos;
    vx_uint32   biasesNumOfDims;
    vx_uint32   biasesDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_enum     biasesFormat;
    vx_uint8    biasesFixedPointPos;
    vx_uint32   rawDataSize;
    vx_uint32   biasPad;
    vx_uint32   weightSize;
    vx_uint32   padXLeft, padXRight, padYTop, padYBottom;

#if DUMP_OFFLINE
    char fileName[1024];
    vx_uint32 * base = streamBufferPtr;
    FILE *outputFile = NULL;
    static vx_uint32 idx = 0;
    vx_uint32 size;
#endif
    if (!vxoContext_IsValid(context)) return VX_NULL;

    context->options.fcZMax = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7) ? (1<<20) - 1 : (1<<14) - 1;

    layerType = readBits(&streamBufferPtr, &bitOffset, 32);
    numOfDims = readBits(&streamBufferPtr, &bitOffset, 32);

    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        inputsDims[i] = readBits(&streamBufferPtr, &bitOffset, 32);
    }

    padXLeft = readBits(&streamBufferPtr, &bitOffset, 32);
    padXRight = readBits(&streamBufferPtr, &bitOffset, 32);
    padYTop = readBits(&streamBufferPtr, &bitOffset, 32);
    padYBottom = readBits(&streamBufferPtr, &bitOffset, 32);
    poolingSizeX = readBits(&streamBufferPtr, &bitOffset, 32);
    poolingSizeY = readBits(&streamBufferPtr, &bitOffset, 32);
    downScaleSizeRounding = readBits(&streamBufferPtr, &bitOffset, 32);

    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        convolutionOutputsDims[i] = readBits(&streamBufferPtr, &bitOffset, 32);
    }
    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        /* Read/skip poolingDims[i]. */
        readBits(&streamBufferPtr, &bitOffset, 32);
    }

    weightsNumOfDims = readBits(&streamBufferPtr, &bitOffset, 32);
    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        weightsDims[i] = readBits(&streamBufferPtr, &bitOffset, 32);
        weightsOrgDims[i] = readBits(&streamBufferPtr, &bitOffset, 32);
    }
    weightsFormat = readBits(&streamBufferPtr, &bitOffset, 32);
    weightsFixedPointPos = (vx_uint8)readBits(&streamBufferPtr, &bitOffset, 32);
    biasesNumOfDims = readBits(&streamBufferPtr, &bitOffset, 32);
    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        biasesDims[i] = readBits(&streamBufferPtr, &bitOffset, 32);
    }
    biasesFormat = readBits(&streamBufferPtr, &bitOffset, 32);
    biasesFixedPointPos = (vx_uint8)readBits(&streamBufferPtr, &bitOffset, 32);
    rawDataSize = readBits(&streamBufferPtr, &bitOffset, 32);

    weights_bias = (vx_weights_biases_parameter)vxoWeightsBiasesFromStream_Create(context,
                                                                   layerType,
                                                                   numOfDims,
                                                                   inputsDims,
                                                                   padXLeft,
                                                                   padXRight,
                                                                   padYTop,
                                                                   padYBottom,
                                                                   poolingSizeX,
                                                                   poolingSizeY,
                                                                   downScaleSizeRounding,
                                                                   convolutionOutputsDims,
                                                                   weightsNumOfDims,
                                                                   weightsDims,
                                                                   weightsOrgDims,
                                                                   weightsFormat,
                                                                   weightsFixedPointPos,
                                                                   biasesNumOfDims,biasesDims,
                                                                   biasesFormat,
                                                                   biasesFixedPointPos);

    if (vxoReference_GetStatus((vx_reference)weights_bias) != VX_SUCCESS) return VX_NULL;

    if (!WeightBiasBufferAllocate(context, weights_bias, rawDataSize - weights_bias->memroy_head_offset, vx_false_e))
    {
        status = VX_ERROR_NO_MEMORY;
        goto exit;
    }
    weights_bias->memory_size = rawDataSize;

    if (!vxoNNExternsionAdjustWeightsBiases(weights_bias, vx_true_e, vx_true_e, weights_bias->memory_size))
    {
        status = VX_FAILURE;
        goto exit;
    }

    for (i = 0; i < MAX_WEIGHT_BIAS_GROUPS; i++)
    {
        weights_bias->zgroup_array[i] = readBits(&streamBufferPtr, &bitOffset, 32);
        weights_bias->outImageTileXSize[0][i] = readBits(&streamBufferPtr, &bitOffset, 32);
        weights_bias->outImageTileYSize[0][i] = readBits(&streamBufferPtr, &bitOffset, 32);
        weights_bias->kernelsPerCore[0][i] = readBits(&streamBufferPtr, &bitOffset, 32);
    }
    weights_bias->current_mad_per_core = readBits(&streamBufferPtr, &bitOffset, 32);
    weights_bias->use_fc_accel = (vx_bool)readBits(&streamBufferPtr, &bitOffset, 32);
    weights_bias->fc_accel_large_size = (vx_bool)readBits(&streamBufferPtr, &bitOffset, 32);
    weights_bias->input_nonzero_count = readBits(&streamBufferPtr, &bitOffset, 32);
    weights_bias->weightFixedPointPos = (vx_uint8)readBits(&streamBufferPtr, &bitOffset, 32);
    weights_bias->kernelStreamSize = readBits(&streamBufferPtr, &bitOffset, 32);

    weightSize = (vx_uint32)vxDataType_GetSize((vx_type_e)weights_bias->weights_data_format);

    /* Copy kernel stream data*/
    streamBufferPtr++;

    if (weights_bias->use_fc_accel && layerType == VX_CONVOLUTIONAL_NETWORK_FULLYCONNECTED_LAYER)
    {
        biasPad = weights_bias->org_weights_sizes[2] < context->options.fcZMax ? 1 : weights_bias->weights_sizes[1];

        weights_bias->tmp_fcaccel_input_ptr = vxAllocateAndZeroMemory((weights_bias->org_weights_sizes[2] + biasPad) * weightSize);
        weights_bias->tmp_fcaccel_wb_ptr = vxAllocateAndZeroMemory(weights_bias->memory_size - weights_bias->memroy_head_offset);
        if (weights_bias->tmp_fcaccel_input_ptr == VX_NULL || weights_bias->tmp_fcaccel_wb_ptr == VX_NULL)
        {
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }
        memcpy(weights_bias->tmp_fcaccel_wb_ptr, streamBufferPtr, weights_bias->memory_size - weights_bias->memroy_head_offset);
    }
    else
        memcpy(weights_bias->memory.logicals[0] - weights_bias->memroy_head_offset, streamBufferPtr, weights_bias->memory_size);

#if DUMP_OFFLINE
    sprintf(fileName, "weightsBias_buffer_offline_lev%d.dat", idx++);
    outputFile = fopen(fileName,"wb");
    if (outputFile == NULL)
    {
        printf("can't find file %s", fileName);

    }
    size = (vx_uint32)((vx_uint8*)streamBufferPtr - (vx_uint8*)base) + (vx_uint32)weights_bias->memory_size;
    fwrite(base, size, 1, outputFile);
    fclose(outputFile);


#endif

exit:
    if (status == VX_SUCCESS)
        return weights_bias;
    else
    {
        vxError("vxCreateWeightsBiasesParameterFromStream failed: status is 0x%x", status);
        return VX_NULL;
    }
}

vx_status vxnneLayer_Deinitialize(struct _vxnne_layer_s* layer)
{
    vx_uint32 i;

    for (i = 0; i < VX_MAX_TEMP_TENSORS; i++)
    {
        if (layer->temp_tensors[i] != VX_NULL)
        {
            vxoTensor_ReleaseMemory(layer->temp_tensors[i]);
        }
    }

    if (layer->cmdNNBuff != VX_NULL)
    {
        vxReleaseArray(&layer->cmdNNBuff);
    }

    if (layer->cmdTPBuff != VX_NULL)
    {
        vxReleaseArray(&layer->cmdTPBuff);
    }

    for (i = 0; i < layer->num_operations; i++)
    {
        if (layer->operations[i]->deinitialize != VX_NULL)
        {
            layer->operations[i]->deinitialize(layer->operations[i]);
        }
    }

    return VX_SUCCESS;
}

vx_status vxnneLayer_Dump(
    struct _vxnne_layer_s* layer,
    int verbose
    )
{
    vx_uint32        paramIndex = 0;
    vx_uint32        width;
    vx_uint32        height;
    vx_uint32        depth;
    vx_uint32        batch;
    vx_tensor        outputs;
    vx_char          fileName[256] = {'\0'};
    FILE             *fp;
    static vx_uint32 layerNum = 0;
    vx_uint32        index;
    vx_uint32        elementCount;
    void             *outputsBase = VX_NULL;

    printf("start excute layer %s_%d\n", layer->name, layerNum);
    gcoVX_Flush(gcvTRUE);
    printf("end excute layer %s_%d\n", layer->name, layerNum);

    for (paramIndex = 0; paramIndex < layer->node->kernel->signature.paramCount; paramIndex++)
    {
        vx_reference paramRef = layer->node->paramTable[paramIndex];

        if (paramRef == VX_NULL) continue;

        if (!vxmIS_OUTPUT_OR_BIDIRECTION(layer->node->kernel->signature.directionTable[paramIndex])) continue;

        outputs     = (vx_tensor)paramRef;
        width       = TENSOR_VIEW_SIZE_INDEX(outputs, 0);
        height      = TENSOR_VIEW_SIZE_INDEX(outputs, 1);
        depth       = TENSOR_VIEW_SIZE_INDEX(outputs, 2);
        batch       = TENSOR_VIEW_SIZE_INDEX(outputs, 3);
        vxoTensor_GetTensorViewMemory(outputs, &outputsBase, VX_NULL);

        sprintf(fileName,"%s_%d_%d_%d_%d_%d.txt", layer->name, layerNum, width, height, depth, batch);

        fp = fopen(fileName,"w");
        if(!fp)
        {
            printf("can't open the file %s\n",fileName);
            fclose(fp);
            continue;
        }

        if(batch == 0)
        {
            batch = 1;
        }

        printf("***********dump layer :%2d***************\n",layerNum);

        elementCount = width * height * depth * batch;
        for(index = 0; index < elementCount; index++)
        {
            fprintf(fp, "%f\n", vxnneGetData((vx_type_e)outputs->tensorBuffer->dataFormat, index, (vx_uint8_ptr)outputsBase, outputs->tensorBuffer->fixedPointPos));
        }
        fclose(fp);
    }

    layerNum++;

    return VX_SUCCESS;
}

vx_status vxnneLayer_Free(struct _vxnne_layer_s* layer)
{
    layer->deinitialize(layer);
    gcoOS_Free(gcvNULL, layer);

    return VX_SUCCESS;
}

vx_status vxnneLayer_Execute(vxnne_layer layer)
{
    vx_uint32 i;
#if defined(__linux__)
    vx_uint32 layerExcuteTime = 0;
    if (layer->node->base.context->options.enableCNNPerf)
        printf("layer name: %s\n", layer->name);
#endif

    for (i = 0; i < layer->num_operations; i++)
    {
#if defined(__linux__)
        struct timeval operatorStart = {0};
        vx_uint32 opertorExcuteTime = 0;

        operatorStart = gcfVX_PerfStart((vx_reference)layer->node);
#endif

        layer->operations[i]->execute(layer->operations[i]);

#if defined(__linux__)
        opertorExcuteTime = gcfVX_PerfEnd((vx_reference)layer->node, operatorStart);
        if (layer->node->base.context->options.enableCNNPerf)
        {
            layerExcuteTime += opertorExcuteTime;

            printf("operation[%d] type %-40s target %s execution time:%10d us\n", i, vxnneGetOperatorTypeName(layer->operations[i]->operatorType), vxnneGetOperatorTargetName(layer->operations[i]->target), opertorExcuteTime);
        }
#endif
    }

#if defined(__linux__)
    if (layer->node->base.context->options.enableCNNPerf)
    {
        printf("%s execution time:%10d us\n", layer->name, layerExcuteTime);
        printf("\n");
    }
#endif

    if (layer->node->base.context->options.enableNNLayerDump)
    {
        layer->dump = vxnneLayer_Dump;
        layer->dump(layer, 0);
    }

    return VX_SUCCESS;
}

vx_status vxnneLayer_Initialize(
    vxnne_layer                 layer,
    vx_char                     *name,
    vx_node                     node,
    vxnne_operation             *operation,
    vxnne_layer_deinitialize_f  deinitialize
    )
{
    layer->name         = name;
    layer->node         = node;
    layer->operations   = operation;

    layer->num_temp_tensors      = 0;
    layer->dump                  = VX_NULL;
    layer->deinitialize          = (deinitialize ? deinitialize :  vxnneLayer_Deinitialize);

    return VX_SUCCESS;
}

vx_status vxnneOperation_Deinitialize(vxnne_operation_s *operation)
{
    return VX_SUCCESS;
}

vx_status vxnneOperation_TP_Deinitialize(vxnne_operation_s *operation)
{
    vxnne_tp_operation op = (vxnne_tp_operation)operation;

    if (op->buffer != VX_NULL)
    {
        vxReleaseArray(&op->buffer);
    }

    return VX_SUCCESS;
}

vx_status vxnneOperation_Initialize(
                vxnne_operation_s               *operation,
                vxnne_layer                     layer,
                vxnne_operation_target_e        target,
                vxnne_operator_e                operatorType,
                vxnne_operation_execute_f       execute,
                vxnne_operation_deinitialize_f  deinitialize
                )
{
    operation->layer         = layer;
    operation->target        = target;
    operation->operatorType = operatorType;
    operation->execute       = execute;
    operation->initialize    = VX_NULL;
    operation->deinitialize  = (deinitialize ? deinitialize :  vxnneOperation_Deinitialize);
    operation->dump          = VX_NULL;
    return VX_SUCCESS;
}

vx_status vxnneShaderOperation_Execute(vxnne_operation_s *operation)
{
    vx_status status;
    vx_uint32 i;
    vxnne_shader_operation shaderOperation  = (vxnne_shader_operation)operation;
    vx_shader    kernelShader;

    kernelShader = shaderOperation->shaderExecutable->kernelShader;

    status = vxoShader_SetParameters(kernelShader, shaderOperation->shaderExecutable->param, shaderOperation->shaderExecutable->paramNum, VX_NULL);
    if (status != VX_SUCCESS) goto error;

    for(i = 0; i < shaderOperation->shaderExecutable->uniformCount; i++)
    {
        status = vxoShader_SetUniform(
                        kernelShader,
                        shaderOperation->shaderExecutable->uniforms[i].name,
                        shaderOperation->shaderExecutable->uniforms[i].count,
                        shaderOperation->shaderExecutable->uniforms[i].data);
        if (status != VX_SUCCESS) goto error;
    }

    status = vxoShader_Execute(kernelShader,
                                    &shaderOperation->shaderExecutable->borderMode,
                                    &shaderOperation->shaderExecutable->shaderParam,
                                    shaderOperation->base.layer->node->base.context->devices,
                                    shaderOperation->base.layer->node->base.context->deviceCount);

    if (operation->layer->node->base.context->options.enableCNNPerf)
    {
        gcoVX_Flush(gcvTRUE);
    }

error:
    return status;
}

vx_status vxnneShaderExecutable_Destroy(vxnne_shader_executable shaderExecutable)
{
    vx_uint32 i;

    for (i = 0; i < shaderExecutable->paramNum; i++)
    {
       vxoReference_Release(&shaderExecutable->param[i], shaderExecutable->param[i]->type, VX_REF_INTERNAL);
    }

    if (shaderExecutable->uniforms)
    {
        for(i = 0 ; i < shaderExecutable->uniformCount; i++)
        {
            gcoOS_Free(gcvNULL, shaderExecutable->uniforms[i].data);
        }

        gcoOS_Free(gcvNULL, shaderExecutable->uniforms);
    }

    gcoOS_Free(gcvNULL, shaderExecutable);

    return VX_SUCCESS;
}

vx_status vxnneShaderOperation_Deinitialize(vxnne_operation_s *operation)
{
    vxnne_shader_operation shader_operation = (vxnne_shader_operation)operation;
    if (shader_operation->shaderExecutable)
    {
        vxnneShaderExecutable_Destroy(shader_operation->shaderExecutable);

        shader_operation->shaderExecutable = VX_NULL;
    }

    return VX_SUCCESS;
}

vx_status vxnneShaderExecutable_SetParameters(vxnne_shader_executable shaderExecutable, vx_reference parameters[], vx_uint32 paramNum)
{
    vx_uint32 i;

    if (paramNum > VX_MAX_SHADER_PARAMETERS) goto error;

    for (i = 0; i < paramNum; i++)
    {
        shaderExecutable->param[i] = parameters[i];
        vxoReference_Increment(shaderExecutable->param[i], VX_REF_INTERNAL);
    }

    shaderExecutable->paramNum = paramNum;

    return VX_SUCCESS;
error:
    return VX_FAILURE;
}

vx_status vxnneShaderExecutable_SetExecutionParameters(vxnne_shader_executable shaderExecutable, vx_kernel_execution_parameters_t *shaderParam)
{
    shaderExecutable->shaderParam = *shaderParam;

    return VX_SUCCESS;
}

vx_status vxnneShaderExecutable_SetUniform(vxnne_shader_executable shaderExecutable, vx_char *name, vx_uint32 count, void * value)
{
    vx_uint32 size;
    vx_status vStatus = VX_FAILURE;
    gceSTATUS status;

    if (shaderExecutable->uniformCount >= shaderExecutable->kernelShader->numArgs) goto error;

    if (!shaderExecutable->uniforms)
    {
        /*allocat the maximum number uniforms */
        status = gcoOS_Allocate(gcvNULL, shaderExecutable->kernelShader->numArgs * gcmSIZEOF(vx_node_s), (gctPOINTER*)&shaderExecutable->uniforms);
        if (gcmIS_ERROR(status))
        {
            vStatus = VX_FAILURE;
            goto error;
        }
    }

    vStatus = vxoShader_GetUniformSize(shaderExecutable->kernelShader, name, &size);
    if (vStatus != VX_SUCCESS) goto error;

    status = gcoOS_Allocate(gcvNULL, size, (gctPOINTER*)&shaderExecutable->uniforms[shaderExecutable->uniformCount].data);
    if (gcmIS_ERROR(status))
    {
        vStatus = VX_FAILURE;
        goto error;
    }

    gcoOS_MemCopy(shaderExecutable->uniforms[shaderExecutable->uniformCount].data, value, size);

    shaderExecutable->uniforms[shaderExecutable->uniformCount].count = count;
    shaderExecutable->uniforms[shaderExecutable->uniformCount].name  = name;
    shaderExecutable->uniforms[shaderExecutable->uniformCount].size  = size;

    shaderExecutable->uniformCount++;

error:
    return vStatus;
}

vxnne_shader_executable  vxnneKernelShaders_CreateShaderExecutable(vxnne_kernel_shaders kernel, vx_char * subName, vx_border_mode_t *borderMode)
{
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vx_char     kernelName[256]     = {0};
    vx_uint32   i, shaderID;

    gceSTATUS status = gcoOS_Allocate(gcvNULL, gcmSIZEOF(vxnne_shader_executable_s), (gctPOINTER*)&shaderExecutable);
    if (gcmIS_ERROR(status)) goto error;

    gcoOS_ZeroMemory((gctPOINTER)shaderExecutable, gcmSIZEOF(vxnne_shader_executable_s));

    shaderExecutable->borderMode = *borderMode;

    gcoOS_StrCopySafe(kernelName, 256, kernel->kernelName);

    if (subName)
    {
        gcoOS_StrCatSafe(kernelName, 256, subName);
    }

    for(i = 0; i < kernel->kernelShaderCount; i++)
    {
        if (gcoOS_StrCmp(kernel->kernelShader[i*2]->name, kernelName) == 0)
            break;
    }

    if (i == kernel->kernelShaderCount) goto error;

    shaderID = ((shaderExecutable->borderMode.mode == VX_BORDER_MODE_CONSTANT) ? 1 : 0);

    shaderExecutable->kernelShader = kernel->kernelShader[i*2 + shaderID];

    return shaderExecutable;

error:
    if (shaderExecutable) gcoOS_Free(gcvNULL, (gctPOINTER)shaderExecutable);

    return VX_NULL;
}


vx_status vxnneShaderOperation_Initialize(
    vxnne_shader_operation_s            *operation,
    vxnne_layer                         layer,
    vxnne_operator_e                    operatorType,
    vxnne_shader_executable                 shaderExecutable
    )
{
    operation->base.layer           = layer;
    operation->base.dump            = VX_NULL;
    operation->base.execute         = vxnneShaderOperation_Execute;
    operation->base.operatorType   = operatorType;
    operation->base.deinitialize    = vxnneShaderOperation_Deinitialize;
    operation->base.target          = VXNNE_OPERATION_TARGET_SH;
    operation->shaderExecutable        = shaderExecutable;
    return VX_SUCCESS;
}

vxnne_kernel_shaders vxnneGetKernelShadersByEnum(vx_context context, vx_enum kernelEnum)
{
    if (context->kernels[kernelEnum].kernelShader)
    {
        return &context->kernels[kernelEnum];
    }
    else
    {
        return VX_NULL;
    }
}

vxnne_kernel_shaders vxnneAddKernelShadersInProgram(vx_context context, vx_char* kernelName, vx_program program, vx_uint32  paramNum, vx_enum kernelEnum)
{
    vxnne_kernel_shaders kernel = &context->kernels[kernelEnum];

    /* if exists then failed to add */
    if (kernel->kernelShader) return VX_NULL;

    kernel->kernelName  = kernelName;
    kernel->kernelEnum  = kernelEnum;
    kernel->paramNum    = paramNum;

    vxoKernel_CreateShaders(
            program,
            kernelName,
            &kernel->kernelShaderCount,
            &kernel->kernelShader);

    return kernel;
}

vx_status vxnneExecuteSWReshuffle(struct _vxnne_operation_s *operation)
{
    vxnne_reshuffle_operation           reshuffleOperation   = (vxnne_reshuffle_operation)operation;

    vx_tensor inputs = (vx_tensor)reshuffleOperation->inputs;
    vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter)reshuffleOperation->weights_biases;
    vx_enum   padMode = reshuffleOperation->pad_mode;
    vx_scalar padConst = reshuffleOperation->pad_const;
    vx_tensor outputs = (vx_tensor)reshuffleOperation->outputs;
    vx_uint32 stride_x, stride_y;
    vx_uint32 padXLeft;
    vx_uint32 padXRight;
    vx_uint32 padYTop;
    vx_uint32 padYBottom;
    void * padConstPtr = VX_NULL;

    vx_status status = VX_SUCCESS;


    padXLeft   = reshuffleOperation->pad_x_left;
    padXRight  = reshuffleOperation->pad_x_right;
    padYTop    = reshuffleOperation->pad_y_top;
    padYBottom = reshuffleOperation->pad_y_bottom;

    stride_x = stride_y = weights_biases->stride;

    if (padConst != VX_NULL)
    {
        padConstPtr = (void*)malloc(sizeof(vx_float32));
        vxReadScalarValue(padConst, padConstPtr);
        vxWriteScalarValue(padConst, padConstPtr);
    }

    /* if stride > 1, need do reshuffle with input buffer */
    gcmASSERT (weights_biases->stride > 1);

    {
        vxoNNExternsionDoReshuffle(inputs, outputs, padXLeft, padXRight, padYTop, padYBottom, padMode, padConstPtr, stride_x, stride_y);
    }

    if (padConst != VX_NULL)
    {
        free(padConstPtr);
    }

    return status;
}

vx_status vxnneExecuteTPGeneric(
    vx_node                     node,
    vx_tensor                   inputs[],
    vx_weights_biases_parameter wb,
    vx_array                    buffer,
    vx_tensor                   outputs[],
    vx_array                    cmd,
    vx_uint32                   opnum,
    vx_bool                     multensor
    )
{
    vx_status status;
    vx_uint32 i, inSliceSize, inSize=0, outSliceSize, outSize=0, inOffset=0, outOffset=0;

    if (wb != VX_NULL && node->base.context->options.enableNNArchPerfPrint == 1)
    {
        vxoWeightsBiasesParameter_ShowPerformance(node->base.context, wb);
    }

    if (wb == VX_NULL)
    {
        if (opnum > 1 && !multensor)
        {
            inSliceSize = TENSOR_VIEW_SIZE_INDEX(inputs[0], 0) * TENSOR_VIEW_SIZE_INDEX(inputs[0], 1);
            inSize = inSliceSize * TENSOR_DATA_SIZE(inputs[0]);
            outSliceSize = TENSOR_VIEW_SIZE_INDEX(outputs[0], 0) * TENSOR_VIEW_SIZE_INDEX(outputs[0], 1);
            outSize = outSliceSize * TENSOR_DATA_SIZE(outputs[0]);
        }
        else if (!multensor)
        {
            vxoTensor_GetTensorSize(inputs[0], &inSize);
            vxoTensor_GetTensorSize(outputs[0], &outSize);
        }

        for (i = 0; i < opnum; i++)
        {
            vx_tensor input, output;

            if (multensor)
            {
                vxoTensor_GetTensorSize(inputs[i], &inSize);
                vxoTensor_GetTensorSize(outputs[i], &outSize);
                inOffset = 0;
                outOffset = 0;
                input = inputs[i];
                output = outputs[i];
            }
            else
            {
                input = inputs[0];
                output = outputs[0];
            }

            status = vxTPExecute(node, opnum,
                                 cmd, TP_COMMAND_SIZE*i, TP_COMMAND_SIZE,
                                 VX_NULL, 0, 0,
                                 input, inOffset, inSize,
                                 output, outOffset, outSize,
                                 buffer);
            if (status != VX_SUCCESS) return status;
            inOffset  += inSize;
            outOffset += outSize;
        }
    }
    else
    {
        vxoTensor_GetTensorSize(inputs[0], &inSize);
        for (i = 0; i < wb->zgroup_num; i++)
        {
            outSize = wb->zgroup_array[i] * TENSOR_DATA_SIZE(outputs[0]);

            status = vxTPExecute(node, wb->zgroup_num,
                                 cmd, TP_COMMAND_SIZE * i, TP_COMMAND_SIZE,
                                 wb, (vx_uint32)wb->memory_offset_array[i], (vx_uint32)wb->memory_sizes_array[i],
                                 inputs[0], 0, inSize,
                                 outputs[0], outOffset, outSize,
                                 VX_NULL);
            if (status != VX_SUCCESS) return status;

            outOffset += outSize;
        }
    }

    return VX_SUCCESS;
}

vx_status vxnneExecuteTPReshuffle(struct _vxnne_operation_s *operation)
{
    vxnne_convolution_relu_pooling_layer layer = (vxnne_convolution_relu_pooling_layer)operation->layer;
    vxnne_tp_operation reshuffleOperation = (vxnne_tp_operation)operation;

    vx_node node = layer->base.node;
    vx_tensor* inputs = reshuffleOperation->inputs;
    vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter)reshuffleOperation->weights_biases;
    vx_tensor* outputs = reshuffleOperation->outputs;

    if (!(weights_biases->stride > 1))
    {
        gcmASSERT(gcvFALSE);
    }

    return vxnneExecuteTPGeneric(node, inputs, VX_NULL, VX_NULL, outputs, layer->base.cmdTPBuff, reshuffleOperation->op_num, reshuffleOperation->multi_tensor);
}

vx_status vxnneExecuteSWReSizeCopy(struct _vxnne_operation_s *operation)
{
    vxnne_resize_operation           resizeOperation   = (vxnne_resize_operation)operation;

    vx_tensor srcTensor = (vx_tensor)resizeOperation->inputs;
    vx_tensor dstTensor = (vx_tensor)resizeOperation->outputs;

    gctPOINTER srcLogical = VX_NULL;
    gctPOINTER dstLogical = VX_NULL;

    vx_uint32 dstSize;

    vxoTensor_GetTensorViewMemory(srcTensor, &srcLogical, VX_NULL);
    vxoTensor_GetTensorViewMemory(dstTensor, &dstLogical, VX_NULL);

    dstSize = (vx_uint32)vxoMemory_ComputeSize(&dstTensor->tensorBuffer->memory, 0);

    memset(dstLogical,
           0,
           dstSize);

    vxoTensor_CopyTensorPatchEx(
        (vx_uint8_ptr)srcLogical,
        (vx_uint8_ptr)dstLogical,
        2,
        TENSOR_SIZES(srcTensor),
        TENSOR_STRIDES(srcTensor),
        TENSOR_STRIDES(dstTensor),
        srcTensor->tensorBuffer->dataFormat,
        dstTensor->tensorBuffer->dataFormat
        );

    return VX_SUCCESS;
}

vx_status vxnneExecuteConvolutionReluPooling(struct _vxnne_operation_s *operation)
{
    vxnne_convolution_relu_pooling_layer layer                         = (vxnne_convolution_relu_pooling_layer)operation->layer;
    vxnne_convolution_relu_pooling_operation   convolutionOperation   = (vxnne_convolution_relu_pooling_operation)operation;

    vx_node node = layer->base.node;

    vx_tensor inputs = (vx_tensor)convolutionOperation->inputs;
    vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter)convolutionOperation->weights_biases;
    vx_tensor outputs = (vx_tensor)convolutionOperation->outputs;

    vx_uint32 input_offset = 0;
    vx_uint32 output_offset = 0;
    vx_uint32 dim_index;
    vx_status status = VX_SUCCESS;

    /* need compute input and output offset */
    if (inputs->isViewed)
    {
        for (dim_index = 0; dim_index < inputs->viewRegion.dimCount; dim_index++)
        {
            input_offset += inputs->viewRegion.viewStarts[dim_index] * TENSOR_STRIDE_INDEX(inputs, dim_index);
        }
    }

    if (outputs->isViewed)
    {
        for (dim_index = 0; dim_index < outputs->viewRegion.dimCount; dim_index++)
        {
            output_offset += outputs->viewRegion.viewStarts[dim_index] * TENSOR_STRIDE_INDEX(outputs, dim_index);
        }
    }


    if (weights_biases != VX_NULL && node->base.context->options.enableNNArchPerfPrint == 1)
    {
        vxoWeightsBiasesParameter_ShowPerformance(node->base.context, weights_biases);
    }

    /* NN Engine programming. */
    status = vxNNExecute(node, layer->base.cmdNNBuff, 0, weights_biases, 0, inputs, input_offset, outputs, output_offset);

    return status;

}

vx_status vxnneExecuteSWFullyConnected(struct _vxnne_operation_s *operation)
{
    vxnne_fully_connected_sw_operation           fullyConnectedOperation   = (vxnne_fully_connected_sw_operation)operation;

    vx_tensor inputs  = (vx_tensor)fullyConnectedOperation->inputs;
    vx_tensor weights = (vx_tensor)fullyConnectedOperation->weights;
    vx_tensor biases  = (vx_tensor)fullyConnectedOperation->biases;
    vx_tensor outputs = (vx_tensor)fullyConnectedOperation->outputs;
    gctPOINTER inputsBaseLogicalAddr = VX_NULL, outputsBaseLogicalAddr = VX_NULL;
    gctPOINTER weightsBaseLogicalAddr = VX_NULL, biasesBaseLogicalAddr = VX_NULL;
    vx_uint32 i = 0, j = 0;
    vx_uint32 inputCount, outputCount;
    vx_float32 madValue, inputValue, weightValue, biasValue = 0.0f;
    vx_enum srcType, dstType, weightsType, biasesType, outputRoundingMode;
    vx_uint8 inputFpPos = 0, weightFpPos = 0, biasFpPos = 0, outputFpPos = 0;
    vx_float32 result = 0.0f;
    vx_status status = VX_SUCCESS;

    srcType = inputs->tensorBuffer->dataFormat;
    dstType = outputs->tensorBuffer->dataFormat;
    weightsType = weights->tensorBuffer->dataFormat;
    biasesType = biases->tensorBuffer->dataFormat;

    vxoTensor_GetTensorViewMemory(inputs, &inputsBaseLogicalAddr, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, &outputsBaseLogicalAddr, VX_NULL);
    vxoTensor_GetTensorViewMemory(weights, &weightsBaseLogicalAddr, VX_NULL);
    vxoTensor_GetTensorViewMemory(biases, &biasesBaseLogicalAddr, VX_NULL);

    inputCount = (vx_uint32)vxoMemory_ComputeElementCount(&inputs->tensorBuffer->memory, 0);
    outputCount = (vx_uint32)vxoMemory_ComputeElementCount(&outputs->tensorBuffer->memory, 0);

    inputFpPos = inputs->tensorBuffer->fixedPointPos;
    weightFpPos = weights->tensorBuffer->fixedPointPos;
    biasFpPos = biases->tensorBuffer->fixedPointPos;
    outputFpPos = outputs->tensorBuffer->fixedPointPos;
    outputRoundingMode = outputs->tensorBuffer->roundingMode;

    for (i = 0; i < outputCount; i++)
    {
        madValue = 0.0;
        for (j = 0; j < inputCount; j++)
        {
            if (((srcType == VX_TYPE_FLOAT16) && (weightsType == VX_TYPE_FLOAT16) && (biasesType == VX_TYPE_FLOAT32)) ||
                ((srcType == VX_TYPE_FLOAT32) && (weightsType == VX_TYPE_FLOAT32) && (biasesType ==  VX_TYPE_FLOAT32)) ||
                ((srcType == VX_TYPE_INT8) && (weightsType == VX_TYPE_INT8) && (biasesType == VX_TYPE_INT32 || biasesType == VX_TYPE_FLOAT32)))
            {
                inputValue  = vxnneGetData((vx_type_e)srcType, j, (vx_uint8_ptr)inputsBaseLogicalAddr, inputFpPos);
                weightValue = vxnneGetData((vx_type_e)weightsType, inputCount * i + j, (vx_uint8_ptr)weightsBaseLogicalAddr, weightFpPos);

                madValue += inputValue * weightValue;
            }
            else
            {
                /* other format not surpport now */
                printf("can't support this input data format\n");
                gcmASSERT(0);
            }
        }

        if (biasesType == VX_TYPE_FLOAT32 || biasesType == VX_TYPE_INT32)
        {
            biasValue = vxnneGetData((vx_type_e)biasesType, i, (vx_uint8_ptr)biasesBaseLogicalAddr, biasFpPos);
        }
        else
        {
            printf("can't support this bias data format\n");
            gcmASSERT(0);
        }

        result = madValue + biasValue;

        vxnneSaveData((vx_type_e)dstType, i, result, outputsBaseLogicalAddr, outputFpPos, outputRoundingMode);
    }

    return status;
}

vx_status vxnneExecuteFullyConnectReluLayer(struct _vxnne_operation_s *operation)
{
    vxnne_fully_connected_relu_layer         layer                         = (vxnne_fully_connected_relu_layer)operation->layer;
    vxnne_fully_connected_relu_nne_operation fullyConnectedReluOperation   = (vxnne_fully_connected_relu_nne_operation)operation;

    vx_tensor inputs  = fullyConnectedReluOperation->inputs;
    vx_weights_biases_parameter weights_biases = fullyConnectedReluOperation->weights_biases;
    vx_tensor outputs = fullyConnectedReluOperation->outputs;

    vx_node node = layer->base.node;

    if (weights_biases->use_fc_accel)
    {
        vx_uint32 tmpPhysical;
        vx_uint8_ptr tmpLogical;

        /* interchange weight_bias and input address */
        tmpPhysical = weights_biases->memory.physicals[0];
        weights_biases->memory.physicals[0] = inputs->tensorBuffer->memory.physicals[0];
        inputs->tensorBuffer->memory.physicals[0] = tmpPhysical;

        tmpLogical = weights_biases->memory.logicals[0];
        weights_biases->memory.logicals[0] = inputs->tensorBuffer->memory.logicals[0];
        inputs->tensorBuffer->memory.logicals[0] = tmpLogical;

        vxNNExecute(node, layer->base.cmdNNBuff, 0, weights_biases, 0, inputs, 0, outputs, 0);

        /* restore weight_bias and input address */
        tmpPhysical = weights_biases->memory.physicals[0];
        weights_biases->memory.physicals[0] = inputs->tensorBuffer->memory.physicals[0];
        inputs->tensorBuffer->memory.physicals[0] = tmpPhysical;

        tmpLogical = weights_biases->memory.logicals[0];
        weights_biases->memory.logicals[0] = inputs->tensorBuffer->memory.logicals[0];
        inputs->tensorBuffer->memory.logicals[0] = tmpLogical;
    }
    else
    {
        vx_uint32 i, wbOffset, outputOffset = 0;

        for (i = 0; i < weights_biases->zgroup_num; i++)
        {
            wbOffset = (vx_uint32)weights_biases->memory_offset_array[i];

            if (weights_biases != VX_NULL && node->base.context->options.enableNNArchPerfPrint == 1)
            {
                vxoWeightsBiasesParameter_ShowPerformance(node->base.context, weights_biases);
            }

            /* NN Engine programming. */
            vxNNExecute(node, layer->base.cmdNNBuff, i*NNE_COMMAND_SIZE, weights_biases, wbOffset, inputs, 0, outputs, outputOffset);

            outputOffset += outputs->finalDims[0] * outputs->finalDims[1] * outputs->tensorBuffer->elementSize * weights_biases->zgroup_array[i];
        }
    }

    return VX_SUCCESS;
}

vx_status vxnneExecuteTPFullyConnectReluLayer(struct _vxnne_operation_s *operation)
{
    vxnne_fully_connected_relu_layer layer = (vxnne_fully_connected_relu_layer)operation->layer;
    vxnne_tp_operation fullyConnectedReluOperation = (vxnne_tp_operation)operation;

    vx_node node = layer->base.node;
    vx_tensor* inputs  = fullyConnectedReluOperation->inputs;
    vx_weights_biases_parameter weights_biases = fullyConnectedReluOperation->weights_biases;
    vx_tensor* outputs = fullyConnectedReluOperation->outputs;

    return vxnneExecuteTPGeneric(node, inputs, weights_biases, VX_NULL, outputs, layer->base.cmdTPBuff, 0, fullyConnectedReluOperation->multi_tensor);
}

vx_status vxnneExecuteSWInputConvertWeight(struct _vxnne_operation_s *operation)
{
    vxnne_fully_connected_relu_layer layer = (vxnne_fully_connected_relu_layer)operation->layer;
    vxnne_intput_convert_weight_operation input2weightOperation = (vxnne_intput_convert_weight_operation)operation;

    vx_bool enableRelu = input2weightOperation->enable_relu;
    vx_tensor inputs  = input2weightOperation->inputs;
    vx_weights_biases_parameter weights_biases = input2weightOperation->weights_biases;
    vx_tensor outputs = input2weightOperation->outputs;
    vx_uint8 origFiexedPointPos, newFiexedPointPos = input2weightOperation->output_fp_pos;

    vx_context context = vxGetContext((vx_reference)weights_biases);
    vx_uint32 nnCoreCount = context->nnConfig.nnCoreCount;

    vx_uint32 i, j, k = 0, ySize, sliceSize, elementSize = (vx_uint32)vxDataType_GetSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
    vx_float32 halfOneF=1.0f;
    vx_uint8 *inputBufferPtr;
    vx_uint8 *wbOrigBufferPtr, *wbCurBufferPtr;

    weights_biases->fc_accel_large_size = vx_false_e;
    vxoTensor_GetTensorViewMemory(inputs, (gctPOINTER*)&inputBufferPtr, VX_NULL);
    wbOrigBufferPtr =(vx_uint8*) weights_biases->tmp_fcaccel_wb_ptr;
    wbCurBufferPtr = (vx_uint8*) weights_biases->memory.logicals[0];
    sliceSize = weights_biases->org_weights_sizes[3] * elementSize;

    {
        vx_uint32 weightSize = elementSize;
        vx_uint32 weightBitSize = weightSize * 8;
        vx_uint32 biasSize = (vx_uint32)vxDataType_GetSize(VX_TYPE_FLOAT32);
        vx_uint32 biasBitSize = weights_biases->weights_data_format != VX_TYPE_INT8 ? biasSize * 8 :
                                gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7) ? NN_INTEGER_BIAS_BITS_VIP_V7 : NN_INTEGER_BIAS_BITS;

        vx_uint32* kernelBufferPtr = (vx_uint32*)outputs->tensorBuffer->memory.logicals[0];
        vx_uint32 bitOffset = 0, streamSizeBitOffset = 0, alignedOffset = ((vx_uint32)(gctUINTPTR_T)kernelBufferPtr) & 0x3F;
        vx_uint32 kernelStreamSize = 0;
        vx_uint32 *kernelBaseBufferPtr, *kernelSizeBufferPtr = kernelBufferPtr;

        vx_bool needInsertBias = vx_true_e, needWB = vx_true_e;
        vx_uint32 weightData, biasData = 0;

        /* TODO: if core > 16, need more head size. */
        writeBits(&kernelBufferPtr, &bitOffset, kernelStreamSize, 32);
        packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);

        while (nnCoreCount--)
        {
            kernelBaseBufferPtr = kernelBufferPtr;
            writeBits(&kernelBufferPtr, &bitOffset, 0, 8);
            writeBits(&kernelBufferPtr, &bitOffset, needWB?1:0, 16);

            if (needWB && weights_biases->org_weights_sizes[2] <= weights_biases->base.context->options.fcZMax)
            {
                for (i = 0; i < weights_biases->org_weights_sizes[2]; i++, inputBufferPtr+=elementSize)
                {
                    if (weightSize == 1)
                        weightData = *((vx_int8 *)inputBufferPtr);
                    else
                        weightData = *((vx_uint16 *)inputBufferPtr);

                    if (weightData)
                    {
                        writeBits(&kernelBufferPtr, &bitOffset, weightData, weightBitSize);
                        if (needInsertBias)
                        {
                            writeBits(&kernelBufferPtr, &bitOffset, biasData, biasBitSize);
                            needInsertBias = vx_false_e;
                        }
                        k++;

                        memcpy(wbCurBufferPtr, wbOrigBufferPtr, sliceSize);
                        wbCurBufferPtr += sliceSize;
                    }
                    wbOrigBufferPtr += sliceSize;
                }
            }
            else if (needWB)
            {
                weights_biases->fc_accel_large_size = vx_true_e;
                ySize = weights_biases->weights_sizes[1] * elementSize;
                sliceSize = weights_biases->weights_sizes[1] * weights_biases->org_weights_sizes[3] * elementSize;

                for (i = 0; i < weights_biases->weights_sizes[2]; i++, inputBufferPtr+=ySize)
                {
                    vx_uint8* inputBufferPtrTmp = inputBufferPtr;
                    for (j = 0; j < weights_biases->weights_sizes[1]; j++, inputBufferPtrTmp+=elementSize)
                    {
                        if (weightSize == 1)
                            weightData = *((vx_int8 *)inputBufferPtrTmp);
                        else
                            weightData = *((vx_uint16 *)inputBufferPtrTmp);
                        if (weightData) break;
                    }

                    if (j < weights_biases->weights_sizes[1])
                    {
                        inputBufferPtrTmp = inputBufferPtr;

                        for (j = 0; j < weights_biases->weights_sizes[1]; j++, inputBufferPtrTmp+=elementSize)
                        {
                            if (weightSize == 1)
                                weightData = *((vx_int8 *)inputBufferPtrTmp);
                            else
                                weightData = *((vx_uint16 *)inputBufferPtrTmp);

                            writeBits(&kernelBufferPtr, &bitOffset, weightData, weightBitSize);

                            if (needInsertBias)
                            {
                                writeBits(&kernelBufferPtr, &bitOffset, biasData, biasBitSize);
                                needInsertBias = vx_false_e;
                            }
                        }

                        k++;

                        memcpy(wbCurBufferPtr, wbOrigBufferPtr, sliceSize);
                        wbCurBufferPtr += sliceSize;
                    }

                    wbOrigBufferPtr += sliceSize;
                }
            }

            if (needWB)
            {
                vxnneSaveData((vx_type_e)TENSOR_DATA_TYPE(inputs), 0, halfOneF, &weightData, inputs->tensorBuffer->fixedPointPos, inputs->tensorBuffer->roundingMode);
                writeBits(&kernelBufferPtr, &bitOffset, weightData, weightBitSize);
                k++;

                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                    writeBits(&kernelBufferPtr, &bitOffset, 0, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
                else
                    writeBits(&kernelBufferPtr, &bitOffset, 0, NN_Z_POSITION_OFFSET_BITS);
            }

            packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);
            kernelStreamSize = (vx_uint32)((vx_uint8*)kernelBufferPtr - (vx_uint8*)kernelBaseBufferPtr);
            writeBits(&kernelSizeBufferPtr, &streamSizeBitOffset, kernelStreamSize, 32);

            needWB = vx_false_e;
        }

        memcpy(wbCurBufferPtr, wbOrigBufferPtr, sliceSize);
    }

    weights_biases->input_nonzero_count = k;

    origFiexedPointPos = outputs->tensorBuffer->fixedPointPos;
    outputs->tensorBuffer->fixedPointPos = newFiexedPointPos;

    fillinCmmdBuff(
        inputs,
        weights_biases,
        0, 0, 0, 0, VX_PAD_CONSTANT, VX_NULL,
        VX_CONVOLUTIONAL_NETWORK_DS_SIZE_ROUNDING_FLOOR,
        enableRelu,
        VIV_NN_NONLINEAR_NON,
        0, 0,
        outputs,
        layer->base.cmdNNBuff,
        vx_true_e,
        0);

    outputs->tensorBuffer->fixedPointPos = origFiexedPointPos;

    return VX_SUCCESS;
}

vx_status vxnneExecuteSWActivation(struct _vxnne_operation_s *operation)
{
    vxnne_activation_sw_operation           activationOperation   = (vxnne_activation_sw_operation)operation;

    vx_tensor inputs  = (vx_tensor)activationOperation->inputs;
    vx_scalar func = (vx_scalar)activationOperation->func;
    vx_scalar a  = (vx_scalar)activationOperation->a;
    vx_scalar b = (vx_scalar)activationOperation->b;
    vx_tensor outputs = (vx_tensor)activationOperation->outputs;

    vx_enum   func_v = func->value->e;
    vx_int32  a_v = a->value->n32;
    vx_int32  b_v = b->value->n32;

    vx_uint32 elementCount = 0;
    vx_uint32 i;
    vx_float32 value = 0.0f, result = 0.0f;
    gctPOINTER inputBase;
    gctPOINTER outputBase;

    vx_type_e inputFormat = (vx_type_e)inputs->tensorBuffer->dataFormat;
    vx_type_e outputFormat = (vx_type_e)outputs->tensorBuffer->dataFormat;
    vx_uint8 inputFpPos = inputs->tensorBuffer->fixedPointPos;
    vx_uint8 outputFpPos = outputs->tensorBuffer->fixedPointPos;
    vx_enum outputRoundingMode = outputs->tensorBuffer->roundingMode;

    vx_status status = VX_SUCCESS;

    elementCount = (vx_uint32)vxoMemory_ComputeElementCount(&inputs->tensorBuffer->memory, 0);
    vxoTensor_GetTensorViewMemory(inputs, &inputBase, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, &outputBase, VX_NULL);

    if ((inputs->tensorBuffer->dataFormat != VX_TYPE_FLOAT16 && inputs->tensorBuffer->dataFormat != VX_TYPE_FLOAT32 && inputs->tensorBuffer->dataFormat != VX_TYPE_INT8)
        || (outputs->tensorBuffer->dataFormat != VX_TYPE_FLOAT16 && outputs->tensorBuffer->dataFormat != VX_TYPE_FLOAT32 && outputs->tensorBuffer->dataFormat != VX_TYPE_INT8))
    {
        gcmPRINT("input or outputs format is not support");
        status = VX_ERROR_NOT_SUPPORTED;
        return status;
    }

    for (i = 0; i < elementCount; i++)
    {
        value = vxnneGetData(inputFormat, i, (vx_uint8_ptr)inputBase, inputFpPos);

        switch (func_v)
        {
        case VX_CONVOLUTIONAL_NETWORK_ACTIVATION_LOGISTIC:
            {
                result = 1.0f / (1 + gcoMATH_Exp(value * (-1)));
            }
            break;

        case VX_CONVOLUTIONAL_NETWORK_ACTIVATION_HYPERBOLIC_TAN:
            {
                result = a_v * gcoMATH_Tangent(b_v * value);
            }
            break;

        case VX_CONVOLUTIONAL_NETWORK_ACTIVATION_RELU:
            {
                result = gcoMATH_MAX(0.0f, value);
            }
            break;

        case VX_CONVOLUTIONAL_NETWORK_ACTIVATION_BRELU:
            {
                result = gcoMATH_MIN(a_v, gcoMATH_MAX(0.0f, value));
            }
            break;

        case VX_CONVOLUTIONAL_NETWORK_ACTIVATION_SOFTRELU:
            {
                result = gcoMATH_Log(1 + gcoMATH_Exp(value));
            }
            break;

        case VX_CONVOLUTIONAL_NETWORK_ACTIVATION_ABS:
            {
                result = gcoMATH_Absolute(value);
            }
            break;

        case VX_CONVOLUTIONAL_NETWORK_ACTIVATION_SQUARE:
            {
                result = gcoMATH_Power(value, 2);
            }
            break;

        case VX_CONVOLUTIONAL_NETWORK_ACTIVATION_SQRT:
            {
                result = gcoMATH_SquareRoot(value);
            }
            break;

        case VX_CONVOLUTIONAL_NETWORK_ACTIVATION_LINEAR:
            {
                result = a_v * value + b_v;
            }
            break;

        case VX_CONVOLUTIONAL_NETWORK_ACTIVATION_LEAKYRELU:
            {
                result = (value > 0.0f) ? value : 0.1f * value;
            }
            break;

        default:
            gcmPRINT("this activation func not support");
            status = VX_ERROR_NOT_SUPPORTED;
            return status;
        }

        vxnneSaveData(outputFormat, i, result, outputBase, outputFpPos, outputRoundingMode);
    }

    return status;

}

vx_status vxnneExecuteSWSoftmax(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_softmax_operation softmaxOperation   = (vxnne_softmax_operation)operation;

    vx_tensor input     = (vx_tensor)softmaxOperation->inputs;
    vx_tensor output    = (vx_tensor)softmaxOperation->outputs;

    vx_type_e input_format  = (vx_type_e)input->tensorBuffer->dataFormat;
    vx_type_e output_format = (vx_type_e)output->tensorBuffer->dataFormat;
    vx_int8 input_fp        = input->tensorBuffer->fixedPointPos;
    vx_int8 output_fp       = output->tensorBuffer->fixedPointPos;
    vx_enum outputRMode     = output->tensorBuffer->roundingMode;

    vx_uint32 width,height,channel,batch,dims;
    vx_uint32 i,b,c,index;
    vx_uint32 Dim,ItemCount;
    vx_float32_ptr p_pfProSum, pfProSum = NULL;
    vx_float32_ptr p_pfMax, pfMax = NULL;
    vx_float32_ptr p_pfProbFP32, pfProbFP32 = VX_NULL;
    vx_float32_ptr p_pfInput, pfInput = VX_NULL;
    vx_uint8_ptr input_data_ptr = NULL;
    vx_uint8_ptr output_data_ptr = NULL;

    vxoTensor_GetTensorViewMemory(input, (gctPOINTER *)&input_data_ptr, VX_NULL);
    vxoTensor_GetTensorViewMemory(output, (gctPOINTER *)&output_data_ptr, VX_NULL);

    dims = TENSOR_DIM_NUM(input);
    switch(dims)
    {
        case 1:
            channel = TENSOR_VIEW_SIZE_INDEX(input, 0);
            batch   = 1;
            width   = 1;
            height  = 1;
            break;
        case 2:
            channel = TENSOR_VIEW_SIZE_INDEX(input, 0);
            batch   = TENSOR_VIEW_SIZE_INDEX(input, 1);
            width   = 1;
            height  = 1;
            break;
        case 3:
            width   = TENSOR_VIEW_SIZE_INDEX(input, 0);
            height  = TENSOR_VIEW_SIZE_INDEX(input, 1);
            channel = TENSOR_VIEW_SIZE_INDEX(input, 2);
            batch   = 1;
            break;
        case 4:
            width   = TENSOR_VIEW_SIZE_INDEX(input, 0);
            height  = TENSOR_VIEW_SIZE_INDEX(input, 1);
            channel = TENSOR_VIEW_SIZE_INDEX(input, 2);
            batch   = TENSOR_VIEW_SIZE_INDEX(input, 3);
            break;
        default:
            printf("Input tensor error dimension[%u]\n", dims);
            return VX_ERROR_INVALID_DIMENSION;
    }

    ItemCount = width * height;
    Dim       = (batch * channel * width * height) / batch; /* default axis = 3, so softmax it in channel */

    pfMax       = (vx_float32_ptr)malloc(batch * ItemCount * sizeof(vx_float32));
    pfProSum    = (vx_float32_ptr)malloc(batch * ItemCount * sizeof(vx_float32));
    pfInput     = (vx_float32_ptr)malloc(batch * channel * ItemCount * sizeof(vx_float32));
    pfProbFP32  = (vx_float32_ptr)malloc(batch * channel * ItemCount * sizeof(vx_float32));
    memset(pfMax, 0, batch * ItemCount * sizeof(vx_float32));
    memset(pfProSum, 0, batch * ItemCount * sizeof(vx_float32));
    memset(pfInput, 0, batch * channel * ItemCount * sizeof(vx_float32));
    memset(pfProbFP32, 0, batch * channel * ItemCount * sizeof(vx_float32));

    index = 0;
    p_pfInput = pfInput;
    p_pfMax = pfMax;
    for(b = 0; b < batch; b++)
    {
        for(c = 0; c < channel; c++)
        {
            for(i = 0; i < ItemCount; i++)
            {
                index = b * Dim + c * ItemCount + i;
                p_pfInput[i] = vxnneGetData(input_format, index, (vx_uint8_ptr)input_data_ptr, input_fp);
                p_pfMax[i]   = gcmMAX(p_pfMax[i], p_pfInput[i]);
            }
            p_pfInput += ItemCount;
        }
        p_pfMax += ItemCount;
    }

    p_pfProbFP32 = pfProbFP32;
    p_pfInput = pfInput;
    p_pfMax = pfMax;
    p_pfProSum = pfProSum;
    for(b = 0; b < batch; b++)
    {
        for(c = 0; c < channel; c++)
        {
            for(i = 0; i < ItemCount; i++)
            {
                p_pfProbFP32[i] = gcoMATH_Exp(p_pfInput[i] - p_pfMax[i]);
            }
            p_pfProbFP32 += ItemCount;
            p_pfInput += ItemCount;
        }
    }

    p_pfProbFP32 = pfProbFP32;
    p_pfProSum = pfProSum;
    for(b = 0; b < batch; b++)
    {
        for(c = 0; c < channel; c++)
        {
            for(i = 0; i < ItemCount; i++)
            {
                index = b * Dim + c * ItemCount + i;
                p_pfProSum[i] += pfProbFP32[index];
            }
        }
        p_pfProSum += ItemCount;
    }

    p_pfProbFP32 = pfProbFP32;
    p_pfProSum = pfProSum;
    index = 0;
    for(b = 0; b < batch; b++)
    {
        for(c = 0; c < channel; c++)
        {
            for(i = 0; i < ItemCount; i++)
            {
                vx_float32 div = p_pfProbFP32[i] / p_pfProSum[i];


                vxnneSaveData(output_format, index++, div, (vx_uint8_ptr)output_data_ptr, output_fp, outputRMode);
            }
            p_pfProbFP32 += ItemCount;
        }
        p_pfProSum += ItemCount;
    }

    if(pfMax)free(pfMax);
    if(pfProSum)free(pfProSum);
    if(pfInput)free(pfInput);
    if(pfProbFP32)free(pfProbFP32);
    return status;
}

vx_status vxnneExecuteSWConcat2(struct _vxnne_operation_s *operation)
{
    vxnne_concat2_sw_operation           concatOperation   = (vxnne_concat2_sw_operation)operation;

    vx_tensor input0  = (vx_tensor)concatOperation->inputs0;
    vx_tensor input1  = (vx_tensor)concatOperation->inputs1;
    vx_tensor output = (vx_tensor)concatOperation->outputs;
    vx_type_e   input0_format = (vx_type_e)input0->tensorBuffer->dataFormat;
    vx_type_e   input1_format = (vx_type_e)input1->tensorBuffer->dataFormat;
    vx_type_e   output_format = (vx_type_e)output->tensorBuffer->dataFormat;
    vx_int8   src0FixPointPos  = input0->tensorBuffer->fixedPointPos;
    vx_int8   src1FixPointPos  = input1->tensorBuffer->fixedPointPos;
    vx_int8   dstFixPointPos  = output->tensorBuffer->fixedPointPos;
    vx_enum   dstRoundingMode = output->tensorBuffer->roundingMode;
    vx_uint32  m = 0;
    vx_uint32  index = 0;

    vx_uint8_ptr pInput0Buf = NULL;
    vx_uint8_ptr pInput1Buf = NULL;
    vx_uint8_ptr pOutputBuf = NULL;

    vx_uint32 input0Size = 0, input1Size = 0;

    vxoTensor_GetTensorViewMemory(input0, (gctPOINTER *)&pInput0Buf, VX_NULL);
    vxoTensor_GetTensorViewMemory(input1, (gctPOINTER *)&pInput1Buf, VX_NULL);
    vxoTensor_GetTensorViewMemory(output, (gctPOINTER *)&pOutputBuf, VX_NULL);

    input0Size = (vx_uint32)vxoMemory_ComputeSize(&input0->tensorBuffer->memory, 0);
    input1Size = (vx_uint32)vxoMemory_ComputeSize(&input1->tensorBuffer->memory, 0);

    if((input0_format == VX_TYPE_FLOAT16 && input1_format == VX_TYPE_FLOAT16 && output_format == VX_TYPE_FLOAT16)
    ||(input0_format == VX_TYPE_INT8 && input1_format == VX_TYPE_INT8 && output_format == VX_TYPE_INT8 && src0FixPointPos == src1FixPointPos && src0FixPointPos == dstFixPointPos))
    {
        vxMemCopy(pOutputBuf, pInput0Buf, input0Size);
        vxMemCopy(&pOutputBuf[input0Size], pInput1Buf, input1Size);
    }
    else
    {
        input0Size = input0Size / input0->tensorBuffer->elementSize;
        input1Size = input1Size / input1->tensorBuffer->elementSize;
        for (m = 0; m < input0Size; m ++, index ++)
        {
            vx_float32 src0 = vxnneGetData(input0_format, m, (vx_uint8_ptr)pInput0Buf, src0FixPointPos);
            vxnneSaveData(output_format, index, src0, (vx_uint8_ptr)pOutputBuf, dstFixPointPos, dstRoundingMode);
        }

        for (m = 0; m < input1Size; m ++, index ++)
        {
            vx_float32 src1 = vxnneGetData(input1_format, m, (vx_uint8_ptr)pInput1Buf, src1FixPointPos);
            vxnneSaveData(output_format, index, src1, (vx_uint8_ptr)pOutputBuf, dstFixPointPos, dstRoundingMode);
        }
    }

    return VX_SUCCESS;
}

vx_status vxnneExecuteSWTensorCopy(struct _vxnne_operation_s *operation)
{
    vxnne_tensor_copy_sw_operation           copyOperation   = (vxnne_tensor_copy_sw_operation)operation;

    vx_tensor src  = (vx_tensor)copyOperation->src;
    vx_tensor dst = (vx_tensor)copyOperation->dst;
    gctPOINTER srcLogical = VX_NULL;
    gctPOINTER dstLogical = VX_NULL;

    vx_uint32 dstSize;
    vx_uint32 sizes[VX_CONTEXT_TENSOR_MAX_DIMENSION];

    vxoTensor_GetTensorViewMemory(src, &srcLogical, VX_NULL);
    vxoTensor_GetTensorViewMemory(dst, &dstLogical, VX_NULL);

    dstSize = (vx_uint32)vxoTensor_GetTensorSize(dst, 0);

    memset(dstLogical,
           0,
           dstSize);

    if (src->isViewed)
    {
        vx_uint32 i;

        for (i = 0; i < src->viewRegion.dimCount; i++)
        {
            sizes[i] = src->viewRegion.viewEnds[i] - src->viewRegion.viewStarts[i];
        }
        vxoTensor_CopyTensorPatchEx(
            (vx_uint8_ptr)srcLogical,
            (vx_uint8_ptr)dstLogical,
            2,
            sizes,
            TENSOR_STRIDES(src),
            TENSOR_STRIDES(dst),
            src->tensorBuffer->dataFormat,
            dst->tensorBuffer->dataFormat
            );
    }
    else
    {
        vxoTensor_CopyTensorPatchEx(
            (vx_uint8_ptr)srcLogical,
            (vx_uint8_ptr)dstLogical,
            2,
            TENSOR_SIZES(src),
            TENSOR_STRIDES(src),
            TENSOR_STRIDES(dst),
            src->tensorBuffer->dataFormat,
            dst->tensorBuffer->dataFormat
            );
    }

    return VX_SUCCESS;
}

vx_status vxnneExecuteSWNormalization(struct _vxnne_operation_s *operation)
{
     vxnne_normalization_operation           normalizationOperation   = (vxnne_normalization_operation)operation;

    vx_tensor input  = (vx_tensor)normalizationOperation->inputs;
    vx_tensor output = (vx_tensor)normalizationOperation->outputs;

    void *inputsBase;
    void *outputsBase;

    vx_int32 dimCount = TENSOR_DIM_NUM(input);
    vx_int32 width    = TENSOR_SIZE_INDEX(input, 0);
    vx_int32 height   = TENSOR_SIZE_INDEX(input, 1);
    vx_int32 channel  = TENSOR_SIZE_INDEX(input, 2);
    vx_int32 batch    = TENSOR_SIZE_INDEX(input, 3);

    vx_int32   norm_size = normalizationOperation->norm_size;
    vx_float32 alpha     = normalizationOperation->alpha;
    vx_float32 beta      = normalizationOperation->beta;
    vx_int32   nsz2      = norm_size/2;
    vx_int32   type      = normalizationOperation->type;
    vx_int32   w=0,h=0,c=0,n=0,b=0,i=0,j=0;
    vx_type_e  inputFormat   = (vx_type_e)input->tensorBuffer->dataFormat;
    vx_type_e  outputFormat  = (vx_type_e)output->tensorBuffer->dataFormat;
    vx_uint8   inputFpPos    = input->tensorBuffer->fixedPointPos;
    vx_uint8   outputFpPos   = output->tensorBuffer->fixedPointPos;
    vx_enum    outputRoundingMode = output->tensorBuffer->roundingMode;
    vx_int32   inputStridec  = TENSOR_STRIDE_INDEX(input, 2)/(vx_int32)vxDataType_GetSize(inputFormat);
    vx_int32   outputStridec = TENSOR_STRIDE_INDEX(output, 2)/(vx_int32)vxDataType_GetSize(outputFormat);

    vxoTensor_GetTensorViewMemory(input, (gctPOINTER *)&inputsBase, VX_NULL);
    vxoTensor_GetTensorViewMemory(output, (gctPOINTER *)&outputsBase, VX_NULL);

    if(dimCount==3)
        batch = 1;

    if(type == VX_CONVOLUTIONAL_NETWORK_NORM_SAME_MAP)
    {
        for(b = 0; b < batch; b++)
        {
            for(c = 0; c < channel; c++)
            {
                 for(h=0;h<height;h++)
                 {
                    for(w=0;w<width;w++)
                    {
                        vx_float32 sum=0;
                        vx_float32 val;
                         for(j = h-nsz2;j <= h+nsz2;j++)
                         {
                            for(i = w-nsz2;i <= w+nsz2;i++)
                            {
                               if(j>=0 && i>=0 && j<height && i<width)
                               {
                                   val = vxnneGetData(inputFormat, (c+b*channel)*inputStridec + width*j + i, (vx_uint8_ptr)inputsBase, inputFpPos);
                                   sum += val*val;
                               }
                            }
                         }

                         val = vxnneGetData(inputFormat, (c+b*channel)*inputStridec + width*h + w, (vx_uint8_ptr)inputsBase, inputFpPos);
                         val = val/(vx_float32)pow((1+(alpha/(norm_size*norm_size))*sum),beta);
                         vxnneSaveData(outputFormat, (c+b*channel)*outputStridec + width*h + w, val, (vx_uint8_ptr)outputsBase, outputFpPos, outputRoundingMode);
                    }
                 }
            }
        }
    }
    else
    {
        for(b = 0; b < batch; b++)
        {
            for(c = 0; c < channel; c++)
            {
                 for(h=0;h<height;h++)
                 {
                    for(w=0;w<width;w++)
                    {
                        vx_float32 sum=0;
                        vx_float32 val;
                        for(n = c-nsz2; n <= c+nsz2; n++)
                        {
                            if(n<0 || n >= channel)
                                continue;
                            val = vxnneGetData(inputFormat, (n+b*channel)*inputStridec + width*h + w, (vx_uint8_ptr)inputsBase, inputFpPos);
                            sum += val*val;
                        }
                        val = vxnneGetData(inputFormat, (c+b*channel)*inputStridec + width*h + w, (vx_uint8_ptr)inputsBase, inputFpPos);
                        val =val/(vx_float32)pow((1+(alpha/norm_size)*sum),beta);
                        vxnneSaveData(outputFormat, (c+b*channel)*outputStridec + width*h + w, val, (vx_uint8_ptr)outputsBase, outputFpPos, outputRoundingMode);
                    }
                 }
            }
        }
    }

    return VX_SUCCESS;

}

vx_status vxnneExecuteSWBrickMode(struct _vxnne_operation_s *operation)
{
    vxnne_brick_operation           brickModeOperation   = (vxnne_brick_operation)operation;

    vx_tensor inputs = (vx_tensor)brickModeOperation->inputs;
    vx_tensor outputs = (vx_tensor)brickModeOperation->outputs;
    vx_uint32 kernel_x, kernel_y;
    vx_uint32 padXLeft;
    vx_uint32 padXRight;
    vx_uint32 padYTop;
    vx_uint32 padYBottom;
    vx_uint32 inImageTileSizeX, inImageTileSizeY;
    vx_uint32 numOfImageTileX, numOfImageTileY;
    vx_uint32 distSize;
    vx_uint32 stride;
    vx_uint32 input_width, input_height, input_z, input_b;
    vx_uint32 i, j, x, y, z;
    vx_uint32 outTileX, outTileY;
    vx_uint8_ptr temp_buffer = VX_NULL;
    vx_context context = vxGetContext((vx_reference)inputs);

    padXLeft   = brickModeOperation->pad_x_left;
    padXRight  = brickModeOperation->pad_x_right;
    padYTop    = brickModeOperation->pad_y_top;
    padYBottom = brickModeOperation->pad_y_bottom;

    kernel_x = brickModeOperation->kernel_x;
    kernel_y = brickModeOperation->kernel_y;
    outTileX = brickModeOperation->outTileX;
    outTileY = brickModeOperation->outTileY;

    if (inputs->isViewed)
    {
        input_width = inputs->viewRegion.viewEnds[0] - inputs->viewRegion.viewStarts[0];
        input_height = inputs->viewRegion.viewEnds[1] - inputs->viewRegion.viewStarts[1];
        input_z = inputs->viewRegion.viewEnds[2] - inputs->viewRegion.viewStarts[2];
    }
    else
    {
        input_width = TENSOR_SIZE_INDEX(inputs, 0);
        input_height = TENSOR_SIZE_INDEX(inputs, 1);
        input_z = TENSOR_SIZE_INDEX(inputs, 2);
    }
    input_b = TENSOR_SIZE_INDEX(inputs, 3);

    stride = brickModeOperation->stride;

    if (stride > 1)
    {
        input_width /= stride;
        input_height /= stride;
        input_z = input_z * stride * stride;
    }

    inImageTileSizeX = outTileX - 1 + kernel_x;
    inImageTileSizeY = outTileY - 1 + kernel_y;

    distSize = inImageTileSizeX * inImageTileSizeY * input_z * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
    numOfImageTileX = brickModeOperation->num_tile_x;
    numOfImageTileY = brickModeOperation->num_tile_y;

    temp_buffer = (vx_uint8_ptr)vxAllocateAndZeroMemory(distSize * numOfImageTileX * numOfImageTileY);

    if (context->options.enableTP)
        gcfVX_Flush(gcvTRUE);

    if (padXLeft != 0 || padXRight != 0 || padYBottom != 0 || padYTop != 0)
    {
        for (i = 0; i < numOfImageTileY; i++)
        {
            for (j = 0; j < numOfImageTileX; j++)
            {
                vx_uint32 offsetDist = (j + i * numOfImageTileX) * distSize;

                for (z = 0; z < input_z; z++)
                {
                    vx_uint32 dstX = 0, dstY = 0;
                    vx_int32 inX = 0, inY = 0;

                    /* (Top, Left) */
                    if (i == 0 && j == 0)
                    {
                        vx_uint32 tempX = inImageTileSizeX - padXLeft;
                        vx_uint32 tempY = inImageTileSizeY - padYTop;

                        if (tempX > input_width)
                            tempX = input_width;
                        if (tempY > input_height)
                            tempY = input_height;

                        for (y = 0; y < tempY; y++)
                        {

                            if (dstY >= tempY || dstY >= input_height)
                                dstY = 0;

                            for (x = 0; x < tempX; x++)
                            {

                                vx_uint32 srcOffsetSize, dstOffserSize;
                                vx_uint8_ptr input_ptr = VX_NULL, output_ptr = VX_NULL;

                                if (dstX >= tempX || dstX >= input_width)
                                    dstX = 0;

                                inX = x + j * outTileX;
                                inY = y + i * outTileY;
                                /* Skip out of border value */
                                if (inX < 0 || inY < 0 || inX >= (vx_int32)input_width || inY >= (vx_int32)input_height)
                                    continue;

                                if (inputs->isViewed)
                                    srcOffsetSize = ((inX + inputs->viewRegion.viewStarts[0]) + input_width * ((inY + inputs->viewRegion.viewStarts[1]) + input_height * (z + inputs->viewRegion.viewStarts[2]))) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);

                                else
                                    srcOffsetSize = (inX + input_width * (inY + input_height * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                dstOffserSize = (dstX + tempX * (dstY + tempY * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                input_ptr = inputs->tensorBuffer->memory.logicals[0] + srcOffsetSize;
                                output_ptr = temp_buffer + dstOffserSize + offsetDist;

                                if (inputs->tensorBuffer->dataFormat == VX_TYPE_FLOAT16)
                                    *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
                                else if (inputs->tensorBuffer->dataFormat == VX_TYPE_INT8)
                                    *(vx_int8*)output_ptr = *(vx_int8*)input_ptr;
                                else
                                    *(vx_int32*)output_ptr = *(vx_int32*)input_ptr;
                                dstX++;
                            }
                            if (inY < 0)
                                continue;
                            dstY++;
                        }
                    }
                    /* (Bottom, Right) */
                    else if (i == numOfImageTileY - 1 && j == numOfImageTileX - 1)
                    {
                        vx_uint32 tempX = input_width - (numOfImageTileX - 1) * outTileX + padXLeft;
                        vx_uint32 tempY = input_height - (numOfImageTileY - 1) * outTileY + padYTop;

                        if (tempX > input_width)
                            tempX = input_width;
                        if (tempY > input_height)
                            tempY = input_height;

                        for (y = 0; y < tempY; y++)
                        {

                            if (dstY >= tempY || dstY >= input_height)
                                dstY = 0;

                            for (x = 0; x < tempX; x++)
                            {

                                vx_uint32 srcOffsetSize, dstOffserSize;
                                vx_uint8_ptr input_ptr = VX_NULL, output_ptr = VX_NULL;

                                if (dstX >= tempX || dstX >= input_width)
                                    dstX = 0;

                                if (j != 0)
                                    inX = x + j * outTileX - padXLeft;
                                else
                                    inX = x;
                                inY = y + i * outTileY - padYTop;
                                /* Skip out of border value */
                                if (inX < 0 || inY < 0 || inX >= (vx_int32)input_width || inY >= (vx_int32)input_height)
                                    continue;

                                if (inputs->isViewed)
                                    srcOffsetSize = ((inX + inputs->viewRegion.viewStarts[0]) + input_width * ((inY + inputs->viewRegion.viewStarts[1]) + input_height * (z + inputs->viewRegion.viewStarts[2]))) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);

                                else
                                    srcOffsetSize = (inX + input_width * (inY + input_height * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                dstOffserSize = (dstX + tempX * (dstY + tempY * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                input_ptr = inputs->tensorBuffer->memory.logicals[0] + srcOffsetSize;
                                output_ptr = temp_buffer + dstOffserSize + offsetDist;

                                if (inputs->tensorBuffer->dataFormat == VX_TYPE_FLOAT16)
                                    *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
                                else if (inputs->tensorBuffer->dataFormat == VX_TYPE_INT8)
                                    *(vx_int8*)output_ptr = *(vx_int8*)input_ptr;
                                else
                                    *(vx_int32*)output_ptr = *(vx_int32*)input_ptr;
                                dstX++;
                            }
                            if (inY < 0)
                                continue;
                            dstY++;
                        }
                    }
                    /* (Bottom, Left) */
                    else if (i == numOfImageTileY - 1 && j == 0)
                    {
                        vx_uint32 tempX = inImageTileSizeX - padXLeft;
                        vx_uint32 tempY = input_height - (numOfImageTileY - 1) * outTileY + padYTop;

                        if (tempX > input_width)
                            tempX = input_width;
                        if (tempY > input_height)
                            tempY = input_height;

                        for (y = 0; y < tempY; y++)
                        {

                            if (dstY >= tempY || dstY >= input_height)
                                dstY = 0;

                            for (x = 0; x < tempX; x++)
                            {

                                vx_uint32 srcOffsetSize, dstOffserSize;
                                vx_uint8_ptr input_ptr = VX_NULL, output_ptr = VX_NULL;

                                if (dstX >= tempX || dstX >= input_width)
                                    dstX = 0;

                                inX = x + j * outTileX;
                                inY = y + i * outTileY - padYTop;
                                /* Skip out of border value */
                                if (inX < 0 || inY < 0 || inX >= (vx_int32)input_width || inY >= (vx_int32)input_height)
                                    continue;

                                if (inputs->isViewed)
                                    srcOffsetSize = ((inX + inputs->viewRegion.viewStarts[0]) + input_width * ((inY + inputs->viewRegion.viewStarts[1]) + input_height * (z + inputs->viewRegion.viewStarts[2]))) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);

                                else
                                    srcOffsetSize = (inX + input_width * (inY + input_height * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                dstOffserSize = (dstX + tempX * (dstY + tempY * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                input_ptr = inputs->tensorBuffer->memory.logicals[0] + srcOffsetSize;
                                output_ptr = temp_buffer + dstOffserSize + offsetDist;

                                if (inputs->tensorBuffer->dataFormat == VX_TYPE_FLOAT16)
                                    *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
                                else if (inputs->tensorBuffer->dataFormat == VX_TYPE_INT8)
                                    *(vx_int8*)output_ptr = *(vx_int8*)input_ptr;
                                else
                                    *(vx_int32*)output_ptr = *(vx_int32*)input_ptr;
                                dstX++;
                            }
                            if (inY < 0)
                                continue;
                            dstY++;
                        }
                    }
                    /* (Top, Right) */
                    else if (j == numOfImageTileX - 1 && i == 0)
                    {
                        vx_uint32 tempX = input_width - (numOfImageTileX - 1) * outTileX + padXLeft;
                        vx_uint32 tempY = inImageTileSizeY - padYTop;

                        if (tempX > input_width)
                            tempX = input_width;
                        if (tempY > input_height)
                            tempY = input_height;

                        for (y = 0; y < tempY; y++)
                        {

                            if (dstY >= tempY || dstY >= input_height)
                                dstY = 0;

                            for (x = 0; x < tempX; x++)
                            {

                                vx_uint32 srcOffsetSize, dstOffserSize;
                                vx_uint8_ptr input_ptr = VX_NULL, output_ptr = VX_NULL;

                                if (dstX >= tempX || dstX >= input_width)
                                    dstX = 0;

                                inX = x + j * outTileX - padXLeft;
                                inY = y + i * outTileY;
                                /* Skip out of border value */
                                if (inX < 0 || inY < 0 || inX >= (vx_int32)input_width || inY >= (vx_int32)input_height)
                                    continue;

                                if (inputs->isViewed)
                                    srcOffsetSize = ((inX + inputs->viewRegion.viewStarts[0]) + input_width * ((inY + inputs->viewRegion.viewStarts[1]) + input_height * (z + inputs->viewRegion.viewStarts[2]))) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);

                                else
                                    srcOffsetSize = (inX + input_width * (inY + input_height * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                dstOffserSize = (dstX + tempX * (dstY + tempY * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                input_ptr = inputs->tensorBuffer->memory.logicals[0] + srcOffsetSize;
                                output_ptr = temp_buffer + dstOffserSize + offsetDist;

                                if (inputs->tensorBuffer->dataFormat == VX_TYPE_FLOAT16)
                                    *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
                                else if (inputs->tensorBuffer->dataFormat == VX_TYPE_INT8)
                                    *(vx_int8*)output_ptr = *(vx_int8*)input_ptr;
                                else
                                    *(vx_int32*)output_ptr = *(vx_int32*)input_ptr;
                                dstX++;
                            }
                            if (inY < 0)
                                continue;
                            dstY++;
                        }
                    }
                    /* Other Top */
                    else if (i == 0)
                    {
                        vx_uint32 tempY = inImageTileSizeY - padYTop;

                        if (tempY > input_height)
                            tempY = input_height;

                        for (y = 0; y < tempY; y++)
                        {

                            if (dstY >= tempY || dstY >= input_height)
                                    dstY = 0;

                            for (x = 0; x < inImageTileSizeX; x++)
                            {

                                vx_uint32 srcOffsetSize, dstOffserSize;
                                vx_uint8_ptr input_ptr = VX_NULL, output_ptr = VX_NULL;

                                if (dstX >= inImageTileSizeX || dstX >= input_width)
                                    dstX = 0;

                                inX = x + j * outTileX - padXLeft;
                                inY = y + i * outTileY;
                                /* Skip out of border value */
                                if (inX < 0 || inY < 0 || inX >= (vx_int32)input_width || inY >= (vx_int32)input_height)
                                    continue;

                                if (inputs->isViewed)
                                    srcOffsetSize = ((inX + inputs->viewRegion.viewStarts[0]) + input_width * ((inY + inputs->viewRegion.viewStarts[1]) + input_height * (z + inputs->viewRegion.viewStarts[2]))) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);

                                else
                                    srcOffsetSize = (inX + input_width * (inY + input_height * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                dstOffserSize = (dstX + inImageTileSizeX * (dstY + tempY * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                input_ptr = inputs->tensorBuffer->memory.logicals[0] + srcOffsetSize;
                                output_ptr = temp_buffer + dstOffserSize + offsetDist;

                                if (inputs->tensorBuffer->dataFormat == VX_TYPE_FLOAT16)
                                    *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
                                else if (inputs->tensorBuffer->dataFormat == VX_TYPE_INT8)
                                    *(vx_int8*)output_ptr = *(vx_int8*)input_ptr;
                                else
                                    *(vx_int32*)output_ptr = *(vx_int32*)input_ptr;
                                dstX++;
                            }
                            if (inY < 0)
                                continue;
                            dstY++;
                        }
                    }
                    /* Other Bottom */
                    else if (i == numOfImageTileY - 1)
                    {
                        vx_uint32 tempY = input_height - (numOfImageTileY - 1) * outTileY + padYTop;

                        if (tempY > input_height)
                            tempY = input_height;

                        for (y = 0; y < tempY; y++)
                        {

                            if (dstY >= tempY || dstY >= input_height)
                                    dstY = 0;

                            for (x = 0; x < inImageTileSizeX; x++)
                            {

                                vx_uint32 srcOffsetSize, dstOffserSize;
                                vx_uint8_ptr input_ptr = VX_NULL, output_ptr = VX_NULL;

                                if (dstX >= inImageTileSizeX || dstX >= input_width)
                                    dstX = 0;

                                inX = x + j * outTileX - padXLeft;
                                inY = y + i * outTileY - padYTop;
                                /* Skip out of border value */
                                if (inX < 0 || inY < 0 || inX >= (vx_int32)input_width || inY >= (vx_int32)input_height)
                                    continue;

                                if (inputs->isViewed)
                                    srcOffsetSize = ((inX + inputs->viewRegion.viewStarts[0]) + input_width * ((inY + inputs->viewRegion.viewStarts[1]) + input_height * (z + inputs->viewRegion.viewStarts[2]))) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);

                                else
                                    srcOffsetSize = (inX + input_width * (inY + input_height * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                dstOffserSize = (dstX + inImageTileSizeX * (dstY + tempY * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                input_ptr = inputs->tensorBuffer->memory.logicals[0] + srcOffsetSize;
                                output_ptr = temp_buffer + dstOffserSize + offsetDist;

                                if (inputs->tensorBuffer->dataFormat == VX_TYPE_FLOAT16)
                                    *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
                                else if (inputs->tensorBuffer->dataFormat == VX_TYPE_INT8)
                                    *(vx_int8*)output_ptr = *(vx_int8*)input_ptr;
                                else
                                    *(vx_int32*)output_ptr = *(vx_int32*)input_ptr;
                                dstX++;
                            }
                            if (inY < 0)
                                continue;
                            dstY++;
                        }
                    }
                    /* Other Left */
                    else if (j == 0)
                    {
                        vx_uint32 tempX = inImageTileSizeX - padXLeft;

                        if (tempX > input_width)
                            tempX = input_width;

                        for (y = 0; y < inImageTileSizeY; y++)
                        {

                            if (dstY >= inImageTileSizeY || dstY >= input_height)
                                    dstY = 0;

                            for (x = 0; x < tempX; x++)
                            {

                                vx_uint32 srcOffsetSize, dstOffserSize;
                                vx_uint8_ptr input_ptr = VX_NULL, output_ptr = VX_NULL;

                                if (dstX >= tempX || dstX >= input_width)
                                    dstX = 0;

                                inX = x + j * outTileX;
                                inY = y + i * outTileY - padYTop;
                                /* Skip out of border value */
                                if (inX < 0 || inY < 0 || inX >= (vx_int32)input_width || inY >= (vx_int32)input_height)
                                    continue;

                                if (inputs->isViewed)
                                    srcOffsetSize = ((inX + inputs->viewRegion.viewStarts[0]) + input_width * ((inY + inputs->viewRegion.viewStarts[1]) + input_height * (z + inputs->viewRegion.viewStarts[2]))) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);

                                else
                                    srcOffsetSize = (inX + input_width * (inY + input_height * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                dstOffserSize = (dstX + tempX * (dstY + inImageTileSizeY * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                input_ptr = inputs->tensorBuffer->memory.logicals[0] + srcOffsetSize;
                                output_ptr = temp_buffer + dstOffserSize + offsetDist;

                                if (inputs->tensorBuffer->dataFormat == VX_TYPE_FLOAT16)
                                    *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
                                else if (inputs->tensorBuffer->dataFormat == VX_TYPE_INT8)
                                    *(vx_int8*)output_ptr = *(vx_int8*)input_ptr;
                                else
                                    *(vx_int32*)output_ptr = *(vx_int32*)input_ptr;
                                dstX++;
                            }
                            if (inY < 0)
                                continue;
                            dstY++;
                        }
                    }
                    /* Other Right */
                    else if (j == numOfImageTileX - 1)
                    {
                        vx_uint32 tempX = input_width - (numOfImageTileX - 1) * outTileX + padXLeft;

                        if (tempX > input_width)
                            tempX = input_width;

                        for (y = 0; y < inImageTileSizeY; y++)
                        {

                            if (dstY >= inImageTileSizeY || dstY >= input_height)
                                    dstY = 0;

                            for (x = 0; x < tempX; x++)
                            {

                                vx_uint32 srcOffsetSize, dstOffserSize;
                                vx_uint8_ptr input_ptr = VX_NULL, output_ptr = VX_NULL;

                                if (dstX >= tempX || dstX >= input_width)
                                    dstX = 0;

                                inX = x + j * outTileX - padXLeft;
                                inY = y + i * outTileY - padYTop;
                                /* Skip out of border value */
                                if (inX < 0 || inY < 0 || inX >= (vx_int32)input_width || inY >= (vx_int32)input_height)
                                    continue;

                                if (inputs->isViewed)
                                    srcOffsetSize = ((inX + inputs->viewRegion.viewStarts[0]) + input_width * ((inY + inputs->viewRegion.viewStarts[1]) + input_height * (z + inputs->viewRegion.viewStarts[2]))) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);

                                else
                                    srcOffsetSize = (inX + input_width * (inY + input_height * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                dstOffserSize = (dstX + tempX * (dstY + inImageTileSizeY * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                input_ptr = inputs->tensorBuffer->memory.logicals[0] + srcOffsetSize;
                                output_ptr = temp_buffer + dstOffserSize + offsetDist;

                                if (inputs->tensorBuffer->dataFormat == VX_TYPE_FLOAT16)
                                    *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
                                else if (inputs->tensorBuffer->dataFormat == VX_TYPE_INT8)
                                    *(vx_int8*)output_ptr = *(vx_int8*)input_ptr;
                                else
                                    *(vx_int32*)output_ptr = *(vx_int32*)input_ptr;
                                dstX++;
                            }
                            if (inY < 0)
                                continue;
                            dstY++;
                        }
                    }
                    else
                    {
                        for (y = 0; y < inImageTileSizeY; y++)
                        {

                            if (dstY >= inImageTileSizeY || dstY >= input_height)
                                    dstY = 0;

                            for (x = 0; x < inImageTileSizeX; x++)
                            {

                                vx_uint32 srcOffsetSize, dstOffserSize;
                                vx_uint8_ptr input_ptr = VX_NULL, output_ptr = VX_NULL;

                                if (dstX >= inImageTileSizeX || dstX >= input_width)
                                    dstX = 0;

                                inX = x + j * outTileX - padXLeft;
                                inY = y + i * outTileY - padYTop;
                                /* Skip out of border value */
                                if (inX < 0 || inY < 0 || inX >= (vx_int32)input_width || inY >= (vx_int32)input_height)
                                    continue;

                                if (inputs->isViewed)
                                    srcOffsetSize = ((inX + inputs->viewRegion.viewStarts[0]) + input_width * ((inY + inputs->viewRegion.viewStarts[1]) + input_height * (z + inputs->viewRegion.viewStarts[2]))) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);

                                else
                                    srcOffsetSize = (inX + input_width * (inY + input_height * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                if (inImageTileSizeX > input_width && inImageTileSizeY > input_height)
                                    dstOffserSize = (dstX + input_width * (dstY + input_height * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                else
                                    dstOffserSize = (dstX + inImageTileSizeX * (dstY + inImageTileSizeY * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                input_ptr = inputs->tensorBuffer->memory.logicals[0] + srcOffsetSize;
                                output_ptr = temp_buffer + dstOffserSize + offsetDist;

                                if (inputs->tensorBuffer->dataFormat == VX_TYPE_FLOAT16)
                                    *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
                                else if (inputs->tensorBuffer->dataFormat == VX_TYPE_INT8)
                                    *(vx_int8*)output_ptr = *(vx_int8*)input_ptr;
                                else
                                    *(vx_int32*)output_ptr = *(vx_int32*)input_ptr;
                                dstX++;
                            }
                            if (inY < 0)
                                continue;
                            dstY++;
                        }
                    }
                }
            }
        }
    }
    else /* no padding*/
    {
        for (i = 0; i < numOfImageTileY; i++)
        {
            for (j = 0; j < numOfImageTileX; j++)
            {
                vx_uint32 offsetDist = (j + i * numOfImageTileX) * distSize;

                for (z = 0; z < input_z; z++)
                {
                    vx_uint32 dstX = 0, dstY = 0;
                    vx_int32 inX = 0, inY = 0;
                    if (i == numOfImageTileY - 1 && j == numOfImageTileX -1 && input_width % outTileX && input_height % outTileY && inImageTileSizeX < input_width && inImageTileSizeY < input_height)
                    {
                        vx_uint32 tempX = input_width - (numOfImageTileX - 1) * outTileX;
                        vx_uint32 tempY = input_height - (numOfImageTileY - 1) * outTileY;
                        for (y = 0; y < tempY; y++)
                        {

                            if (dstY >= tempY || dstY >= input_height)
                                dstY = 0;

                            for (x = 0; x < tempX; x++)
                            {

                                vx_uint32 srcOffsetSize, dstOffserSize;
                                vx_uint8_ptr input_ptr = VX_NULL, output_ptr = VX_NULL;

                                if (dstX >= tempX || dstX >= input_width)
                                    dstX = 0;

                                inX = x + j * outTileX;
                                inY = y + i * outTileY;
                                /* Skip out of border value */
                                if (inX < 0 || inY < 0 || inX >= (vx_int32)input_width || inY >= (vx_int32)input_height)
                                    continue;

                                if (inputs->isViewed)
                                    srcOffsetSize = ((inX + inputs->viewRegion.viewStarts[0]) + input_width * ((inY + inputs->viewRegion.viewStarts[1]) + input_height * (z + inputs->viewRegion.viewStarts[2]))) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);

                                else
                                    srcOffsetSize = (inX + input_width * (inY + input_height * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                dstOffserSize = (dstX + tempX * (dstY + tempY * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                input_ptr = inputs->tensorBuffer->memory.logicals[0] + srcOffsetSize;
                                output_ptr = temp_buffer + dstOffserSize + offsetDist;

                                if (inputs->tensorBuffer->dataFormat == VX_TYPE_FLOAT16)
                                    *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
                                else if (inputs->tensorBuffer->dataFormat == VX_TYPE_INT8)
                                    *(vx_int8*)output_ptr = *(vx_int8*)input_ptr;
                                else
                                    *(vx_int32*)output_ptr = *(vx_int32*)input_ptr;
                                dstX++;
                            }
                            if (inY < 0)
                                continue;
                            dstY++;
                        }
                    }
                    else if (i == numOfImageTileY - 1 && input_height % outTileY && inImageTileSizeY < input_height)
                    {
                        vx_uint32 tempY = input_height - (numOfImageTileY - 1) * outTileY;
                        for (y = 0; y < tempY; y++)
                        {

                            if (dstY >= tempY || dstY >= input_height)
                                dstY = 0;

                            for (x = 0; x < inImageTileSizeX; x++)
                            {

                                vx_uint32 srcOffsetSize, dstOffserSize;
                                vx_uint8_ptr input_ptr = VX_NULL, output_ptr = VX_NULL;

                                if (dstX >= inImageTileSizeX || dstX >= input_width)
                                    dstX = 0;

                                inX = x + j * outTileX;
                                inY = y + i * outTileY;
                                /* Skip out of border value */
                                if (inX < 0 || inY < 0 || inX >= (vx_int32)input_width || inY >= (vx_int32)input_height)
                                    continue;

                                if (inputs->isViewed)
                                    srcOffsetSize = ((inX + inputs->viewRegion.viewStarts[0]) + input_width * ((inY + inputs->viewRegion.viewStarts[1]) + input_height * (z + inputs->viewRegion.viewStarts[2]))) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);

                                else
                                    srcOffsetSize = (inX + input_width * (inY + input_height * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                dstOffserSize = (dstX + inImageTileSizeX * (dstY + tempY * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                input_ptr = inputs->tensorBuffer->memory.logicals[0] + srcOffsetSize;
                                output_ptr = temp_buffer + dstOffserSize + offsetDist;

                                if (inputs->tensorBuffer->dataFormat == VX_TYPE_FLOAT16)
                                    *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
                                else if (inputs->tensorBuffer->dataFormat == VX_TYPE_INT8)
                                    *(vx_int8*)output_ptr = *(vx_int8*)input_ptr;
                                else
                                    *(vx_int32*)output_ptr = *(vx_int32*)input_ptr;
                                dstX++;
                            }
                            if (inY < 0)
                                continue;
                            dstY++;
                        }
                    }
                    else if (j == numOfImageTileX - 1 && input_width % outTileX && inImageTileSizeX < input_width && inImageTileSizeY < input_height)
                    {
                        vx_uint32 tempX = input_width - (numOfImageTileX - 1) * outTileX;
                        for (y = 0; y < inImageTileSizeY; y++)
                        {

                            if (dstY >= inImageTileSizeY || dstY >= input_height)
                                dstY = 0;

                            for (x = 0; x < tempX; x++)
                            {

                                vx_uint32 srcOffsetSize, dstOffserSize;
                                vx_uint8_ptr input_ptr = VX_NULL, output_ptr = VX_NULL;

                                if (dstX >= tempX || dstX >= input_width)
                                    dstX = 0;

                                inX = x + j * outTileX;
                                inY = y + i * outTileY;
                                /* Skip out of border value */
                                if (inX < 0 || inY < 0 || inX >= (vx_int32)input_width || inY >= (vx_int32)input_height)
                                    continue;

                                if (inputs->isViewed)
                                    srcOffsetSize = ((inX + inputs->viewRegion.viewStarts[0]) + input_width * ((inY + inputs->viewRegion.viewStarts[1]) + input_height * (z + inputs->viewRegion.viewStarts[2]))) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);

                                else
                                    srcOffsetSize = (inX + input_width * (inY + input_height * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                dstOffserSize = (dstX + tempX * (dstY + inImageTileSizeY * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                input_ptr = inputs->tensorBuffer->memory.logicals[0] + srcOffsetSize;
                                output_ptr = temp_buffer + dstOffserSize + offsetDist;

                                if (inputs->tensorBuffer->dataFormat == VX_TYPE_FLOAT16)
                                    *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
                                else if (inputs->tensorBuffer->dataFormat == VX_TYPE_INT8)
                                    *(vx_int8*)output_ptr = *(vx_int8*)input_ptr;
                                else
                                    *(vx_int32*)output_ptr = *(vx_int32*)input_ptr;
                                dstX++;
                            }
                            if (inY < 0)
                                continue;
                            dstY++;
                        }
                    }
                    else
                    {
                        for (y = 0; y < inImageTileSizeY; y++)
                        {

                            if (dstY >= inImageTileSizeY || dstY >= input_height)
                                    dstY = 0;

                            for (x = 0; x < inImageTileSizeX; x++)
                            {

                                vx_uint32 srcOffsetSize, dstOffserSize;
                                vx_uint8_ptr input_ptr = VX_NULL, output_ptr = VX_NULL;

                                if (dstX >= inImageTileSizeX || dstX >= input_width)
                                    dstX = 0;

                                inX = x + j * outTileX;
                                inY = y + i * outTileY;
                                /* Skip out of border value */
                                if (inX < 0 || inY < 0 || inX >= (vx_int32)input_width || inY >= (vx_int32)input_height)
                                    continue;

                                if (inputs->isViewed)
                                    srcOffsetSize = ((inX + inputs->viewRegion.viewStarts[0]) + input_width * ((inY + inputs->viewRegion.viewStarts[1]) + input_height * (z + inputs->viewRegion.viewStarts[2]))) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);

                                else
                                    srcOffsetSize = (inX + input_width * (inY + input_height * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                if (inImageTileSizeX > input_width && inImageTileSizeY > input_height)
                                    dstOffserSize = (dstX + input_width * (dstY + input_height * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                else
                                    dstOffserSize = (dstX + inImageTileSizeX * (dstY + inImageTileSizeY * z)) * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);
                                input_ptr = inputs->tensorBuffer->memory.logicals[0] + srcOffsetSize;
                                output_ptr = temp_buffer + dstOffserSize + offsetDist;

                                if (inputs->tensorBuffer->dataFormat == VX_TYPE_FLOAT16)
                                    *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
                                else if (inputs->tensorBuffer->dataFormat == VX_TYPE_INT8)
                                    *(vx_int8*)output_ptr = *(vx_int8*)input_ptr;
                                else
                                    *(vx_int32*)output_ptr = *(vx_int32*)input_ptr;
                                dstX++;
                            }
                            if (inY < 0)
                                continue;
                            dstY++;
                        }
                    }
                }
            }
        }
    }

    gcoVX_FreeMemory((gcsSURF_NODE_PTR)outputs->tensorBuffer->memory.nodePtrs[0]);
    outputs->tensorBuffer->memory.logicals[0]    = VX_NULL;
    outputs->tensorBuffer->memory.nodePtrs[0]    = VX_NULL;

    gcoVX_AllocateMemory(distSize * numOfImageTileX * numOfImageTileY, (gctPOINTER*)&outputs->tensorBuffer->memory.logicals[0], &outputs->tensorBuffer->memory.physicals[0], &outputs->tensorBuffer->memory.nodePtrs[0]);
    vxMemCopy(outputs->tensorBuffer->memory.logicals[0], temp_buffer, distSize * numOfImageTileX * numOfImageTileY);
    vxFree(temp_buffer);
    temp_buffer = VX_NULL;

    return VX_SUCCESS;

}

vx_status vxnneExecuteTPNormalization(struct _vxnne_operation_s *operation)
{
    vxnne_normalization_layer layer = (vxnne_normalization_layer)operation->layer;
    vxnne_tp_operation normalizationOperation = (vxnne_tp_operation)operation;

    vx_tensor* inputs  = normalizationOperation->inputs;
    vx_array lut = (vx_array)normalizationOperation->buffer;
    vx_tensor* outputs = normalizationOperation->outputs;
    vx_node node = layer->base.node;

    return vxnneExecuteTPGeneric(node, inputs, VX_NULL, lut, outputs, layer->base.cmdTPBuff, normalizationOperation->op_num, normalizationOperation->multi_tensor);
}



vx_status vxnneExecuteSWPooling(struct _vxnne_operation_s *operation)
{
    vxnne_pooling_operation           poolingOperation   = (vxnne_pooling_operation)operation;

    vx_tensor inputs  = (vx_tensor)poolingOperation->inputs;
    vx_tensor outputs = (vx_tensor)poolingOperation->outputs;

    vx_enum  poolType_v = poolingOperation->pool_type;
    vx_uint32 poolSizeX_v = poolingOperation->pool_size_x;
    vx_uint32 poolPadXLeft_v = poolingOperation->pool_pad_x_left;
    vx_uint32 poolPadXRight_v = poolingOperation->pool_pad_x_right;
    vx_uint32 poolPadYTop_v = poolingOperation->pool_pad_y_top;
    vx_uint32 poolPadYBottom_v = poolingOperation->pool_pad_y_bottom;
    vx_enum rounding_v = poolingOperation->rounding;

    vx_status status = VX_SUCCESS;

    gctPOINTER inputsBaseLogicalAddr = VX_NULL;
    gctPOINTER outputsBaseLogicalAddr = VX_NULL;

    vx_int32 inputs_width, inputs_height, depth, outputs_width, outputs_height, out_w, out_h;
    vx_uint32 stride;
    vx_int32 type;

    vxoTensor_GetTensorViewMemory(inputs, &inputsBaseLogicalAddr, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, &outputsBaseLogicalAddr, VX_NULL);

    inputs_width   = TENSOR_SIZE_INDEX(inputs, 0);
    inputs_height  = TENSOR_SIZE_INDEX(inputs, 1);
    depth          = TENSOR_SIZE_INDEX(inputs, 2);
    outputs_width  = TENSOR_SIZE_INDEX(outputs, 0);
    outputs_height = TENSOR_SIZE_INDEX(outputs, 1);

    switch (poolType_v)
    {
    case VX_CONVOLUTIONAL_NETWORK_POOLING_MAX:
        type = 1;
        break;
    case VX_CONVOLUTIONAL_NETWORK_POOLING_AVG:
        type = 2;
        break;
    default:
        gcmPRINT("not support this pool type");
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if ((outputs_width - 1) == 0)
    {
        stride = 1;
    }
    else
    {
        /* Calculate stride = (w + 2*pad - weight)/(output_w - 1) */
        stride = vxoNNExternsionConvlutionRound((vx_float32)(inputs_width + poolPadXLeft_v + poolPadXRight_v - poolSizeX_v) / (outputs_width - 1), rounding_v);
    }

    status = vxnnePoolingCpu((vx_uint8_ptr)inputsBaseLogicalAddr,
                             inputs->tensorBuffer->fixedPointPos,
                             type,
                             (vx_type_e)inputs->tensorBuffer->dataFormat,
                             inputs_width,
                             inputs_height,
                             depth,
                             &out_w,
                             &out_h,
                             stride,
                             poolSizeX_v,
                             poolPadXLeft_v,
                             poolPadXRight_v,
                             poolPadYTop_v,
                             poolPadYBottom_v,
                             rounding_v,
                             (vx_uint8_ptr)outputsBaseLogicalAddr,
                             outputs->tensorBuffer->fixedPointPos,
                             outputs->tensorBuffer->roundingMode,
                             (vx_type_e)outputs->tensorBuffer->dataFormat);
    gcmASSERT((out_w == outputs_width) && (out_h == outputs_height));

    return status;

}

vx_status vxnneExecuteTPPooling(struct _vxnne_operation_s *operation)
{
    vxnne_pooling_layer layer = (vxnne_pooling_layer)operation->layer;
    vxnne_tp_operation poolingOperation = (vxnne_tp_operation)operation;

    vx_node node = layer->base.node;
    vx_tensor* inputs = poolingOperation->inputs;
    vx_tensor* outputs = poolingOperation->outputs;

    return vxnneExecuteTPGeneric(node, inputs, VX_NULL, VX_NULL, outputs, layer->base.cmdTPBuff, poolingOperation->op_num, poolingOperation->multi_tensor);
}

vx_status vxnneExecutePooling(struct _vxnne_operation_s *operation)
{
    vxnne_pooling_layer layer = (vxnne_pooling_layer)operation->layer;
    vxnne_pooling_operation poolingOperation = (vxnne_pooling_operation)operation;

    vx_tensor inputs  = (vx_tensor)poolingOperation->inputs;
    vx_tensor outputs = (vx_tensor)poolingOperation->outputs;

    vx_node node = layer->base.node;

    /* NN Engine programming. */
    vxNNExecute(node, layer->base.cmdNNBuff, 0, poolingOperation->weights_biases, 0, inputs, 0, outputs, 0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNConvolutionReluPoolingLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxnneConvolutionReluPoolingInitializer(
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
    vx_tensor outputs
    )
{
    vx_status status = VX_SUCCESS;

    vx_context context = vxGetContext((vx_reference)node);
    vx_uint32 padXLeftValue, padXRightValue, padYTopValue, padYBottomValue;

    vxnne_convolution_relu_pooling_layer  convolutionReluPoolingLayer = gcvNULL;

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_convolution_relu_pooling_layer_s), (gctPOINTER*)&convolutionReluPoolingLayer);
    if (!convolutionReluPoolingLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(convolutionReluPoolingLayer, sizeof(vxnne_convolution_relu_pooling_layer_s));

    vxnneLayer_Initialize(&convolutionReluPoolingLayer->base,
                            name,
                            node,
                            convolutionReluPoolingLayer->operations,
                            VX_NULL);


    vxnneOperation_Initialize(&convolutionReluPoolingLayer->convolution_operation.base,
                              &convolutionReluPoolingLayer->base,
                              VXNNE_OPERATION_TARGET_NN,
                              VXNNE_OPERATOR_CONVOLUTION,
                              vxnneExecuteConvolutionReluPooling,
                              VX_NULL);

    if (((vx_int32)pad_x_left < 0) || ((vx_int32)pad_y_top < 0))
    {
        vxnneGetPadValue((vx_int32)pad_x_left, (vx_int32)pad_y_top, &padXLeftValue, &padXRightValue, &padYTopValue, &padYBottomValue);
    }
    else
    {
        padXLeftValue   = pad_x_left;
        padXRightValue  = (pad_x_right == 0) ? pad_x_left : pad_x_right;
        padYTopValue    = pad_y_top;
        padYBottomValue = (pad_y_bottom == 0) ? pad_y_top : pad_y_bottom;;
    }

    if (weights_biases->stride > 1)
    {

        vx_uint32 dims = TENSOR_DIM_NUM(inputs);

        vx_uint32 sizes[3] = {vxnneAlignWithStride(TENSOR_SIZE_INDEX(inputs, 0) + padXLeftValue + padXRightValue, weights_biases->stride),
                              vxnneAlignWithStride(TENSOR_SIZE_INDEX(inputs, 1) + padYTopValue + padYBottomValue, weights_biases->stride),
                              TENSOR_SIZE_INDEX(inputs, 2)};

        vx_tensor brickTensor = VX_NULL;
        vx_tensor reshuffleTensor = VX_NULL;

        reshuffleTensor = vxoTensor_CreateTensor(node->base.context, dims, sizes, TENSOR_DATA_TYPE(inputs), vx_false_e);

        if (vxoTensor_AllocateMemory(reshuffleTensor) != VX_SUCCESS)
        {
            gcmPRINT("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }

        if (!context->options.enableBrickMode)
            convolutionReluPoolingLayer->base.num_operations    = 2;
        else
        {
            convolutionReluPoolingLayer->base.num_operations    = 3;
            brickTensor = vxoTensor_CreateTensor(node->base.context, dims, sizes, TENSOR_DATA_TYPE(inputs), vx_false_e);
            if (vxoTensor_AllocateMemory(brickTensor) != VX_SUCCESS)
            {
                gcmPRINT("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
                status = VX_ERROR_NO_MEMORY;
                goto exit;
            }
        }
        if (context->nnConfig.tpCoreCount && context->options.enableTP && context->options.flagTPFunc[TP_RESHUFFLE])
        {
            vx_uint32 i, slice;
            vx_uint32 stride = context->options.typeTPFunc[TP_RESHUFFLE] == 2 ? weights_biases->stride : 1;
            vx_tp_conv_cmd conv;

            vxnneOperation_Initialize(&convolutionReluPoolingLayer->reshuffle_tp_operation.base,
                                      &convolutionReluPoolingLayer->base,
                                      VXNNE_OPERATION_TARGET_TP,
                                      VXNNE_OPERATOR_RESHUFFLE,
                                      vxnneExecuteTPReshuffle,
                                      VX_NULL);

            slice = context->options.enableMultiTP && context->nnConfig.tpCoreCount > 1 ? TENSOR_SIZE_INDEX(inputs, 2) : 1;

            /* create cmd buffer for TP operation */
            context = vxGetContext((vx_reference)node);
            convolutionReluPoolingLayer->base.cmdTPBuff = vxCreateArray(context, VX_TYPE_CHAR, TP_COMMAND_SIZE * slice * stride);
            if (!vxoArray_AllocateMemory(convolutionReluPoolingLayer->base.cmdTPBuff))
            {
                status = VX_ERROR_NO_MEMORY;
                gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
                goto exit;
            }

            conv.pad_x = padXLeftValue;
            conv.pad_y = padYTopValue;
            conv.pool_size_x = conv.pool_size_y = 0;
            conv.enable_relu = vx_false_e;
            for (i = 0; i < slice * stride; i++)
            {
                fillInCmdTPBuffer(
                        inputs, (vx_reference)weights_biases, reshuffleTensor,
                        convolutionReluPoolingLayer->base.cmdTPBuff, VX_NULL,
                        &conv, VX_NULL,
                        TP_RESHUFFLE,
                        i, slice > 1,
                        vx_true_e);
            }

            convolutionReluPoolingLayer->operations[0] = (vxnne_operation)&convolutionReluPoolingLayer->reshuffle_tp_operation;
            convolutionReluPoolingLayer->reshuffle_tp_operation.inputs[0]      = inputs;
            convolutionReluPoolingLayer->reshuffle_tp_operation.weights_biases = weights_biases;
            convolutionReluPoolingLayer->reshuffle_tp_operation.outputs[0]     = reshuffleTensor;
            convolutionReluPoolingLayer->reshuffle_tp_operation.op_num         = slice * stride;
            convolutionReluPoolingLayer->reshuffle_tp_operation.multi_tensor   = vx_false_e;
        }
        else
        {
            vx_uint32      stride_x                = weights_biases->stride;
            vx_uint32      stride_y                = weights_biases->stride;
            vx_uint32      dstWidth                = TENSOR_SIZE_INDEX(reshuffleTensor, 0) / stride_x;
            vx_uint32      dstHeight               = TENSOR_SIZE_INDEX(reshuffleTensor, 1) / stride_y;
            vx_uint32      dstDepth                = TENSOR_SIZE_INDEX(reshuffleTensor, 2) * stride_x * stride_y;
            vx_bool        shExe_flag              = vx_true_e;
            vx_enum        inputFormat             = inputs->tensorBuffer->dataFormat;
            vx_enum        outputFormat            = reshuffleTensor->tensorBuffer->dataFormat;
            vx_bool        padMode_flag            = (vx_bool)(padMode == VX_PAD_CONSTANT || padMode == VX_PAD_REPLICATE || (padXLeftValue == 0 && padXRightValue == 0 && padYTopValue == 0 && padYBottomValue == 0));
            vx_uint32      padConstValue           =  padConst != VX_NULL ? padConst->value->u32 : 0;

            shExe_flag    = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16 && stride_x == 2 && stride_y == 2)
                                    ||(inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16 && stride_x == 4 && stride_y == 4));
            if(shExe_flag && padMode_flag)
            {
                vxnne_shader_executable shaderExecutable;
                vx_tensor                outTensor = NULL;
                if(outTensor == NULL)
                {
                    vx_int32   new_size[3] = {dstWidth, dstHeight, dstDepth};
                    vx_uint32  outputs_dims = 3;
                    outTensor = vxReshapeTensor(reshuffleTensor, new_size, outputs_dims);
                }

                shaderExecutable = vxnneReshuffleShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_RESHUFFLE, &node->kernelAttributes.borderMode, inputs, stride_x, stride_y, padMode, padConstValue, padXLeftValue, padXRightValue, padYTopValue, padYBottomValue, outTensor);


                if(outTensor)
                    vxReleaseTensor(&outTensor);

                if (!shaderExecutable)
                {
                    status = VX_FAILURE;
                    goto exit;
                }
                status = vxnneShaderOperation_Initialize(&convolutionReluPoolingLayer->reshuffle_shader_operation,
                    &convolutionReluPoolingLayer->base,
                    VXNNE_OPERATOR_RESHUFFLE,
                    shaderExecutable);

                if (status != VX_SUCCESS)
                    goto exit;

                convolutionReluPoolingLayer->operations[0] = (vxnne_operation)&convolutionReluPoolingLayer->reshuffle_shader_operation.base;
            }
            else
            {
                vxnneOperation_Initialize(&convolutionReluPoolingLayer->reshuffle_operation.base,
                    &convolutionReluPoolingLayer->base,
                    VXNNE_OPERATION_TARGET_SW,
                    VXNNE_OPERATOR_RESHUFFLE,
                    vxnneExecuteSWReshuffle,
                    VX_NULL);

                convolutionReluPoolingLayer->operations[0] = (vxnne_operation)&convolutionReluPoolingLayer->reshuffle_operation;

                convolutionReluPoolingLayer->reshuffle_operation.inputs         = inputs;
                convolutionReluPoolingLayer->reshuffle_operation.weights_biases = weights_biases;
                convolutionReluPoolingLayer->reshuffle_operation.pad_x_left     = padXLeftValue;
                convolutionReluPoolingLayer->reshuffle_operation.pad_x_right    = padXRightValue;
                convolutionReluPoolingLayer->reshuffle_operation.pad_y_top      = padYTopValue;
                convolutionReluPoolingLayer->reshuffle_operation.pad_y_bottom   = padYBottomValue;
                convolutionReluPoolingLayer->reshuffle_operation.pad_mode       = padMode;
                convolutionReluPoolingLayer->reshuffle_operation.pad_const      = padConst;
                convolutionReluPoolingLayer->reshuffle_operation.outputs        = reshuffleTensor;
            }

        }

        if (!context->options.enableBrickMode)
        {
            convolutionReluPoolingLayer->operations[1] = (vxnne_operation)&convolutionReluPoolingLayer->convolution_operation;

            convolutionReluPoolingLayer->convolution_operation.inputs           = reshuffleTensor;
            convolutionReluPoolingLayer->convolution_operation.weights_biases   = weights_biases;
            convolutionReluPoolingLayer->convolution_operation.outputs          = outputs;

            convolutionReluPoolingLayer->base.num_temp_tensors                  = 1;
            convolutionReluPoolingLayer->base.temp_tensors[0] = reshuffleTensor;
        }
        else
        {
            vxnneOperation_Initialize(&convolutionReluPoolingLayer->brick_operation.base,
                                            &convolutionReluPoolingLayer->base,
                                            VXNNE_OPERATION_TARGET_SW,
                                            VXNNE_OPERATOR_BRICK,
                                            vxnneExecuteSWBrickMode,
                                            VX_NULL);
            convolutionReluPoolingLayer->operations[1] = (vxnne_operation)&convolutionReluPoolingLayer->brick_operation;
            convolutionReluPoolingLayer->brick_operation.inputs         = reshuffleTensor;
            convolutionReluPoolingLayer->brick_operation.outputs        = brickTensor;
            convolutionReluPoolingLayer->brick_operation.pad_x_left     = padXLeftValue;
            convolutionReluPoolingLayer->brick_operation.pad_x_right    = padXRightValue;
            convolutionReluPoolingLayer->brick_operation.pad_y_top      = padYTopValue;
            convolutionReluPoolingLayer->brick_operation.pad_y_bottom   = padYBottomValue;
            convolutionReluPoolingLayer->brick_operation.kernel_x       = weights_biases->weights_sizes[0];
            convolutionReluPoolingLayer->brick_operation.kernel_y       = weights_biases->weights_sizes[1];
            convolutionReluPoolingLayer->brick_operation.outTileX       = weights_biases->outImageTileXSize[0][0];
            convolutionReluPoolingLayer->brick_operation.outTileY       = weights_biases->outImageTileYSize[0][0];
            convolutionReluPoolingLayer->brick_operation.stride         = weights_biases->stride;
            convolutionReluPoolingLayer->brick_operation.num_tile_x     = (weights_biases->output_sizes[0] % weights_biases->outImageTileXSize[0][0]) ?
 weights_biases->output_sizes[0] / weights_biases->outImageTileXSize[0][0] + 1 :
 weights_biases->output_sizes[0] / weights_biases->outImageTileXSize[0][0];
            convolutionReluPoolingLayer->brick_operation.num_tile_y     = (weights_biases->output_sizes[1] % weights_biases->outImageTileYSize[0][0]) ?
 weights_biases->output_sizes[1] / weights_biases->outImageTileYSize[0][0] + 1 :
 weights_biases->output_sizes[1] / weights_biases->outImageTileYSize[0][0];

            convolutionReluPoolingLayer->operations[2] = (vxnne_operation)&convolutionReluPoolingLayer->convolution_operation;

            convolutionReluPoolingLayer->convolution_operation.inputs           = brickTensor;
            convolutionReluPoolingLayer->convolution_operation.weights_biases   = weights_biases;
            convolutionReluPoolingLayer->convolution_operation.outputs          = outputs;

            convolutionReluPoolingLayer->base.num_temp_tensors                  = 2;
            convolutionReluPoolingLayer->base.temp_tensors[0] = reshuffleTensor;
            convolutionReluPoolingLayer->base.temp_tensors[1] = brickTensor;
        }
    }
    else
    {
        vx_uint32 input_width   = TENSOR_SIZE_INDEX(inputs, 0);
        vx_uint32 input_height  = TENSOR_SIZE_INDEX(inputs, 1);
        vx_uint32 org_kernel_x  = weights_biases->org_weights_sizes[0];
        vx_uint32 org_kernel_y  = weights_biases->org_weights_sizes[1];
        vx_uint32 conv_stride_x = weights_biases->stride;
        vx_uint32 conv_stride_y = conv_stride_x;

        vxoNNExternsionInputOutputArguments(
            input_width, input_height,
            conv_stride_x, conv_stride_y,
            padXLeftValue, padXRightValue, padYTopValue, padYBottomValue,
            org_kernel_x, org_kernel_y,
            conv_rounding_type,
            pool_size_x,
            &inputs->insideDims[0], &inputs->insideDims[1],
            &outputs->insideDims[0], &outputs->insideDims[1],
            &inputs->finalDims[0], &inputs->finalDims[1],
            &outputs->finalDims[0], &outputs->finalDims[1]);

        if (inputs->insideDims[0] != inputs->finalDims[0] || inputs->insideDims[1] != inputs->finalDims[1])
        {
            vx_uint32 sizes[3] =
            {
                inputs->finalDims[0],
                inputs->finalDims[1],
                TENSOR_SIZE_INDEX(inputs, 2)
            };

            vx_tensor tmp_tensor;
            vx_tensor brickTensor = VX_NULL;

            if (context->options.enableBrickMode)
            {
                brickTensor = vxoTensor_CreateTensor(node->base.context, TENSOR_DIM_NUM(inputs), sizes, TENSOR_DATA_TYPE(inputs), vx_false_e);
                if (vxoTensor_AllocateMemory(brickTensor) != VX_SUCCESS)
                {
                    gcmPRINT("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
                    status = VX_ERROR_NO_MEMORY;
                    goto exit;
                }
            }
            tmp_tensor = vxoTensor_CreateTensor(
                            inputs->base.context,
                            TENSOR_DIM_NUM(inputs),
                            sizes,
                            inputs->tensorBuffer->dataFormat,
                            vx_false_e);

            if (tmp_tensor == VX_NULL ||
                vxoTensor_AllocateMemory(tmp_tensor) != VX_SUCCESS)
            {
                gcmPRINT("Fail at function %s, line %d", __FUNCTION__, __LINE__);
                status = VX_ERROR_NO_MEMORY;
                goto exit;
            }

            vxnneOperation_Initialize(&convolutionReluPoolingLayer->resize_operation.base,
                                        &convolutionReluPoolingLayer->base,
                                        VXNNE_OPERATION_TARGET_SW,
                                        VXNNE_OPERATOR_RESIZE,
                                        vxnneExecuteSWReSizeCopy,
                                        VX_NULL);

            convolutionReluPoolingLayer->base.num_operations    = 2;
            convolutionReluPoolingLayer->operations[0] = (vxnne_operation)&convolutionReluPoolingLayer->resize_operation;

            convolutionReluPoolingLayer->resize_operation.inputs         = inputs;
            convolutionReluPoolingLayer->resize_operation.outputs        = tmp_tensor;
            if (context->options.enableBrickMode)
            {
                convolutionReluPoolingLayer->base.num_operations    = 3;
                vxnneOperation_Initialize(&convolutionReluPoolingLayer->brick_operation.base,
                                                &convolutionReluPoolingLayer->base,
                                                VXNNE_OPERATION_TARGET_SW,
                                                VXNNE_OPERATOR_BRICK,
                                                vxnneExecuteSWBrickMode,
                                                VX_NULL);
                convolutionReluPoolingLayer->operations[1] = (vxnne_operation)&convolutionReluPoolingLayer->brick_operation;
                convolutionReluPoolingLayer->brick_operation.inputs         = tmp_tensor;
                convolutionReluPoolingLayer->brick_operation.outputs        = brickTensor;
                convolutionReluPoolingLayer->brick_operation.pad_x_left     = padXLeftValue;
                convolutionReluPoolingLayer->brick_operation.pad_x_right    = padXRightValue;
                convolutionReluPoolingLayer->brick_operation.pad_y_top      = padYTopValue;
                convolutionReluPoolingLayer->brick_operation.pad_y_bottom   = padYBottomValue;
                convolutionReluPoolingLayer->brick_operation.kernel_x       = weights_biases->weights_sizes[0];
                convolutionReluPoolingLayer->brick_operation.kernel_y       = weights_biases->weights_sizes[1];
                convolutionReluPoolingLayer->brick_operation.stride         = weights_biases->stride;
                convolutionReluPoolingLayer->brick_operation.outTileX       = weights_biases->outImageTileXSize[0][0];
                convolutionReluPoolingLayer->brick_operation.outTileY       = weights_biases->outImageTileYSize[0][0];
                convolutionReluPoolingLayer->brick_operation.num_tile_x     = (weights_biases->output_sizes[0] % weights_biases->outImageTileXSize[0][0]) ?
 weights_biases->output_sizes[0] / weights_biases->outImageTileXSize[0][0] + 1 :
 weights_biases->output_sizes[0] / weights_biases->outImageTileXSize[0][0];
                convolutionReluPoolingLayer->brick_operation.num_tile_y     = (weights_biases->output_sizes[1] % weights_biases->outImageTileYSize[0][0]) ?
 weights_biases->output_sizes[1] / weights_biases->outImageTileYSize[0][0] + 1 :
 weights_biases->output_sizes[1] / weights_biases->outImageTileYSize[0][0];

                convolutionReluPoolingLayer->operations[2] = (vxnne_operation)&convolutionReluPoolingLayer->convolution_operation;

                convolutionReluPoolingLayer->convolution_operation.inputs           = brickTensor;
                convolutionReluPoolingLayer->convolution_operation.weights_biases   = weights_biases;
                convolutionReluPoolingLayer->convolution_operation.outputs          = outputs;

                convolutionReluPoolingLayer->base.num_temp_tensors                  = 2;
                convolutionReluPoolingLayer->base.temp_tensors[0] = tmp_tensor;
                convolutionReluPoolingLayer->base.temp_tensors[1] = brickTensor;
            }
            else
            {
                convolutionReluPoolingLayer->operations[1] = (vxnne_operation)&convolutionReluPoolingLayer->convolution_operation;

                convolutionReluPoolingLayer->convolution_operation.inputs           = tmp_tensor;
                convolutionReluPoolingLayer->convolution_operation.weights_biases   = weights_biases;
                convolutionReluPoolingLayer->convolution_operation.outputs          = outputs;

                convolutionReluPoolingLayer->base.num_temp_tensors                  = 1;
                convolutionReluPoolingLayer->base.temp_tensors[0] = tmp_tensor;
            }
        }
        else
        {
            if (context->options.enableBrickMode)
            {
                vx_uint32 dims = TENSOR_DIM_NUM(inputs);
                vx_uint32 sizes[3] = {TENSOR_SIZE_INDEX(inputs, 0),
                                      TENSOR_SIZE_INDEX(inputs, 1),
                                      TENSOR_SIZE_INDEX(inputs, 2)};

                vx_tensor brickTensor = VX_NULL;

                if (inputs->isViewed)
                {
                    sizes[0] = inputs->viewRegion.viewEnds[0] - inputs->viewRegion.viewStarts[0];
                    sizes[1] = inputs->viewRegion.viewEnds[1] - inputs->viewRegion.viewStarts[1];
                    sizes[2] = inputs->viewRegion.viewEnds[2] - inputs->viewRegion.viewStarts[2];
                }

                brickTensor = vxoTensor_CreateTensor(node->base.context, dims, sizes, TENSOR_DATA_TYPE(inputs), vx_false_e);
                if (vxoTensor_AllocateMemory(brickTensor) != VX_SUCCESS)
                {
                    gcmPRINT("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
                    status = VX_ERROR_NO_MEMORY;
                    goto exit;
                }

                convolutionReluPoolingLayer->base.num_operations    = 2;

                vxnneOperation_Initialize(&convolutionReluPoolingLayer->brick_operation.base,
                                                &convolutionReluPoolingLayer->base,
                                                VXNNE_OPERATION_TARGET_SW,
                                                VXNNE_OPERATOR_BRICK,
                                                vxnneExecuteSWBrickMode,
                                                VX_NULL);
                convolutionReluPoolingLayer->operations[0] = (vxnne_operation)&convolutionReluPoolingLayer->brick_operation;
                convolutionReluPoolingLayer->brick_operation.inputs         = inputs;
                convolutionReluPoolingLayer->brick_operation.outputs        = brickTensor;
                convolutionReluPoolingLayer->brick_operation.pad_x_left     = padXLeftValue;
                convolutionReluPoolingLayer->brick_operation.pad_x_right    = padXRightValue;
                convolutionReluPoolingLayer->brick_operation.pad_y_top      = padYTopValue;
                convolutionReluPoolingLayer->brick_operation.pad_y_bottom   = padYBottomValue;
                convolutionReluPoolingLayer->brick_operation.kernel_x       = weights_biases->weights_sizes[0];
                convolutionReluPoolingLayer->brick_operation.kernel_y       = weights_biases->weights_sizes[1];
                convolutionReluPoolingLayer->brick_operation.outTileX       = weights_biases->outImageTileXSize[0][0];
                convolutionReluPoolingLayer->brick_operation.outTileY       = weights_biases->outImageTileYSize[0][0];
                convolutionReluPoolingLayer->brick_operation.num_tile_x     = (weights_biases->output_sizes[0] % weights_biases->outImageTileXSize[0][0]) ?
 weights_biases->output_sizes[0] / weights_biases->outImageTileXSize[0][0] + 1 :
 weights_biases->output_sizes[0] / weights_biases->outImageTileXSize[0][0];
                convolutionReluPoolingLayer->brick_operation.num_tile_y     = (weights_biases->output_sizes[1] % weights_biases->outImageTileYSize[0][0]) ?
 weights_biases->output_sizes[1] / weights_biases->outImageTileYSize[0][0] + 1 :
 weights_biases->output_sizes[1] / weights_biases->outImageTileYSize[0][0];
                convolutionReluPoolingLayer->brick_operation.stride         = weights_biases->stride;

                convolutionReluPoolingLayer->operations[1] = (vxnne_operation)&convolutionReluPoolingLayer->convolution_operation;

                convolutionReluPoolingLayer->convolution_operation.inputs           = brickTensor;
                convolutionReluPoolingLayer->convolution_operation.weights_biases   = weights_biases;
                convolutionReluPoolingLayer->convolution_operation.outputs          = outputs;

                convolutionReluPoolingLayer->base.num_temp_tensors                  = 1;
                convolutionReluPoolingLayer->base.temp_tensors[0] = brickTensor;
            }
            else
            {
                convolutionReluPoolingLayer->base.num_operations    = 1;
                convolutionReluPoolingLayer->operations[0] = (vxnne_operation)&convolutionReluPoolingLayer->convolution_operation;

                convolutionReluPoolingLayer->convolution_operation.inputs           = inputs;
                convolutionReluPoolingLayer->convolution_operation.weights_biases   = weights_biases;
                convolutionReluPoolingLayer->convolution_operation.outputs          = outputs;
            }
        }
    }


    /* create cmd buffer for nn layer */
    convolutionReluPoolingLayer->base.cmdNNBuff  = vxCreateArray(context, VX_TYPE_CHAR, NNE_COMMAND_SIZE);
    if (!vxoArray_AllocateMemory(convolutionReluPoolingLayer->base.cmdNNBuff))
    {
        status = VX_ERROR_NO_MEMORY;
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }
    if (enable_pooling)
    {
        fillinCmmdBuff(inputs,
                       weights_biases,
                       padXLeftValue,
                       padXRightValue,
                       padYTopValue,
                       padYBottomValue,
                       padMode,
                       (padConst != VX_NULL) ? &(padConst->value->n32) : VX_NULL,
                       conv_rounding_type,
                       enable_relu,
                       pool_type,
                       pool_size_x,
                       pool_size_y,
                       outputs,
                       convolutionReluPoolingLayer->base.cmdNNBuff,
                       vx_false_e,
                       0);
    }
    else
    {
        fillinCmmdBuff(inputs,
                       weights_biases,
                       padXLeftValue,
                       padXRightValue,
                       padYTopValue,
                       padYBottomValue,
                       padMode,
                       (padConst != VX_NULL) ? &(padConst->value->n32) : VX_NULL,
                       conv_rounding_type,
                       enable_relu,
                       pool_type,
                       0,
                       0,
                       outputs,
                       convolutionReluPoolingLayer->base.cmdNNBuff,
                       vx_false_e,
                       0);
    }

    node->layer = &convolutionReluPoolingLayer->base;
    return status;

exit:
    if (convolutionReluPoolingLayer) gcoOS_Free(gcvNULL, (gctPOINTER)convolutionReluPoolingLayer);
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_tensor                   inputs = (vx_tensor)parameters[0];
    vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter)parameters[1]; // need modify
    vx_scalar                   pad_x_s = (vx_scalar)parameters[2];
    vx_scalar                   pad_y_s = (vx_scalar)parameters[3];
    vx_scalar                   down_scale_size_rounding_s = (vx_scalar)parameters[7];
    vx_scalar                   enable_relu_s = (vx_scalar)parameters[8];
    vx_scalar                   pool_type_s = (vx_scalar)parameters[9];
    vx_scalar                   pool_size_x_s = (vx_scalar)parameters[10];
    vx_scalar                   pool_size_y_s = (vx_scalar)parameters[11];
    vx_tensor                   outputs = (vx_tensor)parameters[12];

    vx_enum                     conv_rounding_type;
    vx_enum                     pool_type;
    vx_uint32                   pool_size_x;
    vx_uint32                   pool_size_y;
    vx_bool                     enable_relu;

    vx_status                   status = VX_SUCCESS;

    conv_rounding_type   = down_scale_size_rounding_s->value->e;
    pool_type            = pool_type_s->value->e;
    pool_size_x          = pool_size_x_s->value->u32;
    pool_size_y          = pool_size_y_s->value->u32;
    enable_relu          = enable_relu_s->value->b;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    status = vxnneConvolutionReluPoolingInitializer(node,
                                                     "ConvolutionReluPooingLayer",
                                                      inputs,
                                                      weights_biases,
                                                      0,
                                                      0,
                                                      pad_x_s->value->u32,
                                                      0,
                                                      pad_y_s->value->u32,
                                                      0,
                                                      conv_rounding_type,
                                                      enable_relu,
                                                      vx_true_e,
                                                      pool_type,
                                                      pool_size_x,
                                                      pool_size_y,
                                                      VX_PAD_CONSTANT,
                                                      VX_NULL,
                                                      outputs);

    return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNConvolutionReluPoolingLayer2(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer2_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer2_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer2_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_tensor                   inputs = (vx_tensor)parameters[0];
    vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter)parameters[1]; // need modify
    vx_scalar                   dilation_x_s = (vx_scalar)parameters[2];
    vx_scalar                   dilation_y_s = (vx_scalar)parameters[3];
    vx_scalar                   pad_x_left_s = (vx_scalar)parameters[4];
    vx_scalar                   pad_x_right_s = (vx_scalar)parameters[5];
    vx_scalar                   pad_y_top_s = (vx_scalar)parameters[6];
    vx_scalar                   pad_y_bottom_s = (vx_scalar)parameters[7];
    vx_scalar                   down_scale_size_rounding_s = (vx_scalar)parameters[11];
    vx_scalar                   enable_relu_s = (vx_scalar)parameters[12];
    vx_scalar                   pool_type_s = (vx_scalar)parameters[13];
    vx_scalar                   pool_size_x_s = (vx_scalar)parameters[14];
    vx_scalar                   pool_size_y_s = (vx_scalar)parameters[15];
    vx_scalar                   pad_mode_s = (vx_scalar)parameters[16];
    vx_scalar                   pad_const_s = (vx_scalar)parameters[17];
    vx_tensor                   outputs = (vx_tensor)parameters[18];

    vx_enum                     conv_rounding_type;
    vx_enum                     pool_type;
    vx_uint32                   pool_size_x;
    vx_uint32                   pool_size_y;
    vx_bool                     enable_relu;

    vx_status                   status = VX_SUCCESS;

    conv_rounding_type   = down_scale_size_rounding_s->value->e;
    pool_type            = pool_type_s->value->e;
    pool_size_x          = pool_size_x_s->value->u32;
    pool_size_y          = pool_size_y_s->value->u32;
    enable_relu          = enable_relu_s->value->b;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    status = vxnneConvolutionReluPoolingInitializer(node,
                                                     "ConvolutionReluPooingLayer2",
                                                      inputs,
                                                      weights_biases,
                                                      dilation_x_s->value->s,
                                                      dilation_y_s->value->s,
                                                      pad_x_left_s->value->u32,
                                                      pad_x_right_s->value->u32,
                                                      pad_y_top_s->value->u32,
                                                      pad_y_bottom_s->value->u32,
                                                      conv_rounding_type,
                                                      enable_relu,
                                                      vx_true_e,
                                                      pool_type,
                                                      pool_size_x,
                                                      pool_size_y,
                                                      pad_mode_s->value->e,
                                                      pad_const_s,
                                                      outputs);

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer2_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNConvolutionReluLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    if (index != vxmLENGTH_OF(nn_ConvolutionReluLayer_params) - 1) return VX_ERROR_INVALID_PARAMETERS;

    ptr->type                 = VX_TYPE_TENSOR;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{

    vx_tensor                   inputs = (vx_tensor)parameters[0];
    vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter)parameters[1]; // need modify
    vx_scalar                   pad_x_s = (vx_scalar)parameters[2];
    vx_scalar                   pad_y_s = (vx_scalar)parameters[3];
    vx_scalar                   down_scale_size_rounding_s = (vx_scalar)parameters[7];
    vx_scalar                   enable_relu_s = (vx_scalar)parameters[8];
    vx_tensor                   outputs = (vx_tensor)parameters[9];
    vx_enum                     conv_rounding_type;
    vx_bool                     enable_relu;

    vx_status                   status = VX_SUCCESS;

    conv_rounding_type   = down_scale_size_rounding_s->value->e;
    enable_relu          = enable_relu_s->value->b;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    status = vxnneConvolutionReluPoolingInitializer(node,
                                                      "ConvolutionReluLayer",
                                                      inputs,
                                                      weights_biases,
                                                      0,
                                                      0,
                                                      pad_x_s->value->u32,
                                                      0,
                                                      pad_y_s->value->u32,
                                                      0,
                                                      conv_rounding_type,
                                                      enable_relu,
                                                      vx_false_e,
                                                      VIV_NN_POOLING_NON,
                                                      0,
                                                      0,
                                                      VX_PAD_CONSTANT,
                                                      VX_NULL,
                                                      outputs);

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNFullyConnectedReluLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedReluLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedReluLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    if (index != vxmLENGTH_OF(nn_FullyConnectedReluLayer_params) - 1) return VX_ERROR_INVALID_PARAMETERS;

    ptr->type                 = VX_TYPE_TENSOR;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedReluLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_context                  context = vxGetContext((vx_reference)node);
    vx_tensor                   inputs = (vx_tensor)parameters[0];
    vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter)parameters[1]; // need modify
    vx_scalar                   pad_s = (vx_scalar)parameters[2];
    vx_scalar                   down_scale_size_rounding_s = (vx_scalar)parameters[6];
    vx_scalar                   enable_relu_s = (vx_scalar)parameters[7];
    vx_tensor                   outputs = (vx_tensor)parameters[8];

    vx_uint32                   pad;
    vx_enum                     conv_rounding_type;
    vx_bool                     enable_relu;
    vx_uint32                   i;

    vx_status                   status = VX_SUCCESS;
    vxnne_fully_connected_relu_layer  fullyConnectReluLayer = gcvNULL;

    pad                  = pad_s->value->u32;
    conv_rounding_type   = down_scale_size_rounding_s->value->e;
    enable_relu          = enable_relu_s->value->b;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_fully_connected_relu_layer_s), (gctPOINTER*)&fullyConnectReluLayer);
    if (!fullyConnectReluLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(fullyConnectReluLayer, sizeof(vxnne_fully_connected_relu_layer_s));

    vxnneLayer_Initialize(&fullyConnectReluLayer->base,
                            "FullyConnectReluLayer",
                            node,
                            fullyConnectReluLayer->operations,
                            VX_NULL);


    if (context->nnConfig.tpCoreCount && context->options.enableTP && weights_biases->use_tp_fc)
    {
        vx_tp_conv_cmd conv;

        vxnneOperation_Initialize(&fullyConnectReluLayer->fully_connected_TPoperation.base,
                                    &fullyConnectReluLayer->base,
                                    VXNNE_OPERATION_TARGET_TP,
                                    VXNNE_OPERATOR_FULLYCONNECTED,
                                    vxnneExecuteTPFullyConnectReluLayer,
                                    VX_NULL);

        fullyConnectReluLayer->base.num_operations = 1;
        fullyConnectReluLayer->operations[0] = (vxnne_operation)&fullyConnectReluLayer->fully_connected_TPoperation;
        fullyConnectReluLayer->fully_connected_TPoperation.inputs[0]         = inputs;
        fullyConnectReluLayer->fully_connected_TPoperation.weights_biases    = weights_biases;
        fullyConnectReluLayer->fully_connected_TPoperation.outputs[0]        = outputs;
        fullyConnectReluLayer->fully_connected_TPoperation.op_num            = 1;
        fullyConnectReluLayer->fully_connected_TPoperation.multi_tensor      = vx_false_e;

        node->layer = &fullyConnectReluLayer->base;

        context = vxGetContext((vx_reference)node);
        node->layer->cmdTPBuff = vxCreateArray(context, VX_TYPE_CHAR, TP_COMMAND_SIZE * weights_biases->zgroup_num);
        if (!vxoArray_AllocateMemory(node->layer->cmdTPBuff))
        {
            status = VX_ERROR_NO_MEMORY;
            gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            goto exit;
        }

        conv.pad_x = pad;
        conv.pad_y = pad;
        conv.pool_size_x = 0;
        conv.pool_size_y = 0;
        conv.enable_relu = enable_relu;
        conv.conv_rounding_type = 0;
        for (i = 0; i < weights_biases->zgroup_num; i++)
        {
            /* fill in cmd buffer */
            fillInCmdTPBuffer(
                    inputs, (vx_reference)weights_biases, outputs,
                    node->layer->cmdTPBuff, VX_NULL,
                    &conv, VX_NULL,
                    TP_SINGLE_FC,
                    i, 0,
                    vx_true_e);
        }

#if TP_FC_DUMP_INPUTS
        {
            vx_uint32 inZSize = TENSOR_SIZE_INDEX(inputs, 0) * TENSOR_SIZE_INDEX(inputs, 1) * TENSOR_SIZE_INDEX(inputs, 2);
            char fileName[32];
            FILE *fp;
            sprintf(fileName, "tp_input.txt");
            fp = fopen(fileName, "w");

            if (inputs->tensorBuffer->dataFormat == VX_TYPE_INT8)
            {
                vx_int8 * inputData = (vx_int8 *) inputs->tensorBuffer->memory.logicals[0];
                vx_uint32 z;

                for (z = 0; z < inZSize; z++)
                {
                    fprintf(fp, "z=%4d input=%d\n", z, *inputData);
                    inputData++;
                }
            }
            else
            {
                vx_uint16 * inputData = (vx_uint16 *) inputs->tensorBuffer->memory.logicals[0];
                vx_uint32 z;

                for (z = 0; z < inZSize; z++)
                {
                    vx_float32 finput = Fp16toFp32(*inputData);

                    fprintf(fp, "z=%4d input=0x%04x (%f)\n", z, *inputData, finput);
                    inputData++;
                }
            }
            fclose(fp);
        }
#endif
    }
    else
    {
        vxnneOperation_Initialize(&fullyConnectReluLayer->fully_connected_relu_operation.base,
                                    &fullyConnectReluLayer->base,
                                    VXNNE_OPERATION_TARGET_NN,
                                    VXNNE_OPERATOR_FULLYCONNECTED,
                                    vxnneExecuteFullyConnectReluLayer,
                                    VX_NULL);

        if (weights_biases->use_fc_accel)
        {
            vx_uint32 size = gcmALIGN(((weights_biases->org_weights_sizes[2] + weights_biases->weights_sizes[1]) +
                                        weights_biases->memory_pad / inputs->tensorBuffer->elementSize), 64 / inputs->tensorBuffer->elementSize);
            vx_tensor convertedTensor = vxoTensor_CreateTensor(node->base.context, 1, &size, inputs->tensorBuffer->dataFormat, vx_false_e);

            vxnneOperation_Initialize(&fullyConnectReluLayer->input_convert_weight_operation.base,
                                        &fullyConnectReluLayer->base,
                                        VXNNE_OPERATION_TARGET_SW,
                                        VXNNE_OPERATOR_INPUT2WEIGHT,
                                        vxnneExecuteSWInputConvertWeight,
                                        VX_NULL);

            if (vxoTensor_AllocateMemory(convertedTensor) != VX_SUCCESS)
            {
                gcmPRINT("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
                status = VX_ERROR_NO_MEMORY;
                goto exit;
            }

            fullyConnectReluLayer->base.num_operations = 2;

            fullyConnectReluLayer->operations[0] = (vxnne_operation)&fullyConnectReluLayer->input_convert_weight_operation;
            fullyConnectReluLayer->input_convert_weight_operation.inputs                   = inputs;
            fullyConnectReluLayer->input_convert_weight_operation.weights_biases           = weights_biases;
            fullyConnectReluLayer->input_convert_weight_operation.outputs                  = convertedTensor;
            fullyConnectReluLayer->input_convert_weight_operation.enable_relu              = enable_relu;
            fullyConnectReluLayer->input_convert_weight_operation.output_fp_pos            = outputs->tensorBuffer->fixedPointPos;

            fullyConnectReluLayer->operations[1] = (vxnne_operation)&fullyConnectReluLayer->fully_connected_relu_operation;
            fullyConnectReluLayer->fully_connected_relu_operation.inputs                   = convertedTensor;
            fullyConnectReluLayer->fully_connected_relu_operation.weights_biases           = weights_biases;
            fullyConnectReluLayer->fully_connected_relu_operation.outputs                  = outputs;
            convertedTensor->tensorBuffer->fixedPointPos                                   = inputs->tensorBuffer->fixedPointPos;

            fullyConnectReluLayer->base.num_temp_tensors                                   = 1;
            fullyConnectReluLayer->base.temp_tensors[0]                                    = convertedTensor;
        }
        else
        {
            fullyConnectReluLayer->base.num_operations = 1;
            fullyConnectReluLayer->operations[0] = (vxnne_operation)&fullyConnectReluLayer->fully_connected_relu_operation;
            fullyConnectReluLayer->fully_connected_relu_operation.inputs                   = inputs;
            fullyConnectReluLayer->fully_connected_relu_operation.weights_biases           = weights_biases;
            fullyConnectReluLayer->fully_connected_relu_operation.pad                      = pad;
            fullyConnectReluLayer->fully_connected_relu_operation.down_scale_size_rounding = conv_rounding_type;
            fullyConnectReluLayer->fully_connected_relu_operation.outputs                  = outputs;
        }

        node->layer = &fullyConnectReluLayer->base;

        node->layer->cmdNNBuff = vxCreateArray(context, VX_TYPE_CHAR, NNE_COMMAND_SIZE * weights_biases->zgroup_num);
        if (!vxoArray_AllocateMemory(node->layer->cmdNNBuff))
        {
            status |= VX_ERROR_NO_MEMORY;
            gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            goto exit;
        }

        for (i = 0; i < weights_biases->zgroup_num; i++)
        {
            /* fill in cmd buffer */
            fillinCmmdBuff(inputs,
                           weights_biases,
                           pad, pad, pad, pad,
                           VX_PAD_CONSTANT,
                           VX_NULL,
                           conv_rounding_type,
                           enable_relu,
                           VIV_NN_NONLINEAR_NON,
                           0,
                           0,
                           outputs,
                           node->layer->cmdNNBuff,
                           vx_true_e,
                           i);
        }
    }

    return status;

exit:
    if (fullyConnectReluLayer) gcoOS_Free(gcvNULL, (gctPOINTER)fullyConnectReluLayer);
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedReluLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }
    return VX_SUCCESS;
}

vx_status VX_CALLBACK vxoBaseKernel_NNSoftmaxLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
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
    vx_enum   inputFormat                 = inputs->tensorBuffer->dataFormat;
    vx_enum   outputFormat                = outputs->tensorBuffer->dataFormat;
    vx_uint32  width                      = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
    vx_uint32  height                     = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
    vx_uint32  channel                    = TENSOR_VIEW_SIZE_INDEX(inputs, 2);
    vx_uint32  batch                      = TENSOR_VIEW_SIZE_INDEX(inputs, 3);
    vx_bool    useShadeExe                = vx_false_e;
    vxnne_softmax_layer  softmaxLayer = gcvNULL;

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
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(softmaxLayer, sizeof(vxnne_softmax_layer_s));

    vxnneLayer_Initialize(&softmaxLayer->base,
                          "SoftmaxLayer",
                          node,
                          softmaxLayer->operations,
                          VX_NULL);
#define IMAGE_SIZE_4K  (4096)
    if(batch == 0)
        batch = 1;
    useShadeExe = ((inputFormat == VX_TYPE_INT8 ||  inputFormat == VX_TYPE_FLOAT16) && (outputFormat == VX_TYPE_FLOAT16 || outputFormat == VX_TYPE_FLOAT32) && channel <= IMAGE_SIZE_4K && width == 1 && height == 1 && batch == 1)
                || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16 && channel == 2);
   if(useShadeExe)
   {
        vxnne_shader_executable shaderExecutable;

        shaderExecutable = vxnneGetSoftmaxShaderExecutable(node->base.context, VXNNE_KERNEL_SOFTMAX, &node->kernelAttributes.borderMode, inputs, outputs);

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&softmaxLayer->softmax_SHoperation,
            &softmaxLayer->base,
            VXNNE_OPERATOR_SOFTMAX,
            shaderExecutable);

        if (status != VX_SUCCESS) {
            goto exit;
        }

        softmaxLayer->base.num_operations = 1;
        softmaxLayer->operations[0] = &softmaxLayer->softmax_SHoperation.base;

    }
    else
    {
        vxnneOperation_Initialize(&softmaxLayer->softmax_sw_operation.base,
            &softmaxLayer->base,
            VXNNE_OPERATION_TARGET_SW,
            VXNNE_OPERATOR_SOFTMAX,
            vxnneExecuteSWSoftmax,
            VX_NULL);

        softmaxLayer->base.num_operations    = 1;
        softmaxLayer->operations[0] = (vxnne_operation)&softmaxLayer->softmax_sw_operation;

        softmaxLayer->softmax_sw_operation.inputs           = inputs;
        softmaxLayer->softmax_sw_operation.outputs          = outputs;
    }

    node->layer = &softmaxLayer->base;
    return status;

exit:
    if (softmaxLayer) gcoOS_Free(gcvNULL, (gctPOINTER)softmaxLayer);
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoSoftmaxLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNConcat2Layer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcat2Layer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcat2Layer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcat2Layer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status  status                     = VX_SUCCESS;

    vx_tensor  in0                     = (vx_tensor)parameters[0];
    vx_tensor  in1                     = (vx_tensor)parameters[1];
    vx_tensor  out                     = (vx_tensor)parameters[2];

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    {
        vxnne_concat2_layer  concatLayer;

        gcoOS_Allocate(gcvNULL, sizeof(vxnne_concat2_layer_s), (gctPOINTER*)&concatLayer);
        if (!concatLayer)
        {
            status = VX_ERROR_NO_MEMORY;
            gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            goto exit;
        }

        gcoOS_ZeroMemory(concatLayer, sizeof(vxnne_concat2_layer_s));

        vxnneLayer_Initialize(&concatLayer->base,
                              "ConcatLayer",
                              node,
                              concatLayer->operations,
                              VX_NULL);

        vxnneOperation_Initialize(&concatLayer->concat2_operation.base,
                                  &concatLayer->base,
                                  VXNNE_OPERATION_TARGET_SW,
                                  VXNNE_OPERATOR_CONCAT2,
                                  vxnneExecuteSWConcat2,
                                  VX_NULL);

        concatLayer->base.num_operations    = 1;
        concatLayer->operations[0] = (vxnne_operation)&concatLayer->concat2_operation;

        concatLayer->concat2_operation.inputs0           = in0;
        concatLayer->concat2_operation.inputs1           = in1;
        concatLayer->concat2_operation.outputs           = out;

        node->layer = &concatLayer->base;
    }

exit:
    return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcat2Layer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorCopy(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorCopy_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorCopy_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorCopy_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status  status                     = VX_SUCCESS;

    vx_tensor  src                     = (vx_tensor)parameters[0];
    vx_tensor  dst                     = (vx_tensor)parameters[1];

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    {
        vxnne_tensor_copy  copyNode;

        gcoOS_Allocate(gcvNULL, sizeof(vxnne_tensor_copy_s), (gctPOINTER*)&copyNode);
        if (!copyNode)
        {
            status = VX_ERROR_NO_MEMORY;
            gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            goto exit;
        }

        gcoOS_ZeroMemory(copyNode, sizeof(vxnne_tensor_copy_s));

        vxnneLayer_Initialize(&copyNode->base,
                              "TensorCopy",
                              node,
                              copyNode->operations,
                              VX_NULL);

        vxnneOperation_Initialize(&copyNode->tensor_copy_operation.base,
                                  &copyNode->base,
                                  VXNNE_OPERATION_TARGET_SW,
                                  VXNNE_OPERATOR_TENSOR_COPY,
                                  vxnneExecuteSWTensorCopy,
                                  VX_NULL);

        copyNode->base.num_operations    = 1;
        copyNode->operations[0] = (vxnne_operation)&copyNode->tensor_copy_operation;

        copyNode->tensor_copy_operation.src           = src;
        copyNode->tensor_copy_operation.dst           = dst;

        node->layer = &copyNode->base;
    }

exit:
    return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorCopy_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNNormalization(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    status = vxnneLayer_Execute(node->layer);

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNormalization_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxoNormalization_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    if (index != vxmLENGTH_OF(nn_Normalization_params) - 1)
       return VX_ERROR_INVALID_PARAMETERS;

    ptr->type = VX_TYPE_TENSOR;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNormalization_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vx_context context = vxGetContext((vx_reference)node);

    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_scalar  type_s                     = (vx_scalar)parameters[1];
    vx_scalar  norm_size_s                = (vx_scalar)parameters[2];
    vx_scalar  alpha_s                    = (vx_scalar)parameters[3];
    vx_scalar  beta_s                     = (vx_scalar)parameters[4];
    vx_tensor  outputs                    = (vx_tensor)parameters[5];

    vx_enum    norm_type                  = type_s->value->e;
    vx_uint32  norm_size                  = norm_size_s->value->u32;
    vx_enum    inputFormat                = inputs->tensorBuffer->dataFormat;
    vx_enum    outputFormat               = outputs->tensorBuffer->dataFormat;
    vx_bool    sammap_flag                = vx_false_e;
    vx_bool    acrossmap_flag             = vx_false_e;
    vx_bool    dataformat_flag            = vx_false_e;
    vxnne_normalization_layer  normalizationLayer;
    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }
    gcoOS_Allocate(gcvNULL, sizeof(vxnne_normalization_layer_s), (gctPOINTER*)&normalizationLayer);
    if (!normalizationLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(normalizationLayer, sizeof(vxnne_normalization_layer_s));

    vxnneLayer_Initialize(&normalizationLayer->base,
                          "NormalizationLayer",
                          node,
                          normalizationLayer->operations,
                          VX_NULL);

    if (context->nnConfig.tpCoreCount && context->options.enableTP && context->options.flagTPFunc[TP_LRN])
    {
        vx_uint32 i, slice;
        vx_tp_value_cmd values;
        vx_array lut = normalizationLayer->normalization_tp_operation.buffer;

        vxnneOperation_Initialize(&normalizationLayer->normalization_tp_operation.base,
                                  &normalizationLayer->base,
                                  VXNNE_OPERATION_TARGET_TP,
                                  VXNNE_OPERATOR_NORMALIZATION,
                                  vxnneExecuteTPNormalization,
                                  vxnneOperation_TP_Deinitialize);

        slice = type_s->value->e == VX_CONVOLUTIONAL_NETWORK_NORM_ACROSS_MAPS ?
                                    1 : (context->options.enableMultiTP && context->nnConfig.tpCoreCount > 1 ? TENSOR_SIZE_INDEX(inputs, 2) : 1);

        /* create cmd buffer for TP operation */
        normalizationLayer->base.cmdTPBuff = vxCreateArray(context, VX_TYPE_CHAR, TP_COMMAND_SIZE * slice);
        if (!vxoArray_AllocateMemory(normalizationLayer->base.cmdTPBuff))
        {
            status = VX_ERROR_NO_MEMORY;
            gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            goto exit;
        }

        if (lut == VX_NULL)
        {
            /* Prepare PWL LUT. */
            lut = vxCreateArray(context, VX_TYPE_UINT16, TP_LUT_BUFF_SIZE);
            if (!vxoArray_AllocateMemory(lut))
            {
                status = VX_ERROR_NO_MEMORY;
                gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
                goto exit;
            }
            lut->itemCount = TP_LUT_BUFF_SIZE;
            lut->base.isStage = vx_true_e;
        }

        values.e32[0] = type_s->value->e;
        values.u32[0] = norm_size_s->value->u32;
        values.f32[0] = alpha_s->value->f32;
        values.f32[1] = beta_s->value->f32;
        for (i = 0; i < slice; i++)
        {
            fillInCmdTPBuffer(
                    inputs, VX_NULL, outputs,
                    normalizationLayer->base.cmdTPBuff, lut,
                    VX_NULL, &values,
                    TP_LRN,
                    i, slice > 1,
                    vx_true_e);
        }

        normalizationLayer->base.num_operations = 1;
        normalizationLayer->operations[0] = (vxnne_operation)&normalizationLayer->normalization_tp_operation;
        normalizationLayer->normalization_tp_operation.inputs[0]    = inputs;
        normalizationLayer->normalization_tp_operation.buffer       = lut;
        normalizationLayer->normalization_tp_operation.outputs[0]   = outputs;
        normalizationLayer->normalization_tp_operation.op_num       = slice;
        normalizationLayer->normalization_tp_operation.multi_tensor = vx_false_e;
    }
    else
    {
        sammap_flag     = (vx_bool)((norm_type == VX_CONVOLUTIONAL_NETWORK_NORM_SAME_MAP) && (norm_size == 3));
        acrossmap_flag  = (vx_bool)((norm_type == VX_CONVOLUTIONAL_NETWORK_NORM_ACROSS_MAPS) && (norm_size == 5));
        dataformat_flag = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 || inputFormat == VX_TYPE_INT8) && (outputFormat == VX_TYPE_FLOAT16 || outputFormat == VX_TYPE_INT8));
        if(dataformat_flag && (sammap_flag || acrossmap_flag)) // shader inplement
        {
            vxnne_shader_executable shaderExecutable;
            shaderExecutable = vxnneGetNormalizationShaderExecutable(node->base.context, VXNNE_KERNEL_NORMALIZATION, &node->kernelAttributes.borderMode, inputs, type_s, norm_size_s, alpha_s, beta_s, outputs);
            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto exit;
            }

            status = vxnneShaderOperation_Initialize(&normalizationLayer->normalization_sh_operation,
                                          &normalizationLayer->base,
                                          VXNNE_OPERATOR_NORMALIZATION,
                                          shaderExecutable);

            if (status != VX_SUCCESS) goto exit;
            normalizationLayer->base.num_operations = 1;
            normalizationLayer->operations[0] = &normalizationLayer->normalization_sh_operation.base;
        }

        else //NNE inplement
        {
            vxnneOperation_Initialize(&normalizationLayer->normalization_sw_operation.base,
                                      &normalizationLayer->base,
                                      VXNNE_OPERATION_TARGET_SW,
                                      VXNNE_OPERATOR_NORMALIZATION,
                                      vxnneExecuteSWNormalization,
                                      VX_NULL);

            normalizationLayer->base.num_operations    = 1;
            normalizationLayer->operations[0] = (vxnne_operation)&normalizationLayer->normalization_sw_operation;

            normalizationLayer->normalization_sw_operation.inputs           = inputs;
            normalizationLayer->normalization_sw_operation.type             = type_s->value->e;
            normalizationLayer->normalization_sw_operation.norm_size        = norm_size_s->value->u32;
            normalizationLayer->normalization_sw_operation.alpha            = alpha_s->value->f32;
            normalizationLayer->normalization_sw_operation.beta             = beta_s->value->f32;
            normalizationLayer->normalization_sw_operation.outputs          = outputs;
        }
    }

    node->layer = &normalizationLayer->base;
    return status;

exit:
    if (normalizationLayer != NULL)
        gcoOS_Free(NULL, normalizationLayer);

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNormalization_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNNormalizeImage(vx_node node, const vx_reference *parameters, vx_uint32 num)
{

  /* Need rename this kernel.Uuse this to do softmax */
    vx_tensor inputs_t    = (vx_tensor)parameters[0];        /* fp16*/
    vx_tensor outputs_t   = (vx_tensor)parameters[1];        /* fp16 */

    vx_int16 *inputs          = (vx_int16*)(inputs_t->tensorBuffer->memory.logicals[0]);
    vx_int16 *outputs         = (vx_int16*)(outputs_t->tensorBuffer->memory.logicals[0]);

#if defined(__linux__)
    struct timeval start = gcfVX_PerfStart((vx_reference)node);
#endif
    vxoTensor_GetTensorViewMemory(inputs_t, (gctPOINTER *)&inputs, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs_t, (gctPOINTER *)&outputs, VX_NULL);
    {
        vx_uint32 width   = TENSOR_SIZE_INDEX(inputs_t, 0);
        vx_uint32 height  = TENSOR_SIZE_INDEX(inputs_t, 1);
        vx_uint32 channel = TENSOR_SIZE_INDEX(inputs_t, 2);
        vx_uint32 batch   = TENSOR_SIZE_INDEX(inputs_t, 3);
        vx_uint32 w=0,h=0,c=0,b=0;
        vx_uint32 stridec =  TENSOR_STRIDE_INDEX(inputs_t, 2)/2;
        if(batch == 0)batch = 1;
        for(b = 0; b < batch; b++)
        {
            vx_float32 sum=0;
            vx_float32 val;
            vx_float32 nmsqr ;
            for(c = 0; c < channel; c++)
            {
                 for(h=0;h<height;h++)
                 {
                    for(w=0;w<width;w++)
                    {
                        val = Fp16toFp32(inputs[(b*channel + c)*stridec + width*h + w]);
                        sum += val*val;
                    }
                 }
            }
            nmsqr = (vx_float32)(1.0/sqrtf(sum));
            for(c = 0; c < channel; c++)
            {
                 for(h=0;h<height;h++)
                 {
                    for(w=0;w<width;w++)
                    {
                        val = Fp16toFp32(inputs[(b*channel + c)*stridec + width*h + w]);
                        val = val * nmsqr;
                        outputs[(b*channel + c)*stridec + width*h + w] = Fp32toFp16(val);
                    }
                 }
            }
        }
    }
#if defined(__linux__)
    if (node->base.context->options.enableCNNPerf)
        printf("normalization image        CPU  time:%10d us\n", gcfVX_PerfEnd((vx_reference)node, start));
#endif

    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxoNormalizeImage_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNormalizeImage_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    if (index != vxmLENGTH_OF(nn_NormalizeImage_params) - 1)
       return VX_ERROR_INVALID_PARAMETERS;

    ptr->type = VX_TYPE_TENSOR;
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNPoolingLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    if (index != vxmLENGTH_OF(nn_PoolingLayer_params) - 1) return VX_ERROR_INVALID_PARAMETERS;

    ptr->type                 = VX_TYPE_TENSOR;

    return VX_SUCCESS;
}

vx_status vxnnePoolingOperation_Deinitialize(vxnne_operation_s *operation)
{
    vxnne_pooling_operation pooling_operation = (vxnne_pooling_operation)operation;

    if (pooling_operation->weights_biases != VX_NULL)
    {
        vxReleaseWeightsBiasesParameter(&pooling_operation->weights_biases);
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_scalar  poolType                   = (vx_scalar)parameters[1];
    vx_scalar  poolSizeX                  = (vx_scalar)parameters[2];
    vx_scalar  poolSizeY                  = (vx_scalar)parameters[3];
    vx_scalar  poolPadX                   = (vx_scalar)parameters[4];
    vx_scalar  poolPadY                   = (vx_scalar)parameters[5];
    vx_scalar  rounding                   = (vx_scalar)parameters[6];
    vx_tensor  outputs                    = (vx_tensor)parameters[7];

    vx_status  status                     = VX_SUCCESS;

    vx_enum poolTypeValue    = poolType->value->e;
    vx_uint32 poolSizeXValue = poolSizeX->value->u32;
    vx_uint32 poolSizeYValue = poolSizeY->value->u32;
    vx_enum roundingValue    = rounding->value->e;
    vx_enum inputdata_format  = inputs->tensorBuffer->dataFormat;
    vx_enum outputdata_format = outputs->tensorBuffer->dataFormat;
    vx_bool kernel_AvgPool_flag[6] = {vx_false_e};
    vx_bool dataFormat_AvgPool_flag[4] = {vx_false_e};
    vx_bool globalPool_flag = vx_false_e;
    vx_bool avgPool_flag = vx_false_e;

    vx_uint32 inputsWidth, inputsHeight, outputsWidth, outputsHeight;
    vx_int32  inputsDepth, outputsDepth;
    vx_uint32 poolPadXLeftValue, poolPadXRightValue, poolPadYTopValue, poolPadYBottomValue;
    vxnne_pooling_layer  poolingLayer = gcvNULL;
    vx_uint32  stride = 0;
    vx_uint32 totalSize = 0;
    vx_uint32 maxAllocateSize = 256 * 1024 * 1024; /* set max allocate size because fpga out of memory when using nn do avg pooling, max value is 256M */

    vxnneGetPadValue(poolPadX->value->n32, poolPadY->value->n32, &poolPadXLeftValue, &poolPadXRightValue, &poolPadYTopValue, &poolPadYBottomValue);

    inputsWidth   = TENSOR_SIZE_INDEX(inputs, 0);
    inputsHeight  = TENSOR_SIZE_INDEX(inputs, 1);
    inputsDepth   = TENSOR_SIZE_INDEX(inputs, 2);
    outputsWidth  = TENSOR_SIZE_INDEX(outputs, 0);;
    outputsHeight = TENSOR_SIZE_INDEX(outputs, 1);
    outputsDepth  = TENSOR_SIZE_INDEX(outputs, 2);

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_pooling_layer_s), (gctPOINTER*)&poolingLayer);
    if (!poolingLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(poolingLayer, sizeof(vxnne_pooling_layer_s));

    poolingLayer->base.name                  = "PoolingLayer";
    poolingLayer->base.node                  = node;
    poolingLayer->base.operations            = poolingLayer->operations;

    poolingLayer->base.num_temp_tensors      = 0;

    poolingLayer->base.dump                  = VX_NULL;
    poolingLayer->base.deinitialize          = vxnneLayer_Deinitialize;

    poolingLayer->base.num_operations    = 1;

    if (outputsWidth == 1)
    {
        stride = 1;
    }
    else
    {
        stride = vxoNNExternsionConvlutionRound((vx_float32)(inputsWidth + poolPadXLeftValue + poolPadXRightValue - poolSizeXValue) / (outputsWidth - 1), roundingValue);
    }

    totalSize = poolSizeXValue * poolSizeYValue * inputsDepth * outputsDepth * (vx_uint32)vxDataType_GetSize((vx_type_e)inputs->tensorBuffer->dataFormat) + outputsDepth * sizeof(vx_float32);

    globalPool_flag            = (vx_bool)(poolSizeXValue == inputsWidth && poolSizeYValue ==  inputsHeight && outputsWidth == 1 && outputsHeight == 1);
    kernel_AvgPool_flag[0]     = (vx_bool)(stride == 1 && poolSizeXValue == 13 && poolPadXLeftValue == 0);
    kernel_AvgPool_flag[1]     = (vx_bool)(stride == 1 && poolSizeXValue == 7 && poolPadXLeftValue == 0);
    kernel_AvgPool_flag[2]     = (vx_bool)(stride == 1 && poolSizeXValue == 6 && poolPadXLeftValue == 0);
    kernel_AvgPool_flag[3]     = (vx_bool)(poolSizeXValue <= 8);
    kernel_AvgPool_flag[4]     = (vx_bool)(poolSizeXValue <= 13);
    kernel_AvgPool_flag[5]     = (vx_bool)(stride == 1 && poolSizeXValue == 3 && poolPadXLeftValue == 1);
    dataFormat_AvgPool_flag[0] = (vx_bool)(inputdata_format == VX_TYPE_INT8 && outputdata_format == VX_TYPE_FLOAT16);
    dataFormat_AvgPool_flag[1] = (vx_bool)(inputdata_format == VX_TYPE_FLOAT16 && outputdata_format == VX_TYPE_INT8);
    dataFormat_AvgPool_flag[2] = (vx_bool)(inputdata_format == VX_TYPE_INT8 && outputdata_format == VX_TYPE_INT8);
    dataFormat_AvgPool_flag[3] = (vx_bool)(inputdata_format == VX_TYPE_FLOAT16 && outputdata_format == VX_TYPE_FLOAT16);
    avgPool_flag   = (vx_bool)(((kernel_AvgPool_flag[0] && dataFormat_AvgPool_flag[0])
                            || (kernel_AvgPool_flag[1] && dataFormat_AvgPool_flag[1])
                            || (kernel_AvgPool_flag[1] && dataFormat_AvgPool_flag[2])
                            || (kernel_AvgPool_flag[2] && dataFormat_AvgPool_flag[0])
                            || (kernel_AvgPool_flag[5] && dataFormat_AvgPool_flag[2])
                            || (kernel_AvgPool_flag[5] && dataFormat_AvgPool_flag[3])
                            || (kernel_AvgPool_flag[3] && dataFormat_AvgPool_flag[2] && globalPool_flag)
                            || (kernel_AvgPool_flag[3] && dataFormat_AvgPool_flag[1] && globalPool_flag)
                            || (kernel_AvgPool_flag[4] && dataFormat_AvgPool_flag[3] && globalPool_flag)
                            || (kernel_AvgPool_flag[4] && dataFormat_AvgPool_flag[0] && globalPool_flag))
                            && (poolTypeValue == VX_CONVOLUTIONAL_NETWORK_POOLING_AVG));

    /* if the needed total size is larger than maxAllocateSize, do pooling with CPU version. maybe need implement avg pooling with shader */
    if ((poolTypeValue == VX_CONVOLUTIONAL_NETWORK_POOLING_AVG) && (stride == 1) && totalSize <= maxAllocateSize && avgPool_flag == vx_false_e)
    {
        /* nne only support NxN average pooling and stride = 1 */
        poolingLayer->pooling_nne_operation.base.layer         = &poolingLayer->base;
        poolingLayer->pooling_nne_operation.base.target        = VXNNE_OPERATION_TARGET_NN;
        poolingLayer->pooling_nne_operation.base.operatorType = VXNNE_OPERATOR_POOLING;
        poolingLayer->pooling_nne_operation.base.execute       = vxnneExecutePooling;
        poolingLayer->pooling_nne_operation.base.deinitialize  = vxnnePoolingOperation_Deinitialize;
        poolingLayer->pooling_nne_operation.base.dump          = VX_NULL;

        poolingLayer->operations[0] = (vxnne_operation)&poolingLayer->pooling_nne_operation;

        poolingLayer->pooling_nne_operation.inputs            = inputs;
        poolingLayer->pooling_nne_operation.pool_type         = poolTypeValue;
        poolingLayer->pooling_nne_operation.pool_size_x       = poolSizeXValue;
        poolingLayer->pooling_nne_operation.pool_size_y       = poolSizeYValue;
        poolingLayer->pooling_nne_operation.pool_pad_x_left   = poolPadXLeftValue;
        poolingLayer->pooling_nne_operation.pool_pad_x_right  = poolPadXRightValue;
        poolingLayer->pooling_nne_operation.pool_pad_y_top    = poolPadYTopValue;
        poolingLayer->pooling_nne_operation.pool_pad_y_bottom = poolPadYBottomValue;
        poolingLayer->pooling_nne_operation.rounding          = roundingValue;
        poolingLayer->pooling_nne_operation.outputs           = outputs;

        /* prepare data for nne */
        {
            vx_int8 *weightData = VX_NULL;
            vx_float32 *biasData = VX_NULL;
            vx_uint32 i, j;
            vx_int32 w, z;
            vx_int8 *weightsValuePtr;
            vx_int8 *zerosValuePtr;
            vx_uint32 weightItemCount;
            vx_int8 *pWeightData;
            vx_float32 *pBiasData;
            vx_tensor weights = VX_NULL;
            vx_tensor biases = VX_NULL;
            vx_context context = VX_NULL;
            vx_uint32 numWeightDims = 4, numBiasDims = 1;
            vx_uint32 weightSize[4] = {poolSizeXValue, poolSizeYValue, inputsDepth, outputsDepth};
            vx_uint32 weightStrideSize[4];
            vx_uint32 biasSize[4] = {outputsDepth};
            vx_uint32 biasStrideSize[1];
            vx_tensor_addressing weightUserAddr = NULL;
            vx_tensor_addressing biasUserAddr = NULL;
            vx_weights_biases_parameter weights_biases = VX_NULL;
            vx_type_e inputDataFormat = (vx_type_e)inputs->tensorBuffer->dataFormat;
            vx_type_e weightDataFormat = inputDataFormat;
            vx_type_e biasDataFormat = (weightDataFormat == VX_TYPE_INT8) ? VX_TYPE_INT32 : VX_TYPE_FLOAT32;
            vx_int32 weightItemSize = vxnneGetTypeSize(weightDataFormat);
            vx_int32 biasItemSize = vxnneGetTypeSize(biasDataFormat);
            vx_int8 weightFixPointPos, biasFixPointPos;
            vx_enum weightRoundingMode;

            context = vxGetContext((vx_reference)node);
            if (context == VX_NULL)
            {
                gcmPRINT("vxGetContext fail at function %s line %d", __FUNCTION__, __LINE__);
                goto exit;
            }

            /* generate special weight and bias data for average pooling. create weightsBiasesParameters from this specail weight and bias */
#if VX_C_MEMORY_MANAGE

            vxoMemory_CAllocate(node->base.context, (void**)&weightsValuePtr, poolSizeXValue * poolSizeYValue * weightItemSize);
            vxoMemory_CAllocate(node->base.context, (void**)&zerosValuePtr, poolSizeXValue * poolSizeYValue * weightItemSize);

            vxoMemory_CAllocate(node->base.context, (void**)&weightData, poolSizeXValue * poolSizeYValue * inputsDepth * outputsDepth * weightItemSize);
            vxoMemory_CAllocate(node->base.context, (void**)&biasData, outputsDepth * biasItemSize);

#else
            weightsValuePtr = (vx_int8*)malloc(poolSizeXValue * poolSizeYValue * weightItemSize);
            zerosValuePtr = (vx_int8*)malloc(poolSizeXValue * poolSizeYValue * weightItemSize);

            weightData = (vx_float32*)malloc(pool_size_x_v * pool_size_y_v * inputs_depth * outputs_depth * weightItemSize);
            biasData   = (vx_float32*)malloc(outputs_depth * biasItemSize);
#endif

            weightItemCount = poolSizeXValue * poolSizeYValue;

            weightFixPointPos = (vx_int8)(8 - gcoMATH_Ceiling(gcoMATH_Log(1.0f/weightItemCount) + 1));
            biasFixPointPos = weightFixPointPos + (vx_int8)inputs->tensorBuffer->fixedPointPos;

            weights = vxCreateTensor(context, numWeightDims, weightSize, weightDataFormat, weightFixPointPos);

            weightRoundingMode = weights->tensorBuffer->roundingMode;

            for (j = 0; j < weightItemCount; j++)
            {
                vxnneSaveData(weightDataFormat, j, 1.0f/weightItemCount, weightsValuePtr, weightFixPointPos, weightRoundingMode);
                vxnneSaveData(weightDataFormat, j, 0.0f, zerosValuePtr, weightFixPointPos, weightRoundingMode);
            }

            pWeightData = weightData;
            pBiasData = biasData;

            for (w = 0; w < outputsDepth; w++)
            {
                for (z = 0; z < inputsDepth; z++)
                {
                    if (w == z)
                    {
                        memcpy(pWeightData, weightsValuePtr, weightItemCount * weightItemSize);
                    }
                    else
                    {
                        memcpy(pWeightData, zerosValuePtr, weightItemCount * weightItemSize);
                    }
                    pWeightData += weightItemCount * weightItemSize;
                }
                *pBiasData++ = 0.0f;
            }

            weightStrideSize[0] = weightItemSize;
            for (i = 1; i < numWeightDims; i++)
            {
                weightStrideSize[i] =  weightStrideSize[i-1] * weightSize[i-1];
            }


            weightUserAddr = vxCreateTensorAddressing(context, weightSize, weightStrideSize, (vx_uint8)numWeightDims);

            vxCopyTensorPatch(weights, VX_NULL, weightUserAddr, weightData, VX_WRITE_ONLY,0);

            biasStrideSize[0] = biasItemSize;
            biases = vxCreateTensor(context, numBiasDims, biasSize, biasDataFormat, biasFixPointPos);
            biasUserAddr = vxCreateTensorAddressing(context, biasSize, biasStrideSize, (vx_uint8)numBiasDims);

            vxCopyTensorPatch(biases, VX_NULL, biasUserAddr, biasData, VX_WRITE_ONLY,0);

            weights_biases = _createWeightsBiasesParameterFromTensors(context,
                                                                       VX_CONVOLUTIONAL_NETWORK_CONVOLUTION_LAYER,
                                                                       TENSOR_DIM_NUM(inputs),
                                                                       (vx_uint32*)(TENSOR_SIZES(inputs)),
                                                                       poolPadX->value->n32,
                                                                       poolPadX->value->n32,
                                                                       poolPadY->value->n32,
                                                                       poolPadY->value->n32,
                                                                       0,
                                                                       0,
                                                                       roundingValue,
                                                                       (vx_uint32*)(TENSOR_SIZES(outputs)),
                                                                       VX_NULL,
                                                                       VX_NULL,
                                                                       weights,
                                                                       biases);

#if VX_C_MEMORY_MANAGE

            if (weightsValuePtr != VX_NULL)
                vxoMemory_CFree(node->base.context, (void**)&weightsValuePtr);

            if (zerosValuePtr != VX_NULL)
                vxoMemory_CFree(node->base.context, (void**)&zerosValuePtr);

            if (weightData != VX_NULL)
                vxoMemory_CFree(node->base.context, (void**)&weightData);

            if (biasData != VX_NULL)
                vxoMemory_CFree(node->base.context, (void**)&biasData);
#else
            if (weightsValuePtr != VX_NULL)
                free(&weightsValuePtr);
            if (zerosValuePtr != VX_NULL)
                free(zerosValuePtr);
            if (weightData != VX_NULL)
                free(weightData);
            if (biasData != VX_NULL)
                free(biasData);
#endif

            /* create cmd buffer for nn  layer */
            poolingLayer->base.cmdNNBuff = vxCreateArray(context, VX_TYPE_CHAR, NNE_COMMAND_SIZE);
            if (!vxoArray_AllocateMemory(poolingLayer->base.cmdNNBuff))
            {
                status = VX_ERROR_NO_MEMORY;
                gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
                goto exit;
            }

            /* fill in cmd buffer */
            fillinCmmdBuff(inputs,
                           weights_biases,
                           poolPadXLeftValue,
                           poolPadXRightValue,
                           poolPadYTopValue,
                           poolPadYBottomValue,
                           VX_PAD_CONSTANT,
                           VX_NULL,
                           roundingValue,
                           vx_false_e,
                           VIV_NN_POOLING_NON,
                           0,
                           0,
                           outputs,
                           poolingLayer->base.cmdNNBuff,
                           vx_false_e,
                           0);

            poolingLayer->pooling_nne_operation.weights_biases = weights_biases;

            if (weights != VX_NULL)
            {
                vxReleaseTensor(&weights);
            }

            if (biases != VX_NULL)
            {
                vxReleaseTensor(&biases);
            }

            if (weightUserAddr != VX_NULL)
            {
                vxReleaseTensorAddressing(&weightUserAddr);
            }

            if (biasUserAddr != VX_NULL)
            {
                vxReleaseTensorAddressing(&biasUserAddr);
            }
        }
    }
    else
    {
        vx_context context = vxGetContext((vx_reference)node);

        /* stride!=2 is not supported yet */
        if (poolTypeValue == VX_CONVOLUTIONAL_NETWORK_POOLING_MAX &&
            context->nnConfig.tpCoreCount && context->options.enableTP && context->options.flagTPFunc[TP_MAX_POOLING] &&
            (stride == 2 && (stride == poolSizeXValue || stride == poolSizeXValue-1) && poolSizeXValue <= 64))
        {
            vx_uint32 i, slice;
            vx_tp_conv_cmd conv;

            slice = context->options.enableMultiTP && context->nnConfig.tpCoreCount > 1 ? TENSOR_SIZE_INDEX(inputs, 2) : 1;

            vxnneOperation_Initialize(
                    &poolingLayer->pooling_tp_operation.base,
                    &poolingLayer->base,
                    VXNNE_OPERATION_TARGET_TP,
                    VXNNE_OPERATOR_POOLING,
                    vxnneExecuteTPPooling,
                    VX_NULL);

            /* create cmd buffer for TP operation */
            poolingLayer->base.cmdTPBuff = vxCreateArray(context, VX_TYPE_CHAR, TP_COMMAND_SIZE * slice);
            if (!vxoArray_AllocateMemory(poolingLayer->base.cmdTPBuff))
            {
                status = VX_ERROR_NO_MEMORY;
                gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
                goto exit;
            }

            conv.pad_x = poolPadXLeftValue;
            conv.pad_y = poolPadYTopValue;
            conv.pool_size_x = poolSizeXValue;
            conv.pool_size_y = poolSizeYValue;
            conv.pool_stride = stride;
            conv.enable_relu = vx_false_e;
            conv.conv_rounding_type = roundingValue;
            for (i = 0; i < slice; i++)
            {
                fillInCmdTPBuffer(
                        inputs, VX_NULL, outputs,
                        poolingLayer->base.cmdTPBuff, VX_NULL,
                        &conv, VX_NULL,
                        TP_MAX_POOLING,
                        i, slice > 1,
                        vx_true_e);
            }

            poolingLayer->base.num_operations = 1;
            poolingLayer->operations[0] = (vxnne_operation)&poolingLayer->pooling_tp_operation;
            poolingLayer->pooling_tp_operation.inputs[0]    = inputs;
            poolingLayer->pooling_tp_operation.outputs[0]   = outputs;
            poolingLayer->pooling_tp_operation.op_num       = slice;
            poolingLayer->pooling_tp_operation.multi_tensor = vx_false_e;
        }
        else
        {
            vx_bool kernel_MaxPool_flag     = vx_false_e;
            vx_bool dataformat_MaxPool_flag = vx_false_e;
            vx_bool maxPool_flag            = vx_false_e;
            kernel_MaxPool_flag     = (vx_bool)((stride == 2 && poolSizeXValue == 3 && poolPadXLeftValue == 1) || (stride == 2 && poolSizeXValue == 2 && poolPadXLeftValue == 0)
                                       || (stride == 2 && poolSizeXValue == 3 && poolPadXLeftValue == 0) || (stride == 1 && poolSizeXValue == 3 && poolPadXLeftValue == 1));
            dataformat_MaxPool_flag = (vx_bool)((inputdata_format == VX_TYPE_FLOAT16 || inputdata_format == VX_TYPE_INT8)
                                       && (outputdata_format == VX_TYPE_FLOAT16 || outputdata_format == VX_TYPE_INT8));
            maxPool_flag            = (vx_bool)(kernel_MaxPool_flag && dataformat_MaxPool_flag && (poolTypeValue == VX_CONVOLUTIONAL_NETWORK_POOLING_MAX));

            if (avgPool_flag || maxPool_flag)
            {
                vxnne_shader_executable shaderExecutable = NULL;
                vx_scalar stride_s = NULL;
                stride_s = vxCreateScalar(node->base.context, VX_TYPE_UINT32, &stride);
                if (!stride_s)
                {
                    status = VX_FAILURE;
                    goto exit;
                }
                if(maxPool_flag)
                    shaderExecutable = vxnneGetMaxPoolingShaderExecutable(node->base.context, VXNNE_KERNEL_MAXPOOLING, &node->kernelAttributes.borderMode,
                                                                   inputs, poolType, stride_s, poolSizeX, poolSizeY, poolPadX, poolPadY, rounding, outputs);
                else if(avgPool_flag)
                    shaderExecutable = vxnneGetAvgPoolingShaderExecutable(node->base.context, VXNNE_KERNEL_AVGPOOLING, &node->kernelAttributes.borderMode,
                    inputs, poolType, stride_s, poolSizeX, poolSizeY, poolPadX, poolPadY, rounding, outputs);

                if (!shaderExecutable)
                {
                    status = VX_FAILURE;
                    vxReleaseScalar(&stride_s);
                    goto exit;
                }

                status = vxnneShaderOperation_Initialize(&poolingLayer->pooling_sh_operation,
                                                &poolingLayer->base,
                                                VXNNE_OPERATOR_POOLING,
                                                shaderExecutable);

                if (status != VX_SUCCESS)
                {
                    vxReleaseScalar(&stride_s);
                    goto exit;
                }

                poolingLayer->base.num_operations = 1;
                poolingLayer->operations[0] = &poolingLayer->pooling_sh_operation.base;
                if (stride_s) (vxReleaseScalar(&stride_s));
            }
            else
            {
                poolingLayer->pooling_sw_operation.base.layer         = &poolingLayer->base;
                poolingLayer->pooling_sw_operation.base.target        = VXNNE_OPERATION_TARGET_SW;
                poolingLayer->pooling_sw_operation.base.operatorType = VXNNE_OPERATOR_POOLING;
                poolingLayer->pooling_sw_operation.base.execute       = vxnneExecuteSWPooling;
                poolingLayer->pooling_sw_operation.base.deinitialize  = VX_NULL;
                poolingLayer->pooling_sw_operation.base.dump          = VX_NULL;

                poolingLayer->operations[0] = (vxnne_operation)&poolingLayer->pooling_sw_operation;

                poolingLayer->pooling_sw_operation.inputs            = inputs;
                poolingLayer->pooling_sw_operation.pool_type         = poolType->value->e;
                poolingLayer->pooling_sw_operation.pool_size_x       = poolSizeX->value->u32;
                poolingLayer->pooling_sw_operation.pool_size_y       = poolSizeY->value->u32;
                poolingLayer->pooling_sw_operation.pool_pad_x_left   = poolPadXLeftValue;
                poolingLayer->pooling_sw_operation.pool_pad_x_right  = poolPadXRightValue;
                poolingLayer->pooling_sw_operation.pool_pad_y_top    = poolPadYTopValue;
                poolingLayer->pooling_sw_operation.pool_pad_y_bottom = poolPadYBottomValue;
                poolingLayer->pooling_sw_operation.rounding          = rounding->value->e;
                poolingLayer->pooling_sw_operation.outputs           = outputs;
            }
        }
    }

    node->layer = &poolingLayer->base;
    return status;

exit:
    if (poolingLayer) gcoOS_Free(gcvNULL, (gctPOINTER)poolingLayer);
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNFullyConnectedLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
#define IMG_MAX_WIDTH 65536

    vx_status  status                     = VX_SUCCESS;

    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_tensor  weights                    = (vx_tensor)parameters[1];
    vx_tensor  biases                     = (vx_tensor)parameters[2];
    vx_tensor  outputs                    = (vx_tensor)parameters[8];

    vx_bool   input_flag = vx_false_e;
    vx_bool   weight_flag = vx_false_e;
    vx_bool   bias_flag = vx_false_e;
    vx_bool   output_flag = vx_false_e;
    vx_enum   input_dataformat = inputs->tensorBuffer->dataFormat;
    vx_enum   weight_dataformat = weights->tensorBuffer->dataFormat;
    vx_enum   bias_dataformat = biases->tensorBuffer->dataFormat;
    vx_enum   output_dataformat = outputs->tensorBuffer->dataFormat;
    vx_uint32 num_of_dim = 0;
    vx_int32  inputs_of_dim = inputs->dimCount;
    vx_uint32 input_size[4] = {0, 0, 0, 0};
    vxnne_fully_connected_relu_layer  fullyConnectedLayer = gcvNULL;
    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    status = vxQueryTensor(inputs, VX_TENSOR_DIMS, input_size, sizeof(input_size));
    if (status != VX_SUCCESS) goto exit;
    input_flag = (vx_bool)((input_dataformat == VX_TYPE_FLOAT16 || (input_dataformat == VX_TYPE_INT8)) && ((input_size[0] * input_size[1] * input_size[2]) < IMG_MAX_WIDTH));

    status = vxQueryTensor(weights, VX_TENSOR_NUM_OF_DIMS, &num_of_dim, sizeof(num_of_dim));
    if (status != VX_SUCCESS) goto exit;
    status = vxQueryTensor(weights, VX_TENSOR_DIMS, input_size, sizeof(input_size));
    if (status != VX_SUCCESS) goto exit;
    weight_flag = (vx_bool)((num_of_dim == 4) && (weight_dataformat == VX_TYPE_FLOAT16 || (weight_dataformat == VX_TYPE_INT8)) && ((input_size[0] * input_size[1] * input_size[2]) < IMG_MAX_WIDTH) && (input_size[3] < IMG_MAX_WIDTH));

    bias_flag = (vx_bool)(bias_dataformat == VX_TYPE_FLOAT32 || (bias_dataformat == VX_TYPE_INT32));

    output_flag = (vx_bool)(output_dataformat == VX_TYPE_FLOAT16 || (output_dataformat == VX_TYPE_INT8));

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_fully_connected_relu_layer_s), (gctPOINTER*)&fullyConnectedLayer);
    if (!fullyConnectedLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(fullyConnectedLayer, sizeof(vxnne_fully_connected_relu_layer_s));

    fullyConnectedLayer->base.name                  = "FullyConnectedLayer";
    fullyConnectedLayer->base.node                  = node;
    fullyConnectedLayer->base.operations            = fullyConnectedLayer->operations;

    fullyConnectedLayer->base.num_temp_tensors      = 0;

    fullyConnectedLayer->base.dump                  = VX_NULL;
    fullyConnectedLayer->base.deinitialize          = vxnneLayer_Deinitialize;

    if (input_flag && weight_flag && bias_flag && output_flag && inputs_of_dim < 4)
    {
        vxnne_shader_executable shaderExecutable;

        shaderExecutable = vxnneGetFullyConnectedShaderExecutable(node->base.context, VXNNE_KERNEL_FULLYCONNECTED, &node->kernelAttributes.borderMode, inputs, weights, biases, outputs);

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&fullyConnectedLayer->fully_connected_SHoperation,
            &fullyConnectedLayer->base,
            VXNNE_OPERATOR_FULLYCONNECTED,
            shaderExecutable);

        if (status != VX_SUCCESS) goto exit;

        fullyConnectedLayer->base.num_operations = 1;
        fullyConnectedLayer->operations[0] = &fullyConnectedLayer->fully_connected_SHoperation.base;
    }
    else
    {
        fullyConnectedLayer->fully_connected_operation.base.layer         = &fullyConnectedLayer->base;
        fullyConnectedLayer->fully_connected_operation.base.target        = VXNNE_OPERATION_TARGET_SW;
        fullyConnectedLayer->fully_connected_operation.base.operatorType = VXNNE_OPERATOR_FULLYCONNECTED;
        fullyConnectedLayer->fully_connected_operation.base.execute       = vxnneExecuteSWFullyConnected;
        fullyConnectedLayer->fully_connected_operation.base.deinitialize  = VX_NULL;
        fullyConnectedLayer->fully_connected_operation.base.dump          = VX_NULL;

        fullyConnectedLayer->base.num_operations    = 1;
        fullyConnectedLayer->operations[0] = (vxnne_operation)&fullyConnectedLayer->fully_connected_operation;

        fullyConnectedLayer->fully_connected_operation.inputs           = inputs;
        fullyConnectedLayer->fully_connected_operation.weights          = weights;
        fullyConnectedLayer->fully_connected_operation.biases           = biases;
        fullyConnectedLayer->fully_connected_operation.outputs          = outputs;
    }

    node->layer = &fullyConnectedLayer->base;
    return status;

exit:
    if (fullyConnectedLayer) gcoOS_Free(gcvNULL, (gctPOINTER)fullyConnectedLayer);
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNActivationLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNActivationLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNActivationLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNActivationLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status  status                     = VX_SUCCESS;

    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_scalar  func_s                     = (vx_scalar)parameters[1];
    vx_scalar  a_s                        = (vx_scalar)parameters[2];
    vx_scalar  b_s                        = (vx_scalar)parameters[3];
    vx_tensor  outputs                    = (vx_tensor)parameters[4];
    vx_enum   inputFormat                = inputs->tensorBuffer->dataFormat;
    vx_enum   outputFormat               = outputs->tensorBuffer->dataFormat;
    vx_enum   func_v = func_s->value->e;
    vx_bool   support_dataType = vx_false_e;
    vx_uint32 input_width   = TENSOR_SIZE_INDEX(inputs, 0);
    vx_uint32 input_height  = TENSOR_SIZE_INDEX(inputs, 1);
    vx_uint32 img2DSize = input_width * input_height;
    vxnne_activation_layer  activationLayer = gcvNULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_activation_layer_s), (gctPOINTER*)&activationLayer);
    if (!activationLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(activationLayer, sizeof(vxnne_activation_layer_s));

    activationLayer->base.name                  = "ActivationLayer";
    activationLayer->base.node                  = node;
    activationLayer->base.operations            = activationLayer->operations;

    activationLayer->base.num_temp_tensors      = 0;

    activationLayer->base.dump                  = VX_NULL;
    activationLayer->base.deinitialize          = vxnneLayer_Deinitialize;
    support_dataType = (vx_bool)((inputFormat == VX_TYPE_INT8 || inputFormat == VX_TYPE_FLOAT16) && (outputFormat == VX_TYPE_INT8 || outputFormat == VX_TYPE_FLOAT16));

    if(img2DSize < IMG_MAX_WIDTH && func_v == VX_CONVOLUTIONAL_NETWORK_ACTIVATION_RELU && support_dataType)
    {
        vxnne_shader_executable shaderExecutable;

        shaderExecutable = vxnneGetActivationReluShaderExecutable(node->base.context, VXNNE_KERNEL_ACTIVATION_RELU, &node->kernelAttributes.borderMode, inputs, outputs);

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&activationLayer->activation_SHoperation,
            &activationLayer->base,
            VXNNE_OPERATOR_ACTIVATION,
            shaderExecutable);

        if (status != VX_SUCCESS) goto exit;

        activationLayer->base.num_operations = 1;
        activationLayer->operations[0] = &activationLayer->activation_SHoperation.base;

    }
    else
    {
        activationLayer->activation_operation.base.layer         = &activationLayer->base;
        activationLayer->activation_operation.base.target        = VXNNE_OPERATION_TARGET_SW;
        activationLayer->activation_operation.base.operatorType = VXNNE_OPERATOR_ACTIVATION;
        activationLayer->activation_operation.base.execute       = vxnneExecuteSWActivation;
        activationLayer->activation_operation.base.deinitialize  = VX_NULL;
        activationLayer->activation_operation.base.dump          = VX_NULL;

        activationLayer->base.num_operations    = 1;
        activationLayer->operations[0] = (vxnne_operation)&activationLayer->activation_operation;

        activationLayer->activation_operation.inputs           = inputs;
        activationLayer->activation_operation.func             = func_s;
        activationLayer->activation_operation.a                = a_s;
        activationLayer->activation_operation.b                = b_s;
        activationLayer->activation_operation.outputs          = outputs;
    }

    node->layer = &activationLayer->base;
    return status;

exit:
    if (activationLayer) gcoOS_Free(gcvNULL, (gctPOINTER)activationLayer);
    return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNActivationLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

/***************************************************************************************************************************
 *                                                       Leaky Relu
 ***************************************************************************************************************************/

vx_status vxnneExecuteSWLeakyRelu(struct _vxnne_operation_s *operation)
{
    vxnne_activation_sw_operation activationOperation   = (vxnne_activation_sw_operation)operation;

    vx_tensor inputs  = (vx_tensor)activationOperation->inputs;
    vx_scalar negative_slopes  = (vx_scalar)activationOperation->a;
    vx_tensor outputs = (vx_tensor)activationOperation->outputs;

    vx_float32  negative_slope_v = negative_slopes->value->f32;

    vx_uint32 elementCount = 0;
    vx_uint32 i;
    vx_float32 result = 0.0f;
    gctPOINTER inputBase;
    gctPOINTER outputBase;
    vx_type_e inputFormat  = (vx_type_e)inputs->tensorBuffer->dataFormat;
    vx_type_e outputFormat = (vx_type_e)outputs->tensorBuffer->dataFormat;

    vx_status status = VX_SUCCESS;

    elementCount = (vx_uint32)vxoMemory_ComputeElementCount(&inputs->tensorBuffer->memory, 0);
    vxoTensor_GetTensorViewMemory(inputs, &inputBase, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, &outputBase, VX_NULL);

    if ((inputs->tensorBuffer->dataFormat != VX_TYPE_FLOAT16 && inputs->tensorBuffer->dataFormat != VX_TYPE_FLOAT32 && inputs->tensorBuffer->dataFormat != VX_TYPE_INT8)
        || (outputs->tensorBuffer->dataFormat != VX_TYPE_FLOAT16 && outputs->tensorBuffer->dataFormat != VX_TYPE_FLOAT32 && outputs->tensorBuffer->dataFormat != VX_TYPE_INT8))
    {
        gcmPRINT("input or outputs format is not support");
        status = VX_ERROR_NOT_SUPPORTED;
        return status;
    }

    for (i = 0; i < elementCount; i++)
    {
        vx_float32 data = vxnneGetData(inputFormat, i, (vx_uint8_ptr)inputBase, inputs->tensorBuffer->fixedPointPos);

        result = (data > 0.0f) ? data : negative_slope_v * data;

        vxnneSaveData(outputFormat, i, result, (vx_uint8_ptr)outputBase, outputs->tensorBuffer->fixedPointPos, outputs->tensorBuffer->roundingMode);
    }
    return status;

}

vx_status vxnneExecuteTPLeakyRelu(struct _vxnne_operation_s *operation)
{
    vxnne_activation_layer layer = (vxnne_activation_layer)operation->layer;
    vxnne_tp_operation activationOperation = (vxnne_tp_operation)operation;

    vx_node node = layer->base.node;
    vx_tensor* inputs = activationOperation->inputs;
    vx_array lut = activationOperation->buffer;
    vx_tensor* outputs = activationOperation->outputs;

    return vxnneExecuteTPGeneric(node, inputs, VX_NULL, lut, outputs, layer->base.cmdTPBuff, activationOperation->op_num, activationOperation->multi_tensor);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNLeakyReluLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNLeakyReluLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNLeakyReluLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNLeakyReluLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status                     = VX_SUCCESS;
    vx_context context                   = vxGetContext((vx_reference)node);

    vx_tensor inputs                     = (vx_tensor)parameters[0];
    vx_scalar negative_slopes            = (vx_scalar)parameters[1];
    vx_tensor outputs                    = (vx_tensor)parameters[2];
    vx_enum   inputFormat                = inputs->tensorBuffer->dataFormat;
    vx_enum   outputFormat               = outputs->tensorBuffer->dataFormat;

    vxnne_activation_layer  activationLayer = VX_NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_activation_layer_s), (gctPOINTER*)&activationLayer);
    if (!activationLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(activationLayer, sizeof(vxnne_activation_layer_s));

    activationLayer->base.name                  = "LeakyReluLayer";
    activationLayer->base.node                  = node;
    activationLayer->base.operations            = activationLayer->operations;

    activationLayer->base.num_temp_tensors      = 0;

    activationLayer->base.dump                  = VX_NULL;
    activationLayer->base.deinitialize          = vxnneLayer_Deinitialize;

    if (context->nnConfig.tpCoreCount && context->options.enableTP && context->options.flagTPFunc[TP_LEAKY_RELU])
    {
        vx_uint32 i, slice;
        vx_tp_conv_cmd conv;
        vx_tp_value_cmd values;
        vx_array lut = activationLayer->activation_tp_operation.buffer;

        vxnneOperation_Initialize(
                &activationLayer->activation_tp_operation.base,
                &activationLayer->base,
                VXNNE_OPERATION_TARGET_TP,
                VXNNE_OPERATOR_ACTIVATION,
                vxnneExecuteTPLeakyRelu,
                vxnneOperation_TP_Deinitialize);

        slice = context->options.enableMultiTP && context->nnConfig.tpCoreCount > 1 ? TENSOR_SIZE_INDEX(inputs, 2) : 1;

        /* create cmd buffer for TP operation */
        activationLayer->base.cmdTPBuff = vxCreateArray(context, VX_TYPE_CHAR, TP_COMMAND_SIZE * slice);
        if (!vxoArray_AllocateMemory(activationLayer->base.cmdTPBuff))
        {
            status = VX_ERROR_NO_MEMORY;
            gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            goto exit;
        }

        if (lut == VX_NULL)
        {
            /* Prepare PWL LUT. */
            lut = vxCreateArray(context, VX_TYPE_UINT16, TP_LUT_BUFF_SIZE);
            if (!vxoArray_AllocateMemory(lut))
            {
                status = VX_ERROR_NO_MEMORY;
                gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
                goto exit;
            }
            lut->itemCount = TP_LUT_BUFF_SIZE;
            lut->base.isStage = vx_true_e;
        }

        memset(&conv, 0, sizeof(conv));
        values.f32[0] = negative_slopes->value->f32;
        for (i = 0; i < slice; i++)
        {
            fillInCmdTPBuffer(
                    inputs, VX_NULL, outputs,
                    activationLayer->base.cmdTPBuff, lut,
                    &conv, &values,
                    TP_LEAKY_RELU,
                    i, slice > 1,
                    vx_true_e);
        }

        activationLayer->base.num_operations = 1;
        activationLayer->operations[0] = (vxnne_operation)&activationLayer->activation_tp_operation;
        activationLayer->activation_tp_operation.inputs[0]    = inputs;
        activationLayer->activation_tp_operation.buffer       = lut;
        activationLayer->activation_tp_operation.outputs[0]   = outputs;
        activationLayer->activation_tp_operation.op_num       = slice;
        activationLayer->activation_tp_operation.multi_tensor = vx_false_e;
    }
    else if((inputFormat == VX_TYPE_FLOAT16 || inputFormat == VX_TYPE_INT8) && (outputFormat == VX_TYPE_FLOAT16 || outputFormat == VX_TYPE_INT8))
    {
        vxnne_shader_executable shaderExecutable;

        shaderExecutable = vxnneGetLeakyReluShaderExecutable(node->base.context, VXNNE_KERNEL_NN_LEAKY, &node->kernelAttributes.borderMode, inputs, negative_slopes, outputs);

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&activationLayer->activation_SHoperation,
                                        &activationLayer->base,
                                        VXNNE_OPERATOR_ACTIVATION,
                                        shaderExecutable);

        if (status != VX_SUCCESS) goto exit;

        activationLayer->base.num_operations = 1;
        activationLayer->operations[0] = &activationLayer->activation_SHoperation.base;

    }
    else
    {
        activationLayer->activation_operation.base.layer         = &activationLayer->base;
        activationLayer->activation_operation.base.target        = VXNNE_OPERATION_TARGET_SW;
        activationLayer->activation_operation.base.operatorType  = VXNNE_OPERATOR_ACTIVATION;
        activationLayer->activation_operation.base.execute       = vxnneExecuteSWLeakyRelu;
        activationLayer->activation_operation.base.deinitialize  = VX_NULL;
        activationLayer->activation_operation.base.dump          = VX_NULL;

        activationLayer->base.num_operations = 1;
        activationLayer->operations[0] = (vxnne_operation)&activationLayer->activation_operation;

        activationLayer->activation_operation.inputs           = inputs;
        activationLayer->activation_operation.func             = VX_NULL;
        activationLayer->activation_operation.a                = negative_slopes;
        activationLayer->activation_operation.b                = VX_NULL;
        activationLayer->activation_operation.outputs          = outputs;
    }

    node->layer = &activationLayer->base;
    return status;

exit:
    if (activationLayer) gcoOS_Free(gcvNULL, (gctPOINTER)activationLayer);
    return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNLeakyReluLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

/***************************************************************************************************************************
 *                                                       Batch Normalization
 ***************************************************************************************************************************/

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNBatchNormalizationLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNBatchNormalizationLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNBatchNormalizationLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

vx_status vxnneExecuteSWBatchNormalization(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;

    vx_tensor input,output;
    vx_tensor means,variances,gamma,beta;
    vx_scalar epss;
    vx_uint8_ptr inputLogic,outputLogic;
    vx_uint8_ptr meanLogic,varianceLogic,gammaLogic,betaLogic;

    vx_uint32 width,height,channel,batch;
    vx_uint32 b,c,i,spatial;
    vx_float32 meanf,variancef,gammaf,betaf;
    vx_float32 inputf,outputf;
    vx_type_e inFormat,outFormat,meanFormat,varianceFormat,gammaFormat,betaFormat;
    vx_float32 eps;
    vx_float32 normalize;
    vx_uint32 index;

    vxnne_batchnorm_sw_operation activationOperation = (vxnne_batchnorm_sw_operation)operation;
    means       = (vx_tensor)activationOperation->mean;
    variances   = (vx_tensor)activationOperation->variance;
    gamma       = (vx_tensor)activationOperation->gamma;
    beta        = (vx_tensor)activationOperation->beta;

    input       = (vx_tensor)activationOperation->input;
    output      = (vx_tensor)activationOperation->output;

    epss        = (vx_scalar)activationOperation->eps;
    eps         = epss->value->f32;

    width       = TENSOR_VIEW_SIZE_INDEX(input, 0);
    height      = TENSOR_VIEW_SIZE_INDEX(input, 1);
    channel     = TENSOR_VIEW_SIZE_INDEX(input, 2);
    batch       = TENSOR_VIEW_SIZE_INDEX(input, 3);
    spatial     = width*height;

    inFormat        = (vx_type_e)(input->tensorBuffer->dataFormat);
    outFormat       = (vx_type_e)(output->tensorBuffer->dataFormat);
    meanFormat      = (vx_type_e)(means->tensorBuffer->dataFormat);
    varianceFormat  = (vx_type_e)(variances->tensorBuffer->dataFormat);
    gammaFormat     = (vx_type_e)(gamma->tensorBuffer->dataFormat);
    betaFormat      = (vx_type_e)(beta->tensorBuffer->dataFormat);

    vxoTensor_GetTensorViewMemory(input, (gctPOINTER*)&inputLogic, VX_NULL);
    vxoTensor_GetTensorViewMemory(output, (gctPOINTER*)&outputLogic, VX_NULL);
    vxoTensor_GetTensorViewMemory(means, (gctPOINTER*)&meanLogic, VX_NULL);
    vxoTensor_GetTensorViewMemory(variances, (gctPOINTER*)&varianceLogic, VX_NULL);
    vxoTensor_GetTensorViewMemory(gamma, (gctPOINTER*)&gammaLogic, VX_NULL);
    vxoTensor_GetTensorViewMemory(beta, (gctPOINTER*)&betaLogic, VX_NULL);

    for(b=0; b<batch; b++)
    {
        for(c=0; c<channel; c++)
        {
            meanf       = vxnneGetData(meanFormat, c, meanLogic, means->tensorBuffer->fixedPointPos);
            variancef   = vxnneGetData(varianceFormat, c, varianceLogic, variances->tensorBuffer->fixedPointPos);
            gammaf      = vxnneGetData(gammaFormat, c, gammaLogic, gamma->tensorBuffer->fixedPointPos);
            betaf       = vxnneGetData(betaFormat, c, betaLogic, beta->tensorBuffer->fixedPointPos);

            //printf("meanf[%f], variancef[%f], gammaf[%f], betaf[%f]==================\n", meanf, variancef, gammaf, betaf);
            for(i=0; i<spatial; i++)
            {
                index       = b * channel * spatial + c * spatial + i;

                inputf      = vxnneGetData(inFormat, index, inputLogic, input->tensorBuffer->fixedPointPos);
                //printf("index[%u] inputf[%f] ---> ", index, inputf);

                /* Compute Normalize */
                normalize   = (inputf - meanf)/sqrtf(variancef + eps);

                /* Scale and Shift */
                outputf     = gammaf * normalize + betaf;
                //printf("outputf[%f]\n", outputf);

                vxnneSaveData(outFormat, index, outputf, outputLogic, output->tensorBuffer->fixedPointPos, output->tensorBuffer->roundingMode);
            }
        }
    }




    return status;
}


VX_PRIVATE_API vx_status VX_CALLBACK vxoNNBatchNormalizationLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status  status                     = VX_SUCCESS;

    vx_scalar  epss                       = (vx_scalar)parameters[0];
    vx_tensor  means                      = (vx_tensor)parameters[1];
    vx_tensor  variances                  = (vx_tensor)parameters[2];
    vx_tensor  gamma                      = (vx_tensor)parameters[3];
    vx_tensor  beta                       = (vx_tensor)parameters[4];
    vx_tensor  input                      = (vx_tensor)parameters[5];
    vx_tensor  output                     = (vx_tensor)parameters[6];
    vx_enum    inputFormat                = input->tensorBuffer->dataFormat;
    vx_enum    outputFormat               = output->tensorBuffer->dataFormat;

    vxnne_batchnorm_layer  batchnormLayer = NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_batchnorm_layer_s), (gctPOINTER*)&batchnormLayer);
    if (!batchnormLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(batchnormLayer, sizeof(vxnne_batchnorm_layer_s));

    batchnormLayer->base.name                  = "BatchNormalizationLayer";
    batchnormLayer->base.node                  = node;
    batchnormLayer->base.operations            = batchnormLayer->operations;

    batchnormLayer->base.num_temp_tensors      = 0;

    batchnormLayer->base.dump                  = VX_NULL;
    batchnormLayer->base.deinitialize          = vxnneLayer_Deinitialize;

    if (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
    {
        vxnne_shader_executable shaderExecutable;

        shaderExecutable = vxnneGetBatchNormShaderExecutable(node->base.context, VXNNE_KERNEL_BATCHNORM, &node->kernelAttributes.borderMode,
                                                              input, epss, means, variances, gamma, beta, output);

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&batchnormLayer->batchnorm_sh_operation,
                                        &batchnormLayer->base,
                                        VXNNE_OPERATOR_BATCHNORM,
                                        shaderExecutable);

        if (status != VX_SUCCESS) goto exit;

        batchnormLayer->base.num_operations = 1;
        batchnormLayer->operations[0] = &batchnormLayer->batchnorm_sh_operation.base;

    }
    else
    {
        batchnormLayer->batchnorm_operation.base.layer         = &batchnormLayer->base;
        batchnormLayer->batchnorm_operation.base.target        = VXNNE_OPERATION_TARGET_SW;
        batchnormLayer->batchnorm_operation.base.operatorType  = VXNNE_OPERATOR_BATCHNORM;
        batchnormLayer->batchnorm_operation.base.execute       = vxnneExecuteSWBatchNormalization;
        batchnormLayer->batchnorm_operation.base.deinitialize  = VX_NULL;
        batchnormLayer->batchnorm_operation.base.dump          = VX_NULL;

        batchnormLayer->base.num_operations    = 1;
        batchnormLayer->operations[0] = (vxnne_operation)&batchnormLayer->batchnorm_operation;

        batchnormLayer->batchnorm_operation.eps              = epss;
        batchnormLayer->batchnorm_operation.mean             = means;
        batchnormLayer->batchnorm_operation.variance         = variances;
        batchnormLayer->batchnorm_operation.gamma            = gamma;
        batchnormLayer->batchnorm_operation.beta             = beta;
        batchnormLayer->batchnorm_operation.input            = input;
        batchnormLayer->batchnorm_operation.output           = output;
    }

    node->layer = &batchnormLayer->base;
    return status;

exit:
    if (batchnormLayer) gcoOS_Free(gcvNULL, (gctPOINTER)batchnormLayer);
    return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNBatchNormalizationLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

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
    vx_int32 size = TENSOR_SIZE_INDEX(input1, 0) * TENSOR_SIZE_INDEX(input1, 1) * TENSOR_SIZE_INDEX(input1, 2);
    vx_uint8_ptr input1_ptr = TENSOR_LOGICAL_ADDR(input1);
    vx_uint8_ptr input2_ptr = TENSOR_LOGICAL_ADDR(input2);
    vx_uint8_ptr output_ptr = TENSOR_LOGICAL_ADDR(output);

    vx_uint8 input1FixPointPos = input1->tensorBuffer->fixedPointPos;
    vx_uint8 input2FixPointPos = input2->tensorBuffer->fixedPointPos;
    vx_uint8 outputFixPointPos = output->tensorBuffer->fixedPointPos;

    vx_enum input1Format = input1->tensorBuffer->dataFormat;
    vx_enum input2Format = input2->tensorBuffer->dataFormat;
    vx_enum outputFormat = output->tensorBuffer->dataFormat;

    vx_enum outputRoundingMode = output->tensorBuffer->roundingMode;

    if (dim1 == dim2)
    {
        switch(kernel)
        {
        case VX_KERNEL_NN_TENSOR_ADD:
            eltwise(input1_ptr, input1FixPointPos, input1Format, input2_ptr, input2FixPointPos, input2Format, size, 1.f, overflow, VX_ROUND_POLICY_TO_ZERO, VX_TENSOR_OP_ADD, output_ptr, outputFixPointPos, outputRoundingMode, outputFormat);
            break;
        case VX_KERNEL_NN_TENSOR_SUB:
            eltwise(input1_ptr, input1FixPointPos, input1Format, input2_ptr, input2FixPointPos, input2Format, size, 1.f, overflow, VX_ROUND_POLICY_TO_ZERO, VX_TENSOR_OP_SUB, output_ptr, outputFixPointPos, outputRoundingMode, outputFormat);
            break;
        case VX_KERNEL_NN_TENSOR_MUL:
            {
                vx_enum rounding = eltwiseOperation->rounding->value->e;
                vx_float32 scale = eltwiseOperation->scale->value->f32;
                eltwise(input1_ptr, input1FixPointPos, input1Format, input2_ptr, input2FixPointPos, input2Format, size, scale, overflow, rounding, VX_TENSOR_OP_MUL, output_ptr, outputFixPointPos, outputRoundingMode, outputFormat);
            }
            break;
        default:
            gcmPRINT("Not support kenrel: %d\n", kernel);
            break;
        }
    }
    else
        gcmPRINT("Difference dim\n");
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorEltwise(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorEltwise_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorEltwise_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorEltwise_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
   vx_status  status  = VX_SUCCESS;

    vx_tensor input1   = (vx_tensor)parameters[0];
    vx_tensor input2   = (vx_tensor)parameters[1];
    vx_scalar scale = NULL;
    vx_scalar overflow = (vx_scalar)parameters[2];
    vx_scalar rounding = NULL;
    vx_tensor output   = (vx_tensor)parameters[3];
    vx_enum kernel     = node->kernel->enumeration;
    vxnne_eltwise_layer eltwiseLayer = {0};
    vxnne_eltwise_sw_operation_s * operation = VX_NULL;

    if (kernel == VX_KERNEL_NN_TENSOR_MUL)
    {
        scale = (vx_scalar)parameters[2];
        overflow = (vx_scalar)parameters[3];
        rounding = (vx_scalar)parameters[4];
        output   = (vx_tensor)parameters[5];
    }

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_eltwise_layer_s), (gctPOINTER*)&eltwiseLayer);
    if (!eltwiseLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(eltwiseLayer, sizeof(vxnne_eltwise_layer_s));

    eltwiseLayer->base.name                  = "eltwiseLayer";
    eltwiseLayer->base.node                  = node;
    eltwiseLayer->base.operations            = eltwiseLayer->operations;

    eltwiseLayer->base.num_temp_tensors      = 0;

    eltwiseLayer->base.dump                  = VX_NULL;
    eltwiseLayer->base.deinitialize          = vxnneLayer_Deinitialize;

    operation = &eltwiseLayer->eltwise_operation;

    operation->base.layer         = &eltwiseLayer->base;
    operation->base.target        = VXNNE_OPERATION_TARGET_SW;
    operation->base.operatorType = VXNNE_OPERATOR_ACTIVATION;
    operation->base.execute       = vxnneExecuteSWEltwise;
    operation->base.deinitialize  = VX_NULL;
    operation->base.dump          = VX_NULL;

    eltwiseLayer->base.num_operations    = 1;
    eltwiseLayer->operations[0] = (vxnne_operation)&eltwiseLayer->eltwise_operation;

    operation->kernel           = node->kernel->enumeration;
    operation->input1           = input1;
    operation->input2           = input2;
    operation->scale            = scale;
    operation->overflow         = overflow;
    operation->rounding         = rounding;
    operation->output           = output;

    node->layer = &eltwiseLayer->base;

exit:
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorEltwise_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }
    return VX_SUCCESS;
}


vx_status vxnneExecuteSWTensorAdd(vxnne_operation operation)
{
    vxnne_tensor_add_operation eltwiseOperation   = (vxnne_tensor_add_operation)operation;

    vx_tensor input1 = eltwiseOperation->input0;
    vx_tensor input2 = eltwiseOperation->input1;
    vx_tensor output = eltwiseOperation->output;

    vx_int32 dim1 = input1->viewRegion.dimCount;
    vx_int32 dim2 = input2->viewRegion.dimCount;
    vx_enum overflow = eltwiseOperation->policy->value->e;
    vx_int32 size = TENSOR_SIZE_INDEX(input1, 0) * TENSOR_SIZE_INDEX(input1, 1) * TENSOR_SIZE_INDEX(input1, 2);
    vx_uint8_ptr input1_ptr = TENSOR_LOGICAL_ADDR(input1);
    vx_uint8_ptr input2_ptr = TENSOR_LOGICAL_ADDR(input2);
    vx_uint8_ptr output_ptr = TENSOR_LOGICAL_ADDR(output);
    vx_enum input1Format = input1->tensorBuffer->dataFormat;
    vx_enum input2Format = input2->tensorBuffer->dataFormat;
    vx_enum outputFormat = output->tensorBuffer->dataFormat;
    vx_uint8 input1FpPos = input1->tensorBuffer->fixedPointPos;
    vx_uint8 input2FpPos = input2->tensorBuffer->fixedPointPos;
    vx_uint8 outputFpPos = output->tensorBuffer->fixedPointPos;
    vx_enum outputRoundingMode = output->tensorBuffer->roundingMode;

    if (dim1 == dim2)
    {
        eltwise(input1_ptr, input1FpPos, input1Format, input2_ptr, input2FpPos, input2Format, size, 1.f, overflow, VX_ROUND_POLICY_TO_ZERO, VX_TENSOR_OP_ADD, output_ptr, outputFpPos, outputRoundingMode, outputFormat);
    }
    else
        gcmPRINT("Difference dim\n");

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorAdd(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorAdd_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorAdd_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorAdd_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    vx_tensor input0   = (vx_tensor)parameters[0];
    vx_tensor input1   = (vx_tensor)parameters[1];
    vx_scalar policy   = (vx_scalar)parameters[2];
    vx_tensor output   = (vx_tensor)parameters[3];

    vx_type_e input0Format = input0->tensorBuffer->dataFormat;
    vx_type_e input1Format = input1->tensorBuffer->dataFormat;
    vx_type_e outputFormat = output->tensorBuffer->dataFormat;

    vx_bool format_flag = vx_false_e;

    vxnne_tensor_add_layer tensor_add_layer = VX_NULL;

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
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(tensor_add_layer, sizeof(vxnne_tensor_add_layer_s));

    vxnneLayer_Initialize(&tensor_add_layer->base, "TensorAddLayer", node, tensor_add_layer->operations, VX_NULL);

    format_flag = (vx_bool)((input0Format == VX_TYPE_FLOAT16) && (input1Format == VX_TYPE_FLOAT16) && (outputFormat == VX_TYPE_FLOAT16));
    format_flag |= (vx_bool)((input0Format == VX_TYPE_INT8) && (input1Format == VX_TYPE_INT8) && (outputFormat == VX_TYPE_FLOAT16));
    format_flag |= (vx_bool)((input0Format == VX_TYPE_INT8) && (input1Format == VX_TYPE_INT8) && (outputFormat == VX_TYPE_INT8));
    format_flag |= (vx_bool)((input0Format == VX_TYPE_FLOAT16) && (input1Format == VX_TYPE_INT8) && (outputFormat == VX_TYPE_INT8));
    format_flag |= (vx_bool)((input0Format == VX_TYPE_INT8) && (input1Format == VX_TYPE_FLOAT16) && (outputFormat == VX_TYPE_INT8));
    if (format_flag)
    {
        vxnne_shader_executable shaderExecutable;
        if(input0Format == VX_TYPE_INT8 && input1Format == VX_TYPE_FLOAT16)
            shaderExecutable = vxnneGetTensorAddShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_ADD, &node->kernelAttributes.borderMode, input1, input0, policy, output);
        else
            shaderExecutable = vxnneGetTensorAddShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_ADD, &node->kernelAttributes.borderMode, input0, input1, policy, output);

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&tensor_add_layer->tensorAddSH,
                                        &tensor_add_layer->base,
                                        VXNNE_OPERATOR_TENSOR_ADD,
                                        shaderExecutable);

        if (status != VX_SUCCESS)
            goto exit;

        tensor_add_layer->base.num_operations = 1;
        tensor_add_layer->operations[0] = &tensor_add_layer->tensorAddSH.base;
    }
    else
    {
        vxnneOperation_Initialize(&tensor_add_layer->tensorAddSW.base,
                                &tensor_add_layer->base,
                                VXNNE_OPERATION_TARGET_SW,
                                VXNNE_OPERATOR_TENSOR_ADD,
                                vxnneExecuteSWTensorAdd,
                                VX_NULL);

        tensor_add_layer->base.num_operations = 1;
        tensor_add_layer->operations[0] = &tensor_add_layer->tensorAddSW.base;

        tensor_add_layer->tensorAddSW.input0    = input0;
        tensor_add_layer->tensorAddSW.input1    = input1;
        tensor_add_layer->tensorAddSW.policy    = policy;
        tensor_add_layer->tensorAddSW.output    = output;
    }

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
    }

    return VX_SUCCESS;
}
//Tensor sub
vx_status vxnneExecuteSWTensorSub(vxnne_operation operation)
{
    vxnne_tensor_sub_operation eltwiseOperation   = (vxnne_tensor_sub_operation)operation;

    vx_tensor input1 = eltwiseOperation->input0;
    vx_tensor input2 = eltwiseOperation->input1;
    vx_tensor output = eltwiseOperation->output;

    vx_int32 dim1 = input1->viewRegion.dimCount;
    vx_int32 dim2 = input2->viewRegion.dimCount;
    vx_enum overflow = eltwiseOperation->policy->value->e;
    vx_int32 size = TENSOR_SIZE_INDEX(input1, 0) * TENSOR_SIZE_INDEX(input1, 1) * TENSOR_SIZE_INDEX(input1, 2);
    vx_uint8_ptr input1_ptr = TENSOR_LOGICAL_ADDR(input1);
    vx_uint8_ptr input2_ptr = TENSOR_LOGICAL_ADDR(input2);
    vx_uint8_ptr output_ptr = TENSOR_LOGICAL_ADDR(output);
    vx_enum input1Format = input1->tensorBuffer->dataFormat;
    vx_enum input2Format = input2->tensorBuffer->dataFormat;
    vx_enum outputFormat = output->tensorBuffer->dataFormat;
    vx_uint8 input1FpPos = input1->tensorBuffer->fixedPointPos;
    vx_uint8 input2FpPos = input2->tensorBuffer->fixedPointPos;
    vx_uint8 outputFpPos = output->tensorBuffer->fixedPointPos;
    vx_enum outputRoundingMode = output->tensorBuffer->roundingMode;

    if (dim1 == dim2)
    {
        eltwise(input1_ptr, input1FpPos, input1Format, input2_ptr, input2FpPos, input2Format, size, 1.f, overflow, VX_ROUND_POLICY_TO_ZERO, VX_TENSOR_OP_SUB, output_ptr, outputFpPos, outputRoundingMode, outputFormat);
    }
    else
        gcmPRINT("Difference dim\n");

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorSub(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorSub_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorSub_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorSub_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    vx_tensor input0   = (vx_tensor)parameters[0];
    vx_tensor input1   = (vx_tensor)parameters[1];
    vx_scalar policy   = (vx_scalar)parameters[2];
    vx_tensor output   = (vx_tensor)parameters[3];

    //vx_type_e input0Format = input0->tensorBuffer->dataFormat;
    //vx_type_e input1Format = input1->tensorBuffer->dataFormat;
    //vx_type_e outputFormat = output->tensorBuffer->dataFormat;

    vx_bool format_flag = vx_false_e;

    vxnne_tensor_sub_layer tensor_sub_layer = VX_NULL;

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
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(tensor_sub_layer, sizeof(vxnne_tensor_sub_layer_s));

    vxnneLayer_Initialize(&tensor_sub_layer->base, "TensorSubLayer", node, tensor_sub_layer->operations, VX_NULL);

    /*
    format_flag = (vx_bool)((input0Format == VX_TYPE_FLOAT16) && (input1Format == VX_TYPE_FLOAT16) && (outputFormat == VX_TYPE_FLOAT16));
    format_flag |= (vx_bool)((input0Format == VX_TYPE_FLOAT16) && (input1Format == VX_TYPE_FLOAT16) && (outputFormat == VX_TYPE_INT8));
    format_flag |= (vx_bool)((input0Format == VX_TYPE_INT8) && (input1Format == VX_TYPE_FLOAT16) && (outputFormat == VX_TYPE_FLOAT16));
    format_flag |= (vx_bool)((input0Format == VX_TYPE_INT8) && (input1Format == VX_TYPE_FLOAT16) && (outputFormat == VX_TYPE_INT8));
    format_flag |= (vx_bool)((input0Format == VX_TYPE_FLOAT16) && (input1Format == VX_TYPE_INT8) && (outputFormat == VX_TYPE_FLOAT16));
    format_flag |= (vx_bool)((input0Format == VX_TYPE_FLOAT16) && (input1Format == VX_TYPE_INT8) && (outputFormat == VX_TYPE_INT8));
    format_flag |= (vx_bool)((input0Format == VX_TYPE_INT8) && (input1Format == VX_TYPE_INT8) && (outputFormat == VX_TYPE_FLOAT16));
    format_flag |= (vx_bool)((input0Format == VX_TYPE_INT8) && (input1Format == VX_TYPE_INT8) && (outputFormat == VX_TYPE_INT8));
    */
    format_flag = (vx_bool)(policy->value->e == VX_CONVERT_POLICY_SATURATE);
    //format_flag = vx_false_e;

    if (format_flag)
    {
        vxnne_shader_executable shaderExecutable;
        shaderExecutable = vxnneGetTensorSubShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_SUB, &node->kernelAttributes.borderMode, input0, input1, policy, output);

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&tensor_sub_layer->tensorSubSH,
                                        &tensor_sub_layer->base,
                                        VXNNE_OPERATOR_TENSOR_SUB,
                                        shaderExecutable);

        if (status != VX_SUCCESS)
            goto exit;

        tensor_sub_layer->base.num_operations = 1;
        tensor_sub_layer->operations[0] = &tensor_sub_layer->tensorSubSH.base;
    }
    else
    {
        vxnneOperation_Initialize(&tensor_sub_layer->tensorSubSW.base,
                                &tensor_sub_layer->base,
                                VXNNE_OPERATION_TARGET_SW,
                                VXNNE_OPERATOR_TENSOR_SUB,
                                vxnneExecuteSWTensorSub,
                                VX_NULL);

        tensor_sub_layer->base.num_operations = 1;
        tensor_sub_layer->operations[0] = &tensor_sub_layer->tensorSubSW.base;

        tensor_sub_layer->tensorSubSW.input0    = input0;
        tensor_sub_layer->tensorSubSW.input1    = input1;
        tensor_sub_layer->tensorSubSW.policy    = policy;
        tensor_sub_layer->tensorSubSW.output    = output;
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
    }

    return VX_SUCCESS;
}
//end tensor sub
//Tensor mul
vx_status vxnneExecuteSWTensorMul(vxnne_operation operation)
{
    vxnne_tensor_mul_operation eltwiseOperation   = (vxnne_tensor_mul_operation)operation;

    vx_tensor input1 = eltwiseOperation->input0;
    vx_tensor input2 = eltwiseOperation->input1;
    vx_tensor output = eltwiseOperation->output;

    vx_int32 dim1 = input1->viewRegion.dimCount;
    vx_int32 dim2 = input2->viewRegion.dimCount;
    vx_enum overflow = eltwiseOperation->overflow->value->e;
    vx_int32 size = TENSOR_SIZE_INDEX(input1, 0) * TENSOR_SIZE_INDEX(input1, 1) * TENSOR_SIZE_INDEX(input1, 2);
    vx_uint8_ptr input1_ptr = TENSOR_LOGICAL_ADDR(input1);
    vx_uint8_ptr input2_ptr = TENSOR_LOGICAL_ADDR(input2);
    vx_uint8_ptr output_ptr = TENSOR_LOGICAL_ADDR(output);
    vx_enum input1Format = input1->tensorBuffer->dataFormat;
    vx_enum input2Format = input2->tensorBuffer->dataFormat;
    vx_enum outputFormat = output->tensorBuffer->dataFormat;
    vx_uint8 input1FpPos = input1->tensorBuffer->fixedPointPos;
    vx_uint8 input2FpPos = input2->tensorBuffer->fixedPointPos;
    vx_uint8 outputFpPos = output->tensorBuffer->fixedPointPos;
    vx_enum outputRoundingMode = output->tensorBuffer->roundingMode;

    if (dim1 == dim2)
    {
        vx_enum rounding = eltwiseOperation->rounding->value->e;
        vx_float32 scale = eltwiseOperation->scale->value->f32;
        eltwise(input1_ptr, input1FpPos, input1Format, input2_ptr, input2FpPos, input2Format, size, scale, overflow, rounding, VX_TENSOR_OP_MUL, output_ptr, outputFpPos, outputRoundingMode, outputFormat);
    }
    else
        gcmPRINT("Difference dim\n");

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorMul(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorMul_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorMul_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorMul_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    vx_tensor input0   = (vx_tensor)parameters[0];
    vx_tensor input1   = (vx_tensor)parameters[1];
    vx_scalar scale    = (vx_scalar)parameters[2];
    vx_scalar overflow = (vx_scalar)parameters[3];
    vx_scalar rounding = (vx_scalar)parameters[4];
    vx_tensor output   = (vx_tensor)parameters[5];

    vx_type_e input0Format = input0->tensorBuffer->dataFormat;
    vx_type_e input1Format = input1->tensorBuffer->dataFormat;
    //vx_type_e outputFormat = output->tensorBuffer->dataFormat;

    vx_bool format_flag = vx_false_e;

    vxnne_tensor_mul_layer tensor_mul_layer = VX_NULL;

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
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(tensor_mul_layer, sizeof(vxnne_tensor_mul_layer_s));

    vxnneLayer_Initialize(&tensor_mul_layer->base, "TensorMulLayer", node, tensor_mul_layer->operations, VX_NULL);

    format_flag = (vx_bool)(rounding->value->e == VX_ROUND_POLICY_TO_NEAREST_EVEN);
    format_flag &= (vx_bool)(overflow->value->e == VX_CONVERT_POLICY_SATURATE);
    //format_flag = vx_false_e;

    if (format_flag)
    {
        vxnne_shader_executable shaderExecutable;
        if(input0Format == VX_TYPE_FLOAT16 && input1Format == VX_TYPE_INT8)//exchange input0, input1
            shaderExecutable = vxnneGetTensorMulShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_MUL, &node->kernelAttributes.borderMode, input1, input0, scale, overflow, rounding, output);
        else
            shaderExecutable = vxnneGetTensorMulShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_MUL, &node->kernelAttributes.borderMode, input0, input1, scale, overflow, rounding, output);

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&tensor_mul_layer->tensorMulSH,
                                        &tensor_mul_layer->base,
                                        VXNNE_OPERATOR_TENSOR_MUL,
                                        shaderExecutable);

        if (status != VX_SUCCESS)
            goto exit;

        tensor_mul_layer->base.num_operations = 1;
        tensor_mul_layer->operations[0] = &tensor_mul_layer->tensorMulSH.base;
    }
    else
    {
        vxnneOperation_Initialize(&tensor_mul_layer->tensorMulSW.base,
                                &tensor_mul_layer->base,
                                VXNNE_OPERATION_TARGET_SW,
                                VXNNE_OPERATOR_TENSOR_MUL,
                                vxnneExecuteSWTensorMul,
                                VX_NULL);

        tensor_mul_layer->base.num_operations = 1;
        tensor_mul_layer->operations[0] = &tensor_mul_layer->tensorMulSW.base;

        tensor_mul_layer->tensorMulSW.input0    = input0;
        tensor_mul_layer->tensorMulSW.input1    = input1;
        tensor_mul_layer->tensorMulSW.scale = scale;
        tensor_mul_layer->tensorMulSW.overflow  = overflow;
        tensor_mul_layer->tensorMulSW.rounding = rounding;
        tensor_mul_layer->tensorMulSW.output    = output;
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
    }

    return VX_SUCCESS;
}
//end tensor mul
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
    vx_int32 size = TENSOR_SIZE_INDEX(input1, 0) * TENSOR_SIZE_INDEX(input1, 1) * TENSOR_SIZE_INDEX(input1, 2);
    vx_uint8_ptr input1_ptr = TENSOR_LOGICAL_ADDR(input1);
    vx_uint8_ptr input2_ptr = TENSOR_LOGICAL_ADDR(input2);
    vx_uint8_ptr output_ptr = TENSOR_LOGICAL_ADDR(output);
    vx_enum input1Format = input1->tensorBuffer->dataFormat;
    vx_enum input2Format = input2->tensorBuffer->dataFormat;
    vx_enum outputFormat = output->tensorBuffer->dataFormat;
    vx_uint8 input1FpPos = input1->tensorBuffer->fixedPointPos;
    vx_uint8 input2FpPos = input2->tensorBuffer->fixedPointPos;
    vx_uint8 outputFpPos = output->tensorBuffer->fixedPointPos;
    vx_enum outputRoundingMode = output->tensorBuffer->roundingMode;

    if (dim1 == dim2)
    {
        vx_enum rounding = eltwiseOperation->rounding->value->e;
        vx_float32 scale = eltwiseOperation->scale->value->f32;
        eltwise(input1_ptr, input1FpPos, input1Format, input2_ptr, input2FpPos, input2Format, size, scale, overflow, rounding, VX_TENSOR_OP_DIV, output_ptr, outputFpPos, outputRoundingMode, outputFormat);
    }
    else
        gcmPRINT("Difference dim\n");

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorDiv(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
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

    //vx_type_e input0Format = input0->tensorBuffer->dataFormat;
    //vx_type_e input1Format = input1->tensorBuffer->dataFormat;
    //vx_type_e outputFormat = output->tensorBuffer->dataFormat;

    vx_bool format_flag = vx_false_e;

    vxnne_tensor_div_layer tensor_div_layer = VX_NULL;

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
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(tensor_div_layer, sizeof(vxnne_tensor_div_layer_s));

    vxnneLayer_Initialize(&tensor_div_layer->base, "TensorDivLayer", node, tensor_div_layer->operations, VX_NULL);

    format_flag = (vx_bool)(rounding->value->e == VX_ROUND_POLICY_TO_NEAREST_EVEN);
    format_flag &= (vx_bool)(overflow->value->e == VX_CONVERT_POLICY_SATURATE);
    //format_flag = vx_false_e;

    if (format_flag)
    {
        vxnne_shader_executable shaderExecutable;
        shaderExecutable = vxnneGetTensorDivShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_DIV, &node->kernelAttributes.borderMode, input0, input1, scale, overflow, rounding, output);

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&tensor_div_layer->tensorDivSH,
                                        &tensor_div_layer->base,
                                        VXNNE_OPERATOR_TENSOR_DIV,
                                        shaderExecutable);

        if (status != VX_SUCCESS)
            goto exit;

        tensor_div_layer->base.num_operations = 1;
        tensor_div_layer->operations[0] = &tensor_div_layer->tensorDivSH.base;
    }
    else
    {
        vxnneOperation_Initialize(&tensor_div_layer->tensorDivSW.base,
                                &tensor_div_layer->base,
                                VXNNE_OPERATION_TARGET_SW,
                                VXNNE_OPERATOR_TENSOR_DIV,
                                vxnneExecuteSWTensorDiv,
                                VX_NULL);

        tensor_div_layer->base.num_operations = 1;
        tensor_div_layer->operations[0] = &tensor_div_layer->tensorDivSW.base;

        tensor_div_layer->tensorDivSW.input0    = input0;
        tensor_div_layer->tensorDivSW.input1    = input1;
        tensor_div_layer->tensorDivSW.scale = scale;
        tensor_div_layer->tensorDivSW.overflow  = overflow;
        tensor_div_layer->tensorDivSW.rounding = rounding;
        tensor_div_layer->tensorDivSW.output    = output;
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
    }

    return VX_SUCCESS;
}
//end tensor div
VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorTrans(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorTrans_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorTrans_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

vx_status vxnneExecuteTPTensorTranspose(struct _vxnne_operation_s *operation)
{
    vxnne_tensor_trans_layer layer = (vxnne_tensor_trans_layer)operation->layer;
    vxnne_tp_operation tensorTransOperation = (vxnne_tp_operation)operation;

    vx_tensor* inputs  = tensorTransOperation->inputs;
    vx_tensor* outputs = tensorTransOperation->outputs;
    vx_node node = layer->base.node;

    return vxnneExecuteTPGeneric(node, inputs, VX_NULL, VX_NULL, outputs, layer->base.cmdTPBuff, tensorTransOperation->op_num, tensorTransOperation->multi_tensor);
}



VX_PRIVATE_API vx_status vxnneExecuteSWTensorTranspose(struct _vxnne_operation_s *operation)
{
    vxnne_tensor_trans_operation transOperation = (vxnne_tensor_trans_operation)operation;

    vx_tensor input  = (vx_tensor)transOperation->input;
    vx_tensor output = (vx_tensor)transOperation->output;

    vx_uint32_ptr perm = (vx_uint32_ptr)transOperation->perm->memory.logicals[0];
    vx_uint32 pnum = transOperation->pnum->value->u32;

    vx_uint8_ptr inaddr, outaddr;
    vx_uint32 dims[VX_CONTEXT_TENSOR_MAX_DIMENSION], strides[VX_CONTEXT_TENSOR_MAX_DIMENSION], tstrides[VX_CONTEXT_TENSOR_MAX_DIMENSION];

    vxoTensor_GetTensorViewMemory(input, (gctPOINTER*)&inaddr, VX_NULL);
    vxoTensor_GetTensorViewMemory(output, (gctPOINTER*)&outaddr, VX_NULL);

    vxoTensor_GetTensorDimStride(input, &pnum, dims, strides);
    vxoTensor_GetTensorDimStride(output, &pnum, VX_NULL, tstrides);

    if (pnum == 1)
    {
        memcpy(outaddr, inaddr, dims[0] * strides[0]);
    }
    else
    {
        _TransposeTensor(inaddr, outaddr, input->tensorBuffer->elementSize, dims, strides, tstrides, perm, pnum-1);
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorTrans_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vx_context context = vxGetContext((vx_reference)node);

    vx_tensor input   = (vx_tensor)parameters[0];
    vx_array  perm    = (vx_array)parameters[1];
    vx_scalar pnum    = (vx_scalar)parameters[2];
    vx_tensor output  = (vx_tensor)parameters[3];

    vxnne_tensor_trans_layer tensor_trans_layer = VX_NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_tensor_trans_layer_s), (gctPOINTER*)&tensor_trans_layer);
    if (!tensor_trans_layer)
    {
        status = VX_ERROR_NO_MEMORY;
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(tensor_trans_layer, sizeof(vxnne_tensor_trans_layer_s));

    vxnneLayer_Initialize(&tensor_trans_layer->base, "TensorTransLayer", node, tensor_trans_layer->operations, VX_NULL);

    if (context->nnConfig.tpCoreCount && context->options.enableTP && context->options.flagTPFunc[TP_TRANSPOSE] && pnum->value->u32 > 1)
    {
        vx_tp_value_cmd values;

        vxnneOperation_Initialize(&tensor_trans_layer->tensor_trans_tp_operation.base,
                                  &tensor_trans_layer->base,
                                  VXNNE_OPERATION_TARGET_TP,
                                  VXNNE_OPERATOR_TENSOR_TRANS,
                                  vxnneExecuteTPTensorTranspose,
                                  VX_NULL);

        tensor_trans_layer->base.num_operations = 1;
        tensor_trans_layer->operations[0] = (vxnne_operation)&tensor_trans_layer->tensor_trans_tp_operation;
        tensor_trans_layer->tensor_trans_tp_operation.inputs[0]    = input;
        tensor_trans_layer->tensor_trans_tp_operation.outputs[0]   = output;
        tensor_trans_layer->tensor_trans_tp_operation.op_num       = 1;
        tensor_trans_layer->tensor_trans_tp_operation.multi_tensor = vx_false_e;

        /* create cmd buffer for TP operation */
        tensor_trans_layer->base.cmdTPBuff = vxCreateArray(context, VX_TYPE_CHAR, TP_COMMAND_SIZE);
        if (!vxoArray_AllocateMemory(tensor_trans_layer->base.cmdTPBuff))
        {
            status = VX_ERROR_NO_MEMORY;
            gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            goto exit;
        }

        values.u32[0] = pnum->value->u32;
        values.p8[0] = perm->memory.logicals[0];
        fillInCmdTPBuffer(
                input, VX_NULL, output,
                tensor_trans_layer->base.cmdTPBuff, VX_NULL,
                VX_NULL, &values,
                TP_TRANSPOSE,
                0, vx_false_e,
                vx_true_e);
    }
    else
    {
        vx_uint32_ptr  pPerm                   = (vx_uint32_ptr)perm->memory.logicals[0];
        vx_uint32      num                     = pnum->value->u32;
        vx_bool        shExe_flag              = vx_true_e;
        vx_enum        inputFormat             = input->tensorBuffer->dataFormat;
        vx_enum        outputFormat            = output->tensorBuffer->dataFormat;

        shExe_flag    = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16 && pPerm[0] == 2 && pPerm[1] == 0 && pPerm[2] == 1 && num == 3));

        if(shExe_flag)
        {
            vxnne_shader_executable shaderExecutable;
            shaderExecutable = vxnneTensorTransposeShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_TRANSPOSE, &node->kernelAttributes.borderMode, input, pPerm, num, output);

            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto exit;
            }
            status = vxnneShaderOperation_Initialize(&tensor_trans_layer->tensor_trans_shader_operation,
                &tensor_trans_layer->base,
                VXNNE_OPERATOR_TENSOR_TRANS,
                shaderExecutable);

            if (status != VX_SUCCESS)
                goto exit;

            tensor_trans_layer->base.num_operations = 1;
            tensor_trans_layer->operations[0] = (vxnne_operation)&tensor_trans_layer->tensor_trans_shader_operation.base;
        }
        else
        {
            vxnneOperation_Initialize(&tensor_trans_layer->tensor_trans_sw_operation.base,
                                      &tensor_trans_layer->base,
                                      VXNNE_OPERATION_TARGET_SW,
                                      VXNNE_OPERATOR_TENSOR_TRANS,
                                      vxnneExecuteSWTensorTranspose,
                                      VX_NULL);

            tensor_trans_layer->base.num_operations = 1;
            tensor_trans_layer->operations[0] = &tensor_trans_layer->tensor_trans_sw_operation.base;

            tensor_trans_layer->tensor_trans_sw_operation.input   = input;
            tensor_trans_layer->tensor_trans_sw_operation.perm    = perm;
            tensor_trans_layer->tensor_trans_sw_operation.pnum    = pnum;
            tensor_trans_layer->tensor_trans_sw_operation.output  = output;
        }
    }

    node->layer = &tensor_trans_layer->base;
    return status;

exit:
    if (tensor_trans_layer)
        gcoOS_Free(NULL, (gctPOINTER)tensor_trans_layer);

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorTrans_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNRPNLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNRPNLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNRPNLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}



vx_status vxnneExecuteSWRPN_Softmax(struct _vxnne_operation_s *operation)
{
    vx_status  status = VX_SUCCESS;
    vxnne_tensor_rpn_softmax_operation rpnSoftmaxOperation = (vxnne_tensor_rpn_softmax_operation)operation;

    vx_tensor input     = rpnSoftmaxOperation->input;
    vx_tensor output    = rpnSoftmaxOperation->output;

    vx_uint32 channel   = TENSOR_VIEW_SIZE_INDEX(input, 2);
    vx_uint32 height    = TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_uint32 width     = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32 count     = width * height * channel / 2;

    vx_type_e in_format     = (vx_type_e)input->tensorBuffer->dataFormat;
    vx_type_e out_format    = (vx_type_e)output->tensorBuffer->dataFormat;
    vx_int8 in_fp           = input->tensorBuffer->fixedPointPos;
    vx_int8 out_fp          = input->tensorBuffer->fixedPointPos;

    vx_bool input_stage = rpnSoftmaxOperation->input_stage;
    vx_bool output_stage = rpnSoftmaxOperation->output_stage;

    vx_uint32 i;
    vx_uint8_ptr input_data,output_data;

    vxnneGetTensorMemeory(input, (vx_ptr_ptr)&input_data, input_stage, vx_false_e);
    vxnneGetTensorMemeory(output, (vx_ptr_ptr)&output_data, output_stage, vx_true_e);

    for(i = 0; i < count; i++)
    {
        vx_float32 value0,value1;
        vx_float32 score0 = (vx_float32)vxnneGetData(in_format, i, (vx_uint8_ptr)input_data, in_fp);
        vx_float32 score1 = (vx_float32)vxnneGetData(in_format, i + count, (vx_uint8_ptr)input_data, in_fp);
        vx_float32 sum = 0.0f, max = gcmMAX(score0, score1);

        score0 -= max;
        score1 -= max;

        score0 = expf(score0);
        score1 = expf(score1);
        sum = score0 + score1;

        value0 = score0 / sum;
        value1 = score1 / sum;
        vxnneSaveData(out_format, i, value0, (vx_uint8_ptr)output_data, out_fp, output->tensorBuffer->roundingMode);
        vxnneSaveData(out_format, (i + count), value1, (vx_uint8_ptr)output_data, out_fp, output->tensorBuffer->roundingMode);
    }

    /*
    for(i = 0; i < count; i++)
    {
        vx_float32 value = (vx_float32)vxnneGetData(out_format, (i + count), (vx_uint8_ptr)output_data, out_fp);
        printf("i[%u], value = %f\n", i, value);
    }
    */

    if(input_stage)
    {
        vxFree(input_data);
    }
    if(output_stage)
    {
        vx_uint32 output_size = 0;
        vx_ptr output_logical = VX_NULL;
        vxoTensor_GetTensorSize(output, &output_size);
        vxoTensor_GetTensorViewMemory(output, &output_logical, VX_NULL);
        gcoOS_MemCopy(output_logical, output_data, output_size);

        vxFree(output_data);
    }

    return status;
}

vx_status vxnneExecuteSWRPN_Regression(struct _vxnne_operation_s *operation)
{
    vx_status  status = VX_SUCCESS;
    vxnne_tensor_rpn_regression_operation rpnRegOperation = (vxnne_tensor_rpn_regression_operation)operation;

    vx_tensor score         = rpnRegOperation->score_buffer;
    vx_tensor bbox          = rpnRegOperation->bbox;
    vx_tensor anchor        = rpnRegOperation->anchors;
    vx_tensor img_info      = rpnRegOperation->img_info;
    vx_tensor output        = rpnRegOperation->output;

    vx_uint32 feat_stride   = rpnRegOperation->feature_stride->value->u32;
    vx_uint32 min_size      = rpnRegOperation->min_size->value->u32;

    vx_bool input_stage     = rpnRegOperation->input_stage;
    vx_bool output_stage    = rpnRegOperation->output_stage;

    vx_type_e in_score_format   = (vx_type_e)score->tensorBuffer->dataFormat;
    vx_type_e in_bbox_format    = (vx_type_e)bbox->tensorBuffer->dataFormat;
    vx_type_e in_anchor_format  = (vx_type_e)anchor->tensorBuffer->dataFormat;
    vx_type_e in_img_format     = (vx_type_e)img_info->tensorBuffer->dataFormat;
    vx_int8 in_bbox_fp          = bbox->tensorBuffer->fixedPointPos;
    vx_int8 in_anchor_fp        = anchor->tensorBuffer->fixedPointPos;
    vx_int8 in_img_fp           = img_info->tensorBuffer->fixedPointPos;

    vx_uint32 score_channel = TENSOR_VIEW_SIZE_INDEX(score, 2);
    vx_uint32 score_height  = TENSOR_VIEW_SIZE_INDEX(score, 1);
    vx_uint32 score_width   = TENSOR_VIEW_SIZE_INDEX(score, 0);
    vx_uint32 score_count   = score_width * score_height * score_channel/2;

    vx_uint32 proposal_width    = score_width;
    vx_uint32 proposal_height   = score_height;
    vx_uint32 anchor_count      = TENSOR_VIEW_SIZE_INDEX(anchor, 3); /* anchor batch = anchor number */

    vx_uint32 proposal_area     = proposal_width * proposal_height;

    vx_uint8_ptr bbox_data,img_data,anchor_data,score_data,out_data;
    vx_float32_ptr foreground_score, proposals, p_proposals;
    vx_float32 img_W,img_H,img_scale_H,img_scale_W,min_box_H,min_box_W;
    vx_uint32 w, h, k;

    vxnneGetTensorMemeory(score, (vx_ptr_ptr)&score_data, input_stage, vx_false_e);
    vxnneGetTensorMemeory(bbox, (vx_ptr_ptr)&bbox_data, input_stage, vx_false_e);
    vxnneGetTensorMemeory(img_info, (vx_ptr_ptr)&img_data, input_stage, vx_false_e);
    vxnneGetTensorMemeory(anchor, (vx_ptr_ptr)&anchor_data, input_stage, vx_false_e);
    vxnneGetTensorMemeory(output, (vx_ptr_ptr)&out_data, output_stage, vx_true_e);

    /* because temp tensor data format is always FP32 */
    foreground_score = (vx_float32_ptr)(score_data + score_count*vxnneGetTypeSize(in_score_format));
    proposals        = (vx_float32_ptr)out_data;

    img_W       = (vx_float32)vxnneGetData(in_img_format, 0, (vx_uint8_ptr)img_data, in_img_fp);
    img_H       = (vx_float32)vxnneGetData(in_img_format, 1, (vx_uint8_ptr)img_data, in_img_fp);
    img_scale_W = (vx_float32)vxnneGetData(in_img_format, 2, (vx_uint8_ptr)img_data, in_img_fp);
    img_scale_H = (vx_float32)vxnneGetData(in_img_format, 3, (vx_uint8_ptr)img_data, in_img_fp);
    min_box_W   = min_size * img_scale_W;
    min_box_H   = min_size * img_scale_H;

    /*
    for(i = 0; i < score_count; i++)
    {
        printf("i[%u] f_score[%f]\n", i, foreground_score[i]);
    }
    */

    p_proposals = proposals;
    for(h=0; h<proposal_height; h++)
    {
        for(w=0; w<proposal_width; w++)
        {
            vx_uint32 x = w * feat_stride;
            vx_uint32 y = h * feat_stride;
            vx_uint32 offset    = h * proposal_width + w;
            vx_uint8_ptr p_box  = bbox_data + offset * vxnneGetTypeSize(in_bbox_format);
            vx_float32 *p_score = foreground_score + offset;

            for(k=0; k<anchor_count; k++)
            {
                vx_float32 dx = vxnneGetData(in_bbox_format, (k*4+0)*proposal_area, (vx_uint8_ptr)p_box, in_bbox_fp);
                vx_float32 dy = vxnneGetData(in_bbox_format, (k*4+1)*proposal_area, (vx_uint8_ptr)p_box, in_bbox_fp);
                vx_float32 d_log_w = vxnneGetData(in_bbox_format, (k*4+2)*proposal_area, (vx_uint8_ptr)p_box, in_bbox_fp);
                vx_float32 d_log_h = vxnneGetData(in_bbox_format, (k*4+3)*proposal_area, (vx_uint8_ptr)p_box, in_bbox_fp);

                /* proposals = {x1, y1, x2, y2, score} */
                p_proposals[0] = x + vxnneGetData(in_anchor_format, (k*4+0), (vx_uint8_ptr)anchor_data, in_anchor_fp);
                p_proposals[1] = y + vxnneGetData(in_anchor_format, (k*4+1), (vx_uint8_ptr)anchor_data, in_anchor_fp);
                p_proposals[2] = x + vxnneGetData(in_anchor_format, (k*4+2), (vx_uint8_ptr)anchor_data, in_anchor_fp);
                p_proposals[3] = y + vxnneGetData(in_anchor_format, (k*4+3), (vx_uint8_ptr)anchor_data, in_anchor_fp);

                p_proposals[4] = vx_nn_rpn_transform_box(
                                    p_proposals,
                                    dx, dy,
                                    d_log_w, d_log_h,
                                    img_W, img_H,
                                    min_box_W, min_box_H
                                  ) * p_score[k * proposal_area];
                p_proposals += 5;
            }
        }
    }

    /*
    for(w = 0; w < (proposal_height*proposal_width*anchor_count); w++)
    {
        vx_float32 x1 = vxnneGetData((vx_type_e)output->tensorBuffer->dataFormat, w*5+0, (vx_uint8_ptr)out_data, output->tensorBuffer->fixedPointPos);
        vx_float32 y1 = vxnneGetData((vx_type_e)output->tensorBuffer->dataFormat, w*5+1, (vx_uint8_ptr)out_data, output->tensorBuffer->fixedPointPos);
        vx_float32 x2 = vxnneGetData((vx_type_e)output->tensorBuffer->dataFormat, w*5+2, (vx_uint8_ptr)out_data, output->tensorBuffer->fixedPointPos);
        vx_float32 y2 = vxnneGetData((vx_type_e)output->tensorBuffer->dataFormat, w*5+3, (vx_uint8_ptr)out_data, output->tensorBuffer->fixedPointPos);
        vx_float32 sc = vxnneGetData((vx_type_e)output->tensorBuffer->dataFormat, w*5+4, (vx_uint8_ptr)out_data, output->tensorBuffer->fixedPointPos);

        printf("b[%u] score[%f] coordinate[ %f|%f|%f|%f ]\n", w, sc, x1, y1, x2, y2);
    }
    */

    if (input_stage)
    {
        vxFree(score_data);
        vxFree(bbox_data);
        vxFree(img_data);
        vxFree(anchor_data);
    }

    if (output_stage)
    {
        vx_uint32 output_size = 0;
        vx_ptr output_logical = VX_NULL;
        vxoTensor_GetTensorSize(output, &output_size);
        vxoTensor_GetTensorViewMemory(output, &output_logical, VX_NULL);
        gcoOS_MemCopy(output_logical, out_data, output_size);

        vxFree(out_data);
    }

    return status;
}

vx_status vxnneExecuteSWRPN_Sort(struct _vxnne_operation_s *operation)
{
    vx_status  status = VX_SUCCESS;
    vxnne_tensor_rpn_sort_operation rpnSortOperation = (vxnne_tensor_rpn_sort_operation)operation;
    //vx_uint32 w;

    vx_tensor proposals     = rpnSortOperation->proposal;
    vx_uint32 pre_nms_topn  = rpnSortOperation->pre_nms_topn->value->u32;

    vx_bool output_stage    = rpnSortOperation->output_stage;

    vx_uint32 proposal_count = TENSOR_VIEW_SIZE_INDEX(proposals, 3);
    vx_uint8_ptr proposals_data = NULL;
    vx_float32_ptr proposals_ptr = NULL;

    vxnneGetTensorMemeory(proposals, (vx_ptr_ptr)&proposals_data, output_stage, vx_false_e);
    proposals_ptr = (vx_float32_ptr)proposals_data;

    vx_nn_rpn_qsort_box(proposals_ptr, 0, proposal_count-1, pre_nms_topn);

    /*
    for(w = 0; w < proposal_count; w++)
    {
        vx_float32 x1 = vxnneGetData((vx_type_e)proposals->tensorBuffer->dataFormat, w*5+0, (vx_uint8_ptr)proposals_data, proposals->tensorBuffer->fixedPointPos);
        vx_float32 y1 = vxnneGetData((vx_type_e)proposals->tensorBuffer->dataFormat, w*5+1, (vx_uint8_ptr)proposals_data, proposals->tensorBuffer->fixedPointPos);
        vx_float32 x2 = vxnneGetData((vx_type_e)proposals->tensorBuffer->dataFormat, w*5+2, (vx_uint8_ptr)proposals_data, proposals->tensorBuffer->fixedPointPos);
        vx_float32 y2 = vxnneGetData((vx_type_e)proposals->tensorBuffer->dataFormat, w*5+3, (vx_uint8_ptr)proposals_data, proposals->tensorBuffer->fixedPointPos);
        vx_float32 sc = vxnneGetData((vx_type_e)proposals->tensorBuffer->dataFormat, w*5+4, (vx_uint8_ptr)proposals_data, proposals->tensorBuffer->fixedPointPos);

        printf("b[%u] score[%f] coordinate[ %f|%f|%f|%f ]\n", w, sc, x1, y1, x2, y2);
    }
    */

    if(output_stage)
    {
        vx_uint32 output_size = 0;
        vx_ptr output_logical = VX_NULL;
        vxoTensor_GetTensorSize(proposals, &output_size);
        vxoTensor_GetTensorViewMemory(proposals, &output_logical, VX_NULL);
        gcoOS_MemCopy(output_logical, proposals_data, output_size);

        vxFree(proposals_data);
    }

    return status;
}

vx_status vxnneExecuteSWRPN_NMS(struct _vxnne_operation_s *operation)
{
    vx_status  status = VX_SUCCESS;
    vxnne_tensor_rpn_nms_operation rpnNmsOperation = (vxnne_tensor_rpn_nms_operation)operation;

    vx_tensor proposal      = rpnNmsOperation->proposal;
    vx_tensor roi_indices   = rpnNmsOperation->roi_indices;
    vx_tensor real_roi_t    = rpnNmsOperation->real_roi_t;
    vx_uint32 pre_nms_topn  = rpnNmsOperation->pre_nms_topn->value->u32;
    vx_uint32 post_nms_topn = rpnNmsOperation->post_nms_topn->value->u32;
    vx_float32 nms_thresh   = rpnNmsOperation->nms_thresh->value->f32;
    vx_bool output_stage    = rpnNmsOperation->output_stage;

    vx_uint32 roi_count         = TENSOR_VIEW_SIZE_INDEX(roi_indices, 0);
    vx_type_e roi_ind_format    = (vx_type_e)roi_indices->tensorBuffer->dataFormat;
    vx_type_e real_roi_format   = (vx_type_e)real_roi_t->tensorBuffer->dataFormat;
    vx_int8 roi_ind_fp          = roi_indices->tensorBuffer->fixedPointPos;
    vx_int8 real_roi_fp         = real_roi_t->tensorBuffer->fixedPointPos;
    vx_enum roi_ind_rMode       = roi_indices->tensorBuffer->roundingMode;
    vx_enum real_roi_rMode      = real_roi_t->tensorBuffer->roundingMode;

    vx_uint8_ptr proposals_data = NULL, roi_indices_data = NULL, real_roi_t_data = NULL;
    vx_float32_ptr proposals_ptr = NULL;
    vx_uint32_ptr roi_indices_ptr = NULL;
    vx_uint32 i,real_roi = 0;

    roi_indices_ptr = (vx_uint32_ptr)malloc(roi_count * sizeof(vx_uint32));

    gcoOS_MemFill(roi_indices_ptr, 0, roi_count * sizeof(vx_uint32));
    vxnneGetTensorMemeory(proposal, (vx_ptr_ptr)&proposals_data, output_stage, vx_false_e);
    vxnneGetTensorMemeory(roi_indices, (vx_ptr_ptr)&roi_indices_data, output_stage, vx_true_e);
    vxoTensor_GetTensorViewMemory(real_roi_t, (vx_ptr_ptr)&real_roi_t_data, VX_NULL);
    proposals_ptr   = (vx_float32_ptr)proposals_data;

    vx_nn_rpn_nms_cpu(pre_nms_topn, proposals_ptr, roi_indices_ptr, &real_roi, 0, nms_thresh, post_nms_topn);
    vxnneSaveData(real_roi_format, 0, real_roi, real_roi_t_data, real_roi_fp, real_roi_rMode);

    for(i = 0; i < roi_count; i++)
    {
        vxnneSaveData(roi_ind_format, i, roi_indices_ptr[i], roi_indices_data, roi_ind_fp, roi_ind_rMode);
    }

    /*
    for(i=0; i<real_roi; i++)
    {
        vx_float32_ptr ptr = proposals_ptr + roi_indices_ptr[i] * 5;
        printf("i[%u] roi_ind[i]=%u, roi_f[ %f|%f|%f|%f -- %f ]\n", i, roi_indices_ptr[i], ptr[0],ptr[1],ptr[2],ptr[3],ptr[4]);
    }

    for(i=0; i<real_roi; i++)
    {
        vx_float32 f = vxnneGetData((vx_type_e)roi_indices->tensorBuffer->dataFormat, i, (vx_uint8_ptr)roi_indices_data, roi_indices->tensorBuffer->fixedPointPos);
        printf("i[%u] index[i]=%f\n", i, f);
    }
    */

    if(roi_indices_ptr)
        free(roi_indices_ptr);

    if(output_stage)
    {
        vx_uint32 output_size = 0;
        vx_ptr output_logical = VX_NULL;

        vxoTensor_GetTensorSize(proposal, &output_size);
        vxoTensor_GetTensorViewMemory(proposal, &output_logical, VX_NULL);
        gcoOS_MemCopy(output_logical, proposals_data, output_size);

        vxoTensor_GetTensorSize(roi_indices, &output_size);
        vxoTensor_GetTensorViewMemory(roi_indices, &output_logical, VX_NULL);
        gcoOS_MemCopy(output_logical, roi_indices_data, output_size);

        vxFree(proposals_data);
        vxFree(roi_indices_data);
    }

    return status;
}

vx_status vxnneExecuteSWRPN_Retrieve(struct _vxnne_operation_s *operation)
{
    vx_status  status = VX_SUCCESS;
    vxnne_tensor_rpn_retrieve_operation rpnRetOperation = (vxnne_tensor_rpn_retrieve_operation)operation;

    vx_tensor proposal      = rpnRetOperation->proposal;
    vx_tensor roi_indices   = rpnRetOperation->roi_indices;
    vx_tensor real_roi_t    = rpnRetOperation->real_roi_t;
    vx_tensor roi_output    = rpnRetOperation->roi_output;
    vx_tensor score_output  = rpnRetOperation->score_output;

    vx_bool input_stage     = rpnRetOperation->input_stage;
    vx_bool output_stage    = rpnRetOperation->output_stage;

    vx_type_e roi_out_format   = (vx_type_e)roi_output->tensorBuffer->dataFormat;
    vx_type_e real_roi_format  = (vx_type_e)real_roi_t->tensorBuffer->dataFormat;
    //vx_type_e roi_ind_format  = (vx_type_e)roi_indices->tensorBuffer->dataFormat;
    vx_int8 roi_out_fp         = roi_output->tensorBuffer->fixedPointPos;
    vx_int8 real_roi_fp        = real_roi_t->tensorBuffer->fixedPointPos;
    //vx_int8 roi_ind_fp         = roi_indices->tensorBuffer->fixedPointPos;
    vx_enum roi_out_rMode      = roi_output->tensorBuffer->roundingMode;

    vx_uint8_ptr proposal_data = NULL, roi_indices_data = NULL, real_roi_t_data = NULL;
    vx_uint8_ptr roi_output_data = NULL;
    vx_float32_ptr proposal_ptr = NULL, p_proposal_ptr = NULL, roi_indices_ptr = NULL;
    vx_uint32 i,real_roi = 0;

    vx_type_e out_score_format = 0;
    vx_int8 out_score_fp = 0;
    vx_uint8_ptr out_score_data = NULL;
    vx_enum out_score_rMode = 0;

    vxnneGetTensorMemeory(proposal, (vx_ptr_ptr)&proposal_data, input_stage, vx_false_e);
    vxnneGetTensorMemeory(roi_indices, (vx_ptr_ptr)&roi_indices_data, input_stage, vx_false_e);
    vxnneGetTensorMemeory(roi_output, (vx_ptr_ptr)&roi_output_data, output_stage, vx_true_e);
    proposal_ptr = (vx_float32_ptr)proposal_data;
    roi_indices_ptr = (vx_float32_ptr)roi_indices_data;

    if(score_output)
    {
        out_score_format    = (vx_type_e)score_output->tensorBuffer->dataFormat;
        out_score_fp        = score_output->tensorBuffer->fixedPointPos;
        out_score_rMode     = score_output->tensorBuffer->roundingMode;
        vxnneGetTensorMemeory(score_output, (vx_ptr_ptr)&out_score_data, output_stage, vx_true_e);
    }

    vxoTensor_GetTensorViewMemory(real_roi_t, (vx_ptr_ptr)&real_roi_t_data, VX_NULL);
    real_roi = (vx_uint32)vxnneGetData(real_roi_format, 0, (vx_uint8_ptr)real_roi_t_data, real_roi_fp);

    for(i = 0; i < real_roi; i++)
    {
        vx_float32 item_index = 0.0f; /* item_index = input score batch, but we only supported single batch */
        vx_float32 findex = roi_indices_ptr[i];
        vx_uint32 index = (vx_uint32)findex;
        p_proposal_ptr = proposal_ptr + index * 5;

        /*
        printf("i[%u] index[%u], roi=[%f|%f|%f|%f], score=[%f]\n",
            i, index, p_proposal_ptr[0], p_proposal_ptr[1], p_proposal_ptr[2], p_proposal_ptr[3], p_proposal_ptr[4]);
        */

        /* Copy proposals coordinate(x1, y1, x2, y2) to roi output tensor */
        vxnneSaveData(roi_out_format, (i * 5 + 0), item_index, (vx_uint8_ptr)roi_output_data, roi_out_fp, roi_out_rMode);
        vxnneSaveData(roi_out_format, (i * 5 + 1), p_proposal_ptr[0], (vx_uint8_ptr)roi_output_data, roi_out_fp, roi_out_rMode);
        vxnneSaveData(roi_out_format, (i * 5 + 2), p_proposal_ptr[1], (vx_uint8_ptr)roi_output_data, roi_out_fp, roi_out_rMode);
        vxnneSaveData(roi_out_format, (i * 5 + 3), p_proposal_ptr[2], (vx_uint8_ptr)roi_output_data, roi_out_fp, roi_out_rMode);
        vxnneSaveData(roi_out_format, (i * 5 + 4), p_proposal_ptr[3], (vx_uint8_ptr)roi_output_data, roi_out_fp, roi_out_rMode);

        /* Copy proposals score to score output tensor */
        if(score_output)
        {
            vxnneSaveData(out_score_format, i, p_proposal_ptr[4], (vx_uint8_ptr)out_score_data, out_score_fp, out_score_rMode);
        }

    }

    if (input_stage)
    {
        vxFree(proposal_data);
        vxFree(roi_indices_data);
    }

    if (output_stage)
    {
        vx_uint32 roi_output_size = 0;
        vx_ptr roi_output_logical = VX_NULL;
        vxoTensor_GetTensorSize(roi_output, &roi_output_size);
        vxoTensor_GetTensorViewMemory(roi_output, &roi_output_logical, VX_NULL);
        gcoOS_MemCopy(roi_output_logical, roi_output_data, roi_output_size);

        vxFree(roi_output_data);
    }

    if(score_output && output_stage == vx_true_e)
    {
        vx_uint32 score_output_size = 0;
        vx_ptr score_output_logical = VX_NULL;
        vxoTensor_GetTensorSize(score_output, &score_output_size);
        vxoTensor_GetTensorViewMemory(score_output, &score_output_logical, VX_NULL);
        gcoOS_MemCopy(score_output_logical, out_score_data, score_output_size);

        vxFree(out_score_data);
    }

    return status;
}


VX_PRIVATE_API vx_status VX_CALLBACK vxoNNRPNLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status  status                     = VX_SUCCESS;

    vx_tensor  score                      = (vx_tensor)parameters[0];
    vx_tensor  bbox                       = (vx_tensor)parameters[1];
    vx_tensor  anchors                    = (vx_tensor)parameters[2];
    vx_tensor  img_info                   = (vx_tensor)parameters[3];
    vx_scalar  feature_stride             = (vx_scalar)parameters[4];
    vx_scalar  min_size                   = (vx_scalar)parameters[5];
    vx_scalar  pre_nms_topn               = (vx_scalar)parameters[6];
    vx_scalar  post_nms_topn              = (vx_scalar)parameters[7];
    vx_scalar  nms_thresh                 = (vx_scalar)parameters[8];
    vx_tensor  roi_output                 = (vx_tensor)parameters[9];
    vx_tensor  score_output               = (vx_tensor)parameters[10];

    vxnne_tensor_rpn_layer rpnLayer;
    vx_uint32 dims,sizes[4] = {0,0,0,0};
    vx_tensor socreBufferTensor = VX_NULL;
    vx_tensor proposalTensor = VX_NULL;
    vx_tensor roiIndicesTensor = VX_NULL;
    vx_tensor realRoiTensor = VX_NULL;

    vx_enum temp_format     = VX_TYPE_FLOAT32; /* To avoid the loss of accuracy */
    vx_bool input_stage     = vx_true_e;
    vx_bool output_stage    = vx_true_e;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    {
        rpnLayer = VX_NULL;

        gcoOS_Allocate(gcvNULL, sizeof(vxnne_tensor_rpn_layer_s), (gctPOINTER*)&rpnLayer);
        if (!rpnLayer)
        {
            status = VX_ERROR_NO_MEMORY;
            gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            return status;
        }

        gcoOS_ZeroMemory(rpnLayer, sizeof(vxnne_tensor_rpn_layer_s));

        vxnneLayer_Initialize(&rpnLayer->base,
                                "RpnLayer",
                                node,
                                rpnLayer->operations,
                                VX_NULL);

        /* -----------RPN Softmax------------ */
        /* create a temp tensor to store the scores. */
        dims        = TENSOR_DIM_NUM(score);
        sizes[0]    = TENSOR_SIZE_INDEX(score, 0);
        sizes[1]    = TENSOR_SIZE_INDEX(score, 1);
        sizes[2]    = TENSOR_SIZE_INDEX(score, 2);
        sizes[3]    = TENSOR_SIZE_INDEX(score, 3);

        socreBufferTensor = vxoTensor_CreateTensor(node->base.context, dims, sizes, temp_format, vx_false_e);
        if (vxoTensor_AllocateMemory(socreBufferTensor) != VX_SUCCESS)
        {
            gcmPRINT("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }

        vxnneOperation_Initialize(&rpnLayer->tensorRpnSoftmaxSW.base,
                                    &rpnLayer->base,
                                    VXNNE_OPERATION_TARGET_SW,
                                    VXNNE_OPERATOR_RPN_SOFTMAX,
                                    vxnneExecuteSWRPN_Softmax,
                                    VX_NULL);

        rpnLayer->tensorRpnSoftmaxSW.input          = score;
        rpnLayer->tensorRpnSoftmaxSW.output         = socreBufferTensor;

        rpnLayer->tensorRpnSoftmaxSW.input_stage    = input_stage;
        rpnLayer->tensorRpnSoftmaxSW.output_stage   = output_stage;

        rpnLayer->base.num_temp_tensors             = 1;
        rpnLayer->base.temp_tensors[0]              = socreBufferTensor;


        /* -----------RPN bbox regression------------- */
        /*
            create a temp tensor to store the proposal buffer
            proposal buffer: N*5*1*1
                N: score_width*score_height*anchor_number
                5: 2 sets of coordinates + scores --- (x1, y1, x2, y2, score)
        */
        dims = 4;
        sizes[0]    = 1;
        sizes[1]    = 1;
        sizes[2]    = 5;
        sizes[3]    = TENSOR_SIZE_INDEX(score, 0) * TENSOR_SIZE_INDEX(score, 1) * TENSOR_SIZE_INDEX(anchors, 3);

        proposalTensor = vxoTensor_CreateTensor(node->base.context, dims, sizes, temp_format, vx_false_e);
        if (vxoTensor_AllocateMemory(proposalTensor) != VX_SUCCESS)
        {
            gcmPRINT("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }

        vxnneOperation_Initialize(&rpnLayer->tensorRpnRegressionSW.base,
                                    &rpnLayer->base,
                                    VXNNE_OPERATION_TARGET_SW,
                                    VXNNE_OPERATOR_RPN_REGRESSION,
                                    vxnneExecuteSWRPN_Regression,
                                    VX_NULL);

        rpnLayer->tensorRpnRegressionSW.feature_stride  = feature_stride;
        rpnLayer->tensorRpnRegressionSW.min_size        = min_size;
        rpnLayer->tensorRpnRegressionSW.score_buffer    = socreBufferTensor;
        rpnLayer->tensorRpnRegressionSW.bbox            = bbox;
        rpnLayer->tensorRpnRegressionSW.anchors         = anchors;
        rpnLayer->tensorRpnRegressionSW.img_info        = img_info;
        rpnLayer->tensorRpnRegressionSW.output          = proposalTensor;

        rpnLayer->tensorRpnRegressionSW.input_stage     = input_stage;
        rpnLayer->tensorRpnRegressionSW.output_stage    = output_stage;

        rpnLayer->base.num_temp_tensors                 += 1;
        rpnLayer->base.temp_tensors[1]                  = proposalTensor;


        /* ------------RPN Sort--------------------- */
        vxnneOperation_Initialize(&rpnLayer->tensorRpnSortSW.base,
                                    &rpnLayer->base,
                                    VXNNE_OPERATION_TARGET_SW,
                                    VXNNE_OPERATOR_RPN_SORT,
                                    vxnneExecuteSWRPN_Sort,
                                    VX_NULL);

        rpnLayer->tensorRpnSortSW.pre_nms_topn  = pre_nms_topn;
        rpnLayer->tensorRpnSortSW.proposal      = proposalTensor;

        rpnLayer->tensorRpnSortSW.input_stage     = input_stage;
        rpnLayer->tensorRpnSortSW.output_stage    = output_stage;

        /* ------------RPN NMS--------------------- */
        /* create a temp tensor to store the index of the nms's proposal */
        dims        = 1;
        sizes[0]    = post_nms_topn->value->u32;
        sizes[1]    = 0;
        sizes[2]    = 0;
        sizes[3]    = 0;
        roiIndicesTensor = vxoTensor_CreateTensor(node->base.context, dims, sizes, temp_format, vx_false_e);
        if (vxoTensor_AllocateMemory(roiIndicesTensor) != VX_SUCCESS)
        {
            gcmPRINT("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }

        /* create a temp tensor to store the real proposal number. because maybe real_roi < post_nms_topn */
        dims        = 1;
        sizes[0]    = 1; /* we only save a uint32 value */
        sizes[1]    = 0;
        sizes[2]    = 0;
        sizes[3]    = 0;
        realRoiTensor = vxoTensor_CreateTensor(node->base.context, dims, sizes, temp_format, vx_false_e);
        if (vxoTensor_AllocateMemory(realRoiTensor) != VX_SUCCESS)
        {
            gcmPRINT("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }

        vxnneOperation_Initialize(&rpnLayer->tensorRpnNmsSW.base,
                                    &rpnLayer->base,
                                    VXNNE_OPERATION_TARGET_SW,
                                    VXNNE_OPERATOR_RPN_NMS,
                                    vxnneExecuteSWRPN_NMS,
                                    VX_NULL);

        rpnLayer->tensorRpnNmsSW.pre_nms_topn   = pre_nms_topn;
        rpnLayer->tensorRpnNmsSW.post_nms_topn  = post_nms_topn;
        rpnLayer->tensorRpnNmsSW.nms_thresh     = nms_thresh;
        rpnLayer->tensorRpnNmsSW.proposal       = proposalTensor;
        rpnLayer->tensorRpnNmsSW.roi_indices    = roiIndicesTensor;
        rpnLayer->tensorRpnNmsSW.real_roi_t     = realRoiTensor;

        rpnLayer->tensorRpnNmsSW.input_stage     = input_stage;
        rpnLayer->tensorRpnNmsSW.output_stage    = output_stage;

        rpnLayer->base.num_temp_tensors         += 2;
        rpnLayer->base.temp_tensors[2]          = roiIndicesTensor;
        rpnLayer->base.temp_tensors[3]          = realRoiTensor;


        /* ------------RPN Retrieve--------------------- */
        vxnneOperation_Initialize(&rpnLayer->tensorRpnRetrieveSW.base,
                                    &rpnLayer->base,
                                    VXNNE_OPERATION_TARGET_SW,
                                    VXNNE_OPERATOR_RPN_RETRIEVE,
                                    vxnneExecuteSWRPN_Retrieve,
                                    VX_NULL);

        rpnLayer->tensorRpnRetrieveSW.proposal       = proposalTensor;
        rpnLayer->tensorRpnRetrieveSW.roi_indices    = roiIndicesTensor;
        rpnLayer->tensorRpnRetrieveSW.real_roi_t     = realRoiTensor;
        rpnLayer->tensorRpnRetrieveSW.roi_output     = roi_output;
        rpnLayer->tensorRpnRetrieveSW.score_output   = score_output;

        rpnLayer->tensorRpnRetrieveSW.input_stage    = input_stage;
        rpnLayer->tensorRpnRetrieveSW.output_stage   = output_stage;
        /* --------------------------------- */

        rpnLayer->base.num_operations       = 5;
        rpnLayer->operations[0]             = (vxnne_operation)&rpnLayer->tensorRpnSoftmaxSW;
        rpnLayer->operations[1]             = (vxnne_operation)&rpnLayer->tensorRpnRegressionSW;
        rpnLayer->operations[2]             = (vxnne_operation)&rpnLayer->tensorRpnSortSW;
        rpnLayer->operations[3]             = (vxnne_operation)&rpnLayer->tensorRpnNmsSW;
        rpnLayer->operations[4]             = (vxnne_operation)&rpnLayer->tensorRpnRetrieveSW;
        node->layer = &rpnLayer->base;
    }
    return status;

exit:
    if (rpnLayer) gcoOS_Free(gcvNULL, (gctPOINTER)rpnLayer);
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNRPNLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

/**************************************************************************************
 *                     ROI POOL
 *************************************************************************************/
vx_status vxnneExecuteTPROIPooling(struct _vxnne_operation_s *operation)
{
    vxnne_tensor_roipool_layer layer = (vxnne_tensor_roipool_layer)operation->layer;
    vxnne_tp_operation roiOperation = (vxnne_tp_operation)operation;

    vx_node node = layer->base.node;
    vx_tensor* inputs = roiOperation->inputs;
    vx_array list = roiOperation->buffer;
    vx_tensor* outputs = roiOperation->outputs;

    return vxnneExecuteTPGeneric(node, inputs, VX_NULL, list, outputs, layer->base.cmdTPBuff, roiOperation->op_num, roiOperation->multi_tensor);
}

vx_status vxnneExecuteSWROIPooling(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;

    vxnne_tensor_roipool_operation roipoolOperation = (vxnne_tensor_roipool_operation)operation;

    vx_tensor input_data  = roipoolOperation->input_data;
    vx_tensor input_roi   = roipoolOperation->input_rois;
    vx_tensor output      = roipoolOperation->output;

    vx_float32 spatial_scale    = roipoolOperation->spatial_scale->value->f32;
    vx_int32 pooled_height      = roipoolOperation->pooled_height->value->u32;
    vx_int32 pooled_width       = roipoolOperation->pooled_width->value->u32;

    vx_int32 num_rois       = TENSOR_VIEW_SIZE_INDEX(output, 3);
    vx_int32 channel        = TENSOR_VIEW_SIZE_INDEX(input_data, 2);
    vx_int32 height         = TENSOR_VIEW_SIZE_INDEX(input_data, 1);
    vx_int32 width          = TENSOR_VIEW_SIZE_INDEX(input_data, 0);

    vx_int32 stride_w       = TENSOR_VIEW_SIZE_INDEX(input_roi, 2); /* 5 */

    vx_type_e in_data_format    = (vx_type_e)input_data->tensorBuffer->dataFormat;
    vx_type_e in_roi_format     = (vx_type_e)input_roi->tensorBuffer->dataFormat;
    vx_type_e out_format        = (vx_type_e)output->tensorBuffer->dataFormat;
    vx_int8 in_data_fp          = input_data->tensorBuffer->fixedPointPos;
    vx_int8 in_roi_fp           = input_roi->tensorBuffer->fixedPointPos;
    vx_int8 out_fp              = output->tensorBuffer->fixedPointPos;
    vx_enum in_roi_rMode        = input_roi->tensorBuffer->roundingMode;
    vx_enum out_rMode           = output->tensorBuffer->roundingMode;

    vx_int32 in_data_items      = vxnneGetTypeSize(in_data_format);
    vx_int32 in_roi_items       = vxnneGetTypeSize(in_roi_format);
    vx_int32 out_items          = vxnneGetTypeSize(out_format);

    vx_uint8_ptr input_data_ptr,rois_data_ptr,output_data_ptr;
    vx_int32 n, c, ph, pw, h, w;

    vxoTensor_GetTensorViewMemory(input_data, (gctPOINTER *)&input_data_ptr, VX_NULL);
    vxoTensor_GetTensorViewMemory(input_roi, (gctPOINTER *)&rois_data_ptr, VX_NULL);
    vxoTensor_GetTensorViewMemory(output, (gctPOINTER *)&output_data_ptr, VX_NULL);

    //vx_int32 test_index = 0; // for debug
    for(n = 0; n < num_rois; n++)
    {
        vx_int32 offset = 0, roi_batch_ind = 0, roi_start_w = 0, roi_start_h = 0, roi_end_w = 0, roi_end_h = 0;
        vx_int32 roi_height = 0, roi_width = 0;
        vx_float32 roi_size_scale_h = 0, roi_size_scale_w = 0;
        vx_uint8_ptr batch_data = VX_NULL;

        if (stride_w == 5)
        {
            offset = 1;
            roi_batch_ind = (vx_int32)vxnneGetData(in_roi_format, 0, rois_data_ptr, in_roi_fp);
        }

        /* map the roi coordinates to the feature map */
        roi_start_w = (vx_int32)vxnneRound((vx_float32)vxnneGetData(in_roi_format, offset, rois_data_ptr, in_roi_fp) * spatial_scale, in_roi_rMode);
        roi_start_h = (vx_int32)vxnneRound((vx_float32)vxnneGetData(in_roi_format, offset + 1, rois_data_ptr, in_roi_fp) * spatial_scale, in_roi_rMode);
        roi_end_w = (vx_int32)vxnneRound((vx_float32)vxnneGetData(in_roi_format, offset + 2, rois_data_ptr, in_roi_fp) * spatial_scale, in_roi_rMode);
        roi_end_h = (vx_int32)vxnneRound((vx_float32)vxnneGetData(in_roi_format, offset + 3, rois_data_ptr, in_roi_fp) * spatial_scale, in_roi_rMode);

        /* compute the roi rectangle on the feature map */
        roi_height = (vx_int32)gcmMAX(roi_end_h - roi_start_h + 1, 1);
        roi_width = (vx_int32)gcmMAX(roi_end_w - roi_start_w + 1, 1);
        roi_size_scale_h = (vx_float32)(roi_height) / (vx_float32)(pooled_height);
        roi_size_scale_w = (vx_float32)(roi_width) / (vx_float32)(pooled_width);

        batch_data = input_data_ptr + roi_batch_ind * channel * width * height * in_data_items;

        for(c = 0; c < channel; c++)
        {
            for(ph = 0; ph < pooled_height; ph++)
            {
                for(pw = 0; pw < pooled_width; pw++)
                {
                    vx_int32 pool_index = 0;
                    vx_bool is_empty = vx_false_e;
                    vx_float32 output_data_v = 0;

                    /*
                        Compute pooling region for this output unit
                        so we can compute its upper left and lower right coordinates.
                    */
                    vx_int32 hstart = (vx_int32)(floor((vx_float32)(ph) * roi_size_scale_h));
                    vx_int32 wstart = (vx_int32)(floor((vx_float32)(pw) * roi_size_scale_w));
                    vx_int32 hend = (vx_int32)(ceil((vx_float32)(ph + 1) * roi_size_scale_h));
                    vx_int32 wend = (vx_int32)(ceil((vx_float32)(pw + 1) * roi_size_scale_w));
                    hstart = gcmMIN(gcmMAX(hstart + roi_start_h, 0), height);
                    hend = gcmMIN(gcmMAX(hend + roi_start_h, 0), height);
                    wstart = gcmMIN(gcmMAX(wstart + roi_start_w, 0), width);
                    wend = gcmMIN(gcmMAX(wend + roi_start_w, 0), width);

                    pool_index = ph * pooled_width + pw;

                    /* remove some rectangles that do not meet the requirements */
                    is_empty = (vx_bool)((hend <= hstart) || (wend <= wstart));
                    if(is_empty)
                    {
                        output_data_v = 0;
                    }
                    else
                    {
                        /* find the max value in the current pooling region */
                        for(h = hstart; h < hend; h++)
                        {
                            for(w = wstart; w < wend; w++)
                            {
                                const vx_int32 index = h * width + w;
                                vx_float32 batch_data_v = (vx_float32)vxnneGetData(in_data_format, index, batch_data, in_data_fp);

                                if (batch_data_v > output_data_v)
                                    output_data_v = batch_data_v;
                            }
                        }
                    }

                    /* Save the max value to the output */
                    vxnneSaveData(out_format, pool_index, output_data_v, output_data_ptr, out_fp, out_rMode);
                }
            }

            /* Increment all data pointers by one channel*/
            batch_data      += width * height * in_data_items;
            output_data_ptr += pooled_width * pooled_height * out_items;
        }

        /* Increment ROI data pointer */
        if (stride_w == 5)
            rois_data_ptr += 5 * in_roi_items;
        else
            rois_data_ptr += 4 * in_roi_items;
    }

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNROIPoolLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status  status                     = VX_SUCCESS;
    vx_context context                    = vxGetContext((vx_reference)node);

    vx_tensor  input_data                 = (vx_tensor)parameters[0];
    vx_tensor  input_rois                 = (vx_tensor)parameters[1];
    vx_scalar  pool_types                 = (vx_scalar)parameters[2];
    vx_scalar  spatial_scales             = (vx_scalar)parameters[3];
    vx_scalar  pooled_heights             = (vx_scalar)parameters[4];
    vx_scalar  pooled_widths              = (vx_scalar)parameters[5];
    vx_tensor  outputs                    = (vx_tensor)parameters[6];

    vx_float32 spatial_scale              = spatial_scales->value->f32;
    vx_uint32  pool_width                 = pooled_widths->value->u32;
    vx_uint32  pool_height                = pooled_heights->value->u32;
    vx_bool    shExe_flag                 = vx_true_e;
    vx_uint32  width                      = TENSOR_VIEW_SIZE_INDEX(input_data, 0);
    vx_uint32  height                     = TENSOR_VIEW_SIZE_INDEX(input_data, 1);
    vx_uint32  depth                      = TENSOR_VIEW_SIZE_INDEX(input_data, 2);
    vx_uint32  roi_stride                 = TENSOR_VIEW_SIZE_INDEX(input_rois, 2);
    vx_uint32  rois_num                   = TENSOR_VIEW_SIZE_INDEX(input_rois, 3);
    vx_enum    inputFormat                = input_data->tensorBuffer->dataFormat;
    vx_enum    roisFormat                 = input_rois->tensorBuffer->dataFormat;
    vx_enum    outputFormat               = outputs->tensorBuffer->dataFormat;
    vxnne_tensor_roipool_layer roipoolLayer;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    {
        roipoolLayer = NULL;
        gcoOS_Allocate(gcvNULL, sizeof(vxnne_tensor_roipool_layer_s), (gctPOINTER*)&roipoolLayer);
        if (!roipoolLayer)
        {
            status = VX_ERROR_NO_MEMORY;
            gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            return status;
        }


        gcoOS_ZeroMemory(roipoolLayer, sizeof(vxnne_tensor_roipool_layer_s));

        vxnneLayer_Initialize(&roipoolLayer->base,
            "ROIPoolLayer",
            node,
            roipoolLayer->operations,
            VX_NULL);

        if (context->nnConfig.tpCoreCount && context->options.enableTP && context->options.flagTPFunc[TP_ROI_POOLING])
        {
            vx_uint32 num, size, maxpool, poolx, pooly, poolz;
            vx_tp_value_cmd values;
            vx_tensor tmpTensor = VX_NULL;
            vx_array list = roipoolLayer->roipool_tp_operation.buffer;

            vxnneOperation_Initialize(&roipoolLayer->roipool_tp_operation.base,
                                      &roipoolLayer->base,
                                      VXNNE_OPERATION_TARGET_TP,
                                      VXNNE_OPERATOR_ROIPOOL,
                                      vxnneExecuteTPROIPooling,
                                      vxnneOperation_TP_Deinitialize);

            /* create cmd buffer for TP operation */
            roipoolLayer->base.cmdTPBuff = vxCreateArray(context, VX_TYPE_CHAR, TP_COMMAND_SIZE * 2);
            if (!vxoArray_AllocateMemory(roipoolLayer->base.cmdTPBuff))
            {
                status = VX_ERROR_NO_MEMORY;
                gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
                goto exit;
            }

            /* Prepare ROI list. */
            num = TENSOR_VIEW_SIZE_INDEX(outputs, 3) * sizeof(vx_tp_roi_pool) / sizeof(vx_uint32);
            list = vxCreateArray(context, VX_TYPE_UINT32, num);
            if (!vxoArray_AllocateMemory(list))
            {
                status = VX_ERROR_NO_MEMORY;
                gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
                goto exit;
            }
            list->itemCount = num;
            list->base.isStage = vx_true_e;

            /* Prepare ROI intermediate output buffer. */
            maxpool = (TENSOR_VIEW_SIZE_INDEX(input_data, 0) + pool_width - 1) / pool_width;
            poolx = 1 << (vx_uint32) ceil(log(TENSOR_VIEW_SIZE_INDEX(input_data, 0)) / log(2));
            pooly = 1 << (vx_uint32) ceil(log(TENSOR_VIEW_SIZE_INDEX(input_data, 1)) / log(2));
            poolz = 1 << (vx_uint32) ceil(log(TENSOR_VIEW_SIZE_INDEX(input_data, 2)) / log(2));
            size = poolx * pooly * poolz * maxpool;
            tmpTensor = vxoTensor_CreateTensor(node->base.context, 1, &size, TENSOR_DATA_TYPE(input_data), vx_false_e);
            if (vxoTensor_AllocateMemory(tmpTensor) != VX_SUCCESS)
            {
                gcmPRINT("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
                status = VX_ERROR_NO_MEMORY;
                goto exit;
            }

            values.u32[0] = pool_height;
            values.u32[1] = pool_width;
            values.f32[0] = spatial_scale;
            values.u32[2] = maxpool;
            values.u32[3] = poolx;
            values.u32[4] = pooly;
            values.u32[5] = poolz;
            values.u32[6] = TENSOR_PHYSICAL_ADDR(tmpTensor);

            values.e32[0] = 0;
            fillInCmdTPBuffer(
                    input_data, (vx_reference)input_rois, outputs,
                    roipoolLayer->base.cmdTPBuff, VX_NULL,
                    VX_NULL, &values,
                    TP_ROI_POOLING,
                    0, vx_false_e,
                    vx_true_e);

            values.e32[0] = 1;
            fillInCmdTPBuffer(
                    input_data, (vx_reference)input_rois, outputs,
                    roipoolLayer->base.cmdTPBuff, list,
                    VX_NULL, &values,
                    TP_ROI_POOLING,
                    1, vx_false_e,
                    vx_true_e);

            roipoolLayer->base.num_operations = 1;
            roipoolLayer->operations[0] = (vxnne_operation)&roipoolLayer->roipool_tp_operation;
            roipoolLayer->roipool_tp_operation.inputs[0]    = input_data;
            roipoolLayer->roipool_tp_operation.outputs[0]   = tmpTensor;
            roipoolLayer->roipool_tp_operation.inputs[1]    = tmpTensor;
            roipoolLayer->roipool_tp_operation.buffer       = list;
            roipoolLayer->roipool_tp_operation.outputs[1]   = outputs;
            roipoolLayer->roipool_tp_operation.op_num       = 2;
            roipoolLayer->roipool_tp_operation.multi_tensor = vx_true_e;

            roipoolLayer->base.num_temp_tensors = 1;
            roipoolLayer->base.temp_tensors[0] = tmpTensor;
            node->layer = &roipoolLayer->base;
        }
        else
        {
            shExe_flag = (vx_bool)(width == 20 && height == 16 && roi_stride == 5 && pool_width == 6 && pool_height == 6 && inputFormat == VX_TYPE_FLOAT16 && roisFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16);
            if (shExe_flag)
            {
                vxnne_shader_executable shaderExecutable;
                vx_uint32 imgNum                = 4;
                vx_uint32 dims                  = 3;
                vx_uint32 tmp_sizes0[3]         = {width, height, depth * imgNum };
                vx_uint32 tmp_sizes1[3]         = {84, rois_num, 1 };
                vx_uint32 outputs_dims            = outputs->dimCount;
                vx_tensor outputs_reshp            = NULL;
                vx_tensor vertMaxPoolTensor     = vxoTensor_CreateTensor(node->base.context, dims, tmp_sizes0, TENSOR_DATA_TYPE(input_data), vx_false_e);
                vx_tensor preTreatedRectTensor  = vxoTensor_CreateTensor(node->base.context, dims, tmp_sizes1, TENSOR_DATA_TYPE(input_rois), vx_false_e);

                roipoolLayer->base.num_temp_tensors                  = 2;
                roipoolLayer->base.temp_tensors[0] = vertMaxPoolTensor;
                roipoolLayer->base.temp_tensors[1] = preTreatedRectTensor;

                //operation1:vertMaxPool
                shaderExecutable = vxnneVertMaxPoolShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_VERTMAXPOOL, &node->kernelAttributes.borderMode, input_data, pool_width, pool_height, vertMaxPoolTensor);

                if (!shaderExecutable)
                {
                    status = VX_FAILURE;
                    goto exit;
                }
                status = vxnneShaderOperation_Initialize(&roipoolLayer->vertmaxpool_operation.vertmaxpool_SHoperation,
                    &roipoolLayer->base,
                    VXNNE_OPERATOR_VERTMAXPOOL,
                    shaderExecutable);

                if (status != VX_SUCCESS)
                    goto exit;

                //operation2:preTreatedRect
                shaderExecutable = vxnnePreTreatedRectShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_PRETREATEDRECT, &node->kernelAttributes.borderMode, input_rois, roi_stride, rois_num, width, height, spatial_scale, preTreatedRectTensor);

                if (!shaderExecutable)
                {
                    status = VX_FAILURE;
                    goto exit;
                }
                status = vxnneShaderOperation_Initialize(&roipoolLayer->pretreatedrect_operation.pretreatedrect_SHoperation,
                    &roipoolLayer->base,
                    VXNNE_OPERATOR_PRETREATEDRECT,
                    shaderExecutable);

                if (status != VX_SUCCESS)
                    goto exit;

                //operation3:horzMaxPool
                if(outputs_dims == 4)
                {
                    vx_int32 new_size[3] = {pool_width * pool_height, depth, rois_num};
                    outputs_dims = 3;
                    outputs_reshp = vxReshapeTensor(outputs, new_size, outputs_dims);
                }

                if(outputs_reshp)
                {
                    shaderExecutable = vxnneHorzMaxPoolShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_HORZMAXPOOL, &node->kernelAttributes.borderMode, vertMaxPoolTensor, preTreatedRectTensor, outputs_reshp);
                    vxReleaseTensor(&outputs_reshp);
                }
                else
                {
                    shaderExecutable = vxnneHorzMaxPoolShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_HORZMAXPOOL, &node->kernelAttributes.borderMode, vertMaxPoolTensor, preTreatedRectTensor, outputs);
                }

                if (!shaderExecutable)
                {
                    status = VX_FAILURE;
                    goto exit;
                }
                status = vxnneShaderOperation_Initialize(&roipoolLayer->horzmaxpool_operation.horzmaxpool_SHoperation,
                    &roipoolLayer->base,
                    VXNNE_OPERATOR_HORZMAXPOOL,
                    shaderExecutable);

                if (status != VX_SUCCESS)
                    goto exit;


                roipoolLayer->base.num_operations = 3;
                roipoolLayer->operations[0] = &roipoolLayer->vertmaxpool_operation.vertmaxpool_SHoperation.base;
                roipoolLayer->operations[1] = &roipoolLayer->pretreatedrect_operation.pretreatedrect_SHoperation.base;
                roipoolLayer->operations[2] = &roipoolLayer->horzmaxpool_operation.horzmaxpool_SHoperation.base;

                node->layer = &roipoolLayer->base;
            }
            else
            {

                vxnneOperation_Initialize(&roipoolLayer->tensorROIPoolSW.base,
                    &roipoolLayer->base,
                    VXNNE_OPERATION_TARGET_SW,
                    VXNNE_OPERATOR_ROIPOOL,
                    vxnneExecuteSWROIPooling,
                    VX_NULL);

                roipoolLayer->base.num_operations    = 1;
                roipoolLayer->operations[0] = (vxnne_operation)&roipoolLayer->tensorROIPoolSW.base;

                roipoolLayer->tensorROIPoolSW.input_data      = input_data;
                roipoolLayer->tensorROIPoolSW.input_rois      = input_rois;
                roipoolLayer->tensorROIPoolSW.pool_type       = pool_types;
                roipoolLayer->tensorROIPoolSW.pooled_height   = pooled_heights;
                roipoolLayer->tensorROIPoolSW.pooled_width    = pooled_widths;
                roipoolLayer->tensorROIPoolSW.spatial_scale   = spatial_scales;
                roipoolLayer->tensorROIPoolSW.output          = outputs;

                node->layer = &roipoolLayer->base;
            }
        }
    }

    return status;

exit:
    if (roipoolLayer) gcoOS_Free(gcvNULL, (gctPOINTER)roipoolLayer);
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

vx_status vxnneExecuteSWConvolution(vxnne_operation operation)
{
    vxnne_convolution_operation convolutionOperation   = (vxnne_convolution_operation)operation;

    vx_tensor inputs                = convolutionOperation->inputs;
    vx_tensor weights               = convolutionOperation->weights;
    vx_tensor biases                = convolutionOperation->biases;
    vx_scalar padX                  = convolutionOperation->padX;
    vx_scalar padY                  = convolutionOperation->padY;
    vx_scalar downScaleSizeRounding = convolutionOperation->downScaleSizeRounding;
    vx_tensor outputs               = convolutionOperation->outputs;

    vx_uint32 batch = 1;
    void * inputBaseLogical;
    void * outputBaseLogical;

    void *weightsBaseLogical;
    void *biasesBaseLogical;

    vx_uint32 inputWidth, inputHeight, inputDepth, outputWidth, outputHeight, outputDepth;
    vx_uint32 kernelXYSize, stride;
    vx_uint32 k, p, j, i;
    vx_uint8_ptr dataSrc;
    vx_uint8_ptr dataDst;
    vx_uint8_ptr dataWeight;
    vx_uint8_ptr dataBias;
    vx_type_e inputFormat;
    vx_type_e weightFormat;
    vx_type_e biasFormat;
    vx_type_e outputFormat;

    vx_enum downScaleSizeRoundingValue = downScaleSizeRounding->value->e;
    vx_uint32 padXLeft;
    vx_uint32 padXRight;
    vx_uint32 padYTop;
    vx_uint32 padYBottom;

    vxnneGetPadValue(padX->value->n32, padY->value->n32, &padXLeft, &padXRight, &padYTop, &padYBottom);

    batch = (TENSOR_SIZE_INDEX(inputs, 3) == 0) ? 1 : TENSOR_SIZE_INDEX(inputs, 3);

    vxoTensor_GetTensorViewMemory(inputs, &inputBaseLogical, VX_NULL);
    vxoTensor_GetTensorViewMemory(weights, &weightsBaseLogical, VX_NULL);
    vxoTensor_GetTensorViewMemory(biases, &biasesBaseLogical, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, &outputBaseLogical, VX_NULL);

    dataSrc      = (vx_uint8_ptr)inputBaseLogical;
    dataDst      = (vx_uint8_ptr)outputBaseLogical;
    dataWeight   = (vx_uint8_ptr)weightsBaseLogical;
    dataBias     = (vx_uint8_ptr)biasesBaseLogical;
    inputFormat  = (vx_type_e)(inputs->tensorBuffer->dataFormat);
    weightFormat = (vx_type_e)(weights->tensorBuffer->dataFormat);
    biasFormat   = (vx_type_e)(biases->tensorBuffer->dataFormat);
    outputFormat = (vx_type_e)(outputs->tensorBuffer->dataFormat);

    inputWidth   = TENSOR_SIZE_INDEX(inputs, 0);
    inputHeight  = TENSOR_SIZE_INDEX(inputs, 1);
    inputDepth   = TENSOR_SIZE_INDEX(inputs, 2);
    outputWidth  = TENSOR_SIZE_INDEX(outputs, 0);
    outputHeight = TENSOR_SIZE_INDEX(outputs, 1);
    outputDepth  = TENSOR_SIZE_INDEX(outputs, 2);

    kernelXYSize = TENSOR_SIZE_INDEX(weights, 0);

    /* Calculate stride = (w + padXLeft + padXRight - weight)/(output_w - 1) */
    stride = vxoNNExternsionConvlutionRound((vx_float32)(inputWidth + padXLeft + padXRight - kernelXYSize) / (outputWidth - 1), downScaleSizeRoundingValue);

    gcoOS_MemFill(outputBaseLogical, 0, outputWidth * outputHeight * outputDepth * vxnneGetTypeSize(outputFormat));

    for (k = 0; k < batch; k++)
    {
        dataSrc    = (vx_uint8_ptr)inputBaseLogical + k * inputWidth * inputHeight * inputDepth * vxnneGetTypeSize(inputFormat);
        dataWeight = (vx_uint8_ptr)weightsBaseLogical;
        dataDst    = (vx_uint8_ptr)outputBaseLogical + k * outputWidth * outputHeight * outputDepth * vxnneGetTypeSize(outputFormat);

        for (p = 0; p < outputDepth; p ++)
        {
            for (j = 0; j < outputHeight; j ++)
            {
                for (i = 0; i < outputWidth; i ++)
                {
                    vx_int32 hStart = j * stride - padYTop;
                    vx_int32 wStart = i * stride - padXLeft;
                    vx_int32 hEnd = gcmMIN(hStart + kernelXYSize, inputHeight);
                    vx_int32 wEnd = gcmMIN(wStart + kernelXYSize, inputWidth);
                    vx_int32 indexOut = 0;
                    vx_int32 indexBias = 0;
                    vx_int32 h, w = 0;
                    vx_int32 m, n = 0;
                    vx_uint32 d;
                    vx_float32 sum = 0;
                    vx_uint32 kernelXStart, kernelYStart;

                    kernelYStart = hStart < 0 ? padYTop : 0;
                    kernelXStart = wStart < 0 ? padXLeft : 0;
                    hStart = gcmMAX(hStart, 0);
                    wStart = gcmMAX(wStart, 0);

                    indexOut = j * (outputWidth) + i;

                    for (d = 0; d < inputDepth; d++)
                    {
                        for (h = hStart, n = kernelYStart; h < hEnd; ++ h, n++)
                        {
                            for (w = wStart, m = kernelXStart; w < wEnd; ++ w, m++)
                            {
                                const vx_int32 indexSrc = d * inputWidth * inputHeight + h * (inputWidth) + w;
                                const vx_int32 indexWeight = d * kernelXYSize * kernelXYSize + n * kernelXYSize + m;
                                sum += vxnneGetData(inputFormat, indexSrc, (vx_uint8_ptr)dataSrc, inputs->tensorBuffer->fixedPointPos) *
                                       vxnneGetData(weightFormat, indexWeight, (vx_uint8_ptr)dataWeight, weights->tensorBuffer->fixedPointPos);
                            }
                        }
                    }

                    indexBias = p;

                    if (biasFormat == VX_TYPE_FLOAT32 || biasFormat == VX_TYPE_INT32)
                    {
                        sum += vxnneGetData(biasFormat, indexBias, (vx_uint8_ptr)dataBias, biases->tensorBuffer->fixedPointPos);
                    }
                    else
                    {
                        printf("can't support this bias data format\n");
                        gcmASSERT(0);
                    }

                    vxnneSaveData(outputFormat, indexOut, sum, dataDst, outputs->tensorBuffer->fixedPointPos, outputs->tensorBuffer->roundingMode);
                }
            }

            dataWeight += kernelXYSize * kernelXYSize * inputDepth * vxnneGetTypeSize(weightFormat);
            dataDst += outputWidth * outputHeight * vxnneGetTypeSize(outputFormat);
        }
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNConvolutionLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    vx_tensor inputs                = (vx_tensor)parameters[0];
    vx_tensor weights               = (vx_tensor)parameters[1];
    vx_tensor biases                = (vx_tensor)parameters[2];
    vx_scalar padX                  = (vx_scalar)parameters[3];
    vx_scalar padY                  = (vx_scalar)parameters[4];
    vx_scalar downScaleSizeRounding = (vx_scalar)parameters[5];
    vx_tensor outputs               = (vx_tensor)parameters[6];

    vxnne_convolution_layer convolutionLayer = VX_NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_convolution_layer_s), (gctPOINTER*)&convolutionLayer);
    if (!convolutionLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(convolutionLayer, sizeof(vxnne_convolution_layer_s));

    vxnneLayer_Initialize(&convolutionLayer->base, "ConvolutionLayer", node, convolutionLayer->operations, VX_NULL);

    vxnneOperation_Initialize(&convolutionLayer->convolutionSW.base,
                            &convolutionLayer->base,
                            VXNNE_OPERATION_TARGET_SW,
                            VXNNE_OPERATOR_CONVOLUTION,
                            vxnneExecuteSWConvolution,
                            VX_NULL);

    convolutionLayer->base.num_operations = 1;
    convolutionLayer->operations[0] = &convolutionLayer->convolutionSW.base;

    convolutionLayer->convolutionSW.inputs                = inputs;
    convolutionLayer->convolutionSW.weights               = weights;
    convolutionLayer->convolutionSW.biases                = biases;
    convolutionLayer->convolutionSW.padX                  = padX;
    convolutionLayer->convolutionSW.padY                  = padY;
    convolutionLayer->convolutionSW.downScaleSizeRounding = downScaleSizeRounding;
    convolutionLayer->convolutionSW.outputs               = outputs;


    node->layer = &convolutionLayer->base;

exit:
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

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
    vx_type_e  inputFormat      = (vx_type_e)inputs->tensorBuffer->dataFormat;
    vx_type_e  outputFormat     = (vx_type_e)outputs->tensorBuffer->dataFormat;
    vx_uint8   inputFixedPoint  = inputs->tensorBuffer->fixedPointPos;
    vx_uint8   outputFixedPoint = outputs->tensorBuffer->fixedPointPos;

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

    if ((inputs->tensorBuffer->dataFormat != VX_TYPE_FLOAT16 && inputs->tensorBuffer->dataFormat != VX_TYPE_FLOAT32 && inputs->tensorBuffer->dataFormat != VX_TYPE_INT8)
        || (outputs->tensorBuffer->dataFormat != VX_TYPE_FLOAT16 && outputs->tensorBuffer->dataFormat != VX_TYPE_FLOAT32 && outputs->tensorBuffer->dataFormat != VX_TYPE_INT8))
    {
        gcmPRINT("input or outputs format is not support");
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

                    data = vxnneGetData(inputFormat, out_index, (vx_uint8_ptr)inputBase, inputFixedPoint);
                    vxnneSaveData(outputFormat, in_index, data, (vx_uint8_ptr)outputBase, outputFixedPoint, outputs->tensorBuffer->roundingMode);
                }
            }
        }
    }

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNReorgLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNReorgLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNReorgLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNReorgLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_scalar  stride_s                   = (vx_scalar)parameters[1];
    vx_tensor  outputs                    = (vx_tensor)parameters[2];
    vx_uint32  stride                     = stride_s->value->u32;
    vx_enum    inputFormat                = inputs->tensorBuffer->dataFormat;
    vx_enum    outputFormat               = outputs->tensorBuffer->dataFormat;
    vx_uint32  input_depth                = TENSOR_SIZE_INDEX(inputs, 2);

    vxnne_reorg_layer  reorgLayer         = VX_NULL;
    vx_bool            dataFormat_flag    = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8));
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
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(reorgLayer, sizeof(vxnne_reorg_layer_s));

    vxnneLayer_Initialize(&reorgLayer->base,
                          "ReorgLayer",
                          node,
                          reorgLayer->operations,
                          VX_NULL);
    if (stride == 2 && dataFormat_flag)
    {
        vx_scalar outc_s   = vxCreateScalar(node->base.context, VX_TYPE_UINT32, &input_depth);

        vxnne_shader_executable shaderExecutable;

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
                                        shaderExecutable);

        if (status != VX_SUCCESS)
        {
            if (!outc_s) vxReleaseScalar(&outc_s);
            goto exit;
        }

        reorgLayer->base.num_operations = 1;
        reorgLayer->operations[0] = &reorgLayer->reorg_sh_operation.base;

    }
    else
    {
        vxnneOperation_Initialize(&reorgLayer->reorg_sw_operation.base,
                                  &reorgLayer->base,
                                  VXNNE_OPERATION_TARGET_SW,
                                  VXNNE_OPERATOR_REORG,
                                  vxnneExecuteSWReorg,
                                  VX_NULL);

        reorgLayer->base.num_operations = 1;
        reorgLayer->operations[0] = (vxnne_operation)&reorgLayer->reorg_sw_operation;

        reorgLayer->reorg_sw_operation.inputs           = inputs;
        reorgLayer->reorg_sw_operation.stride           = stride_s;
        reorgLayer->reorg_sw_operation.outputs          = outputs;
    }

    node->layer = &reorgLayer->base;
    return status;

exit:
    if (reorgLayer) gcoOS_Free(gcvNULL, (gctPOINTER)reorgLayer);
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNReorgLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}


/***************************************************************************************************************************
 *                                                 DeConvolution
 ***************************************************************************************************************************/
VX_PRIVATE_API void vxnneLayerSW_gemm_nn(vx_type_e A_format, vx_type_e B_format, vx_type_e C_format, vx_int32 roundMode, vx_uint8 fixedPointPos,
        vx_int32 M, vx_int32 N, vx_int32 K, vx_float32 ALPHA,
        vx_uint8_ptr A, vx_int32 lda,
        vx_uint8_ptr B, vx_int32 ldb,
        vx_uint8_ptr C, vx_int32 ldc)
{
    vx_int32 i,j,k;
    for(i = 0; i < M; ++i){
        for(k = 0; k < K; ++k){
            /*register float A_PART = ALPHA*A[i*lda+k];*/
            register float A_PART = ALPHA*vxnneGetData(A_format, i*lda+k, A, fixedPointPos);
            for(j = 0; j < N; ++j){
                /*C[i*ldc+j] += A_PART*B[k*ldb+j];*/
                vxnneSaveData(C_format, i*ldc+j, vxnneGetData(C_format, i*ldc+j, C, fixedPointPos) + A_PART*vxnneGetData(B_format, k*ldb+j, B, fixedPointPos), C, fixedPointPos, roundMode);
            }
        }
    }
}

VX_PRIVATE_API void vxnneLayerSW_gemm_nt(vx_type_e A_format, vx_type_e B_format, vx_type_e C_format, vx_int32 roundMode, vx_uint8 fixedPointPos,
        vx_int32 M, vx_int32 N, vx_int32 K, vx_float32 ALPHA,
        vx_uint8_ptr A, vx_int32 lda,
        vx_uint8_ptr B, vx_int32 ldb,
        vx_uint8_ptr C, vx_int32 ldc)
{
    vx_int32 i,j,k;
    for(i = 0; i < M; ++i){
        for(j = 0; j < N; ++j){
            register float sum = 0;
            for(k = 0; k < K; ++k){
                /*sum += ALPHA*A[i*lda+k]*B[j*ldb + k];*/
                sum += ALPHA * vxnneGetData(A_format, i*lda+k, A, fixedPointPos) * vxnneGetData(B_format, j*ldb + k, B, fixedPointPos);
            }
            /*C[i*ldc+j] += sum;*/
            vxnneSaveData(C_format, i*ldc+j, vxnneGetData(C_format, i*ldc+j, C, fixedPointPos) + sum, C, fixedPointPos, roundMode);
        }
    }
}

VX_PRIVATE_API void vxnneLayerSW_gemm_tn(vx_type_e A_format, vx_type_e B_format, vx_type_e C_format, vx_int32 roundMode, vx_uint8 fixedPointPos,
        vx_int32 M, vx_int32 N, vx_int32 K, vx_float32 ALPHA,
        vx_uint8_ptr A, vx_int32 lda,
        vx_uint8_ptr B, vx_int32 ldb,
        vx_uint8_ptr C, vx_int32 ldc)
{
    vx_int32 i,j,k;
    for(i = 0; i < M; ++i){
        for(k = 0; k < K; ++k){
            /*register float A_PART = ALPHA*A[k*lda+i];*/
            register float A_PART = ALPHA*vxnneGetData(A_format, k*lda+i, A, fixedPointPos);
            for(j = 0; j < N; ++j){
                /*C[i*ldc+j] += A_PART*B[k*ldb+j];*/
                vxnneSaveData(C_format, i*ldc+j, vxnneGetData(C_format, i*ldc+j, C, fixedPointPos) + A_PART*vxnneGetData(B_format, k*ldb+j, B, fixedPointPos), C, fixedPointPos, roundMode);
            }
        }
    }
}

VX_PRIVATE_API void vxnneLayerSW_gemm_tt(vx_type_e A_format, vx_type_e B_format, vx_type_e C_format, vx_int32 roundMode, vx_uint8 fixedPointPos,
        vx_int32 M, vx_int32 N, vx_int32 K, vx_float32 ALPHA,
        vx_uint8_ptr A, vx_int32 lda,
        vx_uint8_ptr B, vx_int32 ldb,
        vx_uint8_ptr C, vx_int32 ldc)
{
    vx_int32 i,j,k;
    for(i = 0; i < M; ++i){
        for(j = 0; j < N; ++j){
            register float sum = 0;
            for(k = 0; k < K; ++k){
                /*sum += ALPHA*A[i+k*lda]*B[k+j*ldb];*/
                sum += ALPHA * vxnneGetData(A_format, i+k*lda, A, fixedPointPos) * vxnneGetData(B_format, k+j*ldb, B, fixedPointPos);
            }
            /*C[i*ldc+j] += sum;*/
            vxnneSaveData(C_format, i*ldc+j, sum, C, fixedPointPos, roundMode);
        }
    }
}


VX_PRIVATE_API void vxnneLayerSW_gemm(vx_type_e A_format, vx_type_e B_format, vx_type_e C_format, vx_int32 roundMode, vx_uint8 fixedPointPos,
        vx_int32 TA, vx_int32 TB, vx_int32 M, vx_int32 N, vx_int32 K, vx_float32 ALPHA,
        vx_uint8_ptr A, vx_int32 lda,
        vx_uint8_ptr B, vx_int32 ldb,
        vx_float32 BETA,
        vx_uint8_ptr C, vx_int32 ldc)
{
    /*printf("cpu: %d %d %d %d %d %f %d %d %f %d\n",TA, TB, M, N, K, ALPHA, lda, ldb, BETA, ldc);*/
    int i, j;
    for(i = 0; i < M; ++i){
        for(j = 0; j < N; ++j){
            /*C[i*ldc + j] *= BETA;*/
            vxnneSaveData(C_format, i*ldc + j, BETA * vxnneGetData(C_format, i*ldc + j, C, fixedPointPos), C, fixedPointPos, roundMode);
        }
    }
    if(!TA && !TB)
        vxnneLayerSW_gemm_nn(A_format, B_format, C_format, roundMode, fixedPointPos, M, N, K, ALPHA,A,lda, B, ldb, C,ldc);
    else if(TA && !TB)
        vxnneLayerSW_gemm_tn(A_format, B_format, C_format, roundMode, fixedPointPos, M, N, K, ALPHA,A,lda, B, ldb, C,ldc);
    else if(!TA && TB)
        vxnneLayerSW_gemm_nt(A_format, B_format, C_format, roundMode, fixedPointPos, M, N, K, ALPHA,A,lda, B, ldb, C,ldc);
    else
        vxnneLayerSW_gemm_tt(A_format, B_format, C_format, roundMode, fixedPointPos, M, N, K, ALPHA,A,lda, B, ldb, C,ldc);
}

VX_PRIVATE_API vx_bool vxnneLayerSW_is_a_ge_zero_and_a_lt_b(vx_int32 a, vx_int32 b) {
  return (((vx_uint32)a) < (vx_uint32)(b))?vx_true_e:vx_false_e;
}

VX_PRIVATE_API void vxnneLayerSW_col2im(vx_type_e col_format, vx_type_e im_format, vx_int32 roundMode, vx_uint8 fixedPointPos,
    vx_uint8_ptr data_col, const vx_int32 channels,
    const vx_int32 height, const vx_int32 width, const vx_int32 kernel_h, const vx_int32 kernel_w,
    const vx_int32 pad_h, const vx_int32 pad_w,
    const vx_int32 stride_h, const vx_int32 stride_w,
    const vx_int32 dilation_h, const vx_int32 dilation_w,
    vx_uint8_ptr data_im) {
    vx_int32 channel = 0, kernel_row = 0, kernel_col = 0, input_row = 0, output_rows = 0, input_col = 0;
    vx_int32 output_col = 0;

    const vx_int32 output_h = (height + 2 * pad_h - (dilation_h * (kernel_h - 1) + 1)) / stride_h + 1;
    const vx_int32 output_w = (width + 2 * pad_w - (dilation_w * (kernel_w - 1) + 1)) / stride_w + 1;
    const vx_int32 channel_size = height * width;
    for (channel = channels; channel--; data_im += vxnneGetTypeSize(im_format) *channel_size) {
        for (kernel_row = 0; kernel_row < kernel_h; kernel_row++) {
            for (kernel_col = 0; kernel_col < kernel_w; kernel_col++) {
                input_row = -pad_h + kernel_row * dilation_h;
                for (output_rows = output_h; output_rows; output_rows--) {
                    if (!vxnneLayerSW_is_a_ge_zero_and_a_lt_b(input_row, height)) {
                        /*data_col += output_w;*/
                        data_col += vxnneGetTypeSize(col_format) * output_w;
                    } else {
                          input_col = -pad_w + kernel_col * dilation_w;
                          for (output_col = output_w; output_col; output_col--) {
                              if (vxnneLayerSW_is_a_ge_zero_and_a_lt_b(input_col, width)) {
                                  /*data_im[input_row * width + input_col] += *data_col;*/
                                  vx_int32 idx = input_row * width + input_col;
                                  vx_float64 val1 = vxnneGetData(col_format, 0, data_col, fixedPointPos);
                                  vx_float64 val2 =  vxnneGetData(im_format, input_row * width + input_col, data_im, fixedPointPos);
                                  vx_float64 val = val1 + val2;
                                  vxnneSaveData(im_format, idx, val, data_im, fixedPointPos, roundMode);
                              }
                              /*data_col++;*/
                              data_col += vxnneGetTypeSize(col_format);

                              input_col += stride_w;
                          }
                    }
                    input_row += stride_h;
                }
            }
        }
    }
}

VX_PRIVATE_API void vxnneLayerSW_add_bias(vx_type_e output_format, vx_type_e bias_format, vx_int32 roundMode, vx_uint8 fixedPointPos,
                vx_uint8_ptr output, vx_uint8_ptr biases, vx_int32 batch, vx_int32 n, vx_int32 size)
{
    vx_int32 i,j,b;
    for(b = 0; b < batch; ++b){
        for(i = 0; i < n; ++i){
            for(j = 0; j < size; ++j){
                /*output[(b*n + i)*size + j] += biases[i];*/
                vxnneSaveData(output_format, (b*n + i)*size + j, vxnneGetData(bias_format, i, biases, fixedPointPos) + vxnneGetData(output_format, (b*n + i)*size + j, output, fixedPointPos), output, fixedPointPos, roundMode);
            }
        }
    }
}

#define INDEX_WIDTH 0
#define INDEX_HEIGHT 1
#define INDEX_DEPTH 2
#define INDEX_BATCH 3

VX_PRIVATE_API vx_status vxnneGetTensorMemeory(vx_tensor tensor, vx_ptr_ptr ptr, vx_bool stage, vx_bool zero)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 tensor_size = 0;

    gcmASSERT(tensor != VX_NULL);

    vxoTensor_GetTensorSize(tensor, &tensor_size);

    if (stage)
    {
        vx_ptr data = VX_NULL;

        vxoTensor_GetTensorViewMemory(tensor, &data, VX_NULL);
        *ptr = vxAllocate(tensor_size);
        gcoOS_MemCopy(*ptr, data, tensor_size);
    }
    else
        vxoTensor_GetTensorViewMemory(tensor, (vx_ptr_ptr)ptr, VX_NULL);

    if (zero)
        gcoOS_MemFill(*ptr, 0, tensor_size);

    return status;
}

VX_PRIVATE_API vx_status vxnneExecuteSWDeConvolution(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_deconvolution_operation deconvOperation   = (vxnne_deconvolution_operation)operation;

    vx_tensor inputs        = deconvOperation->inputs;
    vx_tensor weights       = deconvOperation->weights;
    vx_tensor bias          = (deconvOperation->biases != VX_NULL)?deconvOperation->biases:VX_NULL;
    vx_tensor outputs       = deconvOperation->outputs;
    vx_int32 padding_x      = deconvOperation->padding_x->value->n32;
    vx_int32 padding_y      = deconvOperation->padding_y->value->n32;
    vx_enum rounding_policy = deconvOperation->rounding_policy->value->e;
    vx_int32 a_x            = deconvOperation->a_x->value->n32;
    vx_int32 a_y            = deconvOperation->a_y->value->n32;

    vx_type_e input_format  = (vx_type_e)(inputs->tensorBuffer->dataFormat);
    vx_type_e output_format = (vx_type_e)(outputs->tensorBuffer->dataFormat);
    vx_type_e weight_format = (vx_type_e)(weights->tensorBuffer->dataFormat);

    vx_int32 group = (deconvOperation->group->value->n32 > 0)?deconvOperation->group->value->n32:1;
    vx_int32 g = 0;
    vx_int32 kernel_size_x = weights->viewRegion.viewEnds[INDEX_WIDTH] - weights->viewRegion.viewStarts[INDEX_WIDTH];
    vx_int32 kernel_size_y = weights->viewRegion.viewEnds[INDEX_HEIGHT] - weights->viewRegion.viewStarts[INDEX_HEIGHT];
    vx_int32 kernel_size_c = weights->viewRegion.viewEnds[INDEX_DEPTH] - weights->viewRegion.viewStarts[INDEX_DEPTH];
    vx_int32 in_h = inputs->viewRegion.viewEnds[INDEX_HEIGHT] - inputs->viewRegion.viewStarts[INDEX_HEIGHT];
    vx_int32 in_w = inputs->viewRegion.viewEnds[INDEX_WIDTH] - inputs->viewRegion.viewStarts[INDEX_WIDTH];
    vx_int32 out_h = outputs->viewRegion.viewEnds[INDEX_HEIGHT] - outputs->viewRegion.viewStarts[INDEX_HEIGHT];
    vx_int32 out_w = outputs->viewRegion.viewEnds[INDEX_WIDTH] - outputs->viewRegion.viewStarts[INDEX_WIDTH];
    vx_int32 slice_size = out_h * out_w;
    vx_int32 stride_w = (out_w - 1)/(in_w + 2 * padding_x - kernel_size_x);
    vx_int32 stride_h = (out_h - 1)/(in_h + 2 * padding_y - kernel_size_y);

    vx_int32 conv_in_channels = inputs->viewRegion.viewEnds[INDEX_DEPTH] - inputs->viewRegion.viewStarts[INDEX_DEPTH];
    vx_int32 conv_out_channels = outputs->viewRegion.viewEnds[INDEX_DEPTH] - outputs->viewRegion.viewStarts[INDEX_DEPTH];
    vx_int32 kernel_dim = kernel_size_x * kernel_size_y * kernel_size_c;
    vx_int32 conv_in_spatial_dim = in_h * in_w;

    vx_int32 conv_out_spatial_dim = out_h * out_w;
    vx_int32 input_offset = conv_in_channels * conv_in_spatial_dim / group;
    vx_int32 weight_offset = conv_out_channels * kernel_dim / group;
    vx_uint8_ptr col_ptr = (vx_uint8_ptr)malloc(sizeof(vx_float32) * kernel_dim * conv_out_spatial_dim * group);
    vx_uint8_ptr output_ptr = VX_NULL;
    vx_uint8_ptr weight_ptr = VX_NULL;
    vx_uint8_ptr input_ptr = VX_NULL/*(conv_out_channels/group)*height*width*/;

    vx_bool input_stage = vx_false_e, output_stage = vx_false_e;

    vxnneGetTensorMemeory(inputs, (vx_ptr_ptr)&input_ptr, input_stage, vx_false_e);
    vxnneGetTensorMemeory(weights, (vx_ptr_ptr)&weight_ptr, input_stage, vx_false_e);

    vxnneGetTensorMemeory(outputs, (vx_ptr_ptr)&output_ptr, output_stage, vx_true_e);

    gcoOS_MemFill(col_ptr, 0, sizeof(vx_float32) * kernel_dim * conv_out_spatial_dim * group);

    for(g = 0; g < group; ++g){
        vx_uint8_ptr g_col_ptr = sizeof(vx_float32) * kernel_dim * conv_out_spatial_dim * g + col_ptr;
        vx_uint8_ptr g_weight_ptr = weight_offset * vxnneGetTypeSize(weight_format) * g + weight_ptr;
        vx_uint8_ptr g_input_ptr =  input_offset * vxnneGetTypeSize(input_format) * g + input_ptr;
        vx_uint8_ptr g_output_ptr = conv_out_channels * conv_out_spatial_dim / group * vxnneGetTypeSize(output_format) * g + output_ptr;
#define CblasTrans 1
#define CblasNoTrans 0

        /* vxnneLayerSW_gemm(
                vx_int32 TA, vx_int32 TB,
                vx_int32 M, vx_int32 N, vx_int32 K, vx_float32 ALPHA,
                vx_uint8_ptr A, vx_int32 lda,
                vx_uint8_ptr B, vx_int32 ldb,
                vx_float32 BETA,
                vx_uint8_ptr C, vx_int32 ldc);

            CblasTrans  : 1;
            CblasNoTrans: 0;
            int lda = (TransA == CblasNoTrans) ? K : M;
            int ldb = (TransB == CblasNoTrans) ? N : K;


            vxnneLayerSW_col2im(
                vx_uint8_ptr data_col, const vx_int32 channels,
                const vx_int32 height, const vx_int32 width, const vx_int32 kernel_h, const vx_int32 kernel_w,
                const vx_int32 pad_h, const vx_int32 pad_w,
                const vx_int32 stride_h, const vx_int32 stride_w,
                const vx_int32 dilation_h, const vx_int32 dilation_w,
                vx_uint8_ptr data_im);

        */
        vxnneLayerSW_gemm(weight_format, input_format, VX_TYPE_FLOAT32, rounding_policy, 0,
            CblasTrans, CblasNoTrans,
            kernel_dim, conv_in_spatial_dim, (conv_out_channels/group), 1,
            g_weight_ptr, kernel_dim,
            g_input_ptr, conv_in_spatial_dim,
            0,
            g_col_ptr, conv_in_spatial_dim);

        vxnneLayerSW_col2im(VX_TYPE_FLOAT32, output_format, rounding_policy, 0,
            g_col_ptr, conv_out_channels/group,
            out_h, out_w, kernel_size_y, kernel_size_x,
            padding_y, padding_x,
            stride_h, stride_w,
            a_x, a_y,
            g_output_ptr);
    }
    if (bias)
    {
        vx_type_e bias_format  = (vx_type_e)(bias->tensorBuffer->dataFormat);

        vxnneLayerSW_add_bias(output_format, bias_format, rounding_policy, 0,
            outputs->tensorBuffer->memory.logicals[0], bias->tensorBuffer->memory.logicals[0],
            1, conv_out_channels, slice_size);
    }
    free(col_ptr);

    if (input_stage)
    {
        vxFree(input_ptr);
        vxFree(weight_ptr);
    }

    if (output_stage)
    {
        vx_uint32 tensor_size = 0;
        vx_ptr outputs_tensor_logical = VX_NULL;
        vxoTensor_GetTensorSize(outputs, &tensor_size);
        vxoTensor_GetTensorViewMemory(outputs, &outputs_tensor_logical, VX_NULL);

        gcoOS_MemCopy(outputs_tensor_logical, output_ptr, tensor_size);

        vxFree(output_ptr);
    }

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNDeConvolutionLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNDeConvolutionLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNDeConvolutionLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNDeConvolutionLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_tensor  weights                    = (vx_tensor)parameters[1];
    vx_tensor  bias                       = (vx_tensor)parameters[2];
    vx_scalar  padding_x                  = (vx_scalar)parameters[3];
    vx_scalar  padding_y                  = (vx_scalar)parameters[4];
    vx_scalar  overflow_policy            = (vx_scalar)parameters[5];
    vx_scalar  rounding_policy            = (vx_scalar)parameters[6];
    vx_scalar  a_x                        = (vx_scalar)parameters[7];
    vx_scalar  a_y                        = (vx_scalar)parameters[8];
    vx_scalar  group                      = (vx_scalar)parameters[9];
    vx_tensor  outputs                    = (vx_tensor)parameters[10];
    vx_bool    enable_gpu                 = vx_false_e;

    vxnne_deconvolution_layer  deconvolutionLayer = VX_NULL;
    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }
    gcoOS_Allocate(gcvNULL, sizeof(vxnne_deconvolution_layer_s), (gctPOINTER*)&deconvolutionLayer);
    if (!deconvolutionLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(deconvolutionLayer, sizeof(vxnne_deconvolution_layer_s));

    vxnneLayer_Initialize(&deconvolutionLayer->base,
                          "DeConvolutionLayer",
                          node,
                          deconvolutionLayer->operations,
                         VX_NULL);
    {
       int input_size[6],output_size[6],group_size;
       status |= vxQueryTensor((vx_tensor)inputs, VX_TENSOR_DIMS, input_size, sizeof(input_size));
       status |= vxQueryTensor((vx_tensor)outputs, VX_TENSOR_DIMS, output_size, sizeof(input_size));
        group_size = group->value->u32;
       enable_gpu = (vx_bool)(input_size[0]  == 10
                    &&input_size[1]  == 8
                    &&output_size[0] == 20
                    &&output_size[1] == 16
                    &&bias == NULL
                   );
    }
    if (enable_gpu)
    {

            vxnne_shader_executable shaderExecutable;
            vx_tensor rs_weights = NULL;
            vx_bool rs = (vx_bool)(weights->dimCount == 4);
            if(rs)
            {
                vx_int32 new_size[6] = {weights->dims[0],weights->dims[1], weights->dims[2]*weights->dims[3],1,1,1};
                rs_weights = vxReshapeTensor(weights, new_size, 3);
            }

            shaderExecutable = vxnneDeConvolutionShaderExecutable(node->base.context, VXNNE_KERNEL_DECONVOLUTION, &node->kernelAttributes.borderMode,
                inputs,
                rs ? rs_weights : weights,
                bias,
                padding_x,
                padding_y,
                overflow_policy,
                rounding_policy,
                a_x,
                a_y,
                group,
                outputs);
            if(rs)
                vxReleaseTensor(&rs_weights);

            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto exit;
            }

            status = vxnneShaderOperation_Initialize(&deconvolutionLayer->deconvolution_sh_operation,
                                          &deconvolutionLayer->base,
                                          VXNNE_OPERATOR_DECONVOLUTION,
                                          shaderExecutable);

            if (status != VX_SUCCESS) goto exit;

            deconvolutionLayer->base.num_operations = 1;
            deconvolutionLayer->operations[0] = &deconvolutionLayer->deconvolution_sh_operation.base;
    }
    else
    {
        vxnneOperation_Initialize(&deconvolutionLayer->deconvolution_sw_operation.base,
                                  &deconvolutionLayer->base,
                                  VXNNE_OPERATION_TARGET_SW,
                                  VXNNE_OPERATOR_REORG,
                                  vxnneExecuteSWDeConvolution,
                                  VX_NULL);

        deconvolutionLayer->base.num_operations = 1;
        deconvolutionLayer->operations[0] = (vxnne_operation)&deconvolutionLayer->deconvolution_sw_operation;

        deconvolutionLayer->deconvolution_sw_operation.inputs           = inputs;
        deconvolutionLayer->deconvolution_sw_operation.weights          = weights;
        deconvolutionLayer->deconvolution_sw_operation.biases           = bias;
        deconvolutionLayer->deconvolution_sw_operation.padding_x        = padding_x;
        deconvolutionLayer->deconvolution_sw_operation.padding_y        = padding_y;
        deconvolutionLayer->deconvolution_sw_operation.overflow_policy  = overflow_policy;
        deconvolutionLayer->deconvolution_sw_operation.rounding_policy  = rounding_policy;
        deconvolutionLayer->deconvolution_sw_operation.a_x              = a_x;
        deconvolutionLayer->deconvolution_sw_operation.a_y              = a_y;
        deconvolutionLayer->deconvolution_sw_operation.group            = group;
        deconvolutionLayer->deconvolution_sw_operation.outputs          = outputs;
    }

    node->layer = &deconvolutionLayer->base;
     return status;
exit:
    if (deconvolutionLayer != NULL)
        gcoOS_Free(NULL, deconvolutionLayer);
     return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNDeConvolutionLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

/***************************************************************************************************************************
 *                                                 L2Normalize
 ***************************************************************************************************************************/

vx_status vxnneExecuteSWL2Normalize(struct _vxnne_operation_s *operation)
{
    vxnne_l2normalize_operation l2normalizeOperation   = (vxnne_l2normalize_operation)operation;

    vx_tensor  inputs           = (vx_tensor)l2normalizeOperation->inputs;
    vx_tensor  outputs          = (vx_tensor)l2normalizeOperation->outputs;

    vx_uint32  input_width      = TENSOR_SIZE_INDEX(inputs, 0);
    vx_uint32  input_height     = TENSOR_SIZE_INDEX(inputs, 1);
    vx_uint32  input_depth      = TENSOR_SIZE_INDEX(inputs, 2);
    vx_uint32  input_batch      = TENSOR_SIZE_INDEX(inputs, 3);
    vx_type_e  inputFormat      = (vx_type_e)inputs->tensorBuffer->dataFormat;
    vx_type_e  outputFormat     = (vx_type_e)outputs->tensorBuffer->dataFormat;
    vx_uint8   inputFixedPoint  = inputs->tensorBuffer->fixedPointPos;
    vx_uint8   outputFixedPoint = outputs->tensorBuffer->fixedPointPos;

    vx_uint32  i,j,k,b;
    vx_float32 sum = 0.0f;
    vx_float32 epsilon = (vx_float32)10e-12;
    vx_float32 rsqrt = 0.0f;
    gctPOINTER inputBase;
    gctPOINTER outputBase;

    vx_status status = VX_SUCCESS;

    if (input_batch == 0)
    {
        input_batch = 1;
    }

    vxoTensor_GetTensorViewMemory(inputs, &inputBase, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, &outputBase, VX_NULL);

    if ((inputs->tensorBuffer->dataFormat != VX_TYPE_FLOAT16 && inputs->tensorBuffer->dataFormat != VX_TYPE_FLOAT32 && inputs->tensorBuffer->dataFormat != VX_TYPE_INT8)
        || (outputs->tensorBuffer->dataFormat != VX_TYPE_FLOAT16 && outputs->tensorBuffer->dataFormat != VX_TYPE_FLOAT32 && outputs->tensorBuffer->dataFormat != VX_TYPE_INT8))
    {
        gcmPRINT("input or outputs format is not support");
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
                    vx_float32 data = 0.0f;

                    data = vxnneGetData(inputFormat, in_index, (vx_uint8_ptr)inputBase, inputFixedPoint);
                    sum += data * data;
                    vxnneSaveData(outputFormat, in_index, data, (vx_uint8_ptr)outputBase, outputFixedPoint, outputs->tensorBuffer->roundingMode);
                }
            }
        }
    }

    rsqrt = gcoMATH_ReciprocalSquareRoot(gcoMATH_MAX(sum,epsilon));

    for(b = 0; b < input_batch; ++b)
    {
        for(k = 0; k < input_depth; ++k)
        {
            for(j = 0; j < input_height; ++j)
            {
                for(i = 0; i < input_width; ++i)
                {
                    vx_int32 index  = i + input_width * (j + input_height * (k + input_depth * b));
                    vx_float32 data = 0.0f;

                    data = vxnneGetData(inputFormat, index, (vx_uint8_ptr)inputBase, inputFixedPoint);
                    data = data * rsqrt;
                    vxnneSaveData(outputFormat, index, data, (vx_uint8_ptr)outputBase, outputFixedPoint, outputs->tensorBuffer->roundingMode);
                }
            }
        }
    }

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNL2NormalizeLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNL2NormalizeLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNL2NormalizeLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNL2NormalizeLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_tensor  outputs                    = (vx_tensor)parameters[1];

    vxnne_l2normalize_layer  l2normalizeLayer = VX_NULL;
    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }
    gcoOS_Allocate(gcvNULL, sizeof(vxnne_l2normalize_layer_s), (gctPOINTER*)&l2normalizeLayer);
    if (!l2normalizeLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(l2normalizeLayer, sizeof(vxnne_l2normalize_layer_s));

    vxnneLayer_Initialize(&l2normalizeLayer->base,
                          "L2NormalizeLayer",
                          node,
                          l2normalizeLayer->operations,
                          VX_NULL);
    vxnneOperation_Initialize(&l2normalizeLayer->l2normalize_sw_operation.base,
                              &l2normalizeLayer->base,
                              VXNNE_OPERATION_TARGET_SW,
                              VXNNE_OPERATOR_L2NORMALIZE,
                              vxnneExecuteSWL2Normalize,
                              VX_NULL);

    l2normalizeLayer->base.num_operations = 1;
    l2normalizeLayer->operations[0] = (vxnne_operation)&l2normalizeLayer->l2normalize_sw_operation;

    l2normalizeLayer->l2normalize_sw_operation.inputs           = inputs;
    l2normalizeLayer->l2normalize_sw_operation.outputs          = outputs;

    node->layer = &l2normalizeLayer->base;

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNL2NormalizeLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

