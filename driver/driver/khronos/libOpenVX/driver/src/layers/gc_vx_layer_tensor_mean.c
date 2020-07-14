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
#include <layers/gc_vx_layer_tensor_mean.h>

extern vx_status vxnneExecuteSWTensorTranspose(struct _vxnne_operation_s *operation);

/***************************************************************************************************************************
*                                                 TENSOR MEAN
***************************************************************************************************************************/

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorMean(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorMean_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorMean_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

vx_status vxnneExecuteSWTensorMean_GetSlices(vx_int32_ptr dims, vx_int32 dim_count, vx_int32_ptr front_slice, vx_int32 axis, vx_int32_ptr back_slice)
{
    vx_int32 i = 0, _axis = axis < 0 ? axis + dim_count : axis;

    gcmASSERT(dim_count > 0);
    gcmASSERT(front_slice != VX_NULL);
    gcmASSERT(back_slice != VX_NULL);

    if (_axis < dim_count)
    {
        *front_slice = 1;
        for (i = 0; i < _axis; i++)
            *front_slice *= dims[i];

        *back_slice = 1;
        for (i = _axis + 1; i < dim_count; i++)
            *back_slice *= dims[i];
    }

    return VX_SUCCESS;
}

vx_status vxnneExecuteSWTensorMean_ReduceSum(vx_uint8_ptr input_base, vx_int32_ptr input_dims, vx_int32 dim_count,
    vx_enum in_format, vx_enum in_quant_format, vx_uint8 in_fp, vx_int32 in_zp, vx_float32 in_scale,
    vx_enum out_format, vx_enum out_quant_format, vx_uint8 out_fp, vx_int32 out_zp, vx_float32 out_scale, vx_enum rounding,
   vx_int32_ptr axis_dims, vx_int32 axis_count, vx_int32 current_axis_index, vx_int32 count, vx_uint8_ptr output_base)
{
    /*****************************************************************************************************************************************************************
    *                 output                   axis                    input     *        output                   axis                    input
    *                 C H W                   1, 2                     C H W     *        C H W                   0, 2                     C H W
    *                 2 3 4                     =>                     2 1 1     *        2 3 4                     =>                     3 1 1
    *             ____________________                                           *    ____________________
    *            |  1  2|  3  4|  5  6|                                          *   |  1  2|  3  4|  5  6|
    *            |______|______|______|               _______                    *   |______|______|______|               ______________
    *            |  7  8|  9 10| 11 12|              | 12| 13|                   *   |  7  8|  9 10| 11 12|              |10.5|12.5|14.5|
    *            |______|______|______|         =>   |___|___|                   *   |______|______|______|         =>   |____|____|____|
    *            | 13 14| 15 16| 17 18|                                          *   | 13 14| 15 16| 17 18|
    *            |______|______|______|                                          *   |______|______|______|
    *            | 19 20| 21 22| 23 24|                                          *   | 19 20| 21 22| 23 14|
    *            |______|______|______|                                          *   |______|______|______|
    *****************************************************************************************************************************************************************/
    if (current_axis_index < axis_count)
    {
        vx_uint8_ptr output_ptr = VX_NULL;
        vx_int32 axis = axis_dims[current_axis_index];
        vx_int32 current_dim = input_dims[axis];

        vx_int32 i = 0, b = 0, f = 0, front_slice = 1, back_slice = 1;

        vx_int32 out_item_size = vxnneGetTypeSize((vx_type_e)out_format);
        vx_bool last = (current_axis_index < (axis_count - 1)) ? vx_false_e : vx_true_e;
        vx_float32 sum = 0.f;
        vx_int32 _count = last ? count * current_dim : 1;

        vxnneExecuteSWTensorMean_GetSlices(input_dims, dim_count, &front_slice, axis, &back_slice);

        output_ptr = last? output_base:(vx_uint8_ptr)malloc(front_slice * back_slice * out_item_size);


        for (b = 0; b < back_slice; b++)/*4*/
        {
            for (f = 0; f < front_slice; f++)/*2*/
            {
                sum = 0.f;
                for (i = 0; i < current_dim; i++)/*3*/
                    sum += vxnneGetDataExt((vx_type_e)in_format, in_quant_format, f + i * front_slice + b * current_dim * front_slice, input_base, in_fp, in_zp, in_scale);

                vxnneSaveDataExt((vx_type_e)out_format, out_quant_format, f + b * front_slice, sum / _count, output_ptr, out_fp, out_zp, out_scale, rounding);
            }
        }

        input_dims[axis] = 1;

        if (current_axis_index > 0)
            free(input_base);


        return vxnneExecuteSWTensorMean_ReduceSum(output_ptr, input_dims, dim_count,
            in_format, in_quant_format, in_fp, in_zp, in_scale,
            out_format, out_quant_format, out_fp, out_zp, out_scale, rounding,
            axis_dims, axis_count, current_axis_index + 1, count * current_dim, output_base);
    }
    else
        return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxnneExecuteSWTensorMean(struct _vxnne_operation_s *operation)
{

    vxnne_tensor_mean_operation transOperation = (vxnne_tensor_mean_operation)operation;
    vx_tensor input     = (vx_tensor)transOperation->input;
    vx_tensor axis      = (vx_tensor)transOperation->axis;
    /*vx_scalar keep_dims = (vx_scalar)transOperation->keep_dims;*/
    vx_tensor output    = (vx_tensor)transOperation->output;
    vx_int32_ptr axis_base = VX_NULL;
    vx_uint8_ptr input_ptr = VX_NULL, output_ptr = VX_NULL;

    vx_uint32 i = 0, j = 0, count = 1;
    vx_int32 resolved_dim[4] = {-1, -1, -1, -1};
    vx_int32 resolved_dim_count = 0;
    vx_int32 dims[4] = { TENSOR_SIZE_INDEX(input, 0), TENSOR_SIZE_INDEX(input, 1), TENSOR_SIZE_INDEX(input, 2), TENSOR_SIZE_INDEX(input, 3) };

    vxoTensor_GetTensorViewMemory(axis, (gctPOINTER*)&axis_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(input, (gctPOINTER*)&input_ptr, VX_NULL);
    vxoTensor_GetTensorViewMemory(output, (gctPOINTER*)&output_ptr, VX_NULL);

    gcmASSERT(axis->dimCount == 1);

    /*****************************************************************************************************************************************************************
    *                                                                            *
    *                 output                   axis                    input     *        output                   axis                    input
    *                 C H W                   1, 2                     C H W     *        C H W                   0, 2                     C H W
    *                 2 3 4                     =>                     2 1 1     *        2 3 4                     =>                     3 1 1
    *             ____________________                                           *    ____________________
    *            |  1  2|  3  4|  5  6|                                          *   |  1  2|  3  4|  5  6|
    *            |______|______|______|               _______                    *   |______|______|______|               ______________
    *            |  7  8|  9 10| 11 12|              | 12| 13|                   *   |  7  8|  9 10| 11 12|              |10.5|12.5|14.5|
    *            |______|______|______|         =>   |___|___|                   *   |______|______|______|         =>   |____|____|____|
    *            | 13 14| 15 16| 17 18|                                          *   | 13 14| 15 16| 17 18|
    *            |______|______|______|                                          *   |______|______|______|
    *            | 19 20| 21 22| 23 24|                                          *   | 19 20| 21 22| 23 14|
    *            |______|______|______|                                          *   |______|______|______|
    *                                                                            *
    *                            output                axis     input            *        output                   axis                    input
    *                            W H C                 1, 2     W H C            *        C H W                   0, 2                     C H W
    *                            4 3 2                  =>      1 1 2            *        2 3 4                     =>                     3 1 1
    *             _______________     _______________                            *    ___________     ___________
    *            |  1|  3|  5|  7|   |  2|  4|  6|  8|                           *   |  1|  3|  5|   |  2|  4|  6|
    *            |___|___|___|___|   |___|___|___|___|        _______            *   |___|___|___|   |___|___|___|         ______________
    *            |  9| 11| 13| 15|   | 10| 12| 14| 16|       | 12| 13|           *   |  7|  9| 11|   |  8| 10| 12|        |10.5|12.5|14.5|
    *            |___|___|___|___|   |___|___|___|___|  =>   |___|___|           *   |___|___|___|   |___|___|___|   =>   |____|____|____|
    *            | 17| 19| 21| 23|   | 18| 20| 22| 24|                           *   | 13| 15| 17|   | 14| 16| 18|
    *            |___|___|___|___|   |___|___|___|___|                           *   |___|___|___|   |___|___|___|
                                                                                                                               *
    *****************************************************************************************************************************************************************/
    /* resolve axis dims */
    for (i = 0; i < TENSOR_SIZE_INDEX(axis, 0); i++)
    {
        vx_int32 current_axis = axis_base[i] < 0 ? TENSOR_DIM_NUM(input) + axis_base[i] : axis_base[i];
        for (j = 0; j < 4; j++)
        {
            if (resolved_dim[j] == current_axis)
                break;
        }

        if (j == 4)
            resolved_dim[resolved_dim_count++] = current_axis;
    }

    vxnneExecuteSWTensorMean_ReduceSum(input_ptr, dims, TENSOR_DIM_NUM(input),
        TENSOR_DATA_TYPE(input), TENSOR_QUANT_TYPE(input), TENSOR_POS(input), TENSOR_TF_ZEROPOINT(input), TENSOR_TF_SCALE(input),
        TENSOR_DATA_TYPE(output), TENSOR_QUANT_TYPE(output), TENSOR_POS(output), TENSOR_TF_ZEROPOINT(output), TENSOR_TF_SCALE(output), TENSOR_ROUNDING_MODE(output),
        axis_base, resolved_dim_count, 0, count, output_ptr
    );
    return VX_SUCCESS;
}
#if REGISTER_FRAME
VX_PRIVATE_API vx_status vxoNNTensorMean_SW_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vxnne_tensor_mean_layer  tensor_mean_layer = (vxnne_tensor_mean_layer)ops_layer;

    vx_tensor input     = (vx_tensor)parameters[0];
    vx_tensor axis      = (vx_tensor)parameters[1];
    vx_scalar keep_dims = (vx_scalar)parameters[2];
    vx_tensor output    = (vx_tensor)parameters[3];

    vx_uint32 dims      = TENSOR_DIM_NUM(input);
    vx_uint32 batchCount= dims > 3 ? TENSOR_VIEW_SIZE_INDEX(input, 3) : 1;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxnneOperation_Initialize(&tensor_mean_layer->tensor_mean_sw_operation.base,
        &tensor_mean_layer->base,
        VXNNE_OPERATION_TARGET_SW,
        VXNNE_OPERATOR_TENSOR_MEAN,
        vxnneExecuteSWTensorMean,
        VX_NULL,
        batchCount,
        0));

    vxmONERROR(vxnneLayer_SetOperation(
        &tensor_mean_layer->base,
        &tensor_mean_layer->tensor_mean_sw_operation.base,
        0));

    tensor_mean_layer->tensor_mean_sw_operation.input     = input;
    tensor_mean_layer->tensor_mean_sw_operation.axis      = axis;
    tensor_mean_layer->tensor_mean_sw_operation.keep_dims = keep_dims;
    tensor_mean_layer->tensor_mean_sw_operation.output    = output;

    vxmONERROR(vxnneOperation_AddReference(&tensor_mean_layer->tensor_mean_sw_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&tensor_mean_layer->tensor_mean_sw_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT));
OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNTensorMean_SH_EVIS_Support_Ext(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_tensor input     = (vx_tensor)parameters[0];
    vx_tensor axis      = (vx_tensor)parameters[1];
    vx_tensor output    = (vx_tensor)parameters[3];

    vx_int32_ptr axis_base          = VX_NULL;
    vx_int32     resolved_dim[4]    = {-1, -1, -1, -1};
    vx_int32     resolved_dim_count = 0;
    vx_uint32    i                  = 0;
    vx_uint32    j                  = 0;
    vx_uint32   input_axis;


    vx_enum     inputFormat         = TENSOR_DATA_TYPE(input);
    vx_enum     outputFormat        = TENSOR_DATA_TYPE(output);

    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    vxoTensor_GetTensorViewMemory(axis, (gctPOINTER*)&axis_base, VX_NULL);

    for (i = 0; i < TENSOR_SIZE_INDEX(axis, 0); i++)
    {
        vx_int32 current_axis = axis_base[i] < 0 ? TENSOR_DIM_NUM(input) + axis_base[i] : axis_base[i];

        if (current_axis < 0 || current_axis >= (vx_int32)TENSOR_DIM_NUM(input))
        {
            vxError("error: the axis value must be in the range [0, %d)\n", TENSOR_DIM_NUM(input));
            gcmASSERT(0);
        }

        for (j = 0; j < 4; j++)
        {
            if (resolved_dim[j] == current_axis)
                break;
        }

        if (j == 4)
            resolved_dim[resolved_dim_count++] = current_axis;
    }

    input_axis = 0;

    {
        vx_uint32 dst_elementCount = 0;

        vxoTensor_GetTensorElementCount(output, &dst_elementCount);

        if (dst_elementCount == 1) /* reudce mean all*/
        {
            vx_uint32  reshpTensor_Sizes[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {1};
            vx_uint32  reshpTensor_Dims = 2;

            resolved_dim_count = 0;

            vxoElementOptimization_GetTensorShape(input, reshpTensor_Sizes, &reshpTensor_Dims);

            for (i = 0; i < reshpTensor_Dims; i++)
            {
                if (reshpTensor_Sizes[i] != 1)
                {
                    resolved_dim[resolved_dim_count++] = i;
                }
                else
                    break;
            }
        }
    }

    if(evis)
    {
        support  = support && ((inputFormat != VX_TYPE_FLOAT32 && outputFormat != VX_TYPE_FLOAT32 && resolved_dim_count == 2 && resolved_dim[0] <= 3 && resolved_dim[1] <= 3)
                               || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16 && resolved_dim_count == 1 && resolved_dim[0] <= 3)
                               || (inputFormat == VX_TYPE_BFLOAT16 && outputFormat == VX_TYPE_BFLOAT16 && resolved_dim_count == 1 && resolved_dim[0] <= 3)
                               || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT16 && resolved_dim_count == 1 && resolved_dim[0] <= 3)
                               || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT8 && resolved_dim_count == 1 && resolved_dim[0] <= 3)
                               || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_UINT8 && resolved_dim_count == 1 && resolved_dim[0] <= 3)
                               || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16 && resolved_dim_count == 1 && resolved_dim[0] <= 3)
                               || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_FLOAT16 && resolved_dim_count == 1 && resolved_dim[0] <= 3)
                               || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8 && resolved_dim_count == 1 && resolved_dim[0] <= 3)
                               || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_FLOAT16 && resolved_dim_count == 1 && resolved_dim[0] <= 3)
                               || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8 && resolved_dim_count == 1 && resolved_dim[0] <= 3)
                               || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT16 && resolved_dim_count == 1 && resolved_dim[0] <= 3));
    }
    else
    {
        support  = support && ((inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32 && resolved_dim_count == 2 && resolved_dim[0] <= 3 && resolved_dim[1] <= 3)
                               || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16 && resolved_dim_count == 1 && resolved_dim[0] <= 3)
                               || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32 && resolved_dim_count == 1 && resolved_dim[0] <= 3)
                               || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16 && resolved_dim_count == 2 && resolved_dim[0] <= 3 && resolved_dim[1] <= 3)
                               || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8 && resolved_dim_count == 2 && resolved_dim[0] <= 3 && resolved_dim[1] <= 3)
                               || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8 && resolved_dim_count == 1 && resolved_dim[0] <= 3));
    }

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_bool vxoNNTensorMean_SH_EVIS_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && node->base.context->evisNoInst.supportEVIS;

    if (!support)return support;

    support = support && vxoNNTensorMean_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_true_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNTensorMean_SH_EVIS_Initialize_Ext(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool evis, vx_bool tp_trans)
{
    vx_status status = VX_SUCCESS;

    vx_tensor input     = (vx_tensor)parameters[0];
    vx_tensor axis      = (vx_tensor)parameters[1];

    vx_tensor output    = (vx_tensor)parameters[3];
    vx_uint32 batchCount = 1;

    vx_int32_ptr axis_base          = VX_NULL;
    vx_int32     resolved_dim[4]    = {-1, -1, -1, -1};
    vx_int32     resolved_dim_count = 0;
    vx_uint32    i                  = 0;
    vx_uint32    j                  = 0;
    vx_uint32   input_axis;
    vx_enum     inputFormat         = TENSOR_DATA_TYPE(input);

    vx_uint32   tmpTensorIndex      = 0;
    vx_uint32   operationIdx        = 0;

    vx_context  context             = vxGetContext((vx_reference)ops_layer->node);

    vxnne_tensor_mean_layer  tensor_mean_layer = (vxnne_tensor_mean_layer)ops_layer;

    vx_tensor_create_params_t tensor_create_params;
    vx_uint32 dims          = TENSOR_DIM_NUM(input);
    vx_uint32 width         = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32 height        = dims > 1 ? TENSOR_VIEW_SIZE_INDEX(input, 1) : 1;
    vx_uint32 depth         = dims > 2 ? TENSOR_VIEW_SIZE_INDEX(input, 2) : 1;
    vx_uint32 batch         = dims > 3 ? TENSOR_VIEW_SIZE_INDEX(input, 3) : 1;
    vx_uint32 sizes[4]      = {width, height, depth, batch};
    vx_uint32 new_sizes[4]  = {width, height, depth, batch};
    vx_uint32 perm_array[4] = {0, 1, 2, 3};
    vx_tensor transTensor   = NULL;
    vx_tensor dst           = NULL;
    vx_bool   enable_trans  = vx_false_e;
    vx_bool   enable_axis  = vx_false_e;
    vx_bool   is_trans_sw   = vx_false_e;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxoTensor_GetTensorViewMemory(axis, (gctPOINTER*)&axis_base, VX_NULL);

    for (i = 0; i < TENSOR_SIZE_INDEX(axis, 0); i++)
    {
        vx_int32 current_axis = axis_base[i] < 0 ? TENSOR_DIM_NUM(input) + axis_base[i] : axis_base[i];

        if (current_axis < 0 || current_axis >= (vx_int32)TENSOR_DIM_NUM(input))
        {
            vxError("error: the axis value must be in the range [0, %d)\n", TENSOR_DIM_NUM(input));
            gcmASSERT(0);
        }

        for (j = 0; j < 4; j++)
        {
            if (resolved_dim[j] == current_axis)
                break;
        }

        if (j == 4)
            resolved_dim[resolved_dim_count++] = current_axis;
    }

    input_axis = 0;

    {
        vx_uint32 dst_elementCount = 0;

        status = vxoTensor_GetTensorElementCount(output, &dst_elementCount);

        if (dst_elementCount == 1) /* reudce mean all*/
        {
            vx_uint32  reshpTensor_Sizes[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {1};
            vx_uint32  reshpTensor_Dims = 2;

            resolved_dim_count = 0;

            vxoElementOptimization_GetTensorShape(input, reshpTensor_Sizes, &reshpTensor_Dims);

            for (i = 0; i < reshpTensor_Dims; i++)
            {
                if (reshpTensor_Sizes[i] != 1)
                {
                    resolved_dim[resolved_dim_count++] = i;
                }
                else
                    break;
            }

            input = vxoTensor_ReshapeTensor((vx_tensor)parameters[0], (vx_int32*)reshpTensor_Sizes, reshpTensor_Dims);

            for (i = 0; i < reshpTensor_Dims; i++)
            {
                new_sizes[i] = reshpTensor_Sizes[i];
            }

            for (; i < 4; i++)
            {
                new_sizes[i] = 1;
            }

            tensor_mean_layer->base.temp_tensors[tmpTensorIndex++] = input;
            tensor_mean_layer->base.num_temp_tensors = tmpTensorIndex;
        }
    }

    if (resolved_dim_count == 1)
    {
        enable_axis = vx_true_e;
    }

    if (resolved_dim[0] + resolved_dim[1] != 1 && resolved_dim_count == 2)
    {
        enable_trans = vx_true_e;

        if (resolved_dim[0] + resolved_dim[1] == 2)
        {
            perm_array[0] = 0;
            perm_array[1] = 2;
            perm_array[2] = 1;
            perm_array[3] = 3;
        }
        else if (resolved_dim[0] + resolved_dim[1] == 3)
        {
            if (abs(resolved_dim[0] - resolved_dim[1]) == 3)
            {
                perm_array[0] = 0;
                perm_array[1] = 3;
                perm_array[2] = 1;
                perm_array[3] = 2;
                is_trans_sw   = vx_true_e;
            }
            else
            {
                perm_array[0] = 1;
                perm_array[1] = 2;
                perm_array[2] = 0;
                perm_array[3] = 3;
            }
        }
        else if (resolved_dim[0] + resolved_dim[1] == 4)
        {
            perm_array[0] = 1;
            perm_array[1] = 3;
            perm_array[2] = 0;
            perm_array[3] = 2;
            is_trans_sw   = vx_true_e;
        }
        else if (resolved_dim[0] + resolved_dim[1] == 5)
        {
            perm_array[0] = 2;
            perm_array[1] = 3;
            perm_array[2] = 0;
            perm_array[3] = 1;
            is_trans_sw   = vx_true_e;
        }
    }
    else if (resolved_dim[0] != 0 && resolved_dim_count == 1)
    {
        if (resolved_dim[0] == 3)
        {
            enable_trans = vx_true_e;
            input_axis = 0;
            perm_array[0] = 3;
            perm_array[1] = 0;
            perm_array[2] = 1;
            perm_array[3] = 2;
            is_trans_sw   = vx_true_e;
        }
        else
        {
            transTensor = input;
            input_axis = resolved_dim[0];
        }
    }
    else
    {
        transTensor = input;
    }

    if (enable_trans)
    {
        for (i = 0; i < 4; i ++)
        {
            new_sizes[i] = sizes[perm_array[i]];
        }

        gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
        tensor_create_params.num_of_dims = dims;
        tensor_create_params.sizes = new_sizes;
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

        transTensor = vxoTensor_CreateTensor(ops_layer->node->base.context, ops_layer->node->graph, &tensor_create_params, vx_true_e);

        tensor_mean_layer->base.temp_tensors[tmpTensorIndex++] = transTensor;

        if (tp_trans)
        {
            vx_op_param_s conv = {0};
            vx_uint32 dnum = 4;

            vxmONERROR(vxnneOperation_Initialize(&tensor_mean_layer->tensor_mean_trans_tp_operation.base,
                &tensor_mean_layer->base,
                VXNNE_OPERATION_TARGET_TP,
                VXNNE_OPERATOR_TENSOR_TRANS,
                VX_NULL,
                vxnneOperation_TP_Deinitialize,
                1,
                0));

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
            conv.other_ref = (vx_reference)input;
            conv.data_buff = gcvNULL;
            conv.tp_value = (vx_tp_value_cmd)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
            conv.tp_value->u32[0] = dnum;
            conv.tp_value->p8[0] = (vx_uint8_ptr)vxAllocateAndZeroMemory(sizeof(vx_uint32) * dnum);
            vxMemCopy(conv.tp_value->p8[0], perm_array, sizeof(vx_uint32) * dnum);

            vxMemCopy(&tensor_mean_layer->tensor_mean_trans_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));

            vxmONERROR(vxnneLayer_SetOperation(
                &tensor_mean_layer->base,
                &tensor_mean_layer->tensor_mean_trans_tp_operation.base,
                operationIdx++));

            tensor_mean_layer->tensor_mean_trans_tp_operation.input  = input;
            tensor_mean_layer->tensor_mean_trans_tp_operation.output = transTensor;

            vxmONERROR(vxnneOperation_AddReference(&tensor_mean_layer->tensor_mean_trans_tp_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT));
            vxmONERROR(vxnneOperation_AddReference(&tensor_mean_layer->tensor_mean_trans_tp_operation.base, (vx_reference)transTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT));
        }
        else
        {
            vx_bool enable_shader_execute = vx_true_e;
            vxnne_shader_executable shaderExecutable = VX_NULL;
            vx_uint32 pnum = dims;

            if (dims > 4)
            {
                vx_uint32 i = 0;
                vx_uint32 elementCnt = 1;

                for (i = 4; i < dims; i++)
                {
                    elementCnt *= TENSOR_VIEW_SIZE_INDEX(input, i);
                }

                if (elementCnt == 1)
                    pnum = 3;
                else
                    enable_shader_execute = vx_false_e;
            }

            if (is_trans_sw)
            {
                enable_shader_execute = vx_false_e;
            }

            if (enable_shader_execute && vxoContext_IsFeatureAvailable(ops_layer->node->base.context, VX_NN_FEATURE_SHADER))
            {
                if (dims == 4)
                {
                    pnum = 3;
                }
                if(evis)
                {
                    shaderExecutable = vxnneTensorTransposeShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_TRANSPOSE, &ops_layer->node->kernelAttributes.borderMode, input, perm_array, pnum, transTensor);
                }
                else
                {
                    shaderExecutable = vxnneGPUTensorTransposeShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_TRANSPOSE, &ops_layer->node->kernelAttributes.borderMode, input, perm_array, pnum, transTensor);
                }

                if (!shaderExecutable)
                {
                    status = VX_FAILURE;
                    goto OnError;
                }

            batchCount = new_sizes[3];
            vxmONERROR(vxnneShaderOperation_Initialize(&tensor_mean_layer->tensor_mean_trans_sh_operation,
                &tensor_mean_layer->base,
                VXNNE_OPERATOR_TENSOR_TRANS,
                batchCount,
                shaderExecutable));

            vxmONERROR(vxnneOperation_AddReference(&tensor_mean_layer->tensor_mean_trans_sh_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT));
            vxmONERROR(vxnneOperation_AddReference(&tensor_mean_layer->tensor_mean_trans_sh_operation.base, (vx_reference)transTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT));

            vxmONERROR(vxnneLayer_SetOperation(
                    &tensor_mean_layer->base,
                    &tensor_mean_layer->tensor_mean_trans_sh_operation.base,
                    operationIdx++));
            }
            else
            {
                vx_array perm = vxCreateArray(context, VX_TYPE_UINT32, pnum);
                vx_scalar pnum_s = NULL;

                if (!vxoArray_AllocateMemory(perm))
                {
                    status = VX_ERROR_NO_MEMORY;
                    vxError("Fail to vxoArray_AllocateMemory of perm function %s line %d", __FUNCTION__, __LINE__);
                    goto OnError;
                }
                else
                {
                    vx_uint32* pos = (vx_uint32*)perm->memory.logicals[0];
                    memcpy(pos, perm_array, pnum * sizeof(vx_uint32));
                }

                pnum_s = vxCreateScalar(context, VX_TYPE_UINT32, &pnum);
                batchCount = new_sizes[3];

                vxmONERROR(vxnneOperation_Initialize(&tensor_mean_layer->tensor_trans_sw_operation.base,
                                          &tensor_mean_layer->base,
                                          VXNNE_OPERATION_TARGET_SW,
                                          VXNNE_OPERATOR_TENSOR_TRANS,
                                          vxnneExecuteSWTensorTranspose,
                                          VX_NULL,
                                          batchCount,
                                          0));

                vxmONERROR(vxnneLayer_SetOperation(
                    &tensor_mean_layer->base,
                    &tensor_mean_layer->tensor_trans_sw_operation.base,
                    operationIdx++));

                tensor_mean_layer->tensor_trans_sw_operation.input   = input;
                tensor_mean_layer->tensor_trans_sw_operation.perm    = perm;
                tensor_mean_layer->tensor_trans_sw_operation.pnum    = pnum_s;
                tensor_mean_layer->tensor_trans_sw_operation.output  = transTensor;

                vxmONERROR(vxnneOperation_AddReference(&tensor_mean_layer->tensor_trans_sw_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT));
                vxmONERROR(vxnneOperation_AddReference(&tensor_mean_layer->tensor_trans_sw_operation.base, (vx_reference)transTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT));
            }
        }
    }

    if (resolved_dim_count == 2)
    {
        vx_uint32 output_dims         = dims < 3 ? 3 : dims;

        sizes[0] = 1;
        sizes[1] = 1;
        sizes[2] = new_sizes[2];
        sizes[3] = new_sizes[3];
        dst = vxoTensor_ReshapeTensor(output, (vx_int32*)sizes, output_dims);

        tensor_mean_layer->base.temp_tensors[tmpTensorIndex++] = dst;
    }
    else if (resolved_dim_count == 1)
    {
        vx_uint32 output_dims         = dims < 3 ? 3 : dims;

        sizes[0] = new_sizes[0];
        sizes[1] = new_sizes[1];
        sizes[2] = new_sizes[2];
        sizes[3] = new_sizes[3];
        sizes[input_axis] = 1;
        dst = vxoTensor_ReshapeTensor(output, (vx_int32*)sizes, output_dims);

        tensor_mean_layer->base.temp_tensors[tmpTensorIndex++] = dst;
    }

    if (resolved_dim_count == 2)
    {
        vxnne_shader_executable shaderExecutable = NULL;
        vx_bool   enable_tf_quantize  = (vx_bool)(inputFormat == VX_TYPE_UINT8);
        vx_bool   enable_int16_sh     = (vx_bool)(inputFormat == VX_TYPE_INT16);
        vx_uint32 stride              = 1;
        vx_scalar stride_s            = vxCreateScalar(ops_layer->node->base.context, VX_TYPE_UINT32, &stride);
        vx_scalar poolSizeX           = vxCreateScalar(ops_layer->node->base.context, VX_TYPE_UINT32, &new_sizes[0]);
        vx_scalar poolSizeY           = vxCreateScalar(ops_layer->node->base.context, VX_TYPE_UINT32, &new_sizes[1]);
        vx_uint32 pad_x_left          = 0;
        vx_uint32 pad_y_top           = 0;
        vx_uint32 batch               = new_sizes[3];

        if (stride_s == NULL || poolSizeX == NULL || poolSizeY == NULL)
        {
            if (stride_s)  vxReleaseScalar(&stride_s);
            if (poolSizeX) vxReleaseScalar(&poolSizeX);
            if (poolSizeY) vxReleaseScalar(&poolSizeY);

            status = VX_FAILURE;
            goto OnError;
        }

        if(evis)
        {
            if(enable_tf_quantize)
                shaderExecutable = vxnneGetAvgPooling_UInt8ShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_AVGPOOLING_UINT8, &ops_layer->node->kernelAttributes.borderMode,
                transTensor, NULL, stride_s, stride_s, poolSizeX, poolSizeY, pad_x_left, pad_y_top, NULL, VX_NN_ACTIVATION_NONE, dst);
            else if(enable_int16_sh)
                shaderExecutable = vxnneGetAvgPooling_Int16ShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_AVGPOOLING_INT16, &ops_layer->node->kernelAttributes.borderMode,
                transTensor, NULL, stride_s, stride_s, poolSizeX, poolSizeY, pad_x_left, pad_y_top, NULL, VX_NN_ACTIVATION_NONE, dst);
            else
                shaderExecutable = vxnneGetAvgPoolingShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_AVGPOOLING, &ops_layer->node->kernelAttributes.borderMode,
                transTensor, NULL, stride_s, stride_s, poolSizeX, poolSizeY, pad_x_left, pad_y_top, NULL, dst);
        }
        else
        {
            shaderExecutable = vxnneGetGPUAvgPoolingShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_AVGPOOLING, &ops_layer->node->kernelAttributes.borderMode,
                transTensor, NULL, stride_s, stride_s, poolSizeX, poolSizeY, pad_x_left, pad_y_top, pad_x_left, pad_y_top, NULL, vx_false_e, 0, 0, dst);
        }

        if (stride_s)  vxReleaseScalar(&stride_s);
        if (poolSizeX) vxReleaseScalar(&poolSizeX);
        if (poolSizeY) vxReleaseScalar(&poolSizeY);

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto OnError;
        }

        vxmONERROR(vxnneShaderOperation_Initialize(&tensor_mean_layer->tensor_mean_pool_sh_operation,
            &tensor_mean_layer->base,
            VXNNE_OPERATOR_POOLING,
            batch,
            shaderExecutable));

        vxmONERROR(vxnneOperation_AddReference(&tensor_mean_layer->tensor_mean_pool_sh_operation.base, (vx_reference)transTensor, VXNNE_OPERATION_REFENRENCE_INPUT));
        vxmONERROR(vxnneOperation_AddReference(&tensor_mean_layer->tensor_mean_pool_sh_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT));

        vxmONERROR(vxnneLayer_SetOperation(
            &tensor_mean_layer->base,
            &tensor_mean_layer->tensor_mean_pool_sh_operation.base,
            operationIdx++));
    }

    if (enable_axis)
    {
        vxnne_shader_executable shaderExecutable = NULL;
        vx_float32              axis_coef        = 1.0f / (vx_float32)(TENSOR_VIEW_SIZE_INDEX(transTensor,input_axis));
        vx_uint32               batch            = new_sizes[3];

        if(evis)
        {
            shaderExecutable = vxnneGetTensorMeanAxisShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_MEAN_AXIS, &ops_layer->node->kernelAttributes.borderMode, axis_coef, transTensor, dst, input_axis);
        }
        else
        {
            shaderExecutable = vxnneGetGPUTensorMeanAxisShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_MEAN_AXIS, &ops_layer->node->kernelAttributes.borderMode, axis_coef, transTensor, dst, input_axis);
        }

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto OnError;
        }

        vxmONERROR(vxnneShaderOperation_Initialize(&tensor_mean_layer->tensor_mean_axis0_sh_operation,
            &tensor_mean_layer->base,
            VXNNE_OPERATOR_TENSOR_MEAN,
            batch,
            shaderExecutable));

        vxmONERROR(vxnneOperation_AddReference(&tensor_mean_layer->tensor_mean_axis0_sh_operation.base, (vx_reference)transTensor, VXNNE_OPERATION_REFENRENCE_INPUT));
        vxmONERROR(vxnneOperation_AddReference(&tensor_mean_layer->tensor_mean_axis0_sh_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT));

        vxmONERROR(vxnneLayer_SetOperation(
            &tensor_mean_layer->base,
            &tensor_mean_layer->tensor_mean_axis0_sh_operation.base,
            operationIdx++));
    }

    tensor_mean_layer->base.num_temp_tensors = tmpTensorIndex;

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

OnError:
    return status;
}

VX_PRIVATE_API vx_status vxoNNTensorMean_SH_EVIS_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxoNNTensorMean_SH_EVIS_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_true_e, vx_false_e));

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

OnError:
    return status;
}
VX_PRIVATE_API vx_bool vxoNNTensorMean_SH_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && vxoNNTensorMean_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_false_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNTensorMean_SH_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxoNNTensorMean_SH_EVIS_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_false_e, vx_false_e));

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);
OnError:
    return status;
}

VX_PRIVATE_API vx_bool vxoNNTensorMean_SH_TP_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_tensor input     = (vx_tensor)parameters[0];
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_TP, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && vxoNNTensorMean_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_true_e);

    if (!support)return support;

    support = support && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_TP_TRANSPOSE) &&
                            vxnneIsTPSupportFormat(node->base.context, input, VX_NULL, VX_NULL));

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNTensorMean_SH_TP_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxoNNTensorMean_SH_EVIS_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_true_e, vx_true_e));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_status vxoNNLayer_GetOperations(vxnne_layer ops_layer, vx_uint32_ptr max_num_operations, vxnne_operation **operations)
{
    vx_status  status = VX_SUCCESS;
    vxnne_tensor_mean_layer  tensor_mean_layer = (vxnne_tensor_mean_layer)ops_layer;

    *max_num_operations = gcmCOUNTOF(tensor_mean_layer->operations);

    *operations = tensor_mean_layer->operations;

    return status;
}
#endif

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorMean_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
#if REGISTER_FRAME
    vxnne_layer_imp_s registerTensorMeanLayers[] = {/* Please DON'T adjust the order, it's importent */
        { "TensorMean NN", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "TensorMean SH/TP", vxoNNTensorMean_SH_TP_Support, vxoNNTensorMean_SH_TP_Initialize, VX_NULL },
        { "TensorMean SH EVIS", vxoNNTensorMean_SH_EVIS_Support, vxoNNTensorMean_SH_EVIS_Initialize, VX_NULL },
        { "TensorMean SH F32", vxoNNTensorMean_SH_Support, vxoNNTensorMean_SH_Initialize, VX_NULL },
        { "TensorMean SW", vxoNNCommon_Support, vxoNNTensorMean_SW_Initialize, VX_NULL },
    };

    REGISTER_LAYERS(registerTensorMeanLayers, vxnne_tensor_mean_layer_s, "BatchNormalizationLayer", vxoNNLayer_GetOperations);

OnError:
#else

    vx_tensor input     = (vx_tensor)parameters[0];
    vx_tensor axis      = (vx_tensor)parameters[1];
    vx_scalar keep_dims = (vx_scalar)parameters[2];
    vx_tensor output    = (vx_tensor)parameters[3];
    vx_uint32 batchCount = 1;

    vx_int32_ptr axis_base          = VX_NULL;
    vx_int32     resolved_dim[4]    = {-1, -1, -1, -1};
    vx_int32     resolved_dim_count = 0;
    vx_uint32    i                  = 0;
    vx_uint32    j                  = 0;
    vx_uint32   input_axis;
    vx_enum     inputFormat         = TENSOR_DATA_TYPE(input);
    vx_enum     outputFormat        = TENSOR_DATA_TYPE(output);
    vx_uint32   tmpTensorIndex      = 0;
    vx_uint32   operationIdx        = 0;

    vx_bool     shExe_flag          = vx_false_e;

    vx_context  context             = vxGetContext((vx_reference)node);

    vxnne_tensor_mean_layer tensor_mean_layer = VX_NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_tensor_mean_layer_s), (gctPOINTER*)&tensor_mean_layer);
    if (!tensor_mean_layer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(tensor_mean_layer, sizeof(vxnne_tensor_mean_layer_s));

    vxnneLayer_Initialize(&tensor_mean_layer->base,
        "TensorMean",
        node,
        vxmOPERATION_COUNT(tensor_mean_layer),
        tensor_mean_layer->operations,
        VX_NULL);

    vxoTensor_GetTensorViewMemory(axis, (gctPOINTER*)&axis_base, VX_NULL);

    for (i = 0; i < TENSOR_SIZE_INDEX(axis, 0); i++)
    {
        vx_int32 current_axis = axis_base[i] < 0 ? TENSOR_DIM_NUM(input) + axis_base[i] : axis_base[i];

        if (current_axis < 0 || current_axis >= (vx_int32)TENSOR_DIM_NUM(input))
        {
            vxError("error: the axis value must be in the range [0, %d)\n", TENSOR_DIM_NUM(input));
            gcmASSERT(0);
        }

        for (j = 0; j < 4; j++)
        {
            if (resolved_dim[j] == current_axis)
                break;
        }

        if (j == 4)
            resolved_dim[resolved_dim_count++] = current_axis;
    }

    input_axis = 0;

    {
        vx_uint32 dst_elementCount = 0;

        status = vxoTensor_GetTensorElementCount(output, &dst_elementCount);

        if (dst_elementCount == 1) /* reudce mean all*/
        {
            vx_uint32  reshpTensor_Sizes[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {1};
            vx_uint32  reshpTensor_Dims = 2;

            resolved_dim_count = 0;

            vxoElementOptimization_GetTensorShape(input, reshpTensor_Sizes, &reshpTensor_Dims);

            for (i = 0; i < reshpTensor_Dims; i++)
            {
                if (reshpTensor_Sizes[i] != 1)
                {
                    resolved_dim[resolved_dim_count++] = i;
                }
                else
                    break;
            }

            input = vxoTensor_ReshapeTensor((vx_tensor)parameters[0], (vx_int32*)reshpTensor_Sizes, reshpTensor_Dims);

            tensor_mean_layer->base.temp_tensors[tmpTensorIndex++] = input;
            tensor_mean_layer->base.num_temp_tensors = tmpTensorIndex;
        }
    }

    if(context->evisNoInst.supportEVIS)
    {
        shExe_flag = (vx_bool)((inputFormat != VX_TYPE_FLOAT32 && outputFormat != VX_TYPE_FLOAT32 && resolved_dim_count == 2 && resolved_dim[0] <= 3 && resolved_dim[1] <= 3)
                           || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16 && resolved_dim_count == 1 && resolved_dim[0] <= 3)
                           || (inputFormat == VX_TYPE_BFLOAT16 && outputFormat == VX_TYPE_BFLOAT16 && resolved_dim_count == 1 && resolved_dim[0] <= 3)
                           || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT16 && resolved_dim_count == 1 && resolved_dim[0] <= 3)
                           || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT8 && resolved_dim_count == 1 && resolved_dim[0] <= 3)
                           || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_UINT8 && resolved_dim_count == 1 && resolved_dim[0] <= 3)
                           || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16 && resolved_dim_count == 1 && resolved_dim[0] <= 3)
                           || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_FLOAT16 && resolved_dim_count == 1 && resolved_dim[0] <= 3)
                           || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8 && resolved_dim_count == 1 && resolved_dim[0] <= 3)
                           || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_FLOAT16 && resolved_dim_count == 1 && resolved_dim[0] <= 3)
                           || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8 && resolved_dim_count == 1 && resolved_dim[0] <= 3)
                           || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT16 && resolved_dim_count == 1 && resolved_dim[0] <= 3));
    }
    else
    {
        shExe_flag = (vx_bool)((inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32 && resolved_dim_count == 2 && resolved_dim[0] <= 3 && resolved_dim[1] <= 3)
                           || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16 && resolved_dim_count == 1 && resolved_dim[0] <= 3)
                           || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32 && resolved_dim_count == 1 && resolved_dim[0] <= 3)
                           || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16 && resolved_dim_count == 2 && resolved_dim[0] <= 3 && resolved_dim[1] <= 3)
                           || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8 && resolved_dim_count == 2 && resolved_dim[0] <= 3 && resolved_dim[1] <= 3)
                           || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8 && resolved_dim_count == 1 && resolved_dim[0] <= 3));
    }

    if(shExe_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
    {
        vx_tensor_create_params_t tensor_create_params;
        vx_uint32 dims          = TENSOR_DIM_NUM(input);
        vx_uint32 width         = TENSOR_VIEW_SIZE_INDEX(input, 0);
        vx_uint32 height        = dims > 1 ? TENSOR_VIEW_SIZE_INDEX(input, 1) : 1;
        vx_uint32 depth         = dims > 2 ? TENSOR_VIEW_SIZE_INDEX(input, 2) : 1;
        vx_uint32 batch         = dims > 3 ? TENSOR_VIEW_SIZE_INDEX(input, 3) : 1;
        vx_uint32 sizes[4]      = {width, height, depth, batch};
        vx_uint32 new_sizes[4]  = {width, height, depth, batch};
        vx_uint32 perm_array[4] = {0, 1, 2, 3};
        vx_tensor transTensor   = NULL;
        vx_tensor dst           = NULL;
        vx_bool   enable_trans  = vx_false_e;
        vx_bool   enable_axis  = vx_false_e;
        vx_bool   is_trans_sw   = vx_false_e;

        if (resolved_dim_count == 1)
        {
            enable_axis = vx_true_e;
        }

        if (resolved_dim[0] + resolved_dim[1] != 1 && resolved_dim_count == 2)
        {
            enable_trans = vx_true_e;

            if (resolved_dim[0] + resolved_dim[1] == 2)
            {
                perm_array[0] = 0;
                perm_array[1] = 2;
                perm_array[2] = 1;
                perm_array[3] = 3;
            }
            else if (resolved_dim[0] + resolved_dim[1] == 3)
            {
                if (abs(resolved_dim[0] - resolved_dim[1]) == 3)
                {
                    perm_array[0] = 0;
                    perm_array[1] = 3;
                    perm_array[2] = 1;
                    perm_array[3] = 2;
                    is_trans_sw   = vx_true_e;
                }
                else
                {
                    perm_array[0] = 1;
                    perm_array[1] = 2;
                    perm_array[2] = 0;
                    perm_array[3] = 3;
                }
            }
            else if (resolved_dim[0] + resolved_dim[1] == 4)
            {
                perm_array[0] = 1;
                perm_array[1] = 3;
                perm_array[2] = 0;
                perm_array[3] = 2;
                is_trans_sw   = vx_true_e;
            }
            else if (resolved_dim[0] + resolved_dim[1] == 5)
            {
                perm_array[0] = 2;
                perm_array[1] = 3;
                perm_array[2] = 0;
                perm_array[3] = 1;
                is_trans_sw   = vx_true_e;
            }
        }
        else if (resolved_dim[0] != 0 && resolved_dim_count == 1)
        {
            if (resolved_dim[0] == 3)
            {
                enable_trans = vx_true_e;
                input_axis = 0;
                perm_array[0] = 3;
                perm_array[1] = 0;
                perm_array[2] = 1;
                perm_array[3] = 2;
                is_trans_sw   = vx_true_e;
            }
            else
            {
                transTensor = input;
                input_axis = resolved_dim[0];
            }
        }
        else
        {
            transTensor = input;
        }

        if (enable_trans)
        {
            for (i = 0; i < 4; i ++)
            {
                new_sizes[i] = sizes[perm_array[i]];
            }

            gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
            tensor_create_params.num_of_dims = dims;
            tensor_create_params.sizes = new_sizes;
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

            transTensor = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);

            tensor_mean_layer->base.temp_tensors[tmpTensorIndex++] = transTensor;

            if (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_TP_TRANSPOSE) &&
                vxnneIsTPSupportFormat(context, input, VX_NULL, transTensor))
            {
                vx_op_param_s conv = {0};
                vx_uint32 dnum = 4;

                status = vxnneOperation_Initialize(&tensor_mean_layer->tensor_mean_trans_tp_operation.base,
                    &tensor_mean_layer->base,
                    VXNNE_OPERATION_TARGET_TP,
                    VXNNE_OPERATOR_TENSOR_TRANS,
                    VX_NULL,
                    vxnneOperation_TP_Deinitialize,
                    1,
                    0);
                if (status != VX_SUCCESS) goto exit;

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
                conv.other_ref = (vx_reference)input;
                conv.data_buff = gcvNULL;
                conv.tp_value = (vx_tp_value_cmd)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
                conv.tp_value->u32[0] = dnum;
                conv.tp_value->p8[0] = (vx_uint8_ptr)vxAllocateAndZeroMemory(sizeof(vx_uint32) * dnum);
                vxMemCopy(conv.tp_value->p8[0], perm_array, sizeof(vx_uint32) * dnum);

                vxMemCopy(&tensor_mean_layer->tensor_mean_trans_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));

                vxnneLayer_SetOperation(
                    &tensor_mean_layer->base,
                    &tensor_mean_layer->tensor_mean_trans_tp_operation.base,
                    operationIdx++);

                tensor_mean_layer->tensor_mean_trans_tp_operation.input  = input;
                tensor_mean_layer->tensor_mean_trans_tp_operation.output = transTensor;

                vxnneOperation_AddReference(&tensor_mean_layer->tensor_mean_trans_tp_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&tensor_mean_layer->tensor_mean_trans_tp_operation.base, (vx_reference)transTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            }
            else
            {
                vx_bool enable_shader_execute = vx_true_e;
                vxnne_shader_executable shaderExecutable = VX_NULL;
                vx_uint32 pnum = dims;

                if (dims > 4)
                {
                    vx_uint32 i = 0;
                    vx_uint32 elementCnt = 1;

                    for (i = 4; i < dims; i++)
                    {
                        elementCnt *= TENSOR_VIEW_SIZE_INDEX(input, i);
                    }

                    if (elementCnt == 1)
                        pnum = 3;
                    else
                        enable_shader_execute = vx_false_e;
                }

                if (is_trans_sw)
                {
                    enable_shader_execute = vx_false_e;
                }

                if (enable_shader_execute && vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER))
                {
                    if (dims == 4)
                    {
                        pnum = 3;
                    }
                    if(node->base.context->evisNoInst.supportEVIS)
                    {
                        shaderExecutable = vxnneTensorTransposeShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_TRANSPOSE, &node->kernelAttributes.borderMode, input, perm_array, pnum, transTensor);
                    }
                    else
                    {
                        shaderExecutable = vxnneGPUTensorTransposeShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_TRANSPOSE, &node->kernelAttributes.borderMode, input, perm_array, pnum, transTensor);
                    }

                    if (!shaderExecutable)
                    {
                        status = VX_FAILURE;
                        goto exit;
                    }
                    batchCount = new_sizes[3];
                    status = vxnneShaderOperation_Initialize(&tensor_mean_layer->tensor_mean_trans_sh_operation,
                        &tensor_mean_layer->base,
                        VXNNE_OPERATOR_TENSOR_TRANS,
                        batchCount,
                        shaderExecutable);

                if (status != VX_SUCCESS)
                    goto exit;

                vxnneOperation_AddReference(&tensor_mean_layer->tensor_mean_trans_sh_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&tensor_mean_layer->tensor_mean_trans_sh_operation.base, (vx_reference)transTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                vxnneLayer_SetOperation(
                        &tensor_mean_layer->base,
                        &tensor_mean_layer->tensor_mean_trans_sh_operation.base,
                        operationIdx++);
                }
                else
                {
                    vx_array perm = vxCreateArray(context, VX_TYPE_UINT32, pnum);
                    vx_scalar pnum_s = NULL;

                    if (!vxoArray_AllocateMemory(perm))
                    {
                        status = VX_ERROR_NO_MEMORY;
                        vxError("Fail to vxoArray_AllocateMemory of perm function %s line %d", __FUNCTION__, __LINE__);
                        goto exit;
                    }
                    else
                    {
                        vx_uint32* pos = (vx_uint32*)perm->memory.logicals[0];
                        memcpy(pos, perm_array, pnum * sizeof(vx_uint32));
                    }

                    pnum_s = vxCreateScalar(context, VX_TYPE_UINT32, &pnum);

                    batchCount = new_sizes[3];
                    vxnneOperation_Initialize(&tensor_mean_layer->tensor_trans_sw_operation.base,
                                              &tensor_mean_layer->base,
                                              VXNNE_OPERATION_TARGET_SW,
                                              VXNNE_OPERATOR_TENSOR_TRANS,
                                              vxnneExecuteSWTensorTranspose,
                                              VX_NULL,
                                              batchCount,
                                              0);

                    vxnneLayer_SetOperation(
                        &tensor_mean_layer->base,
                        &tensor_mean_layer->tensor_trans_sw_operation.base,
                        operationIdx++);

                    tensor_mean_layer->tensor_trans_sw_operation.input   = input;
                    tensor_mean_layer->tensor_trans_sw_operation.perm    = perm;
                    tensor_mean_layer->tensor_trans_sw_operation.pnum    = pnum_s;
                    tensor_mean_layer->tensor_trans_sw_operation.output  = transTensor;

                    vxnneOperation_AddReference(&tensor_mean_layer->tensor_trans_sw_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
                    vxnneOperation_AddReference(&tensor_mean_layer->tensor_trans_sw_operation.base, (vx_reference)transTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);
                }
            }
        }

        if (resolved_dim_count == 2)
        {
            vx_uint32 output_dims         = dims < 3 ? 3 : dims;

            sizes[0] = 1;
            sizes[1] = 1;
            sizes[2] = new_sizes[2];
            sizes[3] = new_sizes[3];
            dst = vxoTensor_ReshapeTensor(output, (vx_int32*)sizes, output_dims);

            tensor_mean_layer->base.temp_tensors[tmpTensorIndex++] = dst;
        }
        else if (resolved_dim_count == 1)
        {
            vx_uint32 output_dims         = dims < 3 ? 3 : dims;

            sizes[0] = new_sizes[0];
            sizes[1] = new_sizes[1];
            sizes[2] = new_sizes[2];
            sizes[3] = new_sizes[3];
            sizes[input_axis] = 1;
            dst = vxoTensor_ReshapeTensor(output, (vx_int32*)sizes, output_dims);

            tensor_mean_layer->base.temp_tensors[tmpTensorIndex++] = dst;
        }

        if (resolved_dim_count == 2)
        {
            vxnne_shader_executable shaderExecutable = NULL;
            vx_bool   enable_tf_quantize  = (vx_bool)(inputFormat == VX_TYPE_UINT8);
            vx_bool   enable_int16_sh     = (vx_bool)(inputFormat == VX_TYPE_INT16);
            vx_uint32 stride              = 1;
            vx_scalar stride_s            = vxCreateScalar(node->base.context, VX_TYPE_UINT32, &stride);
            vx_scalar poolSizeX           = vxCreateScalar(node->base.context, VX_TYPE_UINT32, &new_sizes[0]);
            vx_scalar poolSizeY           = vxCreateScalar(node->base.context, VX_TYPE_UINT32, &new_sizes[1]);
            vx_uint32 pad_x_left          = 0;
            vx_uint32 pad_y_top           = 0;
            vx_uint32 batch               = new_sizes[3];

            if (stride_s == NULL || poolSizeX == NULL || poolSizeY == NULL)
            {
                if (stride_s)  vxReleaseScalar(&stride_s);
                if (poolSizeX) vxReleaseScalar(&poolSizeX);
                if (poolSizeY) vxReleaseScalar(&poolSizeY);

                status = VX_FAILURE;
                goto exit;
            }

            if(node->base.context->evisNoInst.supportEVIS)
            {
                if(enable_tf_quantize)
                    shaderExecutable = vxnneGetAvgPooling_UInt8ShaderExecutable(node->base.context, VXNNE_KERNEL_AVGPOOLING_UINT8, &node->kernelAttributes.borderMode,
                    transTensor, NULL, stride_s, stride_s, poolSizeX, poolSizeY, pad_x_left, pad_y_top, NULL, VX_NN_ACTIVATION_NONE, dst);
                else if(enable_int16_sh)
                    shaderExecutable = vxnneGetAvgPooling_Int16ShaderExecutable(node->base.context, VXNNE_KERNEL_AVGPOOLING_INT16, &node->kernelAttributes.borderMode,
                    transTensor, NULL, stride_s, stride_s, poolSizeX, poolSizeY, pad_x_left, pad_y_top, NULL, VX_NN_ACTIVATION_NONE, dst);
                else
                    shaderExecutable = vxnneGetAvgPoolingShaderExecutable(node->base.context, VXNNE_KERNEL_AVGPOOLING, &node->kernelAttributes.borderMode,
                    transTensor, NULL, stride_s, stride_s, poolSizeX, poolSizeY, pad_x_left, pad_y_top, NULL, dst);
            }
            else
            {
                shaderExecutable = vxnneGetGPUAvgPoolingShaderExecutable(node->base.context, VXNNE_KERNEL_AVGPOOLING, &node->kernelAttributes.borderMode,
                    transTensor, NULL, stride_s, stride_s, poolSizeX, poolSizeY, pad_x_left, pad_y_top, pad_x_left, pad_y_top, NULL, vx_false_e, 0, 0, dst);
            }

            if (stride_s)  vxReleaseScalar(&stride_s);
            if (poolSizeX) vxReleaseScalar(&poolSizeX);
            if (poolSizeY) vxReleaseScalar(&poolSizeY);

            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto exit;
            }

            status = vxnneShaderOperation_Initialize(&tensor_mean_layer->tensor_mean_pool_sh_operation,
                &tensor_mean_layer->base,
                VXNNE_OPERATOR_POOLING,
                batch,
                shaderExecutable);

            if (status != VX_SUCCESS)
            {
                goto exit;
            }

            vxnneOperation_AddReference(&tensor_mean_layer->tensor_mean_pool_sh_operation.base, (vx_reference)transTensor, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&tensor_mean_layer->tensor_mean_pool_sh_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            vxnneLayer_SetOperation(
                &tensor_mean_layer->base,
                &tensor_mean_layer->tensor_mean_pool_sh_operation.base,
                operationIdx++);
        }

        if (enable_axis)
        {
            vxnne_shader_executable shaderExecutable = NULL;
            vx_float32              axis_coef        = 1.0f / (vx_float32)(TENSOR_VIEW_SIZE_INDEX(transTensor,input_axis));
            vx_uint32               batch            = new_sizes[3];

            if(node->base.context->evisNoInst.supportEVIS)
            {
                shaderExecutable = vxnneGetTensorMeanAxisShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_MEAN_AXIS, &node->kernelAttributes.borderMode, axis_coef, transTensor, dst, input_axis);
            }
            else
            {
                shaderExecutable = vxnneGetGPUTensorMeanAxisShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_MEAN_AXIS, &node->kernelAttributes.borderMode, axis_coef, transTensor, dst, input_axis);
            }

            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto exit;
            }

            status = vxnneShaderOperation_Initialize(&tensor_mean_layer->tensor_mean_axis0_sh_operation,
                &tensor_mean_layer->base,
                VXNNE_OPERATOR_TENSOR_MEAN,
                batch,
                shaderExecutable);

            if (status != VX_SUCCESS)
            {
                goto exit;
            }

            vxnneOperation_AddReference(&tensor_mean_layer->tensor_mean_axis0_sh_operation.base, (vx_reference)transTensor, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&tensor_mean_layer->tensor_mean_axis0_sh_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            vxnneLayer_SetOperation(
                &tensor_mean_layer->base,
                &tensor_mean_layer->tensor_mean_axis0_sh_operation.base,
                operationIdx++);
        }

        tensor_mean_layer->base.num_temp_tensors = tmpTensorIndex;
    }
    else
    {
        vx_uint32 dims          = TENSOR_DIM_NUM(input);
        vx_uint32 batch         = dims > 3 ? TENSOR_VIEW_SIZE_INDEX(input, 3) : 1;

        batchCount = batch;
        vxnneOperation_Initialize(&tensor_mean_layer->tensor_mean_sw_operation.base,
            &tensor_mean_layer->base,
            VXNNE_OPERATION_TARGET_SW,
            VXNNE_OPERATOR_TENSOR_MEAN,
            vxnneExecuteSWTensorMean,
            VX_NULL,
            batchCount,
            0);

        vxnneLayer_SetOperation(
            &tensor_mean_layer->base,
            &tensor_mean_layer->tensor_mean_sw_operation.base,
            0);

        tensor_mean_layer->tensor_mean_sw_operation.input     = input;
        tensor_mean_layer->tensor_mean_sw_operation.axis      = axis;
        tensor_mean_layer->tensor_mean_sw_operation.keep_dims = keep_dims;
        tensor_mean_layer->tensor_mean_sw_operation.output    = output;

        vxnneOperation_AddReference(&tensor_mean_layer->tensor_mean_sw_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&tensor_mean_layer->tensor_mean_sw_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }

    node->layer = &tensor_mean_layer->base;
    return status;

exit:
    if (tensor_mean_layer)
        gcoOS_Free(NULL, (gctPOINTER)tensor_mean_layer);
#endif

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorMean_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

