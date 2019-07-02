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


#include <ops/gc_vx_op_fc.h>
#include <ops/gc_vx_op_tensor_copy.h>
#include <gc_vx_nn_util.h>

extern vx_tensor _createTensor(vx_graph graph, vx_bool is_virtual,
    vx_uint32 num_of_dims, vx_uint32 * sizes, vx_enum data_format, vx_enum quant_format,
    vx_int8 fixed_point_pos,
    vx_float32 scale, vx_int32 zeroPoint);
extern vx_tensor _createSimilarTensor(vx_graph graph, vx_bool is_virtual, vx_uint32 num_of_dims, vx_uint32 * sizes, vx_tensor tensor);

extern vx_status vxnneExecuteSWConvolution(vxnne_operation operation);

extern vx_status vxnneExecuteSWTensorTranspose(struct _vxnne_operation_s *operation);

extern vx_status vxnneGetAlignStrides(vx_tensor tensor, vx_uint32_ptr strides, vx_bool align64);
extern vx_status vxnneOperation_TP_Deinitialize(vxnne_operation_s *operation);
extern vx_status vxnneOperation_AddReference(
    vxnne_operation_s*            operation,
    vx_reference                  reference,
    vxnne_operation_reference_e   refType
    );
extern vx_tensor
vxoTensor_CreateTensorExt(
    vx_context context,
    vx_graph graph,
    const vx_tensor_create_params_t* tensor_create_params,
    vx_uint32 * strides,
    vx_view_region_s * viewRegion,
    vx_tensor_buffer_s * tensorBuffer,
    vx_bool is_virtual,
    vx_reference_kind_e kind
);

vx_status vxoFC_NN_Trans_Initialize(
    vxnne_tp_operation operation_tp,
    vxnne_shader_operation operation_sh,
    vxnne_tensor_trans_operation operation_sw,
    vxnne_layer layer,
    vx_tensor inputs,
    vx_uint32 batch_count,
    vx_border_t* border_mode,
    vx_tensor outputs,
    vx_uint32_ptr op_index
    );

#define IMG_MAX_WIDTH 65536

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

    supportDataFormat0 = (vx_bool)(input_dataformat == VX_TYPE_FLOAT16 && weight_dataformat == VX_TYPE_FLOAT16 && bias_dataformat == VX_TYPE_FLOAT32 && output_dataformat == VX_TYPE_FLOAT16);
    supportDataFormat1 = (vx_bool)(input_dataformat == VX_TYPE_INT8 && weight_dataformat == VX_TYPE_INT8 && bias_dataformat == VX_TYPE_INT32 && output_dataformat == VX_TYPE_INT8);
    supportDataFormat2 = (vx_bool)(input_dataformat == VX_TYPE_INT16 && weight_dataformat == VX_TYPE_INT16 && bias_dataformat == VX_TYPE_INT32 && output_dataformat == VX_TYPE_INT16);
    supportDataFormat3 = (vx_bool)(input_dataformat == VX_TYPE_UINT8 && weight_dataformat == VX_TYPE_UINT8 && bias_dataformat == VX_TYPE_INT32 && output_dataformat != VX_TYPE_FLOAT32);
    enable_shader = (supportDataFormat0 || supportDataFormat1 || supportDataFormat2 || supportDataFormat3) && (inputDims < IMG_MAX_WIDTH);

    /* Choose the accelerator. */
    if (vxnneIsNNSupportFormat(context, inputs, VX_NULL, outputs) && batch_count > 1 && support_1xn)
    {
        target = VXNNE_OPERATION_TARGET_NN;
    }
    else if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_SINGLE_FC) &&
        vxnneIsTPSupportFormat(context, inputs, VX_NULL, outputs))
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

            aligned_tensor = vxoTensor_CreateTensorExt(context, node->graph, &params, strides, VX_NULL, VX_NULL, vx_true_e, VX_REF_INTERNAL);
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

                weights = wb->wb_base->origWeight;
                biases = wb->wb_base->origBias;
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

                wb = _createWeightsBiasesParameterFromTensors(context,
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
                                                              TENSOR_DATA_TYPE(outputs),
                                                              0,
                                                              VX_TENSOR_RANK_WHCN,
                                                              weights,
                                                              biases,
                                                              VX_NULL,
                                                              vx_false_e,
                                                              vx_false_e);

                if (!wb)
                {
                    vxmONERROR(VX_ERROR_NO_RESOURCES);
                }

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
        break;

    case VXNNE_OPERATION_TARGET_NN:
    {
        vx_uint32 trans_sizes[][4] = {
            {TENSOR_SIZE_INDEX(inputs, 0)/2, 2, TENSOR_SIZE_INDEX(inputs, 1), 1}, /* reshape: 1312x378 => 656x2x378x1  */
            {TENSOR_SIZE_INDEX(inputs, 1), 2, TENSOR_SIZE_INDEX(inputs, 0)/2, 1}, /* input  : 1312x378 => 378x2x656x1  */
            {TENSOR_SIZE_INDEX(outputs, 1), 1, TENSOR_SIZE_INDEX(outputs, 0), 1}, /* output : 4096x378 <= 378x1x4096x1 */
            {TENSOR_SIZE_INDEX(outputs, 0), 1, TENSOR_SIZE_INDEX(outputs, 1), 1}, /* output : 378x1x4096x1 => 4096x1x378x1 */
        };

        vx_tensor input_reshape = vxoTensor_ReshapeTensor(inputs, (vx_int32_ptr)trans_sizes[0], gcmCOUNTOF(trans_sizes[0]));
        vx_tensor input_trans   = _createSimilarTensor(layer->node->graph, vx_false_e, gcmCOUNTOF(trans_sizes[1]), trans_sizes[1], inputs);
        vx_tensor output_trans  = _createSimilarTensor(layer->node->graph, vx_false_e, gcmCOUNTOF(trans_sizes[2]), trans_sizes[2], outputs);
        vx_tensor outputs_reshape = vxoTensor_ReshapeTensor(outputs, (vx_int32_ptr)trans_sizes[3], gcmCOUNTOF(trans_sizes[3]));

        vxmONERROR(vxoTensor_AllocateMemory(input_trans));
        vxmONERROR(vxoTensor_AllocateMemory(output_trans));

        layer->temp_tensors[layer->num_temp_tensors++] = input_reshape;
        layer->temp_tensors[layer->num_temp_tensors++] = outputs_reshape;
        layer->temp_tensors[layer->num_temp_tensors++] = input_trans;
        layer->temp_tensors[layer->num_temp_tensors++] = output_trans;

        if (wb == VX_NULL)
        {

            vx_uint32 pnum = 4;
            vx_int32 zdp_size = 2;
            vx_uint32 perm[] = {1, 0, 2, 3}; /* transpose weights dim[0] and dim[1] */
            vx_int32 weight_sizes[][4] = {
                {gcmALIGN_NP2(TENSOR_SIZE_INDEX(weights, 0), zdp_size)/zdp_size, zdp_size, TENSOR_SIZE_INDEX(weights, 1), 1}, /* reshape: 90x400 => 45x2x400x1*/
                {zdp_size, gcmALIGN_NP2(TENSOR_SIZE_INDEX(weights, 0), zdp_size)/zdp_size, TENSOR_SIZE_INDEX(weights, 1), 1}, /* trans  : 45x2x400x1 => 2x45x400x1*/
                {1, zdp_size, gcmALIGN_NP2(TENSOR_SIZE_INDEX(weights, 0), zdp_size)/zdp_size, TENSOR_SIZE_INDEX(weights, 1)}, /* reshape: 2x45x400x1 => 1x2x45x400*/
            };

            vx_tensor _weights_pre      = vxoTensor_ReshapeTensor(weights, weight_sizes[0], gcmCOUNTOF(weight_sizes[0]));
            vx_tensor _weights_trans    = _createSimilarTensor(layer->node->graph, vx_false_e, gcmCOUNTOF(weight_sizes[1]), (vx_uint32_ptr)weight_sizes[1], weights);
            vx_tensor _weights          = vxoTensor_ReshapeTensor(_weights_trans, weight_sizes[2], gcmCOUNTOF(weight_sizes[2]));

            layer->temp_tensors[layer->num_temp_tensors++] = _weights_pre;
            layer->temp_tensors[layer->num_temp_tensors++] = _weights;
            layer->temp_tensors[layer->num_temp_tensors++] = _weights_trans;

            vxmONERROR(vxoTensor_AllocateMemory(_weights_trans));

            vxmONERROR(vxoFCOperation_TransposeTensor(_weights_pre, _weights_trans, perm, pnum));

            if (weights_biases && *weights_biases)
            {
                wb = *weights_biases;

                weights = wb->wb_base->origWeight;
                biases = wb->wb_base->origBias;
            }
            else if (!weights_biases || !*weights_biases)
            {
                if (TENSOR_QUANT_TYPE(inputs) == VX_QUANT_AFFINE_SCALE)
                {
                    vxZeroMemory(&wb_opt, gcmSIZEOF(wb_opt));
                    wb_opt.inputZeroPoint = TENSOR_TF_ZEROPOINT(input_trans);
                    wb_opt.zrl = -1;
                    wb_opt.outputFormat = TENSOR_DATA_TYPE(output_trans);

                    opt = &wb_opt;
                }

                wb = _createWeightsBiasesParameterFromTensors(context,
                                                              VX_NN_CONVOLUTION_LAYER,
                                                              input_trans->dims, /* inputs_dims */
                                                              input_trans->dimCount,
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
                                                              TENSOR_DATA_TYPE(output_trans),
                                                              0,
                                                              VX_TENSOR_RANK_WHCN,
                                                              _weights,
                                                              biases,
                                                              VX_NULL,
                                                              vx_false_e,
                                                              vx_false_e);

                if (!wb)
                {
                    vxmONERROR(VX_ERROR_NO_RESOURCES);
                }

                operation->weights_biases = wb;

                if (weights_biases)
                {
                    *weights_biases = wb;
                }
            }
        }

        if (wb)
        {
            vxmONERROR(vxoFC_NN_Trans_Initialize(&operation->nn_trans_tp_operation[0],
                                                    &operation->nn_trans_sh_operation[0],
                                                    &operation->nn_trans_sw_operation[0],
                                                    layer,
                                                    input_reshape,
                                                    1/*batch_count*/,
                                                    &node->kernelAttributes.borderMode,
                                                    input_trans,
                                                    op_index));
            vxmONERROR(vxoFCOperationNN_Initialize(&operation->nn_operation,
                                                    &operation->nn_operation_sw,
                                                    layer,
                                                    input_trans,
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
                                                    output_trans,
                                                    1/*batch_count*/,
                                                    &node->kernelAttributes.borderMode,
                                                    outputs_reshape,
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

    vx_uint32 kzgroup = weights_biases->slice_kz_num;
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

    tp_value = (vx_tp_value_cmd)vxAllocateAndZeroMemory(gcmSIZEOF(vx_tp_value_cmd_s) * weights_biases->slice_num);
    vxmONERROR_NULLPTR(tp_value);

    for (i = 0; i < weights_biases->slice_num; i++)
    {
        tp_value[i].e32[0] = 0; /* 0: FC, 1: Sum filter. */
        tp_value[i].u32[0] = kzgroup;
        tp_value[i].u32[1] = zoffset;
        tp_value[i].u32[2] = kzoffset;
        tp_value[i].u32[3] = kzoffset2;

        /* Advance to the next. */
        if ((i + 1) % kzgroup)
        {
            kzoffset += weights_biases->slice_array[i].kz_count;
            kzoffset2 += weights_biases->weights_sizes[3];
        }
        else
        {
            kzoffset = kzoffset2 = 0;
            zoffset += weights_biases->slice_array[i].z_count;
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
        tp_value->u32[1] = weights_biases->weights_sizes[3];

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
    ptr_base[0] = 2;
    ptr_base[1] = 1;
    ptr_base[2] = 0;
    ptr_base[3] = 3;

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

        if (status != VX_SUCCESS)
            goto OnError;

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
        operation_sw->weights = weights_biases->wb_base->origWeight;
        operation_sw->biases = weights_biases->wb_base->origBias;
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
        vxnneOperation_AddReference(&operation_sw->base, (vx_reference)weights_biases->wb_base->origWeight, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&operation_sw->base, (vx_reference)weights_biases->wb_base->origBias, VXNNE_OPERATION_REFENRENCE_INPUT);
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
                                             NNE_COMMAND_SIZE * weights_biases->slice_num));

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
    enum vx_nn_activation_function_e activation = VX_NN_ACTIVATION_NONE;
    vxnne_shader_executable shaderExecutable;

    vx_status status = VX_SUCCESS;

    if (enable_relu)
    {
        activation = VX_NN_ACTIVATION_RELU;
    }

    if (TENSOR_DIM_NUM(inputs) < 3) batch_count = 1;

    shaderExecutable = vxnneGetFullyConnectedShaderExecutable(node->base.context, VXNNE_KERNEL_FULLYCONNECTED, &node->kernelAttributes.borderMode, inputs, weights, biases, activation, outputs);
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

