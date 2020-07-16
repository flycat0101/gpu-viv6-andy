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


#include <ops/gc_vx_op_fc.h>
#include <ops/gc_vx_op_tensor_copy.h>
#include <gc_vx_nn_util.h>
#include <gc_vx_nn_wb.h>

extern vx_tensor _createTensor(vx_graph graph, vx_bool is_virtual,
    vx_uint32 num_of_dims, vx_uint32 * sizes, vx_enum data_format, vx_enum quant_format,
    vx_int8 fixed_point_pos,
    vx_float32 scale, vx_int32 zeroPoint);
extern vx_tensor _createSimilarTensor(vx_graph graph, vx_bool is_virtual, vx_uint32 num_of_dims, vx_uint32 * sizes, vx_tensor tensor);

extern vx_status vxnneExecuteSWConvolution(vxnne_operation operation);

extern vx_status vxnneExecuteSWTensorTranspose(struct _vxnne_operation_s *operation);

extern vx_status vxnneExecuteCopy(vx_node node, vxnne_layer_s* layer,
                         vxnne_tp_operation lstm_copy_tp_operation,
                         vxnne_tensor_copy_sw_operation lstm_copy_sw_operation,
                         vxnne_shader_operation lstm_copy_sh_operation,
                         vx_tensor inputs, vx_tensor outputs,
                         vx_bool enable_sw_tensor_copy, vx_int32_ptr count, vx_int32 batch);

extern vx_status vxnneGetAlignStrides(vx_tensor tensor, vx_uint32_ptr strides, vx_bool align64);
extern vx_status vxnneOperation_TP_Deinitialize(vxnne_operation_s *operation);
extern vx_status vxnneOperation_AddReference(
    vxnne_operation_s*            operation,
    vx_reference                  reference,
    vxnne_operation_reference_e   refType
    );

vx_status vxoFC_NN_Trans_Initialize(
    vxnne_tp_operation operation_tp,
    vxnne_shader_operation operation_sh,
    vxnne_tensor_trans_operation operation_sw,
    vxnne_layer layer,
    vx_tensor inputs,
    vx_uint32_ptr perms,
    vx_uint32 batch_count,
    vx_border_t* border_mode,
    vx_tensor outputs,
    vx_uint32_ptr op_index
    );


vx_status vxoFCOperation_TransposeTensor(vx_tensor weights, vx_tensor weight_conv, vx_uint32* perm, vx_uint32 pnum)
{
    vx_status status = VX_SUCCESS;

    vx_uint8_ptr inaddr, outaddr;
    vx_uint32 dims[VX_CONTEXT_TENSOR_MAX_DIMENSION], strides[VX_CONTEXT_TENSOR_MAX_DIMENSION], tstrides[VX_CONTEXT_TENSOR_MAX_DIMENSION];

    vxoTensor_GetTensorViewMemory(weights, (gctPOINTER*)&inaddr, VX_NULL);
    vxoTensor_GetTensorViewMemory(weight_conv, (gctPOINTER*)&outaddr, VX_NULL);

    vxoTensor_GetTensorDimStride(weights, &pnum, dims, strides);
    vxoTensor_GetTensorDimStride(weight_conv, &pnum, VX_NULL, tstrides);

    _TransposeTensor(inaddr, outaddr,TENSOR_DATA_SIZE(weights), dims, strides, tstrides, perm, pnum - 1);

    return status;
}

vx_status vxoFCOperation_CopyTensor2D(vx_tensor src, vx_tensor dst, vx_bool transpose)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 size = 0;
    vx_uint8_ptr inaddr, outaddr;
    vx_uint32 dims[VX_CONTEXT_TENSOR_MAX_DIMENSION], strides[VX_CONTEXT_TENSOR_MAX_DIMENSION], tstrides[VX_CONTEXT_TENSOR_MAX_DIMENSION];

    vxoTensor_GetTensorViewMemory(src, (gctPOINTER*)&inaddr, VX_NULL);
    vxoTensor_GetTensorViewMemory(dst, (gctPOINTER*)&outaddr, VX_NULL);

    vxoTensor_GetTensorDimStride(src, &size, dims, strides);
    vxoTensor_GetTensorDimStride(dst, &size, VX_NULL, tstrides);

    if (TENSOR_SIZE_INDEX(src, 0) == TENSOR_SIZE_INDEX(dst, 0) && TENSOR_SIZE_INDEX(src, 1) == TENSOR_SIZE_INDEX(dst, 1))
    {
        vx_uint32 i = 0, j = 0;
        for (i = 0; i < TENSOR_SIZE_INDEX(src, 1); i ++)
        {
            gcoOS_MemCopy(outaddr + i * tstrides[1], inaddr + i * strides[1], strides[1]);

            if (((tstrides[1] - strides[1]) > 0) && (TENSOR_QUANT_TYPE(dst) == VX_QUANT_AFFINE_SCALE))
            {
                for (j = 0; j < (tstrides[1] - strides[1]); j ++)
                    gcoOS_MemFill(outaddr + i * tstrides[1] + strides[1] + j, (vx_uint8)TENSOR_TF_ZEROPOINT(dst), sizeof(vx_uint8));
            }
        }

    }

    return status;
}

VX_PRIVATE_API vx_status _GetBatchAndInOutCountOfFC(
    vx_tensor inputs,
    vx_tensor outputs,
    vx_uint32_ptr batch_count,
    vx_uint32_ptr input_count,
    vx_uint32_ptr output_count
    )
{
    vx_status status = VX_SUCCESS;

    vx_uint32 input_dim_num = TENSOR_VIEW_DIM_NUM(inputs);
    vx_uint32 output_dim_num = TENSOR_VIEW_DIM_NUM(outputs);
    vx_uint32 input_batch_pos = input_dim_num;
    vx_uint32 output_batch_pos = output_dim_num;
    vx_uint32 i;

    if (input_dim_num == 2)
    {
        input_batch_pos = 1;
    }
    else if (input_dim_num > 3)
    {
        input_batch_pos = 3;
    }
    else
    {
        /*
         * No batch is specified.
         * Set batch_pos out of bound to use the
         * default batch value of 1.
         */
        input_batch_pos = input_dim_num;
    }

    output_batch_pos = output_dim_num - (input_dim_num - input_batch_pos);

    if (batch_count)
    {
        vx_uint32 batch = 1;

        for (i = input_batch_pos; i < input_dim_num; i++)
        {
            batch *= TENSOR_VIEW_SIZE_INDEX(inputs, i);
        }

        *batch_count = batch;
    }

    if (input_count)
    {
        vx_uint32 in_count = 1;

        for (i = 0; i < input_batch_pos; i++)
        {
            in_count *= TENSOR_VIEW_SIZE_INDEX(inputs, i);
        }

        *input_count = in_count;
    }

    if (output_count)
    {
        vx_uint32 out_count = 1;

        for (i = 0; i < output_batch_pos; i++)
        {
            out_count *= TENSOR_VIEW_SIZE_INDEX(outputs, i);
        }

        *output_count = out_count;
    }

    return status;
}

VX_INTERNAL_API vx_status _FCOperationSW(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;

    vxnne_fully_connected_sw_operation fullyConnectedOperation = (vxnne_fully_connected_sw_operation)operation;

    vx_tensor inputs  = (vx_tensor)fullyConnectedOperation->inputs;
    vx_tensor weights = (vx_tensor)fullyConnectedOperation->weights;
    vx_tensor biases  = (vx_tensor)fullyConnectedOperation->biases;
    vx_tensor outputs = (vx_tensor)fullyConnectedOperation->outputs;
    gctPOINTER inputsBaseLogicalAddr = VX_NULL, outputsBaseLogicalAddr = VX_NULL;
    gctPOINTER weightsBaseLogicalAddr = VX_NULL, biasesBaseLogicalAddr = VX_NULL;
    vx_uint32 i = 0, j = 0, b = 0;
    vx_uint32 batch_count, input_count, output_count;
    vx_float32 madValue, inputValue, weightValue, biasValue = 0.0f;
    vx_enum srcType, dstType, weightsType, biasesType, outputRoundingMode;
    vx_int8 inputFpPos = 0, weightFpPos = 0, biasFpPos = 0, outputFpPos = 0;
    vx_float32 result = 0.0f;

    vxmONERROR(_GetBatchAndInOutCountOfFC(inputs,
                                          outputs,
                                          &batch_count,
                                          &input_count,
                                          &output_count));

    srcType = TENSOR_DATA_TYPE(inputs);
    dstType = TENSOR_DATA_TYPE(outputs);
    weightsType = TENSOR_DATA_TYPE(weights);
    biasesType = TENSOR_DATA_TYPE(biases);

    vxoTensor_GetTensorViewMemory(inputs, &inputsBaseLogicalAddr, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, &outputsBaseLogicalAddr, VX_NULL);
    vxoTensor_GetTensorViewMemory(weights, &weightsBaseLogicalAddr, VX_NULL);
    vxoTensor_GetTensorViewMemory(biases, &biasesBaseLogicalAddr, VX_NULL);

    inputFpPos = TENSOR_POS(inputs);
    weightFpPos = TENSOR_POS(weights);
    biasFpPos = TENSOR_POS(biases);
    outputFpPos = TENSOR_POS(outputs);
    outputRoundingMode = TENSOR_ROUNDING_MODE(outputs);

    for (b = 0; b < batch_count; b ++)
    {
        for (i = 0; i < output_count; i++)
        {
            madValue = 0.0;

            for (j = 0; j < input_count; j++)
            {
                if (((srcType == VX_TYPE_FLOAT16) && (weightsType == VX_TYPE_FLOAT16) && (biasesType == VX_TYPE_FLOAT32)) ||
                    ((srcType == VX_TYPE_FLOAT32) && (weightsType == VX_TYPE_FLOAT32) && (biasesType ==  VX_TYPE_FLOAT32)) ||
                    ((srcType == VX_TYPE_INT8) && (weightsType == VX_TYPE_INT8) && (biasesType == VX_TYPE_INT32 || biasesType == VX_TYPE_FLOAT32)) ||
                    ((srcType == VX_TYPE_INT16) && (weightsType == VX_TYPE_INT16) && (biasesType == VX_TYPE_INT32 || biasesType == VX_TYPE_FLOAT32)))
                {
                    inputValue  = vxnneGetDataExt((vx_type_e)srcType, TENSOR_QUANT_TYPE(inputs), j + b * input_count, (vx_uint8_ptr)inputsBaseLogicalAddr, inputFpPos, TENSOR_TF_ZEROPOINT(inputs), TENSOR_TF_SCALE(inputs));
                    weightValue = vxnneGetDataExt((vx_type_e)weightsType, TENSOR_QUANT_TYPE(weights), input_count * i + j, (vx_uint8_ptr)weightsBaseLogicalAddr, weightFpPos, TENSOR_TF_ZEROPOINT(weights), TENSOR_TF_SCALE(weights));

                    madValue += inputValue * weightValue;
                }
                else if((srcType == VX_TYPE_UINT8) && (weightsType == VX_TYPE_UINT8) && (biasesType == VX_TYPE_INT32 || biasesType == VX_TYPE_FLOAT32) && TENSOR_QUANT_TYPE(inputs) == VX_QUANT_AFFINE_SCALE)
                {
                    inputValue  = vxnneGetDataQuant((vx_type_e)srcType, j + b * input_count, (vx_uint8_ptr)inputsBaseLogicalAddr, TENSOR_TF_ZEROPOINT(inputs), TENSOR_TF_SCALE(inputs));
                    weightValue = vxnneGetDataQuant((vx_type_e)weightsType, input_count * i + j, (vx_uint8_ptr)weightsBaseLogicalAddr, TENSOR_TF_ZEROPOINT(weights), TENSOR_TF_SCALE(weights));

                    madValue += inputValue * weightValue;
                }
                else
                {
                    /* other format not surpport now */
                    vxError("can't support this input data format\n");
                    gcmASSERT(0);
                }
            }

            if (biasesType == VX_TYPE_FLOAT32 || biasesType == VX_TYPE_INT32)
            {
                biasValue = vxnneGetDataExt((vx_type_e)biasesType, TENSOR_QUANT_TYPE(biases), i, (vx_uint8_ptr)biasesBaseLogicalAddr, biasFpPos, TENSOR_TF_ZEROPOINT(biases), TENSOR_TF_SCALE(biases));
            }
            else
            {
                vxError("can't support this bias data format\n");
                gcmASSERT(0);
            }

            result = madValue + biasValue;

            vxnneSaveDataExt((vx_type_e)dstType, TENSOR_QUANT_TYPE(outputs), i + b * output_count, result, outputsBaseLogicalAddr, outputFpPos, TENSOR_TF_ZEROPOINT(outputs), TENSOR_TF_SCALE(outputs), outputRoundingMode);
        }
    }

OnError:
    return status;
}

enum
{
    bfc2conv_mode_1xn = 0,
    bfc2conv_mode_nx1,
};

vx_status vxoFCOperation_Initialize(
    vxnne_fc_operation operation,
    vxnne_layer layer,
    vx_tensor inputs,
    vx_weights_biases_parameter *weights_biases,
    vx_tensor weights,
    vx_tensor biases,
    vx_enum overflow_policy, /* type of vx_convert_policy_e */
    vx_enum rounding_policy, /* type of vx_round_policy_e */
    vx_bool enable_relu,
    vx_tensor outputs,
    vx_uint32_ptr op_index
    )
{
    vx_status status = VX_SUCCESS;

    vx_node node = layer->node;
    vx_context context = vxGetContext((vx_reference)node);
    vx_uint32 batch_count = 1;
    vx_bool enable_shader = vx_false_e;
    vx_bool supportDataFormat0 = vx_false_e;
    vx_bool supportDataFormat1 = vx_false_e;
    vx_bool supportDataFormat2 = vx_false_e;
    vx_bool supportDataFormat3 = vx_false_e;
    vx_enum input_dataformat = TENSOR_DATA_TYPE(inputs);
    vx_enum weight_dataformat;
    vx_enum bias_dataformat;
    vx_enum output_dataformat = TENSOR_DATA_TYPE(outputs);
    vx_uint32 dims = TENSOR_VIEW_DIM_NUM(inputs);
    vx_uint32 width = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
    vx_uint32 height = (dims > 1) ? TENSOR_VIEW_SIZE_INDEX(inputs, 1) : 1;
    vx_uint32 depth = (dims > 2) ? TENSOR_VIEW_SIZE_INDEX(inputs, 2) : 1;
    vx_uint32 inputDims = dims > 2 ? width * height * depth : width;
    vxnne_operation_target_e target = VXNNE_OPERATION_TARGET_NONE;
    vx_weights_biases_parameter wb = VX_NULL;
    vx_weights_biases_parameter_optimizations_t wb_opt;
    vx_weights_biases_parameter_optimizations_t *opt = VX_NULL;
    vx_bool tensor_copy = vx_false_e;
    vx_tensor aligned_tensor = VX_NULL;
    vx_tensor tp_fc_inputs = VX_NULL;

    vx_bool support_1xn = (width % 2 == 0)?vx_true_e:vx_false_e;
    vx_int32 zdp_size = 3;
    vx_enum mode = bfc2conv_mode_1xn;
    gctSTRING env = VX_NULL;

    gcoOS_GetEnv(gcvNULL, "BFC2CONV_MODE", &env);
    if (env != NULL)
    {
        if (gcoOS_StrStr(env, "1xn", VX_NULL))
            mode = bfc2conv_mode_1xn;
        else if (gcoOS_StrStr(env, "nx1", VX_NULL))
            mode = bfc2conv_mode_nx1;
    }

    gcoOS_GetEnv(gcvNULL, "BFC2CONV_ZDP", &env);
    if (env != NULL)
        zdp_size = atoi(env);

    if (mode == bfc2conv_mode_nx1 && !vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP0))
    {
        vxError(" V7 not support Nx1, force mode to 1x%d \n", zdp_size);
        mode = bfc2conv_mode_1xn;
    }
    else if (zdp_size == 2 && vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP0))
    {
        vxError(" V8 not support 2x1 or 1x2, force zdp size = 3\n");
        zdp_size = 3;
    }

    if ((!weights_biases || !*weights_biases) &&
        (!weights || !biases))
    {
        vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
    }


    weight_dataformat = TENSOR_DATA_TYPE(weights);
    bias_dataformat = TENSOR_DATA_TYPE(biases);

    vxmONERROR(_GetBatchAndInOutCountOfFC(inputs,
                                          outputs,
                                          &batch_count,
                                          VX_NULL,
                                          VX_NULL));

    if (node->base.context->evisNoInst.supportEVIS)
    {
        supportDataFormat0 = (vx_bool)(input_dataformat == VX_TYPE_FLOAT16 && weight_dataformat == VX_TYPE_FLOAT16 && bias_dataformat == VX_TYPE_FLOAT32 && output_dataformat == VX_TYPE_FLOAT16);
        supportDataFormat1 = (vx_bool)(input_dataformat == VX_TYPE_INT8 && weight_dataformat == VX_TYPE_INT8 && bias_dataformat == VX_TYPE_INT32 && output_dataformat == VX_TYPE_INT8);
        supportDataFormat2 = (vx_bool)(input_dataformat == VX_TYPE_INT16 && weight_dataformat == VX_TYPE_INT16 && (bias_dataformat == VX_TYPE_INT32 || bias_dataformat == VX_TYPE_INT64) && output_dataformat == VX_TYPE_INT16);
        supportDataFormat3 = (vx_bool)(input_dataformat == VX_TYPE_UINT8 && weight_dataformat == VX_TYPE_UINT8 && bias_dataformat == VX_TYPE_INT32 && output_dataformat != VX_TYPE_FLOAT32);
        enable_shader = (supportDataFormat0 || supportDataFormat1 || supportDataFormat2 || supportDataFormat3) && (inputDims < IMG_MAX_WIDTH);
    }
    else
    {
        supportDataFormat0 = (vx_bool)((input_dataformat == VX_TYPE_FLOAT16 && weight_dataformat == VX_TYPE_FLOAT16 && (bias_dataformat == VX_TYPE_INVALID || bias_dataformat == VX_TYPE_FLOAT32) && output_dataformat == VX_TYPE_FLOAT16) ||
                                        (input_dataformat == VX_TYPE_FLOAT32 && weight_dataformat == VX_TYPE_FLOAT32 && (bias_dataformat == VX_TYPE_INVALID || bias_dataformat == VX_TYPE_FLOAT32) && output_dataformat == VX_TYPE_FLOAT32));
        supportDataFormat3 = (vx_bool)(input_dataformat == VX_TYPE_UINT8 && weight_dataformat == VX_TYPE_UINT8 && (bias_dataformat == VX_TYPE_INVALID || bias_dataformat == VX_TYPE_INT32) &&
                                        ((output_dataformat == VX_TYPE_UINT8) || (output_dataformat == VX_TYPE_INT16)));

        enable_shader      = (supportDataFormat0 || supportDataFormat3);
    }

    /* Choose the accelerator. */
    if (vxnneIsNNSupportFormat(context, node->graph, inputs, VX_NULL, outputs) && batch_count > 1 && support_1xn)
    {
        target = VXNNE_OPERATION_TARGET_NN;
    }
    else if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_SINGLE_FC) &&
        vxnneIsTPSupportFormat(node->graph, inputs, VX_NULL, outputs))
    {
        target = VXNNE_OPERATION_TARGET_TP;
    }
    else if (enable_shader && vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_SHADER))
    {
        target = VXNNE_OPERATION_TARGET_SH;
    }
    else
    {
        target = VXNNE_OPERATION_TARGET_SW;
    }


    /* Prepare for the tensor-copy operation for the non-64byte-aligned TP FC inputs. */
    if (target == VXNNE_OPERATION_TARGET_TP)
    {
        vx_uint32 input_size;
        vx_uint32 input_unit_size;
        vx_uint32 offset = 0;

        vxmONERROR(vxoTensor_GetTensorViewOffset(inputs, &offset));

        vxmONERROR(vxoTensor_GetTensorSize(inputs, &input_size));
        input_unit_size = input_size / batch_count;

        if (!vxmIS_ALIGNED(offset, 64) ||
            ((batch_count > 1) && (!vxmIS_ALIGNED(input_unit_size, 64))))
        {
            vx_tensor_create_params_t params;
            vx_uint32 strides[6] = { 0 };

            tensor_copy = vx_true_e;

            vxnneGetAlignStrides(inputs, strides, vx_false_e);

            vxZeroMemory(&params, gcmSIZEOF(vx_tensor_create_params_t));
            params.num_of_dims = TENSOR_DIM_NUM(inputs);
            params.sizes = TENSOR_SIZES(inputs);
            params.data_format = TENSOR_DATA_TYPE(inputs);
            params.quant_format = TENSOR_QUANT_TYPE(inputs);
            if (params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
            {
                params.quant_data.dfp.fixed_point_pos = TENSOR_POS(inputs);
            }
            else
            {
                params.quant_data.affine.scale = TENSOR_TF_SCALE(inputs);
                params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(inputs);
            }

            aligned_tensor = vxoTensor_CreateTensorWithStrides(context, node->graph, &params, strides, vx_true_e);
            if (!aligned_tensor)
            {
                vxError("%s() - %d: Failed to create tensor.", __FUNCTION__, __LINE__);
                vxmONERROR(VX_ERROR_NO_RESOURCES);
            }

            layer->temp_tensors[layer->num_temp_tensors++] = aligned_tensor;

            tp_fc_inputs = aligned_tensor;
        }
        else
        {
            tp_fc_inputs = inputs;
        }
    }

    switch (target)
    {
    case VXNNE_OPERATION_TARGET_TP:
        if (tensor_copy)
        {
            vxmONERROR(vxoTensorCopyOperationTP_Initialize(&operation->copy_tp_operation,
                                                           layer,
                                                           inputs,
                                                           1,
                                                           aligned_tensor,
                                                           op_index));
        }

        if (wb == VX_NULL)
        {
            if (weights_biases && *weights_biases)
            {
                wb = *weights_biases;

                weights = WB_WEIGHT_TENSOR(wb);
                biases = WB_BIAS_TENSOR(wb);
            }
            else if (!weights_biases || !*weights_biases)
            {
                if (TENSOR_QUANT_TYPE(inputs) == VX_QUANT_AFFINE_SCALE)
                {
                    vxZeroMemory(&wb_opt, gcmSIZEOF(wb_opt));
                    wb_opt.inputZeroPoint = TENSOR_TF_ZEROPOINT(inputs);
                    wb_opt.zrl = -1;
                    wb_opt.outputFormat = TENSOR_DATA_TYPE(outputs);

                    opt = &wb_opt;
                }

                wb = vxoCreateWeightsBiasesParameterFromTensors(context,
                                                              VX_NN_FULLYCONNECTED_LAYER,
                                                              inputs->dims, /* inputs_dims */
                                                              inputs->dimCount,
                                                              outputs->dimCount,
                                                              0,
                                                              0,
                                                              0,
                                                              0,
                                                              0, /* pooling_size_x */
                                                              0, /* pooling_size_y */
                                                              0,
                                                              0,
                                                              VX_NN_DS_SIZE_ROUNDING_FLOOR,
                                                              outputs->dims, /* convolution_outputs_dims */
                                                              outputs->dims, /* pool_outputs_dims */
                                                              opt, /*optimizations,*/
                                                              0,
                                                              TENSOR_DATA_TYPE(outputs),
                                                              0,
                                                              VX_TENSOR_RANK_WHCN,
                                                              weights,
                                                              biases,
                                                              VX_NULL,
                                                              VX_NN_CONV_ONLY,
                                                              vx_false_e);

                if (!wb)
                {
                    vxmONERROR(VX_ERROR_NO_RESOURCES);
                }

                vxmONERROR(wb->compress(wb, VXNNE_OPERATION_TARGET_TP, 0, TENSOR_STRIDE_INDEX(outputs, 2)));

                operation->weights_biases = wb;

                if (weights_biases)
                {
                    *weights_biases = wb;
                }
            }
        }

        vxmONERROR(vxoFCOperationTP_Initialize(&operation->tp_operation,
                                               &operation->aux_tp_operation,
                                               layer,
                                               tp_fc_inputs,
                                               batch_count,
                                               wb,
                                               overflow_policy,
                                               rounding_policy,
                                               enable_relu,
                                               outputs,
                                               op_index));
        if (context->options.enableMultiVIPCombined && node->graph->gpuCount > 1 && wb)
        {
            wb->free_compressed(wb);
        }
        break;

    case VXNNE_OPERATION_TARGET_NN:
    {
        vx_bool aligned = (TENSOR_SIZE_INDEX(inputs, 0) % zdp_size == 0) ? vx_true_e : vx_false_e;
        vx_tensor input_conv = VX_NULL, input_reshape = VX_NULL, input_align = VX_NULL, input_copy = VX_NULL, input_trans = VX_NULL;
        vx_tensor output_trans = VX_NULL, outputs_reshape = VX_NULL;

        /***************************************************************************************************************************************
         *            BatchFC to Conv [1XN] : zdp_size = 3 batch = 378 input_size = 1312 output_size = 4096
         *
         *
         *                                          _________________
         *                                         |                 |
         *                                         |                 |                     _________________
         *                                         |                 |                   _|_______________  |
         *                                         |                 |  reshpe         _|______________   | |
         *                                         |                 |  <===         _|_______________  | |_|
         *                                         |                 |             _|_______________  | |_|
         *                                         |                 |           _|_______________  | |_|
         *                                         |                 |          |                 | |_|   _
         *                                         |                 |          |                 |_|      |
         *                                         |_________________| 1312     |_________________|        |
         *                                         |_________________| 1314         378 x 3 x 438          |
         *                                             378 x 1314                    input_conv            |
         *                                           input_reshape                                         |
         *                                                 ||  view                                        |                                               _________________           _________________________
         *                                                 \/                                              |                                              |                 |         |                         |
         *      _________________________           _________________                                      |                  ________________            |                 |         |                         |
         *     |                         |         |                 |                                     |                _|______________ _|    reshpe |                 |transpose|                         |
         *     |                         |         |                 |                                     |              _|______________ _|          => |                 | =>      |                         |
         *     |                         |transpose|                 |                                     |            _|_______________ _|              |                 |         |                         |
         *     |                         |   ==>   |                 |                                     | conv     _|_______________ _|                |                 |         |                         |
         *     |                         |         |                 |                                     | ===>   _|_______________ _|                  |                 |         |                         |
         *     |                         |         |                 |                                     |       |_________________|                    |                 |         |                         |
         *     |                         |         |                 |                                     |                                              |                 |         |_________________________|
         *     |                         |         |                 |                                     |          378 x 1 x 4096                      |                 |                 4096 x 378
         *     |_________________________|         |                 |                                     |           output_trans                       |_________________|                   OUTPUTS
         *             1312 x 378                  |                 |                                     |                                                   378 x 4096
         *              INPUTS                     |_________________|                                     |                                                 outputs_reshape
         *                                               378 x 1312                                        |
         *                                               input_trans                          ____         |
         *           _________________              ___________________                     _|__  |        |
         *          |                 |            |       1312      | |                  _|__  | |        |
         *          |                 |            |                 | |                _|__  | |_|        |
         *          |                 |     view   |                 | | reshpe       _|__  | |_|          |
         *          |                 |      ==>   |                 | | <===       _|__  | |_|            |
         *          |                 |            |                 | |           |    | |_|             _|
         *          |                 |            |                 | |           |    |_|   ____
         *          |                 |            |                 | |           |____|   _|__  |
         *          |                 |            |                 | |             .    _|__  | |
         *          |                 |            |                 | |             .  _|__  | |_|
         *          |_________________|            |_________________|_|             ._|__  | |_|
         *               1312 x 4096                    1314 x 4096                 _|__  | |_|
         *                 WEIGHTS                      _weights_pre               |    | |_|
         *                                                                         |    |_|
         *                                                                         |____|
         *                                                                  1 x 3 x 438 x 4096
         *                                                                      _weights
         *
         *
         *            BatchFC to Conv [Nx1] : zdp_size = 3 batch = 378 input_size = 1312 output_size = 4096
         *
         *                 ______________________                          ___
         *                |                      |                       _|_  |
         *                |                      |                     _|_  | |                input_conv
         *                |                      | reshap            _|_  | | |                      ___
         *                |       aligned        |  ==>            _|_  | | | |                    _|_  |
         *                |                      |               _|_  | | | | |     1<=>2        _|_  | |
         *                |                      |              |   | | | | | |   transpose    _|_  | | |
         *                |                      |              |   | | | | |_|      =>      _|_  | | | |
         *                |                      |              |   | | | |_|              _|_  | | | | |
         *                |______________________|              |   | | |_|               |   | | | | | | _
         *                        1312 x 378                    |   | |_|                 |   | | | | |_|  |
         *                         inputs                       |   |_|                   |   | | | |_|    |
         *                                                      |___|                     |   | | |_|      |
         *                                              3 x 438 x 378  input_reshape      |   | |_|        |
         *                                                                                |   |_|          |
         *    ----------------------------------------------------------   /\             |___|            |                                               _________________           _________________________
         *                                                                 ||                3 x 378 x 438 |                                              |                 |         |                         |
         *   ______________________         _____________________        _________________________         |                  ________________            |                 |         |                         |
         *  |                      |       |                     |      |                       | |        |                _|______________ _|    reshpe |                 |transpose|                         |
         *  |                      |       |                     |      |                       | |        |              _|______________ _|          => |                 | =>      |                         |
         *  |                      | copy  |                     | view |                       | |        |            _|_______________ _|              |                 |         |                         |
         *  |      unaligned       | ==>   |                     | <==  |                       | |        | conv     _|_______________ _|                |                 |         |                         |
         *  |                      |       |                     |      |                       | |        | ===>   _|_______________ _|                  |                 |         |                         |
         *  |                      |       |                     |      |                       | |        |       |_________________|                    |                 |         |                         |
         *  |                      |       |                     |      |                       | |        |                                              |                 |         |_________________________|
         *  |                      |       |                     |      |                       | |        |          1 x 378 x 4096                      |                 |                 4096 x 378
         *  |______________________|       |_____________________|      |_______________________|_|        |           output_trans                       |_________________|                   OUTPUTS
         *          1312 x 378                    1312 x 378                    1314 x 378                 |                                                   378 x 4096
         *           inputs                       input_copy                  input_align                  |                                                 outputs_reshape
         *                                                                                                 |
         *                                               input_trans                          ____         |
         *           _________________              ___________________                     _|__ _|        |
         *          |                 |            |       1312      |0|                  _|__ _|          |
         *          |                 |            |                 |0|                _|__ _|            |
         *          |                 |     view   |                 |0| reshpe       _|__ _|              |
         *          |                 |      ==>   |                 |0| <===       _|__ _|                |
         *          |                 |            |                 |0|           |____|                 _|
         *          |                 |            |                 |0|                      ____
         *          |                 |            |                 |0|             .      _|__ _|
         *          |                 |            |                 |0|             .    _|__ _|
         *          |                 |            |                 |0|             .  _|__ _|
         *          |_________________|            |_________________|0|              _|__ _|
         *               1312 x 4096                    1314 x 4096                 _|__ _|
         *                 WEIGHTS                      _weights_pre               |____|
         *
         *
         *                                                                  3 x 1 x 438 x 4096
         *                                                                      _weights
         ***************************************************************************************************************************************/
        vx_uint32 inputs_sizes_1xn[][4] = {
            {TENSOR_SIZE_INDEX(inputs, 1), zdp_size, gcmALIGN_NP2(TENSOR_SIZE_INDEX(inputs, 0), zdp_size)/zdp_size, 1}, /* input_conv(input aligned)  : 80 x 2 x 45(378 x 3 x 438)  */
            {TENSOR_SIZE_INDEX(inputs, 1), gcmALIGN_NP2(TENSOR_SIZE_INDEX(inputs, 0), zdp_size), 1, 1}, /* input_reshape(input viewed)   : 80 x 90 (378 x 1312 => 378 x 1314) */
        };

        vx_uint32 inputs_sizes_nx1[][4] = {
            {zdp_size, TENSOR_SIZE_INDEX(inputs, 1), gcmALIGN_NP2(TENSOR_SIZE_INDEX(inputs, 0), zdp_size)/zdp_size, 1}, /* input_conv(input aligned)  : 3 x 378 x 438 x 1 */
            {zdp_size, gcmALIGN_NP2(TENSOR_SIZE_INDEX(inputs, 0), zdp_size)/zdp_size, TENSOR_SIZE_INDEX(inputs, 1), 1}, /* input_reshape(input reshape)  : 3 x 438 x 378 x 1 */
            {gcmALIGN_NP2(TENSOR_SIZE_INDEX(inputs, 0), zdp_size), TENSOR_SIZE_INDEX(inputs, 1), 1, 1}, /* input_align(input viewed)   : 1312 x 378 => 1314 x 378 x 1 x 1*/
        };

        vx_int32 weights_sizes[][4] = {
            {gcmALIGN_NP2(TENSOR_SIZE_INDEX(weights, 0), zdp_size), TENSOR_SIZE_INDEX(weights, 1), 1, 1}, /* _weights_pre(weights transpose): 90 x 400 (1312 x 4096 => 1314 x 4096)  */
            {(mode == bfc2conv_mode_1xn)?
1:zdp_size, (mode == bfc2conv_mode_1xn)?
zdp_size:
1, gcmALIGN_NP2(TENSOR_SIZE_INDEX(weights, 0), zdp_size)/zdp_size, TENSOR_SIZE_INDEX(weights, 1)},/* _weights(aligned weights)  :
 1 x 2 x 45 x 400(1 x 3 x 438 x 4096/3 x 1 x 438 x 4096)  */
        };

        vx_uint32 outputs_sizes[][4] = {
            {(mode == bfc2conv_mode_1xn)?TENSOR_SIZE_INDEX(outputs, 1):1, (mode == bfc2conv_mode_1xn)?1:TENSOR_SIZE_INDEX(outputs, 1), TENSOR_SIZE_INDEX(outputs, 0), 1}, /* output_trans(output reshape)   : 378 x 1 x 4096/1 x 378 x 4096 */
            {TENSOR_SIZE_INDEX(outputs, 1), TENSOR_SIZE_INDEX(outputs, 0), 1, 1}, /* outputs_reshape(output transpose) : 378 x 4096 => 4096x378x1 */
        };

        if (mode == bfc2conv_mode_1xn)
        {
            vx_uint32 start[4] = {0, 0, 0, 0}, end[4] = {TENSOR_SIZE_INDEX(inputs, 1), TENSOR_SIZE_INDEX(inputs, 0), 0, 0};
            vx_tensor_view view = vxCreateTensorView(context, start, end, 4);
            input_conv    = _createSimilarTensor(layer->node->graph, vx_false_e, gcmCOUNTOF(inputs_sizes_1xn[0]), inputs_sizes_1xn[0], inputs);
            input_reshape = vxoTensor_ReshapeTensor(input_conv, (vx_int32_ptr)inputs_sizes_1xn[1], gcmCOUNTOF(inputs_sizes_1xn[1]), VX_NULL);
            input_trans   = vxoTensor_CreateTensorFromView(input_reshape, view);

            vxReleaseTensorView(&view);

        vxmONERROR(vxoTensor_AllocateMemory(input_trans));
        }
        else if (mode == bfc2conv_mode_nx1)
        {
            input_conv    = _createSimilarTensor(layer->node->graph, vx_false_e, gcmCOUNTOF(inputs_sizes_nx1[0]), inputs_sizes_nx1[0], inputs);
            if (aligned)
            {
                input_copy    = inputs;
                input_reshape = vxoTensor_ReshapeTensor(inputs, (vx_int32_ptr)inputs_sizes_nx1[1], gcmCOUNTOF(inputs_sizes_nx1[1]), VX_NULL);
            }
            else
            {
                vx_uint32 start[4] = {0, 0, 0, 0}, end[4] = {TENSOR_SIZE_INDEX(inputs, 0), TENSOR_SIZE_INDEX(inputs, 1), 0, 0};
                vx_tensor_view view = vxCreateTensorView(context, start, end, 4);

                input_align   = _createSimilarTensor(layer->node->graph, vx_false_e, gcmCOUNTOF(inputs_sizes_nx1[2]), inputs_sizes_nx1[2], inputs);
                input_reshape = vxoTensor_ReshapeTensor(input_align, (vx_int32_ptr)inputs_sizes_nx1[1], gcmCOUNTOF(inputs_sizes_nx1[1]), VX_NULL);
                input_copy    = vxoTensor_CreateTensorFromView(input_align, view);

                vxReleaseTensorView(&view);

            }

            vxmONERROR(vxoTensor_AllocateMemory(input_conv));
        }

        output_trans  = _createSimilarTensor(layer->node->graph, vx_false_e, gcmCOUNTOF(outputs_sizes[0]), outputs_sizes[0], outputs);
        outputs_reshape = vxoTensor_ReshapeTensor(output_trans, (vx_int32_ptr)outputs_sizes[1], gcmCOUNTOF(outputs_sizes[1]), VX_NULL);
        vxmONERROR(vxoTensor_AllocateMemory(output_trans));

        if (input_conv)     layer->temp_tensors[layer->num_temp_tensors++] = input_conv;
        if (input_reshape)  layer->temp_tensors[layer->num_temp_tensors++] = input_reshape;
        if (input_align)    layer->temp_tensors[layer->num_temp_tensors++] = input_align;
        if (input_trans)    layer->temp_tensors[layer->num_temp_tensors++] = input_trans;
        if (input_copy)     layer->temp_tensors[layer->num_temp_tensors++] = input_copy;
        if (outputs_reshape)layer->temp_tensors[layer->num_temp_tensors++] = outputs_reshape;
        if (output_trans)   layer->temp_tensors[layer->num_temp_tensors++] = output_trans;

        if (wb == VX_NULL)
        {

            vx_tensor _weights          = _createSimilarTensor(layer->node->graph, vx_false_e, gcmCOUNTOF(weights_sizes[1]), (vx_uint32_ptr)weights_sizes[1], weights);
            vx_tensor _weights_pre      = vxoTensor_ReshapeTensor(_weights, weights_sizes[0], gcmCOUNTOF(weights_sizes[0]), VX_NULL);
            vx_uint32 _start[4] = {0, 0, 0, 0}, _end[4] = {TENSOR_SIZE_INDEX(weights, 0), TENSOR_SIZE_INDEX(weights, 1), 0, 0};
            vx_tensor_view _view = vxCreateTensorView(context, _start, _end, 4);
            vx_tensor _weights_trans    = vxoTensor_CreateTensorFromView(_weights_pre, _view);

            vxReleaseTensorView(&_view);

            layer->temp_tensors[layer->num_temp_tensors++] = _weights_pre;
            layer->temp_tensors[layer->num_temp_tensors++] = _weights;
            layer->temp_tensors[layer->num_temp_tensors++] = _weights_trans;

            vxmONERROR(vxoTensor_AllocateMemory(_weights));

            vxmONERROR(vxoFCOperation_CopyTensor2D(weights, _weights_trans, vx_true_e));

            if (weights_biases && *weights_biases)
            {
                wb = *weights_biases;

                weights = WB_WEIGHT_TENSOR(wb);
                biases = WB_BIAS_TENSOR(wb);
            }
            else if (!weights_biases || !*weights_biases)
            {
                if (TENSOR_QUANT_TYPE(inputs) == VX_QUANT_AFFINE_SCALE)
                {
                    vxZeroMemory(&wb_opt, gcmSIZEOF(wb_opt));
                    wb_opt.inputZeroPoint = TENSOR_TF_ZEROPOINT(inputs);
                    wb_opt.zrl = -1;
                    wb_opt.outputFormat = TENSOR_DATA_TYPE(output_trans);

                    opt = &wb_opt;
                }

                wb = vxoCreateWeightsBiasesParameterFromTensors(context,
                                                              VX_NN_CONVOLUTION_LAYER,
                                                              (mode == bfc2conv_mode_1xn)?input_trans->dims:input_conv->dims, /* inputs_dims */
                                                              (mode == bfc2conv_mode_1xn)?input_trans->dimCount:input_conv->dimCount,
                                                              output_trans->dimCount,
                                                              0,
                                                              0,
                                                              0,
                                                              0,
                                                              0, /* pooling_size_x */
                                                              0, /* pooling_size_y */
                                                              0,
                                                              0,
                                                              VX_NN_DS_SIZE_ROUNDING_FLOOR,
                                                              output_trans->dims, /* convolution_outputs_dims */
                                                              output_trans->dims, /* pool_outputs_dims */
                                                              opt, /*optimizations,*/
                                                              0,
                                                              TENSOR_DATA_TYPE(output_trans),
                                                              0,
                                                              VX_TENSOR_RANK_WHCN,
                                                              _weights,
                                                              biases,
                                                              VX_NULL,
                                                              VX_NN_CONV_ONLY,
                                                              vx_false_e);

                if (!wb)
                {
                    vxmONERROR(VX_ERROR_NO_RESOURCES);
                }

                vxmONERROR(wb->compress(wb, VXNNE_OPERATION_TARGET_NN, WB_OUTPUT_Z(wb), TENSOR_STRIDE_INDEX(output_trans, 2)));

                operation->weights_biases = wb;

                if (weights_biases)
                {
                    *weights_biases = wb;
                }
            }
        }

        if (wb)
        {
            vx_uint32 perm[] = {0, 2, 1, 3}; /*transpose dim[1] and dim[2]*/
            vx_uint32 perm1[] = {1, 0, 2, 3}; /*transpose dim[0] and dim[1]*/

            if (!aligned && (mode == bfc2conv_mode_nx1))
            {
                vxmONERROR(vxnneExecuteCopy(node, layer,
                                                    &operation->nn_operation_cp_tp_operation,
                                                    &operation->nn_operation_cp_sw_operation,
                                                    VX_NULL,
                                                    inputs,
                                                    input_copy,
                                                    vx_false_e,
                                                    (vx_int32_ptr)op_index,
                                                    1));
            }
            vxmONERROR(vxoFC_NN_Trans_Initialize(&operation->nn_trans_tp_operation[0],
                                                    &operation->nn_trans_sh_operation[0],
                                                    &operation->nn_trans_sw_operation[0],
                                                    layer,
                                                    (mode == bfc2conv_mode_1xn)?inputs:input_reshape,
                                                    (mode == bfc2conv_mode_1xn)?perm1:perm,
                                                    1/*batch_count*/,
                                                    &node->kernelAttributes.borderMode,
                                                    (mode == bfc2conv_mode_1xn)?input_trans:input_conv,
                                                    op_index));
            vxmONERROR(vxoFCOperationNN_Initialize(&operation->nn_operation,
                                                    &operation->nn_operation_sw,
                                                    layer,
                                                    input_conv,
                                                    1/*batch_count*/,
                                                    wb,
                                                    overflow_policy,
                                                    rounding_policy,
                                                    enable_relu,
                                                    output_trans,
                                                    op_index));

            vxmONERROR(vxoFC_NN_Trans_Initialize(&operation->nn_trans_tp_operation[1],
                                                    &operation->nn_trans_sh_operation[1],
                                                    &operation->nn_trans_sw_operation[1],
                                                    layer,
                                                    outputs_reshape,
                                                    perm1,
                                                    1/*batch_count*/,
                                                    &node->kernelAttributes.borderMode,
                                                    outputs,
                                                    op_index));
        }
    }
        break;

    case VXNNE_OPERATION_TARGET_SH:
        vxmONERROR(vxoFCOperationSH_Initialize(&operation->sh_operation,
                                               layer,
                                               inputs,
                                               batch_count,
                                               weights,
                                               biases,
                                               overflow_policy,
                                               rounding_policy,
                                               enable_relu,
                                               outputs,
                                               op_index));
        break;

    case VXNNE_OPERATION_TARGET_SW:
        vxmONERROR(vxoFCOperationSW_Initialize(&operation->sw_operation,
                                               layer,
                                               inputs,
                                               batch_count,
                                               weights,
                                               biases,
                                               overflow_policy,
                                               rounding_policy,
                                               enable_relu,
                                               outputs,
                                               op_index));
        break;

    default:
        vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
        break;
    }

OnError:
    return status;
}

vx_status vxoFCOperation_Deinitialize(
    vxnne_fc_operation operation
    )
{
    vx_status status = VX_SUCCESS;

    if (operation->weights_biases)
    {
        vxReleaseWeightsBiasesParameter(&operation->weights_biases);
    }

    return status;
}

vx_status vxoFCOperationTP_Initialize(
    vxnne_tp_operation operation,
    vxnne_tp_operation aux_operation,
    vxnne_layer layer,
    vx_tensor inputs,
    vx_uint32 batch_count,
    vx_weights_biases_parameter weights_biases,
    vx_enum overflow_policy,
    vx_enum rounding_policy,
    vx_bool enable_relu,
    vx_tensor outputs,
    vx_uint32_ptr op_index
    )
{
    vx_status status = VX_SUCCESS;

    vx_uint32 kzgroup = WB_KERNEL_Z_SLICE_NUM(weights_biases);
    vx_op_param op_param = VX_NULL;
    vx_tp_value_cmd tp_value = VX_NULL;
    vx_tensor_create_params_t tensor_create_params;
    vx_uint32 zoffset = 0, kzoffset = 0, kzoffset2 = 0;
    vx_tensor fc_outputs = outputs;
    vx_tensor tmp_buffer = VX_NULL;
    vx_uint32 i;

    if (!op_index)
    {
        vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
    }

    if (kzgroup > 1)
    {
        /* Need a tmp buffer since it is broken down into 2 separated operations. */
        vx_uint32 size = 0;
        vx_enum data_format = TENSOR_DATA_TYPE(outputs);
        vx_enum quant_format = TENSOR_QUANT_TYPE(outputs);

        vxmONERROR(vxoTensor_GetTensorSize(outputs, &size));
        size *= kzgroup;

        vxZeroMemory(&tensor_create_params, gcmSIZEOF(vx_tensor_create_params_t));

        tensor_create_params.num_of_dims  = 1;
        tensor_create_params.sizes        = &size;
        tensor_create_params.data_format  = data_format;
        tensor_create_params.quant_format = quant_format;
        if (quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
        {
            tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(outputs);
        }
        else if (quant_format == VX_QUANT_AFFINE_SCALE)
        {
            tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(outputs);
            tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(outputs);
        }

        tmp_buffer = vxoTensor_CreateVirtualTensor2(layer->node->graph, &tensor_create_params, gcmSIZEOF(tensor_create_params));
        if (!tmp_buffer)
        {
            vxError("%s() - %d: Failed to create virtual tensor.", __FUNCTION__, __LINE__);
            vxmONERROR(VX_ERROR_NO_RESOURCES);
        }

        layer->temp_tensors[layer->num_temp_tensors++] = tmp_buffer;

        fc_outputs = tmp_buffer;
    }

    /* FC operation. */
    vxmONERROR(vxnneOperation_Initialize(&operation->base,
                                         layer,
                                         VXNNE_OPERATION_TARGET_TP,
                                         VXNNE_OPERATOR_FULLYCONNECTED,
                                         VX_NULL,
                                         vxnneOperation_TP_Deinitialize,
                                         batch_count,
                                         0));

    op_param = &operation->base.parameter;

    op_param->pad_x_left = 0;
    op_param->pad_y_top = 0;
    op_param->pad_x_right = 0;
    op_param->pad_y_bottom = 0;
    op_param->pool_size_x = 0;
    op_param->pool_size_y = 0;
    op_param->pool_stride = 1;
    op_param->enable_relu = enable_relu;
    op_param->pad_mode = VX_PAD_CONSTANT;
    op_param->pad_const = 0;
    op_param->tpType = TP_SINGLE_FC;
    op_param->other_ref = (vx_reference)weights_biases;;
    op_param->data_buff = VX_NULL;
    op_param->conv_rounding_type = rounding_policy;

    tp_value = (vx_tp_value_cmd)vxAllocateAndZeroMemory(gcmSIZEOF(vx_tp_value_cmd_s) * WB_TOTAL_SLICE_NUM(weights_biases));
    vxmONERROR_NULLPTR(tp_value);

    for (i = 0; i < WB_TOTAL_SLICE_NUM(weights_biases); i++)
    {
        tp_value[i].e32[0] = 0; /* 0: FC, 1: Sum filter. */
        tp_value[i].u32[0] = kzgroup;
        tp_value[i].u32[1] = zoffset;
        tp_value[i].u32[2] = kzoffset;
        tp_value[i].u32[3] = kzoffset2;

        /* Advance to the next. */
        if ((i + 1) % kzgroup)
        {
            kzoffset += WB_KERNEL_Z_INDEX(weights_biases, i);
            kzoffset2 += WB_OUTPUT_Z(weights_biases);
        }
        else
        {
            kzoffset = kzoffset2 = 0;
            zoffset += WB_OUTPUT_Z_INDEX(weights_biases, i);
        }
    }

    operation->base.parameter.tp_value = tp_value;
    operation->input = inputs;
    operation->weights_biases = weights_biases;
    operation->output = fc_outputs;

    vxmONERROR(vxnneLayer_SetOperation(layer,
                                       &operation->base,
                                       (*op_index)++));

    vxnneOperation_AddReference(&operation->base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
    vxnneOperation_AddReference(&operation->base, (vx_reference)fc_outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

    /* Sum filter. */
    if (kzgroup > 1)
    {
        vxmONERROR(vxnneOperation_Initialize(&aux_operation->base,
                                             layer,
                                             VXNNE_OPERATION_TARGET_TP,
                                             VXNNE_OPERATOR_FULLYCONNECTED,
                                             VX_NULL,
                                             vxnneOperation_TP_Deinitialize,
                                             batch_count,
                                             0));

        tp_value = (vx_tp_value_cmd)vxAllocateAndZeroMemory(gcmSIZEOF(vx_tp_value_cmd_s));
        if (!tp_value)
        {
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }

        tp_value->e32[0] = 1; /* 0: FC, 1: Sum filter. */
        tp_value->u32[0] = kzgroup;
        tp_value->u32[1] = WB_OUTPUT_Z(weights_biases);

        aux_operation->base.parameter.tp_value = tp_value;
        aux_operation->input = fc_outputs;
        aux_operation->weights_biases = weights_biases;
        aux_operation->output = outputs;

        vxmONERROR(vxnneLayer_SetOperation(layer,
                                           &aux_operation->base,
                                           (*op_index)++));

        vxnneOperation_AddReference(&aux_operation->base, (vx_reference)fc_outputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&aux_operation->base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }

    return status;

OnError:
    if (tmp_buffer)
    {
        vxoTensor_ReleaseTensor(&tmp_buffer);
    }

    if (operation->base.parameter.tp_value)
    {
        vxFree(operation->base.parameter.tp_value);
        operation->base.parameter.tp_value = VX_NULL;
    }

    if (aux_operation && aux_operation->base.parameter.tp_value)
    {
        vxFree(aux_operation->base.parameter.tp_value);
        aux_operation->base.parameter.tp_value = VX_NULL;
    }

    return status;
}

vx_status vxoFC_NN_Trans_Initialize(
    vxnne_tp_operation operation_tp,
    vxnne_shader_operation operation_sh,
    vxnne_tensor_trans_operation operation_sw,
    vxnne_layer layer,
    vx_tensor inputs,
    vx_uint32_ptr perms,
    vx_uint32 batch_count,
    vx_border_t* border_mode,
    vx_tensor outputs,
    vx_uint32_ptr op_index
    )
{
    vx_status status = VX_SUCCESS;
    vx_context context = vxGetContext((vx_reference)inputs);
    vx_int32 batch = batch_count;
    vx_uint32 _sizes[] = {4, 1, 1, 1};
    vx_tensor perm = _createTensor(layer->node->graph, vx_false_e, 4, _sizes, VX_TYPE_INT32, VX_QUANT_DYNAMIC_FIXED_POINT, 0, 1.0f, 0);
    vx_uint32_ptr ptr_base = VX_NULL;
    vx_bool enable_sw = vx_false_e;

    layer->temp_tensors[layer->num_temp_tensors++] = perm;

    vxmONERROR(vxoTensor_AllocateMemory(perm));

    vxmONERROR(vxoTensor_GetTensorViewMemory(perm, (gctPOINTER*)&ptr_base, VX_NULL));

    gcoOS_MemCopy(ptr_base, perms, sizeof(vx_uint32) * 4);

    if (enable_sw)
    {
        vx_array perm = vxCreateArray(context, VX_TYPE_UINT32, 4);
        vx_scalar pnum = vxCreateScalar(context, VX_TYPE_INT32, &TENSOR_DIM_NUM(inputs));

        vxoArray_AllocateMemory(perm);

        memcpy(perm->memory.logicals[0], ptr_base, 4 * sizeof(vx_int32));

        vxnneOperation_Initialize(&operation_sw->base,
                                  layer,
                                  VXNNE_OPERATION_TARGET_SW,
                                  VXNNE_OPERATOR_TENSOR_TRANS,
                                  vxnneExecuteSWTensorTranspose,
                                  VX_NULL,
                                  batch_count,
                                  0);

        vxnneLayer_SetOperation(
            layer,
            &operation_sw->base,
            (*op_index) ++);

        operation_sw->input   = inputs;
        operation_sw->perm    = perm;
        operation_sw->pnum    = pnum;
        operation_sw->output  = outputs;

        vxnneOperation_AddReference(&operation_sw->base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&operation_sw->base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }
    else if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP) && 0)
    {
        vxnne_tp_operation tp_op = (vxnne_tp_operation)operation_tp;
        vx_op_param_s conv = { 0 };
        vx_tp_value_cmd values;
        vx_uint32 dnum = 4;

        vxmONERROR(vxnneOperation_Initialize(&tp_op->base,
            layer,
            VXNNE_OPERATION_TARGET_TP,
            VXNNE_OPERATOR_TENSOR_TRANS,
            VX_NULL,
            vxnneOperation_TP_Deinitialize,
            batch,
            0));

        vxmONERROR(vxnneLayer_SetOperation(
            layer,
            &tp_op->base,
            (*op_index) ++));

        tp_op->input = inputs;
        tp_op->output = outputs;

        vxmONERROR(vxnneOperation_AddReference(&tp_op->base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT));
        vxmONERROR(vxnneOperation_AddReference(&tp_op->base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

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
        conv.tpType = TP_TRANSPOSE;
        conv.other_ref = (vx_reference)inputs;
        conv.data_buff = gcvNULL;

        conv.tp_value = (vx_tp_value_cmd)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
        values = (vx_tp_value_cmd)conv.tp_value;
        values->u32[0] = dnum;
        values->p8[0] = (vx_uint8_ptr)vxAllocateAndZeroMemory(sizeof(vx_uint32) * dnum);
        vxMemCopy(values->p8[0], ptr_base, sizeof(vx_uint32) * dnum);

        vxMemCopy(&tp_op->base.parameter, &conv, sizeof(vx_op_param_s));
    }
    else
    {
        vxnne_shader_operation sh_op = (vxnne_shader_operation)operation_sh;
        vxnne_shader_executable shaderExecutable = VX_NULL;

        shaderExecutable = vxnneTensorTransposeShaderExecutable(context, VXNNE_KERNEL_TENSOR_TRANSPOSE, border_mode, inputs, ptr_base, 3, outputs);
        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto OnError;
        }

        vxmONERROR(vxnneShaderOperation_Initialize(sh_op,
            layer,
            VXNNE_OPERATOR_TENSOR_TRANS,
            batch,
            shaderExecutable));

        vxmONERROR(vxnneOperation_AddReference(&sh_op->base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT));
        vxmONERROR(vxnneOperation_AddReference(&sh_op->base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

        vxmONERROR(vxnneLayer_SetOperation(
            layer,
            &sh_op->base,
            (*op_index) ++));
    }

OnError:
    return status;
}

vx_status vxoFCOperationNN_Initialize(
    vxnne_convolution_relu_pooling_operation operation,
    vxnne_convolution_operation operation_sw,
    vxnne_layer layer,
    vx_tensor inputs,
    vx_uint32 batch_count,
    vx_weights_biases_parameter weights_biases,
    vx_enum overflow_policy,
    vx_enum rounding_policy,
    vx_bool enable_relu,
    vx_tensor outputs,
    vx_uint32_ptr op_index
    )
{
    vx_status status = VX_SUCCESS;

    vx_op_param op_param = VX_NULL;
    vx_uint32 input_unit_count, output_unit_count;
    vx_tensor fc_inputs = inputs, fc_outputs = outputs;
    vx_bool enable_sw_convolution = vx_false_e;

    if (!op_index)
    {
        vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
    }

    if (enable_sw_convolution)
    {
        vx_context context = vxGetContext((vx_reference)layer->node);
        vx_int32  pad = 0;
        vx_int32  dilation = 0;
        vx_int32  stride = 1;
        vx_enum downScaleSizeRoundingValue = VX_NN_DS_SIZE_ROUNDING_FLOOR;
        vx_scalar padXLeft = vxCreateScalar(context, VX_TYPE_INT32, &pad);
        vx_scalar padXRight = vxCreateScalar(context, VX_TYPE_INT32, &pad);
        vx_scalar padYTop = vxCreateScalar(context, VX_TYPE_INT32, &pad);
        vx_scalar padYBottom = vxCreateScalar(context, VX_TYPE_INT32, &pad);
        vx_scalar strideX = vxCreateScalar(context, VX_TYPE_INT32, &stride);
        vx_scalar strideY = vxCreateScalar(context, VX_TYPE_INT32, &stride);
        vx_scalar dilationX = vxCreateScalar(context, VX_TYPE_INT32, &dilation);
        vx_scalar dilationY = vxCreateScalar(context, VX_TYPE_INT32, &dilation);
        vx_scalar downScaleSizeRounding = vxCreateScalar(context, VX_TYPE_INT32, &downScaleSizeRoundingValue);
        vxnneOperation_Initialize(&operation_sw->base,
            layer,
            VXNNE_OPERATION_TARGET_SW,
            VXNNE_OPERATOR_CONVOLUTION,
            vxnneExecuteSWConvolution,
            VX_NULL,
            batch_count,
            0);

        vxnneLayer_SetOperation(
            layer,
            &operation_sw->base,
            (*op_index)++);

        operation_sw->inputs = fc_inputs;
        operation_sw->weights = WB_WEIGHT_TENSOR(weights_biases);
        operation_sw->biases = WB_BIAS_TENSOR(weights_biases);
        operation_sw->padX = padXLeft;
        operation_sw->padXRight = padXRight;
        operation_sw->padY = padYTop;
        operation_sw->padYBottom = padYBottom;
        operation_sw->strideX = strideX;
        operation_sw->strideY = strideY;
        operation_sw->dilationX = dilationX;
        operation_sw->dilationY = dilationY;
        operation_sw->downScaleSizeRounding = downScaleSizeRounding;
        operation_sw->outputs = fc_outputs;

        vxnneOperation_AddReference(&operation_sw->base, (vx_reference)fc_inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&operation_sw->base, (vx_reference)WB_WEIGHT_TENSOR(weights_biases), VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&operation_sw->base, (vx_reference)WB_BIAS_TENSOR(weights_biases), VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&operation_sw->base, (vx_reference)fc_outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }
    else
    {

        vxmONERROR(vxnneOperation_Initialize(&operation->base,
                                             layer,
                                             VXNNE_OPERATION_TARGET_NN,
                                             VXNNE_OPERATOR_FULLYCONNECTED,
                                             VX_NULL,
                                             VX_NULL,
                                             batch_count,
                                             NNE_COMMAND_SIZE * WB_TOTAL_SLICE_NUM(weights_biases)));

        op_param = &operation->base.parameter;

        op_param->pad_x_left = 0;
        op_param->pad_y_top = 0;
        op_param->pad_x_right = 0;
        op_param->pad_y_bottom = 0;
        op_param->pad_mode = VX_PAD_CONSTANT;
        op_param->pad_const = 0;
        op_param->pool_type = VIV_NN_POOLING_NON;
        op_param->pool_size_x = 0;
        op_param->pool_size_y = 0;
        op_param->pool_stride = 1;
        op_param->enable_relu = enable_relu;
        op_param->conv_rounding_type = rounding_policy;

        /*
         * Dimension layouts.
         *      weights_biases:     [1 x 1 x #IFM x #OFM]
         *      inputs (reshaped):  [1 x 1 x #IFM x #batch]
         *      outputs (reshaped): [1 x 1 x #OFM x #batch]
         */
        vxmONERROR(_GetBatchAndInOutCountOfFC(inputs,
                                              outputs,
                                              &batch_count,
                                              &input_unit_count,
                                              &output_unit_count));


        operation->inputs = fc_inputs;
        operation->outputs = fc_outputs;
        operation->weights_biases = weights_biases;
        operation->pad_x_left = 0;
        operation->pad_x_right = 0;
        operation->pad_y_top = 0;
        operation->pad_y_bottom = 0;
        operation->down_scale_size_rounding = rounding_policy;
        operation->enable_relu = enable_relu;

        vxmONERROR(vxnneLayer_SetOperation(layer,
                                           &operation->base,
                                           (*op_index)++));

        vxnneOperation_AddReference(&operation->base, (vx_reference)fc_inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&operation->base, (vx_reference)fc_outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }

OnError:
    return status;
}

vx_status vxoFCOperationSH_Initialize(
    vxnne_shader_operation operation,
    vxnne_layer layer,
    vx_tensor inputs,
    vx_uint32 batch_count,
    vx_tensor weights,
    vx_tensor biases,
    vx_enum overflow_policy,
    vx_enum rounding_policy,
    vx_bool enable_relu,
    vx_tensor outputs,
    vx_uint32_ptr op_index
    )
{
    vx_node node = layer->node;
    vxnne_shader_executable shaderExecutable;
    enum vx_nn_activation_function_e activation = VX_NN_ACTIVATION_NONE;
    vx_status status                            = VX_SUCCESS;

    if (enable_relu)
    {
        activation = VX_NN_ACTIVATION_RELU;
    }

    if (TENSOR_DIM_NUM(inputs) < 3) batch_count = 1;

    if (node->base.context->evisNoInst.supportEVIS)
    {
        shaderExecutable = vxnneGetFullyConnectedShaderExecutable(node->base.context, VXNNE_KERNEL_FULLYCONNECTED,
                                    &node->kernelAttributes.borderMode, inputs, weights, biases, activation, overflow_policy, VX_NULL, outputs);
    }
    else
    {
        shaderExecutable = vxnneGetGPUFullyConnectedShaderExecutable(node->base.context, VXNNE_KERNEL_GPU_FULLYCONNECTED,
                                    &node->kernelAttributes.borderMode, 0, inputs, weights, biases, activation, VX_NULL, outputs);
    }
    if (!shaderExecutable)
    {
        vxmONERROR(VX_FAILURE);
    }

    batch_count = 1;

    vxmONERROR(vxnneShaderOperation_Initialize(operation,
        layer,
        VXNNE_OPERATOR_FULLYCONNECTED,
        batch_count,
        shaderExecutable));

    vxmONERROR(vxnneLayer_SetOperation(
        layer,
        &operation->base,
        (*op_index)++));

    vxnneOperation_AddReference(&operation->base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
    vxnneOperation_AddReference(&operation->base, (vx_reference)weights, VXNNE_OPERATION_REFENRENCE_INPUT);
    vxnneOperation_AddReference(&operation->base, (vx_reference)biases, VXNNE_OPERATION_REFENRENCE_INPUT);
    vxnneOperation_AddReference(&operation->base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

OnError:
    return status;
}

vx_status vxoFCOperationSW_Initialize(
    vxnne_fully_connected_sw_operation operation,
    vxnne_layer layer,
    vx_tensor inputs,
    vx_uint32 batch_count,
    vx_tensor weights,
    vx_tensor biases,
    vx_enum overflow_policy,
    vx_enum rounding_policy,
    vx_bool enable_relu,
    vx_tensor outputs,
    vx_uint32_ptr op_index
    )
{
    vx_status status = VX_SUCCESS;

    if (!op_index)
    {
        vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
    }

    batch_count = 1;

    vxmONERROR(vxnneOperation_Initialize(&operation->base,
                                         layer,
                                         VXNNE_OPERATION_TARGET_SW,
                                         VXNNE_OPERATOR_FULLYCONNECTED,
                                         _FCOperationSW,
                                         VX_NULL,
                                         batch_count,
                                         0));

    operation->inputs  = inputs;
    operation->weights = weights;
    operation->biases  = biases;
    operation->outputs = outputs;

    vxmONERROR(vxnneLayer_SetOperation(layer,
                                       &operation->base,
                                       (*op_index)++));

    vxnneOperation_AddReference(&operation->base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
    vxnneOperation_AddReference(&operation->base, (vx_reference)weights, VXNNE_OPERATION_REFENRENCE_INPUT);
    vxnneOperation_AddReference(&operation->base, (vx_reference)biases, VXNNE_OPERATION_REFENRENCE_INPUT);
    vxnneOperation_AddReference(&operation->base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

OnError:
    return status;
}

