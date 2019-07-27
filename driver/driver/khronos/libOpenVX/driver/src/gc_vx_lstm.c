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


#include <gc_vx_lstm.h>
#include <ops/gc_vx_ops.h>
#include <layers/gc_vx_layer_lstm.h>

#define NEW_LSTM_LAYER_PATH

extern vx_status vxnneExecuteSWFullyConnected(struct _vxnne_operation_s *operation);
extern vx_status vxnneOperation_TP_Deinitialize(vxnne_operation_s *operation);
extern vx_status vxnneExecuteSWTensorCopy(struct _vxnne_operation_s *operation);
extern vx_status vxnneExecuteSWConvolution(vxnne_operation operation);
extern vx_status vxoNNFullyConnectedLayerInitializer(
    vx_node node,
    vxnne_layer layer,
    vxnne_tp_operation tp_operation0,
    vxnne_tp_operation tp_operation1,
    vxnne_convolution_relu_pooling_operation nn_operation,
    vxnne_shader_operation sh_operation,
    vx_tensor inputs,
    vx_weights_biases_parameter weights_biases,
    vx_uint32 pad,
    vx_enum conv_rounding_type,
    vx_bool enable_relu,
    vx_int32_ptr count,
    vx_tensor outputs);

vx_tensor _createTensor(vx_graph graph, vx_bool is_virtual,
    vx_uint32 num_of_dims, vx_uint32 * sizes, vx_enum data_format, vx_enum quant_format,
    vx_int8 fixed_point_pos,
    vx_float32 scale, vx_int32 zeroPoint)
{
    vx_tensor_create_params_t params = { num_of_dims, sizes, data_format, quant_format};
    vx_tensor tensor = VX_NULL;

    if (quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
    {
        params.quant_data.dfp.fixed_point_pos = fixed_point_pos;
    }
    else
    {
        params.quant_data.affine.scale = scale;
        params.quant_data.affine.zeroPoint = zeroPoint;
    }

    if (is_virtual)
        tensor = vxoTensor_CreateVirtualTensor2(graph, &params, sizeof(vx_tensor_create_params_t));
    else
        tensor = vxoTensor_CreateTensor2(vxGetContext((vx_reference)graph), &params, sizeof(vx_tensor_create_params_t));

    return tensor;
}

vx_tensor _createSimilarTensor(vx_graph graph, vx_bool is_virtual, vx_uint32 num_of_dims, vx_uint32 * sizes, vx_tensor tensor)
{
    return _createTensor(graph, is_virtual, num_of_dims, sizes,
        TENSOR_DATA_TYPE(tensor), TENSOR_QUANT_TYPE(tensor),
        TENSOR_POS(tensor),
        TENSOR_TF_SCALE(tensor), TENSOR_TF_ZEROPOINT(tensor));
}


/***************************************************************************************************************************
*                                                 RNN
***************************************************************************************************************************/

vx_status vxnneExecuteSWRNN(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_rnn_operation rnnOperation = (vxnne_rnn_operation)operation;

    vx_tensor  inputs = rnnOperation->inputs;
    vx_tensor  weights = rnnOperation->weights;
    vx_tensor  recurrent_weights = rnnOperation->recurrent_weights;
    vx_tensor  biases = rnnOperation->bias;
    vx_tensor  state_ins = rnnOperation->state_in;
    vx_tensor  activations = rnnOperation->activation;
    vx_tensor  state_outs = rnnOperation->state_out;
    vx_tensor  outputs = rnnOperation->outputs;

    vx_int32 batch_size = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
    vx_int32 input_size = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
    vx_int32 num_unit = TENSOR_VIEW_SIZE_INDEX(weights, 1);
    vx_int32 b = 0, i = 0, j = 0;

    vx_float32 in = 0, w = 0;

    vx_enum acts[] = {
        VX_NN_ACTIVATION_NONE, /* kNone */
        VX_NN_ACTIVATION_RELU, /* kRelu */
        VX_NN_ACTIVATION_RELU1, /* kRelu1 */
        VX_NN_ACTIVATION_RELU6, /* kRelu6 */
    };


    vx_enum activation = (activations != VX_NULL) ? acts[*(vx_int32_ptr)TENSOR_LOGICAL_ADDR(activations)] : VX_NN_ACTIVATION_NONE;

    vx_float32_ptr buffer = (vx_float32_ptr)vxAllocateAndZeroMemory(sizeof(vx_float32) * num_unit);

    for (b = 0; b < batch_size; b++)
    {
        memset(buffer, 0, sizeof(vx_float32) * num_unit);

        /* output = biases */
        for (i = 0; i < num_unit; i++)
        {
            buffer[i] = VX_GET_DATA_FROM_TENSOR(biases, i);
        }


        /* output += weights * inputs */
        for (i = 0; i < num_unit; i++)
        {

            for (j = 0; j < input_size; j++)
            {
                in = VX_GET_DATA_FROM_TENSOR(inputs, j + b * input_size);
                w = VX_GET_DATA_FROM_TENSOR(weights, i * input_size + j);

                buffer[i] += in * w;

            }

        }


        /* output += recurrent_weights * state_in */
        for (i = 0; i < num_unit; i++)
        {
            for (j = 0; j < num_unit; j++)
            {
                in = VX_GET_DATA_FROM_TENSOR(state_ins, j + b * num_unit);
                w = VX_GET_DATA_FROM_TENSOR(recurrent_weights, i * num_unit + j);

                buffer[i] += in * w;
            }
        }


        for (i = 0; i < num_unit; i++)
        {
            if (activation != VX_NN_ACTIVATION_NONE)
                buffer[i] = vxnneActivation(activation, 0, 0, buffer[i]);

            VX_SAVE_DATA_TO_TENSOR(outputs, buffer[i], i + b * num_unit);
            VX_SAVE_DATA_TO_TENSOR(state_outs, buffer[i], i + b * num_unit);
        }
    }

    vxFree(buffer);

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNRNNLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoRNNLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxoRNNLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status vxoRNNLayer_ReshuffleInput(vx_tensor input, vx_tensor state_in, vx_tensor new_input)
{
    vx_status status = VX_FAILURE;

    if (vxoTensor_AllocateMemory(new_input) == VX_SUCCESS)
    {
        vx_int32 i = 0, j = 0, batch = TENSOR_SIZE_INDEX(input, 1), input_size = TENSOR_SIZE_INDEX(input, 0), output_size = TENSOR_SIZE_INDEX(state_in, 0);
        vx_uint8_ptr new_input_base = TENSOR_LOGICAL_ADDR(new_input);
        vx_uint8_ptr input_base = TENSOR_LOGICAL_ADDR(input);
        vx_uint8_ptr state_in_base = TENSOR_LOGICAL_ADDR(state_in);
        vx_float32 data = 0.f;

        for (j = 0; j < batch; j++)
        {
            for (i = 0; i < input_size + output_size; )
            {
                if ((TENSOR_DATA_TYPE(new_input) == TENSOR_DATA_TYPE(input)) && (TENSOR_DATA_TYPE(new_input) == TENSOR_DATA_TYPE(state_in)))
                {
                    vx_int32 item = TENSOR_DATA_SIZE(state_in);
                    if (i < input_size)
                    {
                        memcpy(new_input_base + j * (input_size + output_size) * item, input_base + j * input_size * item, input_size * item);
                        i += input_size;
                    }
                    else
                    {
                        memcpy(new_input_base + (j * (input_size + output_size) + input_size) * item, state_in_base + j * output_size * item, output_size * item);
                        i += output_size;
                    }
                }
                else
                {
                    if (i < input_size)
                    {
                        data = VX_GET_DATA_FROM_TENSOR(input, j * input_size + i);
                        VX_SAVE_DATA_TO_TENSOR(new_input, data, j * (input_size + output_size) + i);
                    }
                    else
                    {
                        data = VX_GET_DATA_FROM_TENSOR(input, j * output_size + (i - input_size));
                        VX_SAVE_DATA_TO_TENSOR(new_input, data, j * (input_size + output_size) + i);
                    }
                    i++;
                }
            }
        }

        status = VX_SUCCESS;
    }

    return status;
}


vx_status vxnneExecuteSWRNNReshuffleInput(struct _vxnne_operation_s *operation)
{

    vxnne_rnn_input_reshuffle_operation rnnReshuffleOperation = (vxnne_rnn_input_reshuffle_operation)operation;

    vx_tensor  input0 = rnnReshuffleOperation->input0;
    vx_tensor  input1 = rnnReshuffleOperation->input1;
    vx_tensor  new_input = rnnReshuffleOperation->new_input;

    return vxoRNNLayer_ReshuffleInput(input0, input1, new_input);
}

VX_PRIVATE_API vx_status vxoRNNLayer_ReshuffleWeightBias(vx_tensor weight, vx_tensor recurrent_weight, vx_tensor new_weight, vx_tensor bias, vx_tensor new_bias)
{
    vx_status status = VX_FAILURE;

    if (new_weight && vxoTensor_AllocateMemory(new_weight) == VX_SUCCESS)
    {
        vx_int32 i = 0, j = 0, input_size = TENSOR_SIZE_INDEX(weight, 0), unit_num = TENSOR_SIZE_INDEX(weight, 1), output_size = TENSOR_SIZE_INDEX(recurrent_weight, 0);
        vx_uint8_ptr new_weight_base = TENSOR_LOGICAL_ADDR(new_weight);
        vx_uint8_ptr weight_base = TENSOR_LOGICAL_ADDR(weight);
        vx_uint8_ptr recurrent_weight_base = TENSOR_LOGICAL_ADDR(recurrent_weight);
        vx_float32 data = 0.f;

        if (new_weight_base == VX_NULL)
            vxoTensor_AllocateMemory(new_weight);

        for (j = 0; j < unit_num; j++)
        {
            for (i = 0; i < input_size + output_size; )
            {
                if ((TENSOR_DATA_TYPE(new_weight) == TENSOR_DATA_TYPE(weight)) && (TENSOR_DATA_TYPE(new_weight) == TENSOR_DATA_TYPE(recurrent_weight)))
                {
                    vx_int32 item = TENSOR_DATA_SIZE(weight);
                    if (i < input_size)
                    {
                        memcpy(new_weight_base + j * (input_size + output_size) * item, weight_base + j * input_size * item, input_size * item);
                        i += input_size;
                    }
                    else
                    {
                        memcpy(new_weight_base + (j * (input_size + output_size) + input_size) * item, recurrent_weight_base + j * output_size * item, output_size * item);
                        i += output_size;
                    }
                }
                else
                {
                    if (i < input_size)
                    {
                        data = VX_GET_DATA_FROM_TENSOR(weight, j * input_size + i);
                        VX_SAVE_DATA_TO_TENSOR(new_weight, data, j * (input_size + output_size) + i);
                    }
                    else
                    {
                        data = VX_GET_DATA_FROM_TENSOR(recurrent_weight, j * output_size + (i - input_size));
                        VX_SAVE_DATA_TO_TENSOR(new_weight, data, j * (input_size + output_size) + i);
                    }
                    i++;
                }
            }
        }

        status = VX_SUCCESS;
    }


    if (new_bias && vxoTensor_AllocateMemory(new_bias) == VX_SUCCESS)
    {
        vx_int32 i = 0, size = TENSOR_SIZE_INDEX(bias, 0), new_size = TENSOR_SIZE_INDEX(new_bias, 0);
        vx_float32 data = 0.f;

        for (i = 0; i < new_size; )
        {
            if (i < size)
            {
                if (TENSOR_DATA_TYPE(new_bias) == TENSOR_DATA_TYPE(bias))
                {
                    memcpy(TENSOR_LOGICAL_ADDR(new_bias), TENSOR_LOGICAL_ADDR(bias), size * TENSOR_DATA_SIZE(bias));
                    i += size;
                }
                else
                {
                    data = VX_GET_DATA_FROM_TENSOR(bias, i);
                    VX_SAVE_DATA_TO_TENSOR(new_bias, data, i);
                    i++;
                }
            }
            else
                VX_SAVE_DATA_TO_TENSOR(new_bias, 0, i++);
        }

    }

    return status;
}

vx_status vxnneExecuteSWRNNReshuffleWeightsBiases(struct _vxnne_operation_s *operation)
{

    vxnne_rnn_input_reshuffle_operation rnnReshuffleOperation = (vxnne_rnn_input_reshuffle_operation)operation;

    vx_tensor  weight = rnnReshuffleOperation->input0;
    vx_tensor  recurrent_weight = rnnReshuffleOperation->input1;
    vx_tensor  new_weight = rnnReshuffleOperation->new_input;

    vx_tensor  bias = rnnReshuffleOperation->input2;
    vx_tensor  new_bias = rnnReshuffleOperation->new_input2;

    return vxoRNNLayer_ReshuffleWeightBias(weight, recurrent_weight, new_weight, bias, new_bias);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoRNNLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vx_tensor  inputs = (vx_tensor)parameters[0];
    vx_tensor  weights = (vx_tensor)parameters[1];
    vx_tensor  recurrent_weights = (vx_tensor)parameters[2];
    vx_tensor  bias = (vx_tensor)parameters[3];
    vx_tensor  state_in = (vx_tensor)parameters[4];
    vx_tensor  activation = (vx_tensor)parameters[5];
    vx_tensor  state_out = (vx_tensor)parameters[6];
    vx_tensor  outputs = (vx_tensor)parameters[7];
    vx_uint32  batchCount = TENSOR_SIZE_INDEX(inputs, 3);
    vxnne_rnn_layer  rnnLayer = VX_NULL;
    vx_enum    inputFormat = TENSOR_DATA_TYPE(inputs);
    vx_enum    outputFormat = TENSOR_DATA_TYPE(outputs);
    vx_enum    biasFormat = TENSOR_DATA_TYPE(bias);

    vx_bool    dataFormat_flag = vx_false_e;
    vx_bool    nne_support = vx_false_e;
    vx_context context = vxGetContext((vx_reference)node);

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }
    gcoOS_Allocate(gcvNULL, sizeof(vxnne_rnn_layer_s), (gctPOINTER*)&rnnLayer);
    if (!rnnLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("Out of Memory at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    dataFormat_flag = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16 && (biasFormat == VX_TYPE_FLOAT16 || biasFormat == VX_TYPE_FLOAT32))
        || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32 && biasFormat == VX_TYPE_FLOAT32)
        || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8 && biasFormat == VX_TYPE_UINT8));
    gcoOS_ZeroMemory(rnnLayer, sizeof(vxnne_rnn_layer_s));

    vxnneLayer_Initialize(&rnnLayer->base,
        "RNN",
        node,
        vxmOPERATION_COUNT(rnnLayer),
        rnnLayer->operations,
        VX_NULL);

    if (CHECK_LIFETIME_IS_STATIC(weights) &&
        CHECK_LIFETIME_IS_STATIC(recurrent_weights) &&
        vxnneIsNNSupportFormat(context, inputs, VX_NULL, outputs))
    {
        nne_support = vx_true_e;
    }

    if (nne_support)
    {
        vx_int32 opt_index = 0;
        vx_uint32 s = 0;
        vx_int32 batch = TENSOR_SIZE_INDEX(inputs, 1), input_size = TENSOR_SIZE_INDEX(inputs, 0), output_size = TENSOR_SIZE_INDEX(state_in, 0), unit_num = TENSOR_SIZE_INDEX(state_in, 0);
        vx_int32 sizes[][4] = {
            { 1, 1, unit_num + input_size, batch }, /* input   : 1 * 1 * ((input_size + output size) * batch        */
            { 1, 1, unit_num + input_size, unit_num }, /* weights : 1 * 1 * (unit_num + input_size)     * unit_num     */
            { unit_num + input_size, 1, 1, 1 }, /* biases  : (unit_num + input_size) * 1 * 1 * 1                */
            { 1, 1, output_size, batch }, /* outputs : 1 * 1 * output size                 * batch        */
        };
        vx_context context = vxGetContext((vx_reference)node);
        vx_tensor_create_params_t tensor_create_params;
        vx_tensor stages[4];
        vx_int32 zero = 0;
        vx_enum rounding = VX_NN_DS_SIZE_ROUNDING_FLOOR;
        vx_bool nne_conv = vx_true_e;

        vx_enum acts[] = {
            VX_NN_ACTIVATION_NONE, /* kNone */
            VX_NN_ACTIVATION_RELU, /* kRelu */
            VX_NN_ACTIVATION_RELU1, /* kRelu1 */
            VX_NN_ACTIVATION_RELU6, /* kRelu6 */
        };

        for (s = 0; s < (vx_uint32)sizeof(sizes) / sizeof(sizes[0]) - 1; s++)
        {
            /* Bias still create float32 tensor */

            gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
            tensor_create_params.num_of_dims = sizeof(sizes[s]) / sizeof(sizes[s][0]);
            tensor_create_params.sizes = (vx_uint32_ptr)sizes[s];
            tensor_create_params.data_format = (s == 2) ? VX_TYPE_FLOAT32 : VX_TYPE_FLOAT16;
            tensor_create_params.quant_format = 0;

            stages[s] = vxoTensor_CreateTensor(context, node->graph, &tensor_create_params, vx_false_e);

            status = vxoReference_GetStatus((vx_reference)stages[s]);


            status = vxoTensor_AllocateMemory(stages[s]);

            rnnLayer->base.temp_tensors[rnnLayer->base.num_temp_tensors++] = stages[s];
        }

        /* reshape output from (batch * output_size) to (1 * 1 * output_size * batch) */
        stages[3] = vxoTensor_ReshapeTensor(outputs, sizes[3], 4);
        rnnLayer->base.temp_tensors[rnnLayer->base.num_temp_tensors++] = stages[3];

        /* SW implement of weight and bias reshuffle */
        vxoRNNLayer_ReshuffleWeightBias(weights, recurrent_weights, stages[1], bias, stages[2]);

        if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP))
        {
            vx_op_param_s conv = { 0 };

            conv.pad_x_left = 0;
            conv.pad_y_top = 0;
            conv.pool_size_x = 0;
            conv.pool_size_y = 0;
            conv.pool_stride = 1;
            conv.enable_relu = vx_false_e;
            conv.pad_mode = VX_PAD_CONSTANT;
            conv.pad_const = 0;
            /* interleave input to new input tensor */
            vxnneOperation_Initialize(&rnnLayer->rnn_input_reshuffle_tp_operation.base,
                &rnnLayer->base,
                VXNNE_OPERATION_TARGET_TP,
                VXNNE_OPERATOR_INTERLEAVE,
                VX_NULL,
                vxnneOperation_TP_Deinitialize,
                1,
                0);

            conv.tpType = TP_RNN_INTERLEAVE;
            conv.other_ref = gcvNULL;
            conv.data_buff = gcvNULL;
            memcpy(&rnnLayer->rnn_input_reshuffle_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));

            vxnneLayer_SetOperation(
                &rnnLayer->base,
                &rnnLayer->rnn_input_reshuffle_tp_operation.base,
                opt_index++);

            rnnLayer->rnn_input_reshuffle_tp_operation.input = inputs;
            rnnLayer->rnn_input_reshuffle_tp_operation.output = stages[0];

            vxnneOperation_AddReference(&rnnLayer->rnn_input_reshuffle_tp_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rnnLayer->rnn_input_reshuffle_tp_operation.base, (vx_reference)stages[0], VXNNE_OPERATION_REFENRENCE_OUTPUT);

            /******************************************************************/
            /* interleave state in to new input tensor */
            vxnneOperation_Initialize(&rnnLayer->rnn_input_reshuffle_tp_operation2.base,
                &rnnLayer->base,
                VXNNE_OPERATION_TARGET_TP,
                VXNNE_OPERATOR_INTERLEAVE,
                VX_NULL,
                vxnneOperation_TP_Deinitialize,
                1,
                0);

            conv.tpType = TP_RNN_INTERLEAVE;
            conv.other_ref = gcvNULL;
            conv.data_buff = gcvNULL;

            /* output start address offset */
            conv.pad_x_left = TENSOR_SIZE_INDEX(inputs, 0) * TENSOR_DATA_SIZE(inputs);

            memcpy(&rnnLayer->rnn_input_reshuffle_tp_operation2.base.parameter, &conv, sizeof(vx_op_param_s));

            vxnneLayer_SetOperation(
                &rnnLayer->base,
                &rnnLayer->rnn_input_reshuffle_tp_operation2.base,
                opt_index++);

            rnnLayer->rnn_input_reshuffle_tp_operation2.input = state_in;
            rnnLayer->rnn_input_reshuffle_tp_operation2.output = stages[0];

            vxnneOperation_AddReference(&rnnLayer->rnn_input_reshuffle_tp_operation2.base, (vx_reference)state_in, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rnnLayer->rnn_input_reshuffle_tp_operation2.base, (vx_reference)stages[0], VXNNE_OPERATION_REFENRENCE_OUTPUT);

        }
        else
        {
            /* SW implement of input reshuffle */
            vxnneOperation_Initialize(&rnnLayer->rnn_input_reshuffle_operation.base,
                &rnnLayer->base,
                VXNNE_OPERATION_TARGET_SW,
                VXNNE_OPERATOR_INTERLEAVE,
                vxnneExecuteSWRNNReshuffleInput,
                VX_NULL,
                batchCount,
                0);

            vxnneLayer_SetOperation(
                &rnnLayer->base,
                &rnnLayer->rnn_input_reshuffle_operation.base,
                opt_index++);

            rnnLayer->rnn_input_reshuffle_operation.input0 = inputs;
            rnnLayer->rnn_input_reshuffle_operation.input1 = state_in;
            rnnLayer->rnn_input_reshuffle_operation.new_input = stages[0];

            vxnneOperation_AddReference(&rnnLayer->rnn_input_reshuffle_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rnnLayer->rnn_input_reshuffle_operation.base, (vx_reference)state_in, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rnnLayer->rnn_input_reshuffle_operation.base, (vx_reference)stages[0], VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }

        if (nne_conv)
        {
            vx_nn_convolution_relu_pooling_params_ext2_t params = {
                {
                    { 0, 0, 0, 0, 0, 0, 0,
                    VX_CONVERT_POLICY_WRAP, VX_ROUND_POLICY_TO_ZERO, VX_NN_DS_SIZE_ROUNDING_FLOOR,
                    vx_true_e, 0, 0, 0, VX_PAD_CONSTANT, 0 },
                    1, 1
                },
                0, VX_TENSOR_RANK_WHCN, VX_TYPE_FLOAT16
            };

            vx_weights_biases_parameter weights_biases = vxCreateWeightsBiasesParameterFromTensors2(
                VX_NN_CONVOLUTION_LAYER,
                4, (vx_uint32_ptr)sizes[0],
                (vx_uint32_ptr)sizes[3], (vx_uint32_ptr)sizes[3],
                VX_TYPE_FLOAT16,
                (vx_nn_convolution_relu_pooling_params)&params, sizeof(vx_nn_convolution_relu_pooling_params_ext2_t),
                NULL, stages[1], stages[2]);

            vx_int32 batchCount = TENSOR_SIZE_INDEX(stages[0], 3);

            vx_int32 activ = (activation != VX_NULL) ? *(vx_int32_ptr)TENSOR_LOGICAL_ADDR(activation) : -1;
            vx_bool enable_relu = (activ == 1) ? vx_true_e : vx_false_e;

            gcmASSERT(activ == 0 || activ == 1);

            /* Initialize covolution operation */
            status = vxnneOperation_Initialize(&rnnLayer->convolution_nn_convolution_operation0.base,
                &rnnLayer->base,
                VXNNE_OPERATION_TARGET_NN,
                VXNNE_OPERATOR_CONVOLUTION,
                VX_NULL,
                NULL,
                batchCount,
                NNE_COMMAND_SIZE);

            if (status != VX_SUCCESS)
                return status;

            vxnneLayer_SetOperation(
                &rnnLayer->base,
                &rnnLayer->convolution_nn_convolution_operation0.base,
                opt_index++);

            rnnLayer->convolution_nn_convolution_operation0.inputs = stages[0];
            rnnLayer->convolution_nn_convolution_operation0.weights_biases = weights_biases;
            rnnLayer->convolution_nn_convolution_operation0.enable_relu = enable_relu;
            rnnLayer->convolution_nn_convolution_operation0.outputs = stages[3];

            vxnneOperation_AddReference(&rnnLayer->convolution_nn_convolution_operation0.base, (vx_reference)stages[0], VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rnnLayer->convolution_nn_convolution_operation0.base, (vx_reference)stages[3], VXNNE_OPERATION_REFENRENCE_OUTPUT);

            {
                vx_op_param_s conv = { 0 };

                conv.pad_x_left = 0;
                conv.pad_x_right = 0;
                conv.pad_y_top = 0;
                conv.pad_y_bottom = 0;
                conv.pad_mode = VX_PAD_CONSTANT;
                conv.pad_const = 0;
                conv.pool_type = 0;
                conv.conv_rounding_type = VX_NN_DS_SIZE_ROUNDING_FLOOR;
                conv.enable_relu = enable_relu;
                conv.pool_size_x = 0;
                conv.pool_size_y = 0;

                memcpy(&rnnLayer->convolution_nn_convolution_operation0.base.parameter, &conv, sizeof(vx_op_param_s));
            }
        }
        else
        {

            vx_enum relu_e = (activation != VX_NULL) ? acts[*(vx_int32_ptr)TENSOR_LOGICAL_ADDR(activation)] : VX_NN_ACTIVATION_NONE;

            /* For SW convolution, must enable relue */
            vxnneOperation_Initialize(&rnnLayer->rnn_sw_operation2.base,
                &rnnLayer->base,
                VXNNE_OPERATION_TARGET_SW,
                VXNNE_OPERATOR_CONVOLUTION,
                vxnneExecuteSWConvolution,
                VX_NULL,
                batchCount,
                0);

            vxnneLayer_SetOperation(
                &rnnLayer->base,
                &rnnLayer->rnn_sw_operation2.base,
                opt_index++);

            rnnLayer->rnn_sw_operation2.inputs = stages[0];
            rnnLayer->rnn_sw_operation2.weights = stages[1];
            rnnLayer->rnn_sw_operation2.biases = stages[2];
            rnnLayer->rnn_sw_operation2.outputs = stages[3];

            rnnLayer->rnn_sw_operation2.padX = vxCreateScalar(context, VX_TYPE_INT32, &zero);
            rnnLayer->rnn_sw_operation2.padXRight = vxCreateScalar(context, VX_TYPE_INT32, &zero);
            rnnLayer->rnn_sw_operation2.padY = vxCreateScalar(context, VX_TYPE_INT32, &zero);
            rnnLayer->rnn_sw_operation2.padYBottom = vxCreateScalar(context, VX_TYPE_INT32, &zero);
            rnnLayer->rnn_sw_operation2.dilationX = vxCreateScalar(context, VX_TYPE_INT32, &zero);
            rnnLayer->rnn_sw_operation2.dilationY = vxCreateScalar(context, VX_TYPE_INT32, &zero);
            rnnLayer->rnn_sw_operation2.relu = vxCreateScalar(context, VX_TYPE_ENUM, &relu_e);
            rnnLayer->rnn_sw_operation2.downScaleSizeRounding = vxCreateScalar(context, VX_TYPE_ENUM, &rounding);

            vxnneOperation_AddReference(&rnnLayer->rnn_sw_operation2.base, (vx_reference)stages[0], VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rnnLayer->rnn_sw_operation2.base, (vx_reference)stages[1], VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rnnLayer->rnn_sw_operation2.base, (vx_reference)stages[2], VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rnnLayer->rnn_sw_operation2.base, (vx_reference)stages[3], VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }
        /* Tensor Copy output to state out */
        if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP))
        {
            vx_op_param_s conv = { 0 };

            conv.pad_x_left = 0;
            conv.pad_y_top = 0;
            conv.pool_size_x = 0;
            conv.pool_size_y = 0;
            conv.pool_stride = 1;
            conv.enable_relu = vx_false_e;
            conv.pad_mode = VX_PAD_CONSTANT;
            conv.pad_const = 0;

            vxnneOperation_Initialize(&rnnLayer->rnn_tensor_copy.base,
                &rnnLayer->base,
                VXNNE_OPERATION_TARGET_TP,
                VXNNE_OPERATOR_TENSOR_COPY,
                VX_NULL,
                vxnneOperation_TP_Deinitialize,
                batchCount,
                0);

            vxnneLayer_SetOperation(
                &rnnLayer->base,
                &rnnLayer->rnn_tensor_copy.base,
                opt_index++);

            rnnLayer->rnn_tensor_copy.input = outputs;
            rnnLayer->rnn_tensor_copy.output = state_out;

            vxnneOperation_AddReference(&rnnLayer->rnn_tensor_copy.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rnnLayer->rnn_tensor_copy.base, (vx_reference)state_out, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            conv.tpType = TP_TENSOR_COPY;
            conv.other_ref = gcvNULL;
            conv.data_buff = gcvNULL;

            memcpy(&rnnLayer->rnn_tensor_copy.base.parameter, &conv, sizeof(vx_op_param_s));
        }
        else
        {
            vxnneOperation_Initialize(&rnnLayer->rnn_sw_tensor_copy.base,
                &rnnLayer->base,
                VXNNE_OPERATION_TARGET_SW,
                VXNNE_OPERATOR_TENSOR_COPY,
                vxnneExecuteSWTensorCopy,
                VX_NULL,
                batchCount,
                0);

            vxnneLayer_SetOperation(
                &rnnLayer->base,
                &rnnLayer->rnn_sw_tensor_copy.base,
                opt_index++);

            rnnLayer->rnn_sw_tensor_copy.src = outputs;
            rnnLayer->rnn_sw_tensor_copy.dst = state_out;

            vxnneOperation_AddReference(&rnnLayer->rnn_sw_tensor_copy.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rnnLayer->rnn_sw_tensor_copy.base, (vx_reference)state_out, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }
    }
    else if (dataFormat_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
    {
        vxnne_shader_executable shaderExecutable;

        if (node->base.context->evisNoInst.supportEVIS)
        {
            shaderExecutable = vxnneGetRnnShaderExecutable(node->base.context, VXNNE_KERNEL_RNN, &node->kernelAttributes.borderMode,
                inputs, bias, weights, state_in, recurrent_weights, activation, state_out, outputs);
        }
        else
        {
            shaderExecutable = vxnneGetGPURnnShaderExecutable(node->base.context, VXNNE_KERNEL_RNN, &node->kernelAttributes.borderMode,
                inputs, bias, weights, state_in, recurrent_weights, activation, state_out, outputs);
        }

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&rnnLayer->rnn_sh_operation,
            &rnnLayer->base,
            VXNNE_OPERATOR_RNN,
            batchCount,
            shaderExecutable);
        if (status != VX_SUCCESS) goto exit;

        vxnneOperation_AddReference(&rnnLayer->rnn_sh_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&rnnLayer->rnn_sh_operation.base, (vx_reference)weights, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&rnnLayer->rnn_sh_operation.base, (vx_reference)recurrent_weights, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&rnnLayer->rnn_sh_operation.base, (vx_reference)bias, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&rnnLayer->rnn_sh_operation.base, (vx_reference)state_in, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&rnnLayer->rnn_sh_operation.base, (vx_reference)activation, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&rnnLayer->rnn_sh_operation.base, (vx_reference)state_out, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        vxnneOperation_AddReference(&rnnLayer->rnn_sh_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        vxnneLayer_SetOperation(
            &rnnLayer->base,
            &rnnLayer->rnn_sh_operation.base,
            0);
    }
    else
    {
        vxnneOperation_Initialize(&rnnLayer->rnn_sw_operation.base,
            &rnnLayer->base,
            VXNNE_OPERATION_TARGET_SW,
            VXNNE_OPERATOR_RNN,
            vxnneExecuteSWRNN,
            VX_NULL,
            batchCount,
            0);

        vxnneLayer_SetOperation(
            &rnnLayer->base,
            &rnnLayer->rnn_sw_operation.base,
            0);

        rnnLayer->rnn_sw_operation.inputs = inputs;
        rnnLayer->rnn_sw_operation.weights = weights;
        rnnLayer->rnn_sw_operation.recurrent_weights = recurrent_weights;
        rnnLayer->rnn_sw_operation.bias = bias;
        rnnLayer->rnn_sw_operation.state_in = state_in;
        rnnLayer->rnn_sw_operation.activation = activation;
        rnnLayer->rnn_sw_operation.state_out = state_out;
        rnnLayer->rnn_sw_operation.outputs = outputs;

        vxnneOperation_AddReference(&rnnLayer->rnn_sw_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&rnnLayer->rnn_sw_operation.base, (vx_reference)weights, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&rnnLayer->rnn_sw_operation.base, (vx_reference)recurrent_weights, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&rnnLayer->rnn_sw_operation.base, (vx_reference)bias, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&rnnLayer->rnn_sw_operation.base, (vx_reference)state_in, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&rnnLayer->rnn_sw_operation.base, (vx_reference)activation, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&rnnLayer->rnn_sw_operation.base, (vx_reference)state_out, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        vxnneOperation_AddReference(&rnnLayer->rnn_sw_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }
    node->layer = &rnnLayer->base;

    return status;

exit:
    if (rnnLayer) gcoOS_Free(gcvNULL, (gctPOINTER)rnnLayer);
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoRNNLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vx_uint32 i = 0;
        vxnne_rnn_layer layer = (vxnne_rnn_layer)node->layer;

        for (i = 0; i < layer->base.num_operations; i++)
        {
            if (layer->base.operations[i] != VX_NULL && layer->base.operations[i] == (vxnne_operation)&layer->convolution_nn_convolution_operation0)
            {
                vxnne_convolution_relu_pooling_operation operation = (vxnne_convolution_relu_pooling_operation)layer->base.operations[i];

                if (operation->inputs)vxoTensor_ReleaseTensor(&operation->inputs);
                if (operation->outputs)vxoTensor_ReleaseTensor(&operation->outputs);
            }
            else if (layer->base.operations[i] != VX_NULL && layer->base.operations[i] == (vxnne_operation)&layer->convolution_nn_convolution_operation1)
            {

            }
            else if (layer->base.operations[i] != VX_NULL && layer->base.operations[i] == (vxnne_operation)&layer->rnn_input_reshuffle_operation)
            {
                vxnne_rnn_input_reshuffle_operation operation = (vxnne_rnn_input_reshuffle_operation)layer->base.operations[i];

                if (operation->new_input)vxoTensor_ReleaseTensor(&operation->new_input);

            }
            else if (layer->base.operations[i] != VX_NULL && layer->base.operations[i] == (vxnne_operation)&layer->rnn_input_reshuffle_tp_operation)
            {
                vxnne_tp_operation operation = (vxnne_tp_operation)layer->base.operations[i];

                if (operation->output)
                    vxoTensor_ReleaseTensor(&operation->output);
            }
            else if (layer->base.operations[i] != VX_NULL && layer->base.operations[i] == (vxnne_operation)&layer->rnn_input_reshuffle_tp_operation2)
            {
                vxnne_tp_operation operation = (vxnne_tp_operation)layer->base.operations[i];

                if (operation->output)
                    vxoTensor_ReleaseTensor(&operation->output);
            }
            else if (layer->base.operations[i] != VX_NULL && layer->base.operations[i] == (vxnne_operation)&layer->rnn_sw_operation)
            {

            }
            else if (layer->base.operations[i] != VX_NULL && layer->base.operations[i] == (vxnne_operation)&layer->rnn_sw_operation2)
            {
                vxnne_convolution_operation operation = (vxnne_convolution_operation)layer->base.operations[i];

                if (operation->inputs)vxoTensor_ReleaseTensor(&operation->inputs);
                if (operation->outputs)vxoTensor_ReleaseTensor(&operation->outputs);
                if (operation->weights)vxoTensor_ReleaseTensor(&operation->weights);
                if (operation->biases)vxoTensor_ReleaseTensor(&operation->biases);

                if (operation->padX)vxReleaseScalar(&operation->padX);
                if (operation->padXRight)vxReleaseScalar(&operation->padXRight);
                if (operation->padY)vxReleaseScalar(&operation->padY);
                if (operation->padYBottom)vxReleaseScalar(&operation->padYBottom);
                if (operation->dilationX)vxReleaseScalar(&operation->dilationX);
                if (operation->dilationY)vxReleaseScalar(&operation->dilationY);
                if (operation->relu)vxReleaseScalar(&operation->relu);
                if (operation->downScaleSizeRounding)vxReleaseScalar(&operation->downScaleSizeRounding);

            }
            else if (layer->base.operations[i] != VX_NULL && layer->base.operations[i] == (vxnne_operation)&layer->rnn_sw_tensor_copy)
            {

            }
            else if (layer->base.operations[i] != VX_NULL && layer->base.operations[i] == (vxnne_operation)&layer->rnn_tensor_copy)
            {

            }
            else
            {
                vxError("Unkown operation!");
            }

        }

        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

/***************************************************************************************************************************
*                                                 SVDF
***************************************************************************************************************************/
vx_status vxnneExecuteSWSVDF(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_svdf_operation svdfOperation = (vxnne_svdf_operation)operation;

    vx_tensor  inputs = svdfOperation->inputs;
    vx_tensor  weights_feature = svdfOperation->weights_feature;
    vx_tensor  recurrent_weights = svdfOperation->recurrent_time;
    vx_tensor  biases = svdfOperation->bias;
    vx_tensor  state_ins = svdfOperation->state_in;
    vx_tensor  rank = svdfOperation->rank;
    vx_tensor  activations = svdfOperation->activation;
    vx_tensor  state_outs = svdfOperation->state_out;
    vx_tensor  outputs = svdfOperation->outputs;

    vx_int32 batch_size = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
    vx_int32 input_size = TENSOR_VIEW_SIZE_INDEX(inputs, 0);

    vx_int32 rank_size = (vx_int32)VX_GET_DATA_FROM_TENSOR(rank, 0);
    vx_int32 num_filter = TENSOR_VIEW_SIZE_INDEX(weights_feature, 1);
    vx_int32 num_unit;
    /*vx_int32 weights_feature_stride   = TENSOR_VIEW_SIZE_INDEX(weights_feature, 0);*/
    vx_int32 memory_size = TENSOR_VIEW_SIZE_INDEX(recurrent_weights, 0);
    vx_int32 recurrent_time_stride = memory_size;

    vx_int32 b = 0, i = 0, j = 0, r = 0;

    vx_float32 in = 0, w = 0;
    vx_float32 activation = 0.0;

    vx_enum acts[] = {
        VX_NN_ACTIVATION_NONE, /* kNone */
        VX_NN_ACTIVATION_RELU, /* kRelu */
        VX_NN_ACTIVATION_RELU1, /* kRelu1 */
        VX_NN_ACTIVATION_RELU6, /* kRelu6 */
    };

    vx_enum act = (activations != VX_NULL) ? acts[*(vx_int32_ptr)TENSOR_LOGICAL_ADDR(activations)] : VX_NN_ACTIVATION_NONE;

    vx_float32 data = .0f;

    vxmASSERT(rank_size != 0);
    num_unit = num_filter / rank_size;

    for (b = 0; b < batch_size; b++)
    {
        vx_int32 input_offset = b * input_size;
        vx_int32 output_offset = b * num_unit;
        vx_int32 state_offset = b * num_filter * memory_size;

        for (i = 0; i < num_unit; i++)
        {
            /* output = biases */
            data = VX_GET_DATA_FROM_TENSOR(biases, i);

            for (r = 0; r < rank_size; r++)
            {
                activation = .0f;

                /* output += weights_feature * inputs */
                for (j = 0; j < input_size; j++)
                {
                    in = VX_GET_DATA_FROM_TENSOR(inputs, j + input_offset);
                    w = VX_GET_DATA_FROM_TENSOR(weights_feature, (i * rank_size + r) * input_size + j);

                    activation += in * w;
                }

                data += VX_GET_DATA_FROM_TENSOR(recurrent_weights, (memory_size - 1) + (i * rank_size + r) * recurrent_time_stride) * activation;

                for (j = 0; j < memory_size - 1; j++)
                {
                    data += VX_GET_DATA_FROM_TENSOR(recurrent_weights, j + (i * rank_size + r) * recurrent_time_stride) * VX_GET_DATA_FROM_TENSOR(state_ins, j + (i * rank_size + r) * memory_size + state_offset);
                }

                /* right shift state */
                for (j = 0; j < memory_size - 1 - 1; j++) {
                    VX_SAVE_DATA_TO_TENSOR(state_outs, VX_GET_DATA_FROM_TENSOR(state_ins, j + 1 + (i * rank_size + r) * memory_size + state_offset), j + (i * rank_size + r) * memory_size + state_offset);
                }

                VX_SAVE_DATA_TO_TENSOR(state_outs, activation, (memory_size - 1 - 1) + (i * rank_size + r) * memory_size + state_offset);

                VX_SAVE_DATA_TO_TENSOR(state_outs, 0.0f, (memory_size - 1) + (i * rank_size + r) * memory_size + state_offset);
            }

            VX_SAVE_DATA_TO_TENSOR(outputs, (act != VX_NN_ACTIVATION_NONE) ? vxnneActivation(act, 0, 0, data) : data, i + output_offset);
        }
    }

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNSVDFLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoSVDFLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxoSVDFLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status vxnneExecuteSWSVDF_MAP(struct _vxnne_operation_s *operation)
{
    vxnne_svdf_operation svdfOperation = (vxnne_svdf_operation)operation;

    vx_tensor input = svdfOperation->inputs;
    vx_tensor output = svdfOperation->outputs;

    vx_int32 input_size = TENSOR_SIZE_INDEX(input, 2);
    vx_int32 batch = TENSOR_SIZE_INDEX(input, 3);

    vx_int32 output_w = TENSOR_SIZE_INDEX(output, 2);
    /*vx_int32 output_h = TENSOR_SIZE_INDEX(output, 3);*/
    vx_int32 i = 0, j = 0, stride = output_w / input_size;
    vx_float32 data = 0.f;

    for (i = 0; i < batch; i++)
    {
        for (j = 0; j < input_size; j++)
        {
            data = VX_GET_DATA_FROM_TENSOR(input, i * input_size + j);

            VX_SAVE_DATA_TO_TENSOR(output, data, i * output_w + j * stride + (stride - 1));
        }
    }


    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxnneExecuteSWSVDF_ROTATION(struct _vxnne_operation_s *operation)
{


    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxnneExecuteSVDF_DepthWeightsTime(vx_tensor input, vx_tensor output, vx_int32 rank)
{
    vx_uint8_ptr input_ptr = VX_NULL, output_ptr = VX_NULL;
    vx_int32 i = 0, j = 0;

    vx_int32 item_size = TENSOR_DATA_SIZE(input);
    vx_int32 input_w = TENSOR_SIZE_INDEX(input, 0);
    /*vx_int32 input_h = TENSOR_SIZE_INDEX(input, 1);*/

    vx_int32 output_w = 0;
    vx_int32 output_h = 0;
    vx_int32 stride = input_w * rank;
    vxoTensor_GetTensorViewMemory(input, (gctPOINTER*)&input_ptr, VX_NULL);
    vxoTensor_AllocateMemory(output);
    vxoTensor_GetTensorViewMemory(output, (gctPOINTER*)&output_ptr, VX_NULL);

    output_w = TENSOR_SIZE_INDEX(output, 2);
    output_h = TENSOR_SIZE_INDEX(output, 3);

    for (i = 0; i < output_h; i++)
    {
        for (j = 0; j < output_w; j += stride)
        {
            if (j / stride == i)
                memcpy(output_ptr + (j + i * output_w) * item_size, input_ptr + i * stride * item_size, stride * item_size);
            else
                memset(output_ptr + (j + i * output_w) * item_size, 0, stride * item_size);
        }
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoSVDFLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vx_tensor  inputs = (vx_tensor)parameters[0];
    vx_tensor  weights_feature = (vx_tensor)parameters[1];
    vx_tensor  recurrent_time = (vx_tensor)parameters[2];
    vx_tensor  bias = (vx_tensor)parameters[3];
    vx_tensor  state_in = (vx_tensor)parameters[4];
    vx_tensor  ranks = (vx_tensor)parameters[5];
    vx_tensor  activation = (vx_tensor)parameters[6];
    vx_tensor  state_out = (vx_tensor)parameters[7];
    vx_tensor  outputs = (vx_tensor)parameters[8];

    vx_uint32  batchCount = TENSOR_SIZE_INDEX(inputs, 3);
    vx_enum    inputFormat = TENSOR_DATA_TYPE(inputs);
    vx_enum    outputFormat = TENSOR_DATA_TYPE(outputs);
    vx_enum    biasFormat = TENSOR_DATA_TYPE(bias);

    vxnne_svdf_layer  svdfLayer = VX_NULL;
    vx_bool           dataFormat_flag = vx_false_e;
    vx_enum actType = VX_NN_ACTIVATION_NONE;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }
    gcoOS_Allocate(gcvNULL, sizeof(vxnne_svdf_layer_s), (gctPOINTER*)&svdfLayer);
    if (!svdfLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("Out of Memory at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    if(node->base.context->evisNoInst.supportEVIS)
    {
        dataFormat_flag = (vx_bool)(inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16
            && (biasFormat == VX_TYPE_FLOAT16 || biasFormat == VX_TYPE_FLOAT32));
    }
    else
    {
         dataFormat_flag = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16 && (biasFormat == VX_TYPE_FLOAT16 || biasFormat == VX_TYPE_FLOAT32)) ||
                                     (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32 && biasFormat == VX_TYPE_FLOAT32));
    }

    gcoOS_ZeroMemory(svdfLayer, sizeof(vxnne_svdf_layer_s));

    vxnneLayer_Initialize(&svdfLayer->base,
        "SVDF",
        node,
        vxmOPERATION_COUNT(svdfLayer),
        svdfLayer->operations,
        VX_NULL);

    if (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_TP) &&
        vxnneIsTPSupportFormat(node->base.context, inputs, VX_NULL, outputs)
        && weights_feature->tensorBuffer->data_lifetime == VX_TENSOR_LIFE_TIME_STATIC
        && recurrent_time->tensorBuffer->data_lifetime == VX_TENSOR_LIFE_TIME_STATIC
        )
    {
        vx_int32 count = 0;
        vx_op_param_s conv = { 0 };
        vx_tensor output_feature = VX_NULL, depth_weight_time = VX_NULL, input_fc = VX_NULL, state_in_fc = VX_NULL;
        vx_weights_biases_parameter weights_biases_feature = VX_NULL, weights_biases = VX_NULL;
        vx_bool feature_sw = vx_false_e, state_map_sw = vx_false_e, time_sw = vx_false_e/*, rotation_sw = vx_true_e*/;
        /* step2, map result to state in */
        vx_int32 batch_size = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
        vx_int32 input_size = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
        vx_int32 rank = (ranks != VX_NULL) ? (vx_int32)VX_GET_DATA_FROM_TENSOR(ranks, 0) : 1;
        vx_int32 num_unit = TENSOR_VIEW_SIZE_INDEX(weights_feature, 1);
        vx_int32 memory_size = TENSOR_VIEW_SIZE_INDEX(recurrent_time, 0);
        vx_bool aligned64 = ((batch_size > 1) && (input_size * TENSOR_DATA_SIZE(inputs)) % 64 == 0) ? vx_true_e : vx_false_e;

        vx_tensor_create_params_t tensor_create_params;
        vx_int32 sizes[][4] = {
            { 1, 1, num_unit, batch_size }, /*output feature*/
            { 1, 1, memory_size *  num_unit, num_unit / rank }, /*depth weight time*/
            { 1, 1, input_size, batch_size }, /*input fc reshape*/
            { 1, 1, memory_size *  num_unit, batch_size }, /*state in fc reshape*/
            { 1, 1, input_size, num_unit }, /*output feature*/
        };

        gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
        tensor_create_params.num_of_dims = 4;
        tensor_create_params.sizes = (vx_uint32_ptr)sizes[0];
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

        output_feature = vxoTensor_CreateVirtualTensor2(node->graph, &tensor_create_params, sizeof(vx_tensor_create_params_t));

        tensor_create_params.sizes = (vx_uint32_ptr)sizes[1];
        depth_weight_time = vxoTensor_CreateTensor2(node->base.context, &tensor_create_params, sizeof(vx_tensor_create_params_t));

        input_fc = vxoTensor_ReshapeTensor(inputs, sizes[2], 4);

        state_in_fc = vxoTensor_ReshapeTensor(state_in, sizes[3], 4);

        svdfLayer->base.temp_tensors[svdfLayer->base.num_temp_tensors++] = output_feature;
        svdfLayer->base.temp_tensors[svdfLayer->base.num_temp_tensors++] = depth_weight_time;
        svdfLayer->base.temp_tensors[svdfLayer->base.num_temp_tensors++] = input_fc;
        svdfLayer->base.temp_tensors[svdfLayer->base.num_temp_tensors++] = state_in_fc;

        vxnneExecuteSVDF_DepthWeightsTime(recurrent_time, depth_weight_time, rank);

        if (feature_sw)
        {

            status = vxnneOperation_Initialize(&svdfLayer->svdf_sw_fc_operations[0].base,
                &svdfLayer->base,
                VXNNE_OPERATION_TARGET_SW,
                VXNNE_OPERATOR_FULLYCONNECTED,
                vxnneExecuteSWFullyConnected,
                VX_NULL,
                batchCount,
                0);
            vxnneLayer_SetOperation(
                &svdfLayer->base,
                &svdfLayer->svdf_sw_fc_operations[0].base,
                count++);

            svdfLayer->svdf_sw_fc_operations[0].inputs = input_fc;
            svdfLayer->svdf_sw_fc_operations[0].weights = weights_feature;
            svdfLayer->svdf_sw_fc_operations[0].biases = VX_NULL;
            svdfLayer->svdf_sw_fc_operations[0].outputs = output_feature;

            vxnneOperation_AddReference(&svdfLayer->svdf_sw_fc_operations[0].base, (vx_reference)input_fc, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&svdfLayer->svdf_sw_fc_operations[0].base, (vx_reference)weights_feature, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&svdfLayer->svdf_sw_fc_operations[0].base, (vx_reference)output_feature, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }
        else if (aligned64)
        {
            /* step1, input * weight_feature */
            weights_biases_feature = _createWeightsBiasesParameterFromTensors(
                node->base.context,
                VX_NN_FULLYCONNECTED_LAYER,
                input_fc->dims,/*inputs_dims,*/
                input_fc->dimCount,
                output_feature->dimCount,
                0,
                0,
                0,
                0,
                0,/*pooling_size_x,*/
                0,/*pooling_size_y,*/
                0,
                0,
                VX_NN_DS_SIZE_ROUNDING_FLOOR,
                output_feature->dims,/*convolution_outputs_dims,*/
                output_feature->dims,/*pool_outputs_dims,*/
                NULL, /*optimizations,*/
                TENSOR_DATA_TYPE(weights_feature),
                0,
                VX_TENSOR_RANK_SN,
                weights_feature,
                VX_NULL,
                VX_NULL,
                vx_false_e,
                vx_false_e
            );

            vxoNNFullyConnectedLayerInitializer(
                node,
                &svdfLayer->base,
                &svdfLayer->svdf_tp_operation[count],
                VX_NULL,
                VX_NULL,
                VX_NULL,
                inputs,
                weights_biases_feature,
                0,
                0,
                vx_false_e,
                &count,
                output_feature);
        }
        else
        {
            /* NN implement */
            /*vx_op_param_s conv = { 0 };*/
            vx_tensor weights_feature_fc = VX_NULL;

            weights_feature_fc = vxoTensor_ReshapeTensor(weights_feature, sizes[4], 4);

            svdfLayer->base.temp_tensors[svdfLayer->base.num_temp_tensors++] = weights_feature_fc;

            weights_biases_feature = _createWeightsBiasesParameterFromTensors(
                vxGetContext((vx_reference)node),
                VX_NN_CONVOLUTION_LAYER,
                input_fc->dims,/*inputs_dims,*/
                input_fc->dimCount,
                output_feature->dimCount,
                0,
                0,
                0,
                0,
                0,/*pooling_size_x,*/
                0,/*pooling_size_y,*/
                0,
                0,
                VX_NN_DS_SIZE_ROUNDING_FLOOR,
                output_feature->dims,/*convolution_outputs_dims,*/
                output_feature->dims,/*pool_outputs_dims,*/
                NULL, /*optimizations,*/
                TENSOR_DATA_TYPE(weights_feature_fc),
                0,
                VX_TENSOR_RANK_WHCN,
                weights_feature_fc,
                VX_NULL,
                VX_NULL,
                vx_false_e,
                vx_false_e
            );

            status = vxnneOperation_Initialize(&svdfLayer->svdf_nn_operation[0].base,
                &svdfLayer->base,
                VXNNE_OPERATION_TARGET_NN,
                VXNNE_OPERATOR_CONVOLUTION,
                VX_NULL,
                VX_NULL,
                batch_size,
                NNE_COMMAND_SIZE);

            if (status != VX_SUCCESS) goto exit;

            conv.pad_x_left = conv.pad_x_right = conv.pad_y_top = conv.pad_y_bottom = 0;
            conv.pad_mode = VX_PAD_CONSTANT;
            conv.pad_const = 0;
            conv.pool_type = 0;
            conv.pool_size_x = conv.pool_size_y = 0;
            conv.conv_rounding_type = 0;
            conv.enable_relu = vx_false_e;

            vxnneLayer_SetOperation(
                &svdfLayer->base,
                &svdfLayer->svdf_nn_operation[0].base,
                count++);

            svdfLayer->svdf_nn_operation[0].orig_inputs = inputs;
            svdfLayer->svdf_nn_operation[0].inputs = input_fc;
            svdfLayer->svdf_nn_operation[0].weights_biases = weights_biases_feature;
            svdfLayer->svdf_nn_operation[0].outputs = output_feature;

            vxnneOperation_AddReference(&svdfLayer->svdf_nn_operation[0].base, (vx_reference)input_fc, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&svdfLayer->svdf_nn_operation[0].base, (vx_reference)output_feature, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            memcpy(&svdfLayer->svdf_nn_operation[0].base.parameter, &conv, sizeof(vx_op_param_s));
        }


        /* step2, map result to state in */
        if (state_map_sw)
        {
            status = vxnneOperation_Initialize(&svdfLayer->svdf_sw_operations[0].base,
                &svdfLayer->base,
                VXNNE_OPERATION_TARGET_SW,
                VXNNE_OPERATOR_SVDF_MAP,
                vxnneExecuteSWSVDF_MAP,
                VX_NULL,
                batchCount,
                0);
            vxnneLayer_SetOperation(
                &svdfLayer->base,
                &svdfLayer->svdf_sw_operations[0].base,
                count++);

            svdfLayer->svdf_sw_operations[0].inputs = output_feature;
            svdfLayer->svdf_sw_operations[0].outputs = state_in_fc;

            vxnneOperation_AddReference(&svdfLayer->svdf_sw_operations[0].base, (vx_reference)output_feature, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&svdfLayer->svdf_sw_operations[0].base, (vx_reference)state_in_fc, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }
        else
        {
            conv.pad_x_left = 0;
            conv.pad_y_top = 0;
            conv.pool_size_x = 0;
            conv.pool_size_y = 0;
            conv.pool_stride = 1;
            conv.enable_relu = vx_false_e;
            conv.pad_mode = VX_PAD_CONSTANT;
            conv.pad_const = 0;

            vxnneOperation_Initialize(&svdfLayer->svdf_tp_operation[count].base,
                &svdfLayer->base,
                VXNNE_OPERATION_TARGET_TP,
                VXNNE_OPERATOR_SVDF_MAP,
                VX_NULL,
                vxnneOperation_TP_Deinitialize,
                batch_size,
                0);

            vxnneLayer_SetOperation(
                &svdfLayer->base,
                &svdfLayer->svdf_tp_operation[1].base,
                count++);

            svdfLayer->svdf_tp_operation[count - 1].input = output_feature;
            svdfLayer->svdf_tp_operation[count - 1].output = state_in;

            vxnneOperation_AddReference(&svdfLayer->svdf_tp_operation[count - 1].base, (vx_reference)output_feature, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&svdfLayer->svdf_tp_operation[count - 1].base, (vx_reference)state_in, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            conv.tpType = TP_TENSOR_SVDF_MAP;
            conv.other_ref = (vx_reference)output_feature;

            conv.data_buff = gcvNULL;

            memcpy(&svdfLayer->svdf_tp_operation[count - 1].base.parameter, &conv, sizeof(vx_op_param_s));
        }

        aligned64 = ((batch_size > 1) && (memory_size *  num_unit * TENSOR_DATA_SIZE(inputs)) % 64 == 0) ? vx_true_e : vx_false_e;

        /* step3, state_in * weight_time */
        if (time_sw)
        {
            status = vxnneOperation_Initialize(&svdfLayer->svdf_sw_fc_operations[1].base,
                &svdfLayer->base,
                VXNNE_OPERATION_TARGET_SW,
                VXNNE_OPERATOR_FULLYCONNECTED,
                vxnneExecuteSWFullyConnected,
                VX_NULL,
                batchCount,
                0);
            vxnneLayer_SetOperation(
                &svdfLayer->base,
                &svdfLayer->svdf_sw_fc_operations[1].base,
                count++);

            svdfLayer->svdf_sw_fc_operations[1].inputs = state_in_fc;
            svdfLayer->svdf_sw_fc_operations[1].weights = depth_weight_time;
            svdfLayer->svdf_sw_fc_operations[1].biases = bias;
            svdfLayer->svdf_sw_fc_operations[1].outputs = outputs;

            vxnneOperation_AddReference(&svdfLayer->svdf_sw_fc_operations[1].base, (vx_reference)state_in_fc, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&svdfLayer->svdf_sw_fc_operations[1].base, (vx_reference)depth_weight_time, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&svdfLayer->svdf_sw_fc_operations[1].base, (vx_reference)bias, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&svdfLayer->svdf_sw_fc_operations[1].base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }
        else if (aligned64)
        {
            weights_biases = _createWeightsBiasesParameterFromTensors(
                node->base.context,
                VX_NN_FULLYCONNECTED_LAYER,
                state_in->dims,/*inputs_dims,*/
                state_in->dimCount,
                outputs->dimCount,
                0,
                0,
                0,
                0,
                0,/*pooling_size_x,*/
                0,/*pooling_size_y,*/
                0,
                0,
                VX_NN_DS_SIZE_ROUNDING_FLOOR,
                outputs->dims,/*convolution_outputs_dims,*/
                outputs->dims,/*pool_outputs_dims,*/
                NULL, /*optimizations,*/
                TENSOR_DATA_TYPE(depth_weight_time),
                0,
                VX_TENSOR_RANK_SN,
                depth_weight_time,
                bias,
                VX_NULL,
                vx_false_e,
                vx_false_e
            );

            vxoNNFullyConnectedLayerInitializer(
                node,
                &svdfLayer->base,
                &svdfLayer->svdf_tp_operation[count],
                VX_NULL,
                VX_NULL,
                VX_NULL,
                state_in,
                weights_biases,
                0,
                0,
                vx_false_e,
                &count,
                outputs);
        }
        else
        {

            /* NN implement */
            /*vx_op_param_s conv = { 0 };*/
            vx_tensor output_fc = VX_NULL;
            vx_int32 nn_sizes[] = { 1, 1, TENSOR_SIZE_INDEX(outputs, 0), TENSOR_SIZE_INDEX(outputs, 1) };

            output_fc = vxoTensor_ReshapeTensor(outputs, nn_sizes, 4);

            svdfLayer->base.temp_tensors[svdfLayer->base.num_temp_tensors++] = output_fc;

            weights_biases = _createWeightsBiasesParameterFromTensors(
                vxGetContext((vx_reference)node),
                VX_NN_CONVOLUTION_LAYER,
                state_in_fc->dims,/*inputs_dims,*/
                state_in_fc->dimCount,
                output_fc->dimCount,
                0,
                0,
                0,
                0,
                0,/*pooling_size_x,*/
                0,/*pooling_size_y,*/
                0,
                0,
                VX_NN_DS_SIZE_ROUNDING_FLOOR,
                output_fc->dims,/*convolution_outputs_dims,*/
                output_fc->dims,/*pool_outputs_dims,*/
                NULL, /*optimizations,*/
                TENSOR_DATA_TYPE(depth_weight_time),
                0,
                VX_TENSOR_RANK_WHCN,
                depth_weight_time,
                bias,
                VX_NULL,
                vx_false_e,
                vx_false_e
            );

            status = vxnneOperation_Initialize(&svdfLayer->svdf_nn_operation[1].base,
                &svdfLayer->base,
                VXNNE_OPERATION_TARGET_NN,
                VXNNE_OPERATOR_CONVOLUTION,
                VX_NULL,
                VX_NULL,
                batch_size,
                NNE_COMMAND_SIZE);

            if (status != VX_SUCCESS) goto exit;

            conv.pad_x_left = conv.pad_x_right = conv.pad_y_top = conv.pad_y_bottom = 0;
            conv.pad_mode = VX_PAD_CONSTANT;
            conv.pad_const = 0;
            conv.pool_type = 0;
            conv.pool_size_x = conv.pool_size_y = 0;
            conv.conv_rounding_type = 0;
            conv.enable_relu = vx_false_e;

            vxnneLayer_SetOperation(
                &svdfLayer->base,
                &svdfLayer->svdf_nn_operation[1].base,
                count++);

            svdfLayer->svdf_nn_operation[1].orig_inputs = state_in;
            svdfLayer->svdf_nn_operation[1].inputs = state_in_fc;
            svdfLayer->svdf_nn_operation[1].weights_biases = weights_biases;
            svdfLayer->svdf_nn_operation[1].outputs = output_fc;

            vxnneOperation_AddReference(&svdfLayer->svdf_nn_operation[1].base, (vx_reference)state_in_fc, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&svdfLayer->svdf_nn_operation[1].base, (vx_reference)output_fc, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            memcpy(&svdfLayer->svdf_nn_operation[1].base.parameter, &conv, sizeof(vx_op_param_s));
        }
    }
    else if (dataFormat_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
    {
        vxnne_shader_executable shaderExecutable;

        if (node->base.context->evisNoInst.supportEVIS)
        {
            shaderExecutable = vxnneGetSvdfShaderExecutable(node->base.context, VXNNE_KERNEL_SVDF, &node->kernelAttributes.borderMode,
                inputs, bias, weights_feature, recurrent_time, activation, ranks, state_in, state_out, outputs);
        }
        else
        {
            vx_int32 rankValue = (ranks != VX_NULL) ? (vx_int32)VX_GET_DATA_FROM_TENSOR(ranks, 0) : 1;

            if (activation != VX_NULL)
            {
                switch (*(vx_int32_ptr)TENSOR_LOGICAL_ADDR(activation))
                {
                case 0:
                default:
                    actType = VX_NN_ACTIVATION_NONE;
                    break;
                case 1:
                    actType = VX_NN_ACTIVATION_RELU;
                    break;
                case 2:
                    actType = VX_NN_ACTIVATION_RELU1;
                    break;
                case 3:
                    actType = VX_NN_ACTIVATION_RELU6;
                    break;
                }
            }

            shaderExecutable = vxnneGetGPUSvdfShaderExecutable(node->base.context, VXNNE_KERNEL_SVDF, &node->kernelAttributes.borderMode,
                inputs, bias, weights_feature, recurrent_time, actType, rankValue, state_in, state_out, outputs);
        }

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&svdfLayer->svdf_sh_operation,
            &svdfLayer->base,
            VXNNE_OPERATOR_SVDF,
            batchCount,
            shaderExecutable);
        if (status != VX_SUCCESS) goto exit;

        vxnneOperation_AddReference(&svdfLayer->svdf_sh_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&svdfLayer->svdf_sh_operation.base, (vx_reference)weights_feature, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&svdfLayer->svdf_sh_operation.base, (vx_reference)recurrent_time, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&svdfLayer->svdf_sh_operation.base, (vx_reference)bias, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&svdfLayer->svdf_sh_operation.base, (vx_reference)activation, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&svdfLayer->svdf_sh_operation.base, (vx_reference)ranks, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&svdfLayer->svdf_sh_operation.base, (vx_reference)state_in, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&svdfLayer->svdf_sh_operation.base, (vx_reference)state_out, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        vxnneOperation_AddReference(&svdfLayer->svdf_sh_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        vxnneLayer_SetOperation(
            &svdfLayer->base,
            &svdfLayer->svdf_sh_operation.base,
            0);
    }
    else
    {
        vxnneOperation_Initialize(&svdfLayer->svdf_sw_operation.base,
            &svdfLayer->base,
            VXNNE_OPERATION_TARGET_SW,
            VXNNE_OPERATOR_SVDF,
            vxnneExecuteSWSVDF,
            VX_NULL,
            batchCount,
            0);

        vxnneLayer_SetOperation(
            &svdfLayer->base,
            &svdfLayer->svdf_sw_operation.base,
            0);

        svdfLayer->svdf_sw_operation.inputs = inputs;
        svdfLayer->svdf_sw_operation.weights_feature = weights_feature;
        svdfLayer->svdf_sw_operation.recurrent_time = recurrent_time;
        svdfLayer->svdf_sw_operation.bias = bias;
        svdfLayer->svdf_sw_operation.state_in = state_in;
        svdfLayer->svdf_sw_operation.rank = ranks;
        svdfLayer->svdf_sw_operation.activation = activation;
        svdfLayer->svdf_sw_operation.state_out = state_out;
        svdfLayer->svdf_sw_operation.outputs = outputs;

        vxnneOperation_AddReference(&svdfLayer->svdf_sw_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&svdfLayer->svdf_sw_operation.base, (vx_reference)weights_feature, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&svdfLayer->svdf_sw_operation.base, (vx_reference)recurrent_time, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&svdfLayer->svdf_sw_operation.base, (vx_reference)bias, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&svdfLayer->svdf_sw_operation.base, (vx_reference)state_in, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&svdfLayer->svdf_sw_operation.base, (vx_reference)state_out, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        vxnneOperation_AddReference(&svdfLayer->svdf_sw_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }
    node->layer = &svdfLayer->base;

    return status;
exit:
    if (svdfLayer) gcoOS_Free(NULL, (gctPOINTER)svdfLayer);
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoSVDFLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}


vx_status vxnneExecuteSWLstmPreProcessConcat(vx_tensor* inputs, vx_uint32 input_num, vx_tensor output)
{
    gctPOINTER inputBase[4] = { 0 };
    gctPOINTER outputBase;
    vx_uint32  index;

    vx_status status = VX_SUCCESS;
    vx_uint32 dims = TENSOR_DIM_NUM(inputs[0]);
    vx_uint32 size = TENSOR_SIZE_INDEX(inputs[0], dims - 1) * TENSOR_STRIDE_INDEX(inputs[0], dims - 1);
    vx_uint32 width = TENSOR_VIEW_SIZE_INDEX(inputs[0], 0);
    vx_uint32 height = dims > 1 ? TENSOR_VIEW_SIZE_INDEX(inputs[0], 1) : 1;
    vx_uint32 depth = dims > 1 ? TENSOR_VIEW_SIZE_INDEX(inputs[0], 2) : 1;
    vx_uint32 elementCount = width * height *depth;

    for (index = 0; index < input_num; index++)
    {
        vxoTensor_GetTensorViewMemory(inputs[index], &(inputBase[index]), VX_NULL);
    }
    vxoTensor_GetTensorViewMemory(output, &outputBase, VX_NULL);

    if (TENSOR_DATA_TYPE(inputs[0]) == TENSOR_DATA_TYPE(output))
    {
        for (index = 0; index < input_num; index++)
        {
            memcpy(((vx_int8 *)outputBase) + index * size, inputBase[index], size);
        }
    }
    else
    {

        for (index = 0; index < input_num; index++)
        {
            vx_uint32 element = 0;
            for (element = 0; element < elementCount; element++)
            {
                vx_float32 data = vxnneGetDataExt(TENSOR_DATA_TYPE(inputs[index]), TENSOR_QUANT_TYPE(inputs[index]), element, inputBase[index], TENSOR_POS(inputs[index]), TENSOR_TF_ZEROPOINT(inputs[index]), TENSOR_TF_SCALE(inputs[index]));

                vxnneSaveDataExt(TENSOR_DATA_TYPE(output), TENSOR_QUANT_TYPE(output), index * elementCount + element, data, (vx_int8 *)outputBase, TENSOR_POS(output), TENSOR_TF_ZEROPOINT(output), TENSOR_TF_SCALE(output), TENSOR_ROUNDING_MODE(output));
            }
        }
    }

    return status;
}

/*
 * Concat multiple tensors into one according to the last axis.
 *
 * Prerequests of the input tensors:
 * 1. Have the same dimensions.
 * 2. Have the same data format and quantization format (if any).
 *
 * The output tensor has the same dimensions as inputs except for
 * the last axis. And it is the same data format and quantization
 * format (if any) as inputs.
 */
vx_status _ConcatTensors(
    vx_node node,
    vx_tensor *inputs,
    vx_uint32 input_num,
    vx_type_e output_data_format,
    vx_tensor *output)
{
    vx_tensor target = VX_NULL;
    vx_uint32 dim_num = TENSOR_DIM_NUM(inputs[0]);
    vx_uint32_ptr sizes = VX_NULL;
    vx_tensor_create_params_t tensor_param;
    vx_uint32 i = 0;

    vx_status status = VX_SUCCESS;

    sizes = (vx_uint32_ptr)vxAllocate(gcmSIZEOF(vx_uint32) * dim_num);
    if (!sizes)
    {
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }

    for (i = 0; i < dim_num - 1; i++)
    {
        sizes[i] = TENSOR_SIZE_INDEX(inputs[0], i);
    }

    for (i = 0; i < input_num; i++)
    {
        sizes[dim_num - 1] += TENSOR_SIZE_INDEX(inputs[i], dim_num - 1);
    }

    vxZeroMemory(&tensor_param, gcmSIZEOF(vx_tensor_create_params_t));

    tensor_param.num_of_dims = dim_num;
    tensor_param.sizes = sizes;
    tensor_param.data_format = output_data_format;
    tensor_param.quant_format = TENSOR_QUANT_TYPE(inputs[0]);
    if (tensor_param.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
    {
        tensor_param.quant_data.dfp.fixed_point_pos = TENSOR_POS(inputs[0]);
    }
    else if (tensor_param.quant_format == VX_QUANT_AFFINE_SCALE)
    {
        tensor_param.quant_data.affine.scale = TENSOR_TF_SCALE(inputs[0]);
        tensor_param.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(inputs[0]);
    }

    target = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_param, vx_false_e);
    if (!target)
    {
        vxmONERROR(VX_ERROR_NO_RESOURCES);
    }

    vxmONERROR(vxoTensor_AllocateMemory(target));

    vxmONERROR(vxnneExecuteSWLstmPreProcessConcat(inputs,
                                                  input_num,
                                                  target));

    *output = target;

    /* Release the resources. */
    vxFree(sizes);

    return status;

OnError:
    if (target)
    {
        vxoTensor_ReleaseTensor(&target);
    }

    if (sizes)
    {
        vxFree(sizes);
    }

    return status;
}

static vx_float32 sigmoid(vx_float32 x) {
    return (vx_float32)(1. / (1. + exp(-x)));
}

/*TODO: the function vxnneGetData/vxnneSaveData called in this function need to be replaced by vxnneGetDataExt/vxnneSaveDataExt !
But as didn't find any place to call this function, delay the change as the interface need to be refined too! */
VX_PRIVATE_API vx_status vxnneSW_LSTMUnit(vx_int32 batch, vx_int32 output_size, vx_type_e input_format, vx_type_e output_format,
    vx_uint8_ptr c_i_1, vx_int8 c_i_1_fixed_position,
    vx_uint8_ptr input_i, vx_int8 input_i_fixed_position,
    vx_uint8_ptr forget_i, vx_int8 forget_i_fixed_position,
    vx_uint8_ptr output_i, vx_int8 output_i_fixed_position,
    vx_uint8_ptr gate_i, vx_int8 gate_i_fixed_position,
    vx_uint8_ptr cont_i, vx_int8 cont_i_fixed_position,
    vx_uint8_ptr c_i, vx_int8 c_i_fixed_position,
    vx_uint8_ptr h_i, vx_int8 h_i_fixed_position
)
{
    vx_int32 n = 0, d = 0;
    vx_int32 x_dim = output_size * 4;
    vx_int32 item_size = vxnneGetTypeSize(input_format);

    for (n = 0; n < batch; n++)
    {
        for (d = 0; d < output_size; d++)
        {
            const vx_float64 i = sigmoid(vxnneGetData(input_format, d, input_i, input_i_fixed_position));
            const vx_float64 f = (*cont_i == 0) ? 0 :
                (*cont_i * sigmoid(vxnneGetData(input_format, d, forget_i, forget_i_fixed_position)));
            const vx_float64 o = sigmoid(vxnneGetData(input_format, d, output_i, output_i_fixed_position));
            const vx_float64 g = tanh((vx_float64)(vxnneGetData(input_format, d, gate_i, gate_i_fixed_position)));
            const vx_float64 c_prev_data = vxnneGetData(input_format, d, c_i_1, c_i_1_fixed_position);
            const vx_float64 c_data = (vx_float64)(f * c_prev_data + i * g);
            vx_float64 tanh_c = 0;
            /*c_i[d] = (vx_float32)c_data;*/
            vxnneSaveData(output_format, d, c_data, c_i, c_i_fixed_position, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING);
            tanh_c = tanh(c_data);
            /*h_i[d] = (vx_float32)(o * tanh_c);*/
            vxnneSaveData(output_format, d, o * tanh_c, h_i, h_i_fixed_position, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING);
        }

        c_i_1 += output_size * item_size;

        input_i += x_dim * item_size;
        forget_i += x_dim * item_size;
        output_i += x_dim * item_size;
        gate_i += x_dim * item_size;

        c_i += output_size * item_size;
        h_i += output_size * item_size;

        cont_i += item_size;
    }

    return VX_SUCCESS;
}

typedef enum _lstm_activation_e
{
    lstm_activation_none = 0,
    lstm_activation_relu = 1,
    lstm_activation_relu6 = 3,
    lstm_activation_tanh = 4,
    lstm_activation_sigmoid = 6,
}
lstm_activation_e;

void vxnneLSTM_MatrixBatchVectorMultiplyAccumulate(vx_type_e vec_format, vx_type_e mat_format, vx_type_e result_format, const vx_uint8_ptr matrix,
    vx_int32 m_rows, vx_int32 m_cols, vx_int8 mat_fixPointPos,
    const vx_uint8_ptr vector,
    vx_int32 n_batch, vx_int8 vec_fixPointPos, vx_uint8_ptr result, vx_int8 result_fixPointPos,
    vx_int32 result_stride)
{
    vx_int32 b = 0, r = 0, c = 0;
    vx_uint8_ptr result_in_batch = result;
    vx_float32 data = 0.f;
    for (b = 0; b < n_batch; b++)
    {
        vx_uint8_ptr matrix_ptr = matrix;
        for (r = 0; r < m_rows; r++)
        {
            vx_uint8_ptr vector_in_batch = vector + b * m_cols * vxnneGetTypeSize(vec_format);
            for (c = 0; c < m_cols; c++)
            {
                /* *result_in_batch += *matrix_ptr++ * *vector_in_batch++;*/
                data = vxnneGetData(result_format, 0, result_in_batch, result_fixPointPos);
                data += vxnneGetData(mat_format, 0, matrix_ptr, mat_fixPointPos) * vxnneGetData(vec_format, 0, vector_in_batch, vec_fixPointPos);
                vxnneSaveData(result_format, 0, data, result_in_batch, result_fixPointPos, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING);

                matrix_ptr += vxnneGetTypeSize(mat_format);
                vector_in_batch += vxnneGetTypeSize(vec_format);

            }
            result_in_batch += result_stride * vxnneGetTypeSize(result_format);
        }
    }
}

void vxnneLSTM_VectorBatchVectorCwiseProductAccumulate(vx_type_e vec_format, vx_type_e batch_format, vx_type_e result_format, vx_uint8* vector,
    vx_int32 v_size, vx_int8 vec_fixPointPos,
    vx_uint8* batch_vector,
    vx_int32 n_batch, vx_int8 batch_fixPointPos,
    vx_uint8* result, vx_int8 result_fixPointPos) {
    vx_int32 v = 0, b = 0;
    vx_float32 data = 0.f;
    for (b = 0; b < n_batch; b++)
    {
        for (v = 0; v < v_size; v++)
        {
            /**result++ += vector[v] * *batch_vector++;*/
            data = vxnneGetData(result_format, 0, result, result_fixPointPos);
            data += vxnneGetData(vec_format, v, vector, vec_fixPointPos) * vxnneGetData(batch_format, 0, batch_vector, batch_fixPointPos);
            vxnneSaveData(result_format, 0, data, result, result_fixPointPos, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING);
            batch_vector += vxnneGetTypeSize(batch_format);
            result += vxnneGetTypeSize(result_format);
        }
    }
}

void vxnneLSTM_VectorBatchVectorCwiseProduct(vx_type_e vec_format, vx_type_e batch_format, vx_type_e result_format, vx_uint8* vector,
    vx_int32 v_size, vx_int8 vec_fixPointPos,
    vx_uint8* batch_vector,
    vx_int32 n_batch, vx_int8 batch_fixPointPos,
    vx_uint8* result, vx_int8 result_fixPointPos) {
    vx_int32 v = 0, b = 0;
    vx_float32 data = 0.f;
    for (b = 0; b < n_batch; b++)
    {
        for (v = 0; v < v_size; v++)
        {
            /**result++ = vector[v] * *batch_vector++;*/
            data = vxnneGetData(vec_format, v, vector, vec_fixPointPos) * vxnneGetData(batch_format, 0, batch_vector, batch_fixPointPos);
            vxnneSaveData(result_format, 0, data, result, result_fixPointPos, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING);
            batch_vector += vxnneGetTypeSize(batch_format);
            result       += vxnneGetTypeSize(result_format);
        }
    }
}

void vxnneLSTM_VectorBatchVectorAdd(vx_type_e vec_format, vx_type_e batch_format, vx_uint8* vector, vx_int8 vec_fpp, vx_int32 v_size, vx_int32 n_batch,
    vx_uint8* batch_vector, vx_int8 batch_fpp) {
    vx_float32 data = 0.f;
    vx_int32 i = 0, b = 0;

    for (b = 0; b < n_batch; b++) {
        for (i = 0; i < v_size; ++i) {
            /*batch_vector[i] += vector[i];*/
            data = vxnneGetData(batch_format, i, batch_vector, batch_fpp);
            data += vxnneGetData(vec_format, i, vector, vec_fpp);
            vxnneSaveData(batch_format, i, data, batch_vector, batch_fpp, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING);
        }
        batch_vector += v_size * vxnneGetTypeSize(batch_format);
    }
}

void vxnneLSTM_ZeroVector(vx_type_e vec_format, vx_uint8* vector, vx_int8 vec_fpp, vx_int32 v_size) {
    vx_int32 i = 0;
     for (i = 0; i < v_size; ++i) {
         /*vector[i] = 0;*/
         vxnneSaveData(vec_format, i, 0, vector, vec_fpp, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING);
     }
}

void vxnneLSTM_VectorBatchVectorAssign(vx_type_e vec_format, vx_type_e bat_format, const vx_uint8* vector, vx_int32 v_size, vx_int8 vec_fixPointPos,
    vx_int32 n_batch, vx_uint8* batch_vector, vx_int8 batch_fixPointPos) {
    vx_int32 b = 0;
    vx_int32 item_size = vxnneGetTypeSize(vec_format);
    for (b = 0; b < n_batch; b++)
    {
        if (vec_format == bat_format)
            memcpy(batch_vector + b * v_size * item_size, vector, v_size * item_size);
        else
        {
            vx_int32 i = 0;
            vx_float32 data = 0.f;
            for (i = 0; i < v_size; i++)
            {
                data = vxnneGetDataExt(vec_format, 0, i, (vx_uint8*)vector, vec_fixPointPos, 0, 0.f);
                vxnneSaveData(bat_format, b * v_size + i, data, batch_vector, batch_fixPointPos, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING);
            }
        }
    }
}

void vxnneLSTM_VectorVectorAssign(vx_type_e vec_format, vx_type_e bat_format, const vx_uint8* vector, vx_int32 v_size, vx_int8 vec_fixPointPos,
    vx_int32 n_batch, vx_uint8* batch_vector, vx_int8 batch_fixPointPos) {
    vx_int32 b = 0;
    vx_int32 item_size = vxnneGetTypeSize(vec_format);

    if (vec_format == bat_format)
        memcpy(batch_vector, vector, v_size * n_batch * item_size);
    else
    {
        for (b = 0; b < n_batch; b++)
        {
            vx_int32 i = 0;
            vx_float32 data = 0.f;
            for (i = 0; i < v_size; i++)
            {
                data = vxnneGetDataExt(vec_format, 0, i + b * v_size, (vx_uint8*)vector, vec_fixPointPos, 0, 0.f);
                vxnneSaveData(bat_format, b * v_size + i, data, batch_vector, batch_fixPointPos, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING);
            }
        }
    }
}

void vxnneLSTM_VectorVectorCwiseProduct(vx_type_e vec1_format, vx_type_e vec2_format, vx_type_e result_format, vx_uint8* vector1,
    vx_uint8* vector2, vx_int32 v_size, vx_int8 vec1_fixPointPos, vx_int8 vec2_fixPointPos,
    vx_uint8* result, vx_int8 result_fixPointPos) {
    vx_int32 v = 0;
    for (v = 0; v < v_size; v++)
    {
        /**result++ = *vector1++ * *vector2++;*/
        vxnneSaveData(result_format, 0, vxnneGetData(vec1_format, 0, vector1, vec1_fixPointPos) * vxnneGetData(vec2_format, 0, vector2, vec2_fixPointPos), result, result_fixPointPos, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING);
        vector1 += vxnneGetTypeSize(vec1_format);
        vector2 += vxnneGetTypeSize(vec2_format);
        result += vxnneGetTypeSize(result_format);
    }
}

#define RELU(a) (((a) < 0.f) ? 0.f : (a))

#define RELU6(a) gcmMAX(0.f, gcmMIN((a), 6.f))

void vxnneLSTM_ActivationToVector(vx_type_e vec_format, vx_type_e result_format, vx_uint8* vector, vx_int32 v_size,
    vx_enum activation, vx_int8 vec_fixPointPos,
    vx_uint8* result, vx_int8 result_fixPointPos)
{
    vx_int32 v = 0;
    switch (activation)
    {
    case lstm_activation_relu:
        for (v = 0; v < v_size; v++) {
            /**result++ = RELU(*vector++);*/
            vxnneSaveData(result_format, 0, RELU(vxnneGetData(vec_format, 0, vector, vec_fixPointPos)), result, result_fixPointPos, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING);

            vector += vxnneGetTypeSize(vec_format);
            result += vxnneGetTypeSize(result_format);
        }
        break;
    case lstm_activation_relu6:
        for (v = 0; v < v_size; v++) {
            /**result++ = RELU6(*vector++);*/
            vxnneSaveData(result_format, 0, RELU6(vxnneGetData(vec_format, 0, vector, vec_fixPointPos)), result, result_fixPointPos, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING);

            vector += vxnneGetTypeSize(vec_format);
            result += vxnneGetTypeSize(result_format);
        }
        break;
    case lstm_activation_tanh:
        for (v = 0; v < v_size; v++) {
            /**result++ = (vx_float32)tanh(*vector++);*/
            vxnneSaveData(result_format, 0, tanh(vxnneGetData(vec_format, 0, vector, vec_fixPointPos)), result, result_fixPointPos, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING);

            vector += vxnneGetTypeSize(vec_format);
            result += vxnneGetTypeSize(result_format);
        }
        break;
    case lstm_activation_sigmoid:
        for (v = 0; v < v_size; v++) {
            /**result++ = (vx_float32)sigmoid(*vector++);*/
            vxnneSaveData(result_format, 0, sigmoid(vxnneGetData(vec_format, 0, vector, vec_fixPointPos)), result, result_fixPointPos, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING);

            vector += vxnneGetTypeSize(vec_format);
            result += vxnneGetTypeSize(result_format);
        }
        break;
    case lstm_activation_none:
    default:
        break;
    }
}

void vxnneLSTM_SigmoidToVector(vx_type_e vec_format, vx_type_e result_format, vx_uint8* vector, vx_int32 v_size, vx_int8 vec_fixPointPos,
    vx_uint8* result, vx_int8 result_fixPointPos) {
    vxnneLSTM_ActivationToVector(vec_format, result_format, vector, v_size, lstm_activation_sigmoid, vec_fixPointPos, result, result_fixPointPos);
}

void vxnneLSTM_Sub1Vector(vx_type_e vec_format, vx_type_e result_format, vx_uint8* vector, vx_int32 v_size, vx_int8 vec_fixPointPos, vx_uint8* result, vx_int8 result_fixPointPos) {
    vx_int32 v = 0;
    for (v = 0; v < v_size; v++) {
        /**result++ = 1.0f - *vector++;*/
        vxnneSaveData(result_format, 0, 1.0f - vxnneGetData(vec_format, 0, vector, vec_fixPointPos), result, result_fixPointPos, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING);
        vector += vxnneGetTypeSize(vec_format);
        result += vxnneGetTypeSize(result_format);
    }
}

void vxnneLSTM_BiasVector(vx_type_e vec_format, vx_type_e result_format, vx_uint8* vector, vx_int32 v_size, vx_float32 bias, vx_int8 vec_fixPointPos, vx_uint8* result, vx_int8 result_fixPointPos) {
    vx_int32 v = 0;
    for (v = 0; v < v_size; v++) {
        /**result++ = 1.0f - *vector++;*/
        vxnneSaveData(result_format, 0, bias + vxnneGetData(vec_format, 0, vector, vec_fixPointPos), result, result_fixPointPos, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING);
        vector += vxnneGetTypeSize(vec_format);
        result += vxnneGetTypeSize(result_format);
    }
}

void vxnneLSTM_VectorVectorCwiseProductAccumulate(vx_type_e vec1_format, vx_type_e vec2_format, vx_type_e result_format, vx_uint8* vector1,
    vx_uint8* vector2, vx_int8 vec1_fixPointPos, vx_int8 vec2_fixPointPos,
    vx_int32 v_size, vx_uint8* result, vx_int8 result_fixPointPos) {
    vx_int32 v = 0;
    vx_float32 data = 0;
    for (v = 0; v < v_size; v++) {
        /**result++ += *vector1++ * *vector2++;*/
        data = vxnneGetData(result_format, 0, result, result_fixPointPos);
        data += vxnneGetData(vec1_format, 0, vector1, vec1_fixPointPos) * vxnneGetData(vec2_format, 0, vector2, vec2_fixPointPos);
        vxnneSaveData(result_format, 0, data, result, result_fixPointPos, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING);
        vector1 += vxnneGetTypeSize(vec1_format);
        vector2 += vxnneGetTypeSize(vec2_format);
        result += vxnneGetTypeSize(result_format);
    }
}

static vx_float32 vxnneLSTM_Clip(vx_float32 f, vx_float32 abs_limit) {
    vx_float32 result = (abs_limit < f) ? abs_limit : f;
    result = (-abs_limit > result) ? -abs_limit : result;
    return result;
}

void vxnneLSTM_ClipVector(vx_type_e vec_format, vx_type_e result_format, vx_uint8* vector, vx_int32 v_size, vx_int8 vec_fixPointPos, vx_float32 abs_limit,
    vx_uint8* result, vx_int8 result_fixPointPos) {
    vx_int32 v = 0;
    for (v = 0; v < v_size; v++) {
        /**result++ = vxnneLSTM_Clip(format, *vector++, abs_limit);*/
        vxnneSaveData(result_format, 0, vxnneLSTM_Clip(vxnneGetData(vec_format, 0, vector, vec_fixPointPos), abs_limit), result, result_fixPointPos, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING);
        vector += vxnneGetTypeSize(vec_format);
        result += vxnneGetTypeSize(result_format);
    }
}

void vxnneLSTM_MeanStddevNormalization(vx_type_e in_format, vx_type_e out_format, vx_uint8* input_vector, vx_int8 in_fpp,
    vx_int32 v_size, vx_int32 n_batch, vx_float32 normalization_epsilon, vx_uint8* output_vector, vx_int8 out_fpp)
{
    vx_int32 batch = 0, i = 0;
    vx_float32 mean = .0f, stddev_inv = .0f, variance = .0f, input_d = .0f;

    for (batch = 0; batch < n_batch; ++batch)
    {
        vx_float32 sum = 0.0f;
        vx_float32 sum_sq = 0.0f;
        for (i = 0; i < v_size; ++i)
        {
            input_d = vxnneGetData(in_format, i, input_vector, in_fpp);
            sum += input_d;
            sum_sq += input_d * input_d;
        }

        mean = sum / v_size;
        stddev_inv = 0.0f;
        variance = sum_sq / v_size - mean * mean;

        if (variance == 0)
        {
            stddev_inv = (vx_float32)(1.0f / sqrt(normalization_epsilon));
        }
        else
        {
            stddev_inv = (vx_float32)(1.0f / sqrt(variance));
        }

        for (i = 0; i < v_size; ++i)
        {
            input_d = vxnneGetData(in_format, i, input_vector, in_fpp);
            /*output_vector[i] = (input_d - mean) * stddev_inv;*/
            vxnneSaveData(out_format, i, (input_d - mean) * stddev_inv, output_vector, out_fpp, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING);
        }
        input_vector += v_size * vxnneGetTypeSize(in_format);
        output_vector += v_size * vxnneGetTypeSize(out_format);
    }
}

vx_status vxnneExecuteSW_LSTMUnit(struct _vxnne_operation_s *operation)
{
    vxnne_lstm_unit_sw_operation lstmUnitOperation = (vxnne_lstm_unit_sw_operation)operation;

    vx_tensor input = (vx_tensor)lstmUnitOperation->input;

    vx_tensor input2input_weight = (vx_tensor)lstmUnitOperation->input2input_weight;
    vx_tensor input2forget_weight = (vx_tensor)lstmUnitOperation->input2forget_weight;
    vx_tensor input2cell_weight = (vx_tensor)lstmUnitOperation->input2cell_weight;
    vx_tensor input2output_weight = (vx_tensor)lstmUnitOperation->input2output_weight;

    vx_tensor recurrent2input_weight = (vx_tensor)lstmUnitOperation->recurrent2input_weight;
    vx_tensor recurrent2forget_weight = (vx_tensor)lstmUnitOperation->recurrent2forget_weight;
    vx_tensor recurrent2cell_weight = (vx_tensor)lstmUnitOperation->recurrent2cell_weight;
    vx_tensor recurrent2output_weight = (vx_tensor)lstmUnitOperation->recurrent2output_weight;

    vx_tensor layernorm2input_weight = (vx_tensor)lstmUnitOperation->layernorm2input_weight;
    vx_tensor layernorm2forget_weight = (vx_tensor)lstmUnitOperation->layernorm2forget_weight;
    vx_tensor layernorm2cell_weight = (vx_tensor)lstmUnitOperation->layernorm2cell_weight;
    vx_tensor layernorm2output_weight = (vx_tensor)lstmUnitOperation->layernorm2output_weight;

    vx_tensor cell2input_weight = (vx_tensor)lstmUnitOperation->cell2input_weight;
    vx_tensor cell2forget_weight = (vx_tensor)lstmUnitOperation->cell2forget_weight;
    vx_tensor cell2output_weight = (vx_tensor)lstmUnitOperation->cell2output_weight;

    vx_tensor input_gate_bias = (vx_tensor)lstmUnitOperation->input_gate_bias;
    vx_tensor forget_gate_bias = (vx_tensor)lstmUnitOperation->forget_gate_bias;
    vx_tensor cell_bias = (vx_tensor)lstmUnitOperation->cell_bias;
    vx_tensor output_gate_bias = (vx_tensor)lstmUnitOperation->output_gate_bias;

    vx_tensor output_state_in = (vx_tensor)lstmUnitOperation->output_state_in;
    vx_tensor cell_state_in = (vx_tensor)lstmUnitOperation->cell_state_in;

    vx_tensor projection_weight = (vx_tensor)lstmUnitOperation->projection_weight;
    vx_tensor projection_bias = (vx_tensor)lstmUnitOperation->projection_bias;

    vx_tensor scratch = (vx_tensor)lstmUnitOperation->scratch;
    vx_tensor output_state_out = (vx_tensor)lstmUnitOperation->output_state_out;
    vx_tensor cell_state_out = (vx_tensor)lstmUnitOperation->cell_state_out;
    vx_tensor output = (vx_tensor)lstmUnitOperation->output;

    vx_enum activation = (lstmUnitOperation->activation != VX_NULL) ? (vx_enum)VX_GET_DATA_FROM_TENSOR(lstmUnitOperation->activation, 0) : VX_NN_ACTIVATION_NONE;

    vx_float32 forget_bias = (lstmUnitOperation->forget_bias != VX_NULL) ? VX_GET_DATA_FROM_TENSOR(lstmUnitOperation->forget_bias, 0) : 0.f;
    vx_float32 cell_clip = (lstmUnitOperation->cell_clip != VX_NULL) ? VX_GET_DATA_FROM_TENSOR(lstmUnitOperation->cell_clip, 0) : 0.f;
    vx_float32 proj_clip = (lstmUnitOperation->proj_clip != VX_NULL) ? VX_GET_DATA_FROM_TENSOR(lstmUnitOperation->proj_clip, 0) : 0.f;

    vx_bool enable_cifg = (input2input_weight == VX_NULL) ? vx_true_e : vx_false_e;

    vx_bool enable_peephole = (cell2output_weight != VX_NULL) ? vx_true_e : vx_false_e;
    vx_bool enable_proj_weight = (projection_weight != VX_NULL) ? vx_true_e : vx_false_e;
    vx_bool enable_proj_bias = (projection_bias != VX_NULL) ? vx_true_e : vx_false_e;

    vx_bool enable_layer_norm = (layernorm2forget_weight != VX_NULL) ? vx_true_e : vx_false_e;

    vx_uint32 cell = TENSOR_SIZE_INDEX(input2forget_weight, 1);
    vx_uint32 batch = TENSOR_SIZE_INDEX(input, 1);
    vx_uint32 input_count = TENSOR_SIZE_INDEX(input, 0);
    vx_uint32 output_count = TENSOR_SIZE_INDEX(recurrent2output_weight, 0);

    vx_uint8_ptr scatch_ptr = VX_NULL;
    vx_uint8_ptr scatch_input_ptr = VX_NULL, scatch_forget_ptr = VX_NULL, scatch_cell_ptr = VX_NULL, scatch_output_ptr = VX_NULL;

    vx_type_e input_format = (vx_type_e)TENSOR_DATA_TYPE(input);
    vx_int32 item_size = vxnneGetTypeSize(input_format);

    vx_int8 scratch_fpos = 0;
    vx_type_e scratch_format = (vx_type_e)TENSOR_DATA_TYPE(scratch);/* VX_TYPE_FLOAT32;*/
    vx_int32 scratch_item_size = vxnneGetTypeSize(scratch_format);

    vx_uint8_ptr input_base = VX_NULL, input_gate_bias_base = VX_NULL, forget_gate_bias_base = VX_NULL, cell_bias_base = VX_NULL, output_gate_bias_base = VX_NULL;
    vx_uint8_ptr input2input_weight_base = VX_NULL, input2cell_weight_base = VX_NULL, input2forget_weight_base = VX_NULL, input2output_weight_base = VX_NULL;
    vx_uint8_ptr recurrent2input_weight_base = VX_NULL, recurrent2cell_weight_base = VX_NULL, recurrent2forget_weight_base = VX_NULL, recurrent2output_weight_base = VX_NULL;
    vx_uint8_ptr cell2input_weight_base = VX_NULL, cell2forget_weight_base = VX_NULL, cell2output_weight_base = VX_NULL;
    vx_uint8_ptr layernorm2input_weight_base = VX_NULL, layernorm2cell_weight_base = VX_NULL, layernorm2forget_weight_base = VX_NULL, layernorm2output_weight_base = VX_NULL;
    vx_uint8_ptr cell_state_in_base = VX_NULL, output_state_in_base = VX_NULL, output_state_out_base = VX_NULL, cell_state_out_base = VX_NULL, output_base = VX_NULL;
    vx_uint8_ptr projection_weight_base = VX_NULL, projection_bias_base = VX_NULL;
    vx_uint8_ptr cell_state_base = VX_NULL;
    vx_float32 normalization_epsilon = 1e-8f;

    vxoTensor_GetTensorViewMemory(scratch, (vx_ptr_ptr)&scatch_ptr, VX_NULL);
    vxoTensor_GetTensorViewMemory(input, (vx_ptr_ptr)&input_base, VX_NULL);

    vxoTensor_GetTensorViewMemory(input2input_weight, (vx_ptr_ptr)&input2input_weight_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(input2cell_weight, (vx_ptr_ptr)&input2cell_weight_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(input2forget_weight, (vx_ptr_ptr)&input2forget_weight_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(input2output_weight, (vx_ptr_ptr)&input2output_weight_base, VX_NULL);

    vxoTensor_GetTensorViewMemory(recurrent2input_weight, (vx_ptr_ptr)&recurrent2input_weight_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(recurrent2cell_weight, (vx_ptr_ptr)&recurrent2cell_weight_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(recurrent2forget_weight, (vx_ptr_ptr)&recurrent2forget_weight_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(recurrent2output_weight, (vx_ptr_ptr)&recurrent2output_weight_base, VX_NULL);

    if (enable_layer_norm)
    {
        vxoTensor_GetTensorViewMemory(layernorm2input_weight, (vx_ptr_ptr)&layernorm2input_weight_base, VX_NULL);
        vxoTensor_GetTensorViewMemory(layernorm2cell_weight, (vx_ptr_ptr)&layernorm2cell_weight_base, VX_NULL);
        vxoTensor_GetTensorViewMemory(layernorm2forget_weight, (vx_ptr_ptr)&layernorm2forget_weight_base, VX_NULL);
        vxoTensor_GetTensorViewMemory(layernorm2output_weight, (vx_ptr_ptr)&layernorm2output_weight_base, VX_NULL);
    }

    vxoTensor_GetTensorViewMemory(cell2input_weight, (vx_ptr_ptr)&cell2input_weight_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(cell2forget_weight, (vx_ptr_ptr)&cell2forget_weight_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(cell2output_weight, (vx_ptr_ptr)&cell2output_weight_base, VX_NULL);

    vxoTensor_GetTensorViewMemory(input_gate_bias, (vx_ptr_ptr)&input_gate_bias_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(forget_gate_bias, (vx_ptr_ptr)&forget_gate_bias_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(cell_bias, (vx_ptr_ptr)&cell_bias_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(output_gate_bias, (vx_ptr_ptr)&output_gate_bias_base, VX_NULL);

    vxoTensor_GetTensorViewMemory(output_state_in, (vx_ptr_ptr)&output_state_in_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(cell_state_in, (vx_ptr_ptr)&cell_state_in_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(output_state_out, (vx_ptr_ptr)&output_state_out_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(cell_state_out, (vx_ptr_ptr)&cell_state_out_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(output, (vx_ptr_ptr)&output_base, VX_NULL);

    vxoTensor_GetTensorViewMemory(projection_weight, (vx_ptr_ptr)&projection_weight_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(projection_bias, (vx_ptr_ptr)&projection_bias_base, VX_NULL);

    if (!enable_cifg && enable_peephole && (cell2input_weight == VX_NULL))
    {
        enable_cifg = vx_true_e;
    }

    gcoOS_Allocate(VX_NULL, cell * batch * scratch_item_size, (vx_ptr_ptr)&cell_state_base);

    /* Index the scratch buffers pointers to the global scratch buffer. */
    if (enable_cifg)
    {
        scatch_forget_ptr = scatch_ptr + cell * batch * item_size;
        scatch_cell_ptr = scatch_forget_ptr + cell * batch * item_size;
        scatch_output_ptr = scatch_cell_ptr + cell * batch * item_size;

    }
    else
    {
        scatch_input_ptr = scatch_ptr;
        scatch_forget_ptr = scatch_input_ptr + cell * batch * item_size;
        scatch_cell_ptr = scatch_forget_ptr + cell * batch * item_size;
        scatch_output_ptr = scatch_cell_ptr + cell * batch * item_size;
    }

    if (!enable_layer_norm)
    {

    /* Initialize scratch buffers with bias. Bias_i,c,f,g*/
        if (!enable_cifg && input_gate_bias)
        vxnneLSTM_VectorBatchVectorAssign((vx_type_e)TENSOR_DATA_TYPE(input_gate_bias), input_format, input_gate_bias_base,
            cell, TENSOR_POS(input_gate_bias), batch, (vx_uint8_ptr)scatch_input_ptr, TENSOR_POS(scratch));
        if (cell_bias)

    vxnneLSTM_VectorBatchVectorAssign((vx_type_e)TENSOR_DATA_TYPE(cell_bias), input_format, cell_bias_base,
                cell, TENSOR_POS(cell_bias), batch, (vx_uint8_ptr)scatch_cell_ptr, TENSOR_POS(scratch));

        if (output_gate_bias)
    vxnneLSTM_VectorBatchVectorAssign((vx_type_e)TENSOR_DATA_TYPE(forget_gate_bias), input_format, forget_gate_bias_base,
                cell, TENSOR_POS(forget_gate_bias), batch, (vx_uint8_ptr)scatch_forget_ptr, TENSOR_POS(scratch));

        if (output_gate_bias)
    vxnneLSTM_VectorBatchVectorAssign((vx_type_e)TENSOR_DATA_TYPE(output_gate_bias), input_format, output_gate_bias_base,
                cell, TENSOR_POS(output_gate_bias), batch, (vx_uint8_ptr)scatch_output_ptr, TENSOR_POS(scratch));
    }
    else
    {
        if (!enable_cifg)
            vxnneLSTM_ZeroVector((vx_type_e)TENSOR_DATA_TYPE(scratch), (vx_uint8_ptr)scatch_input_ptr, TENSOR_POS(scratch), cell * batch);

        vxnneLSTM_ZeroVector((vx_type_e)TENSOR_DATA_TYPE(scratch), (vx_uint8_ptr)scatch_cell_ptr, TENSOR_POS(scratch), cell * batch);
        vxnneLSTM_ZeroVector((vx_type_e)TENSOR_DATA_TYPE(scratch), (vx_uint8_ptr)scatch_forget_ptr, TENSOR_POS(scratch), cell * batch);
        vxnneLSTM_ZeroVector((vx_type_e)TENSOR_DATA_TYPE(scratch), (vx_uint8_ptr)scatch_output_ptr, TENSOR_POS(scratch), cell * batch);
    }

    /* For each batch and cell: compute input_weight * input. W_x_i,c,f,o * input*/
    if (!enable_cifg)
        vxnneLSTM_MatrixBatchVectorMultiplyAccumulate(input_format, (vx_type_e)TENSOR_DATA_TYPE(input2input_weight), scratch_format, input2input_weight_base,
            cell, input_count, TENSOR_POS(input2input_weight), input_base, batch, TENSOR_POS(input), (vx_uint8_ptr)scatch_input_ptr, TENSOR_POS(scratch), /*result_stride=*/1);

    vxnneLSTM_MatrixBatchVectorMultiplyAccumulate(input_format, (vx_type_e)TENSOR_DATA_TYPE(input2cell_weight), scratch_format, input2cell_weight_base,
        cell, input_count, TENSOR_POS(input2cell_weight), input_base, batch, TENSOR_POS(input), (vx_uint8_ptr)scatch_cell_ptr, TENSOR_POS(scratch), /*result_stride=*/1);
    vxnneLSTM_MatrixBatchVectorMultiplyAccumulate(input_format, (vx_type_e)TENSOR_DATA_TYPE(input2forget_weight), scratch_format, input2forget_weight_base,
        cell, input_count, TENSOR_POS(input2forget_weight), input_base, batch, TENSOR_POS(input), (vx_uint8_ptr)scatch_forget_ptr, TENSOR_POS(scratch), /*result_stride=*/1);
    vxnneLSTM_MatrixBatchVectorMultiplyAccumulate(input_format, (vx_type_e)TENSOR_DATA_TYPE(input2output_weight), scratch_format, input2output_weight_base,
        cell, input_count, TENSOR_POS(input2output_weight), input_base, batch, TENSOR_POS(input), (vx_uint8_ptr)scatch_output_ptr, TENSOR_POS(scratch), /*result_stride=*/1);

    /* For each batch and cell: compute recurrent_weight * output_state_in. W_h_i,c,f,o * h_t-1*/
    if (!enable_cifg)
        vxnneLSTM_MatrixBatchVectorMultiplyAccumulate((vx_type_e)TENSOR_DATA_TYPE(output_state_in), (vx_type_e)TENSOR_DATA_TYPE(recurrent2input_weight), scratch_format, recurrent2input_weight_base,
            cell, output_count, TENSOR_POS(recurrent2input_weight), output_state_in_base, batch, TENSOR_POS(output_state_in), (vx_uint8_ptr)scatch_input_ptr, TENSOR_POS(scratch), /*result_stride=*/1);

    vxnneLSTM_MatrixBatchVectorMultiplyAccumulate((vx_type_e)TENSOR_DATA_TYPE(output_state_in), (vx_type_e)TENSOR_DATA_TYPE(recurrent2cell_weight), scratch_format, recurrent2cell_weight_base,
        cell, output_count, TENSOR_POS(recurrent2cell_weight), output_state_in_base, batch, TENSOR_POS(output_state_in), (vx_uint8_ptr)scatch_cell_ptr, TENSOR_POS(scratch), /*result_stride=*/1);
    vxnneLSTM_MatrixBatchVectorMultiplyAccumulate((vx_type_e)TENSOR_DATA_TYPE(output_state_in), (vx_type_e)TENSOR_DATA_TYPE(recurrent2forget_weight), scratch_format, recurrent2forget_weight_base,
        cell, output_count, TENSOR_POS(recurrent2forget_weight), output_state_in_base, batch, TENSOR_POS(output_state_in), (vx_uint8_ptr)scatch_forget_ptr, TENSOR_POS(scratch), /*result_stride=*/1);
    vxnneLSTM_MatrixBatchVectorMultiplyAccumulate((vx_type_e)TENSOR_DATA_TYPE(output_state_in), (vx_type_e)TENSOR_DATA_TYPE(recurrent2output_weight), scratch_format, recurrent2output_weight_base,
        cell, output_count, TENSOR_POS(recurrent2output_weight), output_state_in_base, batch, TENSOR_POS(output_state_in), (vx_uint8_ptr)scatch_output_ptr, TENSOR_POS(scratch), /*result_stride=*/1);

    /* For each batch and cell: update input gate. */
    if (!enable_cifg)
    {
        if (enable_peephole)
            vxnneLSTM_VectorBatchVectorCwiseProductAccumulate((vx_type_e)TENSOR_DATA_TYPE(cell2input_weight), (vx_type_e)TENSOR_DATA_TYPE(cell_state_in), scratch_format, cell2input_weight_base,
                cell, TENSOR_POS(cell2input_weight), cell_state_in_base, batch, TENSOR_POS(cell_state_in), (vx_uint8_ptr)scatch_input_ptr, scratch_fpos);

        if (enable_layer_norm)
        {
            vxnneLSTM_MeanStddevNormalization(scratch_format, scratch_format, scatch_input_ptr, TENSOR_POS(scratch), cell, batch, normalization_epsilon, scatch_input_ptr, TENSOR_POS(scratch));

            vxnneLSTM_VectorBatchVectorCwiseProduct((vx_type_e)TENSOR_DATA_TYPE(layernorm2input_weight), scratch_format, scratch_format, layernorm2input_weight_base,
                cell, TENSOR_POS(layernorm2input_weight), scatch_input_ptr, batch, scratch_fpos, (vx_uint8_ptr)scatch_input_ptr, scratch_fpos);

            vxnneLSTM_VectorBatchVectorAdd((vx_type_e)TENSOR_DATA_TYPE(input_gate_bias), scratch_format, input_gate_bias_base, TENSOR_POS(input_gate_bias), cell, batch, (vx_uint8_ptr)scatch_input_ptr, scratch_fpos);
        }

        /*i_t = sigmoid(W_x_i * input + W_h_i * h_t-1 + Bias_i)*/
        vxnneLSTM_SigmoidToVector(scratch_format, scratch_format, (vx_uint8_ptr)scatch_input_ptr, cell * batch, scratch_fpos, (vx_uint8_ptr)scatch_input_ptr, scratch_fpos);
    }

    /* For each batch and cell: update forget gate. */
    if (enable_peephole)
        vxnneLSTM_VectorBatchVectorCwiseProductAccumulate((vx_type_e)TENSOR_DATA_TYPE(cell2forget_weight), (vx_type_e)TENSOR_DATA_TYPE(cell_state_in), scratch_format, cell2forget_weight_base,
            cell, TENSOR_POS(cell2forget_weight), cell_state_in_base, batch, TENSOR_POS(cell_state_in), (vx_uint8_ptr)scatch_forget_ptr, scratch_fpos);

    if (enable_layer_norm)
    {
        vxnneLSTM_MeanStddevNormalization(scratch_format, scratch_format, scatch_forget_ptr, TENSOR_POS(scratch), cell, batch, normalization_epsilon, scatch_forget_ptr, TENSOR_POS(scratch));

        vxnneLSTM_VectorBatchVectorCwiseProduct((vx_type_e)TENSOR_DATA_TYPE(layernorm2forget_weight), scratch_format, scratch_format, layernorm2forget_weight_base,
            cell, TENSOR_POS(layernorm2forget_weight), scatch_forget_ptr, batch, scratch_fpos, (vx_uint8_ptr)scatch_forget_ptr, scratch_fpos);

        vxnneLSTM_VectorBatchVectorAdd((vx_type_e)TENSOR_DATA_TYPE(forget_gate_bias), scratch_format, forget_gate_bias_base, TENSOR_POS(forget_gate_bias), cell, batch, (vx_uint8_ptr)scatch_forget_ptr, scratch_fpos);
    }

    if (forget_bias != 0)
        vxnneLSTM_BiasVector(scratch_format, scratch_format, (vx_uint8_ptr)scatch_forget_ptr, cell * batch, forget_bias, scratch_fpos, (vx_uint8_ptr)scatch_forget_ptr, scratch_fpos);

    /*f_t = sigmoid(W_x_f * input + W_h_f * h_t-1 + Bias_f)*/
    vxnneLSTM_SigmoidToVector(scratch_format, scratch_format, (vx_uint8_ptr)scatch_forget_ptr, cell * batch, scratch_fpos, (vx_uint8_ptr)scatch_forget_ptr, scratch_fpos);

    if (enable_layer_norm)
    {
        vxnneLSTM_MeanStddevNormalization(scratch_format, scratch_format, scatch_cell_ptr, TENSOR_POS(scratch), cell, batch, normalization_epsilon, scatch_cell_ptr, TENSOR_POS(scratch));

        vxnneLSTM_VectorBatchVectorCwiseProduct((vx_type_e)TENSOR_DATA_TYPE(layernorm2cell_weight), scratch_format, scratch_format, layernorm2cell_weight_base,
            cell, TENSOR_POS(layernorm2cell_weight), scatch_cell_ptr, batch, scratch_fpos, (vx_uint8_ptr)scatch_cell_ptr, scratch_fpos);

        vxnneLSTM_VectorBatchVectorAdd((vx_type_e)TENSOR_DATA_TYPE(cell_bias), scratch_format, cell_bias_base, TENSOR_POS(cell_bias), cell, batch, (vx_uint8_ptr)scatch_cell_ptr, scratch_fpos);
    }

    /* For each batch and cell: update the cell. cell_state_out = f_t * c_t-1 = (sigmoid(W_x_f * input + W_h_f * h_t-1 + Bias_f) * c_t-1*/
    vxnneLSTM_VectorVectorCwiseProduct(scratch_format, (vx_type_e)TENSOR_DATA_TYPE(cell_state_in), scratch_format, (vx_uint8_ptr)scatch_forget_ptr, cell_state_in_base,
        cell * batch, scratch_fpos, TENSOR_POS(cell_state_in), cell_state_base, scratch_fpos);

    /*c_t = tanh(W_x_c * input + W_h_c * h_t-1 + Bias_c)*/
    vxnneLSTM_ActivationToVector(scratch_format, scratch_format, (vx_uint8_ptr)scatch_cell_ptr, cell * batch, activation, scratch_fpos, (vx_uint8_ptr)scatch_cell_ptr, scratch_fpos);


    if (enable_cifg)
    {
        vxnneLSTM_Sub1Vector(scratch_format, scratch_format, (vx_uint8_ptr)scatch_forget_ptr, cell * batch, scratch_fpos, (vx_uint8_ptr)scatch_forget_ptr, scratch_fpos);
        vxnneLSTM_VectorVectorCwiseProductAccumulate(scratch_format, scratch_format, scratch_format, (vx_uint8_ptr)scatch_cell_ptr, (vx_uint8_ptr)scatch_forget_ptr, scratch_fpos, scratch_fpos,
            cell * batch, cell_state_base, scratch_fpos);
    }
    else
        /* cell_state_out = f_t * c_t-1 + c_t * i_t = (sigmoid(W_x_f * input + W_h_f * h_t-1 + Bias_f) * c_t-1 + sigmoid(W_x_i * input + W_h_i * h_t-1 + Bias_i) * tanh(W_x_c * input + W_h_c * h_t-1 + Bias_c) */
        vxnneLSTM_VectorVectorCwiseProductAccumulate(scratch_format, scratch_format, scratch_format, (vx_uint8_ptr)scatch_cell_ptr, (vx_uint8_ptr)scatch_input_ptr, scratch_fpos, scratch_fpos,
            cell * batch, cell_state_base, scratch_fpos);

    if (cell_clip > 0.0f)
        vxnneLSTM_ClipVector(scratch_format, scratch_format, cell_state_base,
            cell * batch, scratch_fpos, cell_clip, cell_state_base, scratch_fpos);

    /* For each batch and cell: update the output gate. */
    if (enable_peephole)
        vxnneLSTM_VectorBatchVectorCwiseProductAccumulate((vx_type_e)TENSOR_DATA_TYPE(cell2output_weight), scratch_format, scratch_format, cell2output_weight_base,
            cell, TENSOR_POS(cell2output_weight), cell_state_base, batch, scratch_fpos, (vx_uint8_ptr)scatch_output_ptr, scratch_fpos);

    if (enable_layer_norm)
    {
        vxnneLSTM_MeanStddevNormalization(scratch_format, scratch_format, scatch_output_ptr, TENSOR_POS(scratch), cell, batch, normalization_epsilon, scatch_output_ptr, TENSOR_POS(scratch));

        vxnneLSTM_VectorBatchVectorCwiseProduct((vx_type_e)TENSOR_DATA_TYPE(layernorm2output_weight), scratch_format, scratch_format, layernorm2output_weight_base,
            cell, TENSOR_POS(layernorm2output_weight), scatch_output_ptr, batch, scratch_fpos, (vx_uint8_ptr)scatch_output_ptr, scratch_fpos);

        vxnneLSTM_VectorBatchVectorAdd((vx_type_e)TENSOR_DATA_TYPE(output_gate_bias), scratch_format, output_gate_bias_base, TENSOR_POS(output_gate_bias), cell, batch, (vx_uint8_ptr)scatch_output_ptr, scratch_fpos);
    }

    /*o_t = sigmoid(W_x_o * input + W_h_o * h_t-1 + Bias_o)*/
    vxnneLSTM_SigmoidToVector(scratch_format, scratch_format, (vx_uint8_ptr)scatch_output_ptr, cell * batch, scratch_fpos, (vx_uint8_ptr)scatch_output_ptr, scratch_fpos);

    /*c_t = tanh(cell_state_out)*/
    vxnneLSTM_ActivationToVector(scratch_format, scratch_format, cell_state_base,
        cell * batch, activation, scratch_fpos, (vx_uint8_ptr)scatch_cell_ptr, scratch_fpos);

    vxnneLSTM_VectorVectorAssign(scratch_format, (vx_type_e)TENSOR_DATA_TYPE(cell_state_out), cell_state_base,
        cell, scratch_fpos, batch, (vx_uint8_ptr)cell_state_out_base, TENSOR_POS(cell_state_out));

    /*o_t = c_t * o_t = tanh(cell_state_out) * sigmoid(W_x_o * input + W_h_o * h_t-1 + Bias_o)*/
    vxnneLSTM_VectorVectorCwiseProduct(scratch_format, scratch_format, scratch_format, (vx_uint8_ptr)scatch_output_ptr, (vx_uint8_ptr)scatch_cell_ptr, cell * batch, scratch_fpos, scratch_fpos, (vx_uint8_ptr)scatch_output_ptr, scratch_fpos);

    /* For each batch: update the projection and output_state_out.*/
    if (enable_proj_weight)
    {
        if (enable_proj_bias)
            vxnneLSTM_VectorBatchVectorAssign((vx_type_e)TENSOR_DATA_TYPE(projection_bias), input_format, projection_bias_base,
                output_count, TENSOR_POS(projection_bias), batch, projection_bias_base, TENSOR_POS(projection_bias));
        else
            memset((vx_float32_ptr)output_state_out_base, 0, output_count * batch * item_size);

        vxnneLSTM_MatrixBatchVectorMultiplyAccumulate((vx_type_e)TENSOR_DATA_TYPE(projection_weight), scratch_format, (vx_type_e)TENSOR_DATA_TYPE(output_state_out), projection_weight_base,
            output_count, cell, TENSOR_POS(projection_weight), (vx_uint8_ptr)scatch_output_ptr, batch, scratch_fpos, output_state_out_base, TENSOR_POS(output_state_out), 1);

        if (proj_clip > 0.0f)
            vxnneLSTM_ClipVector((vx_type_e)TENSOR_DATA_TYPE(output_state_out), (vx_type_e)TENSOR_DATA_TYPE(output_state_out), output_state_out_base,
                output_count * batch, TENSOR_POS(output_state_out), proj_clip, output_state_out_base, TENSOR_POS(output_state_out));
    }
    else
        vxnneLSTM_VectorVectorAssign(scratch_format, (vx_type_e)TENSOR_DATA_TYPE(output_state_out), scatch_output_ptr,
            output_count, scratch_fpos, batch, (vx_uint8_ptr)output_state_out_base, TENSOR_POS(output_state_out));


    if (output_base)
        memcpy(output_base, (vx_uint8_ptr)output_state_out_base,
            output_count * batch * item_size);

    if (cell_state_base != VX_NULL)
        gcoOS_Free(VX_NULL, cell_state_base);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NN_LSTMUnit(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_LSTMUnit_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_LSTMUnit_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxnneExecute_LSTM_Convolution_DeInilition(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_convolution_relu_pooling_operation convOperation = (vxnne_convolution_relu_pooling_operation)operation;

    if (convOperation->weights_biases)
        vxReleaseWeightsBiasesParameter(&convOperation->weights_biases);

    return status;
}

VX_PRIVATE_API vx_status vxnneExecute_LSTM_ConvolutionSW_DeInilition(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_convolution_operation convOperation = (vxnne_convolution_operation)operation;

    if (convOperation->weights)
        vxoTensor_ReleaseTensor(&convOperation->weights);

    if (convOperation->padX)
        vxReleaseScalar(&convOperation->padX);

    if (convOperation->padY)
        vxReleaseScalar(&convOperation->padY);

    if (convOperation->downScaleSizeRounding)
        vxReleaseScalar(&convOperation->downScaleSizeRounding);

    return status;
}


VX_PRIVATE_API vx_status vxoNNSWLSTMConv_ReshuffleWeightBias(vx_tensor input2input_weights, vx_tensor input2forget_weights, vx_tensor input2cell_weights, vx_tensor input2output_weights,
    vx_tensor recurrent2input_weights, vx_tensor recurrent2forget_weights, vx_tensor recurrent2cell_weights, vx_tensor recurrent2output_weights,
    vx_tensor cell2input_weights, vx_tensor cell2forget_weights, vx_tensor cell2output_weights,
    vx_tensor input_bias, vx_tensor forget_bias, vx_tensor cell_bias, vx_tensor output_bias,
    vx_tensor weights_conv, vx_tensor biases_conv)
{
    vx_status status = VX_SUCCESS;
    vx_int32 cell = TENSOR_SIZE_INDEX(input2forget_weights, 1);
    vx_int32 input_size = TENSOR_SIZE_INDEX(input2forget_weights, 0);
    vx_int32 output_size = TENSOR_SIZE_INDEX(recurrent2forget_weights, 0);
    vx_int32 depth = 4;
    vx_int32 i = 0, j = 0, d = 0;
    vx_tensor input_weights[] = { input2input_weights, input2forget_weights, input2cell_weights, input2output_weights };
    vx_tensor recurrent_weights[] = { recurrent2input_weights, recurrent2forget_weights, recurrent2cell_weights, recurrent2output_weights };

    vx_tensor input_biases[] = { input_bias, forget_bias, cell_bias, output_bias };
    vx_uint8_ptr weights_conv_base = VX_NULL, biases_conv_base = VX_NULL;

    vxoTensor_AllocateMemory(weights_conv);
    vxoTensor_AllocateMemory(biases_conv);

    vxoTensor_GetTensorViewMemory(weights_conv, (void**)&weights_conv_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(biases_conv, (void**)&biases_conv_base, VX_NULL);

    for (d = 0; d < depth; d++)
    {
        vx_uint8_ptr recurrent_weights_base = VX_NULL, input_weights_base = VX_NULL, input_biases_base = VX_NULL;
        vxoTensor_GetTensorViewMemory(recurrent_weights[d], (void**)&recurrent_weights_base, VX_NULL);
        vxoTensor_GetTensorViewMemory(input_weights[d], (void**)&input_weights_base, VX_NULL);
        vxoTensor_GetTensorViewMemory(input_biases[d], (void**)&input_biases_base, VX_NULL);

        if ((input_weights[d] == VX_NULL) || (recurrent_weights[d] == VX_NULL) || (input_biases[d] == VX_NULL))
            continue;

        /* reshuffle input_weights data */
        if (TENSOR_DATA_TYPE(input_weights[d]) == TENSOR_DATA_TYPE(weights_conv))
        {
            for (i = 0; i < cell; i++)
            {
                vx_int32 item_size = TENSOR_DATA_SIZE(weights_conv);
                memcpy(weights_conv_base + item_size * (input_size + output_size) * (i + cell * d), input_weights_base + i * item_size * input_size, item_size * input_size);
            }
        }
        else
        {
            for (i = 0; i < cell; i++)
            {
                for (j = 0; j < input_size; j++)
                {
                    vx_float32 data = VX_GET_DATA_FROM_TENSOR(input_weights[d], j + i * input_size);
                    VX_SAVE_DATA_TO_TENSOR(weights_conv, data, (input_size + output_size) * (i + cell * d) + j);
                }
            }
        }

        /* reshuffle recurrent_weights data */
        if (TENSOR_DATA_TYPE(recurrent_weights[d]) == TENSOR_DATA_TYPE(weights_conv))
        {
            for (i = 0; i < cell; i++)
            {
                vx_int32 item_size = TENSOR_DATA_SIZE(weights_conv);
                memcpy(weights_conv_base + item_size * ((input_size + output_size) * (i + cell * d) + input_size), recurrent_weights_base + i * item_size * output_size, item_size * output_size);
            }
        }
        else
        {
            for (i = 0; i < cell; i++)
            {
                for (j = 0; j < output_size; j++)
                {
                    vx_float32 data = VX_GET_DATA_FROM_TENSOR(recurrent_weights[d], j + i * output_size);
                    VX_SAVE_DATA_TO_TENSOR(weights_conv, data, (input_size + output_size) * (i + cell * d) + input_size + j);
                }
            }
        }

        /* reshuffle recurrent_biases data */
        if (TENSOR_DATA_TYPE(input_biases[d]) == TENSOR_DATA_TYPE(biases_conv))
        {
            vx_int32 item_size = TENSOR_DATA_SIZE(biases_conv);
            memcpy(biases_conv_base + item_size * output_size * d, input_biases_base, item_size * output_size);
        }
        else
        {
            for (j = 0; j < output_size; j++)
            {
                vx_float32 data = VX_GET_DATA_FROM_TENSOR(input_biases[d], j);
                VX_SAVE_DATA_TO_TENSOR(biases_conv, data, output_size * d + j);
            }
        }
    }

    return status;
}

VX_PRIVATE_API vx_status vxoNNSWLSTMConv_ReshuffleInput(vx_tensor input, vx_tensor output_state_in, vx_tensor cell_state_in, vx_tensor input_conv)
{
    vx_status status = VX_SUCCESS;
    vx_int32 batch = TENSOR_SIZE_INDEX(input, TENSOR_DIM_NUM(input) - 1);
    vx_int32 input_size = TENSOR_SIZE_INDEX(input, 0);
    vx_int32 output_size = TENSOR_SIZE_INDEX(output_state_in, 0);
    /*vx_int32 cell        = TENSOR_SIZE_INDEX(cell_state_in, 0);*/
    vx_int32 j = 0, b = 0;

    vx_uint8_ptr input_conv_base = VX_NULL;
    vx_uint8_ptr input_base = VX_NULL;
    vx_uint8_ptr output_state_in_base = VX_NULL;

    vxoTensor_GetTensorViewMemory(input_conv, (void**)&input_conv_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(input, (void**)&input_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(output_state_in, (void**)&output_state_in_base, VX_NULL);

    for (b = 0; b < batch; b++)
    {
        /* reshuffle input data */
        if (TENSOR_DATA_TYPE(input) == TENSOR_DATA_TYPE(input_conv))
        {
            vx_int32 item_size = TENSOR_DATA_SIZE(input);
            memcpy(input_conv_base + item_size * ((input_size + output_size) * b), input_base, item_size * input_size);
        }
        else
        {
            for (j = 0; j < input_size; j++)
            {
                vx_float32 data = VX_GET_DATA_FROM_TENSOR(input, j);
                VX_SAVE_DATA_TO_TENSOR(input_conv, data, (input_size + output_size) * b + j);
            }
        }

        /* reshuffle output state in data */
        if (TENSOR_DATA_TYPE(output_state_in) == TENSOR_DATA_TYPE(input_conv))
        {
            vx_int32 item_size = TENSOR_DATA_SIZE(input_conv);
            memcpy(input_conv_base + item_size * ((input_size + output_size) * b + input_size), output_state_in_base, item_size * output_size);
        }
        else
        {
            for (j = 0; j < output_size; j++)
            {
                vx_float32 data = VX_GET_DATA_FROM_TENSOR(output_state_in, j);
                VX_SAVE_DATA_TO_TENSOR(input_conv, data, (input_size + output_size) * b + input_size + j);
            }
        }
    }

    return status;
}


VX_PRIVATE_API vx_status vxoNNSWLSTM_ReshuffleInput(vxnne_operation operation)
{
    vxnne_lstm_resuffle_input_operation reshuffleOperation = (vxnne_lstm_resuffle_input_operation)operation;

    return vxoNNSWLSTMConv_ReshuffleInput(reshuffleOperation->input, reshuffleOperation->output_state_in, reshuffleOperation->cell_state_in, reshuffleOperation->reshuffled_input);
}

VX_PRIVATE_API vx_status vxoNNSWLSTM_StateOut(vxnne_operation operation)
{

    vxnne_lstm_unit_sw_operation lstmUnitOperation = (vxnne_lstm_unit_sw_operation)operation;

    vx_tensor input = (vx_tensor)lstmUnitOperation->input;

    vx_tensor input2input_weight = (vx_tensor)lstmUnitOperation->input2input_weight;
    vx_tensor input2forget_weight = (vx_tensor)lstmUnitOperation->input2forget_weight;

    vx_tensor recurrent2output_weight = (vx_tensor)lstmUnitOperation->recurrent2output_weight;
    vx_tensor cell2output_weight = (vx_tensor)lstmUnitOperation->cell2output_weight;
    vx_tensor cell2input_weight = (vx_tensor)lstmUnitOperation->cell2input_weight;
    vx_tensor cell2forget_weight = (vx_tensor)lstmUnitOperation->cell2forget_weight;

    vx_tensor cell_state_in = (vx_tensor)lstmUnitOperation->cell_state_in;

    vx_tensor projection_weight = (vx_tensor)lstmUnitOperation->projection_weight;
    vx_tensor projection_bias = (vx_tensor)lstmUnitOperation->projection_bias;

    vx_tensor output_state_out = (vx_tensor)lstmUnitOperation->output_state_out;
    vx_tensor cell_state_out = (vx_tensor)lstmUnitOperation->cell_state_out;
    vx_tensor output = (vx_tensor)lstmUnitOperation->output;

    vx_enum activation = (lstmUnitOperation->activation != VX_NULL) ? (vx_enum)VX_GET_DATA_FROM_TENSOR(lstmUnitOperation->activation, 0) : 4;
    vx_float32 forget_bias = (lstmUnitOperation->forget_bias != VX_NULL) ? VX_GET_DATA_FROM_TENSOR(lstmUnitOperation->forget_bias, 0) : 0.f;
    vx_float32 cell_clip = (lstmUnitOperation->cell_clip != VX_NULL) ? VX_GET_DATA_FROM_TENSOR(lstmUnitOperation->cell_clip, 0) : 0.f;
    vx_float32 proj_clip = (lstmUnitOperation->proj_clip != VX_NULL) ? VX_GET_DATA_FROM_TENSOR(lstmUnitOperation->proj_clip, 0) : 0.f;

    vx_bool enable_cifg = (input2input_weight == VX_NULL) ? vx_true_e : vx_false_e;

    vx_bool enable_peephole = (cell2output_weight != VX_NULL) ? vx_true_e : vx_false_e;
    vx_bool enable_proj_weight = (projection_weight != VX_NULL) ? vx_true_e : vx_false_e;
    vx_bool enable_proj_bias = (projection_bias != VX_NULL) ? vx_true_e : vx_false_e;

    vx_uint32 cell = TENSOR_SIZE_INDEX(input2forget_weight, 1);
    vx_uint32 batch = TENSOR_SIZE_INDEX(input, 1);
    vx_uint32 output_count = TENSOR_SIZE_INDEX(recurrent2output_weight, 0);

    vx_uint8_ptr scatch_ptr = VX_NULL;
    vx_uint8_ptr scatch_input_ptr = VX_NULL, scatch_forget_ptr = VX_NULL, scatch_cell_ptr = VX_NULL, scatch_output_ptr = VX_NULL;
    vx_uint8_ptr cell2input_weight_base = VX_NULL, cell2forget_weight_base = VX_NULL, cell2output_weight_base = VX_NULL;
    vx_uint8_ptr cell_state_in_base = VX_NULL, output_state_out_base = VX_NULL, cell_state_out_base = VX_NULL, cell_state_base = VX_NULL, output_base = VX_NULL;
    vx_uint8_ptr projection_weight_base = VX_NULL, projection_bias_base = VX_NULL;

    vx_int8 scratch_fpos = 0;
    vx_type_e scratch_format = VX_TYPE_FLOAT32;
    vx_uint8_ptr input_ptr = VX_NULL;

    vx_type_e input_format = (vx_type_e)TENSOR_DATA_TYPE(input);
    vx_type_e output_format = (vx_type_e)TENSOR_DATA_TYPE(input);
    vx_int32 item_size = vxnneGetTypeSize(output_format);
    vx_int32 scratch_item_size = vxnneGetTypeSize(scratch_format);

    vx_uint32 f = 0;
    gctSTRING format = VX_NULL;
    gctSTRING formatNameTable[] = { "int16", "f16", "f32", "qauant8" };
    vx_enum formatTable[] = { VX_TYPE_INT16, VX_TYPE_FLOAT16, VX_TYPE_FLOAT32, VX_TYPE_UINT8 };

    gcoOS_GetEnv(gcvNULL, "LSTM_STAGE_FORMAT", &format);
    if (format != NULL)
    {
        for (f = 0; f < (vx_uint32)gcmCOUNTOF(formatNameTable); f++)
        {
            if (gcoOS_StrStr(format, formatNameTable[f], VX_NULL))
            {
                scratch_format = formatTable[f];
                break;
            }
        }
    }
    scratch_fpos = TENSOR_POS(input);
    scratch_format = (vx_type_e)TENSOR_DATA_TYPE(input);
    scratch_item_size = vxnneGetTypeSize(scratch_format);
    gcoOS_Allocate(VX_NULL, cell * batch * sizeof(vx_float32) * 5, (vx_ptr_ptr)&scatch_ptr);
    vxoTensor_GetTensorViewMemory(input, (vx_ptr_ptr)&input_ptr, VX_NULL);


    vxnneLSTM_VectorBatchVectorAssign(input_format, scratch_format, input_ptr,
        cell * 4, TENSOR_POS(input), batch, (vx_uint8_ptr)scatch_ptr, scratch_fpos);

    vxoTensor_GetTensorViewMemory(cell2input_weight, (vx_ptr_ptr)&cell2input_weight_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(cell2forget_weight, (vx_ptr_ptr)&cell2forget_weight_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(cell2output_weight, (vx_ptr_ptr)&cell2output_weight_base, VX_NULL);

    vxoTensor_GetTensorViewMemory(cell_state_in, (vx_ptr_ptr)&cell_state_in_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(output_state_out, (vx_ptr_ptr)&output_state_out_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(cell_state_out, (vx_ptr_ptr)&cell_state_out_base, VX_NULL);
    if (output)vxoTensor_GetTensorViewMemory(output, (vx_ptr_ptr)&output_base, VX_NULL);

    vxoTensor_GetTensorViewMemory(projection_weight, (vx_ptr_ptr)&projection_weight_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(projection_bias, (vx_ptr_ptr)&projection_bias_base, VX_NULL);

    if (!enable_cifg && enable_peephole && (cell2input_weight == VX_NULL))
    {
        enable_cifg = vx_true_e;
    }

    /* Index the scratch buffers pointers to the global scratch buffer. */
    if (enable_cifg)
    {
        scatch_forget_ptr = scatch_ptr + cell * batch * item_size;
        scatch_cell_ptr = scatch_forget_ptr + cell * batch * item_size;
        scatch_output_ptr = scatch_cell_ptr + cell * batch * item_size;
    }
    else
    {
        scatch_input_ptr = scatch_ptr;
        scatch_forget_ptr = scatch_input_ptr + cell * batch * item_size;
        scatch_cell_ptr = scatch_forget_ptr + cell * batch * item_size;
        scatch_output_ptr = scatch_cell_ptr + cell * batch * item_size;
    }

    /* For each batch and cell: update input gate. */
    if (!enable_cifg)
    {
        if (enable_peephole)
            vxnneLSTM_VectorBatchVectorCwiseProductAccumulate((vx_type_e)TENSOR_DATA_TYPE(cell2input_weight), (vx_type_e)TENSOR_DATA_TYPE(cell_state_in), scratch_format, cell2input_weight_base,
                cell, TENSOR_POS(cell2input_weight), cell_state_in_base, batch, TENSOR_POS(cell_state_in), (vx_uint8_ptr)scatch_input_ptr, scratch_fpos);

        /*i_t = sigmoid(W_x_i * input + W_h_i * h_t-1 + Bias_i)*/
        vxnneLSTM_SigmoidToVector(scratch_format, scratch_format, (vx_uint8_ptr)scatch_input_ptr, cell * batch, scratch_fpos, (vx_uint8_ptr)scatch_input_ptr, scratch_fpos);
    }

    /* For each batch and cell: update forget gate. */
    if (enable_peephole)
        vxnneLSTM_VectorBatchVectorCwiseProductAccumulate((vx_type_e)TENSOR_DATA_TYPE(cell2forget_weight), (vx_type_e)TENSOR_DATA_TYPE(cell_state_in), scratch_format, cell2forget_weight_base,
            cell, TENSOR_POS(cell2forget_weight), cell_state_in_base, batch, TENSOR_POS(cell_state_in), (vx_uint8_ptr)scatch_forget_ptr, scratch_fpos);

    if (forget_bias != 0)
        vxnneLSTM_BiasVector(scratch_format, scratch_format, (vx_uint8_ptr)scatch_forget_ptr, cell * batch, forget_bias, scratch_fpos, (vx_uint8_ptr)scatch_forget_ptr, scratch_fpos);

    /*f_t = sigmoid(W_x_f * input + W_h_f * h_t-1 + Bias_f)*/
    vxnneLSTM_SigmoidToVector(scratch_format, scratch_format, (vx_uint8_ptr)scatch_forget_ptr, cell * batch, scratch_fpos, (vx_uint8_ptr)scatch_forget_ptr, scratch_fpos);

    gcoOS_Allocate(VX_NULL, cell * batch * scratch_item_size, (vx_ptr_ptr)&cell_state_base);

    /* For each batch and cell: update the cell. cell_state_out = f_t * c_t-1 = (sigmoid(W_x_f * input + W_h_f * h_t-1 + Bias_f) * c_t-1*/
    vxnneLSTM_VectorVectorCwiseProduct(scratch_format, (vx_type_e)TENSOR_DATA_TYPE(cell_state_in), scratch_format, (vx_uint8_ptr)scatch_forget_ptr, cell_state_in_base,
        cell * batch, scratch_fpos, TENSOR_POS(cell_state_in), cell_state_base, scratch_fpos);

    /*c_t = tanh(W_x_c * input + W_h_c * h_t-1 + Bias_c)*/
    vxnneLSTM_ActivationToVector(scratch_format, scratch_format, (vx_uint8_ptr)scatch_cell_ptr, cell * batch, activation, scratch_fpos, (vx_uint8_ptr)scatch_cell_ptr, scratch_fpos);


    if (enable_cifg)
    {
        vxnneLSTM_Sub1Vector(scratch_format, scratch_format, (vx_uint8_ptr)scatch_forget_ptr, cell * batch, scratch_fpos, (vx_uint8_ptr)scatch_forget_ptr, scratch_fpos);
        vxnneLSTM_VectorVectorCwiseProductAccumulate(scratch_format, scratch_format, scratch_format, (vx_uint8_ptr)scatch_cell_ptr, (vx_uint8_ptr)scatch_forget_ptr, scratch_fpos, scratch_fpos,
            cell * batch, cell_state_base, scratch_fpos);
    }
    else
        /* cell_state_out = f_t * c_t-1 + c_t * i_t = (sigmoid(W_x_f * input + W_h_f * h_t-1 + Bias_f) * c_t-1 + sigmoid(W_x_i * input + W_h_i * h_t-1 + Bias_i) * tanh(W_x_c * input + W_h_c * h_t-1 + Bias_c) */
        vxnneLSTM_VectorVectorCwiseProductAccumulate(scratch_format, scratch_format, scratch_format, (vx_uint8_ptr)scatch_cell_ptr, (vx_uint8_ptr)scatch_input_ptr, scratch_fpos, scratch_fpos,
            cell * batch, cell_state_base, scratch_fpos);

    if (cell_clip > 0.0f)
        vxnneLSTM_ClipVector(scratch_format, scratch_format, cell_state_base,
            cell * batch, scratch_fpos, cell_clip, cell_state_base, scratch_fpos);

    /* For each batch and cell: update the output gate. */
    if (enable_peephole)
        vxnneLSTM_VectorBatchVectorCwiseProductAccumulate((vx_type_e)TENSOR_DATA_TYPE(cell2forget_weight), scratch_format, scratch_format, cell2output_weight_base,
            cell, TENSOR_POS(cell2output_weight), cell_state_base, batch, scratch_fpos, (vx_uint8_ptr)scatch_output_ptr, scratch_fpos);

    /*o_t = sigmoid(W_x_o * input + W_h_o * h_t-1 + Bias_o)*/
    vxnneLSTM_SigmoidToVector(scratch_format, scratch_format, (vx_uint8_ptr)scatch_output_ptr, cell * batch, scratch_fpos, (vx_uint8_ptr)scatch_output_ptr, scratch_fpos);

    /*c_t = tanh(cell_state_out)*/
    vxnneLSTM_ActivationToVector(scratch_format, scratch_format, cell_state_base,
        cell * batch, activation, scratch_fpos, (vx_uint8_ptr)scatch_cell_ptr, scratch_fpos);

    vxnneLSTM_VectorBatchVectorAssign(scratch_format, (vx_type_e)TENSOR_DATA_TYPE(cell_state_out), cell_state_base,
        cell, scratch_fpos, batch, (vx_uint8_ptr)cell_state_out_base, TENSOR_POS(cell_state_out));

    /*o_t = c_t * o_t = tanh(cell_state_out) * sigmoid(W_x_o * input + W_h_o * h_t-1 + Bias_o)*/
    vxnneLSTM_VectorVectorCwiseProduct(scratch_format, scratch_format, scratch_format, (vx_uint8_ptr)scatch_output_ptr, (vx_uint8_ptr)scatch_cell_ptr, cell * batch, scratch_fpos, scratch_fpos, (vx_uint8_ptr)scatch_output_ptr, scratch_fpos);

    /* For each batch: update the projection and output_state_out.*/
    if (enable_proj_weight)
    {
        if (enable_proj_bias)
            vxnneLSTM_VectorBatchVectorAssign((vx_type_e)TENSOR_DATA_TYPE(projection_bias), input_format, projection_bias_base,
                output_count, TENSOR_POS(projection_bias), batch, projection_bias_base, TENSOR_POS(projection_bias));
        else
            memset((vx_float32_ptr)output_base, 0, output_count * batch * item_size);

        vxnneLSTM_MatrixBatchVectorMultiplyAccumulate((vx_type_e)TENSOR_DATA_TYPE(projection_weight), scratch_format, (vx_type_e)TENSOR_DATA_TYPE(output), projection_weight_base,
            output_count, cell, TENSOR_POS(projection_weight), (vx_uint8_ptr)scatch_output_ptr, batch, scratch_fpos, output_base, TENSOR_POS(output), 1);

        if (proj_clip > 0.0f)
            vxnneLSTM_ClipVector((vx_type_e)TENSOR_DATA_TYPE(output), (vx_type_e)TENSOR_DATA_TYPE(output), output_base,
                output_count * batch, TENSOR_POS(output), proj_clip, output_base, TENSOR_POS(output));
    }
    else
        vxnneLSTM_VectorBatchVectorAssign(scratch_format, (vx_type_e)TENSOR_DATA_TYPE(output_state_out), scatch_output_ptr,
            output_count, scratch_fpos, batch, (vx_uint8_ptr)output_state_out_base, TENSOR_POS(output_state_out));


    if (output_base)
        vxnneLSTM_VectorBatchVectorAssign(scratch_format, (vx_type_e)TENSOR_DATA_TYPE(output), scatch_output_ptr,
            output_count, scratch_fpos, batch, (vx_uint8_ptr)output_base, TENSOR_POS(output));
    if (scatch_ptr != VX_NULL)
        gcoOS_Free(VX_NULL, scatch_ptr);

    if (cell_state_base)
    {
        gcoOS_Free(gcvNULL, cell_state_base);
    }

    return VX_SUCCESS;
}

vx_status vxoNN_LSTMNNLayer_DeInitializer(vxnne_layer_s* layer)
{
    if (((vxnne_lstm_layer)layer)->operations2)
        gcoOS_Free(VX_NULL, ((vxnne_lstm_layer)layer)->operations2);

    return VX_SUCCESS;
}

vx_int32 vxnneExecuteSW_GetInputSize(vx_tensor input)
{
    vx_int32 size = TENSOR_SIZE_INDEX(input, 0);

    if (TENSOR_DIM_NUM(input) > 3)
    {
        vx_uint32 i = 0;

        for (i = 1; i < TENSOR_DIM_NUM(input) - 1; i++)
            size *= TENSOR_SIZE_INDEX(input, i);
    }

    return size;
}

vx_status vxnneGetAlignStrides(vx_tensor tensor, vx_uint32_ptr strides, vx_bool align64)
{
    vx_uint32_ptr sizes = TENSOR_SIZES(tensor);
    vx_uint32_ptr strides_o = TENSOR_STRIDES(tensor);
    vx_uint32 dims_count = TENSOR_DIM_NUM(tensor);

    gcmASSERT(dims_count <= 6);

    if (align64)
        memcpy(strides, strides_o, sizeof(vx_uint32) * dims_count);
    else
    {
        vx_uint32 i = 0;
        strides[0] = strides_o[0];
        strides[1] = gcmALIGN_NP2(strides_o[0], 64);

        for (i = 1; i < 6; i++)
        {
            if (i <= dims_count)
            {
                strides[i] = strides[i - 1] * sizes[i - 1];

                if (sizes[i - 1] > 1)
                    strides[i] = gcmALIGN_NP2(strides[i], 64);
            }
            else
                strides[i] = strides[i - 1];
        }
    }

    return VX_SUCCESS;
}

vx_status vxnneExecuteLSTM_NN_TP_LAYER(vx_node node,
    vx_tensor input,
    vx_tensor input2input_weight, vx_tensor input2forget_weight, vx_tensor input2cell_weight, vx_tensor input2output_weight,
    vx_tensor recurrent2input_weight, vx_tensor recurrent2forget_weight, vx_tensor recurrent2cell_weight, vx_tensor recurrent2output_weight,
    vx_tensor cell2input_weight, vx_tensor cell2forget_weight, vx_tensor cell2output_weight,
    vx_tensor input_gate_bias, vx_tensor forget_gate_bias, vx_tensor cell_bias, vx_tensor output_gate_bias,
    vx_tensor projection_weight, vx_tensor projection_bias,
    vx_tensor output_state_in, vx_tensor cell_state_in,
    vx_tensor activation, vx_tensor forget_bias, vx_tensor cell_clip, vx_tensor proj_clip,
    vx_tensor output_state_out, vx_tensor cell_state_out, vx_tensor output,
    vx_int32 time_steps
)
{
    vx_status  status = VX_SUCCESS;
    vx_uint32 batchCount = 1;
    vx_context context = vxGetContext((vx_reference)input);
    vx_bool sw_fc = vx_false_e, sw_resuffle_input = ((vx_false_e == vxoContext_IsFeatureAvailable(vxGetContext((vx_reference)node), VX_NN_FEATURE_TP)) ? vx_true_e : vx_false_e), sw_state_out = vx_true_e;
    vx_int32 count = 0;
    vx_uint32 batch = TENSOR_SIZE_INDEX(input, TENSOR_DIM_NUM(input) - 2), cell = TENSOR_SIZE_INDEX(input2forget_weight, 1);
    vx_uint32 input_size = vxnneExecuteSW_GetInputSize(input), output_size = TENSOR_SIZE_INDEX(output, 0);
    vx_uint32 sizes[][4] = {
        { 1, 1, input_size + output_size, batch }, /*inputs*/
        { 1, 1, input_size + output_size, 4 * cell }, /*weights*/
        { output_size, 4 }, /*bias*/
        { 1, 1, 4 * output_size, batch }, /*outputs*/
    };

    vx_tensor* input_conv = VX_NULL;
    vx_tensor* output_conv = VX_NULL;
    vx_tensor weights_conv = VX_NULL;
    vx_tensor biases_conv = VX_NULL;
    vx_weights_biases_parameter weights_biases = { 0 };
    vx_tensor_create_params_t tensor_create_params;

    vx_uint32 kzgroup = 1;
    vx_int32 i = 0;

    vx_tensor_view* views = VX_NULL;
    vx_tensor* input_view_tensors = VX_NULL;

    vx_uint32 start[3] = { 0 };
    vx_uint32 end[3] = { input_size, batch, time_steps };

    vxnne_lstm_layer_s* lstmlayer = VX_NULL;

    vx_enum staging_format = TENSOR_DATA_TYPE(output);
    vx_uint8 staging_pos = TENSOR_POS(output);
    vx_op_param_s conv = { 0 };

    if ((TENSOR_DATA_TYPE(output) == VX_TYPE_UINT8 && TENSOR_QUANT_TYPE(output) == VX_QUANT_AFFINE_SCALE) ||
        (TENSOR_DATA_TYPE(output) == VX_TYPE_INT16 && TENSOR_QUANT_TYPE(output) == VX_QUANT_DYNAMIC_FIXED_POINT))
    {

        staging_format = VX_TYPE_FLOAT16;//TENSOR_DATA_TYPE(output);//
        staging_pos = TENSOR_POS(forget_gate_bias) - 16;
    }

    {
        vx_uint32 f = 0;
        gctSTRING format = VX_NULL;
        gctSTRING formatNameTable[] = { "int16", "f16", "f32", "qauant8" };
        vx_enum formatTable[] = { VX_TYPE_INT16, VX_TYPE_FLOAT16, VX_TYPE_FLOAT32, VX_TYPE_UINT8 };

        gcoOS_GetEnv(gcvNULL, "LSTM_COUV_OUT_FORMAT", &format);
        if (format != NULL)
        {
            for (f = 0; f < (vx_uint32)gcmCOUNTOF(formatNameTable); f++)
            {
                if (gcoOS_StrStr(format, formatNameTable[f], VX_NULL))
                {
                    staging_format = formatTable[f];
                    break;
                }
            }
        }

        if (staging_format == VX_TYPE_FLOAT16 || staging_format == VX_TYPE_FLOAT32)
            staging_pos = 0;
        else if (staging_format == VX_TYPE_INT16)
            staging_pos = TENSOR_POS(forget_gate_bias) - 16;
        else
            staging_pos = TENSOR_POS(output);
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_lstm_layer_s), (gctPOINTER*)&lstmlayer);
    if (!lstmlayer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(lstmlayer, sizeof(vxnne_lstm_layer_s));

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_lstm_unit_s) * time_steps * 15, (gctPOINTER*)&lstmlayer->operations2);

    vxnneLayer_Initialize(&lstmlayer->base,
        "_LSTM_Layer",
        node,
        time_steps * 15,
        lstmlayer->operations2,
        vxoNN_LSTMNNLayer_DeInitializer);

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_lstm_unit_s) * time_steps, (gctPOINTER*)&lstmlayer->units);
    if (!lstmlayer->units)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }
    else
        gcoOS_ZeroMemory(lstmlayer->units, sizeof(vxnne_lstm_unit_s) * time_steps);


    if (time_steps > 1)
    {
        gcoOS_Allocate(gcvNULL, time_steps * sizeof(vx_tensor_view), (gctPOINTER*)&views);
        gcoOS_Allocate(gcvNULL, time_steps * sizeof(vx_tensor_view), (gctPOINTER*)&input_view_tensors);
    }

    gcoOS_Allocate(gcvNULL, time_steps * sizeof(vx_tensor), (gctPOINTER*)&input_conv);
    gcoOS_Allocate(gcvNULL, time_steps * sizeof(vx_tensor), (gctPOINTER*)&output_conv);

    gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
    tensor_create_params.num_of_dims = 4;
    tensor_create_params.sizes = sizes[1];
    tensor_create_params.data_format = TENSOR_DATA_TYPE(input2forget_weight);
    tensor_create_params.quant_format = TENSOR_QUANT_TYPE(input2forget_weight);
    if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
    {
        tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(input2forget_weight);
    }
    else
    {
        tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(input2forget_weight);
        tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(input2forget_weight);
    }

    weights_conv = vxoTensor_CreateTensor(context, node->graph, &tensor_create_params, vx_false_e);

    gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
    tensor_create_params.num_of_dims = 2;
    tensor_create_params.sizes = sizes[2];
    tensor_create_params.data_format = TENSOR_DATA_TYPE(forget_gate_bias);
    tensor_create_params.quant_format = TENSOR_QUANT_TYPE(forget_gate_bias);
    if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
    {
        tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(forget_gate_bias);
    }
    else
    {
        tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(forget_gate_bias);
        tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(forget_gate_bias);
    }

    biases_conv = vxoTensor_CreateTensor(context, node->graph, &tensor_create_params, vx_false_e);

    vxoNNSWLSTMConv_ReshuffleWeightBias(input2input_weight, input2forget_weight, input2cell_weight, input2output_weight,
        recurrent2input_weight, recurrent2forget_weight, recurrent2cell_weight, recurrent2output_weight,
        VX_NULL, VX_NULL, VX_NULL, input_gate_bias, forget_gate_bias, cell_bias, output_gate_bias,
        weights_conv, biases_conv);

    for (i = 0; i < time_steps; i++)
    {
        vxnne_lstm_unit  lstmUnitNode = &lstmlayer->units[i];

        if (time_steps > 1)
        {
            start[2] = i;
            end[2] = start[2] + 1;

            views[i] = vxCreateTensorView(context, start, end, 3);
            input_view_tensors[i] = vxoTensor_CreateTensorFromView(input, views[i]);

            lstmlayer->base.temp_tensors[lstmlayer->base.num_temp_tensors++] = input_view_tensors[i];

            vxReleaseTensorView(&views[i]);
        }

        input_conv[i] = vxoTensor_CreateVirtualTensor(node->graph, 4, sizes[0], TENSOR_DATA_TYPE(input), TENSOR_POS(input));
        output_conv[i] = vxoTensor_CreateVirtualTensor(node->graph, 4, sizes[3], staging_format, staging_pos);

        lstmlayer->base.temp_tensors[lstmlayer->base.num_temp_tensors++] = input_conv[i];
        lstmlayer->base.temp_tensors[lstmlayer->base.num_temp_tensors++] = output_conv[i];

        if (sw_resuffle_input)
        {

            /* Initialize covolution operation */
            vxnneOperation_Initialize(&lstmUnitNode->lstm_resuffle_input_operation.base,
                &lstmlayer->base,
                VXNNE_OPERATION_TARGET_SW,
                VXNNE_OPERATOR_LSTM_RESHUFFLE_INPUT,
                vxoNNSWLSTM_ReshuffleInput,
                NULL,
                batchCount,
                0);

            lstmUnitNode->lstm_resuffle_input_operation.input = (time_steps > 1) ? input_view_tensors[i] : input;
            lstmUnitNode->lstm_resuffle_input_operation.output_state_in = output_state_in;
            lstmUnitNode->lstm_resuffle_input_operation.cell_state_in = cell_state_in;
            lstmUnitNode->lstm_resuffle_input_operation.reshuffled_input = input_conv[i];

            vxnneLayer_SetOperation(
                &lstmlayer->base,
                &lstmUnitNode->lstm_resuffle_input_operation.base,
                count++);

            vxnneOperation_AddReference(&lstmUnitNode->lstm_resuffle_input_operation.base, (vx_reference)((time_steps > 1) ? input_view_tensors[i] : input), VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_resuffle_input_operation.base, (vx_reference)output_state_in, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_resuffle_input_operation.base, (vx_reference)cell_state_in, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_resuffle_input_operation.base, (vx_reference)input_conv[i], VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }
        else if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP))
        {
            /* TP implement: Reshuffle input */
            status = vxnneOperation_Initialize(&lstmUnitNode->lstm_resuffle_input_tp_operation.base,
                &lstmlayer->base,
                VXNNE_OPERATION_TARGET_TP,
                VXNNE_OPERATOR_LSTM_RESHUFFLE_INPUT,
                VX_NULL,
                VX_NULL,
                batchCount,
                0);
            if (status != VX_SUCCESS) goto exit;

            lstmUnitNode->lstm_resuffle_input_tp_operation.input = (time_steps > 1) ? input_view_tensors[i] : input;
            lstmUnitNode->lstm_resuffle_input_tp_operation.output = input_conv[i];

            vxnneLayer_SetOperation(
                &lstmlayer->base,
                &lstmUnitNode->lstm_resuffle_input_tp_operation.base,
                count++);

            vxnneOperation_AddReference(&lstmUnitNode->lstm_resuffle_input_tp_operation.base, (vx_reference)((time_steps > 1) ? input_view_tensors[i] : input), VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_resuffle_input_tp_operation.base, (vx_reference)input_conv[i], VXNNE_OPERATION_REFENRENCE_OUTPUT);

            conv.tpType = TP_LSTM_RESHUFFLE_INPUT;
            conv.other_ref = (vx_reference)output_state_in;
            conv.data_buff = gcvNULL;

            memcpy(&lstmUnitNode->lstm_resuffle_input_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));

            /* Reshuffle output state in */
            status = vxnneOperation_Initialize(&lstmUnitNode->lstm_resuffle_input_tp_operation2.base,
                &lstmlayer->base,
                VXNNE_OPERATION_TARGET_TP,
                VXNNE_OPERATOR_LSTM_RESHUFFLE_INPUT,
                VX_NULL,
                VX_NULL,
                batchCount,
                0);
            if (status != VX_SUCCESS) goto exit;

            lstmUnitNode->lstm_resuffle_input_tp_operation2.input = output_state_in;
            lstmUnitNode->lstm_resuffle_input_tp_operation2.output = input_conv[i];

            vxnneLayer_SetOperation(
                &lstmlayer->base,
                &lstmUnitNode->lstm_resuffle_input_tp_operation2.base,
                count++);

            vxnneOperation_AddReference(&lstmUnitNode->lstm_resuffle_input_tp_operation2.base, (vx_reference)output_state_in, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_resuffle_input_tp_operation2.base, (vx_reference)input_conv[i], VXNNE_OPERATION_REFENRENCE_OUTPUT);

            conv.tpType = TP_LSTM_STATE_OUT;
            conv.other_ref = (vx_reference)((time_steps > 1) ? input_view_tensors[i] : input);
            conv.data_buff = gcvNULL;

            memcpy(&lstmUnitNode->lstm_resuffle_input_tp_operation2.base.parameter, &conv, sizeof(vx_op_param_s));
        }
        else
            vxError("Not Support Operation %s[%d]!", __FUNCTION__, __LINE__);

        if (sw_fc)
        {
            status = vxnneOperation_Initialize(&lstmUnitNode->lstm_sw_operation.base,
                &lstmlayer->base,
                VXNNE_OPERATION_TARGET_SW,
                VXNNE_OPERATOR_FULLYCONNECTED,
                vxnneExecuteSWFullyConnected,
                VX_NULL,
                batch,
                0);

            lstmUnitNode->lstm_sw_operation.inputs = input_conv[i];
            lstmUnitNode->lstm_sw_operation.weights = weights_conv;
            lstmUnitNode->lstm_sw_operation.biases = biases_conv;
            lstmUnitNode->lstm_sw_operation.outputs = output_conv[i];

            vxnneLayer_SetOperation(
                &lstmlayer->base,
                &lstmUnitNode->lstm_sw_operation.base,
                count++);

            vxnneOperation_AddReference(&lstmUnitNode->lstm_sw_operation.base, (vx_reference)input_conv[i], VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_sw_operation.base, (vx_reference)output_conv[i], VXNNE_OPERATION_REFENRENCE_OUTPUT);

            lstmlayer->base.temp_tensors[lstmlayer->base.num_temp_tensors++] = weights_conv;
            lstmlayer->base.temp_tensors[lstmlayer->base.num_temp_tensors++] = biases_conv;
        }
        else if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP))
        {
            /* TP implement */
            vx_tp_value_cmd_s values;
            vx_uint32 zoffset = 0, s = 0;

            INITIALIZE_STRUCT(values);

            if (weights_biases == VX_NULL)
            {
                weights_biases = _createWeightsBiasesParameterFromTensors(
                    context,
                    VX_NN_FULLYCONNECTED_LAYER,
                    input_conv[i]->dims,/*inputs_dims,*/
                    input_conv[i]->dimCount,
                    input_conv[i]->dimCount,
                    0,
                    0,
                    0,
                    0,
                    0,/*pooling_size_x,*/
                    0,/*pooling_size_y,*/
                    0,
                    0,
                    VX_NN_DS_SIZE_ROUNDING_FLOOR,
                    output_conv[i]->dims,/*convolution_outputs_dims,*/
                    output_conv[i]->dims,/*pool_outputs_dims,*/
                    NULL, /*optimizations,*/
                    TENSOR_DATA_TYPE(weights_conv),
                    0,
                    VX_TENSOR_RANK_SN,
                    weights_conv,
                    biases_conv,
                    VX_NULL,
                    vx_false_e,
                    vx_false_e
                );

                vxoTensor_ReleaseTensor(&weights_conv);
                vxoTensor_ReleaseTensor(&biases_conv);
            }
            kzgroup = weights_biases->weights_sizes[2] / weights_biases->slice_array[0].kz_count;

            status = vxnneOperation_Initialize(&lstmUnitNode->lstm_tp_fc_operation.base,
                &lstmlayer->base,
                VXNNE_OPERATION_TARGET_TP,
                VXNNE_OPERATOR_FULLYCONNECTED,
                VX_NULL,
                vxnneOperation_TP_Deinitialize,
                batchCount,
                0);

            if (status != VX_SUCCESS) goto exit;

            memset(&conv, 0, sizeof(vx_op_param_s));
            conv.pad_x_left = 0;
            conv.pad_y_top = 0;
            conv.pad_x_right = 0;
            conv.pad_y_bottom = 0;
            conv.pool_size_x = 0;
            conv.pool_size_y = 0;
            conv.pool_stride = 1;
            conv.enable_relu = vx_false_e;
            conv.conv_rounding_type = 0;
            conv.pad_mode = VX_PAD_CONSTANT;
            conv.pad_const = 0;

            conv.tp_value = (vx_tp_value_cmd_s*)vxAllocateAndZeroMemory(weights_biases->slice_num * sizeof(vx_tp_value_cmd_s));

            if (kzgroup == 1)
            {

                conv.tpType = TP_SINGLE_FC;
                conv.other_ref = (vx_reference)weights_biases;
                conv.data_buff = gcvNULL;

                if (conv.tp_value != VX_NULL)
                {
                    for (s = 0; s < weights_biases->slice_num; s++)
                    {
                        values.u32[0] = kzgroup;
                        values.u32[1] = zoffset;
                        memcpy(&conv.tp_value[s], &values, sizeof(vx_tp_value_cmd_s));
                        zoffset += weights_biases->slice_array[s].z_count;
                    }
                }

                memcpy(&lstmUnitNode->lstm_tp_fc_operation.base.parameter, &conv, sizeof(vx_op_param_s));

                lstmUnitNode->lstm_tp_fc_operation.input = input_conv[i];
                lstmUnitNode->lstm_tp_fc_operation.weights_biases = weights_biases;
                lstmUnitNode->lstm_tp_fc_operation.output = output_conv[i];

                vxnneLayer_SetOperation(
                    &lstmlayer->base,
                    &lstmUnitNode->lstm_tp_fc_operation.base,
                    count++);

                vxnneOperation_AddReference(&lstmUnitNode->lstm_tp_fc_operation.base, (vx_reference)input_conv[i], VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&lstmUnitNode->lstm_tp_fc_operation.base, (vx_reference)output_conv[i], VXNNE_OPERATION_REFENRENCE_OUTPUT);

            }
            else
            {
                vxError("Error, not support!");
            }
        }
        else
        {
            /* NN implement */
            if (weights_biases == VX_NULL)
            {
                weights_biases = _createWeightsBiasesParameterFromTensors(
                    context,
                    VX_NN_CONVOLUTION_LAYER,
                    input_conv[i]->dims,/*inputs_dims,*/
                    input_conv[i]->dimCount,
                    input_conv[i]->dimCount,
                    0,
                    0,
                    0,
                    0,
                    0,/*pooling_size_x,*/
                    0,/*pooling_size_y,*/
                    0,
                    0,
                    VX_NN_DS_SIZE_ROUNDING_FLOOR,
                    output_conv[i]->dims,/*convolution_outputs_dims,*/
                    output_conv[i]->dims,/*pool_outputs_dims,*/
                    NULL, /*optimizations,*/
                    TENSOR_DATA_TYPE(weights_conv),
                    0,
                    VX_TENSOR_RANK_SN,
                    weights_conv,
                    biases_conv,
                    VX_NULL,
                    vx_false_e,
                    vx_false_e
                );

                vxoTensor_ReleaseTensor(&weights_conv);
                vxoTensor_ReleaseTensor(&biases_conv);
            }
            status = vxnneOperation_Initialize(&lstmUnitNode->lstm_nn_operation.base,
                &lstmlayer->base,
                VXNNE_OPERATION_TARGET_NN,
                VXNNE_OPERATOR_CONVOLUTION,
                VX_NULL,
                NULL,
                batchCount,
                NNE_COMMAND_SIZE);

            if (status != VX_SUCCESS) goto exit;

            memset(&conv, 0, sizeof(vx_op_param_s));
            conv.pad_x_left = conv.pad_x_right = conv.pad_y_top = conv.pad_y_bottom = 0;
            conv.pad_mode = VX_PAD_CONSTANT;
            conv.pad_const = 0;
            conv.pool_type = 0;
            conv.pool_size_x = conv.pool_size_y = 0;
            conv.conv_rounding_type = 0;
            conv.enable_relu = vx_false_e;

            lstmUnitNode->lstm_nn_operation.orig_inputs = input_conv[i];
            lstmUnitNode->lstm_nn_operation.inputs = input_conv[i];
            lstmUnitNode->lstm_nn_operation.weights_biases = weights_biases;
            lstmUnitNode->lstm_nn_operation.outputs = output_conv[i];

            vxnneLayer_SetOperation(
                &lstmlayer->base,
                &lstmUnitNode->lstm_nn_operation.base,
                count++);

            vxnneOperation_AddReference(&lstmUnitNode->lstm_nn_operation.base, (vx_reference)input_conv[i], VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_nn_operation.base, (vx_reference)output_conv[i], VXNNE_OPERATION_REFENRENCE_OUTPUT);

            memcpy(&lstmUnitNode->lstm_nn_operation.base.parameter, &conv, sizeof(vx_op_param_s));
        }

        if (sw_state_out)
        {
            /* Initialize state out operation */
            vxnneOperation_Initialize(&lstmUnitNode->lstm_unit_operation.base,
                &lstmlayer->base,
                VXNNE_OPERATION_TARGET_SW,
                VXNNE_OPERATOR_LSTM_STATE_OUT,
                vxoNNSWLSTM_StateOut,
                NULL,
                batchCount,
                0);

            lstmUnitNode->lstm_unit_operation.input = output_conv[i];

            lstmUnitNode->lstm_unit_operation.input2input_weight = input2input_weight;
            lstmUnitNode->lstm_unit_operation.input2forget_weight = input2forget_weight;

            lstmUnitNode->lstm_unit_operation.recurrent2output_weight = recurrent2output_weight;

            lstmUnitNode->lstm_unit_operation.cell2input_weight = cell2input_weight;
            lstmUnitNode->lstm_unit_operation.cell2forget_weight = cell2forget_weight;
            lstmUnitNode->lstm_unit_operation.cell2output_weight = cell2output_weight;

            lstmUnitNode->lstm_unit_operation.input_gate_bias = input_gate_bias;
            lstmUnitNode->lstm_unit_operation.forget_gate_bias = forget_gate_bias;
            lstmUnitNode->lstm_unit_operation.cell_bias = cell_bias;
            lstmUnitNode->lstm_unit_operation.output_gate_bias = output_gate_bias;

            lstmUnitNode->lstm_unit_operation.projection_weight = projection_weight;
            lstmUnitNode->lstm_unit_operation.projection_bias = projection_bias;
            lstmUnitNode->lstm_unit_operation.output_state_in = output_state_in;
            lstmUnitNode->lstm_unit_operation.cell_state_in = cell_state_in;

            lstmUnitNode->lstm_unit_operation.activation = activation;
            lstmUnitNode->lstm_unit_operation.forget_bias = forget_bias;
            lstmUnitNode->lstm_unit_operation.cell_clip = cell_clip;
            lstmUnitNode->lstm_unit_operation.proj_clip = proj_clip;

            lstmUnitNode->lstm_unit_operation.output_state_out = output_state_out;
            lstmUnitNode->lstm_unit_operation.forget_bias = forget_bias;
            lstmUnitNode->lstm_unit_operation.cell_state_out = cell_state_out;
            lstmUnitNode->lstm_unit_operation.output = (i == (time_steps - 1)) ? output : VX_NULL;

            vxnneLayer_SetOperation(
                &lstmlayer->base,
                &lstmUnitNode->lstm_unit_operation.base,
                count++);

            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)output_conv, VXNNE_OPERATION_REFENRENCE_INPUT);

            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)input2cell_weight, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)input2forget_weight, VXNNE_OPERATION_REFENRENCE_INPUT);

            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)recurrent2output_weight, VXNNE_OPERATION_REFENRENCE_INPUT);

            if (cell2input_weight)vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)cell2input_weight, VXNNE_OPERATION_REFENRENCE_INPUT);
            if (cell2forget_weight)vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)cell2forget_weight, VXNNE_OPERATION_REFENRENCE_INPUT);
            if (cell2output_weight)vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)cell2output_weight, VXNNE_OPERATION_REFENRENCE_INPUT);

            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)input_gate_bias, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)forget_gate_bias, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)cell_bias, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)output_gate_bias, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)output_state_in, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)cell_state_in, VXNNE_OPERATION_REFENRENCE_INPUT);

            if (projection_weight)vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)projection_weight, VXNNE_OPERATION_REFENRENCE_INPUT);
            if (projection_bias)vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)projection_bias, VXNNE_OPERATION_REFENRENCE_INPUT);

            if (activation)vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)activation, VXNNE_OPERATION_REFENRENCE_INPUT);
            if (forget_bias)vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)forget_bias, VXNNE_OPERATION_REFENRENCE_INPUT);
            if (cell_clip)vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)cell_clip, VXNNE_OPERATION_REFENRENCE_INPUT);
            if (proj_clip)vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)proj_clip, VXNNE_OPERATION_REFENRENCE_INPUT);

            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)output_state_out, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)cell_state_out, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            if (output)vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }
        else
        {
            /* TODO: shader implement */
        }

    }

    gcoOS_Free(VX_NULL, input_conv);
    gcoOS_Free(VX_NULL, output_conv);

    if (time_steps > 1)
    {
        gcoOS_Free(gcvNULL, views);
        gcoOS_Free(gcvNULL, input_view_tensors);
    }

    node->layer = &lstmlayer->base;

    return VX_SUCCESS;

exit:
    if (lstmlayer)
    {
        if (lstmlayer->units)
        {
            gcoOS_Free(VX_NULL, lstmlayer->units);
        }

        gcoOS_Free(VX_NULL, lstmlayer);
    }

    if (conv.tp_value)
    {
        vxFree(conv.tp_value);
        conv.tp_value = VX_NULL;
    }
    return status;
}

/*
 * Reshape LSTM layer tensors to fit FC operation.
 *
 * The FC tensors are always 2-dimentional with the
 *  layout of {#IFM, #batch}.
 */
VX_PRIVATE_API vx_status _ReshapeTensor4FC(
    vx_tensor input,
    vx_uint32 batch_pos,
    vx_tensor *reshaped
    )
{
    vx_status status = VX_SUCCESS;

    vx_uint32 dim_num = TENSOR_VIEW_DIM_NUM(input);
    vx_uint32 size = 1, batch_count = 1;
    vx_uint32 sizes[2] = {1, 1};
    vx_tensor output = VX_NULL;
    vx_uint32 i;

    if (!reshaped)
    {
        vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
    }

    for (i = 0; i < batch_pos; i++)
    {
        size *= TENSOR_VIEW_SIZE_INDEX(input, i);
    }
    sizes[0] = size;

    for (i = batch_pos; i < dim_num; i++)
    {
        batch_count *= TENSOR_VIEW_SIZE_INDEX(input, i);
    }
    sizes[1] = batch_count;

    output = vxoTensor_ReshapeTensor(input, (vx_int32*)sizes, 2);

    *reshaped = output;

OnError:
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_LSTMUnit_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status  status = VX_SUCCESS;

    vx_int32  index = 0;
    vx_tensor input = (vx_tensor)parameters[index++];

    vx_tensor input2input_weight = (vx_tensor)parameters[index++];
    vx_tensor input2forget_weight = (vx_tensor)parameters[index++];
    vx_tensor input2cell_weight = (vx_tensor)parameters[index++];
    vx_tensor input2output_weight = (vx_tensor)parameters[index++];

    vx_tensor recurrent2input_weight = (vx_tensor)parameters[index++];
    vx_tensor recurrent2forget_weight = (vx_tensor)parameters[index++];
    vx_tensor recurrent2cell_weight = (vx_tensor)parameters[index++];
    vx_tensor recurrent2output_weight = (vx_tensor)parameters[index++];

    vx_tensor cell2input_weight = (vx_tensor)parameters[index++];
    vx_tensor cell2forget_weight = (vx_tensor)parameters[index++];
    vx_tensor cell2output_weight = (vx_tensor)parameters[index++];

    vx_tensor layernorm2input_weight = (vx_tensor)parameters[index++];
    vx_tensor layernorm2forget_weight = (vx_tensor)parameters[index++];
    vx_tensor layernorm2cell_weight = (vx_tensor)parameters[index++];
    vx_tensor layernorm2output_weight = (vx_tensor)parameters[index++];

    vx_tensor input_gate_bias = (vx_tensor)parameters[index++];
    vx_tensor forget_gate_bias = (vx_tensor)parameters[index++];
    vx_tensor cell_bias = (vx_tensor)parameters[index++];
    vx_tensor output_gate_bias = (vx_tensor)parameters[index++];

    vx_tensor projection_weight = (vx_tensor)parameters[index++];
    vx_tensor projection_bias = (vx_tensor)parameters[index++];
    vx_tensor output_state_in = (vx_tensor)parameters[index++];
    vx_tensor cell_state_in = (vx_tensor)parameters[index++];

    vx_tensor activation = (vx_tensor)parameters[index++];
    vx_tensor forget_bias = (vx_tensor)parameters[index++];
    vx_tensor cell_clip = (vx_tensor)parameters[index++];
    vx_tensor proj_clip = (vx_tensor)parameters[index++];

    vx_tensor scratch = (vx_tensor)parameters[index++];
    vx_tensor output_state_out = (vx_tensor)parameters[index++];
    vx_tensor cell_state_out = (vx_tensor)parameters[index++];
    vx_tensor output = (vx_tensor)parameters[index++];

    vx_uint32 batchCount = 1;//TENSOR_SIZE_INDEX(input, 1);
    vx_bool   nne_support = vx_false_e;
    vx_bool   shExe_flag = vx_false_e;
    vx_bool   enable_activation = vx_false_e;
    vx_bool   enable_cifg = (input2input_weight == VX_NULL) ? vx_true_e : vx_false_e;
    vx_bool   enable_peephole = (cell2output_weight != VX_NULL) ? vx_true_e : vx_false_e;
    vx_bool   enable_projection = (projection_weight != VX_NULL) ? vx_true_e : vx_false_e;
    //     vx_bool   enable_proj_bias        = (projection_bias != VX_NULL)    ? vx_true_e : vx_false_e;
    vx_enum   inputFormat = TENSOR_DATA_TYPE(input);
    vx_enum   w_xFormat = TENSOR_DATA_TYPE(input2forget_weight);
    vx_enum   outputFormat = TENSOR_DATA_TYPE(output);
    vx_enum   actType = 0;
    vx_float32 cellClipValue = (vx_float32)((cell_clip == NULL) ? 0.0f : *((vx_float32*)TENSOR_LOGICAL_ADDR(cell_clip)));
    vx_float32 projClipValue = (vx_float32)((proj_clip == NULL) ? 0.0f : *((vx_float32*)TENSOR_LOGICAL_ADDR(proj_clip)));
    vx_bool enable_layernorm = ((layernorm2input_weight != VX_NULL) && (layernorm2forget_weight != VX_NULL) && (layernorm2cell_weight != VX_NULL) && (layernorm2output_weight != VX_NULL)) ? vx_true_e : vx_false_e;
    vxnne_lstm_unit  lstmUnitNode = VX_NULL;

    /* make compiler happy */
    enable_layernorm = enable_layernorm;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    enable_activation = (vx_bool)((CHECK_LIFETIME_IS_STATIC(activation) && *((vx_enum*)TENSOR_LOGICAL_ADDR(activation)) == lstm_activation_tanh) || CHECK_LIFETIME_IS_STATIC(activation) == vx_false_e);

    if (!enable_cifg && enable_peephole && (cell2input_weight == VX_NULL))
    {
        enable_cifg = vx_true_e;
    }

    nne_support = (CHECK_LIFETIME_IS_STATIC(input2forget_weight) &&
        CHECK_LIFETIME_IS_STATIC(input2cell_weight) &&
        CHECK_LIFETIME_IS_STATIC(input2output_weight) &&
        CHECK_LIFETIME_IS_STATIC(recurrent2forget_weight) &&
        CHECK_LIFETIME_IS_STATIC(recurrent2cell_weight) &&
        CHECK_LIFETIME_IS_STATIC(recurrent2output_weight) &&
        CHECK_LIFETIME_IS_STATIC(forget_gate_bias) &&
        CHECK_LIFETIME_IS_STATIC(cell_bias) &&
        CHECK_LIFETIME_IS_STATIC(output_gate_bias)) ? vx_true_e : vx_false_e;

    if (node->base.context->evisNoInst.supportEVIS)
    {
        shExe_flag = (vx_bool)(enable_activation &&
            inputFormat == VX_TYPE_FLOAT16 &&
            w_xFormat == VX_TYPE_FLOAT16 &&
            outputFormat == VX_TYPE_FLOAT16);

        if (nne_support && (TENSOR_DATA_TYPE(input) == VX_TYPE_FLOAT16) && (node->base.context->nnConfig.fixedFeature.nnCoreCountFloat16 == 0))
        {
            nne_support = vx_false_e;
        }
    }
    else
    {
        switch (*((vx_enum*)TENSOR_LOGICAL_ADDR(activation)))
        {
        case 0:
            actType = VX_NN_ACTIVATION_NONE;
            break;
        default:
        case 4:
            actType = VX_NN_ACTIVATION_HYPERBOLIC_TAN;
            break;
        case 1:
            actType = VX_NN_ACTIVATION_RELU;
            break;
        case 3:
            actType = VX_NN_ACTIVATION_RELU6;
            break;
        case 6:
            actType = VX_NN_ACTIVATION_LOGISTIC;
            break;
        }

        shExe_flag = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && w_xFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) ||
            (inputFormat == VX_TYPE_FLOAT32 && w_xFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32));

        if (nne_support && (TENSOR_DATA_TYPE(input) == VX_TYPE_FLOAT16 || TENSOR_DATA_TYPE(input) == VX_TYPE_FLOAT32) && (node->base.context->nnConfig.fixedFeature.nnCoreCountFloat16 == 0))
        {
            nne_support = vx_false_e;
        }
    }



    if (nne_support)
    {
        status = vxnneExecuteLSTM_NN_TP_LAYER(node,
            input,
            input2input_weight, input2forget_weight, input2cell_weight, input2output_weight,
            recurrent2input_weight, recurrent2forget_weight, recurrent2cell_weight, recurrent2output_weight,
            cell2input_weight, cell2forget_weight, cell2output_weight,
            input_gate_bias, forget_gate_bias, cell_bias, output_gate_bias,
            projection_weight, projection_bias,
            output_state_in, cell_state_in,
            activation, forget_bias, cell_clip, proj_clip,
            output_state_out, cell_state_out, output,
            1);
    }
    else
    {
        gcoOS_Allocate(gcvNULL, sizeof(vxnne_lstm_unit_s), (gctPOINTER*)&lstmUnitNode);
        if (!lstmUnitNode)
        {
            status = VX_ERROR_NO_MEMORY;
            vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            goto exit;
        }

        gcoOS_ZeroMemory(lstmUnitNode, sizeof(vxnne_lstm_unit_s));

        vxnneLayer_Initialize(&lstmUnitNode->base,
            "_LSTM_Unit",
            node,
            vxmOPERATION_COUNT(lstmUnitNode),
            lstmUnitNode->operations,
            VX_NULL);

        if (shExe_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
        {
            vxnne_shader_executable shaderExecutable;
            vx_uint32 tmpTensorIndex = 0;
            vx_uint32 ts_cp_op_idx = 0;
            vx_tensor w_x = NULL;
            vx_tensor w_h = NULL;
            vx_tensor bias = NULL;
            vx_tensor fc_output = NULL;
            vx_tensor project_out = NULL;
            vx_tensor tensorArray[4] = { NULL };
            vx_uint32 sizes[] = { 1,1,1,1 };
            vx_uint32 dims = TENSOR_DIM_NUM(input2forget_weight);
            vx_uint32 inputCount = 0;
            vx_uint32 numofweights = enable_cifg ? 3 : 4;
            vx_uint32 idx = 0;
            vx_tensor_create_params_t tensor_create_params;
            vx_bool   isDynamicInput = vx_false_e;
            vx_uint32 opIdx = 0;

            /* Concat input weights. */
            sizes[0] = TENSOR_VIEW_SIZE_INDEX(input2forget_weight, 0);
            sizes[1] = TENSOR_VIEW_SIZE_INDEX(input2forget_weight, 1) * numofweights;
            dims = 2;
            idx = 0;

            gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
            tensor_create_params.num_of_dims = dims;
            tensor_create_params.sizes = sizes;
            tensor_create_params.data_format = TENSOR_DATA_TYPE(input2forget_weight);
            tensor_create_params.quant_format = TENSOR_QUANT_TYPE(input2forget_weight);
            if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
            {
                tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(input2forget_weight);
            }
            else
            {
                tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(input2forget_weight);
                tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(input2forget_weight);
            }

            if (enable_cifg == vx_false_e)
                tensorArray[idx++] = input2input_weight;
            tensorArray[idx++] = input2forget_weight;
            tensorArray[idx++] = input2cell_weight;
            tensorArray[idx++] = input2output_weight;
            w_x = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_false_e);
            if (vxoTensor_AllocateMemory(w_x) != VX_SUCCESS)
            {
                vxError("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
                status = VX_ERROR_NO_MEMORY;
                goto exit;
            }
            lstmUnitNode->base.temp_tensors[tmpTensorIndex++] = w_x;

            isDynamicInput = !CHECK_LIFETIME_IS_STATIC(input2forget_weight) ||
                !CHECK_LIFETIME_IS_STATIC(input2cell_weight) ||
                !CHECK_LIFETIME_IS_STATIC(input2output_weight);

            if (isDynamicInput)
            {
                vxnne_shader_executable shaderExecutable;
                vx_tensor       subtensor[4] = { NULL };
                vx_tensor_view  tensor_view[4] = { NULL };
                vx_uint32       start[4] = { 0, 0, 0, 0 };
                vx_uint32       end[4] = { sizes[0], 0, 0, 0 };
                vx_uint32       weight_height = TENSOR_VIEW_SIZE_INDEX(input2forget_weight, 1);
                vx_uint32       i;

                for (i = 0; i < idx; i++)
                {
                    start[1] = end[1];
                    end[1] += weight_height;

                    tensor_view[i] = vxCreateTensorView(node->base.context, start, end, (vx_uint8)dims);
                    if (tensor_view[i] == NULL)
                    {
                        vxError("vxCreateTensorView failure! at line %d\n", __LINE__);
                        goto exit;
                    }

                    subtensor[i] = vxoTensor_CreateTensorFromView(w_x, tensor_view[i]);
                    lstmUnitNode->base.temp_tensors[tmpTensorIndex++] = subtensor[i];

                    if (node->base.context->evisNoInst.supportEVIS)
                    {
                        shaderExecutable = vxnneTensorCopyShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_COPY, &node->kernelAttributes.borderMode, tensorArray[i], subtensor[i]);
                    }
                    else
                    {
                        shaderExecutable = vxnneGPUTensorCopyShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_COPY, &node->kernelAttributes.borderMode, tensorArray[i], subtensor[i]);
                    }

                    if (!shaderExecutable)
                    {
                        status = VX_FAILURE;
                        goto exit;
                    }

                    status = vxnneShaderOperation_Initialize(&lstmUnitNode->ts_cp_sh_operation[ts_cp_op_idx],
                        &lstmUnitNode->base,
                        VXNNE_OPERATOR_TENSOR_COPY,
                        batchCount,
                        shaderExecutable);

                    if (status != VX_SUCCESS) goto exit;

                    vxnneOperation_AddReference(&lstmUnitNode->ts_cp_sh_operation[ts_cp_op_idx].base, (vx_reference)tensorArray[i], VXNNE_OPERATION_REFENRENCE_INPUT);
                    vxnneOperation_AddReference(&lstmUnitNode->ts_cp_sh_operation[ts_cp_op_idx].base, (vx_reference)subtensor[i], VXNNE_OPERATION_REFENRENCE_OUTPUT);

                    vxnneLayer_SetOperation(
                        &lstmUnitNode->base,
                        &lstmUnitNode->ts_cp_sh_operation[ts_cp_op_idx++].base,
                        opIdx++);

                    if (tensor_view[i]) vxReleaseTensorView(&tensor_view[i]);
                }
            }
            else
            {
                vxnneExecuteSWLstmPreProcessConcat(tensorArray, numofweights, w_x);
            }

            /* Concat recurrent weights. */
            sizes[0] = TENSOR_VIEW_SIZE_INDEX(recurrent2forget_weight, 0);
            sizes[1] = TENSOR_VIEW_SIZE_INDEX(recurrent2forget_weight, 1) * numofweights;
            dims = 2;
            idx = 0;
            if (enable_cifg == vx_false_e)
                tensorArray[idx++] = recurrent2input_weight;
            tensorArray[idx++] = recurrent2forget_weight;
            tensorArray[idx++] = recurrent2cell_weight;
            tensorArray[idx++] = recurrent2output_weight;

            gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
            tensor_create_params.num_of_dims = dims;
            tensor_create_params.sizes = sizes;
            tensor_create_params.data_format = TENSOR_DATA_TYPE(recurrent2forget_weight);
            tensor_create_params.quant_format = TENSOR_QUANT_TYPE(recurrent2forget_weight);
            if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
            {
                tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(recurrent2forget_weight);
            }
            else
            {
                tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(recurrent2forget_weight);
                tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(recurrent2forget_weight);
            }

            w_h = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_false_e);
            if (vxoTensor_AllocateMemory(w_h) != VX_SUCCESS)
            {
                vxError("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
                status = VX_ERROR_NO_MEMORY;
                return status;
            }
            lstmUnitNode->base.temp_tensors[tmpTensorIndex++] = w_h;

            isDynamicInput = !CHECK_LIFETIME_IS_STATIC(recurrent2forget_weight) ||
                !CHECK_LIFETIME_IS_STATIC(recurrent2cell_weight) ||
                !CHECK_LIFETIME_IS_STATIC(recurrent2output_weight);

            if (isDynamicInput)
            {
                vxnne_shader_executable shaderExecutable;
                vx_tensor       subtensor[4] = { NULL };
                vx_tensor_view  tensor_view[4] = { NULL };
                vx_uint32       start[4] = { 0, 0, 0, 0 };
                vx_uint32       end[4] = { sizes[0], 0, 0, 0 };
                vx_uint32       weight_height = TENSOR_VIEW_SIZE_INDEX(recurrent2forget_weight, 1);
                vx_uint32       i;

                for (i = 0; i < idx; i++)
                {
                    start[1] = end[1];
                    end[1] += weight_height;

                    tensor_view[i] = vxCreateTensorView(node->base.context, start, end, (vx_uint8)dims);
                    if (tensor_view[i] == NULL)
                    {
                        vxError("vxCreateTensorView failure! at line %d\n", __LINE__);
                        goto exit;
                    }

                    subtensor[i] = vxoTensor_CreateTensorFromView(w_h, tensor_view[i]);
                    lstmUnitNode->base.temp_tensors[tmpTensorIndex++] = subtensor[i];

                    if (node->base.context->evisNoInst.supportEVIS)
                    {
                        shaderExecutable = vxnneTensorCopyShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_COPY, &node->kernelAttributes.borderMode, tensorArray[i], subtensor[i]);
                    }
                    else
                    {
                        shaderExecutable = vxnneGPUTensorCopyShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_COPY, &node->kernelAttributes.borderMode, tensorArray[i], subtensor[i]);
                    }

                    if (!shaderExecutable)
                    {
                        status = VX_FAILURE;
                        goto exit;
                    }

                    status = vxnneShaderOperation_Initialize(&lstmUnitNode->ts_cp_sh_operation[ts_cp_op_idx],
                        &lstmUnitNode->base,
                        VXNNE_OPERATOR_TENSOR_COPY,
                        batchCount,
                        shaderExecutable);

                    if (status != VX_SUCCESS) goto exit;

                    vxnneOperation_AddReference(&lstmUnitNode->ts_cp_sh_operation[ts_cp_op_idx].base, (vx_reference)tensorArray[i], VXNNE_OPERATION_REFENRENCE_INPUT);
                    vxnneOperation_AddReference(&lstmUnitNode->ts_cp_sh_operation[ts_cp_op_idx].base, (vx_reference)subtensor[i], VXNNE_OPERATION_REFENRENCE_OUTPUT);

                    vxnneLayer_SetOperation(
                        &lstmUnitNode->base,
                        &lstmUnitNode->ts_cp_sh_operation[ts_cp_op_idx++].base,
                        opIdx++);

                    if (tensor_view[i]) vxReleaseTensorView(&tensor_view[i]);
                }
            }
            else
            {
                vxnneExecuteSWLstmPreProcessConcat(tensorArray, numofweights, w_h);
            }

            /* Concat biases. */
            sizes[0] = TENSOR_VIEW_SIZE_INDEX(forget_gate_bias, 0) * numofweights;
            sizes[1] = 1;
            dims = 2;
            idx = 0;
            if (enable_cifg == vx_false_e)
                tensorArray[idx++] = input_gate_bias;
            tensorArray[idx++] = forget_gate_bias;
            tensorArray[idx++] = cell_bias;
            tensorArray[idx++] = output_gate_bias;

            gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
            tensor_create_params.num_of_dims = dims;
            tensor_create_params.sizes = sizes;
            tensor_create_params.data_format = TENSOR_DATA_TYPE(forget_gate_bias);
            tensor_create_params.quant_format = TENSOR_QUANT_TYPE(forget_gate_bias);
            if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
            {
                tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(forget_gate_bias);
            }
            else
            {
                tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(forget_gate_bias);
                tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(forget_gate_bias);
            }
            bias = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_false_e);
            if (vxoTensor_AllocateMemory(bias) != VX_SUCCESS)
            {
                vxError("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
                status = VX_ERROR_NO_MEMORY;
                return status;
            }
            lstmUnitNode->base.temp_tensors[tmpTensorIndex++] = bias;

            isDynamicInput = !CHECK_LIFETIME_IS_STATIC(forget_gate_bias) ||
                !CHECK_LIFETIME_IS_STATIC(cell_bias) ||
                !CHECK_LIFETIME_IS_STATIC(forget_gate_bias);

            if (isDynamicInput)
            {
                vxnne_shader_executable shaderExecutable;
                vx_tensor       subtensor[4] = { NULL };
                vx_tensor_view  tensor_view[4] = { NULL };
                vx_uint32       start[4] = { 0, 0, 0, 0 };
                vx_uint32       end[4] = { 0, 1, 1, 1 };
                vx_uint32       bias_width = TENSOR_VIEW_SIZE_INDEX(forget_gate_bias, 0);
                vx_uint32       i;

                for (i = 0; i < idx; i++)
                {
                    start[0] = end[0];
                    end[0] += bias_width;

                    tensor_view[i] = vxCreateTensorView(node->base.context, start, end, (vx_uint8)dims);
                    if (tensor_view[i] == NULL)
                    {
                        vxError("vxCreateTensorView failure! at line %d\n", __LINE__);
                        goto exit;
                    }

                    subtensor[i] = vxoTensor_CreateTensorFromView(bias, tensor_view[i]);
                    lstmUnitNode->base.temp_tensors[tmpTensorIndex++] = subtensor[i];

                    if (node->base.context->evisNoInst.supportEVIS)
                    {
                        shaderExecutable = vxnneTensorCopyShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_COPY, &node->kernelAttributes.borderMode, tensorArray[i], subtensor[i]);
                    }
                    else
                    {
                        shaderExecutable = vxnneGPUTensorCopyShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_COPY, &node->kernelAttributes.borderMode, tensorArray[i], subtensor[i]);
                    }

                    if (!shaderExecutable)
                    {
                        status = VX_FAILURE;
                        goto exit;
                    }

                    status = vxnneShaderOperation_Initialize(&lstmUnitNode->ts_cp_sh_operation[ts_cp_op_idx],
                        &lstmUnitNode->base,
                        VXNNE_OPERATOR_TENSOR_COPY,
                        batchCount,
                        shaderExecutable);

                    if (status != VX_SUCCESS) goto exit;

                    vxnneOperation_AddReference(&lstmUnitNode->ts_cp_sh_operation[ts_cp_op_idx].base, (vx_reference)tensorArray[i], VXNNE_OPERATION_REFENRENCE_INPUT);
                    vxnneOperation_AddReference(&lstmUnitNode->ts_cp_sh_operation[ts_cp_op_idx].base, (vx_reference)subtensor[i], VXNNE_OPERATION_REFENRENCE_OUTPUT);

                    vxnneLayer_SetOperation(
                        &lstmUnitNode->base,
                        &lstmUnitNode->ts_cp_sh_operation[ts_cp_op_idx++].base,
                        opIdx++);

                    if (tensor_view[i]) vxReleaseTensorView(&tensor_view[i]);
                }
            }
            else
            {
                vxnneExecuteSWLstmPreProcessConcat(tensorArray, numofweights, bias);
            }

            vxoTensor_GetTensorElementCount(input, &inputCount);
            sizes[0] = TENSOR_VIEW_SIZE_INDEX(input2forget_weight, 1) * numofweights;
            sizes[1] = inputCount / TENSOR_VIEW_SIZE_INDEX(input2forget_weight, 0);
            dims = 2;

            gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
            tensor_create_params.num_of_dims = dims;
            tensor_create_params.sizes = sizes;
            tensor_create_params.data_format = TENSOR_DATA_TYPE(input);
            tensor_create_params.quant_format = TENSOR_QUANT_TYPE(input);
            if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
            {
                tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(input);
            }
            else
            {
                tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(input);
                tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(input);
            }
            fc_output = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);
            lstmUnitNode->base.temp_tensors[tmpTensorIndex++] = fc_output;

            /*op1 : fc*/
            if (node->base.context->evisNoInst.supportEVIS)
            {
                shaderExecutable = vxnneGetFullyConnectedShaderExecutable(node->base.context, VXNNE_KERNEL_FULLYCONNECTED, &node->kernelAttributes.borderMode, input, w_x, bias, VX_NN_ACTIVATION_NONE, fc_output);
            }
            else
            {
                shaderExecutable = vxnneGetGPUFullyConnectedShaderExecutable(node->base.context, VXNNE_KERNEL_FULLYCONNECTED, &node->kernelAttributes.borderMode, input, w_x, bias, VX_NN_ACTIVATION_NONE, fc_output);
            }

            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto exit;
            }

            status = vxnneShaderOperation_Initialize(&lstmUnitNode->lstm_fc_sh_operation,
                &lstmUnitNode->base,
                VXNNE_OPERATOR_FULLYCONNECTED,
                batchCount,
                shaderExecutable);

            if (status != VX_SUCCESS) goto exit;

            vxnneOperation_AddReference(&lstmUnitNode->lstm_fc_sh_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_fc_sh_operation.base, (vx_reference)w_x, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_fc_sh_operation.base, (vx_reference)bias, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_fc_sh_operation.base, (vx_reference)fc_output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            /*op2 : lstm unit*/
            if (enable_projection)
            {
                sizes[0] = TENSOR_VIEW_SIZE_INDEX(projection_weight, 0);
                sizes[1] = TENSOR_VIEW_SIZE_INDEX(output, 1);
                dims = 2;

                gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
                tensor_create_params.num_of_dims = dims;
                tensor_create_params.sizes = sizes;
                tensor_create_params.data_format = TENSOR_DATA_TYPE(output);
                tensor_create_params.quant_format = TENSOR_QUANT_TYPE(output);
                if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
                {
                    tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(output);
                }
                else
                {
                    tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(output);
                    tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(output);
                }

                project_out = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);

                lstmUnitNode->base.temp_tensors[tmpTensorIndex++] = project_out;

                if (node->base.context->evisNoInst.supportEVIS)
                {
                    shaderExecutable = vxnneLSTMUnitShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_LSTMUNIT, &node->kernelAttributes.borderMode, fc_output, w_h, output_state_in, cell_state_in, cell_clip, enable_cifg, enable_peephole, enable_projection, cell2input_weight, cell2forget_weight, cell2output_weight, cell_state_out, output_state_out, activation, project_out);

                }
                else
                {
                    shaderExecutable = vxnneGPULSTMUnitShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_LSTMUNIT, &node->kernelAttributes.borderMode, fc_output, w_h,
                        output_state_in, cell_state_in, cellClipValue, enable_cifg, enable_peephole, enable_projection, cell2input_weight,
                        cell2forget_weight, cell2output_weight, cell_state_out, output_state_out, actType, project_out);
                }
            }
            else
            {
                if (node->base.context->evisNoInst.supportEVIS)
                {
                    shaderExecutable = vxnneLSTMUnitShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_LSTMUNIT, &node->kernelAttributes.borderMode, fc_output, w_h, output_state_in, cell_state_in, cell_clip, enable_cifg, enable_peephole, enable_projection, cell2input_weight, cell2forget_weight, cell2output_weight, cell_state_out, output_state_out, activation, output);

                }
                else
                {
                    shaderExecutable = vxnneGPULSTMUnitShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_LSTMUNIT, &node->kernelAttributes.borderMode, fc_output, w_h,
                        output_state_in, cell_state_in, cellClipValue, enable_cifg, enable_peephole, enable_projection, cell2input_weight,
                        cell2forget_weight, cell2output_weight, cell_state_out, output_state_out, actType, output);
                }
            }

            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto exit;
            }

            status = vxnneShaderOperation_Initialize(&lstmUnitNode->lstm_unit_sh_operation,
                &lstmUnitNode->base,
                VXNNE_OPERATOR_LSTM_UNIT,
                batchCount,
                shaderExecutable);

            if (status != VX_SUCCESS) goto exit;

            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_sh_operation.base, (vx_reference)fc_output, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_sh_operation.base, (vx_reference)w_h, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_sh_operation.base, (vx_reference)output_state_in, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_sh_operation.base, (vx_reference)cell_state_in, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_sh_operation.base, (vx_reference)activation, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_sh_operation.base, (vx_reference)cell_clip, VXNNE_OPERATION_REFENRENCE_INPUT);
            if (enable_peephole)
            {
                if (cell2input_weight != NULL)
                    vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_sh_operation.base, (vx_reference)cell2input_weight, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_sh_operation.base, (vx_reference)cell2forget_weight, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_sh_operation.base, (vx_reference)cell2output_weight, VXNNE_OPERATION_REFENRENCE_INPUT);
            }
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_sh_operation.base, (vx_reference)cell_state_out, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_sh_operation.base, (vx_reference)output_state_out, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            if (enable_projection)
            {
                vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_sh_operation.base, (vx_reference)project_out, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            }
            else
            {
                vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_sh_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            }


            lstmUnitNode->base.num_temp_tensors = tmpTensorIndex;

            vxnneLayer_SetOperation(
                &lstmUnitNode->base,
                &lstmUnitNode->lstm_fc_sh_operation.base,
                opIdx++);
            vxnneLayer_SetOperation(
                &lstmUnitNode->base,
                &lstmUnitNode->lstm_unit_sh_operation.base,
                opIdx++);

            if (enable_projection)
            {
                if (node->base.context->evisNoInst.supportEVIS)
                {
                    shaderExecutable = vxnneGetLSTMUnitProjectionShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_LSTMUNIT_PROJECTION, &node->kernelAttributes.borderMode, project_out, projection_weight, projection_bias, proj_clip, output_state_out, output);

                }
                else
                {
                    shaderExecutable = vxnneGetGPULSTMUnitProjectionShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_LSTMUNIT_PROJECTION, &node->kernelAttributes.borderMode, project_out, projection_weight, projection_bias, projClipValue, output_state_out, output);

                }

                if (!shaderExecutable)
                {
                    status = VX_FAILURE;
                    goto exit;
                }

                status = vxnneShaderOperation_Initialize(&lstmUnitNode->lstm_unit_prj_sh_operation,
                    &lstmUnitNode->base,
                    VXNNE_OPERATOR_LSTM_UNIT,
                    batchCount,
                    shaderExecutable);

                if (status != VX_SUCCESS) goto exit;

                vxnneLayer_SetOperation(
                    &lstmUnitNode->base,
                    &lstmUnitNode->lstm_unit_prj_sh_operation.base,
                    opIdx++);

                vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_prj_sh_operation.base, (vx_reference)project_out, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_prj_sh_operation.base, (vx_reference)projection_weight, VXNNE_OPERATION_REFENRENCE_INPUT);
                if (projection_bias)
                    vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_prj_sh_operation.base, (vx_reference)projection_bias, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_prj_sh_operation.base, (vx_reference)proj_clip, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_prj_sh_operation.base, (vx_reference)output_state_out, VXNNE_OPERATION_REFENRENCE_OUTPUT);
                vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_prj_sh_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            }
        }
        else
        {
            vxnneOperation_Initialize(&lstmUnitNode->lstm_unit_operation.base,
                &lstmUnitNode->base,
                VXNNE_OPERATION_TARGET_SW,
                VXNNE_OPERATOR_LSTM_UNIT,
                vxnneExecuteSW_LSTMUnit,
                VX_NULL,
                batchCount,
                0);

            vxnneLayer_SetOperation(
                &lstmUnitNode->base,
                &lstmUnitNode->lstm_unit_operation.base,
                0);

            lstmUnitNode->lstm_unit_operation.input = input;

            lstmUnitNode->lstm_unit_operation.input2input_weight = input2input_weight;
            lstmUnitNode->lstm_unit_operation.input2forget_weight = input2forget_weight;
            lstmUnitNode->lstm_unit_operation.input2cell_weight = input2cell_weight;
            lstmUnitNode->lstm_unit_operation.input2output_weight = input2output_weight;

            lstmUnitNode->lstm_unit_operation.recurrent2input_weight = recurrent2input_weight;
            lstmUnitNode->lstm_unit_operation.recurrent2forget_weight = recurrent2forget_weight;
            lstmUnitNode->lstm_unit_operation.recurrent2cell_weight = recurrent2cell_weight;
            lstmUnitNode->lstm_unit_operation.recurrent2output_weight = recurrent2output_weight;

            lstmUnitNode->lstm_unit_operation.cell2input_weight = cell2input_weight;
            lstmUnitNode->lstm_unit_operation.cell2forget_weight = cell2forget_weight;
            lstmUnitNode->lstm_unit_operation.cell2output_weight = cell2output_weight;

            lstmUnitNode->lstm_unit_operation.layernorm2input_weight  = layernorm2input_weight;
            lstmUnitNode->lstm_unit_operation.layernorm2forget_weight = layernorm2forget_weight;
            lstmUnitNode->lstm_unit_operation.layernorm2cell_weight   = layernorm2cell_weight;
            lstmUnitNode->lstm_unit_operation.layernorm2output_weight = layernorm2output_weight;

            lstmUnitNode->lstm_unit_operation.input_gate_bias = input_gate_bias;
            lstmUnitNode->lstm_unit_operation.forget_gate_bias = forget_gate_bias;
            lstmUnitNode->lstm_unit_operation.cell_bias = cell_bias;
            lstmUnitNode->lstm_unit_operation.output_gate_bias = output_gate_bias;

            lstmUnitNode->lstm_unit_operation.projection_weight = projection_weight;
            lstmUnitNode->lstm_unit_operation.projection_bias = projection_bias;
            lstmUnitNode->lstm_unit_operation.output_state_in = output_state_in;
            lstmUnitNode->lstm_unit_operation.cell_state_in = cell_state_in;

            lstmUnitNode->lstm_unit_operation.activation = activation;
            lstmUnitNode->lstm_unit_operation.forget_bias = forget_bias;
            lstmUnitNode->lstm_unit_operation.cell_clip = cell_clip;
            lstmUnitNode->lstm_unit_operation.proj_clip = proj_clip;

            lstmUnitNode->lstm_unit_operation.scratch = scratch;
            lstmUnitNode->lstm_unit_operation.output_state_out = output_state_out;
            lstmUnitNode->lstm_unit_operation.cell_state_out = cell_state_out;
            lstmUnitNode->lstm_unit_operation.output = output;

            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);

            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)input2input_weight, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)input2forget_weight, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)input2cell_weight, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)input2output_weight, VXNNE_OPERATION_REFENRENCE_INPUT);

            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)recurrent2input_weight, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)recurrent2forget_weight, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)recurrent2cell_weight, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)recurrent2output_weight, VXNNE_OPERATION_REFENRENCE_INPUT);

            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)layernorm2input_weight, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)layernorm2forget_weight, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)layernorm2cell_weight, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)layernorm2output_weight, VXNNE_OPERATION_REFENRENCE_INPUT);

            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)cell2input_weight, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)cell2forget_weight, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)cell2output_weight, VXNNE_OPERATION_REFENRENCE_INPUT);

            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)input_gate_bias, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)forget_gate_bias, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)cell_bias, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)output_gate_bias, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)output_state_in, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)cell_state_in, VXNNE_OPERATION_REFENRENCE_INPUT);

            if (projection_weight)vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)projection_weight, VXNNE_OPERATION_REFENRENCE_INPUT);
            if (projection_bias)vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)projection_bias, VXNNE_OPERATION_REFENRENCE_INPUT);

            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)scratch, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)output_state_out, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)cell_state_out, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            vxnneOperation_AddReference(&lstmUnitNode->lstm_unit_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        }

        node->layer = &lstmUnitNode->base;
    }

    return VX_SUCCESS;

exit:
    if (lstmUnitNode)
    {
        gcoOS_Free(VX_NULL, lstmUnitNode);
    }

    return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_LSTMUnit_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}


VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NN_LSTMLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_LSTMLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_LSTMLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

vx_status vxnneExecuteSW_LSTMLayer(struct _vxnne_operation_s *operation)
{
    vxnne_lstm_layer_sw_operation lstmOperation = (vxnne_lstm_layer_sw_operation)operation;
    vxnne_lstm_layer lstmLayer = (vxnne_lstm_layer)lstmOperation->base.layer;

    vx_tensor input = lstmOperation->input;
    vx_uint32 input_num = lstmOperation->input_num;
    vx_tensor output = lstmOperation->output;
    vx_uint32 output_num = lstmOperation->output_num;
    vx_tensor w_x = lstmOperation->w_x;
    vx_tensor bias = lstmOperation->bias;
    vx_tensor w_x_x = lstmOperation->w_x_x;
    vx_tensor w_h = lstmOperation->w_h;
    vx_tensor w_c = lstmOperation->w_c;
    vx_tensor gate_input = lstmOperation->gate_input;
    vx_tensor *hidden_states = lstmLayer->hidden_states;
    vx_tensor *cell_states = lstmLayer->cell_states;
    vx_float32 forget_bias = (lstmOperation->forget_bias != VX_NULL) ? VX_GET_DATA_FROM_TENSOR(lstmOperation->forget_bias, 0) : 0.f;

    vx_uint32 time_step = lstmOperation->time_step;
    vx_uint32 batch = lstmOperation->batch;

    vx_type_e input_format = (vx_type_e)TENSOR_DATA_TYPE(input);
    vx_type_e w_x_format = (vx_type_e)TENSOR_DATA_TYPE(w_x);
    vx_type_e bias_format = (vx_type_e)TENSOR_DATA_TYPE(bias);
    vx_type_e w_x_x_format = (vx_type_e)TENSOR_DATA_TYPE(w_x_x);
    vx_type_e w_h_format = (vx_type_e)TENSOR_DATA_TYPE(w_h);
    vx_type_e gate_format = (vx_type_e)TENSOR_DATA_TYPE(gate_input);
    vx_type_e cell_format = (vx_type_e)TENSOR_DATA_TYPE(cell_states[0]);
    vx_type_e hidden_format = (vx_type_e)TENSOR_DATA_TYPE(hidden_states[0]);
    vx_type_e output_format = (vx_type_e)TENSOR_DATA_TYPE(output);

    gctPOINTER inputBase, inputBase_w_x, inputBase_b, inputBase_w_x_x, inputBase_w_h, inputBase_h_tm1s, inputBase_c_tm1s, inputBase_w_c = VX_NULL;
    gctPOINTER outputBase, outputBase_h_t, outputBase_c_t, outputBase_gate_input;
    vx_uint32 i, j, t, b, k;

    vxoTensor_GetTensorViewMemory(input, &inputBase, VX_NULL);
    vxoTensor_GetTensorViewMemory(w_x, &inputBase_w_x, VX_NULL);
    vxoTensor_GetTensorViewMemory(bias, &inputBase_b, VX_NULL);
    vxoTensor_GetTensorViewMemory(w_x_x, &inputBase_w_x_x, VX_NULL);
    vxoTensor_GetTensorViewMemory(w_h, &inputBase_w_h, VX_NULL);
    vxoTensor_GetTensorViewMemory(gate_input, &outputBase_gate_input, VX_NULL);
    vxoTensor_GetTensorViewMemory(output, &outputBase, VX_NULL);

    if (w_c)
        vxoTensor_GetTensorViewMemory(w_c, &inputBase_w_c, VX_NULL);

    /* w_x_x = w_x * x + b */
    for (t = 0; t < time_step; t++)
    {
        for (b = 0; b < batch; b++)
        {
            /* i, f, g and o */
            for (k = 0; k < 4; k++)
            {
                for (j = 0; j < output_num; j++)
                {
                    vx_int32 bias_index = j + output_num * k;
                    vx_int32 w_x_x_index = j + output_num *(k + 4 * (b + batch * t));
                    vx_float32 sum;

                    /* Asign bias to sum. */
                    sum = vxnneGetDataExt(bias_format, TENSOR_QUANT_TYPE(bias), bias_index, (vx_uint8_ptr)inputBase_b, TENSOR_POS(bias), TENSOR_TF_ZEROPOINT(bias), TENSOR_TF_SCALE(bias));

                    for (i = 0; i < input_num; i++)
                    {
                        vx_int32 w_x_index = i + input_num * (j + output_num * k);
                        vx_int32 input_index = i + input_num * (b + batch * t);
                        vx_float32 w_x_data, input_data;

                        w_x_data = vxnneGetDataExt(w_x_format, TENSOR_QUANT_TYPE(w_x), w_x_index, (vx_uint8_ptr)inputBase_w_x, TENSOR_POS(w_x), TENSOR_TF_ZEROPOINT(w_x), TENSOR_TF_SCALE(w_x));
                        input_data = vxnneGetDataExt(input_format, TENSOR_QUANT_TYPE(input), input_index, (vx_uint8_ptr)inputBase, TENSOR_POS(input), TENSOR_TF_ZEROPOINT(input), TENSOR_TF_SCALE(input));

                        sum += w_x_data * input_data;
                    }

                    vxnneSaveDataExt(w_x_x_format, TENSOR_QUANT_TYPE(w_x_x), w_x_x_index, sum, (vx_uint8_ptr)inputBase_w_x_x, TENSOR_POS(w_x_x), TENSOR_TF_ZEROPOINT(w_x_x), TENSOR_TF_SCALE(w_x_x), TENSOR_ROUNDING_MODE(w_x_x));
                }
            }
        }
    }

    for (t = 0; t < time_step; t++)
    {
        vx_tensor h_tm1s, h_t, c_tm1s, c_t;

        h_tm1s = hidden_states[t];
        h_t = hidden_states[t + 1];
        c_tm1s = cell_states[t];
        c_t = cell_states[t + 1];

        vxoTensor_GetTensorViewMemory(h_tm1s, &inputBase_h_tm1s, VX_NULL);
        vxoTensor_GetTensorViewMemory(c_tm1s, &inputBase_c_tm1s, VX_NULL);
        vxoTensor_GetTensorViewMemory(h_t, &outputBase_h_t, VX_NULL);
        vxoTensor_GetTensorViewMemory(c_t, &outputBase_c_t, VX_NULL);

        if (t == 0)
        {
            memset(inputBase_h_tm1s, 0, output_num * vxnneGetTypeSize(w_h_format));
            memset(inputBase_c_tm1s, 0, output_num * vxnneGetTypeSize(w_h_format));
        }

        /* gate_input = 0 + w_h * h_tm1s */
        for (b = 0; b < batch; b++)
        {
            for (k = 0; k < 4; k++)
            {
                for (j = 0; j < output_num; j++)
                {
                    vx_int32 gate_input_index = j + output_num * (k + 4 * b);
                    vx_float32 gate_input_data;

                    gate_input_data = 0;

                    for (i = 0; i < output_num; i++)
                    {
                        vx_int32 w_h_index = i + output_num * (j + output_num * k);
                        vx_int32 h_tm1s_index = i + output_num * b;
                        vx_float32 w_h_data, h_tm1s_data;

                        w_h_data = vxnneGetDataExt(w_h_format, TENSOR_QUANT_TYPE(w_h), w_h_index, (vx_uint8_ptr)inputBase_w_h, TENSOR_POS(w_h), TENSOR_TF_ZEROPOINT(w_h), TENSOR_TF_SCALE(w_h));
                        h_tm1s_data = vxnneGetDataExt(hidden_format, TENSOR_QUANT_TYPE(h_tm1s), h_tm1s_index, (vx_uint8_ptr)inputBase_h_tm1s, TENSOR_POS(h_tm1s), TENSOR_TF_ZEROPOINT(h_tm1s), TENSOR_TF_SCALE(h_tm1s));

                        gate_input_data += w_h_data * h_tm1s_data;
                    }

                    vxnneSaveDataExt(gate_format, TENSOR_QUANT_TYPE(gate_input), gate_input_index, gate_input_data, (vx_uint8_ptr)outputBase_gate_input, TENSOR_POS(gate_input), TENSOR_TF_ZEROPOINT(gate_input), TENSOR_TF_SCALE(gate_input), TENSOR_ROUNDING_MODE(gate_input));

                }
            }
        }

        /*
        [ i_t' ]
        [ f_t' ]
        [ g_t' ]
        [ o_t' ] := gate_input_t
        i_t := \sigmoid[i_t']
        f_t := \sigmoid[f_t']
        g_t := \tanh[g_t']
        o_t := \sigmoid[o_t']
        c_t := cont_t * (f_t .* c_{t-1}) + (i_t .* g_t)
        h_t := o_t .* \tanh[c_t] */
        for (b = 0; b < batch; b++)
        {
            for (i = 0; i < output_num; i++)
            {
                vx_int32 t0_index = i + output_num * (0 + 4 * b + 4 * batch * t);
                vx_int32 t1_index = i + output_num * (1 + 4 * b + 4 * batch * t);
                vx_int32 t2_index = i + output_num * (2 + 4 * b + 4 * batch * t);
                vx_int32 t3_index = i + output_num * (3 + 4 * b + 4 * batch * t);
                vx_int32 i_t_index = i + output_num * (0 + 4 * b);
                vx_float32 data_i_t = vxnneGetDataExt(gate_format, TENSOR_QUANT_TYPE(gate_input), i_t_index, (vx_uint8_ptr)outputBase_gate_input, TENSOR_POS(gate_input), TENSOR_TF_ZEROPOINT(gate_input), TENSOR_TF_SCALE(gate_input));
                vx_int32 f_t_index = i + output_num * (1 + 4 * b);
                vx_float32 data_f_t = vxnneGetDataExt(gate_format, TENSOR_QUANT_TYPE(gate_input), f_t_index, (vx_uint8_ptr)outputBase_gate_input, TENSOR_POS(gate_input), TENSOR_TF_ZEROPOINT(gate_input), TENSOR_TF_SCALE(gate_input));
                vx_int32 g_t_index = i + output_num * (2 + 4 * b);
                vx_float32 data_g_t = vxnneGetDataExt(gate_format, TENSOR_QUANT_TYPE(gate_input), g_t_index, (vx_uint8_ptr)outputBase_gate_input, TENSOR_POS(gate_input), TENSOR_TF_ZEROPOINT(gate_input), TENSOR_TF_SCALE(gate_input));
                vx_int32 o_t_index = i + output_num * (3 + 4 * b);
                vx_float32 data_o_t = vxnneGetDataExt(gate_format, TENSOR_QUANT_TYPE(gate_input), o_t_index, (vx_uint8_ptr)outputBase_gate_input, TENSOR_POS(gate_input), TENSOR_TF_ZEROPOINT(gate_input), TENSOR_TF_SCALE(gate_input));
                vx_int32 c_t_index = i + output_num * b;
                vx_float32 data_c_t = vxnneGetDataExt(cell_format, TENSOR_QUANT_TYPE(c_tm1s), c_t_index, (vx_uint8_ptr)inputBase_c_tm1s, TENSOR_POS(c_tm1s), TENSOR_TF_ZEROPOINT(c_tm1s), TENSOR_TF_SCALE(c_tm1s));
                vx_float32 data_h_t;
                vx_int32 output_index = i + output_num * (b + batch * t);

                data_i_t += vxnneGetDataExt(w_x_x_format, TENSOR_QUANT_TYPE(w_x_x), t0_index, (vx_uint8_ptr)inputBase_w_x_x, TENSOR_POS(w_x_x), TENSOR_TF_ZEROPOINT(w_x_x), TENSOR_TF_SCALE(w_x_x));
                data_f_t += vxnneGetDataExt(w_x_x_format, TENSOR_QUANT_TYPE(w_x_x), t1_index, (vx_uint8_ptr)inputBase_w_x_x, TENSOR_POS(w_x_x), TENSOR_TF_ZEROPOINT(w_x_x), TENSOR_TF_SCALE(w_x_x));
                data_g_t += vxnneGetDataExt(w_x_x_format, TENSOR_QUANT_TYPE(w_x_x), t2_index, (vx_uint8_ptr)inputBase_w_x_x, TENSOR_POS(w_x_x), TENSOR_TF_ZEROPOINT(w_x_x), TENSOR_TF_SCALE(w_x_x));
                data_o_t += vxnneGetDataExt(w_x_x_format, TENSOR_QUANT_TYPE(w_x_x), t3_index, (vx_uint8_ptr)inputBase_w_x_x, TENSOR_POS(w_x_x), TENSOR_TF_ZEROPOINT(w_x_x), TENSOR_TF_SCALE(w_x_x));

                if (inputBase_w_c)
                {
                    vx_float32 w_i_data = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(w_c), TENSOR_QUANT_TYPE(w_c), i_t_index, (vx_uint8_ptr)inputBase_w_c, TENSOR_POS(w_c), TENSOR_TF_ZEROPOINT(w_c), TENSOR_TF_SCALE(w_c));
                    vx_float32 w_f_data = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(w_c), TENSOR_QUANT_TYPE(w_c), f_t_index, (vx_uint8_ptr)inputBase_w_c, TENSOR_POS(w_c), TENSOR_TF_ZEROPOINT(w_c), TENSOR_TF_SCALE(w_c));
                    data_i_t += w_i_data * data_c_t;
                    data_f_t += w_f_data * data_c_t;
                }

                if (forget_bias != 0)
                    data_f_t += forget_bias;

                data_i_t = 1.0f / (1 + gcoMATH_Exp(data_i_t * (-1)));
                data_f_t = 1.0f / (1 + gcoMATH_Exp(data_f_t * (-1)));
                data_g_t = gcoMATH_TangentH(data_g_t);
                data_c_t = data_f_t * data_c_t + data_i_t * data_g_t;

                /*peephole*/
                if (inputBase_w_c)
                {
                    vx_float32 w_c_data = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(w_c), TENSOR_QUANT_TYPE(w_c), o_t_index, (vx_uint8_ptr)inputBase_w_c, TENSOR_POS(w_c), TENSOR_TF_ZEROPOINT(w_c), TENSOR_TF_SCALE(w_c));

                    data_o_t += w_c_data * data_c_t;
                }
                data_o_t = 1.0f / (1 + gcoMATH_Exp(data_o_t * (-1)));
                data_h_t = data_o_t * gcoMATH_TangentH(data_c_t);

                vxnneSaveDataExt(cell_format, TENSOR_QUANT_TYPE(c_t), c_t_index, data_c_t, (vx_uint8_ptr)outputBase_c_t, TENSOR_POS(c_t), TENSOR_TF_ZEROPOINT(c_t), TENSOR_TF_SCALE(c_t), TENSOR_ROUNDING_MODE(c_t));
                vxnneSaveDataExt(hidden_format, TENSOR_QUANT_TYPE(h_t), c_t_index, data_h_t, (vx_uint8_ptr)outputBase_h_t, TENSOR_POS(h_t), TENSOR_TF_ZEROPOINT(h_t), TENSOR_TF_SCALE(h_t), TENSOR_ROUNDING_MODE(h_t));

                /* Save the results. */
                vxnneSaveDataExt(output_format, TENSOR_QUANT_TYPE(output), output_index, data_h_t, (vx_uint8_ptr)outputBase, TENSOR_POS(output), TENSOR_TF_ZEROPOINT(output), TENSOR_TF_SCALE(output), TENSOR_ROUNDING_MODE(output));
            }
        }
    }

    return VX_SUCCESS;
}

vx_status vxnneLSTMLayerNNOperation_Deinitialize(vxnne_operation_s *operation)
{
    vxnne_convolution_relu_pooling_operation lstm_layer_nn_operation = (vxnne_convolution_relu_pooling_operation)operation;

    if (lstm_layer_nn_operation->weights_biases != VX_NULL)
    {
        vxReleaseWeightsBiasesParameter(&lstm_layer_nn_operation->weights_biases);
    }

    return VX_SUCCESS;
}

#define TENSOR_TYPE_CHECK(tensor, type) ((tensor != VX_NULL) && TENSOR_DATA_TYPE(tensor) == type)

vx_status vxnneExecuteSWHiddenUnitOut_LSTMLayer(struct _vxnne_operation_s *operation)
{
    vxnne_lstm_hidden_unit_sw_operation lstmOperation = (vxnne_lstm_hidden_unit_sw_operation)operation;

    vx_tensor input_fc = lstmOperation->input_fc_in;
    vx_tensor hidden_fc = lstmOperation->hidden_fc_in;
    vx_tensor cell_state = lstmOperation->cell_state;
    vx_tensor cell_state_out = lstmOperation->cell_state_out;
    vx_tensor hidden_state_out = lstmOperation->output_state_out;
    vx_tensor output = lstmOperation->output;

    vx_uint32 output_num = lstmOperation->output_num;
    vx_bool   enable_cell_in = lstmOperation->enable_cell_in;

    vx_bool   enable_packed_mode = lstmOperation->enable_packed_mode;

    vx_float32 forget_bias = (lstmOperation->forget_bias != VX_NULL) ? VX_GET_DATA_FROM_TENSOR(lstmOperation->forget_bias, 0) : 0.f;

    vx_uint32 batch = lstmOperation->batch;

    vx_type_e input_fc_format = (vx_type_e)TENSOR_DATA_TYPE(input_fc);
    vx_type_e hidden_fc_format = VX_TYPE_FLOAT16;
    vx_type_e cell_format = (vx_type_e)TENSOR_DATA_TYPE(cell_state);
    vx_type_e hidden_format = VX_TYPE_FLOAT16;
    vx_type_e output_format = (vx_type_e)TENSOR_DATA_TYPE(output);

    gctPOINTER inputFcBase = NULL;
    gctPOINTER hidden_fcBase = NULL;
    gctPOINTER cell_Base = NULL;
    gctPOINTER cellOut_Base = NULL;
    gctPOINTER hidden_outBase = NULL;
    gctPOINTER outputBase = NULL;

    vx_uint32 i, b;

    vxoTensor_GetTensorViewMemory(input_fc, &inputFcBase, VX_NULL);
    if (hidden_fc)
    {
        vxoTensor_GetTensorViewMemory(hidden_fc, &hidden_fcBase, VX_NULL);
        hidden_fc_format = (vx_type_e)TENSOR_DATA_TYPE(hidden_fc);
    }
    vxoTensor_GetTensorViewMemory(cell_state, &cell_Base, VX_NULL);
    if (hidden_state_out)
    {
        vxoTensor_GetTensorViewMemory(hidden_state_out, &hidden_outBase, VX_NULL);
        hidden_format = (vx_type_e)TENSOR_DATA_TYPE(hidden_state_out);
    }
    vxoTensor_GetTensorViewMemory(output, &outputBase, VX_NULL);
    if (cell_state_out)
    {
        vxoTensor_GetTensorViewMemory(cell_state_out, &cellOut_Base, VX_NULL);
    }

    /*
    [ i_t' ]
    [ f_t' ]
    vx_enum   w_xFormat               = VX_TYPE_FLOAT16;
    vx_enum   b_xFormat               = VX_TYPE_FLOAT16;
    vx_enum   w_hFormat               = VX_TYPE_FLOAT16;
    vx_enum   h_tFormat               = VX_TYPE_FLOAT16;
    vx_enum   c_tFormat               = VX_TYPE_FLOAT16;
    vx_bool   shExe_flag              = vx_false_e;
    c_t := cont_t * (f_t .* c_{t-1}) + (i_t .* g_t)
    h_t := o_t .* \tanh[c_t] */
    for (b = 0; b < batch; b++)
    {
        for (i = 0; i < output_num; i++)
        {
            vx_int32 i_t_index = 0;
            vx_int32 f_t_index = 0;
            vx_int32 g_t_index = 0;
            vx_int32 o_t_index = 0;
            vx_float32 data_i_t = 0;
            vx_float32 data_f_t = 0;
            vx_float32 data_g_t = 0;
            vx_float32 data_o_t = 0;
            vx_int32 c_t_index = i + output_num * b;
            vx_float32 data_c_t = enable_cell_in ? vxnneGetDataExt(cell_format, TENSOR_QUANT_TYPE(cell_state), c_t_index, (vx_uint8_ptr)cell_Base, TENSOR_POS(cell_state), TENSOR_TF_ZEROPOINT(cell_state), TENSOR_TF_SCALE(cell_state)) : 0;
            vx_float32 data_h_t;
            vx_int32 output_index = i + output_num * b;

            if (enable_packed_mode)
            {
                i_t_index = 4 * i + 0 + 4 * output_num * b;
                f_t_index = 4 * i + 1 + 4 * output_num * b;
                g_t_index = 4 * i + 2 + 4 * output_num * b;
                o_t_index = 4 * i + 3 + 4 * output_num * b;
            }
            else
            {
                i_t_index = i + output_num * (0 + 4 * b);
                f_t_index = i + output_num * (1 + 4 * b);
                g_t_index = i + output_num * (2 + 4 * b);
                o_t_index = i + output_num * (3 + 4 * b);
            }


            data_i_t = vxnneGetDataExt(input_fc_format, TENSOR_QUANT_TYPE(input_fc), i_t_index, (vx_uint8_ptr)inputFcBase, TENSOR_POS(input_fc), TENSOR_TF_ZEROPOINT(input_fc), TENSOR_TF_SCALE(input_fc));
            data_f_t = vxnneGetDataExt(input_fc_format, TENSOR_QUANT_TYPE(input_fc), f_t_index, (vx_uint8_ptr)inputFcBase, TENSOR_POS(input_fc), TENSOR_TF_ZEROPOINT(input_fc), TENSOR_TF_SCALE(input_fc));
            data_g_t = vxnneGetDataExt(input_fc_format, TENSOR_QUANT_TYPE(input_fc), g_t_index, (vx_uint8_ptr)inputFcBase, TENSOR_POS(input_fc), TENSOR_TF_ZEROPOINT(input_fc), TENSOR_TF_SCALE(input_fc));
            data_o_t = vxnneGetDataExt(input_fc_format, TENSOR_QUANT_TYPE(input_fc), o_t_index, (vx_uint8_ptr)inputFcBase, TENSOR_POS(input_fc), TENSOR_TF_ZEROPOINT(input_fc), TENSOR_TF_SCALE(input_fc));

            if (hidden_fc)
            {
                data_i_t += vxnneGetDataExt(hidden_fc_format, TENSOR_QUANT_TYPE(hidden_fc), i_t_index, (vx_uint8_ptr)hidden_fcBase, TENSOR_POS(hidden_fc), TENSOR_TF_ZEROPOINT(hidden_fc), TENSOR_TF_SCALE(hidden_fc));
                data_f_t += vxnneGetDataExt(hidden_fc_format, TENSOR_QUANT_TYPE(hidden_fc), f_t_index, (vx_uint8_ptr)hidden_fcBase, TENSOR_POS(hidden_fc), TENSOR_TF_ZEROPOINT(hidden_fc), TENSOR_TF_SCALE(hidden_fc));
                data_g_t += vxnneGetDataExt(hidden_fc_format, TENSOR_QUANT_TYPE(hidden_fc), g_t_index, (vx_uint8_ptr)hidden_fcBase, TENSOR_POS(hidden_fc), TENSOR_TF_ZEROPOINT(hidden_fc), TENSOR_TF_SCALE(hidden_fc));
                data_o_t += vxnneGetDataExt(hidden_fc_format, TENSOR_QUANT_TYPE(hidden_fc), o_t_index, (vx_uint8_ptr)hidden_fcBase, TENSOR_POS(hidden_fc), TENSOR_TF_ZEROPOINT(hidden_fc), TENSOR_TF_SCALE(hidden_fc));
            }

            if (forget_bias != 0)
                data_f_t += forget_bias;

            data_i_t = 1.0f / (1 + gcoMATH_Exp(data_i_t * (-1)));
            data_f_t = 1.0f / (1 + gcoMATH_Exp(data_f_t * (-1)));
            data_g_t = gcoMATH_TangentH(data_g_t);

            data_c_t = data_f_t * data_c_t + data_i_t * data_g_t;

            data_o_t = 1.0f / (1 + gcoMATH_Exp(data_o_t * (-1)));

            data_h_t = data_o_t * gcoMATH_TangentH(data_c_t);

            if (cell_state_out)
            {
                vxnneSaveDataExt(cell_format, TENSOR_QUANT_TYPE(cell_state_out), c_t_index, data_c_t, (vx_uint8_ptr)cellOut_Base, TENSOR_POS(cell_state_out), TENSOR_TF_ZEROPOINT(cell_state_out), TENSOR_TF_SCALE(cell_state_out), TENSOR_ROUNDING_MODE(cell_state_out));

            }
            else
            {
                vxnneSaveDataExt(cell_format, TENSOR_QUANT_TYPE(cell_state), c_t_index, data_c_t, (vx_uint8_ptr)cell_Base, TENSOR_POS(cell_state), TENSOR_TF_ZEROPOINT(cell_state), TENSOR_TF_SCALE(cell_state), TENSOR_ROUNDING_MODE(cell_state));
            }
            if (hidden_state_out)
                vxnneSaveDataExt(hidden_format, TENSOR_QUANT_TYPE(hidden_state_out), c_t_index, data_h_t, (vx_uint8_ptr)hidden_outBase, TENSOR_POS(hidden_state_out), TENSOR_TF_ZEROPOINT(hidden_state_out), TENSOR_TF_SCALE(hidden_state_out), TENSOR_ROUNDING_MODE(hidden_state_out));


            /* Save the results. */
            vxnneSaveDataExt(output_format, TENSOR_QUANT_TYPE(output), output_index, data_h_t, (vx_uint8_ptr)outputBase, TENSOR_POS(output), TENSOR_TF_ZEROPOINT(output), TENSOR_TF_SCALE(output), TENSOR_ROUNDING_MODE(output));
        }
    }

    return VX_SUCCESS;
}

#ifndef NEW_LSTM_LAYER_PATH
#define LSTM_FC_OUTPUT_SUB_FL_DFP16     (16)
#define LSTM_FC_OUTPUT_SUB_FL_DFP8      (8)
#define LSTM_FC_OUTPUT_ASYM_U8_FL       (10)
vx_status vxnneExecuteLSTMLayer_NN_TP_LAYER(vx_node node, vx_tensor input,
    vx_tensor input2input_weight, vx_tensor input2forget_weight, vx_tensor input2cell_weight, vx_tensor input2output_weight,
    vx_tensor recurrent2input_weight, vx_tensor recurrent2forget_weight, vx_tensor recurrent2cell_weight, vx_tensor recurrent2output_weight,
    vx_tensor cell2input_weight, vx_tensor cell2forget_weight, vx_tensor cell2output_weight,
    vx_tensor input_gate_bias, vx_tensor forget_gate_bias, vx_tensor cell_bias, vx_tensor output_gate_bias,
    vx_tensor projection_weight, vx_tensor projection_bias, vx_tensor cell_state, vx_tensor activation, vx_tensor forget_bias,
    vx_tensor cell_clip, vx_tensor proj_clip, vx_tensor output, vx_uint32 time_steps)
{
    vx_status           status = VX_SUCCESS;
    vx_context          context = vxGetContext((vx_reference)input);
    vxnne_lstm_layer_s* lstmLayer = VX_NULL;
    vx_tensor_view*     views = VX_NULL;
    vx_tensor*          input_fc_sub_tensors = VX_NULL;
    vx_tensor*          output_sub_tensors = VX_NULL;
    vx_tensor*          hidden_sub_tensors = VX_NULL;
    vx_tensor           w_x = NULL;
    vx_tensor           w_h = NULL;
    vx_tensor           bias = NULL;
    vx_tensor           bias_zero = NULL;
    vx_tensor           w_x_x = NULL;
    vx_tensor           input_fc = NULL;
    vx_tensor           hidden_fc_out = NULL;
    vx_tensor           output_rs = NULL;
    vx_tensor           wb_inputs[4] = { NULL };

    vx_uint32           operationIndex = 0;
    vx_uint32           tmpTensorIndex = 0;
    vx_uint32           input_num = 0;
    vx_uint32           output_num = 0;
    vx_uint32           input_dims = 0;
    vx_uint32           i = 0;
    vx_uint32           dims = 0;
    vx_uint32           batch = 1;
    vx_uint32           batchCount = 1;
    vx_uint32           sizes[] = { 1, 1, 1, 1 };
    vx_uint32           start[4][4] = { { 0 } };
    vx_uint32           end[4][4] = { { 0 } };

    vx_float32          forgetBias = (forget_bias != VX_NULL) ? VX_GET_DATA_FROM_TENSOR(forget_bias, 0) : 0.f;

    vx_bool             enable_format = vx_false_e;
    vx_bool             shExe_flag = vx_false_e;

    vx_type_e biasDataFormat = (TENSOR_DATA_TYPE(recurrent2input_weight) == VX_TYPE_INT8 || TENSOR_DATA_TYPE(recurrent2input_weight) == VX_TYPE_UINT8) ? VX_TYPE_INT32 : VX_TYPE_FLOAT32;
    vx_float32 *biasData = VX_NULL;
    vx_uint32 bias_size = 0;
    vx_uint32 biasSize[4] = { 0 };

    vx_tensor_create_params_t tensor_create_params;
    vx_weights_biases_parameter weights_biases = NULL;
    vx_bool             enable_packed_mode = vx_false_e; /*(input) == VX_TYPE_INT8;*/
    vx_type_e data_format;
    vx_tensor input_fc_input = VX_NULL;
    vx_tensor input_fc_output = VX_NULL;
    vx_tensor recurrent_fc_input = VX_NULL;
    vx_tensor recurrent_fc_output = VX_NULL;
    vx_uint32 op_num = time_steps * 4 + 3;

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_lstm_layer_s), (gctPOINTER*)&lstmLayer);
    if (!lstmLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }
    gcoOS_ZeroMemory(lstmLayer, sizeof(vxnne_lstm_layer_s));

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_operation) * op_num, (gctPOINTER*)&lstmLayer->operations3);
    gcoOS_ZeroMemory(lstmLayer->operations3, sizeof(vxnne_operation) * op_num);
    vxnneLayer_Initialize(&lstmLayer->base,
        "_LSTM_Layer",
        node,
        op_num,
        lstmLayer->operations3,
        VX_NULL);

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_lstm_hidden_unit_s) * time_steps, (gctPOINTER*)&lstmLayer->hidden_units);
    if (!lstmLayer->hidden_units)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }
    else
        gcoOS_ZeroMemory(lstmLayer->hidden_units, sizeof(vxnne_lstm_hidden_unit_s) * time_steps);

    lstmLayer->hidden_unit_num = time_steps;

    input_dims = TENSOR_DIM_NUM(input);
    if (input_dims < 3)
    {
        status = VX_FAILURE;
        vxError("LSTM input dims should >=3: at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }
    batch = TENSOR_VIEW_SIZE_INDEX(input, input_dims - 2);
    input_num = 1;
    for (i = 0; i < input_dims - 2; i++)
    {
        input_num = input_num * TENSOR_VIEW_SIZE_INDEX(input, i);
    }

    output_num = TENSOR_VIEW_SIZE_INDEX(output, 0);

    if ((_IsSameType(input2input_weight, input2forget_weight) &&
         _IsSameType(input2input_weight, input2cell_weight) &&
         _IsSameType(input2input_weight, input2output_weight)) &&
        (_IsSameType(input_gate_bias, forget_gate_bias) &&
         _IsSameType(input_gate_bias, cell_bias) &&
         _IsSameType(input_gate_bias, output_gate_bias)))
    {
        /* Concat input weights. */
        wb_inputs[0] = input2input_weight;
        wb_inputs[1] = input2forget_weight;
        wb_inputs[2] = input2cell_weight;
        wb_inputs[3] = input2output_weight;

        data_format = (vx_type_e)TENSOR_DATA_TYPE(input2input_weight);

        vxmONERROR(_ConcatTensors(node,
                                  wb_inputs,
                                  4,
                                  data_format,
                                  &w_x));

        lstmLayer->base.temp_tensors[tmpTensorIndex++] = w_x;

        /* Concat biases. */
        wb_inputs[0] = input_gate_bias;
        wb_inputs[1] = forget_gate_bias;
        wb_inputs[2] = cell_bias;
        wb_inputs[3] = output_gate_bias;

        data_format = (vx_type_e)((TENSOR_DATA_TYPE(input_gate_bias) == VX_TYPE_FLOAT16) ? VX_TYPE_FLOAT32 : TENSOR_DATA_TYPE(input_gate_bias));

        vxmONERROR(_ConcatTensors(node,
                                  wb_inputs,
                                  4,
                                  data_format,
                                  &bias));

        lstmLayer->base.temp_tensors[tmpTensorIndex++] = bias;
    }
    else
    {
        vxmASSERT("TODO: input weights are not consistent." && 0);
    }

    if (_IsSameType(recurrent2input_weight, recurrent2forget_weight) &&
        _IsSameType(recurrent2input_weight, recurrent2cell_weight) &&
        _IsSameType(recurrent2input_weight, recurrent2output_weight))
    {
        /* Concat recurrent weights. */
        wb_inputs[0] = recurrent2input_weight;
        wb_inputs[1] = recurrent2forget_weight;
        wb_inputs[2] = recurrent2cell_weight;
        wb_inputs[3] = recurrent2output_weight;

        data_format = (vx_type_e)TENSOR_DATA_TYPE(recurrent2input_weight);

        vxmONERROR(_ConcatTensors(node,
                                  wb_inputs,
                                  4,
                                  data_format,
                                  &w_h));

        lstmLayer->base.temp_tensors[tmpTensorIndex++] = w_h;
    }
    else
    {
        vxmASSERT("TODO: recurrent weights are not consistent." && 0);
    }

    /* Use high precision (FP16) intermediate tensor. */
    sizes[0] = output_num * 4;
    sizes[1] = batch;
    sizes[2] = time_steps;
    dims = 3;

    gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
    tensor_create_params.num_of_dims = dims;
    tensor_create_params.sizes = sizes;
    tensor_create_params.data_format = VX_TYPE_FLOAT16;
    tensor_create_params.quant_format = VX_QUANT_NONE;

    w_x_x = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);
    lstmLayer->base.temp_tensors[tmpTensorIndex++] = w_x_x;

    if (w_x)
    {
        vxmONERROR(_ReshapeTensor4FC(input, TENSOR_VIEW_DIM_NUM(input) - 2, &input_fc_input));
        lstmLayer->base.temp_tensors[tmpTensorIndex++] = input_fc_input;

        vxmONERROR(_ReshapeTensor4FC(w_x_x, TENSOR_VIEW_DIM_NUM(w_x_x) - 2, &input_fc_output));
        lstmLayer->base.temp_tensors[tmpTensorIndex++] = input_fc_output;

        /* Input FC. */
        vxmONERROR(vxoFCOperation_Initialize(&lstmLayer->input_fc_operation,
                                             &lstmLayer->base,
                                             input_fc_input,
                                             VX_NULL,
                                             w_x,
                                             bias,
                                             0,
                                             0,
                                             0,
                                             input_fc_output,
                                             &operationIndex));
    }
    else
    {
        vxmASSERT("TODO: input weights are not consistent.\n" && 0);
    }

    if (time_steps >= 1)
    {
        gcoOS_Allocate(gcvNULL, time_steps * sizeof(vx_tensor_view), (gctPOINTER*)&views);
        gcoOS_Allocate(gcvNULL, time_steps * sizeof(vx_tensor), (gctPOINTER*)&input_fc_sub_tensors);
        gcoOS_Allocate(gcvNULL, time_steps * sizeof(vx_tensor), (gctPOINTER*)&output_sub_tensors);
        gcoOS_Allocate(gcvNULL, time_steps * sizeof(vx_tensor), (gctPOINTER*)&hidden_sub_tensors);
    }

    sizes[0] = output_num * 4;
    sizes[1] = batch * time_steps;
    dims = 2;

    /* Use high precision (FP16) intermediate tensor. */
    gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
    tensor_create_params.num_of_dims = dims;
    tensor_create_params.sizes = sizes;
    tensor_create_params.data_format = VX_TYPE_FLOAT16;
    tensor_create_params.quant_format = VX_QUANT_NONE;

    hidden_fc_out = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);

    lstmLayer->base.temp_tensors[tmpTensorIndex++] = hidden_fc_out;

    sizes[0] = output_num * 4;
    sizes[1] = time_steps * batch;
    sizes[2] = 1;
    sizes[3] = 1;
    dims = 2;
    input_fc = vxoTensor_ReshapeTensor(w_x_x, (vx_int32*)sizes, dims);

    lstmLayer->base.temp_tensors[tmpTensorIndex++] = input_fc;

    sizes[0] = output_num;
    sizes[1] = time_steps * batch;
    sizes[2] = 1;
    sizes[3] = 1;
    dims = 2;
    output_rs = vxoTensor_ReshapeTensor(output, (vx_int32*)sizes, dims);

    lstmLayer->base.temp_tensors[tmpTensorIndex++] = output_rs;

    end[0][0] = output_num * 4;
    end[1][0] = output_num;
    end[2][0] = output_num;
    end[3][0] = output_num * 4;
    dims = 2;

    for (i = 0; i < time_steps; i++)
    {
        start[0][1] = end[0][1];
        end[0][1] += 1;
        views[i] = vxCreateTensorView(context, start[0], end[0], (vx_uint8)dims);
        input_fc_sub_tensors[i] = vxoTensor_CreateTensorFromView(input_fc, views[i]);
        if (views[i]) vxReleaseTensorView(&views[i]);

        start[2][1] = end[2][1];
        end[2][1] += 1;
        views[i] = vxCreateTensorView(context, start[2], end[2], (vx_uint8)dims);
        output_sub_tensors[i] = vxoTensor_CreateTensorFromView(output_rs, views[i]);
        if (views[i]) vxReleaseTensorView(&views[i]);

        start[3][1] = end[3][1];
        end[3][1] += 1;
        views[i] = vxCreateTensorView(context, start[3], end[3], (vx_uint8)dims);
        hidden_sub_tensors[i] = vxoTensor_CreateTensorFromView(hidden_fc_out, views[i]);
        if (views[i]) vxReleaseTensorView(&views[i]);

        lstmLayer->base.temp_tensors[tmpTensorIndex++] = input_fc_sub_tensors[i];
        lstmLayer->base.temp_tensors[tmpTensorIndex++] = output_sub_tensors[i];
        lstmLayer->base.temp_tensors[tmpTensorIndex++] = hidden_sub_tensors[i];
    }

    lstmLayer->base.temp_tensors[tmpTensorIndex++] = cell_state;

    enable_format = (vx_bool)(TENSOR_DATA_TYPE(input) != VX_TYPE_FLOAT32);

    shExe_flag = (vx_bool)(enable_format && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)));

    if (!shExe_flag)
    {
        vxnne_lstm_hidden_unit  lstmHiddenUnitNode = &lstmLayer->hidden_units[0];
        /* Initialize state out operation */
        vxnneOperation_Initialize(&lstmHiddenUnitNode->lstm_hidden_unit_operation.base,
            &lstmLayer->base,
            VXNNE_OPERATION_TARGET_SW,
            VXNNE_OPERATOR_LSTM_STATE_OUT,
            vxnneExecuteSWHiddenUnitOut_LSTMLayer,
            NULL,
            1,
            0);

        lstmHiddenUnitNode->lstm_hidden_unit_operation.input_fc_in = input_fc_sub_tensors[0];
        lstmHiddenUnitNode->lstm_hidden_unit_operation.hidden_fc_in = NULL;

        lstmHiddenUnitNode->lstm_hidden_unit_operation.output_state_out = NULL;
        lstmHiddenUnitNode->lstm_hidden_unit_operation.forget_bias = forget_bias;
        lstmHiddenUnitNode->lstm_hidden_unit_operation.cell_state = cell_state;
        lstmHiddenUnitNode->lstm_hidden_unit_operation.output = output_sub_tensors[0];

        lstmHiddenUnitNode->lstm_hidden_unit_operation.batch = batch;
        lstmHiddenUnitNode->lstm_hidden_unit_operation.output_num = output_num;
        lstmHiddenUnitNode->lstm_hidden_unit_operation.enable_cell_in = vx_false_e;

        lstmHiddenUnitNode->lstm_hidden_unit_operation.enable_packed_mode = enable_packed_mode;
        vxnneLayer_SetOperation(
            &lstmLayer->base,
            &lstmHiddenUnitNode->lstm_hidden_unit_operation.base,
            operationIndex++);

        vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_operation.base, (vx_reference)input_fc_sub_tensors[0], VXNNE_OPERATION_REFENRENCE_INPUT);

        if (forget_bias)vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_operation.base, (vx_reference)forget_bias, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_operation.base, (vx_reference)cell_state, VXNNE_OPERATION_REFENRENCE_INPUT);
        if (output)vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_operation.base, (vx_reference)output_sub_tensors[0], VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }
    else
    {
        vxnne_shader_executable shaderExecutable = VX_NULL;
        vxnne_lstm_hidden_unit  lstmHiddenUnitNode = &lstmLayer->hidden_units[0];

        if (enable_packed_mode)
            shaderExecutable = vxnneGetLSTMUnitHiddenOut_PackedShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_LSTMUNIT_HIDDENOUT, &node->kernelAttributes.borderMode, vx_false_e, input_fc_sub_tensors[0], forgetBias, cell_state, output_sub_tensors[0], NULL, NULL, NULL);

        else
            shaderExecutable = vxnneGetLSTMUnitHiddenOutExtShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_LSTMUNIT_HIDDENOUT_EXT, &node->kernelAttributes.borderMode, vx_false_e, input_fc_sub_tensors[0], forgetBias, cell_state, output_sub_tensors[0], NULL, NULL, NULL);


        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&lstmHiddenUnitNode->lstm_hidden_unit_sh_operation,
            &lstmLayer->base,
            VXNNE_OPERATOR_LSTM_STATE_OUT,
            batchCount,
            shaderExecutable);

        if (status != VX_SUCCESS)
            goto exit;

        vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_sh_operation.base, (vx_reference)input_fc_sub_tensors[0], VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_sh_operation.base, (vx_reference)cell_state, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_sh_operation.base, (vx_reference)output_sub_tensors[0], VXNNE_OPERATION_REFENRENCE_OUTPUT);

        vxnneLayer_SetOperation(
            &lstmLayer->base,
            &lstmHiddenUnitNode->lstm_hidden_unit_sh_operation.base,
            operationIndex++);
    }

    biasSize[0] = TENSOR_VIEW_SIZE_INDEX(w_h, 1);
    biasSize[1] = 1;

    gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
    tensor_create_params.num_of_dims = dims;
    tensor_create_params.sizes = biasSize;
    tensor_create_params.data_format = biasDataFormat;
    tensor_create_params.quant_format = TENSOR_QUANT_TYPE(output);
    if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
    {
        tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(output) + TENSOR_POS(w_h);
    }
    else
    {
        tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(output) * TENSOR_TF_SCALE(w_h);
        tensor_create_params.quant_data.affine.zeroPoint = 0;
    }

    bias_zero = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);

    vxoTensor_AllocateMemory(bias_zero);
    vxoTensor_GetTensorViewMemory(bias_zero, (vx_ptr_ptr)&biasData, VX_NULL);
    vxoTensor_GetTensorElementCount(bias_zero, &bias_size);
    memset(biasData, 0, bias_size * vxnneGetTypeSize(biasDataFormat));

    lstmLayer->base.temp_tensors[tmpTensorIndex++] = bias_zero;

    /* Create weights_biases for recurrent FC. */
    recurrent_fc_input = output_sub_tensors[0];
    recurrent_fc_output = hidden_sub_tensors[0];

    for (i = 1; i < time_steps; i++)
    {
        vxnne_lstm_hidden_unit lstmHiddenUnitNode = &lstmLayer->hidden_units[i];

        /* Recurrent FC. */
        recurrent_fc_input = output_sub_tensors[i - 1];
        recurrent_fc_output = hidden_sub_tensors[i];

        if (w_h)
        {
            vxmONERROR(vxoFCOperation_Initialize(&lstmHiddenUnitNode->recurrent_fc_operation,
                                                 &lstmLayer->base,
                                                 recurrent_fc_input,
                                                 &weights_biases,
                                                 w_h, /* weights tensor */
                                                 bias_zero, /* biases tensor */
                                                 0,
                                                 0,
                                                 0,
                                                 recurrent_fc_output,
                                                 &operationIndex));
        }
        else
        {
            vxmASSERT("TODO: recurrent weights are not consistent.\n" && 0);
        }

        /* State out. */
        if (!shExe_flag)
        {
            /* Initialize state out operation */
            vxnneOperation_Initialize(&lstmHiddenUnitNode->lstm_hidden_unit_operation.base,
                &lstmLayer->base,
                VXNNE_OPERATION_TARGET_SW,
                VXNNE_OPERATOR_LSTM_STATE_OUT,
                vxnneExecuteSWHiddenUnitOut_LSTMLayer,
                NULL,
                1,
                0);

            lstmHiddenUnitNode->lstm_hidden_unit_operation.input_fc_in = input_fc_sub_tensors[i];
            lstmHiddenUnitNode->lstm_hidden_unit_operation.hidden_fc_in = hidden_sub_tensors[i];
            lstmHiddenUnitNode->lstm_hidden_unit_operation.cell_state = cell_state;

            lstmHiddenUnitNode->lstm_hidden_unit_operation.output_state_out = NULL;
            lstmHiddenUnitNode->lstm_hidden_unit_operation.forget_bias = forget_bias;
            lstmHiddenUnitNode->lstm_hidden_unit_operation.output = output_sub_tensors[i];

            lstmHiddenUnitNode->lstm_hidden_unit_operation.batch = batch;
            lstmHiddenUnitNode->lstm_hidden_unit_operation.output_num = output_num;
            lstmHiddenUnitNode->lstm_hidden_unit_operation.enable_cell_in = vx_true_e;

            lstmHiddenUnitNode->lstm_hidden_unit_operation.enable_packed_mode = enable_packed_mode;

            vxnneLayer_SetOperation(
                &lstmLayer->base,
                &lstmHiddenUnitNode->lstm_hidden_unit_operation.base,
                operationIndex++);

            vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_operation.base, (vx_reference)input_fc_sub_tensors[i], VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_operation.base, (vx_reference)hidden_sub_tensors[i], VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_operation.base, (vx_reference)cell_state, VXNNE_OPERATION_REFENRENCE_INPUT);
            if (forget_bias)vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_operation.base, (vx_reference)forget_bias, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_operation.base, (vx_reference)output_sub_tensors[i], VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }
        else
        {
            vxnne_shader_executable shaderExecutable = VX_NULL;

            if (enable_packed_mode)
                shaderExecutable = vxnneGetLSTMUnitHiddenOut_PackedShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_LSTMUNIT_HIDDENOUT, &node->kernelAttributes.borderMode, vx_false_e, input_fc_sub_tensors[i], forgetBias, cell_state, output_sub_tensors[i], NULL, NULL, hidden_sub_tensors[i]);

            else
                shaderExecutable = vxnneGetLSTMUnitHiddenOutExtShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_LSTMUNIT_HIDDENOUT_EXT, &node->kernelAttributes.borderMode, vx_false_e, input_fc_sub_tensors[i], forgetBias, cell_state, output_sub_tensors[i], NULL, NULL, hidden_sub_tensors[i]);


            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto exit;
            }

            vxmONERROR(vxnneShaderOperation_Initialize(&lstmHiddenUnitNode->lstm_hidden_unit_sh_operation,
                &lstmLayer->base,
                VXNNE_OPERATOR_LSTM_STATE_OUT,
                batchCount,
                shaderExecutable));

            vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_sh_operation.base, (vx_reference)input_fc_sub_tensors[i], VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_sh_operation.base, (vx_reference)hidden_sub_tensors[i], VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_sh_operation.base, (vx_reference)cell_state, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_sh_operation.base, (vx_reference)output_sub_tensors[i], VXNNE_OPERATION_REFENRENCE_OUTPUT);

            vxnneLayer_SetOperation(
                &lstmLayer->base,
                &lstmHiddenUnitNode->lstm_hidden_unit_sh_operation.base,
                operationIndex++);
        }

    }

    if (time_steps >= 1)
    {
        gcoOS_Free(gcvNULL, views);
        gcoOS_Free(gcvNULL, input_fc_sub_tensors);
        gcoOS_Free(gcvNULL, output_sub_tensors);
        gcoOS_Free(gcvNULL, hidden_sub_tensors);
    }

    lstmLayer->base.num_temp_tensors = tmpTensorIndex;
    node->layer = &lstmLayer->base;

    return status;

OnError:
exit:
    if (lstmLayer != NULL)
        gcoOS_Free(NULL, lstmLayer);

    return status;
}
#endif

VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_LSTMLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    vx_int32  index = 0;
    vx_tensor input = (vx_tensor)parameters[index++];
    vx_tensor static_input = (vx_tensor)parameters[index++];
    vx_tensor cont = (vx_tensor)parameters[index++];

    vx_tensor input2input_weight = (vx_tensor)parameters[index++];
    vx_tensor input2forget_weight = (vx_tensor)parameters[index++];
    vx_tensor input2cell_weight = (vx_tensor)parameters[index++];
    vx_tensor input2output_weight = (vx_tensor)parameters[index++];

    vx_tensor recurrent2input_weight = (vx_tensor)parameters[index++];
    vx_tensor recurrent2forget_weight = (vx_tensor)parameters[index++];
    vx_tensor recurrent2cell_weight = (vx_tensor)parameters[index++];
    vx_tensor recurrent2output_weight = (vx_tensor)parameters[index++];

    vx_tensor cell2input_weight = (vx_tensor)parameters[index++];
    vx_tensor cell2forget_weight = (vx_tensor)parameters[index++];
    vx_tensor cell2output_weight = (vx_tensor)parameters[index++];

    vx_tensor layernorm2input_weight = (vx_tensor)parameters[index++];
    vx_tensor layernorm2forget_weight = (vx_tensor)parameters[index++];
    vx_tensor layernorm2cell_weight = (vx_tensor)parameters[index++];
    vx_tensor layernorm2output_weight = (vx_tensor)parameters[index++];

    vx_tensor input_gate_bias = (vx_tensor)parameters[index++];
    vx_tensor forget_gate_bias = (vx_tensor)parameters[index++];
    vx_tensor cell_bias = (vx_tensor)parameters[index++];
    vx_tensor output_gate_bias = (vx_tensor)parameters[index++];

    vx_tensor projection_weight = (vx_tensor)parameters[index++];
    vx_tensor projection_bias = (vx_tensor)parameters[index++];

    vx_tensor activation = (vx_tensor)parameters[index++];
    vx_tensor forget_bias = (vx_tensor)parameters[index++];
    vx_tensor cell_clip = (vx_tensor)parameters[index++];
    vx_tensor proj_clip = (vx_tensor)parameters[index++];
    vx_scalar lstm_layer_type = (vx_scalar)parameters[index++];

    vx_tensor output = (vx_tensor)parameters[index++];

    vx_bool enable_layernorm = ((layernorm2input_weight != VX_NULL) && (layernorm2forget_weight != VX_NULL) && (layernorm2cell_weight != VX_NULL) && (layernorm2output_weight != VX_NULL))?vx_true_e:vx_false_e;

#ifndef NEW_LSTM_LAYER_PATH
    vx_context context = vxGetContext((vx_reference)node);
    vx_uint32 input_dims;
    vx_uint32 time_step;
    vx_uint32 batch = 1;
    vx_enum   inputFormat = TENSOR_DATA_TYPE(input);
    vx_enum   w_xFormat = VX_TYPE_FLOAT16;
    vx_enum   b_xFormat = VX_TYPE_FLOAT16;
    vx_enum   w_hFormat = VX_TYPE_FLOAT16;
    vx_enum   h_tFormat = VX_TYPE_FLOAT16;
    vx_bool   shExe_flag = vx_false_e;
    vx_bool   nneExe_flag = vxnneIsNNSupportFormat(context, input, VX_NULL, VX_NULL);
    vx_bool   tpExe_flag = (vx_bool)(vxnneIsTPSupportFormat(context, input, VX_NULL, VX_NULL) && vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_TP_TRANSPOSE));
    gctUINT   maxComputeUnits;
    vx_uint32 input_num, output_num, cell_num = 0;
    vx_uint32 i;
    vx_tensor w_x = NULL;
    vx_tensor w_h = NULL;
    vx_tensor w_c = NULL;
    vx_tensor bias = NULL;
    vx_tensor w_x_x = NULL;
    vx_tensor *cell_states = NULL;
    vx_tensor *hidden_states = NULL;
    vx_tensor gate_input = NULL;
    vx_uint32 dims;
    vxnne_lstm_layer lstmLayer = NULL;
    vx_uint32 operationIndex = 0;
    vx_uint32 tmpTensorIndex = 0;
    vx_float32 forgetBias = (forget_bias != VX_NULL) ? VX_GET_DATA_FROM_TENSOR(forget_bias, 0) : 0.f;

    vx_tensor wb_inputs[4];
    vx_uint32 sizes[] = { 1,1,1,1 };

    vx_tensor_create_params_t tensor_create_params;
    vx_bool nne_tp_support = vx_true_e;

    /* make compiler happy */
    enable_layernorm = enable_layernorm;

    /* usused variable */
    lstm_layer_type = NULL;
    static_input = NULL;
    cont = NULL;

    input_dims = TENSOR_DIM_NUM(input);
    if (input_dims < 3)
    {
        vxError("LSTM input dims should >=3: at function %s line %d", __FUNCTION__, __LINE__);
        vxmONERROR(VX_FAILURE);
    }

    time_step = TENSOR_VIEW_SIZE_INDEX(input, input_dims - 1);
    batch = TENSOR_VIEW_SIZE_INDEX(input, input_dims - 2);

    if (TENSOR_TYPE_CHECK(input_gate_bias, VX_TYPE_FLOAT16)
        || TENSOR_TYPE_CHECK(forget_gate_bias, VX_TYPE_FLOAT16)
        || TENSOR_TYPE_CHECK(cell_bias, VX_TYPE_FLOAT16)
        || TENSOR_TYPE_CHECK(output_gate_bias, VX_TYPE_FLOAT16)
        )
        nne_tp_support = vx_false_e;


    nneExe_flag = vxnneIsNNSupportFormat(context, input, VX_NULL, output);
    tpExe_flag = (vx_bool)(vxnneIsTPSupportFormat(context, input, VX_NULL, output) && vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_TP_TRANSPOSE));


    /* disable by default */
    nneExe_flag = vx_false_e;

    if (nneExe_flag)
    {
        vx_context context = vxGetContext((vx_reference)node);
        vx_tensor cell_state = NULL;

        output_num = TENSOR_VIEW_SIZE_INDEX(output, 0);

        sizes[0] = output_num;
        sizes[1] = batch;
        dims = 2;

        gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
        tensor_create_params.num_of_dims = dims;
        tensor_create_params.sizes = sizes;
        tensor_create_params.quant_format = ((TENSOR_QUANT_TYPE(output) == VX_QUANT_DYNAMIC_FIXED_POINT) || (TENSOR_QUANT_TYPE(output) == VX_QUANT_AFFINE_SCALE)) ?
            0 : TENSOR_QUANT_TYPE(output);
        tensor_create_params.data_format = ((TENSOR_QUANT_TYPE(output) == VX_QUANT_DYNAMIC_FIXED_POINT) || (TENSOR_QUANT_TYPE(output) == VX_QUANT_AFFINE_SCALE)) ?
            VX_TYPE_FLOAT32 : TENSOR_DATA_TYPE(output);

        cell_state = vxoTensor_CreateTensor(context, node->graph, &tensor_create_params, vx_true_e);

        if (node->layer)
        {
            vxnneLayer_Free(node->layer);
            node->layer = VX_NULL;
        }

        return vxnneExecuteLSTMLayer_NN_TP_LAYER(node,
            input,
            input2input_weight, input2forget_weight, input2cell_weight, input2output_weight,
            recurrent2input_weight, recurrent2forget_weight, recurrent2cell_weight, recurrent2output_weight,
            cell2input_weight, cell2forget_weight, cell2output_weight,
            input_gate_bias, forget_gate_bias, cell_bias, output_gate_bias,
            projection_weight, projection_bias,
            cell_state, activation, forget_bias, cell_clip, proj_clip,
            output, time_step);
    }
    else

        if (nne_tp_support)
        {
            vx_context context = vxGetContext((vx_reference)node);
            vx_tensor_create_params_t tensor_create_params;

            vx_tensor cell_state_in;
            vx_tensor output_state_in;

            vx_uint8_ptr cell_state_in_base = VX_NULL, output_state_in_base = VX_NULL;
            vx_uint32 state_size = 0;

            gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
            tensor_create_params.num_of_dims = 2;
            tensor_create_params.sizes = TENSOR_SIZES(output);
            tensor_create_params.data_format = TENSOR_DATA_TYPE(output);
            tensor_create_params.quant_format = TENSOR_QUANT_TYPE(output);
            if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
            {
                tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(output);
            }
            else
            {
                tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(output);
                tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(output);
            }

            cell_state_in = vxoTensor_CreateTensor(context, node->graph, &tensor_create_params, vx_false_e);

            output_state_in = vxoTensor_CreateTensor(context, node->graph, &tensor_create_params, vx_false_e);

            vxoTensor_AllocateMemory(cell_state_in);
            vxoTensor_AllocateMemory(output_state_in);

            vxoTensor_GetTensorViewMemory(cell_state_in, (vx_ptr_ptr)&cell_state_in_base, VX_NULL);
            vxoTensor_GetTensorViewMemory(output_state_in, (vx_ptr_ptr)&output_state_in_base, VX_NULL);

            vxoTensor_GetTensorElementCount(cell_state_in, &state_size);
            memset(cell_state_in_base, 0, state_size);

            vxoTensor_GetTensorElementCount(output_state_in, &state_size);
            memset(output_state_in_base, 0, state_size);

            if (node->layer)
            {
                vxnneLayer_Free(node->layer);
                node->layer = VX_NULL;
            }

            status = vxnneExecuteLSTM_NN_TP_LAYER(node,
                input,
                input2input_weight, input2forget_weight, input2cell_weight, input2output_weight,
                recurrent2input_weight, recurrent2forget_weight, recurrent2cell_weight, recurrent2output_weight,
                cell2input_weight, cell2forget_weight, cell2output_weight,
                input_gate_bias, forget_gate_bias, cell_bias, output_gate_bias,
                projection_weight, projection_bias,
                cell_state_in, output_state_in,
                activation, forget_bias, cell_clip, proj_clip,
                cell_state_in, output_state_in, output,
                time_step);

            {
                vxnne_lstm_layer_s* lstmlayer = (vxnne_lstm_layer_s*)node->layer;
                lstmlayer->base.temp_tensors[lstmlayer->base.num_temp_tensors++] = cell_state_in;
                lstmlayer->base.temp_tensors[lstmlayer->base.num_temp_tensors++] = output_state_in;
            }

            return status;
        }
        else
        {
            vx_bool  enable_format = vx_false_e;

            if (TENSOR_DATA_TYPE(input2forget_weight) == VX_TYPE_INT16)
            {
                nneExe_flag = vxnneIsNNSupportFormat(context, input2forget_weight, VX_NULL, VX_NULL);
                tpExe_flag = (vx_bool)(vxnneIsTPSupportFormat(context, input2forget_weight, VX_NULL, VX_NULL) && vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_TP_TRANSPOSE));
            }

            input_num = 1;
            for (i = 0; i < input_dims - 2; i++)
            {
                input_num = input_num * TENSOR_VIEW_SIZE_INDEX(input, i);
            }

            output_num = TENSOR_VIEW_SIZE_INDEX(output, 0);

            if (cell2input_weight)
                cell_num = TENSOR_VIEW_SIZE_INDEX(cell2input_weight, 0);

            /* destroy the existing layer */
            if (node->layer)
            {
                vxnneLayer_Free(node->layer);
                node->layer = VX_NULL;
            }

            gcoOS_Allocate(gcvNULL, sizeof(vxnne_lstm_layer_s), (gctPOINTER*)&lstmLayer);
            if (!lstmLayer)
            {
                status = VX_ERROR_NO_MEMORY;
                vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
                goto exit;
            }
            gcoOS_ZeroMemory(lstmLayer, sizeof(vxnne_lstm_layer_s));

            vxnneLayer_Initialize(&lstmLayer->base,
                "_LSTM_Layer",
                node,
                vxmOPERATION_COUNT(lstmLayer),
                lstmLayer->operations,
                VX_NULL);

            /* Concat input weights. */
            sizes[0] = input_num;
            sizes[1] = output_num * 4;
            dims = 2;
            wb_inputs[0] = input2input_weight;
            wb_inputs[1] = input2forget_weight;
            wb_inputs[2] = input2cell_weight;
            wb_inputs[3] = input2output_weight;

            gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
            tensor_create_params.num_of_dims = dims;
            tensor_create_params.sizes = sizes;
            tensor_create_params.data_format = TENSOR_DATA_TYPE(input2forget_weight);
            tensor_create_params.quant_format = TENSOR_QUANT_TYPE(input2forget_weight);
            if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
            {
                tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(input2forget_weight);
            }
            else
            {
                tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(input2forget_weight);
                tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(input2forget_weight);
            }

            w_x = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_false_e);
            if (vxoTensor_AllocateMemory(w_x) != VX_SUCCESS)
            {
                vxError("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
                status = VX_ERROR_NO_MEMORY;
                goto exit;
            }
            lstmLayer->base.temp_tensors[tmpTensorIndex++] = w_x;
            vxnneExecuteSWLstmPreProcessConcat(wb_inputs, 4, w_x);

            /* Concat recurrent weights. */
            sizes[0] = output_num;
            sizes[1] = output_num * 4;
            dims = 2;
            wb_inputs[0] = recurrent2input_weight;
            wb_inputs[1] = recurrent2forget_weight;
            wb_inputs[2] = recurrent2cell_weight;
            wb_inputs[3] = recurrent2output_weight;

            gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
            tensor_create_params.num_of_dims = dims;
            tensor_create_params.sizes = sizes;
            tensor_create_params.data_format = TENSOR_DATA_TYPE(recurrent2input_weight);
            tensor_create_params.quant_format = TENSOR_QUANT_TYPE(recurrent2input_weight);
            if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
            {
                tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(recurrent2input_weight);
            }
            else
            {
                tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(recurrent2input_weight);
                tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(recurrent2input_weight);
            }

            w_h = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_false_e);
            if (vxoTensor_AllocateMemory(w_h) != VX_SUCCESS)
            {
                vxError("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
                status = VX_ERROR_NO_MEMORY;
                goto exit;
            }
            lstmLayer->base.temp_tensors[tmpTensorIndex++] = w_h;
            vxnneExecuteSWLstmPreProcessConcat(wb_inputs, 4, w_h);

            /* Concat cell weights. */
            if (cell_num > 0)
            {
                sizes[0] = cell_num;
                sizes[1] = output_num * 4;
                dims = 2;
                wb_inputs[0] = cell2input_weight;
                wb_inputs[1] = cell2forget_weight;
                wb_inputs[2] = NULL;
                wb_inputs[3] = cell2output_weight;

                gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
                tensor_create_params.num_of_dims = dims;
                tensor_create_params.sizes = sizes;
                tensor_create_params.data_format = TENSOR_DATA_TYPE(cell2input_weight);
                tensor_create_params.quant_format = TENSOR_QUANT_TYPE(cell2input_weight);
                if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
                {
                    tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(cell2input_weight);
                }
                else
                {
                    tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(cell2input_weight);
                    tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(cell2input_weight);
                }
                w_c = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_false_e);
                if (vxoTensor_AllocateMemory(w_c) != VX_SUCCESS)
                {
                    vxError("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
                    status = VX_ERROR_NO_MEMORY;
                    goto exit;
                }
                lstmLayer->base.temp_tensors[tmpTensorIndex++] = w_c;
                vxnneExecuteSWLstmPreProcessConcat(wb_inputs, 4, w_c);
            }

            /* Concat biases. */
            sizes[0] = output_num * 4;
            sizes[1] = 1;
            dims = 2;
            wb_inputs[0] = input_gate_bias;
            wb_inputs[1] = forget_gate_bias;
            wb_inputs[2] = cell_bias;
            wb_inputs[3] = output_gate_bias;

            gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
            tensor_create_params.num_of_dims = dims;
            tensor_create_params.sizes = sizes;
            tensor_create_params.data_format = (nneExe_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)) && TENSOR_DATA_TYPE(input_gate_bias) == VX_TYPE_FLOAT16) ?
                VX_TYPE_FLOAT32 : TENSOR_DATA_TYPE(input_gate_bias);
            tensor_create_params.quant_format = TENSOR_QUANT_TYPE(input_gate_bias);
            if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
            {
                tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(input_gate_bias);
            }
            else
            {
                tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(input_gate_bias);
                tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(input_gate_bias);
            }

            bias = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_false_e);
            if (vxoTensor_AllocateMemory(bias) != VX_SUCCESS)
            {
                vxError("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
                status = VX_ERROR_NO_MEMORY;
                goto exit;
            }
            lstmLayer->base.temp_tensors[tmpTensorIndex++] = bias;
            vxnneExecuteSWLstmPreProcessConcat(wb_inputs, 4, bias);

            sizes[0] = output_num * 4;
            sizes[1] = batch;
            sizes[2] = time_step;
            dims = 3;

            gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
            tensor_create_params.num_of_dims = dims;
            tensor_create_params.sizes = sizes;
            tensor_create_params.data_format = TENSOR_DATA_TYPE(input);
            tensor_create_params.quant_format = TENSOR_QUANT_TYPE(bias);
            if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
            {
                if (TENSOR_DATA_TYPE(input) == VX_TYPE_INT16)
                {
                    tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(bias) - LSTM_FC_OUTPUT_SUB_FL_DFP16;
                }
                else if (TENSOR_DATA_TYPE(input) == VX_TYPE_INT8)
                {
                    tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(bias) - LSTM_FC_OUTPUT_SUB_FL_DFP8;
                }
                else
                {
                    tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(bias);
                }
            }
            else
            {
                tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(bias);
                tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(bias);
            }

            w_x_x = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);
            lstmLayer->base.temp_tensors[tmpTensorIndex++] = w_x_x;

            w_xFormat = TENSOR_DATA_TYPE(w_x);
            b_xFormat = TENSOR_DATA_TYPE(input_gate_bias);
            w_hFormat = TENSOR_DATA_TYPE(w_h);
            h_tFormat = TENSOR_DATA_TYPE(output);

            gcoHAL_QueryShaderCaps(gcvNULL,
                gcvNULL,
                gcvNULL,
                gcvNULL,
                gcvNULL,
                &maxComputeUnits,
                gcvNULL,
                gcvNULL,
                gcvNULL);

            enable_format = (vx_bool)(
                (inputFormat == VX_TYPE_FLOAT16 && w_xFormat == inputFormat && w_hFormat == inputFormat && h_tFormat == inputFormat) ||
                ((inputFormat == VX_TYPE_INT16 || inputFormat == VX_TYPE_INT8) && w_xFormat == inputFormat && w_hFormat == inputFormat && h_tFormat == inputFormat && b_xFormat == VX_TYPE_INT32));
            shExe_flag = (vx_bool)(enable_format && output_num <= 1024 && batch == 1);

            if (shExe_flag == vx_false_e || (!vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
            {
                /* allocate tensors for cell state */
                gcoOS_Allocate(gcvNULL, (time_step + 1) * sizeof(vx_tensor), (gctPOINTER*)&cell_states);
                if (!cell_states)
                {
                    status = VX_ERROR_NO_MEMORY;
                    vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
                    return status;
                }
                gcoOS_ZeroMemory(cell_states, (time_step + 1) * sizeof(vx_tensor));

                /* cell_states[i] is [batch, output_num] */
                sizes[0] = output_num;
                sizes[1] = batch;
                dims = 2;

                gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
                tensor_create_params.num_of_dims = dims;
                tensor_create_params.sizes = sizes;
                tensor_create_params.data_format = (TENSOR_QUANT_TYPE(output) == VX_QUANT_DYNAMIC_FIXED_POINT) ? VX_TYPE_FLOAT32 : TENSOR_DATA_TYPE(output);
                tensor_create_params.quant_format = TENSOR_QUANT_TYPE(output);
                if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
                {
                    tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(output);
                }
                else
                {
                    tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(output);
                    tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(output);
                }

                for (i = 0; i <= time_step; ++i)
                {
                    cell_states[i] = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);
                    lstmLayer->base.temp_tensors[tmpTensorIndex++] = cell_states[i];
                }

                /* allocate tensors for hidden state */
                gcoOS_Allocate(gcvNULL, (time_step + 1) * sizeof(vx_tensor), (gctPOINTER*)&hidden_states);
                if (!hidden_states)
                {
                    status = VX_ERROR_NO_MEMORY;
                    vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
                    return status;
                }
                gcoOS_ZeroMemory(hidden_states, (time_step + 1) * sizeof(vx_tensor));

                /* hidden_states[i] is [batch, output_num] */
                sizes[0] = output_num;
                sizes[1] = batch;
                dims = 2;

                gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
                tensor_create_params.num_of_dims = dims;
                tensor_create_params.sizes = sizes;
                tensor_create_params.data_format = TENSOR_DATA_TYPE(output);
                tensor_create_params.quant_format = TENSOR_QUANT_TYPE(output);
                if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
                {
                    tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(output);
                }
                else
                {
                    tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(output);
                    tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(output);
                }
                for (i = 0; i <= time_step; ++i)
                {
                    hidden_states[i] = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);
                    lstmLayer->base.temp_tensors[tmpTensorIndex++] = hidden_states[i];
                }

                /* gate_input is [batch, output_num * 4] */


                sizes[0] = output_num * 4;
                sizes[1] = batch;
                dims = 2;

                gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
                tensor_create_params.num_of_dims = dims;
                tensor_create_params.sizes = sizes;
                tensor_create_params.data_format = TENSOR_DATA_TYPE(output);
                tensor_create_params.quant_format = TENSOR_QUANT_TYPE(output);
                if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
                {
                    if (TENSOR_DATA_TYPE(output) == VX_TYPE_INT16)
                    {
                        tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(output) + TENSOR_POS(w_h) - LSTM_FC_OUTPUT_SUB_FL_DFP16;
                    }
                    else if (TENSOR_DATA_TYPE(output) == VX_TYPE_INT8)
                    {
                        tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(output) + TENSOR_POS(w_h) - LSTM_FC_OUTPUT_SUB_FL_DFP8;
                    }
                    else
                    {
                        tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(output);
                    }
                }
                else
                {
                    tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(output);
                    tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(output);
                }

                gate_input = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);
                lstmLayer->base.temp_tensors[tmpTensorIndex++] = gate_input;
            }

            if (shExe_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
            {
                vxnne_shader_executable shaderExecutable;

                if (nneExe_flag)
                {
                    vx_tensor    input_perm = NULL;
                    vx_tensor    input_conv = NULL;
                    vx_tensor    weights_conv = NULL;
                    vx_tensor    output_conv = NULL;
                    vx_tensor    output_perm = NULL;
                    vx_tensor    perm0 = NULL;
                    vx_tensor    perm1 = NULL;
                    vx_uint32    *perm_ptr = NULL;
                    vx_uint32    dims = TENSOR_DIM_NUM(input);
                    vx_uint32    batch = TENSOR_VIEW_SIZE_INDEX(input, dims - 2);
                    vx_uint32    time_steps = TENSOR_VIEW_SIZE_INDEX(input, dims - 1);
                    vx_uint32    sizes[] = { 1, 1, 1, 1 };
                    vx_weights_biases_parameter    weights_biases = NULL;
                    vx_tensor_create_params_t tensor_create_params;
                    vx_bool     enable_sw_convolution = vx_false_e;
                    vx_bool enable_nn_1x2 = vx_false_e;
                    {
                        enable_nn_1x2 = (vx_bool)(input_num % 2 == 0);
                    }


                    gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));

                    tensor_create_params.data_format = TENSOR_DATA_TYPE(input);
                    tensor_create_params.quant_format = TENSOR_QUANT_TYPE(input);
                    if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
                    {
                        tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(input);
                    }
                    else
                    {
                        tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(input);
                        tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(input);
                    }

                    if (enable_nn_1x2)
                    {
                        sizes[0] = input_num >> 1;
                        sizes[1] = 2;
                        sizes[2] = time_steps * batch;
                        sizes[3] = 1;
                        dims = 3;
                        input_perm = vxoTensor_ReshapeTensor(input, (vx_int32*)sizes, dims);

                        sizes[0] = time_steps * batch;
                        sizes[1] = 2;
                        sizes[2] = input_num >> 1;
                        dims = 3;
                    }
                    else
                    {
                        sizes[0] = input_num;
                        sizes[1] = 1;
                        sizes[2] = time_steps * batch;
                        sizes[3] = 1;
                        dims = 3;
                        input_perm = vxoTensor_ReshapeTensor(input, (vx_int32*)sizes, dims);

                        sizes[0] = time_steps * batch;
                        sizes[1] = 1;
                        sizes[2] = input_num;
                        dims = 3;
                    }

                    tensor_create_params.num_of_dims = dims;
                    tensor_create_params.sizes = sizes;

                    input_conv = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);

                    sizes[0] = 4;
                    sizes[1] = 1;
                    sizes[2] = 1;
                    dims = 3;

                    gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
                    tensor_create_params.num_of_dims = dims;
                    tensor_create_params.sizes = sizes;
                    tensor_create_params.data_format = VX_TYPE_INT32;
                    tensor_create_params.quant_format = VX_QUANT_DYNAMIC_FIXED_POINT;

                    perm0 = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_false_e);
                    if (vxoTensor_AllocateMemory(perm0) != VX_SUCCESS)
                    {
                        vxError("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
                        status = VX_ERROR_NO_MEMORY;
                        goto exit;
                    }

                    perm_ptr = (vx_uint32 *)TENSOR_LOGICAL_ADDR(perm0);
                    perm_ptr[0] = 2;
                    perm_ptr[1] = 1;
                    perm_ptr[2] = 0;

                    // permute input
                    if (tpExe_flag)
                    {
                        vx_op_param_s conv = { 0 };
                        vx_tp_value_cmd values;
                        vx_uint32 dnum = 3;

                        status = vxnneOperation_Initialize(&lstmLayer->tensor_trans0_tp_operation.base,
                            &lstmLayer->base,
                            VXNNE_OPERATION_TARGET_TP,
                            VXNNE_OPERATOR_TENSOR_TRANS,
                            VX_NULL,
                            vxnneOperation_TP_Deinitialize,
                            batch,
                            0);
                        if (status != VX_SUCCESS) goto exit;

                        vxnneLayer_SetOperation(
                            &lstmLayer->base,
                            &lstmLayer->tensor_trans0_tp_operation.base,
                            operationIndex++);

                        lstmLayer->tensor_trans0_tp_operation.input = input_perm;
                        lstmLayer->tensor_trans0_tp_operation.output = input_conv;

                        vxnneOperation_AddReference(&lstmLayer->tensor_trans0_tp_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
                        vxnneOperation_AddReference(&lstmLayer->tensor_trans0_tp_operation.base, (vx_reference)input_conv, VXNNE_OPERATION_REFENRENCE_OUTPUT);

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
                        conv.other_ref = (vx_reference)input_perm;
                        conv.data_buff = gcvNULL;

                        lstmLayer->tensor_trans0_tp_operation.base.parameter.tp_value = (vx_tp_value_cmd)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
                        values = (vx_tp_value_cmd)lstmLayer->tensor_trans0_tp_operation.base.parameter.tp_value;
                        values->u32[0] = dnum;
                        values->p8[0] = (vx_uint8_ptr)vxAllocateAndZeroMemory(sizeof(vx_uint32) * dnum);
                        vxMemCopy(values->p8[0], TENSOR_LOGICAL_ADDR(perm0), sizeof(vx_uint32) * dnum);

                        vxMemCopy(&lstmLayer->tensor_trans0_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));
                    }
                    else
                    {
                        vxnne_shader_executable shaderExecutable = VX_NULL;

                        shaderExecutable = vxnneTensorTransposeShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_TRANSPOSE, &node->kernelAttributes.borderMode, input_perm, perm_ptr, 3, input_conv);
                        if (!shaderExecutable)
                        {
                            status = VX_FAILURE;
                            goto exit;
                        }

                        status = vxnneShaderOperation_Initialize(&lstmLayer->tensor_trans0_shader_operation,
                            &lstmLayer->base,
                            VXNNE_OPERATOR_TENSOR_TRANS,
                            batch,
                            shaderExecutable);

                        if (status != VX_SUCCESS)
                            goto exit;

                        vxnneOperation_AddReference(&lstmLayer->tensor_trans0_shader_operation.base, (vx_reference)input_perm, VXNNE_OPERATION_REFENRENCE_INPUT);
                        vxnneOperation_AddReference(&lstmLayer->tensor_trans0_shader_operation.base, (vx_reference)input_conv, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                        vxnneLayer_SetOperation(
                            &lstmLayer->base,
                            &lstmLayer->tensor_trans0_shader_operation.base,
                            operationIndex++);
                    }

                    /* convolution implement*/
                    if (enable_nn_1x2)
                    {
                        vx_tensor weight_in = NULL;
                        vx_uint32 perm[4] = { 2, 1, 0, 3 };
                        vx_uint32 pnum = 4;
                        vx_uint8_ptr inaddr, outaddr;
                        vx_uint32 _dims[VX_CONTEXT_TENSOR_MAX_DIMENSION], strides[VX_CONTEXT_TENSOR_MAX_DIMENSION], tstrides[VX_CONTEXT_TENSOR_MAX_DIMENSION];

                        sizes[0] = input_num >> 1;
                        sizes[1] = 2;
                        sizes[2] = 1;
                        sizes[3] = output_num * 4;
                        dims = 4;
                        weight_in = vxoTensor_ReshapeTensor(w_x, (vx_int32*)sizes, dims);

                        sizes[0] = 1;
                        sizes[1] = 2;
                        sizes[2] = input_num >> 1;
                        sizes[3] = output_num * 4;
                        dims = 4;

                        gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
                        tensor_create_params.num_of_dims = dims;
                        tensor_create_params.sizes = sizes;
                        tensor_create_params.data_format = TENSOR_DATA_TYPE(w_x);
                        tensor_create_params.quant_format = TENSOR_QUANT_TYPE(w_x);
                        if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
                        {
                            tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(w_x);
                        }
                        else
                        {
                            tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(w_x);
                            tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(w_x);
                        }

                        weights_conv = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);
                        if (vxoTensor_AllocateMemory(weights_conv) != VX_SUCCESS)
                        {
                            vxError("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
                            status = VX_ERROR_NO_MEMORY;
                            goto exit;
                        }

                        vxoTensor_GetTensorViewMemory(weight_in, (gctPOINTER*)&inaddr, VX_NULL);
                        vxoTensor_GetTensorViewMemory(weights_conv, (gctPOINTER*)&outaddr, VX_NULL);

                        vxoTensor_GetTensorDimStride(weight_in, &pnum, _dims, strides);
                        vxoTensor_GetTensorDimStride(weights_conv, &pnum, VX_NULL, tstrides);


                        _TransposeTensor(inaddr, outaddr, TENSOR_DATA_SIZE(weight_in), _dims, strides, tstrides, perm, pnum - 1);

                        lstmLayer->base.temp_tensors[tmpTensorIndex++] = weight_in;
                    }
                    else
                    {
                        sizes[0] = 1;
                        sizes[1] = 1;
                        sizes[2] = input_num;
                        sizes[3] = output_num * 4;
                        dims = 4;
                        weights_conv = vxoTensor_ReshapeTensor(w_x, (vx_int32*)sizes, dims);
                    }

                    sizes[0] = time_steps * batch;
                    sizes[1] = 1;
                    sizes[2] = output_num * 4;
                    sizes[3] = 1;
                    dims = 3;

                    gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
                    tensor_create_params.num_of_dims = dims;
                    tensor_create_params.sizes = sizes;
                    tensor_create_params.data_format = TENSOR_DATA_TYPE(w_x_x);
                    tensor_create_params.quant_format = TENSOR_QUANT_TYPE(w_x_x);
                    if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
                    {
                        tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(w_x_x);
                    }
                    else
                    {
                        tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(w_x_x);
                        tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(w_x_x);
                    }

                    output_conv = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);
                    if (enable_sw_convolution)
                    {
                        vx_int32  pad = 0;
                        vx_int32  dilation = 0;
                        vx_int32  stride = 1;
                        vx_enum downScaleSizeRoundingValue = VX_NN_DS_SIZE_ROUNDING_FLOOR;
                        vx_scalar padXLeft = vxCreateScalar(node->base.context, VX_TYPE_INT32, &pad);
                        vx_scalar padXRight = vxCreateScalar(node->base.context, VX_TYPE_INT32, &pad);
                        vx_scalar padYTop = vxCreateScalar(node->base.context, VX_TYPE_INT32, &pad);
                        vx_scalar padYBottom = vxCreateScalar(node->base.context, VX_TYPE_INT32, &pad);
                        vx_scalar strideX = vxCreateScalar(node->base.context, VX_TYPE_INT32, &stride);
                        vx_scalar strideY = vxCreateScalar(node->base.context, VX_TYPE_INT32, &stride);
                        vx_scalar dilationX = vxCreateScalar(node->base.context, VX_TYPE_INT32, &dilation);
                        vx_scalar dilationY = vxCreateScalar(node->base.context, VX_TYPE_INT32, &dilation);
                        vx_scalar downScaleSizeRounding = vxCreateScalar(node->base.context, VX_TYPE_INT32, &downScaleSizeRoundingValue);
                        vxnneOperation_Initialize(&lstmLayer->lstm_conv_sw_operation.base,
                            &lstmLayer->base,
                            VXNNE_OPERATION_TARGET_SW,
                            VXNNE_OPERATOR_CONVOLUTION,
                            vxnneExecuteSWConvolution,
                            VX_NULL,
                            batch,
                            0);

                        vxnneLayer_SetOperation(
                            &lstmLayer->base,
                            &lstmLayer->lstm_conv_sw_operation.base,
                            operationIndex++);

                        lstmLayer->lstm_conv_sw_operation.inputs = input_conv;
                        lstmLayer->lstm_conv_sw_operation.weights = weights_conv;
                        lstmLayer->lstm_conv_sw_operation.biases = bias;
                        lstmLayer->lstm_conv_sw_operation.padX = padXLeft;
                        lstmLayer->lstm_conv_sw_operation.padXRight = padXRight;
                        lstmLayer->lstm_conv_sw_operation.padY = padYTop;
                        lstmLayer->lstm_conv_sw_operation.padYBottom = padYBottom;
                        lstmLayer->lstm_conv_sw_operation.strideX = strideX;
                        lstmLayer->lstm_conv_sw_operation.strideY = strideY;
                        lstmLayer->lstm_conv_sw_operation.dilationX = dilationX;
                        lstmLayer->lstm_conv_sw_operation.dilationY = dilationY;
                        lstmLayer->lstm_conv_sw_operation.downScaleSizeRounding = downScaleSizeRounding;
                        lstmLayer->lstm_conv_sw_operation.outputs = output_conv;

                        vxnneOperation_AddReference(&lstmLayer->lstm_conv_sw_operation.base, (vx_reference)input_conv, VXNNE_OPERATION_REFENRENCE_INPUT);
                        vxnneOperation_AddReference(&lstmLayer->lstm_conv_sw_operation.base, (vx_reference)weights_conv, VXNNE_OPERATION_REFENRENCE_INPUT);
                        vxnneOperation_AddReference(&lstmLayer->lstm_conv_sw_operation.base, (vx_reference)bias, VXNNE_OPERATION_REFENRENCE_INPUT);
                        vxnneOperation_AddReference(&lstmLayer->lstm_conv_sw_operation.base, (vx_reference)output_conv, VXNNE_OPERATION_REFENRENCE_OUTPUT);
                    }
                    else
                    {
                        vx_op_param_s conv = { 0 };
                        weights_biases = _createWeightsBiasesParameterFromTensors(node->base.context, VX_NN_CONVOLUTION_LAYER, input_conv->dims, input_conv->dimCount, output_conv->dimCount,
                            0, 0, 0, 0, 0, 0, 0, 0, VX_NN_DS_SIZE_ROUNDING_FLOOR, output_conv->dims, output_conv->dims, VX_NULL, TENSOR_DATA_TYPE(output_conv), 0, VX_TENSOR_RANK_WHCN,
                            weights_conv, bias, vx_false_e);

                        status = vxnneOperation_Initialize(&lstmLayer->lstm_nn_operation.base,
                            &lstmLayer->base,
                            VXNNE_OPERATION_TARGET_NN,
                            VXNNE_OPERATOR_CONVOLUTION,
                            VX_NULL,
                            vxnneLSTMLayerNNOperation_Deinitialize,
                            batch,
                            NNE_COMMAND_SIZE * weights_biases->slice_num);

                        if (status != VX_SUCCESS) goto exit;

                        memset(&conv, 0, sizeof(vx_op_param_s));

                        vxnneLayer_SetOperation(
                            &lstmLayer->base,
                            &lstmLayer->lstm_nn_operation.base,
                            operationIndex++);

                        lstmLayer->lstm_nn_operation.inputs = input_conv;
                        lstmLayer->lstm_nn_operation.orig_inputs = input_conv;
                        lstmLayer->lstm_nn_operation.weights_biases = weights_biases;
                        lstmLayer->lstm_nn_operation.outputs = output_conv;

                        vxnneOperation_AddReference(&lstmLayer->lstm_nn_operation.base, (vx_reference)input_conv, VXNNE_OPERATION_REFENRENCE_INPUT);
                        vxnneOperation_AddReference(&lstmLayer->lstm_nn_operation.base, (vx_reference)output_conv, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                        {
                            conv.pad_x_left = conv.pad_x_right = conv.pad_y_top = conv.pad_y_bottom = 0;
                            conv.pad_mode = VX_PAD_CONSTANT;
                            conv.pad_const = 0;
                            conv.pool_type = 0;
                            conv.pool_size_x = conv.pool_size_y = 0;
                            conv.conv_rounding_type = VX_NN_DS_SIZE_ROUNDING_FLOOR;
                            conv.enable_relu = vx_false_e;

                            memcpy(&lstmLayer->lstm_nn_operation.base.parameter, &conv, sizeof(vx_op_param_s));
                        }

                    }

                    /* permute output */
                    sizes[0] = output_num * 4;
                    sizes[1] = 1;
                    sizes[2] = time_steps * batch;
                    sizes[3] = 1;
                    dims = 3;
                    output_perm = vxoTensor_ReshapeTensor(w_x_x, (vx_int32*)sizes, dims);

                    sizes[0] = 4;
                    sizes[1] = 1;
                    sizes[2] = 1;
                    dims = 3;

                    gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
                    tensor_create_params.num_of_dims = dims;
                    tensor_create_params.sizes = sizes;
                    tensor_create_params.data_format = VX_TYPE_INT32;
                    tensor_create_params.quant_format = VX_QUANT_DYNAMIC_FIXED_POINT;

                    perm1 = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_false_e);
                    if (vxoTensor_AllocateMemory(perm1) != VX_SUCCESS)
                    {
                        vxError("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
                        status = VX_ERROR_NO_MEMORY;
                        goto exit;
                    }

                    perm_ptr = (vx_uint32 *)TENSOR_LOGICAL_ADDR(perm1);
                    perm_ptr[0] = 2;
                    perm_ptr[1] = 1;
                    perm_ptr[2] = 0;

                    if (tpExe_flag)
                    {
                        vx_op_param_s conv = { 0 };
                        vx_tp_value_cmd values;
                        vx_uint32 dnum = 3;

                        status = vxnneOperation_Initialize(&lstmLayer->tensor_trans1_tp_operation.base,
                            &lstmLayer->base,
                            VXNNE_OPERATION_TARGET_TP,
                            VXNNE_OPERATOR_TENSOR_TRANS,
                            VX_NULL,
                            vxnneOperation_TP_Deinitialize,
                            batch,
                            0);
                        if (status != VX_SUCCESS) goto exit;

                        vxnneLayer_SetOperation(
                            &lstmLayer->base,
                            &lstmLayer->tensor_trans1_tp_operation.base,
                            operationIndex++);

                        lstmLayer->tensor_trans1_tp_operation.input = output_conv;
                        lstmLayer->tensor_trans1_tp_operation.output = output_perm;

                        vxnneOperation_AddReference(&lstmLayer->tensor_trans1_tp_operation.base, (vx_reference)output_conv, VXNNE_OPERATION_REFENRENCE_INPUT);
                        vxnneOperation_AddReference(&lstmLayer->tensor_trans1_tp_operation.base, (vx_reference)w_x_x, VXNNE_OPERATION_REFENRENCE_OUTPUT);

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
                        conv.other_ref = (vx_reference)output_conv;
                        conv.data_buff = gcvNULL;

                        lstmLayer->tensor_trans1_tp_operation.base.parameter.tp_value = (vx_tp_value_cmd)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
                        values = (vx_tp_value_cmd)lstmLayer->tensor_trans1_tp_operation.base.parameter.tp_value;
                        values->u32[0] = dnum;
                        values->p8[0] = (vx_uint8_ptr)vxAllocateAndZeroMemory(sizeof(vx_uint32) * dnum);
                        vxMemCopy(values->p8[0], TENSOR_LOGICAL_ADDR(perm1), sizeof(vx_uint32) * dnum);

                        vxMemCopy(&lstmLayer->tensor_trans1_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));
                    }
                    else
                    {
                        vxnne_shader_executable shaderExecutable = VX_NULL;

                        shaderExecutable = vxnneTensorTransposeShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_TRANSPOSE, &node->kernelAttributes.borderMode, output_conv, perm_ptr, 3, output_perm);
                        if (!shaderExecutable)
                        {
                            status = VX_FAILURE;
                            goto exit;
                        }

                        status = vxnneShaderOperation_Initialize(&lstmLayer->tensor_trans1_shader_operation,
                            &lstmLayer->base,
                            VXNNE_OPERATOR_TENSOR_TRANS,
                            batch,
                            shaderExecutable);

                        if (status != VX_SUCCESS)
                            goto exit;

                        vxnneOperation_AddReference(&lstmLayer->tensor_trans1_shader_operation.base, (vx_reference)output_conv, VXNNE_OPERATION_REFENRENCE_INPUT);
                        vxnneOperation_AddReference(&lstmLayer->tensor_trans1_shader_operation.base, (vx_reference)w_x_x, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                        vxnneLayer_SetOperation(
                            &lstmLayer->base,
                            &lstmLayer->tensor_trans1_shader_operation.base,
                            operationIndex++);
                    }

                    lstmLayer->base.temp_tensors[tmpTensorIndex++] = input_perm;
                    lstmLayer->base.temp_tensors[tmpTensorIndex++] = input_conv;
                    lstmLayer->base.temp_tensors[tmpTensorIndex++] = output_conv;
                    lstmLayer->base.temp_tensors[tmpTensorIndex++] = output_perm;
                    lstmLayer->base.temp_tensors[tmpTensorIndex++] = perm0;
                    lstmLayer->base.temp_tensors[tmpTensorIndex++] = perm1;

                    if (weights_conv) vxoTensor_ReleaseTensor(&weights_conv);
                }
                else
                {
                    vx_tensor    input_conv = NULL;
                    vx_uint32    dims = TENSOR_DIM_NUM(input);
                    vx_uint32    batch = TENSOR_VIEW_SIZE_INDEX(input, dims - 2);
                    vx_uint32    time_steps = TENSOR_VIEW_SIZE_INDEX(input, dims - 1);
                    vx_int32     sizes[] = { 1, 1, 1, 1 };

                    sizes[0] = input_num;
                    sizes[1] = time_steps * batch;
                    sizes[2] = 1;
                    sizes[3] = 1;
                    dims = 2;
                    input_conv = vxoTensor_ReshapeTensor(input, sizes, dims);

                    shaderExecutable = vxnneGetFullyConnectedShaderExecutable(node->base.context, VXNNE_KERNEL_FULLYCONNECTED, &node->kernelAttributes.borderMode, input_conv, w_x, bias, VX_NN_ACTIVATION_NONE, w_x_x);

                    if (!shaderExecutable)
                    {
                        status = VX_FAILURE;
                        goto exit;
                    }

                    status = vxnneShaderOperation_Initialize(&lstmLayer->fc2_sh_operation,
                        &lstmLayer->base,
                        VXNNE_OPERATOR_LSTM_LAYER,
                        batch,
                        shaderExecutable);

                    if (status != VX_SUCCESS)
                        return status;

                    vxnneLayer_SetOperation(
                        &lstmLayer->base,
                        &lstmLayer->fc2_sh_operation.base,
                        operationIndex++);

                    vxnneOperation_AddReference(&lstmLayer->fc2_sh_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
                    vxnneOperation_AddReference(&lstmLayer->fc2_sh_operation.base, (vx_reference)w_x, VXNNE_OPERATION_REFENRENCE_INPUT);
                    vxnneOperation_AddReference(&lstmLayer->fc2_sh_operation.base, (vx_reference)bias, VXNNE_OPERATION_REFENRENCE_INPUT);
                    vxnneOperation_AddReference(&lstmLayer->fc2_sh_operation.base, (vx_reference)w_x_x, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                    if (input_conv) vxoTensor_ReleaseTensor(&input_conv);
                }

                shaderExecutable = vxnneLSTMLayerShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_LSTMLAYER, &node->kernelAttributes.borderMode, w_x_x, forgetBias, w_h, time_step, output, maxComputeUnits);

                if (!shaderExecutable)
                {
                    status = VX_FAILURE;
                    goto exit;
                }

                status = vxnneShaderOperation_Initialize(&lstmLayer->lstm_sh_operation,
                    &lstmLayer->base,
                    VXNNE_OPERATOR_LSTM_LAYER,
                    batch,
                    shaderExecutable);

                if (status != VX_SUCCESS)
                    return status;

                vxnneLayer_SetOperation(
                    &lstmLayer->base,
                    &lstmLayer->lstm_sh_operation.base,
                    operationIndex++);

                vxnneOperation_AddReference(&lstmLayer->lstm_sh_operation.base, (vx_reference)w_x_x, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&lstmLayer->lstm_sh_operation.base, (vx_reference)w_h, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&lstmLayer->lstm_sh_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            }
            else
            {
                vxnneOperation_Initialize(&lstmLayer->lstm_layer_sw_operation.base,
                    &lstmLayer->base,
                    VXNNE_OPERATION_TARGET_SW,
                    VXNNE_OPERATOR_LSTM_LAYER,
                    vxnneExecuteSW_LSTMLayer,
                    VX_NULL,
                    batch,
                    0);

                lstmLayer->lstm_layer_sw_operation.input = input;
                lstmLayer->lstm_layer_sw_operation.input_num = input_num;
                lstmLayer->lstm_layer_sw_operation.output = output;
                lstmLayer->lstm_layer_sw_operation.output_num = output_num;
                lstmLayer->lstm_layer_sw_operation.time_step = time_step;
                lstmLayer->lstm_layer_sw_operation.batch = batch;
                lstmLayer->lstm_layer_sw_operation.w_x = w_x;
                lstmLayer->lstm_layer_sw_operation.w_h = w_h;
                lstmLayer->lstm_layer_sw_operation.w_c = w_c;
                lstmLayer->lstm_layer_sw_operation.bias = bias;
                lstmLayer->lstm_layer_sw_operation.w_x_x = w_x_x;
                lstmLayer->lstm_layer_sw_operation.gate_input = gate_input;
                lstmLayer->lstm_layer_sw_operation.forget_bias = forget_bias;
                lstmLayer->hidden_states = hidden_states;
                lstmLayer->cell_states = cell_states;

                vxnneOperation_AddReference(&lstmLayer->lstm_layer_sw_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&lstmLayer->lstm_layer_sw_operation.base, (vx_reference)w_x, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&lstmLayer->lstm_layer_sw_operation.base, (vx_reference)w_h, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&lstmLayer->lstm_layer_sw_operation.base, (vx_reference)w_x_x, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&lstmLayer->lstm_layer_sw_operation.base, (vx_reference)bias, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&lstmLayer->lstm_layer_sw_operation.base, (vx_reference)gate_input, VXNNE_OPERATION_REFENRENCE_INPUT);
                for (i = 0; i <= time_step; i++)
                {
                    vxnneOperation_AddReference(&lstmLayer->lstm_layer_sw_operation.base, (vx_reference)hidden_states[i], VXNNE_OPERATION_REFENRENCE_INPUT);
                    vxnneOperation_AddReference(&lstmLayer->lstm_layer_sw_operation.base, (vx_reference)cell_states[i], VXNNE_OPERATION_REFENRENCE_INPUT);
                }
                vxnneOperation_AddReference(&lstmLayer->lstm_layer_sw_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                vxnneLayer_SetOperation(
                    &lstmLayer->base,
                    &lstmLayer->lstm_layer_sw_operation.base,
                    operationIndex++);
            }

            lstmLayer->base.num_temp_tensors = tmpTensorIndex;

            node->layer = &lstmLayer->base;

            return status;
        OnError:
        exit:
            if (lstmLayer != NULL)
                gcoOS_Free(NULL, lstmLayer);

            return status;
        }
#else
    vxnne_lstm_layer lstm_layer = VX_NULL;

    /* make compiler happy */
    enable_layernorm = enable_layernorm;

    /* usused variable */
    lstm_layer_type = NULL;
    static_input = NULL;
    cont = NULL;
    activation = NULL;
    cell2input_weight = NULL;
    cell2forget_weight = NULL;
    cell2output_weight = NULL;
    projection_weight = NULL;
    proj_clip = NULL;
    cell_clip = NULL;
    projection_bias = NULL;

    vxmONERROR(vxoLSTMLayer_Create(&lstm_layer));

    vxmONERROR(vxoLSTMLayer_Initialize(lstm_layer,
                                       node,
                                       input,
                                       input2input_weight,
                                       input2forget_weight,
                                       input2cell_weight,
                                       input2output_weight,
                                       recurrent2input_weight,
                                       recurrent2forget_weight,
                                       recurrent2cell_weight,
                                       recurrent2output_weight,
                                       cell2input_weight,
                                       cell2forget_weight,
                                       cell2output_weight,
                                       input_gate_bias,
                                       forget_gate_bias,
                                       cell_bias,
                                       output_gate_bias,
                                       projection_weight,
                                       projection_bias,
                                       activation,
                                       forget_bias,
                                       cell_clip,
                                       proj_clip,
                                       output));

    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    node->layer = &lstm_layer->base;

    return VX_SUCCESS;

OnError:
    if (lstm_layer)
    {
        vxoLSTMLayer_Destroy(lstm_layer);
    }

    return status;
#endif
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_LSTMLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
#ifndef NEW_LSTM_LAYER_PATH
    vxnne_lstm_layer lstmLayer = (vxnne_lstm_layer)(node->layer);
    vx_uint32 i = 0;

    /* Release recurrent FC resources. */
    if (lstmLayer->hidden_units)
    {
        for (i = 0; i < lstmLayer->hidden_unit_num; i++)
        {
            vxoFCOperation_Deinitialize(&lstmLayer->hidden_units[i].recurrent_fc_operation);
        }

        vxFree(lstmLayer->hidden_units);
        lstmLayer->hidden_units = VX_NULL;
        lstmLayer->hidden_unit_num = 0;
    }

    if (lstmLayer->cell_states != VX_NULL)
    {
        gcoOS_Free(VX_NULL, lstmLayer->cell_states);
        lstmLayer->cell_states = VX_NULL;
    }

    if (lstmLayer->hidden_states != VX_NULL)
    {
        gcoOS_Free(VX_NULL, lstmLayer->hidden_states);
        lstmLayer->hidden_states = VX_NULL;
    }

    if (lstmLayer->sub_wb != VX_NULL)
    {
        vxReleaseWeightsBiasesParameter(&lstmLayer->sub_wb);
    }

    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }
#else
    vxnne_lstm_layer lstm_layer = (vxnne_lstm_layer)(node->layer);

    if (lstm_layer)
    {
        vxoLSTMLayer_Deinitialize(lstm_layer);
        vxoLSTMLayer_Destroy(lstm_layer);
    }
#endif

    return VX_SUCCESS;
}

/**************************************************************************************************************
*
*                          GRU
*
*************************************************************************************************************/
vx_status vxnneExecuteSWGRUUnit(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NN_GRUUnit(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_GRUUnit_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_GRUUnit_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_GRUUnit_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 index = 0;
    vx_tensor  input_s = (vx_tensor)parameters[index];
    vx_tensor  h_prev_s = (vx_tensor)parameters[index++];

    vx_tensor  reset2input_weights = (vx_tensor)parameters[index++];
    vx_tensor  update2input_weights = (vx_tensor)parameters[index++];
    vx_tensor  connection2input_weights = (vx_tensor)parameters[index++];

    vx_tensor  reset2recurrent_weights = (vx_tensor)parameters[index++];
    vx_tensor  update2recurrent_weights = (vx_tensor)parameters[index++];
    vx_tensor  connection2recurrent_weights = (vx_tensor)parameters[index++];

    vx_tensor  gate_input_bias = (vx_tensor)parameters[index++];
    vx_tensor  gate_recurrent_bias = (vx_tensor)parameters[index++];
    vx_tensor  connection_bias = (vx_tensor)parameters[index++];

    vx_tensor  output_s = (vx_tensor)parameters[index++];

    vx_uint32 batchCount = TENSOR_SIZE_INDEX(output_s, 3);
    vxnne_gru_unit  gru_layer = VX_NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    {
        gcoOS_Allocate(gcvNULL, sizeof(vxnne_gru_unit_s), (gctPOINTER*)&gru_layer);
        if (!gru_layer)
        {
            status = VX_ERROR_NO_MEMORY;
            gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            goto exit;
        }

        gcoOS_ZeroMemory(gru_layer, sizeof(vxnne_gru_unit_s));

        vxnneLayer_Initialize(&gru_layer->base,
            "GRUUnit",
            node,
            vxmOPERATION_COUNT(gru_layer),
            gru_layer->operations,
            VX_NULL);

        vxnneOperation_Initialize(&gru_layer->gru_unit_operation.base,
            &gru_layer->base,
            VXNNE_OPERATION_TARGET_SW,
            VXNNE_OPERATOR_GRU,
            vxnneExecuteSWGRUUnit,
            VX_NULL,
            batchCount,
            0);

        vxnneLayer_SetOperation(
            &gru_layer->base,
            &gru_layer->gru_unit_operation.base,
            0);

        gru_layer->gru_unit_operation.input = input_s;
        gru_layer->gru_unit_operation.state_in = h_prev_s;

        gru_layer->gru_unit_operation.update2input_weights = update2input_weights;
        gru_layer->gru_unit_operation.reset2input_weights = reset2input_weights;
        gru_layer->gru_unit_operation.connection2input_weights = connection2input_weights;

        gru_layer->gru_unit_operation.update2recurrent_weights = update2recurrent_weights;
        gru_layer->gru_unit_operation.reset2recurrent_weights = reset2recurrent_weights;
        gru_layer->gru_unit_operation.connection2recurrent_weights = connection2recurrent_weights;

        gru_layer->gru_unit_operation.gate_input_bias = gate_input_bias;
        gru_layer->gru_unit_operation.gate_recurrent_bias = gate_recurrent_bias;
        gru_layer->gru_unit_operation.connection_bias = connection_bias;

        gru_layer->gru_unit_operation.state_out = h_prev_s;
        gru_layer->gru_unit_operation.output = output_s;

        node->layer = &gru_layer->base;
    }

    return VX_SUCCESS;

exit:
    if (gru_layer)
    {
        gcoOS_Free(VX_NULL, gru_layer);
    }

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_GRUUnit_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}


/**************************************************************************************************************
*
*                          GRU Layer
*
*************************************************************************************************************/
vx_status vxnneExecuteSWGRULayer(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NN_GRULayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_GRULayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_GRULayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_GRULayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_GRULayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

/**************************************************************************************************************
*
*                          ConvLSTM
*
*************************************************************************************************************/
vx_status vxnneExecuteSWConvLSTMUnit(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NN_ConvLSTMUnit(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_ConvLSTMUnit_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_ConvLSTMUnit_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_ConvLSTMUnit_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 index = 0;
    vx_tensor  input_s = (vx_tensor)parameters[index];
    vx_tensor  h_prev_s = (vx_tensor)parameters[index++];

    vx_tensor  input2input_weight = (vx_tensor)parameters[index++];
    vx_tensor  input2cell_weight = (vx_tensor)parameters[index++];
    vx_tensor  input2forget_weight = (vx_tensor)parameters[index++];
    vx_tensor  input2output_weight = (vx_tensor)parameters[index++];

    vx_tensor  recurrent2input_weight = (vx_tensor)parameters[index++];
    vx_tensor  recurrent2cell_weight = (vx_tensor)parameters[index++];
    vx_tensor  recurrent2forget_weight = (vx_tensor)parameters[index++];
    vx_tensor  recurrent2output_weight = (vx_tensor)parameters[index++];

    vx_tensor  input_gate_bias = (vx_tensor)parameters[index++];
    vx_tensor  cell_bias = (vx_tensor)parameters[index++];
    vx_tensor  forget_gate_bias = (vx_tensor)parameters[index++];
    vx_tensor  output_gate_bias = (vx_tensor)parameters[index++];

    vx_tensor  output_s = (vx_tensor)parameters[index++];

    vx_uint32 batchCount = TENSOR_SIZE_INDEX(output_s, 3);
    vxnne_convlstm_unit  convlstm_layer = VX_NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    {
        gcoOS_Allocate(gcvNULL, sizeof(vxnne_convlstm_unit_s), (gctPOINTER*)&convlstm_layer);
        if (!convlstm_layer)
        {
            status = VX_ERROR_NO_MEMORY;
            gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            goto exit;
        }

        gcoOS_ZeroMemory(convlstm_layer, sizeof(vxnne_convlstm_unit_s));

        vxnneLayer_Initialize(&convlstm_layer->base,
            "ConvLSTMUnit",
            node,
            vxmOPERATION_COUNT(convlstm_layer),
            convlstm_layer->operations,
            VX_NULL);

        vxnneOperation_Initialize(&convlstm_layer->convlstm_unit_operation.base,
            &convlstm_layer->base,
            VXNNE_OPERATION_TARGET_SW,
            VXNNE_OPERATOR_CONV_LSTM,
            vxnneExecuteSWConvLSTMUnit,
            VX_NULL,
            batchCount,
            0);

        vxnneLayer_SetOperation(
            &convlstm_layer->base,
            &convlstm_layer->convlstm_unit_operation.base,
            0);

        convlstm_layer->convlstm_unit_operation.input = input_s;
        convlstm_layer->convlstm_unit_operation.state_in = h_prev_s;

        convlstm_layer->convlstm_unit_operation.input2input_weight = input2input_weight;
        convlstm_layer->convlstm_unit_operation.input2cell_weight = input2cell_weight;
        convlstm_layer->convlstm_unit_operation.input2forget_weight = input2forget_weight;
        convlstm_layer->convlstm_unit_operation.input2output_weight = input2output_weight;

        convlstm_layer->convlstm_unit_operation.recurrent2input_weight = recurrent2input_weight;
        convlstm_layer->convlstm_unit_operation.recurrent2cell_weight = recurrent2cell_weight;
        convlstm_layer->convlstm_unit_operation.recurrent2forget_weight = recurrent2forget_weight;
        convlstm_layer->convlstm_unit_operation.recurrent2output_weight = recurrent2output_weight;

        convlstm_layer->convlstm_unit_operation.input_gate_bias = input_gate_bias;
        convlstm_layer->convlstm_unit_operation.cell_bias = cell_bias;
        convlstm_layer->convlstm_unit_operation.forget_gate_bias = forget_gate_bias;
        convlstm_layer->convlstm_unit_operation.output_gate_bias = output_gate_bias;

        convlstm_layer->convlstm_unit_operation.state_out = h_prev_s;
        convlstm_layer->convlstm_unit_operation.output = output_s;

        node->layer = &convlstm_layer->base;
    }

    return VX_SUCCESS;

exit:
    if (convlstm_layer)
    {
        gcoOS_Free(VX_NULL, convlstm_layer);
    }

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_ConvLSTMUnit_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

/**************************************************************************************************************
*
*                          ConvLSTM Layer
*
*************************************************************************************************************/
vx_status vxnneExecuteSWConvLSTMLayer(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NN_ConvLSTMLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_ConvLSTMLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_ConvLSTMLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_ConvLSTMLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_ConvLSTMLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

