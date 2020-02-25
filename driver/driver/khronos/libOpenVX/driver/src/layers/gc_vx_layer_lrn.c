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
#include <layers/gc_vx_layer_lrn.h>

/***************************************************************************************************************************
 * NormalizationLayer and NormalizationLayer2
 *
 * See the difference between NormalizationLayer and NormalizationLayer2 as below.
 * 1. NormalizationLayer supports both SAME_MAP and ACROSS_MAP but NormalizationLayer2 supports CROSS_MAP only.
 * 2. For ACROSS_MAP, the common formula is as follows.
 *      b[xyz] = a[xyz] / power(bias + alpha * sum(square(a[xyi]) / div), beta)
 *      i is from max(0, z - kernel / 2) to min(inImageZSize - 1, z + norm_size / 2).
 *    NormalizationLayer does the formula with bias = 1.0f and div = norm_size.
 *    NormalizationLayer2 does the formula with div = 1.
 * 3. For SAME_MAP, NormalizationLayer does as follows.
 *
 ***************************************************************************************************************************/
vx_status vxnneExecuteSWNormalization(struct _vxnne_operation_s *operation)
{
    vxnne_normalization_operation lrn_operation = (vxnne_normalization_operation)operation;

    vx_tensor input  = lrn_operation->inputs;
    vx_tensor output = lrn_operation->outputs;

    gctPOINTER input_base;
    gctPOINTER output_base;

    vx_uint32 width   = TENSOR_SIZE_INDEX(input, 0);
    vx_uint32 height  = TENSOR_SIZE_INDEX(input, 1);
    vx_uint32 channel = TENSOR_SIZE_INDEX(input, 2);
    vx_uint32 batch   = TENSOR_DIM_NUM(input) > 3 ? TENSOR_SIZE_INDEX(input, 3) : 1;

    vx_enum    norm_type = lrn_operation->norm_type;
    vx_uint32  norm_size = lrn_operation->norm_size;
    vx_uint32  div       = lrn_operation->div;
    vx_float32 alpha     = lrn_operation->alpha;
    vx_float32 beta      = lrn_operation->beta;
    vx_float32 bias      = lrn_operation->bias;
    vx_uint32  nsz2      = norm_size / 2;
    vx_type_e  input_format  = (vx_type_e)TENSOR_DATA_TYPE(input);
    vx_type_e  output_format = (vx_type_e)TENSOR_DATA_TYPE(output);
    vx_int8    input_fp_pos   = TENSOR_POS(input);
    vx_int8    output_fp_pos  = TENSOR_POS(output);
    vx_enum    output_rounding_mode = TENSOR_ROUNDING_MODE(output);
    vx_uint32  input_stride, output_stride;
    vx_float32 sum = 0, val = 0;
    vx_uint32  w, h, c, n, b, i, j;
    vx_uint32  start_w, end_w, start_h, end_h, start_c, end_c;
    vx_uint32  input_data_size = vxDataType_GetSize(input_format);
    vx_uint32  output_data_size = vxDataType_GetSize(output_format);
    if ((input_data_size == 0) || (output_data_size == 0))
    {
        return VX_FAILURE;
    }
    else
    {
        input_stride = TENSOR_STRIDE_INDEX(input, 2) / input_data_size;
        output_stride = TENSOR_STRIDE_INDEX(output, 2) / output_data_size;
    }

    vxoTensor_GetTensorViewMemory(input, &input_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(output, &output_base, VX_NULL);

    if (norm_type == VX_NN_NORMALIZATION_SAME_MAP)
    {
        for (b = 0; b < batch; b++)
        {
            for (c = 0; c < channel; c++)
            {
                for (h = 0; h < height; h++)
                {
                    start_h = gcmMAX((vx_int32)(h - nsz2), 0);
                    end_h   = gcmMIN(h + nsz2, height - 1);
                    for (w = 0; w < width; w++)
                    {
                        sum = 0;

                        for (j = start_h; j <= end_h; j++)
                        {
                            start_w = gcmMAX((vx_int32)(w - nsz2), 0);
                            end_w   = gcmMIN(w + nsz2, width - 1);

                            for (i = start_w; i <= end_w; i++)
                            {
                                if (input_format == VX_TYPE_UINT8)
                                {
                                    val = vxnneGetDataQuant((vx_type_e)TENSOR_DATA_TYPE(input),
                                                            (c + b * channel) * input_stride + width * j + i,
                                                            (vx_uint8_ptr)input_base,
                                                            TENSOR_TF_ZEROPOINT(input),
                                                            TENSOR_TF_SCALE(input));
                                }
                                else
                                {
                                    val = vxnneGetData(input_format,
                                                       (c + b * channel) * input_stride + width * j + i,
                                                       (vx_uint8_ptr)input_base, input_fp_pos);
                                }

                                sum += val * val;
                            }
                        }

                        if (input_format == VX_TYPE_UINT8)
                        {
                            val = vxnneGetDataQuant((vx_type_e)TENSOR_DATA_TYPE(input),
                                                    (c + b * channel) * input_stride + width * h + w,
                                                    (vx_uint8_ptr)input_base,
                                                    TENSOR_TF_ZEROPOINT(input),
                                                    TENSOR_TF_SCALE(input));
                        }
                        else
                        {
                            val = vxnneGetData(input_format,
                                               (c + b * channel) * input_stride + width * h + w,
                                               (vx_uint8_ptr)input_base,
                                               input_fp_pos);
                        }

                        val = val / powf((bias + (alpha / div) * sum), beta);

                        if (output_format == VX_TYPE_UINT8)
                        {
                            vxnneSaveDataQuant((vx_type_e)TENSOR_DATA_TYPE(output),
                                               (c + b * channel) * output_stride + width * h + w,
                                               (vx_float64)(val),
                                               output_base,
                                               TENSOR_TF_ZEROPOINT(output),
                                               TENSOR_TF_SCALE(output),
                                               output_rounding_mode);
                        }
                        else
                        {
                            vxnneSaveData(output_format, (c + b * channel) * output_stride + width * h + w,
                                          val,
                                          (vx_uint8_ptr)output_base,
                                          output_fp_pos,
                                          output_rounding_mode);
                        }
                    }
                }
            }
        }
    }
    else
    {
        for (b = 0; b < batch; b++)
        {
            for (c = 0; c < channel; c++)
            {
                start_c = gcmMAX((vx_int32)(c - nsz2), 0);
                end_c   = gcmMIN(c + nsz2, channel - 1);

                for (h = 0; h < height; h++)
                {
                    for (w = 0; w < width; w++)
                    {
                        sum = 0;

                        for(n = start_c; n <= end_c; n++)
                        {
                            if (input_format == VX_TYPE_UINT8)
                            {
                                val = vxnneGetDataQuant((vx_type_e)TENSOR_DATA_TYPE(input),
                                                        (n + b * channel) * input_stride + width * h + w,
                                                        (vx_uint8_ptr)input_base,
                                                        TENSOR_TF_ZEROPOINT(input),
                                                        TENSOR_TF_SCALE(input));
                            }
                            else
                            {
                                val = vxnneGetData(input_format,
                                                   (n + b * channel) * input_stride + width * h + w,
                                                   (vx_uint8_ptr)input_base,
                                                   input_fp_pos);
                            }

                            sum += val * val;
                        }
                        if (input_format == VX_TYPE_UINT8)
                        {
                            val = vxnneGetDataQuant((vx_type_e)TENSOR_DATA_TYPE(input),
                                                    (c + b * channel) * input_stride + width * h + w,
                                                    (vx_uint8_ptr)input_base,
                                                    TENSOR_TF_ZEROPOINT(input),
                                                    TENSOR_TF_SCALE(input));
                        }
                        else
                        {
                            val = vxnneGetData(input_format,
                                               (c + b * channel) * input_stride + width * h + w,
                                               (vx_uint8_ptr)input_base,
                                               input_fp_pos);
                        }

                        val = val / powf((bias + (alpha / div) * sum), beta);

                        if (output_format == VX_TYPE_UINT8)
                        {
                            vxnneSaveDataQuant((vx_type_e)TENSOR_DATA_TYPE(output),
                                               (c + b * channel) * output_stride + width * h + w,
                                               (vx_float64)(val),
                                               output_base,
                                               TENSOR_TF_ZEROPOINT(output),
                                               TENSOR_TF_SCALE(output),
                                               output_rounding_mode);
                        }
                        else
                        {
                            vxnneSaveData(output_format,
                                          (c + b * channel) * output_stride + width * h + w,
                                          val,
                                          (vx_uint8_ptr)output_base,
                                          output_fp_pos,
                                          output_rounding_mode);
                        }
                    }
                }
            }
        }
    }

    return VX_SUCCESS;
}


vx_tensor _AllocateTPLUTorListBuffer(vx_context context, vx_node node, vx_uint32 size, vx_enum type)
{
    vx_tensor buffer = VX_NULL;
    vx_tensor_create_params_t tensor_create_params;

    size = size != 0 ? size : TP_LUT_BUFF_SIZE;
    type = type != VX_TYPE_INVALID ? type :
           gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_TP_REAL_INT16) &&
           !gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_TP_SIMPLE_INT16) ? VX_TYPE_UINT32 : VX_TYPE_UINT16;

    vxZeroMemory(&tensor_create_params, gcmSIZEOF(vx_tensor_create_params_t));
    tensor_create_params.num_of_dims = 1;
    tensor_create_params.sizes = &size;
    tensor_create_params.data_format = type;
    tensor_create_params.quant_format = VX_QUANT_DYNAMIC_FIXED_POINT;
    tensor_create_params.quant_data.dfp.fixed_point_pos = 0;

    buffer = vxoTensor_CreateTensor(context, node->graph, &tensor_create_params, vx_false_e);

    if (buffer == VX_NULL || vxoTensor_AllocateMemory(buffer) != VX_SUCCESS)
    {
        vxError("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
        return VX_NULL;
    }

    return buffer;
}

vx_tensor vxnneAllocateTPLUTBuffer(vx_context context, vx_node node)
{
    return _AllocateTPLUTorListBuffer(context, node, 0, VX_TYPE_INVALID);
}

VX_PRIVATE_API vx_status vxoLRNOperationTP_Initialize(
    vxnne_tp_operation operation,
    vxnne_layer layer,
    vx_tensor input,
    vx_tensor output,
    vx_uint32 batch_count,
    vx_enum norm_type,
    vx_uint32 norm_size,
    vx_uint32 div,
    vx_float32 alpha,
    vx_float32 beta,
    vx_float32 bias,
    vx_uint32 *op_index)
{
    vx_status status = VX_SUCCESS;

    vx_node node = layer->node;
    vx_context context = vxGetContext((vx_reference)node);
    vx_op_param op_param = VX_NULL;

    if (!op_index)
    {
        vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
    }

    vxmONERROR(vxnneOperation_Initialize(&operation->base,
                                         layer,
                                         VXNNE_OPERATION_TARGET_TP,
                                         VXNNE_OPERATOR_NORMALIZATION,
                                         VX_NULL,
                                         vxnneOperation_TP_Deinitialize,
                                         batch_count,
                                         0));

    op_param = &operation->base.parameter;

    op_param->data_buff = vxnneAllocateTPLUTBuffer(context, node);
    if (op_param->data_buff == VX_NULL) vxmONERROR(VX_ERROR_NO_MEMORY);

    op_param->pad_x_left   = 0;
    op_param->pad_y_top    = 0;
    op_param->pad_x_right  = 0;
    op_param->pad_y_bottom = 0;
    op_param->pool_size_x  = 0;
    op_param->pool_size_y  = 0;
    op_param->pool_stride  = 1;
    op_param->enable_relu  = vx_false_e;
    op_param->pad_mode     = VX_PAD_CONSTANT;
    /* For TP PAD_CONST, the HW just uses the data without minus ZERO_POINT. */
    op_param->pad_const    = 0;
    op_param->tpType       = TP_LRN;
    op_param->other_ref    = VX_NULL;
    op_param->conv_rounding_type = 0;

    op_param->tp_value = (vx_tp_value_cmd)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
    if (!op_param->tp_value)
    {
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }
    op_param->tp_value->e32[0] = norm_type;
    op_param->tp_value->u32[0] = norm_size;
    op_param->tp_value->u32[1] = div;
    op_param->tp_value->f32[0] = alpha;
    op_param->tp_value->f32[1] = beta;
    op_param->tp_value->f32[2] = bias;

    vxmONERROR(vxnneLayer_SetOperation(layer,
                                       &operation->base,
                                       (*op_index)++));

    operation->input  = input;
    operation->output = output;

    vxnneOperation_AddReference(&operation->base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
    vxnneOperation_AddReference(&operation->base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

    return status;

OnError:
    if (op_param && op_param->data_buff)
    {
        vxoTensor_ReleaseTensor(&op_param->data_buff);
        op_param->data_buff = VX_NULL;
    }

    if (op_param && op_param->tp_value)
    {
        vxFree(op_param->tp_value);
        op_param->tp_value = VX_NULL;
    }

    return status;
}

VX_PRIVATE_API vx_status vxoLRNOperationSH_Initialize(
    vxnne_shader_operation operation,
    vxnne_layer layer,
    vx_tensor inputs,
    vx_tensor outputs,
    vx_uint32 batchCount,
    vx_enum norm_type,
    vx_uint32 norm_size,
    vx_uint32 div,
    vx_float32 alpha,
    vx_float32 beta,
    vx_float32 bias,
    vx_uint32 *op_index)
{
    vx_status status = VX_SUCCESS;

    vx_node                 node         = layer->node;
    vx_context              context      = vxGetContext((vx_reference)node);
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vx_scalar               type_s       = VX_NULL;
    vx_scalar               norm_size_s  = VX_NULL;
    vx_scalar               alpha_s      = VX_NULL;
    vx_scalar               beta_s       = VX_NULL;
    vx_scalar               bias_s       = VX_NULL;
    vx_enum    inputFormat               = TENSOR_DATA_TYPE(inputs);
    vx_enum    outputFormat              = TENSOR_DATA_TYPE(outputs);
    vx_bool    sammap_flag               = vx_false_e;
    vx_bool    acrossmap_flag            = vx_false_e;
    vx_bool    dataformat_flag[6]        = {vx_false_e};
    vx_bool    norm_config[3]            = {vx_false_e};
    vx_bool    generic_flag              = vx_false_e;
    vx_bool    isuint8_flag              = vx_false_e;
    vx_bool    norm_shader_flag          = vx_false_e;

    if (!op_index)
    {
        vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
    }

    sammap_flag        = (vx_bool)(norm_type == VX_NN_NORMALIZATION_SAME_MAP);
    acrossmap_flag     = (vx_bool)(norm_type == VX_NN_NORMALIZATION_ACROSS_MAPS);
    norm_config[0]     = (vx_bool)(norm_size == 3 && beta == 0.75);
    norm_config[1]     = (vx_bool)(norm_size == 5 && beta == 0.75);
    norm_config[2]     = (vx_bool)(norm_size == 11 && beta == 0.5);
    dataformat_flag[0] = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 || inputFormat == VX_TYPE_INT8) && (outputFormat == VX_TYPE_FLOAT16 || outputFormat == VX_TYPE_INT8));
    dataformat_flag[1] = (vx_bool)(inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16);
    dataformat_flag[2] = (vx_bool)(inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8);
    dataformat_flag[3] = (vx_bool)(inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16);
    dataformat_flag[4] = (vx_bool)(inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32 &&!context->evisNoInst.supportEVIS);
    dataformat_flag[5] = (vx_bool)(inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT16);
    isuint8_flag       = (vx_bool)((acrossmap_flag && dataformat_flag[5])
                                    || dataformat_flag[2]);
    generic_flag       = (vx_bool)((acrossmap_flag && dataformat_flag[0]) || (sammap_flag && dataformat_flag[3])
                                    ||(acrossmap_flag && dataformat_flag[5]));
    norm_shader_flag   = (vx_bool)((sammap_flag && norm_config[0] && dataformat_flag[0])
                                || (acrossmap_flag && norm_config[0] && dataformat_flag[0])
                                || (acrossmap_flag && norm_config[1] && dataformat_flag[0])
                                || (acrossmap_flag && norm_config[2] && (dataformat_flag[0] || dataformat_flag[1]))
                                || generic_flag || isuint8_flag || dataformat_flag[4]);

    if (div == 1)
    {
        if (acrossmap_flag)
        {
            if(context->evisNoInst.supportEVIS)
            {
                alpha = alpha * (vx_float32)norm_size;
            }
            else
            {
                norm_size = norm_size/2;
            }
        }
        else
        {
            alpha = alpha * (vx_float32)(norm_size * norm_size);
        }
    }
    else
    {
        if (acrossmap_flag)
        {
            if(!context->evisNoInst.supportEVIS)
            {
                alpha = alpha / (vx_float32)div;
                norm_size = norm_size / 2;
            }
        }
    }

    type_s      = vxCreateScalar(context, VX_TYPE_ENUM, &norm_type);
    norm_size_s = vxCreateScalar(context, VX_TYPE_UINT32, &norm_size);
    alpha_s     = vxCreateScalar(context, VX_TYPE_FLOAT32, &alpha);
    beta_s      = vxCreateScalar(context, VX_TYPE_FLOAT32, &beta);

    if(context->evisNoInst.supportEVIS)
    {
        if (isuint8_flag)
            shaderExecutable = vxnneGetNormalizationUint8ShaderExecutable(node->base.context, VXNNE_KERNEL_NORMALIZATION, &node->kernelAttributes.borderMode, inputs, type_s, norm_size_s, alpha_s, beta_s, bias, outputs);
        else
            shaderExecutable = vxnneGetNormalizationShaderExecutable(node->base.context, VXNNE_KERNEL_NORMALIZATION, &node->kernelAttributes.borderMode, inputs, type_s, norm_size_s, alpha_s, beta_s, bias, outputs);
    }
    else
    {
        vx_scalar bias_s = vxCreateScalar(context, VX_TYPE_FLOAT32, &bias);

        shaderExecutable = vxnneGetGPUNormalizationShaderExecutable(context, VXNNE_KERNEL_NORMALIZATION, &node->kernelAttributes.borderMode, inputs, type_s, norm_size_s, alpha_s, beta_s, bias_s, outputs);

        if(bias_s) vxReleaseScalar(&bias_s);
    }

    vxReleaseScalar(&type_s);
    vxReleaseScalar(&norm_size_s);
    vxReleaseScalar(&alpha_s);
    vxReleaseScalar(&beta_s);
    vxReleaseScalar(&bias_s);

    if (!shaderExecutable)
    {
        vxmONERROR(VX_FAILURE);
    }

    vxmONERROR(vxnneShaderOperation_Initialize(operation,
                                               layer,
                                               VXNNE_OPERATOR_NORMALIZATION,
                                               batchCount,
                                               shaderExecutable));

    vxmONERROR(vxnneLayer_SetOperation(layer,
                                       &operation->base,
                                       (*op_index)++));

    vxnneOperation_AddReference(&operation->base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
    vxnneOperation_AddReference(&operation->base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

OnError:
    return status;
}

VX_PRIVATE_API vx_status vxoLRNOperationSW_Initialize(
    vxnne_normalization_operation operation,
    vxnne_layer layer,
    vx_tensor input,
    vx_tensor output,
    vx_uint32 batch_count,
    vx_enum norm_type,
    vx_uint32 norm_size,
    vx_uint32 div,
    vx_float32 alpha,
    vx_float32 beta,
    vx_float32 bias,
    vx_uint32 *op_index)
{
    vx_status status = VX_SUCCESS;

    if (!op_index)
    {
        vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
    }

    vxmONERROR(vxnneOperation_Initialize(&operation->base,
                                         layer,
                                         VXNNE_OPERATION_TARGET_SW,
                                         VXNNE_OPERATOR_NORMALIZATION,
                                         vxnneExecuteSWNormalization,
                                         VX_NULL,
                                         batch_count,
                                         0));

    vxmONERROR(vxnneLayer_SetOperation(layer,
                                       &operation->base,
                                       (*op_index)++));

    operation->inputs    = input;
    operation->norm_type = norm_type;
    operation->norm_size = norm_size;
    operation->div       = div;
    operation->alpha     = alpha;
    operation->beta      = beta;
    operation->bias      = bias;
    operation->outputs   = output;

    vxnneOperation_AddReference(&operation->base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
    vxnneOperation_AddReference(&operation->base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

OnError:
    return status;
}

VX_PRIVATE_API vx_status _InitializeLRNOperation(
    vxnne_normalization_layer layer,
    vxnne_operation_target_e target,
    vx_tensor input,
    vx_tensor output,
    vx_uint32 batch_count,
    vx_enum norm_type,
    vx_uint32 norm_size,
    vx_uint32 div,
    vx_float32 alpha,
    vx_float32 beta,
    vx_float32 bias,
    vx_uint32 *op_index)
{
    vx_status status = VX_SUCCESS;

    if (!op_index)
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    switch (target)
    {
    case VXNNE_OPERATION_TARGET_TP:
        vxmONERROR(vxoLRNOperationTP_Initialize(&layer->lrn_tp_operation,
                                                &layer->base,
                                                input,
                                                output,
                                                batch_count,
                                                norm_type,
                                                norm_size,
                                                div,
                                                alpha,
                                                beta,
                                                bias,
                                                op_index));
        break;

    case VXNNE_OPERATION_TARGET_SH:
        vxmONERROR(vxoLRNOperationSH_Initialize(&layer->lrn_sh_operation,
                                                &layer->base,
                                                input,
                                                output,
                                                batch_count,
                                                norm_type,
                                                norm_size,
                                                div,
                                                alpha,
                                                beta,
                                                bias,
                                                op_index));
        break;

    case VXNNE_OPERATION_TARGET_SW:
        vxmONERROR(vxoLRNOperationSW_Initialize(&layer->lrn_sw_operation,
                                                &layer->base,
                                                input,
                                                output,
                                                batch_count,
                                                norm_type,
                                                norm_size,
                                                div,
                                                alpha,
                                                beta,
                                                bias,
                                                op_index));
        break;

    default:
        status = VX_ERROR_NOT_SUPPORTED;
        break;
    }

OnError:
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNNormalization(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
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
    vx_uint32  div                        = norm_type == VX_NN_NORMALIZATION_SAME_MAP ? norm_size * norm_size : norm_size;
    vx_float32 alpha                      = alpha_s->value->f32;
    vx_float32 beta                       = beta_s->value->f32;
    vx_enum    input_format               = TENSOR_DATA_TYPE(inputs);
    vx_enum    output_format              = TENSOR_DATA_TYPE(outputs);
    vx_bool    sammap_flag                = vx_false_e;
    vx_bool    acrossmap_flag             = vx_false_e;
    vx_bool    dataformat_flag[7]         = {vx_false_e};
    vx_bool    norm_config[3]             = {vx_false_e};
    vx_bool    generic_flag               = vx_false_e;
    vx_bool    isuint8_flag               = vx_false_e;
    vx_bool    norm_shader_flag           = vx_false_e;
    vx_uint32  batch_count                = TENSOR_DIM_NUM(inputs) > 3 ? TENSOR_SIZE_INDEX(inputs, 3) : 1;
    vxnne_normalization_layer lrn_layer   = VX_NULL;
    vxnne_operation_target_e target       = VXNNE_OPERATION_TARGET_NONE;
    vx_uint32  size_x, size_y;
    vx_uint32  op_index                   = 0;

    size_x = TENSOR_SIZE_INDEX(inputs, 0);
    size_y = TENSOR_SIZE_INDEX(inputs, 1);

    /* Destroy the existing layer. */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    /* Create the layer. */
    lrn_layer = (vxnne_normalization_layer)vxAllocate(sizeof(vxnne_normalization_layer_s));
    if (!lrn_layer)
    {
        vxError("Allocate memory fail at function %s line %d.", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }

    vxmONERROR(vxnneLayer_Initialize(&lrn_layer->base,
                                     "NormalizationLayer",
                                     node,
                                     vxmOPERATION_COUNT(lrn_layer),
                                     lrn_layer->operations,
                                     VX_NULL));

    sammap_flag        = (vx_bool)(norm_type == VX_NN_NORMALIZATION_SAME_MAP);
    acrossmap_flag     = (vx_bool)(norm_type == VX_NN_NORMALIZATION_ACROSS_MAPS);
    norm_config[0]     = (vx_bool)(norm_size == 3 && beta == 0.75);
    norm_config[1]     = (vx_bool)(norm_size == 5 && beta == 0.75);
    norm_config[2]     = (vx_bool)(norm_size == 11 && beta == 0.5);
    dataformat_flag[0] = (vx_bool)((input_format == VX_TYPE_FLOAT16 || input_format == VX_TYPE_INT8) && (output_format == VX_TYPE_FLOAT16 || output_format == VX_TYPE_INT8));
    dataformat_flag[1] = (vx_bool)(input_format == VX_TYPE_INT16 && output_format == VX_TYPE_INT16);
    dataformat_flag[2] = (vx_bool)(input_format == VX_TYPE_UINT8 && output_format == VX_TYPE_UINT8);
    dataformat_flag[3] = (vx_bool)(input_format == VX_TYPE_FLOAT16 && output_format == VX_TYPE_FLOAT16);
    dataformat_flag[4] = (vx_bool)(input_format == VX_TYPE_FLOAT32 && output_format == VX_TYPE_FLOAT32);
    dataformat_flag[5] = (vx_bool)(input_format == VX_TYPE_UINT8 && output_format == VX_TYPE_FLOAT16);
    dataformat_flag[6] = (vx_bool)(input_format == VX_TYPE_BFLOAT16 && output_format == VX_TYPE_BFLOAT16);
    isuint8_flag       = (vx_bool)((acrossmap_flag && norm_config[0] && dataformat_flag[2])
        || (acrossmap_flag && norm_config[1] && dataformat_flag[2])
        || (acrossmap_flag && norm_config[2] && dataformat_flag[2])
        || dataformat_flag[2]);
    generic_flag       = (vx_bool)((acrossmap_flag && dataformat_flag[0]) || (sammap_flag && dataformat_flag[3])
                                    || (dataformat_flag[1])
                                    || (acrossmap_flag && dataformat_flag[5])
                                    || (dataformat_flag[6]));
    norm_shader_flag   = (vx_bool)((sammap_flag && norm_config[0] && dataformat_flag[0])
        || (acrossmap_flag && norm_config[0] && dataformat_flag[0])
        || (acrossmap_flag && norm_config[1] && dataformat_flag[0])
        || (acrossmap_flag && norm_config[2] && (dataformat_flag[0] || dataformat_flag[1]))
        || generic_flag || isuint8_flag || dataformat_flag[4]);

    /* Choose acceleration path. */
    if (vxnneIsTPSupportFormat(context, inputs, VX_NULL, outputs) &&
        vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_LRN) &&
        norm_size <= 5 &&
        (norm_type == VX_NN_NORMALIZATION_SAME_MAP || size_x * size_y < 65536))
    {
        target = VXNNE_OPERATION_TARGET_TP;
    }
    else if (norm_shader_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
    {
        target = VXNNE_OPERATION_TARGET_SH;
    }
    else
    {
        target = VXNNE_OPERATION_TARGET_SW;
    }

    vxmONERROR(_InitializeLRNOperation(lrn_layer,
                                       target,
                                       inputs,
                                       outputs,
                                       batch_count,
                                       norm_type,
                                       norm_size,
                                       div,
                                       alpha,
                                       beta,
                                       1.0f, /* bias */
                                       &op_index));

    node->layer = &lrn_layer->base;

    return status;

OnError:
    if (lrn_layer != NULL)
    {
        vxFree(lrn_layer);
    }

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNormalization_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNormalizationLayer2_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxoNormalizationLayer2_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNormalizationLayer2_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_scalar  type_s                     = (vx_scalar)parameters[1];
    vx_scalar  norm_size_s                = (vx_scalar)parameters[2];
    vx_scalar  alpha_s                    = (vx_scalar)parameters[3];
    vx_scalar  beta_s                     = (vx_scalar)parameters[4];
    vx_scalar  bias_s                     = (vx_scalar)parameters[5];
    vx_tensor  outputs                    = (vx_tensor)parameters[6];

    vx_enum    norm_type                  = type_s->value->e;
    vx_uint32  norm_size                  = norm_size_s->value->u32;
    vx_float32 alpha                      = alpha_s->value->f32;
    vx_float32 beta                       = beta_s->value->f32;
    vx_float32 bias                       = bias_s->value->f32;
    vx_enum    input_format               = TENSOR_DATA_TYPE(inputs);
    vx_enum    output_format              = TENSOR_DATA_TYPE(outputs);
    vx_bool    sammap_flag                = vx_false_e;
    vx_bool    acrossmap_flag             = vx_false_e;
    vx_bool    dataformat_flag[7]         = {vx_false_e};
    vx_bool    norm_config[3]             = {vx_false_e};
    vx_bool    generic_flag               = vx_false_e;
    vx_bool    isuint8_flag               = vx_false_e;
    vx_bool    norm_shader_flag           = vx_false_e;

    vx_uint32  size_x                     = TENSOR_SIZE_INDEX(inputs, 0);
    vx_uint32  size_y                     = TENSOR_SIZE_INDEX(inputs, 1);
    vx_uint32  batch_count                = TENSOR_DIM_NUM(inputs) > 3 ? TENSOR_SIZE_INDEX(inputs, 3) : 1;
    vx_context context                    = node->base.context;
    vxnne_normalization_layer lrn2_layer  = VX_NULL;
    vxnne_operation_target_e target       = VXNNE_OPERATION_TARGET_NONE;
    vx_uint32 op_index                    = 0;

    /* Destroy the existing layer. */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    /* Create the layer. */
    lrn2_layer = (vxnne_normalization_layer)vxAllocate(gcmSIZEOF(vxnne_normalization_layer_s));
    if (!lrn2_layer)
    {
        vxError("Allocate memory fail at function %s line %d.", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }

    vxmONERROR(vxnneLayer_Initialize(&lrn2_layer->base,
                                     "NormalizationLayer2",
                                     node,
                                     vxmOPERATION_COUNT(lrn2_layer),
                                     lrn2_layer->operations,
                                     VX_NULL));

    sammap_flag        = (vx_bool)(norm_type == VX_NN_NORMALIZATION_SAME_MAP);
    acrossmap_flag     = (vx_bool)(norm_type == VX_NN_NORMALIZATION_ACROSS_MAPS);
    norm_config[0]     = (vx_bool)(norm_size == 3 && beta == 0.75);
    norm_config[1]     = (vx_bool)(norm_size == 5 && beta == 0.75);
    norm_config[2]     = (vx_bool)(norm_size == 11 && beta == 0.5);
    dataformat_flag[0] = (vx_bool)((input_format == VX_TYPE_FLOAT16 || input_format == VX_TYPE_INT8) && (output_format == VX_TYPE_FLOAT16 || output_format == VX_TYPE_INT8));
    dataformat_flag[1] = (vx_bool)(input_format == VX_TYPE_INT16 && output_format == VX_TYPE_INT16);
    dataformat_flag[2] = (vx_bool)(input_format == VX_TYPE_UINT8 && output_format == VX_TYPE_UINT8);
    dataformat_flag[3] = (vx_bool)(input_format == VX_TYPE_FLOAT16 && output_format == VX_TYPE_FLOAT16);
    dataformat_flag[4] = (vx_bool)(input_format == VX_TYPE_FLOAT32 && output_format == VX_TYPE_FLOAT32 && !context->evisNoInst.supportEVIS);
    dataformat_flag[5] = (vx_bool)(input_format == VX_TYPE_UINT8 && output_format == VX_TYPE_FLOAT16);
    dataformat_flag[6] = (vx_bool)(input_format == VX_TYPE_BFLOAT16 && output_format == VX_TYPE_BFLOAT16);
    isuint8_flag       = (vx_bool)((acrossmap_flag && norm_config[0] && dataformat_flag[2])
        || (acrossmap_flag && norm_config[1] && dataformat_flag[2])
        || (acrossmap_flag && norm_config[2] && dataformat_flag[2])
        || dataformat_flag[2]);
    generic_flag       = (vx_bool)((acrossmap_flag && dataformat_flag[0]) || (sammap_flag && dataformat_flag[3])
                                    || (dataformat_flag[1])
                                    || (acrossmap_flag && dataformat_flag[5])
                                    || (dataformat_flag[6]));
    norm_shader_flag   = (vx_bool)((sammap_flag && norm_config[0] && dataformat_flag[0])
        || (acrossmap_flag && norm_config[0] && dataformat_flag[0])
        || (acrossmap_flag && norm_config[1] && dataformat_flag[0])
        || (acrossmap_flag && norm_config[2] && (dataformat_flag[0] || dataformat_flag[1]))
        || generic_flag || isuint8_flag || dataformat_flag[4]);

    /* Choose acceleration path. */
    if (vxnneIsTPSupportFormat(context, inputs, VX_NULL, outputs) &&
        vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_LRN) &&
        norm_size <= 5 &&
        (norm_type == VX_NN_NORMALIZATION_SAME_MAP || size_x * size_y < 65536))
    {
        target = VXNNE_OPERATION_TARGET_TP;
    }
    else if (norm_shader_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
    {
        target = VXNNE_OPERATION_TARGET_SH;
    }
    else
    {
        target = VXNNE_OPERATION_TARGET_SW;
    }

    vxmONERROR(_InitializeLRNOperation(lrn2_layer,
                                       target,
                                       inputs,
                                       outputs,
                                       batch_count,
                                       norm_type,
                                       norm_size,
                                       1, /* div */
                                       alpha,
                                       beta,
                                       bias,
                                       &op_index));

    node->layer = &lrn2_layer->base;

    return status;

OnError:
    if (lrn2_layer)
    {
        if (lrn2_layer->lrn_tp_operation.base.parameter.tp_value)
        {
            vxFree(lrn2_layer->lrn_tp_operation.base.parameter.tp_value);
        }
        vxFree(lrn2_layer);
    }

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNormalizationLayer2_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}
