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


#include <layers/gc_vx_layer_lstm.h>
#include <gc_vx_nn_util.h>
#include <ops/gc_vx_op_fc.h>

#define NEW_CREATE_API 1

#define FORCE_INTERNAL_FORMAT VX_TYPE_INVALID

extern vx_tensor _createSimilarTensor(vx_graph graph, vx_bool is_virtual, vx_uint32 num_of_dims, vx_uint32 * sizes, vx_tensor tensor);
extern vx_tensor _createTensor(vx_graph graph, vx_bool is_virtual,
    vx_uint32 num_of_dims, vx_uint32 * sizes, vx_enum data_format, vx_enum quant_format,
    vx_int8 fixed_point_pos,
    vx_float32 scale, vx_int32 zeroPoint);

#define LSTM_FC_OUTPUT_SUB_FL_DFP16     (16)
#define LSTM_FC_OUTPUT_SUB_FL_DFP8      (8)
#define LSTM_FC_OUTPUT_ASYM_U8_FL       (10)

vx_status vxoFC_GetConfigFromEnv(vx_enum* force_format, vx_int8_ptr force_dfb, vx_int32* force_zp, vx_float32* force_scale)
{
    vx_uint32 f = 0;
    gctSTRING env = VX_NULL;

    gcoOS_GetEnv(gcvNULL, "LSTM_CONV_INTER_FORMAT", &env);
    if (env != VX_NULL && force_format != VX_NULL)
    {
    gctSTRING formatNameTable[] = { "int16", "int8", "qauant8", "f16", "f32" };
    vx_enum formatTable[] = { VX_TYPE_INT16, VX_TYPE_INT8, VX_TYPE_UINT8, VX_TYPE_FLOAT16, VX_TYPE_FLOAT32 };

        for (f = 0; f < (vx_uint32)gcmCOUNTOF(formatNameTable); f++)
        {
            if (gcoOS_StrStr(env, formatNameTable[f], VX_NULL))
            {
                *force_format = formatTable[f];
                break;
            }
        }
    }

    if ((*force_format == VX_TYPE_INT16) || (*force_format == VX_TYPE_INT8))
    {
        gcoOS_GetEnv(gcvNULL, "LSTM_CONV_DFB", &env);
        if (env != NULL && force_dfb != VX_NULL)*force_dfb = (vx_int8)atoi(env);
    }
    else if (*force_format == VX_TYPE_UINT8)
    {
        gcoOS_GetEnv(gcvNULL, "LSTM_CONV_ZP", &env);
        if (env != NULL && force_zp != VX_NULL)*force_zp = atoi(env);

        gcoOS_GetEnv(gcvNULL, "LSTM_CONV_SCALE", &env);
        if (env != NULL && force_scale != VX_NULL)*force_scale = (vx_float32)atof(env);
    }

    return VX_SUCCESS;
}

 vx_tensor _createInteranlTensor(vx_graph graph, vx_enum force_f, vx_bool is_virtual,
    vx_uint32 num_of_dims, vx_uint32 * sizes, vx_enum data_format, vx_enum quant_format,
    vx_int8 fixed_point_pos,
    vx_float32 scale, vx_int32 zeroPoint)
 {
     vx_tensor tensor           = VX_NULL;

     vx_enum _data_format       = data_format;
     vx_enum _quant_format      = quant_format;
     vx_int8 _fixed_point_pos   = fixed_point_pos;
     vx_float32 _scale          = scale;
     vx_int32 _zeroPoint        = zeroPoint;

     vx_enum format = data_format;

    vx_enum force_format = force_f;

    vxoFC_GetConfigFromEnv(&force_format, &_fixed_point_pos, &_zeroPoint, &_scale);

     if (force_format != VX_TYPE_INVALID && force_format != data_format)
     {
         if (quant_format != VX_QUANT_NONE)
         {
            /* quant to float */
             if (force_format == VX_TYPE_FLOAT16)
                _quant_format = VX_QUANT_NONE;
            /* u8 to int16 */
             else if (force_format == VX_TYPE_INT16 && data_format == VX_TYPE_UINT8)
                _quant_format = VX_QUANT_DYNAMIC_FIXED_POINT;
         }
     }

     switch(_quant_format)
     {
     case VX_QUANT_NONE:
         _data_format       = format == VX_TYPE_BFLOAT16? VX_TYPE_BFLOAT16 : VX_TYPE_FLOAT16;
         _quant_format      = VX_QUANT_NONE;
         _fixed_point_pos   = 0;
         _scale             = 1.f;
         _zeroPoint         = 0;
         break;
     case VX_QUANT_DYNAMIC_FIXED_POINT:
     {
         _scale             = 1.f;
         _zeroPoint         = 0;
         switch(format)
         {
         case VX_TYPE_INT16:
             /* fixed_point_pos = TENSOR_POS(bias) */
            _fixed_point_pos    = fixed_point_pos - LSTM_FC_OUTPUT_SUB_FL_DFP16;
             break;
         case VX_TYPE_INT8:
             /* fixed_point_pos = TENSOR_POS(bias) */
            _fixed_point_pos    = fixed_point_pos - LSTM_FC_OUTPUT_SUB_FL_DFP8;
             break;
         case VX_TYPE_UINT8:
            _data_format        = VX_TYPE_INT16;
            _fixed_point_pos    = LSTM_FC_OUTPUT_ASYM_U8_FL;
             break;
         default:
             break;
         }
     }
     break;
     case VX_QUANT_AFFINE_SCALE:
     default:
         _fixed_point_pos = 0;
         break;
     }

    tensor = _createTensor(graph, is_virtual, num_of_dims, sizes, _data_format, _quant_format, _fixed_point_pos, _scale, _zeroPoint);
     return tensor;
 }

extern vx_status vxnneExecuteSWLstmPreProcessConcat(vx_tensor* inputs, vx_uint32 input_num, vx_tensor output);
extern vx_status vxnneExecuteSWHiddenUnitOut_LSTMLayer(struct _vxnne_operation_s *operation);
extern vx_status vxnneOperation_AddReference(
    vxnne_operation_s*            operation,
    vx_reference                  reference,
    vxnne_operation_reference_e   refType
    );

#define VX_GET_DATA_FROM_TENSOR(tensor, index) \
    vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(tensor), TENSOR_QUANT_TYPE(tensor), index, TENSOR_LOGICAL_ADDR(tensor), TENSOR_POS(tensor), TENSOR_TF_ZEROPOINT(tensor), TENSOR_TF_SCALE(tensor))

#define VX_SAVE_DATA_TO_TENSOR(tensor, data, index) \
    vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(tensor), TENSOR_QUANT_TYPE(tensor), index, data, TENSOR_LOGICAL_ADDR(tensor), TENSOR_POS(tensor), TENSOR_TF_ZEROPOINT(tensor), TENSOR_TF_SCALE(tensor), TENSOR_ROUNDING_MODE(tensor))

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
VX_PRIVATE_API vx_status _ConcatTensors(
    vx_node node,
    vx_tensor *inputs,
    vx_uint32 input_element_num,
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

    for (i = 0; i < input_element_num; i++)
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
                                                  input_element_num,
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

/*
 * Reshape LSTM layer tensors to fit FC operation.
 *
 * The dim layout of LSTM layer tensors:
 *   dims[dim_num - 2]: batch
 *   dims[dim_num - 1]: time_step
 *
 * The FC tensors are always 2-dimentional with the
 *  layout of {#IFM, #batch}.
 */
VX_PRIVATE_API vx_status _ReshapeLSTMTensor4FC(
    vx_tensor input,
    vx_tensor *reshaped
    )
{
    vx_status status = VX_SUCCESS;

    vx_uint32 dim_num = TENSOR_VIEW_DIM_NUM(input);
    vx_uint32 batch_pos = dim_num - 2;
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

vx_status vxoLSTMLayer_Create(
    vxnne_lstm_layer *lstm_layer
    )
{
    vx_status status = VX_SUCCESS;

    vxnne_lstm_layer layer = VX_NULL;

    layer = (vxnne_lstm_layer)vxAllocateAndZeroMemory(gcmSIZEOF(vxnne_lstm_layer_s));
    vxmONERROR_NULLPTR(layer);

    *lstm_layer = layer;

OnError:
    return status;
}

vx_status vxoLSTMLayer_Destroy(
    vxnne_lstm_layer lstm_layer
    )
{
    vx_status status = VX_SUCCESS;

    if (lstm_layer)
    {
        vxFree(lstm_layer);
    }

    return status;
}

vx_status vxoLSTMLayer_Initialize(
    vxnne_lstm_layer lstm_layer,
    vx_node node,
    vx_tensor inputs,
    vx_tensor input2input_weights,
    vx_tensor input2forget_weights,
    vx_tensor input2cell_weights,
    vx_tensor input2output_weights,
    vx_tensor recurrent2input_weights,
    vx_tensor recurrent2forget_weights,
    vx_tensor recurrent2cell_weights,
    vx_tensor recurrent2output_weights,
    vx_tensor cell2input_weights,
    vx_tensor cell2forget_weights,
    vx_tensor cell2output_weights,
    vx_tensor input_gate_biases,
    vx_tensor forget_gate_biases,
    vx_tensor cell_biases,
    vx_tensor output_gate_biases,
    vx_tensor projection_weights,
    vx_tensor projection_biases,
    vx_tensor activation,
    vx_tensor forget_biases,
    vx_tensor cell_clip,
    vx_tensor proj_clip,
    vx_tensor outputs
    )
{
    vx_status status = VX_SUCCESS;

    vx_context context = vxGetContext((vx_reference)node);
    vx_tensor_view *views = VX_NULL;
    vx_tensor *input_fc_sub_tensors = VX_NULL;
    vx_tensor *output_sub_tensors = VX_NULL;
    vx_tensor *hidden_sub_tensors = VX_NULL;
    vx_tensor input_fc_weights = VX_NULL;
    vx_tensor recurrent_fc_weights = VX_NULL;
    vx_tensor bias = VX_NULL;
    vx_tensor bias_zero[4] = { VX_NULL };
    vx_tensor output_reshape = VX_NULL;
    vx_tensor input_fc_weights_array[4] = { VX_NULL };
    vx_tensor recurrent_fc_weights_array[4] = { VX_NULL };
    vx_tensor biases_array[4] = { VX_NULL };
    vx_uint32 op_index = 0;
    vx_uint32 input_element_num = 0;
    vx_uint32 output_element_num = 0;
    vx_uint32 input_dim_num = TENSOR_VIEW_DIM_NUM(inputs);
    vx_uint32 output_dim_num = TENSOR_VIEW_DIM_NUM(outputs);
    vx_uint32 i = 0;
    vx_uint32 dim_num = 0;
    vx_uint32 input_batch_pos = TENSOR_VIEW_DIM_NUM(inputs) - 2;
    vx_uint32 output_batch_pos = TENSOR_VIEW_DIM_NUM(outputs) - 2;
    vx_uint32 sizes[] = { 1, 1, 1, 1 };
    vx_uint32 start[4][4] = { { 0 } };
    vx_uint32 end[4][4] = { { 0 } };
    vx_float32 forget_biases_value = (forget_biases != VX_NULL) ? VX_GET_DATA_FROM_TENSOR(forget_biases, 0) : 0.f;
    vx_bool enable_format = vx_false_e;
    vx_bool shExe_flag = vx_false_e;
    vx_type_e bias_format;
    vx_float32 *bias_data = VX_NULL;
    vx_uint32 bias_size = 0;
    vx_uint32 bias_sizes[4] = { 0 };
    vx_tensor_create_params_t tensor_params;
    vx_weights_biases_parameter weights_biases[4] = { VX_NULL };
    vx_bool   enable_packed_mode = /*TENSOR_DATA_TYPE(inputs) == VX_TYPE_INT8*/ vx_false_e;
    vx_type_e data_format;
    vx_tensor input_fc_input = VX_NULL;
    vx_tensor input_fc_output = VX_NULL;
    vx_tensor recurrent_fc_output = VX_NULL;
    vx_tensor recurrent_fc_sub_input = VX_NULL;
    vx_tensor recurrent_fc_sub_output = VX_NULL;
    vx_uint32 time_steps = TENSOR_VIEW_SIZE_INDEX(inputs, TENSOR_VIEW_DIM_NUM(inputs) - 1);
    vx_uint32 batch = TENSOR_VIEW_SIZE_INDEX(inputs, TENSOR_VIEW_DIM_NUM(inputs) - 2);
    vx_tensor cell_state = VX_NULL;
    vxnne_operation *operations = VX_NULL;
    vxnne_lstm_hidden_unit hidden_units = VX_NULL;
    vxnne_layer layer = &lstm_layer->base;
    vx_uint32 op_num = time_steps * 4 + 3;

    vx_enum force_format = (TENSOR_DATA_TYPE(inputs) == VX_TYPE_UINT8) ?VX_TYPE_INT16:FORCE_INTERNAL_FORMAT;

    if (input_dim_num < 3 || output_dim_num < 3)
    {
        vxError("LSTM input/output dims should >=3 (%s(): %d).", __FUNCTION__, __LINE__);
        vxmONERROR(VX_FAILURE);
    }

    operations = (vxnne_operation *)vxAllocateAndZeroMemory(gcmSIZEOF(vxnne_operation) * op_num);
    vxmONERROR_NULLPTR(operations);
    lstm_layer->operations3 = operations;

    vxmONERROR(vxnneLayer_Initialize(layer,
                                     "_LSTM_Layer",
                                     node,
                                     op_num,
                                     lstm_layer->operations3,
                                     VX_NULL));

    /* Calculate the element number of input and output tensors. */
    input_batch_pos = input_dim_num - 2;
    output_batch_pos = output_dim_num - 2;

    input_element_num = 1;
    for (i = 0; i < input_batch_pos; i++)
    {
        input_element_num = input_element_num * TENSOR_VIEW_SIZE_INDEX(inputs, i);
    }

    output_element_num = 1;
    for (i = 0; i < output_batch_pos; i++)
    {
        output_element_num = output_element_num * TENSOR_VIEW_SIZE_INDEX(outputs, i);
    }

    /* Create cell state tensor. */
    sizes[0] = output_element_num;
    sizes[1] = batch;
    dim_num = 2;

    vxZeroMemory(&tensor_params, gcmSIZEOF(vx_tensor_create_params_t));
    tensor_params.num_of_dims = dim_num;
    tensor_params.sizes = sizes;
    tensor_params.data_format = ((TENSOR_QUANT_TYPE(outputs) == VX_QUANT_DYNAMIC_FIXED_POINT) || (TENSOR_QUANT_TYPE(outputs) == VX_QUANT_AFFINE_SCALE)) ?
            VX_TYPE_FLOAT32 : TENSOR_DATA_TYPE(outputs);
    tensor_params.quant_format = ((TENSOR_QUANT_TYPE(outputs) == VX_QUANT_DYNAMIC_FIXED_POINT) || (TENSOR_QUANT_TYPE(outputs) == VX_QUANT_AFFINE_SCALE)) ?
            0 : TENSOR_QUANT_TYPE(outputs);

    cell_state = vxoTensor_CreateTensor(context, node->graph, &tensor_params, vx_true_e);
    layer->temp_tensors[layer->num_temp_tensors++] = cell_state;

    /* Hidden units. */
    hidden_units = (vxnne_lstm_hidden_unit)vxAllocateAndZeroMemory(gcmSIZEOF(vxnne_lstm_hidden_unit_s) * time_steps);
    vxmONERROR_NULLPTR(hidden_units);
    lstm_layer->hidden_units = hidden_units;
    lstm_layer->hidden_unit_num = time_steps;

    input_fc_weights_array[0] = input2input_weights;
    input_fc_weights_array[1] = input2forget_weights;
    input_fc_weights_array[2] = input2cell_weights;
    input_fc_weights_array[3] = input2output_weights;

    biases_array[0] = input_gate_biases;
    biases_array[1] = forget_gate_biases;
    biases_array[2] = cell_biases;
    biases_array[3] = output_gate_biases;

    recurrent_fc_weights_array[0] = recurrent2input_weights;
    recurrent_fc_weights_array[1] = recurrent2forget_weights;
    recurrent_fc_weights_array[2] = recurrent2cell_weights;
    recurrent_fc_weights_array[3] = recurrent2output_weights;

    /* Concat weights. */
    if ((_IsSameType(input2input_weights, input2forget_weights) &&
         _IsSameType(input2input_weights, input2cell_weights) &&
         _IsSameType(input2input_weights, input2output_weights)) &&
        (_IsSameType(input_gate_biases, forget_gate_biases) &&
         _IsSameType(input_gate_biases, cell_biases) &&
         _IsSameType(input_gate_biases, output_gate_biases)))
    {
        /* Inputs weights. */
        data_format = (vx_type_e)TENSOR_DATA_TYPE(input2input_weights);

        vxmONERROR(_ConcatTensors(node,
                                  input_fc_weights_array,
                                  4,
                                  data_format,
                                  &input_fc_weights));

        layer->temp_tensors[layer->num_temp_tensors++] = input_fc_weights;

        /* Biases. */
        data_format = (vx_type_e)((TENSOR_DATA_TYPE(input_gate_biases) == VX_TYPE_FLOAT16) ? VX_TYPE_FLOAT32 : TENSOR_DATA_TYPE(input_gate_biases));

        vxmONERROR(_ConcatTensors(node,
                                  biases_array,
                                  4,
                                  data_format,
                                  &bias));

        layer->temp_tensors[layer->num_temp_tensors++] = bias;
    }

    if (_IsSameType(recurrent2input_weights, recurrent2forget_weights) &&
        _IsSameType(recurrent2input_weights, recurrent2cell_weights) &&
        _IsSameType(recurrent2input_weights, recurrent2output_weights))
    {
        /* Recurrent weights. */
        data_format = (vx_type_e)TENSOR_DATA_TYPE(recurrent2input_weights);

        vxmONERROR(_ConcatTensors(node,
                                  recurrent_fc_weights_array,
                                  4,
                                  data_format,
                                  &recurrent_fc_weights));

        layer->temp_tensors[layer->num_temp_tensors++] = recurrent_fc_weights;
    }

    /* Use high precision (FP16) intermediate tensor. */
    sizes[0] = output_element_num * 4;
    sizes[1] = batch * time_steps;
    dim_num = 2;

#if !NEW_CREATE_API

    vxZeroMemory(&tensor_params, gcmSIZEOF(vx_tensor_create_params_t));
    tensor_params.num_of_dims = dim_num;
    tensor_params.sizes = sizes;
    tensor_params.data_format = VX_TYPE_FLOAT16;
    tensor_params.quant_format = VX_QUANT_NONE;

    input_fc_output = vxoTensor_CreateTensor(context, node->graph, &tensor_params, vx_true_e);
#else
    input_fc_output = _createInteranlTensor(node->graph,
                                            force_format,
                                            vx_true_e,
                                            dim_num,
                                            sizes,
                                            TENSOR_DATA_TYPE(inputs),
                                            TENSOR_QUANT_TYPE(inputs),
                                            TENSOR_POS(bias),
                                            TENSOR_TF_SCALE(bias),
                                            TENSOR_TF_ZEROPOINT(bias));
#endif

    layer->temp_tensors[layer->num_temp_tensors++] = input_fc_output;

    vxmONERROR(_ReshapeLSTMTensor4FC(inputs, &input_fc_input));
    layer->temp_tensors[layer->num_temp_tensors++] = input_fc_input;

    /* Input FC. */
    if (input_fc_weights)
    {
        /* Concated input FC weights. */
        vxmONERROR(vxoFCOperation_Initialize(&lstm_layer->input_fc_operation,
                                             layer,
                                             input_fc_input,
                                             VX_NULL,
                                             input_fc_weights,
                                             bias,
                                             VX_CONVERT_POLICY_WRAP,
                                             VX_ROUND_POLICY_TO_ZERO,
                                             vx_false_e,
                                             input_fc_output,
                                             &op_index));
    }
    else
    {
        vx_tensor_view view = VX_NULL;
        vx_uint32 view_start[2] = { 0, 0 };
        vx_uint32 view_end[2] = { output_element_num, batch * time_steps };
        vx_tensor input_fc_output_sub_tensors[4];

        /* Un-concated input FC weights. */
        for (i = 0; i < 4; i++)
        {
            view = vxCreateTensorView(context, view_start, view_end, 2);
            if (!view)
            {
                vxmONERROR(VX_ERROR_NO_RESOURCES);
            }

            input_fc_output_sub_tensors[i] = vxoTensor_CreateTensorFromView(input_fc_output, view);

            vxReleaseTensorView(&view);

            vxmONERROR(vxoFCOperation_Initialize(&lstm_layer->input_fc_operation,
                                                 layer,
                                                 input_fc_input,
                                                 VX_NULL,
                                                 input_fc_weights_array[i],
                                                 biases_array[i],
                                                 VX_CONVERT_POLICY_WRAP,
                                                 VX_ROUND_POLICY_TO_ZERO,
                                                 vx_false_e,
                                                 input_fc_output_sub_tensors[i],
                                                 &op_index));

            view_start[0] += output_element_num;
            view_end[0] += output_element_num;
        }
    }

    if (time_steps >= 1)
    {
        views = (vx_tensor_view *)vxAllocateAndZeroMemory(gcmSIZEOF(vx_tensor_view) * time_steps);
        input_fc_sub_tensors = (vx_tensor *)vxAllocateAndZeroMemory(gcmSIZEOF(vx_tensor) * time_steps);
        output_sub_tensors = (vx_tensor *)vxAllocateAndZeroMemory(gcmSIZEOF(vx_tensor) * time_steps);
        hidden_sub_tensors = (vx_tensor *)vxAllocateAndZeroMemory(gcmSIZEOF(vx_tensor) * time_steps);
    }

    sizes[0] = output_element_num * 4;
    sizes[1] = batch * time_steps;
    dim_num = 2;

    /* Use high precision (FP16) intermediate tensor. */
    vxZeroMemory(&tensor_params, gcmSIZEOF(vx_tensor_create_params_t));

#if !NEW_CREATE_API
    tensor_params.num_of_dims = dim_num;
    tensor_params.sizes = sizes;
    tensor_params.data_format = VX_TYPE_FLOAT16;
    tensor_params.quant_format = VX_QUANT_NONE;

    recurrent_fc_output = vxoTensor_CreateTensor(context, node->graph, &tensor_params, vx_true_e);
#else
    recurrent_fc_output = _createInteranlTensor(node->graph,
                                                force_format,
                                                vx_true_e,
                                                dim_num,
                                                sizes,
                                                TENSOR_DATA_TYPE(outputs),
                                                TENSOR_QUANT_TYPE(outputs),
                                                TENSOR_POS(bias),
                                                TENSOR_TF_SCALE(bias),
                                                TENSOR_TF_ZEROPOINT(bias));
#endif

    layer->temp_tensors[layer->num_temp_tensors++] = recurrent_fc_output;

    sizes[0] = output_element_num;
    sizes[1] = time_steps * batch;
    dim_num = 2;
    output_reshape = vxoTensor_ReshapeTensor(outputs, (vx_int32_ptr)sizes, dim_num);

    layer->temp_tensors[layer->num_temp_tensors++] = output_reshape;

    end[0][0] = output_element_num * 4;
    end[1][0] = output_element_num;
    end[2][0] = output_element_num;
    end[3][0] = output_element_num * 4;
    dim_num = 2;

    for (i = 0; i < time_steps; i++)
    {
        start[0][1] = end[0][1];
        end[0][1] += 1;
        views[i] = vxCreateTensorView(context, start[0], end[0], (vx_uint8)dim_num);
        input_fc_sub_tensors[i] = vxoTensor_CreateTensorFromView(input_fc_output, views[i]);
        if (views[i])
        {
            vxReleaseTensorView(&views[i]);
        }

        start[2][1] = end[2][1];
        end[2][1] += 1;
        views[i] = vxCreateTensorView(context, start[2], end[2], (vx_uint8)dim_num);
        output_sub_tensors[i] = vxoTensor_CreateTensorFromView(output_reshape, views[i]);
        if (views[i])
        {
            vxReleaseTensorView(&views[i]);
        }

        start[3][1] = end[3][1];
        end[3][1] += 1;
        views[i] = vxCreateTensorView(context, start[3], end[3], (vx_uint8)dim_num);
        hidden_sub_tensors[i] = vxoTensor_CreateTensorFromView(recurrent_fc_output, views[i]);
        if (views[i])
        {
            vxReleaseTensorView(&views[i]);
        }

        layer->temp_tensors[layer->num_temp_tensors++] = input_fc_sub_tensors[i];
        layer->temp_tensors[layer->num_temp_tensors++] = output_sub_tensors[i];
        layer->temp_tensors[layer->num_temp_tensors++] = hidden_sub_tensors[i];
    }

    enable_format = (vx_bool)(TENSOR_DATA_TYPE(inputs) != VX_TYPE_FLOAT32);

    shExe_flag = (vx_bool)(enable_format && (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_SHADER)));

    /* State out on the output of input FC. */
    if (shExe_flag)
    {
        vxnne_shader_executable shaderExecutable = VX_NULL;
        vxnne_lstm_hidden_unit lstmHiddenUnitNode = &lstm_layer->hidden_units[0];

        if (enable_packed_mode)
        {
            shaderExecutable = vxnneGetLSTMUnitHiddenOut_PackedShaderExecutable(context,
                                                                                VXNNE_KERNEL_TENSOR_LSTMUNIT_HIDDENOUT,
                                                                                &node->kernelAttributes.borderMode,
                                                                                vx_false_e,
                                                                                input_fc_sub_tensors[0],
                                                                                forget_biases_value,
                                                                                cell_state,
                                                                                output_sub_tensors[0],
                                                                                NULL, NULL, NULL);
        }
        else if (TENSOR_DATA_TYPE(input_fc_sub_tensors[0]) == VX_TYPE_FLOAT16 || TENSOR_DATA_TYPE(input_fc_sub_tensors[0]) == VX_TYPE_BFLOAT16)
        {
            shaderExecutable = vxnneGetLSTMUnitHiddenOutExtShaderExecutable(context,
                                                                         VXNNE_KERNEL_TENSOR_LSTMUNIT_HIDDENOUT_EXT,
                                                                         &node->kernelAttributes.borderMode,
                                                                         vx_false_e,
                                                                         input_fc_sub_tensors[0],
                                                                         forget_biases_value,
                                                                         cell_state,
                                                                         output_sub_tensors[0],
                                                                         NULL, NULL, NULL);
        }
        else
        {
            shaderExecutable = vxnneGetLSTMUnitHiddenOutShaderExecutable(context,
                                                                         VXNNE_KERNEL_TENSOR_LSTMUNIT_HIDDENOUT,
                                                                         &node->kernelAttributes.borderMode,
                                                                         vx_false_e,
                                                                         input_fc_sub_tensors[0],
                                                                         forget_biases_value,
                                                                         cell_state,
                                                                         output_sub_tensors[0],
                                                                         NULL, NULL, NULL);
        }

        if (!shaderExecutable)
        {
            vxmONERROR(VX_FAILURE);
        }

        vxmONERROR(vxnneShaderOperation_Initialize(&lstmHiddenUnitNode->lstm_hidden_unit_sh_operation,
                                                   &lstm_layer->base,
                                                   VXNNE_OPERATOR_LSTM_STATE_OUT,
                                                   1,
                                                   shaderExecutable));

        vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_sh_operation.base, (vx_reference)input_fc_sub_tensors[0], VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_sh_operation.base, (vx_reference)cell_state, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_sh_operation.base, (vx_reference)output_sub_tensors[0], VXNNE_OPERATION_REFENRENCE_OUTPUT);

        vxnneLayer_SetOperation(layer,
                                &lstmHiddenUnitNode->lstm_hidden_unit_sh_operation.base,
                                op_index++);
    }
    else
    {
        vxnne_lstm_hidden_unit lstmHiddenUnitNode = &lstm_layer->hidden_units[0];

        vxnneOperation_Initialize(&lstmHiddenUnitNode->lstm_hidden_unit_operation.base,
                                  &lstm_layer->base,
                                  VXNNE_OPERATION_TARGET_SW,
                                  VXNNE_OPERATOR_LSTM_STATE_OUT,
                                  vxnneExecuteSWHiddenUnitOut_LSTMLayer,
                                  NULL,
                                  1,
                                  0);

        lstmHiddenUnitNode->lstm_hidden_unit_operation.input_fc_in = input_fc_sub_tensors[0];
        lstmHiddenUnitNode->lstm_hidden_unit_operation.hidden_fc_in = NULL;

        lstmHiddenUnitNode->lstm_hidden_unit_operation.output_state_out = NULL;
        lstmHiddenUnitNode->lstm_hidden_unit_operation.forget_bias = forget_biases;
        lstmHiddenUnitNode->lstm_hidden_unit_operation.cell_state = cell_state;
        lstmHiddenUnitNode->lstm_hidden_unit_operation.output = output_sub_tensors[0];

        lstmHiddenUnitNode->lstm_hidden_unit_operation.batch = batch;
        lstmHiddenUnitNode->lstm_hidden_unit_operation.output_num = output_element_num;
        lstmHiddenUnitNode->lstm_hidden_unit_operation.enable_cell_in = vx_false_e;

        lstmHiddenUnitNode->lstm_hidden_unit_operation.enable_packed_mode = enable_packed_mode;

        vxnneLayer_SetOperation(layer,
                                &lstmHiddenUnitNode->lstm_hidden_unit_operation.base,
                                op_index++);

        vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_operation.base, (vx_reference)input_fc_sub_tensors[0], VXNNE_OPERATION_REFENRENCE_INPUT);

        if (forget_biases)
        {
            vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_operation.base, (vx_reference)forget_biases, VXNNE_OPERATION_REFENRENCE_INPUT);
        }

        vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_operation.base, (vx_reference)cell_state, VXNNE_OPERATION_REFENRENCE_INPUT);

        if (outputs)
        {
            vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_operation.base, (vx_reference)output_sub_tensors[0], VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }
    }

    if (recurrent_fc_weights)
    {
        bias_sizes[0] = TENSOR_VIEW_SIZE_INDEX(recurrent_fc_weights, 1);
        bias_sizes[1] = 1;
        dim_num = 2;

        bias_format = (TENSOR_DATA_TYPE(recurrent_fc_weights) == VX_TYPE_FLOAT16) ? VX_TYPE_FLOAT32 : VX_TYPE_INT32;
        vxZeroMemory(&tensor_params, gcmSIZEOF(vx_tensor_create_params_t));
        tensor_params.num_of_dims = dim_num;
        tensor_params.sizes = bias_sizes;
        tensor_params.data_format = bias_format;
        tensor_params.quant_format = TENSOR_QUANT_TYPE(outputs);
        if (tensor_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
        {
            tensor_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(outputs) + TENSOR_POS(recurrent_fc_weights);
        }
        else if (tensor_params.quant_format == VX_QUANT_AFFINE_SCALE)
        {
            tensor_params.quant_data.affine.scale = TENSOR_TF_SCALE(outputs) * TENSOR_TF_SCALE(recurrent_fc_weights);
            tensor_params.quant_data.affine.zeroPoint = 0;
        }

        bias_zero[0] = vxoTensor_CreateTensor(context, node->graph, &tensor_params, vx_false_e);

        vxoTensor_AllocateMemory(bias_zero[0]);
        vxoTensor_GetTensorViewMemory(bias_zero[0], (vx_ptr_ptr)&bias_data, VX_NULL);
        vxoTensor_GetTensorElementCount(bias_zero[0], &bias_size);
        vxZeroMemory(bias_data, vxnneGetTypeSize(bias_format) * bias_size);

        layer->temp_tensors[layer->num_temp_tensors++] = bias_zero[0];
    }
    else
    {
        for (i = 0; i < 4; i++)
        {
            bias_sizes[0] = TENSOR_VIEW_SIZE_INDEX(recurrent_fc_weights_array[i], 1);
            bias_sizes[1] = 1;
            dim_num = 2;

            bias_format = (TENSOR_DATA_TYPE(recurrent_fc_weights_array[i]) == VX_TYPE_FLOAT16) ? VX_TYPE_FLOAT32 : VX_TYPE_INT32;

            vxZeroMemory(&tensor_params, gcmSIZEOF(vx_tensor_create_params_t));
            tensor_params.num_of_dims = dim_num;
            tensor_params.sizes = bias_sizes;
            tensor_params.data_format = bias_format;
            tensor_params.quant_format = TENSOR_QUANT_TYPE(outputs);
            if (tensor_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
            {
                tensor_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(outputs) + TENSOR_POS(recurrent_fc_weights_array[i]);
            }
            else if (tensor_params.quant_format == VX_QUANT_AFFINE_SCALE)
            {
                tensor_params.quant_data.affine.scale = TENSOR_TF_SCALE(outputs) * TENSOR_TF_SCALE(recurrent_fc_weights_array[i]);
                tensor_params.quant_data.affine.zeroPoint = 0;
            }

            bias_zero[i] = vxoTensor_CreateTensor(context, node->graph, &tensor_params, vx_false_e);

            vxoTensor_AllocateMemory(bias_zero[i]);
            vxoTensor_GetTensorViewMemory(bias_zero[i], (vx_ptr_ptr)&bias_data, VX_NULL);
            vxoTensor_GetTensorElementCount(bias_zero[i], &bias_size);
            vxZeroMemory(bias_data, vxnneGetTypeSize(bias_format) * bias_size);

            layer->temp_tensors[layer->num_temp_tensors++] = bias_zero[i];
        }
    }

    for (i = 1; i < time_steps; i++)
    {
        vxnne_lstm_hidden_unit lstmHiddenUnitNode = &lstm_layer->hidden_units[i];

        /* Recurrent FC. */
        recurrent_fc_sub_input = output_sub_tensors[i - 1];
        recurrent_fc_sub_output = hidden_sub_tensors[i];

        if (recurrent_fc_weights)
        {
            /* Concated recurrent FC weights. */
            vxmONERROR(vxoFCOperation_Initialize(&lstmHiddenUnitNode->recurrent_fc_operation,
                                                 &lstm_layer->base,
                                                 recurrent_fc_sub_input,
                                                 &weights_biases[0],
                                                 recurrent_fc_weights, /* weights tensor */
                                                 bias_zero[0], /* biases tensor */
                                                 VX_CONVERT_POLICY_WRAP,
                                                 VX_ROUND_POLICY_TO_ZERO,
                                                 vx_false_e,
                                                 recurrent_fc_sub_output,
                                                 &op_index));
        }
        else
        {
            vx_tensor_view view = VX_NULL;
            vx_uint32 view_start[2] = { 0, 0 };
            vx_uint32 view_end[2] = { output_element_num, 1 };
            vx_tensor recurrent_fc_sub_output_sub_tensors[4];

            /* Un-concated recurrent FC weights. */
            for (i = 0; i < 4; i++)
            {
                view = vxCreateTensorView(context, view_start, view_end, 2);
                if (!view)
                {
                    vxmONERROR(VX_ERROR_NO_RESOURCES);
                }

                recurrent_fc_sub_output_sub_tensors[i] = vxoTensor_CreateTensorFromView(recurrent_fc_sub_output, view);

                vxReleaseTensorView(&view);

                vxmONERROR(vxoFCOperation_Initialize(&lstmHiddenUnitNode->recurrent_fc_operation,
                                                     &lstm_layer->base,
                                                     recurrent_fc_sub_input,
                                                     &weights_biases[i],
                                                     recurrent_fc_weights_array[i], /* weights tensor */
                                                     bias_zero[i], /* biases tensor */
                                                     VX_CONVERT_POLICY_WRAP,
                                                     VX_ROUND_POLICY_TO_ZERO,
                                                     vx_false_e,
                                                     recurrent_fc_sub_output_sub_tensors[i],
                                                     &op_index));
            }
        }

        /* State out on the output of recurrent FC. */
        if (!shExe_flag)
        {
            vxmONERROR(vxnneOperation_Initialize(&lstmHiddenUnitNode->lstm_hidden_unit_operation.base,
                                                 layer,
                                                 VXNNE_OPERATION_TARGET_SW,
                                                 VXNNE_OPERATOR_LSTM_STATE_OUT,
                                                 vxnneExecuteSWHiddenUnitOut_LSTMLayer,
                                                 NULL,
                                                 1,
                                                 0));

            lstmHiddenUnitNode->lstm_hidden_unit_operation.input_fc_in = input_fc_sub_tensors[i];
            lstmHiddenUnitNode->lstm_hidden_unit_operation.hidden_fc_in = hidden_sub_tensors[i];
            lstmHiddenUnitNode->lstm_hidden_unit_operation.cell_state = cell_state;

            lstmHiddenUnitNode->lstm_hidden_unit_operation.output_state_out = NULL;
            lstmHiddenUnitNode->lstm_hidden_unit_operation.forget_bias = forget_biases;
            lstmHiddenUnitNode->lstm_hidden_unit_operation.output = output_sub_tensors[i];

            lstmHiddenUnitNode->lstm_hidden_unit_operation.batch = batch;
            lstmHiddenUnitNode->lstm_hidden_unit_operation.output_num = output_element_num;
            lstmHiddenUnitNode->lstm_hidden_unit_operation.enable_cell_in = vx_true_e;

            lstmHiddenUnitNode->lstm_hidden_unit_operation.enable_packed_mode = enable_packed_mode;

            vxnneLayer_SetOperation(layer,
                                    &lstmHiddenUnitNode->lstm_hidden_unit_operation.base,
                                    op_index++);

            vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_operation.base, (vx_reference)input_fc_sub_tensors[i], VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_operation.base, (vx_reference)hidden_sub_tensors[i], VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_operation.base, (vx_reference)cell_state, VXNNE_OPERATION_REFENRENCE_INPUT);
            if (forget_biases)
            {
                vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_operation.base, (vx_reference)forget_biases, VXNNE_OPERATION_REFENRENCE_INPUT);
            }
            vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_operation.base, (vx_reference)output_sub_tensors[i], VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }
        else
        {
            vxnne_shader_executable shaderExecutable = VX_NULL;

            if (enable_packed_mode)
            {
                shaderExecutable = vxnneGetLSTMUnitHiddenOut_PackedShaderExecutable(context,
                                                                                    VXNNE_KERNEL_TENSOR_LSTMUNIT_HIDDENOUT,
                                                                                    &node->kernelAttributes.borderMode,
                                                                                    vx_false_e,
                                                                                    input_fc_sub_tensors[i],
                                                                                    forget_biases_value,
                                                                                    cell_state,
                                                                                    output_sub_tensors[i],
                                                                                    NULL,
                                                                                    NULL,
                                                                                    hidden_sub_tensors[i]);
            }
            else if (TENSOR_DATA_TYPE(input_fc_sub_tensors[i]) == VX_TYPE_FLOAT16 || TENSOR_DATA_TYPE(input_fc_sub_tensors[i]) == VX_TYPE_BFLOAT16)
            {
                shaderExecutable = vxnneGetLSTMUnitHiddenOutExtShaderExecutable(context,
                                                                             VXNNE_KERNEL_TENSOR_LSTMUNIT_HIDDENOUT_EXT,
                                                                             &node->kernelAttributes.borderMode,
                                                                             vx_false_e,
                                                                             input_fc_sub_tensors[i],
                                                                             forget_biases_value,
                                                                             cell_state,
                                                                             output_sub_tensors[i],
                                                                             NULL,
                                                                             NULL,
                                                                             hidden_sub_tensors[i]);
            }
            else
            {
                shaderExecutable = vxnneGetLSTMUnitHiddenOutShaderExecutable(context,
                                                                             VXNNE_KERNEL_TENSOR_LSTMUNIT_HIDDENOUT,
                                                                             &node->kernelAttributes.borderMode,
                                                                             vx_false_e,
                                                                             input_fc_sub_tensors[i],
                                                                             forget_biases_value,
                                                                             cell_state,
                                                                             output_sub_tensors[i],
                                                                             NULL,
                                                                             NULL,
                                                                             hidden_sub_tensors[i]);
            }

            if (!shaderExecutable)
            {
                vxmONERROR(VX_FAILURE);
            }

            vxmONERROR(vxnneShaderOperation_Initialize(&lstmHiddenUnitNode->lstm_hidden_unit_sh_operation,
                                                       layer,
                                                       VXNNE_OPERATOR_LSTM_STATE_OUT,
                                                       1,
                                                       shaderExecutable));

            vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_sh_operation.base, (vx_reference)input_fc_sub_tensors[i], VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_sh_operation.base, (vx_reference)hidden_sub_tensors[i], VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_sh_operation.base, (vx_reference)cell_state, VXNNE_OPERATION_REFENRENCE_INPUT);

            vxnneOperation_AddReference(&lstmHiddenUnitNode->lstm_hidden_unit_sh_operation.base, (vx_reference)output_sub_tensors[i], VXNNE_OPERATION_REFENRENCE_OUTPUT);

            vxnneLayer_SetOperation(layer,
                                    &lstmHiddenUnitNode->lstm_hidden_unit_sh_operation.base,
                                    op_index++);
        }
    }

    /* Free resources. */
    if (views)
    {
        vxFree(views);
    }

    if (input_fc_sub_tensors)
    {
        vxFree(input_fc_sub_tensors);
    }

    if (output_sub_tensors)
    {
        vxFree(output_sub_tensors);
    }

    if (hidden_sub_tensors)
    {
        vxFree(hidden_sub_tensors);
    }

    return VX_SUCCESS;

OnError:
    /* Free resources. */
    if (views)
    {
        vxFree(views);
    }

    if (input_fc_sub_tensors)
    {
        vxFree(input_fc_sub_tensors);
    }

    if (output_sub_tensors)
    {
        vxFree(output_sub_tensors);
    }

    if (hidden_sub_tensors)
    {
        vxFree(hidden_sub_tensors);
    }

    if (operations)
    {
        vxFree(operations);
    }

    return status;
}

vx_status vxoLSTMLayer_Deinitialize(
    vxnne_lstm_layer lstm_layer
    )
{
    vx_uint32 i;

    /*
     * Release the following resources.
     * 1. temp_tensors.
     * 2. temp_arrays.
     * 3. Deinitialize each item of layer->operations.
     *
     * It must be called before lstm_layer->hidden_units is freed.
     */
    vxnneLayer_Deinitialize(&lstm_layer->base);

    /* Release vxoFCOperation resources. */
    if (lstm_layer->hidden_units)
    {
        for (i = 0; i < lstm_layer->hidden_unit_num; i++)
        {
            vxoFCOperation_Deinitialize(&lstm_layer->hidden_units[i].recurrent_fc_operation);
        }

        lstm_layer->hidden_unit_num = 0;

        vxFree(lstm_layer->hidden_units);
        lstm_layer->hidden_units = VX_NULL;
    }

    vxoFCOperation_Deinitialize(&lstm_layer->input_fc_operation);

    if (lstm_layer->cell_states != VX_NULL)
    {
        vxFree(lstm_layer->cell_states);
        lstm_layer->cell_states = VX_NULL;
    }

    if (lstm_layer->hidden_states != VX_NULL)
    {
        vxFree(lstm_layer->hidden_states);
        lstm_layer->hidden_states = VX_NULL;
    }

    if (lstm_layer->sub_wb != VX_NULL)
    {
        vxReleaseWeightsBiasesParameter(&lstm_layer->sub_wb);
    }

    if (lstm_layer->operations3)
    {
        vxFree(lstm_layer->operations3);
        lstm_layer->operations3 = VX_NULL;
    }

    return VX_SUCCESS;
}

