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


#include <VX/vx.h>
#ifdef WIN32
#include <dirent_win.h>
#else
#include <dirent.h>
#endif

#include <gc_vx_common.h>
#include <gc_vx_nn_extension_interface.h>
#include <gc_vx_internal_node_api.h>
#include "gc_hal_types.h"
#include "anchors.h"
#include <gc_vx_nn_util.h>
#include <gc_nn_arch_model.h>
#include <stdio.h>
#include <ops/gc_vx_ops.h>



vx_status vxnneGetTensorMemeory(vx_tensor tensor, vx_ptr_ptr ptr, vx_bool stage, vx_bool zero)
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

VX_API_ENTRY vx_status VX_API_CALL vxReleaseWeightsBiasesParameter(vx_weights_biases_parameter *weights_bias)
{
    gcmDUMP_API("$VX vxReleaseWeightsBiasesParameter: weights_bias=%p", weights_bias);

    if ((*weights_bias) == VX_NULL)
    {
        vxError("%s[%d]: Weights_bias is invalid!\n", __FUNCTION__, __LINE__);
        return VX_ERROR_INVALID_PARAMETERS;
    }

    return vxoReference_Release((vx_reference_ptr)weights_bias, VX_TYPE_WEIGHTS_BIASES_PARAMETER, VX_REF_INTERNAL);
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
    vx_int8     weights_fixed_point_pos,
    vx_uint32   biases_num_of_dims,
    vx_uint32 * biases_dims,
    vx_enum     biases_data_format,
    vx_int8     biases_fixed_point_pos,
    vx_uint32   raw_data_size
    )
{
    vx_weights_biases_parameter wb;
    vx_uint32 padXLeft, padXRight, padYTop, padYBottom;

    gcmDUMP_API("$VX vxCreateWeightsBiasesParameter: context=%p, layer_type=0x%x, num_of_dims=0x%x, inputs_dims=%p, pad_x=0x%x, pad_y=0x%x, pooling_size_x=0x%x, pooling_size_y=0x%x,"\
        " down_scale_size_rounding=0x%x, convolution_outputs_dims=%p, pool_outputs_dims=%p, weights_num_of_dims=0x%x, weights_dims=%p, weights_data_format=0x%x, weights_fixed_point_pos=0x%x,"\
        " biases_num_of_dims=0x%x, biases_dims=%p, biases_data_format=0x%x, biases_fixed_point_pos=0x%x, raw_data_size=0x%x", context, layer_type, num_of_dims, inputs_dims, pad_x, pad_y, pooling_size_x, pooling_size_y,
        down_scale_size_rounding, convolution_outputs_dims, pool_outputs_dims, weights_num_of_dims, weights_dims, weights_data_format, weights_fixed_point_pos, biases_num_of_dims, biases_dims, biases_data_format, biases_fixed_point_pos, raw_data_size);

    if (!vxoContext_IsValid(context)) return VX_NULL;

    padXLeft = padXRight = pad_x;
    padYTop = padYBottom = pad_y;

    wb = _createWeightsBiasesParameterFromParams(
            context,
            layer_type,
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
            weights_num_of_dims,
            weights_dims,
            weights_dims,
            weights_data_format,
            VX_QUANT_DYNAMIC_FIXED_POINT,
            weights_fixed_point_pos,
            biases_num_of_dims,
            biases_dims,
            biases_dims,
            biases_data_format,
            VX_QUANT_DYNAMIC_FIXED_POINT,
            biases_fixed_point_pos);

    return wb;
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
    vx_enum outType = TENSOR_DATA_TYPE(weights);

    gcmDUMP_API("$VX vxCreateWeightsBiasesParameterFromTensors: layer_type=0x%x, num_of_dims=0x%x, inputs_dims=%p, pad_x=0x%x, pad_y=0x%x, pooling_size_x=0x%x, pooling_size_y=0x%x,"\
        " down_scale_size_rounding=0x%x, convolution_outputs_dims=%p, pool_outputs_dims=%p, optimizations=%p, weights=%p, biases=%p",
        layer_type, num_of_dims, inputs_dims, pad_x, pad_y, pooling_size_x, pooling_size_y, down_scale_size_rounding, convolution_outputs_dims, pool_outputs_dims, optimizations, weights, biases);

    if (!vxoContext_IsValid(context)) return VX_NULL;


    padXLeft = padXRight = pad_x;
    padYTop = padYBottom = pad_y;

    if (optimizations)
        outType = optimizations->outputFormat;

    wb = _createWeightsBiasesParameterFromTensors(
            context,
            layer_type,
            inputs_dims,
            num_of_dims,
            num_of_dims,
            padXLeft,
            padXRight,
            padYTop,
            padYBottom,
            pooling_size_x,
            pooling_size_y,
            0,
            0,
            down_scale_size_rounding,
            convolution_outputs_dims,
            pool_outputs_dims,
            optimizations,
            outType,
            0,
            VX_TENSOR_RANK_WHCN,
            weights,
            biases,
            VX_NULL,
            vx_false_e,
            vx_true_e);

    return wb;
}

VX_API_ENTRY vx_weights_biases_parameter VX_API_CALL vxCreateWeightsBiasesParameterFromTensors2(
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
    vx_uint32 stride_x = 0;
    vx_uint32 stride_y = 0;
    vx_enum convert_format = 0;
    vx_enum rank_mode = VX_TENSOR_RANK_WHCN;
    vx_enum layer = layer_type;

    gcmDUMP_API("$VX vxCreateWeightsBiasesParameterFromTensors2: layer_type=0x%x, num_of_dims=0x%x, inputs_dims=%p, convolution_outputs_dims=%p, pool_outputs_dims=%p, output_format=0x%x,"\
        " convolution_relu_pooling_params=%p, size_of_convlution_relu_pooling_paramss=0x%lx, optimizations=%p, weights=%p, biases=%p",
        layer_type, num_of_dims, inputs_dims, convolution_outputs_dims, pool_outputs_dims, output_format, convolution_relu_pooling_params, size_of_convlution_relu_pooling_params, optimizations, weights, biases);


    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (size_of_convlution_relu_pooling_params == sizeof(vx_nn_convolution_relu_pooling_params_ext_t))
    {
        vx_nn_convolution_relu_pooling_params_ext_t conv_ext = *((vx_nn_convolution_relu_pooling_params_ext_t*)(convolution_relu_pooling_params));
        stride_x = conv_ext.stride_x;
        stride_y = conv_ext.stride_y;
    }
    else if (size_of_convlution_relu_pooling_params == sizeof(vx_nn_convolution_relu_pooling_params_ext2_t))
    {
        vx_nn_convolution_relu_pooling_params_ext2_t conv_ext2 = *((vx_nn_convolution_relu_pooling_params_ext2_t*)(convolution_relu_pooling_params));
        stride_x = conv_ext2.ext.stride_x;
        stride_y = conv_ext2.ext.stride_y;
        rank_mode = conv_ext2.src_rank_mode;
        convert_format = conv_ext2.convert_dst_format;

        if (conv_ext2.depth_multiplier == 1 &&
            (TENSOR_DATA_TYPE(weights) == VX_TYPE_INT8 || TENSOR_DATA_TYPE(weights) == VX_TYPE_UINT8) &&
            vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_NN_DEPTHWISE_SUPPORT))
            layer = VX_NN_DEPTH_WISE_CONVOLUTION_LAYER;
    }
    else if (size_of_convlution_relu_pooling_params != sizeof(vx_nn_convolution_relu_pooling_params_t))
    {
        vxError("Invalid parameter convolution_relu_pooling_params");
        return VX_NULL;
    }

    wb = _createWeightsBiasesParameterFromTensors(
            context,
            layer,
            inputs_dims,
            num_of_dims,
            num_of_dims,
            convolution_relu_pooling_params->pad_x_left,
            convolution_relu_pooling_params->pad_x_right,
            convolution_relu_pooling_params->pad_y_top,
            convolution_relu_pooling_params->pad_y_bottom,
            convolution_relu_pooling_params->pool_size_x,
            convolution_relu_pooling_params->pool_size_y,
            stride_x,
            stride_y,
            convolution_relu_pooling_params->down_scale_size_rounding,
            convolution_outputs_dims,
            pool_outputs_dims,
            optimizations,
            output_format,
            convert_format,
            rank_mode,
            weights,
            biases,
            VX_NULL,
            vx_false_e,
            vx_true_e);

    return wb;
}


VX_API_ENTRY vx_weights_biases_parameter VX_API_CALL vxCreateWeightsBiasesParameterFromTensors3(
    vx_enum     layer_type,
    vx_uint32 * inputs_dims,
    vx_uint32 * convolution_outputs_dims,
    vx_uint32 * pool_outputs_dims,
    const vx_nn_convolution_relu_pooling_params convolution_relu_pooling_params,
    vx_size size_of_convolution_relu_pooling_params,
    vx_weights_biases_parameter_optimizations_t *optimizations,
    vx_size size_of_optimizations,
    vx_tensor   weights,
    vx_tensor   biases)
{
    vx_weights_biases_parameter wb;
    vx_context context = vxGetContext((vx_reference)weights);
    vx_uint32 stride_x = 0;
    vx_uint32 stride_y = 0;
    vx_uint32 num_of_input_dims = 0;
    vx_uint32 num_of_output_dims = 0;
    vx_enum output_format = TENSOR_DATA_TYPE(weights);
    vx_enum convert_format = 0;
    vx_enum rank_mode = VX_TENSOR_RANK_WHCN;

    vx_enum layer = layer_type;

    gcmDUMP_API("$VX vxCreateWeightsBiasesParameterFromTensors3: layer_type=0x%x, inputs_dims=%p, "
        "convolution_outputs_dims=%p, pool_outputs_dims=%p, convolution_relu_pooling_params=%p, "
        "size_of_convolution_relu_pooling_params=0x%lx, optimizations=%p, size_of_optimizations=0x%lx, weights=%p, biases=%p",
        layer_type, inputs_dims, convolution_outputs_dims, pool_outputs_dims, convolution_relu_pooling_params, size_of_convolution_relu_pooling_params, optimizations, size_of_optimizations, weights, biases);

    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (size_of_convolution_relu_pooling_params == sizeof(vx_nn_convolution_relu_pooling_params_ext_t))
    {
        vx_nn_convolution_relu_pooling_params_ext_t conv_ext = *((vx_nn_convolution_relu_pooling_params_ext_t*)(convolution_relu_pooling_params));
        stride_x = conv_ext.stride_x;
        stride_y = conv_ext.stride_y;
    }
    else if (size_of_convolution_relu_pooling_params == sizeof(vx_nn_convolution_relu_pooling_params_ext2_t))
    {
        vx_nn_convolution_relu_pooling_params_ext2_t conv_ext2 = *((vx_nn_convolution_relu_pooling_params_ext2_t*)(convolution_relu_pooling_params));
        stride_x = conv_ext2.ext.stride_x;
        stride_y = conv_ext2.ext.stride_y;
        rank_mode = conv_ext2.src_rank_mode;
        convert_format = conv_ext2.convert_dst_format;

        if (conv_ext2.depth_multiplier == 1 &&
            (TENSOR_DATA_TYPE(weights) == VX_TYPE_INT8 || TENSOR_DATA_TYPE(weights) == VX_TYPE_UINT8) &&
            vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_NN_DEPTHWISE_SUPPORT))
            layer = VX_NN_DEPTH_WISE_CONVOLUTION_LAYER;
    }
    else if (size_of_convolution_relu_pooling_params != sizeof(vx_nn_convolution_relu_pooling_params_t))
    {
        vxError("Invalid parameter convolution_relu_pooling_params");
        return VX_NULL;
    }

    if (optimizations)
    {
        if (size_of_optimizations == sizeof(vx_weights_biases_parameter_optimizations_t))
            output_format = optimizations->outputFormat;
        else if (size_of_optimizations == sizeof(vx_weights_biases_parameter_optimizations_ext_t))
        {
            vx_weights_biases_parameter_optimizations_ext_t opt = *((vx_weights_biases_parameter_optimizations_ext_t *)optimizations);
            output_format = opt.outputFormat;
            num_of_input_dims = opt.num_of_input_dims;
            num_of_output_dims = opt.num_of_output_dims;
        }
        else
        {
            vxError("Invalid parameter convolution_relu_pooling_params");
            return VX_NULL;
        }
    }

    wb = _createWeightsBiasesParameterFromTensors(
            context,
            layer,
            inputs_dims,
            num_of_input_dims,
            num_of_output_dims,
            convolution_relu_pooling_params->pad_x_left,
            convolution_relu_pooling_params->pad_x_right,
            convolution_relu_pooling_params->pad_y_top,
            convolution_relu_pooling_params->pad_y_bottom,
            convolution_relu_pooling_params->pool_size_x,
            convolution_relu_pooling_params->pool_size_y,
            stride_x,
            stride_y,
            convolution_relu_pooling_params->down_scale_size_rounding,
            convolution_outputs_dims,
            pool_outputs_dims,
            optimizations,
            output_format,
            convert_format,
            rank_mode,
            weights,
            biases,
            VX_NULL,
            vx_false_e,
            vx_true_e);

    return wb;
}

VX_INTERNAL_API vx_weights_biases_parameter vxoCreateWeightsBiasesParameterFromTensorsPRelu(
    vx_enum     layer_type,
    vx_uint32 * inputs_dims,
    vx_uint32 * convolution_outputs_dims,
    vx_uint32 * pool_outputs_dims,
    const vx_nn_convolution_relu_pooling_params convolution_relu_pooling_params,
    vx_size size_of_convolution_relu_pooling_params,
    vx_weights_biases_parameter_optimizations_t *optimizations,
    vx_size size_of_optimizations,
    vx_tensor   weights,
    vx_tensor   biases,
    vx_tensor   alpha)
{
    vx_weights_biases_parameter wb;
    vx_context context = vxGetContext((vx_reference)weights);
    vx_uint32 stride_x = 0;
    vx_uint32 stride_y = 0;
    vx_uint32 num_of_input_dims = 0;
    vx_uint32 num_of_output_dims = 0;
    vx_enum output_format = TENSOR_DATA_TYPE(weights);
    vx_enum convert_format = 0;
    vx_enum rank_mode = VX_TENSOR_RANK_WHCN;

    vx_enum layer = layer_type;

    gcmDUMP_API("$VX vxCreateWeightsBiasesParameterFromTensors3: layer_type=0x%x, inputs_dims=%p, "
        "convolution_outputs_dims=%p, pool_outputs_dims=%p, convolution_relu_pooling_params=%p, "
        "size_of_convolution_relu_pooling_params=0x%lx, optimizations=%p, size_of_optimizations=0x%lx, weights=%p, biases=%p",
        layer_type, inputs_dims, convolution_outputs_dims, pool_outputs_dims, convolution_relu_pooling_params, size_of_convolution_relu_pooling_params, optimizations, size_of_optimizations, weights, biases);

    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (size_of_convolution_relu_pooling_params == sizeof(vx_nn_convolution_relu_pooling_params_ext_t))
    {
        vx_nn_convolution_relu_pooling_params_ext_t conv_ext = *((vx_nn_convolution_relu_pooling_params_ext_t*)(convolution_relu_pooling_params));
        stride_x = conv_ext.stride_x;
        stride_y = conv_ext.stride_y;
    }
    else if (size_of_convolution_relu_pooling_params == sizeof(vx_nn_convolution_relu_pooling_params_ext2_t))
    {
        vx_nn_convolution_relu_pooling_params_ext2_t conv_ext2 = *((vx_nn_convolution_relu_pooling_params_ext2_t*)(convolution_relu_pooling_params));
        stride_x = conv_ext2.ext.stride_x;
        stride_y = conv_ext2.ext.stride_y;
        rank_mode = conv_ext2.src_rank_mode;
        convert_format = conv_ext2.convert_dst_format;

        if (conv_ext2.depth_multiplier == 1 &&
            (TENSOR_DATA_TYPE(weights) == VX_TYPE_INT8 || TENSOR_DATA_TYPE(weights) == VX_TYPE_UINT8) &&
            vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_NN_DEPTHWISE_SUPPORT))
            layer = VX_NN_DEPTH_WISE_CONVOLUTION_LAYER;
    }
    else if (size_of_convolution_relu_pooling_params != sizeof(vx_nn_convolution_relu_pooling_params_t))
    {
        vxError("Invalid parameter convolution_relu_pooling_params");
        return VX_NULL;
    }

    if (optimizations)
    {
        if (size_of_optimizations == sizeof(vx_weights_biases_parameter_optimizations_t))
            output_format = optimizations->outputFormat;
        else if (size_of_optimizations == sizeof(vx_weights_biases_parameter_optimizations_ext_t))
        {
            vx_weights_biases_parameter_optimizations_ext_t opt = *((vx_weights_biases_parameter_optimizations_ext_t *)optimizations);
            output_format = opt.outputFormat;
            num_of_input_dims = opt.num_of_input_dims;
            num_of_output_dims = opt.num_of_output_dims;
        }
        else
        {
            vxError("Invalid parameter convolution_relu_pooling_params");
            return VX_NULL;
        }
    }

    wb = _createWeightsBiasesParameterFromTensors(
            context,
            layer,
            inputs_dims,
            num_of_input_dims,
            num_of_output_dims,
            convolution_relu_pooling_params->pad_x_left,
            convolution_relu_pooling_params->pad_x_right,
            convolution_relu_pooling_params->pad_y_top,
            convolution_relu_pooling_params->pad_y_bottom,
            convolution_relu_pooling_params->pool_size_x,
            convolution_relu_pooling_params->pool_size_y,
            stride_x,
            stride_y,
            convolution_relu_pooling_params->down_scale_size_rounding,
            convolution_outputs_dims,
            pool_outputs_dims,
            optimizations,
            output_format,
            convert_format,
            rank_mode,
            weights,
            biases,
            alpha,
            vx_true_e,
            vx_true_e);

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
    gcmDUMP_API("$VX vxMapWeightsBiasesParameter: weights_biases=%p, map_id=%p, stride=%p, ptr=%p, usage=0x%x, mem_type=0x%x, flags=0x%x",
        weights_biases, map_id, stride, ptr, usage, mem_type, flags);

    if (!vxoWeightsBiasesParameter_IsValid(weights_biases)) return VX_ERROR_INVALID_REFERENCE;

    return vxoWeightsBiasesParameter_Map(weights_biases, map_id, stride, ptr, usage, mem_type, flags);
}

VX_API_ENTRY vx_status VX_API_CALL
vxUnmapWeightsBiasesParameter(
    vx_weights_biases_parameter     weights_biases,
    vx_map_id                       map_id
    )
{
    gcmDUMP_API("$VX vxUnmapWeightsBiasesParameter: weights_biases=%p, map_id=0x%x", weights_biases, map_id);

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
    gcmDUMP_API("$VX vxConfigTarget: context=%p, dp_amount=0x%x, mac_per_core=0x%x, conv_cores=0x%x, in_buffer_depth=0x%x, accum_buffer_height=0x%x, l2_cache_size=0x%x, tp_cores=0x%x",
        context, dp_amount, mac_per_core, conv_cores, in_buffer_depth, accum_buffer_height, l2_cache_size, tp_cores);

    if (!vxoContext_IsValid(context)) return VX_ERROR_INVALID_REFERENCE;

    context->nnConfig.fixedFeature.nnMadPerCore = mac_per_core;
    context->nnConfig.fixedFeature.nnCoreCount = conv_cores;
    context->nnConfig.fixedFeature.tpCoreCount = tp_cores;
    context->nnConfig.fixedFeature.nnInputBufferDepth = in_buffer_depth;
    context->nnConfig.fixedFeature.nnAccumBufferDepth = accum_buffer_height;
    context->nnConfig.derivedFeature.nnDPAmount = dp_amount;
    context->nnConfig.customizedFeature.vipSRAMSize = l2_cache_size;
    context->nnConfig.isSet = gcvTRUE;
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_uint32 vxoWeightsBiasesParameterStreamSize(
    vx_weights_biases_parameter weights_biases_parameter,
    vx_bool onlyHeaderStream
)
{
    vx_weights_biases_parameter wb = weights_biases_parameter;
    vx_uint32 bufferSize = 0;

    /*calculate stream buffer size*/
    if (onlyHeaderStream)
    {
        bufferSize = sizeof(vx_uint32) * 32
            + sizeof(vx_uint32) * VX_CONTEXT_TENSOR_MAX_DIMENSION * 3
            + sizeof(vx_uint32) * wb->slice_num * 10
            + sizeof(vx_uint32) * VX_CONTEXT_TENSOR_MAX_DIMENSION * 2;
    }
    else
    {
        bufferSize = (vx_uint32)WB_RAW_DATA_SIZE(wb)
            + sizeof(vx_uint32) * 32
            + sizeof(vx_uint32) * VX_CONTEXT_TENSOR_MAX_DIMENSION * 3
            + sizeof(vx_uint32) * wb->slice_num * 10
            + sizeof(vx_uint32) * VX_CONTEXT_TENSOR_MAX_DIMENSION * 2;
    }

    bufferSize += sizeof(vx_uint32);
    if (wb->sub_wb_vdata != VX_NULL)
    {
        bufferSize += sizeof(vx_uint32);
        bufferSize += sizeof(vx_uint32) * 1;
        bufferSize += (vx_uint32)wb->sub_wb_vdata->wb_memory_size;
        bufferSize += sizeof(vx_uint32) * 1;
        bufferSize += sizeof(vx_uint32) * wb->sub_wb_vdata->slice_num * 10;
    }

    bufferSize += sizeof(vx_uint32);
    if (weights_biases_parameter->archPerfHandle != VX_NULL) bufferSize += 89 * sizeof(vx_uint32);

    /* support multiVIP, save mGpuWBCount to stream*/
    bufferSize += sizeof(vx_uint32);

    return bufferSize;
}

VX_API_ENTRY vx_uint32* VX_API_CALL vxWeightsBiasesParameterToStream(
    vx_context context,
    vx_weights_biases_parameter weights_biases_parameter,
    vx_uint32 *weights_biases_stream_size,
    vx_bool onlyHeaderStream
)
{
    vx_weights_biases_parameter wb = weights_biases_parameter;
    vx_weights_biases_parameter_base wb_base = wb->wb_base;

    vx_uint32 bufferSize;
    vx_uint32 checkSize;
    vx_uint32 bitOffset;
    vx_uint32* kernelBufferPtr = NULL;
    vx_uint32* base = NULL;
    vx_uint32 i;

    gcmDUMP_API("$VX vxWeightsBiasesParameterToStream: context=%p, weights_biases_parameter=%p, weights_biases_stream_size=%p, onlyHeaderStream=0x%x",
        context, weights_biases_parameter, weights_biases_stream_size, onlyHeaderStream);

    gcmASSERT(context);

    bufferSize = vxoWeightsBiasesParameterStreamSize(wb, onlyHeaderStream);

    kernelBufferPtr = (vx_uint32*)vxAllocateAndZeroMemory(bufferSize);

    if (kernelBufferPtr == VX_NULL)
    {
        vxError("vxWeightsBiasesParameterToStream: OUT OF MEMORY");
        *weights_biases_stream_size = 0;
        return VX_NULL;
    }

    base = kernelBufferPtr;

    /* Write weights biases parameter to the head of data*/
    bitOffset = 0;

    /* wb base */
    writeBits(&kernelBufferPtr, &bitOffset, wb_base->pad_x_left, 32);
    writeBits(&kernelBufferPtr, &bitOffset, wb_base->pad_x_right, 32);
    writeBits(&kernelBufferPtr, &bitOffset, wb_base->pad_y_top, 32);
    writeBits(&kernelBufferPtr, &bitOffset, wb_base->pad_y_bottom, 32);
    writeBits(&kernelBufferPtr, &bitOffset, wb_base->pooling_size_x, 32);
    writeBits(&kernelBufferPtr, &bitOffset, wb_base->pooling_size_y, 32);
    writeBits(&kernelBufferPtr, &bitOffset, wb_base->down_scale_size_rounding, 32);
    writeBits(&kernelBufferPtr, &bitOffset, wb_base->weights_num_of_dims, 32);
    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        if (i < wb_base->weights_num_of_dims)
        {
            writeBits(&kernelBufferPtr, &bitOffset, wb_base->weights_sizes[i], 32);
            writeBits(&kernelBufferPtr, &bitOffset, wb_base->org_weights_sizes[i], 32);
        }
        else
        {
            writeBits(&kernelBufferPtr, &bitOffset, 0, 32);
            writeBits(&kernelBufferPtr, &bitOffset, 0, 32);
        }
    }
    writeBits(&kernelBufferPtr, &bitOffset, wb_base->weights_data_format, 32);
    writeBits(&kernelBufferPtr, &bitOffset, wb_base->weights_quant_format, 32);
    writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb_base->weights_fixed_point_pos, 32);

    writeBits(&kernelBufferPtr, &bitOffset, wb_base->biases_num_of_dims, 32);
    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        if (i < wb_base->biases_num_of_dims)
        {
            writeBits(&kernelBufferPtr, &bitOffset, wb_base->biases_sizes[i], 32);
        }
        else
        {
            writeBits(&kernelBufferPtr, &bitOffset, 0, 32);
        }
    }
    writeBits(&kernelBufferPtr, &bitOffset, wb_base->biases_data_format, 32);
    writeBits(&kernelBufferPtr, &bitOffset, wb_base->biases_quant_format, 32);
    writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb_base->biases_fixed_point_pos, 32);

    /* wb */
    writeBits(&kernelBufferPtr, &bitOffset, wb->layer_type, 32);
    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        if (i < WB_WEIGHT_DIM_NUM(wb))
        {
            writeBits(&kernelBufferPtr, &bitOffset, wb->weights_sizes[i], 32);
            writeBits(&kernelBufferPtr, &bitOffset, wb->biases_sizes[i], 32);
        }
        else
        {
            writeBits(&kernelBufferPtr, &bitOffset, 0, 32);
            writeBits(&kernelBufferPtr, &bitOffset, 0, 32);
        }
    }
    writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)WB_RAW_DATA_SIZE(wb), 32);

    writeBits(&kernelBufferPtr, &bitOffset, wb_base->strideX, 32);
    writeBits(&kernelBufferPtr, &bitOffset, wb_base->strideY, 32);
    writeBits(&kernelBufferPtr, &bitOffset, wb_base->pooling_stride, 32);
    writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb_base->setZeroLength, 32);
    writeBits(&kernelBufferPtr, &bitOffset, wb_base->inputZP, 32);
    writeBits(&kernelBufferPtr, &bitOffset, wb_base->coefZP, 32);
    writeBits(&kernelBufferPtr, &bitOffset, *(vx_uint32 *)&wb_base->coefScale, 32);
    writeBits(&kernelBufferPtr, &bitOffset, *(vx_uint32 *)&wb_base->biasScale, 32);
    writeBits(&kernelBufferPtr, &bitOffset, wb_base->skipValue, 32);
    writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb_base->nn_fc_batch_mode, 32);
    writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb_base->do_fisrt_pixel_pool, 32);
    writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb_base->do_1xN, 32);
    writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb_base->do_zdp_opt, 32);

    writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->use_tp_fc, 32);
    writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->slice_num, 32);

    for (i = 0; i < wb->slice_num; i++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->slice_array[i].kz_count, 32);
        writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->slice_array[i].z_count, 32);

        writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->slice_array[i].memory_offset, 32);
        writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->slice_array[i].memory_size, 32);
        writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->slice_array[i].kernel_orig_size, 32);
        writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->slice_array[i].kernel_stream_size, 32);
        writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->slice_array[i].kernel_align_stream_size, 32);

        writeBits(&kernelBufferPtr, &bitOffset, wb->slice_array[i].non_zero_count, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->slice_array[i].reserve_weight_count, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->slice_array[i].all_count, 32);
    }

    if (wb->sub_wb_vdata != VX_NULL)
    {
        writeBits(&kernelBufferPtr, &bitOffset, 1, 32);
        writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->sub_wb_vdata->wb_memory_size, 32);
        kernelBufferPtr++;
        memcpy(kernelBufferPtr, wb->sub_wb_vdata->wb_memory_ptr, wb->sub_wb_vdata->wb_memory_size);
        kernelBufferPtr = (vx_uint32*)((vx_uint8*)kernelBufferPtr + wb->sub_wb_vdata->wb_memory_size);
        bitOffset = 0; /*clear bitOffset flag after memcpy to fix the current kernelBufferPtr in next writeBits() */
        writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->sub_wb_vdata->slice_num, 32);
        for (i = 0; i < wb->sub_wb_vdata->slice_num; i++)
        {
            writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->sub_wb_vdata->slice_array[i].kz_count, 32);
            writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->sub_wb_vdata->slice_array[i].z_count, 32);

            writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->sub_wb_vdata->slice_array[i].memory_offset, 32);
            writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->sub_wb_vdata->slice_array[i].memory_size, 32);
            writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->sub_wb_vdata->slice_array[i].kernel_orig_size, 32);
            writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->sub_wb_vdata->slice_array[i].kernel_stream_size, 32);
            writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->sub_wb_vdata->slice_array[i].kernel_align_stream_size, 32);

            writeBits(&kernelBufferPtr, &bitOffset, wb->sub_wb_vdata->slice_array[i].non_zero_count, 32);
            writeBits(&kernelBufferPtr, &bitOffset, wb->sub_wb_vdata->slice_array[i].reserve_weight_count, 32);
            writeBits(&kernelBufferPtr, &bitOffset, wb->sub_wb_vdata->slice_array[i].all_count, 32);
        }
        writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->sub_wb_vdata->kernel_per_core, 32);
    }
    else
    {
        writeBits(&kernelBufferPtr, &bitOffset, 0, 32);
    }

    /* for multiVIP */
    writeBits(&kernelBufferPtr, &bitOffset, wb->mGpuWBCount, 32);

    if (wb->archPerfHandle != VX_NULL)
    {
        writeBits(&kernelBufferPtr, &bitOffset, 1, 32);
        /* Copy arch perf data*/
        /*archPerfor base: 7*/
        writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->archPerfHandle->calculated, 32);
        writeBits(&kernelBufferPtr, &bitOffset, *(vx_uint32 *)&wb->archPerfHandle->coefNonZeroRatio, 32);
        writeBits(&kernelBufferPtr, &bitOffset, *((vx_uint32 *)&wb->archPerfHandle->coefNonZeroRatio + 1), 32);
        writeBits(&kernelBufferPtr, &bitOffset, *(vx_uint32 *)&wb->archPerfHandle->coefCompressRatio, 32);
        writeBits(&kernelBufferPtr, &bitOffset, *((vx_uint32 *)&wb->archPerfHandle->coefCompressRatio + 1), 32);
        writeBits(&kernelBufferPtr, &bitOffset, *(vx_uint32 *)&wb->archPerfHandle->imageCompressRatio, 32);
        writeBits(&kernelBufferPtr, &bitOffset, *((vx_uint32 *)&wb->archPerfHandle->imageCompressRatio + 1), 32);
        writeBits(&kernelBufferPtr, &bitOffset, *(vx_uint32 *)&wb->archPerfHandle->imageNonZeroRatio, 32);
        writeBits(&kernelBufferPtr, &bitOffset, *((vx_uint32 *)&wb->archPerfHandle->imageNonZeroRatio + 1), 32);
        writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->archPerfHandle->opType, 32);
        writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->archPerfHandle->opTarget, 32);

        /*archPerfor info: 27*/
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.kx, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.ky, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.kz, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.oinx, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.oiny, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.oinz, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.inx, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.iny, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.inz, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.outx, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.outy, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.outz, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.stridex, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.stridey, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.inputDataSize, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.outputDataSize, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.poolingSize, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.poolingStride, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.xOffSet, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.yOffSet, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.inputDataFormat, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.outputDataFormat, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.nnCores, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.convOutFifoDepth, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.kernelSize, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.pix, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.piy, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.p3, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->info.nextKY, 32);
        writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->archPerfHandle->info.flush, 32);

        /*archPerfor swTilingInfo: 23*/
        writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->archPerfHandle->swTilingInfo.srcBuf, 32);
        writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->archPerfHandle->swTilingInfo.dstBuf, 32);
        writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->archPerfHandle->swTilingInfo.kernelBuf, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->swTilingInfo.cacheSpace, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->swTilingInfo.origInX, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->swTilingInfo.origInY, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->swTilingInfo.origOutX, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->swTilingInfo.origOutY, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->swTilingInfo.origOutZ, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->swTilingInfo.outImageStride, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->swTilingInfo.outImageSlice, 32);
        writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->archPerfHandle->swTilingInfo.kernelCacheMode, 32);
        writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->archPerfHandle->swTilingInfo.imageCacheMode, 32);
        writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->archPerfHandle->swTilingInfo.calcNonFirstCmd, 32);
        writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->archPerfHandle->swTilingInfo.isNonFirstComputeBottleNeck, 32);

        writeBits(&kernelBufferPtr, &bitOffset, *(vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstCycleCount, 32);
        writeBits(&kernelBufferPtr, &bitOffset, *((vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstCycleCount + 1), 32);

        writeBits(&kernelBufferPtr, &bitOffset, *(vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstKernelReadBandWidth, 32);
        writeBits(&kernelBufferPtr, &bitOffset, *((vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstKernelReadBandWidth + 1), 32);
        writeBits(&kernelBufferPtr, &bitOffset, *(vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstInImageReadBandWidth, 32);
        writeBits(&kernelBufferPtr, &bitOffset, *((vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstInImageReadBandWidth + 1), 32);

        writeBits(&kernelBufferPtr, &bitOffset, *(vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstReadBandWidth, 32);
        writeBits(&kernelBufferPtr, &bitOffset, *((vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstReadBandWidth + 1), 32);
        writeBits(&kernelBufferPtr, &bitOffset, *(vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstWriteBandWidth, 32);
        writeBits(&kernelBufferPtr, &bitOffset, *((vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstWriteBandWidth + 1), 32);
        writeBits(&kernelBufferPtr, &bitOffset, *(vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstAXIReadBandWidth, 32);
        writeBits(&kernelBufferPtr, &bitOffset, *((vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstAXIReadBandWidth + 1), 32);
        writeBits(&kernelBufferPtr, &bitOffset, *(vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstAXIWriteBandWidth, 32);
        writeBits(&kernelBufferPtr, &bitOffset, *((vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstAXIWriteBandWidth + 1), 32);

        /*archPerfor resultInfo: 17*/
        writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->archPerfHandle->resultInfo.isFirstComputeBottleNeck, 32);
        writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)wb->archPerfHandle->resultInfo.calculated, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->resultInfo.kernelsPerCore, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->resultInfo.outImageTileXSize, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->resultInfo.outImageTileYSize, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->resultInfo.interleaveMode, 32);
        writeBits(&kernelBufferPtr, &bitOffset, wb->archPerfHandle->resultInfo.nnCoreCount, 32);
        writeBits(&kernelBufferPtr, &bitOffset, *(vx_uint32 *)&wb->archPerfHandle->resultInfo.perfCycleCount, 32);
        writeBits(&kernelBufferPtr, &bitOffset, *((vx_uint32 *)&wb->archPerfHandle->resultInfo.perfCycleCount + 1), 32);
        writeBits(&kernelBufferPtr, &bitOffset, *(vx_uint32 *)&wb->archPerfHandle->resultInfo.perfReadBandWidth, 32);
        writeBits(&kernelBufferPtr, &bitOffset, *((vx_uint32 *)&wb->archPerfHandle->resultInfo.perfReadBandWidth + 1), 32);
        writeBits(&kernelBufferPtr, &bitOffset, *(vx_uint32 *)&wb->archPerfHandle->resultInfo.perfWriteBandWidth, 32);
        writeBits(&kernelBufferPtr, &bitOffset, *((vx_uint32 *)&wb->archPerfHandle->resultInfo.perfWriteBandWidth + 1), 32);
        writeBits(&kernelBufferPtr, &bitOffset, *(vx_uint32 *)&wb->archPerfHandle->resultInfo.perfAXIReadBandWidth, 32);
        writeBits(&kernelBufferPtr, &bitOffset, *((vx_uint32 *)&wb->archPerfHandle->resultInfo.perfAXIReadBandWidth + 1), 32);
        writeBits(&kernelBufferPtr, &bitOffset, *(vx_uint32 *)&wb->archPerfHandle->resultInfo.perfAXIWriteBandWidth, 32);
        writeBits(&kernelBufferPtr, &bitOffset, *((vx_uint32 *)&wb->archPerfHandle->resultInfo.perfAXIWriteBandWidth + 1), 32);

        writeBits(&kernelBufferPtr, &bitOffset, *(vx_uint32 *)&wb->archPerfHandle->resultInfo.perfKernelReadBandWidth, 32);
        writeBits(&kernelBufferPtr, &bitOffset, *((vx_uint32 *)&wb->archPerfHandle->resultInfo.perfKernelReadBandWidth + 1), 32);
        writeBits(&kernelBufferPtr, &bitOffset, *(vx_uint32 *)&wb->archPerfHandle->resultInfo.perfInImageReadBandWidth, 32);
        writeBits(&kernelBufferPtr, &bitOffset, *((vx_uint32 *)&wb->archPerfHandle->resultInfo.perfInImageReadBandWidth + 1), 32);
        kernelBufferPtr++;
    }
    else
    {
        writeBits(&kernelBufferPtr, &bitOffset, 0, 32);
        kernelBufferPtr++;
    }

    /* Copy kernel stream data*/
    if (onlyHeaderStream)
        checkSize = (vx_uint32)((vx_uint8*)kernelBufferPtr - (vx_uint8*)base);
    else
    {
        vxMemCopy(kernelBufferPtr, WB_MEM_LOGICAL_BASE_ADDR(wb), WB_RAW_DATA_SIZE(wb));
        checkSize = (vx_uint32)((vx_uint8*)kernelBufferPtr - (vx_uint8*)base) + (vx_uint32)WB_RAW_DATA_SIZE(wb);
    }

    if (checkSize != bufferSize)
    {
        vxmASSERT(0);
    }

    /* support multiVIP */
    if (wb->mGpuWBCount > 0)
    {
        vx_uint8* streamBufferPtr = VX_NULL;
        vx_uint8* streamBase = VX_NULL;
        vx_uint32 streamSize = bufferSize;
        vx_uint32 i = 0;

        /*1. calculate stream size and allocate memory for stream */
        for (i = 0; i < wb->mGpuWBCount; i++)
        {
            streamSize += vxoWeightsBiasesParameterStreamSize(wb->mGpuWBTable[i], vx_false_e);
        }
        streamBufferPtr = (vx_uint8*)vxAllocateAndZeroMemory(streamSize);
        if (streamBufferPtr == VX_NULL)
        {
            vxError("vxWeightsBiasesParameterToStream: OUT OF MEMORY, fail to allocate memory");
            *weights_biases_stream_size = 0;
            vxFree(base);
            return VX_NULL;
        }
        streamBase = streamBufferPtr;

        /*2. copy parent weight biase stream to streamBufferPtr */
        vxMemCopy(streamBufferPtr, base, bufferSize);
        vxFree(base);
        base = VX_NULL;
        streamBufferPtr += bufferSize;

        /*3. convert the weight_biase_parameter of multiVIP to stream */
        for (i = 0; i < wb->mGpuWBCount; i++)
        {
            vx_uint32 size = 0;
            vx_uint32 *buffer = VX_NULL;
            buffer = vxWeightsBiasesParameterToStream(context, wb->mGpuWBTable[i], &size, vx_false_e);
            vxmASSERT((buffer != VX_NULL) && (size > 0));
            vxMemCopy(streamBufferPtr, buffer, size);
            vxFree(buffer);
            buffer = VX_NULL;
            streamBufferPtr += size;

            vxmASSERT((vx_uint32)(streamBufferPtr - streamBase) <= streamSize);
        }

        *weights_biases_stream_size = streamSize;
        return (vx_uint32*)streamBase;
    }
    else
    {
        *weights_biases_stream_size = bufferSize;
        return base;
    }

}

VX_API_ENTRY vx_status VX_API_CALL vxFreeWeightsBiasesParameterStream(
    vx_uint32 *weights_biases_stream
)
{
    gcmDUMP_API("$VX vxFreeWeightsBiasesParameterStream: weights_biases_stream=%p", weights_biases_stream);
    if (weights_biases_stream != VX_NULL)
        vxFree(weights_biases_stream);
    weights_biases_stream = VX_NULL;

    return VX_SUCCESS;
}

#define DUMP_OFFLINE 0
VX_API_ENTRY vx_weights_biases_parameter VX_API_CALL vxCreateWeightsBiasesParameterFromStream(
    vx_context context,
    vx_uint32 * weights_biases_stream
)
{
    vx_weights_biases_parameter wb;
    vx_uint32 bitOffset = 0;
    vx_uint32* streamBufferPtr = weights_biases_stream;
    vx_uint8* streamBasePtr = (vx_uint8*)weights_biases_stream;
    vx_uint32 i;
    vx_status status = VX_SUCCESS;

    vx_enum     layerType;
    vx_uint32   poolingSizeX, poolingSizeY;
    vx_enum     downScaleSizeRounding;
    vx_uint32   weightsNumOfDims;
    vx_uint32   weightsDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32   weightsOrgDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32   weightsSubDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_enum     weightsDataFormat;
    vx_enum     weightsQuantFormat;
    vx_int8     weightsFixedPointPos;
    vx_uint32   biasesNumOfDims;
    vx_uint32   biasesDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32   biasesSubDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_enum     biasesDataFormat;
    vx_enum     biasesQuantFormat;
    vx_int8     biasesFixedPointPos;
    vx_uint32   rawDataSize;
    vx_uint32   padXLeft, padXRight, padYTop, padYBottom;
    vx_uint32   sub_wb_memory_size;
#if DUMP_OFFLINE
    char fileName[1024];
    vx_uint32 * base = streamBufferPtr;
    FILE *outputFile = NULL;
    static vx_uint32 idx = 0;
    vx_uint32 size;
#endif
    gcmDUMP_API("$VX vxCreateWeightsBiasesParameterFromStream: context=%p, weights_biases_stream=%p", context, weights_biases_stream);

    if (!vxoContext_IsValid(context)) return VX_NULL;

    padXLeft = readBits(&streamBufferPtr, &bitOffset, 32);
    padXRight = readBits(&streamBufferPtr, &bitOffset, 32);
    padYTop = readBits(&streamBufferPtr, &bitOffset, 32);
    padYBottom = readBits(&streamBufferPtr, &bitOffset, 32);
    poolingSizeX = readBits(&streamBufferPtr, &bitOffset, 32);
    poolingSizeY = readBits(&streamBufferPtr, &bitOffset, 32);
    downScaleSizeRounding = readBits(&streamBufferPtr, &bitOffset, 32);
    weightsNumOfDims = readBits(&streamBufferPtr, &bitOffset, 32);
    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        weightsDims[i] = readBits(&streamBufferPtr, &bitOffset, 32);
        weightsOrgDims[i] = readBits(&streamBufferPtr, &bitOffset, 32);
    }
    weightsDataFormat = readBits(&streamBufferPtr, &bitOffset, 32);
    weightsQuantFormat = readBits(&streamBufferPtr, &bitOffset, 32);
    weightsFixedPointPos = (vx_int8)readBits(&streamBufferPtr, &bitOffset, 32);
    biasesNumOfDims = readBits(&streamBufferPtr, &bitOffset, 32);
    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        biasesDims[i] = readBits(&streamBufferPtr, &bitOffset, 32);
    }
    biasesDataFormat = readBits(&streamBufferPtr, &bitOffset, 32);
    biasesQuantFormat = readBits(&streamBufferPtr, &bitOffset, 32);
    biasesFixedPointPos = (vx_int8)readBits(&streamBufferPtr, &bitOffset, 32);

    layerType = readBits(&streamBufferPtr, &bitOffset, 32);
    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        weightsSubDims[i] = readBits(&streamBufferPtr, &bitOffset, 32);
        biasesSubDims[i] = readBits(&streamBufferPtr, &bitOffset, 32);
    }
    rawDataSize = readBits(&streamBufferPtr, &bitOffset, 32);

    wb = _createWeightsBiasesParameterFromParams(
            context,
            layerType,
            VX_NULL,
            padXLeft,
            padXRight,
            padYTop,
            padYBottom,
            poolingSizeX,
            poolingSizeY,
            downScaleSizeRounding,
            VX_NULL,
            VX_NULL,
            weightsNumOfDims,
            weightsDims,
            weightsSubDims,
            weightsDataFormat,
            weightsQuantFormat,
            weightsFixedPointPos,
            biasesNumOfDims,
            biasesDims,
            biasesSubDims,
            biasesDataFormat,
            biasesQuantFormat,
            biasesFixedPointPos);

    if (vxoReference_GetStatus((vx_reference)wb) != VX_SUCCESS) return VX_NULL;

    wb->wb_base->strideX = readBits(&streamBufferPtr, &bitOffset, 32);
    wb->wb_base->strideY = readBits(&streamBufferPtr, &bitOffset, 32);
    wb->wb_base->pooling_stride = readBits(&streamBufferPtr, &bitOffset, 32);
    wb->wb_base->setZeroLength = (vx_int8)readBits(&streamBufferPtr, &bitOffset, 32);
    wb->wb_base->inputZP = readBits(&streamBufferPtr, &bitOffset, 32);
    wb->wb_base->coefZP = readBits(&streamBufferPtr, &bitOffset, 32);
    *(vx_uint32 *)&wb->wb_base->coefScale = readBits(&streamBufferPtr, &bitOffset, 32);
    *(vx_uint32 *)&wb->wb_base->biasScale = readBits(&streamBufferPtr, &bitOffset, 32);
    wb->wb_base->skipValue = readBits(&streamBufferPtr, &bitOffset, 32);

    wb->wb_base->nn_fc_batch_mode = (vx_bool)readBits(&streamBufferPtr, &bitOffset, 32);
    wb->wb_base->do_fisrt_pixel_pool = (vx_bool)readBits(&streamBufferPtr, &bitOffset, 32);
    wb->wb_base->do_1xN = (vx_bool)readBits(&streamBufferPtr, &bitOffset, 32);
    wb->wb_base->do_zdp_opt = (vx_bool)readBits(&streamBufferPtr, &bitOffset, 32);

    wb->use_tp_fc = (vx_bool)readBits(&streamBufferPtr, &bitOffset, 32);
    wb->slice_num = readBits(&streamBufferPtr, &bitOffset, 32);

    wb->slice_array = (vx_weights_biases_slice)vxAllocateAndZeroMemory(sizeof(vx_weights_biases_slice_s) * wb->slice_num);
    if (wb->slice_array == VX_NULL)
    {
        status = VX_ERROR_NO_MEMORY;
        goto exit;
    }

    for (i = 0; i < wb->slice_num; i++)
    {
        wb->slice_array[i].kz_count = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->slice_array[i].z_count = readBits(&streamBufferPtr, &bitOffset, 32);

        wb->slice_array[i].memory_offset = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->slice_array[i].memory_size = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->slice_array[i].kernel_orig_size = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->slice_array[i].kernel_stream_size = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->slice_array[i].kernel_align_stream_size = readBits(&streamBufferPtr, &bitOffset, 32);

        wb->slice_array[i].non_zero_count = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->slice_array[i].reserve_weight_count = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->slice_array[i].all_count = readBits(&streamBufferPtr, &bitOffset, 32);
    }

    if (!WeightBiasBufferAllocate(context, wb, rawDataSize))
    {
        status = VX_ERROR_NO_MEMORY;
        goto exit;
    }

    if (readBits(&streamBufferPtr, &bitOffset, 32) == 1)
    {
        sub_wb_memory_size = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->sub_wb_vdata = (vx_sub_wb_vdata)vxAllocateAndZeroMemory(sizeof(vx_sub_wb_vdata_s));
        if (wb->sub_wb_vdata == VX_NULL)
        {
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }

        wb->sub_wb_vdata->wb_memory_size = sub_wb_memory_size;
        if ((wb->sub_wb_vdata->wb_memory_ptr = (vx_uint8_ptr)vxAllocate(wb->sub_wb_vdata->wb_memory_size)) == VX_NULL)
        {
            status =  VX_ERROR_NO_MEMORY;
            goto exit;
        }
        streamBufferPtr++;
        memcpy(wb->sub_wb_vdata->wb_memory_ptr, streamBufferPtr, wb->sub_wb_vdata->wb_memory_size);
        streamBufferPtr = (vx_uint32*)((vx_uint8*)streamBufferPtr + wb->sub_wb_vdata->wb_memory_size);
        bitOffset = 0; /*clear bitOffset flag after memcpy to fix the current kernelBufferPtr in next readBits() */
        wb->sub_wb_vdata->slice_num = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->sub_wb_vdata->slice_array = (vx_weights_biases_slice)vxAllocateAndZeroMemory(sizeof(vx_weights_biases_slice_s) * wb->sub_wb_vdata->slice_num);
        if (wb->sub_wb_vdata->slice_array == VX_NULL)
        {
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }
        for (i = 0; i < wb->sub_wb_vdata->slice_num; i++)
        {
            wb->sub_wb_vdata->slice_array[i].kz_count = readBits(&streamBufferPtr, &bitOffset, 32);
            wb->sub_wb_vdata->slice_array[i].z_count = readBits(&streamBufferPtr, &bitOffset, 32);

            wb->sub_wb_vdata->slice_array[i].memory_offset = readBits(&streamBufferPtr, &bitOffset, 32);
            wb->sub_wb_vdata->slice_array[i].memory_size = readBits(&streamBufferPtr, &bitOffset, 32);
            wb->sub_wb_vdata->slice_array[i].kernel_orig_size = readBits(&streamBufferPtr, &bitOffset, 32);
            wb->sub_wb_vdata->slice_array[i].kernel_stream_size = readBits(&streamBufferPtr, &bitOffset, 32);
            wb->sub_wb_vdata->slice_array[i].kernel_align_stream_size = readBits(&streamBufferPtr, &bitOffset, 32);

            wb->sub_wb_vdata->slice_array[i].non_zero_count = readBits(&streamBufferPtr, &bitOffset, 32);
            wb->sub_wb_vdata->slice_array[i].reserve_weight_count = readBits(&streamBufferPtr, &bitOffset, 32);
            wb->sub_wb_vdata->slice_array[i].all_count = readBits(&streamBufferPtr, &bitOffset, 32);
        }
        wb->sub_wb_vdata->kernel_per_core = readBits(&streamBufferPtr, &bitOffset, 32);
    }

    /* support multiVIP */
    wb->mGpuWBCount = readBits(&streamBufferPtr, &bitOffset, 32);

    if (readBits(&streamBufferPtr, &bitOffset, 32) == 1)
    {
        wb->archPerfHandle = (vx_arch_perf)vxAllocateAndZeroMemory(sizeof(vx_arch_perf_s));
        if (wb->archPerfHandle == VX_NULL)
        {
            status =  VX_ERROR_NO_MEMORY;
            goto exit;
        }
        /*archPerfor base: 7*/
        wb->archPerfHandle->calculated = (vx_bool)readBits(&streamBufferPtr, &bitOffset, 32);
        *(vx_uint32 *)&wb->archPerfHandle->coefNonZeroRatio = readBits(&streamBufferPtr, &bitOffset, 32);
        *((vx_uint32 *)&wb->archPerfHandle->coefNonZeroRatio + 1) = readBits(&streamBufferPtr, &bitOffset, 32);
        *(vx_uint32 *)&wb->archPerfHandle->coefCompressRatio = readBits(&streamBufferPtr, &bitOffset, 32);
        *((vx_uint32 *)&wb->archPerfHandle->coefCompressRatio + 1) = readBits(&streamBufferPtr, &bitOffset, 32);
        *(vx_uint32 *)&wb->archPerfHandle->imageCompressRatio = readBits(&streamBufferPtr, &bitOffset, 32);
        *((vx_uint32 *)&wb->archPerfHandle->imageCompressRatio + 1) = readBits(&streamBufferPtr, &bitOffset, 32);
        *(vx_uint32 *)&wb->archPerfHandle->imageNonZeroRatio = readBits(&streamBufferPtr, &bitOffset, 32);
        *((vx_uint32 *)&wb->archPerfHandle->imageNonZeroRatio + 1) = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->opType = (vxnne_operator_e)readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->opTarget = (vxnne_operation_target_e)readBits(&streamBufferPtr, &bitOffset, 32);

        /*archPerfor info: 27*/
        wb->archPerfHandle->info.kx = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.ky = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.kz = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.oinx = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.oiny = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.oinz = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.inx = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.iny = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.inz = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.outx = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.outy = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.outz = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.stridex = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.stridey = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.inputDataSize = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.outputDataSize = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.poolingSize = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.poolingStride = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.xOffSet = (vx_int32)readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.yOffSet = (vx_int32)readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.inputDataFormat = (vx_int32)readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.outputDataFormat = (vx_int32)readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.nnCores = (vx_int32)readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.convOutFifoDepth = (vx_int32)readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.kernelSize = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.pix = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.piy = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.p3 = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.nextKY = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->info.flush = (vx_int32)readBits(&streamBufferPtr, &bitOffset, 32);

        /*archPerfor swTilingInfo: 23*/
        wb->archPerfHandle->swTilingInfo.srcBuf = (vx_uint8)readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->swTilingInfo.dstBuf = (vx_uint8)readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->swTilingInfo.kernelBuf = (vx_uint8)readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->swTilingInfo.cacheSpace = (vx_int32)readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->swTilingInfo.origInX = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->swTilingInfo.origInY = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->swTilingInfo.origOutX = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->swTilingInfo.origOutY = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->swTilingInfo.origOutZ = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->swTilingInfo.outImageStride = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->swTilingInfo.outImageSlice = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->swTilingInfo.kernelCacheMode = (vx_enum)readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->swTilingInfo.imageCacheMode = (vx_enum)readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->swTilingInfo.calcNonFirstCmd = (vx_bool)readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->swTilingInfo.isNonFirstComputeBottleNeck = (vx_bool)readBits(&streamBufferPtr, &bitOffset, 32);
        *(vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstCycleCount = readBits(&streamBufferPtr, &bitOffset, 32);
        *((vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstCycleCount + 1) = readBits(&streamBufferPtr, &bitOffset, 32);
        *(vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstKernelReadBandWidth = readBits(&streamBufferPtr, &bitOffset, 32);
        *((vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstKernelReadBandWidth + 1) = readBits(&streamBufferPtr, &bitOffset, 32);
        *(vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstInImageReadBandWidth = readBits(&streamBufferPtr, &bitOffset, 32);
        *((vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstInImageReadBandWidth + 1) = readBits(&streamBufferPtr, &bitOffset, 32);
        *(vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstReadBandWidth = readBits(&streamBufferPtr, &bitOffset, 32);
        *((vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstReadBandWidth + 1) = readBits(&streamBufferPtr, &bitOffset, 32);
        *(vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstWriteBandWidth = readBits(&streamBufferPtr, &bitOffset, 32);
        *((vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstWriteBandWidth + 1) = readBits(&streamBufferPtr, &bitOffset, 32);
        *(vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstAXIReadBandWidth = readBits(&streamBufferPtr, &bitOffset, 32);
        *((vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstAXIReadBandWidth + 1) = readBits(&streamBufferPtr, &bitOffset, 32);
        *(vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstAXIWriteBandWidth = readBits(&streamBufferPtr, &bitOffset, 32);
        *((vx_uint32 *)&wb->archPerfHandle->swTilingInfo.perfNonFirstAXIWriteBandWidth + 1) = readBits(&streamBufferPtr, &bitOffset, 32);

        /*archPerfor resultInfo: 17*/
        wb->archPerfHandle->resultInfo.isFirstComputeBottleNeck = (vx_bool)readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->resultInfo.calculated = (vx_uint8)readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->resultInfo.kernelsPerCore = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->resultInfo.outImageTileXSize = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->resultInfo.outImageTileYSize = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->resultInfo.interleaveMode = readBits(&streamBufferPtr, &bitOffset, 32);
        wb->archPerfHandle->resultInfo.nnCoreCount = readBits(&streamBufferPtr, &bitOffset, 32);
        *(vx_uint32 *)&wb->archPerfHandle->resultInfo.perfCycleCount = readBits(&streamBufferPtr, &bitOffset, 32);
        *((vx_uint32 *)&wb->archPerfHandle->resultInfo.perfCycleCount + 1) = readBits(&streamBufferPtr, &bitOffset, 32);
        *(vx_uint32 *)&wb->archPerfHandle->resultInfo.perfReadBandWidth = readBits(&streamBufferPtr, &bitOffset, 32);
        *((vx_uint32 *)&wb->archPerfHandle->resultInfo.perfReadBandWidth + 1) = readBits(&streamBufferPtr, &bitOffset, 32);
        *(vx_uint32 *)&wb->archPerfHandle->resultInfo.perfWriteBandWidth = readBits(&streamBufferPtr, &bitOffset, 32);
        *((vx_uint32 *)&wb->archPerfHandle->resultInfo.perfWriteBandWidth + 1) = readBits(&streamBufferPtr, &bitOffset, 32);
        *(vx_uint32 *)&wb->archPerfHandle->resultInfo.perfAXIReadBandWidth = readBits(&streamBufferPtr, &bitOffset, 32);
        *((vx_uint32 *)&wb->archPerfHandle->resultInfo.perfAXIReadBandWidth + 1) = readBits(&streamBufferPtr, &bitOffset, 32);
        *(vx_uint32 *)&wb->archPerfHandle->resultInfo.perfAXIWriteBandWidth = readBits(&streamBufferPtr, &bitOffset, 32);
        *((vx_uint32 *)&wb->archPerfHandle->resultInfo.perfAXIWriteBandWidth + 1) = readBits(&streamBufferPtr, &bitOffset, 32);
        *(vx_uint32 *)&wb->archPerfHandle->resultInfo.perfKernelReadBandWidth = readBits(&streamBufferPtr, &bitOffset, 32);
        *((vx_uint32 *)&wb->archPerfHandle->resultInfo.perfKernelReadBandWidth + 1) = readBits(&streamBufferPtr, &bitOffset, 32);
        *(vx_uint32 *)&wb->archPerfHandle->resultInfo.perfInImageReadBandWidth = readBits(&streamBufferPtr, &bitOffset, 32);
        *((vx_uint32 *)&wb->archPerfHandle->resultInfo.perfInImageReadBandWidth + 1) = readBits(&streamBufferPtr, &bitOffset, 32);
        streamBufferPtr++;
        wb->non_zero_ratio = wb->archPerfHandle->coefNonZeroRatio;
        wb->general_compression_ratio = wb->archPerfHandle->coefCompressRatio;
    }
    else
        streamBufferPtr++;

    /* Copy kernel stream data*/
    memcpy(WB_MEM_LOGICAL_BASE_ADDR(wb), streamBufferPtr, WB_RAW_DATA_SIZE(wb));

#if gcdDUMP
    gcmDUMP(gcvNULL, "#[weights and biases]\n");
    gcmDUMP_BUFFER(gcvNULL,
                   gcvDUMP_BUFFER_MEMORY,
                   WB_MEM_PHYSICAL_BASE_ADDR(wb),
                   (gctPOINTER)WB_MEM_LOGICAL_BASE_ADDR(wb),
                   0,
                   WB_RAW_DATA_SIZE(wb));
#endif

#if DUMP_OFFLINE
    sprintf(fileName, "weightsBias_buffer_offline_lev%d.dat", idx++);
    outputFile = fopen(fileName,"wb");
    if (outputFile == NULL)
    {
        vxError("can't find file %s", fileName);
    }
    size = (vx_uint32)((vx_uint8*)streamBufferPtr - (vx_uint8*)base) + (vx_uint32)wb->sub_wb_memory_size;
    fwrite(base, size, 1, outputFile);
    fclose(outputFile);
#endif

    /* support multiVIP */
    if (wb->mGpuWBCount > 0)
    {
        vx_weights_biases_parameter WBTable[gcdMAX_3DGPU_COUNT];
        vx_uint32 i = 0;
        vx_uint32 size = vxoWeightsBiasesParameterStreamSize(wb, vx_false_e);
        vx_uint32 allocateSize = sizeof(vx_weights_biases_parameter) * wb->mGpuWBCount;
        /* this memeory be released in WeightsBiases_Destroy*/
        wb->mGpuWBTable = (vx_weights_biases_parameter*)vxAllocateAndZeroMemory((vx_size)allocateSize);

        streamBasePtr += size;
        vxmASSERT(wb->mGpuWBCount <= gcdMAX_3DGPU_COUNT);
        for (i = 0; i < wb->mGpuWBCount; i++)
        {
            WBTable[i] = vxCreateWeightsBiasesParameterFromStream(context, (vx_uint32*)(streamBasePtr));
            size = vxoWeightsBiasesParameterStreamSize(WBTable[i], vx_false_e);
            streamBasePtr += size;
        }

        for (i = 0; i < wb->mGpuWBCount; i++)
        {
            wb->mGpuWBTable[i] = WBTable[i];
        }
    }

exit:
    if (status == VX_SUCCESS)
        return wb;
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
            vxoTensor_ReleaseTensor(&layer->temp_tensors[i]);
        }
    }

    for (i = 0; i < VX_MAX_TEMP_ARRAYS; i++)
    {
        if (layer->temp_arrays[i] != VX_NULL)
        {
            vxReleaseArray(&layer->temp_arrays[i]);
        }
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

vx_status vxnneLayer_Free(struct _vxnne_layer_s* layer)
{
    layer->deinitialize(layer);

    gcoOS_Free(gcvNULL, layer);

    return VX_SUCCESS;
}

vx_status vxnneLayer_Initialize(
    vxnne_layer                 layer,
    vx_char                     *name,
    vx_node                     node,
    vx_uint32                   max_num_operations,
    vxnne_operation             *operations,
    vxnne_layer_deinitialize_f  deinitialize
    )
{
    layer->name         = name;
    layer->node         = node;
    layer->operations   =  operations;
    layer->num_temp_tensors      = 0;
    layer->dump                  = VX_NULL;
    layer->deinitialize          = (deinitialize ? deinitialize :  vxnneLayer_Deinitialize);
    layer->num_operations        = 0;
    layer->max_num_operations    = max_num_operations;

    return VX_SUCCESS;
}

vx_status vxnneLayer_SetOperation(
    vxnne_layer layer,
    vxnne_operation operation,
    vx_uint32 index
    )
{
    vxmASSERT(index < layer->max_num_operations);
    if (layer->operations[index])
    {
        vxError("layer[%d] %dth operation is overwritten", layer->node->id, index);
    }
    if (layer->num_operations < (index + 1))
    {
        layer->num_operations = index + 1;
    }
    layer->operations[index] = operation;
    operation->id = index;

    return VX_SUCCESS;
}

vx_status vxnneOperation_Deinitialize(vxnne_operation_s *operation)
{

    return VX_SUCCESS;
}

vx_status vxnneOperation_TP_Deinitialize(vxnne_operation_s *operation)
{
    if (operation->parameter.data_buff != VX_NULL)
    {
        vxoTensor_ReleaseTensor(&operation->parameter.data_buff);
    }

    if (operation->parameter.tp_value != VX_NULL)
    {
        if (operation->parameter.tp_value->p8[0] != VX_NULL)
        {
            vxFree(operation->parameter.tp_value->p8[0]);
            operation->parameter.tp_value->p8[0] = VX_NULL;
        }

        vxFree(operation->parameter.tp_value);
        operation->parameter.tp_value = VX_NULL;
    }

    vxnneOperation_Deinitialize(operation);
    return VX_SUCCESS;
}


vx_status vxnneOperation_Initialize(
                vxnne_operation_s               *operation,
                vxnne_layer                     layer,
                vxnne_operation_target_e        target,
                vxnne_operator_e                operatorType,
                vxnne_operation_execute_f       execute,
                vxnne_operation_deinitialize_f  deinitialize,
                vx_uint32                       batchCount,
                vx_uint32                       cmdBuffSize
                )
{
    vx_context context       = layer->node->base.context;

    operation->layer         = layer;
    operation->target        = target;
    operation->operatorType = operatorType;
    operation->execute       = execute;
    operation->initialize    = VX_NULL;
    operation->deinitialize  = (deinitialize ? deinitialize :  vxnneOperation_Deinitialize);
    operation->dump          = VX_NULL;

    operation->inputs        = &operation->references[0];
    operation->outputs       = &operation->references[VX_MAX_OPERTAION_INPUTS_OUTPUTS];
    operation->generics      = &operation->references[VX_MAX_OPERTAION_PARAMETERS-VX_MAX_OPERTAION_GENERICS];

    if (target == VXNNE_OPERATION_TARGET_SW)
    {
        layer->hasCPUFunction = vx_true_e;
    }

    operation->batchCount = batchCount;

    gcmASSERT(operation->batchCount > 0);

    memset(&operation->parameter, 0, sizeof(vx_op_param_s));


    if (context->options.enablePrintOperaTarget)
    {
        vxInfo("layer name %s, operation type %s, operation target %s\n", layer->name, vxnneGetOperatorTypeName(operatorType), vxnneGetOperatorTargetName(target));
    }

    return VX_SUCCESS;
}

vx_status vxnneOperation_AddReference(
    vxnne_operation_s*            operation,
    vx_reference                  reference,
    vxnne_operation_reference_e   refType
    )
{
    if (reference == VX_NULL || (reference->type != VX_TYPE_TENSOR && reference->type != VX_TYPE_IMAGE))
    {
        return VX_FAILURE;
    }

    if (refType == VXNNE_OPERATION_REFENRENCE_ONETIME)
    {
        if (operation->onetimeRefsNum == VX_MAX_OPERTAION_GENERICS)
            return VX_ERROR_NO_RESOURCES;
        operation->onetimeRefs[operation->onetimeRefsNum++] = reference;
    }

    switch (refType)
    {
        case VXNNE_OPERATION_REFENRENCE_INPUT:
            if (operation->inputsNum == VX_MAX_OPERTAION_INPUTS_OUTPUTS)
                return VX_ERROR_NO_RESOURCES;
            operation->inputs[operation->inputsNum++] = reference;
            break;

        case VXNNE_OPERATION_REFENRENCE_OUTPUT:
            if (operation->outputsNum == VX_MAX_OPERTAION_INPUTS_OUTPUTS)
                return VX_ERROR_NO_RESOURCES;
            operation->outputs[operation->outputsNum++] = reference;
            break;

        case VXNNE_OPERATION_REFENRENCE_GENERIC:
            if (operation->genericNum == VX_MAX_OPERTAION_GENERICS)
                return VX_ERROR_NO_RESOURCES;
            operation->generics[operation->genericNum++] = reference;
            break;

        default:
            return VX_ERROR_NO_RESOURCES;
    }

    operation->refNum++;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxnneShaderOperation_Execute(vxnne_operation_s *operation)
{
    vx_status status;
    vx_uint32 i;
    vxnne_shader_operation shaderOperation  = (vxnne_shader_operation)operation;
    vx_shader kernelShader;
    vx_graph graph = shaderOperation->base.layer->node->graph;
    gctUINT8 *stateBuffer = VX_NULL;

    kernelShader = shaderOperation->shaderExecutable->kernelShader;

    vxmONERROR(vxoShader_SetParameters(kernelShader,
                                       shaderOperation->shaderExecutable->params != VX_NULL ?
                                           shaderOperation->shaderExecutable->params : shaderOperation->shaderExecutable->param,
                                       shaderOperation->shaderExecutable->paramNum,
                                       shaderOperation->shaderExecutable->datatypes != VX_NULL ?
                                           shaderOperation->shaderExecutable->datatypes : VX_NULL,
                                       shaderOperation->shaderExecutable->attribue));

    for(i = 0; i < shaderOperation->shaderExecutable->uniformCount; i++)
    {
        vxmONERROR(vxoShader_SetUniform(
                        kernelShader,
                        shaderOperation->shaderExecutable->uniforms[i].name,
                        shaderOperation->shaderExecutable->uniforms[i].count,
                        shaderOperation->shaderExecutable->uniforms[i].data));
    }

    if (graph->binarySave)
    {
        gctPOINTER pointer;
        vx_binary_save binarySave = graph->binarySave;
        vxmONERROR(gcoOS_Allocate(gcvNULL, VX_MAX_SH_OPERATION_STATE_SIZE, (gctPOINTER *)&pointer));
        stateBuffer = (gctUINT8_PTR)pointer;
        if (binarySave->waitCommandsSize > 0) {
            /* append wait commands */
            vxMemCopy(stateBuffer, binarySave->waitCommands, binarySave->waitCommandsSize);
        }
        status = gcfVX_CaptureState(stateBuffer + binarySave->waitCommandsSize,
                                    VX_MAX_SH_OPERATION_STATE_SIZE,
                                    gcvNULL,
                                    gcvTRUE, gcvFALSE);
        if (status != VX_SUCCESS)
        {
            vxError("fail to capture shader states\n");
            vxmONERROR(VX_FAILURE);
        }
    }

    status = vxoShader_Execute(shaderOperation->base.layer->node,
                               kernelShader,
                               &shaderOperation->shaderExecutable->borderMode,
                               &shaderOperation->shaderExecutable->shaderParam,
                               operation->currBatchIndex);

    if (graph->binarySave)
    {
        vx_node node = shaderOperation->base.layer->node;
        vx_reference *shParams = VX_NULL;
        gctUINT32 actualSize = 0;
        vx_binary_save binarySave = graph->binarySave;

        status = gcfVX_CaptureState(gcvNULL, 0, &actualSize, gcvFALSE, gcvFALSE);
        if (actualSize <= 0)
        {
            vxError("error: fail to save layer name : %s to binary in shader operation\n", node->layer->name);
            vxmONERROR(VX_FAILURE);
        }

        if (VXNNE_OPERATOR_USER_VXC == operation->operatorType)
        {
            shParams = shaderOperation->shaderExecutable->params;
        }
        else
        {
            shParams = shaderOperation->shaderExecutable->param;
        }
        vxmONERROR(vxoBinaryGraph_SaveShaderOperation(node, &shaderOperation->base, kernelShader,
                                                    shParams,
                                                    shaderOperation->shaderExecutable->paramNum,
                                                    stateBuffer, actualSize + binarySave->waitCommandsSize,
                                                    operation->currBatchIndex));
        binarySave->waitCommandsSize = 0;
    }

OnError:
    if (stateBuffer != VX_NULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, stateBuffer));
    }

    return status;
}

vx_status vxnneShaderExecutable_Destroy(vxnne_shader_executable shaderExecutable)
{
    vx_uint32 i;

    for (i = 0; i < shaderExecutable->paramNum; i++)
    {
        if (shaderExecutable->param[i] != VX_NULL)
        {
            vxoReference_Release(&shaderExecutable->param[i], shaderExecutable->param[i]->type, VX_REF_INTERNAL);
        }
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

vx_status vxnneShaderOperation_Initialize(
    vxnne_shader_operation_s            *operation,
    vxnne_layer                         layer,
    vxnne_operator_e                    operatorType,
    vx_uint32                           batchCount,
    vxnne_shader_executable             shaderExecutable
    )
{
    vx_context context              = layer->node->base.context;
    operation->base.layer           = layer;
    operation->base.dump            = VX_NULL;
    operation->base.execute         = vxnneShaderOperation_Execute;
    operation->base.operatorType    = operatorType;
    operation->base.deinitialize    = vxnneShaderOperation_Deinitialize;
    operation->base.target          = VXNNE_OPERATION_TARGET_SH;
    operation->base.batchCount      = batchCount;
    operation->shaderExecutable     = shaderExecutable;

    operation->base.inputs        = &operation->base.references[0];
    operation->base.outputs       = &operation->base.references[VX_MAX_OPERTAION_INPUTS_OUTPUTS];
    operation->base.generics      = &operation->base.references[VX_MAX_OPERTAION_PARAMETERS-VX_MAX_OPERTAION_GENERICS];

    gcmASSERT(operation->base.batchCount > 0);


    if (context->options.enablePrintOperaTarget)
    {
        vxInfo("layer name %s, operation type %s, operation target %s", layer->name, vxnneGetOperatorTypeName(operatorType), vxnneGetOperatorTargetName(VXNNE_OPERATION_TARGET_SH));
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

vx_status vxnneShaderExecutable_SetParametersEx(vxnne_shader_executable shaderExecutable, vx_reference parameters[], vx_enum datatypes[], vx_uint32 paramNum)
{
    shaderExecutable->params = parameters;
    shaderExecutable->datatypes = datatypes;
    shaderExecutable->paramNum = paramNum;

    return VX_SUCCESS;
}

vx_status vxnneShaderExecutable_SetParametersAttribute(vxnne_shader_executable shaderExecutable, vx_uint32 index, vx_bitfield attrib)
{
    if (index < VX_MAX_SHADER_PARAMETERS)
    {
        if (!(shaderExecutable->attribue[index] & VXNNE_SHADER_PARAMETERS_ATTRIBUTE_INPUT_BIT)
          && (attrib & VXNNE_SHADER_PARAMETERS_ATTRIBUTE_INPUT_BIT))
        {
            shaderExecutable->inputNum++;
        }

        if (!(shaderExecutable->attribue[index] & VXNNE_SHADER_PARAMETERS_ATTRIBUTE_OUTPUT_BIT)
          && (attrib & VXNNE_SHADER_PARAMETERS_ATTRIBUTE_OUTPUT_BIT))
        {
            shaderExecutable->outputNum++;
        }
        shaderExecutable->attribue[index] |= attrib;
        return VX_SUCCESS;
    }

    return VX_FAILURE;
}

vx_status vxnneShaderExecutable_SetParametersAttributes(vxnne_shader_executable shaderExecutable, vx_uint32 index[], vx_uint32 count, vx_bitfield attrib)
{
    vx_uint32 i;
    vx_status status;

    for (i = 0; i < count; i++)
    {
        status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, index[i], attrib);
        if (VX_FAILURE == status)
            return status;
    }

    return VX_SUCCESS;
}


vx_status vxnneShaderExecutable_SetExecutionParameters(vxnne_shader_executable shaderExecutable, vx_kernel_execution_parameters_t *shaderParam)
{
    vx_uint32 i;

    shaderExecutable->shaderParam = *shaderParam;

    for (i = 0; i < shaderExecutable->shaderParam.workDim; i++)
    {
        shaderExecutable->shaderParam.globalWorkScale[i] = gcmMAX(1, shaderExecutable->shaderParam.globalWorkScale[i]);
    }

    return VX_SUCCESS;
}

vx_status vxnneShaderExecutable_GetMaxWorkGroupSize(
    vxnne_shader_executable shaderExecutable,
    vx_uint32               *maxWorkGroupSize
    )
{
    if (shaderExecutable == NULL)
        return VX_FAILURE;

    *maxWorkGroupSize = (vx_uint32)shaderExecutable->kernelShader->maxWorkGroupSize;

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

vxnne_kernel_shaders vxnneGetKernelShadersByEnum(vx_context context, vx_enum kernelEnum)
{
    if (context->globalData->kernels[kernelEnum].kernelShader)
    {
        return &context->globalData->kernels[kernelEnum];
    }
    else
    {
        return VX_NULL;
    }
}

vxnne_kernel_shaders vxnneAddKernelShadersInProgram(vx_context context, vx_char* kernelName, vx_program program, vx_uint32  paramNum, vx_enum kernelEnum)
{
    vxnne_kernel_shaders kernel = &context->globalData->kernels[kernelEnum];

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


extern vx_bool vxoGetDataDivisors(vx_uint32 input_value, vx_uint32 *divisors, vx_uint32 gcd);
vx_bool vxoElementOptimization_GetTensorShape(vx_tensor input, vx_uint32 sizes[VX_CONTEXT_TENSOR_MAX_DIMENSION], vx_uint32 * num_of_dims)
{
    vx_status status            = VX_SUCCESS;
    vx_uint32 element_count     = 0;
    vx_uint32 i                 = 0;

    status = vxoTensor_GetTensorElementCount(input, &element_count);
    if (status != VX_SUCCESS) return vx_false_e;

    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        sizes[i] = 1;
   }


    if (element_count < IMG_MAX_WIDTH)
    {
        sizes[0] = element_count;

        *num_of_dims = 2;
    }
    else
    {
        vx_uint32 divisors = 1;
        for (i = 0; i < 2; i++)
        {
            divisors = 1;
            vxoGetDataDivisors(element_count, &divisors, 1);

            sizes[i] = divisors;
            element_count = element_count / divisors;
        }

        sizes[2] = element_count;
        *num_of_dims = 3;
    }

    return vx_true_e;
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
    vx_uint32 input_width, input_height, input_z;
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

    inImageTileSizeX = outTileX - 1 + kernel_x;
    inImageTileSizeY = outTileY - 1 + kernel_y;

    distSize = inImageTileSizeX * inImageTileSizeY * input_z * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
    numOfImageTileX = brickModeOperation->num_tile_x;
    numOfImageTileY = brickModeOperation->num_tile_y;

    temp_buffer = (vx_uint8_ptr)vxAllocateAndZeroMemory(distSize * numOfImageTileX * numOfImageTileY);

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP))
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
                                    srcOffsetSize = ((inX + inputs->viewRegion.viewStarts[0]) + input_width * ((inY + inputs->viewRegion.viewStarts[1]) + input_height * (z + inputs->viewRegion.viewStarts[2]))) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));


                                else
                                    srcOffsetSize = (inX + input_width * (inY + input_height * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                dstOffserSize = (dstX + tempX * (dstY + tempY * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                input_ptr = inputs->tensorBuffer->memory.logicals[0] + srcOffsetSize;
                                output_ptr = temp_buffer + dstOffserSize + offsetDist;
                                if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_FLOAT16)
                                    *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
                                else if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_INT8)
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
                                    srcOffsetSize = ((inX + inputs->viewRegion.viewStarts[0]) + input_width * ((inY + inputs->viewRegion.viewStarts[1]) + input_height * (z + inputs->viewRegion.viewStarts[2]))) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));

                                else
                                    srcOffsetSize = (inX + input_width * (inY + input_height * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                dstOffserSize = (dstX + tempX * (dstY + tempY * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                input_ptr = inputs->tensorBuffer->memory.logicals[0] + srcOffsetSize;
                                output_ptr = temp_buffer + dstOffserSize + offsetDist;

                                if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_FLOAT16)
                                    *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
                                else if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_INT8)
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
                                    srcOffsetSize = ((inX + inputs->viewRegion.viewStarts[0]) + input_width * ((inY + inputs->viewRegion.viewStarts[1]) + input_height * (z + inputs->viewRegion.viewStarts[2]))) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));

                                else
                                    srcOffsetSize = (inX + input_width * (inY + input_height * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                dstOffserSize = (dstX + tempX * (dstY + tempY * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                input_ptr = inputs->tensorBuffer->memory.logicals[0] + srcOffsetSize;
                                output_ptr = temp_buffer + dstOffserSize + offsetDist;

                                if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_FLOAT16)
                                    *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
                                else if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_INT8)
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
                                    srcOffsetSize = ((inX + inputs->viewRegion.viewStarts[0]) + input_width * ((inY + inputs->viewRegion.viewStarts[1]) + input_height * (z + inputs->viewRegion.viewStarts[2]))) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));

                                else
                                    srcOffsetSize = (inX + input_width * (inY + input_height * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                dstOffserSize = (dstX + tempX * (dstY + tempY * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                input_ptr = inputs->tensorBuffer->memory.logicals[0] + srcOffsetSize;
                                output_ptr = temp_buffer + dstOffserSize + offsetDist;

                                if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_FLOAT16)
                                    *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
                                else if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_INT8)
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
                                    srcOffsetSize = ((inX + inputs->viewRegion.viewStarts[0]) + input_width * ((inY + inputs->viewRegion.viewStarts[1]) + input_height * (z + inputs->viewRegion.viewStarts[2]))) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));

                                else
                                    srcOffsetSize = (inX + input_width * (inY + input_height * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                dstOffserSize = (dstX + inImageTileSizeX * (dstY + tempY * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                input_ptr = inputs->tensorBuffer->memory.logicals[0] + srcOffsetSize;
                                output_ptr = temp_buffer + dstOffserSize + offsetDist;

                                if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_FLOAT16)
                                    *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
                                else if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_INT8)
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
                                    srcOffsetSize = ((inX + inputs->viewRegion.viewStarts[0]) + input_width * ((inY + inputs->viewRegion.viewStarts[1]) + input_height * (z + inputs->viewRegion.viewStarts[2]))) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));

                                else
                                    srcOffsetSize = (inX + input_width * (inY + input_height * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                dstOffserSize = (dstX + inImageTileSizeX * (dstY + tempY * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                input_ptr = inputs->tensorBuffer->memory.logicals[0] + srcOffsetSize;
                                output_ptr = temp_buffer + dstOffserSize + offsetDist;

                                if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_FLOAT16)
                                    *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
                                else if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_INT8)
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
                                    srcOffsetSize = ((inX + inputs->viewRegion.viewStarts[0]) + input_width * ((inY + inputs->viewRegion.viewStarts[1]) + input_height * (z + inputs->viewRegion.viewStarts[2]))) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));

                                else
                                    srcOffsetSize = (inX + input_width * (inY + input_height * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                dstOffserSize = (dstX + tempX * (dstY + inImageTileSizeY * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                input_ptr = inputs->tensorBuffer->memory.logicals[0] + srcOffsetSize;
                                output_ptr = temp_buffer + dstOffserSize + offsetDist;

                                if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_FLOAT16)
                                    *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
                                else if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_INT8)
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
                                    srcOffsetSize = ((inX + inputs->viewRegion.viewStarts[0]) + input_width * ((inY + inputs->viewRegion.viewStarts[1]) + input_height * (z + inputs->viewRegion.viewStarts[2]))) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));

                                else
                                    srcOffsetSize = (inX + input_width * (inY + input_height * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                dstOffserSize = (dstX + tempX * (dstY + inImageTileSizeY * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                input_ptr = inputs->tensorBuffer->memory.logicals[0] + srcOffsetSize;
                                output_ptr = temp_buffer + dstOffserSize + offsetDist;

                                if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_FLOAT16)
                                    *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
                                else if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_INT8)
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
                                    srcOffsetSize = ((inX + inputs->viewRegion.viewStarts[0]) + input_width * ((inY + inputs->viewRegion.viewStarts[1]) + input_height * (z + inputs->viewRegion.viewStarts[2]))) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));

                                else
                                    srcOffsetSize = (inX + input_width * (inY + input_height * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                if (inImageTileSizeX > input_width && inImageTileSizeY > input_height)
                                    dstOffserSize = (dstX + input_width * (dstY + input_height * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                else
                                    dstOffserSize = (dstX + inImageTileSizeX * (dstY + inImageTileSizeY * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                input_ptr = inputs->tensorBuffer->memory.logicals[0] + srcOffsetSize;
                                output_ptr = temp_buffer + dstOffserSize + offsetDist;

                                if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_FLOAT16)
                                    *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
                                else if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_INT8)
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
                                    srcOffsetSize = ((inX + inputs->viewRegion.viewStarts[0]) + input_width * ((inY + inputs->viewRegion.viewStarts[1]) + input_height * (z + inputs->viewRegion.viewStarts[2]))) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));

                                else
                                    srcOffsetSize = (inX + input_width * (inY + input_height * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                dstOffserSize = (dstX + tempX * (dstY + tempY * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                input_ptr = inputs->tensorBuffer->memory.logicals[0] + srcOffsetSize;
                                output_ptr = temp_buffer + dstOffserSize + offsetDist;

                                if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_FLOAT16)
                                    *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
                                else if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_INT8)
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
                                    srcOffsetSize = ((inX + inputs->viewRegion.viewStarts[0]) + input_width * ((inY + inputs->viewRegion.viewStarts[1]) + input_height * (z + inputs->viewRegion.viewStarts[2]))) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));

                                else
                                    srcOffsetSize = (inX + input_width * (inY + input_height * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                dstOffserSize = (dstX + inImageTileSizeX * (dstY + tempY * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                input_ptr = inputs->tensorBuffer->memory.logicals[0] + srcOffsetSize;
                                output_ptr = temp_buffer + dstOffserSize + offsetDist;

                                if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_FLOAT16)
                                    *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
                                else if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_INT8)
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
                                    srcOffsetSize = ((inX + inputs->viewRegion.viewStarts[0]) + input_width * ((inY + inputs->viewRegion.viewStarts[1]) + input_height * (z + inputs->viewRegion.viewStarts[2]))) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));

                                else
                                    srcOffsetSize = (inX + input_width * (inY + input_height * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                dstOffserSize = (dstX + tempX * (dstY + inImageTileSizeY * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                input_ptr = inputs->tensorBuffer->memory.logicals[0] + srcOffsetSize;
                                output_ptr = temp_buffer + dstOffserSize + offsetDist;

                                if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_FLOAT16)
                                    *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
                                else if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_INT8)
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
                                    srcOffsetSize = ((inX + inputs->viewRegion.viewStarts[0]) + input_width * ((inY + inputs->viewRegion.viewStarts[1]) + input_height * (z + inputs->viewRegion.viewStarts[2]))) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));

                                else
                                    srcOffsetSize = (inX + input_width * (inY + input_height * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                if (inImageTileSizeX > input_width && inImageTileSizeY > input_height)
                                    dstOffserSize = (dstX + input_width * (dstY + input_height * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                else
                                    dstOffserSize = (dstX + inImageTileSizeX * (dstY + inImageTileSizeY * z)) * vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs));
                                input_ptr = inputs->tensorBuffer->memory.logicals[0] + srcOffsetSize;
                                output_ptr = temp_buffer + dstOffserSize + offsetDist;

                                if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_FLOAT16)
                                    *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
                                else if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_INT8)
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

vx_status vxnneExecutionLayer_Create(vx_graph graph, vxnne_execution_layer* executionLayer)
{
    vxnne_execution_layer layer = VX_NULL;

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_execution_layer_s), (gctPOINTER*)&layer);
    if (!layer)
    {
        return VX_ERROR_NO_MEMORY;
    }
    gcoOS_ZeroMemory(layer, sizeof(vxnne_execution_layer_s));

    vxnneLayer_Initialize(&layer->base, "execution_layer", VX_NULL, 0, VX_NULL, vxnneExecutionLayer_Deinitialize);

    layer->base.execute = vxnneExecutionLayer_Execute;
    layer->graph = graph;

    *executionLayer = layer;

    return VX_SUCCESS;
}

vx_status vxnneExecutionLayer_GenerateCommands(vx_context context, vxnne_layer layer)
{
    vx_uint32               i;
    vx_status               status = VX_SUCCESS;
    vxnne_execution_layer   executionLayer = (vxnne_execution_layer)layer;

    for (i = 0; i < executionLayer->opIndicesNum; i++)
    {
        status = vxnneOperationCommand_GenerateCommands(context, &executionLayer->opIndices[i]);
        if (status != VX_SUCCESS)
        {
            goto OnError;
        }
    }

        /*used by swap handel*/
    vxo_insertHandel(executionLayer);

    /* nn and tp have saved done, this is for saving SH reset current offset */
    if (executionLayer->graph->binarySave)
    {
        executionLayer->graph->binarySave->lastOperation0ffset = executionLayer->graph->binarySave->currOperationOffset;
        executionLayer->graph->binarySave->currOperationOffset = 0;
    }

OnError:
    return status;
}

VX_PRIVATE_API vx_status vxnneOperation_ExecuteYUVScalerCommand(vx_node node, vxnne_operation operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_yuv2rgb_scale_operation scaleOp = (vxnne_yuv2rgb_scale_operation)operation;
    vx_uint32 imageInputWidth, imageInputHeight, i, splitCount;
    vx_uint32 outputWidth = TENSOR_VIEW_SIZE_INDEX(scaleOp->outputs, 0); /* x-axis*/
    vx_uint32 outputHeight = TENSOR_VIEW_SIZE_INDEX(scaleOp->outputs, 1); /* y-axis*/
    vx_rectangle_t rect;
    vx_uint32 inputStarts[gcdMAX_3DGPU_COUNT], inputSizes[gcdMAX_3DGPU_COUNT];
    vx_uint32 outputStarts[gcdMAX_3DGPU_COUNT], outputSizes[gcdMAX_3DGPU_COUNT];
    vx_uint16 inputInitErrors[gcdMAX_3DGPU_COUNT], inputInitIntErrors[gcdMAX_3DGPU_COUNT];
    vxnne_yuv2rgb_scale_operation_s SCOperation;

    vxmASSERT(VXNNE_OPERATOR_YUV2RGB_SCALE == operation->operatorType);

    if (node->kernel->signature.directionTable[0] == VX_INPUT &&
        node->kernel->signature.dataTypeTable[0] == VX_TYPE_IMAGE)
    {
        /* Get input image from node directly */
        scaleOp->inputs = (vx_image)node->paramTable[0];
    }

    if (node->kernel->signature.directionTable[1] == VX_INPUT &&
        node->kernel->signature.dataTypeTable[1] == VX_TYPE_ARRAY)
    {
        /* Get input rectangle from node directly */
        vx_array rects = (vx_array)node->paramTable[1];
        rect.start_x = *((vx_uint32_ptr)rects->memory.logicals[0] + 0);
        rect.start_y = *((vx_uint32_ptr)rects->memory.logicals[0] + 1);
        rect.end_x   = *((vx_uint32_ptr)rects->memory.logicals[0] + 2);
        rect.end_y   = *((vx_uint32_ptr)rects->memory.logicals[0] + 3);
    }
    else
    {
        rect = scaleOp->rect;
    }
    if (!rect.end_x || !rect.end_y || rect.end_x < rect.start_x || rect.end_y < rect.start_y)
    {
        vx_image image = scaleOp->inputs;
        if (image->region.start_x > image->region.end_x)
        {
            rect.start_x = 0;
            rect.end_x = image->memory.dims[0][VX_DIM_X];
        }
        else
        {
            rect.start_x = image->region.start_x;
            rect.end_x = image->region.end_x;
        }
        if (image->region.start_y > image->region.end_y)
        {
            rect.start_y = 0;
            rect.end_y = image->memory.dims[0][VX_DIM_Y];
        }
        else
        {
            rect.start_y = image->region.start_y;
            rect.end_y = image->region.end_y;
        }
    }
    scaleOp->rect = rect;

    imageInputWidth = rect.end_x - rect.start_x;
    imageInputHeight  = rect.end_y - rect.start_y;
    scaleOp->x_scale = (imageInputWidth << 15) / outputWidth;
    scaleOp->y_scale = (imageInputHeight << 15) / outputHeight;

    memcpy(&SCOperation, scaleOp, sizeof(vxnne_yuv2rgb_scale_operation_s));

    /* for X */
    splitCount = 1;
    vxmONERROR(vxnneComputeYUV2RGBInputParameter(outputWidth, scaleOp->x_scale, rect.start_x,
                                                 &splitCount,
                                                 outputStarts, outputSizes,
                                                 inputStarts, inputSizes,
                                                 inputInitErrors, inputInitIntErrors));

    SCOperation.rect.start_x = inputStarts[0];
    SCOperation.rect.end_x = SCOperation.rect.start_x + inputSizes[0];
    SCOperation.x_init_error = inputInitErrors[0];
    SCOperation.x_init_int_error = inputInitIntErrors[0];

    /* for split Y */
    if (node->base.context->options.enableMultiVIPCombined)
    {
        gcmONERROR(gcoVX_QueryCoreCount(node->graph->deviceID, &splitCount));
    }
    else
    {
        splitCount = 1;
    }
    vxmONERROR(vxnneComputeYUV2RGBInputParameter(outputHeight, scaleOp->y_scale, rect.start_y,
                                                 &splitCount,
                                                 outputStarts, outputSizes,
                                                 inputStarts, inputSizes,
                                                 inputInitErrors, inputInitIntErrors));

    for (i = 0; i < splitCount; i++)
    {
        vx_uint32 outputSizeStart, outputSizeEnd;

        outputSizeStart = outputStarts[i];
        outputSizeEnd = outputStarts[i] + outputSizes[i];

        SCOperation.outputs = scaleOp->outputs;
        SCOperation.rect.start_y = inputStarts[i];
        SCOperation.rect.end_y = SCOperation.rect.start_y + inputSizes[i];
        SCOperation.y_init_error = inputInitErrors[i];
        SCOperation.y_init_int_error = inputInitIntErrors[i];
        SCOperation.output_y_start = outputSizeStart;
        SCOperation.output_y_end = outputSizeEnd;

        SCOperation.base.gpuId = i;
        SCOperation.base.mGpuSync = i == splitCount - 1 ? vx_true_e : vx_false_e;

        operation->execute((vxnne_operation)&SCOperation);
    }

OnError:
    return status;
}

vx_status vxnneExecutionLayer_Execute(vxnne_layer layer)
{
    vx_uint32               i = 0, j = 0;
    vx_status               status = VX_SUCCESS;
    vxnne_execution_layer   executionLayer = (vxnne_execution_layer)layer;
    vxnne_operation         operation;
    vxnne_operation_target_e operationTarget;
    vx_graph graph = executionLayer->graph;
    vx_node node = VX_NULL;

    vxnneMultiChannel_GetCurrentChannel(&operationTarget);

    for (i = 0; i < executionLayer->opIndicesNum; i++)
    {
        gctUINT64 operatorStart = 0;
        vx_uint32 opertorExcuteTime = 0;
        vx_bool wait = vx_false_e;

        operation = executionLayer->operations[executionLayer->opIndices[i].operationID];

        node = ((operation)->layer)->node;
        if (node->kernel->isUserkernel)
        {
            vxoNode_EnableVirtualAccessible(node);
        }
        if (executionLayer->graph->verified == vx_false_e && executionLayer->graph->base.context->options.enableGraphCommandBuffer == vx_true_e &&
            i == 0 && operation->target == VXNNE_OPERATION_TARGET_SC)
        {
            executionLayer->graph->scalerHead = vx_true_e;
            continue;
        }
        else if (executionLayer->graph->verified == vx_true_e && executionLayer->graph->base.context->options.enableGraphCommandBuffer == vx_true_e &&
                 executionLayer->graph->scalerHead == vx_true_e && (i != 0 || operation->target != VXNNE_OPERATION_TARGET_SC))
        {
            continue;
        }

        vxnneMultiChannel_SetCurrentChannel(operation->target);

        vxnneMultiChannel_ApplySyncMode(executionLayer->opIndices[i].waitMode, executionLayer->opIndices[i].semaWaitHandle);

        if (vxoContext_IsFeatureAvailable(executionLayer->graph->base.context, VX_NN_TP_PARALLEL))
        {
            gctUINT32 actualSize = 0;
            vx_status status = VX_SUCCESS;
            if (graph->binarySave)
            {
                graph->binarySave->waitCommandsSize = 0;
                if ((operation->engineSync.waitCnt > 0) && (i > 0))
                {
                    status = gcfVX_CaptureState(graph->binarySave->waitCommands,
                                                VX_MAX_WAIT_STATE_SIZE,
                                                &actualSize,
                                                gcvTRUE,
                                                gcvFALSE);
                    if (status != VX_SUCCESS)
                    {
                        vxError("failed to capture wait commands\n");
                        vxmASSERT(0);
                    }
                }
            }

            /* th/nn/sh parallel wait event ID */
            if (i > 0)
            {
                for (j = 0; j < operation->engineSync.waitCnt; j++)
                {
                    gcoVX_WaitNNEvent(operation->engineSync.waitId[j]);
                    wait = vx_true_e;
                }
            }


            if ((graph->binarySave) && (operation->engineSync.waitCnt > 0) && (i > 0))
            {
                status = gcfVX_CaptureState(gcvNULL, 0, &actualSize, gcvFALSE, gcvFALSE);
                if (status != VX_SUCCESS)
                {
                    vxError("failed to capture wait commands end\n");
                    vxmASSERT(0);
                }
                graph->binarySave->waitCommandsSize = (vx_uint32)actualSize;
            }

            if (operation->target == VXNNE_OPERATION_TARGET_NN)
            {
                vxInfo("NN %2d eventId: %2d waitId: ", operation->absoluteOperationID, operation->engineSync.eventId[0]);
            }
            else if (operation->target == VXNNE_OPERATION_TARGET_TP)
            {
                vxInfo("TP %2d eventId: %2d waitId: ", operation->absoluteOperationID, operation->engineSync.eventId[operation->engineSync.eventCnt-1]);
            }
            else if (operation->target == VXNNE_OPERATION_TARGET_SH)
            {
                vxInfo("SH %2d eventId:  0 waitId: ", operation->absoluteOperationID);
            }

            for (j = 0; j < operation->engineSync.waitCnt; j++)
            {
                vxInfo("%2d ", operation->engineSync.waitId[j]);
            }
            if (!wait)
            {
                vxInfo("Skip");
            }
            vxInfo("\n");
        }

        vxnneBatch_SetCurrentBatchArray(operation, executionLayer->opIndices[i].batchID);

        if (executionLayer->graph->base.context->options.enableCNNPerf)
        {
            operatorStart = gcfVX_PerfStart((vx_reference)executionLayer->graph);
        }

#if gcdFRAMEINFO_STATISTIC
        {
            gctUINT32 drawID = (operation->id & 0x3FF)                          /* 10 bit operation ID */
                             | ((executionLayer->graph->graphID & 0x3F) << 10)  /* 6  bit graph ID */
                             | ((i & 0xFFFF) << 16);                            /* 16 bit operationIndex */
            gcoHAL_FrameInfoOps(gcvNULL, gcvFRAMEINFO_COMPUTE_NUM, gcvFRAMEINFO_OP_SET, &drawID);
        }
#endif
        if (executionLayer->opIndices[i].commandBuffer.logical)
        {
            if (executionLayer->opIndices[i].dump)
            {
                executionLayer->opIndices[i].dump(&executionLayer->opIndices[i], operation, VXNNE_DUMP_PRE_EXECUTION);
            }

            status = vxnneOperation_ExecuteCommands(operation, &executionLayer->opIndices[i].commandBuffer);

        }
        else
        {
            if (executionLayer->opIndices[i].dump)
            {
                executionLayer->opIndices[i].dump(&executionLayer->opIndices[i], operation, VXNNE_DUMP_PRE_EXECUTION);
            }

            if (operation->target == VXNNE_OPERATION_TARGET_SC)
            {
               status = vxnneOperation_ExecuteYUVScalerCommand(operation->layer->node, operation);
            }
            else
            {
                operation->execute(operation);
            }
        }

        if (vxoContext_IsFeatureAvailable(executionLayer->graph->base.context, VX_NN_TP_PARALLEL) &&
            operation->target == VXNNE_OPERATION_TARGET_SH && operation->engineSync.eventId[0] != 0)
        {
            gcoVX_FlushCache(vx_false_e, vx_false_e, vx_false_e, vx_false_e, vx_true_e, vx_false_e);
        }

        if (executionLayer->opIndices[i].dump)
        {
            executionLayer->opIndices[i].dump(&executionLayer->opIndices[i], operation, VXNNE_DUMP_POST_EXECUTION);
        }

        if (operation->layer->node->base.context->options.enableNNLayerDump)
        {
            gcfVX_Flush(gcvTRUE);
            vxnneOperation_NodeDump(&executionLayer->opIndices[i]);
        }

        vxnneMultiChannel_ApplySyncMode(executionLayer->opIndices[i].wakeMode, executionLayer->opIndices[i].semaWakeHandle);
        if (executionLayer->graph->base.context->options.enableCNNPerf)
        {
            vxInfo("layer id: %d layer name:%s operation[%d]:%s target:%s.\n",
                operation->layer->node->id,
                operation->layer->name,
                operation->id,
                vxnneGetOperatorTypeName(operation->operatorType),
                vxnneGetOperatorTargetName(operation->target));

            gcfVX_Flush(gcvTRUE);

            opertorExcuteTime = gcfVX_PerfEnd((vx_reference)executionLayer->graph, operatorStart);

            vxInfo("execution time:%10d us\n", opertorExcuteTime);

            /* Calculate non-zero ratio, only for TP target */
            if (operation->layer->node->base.context->options.enableNNLayerDump
               && (operation->target == VXNNE_OPERATION_TARGET_TP)
               && (operation->operatorType == VXNNE_OPERATOR_FULLYCONNECTED))
            {
                /* for non-zero ratio calculate */
                vx_tensor  calNonZeroRatioinputs = NULL;
                vx_uint32 inputSize = 0, index = 0,inputZeroCount = 0;
                vx_float32 inputNonZeroRatio = 0.0;
                vx_uint8_ptr inputAddress = NULL;

                calNonZeroRatioinputs = (vx_tensor)operation->inputs[0];
                vxoTensor_GetTensorSize(calNonZeroRatioinputs, &inputSize);
                /*vxInfo("tensor input size is:%d, zero point is %d.\n", inputSize,calNonZeroRatioinputs->zeroPoint);*/

                vxoTensor_GetTensorBatchArrayViewMemory(calNonZeroRatioinputs,operation->currBatchIndex,(gctPOINTER *)&inputAddress,NULL);
                /* find out zero count */
                if(inputSize != 0)
                {
                    /* calculate zero count */
                    for(index = 0; index< inputSize; index++)
                    {
                        if(inputAddress[index] == calNonZeroRatioinputs->zeroPoint)
                            inputZeroCount++;
                    }
                    /*vxInfo("tensor zero count is:%d.\n", inputZeroCount);*/

                    inputNonZeroRatio = 1.0f * (inputSize - inputZeroCount) / inputSize;
                    vxInfo("==layer_id: %d abs_op_id: %d imageNonZeroRatio: %.07f\n", operation->layer->node->id, operation->absoluteOperationID, inputNonZeroRatio);

                    /* clear input Zero count */
                    inputZeroCount = 0;
                }
            }
#if VIVANTE_PROFILER
            vxoProfiler_End((vx_reference)executionLayer->graph);
#endif
        }
        if (node->kernel->isUserkernel)
        {
            vxoNode_DisableVirtualAccessible(node);
        }
 #if VXM_FORCE_PER_OPERATION_IDLE
        else
        {
            gcoVX_Flush(gcvTRUE);

        }
#endif
    }

    if (graph->binarySave)
    {
        gcfVX_CaptureState(graph->binarySave->endCommands,
                            VX_MAX_INITIALIZE_COMMAND_SIZE,
                            &graph->binarySave->endCommandsSize,
                            gcvTRUE,
                            gcvFALSE);
    }

    vxnneMultiChannel_SetCurrentChannel(operationTarget);

    /* Dump all operation info if VIV_VX_OPS_DUMP_PATH enabled */
    vxnneOpDebug_DumpOperatoinInfos(layer);

    return status;
}

vx_status vxnneExecutionLayer_Deinitialize(vxnne_layer layer)
{
    vx_uint32 i;
    vxnne_execution_layer   executionLayer = (vxnne_execution_layer)layer;

    if (executionLayer->opIndices)
    {
        /* free command commandbuffer */
        for (i = 0; i < executionLayer->opIndicesNum; i++)
        {
            if (executionLayer->opIndices[i].commandBuffer.node)
            {
                gcoVX_FreeMemory(executionLayer->opIndices[i].commandBuffer.node);
                executionLayer->opIndices[i].commandBuffer.node = gcvNULL;
            }

            if (executionLayer->opIndices[i].commandBuffer.eventID)
            {
                vxFree(executionLayer->opIndices[i].commandBuffer.eventID);
                executionLayer->opIndices[i].commandBuffer.eventID = gcvNULL;
            }
        }

        gcoOS_Free(gcvNULL, executionLayer->opIndices);
        executionLayer->opIndicesNum    = 0;
        executionLayer->opIndices       = gcvNULL;
    }
    if (executionLayer->operations)
    {
        /* free opertions */
        gcoOS_Free(gcvNULL, executionLayer->operations);
        executionLayer->base.num_operations = 0;
        executionLayer->operations   = gcvNULL;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API
vx_status vxnneUserOperation_Execute(
    vxnne_operation operation
    )
{
    vx_node node = operation->layer->node;
    vx_status status = VX_SUCCESS;

    if (node->kernel->function)
    {
        vxmONERROR(node->kernel->function(node,
                                          node->paramTable,
                                          node->kernel->signature.paramCount));
    }

OnError:
    return status;
}

VX_PRIVATE_API
vx_status VX_CALLBACK vxnneUserNode_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = NULL;
    }

    return VX_SUCCESS;
}

vx_status vxnneWrapUserNode(
    vx_context context,
    vx_node node,
    vxnne_user_node_type_e userNodeType
    )
{
    vxnne_user_layer userLayer = gcvNULL;
    vx_uint32 operationIndex = 0;
    vx_uint32 i;

    vx_status status = VX_SUCCESS;

    if (!node || node->layer)
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    /* Create layer. */
    gcoOS_Allocate(gcvNULL, sizeof(vxnne_user_layer_s), (gctPOINTER*)&userLayer);
    if (!userLayer)
    {
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        return VX_ERROR_NO_MEMORY;
    }

    gcoOS_ZeroMemory(userLayer, sizeof(vxnne_user_layer_s));

    vxnneLayer_Initialize(&userLayer->base,
                          node->kernel->name,
                          node,
                          vxmOPERATION_COUNT(userLayer),
                          userLayer->operations,
                          VX_NULL);

    node->kernel->deinitializeWrapFunction = vxnneUserNode_Deinitializer;

    if (userNodeType == VXNNE_USER_NODE_TYPE_VXC)
    {
        vx_uint32 batchCount = 1;
        vxnne_shader_executable shaderExecutable = VX_NULL;

        shaderExecutable = vxnneGetUserShaderExecutable(node->base.context,
                                                        node->kernel,
                                                        node->paramTable,
                                                        node->kernel->signature.dataTypeTable,
                                                        node->kernel->signature.paramCount,
                                                        node->uniforms,
                                                        node->uniformCount,
                                                        &node->kernelAttributes.borderMode,
                                                        &node->kernelAttributes.shaderParameter);
        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto OnError;
        }

        if (node->kernel->signature.directionTable[0] == VX_INPUT &&
            node->kernel->signature.dataTypeTable[0] == VX_TYPE_TENSOR)
        {
            vx_tensor input = (vx_tensor)node->paramTable[0];
            batchCount = (TENSOR_SIZE_INDEX(input, 3) == 0) ? 1 : TENSOR_SIZE_INDEX(input, 3);
        }

        status = vxnneShaderOperation_Initialize(
                    &userLayer->user_shader_operation,
                    &userLayer->base,
                    VXNNE_OPERATOR_USER_VXC,
                    batchCount,
                    shaderExecutable);
        if (status != VX_SUCCESS)
        {
            goto OnError;
        }

        for (i = 0; i < node->kernel->signature.paramCount; i++)
        {
            if (node->kernel->signature.dataTypeTable[i] == VX_TYPE_TENSOR &&
                (TENSOR_SIZE_INDEX(((vx_tensor)(node->paramTable[i])), 3) == 0 || TENSOR_SIZE_INDEX(((vx_tensor)(node->paramTable[i])), 3) == 1))
            {
                vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, i, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NO_BATCH_BIT);
            }

            if (node->kernel->signature.directionTable[i] == VX_INPUT &&
                (node->kernel->signature.isStaticTable[i] == vx_false_e ||
                 (node->kernel->signature.dataTypeTable[i] == VX_TYPE_TENSOR &&
                  TENSOR_DATA_LIFETIME((vx_tensor)(node->paramTable[i])) == VX_TENSOR_LIFE_TIME_DYNAMIC)))
            {
                vxnneOperation_AddReference(&userLayer->user_shader_operation.base, node->paramTable[i], VXNNE_OPERATION_REFENRENCE_INPUT);
            }
            else if (node->kernel->signature.directionTable[i] == VX_OUTPUT)
            {
                vxnneOperation_AddReference(&userLayer->user_shader_operation.base, node->paramTable[i], VXNNE_OPERATION_REFENRENCE_OUTPUT);
            }
        }

        vxnneLayer_SetOperation(
            &userLayer->base,
            &userLayer->user_shader_operation.base,
            operationIndex++);
    }
    else if (userNodeType == VXNNE_USER_NODE_TYPE_CPU)
    {
        /* Create CPU operation. */
        vxmONERROR(vxnneOperation_Initialize(&userLayer->user_cpu_operation.base,
                                             &userLayer->base,
                                             VXNNE_OPERATION_TARGET_SW,
                                             VXNNE_OPERATOR_USER_CPU,
                                             vxnneUserOperation_Execute,
                                             VX_NULL,
                                             1,
                                             0));

        for (i = 0; i < node->kernel->signature.paramCount; i++)
        {
            if (node->kernel->signature.directionTable[i] == VX_INPUT &&
                (node->kernel->signature.isStaticTable[i] == vx_false_e ||
                 (node->kernel->signature.dataTypeTable[i] == VX_TYPE_TENSOR &&
                  TENSOR_DATA_LIFETIME((vx_tensor)(node->paramTable[i])) == VX_TENSOR_LIFE_TIME_DYNAMIC)))
            {
                vxnneOperation_AddReference(&userLayer->user_cpu_operation.base, node->paramTable[i], VXNNE_OPERATION_REFENRENCE_INPUT);
            }
            else if (node->kernel->signature.directionTable[i] == VX_OUTPUT)
            {
                vxnneOperation_AddReference(&userLayer->user_cpu_operation.base, node->paramTable[i], VXNNE_OPERATION_REFENRENCE_OUTPUT);
            }
        }

        vxnneLayer_SetOperation(
            &userLayer->base,
            &userLayer->user_cpu_operation.base,
            operationIndex++);
    }

    node->layer = &userLayer->base;

    return VX_SUCCESS;

OnError:
    if (userLayer)
    {
        gcoOS_Free(gcvNULL, userLayer);
    }

    return status;
}

#if REGISTER_FRAME
vx_status vxnneLayer_Ops_Initialize(
    vx_node                     node,
    vxnne_layer_ops             ops,
    vx_char*                    name,
    const vx_uint32             size_of_layer,
    const vx_reference          parameters[],
    vx_uint32                   num
    )
{
    vx_status status = VX_ERROR_NOT_SUPPORTED;
    vx_uint32 i = 0, flag = 0;
    vxnne_register_param_s reg_param = { 0 };

    /* destroy the existing layer */
    if (node->layer)
    {
        vxmONERROR(vxnneLayer_Free(node->layer));
        node->layer = VX_NULL;
    }

    for (i = 0; i < ops->imp_count; i++)
    {
        vxnne_layer_imp imp = &ops->imps[i];

        flag = 0;
        gcoOS_ZeroMemory(&reg_param, sizeof(vxnne_register_param_s));

        gcmASSERT((imp->verification != VX_NULL) && (imp->initialize != VX_NULL) && (size_of_layer > 0));

        if (imp->verification(node, parameters, num, &reg_param))
        {
            vx_uint32 max_num_operations = 0;
            vxnne_operation *operations = VX_NULL;

            vxmONERROR(gcoOS_Allocate(gcvNULL, size_of_layer, (gctPOINTER*)&node->layer));
            gcoOS_ZeroMemory(node->layer, size_of_layer);
            node->layer->node = node;

            if (ops->get_operations)ops->get_operations(node->layer, &max_num_operations, &operations);

            vxmONERROR(vxnneLayer_Initialize(node->layer,
                                  name,
                                  node,
                                  max_num_operations,
                                  operations,
                                  imp->deinitialize));

            vxmONERROR(imp->initialize(node->layer, parameters, num, &reg_param));

            break;
        }

    }

    gcmASSERT("Not Support Node!" && (i < ops->imp_count) && (status == VX_SUCCESS));

    return status;

OnError:
    if (node->layer) {
        gcoOS_Free(gcvNULL, node->layer);
        node->layer = VX_NULL;
    }

    return status;
}

vx_status vxoNNLayer_NotSupport_Initializer(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    return VX_ERROR_NOT_SUPPORTED;
}

vx_bool vxoNNCommon_NotSupport(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vx_false_e;

    return support;
}

vx_bool vxoNNCommon_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vx_true_e;

    return support;
}

vx_status vxoNNCommon_Deinitialize(vxnne_layer layer)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_bool vxoLayer_CheckNNFormatSupport(vx_context context, vx_enum type, vx_enum format, vx_uint32_ptr flag)
{
    vx_bool support = vx_true_e;
    gcsNN_FIXED_FEATURE* feature = &context->nnConfig.fixedFeature;

    if (type != VX_NN_QUERY_NN)
        return support;

    support = support && (feature->nnCoreCount > 0);

    switch (format)
    {
    case VX_TYPE_INT8:
        support = support && (feature->nnCoreCountInt8 > 0);
        break;
    case VX_TYPE_INT16:
        support = support && (feature->nnCoreCountInt16 > 0);
        break;
    case VX_TYPE_FLOAT16:
        support = support && (feature->nnCoreCountFloat16 > 0);
        break;
    case VX_TYPE_BFLOAT16:
        support = support && (feature->nnCoreCountBFloat16 > 0);
        break;
    default:
        support = vx_false_e;
        vxError("Not support format: %d\n", format);
        break;
    }

    return support;
}

vx_bool vxoLayer_CheckSupport(vx_context context, vx_enum type, vx_enum format, vx_uint32_ptr flag)
{
    vx_bool support = vx_true_e;

    switch (type)
    {
    case VX_NN_QUERY_NN:
        support = support && vxoLayer_CheckNNFormatSupport(context, type, format, flag);
        break;
    case VX_NN_QUERY_TP:
        support = support && vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP);
        break;
    case VX_NN_QUERY_SHADER:
        support = support && vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_SHADER);
        break;
    default:
        vxError("Cannot check support, Not support type: %d\n", type);
        support = vx_false_e;
        break;
    }

    return support;
}

vx_status vxoLayer_VerificationHead(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    return VX_SUCCESS;
}

vx_status vxoLayer_InitializeHead(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    return VX_SUCCESS;
}
vx_status vxoLayer_VerificationFoot(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool* support)
{
#if REGISTER_FRAME_LAYER_DEBUG
    gctSTRING envctrl = gcvNULL;
    gctCHAR name[128] = { 0 };

    sprintf(name, "VIV_VX_DEBUG_LAYER_%d_%d", node->kernel->enumeration, reg_param->index);
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, name, &envctrl)) && envctrl)
    {
        *support = (atoi(envctrl) == 0) ? vx_false_e : vx_true_e;
    }
#endif


    reg_param->support = *support;

    return VX_SUCCESS;
}

vx_status vxoLayer_InitializeFoot(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{

    return VX_SUCCESS;
}
#endif

