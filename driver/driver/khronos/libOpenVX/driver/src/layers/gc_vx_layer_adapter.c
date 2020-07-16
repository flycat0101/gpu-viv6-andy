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



#include <gc_vxk_common.h>
#include <gc_vx_nn_util.h>
#include <gc_vx_common.h>
#include <layers/gc_vx_layer_adapter.h>

#define INPUT_SIZE_ALIGN_4  (4)
/***************************************************************************************************************************
 *                                                 Adapter
 ***************************************************************************************************************************/
vx_status vxnneAdapter_SWCWHN2WHCN(
            vx_uint8_ptr input_ptr, vx_type_e input_format, vx_enum input_quant_type, vx_uint32 input_depth, vx_uint32 input_width, vx_uint32 input_height,
            vx_uint32 input_batch, vx_int8 in_fixpoint, vx_int32 in_tf_zp, vx_float32 in_tf_scale,
            vx_uint8_ptr output_ptr, vx_type_e output_format, vx_enum output_quant_type, vx_uint32 output_depth, vx_uint32 output_width, vx_uint32 output_height,
            vx_int8 out_fixpoint, vx_int32 out_tf_zp, vx_float32 out_tf_scale, vx_enum out_rounding_mode)
{
    vx_status status = VX_SUCCESS;

    vx_uint32 batch = 0, in_h = 0, in_w = 0;
    vx_float32 data = .0f;
    /*vx_int32 in_item_size = vxnneGetTypeSize(input_format); */

    /**************************************************************************************************
     *       C W H N                                      W H C N
     *       2 4 4 1             =>                       4 4 2 1
     *   ___________________            ___________________         ___________________
     *  |10, |11, |12, |13, |          | 10 | 11 | 12 | 13 |       | 20 | 21 | 22 | 23 |
     *  |__20|__21|__22|__23|          |____|____|____|____|       |____|____|____|____|
     *  |14, |15, |16, |17, |          | 14 | 15 | 16 | 17 |       | 24 | 25 | 26 | 27 |
     *  |__24|__25|__26|__27|    =>    |____|____|____|____|       |____|____|____|____|
     *  |18, |19, |110,|111,|          | 18 | 19 | 110| 111|       | 28 | 29 | 210| 211|
     *  |__28|__29|_210|_211|          |____|____|____|____|       |____|____|____|____|
     *  |112,|113,|114,|115,|          | 112| 113| 114| 115|       | 212| 213| 214| 215|
     *  |_212|_213|_214|_215|          |____|____|____|____|       |____|____|____|____|
     *
     **************************************************************************************************/

    for (batch = 0; batch < input_batch; ++ batch)
    {
        vx_uint32 output_batch_index = batch * output_height * output_width * output_depth;
        vx_uint32 input_batch_index = batch * input_height * input_width * input_depth;

        {
            for (in_h = 0; in_h < input_height; ++ in_h)
            {
                for (in_w = 0; in_w < (input_width * input_depth); in_w ++)
                {
                    vx_int32 out_w = in_w / input_depth;
                    vx_int32 out_h = in_h;
                    vx_int32 out_d = in_w % input_depth;

                    vx_int32 in_index = in_w + in_h * input_width * input_depth + input_batch_index;
                    vx_int32 out_index = (out_w + out_h * output_width) + out_d * output_width * output_height + output_batch_index;

                    /*comment the direct copy, because of dst's quantized parameter may be different from src's*/
                    /*if (in_item_size == vxnneGetTypeSize(output_format))
                    {
                        memcpy(output_ptr + out_index * in_item_size, input_ptr + in_index * in_item_size, in_item_size);
                    }
                    else*/
                    {
                        data = vxnneGetDataExt(input_format, input_quant_type, in_index, input_ptr, in_fixpoint, in_tf_zp, in_tf_scale);

                        vxnneSaveDataExt(output_format, output_quant_type, out_index, data, output_ptr, out_fixpoint, out_tf_zp, out_tf_scale, out_rounding_mode);


                    }
                }
            }
        }
    }

    return status;
}

vx_status vxnneAdapter_Tensor_CWHN2WHCN(vx_tensor inputs, vx_tensor outputs)
{
    vx_status status = VX_SUCCESS;


    vx_type_e  inputFormat      = (vx_type_e)TENSOR_DATA_TYPE(inputs);
    vx_type_e  outputFormat     = (vx_type_e)TENSOR_DATA_TYPE(outputs);

    vx_uint8_ptr inputBase;
    vx_uint8_ptr outputBase;


    vx_uint32 input_batch = TENSOR_SIZE_INDEX(inputs, 0);  /* N */
    vx_uint32 input_height = TENSOR_SIZE_INDEX(inputs, 1); /* H */
    vx_uint32 input_width = TENSOR_SIZE_INDEX(inputs, 2);  /* W */
    vx_uint32 input_depth = TENSOR_SIZE_INDEX(inputs, 3);  /* C */

    vx_uint32 output_width = TENSOR_SIZE_INDEX(outputs, 0);  /* W */
    vx_uint32 output_height = TENSOR_SIZE_INDEX(outputs, 1); /* H */
    vx_uint32 output_depth = TENSOR_SIZE_INDEX(outputs, 2);  /* C */
    vx_uint32 output_batch = TENSOR_SIZE_INDEX(outputs, 3);  /* N */


    vxoTensor_GetTensorViewMemory(inputs, (gctPOINTER*)&inputBase, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, (gctPOINTER*)&outputBase, VX_NULL);

    gcmASSERT(input_width == output_width);
    gcmASSERT(input_height == output_height);
    gcmASSERT(output_depth == input_depth);
    if (output_batch != input_batch)
    {
        gcmASSERT(0);
    }

    //outputBase = TENSOR_LOGICAL_ADDR(outputs);
    //inputBase = TENSOR_LOGICAL_ADDR(inputs);


    status = vxnneAdapter_SWCWHN2WHCN(inputBase, inputFormat, TENSOR_QUANT_TYPE(inputs), input_depth, input_width, input_height, input_batch, TENSOR_POS(inputs), TENSOR_TF_ZEROPOINT(inputs), TENSOR_TF_SCALE(inputs),
        outputBase, outputFormat, TENSOR_QUANT_TYPE(outputs), output_depth, output_width, output_height, TENSOR_POS(outputs), TENSOR_TF_ZEROPOINT(outputs), TENSOR_TF_SCALE(outputs), TENSOR_ROUNDING_MODE(outputs));

    return status;
}

vx_status vxnneAdapter_CWHN2WHCN(struct _vxnne_operation_s *operation)
{

    vxnne_adapter_operation adapterOperation   = (vxnne_adapter_operation)operation;

    vx_tensor  inputs           = (vx_tensor)adapterOperation->inputs;
    vx_tensor  outputs          = (vx_tensor)adapterOperation->outputs;

    return vxnneAdapter_Tensor_CWHN2WHCN(inputs, outputs);
}


vx_status vxnneAdapter_WHCN2CWHN(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;

    vxnne_adapter_operation adapterOperation   = (vxnne_adapter_operation)operation;

    vx_tensor  inputs           = (vx_tensor)adapterOperation->inputs;
    vx_tensor  outputs          = (vx_tensor)adapterOperation->outputs;


    vx_uint8_ptr inputBase;
    vx_uint8_ptr outputBase;


    vx_uint32 input_width = TENSOR_SIZE_INDEX(inputs, 0);  /* W */
    vx_uint32 input_height = TENSOR_SIZE_INDEX(inputs, 1); /* H */
    vx_uint32 input_depth = TENSOR_SIZE_INDEX(inputs, 2);  /* C */
    vx_uint32 input_batch = TENSOR_SIZE_INDEX(inputs, 3);  /* N */

    vx_uint32 output_batch = TENSOR_SIZE_INDEX(outputs, 0);  /* N */
    vx_uint32 output_height = TENSOR_SIZE_INDEX(outputs, 1); /* H */
    vx_uint32 output_width = TENSOR_SIZE_INDEX(outputs, 2);  /* W */
    vx_uint32 output_depth = TENSOR_SIZE_INDEX(outputs, 3);  /* C */

    /*vx_int32 item_size = vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(outputs));*/

    vx_uint32 batch = 0, in_h = 0, in_w = 0, in_d = 0;
    vx_float32 data = .0f;

    vxoTensor_GetTensorViewMemory(inputs, (gctPOINTER*)&inputBase, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, (gctPOINTER*)&outputBase, VX_NULL);

    gcmASSERT(input_width == output_width);
    gcmASSERT(input_height == output_height);
    gcmASSERT(output_depth == input_depth);
    if (output_batch != input_batch)
    {
        gcmASSERT(0);
    }
    //outputBase = TENSOR_LOGICAL_ADDR(outputs);
    //inputBase = TENSOR_LOGICAL_ADDR(inputs);


    /**************************************************************************************************
     *                       W H C N                                        C W H N
     *                       4 4 2 1                          =>            2 4 4 1
     *   ___________________         ___________________               ___________________
     *  | 10 | 11 | 12 | 13 |       | 20 | 21 | 22 | 23 |             |10, |11, |12, |13, |
     *  |____|____|____|____|       |____|____|____|____|             |__20|__21|__22|__23|
     *  | 14 | 15 | 16 | 17 |       | 24 | 25 | 26 | 27 |             |14, |15, |16, |17, |
     *  |____|____|____|____|       |____|____|____|____|     =>      |__24|__25|__26|__27|
     *  | 18 | 19 | 110| 111|       | 28 | 29 | 210| 211|             |18, |19, |110,|111,|
     *  |____|____|____|____|       |____|____|____|____|             |__28|__29|_210|_211|
     *  | 112| 113| 114| 115|       | 212| 213| 214| 215|             |112,|113,|114,|115,|
     *  |____|____|____|____|       |____|____|____|____|             |_212|_213|_214|_215|
     *
     **************************************************************************************************/

    for (batch = 0; batch < input_batch; ++ batch)
    {
        vx_uint32 output_batch_index = batch * output_height * output_width * output_depth;
        vx_uint32 input_batch_index = batch * input_height * input_width * input_depth;

        for (in_d = 0; in_d < input_depth; in_d ++)
        {
            for (in_h = 0; in_h < input_height; ++ in_h)
            {
                for (in_w = 0; in_w < input_width; in_w ++)
                {
                    vx_int32 out_w = in_w * input_depth;
                    vx_int32 out_h = in_h;

                    vx_int32 in_index = in_w + in_h * input_width + in_d * input_width * input_height + input_batch_index;

                    vx_int32 out_index = out_w + in_d + out_h * output_width * input_depth + output_batch_index;

                    /*comment the direct copy, because of dst's quantized parameter may be different from src's*/
                    /*if (item_size == vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(inputs)))
                    {
                        memcpy(outputBase + out_index * item_size, inputBase + in_index * item_size, item_size);
                    }
                    else*/
                    {

                        data = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(inputs), TENSOR_QUANT_TYPE(inputs), in_index, inputBase, TENSOR_POS(inputs), TENSOR_TF_ZEROPOINT(inputs), TENSOR_TF_SCALE(inputs));
                        vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(outputs), TENSOR_QUANT_TYPE(outputs), out_index, data, outputBase, TENSOR_POS(outputs), TENSOR_TF_ZEROPOINT(outputs), TENSOR_TF_SCALE(outputs), TENSOR_ROUNDING_MODE(outputs));
                    }
                }
            }
        }
    }
    return status;
}


vx_status vxnneAdapter_Tensor_FormatConvert(vx_tensor inputs, vx_tensor outputs)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i = 0;
    vx_float32 data = 0;
    const vx_uint32 input_width = TENSOR_SIZE_INDEX(inputs, 0);  /* W */
    const vx_uint32 input_height = TENSOR_SIZE_INDEX(inputs, 1); /* H */
    const vx_uint32 input_depth = TENSOR_SIZE_INDEX(inputs, 2);  /* C */
    const vx_uint32 input_batch = TENSOR_SIZE_INDEX(inputs, 3);  /* N */
    vx_uint8_ptr input_base = VX_NULL, output_base = VX_NULL;

    vxoTensor_GetTensorViewMemory(inputs, (gctPOINTER *)&input_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, (gctPOINTER *)&output_base, VX_NULL);

    for (i = 0; i < input_width * input_height * input_depth * input_batch; i ++)
    {
        data = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(inputs), TENSOR_QUANT_TYPE(inputs), i, input_base, TENSOR_POS(inputs), TENSOR_TF_ZEROPOINT(inputs), TENSOR_TF_SCALE(inputs));
        status |= vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(outputs), TENSOR_QUANT_TYPE(outputs), i, data, output_base, TENSOR_POS(outputs), TENSOR_TF_ZEROPOINT(outputs), TENSOR_TF_SCALE(outputs), TENSOR_ROUNDING_MODE(outputs));
    }

    return status;
}

vx_status vxnneExecuteSWAdapter(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_adapter_operation adapterOperation   = (vxnne_adapter_operation)operation;

    vx_tensor  inputs           = (vx_tensor)adapterOperation->inputs;
    vx_scalar  types            = (vx_scalar)adapterOperation->type;
    vx_tensor  outputs          = (vx_tensor)adapterOperation->outputs;

    vx_enum    type             = types->value->e;

    vxSetTensorAttribute(outputs, VX_TENSOR_VALUE, &TENSOR_VALUED(inputs), sizeof(vx_bool));

    switch (type)
    {
        case VX_ADAPTER_CWHN_TO_WHCN:
            vxnneAdapter_CWHN2WHCN(operation);
            break;
        case VX_ADAPTER_WHCN_TO_CWHN:
            vxnneAdapter_WHCN2CWHN(operation);
            break;

        case VX_ADAPTER_F32_TO_F16:
        case VX_ADAPTER_F16_TO_F32:
            status = vxnneAdapter_Tensor_FormatConvert(inputs, outputs);
            break;

        default:
            break;
    }

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNAdapter(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoAdapter_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxoAdapter_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}
#if REGISTER_FRAME

VX_PRIVATE_API vx_status vxoNNadapterLayer_SW_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vxnne_adapter_layer  adapterLayer = (vxnne_adapter_layer)ops_layer;

    vx_tensor  inputs = (vx_tensor)parameters[0];
    vx_scalar  type_s = (vx_scalar)parameters[1];
    vx_tensor  outputs = (vx_tensor)parameters[2];
    vx_uint32  batchCount = 1;/*TENSOR_SIZE_INDEX(inputs, 3);*/

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxnneOperation_Initialize(&adapterLayer->adapter_sw_operation.base,
        &adapterLayer->base,
        VXNNE_OPERATION_TARGET_SW,
        VXNNE_OPERATOR_ADAPTER,
        vxnneExecuteSWAdapter,
        VX_NULL,
        batchCount,
        0));

    vxmONERROR(vxnneLayer_SetOperation(
        &adapterLayer->base,
        &adapterLayer->adapter_sw_operation.base,
        0));

    adapterLayer->adapter_sw_operation.inputs = inputs;
    adapterLayer->adapter_sw_operation.type = type_s;
    adapterLayer->adapter_sw_operation.outputs = outputs;

    vxmONERROR(vxnneOperation_AddReference(&adapterLayer->adapter_sw_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&adapterLayer->adapter_sw_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNadapterLayer_SH_EVIS_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_tensor  inputs = (vx_tensor)parameters[0];
    vx_scalar  type_s = (vx_scalar)parameters[1];
    vx_tensor  outputs = (vx_tensor)parameters[2];
    vx_enum    inputFormat = TENSOR_DATA_TYPE(inputs);
    vx_enum    outputFormat = TENSOR_DATA_TYPE(outputs);
    vx_enum    type = type_s->value->e;
    vx_bool    enable_dataFormat = vx_false_e;
    vx_bool    enable_dataConvert = vx_false_e;
    vx_uint32  dims = TENSOR_VIEW_DIM_NUM(inputs);
    vx_uint32  width = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
    vx_uint32  height = dims > 1 ? TENSOR_VIEW_SIZE_INDEX(inputs, 1) : 1;
    vx_uint32  depth = dims > 2 ? TENSOR_VIEW_SIZE_INDEX(inputs, 2) : 1;
    vx_uint32  batch = dims > 3 ? TENSOR_VIEW_SIZE_INDEX(inputs, 3) : 1;

    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && node->base.context->evisNoInst.supportEVIS;

    if (!support)return support;

    reg_param->flag = 0;

    enable_dataFormat = (vx_bool)((inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
        || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT32)
        || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT16));

    if ((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT32)
        || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT16))
    {
        if (type == VX_ADAPTER_CWHN_TO_WHCN)
        {
            batch = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
            depth = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
            height = TENSOR_VIEW_SIZE_INDEX(inputs, 2);
            width = TENSOR_VIEW_SIZE_INDEX(inputs, 3);
        }
        else if (type == VX_ADAPTER_WHCN_TO_CWHN)
        {
            batch = TENSOR_SIZE_INDEX(outputs, 0);
            depth = TENSOR_SIZE_INDEX(outputs, 1);
            height = TENSOR_SIZE_INDEX(outputs, 2);
            width = TENSOR_SIZE_INDEX(outputs, 3);
        }

        if (width * height * depth * batch < IMG_MAX_WIDTH)
        {
            enable_dataConvert = vx_true_e;
        }
        else if ((width * height * depth < IMG_MAX_WIDTH) && (width * height * depth % INPUT_SIZE_ALIGN_4 == 0))
        {
            enable_dataConvert = vx_true_e;
        }
        else if ((width * height < IMG_MAX_WIDTH) && (width * height % INPUT_SIZE_ALIGN_4 == 0))
        {
            enable_dataConvert = vx_true_e;
        }
        else if (width % INPUT_SIZE_ALIGN_4 == 0)
        {
            enable_dataConvert = vx_true_e;
        }
        else
        {
            enable_dataFormat = vx_false_e;
        }
    }

    support = support && (enable_dataFormat || enable_dataConvert);

    if (support)
    {
        SETBIT(reg_param->flag, ((enable_dataFormat == vx_true_e) ? 1 : 0), 0);
        SETBIT(reg_param->flag, ((enable_dataConvert == vx_true_e) ? 1 : 0), 1);
    }

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNadapterLayer_SH_Initialize_Ext(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 _num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_status status = VX_SUCCESS;
    vxnne_adapter_layer  adapterLayer = (vxnne_adapter_layer)ops_layer;

    vx_tensor  inputs = (vx_tensor)parameters[0];
    vx_scalar  type_s = (vx_scalar)parameters[1];
    vx_tensor  outputs = (vx_tensor)parameters[2];
    vx_tensor  src = NULL;
    vx_tensor  dst = NULL;
    vx_tensor  temp_tensor[2] = { NULL };
    vx_enum    type = type_s->value->e;
    vx_uint32  perm_array[4] = { 0, 1, 2, 3 };
    vx_uint32  dnum = 0;
    vx_bool    shExe_flag = vx_true_e;
    vx_bool    enable_dataFormat = GETBIT(reg_param->flag, 0);
    vx_bool    enable_dataConvert = GETBIT(reg_param->flag, 1);
    vx_uint32  dims = TENSOR_VIEW_DIM_NUM(inputs);
    vx_uint32  width = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
    vx_uint32  height = dims > 1 ? TENSOR_VIEW_SIZE_INDEX(inputs, 1) : 1;
    vx_uint32  depth = dims > 2 ? TENSOR_VIEW_SIZE_INDEX(inputs, 2) : 1;
    vx_uint32  batch = dims > 3 ? TENSOR_VIEW_SIZE_INDEX(inputs, 3) : 1;
    vx_uint32  temp_tensor_idx = 0;

    vx_uint32_ptr pPerm = VX_NULL;
    vx_uint32     num = 0;
    vx_int32      sizes[] = { 1, 1, 1, 1 };
    vx_uint32     convOPIdx = 0;
    vx_uint32     transOPIdx = 0;
    vx_uint32     batchCount0 = 1;
    vx_uint32     batchCount1 = 1;

    vxoLayer_InitializeHead(ops_layer, parameters, _num, reg_param);

    if (enable_dataConvert)
    {
        if (width * height * depth * batch < IMG_MAX_WIDTH)
        {
            sizes[0] = width * height * depth * batch;
            sizes[1] = 1;

            batchCount0 = 1;
        }
        else if ((width * height * depth < IMG_MAX_WIDTH) && batch < IMG_MAX_WIDTH && (width * height * depth % INPUT_SIZE_ALIGN_4 == 0))
        {
            sizes[0] = width * height * depth;
            sizes[1] = batch;

            batchCount0 = 1;
        }
        else if ((width * height < IMG_MAX_WIDTH) && (depth * batch < IMG_MAX_WIDTH) && (width * height % INPUT_SIZE_ALIGN_4 == 0))
        {
            sizes[0] = width * height;
            sizes[1] = depth * batch;

            batchCount0 = 1;
        }
        else if ((width * height < IMG_MAX_WIDTH) && depth  < IMG_MAX_WIDTH && (width * height % INPUT_SIZE_ALIGN_4 == 0))
        {
            sizes[0] = width * height;
            sizes[1] = depth;
            sizes[2] = 1;
            sizes[3] = batch;

            batchCount0 = batch;
        }
        else
        {
            sizes[0] = width;
            sizes[1] = height;
            sizes[2] = depth;
            sizes[3] = batch;

            batchCount0 = batch;
        }

        if (type == VX_ADAPTER_F16_TO_F32 || type == VX_ADAPTER_F32_TO_F16)
        {
            temp_tensor[0] = vxoTensor_ReshapeTensor(inputs, sizes, dims);
            temp_tensor[1] = vxoTensor_ReshapeTensor(outputs, sizes, dims);
        }
        else if (type == VX_ADAPTER_CWHN_TO_WHCN)
        {
            vx_tensor_create_params_t tensor_create_params;

            gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
            tensor_create_params.num_of_dims = TENSOR_DIM_NUM(inputs);
            tensor_create_params.sizes = (vx_uint32 *)sizes;
            tensor_create_params.data_format = VX_TYPE_FLOAT16;
            tensor_create_params.quant_format = TENSOR_QUANT_TYPE(inputs);

            temp_tensor[0] = vxoTensor_ReshapeTensor(inputs, sizes, dims);
            temp_tensor[1] = vxoTensor_CreateTensor(ops_layer->node->base.context, ops_layer->node->graph, &tensor_create_params, vx_true_e);

            convOPIdx = 0;
            transOPIdx = 1;
        }
        else if (type == VX_ADAPTER_WHCN_TO_CWHN)
        {
            vx_tensor_create_params_t tensor_create_params;

            gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
            tensor_create_params.num_of_dims = TENSOR_DIM_NUM(inputs);
            tensor_create_params.sizes = (vx_uint32 *)sizes;
            tensor_create_params.data_format = VX_TYPE_FLOAT16;
            tensor_create_params.quant_format = TENSOR_QUANT_TYPE(inputs);

            temp_tensor[0] = vxoTensor_CreateTensor(ops_layer->node->base.context, ops_layer->node->graph, &tensor_create_params, vx_true_e);
            temp_tensor[1] = vxoTensor_ReshapeTensor(outputs, sizes, dims);

            convOPIdx = 1;
            transOPIdx = 0;
        }

        adapterLayer->base.temp_tensors[temp_tensor_idx++] = temp_tensor[0];
        adapterLayer->base.temp_tensors[temp_tensor_idx++] = temp_tensor[1];

        enable_dataConvert = vx_true_e;
    }
    else
    {
        temp_tensor[0] = outputs;
        temp_tensor[1] = inputs;
    }

    if (type == VX_ADAPTER_CWHN_TO_WHCN)
    {
        vx_uint32 input_batch = TENSOR_VIEW_SIZE_INDEX(inputs, 0);  /* N */
        vx_uint32 input_height = TENSOR_VIEW_SIZE_INDEX(inputs, 1); /* H */
        vx_uint32 input_width = TENSOR_VIEW_SIZE_INDEX(inputs, 2);  /* W */
        vx_uint32 input_depth = TENSOR_VIEW_SIZE_INDEX(inputs, 3);  /* C */
        vx_uint32 output_width = TENSOR_SIZE_INDEX(outputs, 0);  /* W */
        vx_uint32 output_height = TENSOR_SIZE_INDEX(outputs, 1); /* H */
        vx_uint32 output_depth = TENSOR_SIZE_INDEX(outputs, 2);  /* C */
        vx_uint32 output_batch = TENSOR_SIZE_INDEX(outputs, 3);  /* N */
        dims = TENSOR_VIEW_DIM_NUM(inputs);

        sizes[0] = input_depth;
        sizes[1] = input_width;
        sizes[2] = input_height;
        sizes[3] = input_batch;
        src = vxoTensor_ReshapeTensor(temp_tensor[1], (vx_int32*)sizes, dims);

        sizes[0] = output_width;
        sizes[1] = output_height;
        sizes[2] = output_depth;
        sizes[3] = output_batch;
        dst = vxoTensor_ReshapeTensor(outputs, (vx_int32*)sizes, dims);

        perm_array[0] = 1;
        perm_array[1] = 2;
        perm_array[2] = 0;
        perm_array[3] = 3;

        dnum = 3;

        batchCount1 = dims > 3 ? output_batch : 1;
    }
    else if (type == VX_ADAPTER_WHCN_TO_CWHN)
    {
        vx_uint32 input_width = TENSOR_SIZE_INDEX(inputs, 0);  /* W */
        vx_uint32 input_height = TENSOR_SIZE_INDEX(inputs, 1); /* H */
        vx_uint32 input_depth = TENSOR_SIZE_INDEX(inputs, 2);  /* C */
        vx_uint32 input_batch = TENSOR_SIZE_INDEX(inputs, 3);  /* N */
        vx_uint32 output_batch = TENSOR_SIZE_INDEX(outputs, 0);  /* N */
        vx_uint32 output_height = TENSOR_SIZE_INDEX(outputs, 1); /* H */
        vx_uint32 output_width = TENSOR_SIZE_INDEX(outputs, 2);  /* W */
        vx_uint32 output_depth = TENSOR_SIZE_INDEX(outputs, 3);  /* C */
        dims = TENSOR_VIEW_DIM_NUM(inputs);

        sizes[0] = input_width;
        sizes[1] = input_height;
        sizes[2] = input_depth;
        sizes[3] = input_batch;
        src = vxoTensor_ReshapeTensor(inputs, (vx_int32*)sizes, dims);

        sizes[0] = output_depth;
        sizes[1] = output_width;
        sizes[2] = output_height;
        sizes[3] = output_batch;
        dst = vxoTensor_ReshapeTensor(temp_tensor[0], (vx_int32*)sizes, dims);

        perm_array[0] = 2;
        perm_array[1] = 0;
        perm_array[2] = 1;
        perm_array[3] = 3;

        dnum = 3;

        batchCount1 = dims > 3 ? input_batch : 1;
    }

    if (type == VX_ADAPTER_CWHN_TO_WHCN || type == VX_ADAPTER_WHCN_TO_CWHN)
    {
        pPerm = (vx_uint32_ptr)perm_array;
        num = dnum;

        shExe_flag = (vx_bool)((enable_dataFormat && pPerm[0] == 2 && pPerm[1] == 0 && pPerm[2] == 1 && num == 3)
            || (enable_dataFormat && pPerm[0] == 2 && pPerm[1] == 1 && pPerm[2] == 0 && num == 3)
            || (enable_dataFormat && pPerm[0] == 1 && pPerm[1] == 2 && pPerm[2] == 0 && num == 3)
            || (enable_dataFormat && pPerm[0] == 0 && pPerm[1] == 2 && pPerm[2] == 1 && num == 3)
            || (enable_dataFormat && pPerm[0] == 1 && pPerm[1] == 0 && num <= 3 && num >= 2));

        if (shExe_flag && dnum > 1)
        {
            vxnne_shader_executable shaderExecutable = VX_NULL;

            if (evis)
            {
                if (src && dst)
                    shaderExecutable = vxnneTensorTransposeShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_TRANSPOSE, &ops_layer->node->kernelAttributes.borderMode, src, pPerm, num, dst);
            }
            else
            {
                if (src && dst)
                    shaderExecutable = vxnneGPUTensorTransposeShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_GPU_TENSOR_TRANSPOSE, &ops_layer->node->kernelAttributes.borderMode, src, pPerm, num, dst);
            }

            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto OnError;
            }
            vxmONERROR(vxnneShaderOperation_Initialize(&adapterLayer->adapter_sh_operation,
                &adapterLayer->base,
                VXNNE_OPERATOR_TENSOR_TRANS,
                batchCount1,
                shaderExecutable));

            vxmONERROR(vxnneOperation_AddReference(&adapterLayer->adapter_sh_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT));
            vxmONERROR(vxnneOperation_AddReference(&adapterLayer->adapter_sh_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT));

            vxmONERROR(vxnneLayer_SetOperation(
                &adapterLayer->base,
                &adapterLayer->adapter_sh_operation.base,
                transOPIdx));
        }
    }

    if (enable_dataConvert)
    {
        vxnne_shader_executable shaderExecutable = VX_NULL;

        if (temp_tensor[0] && temp_tensor[1])
            shaderExecutable = vxnneTensorCopyShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_COPY, &ops_layer->node->kernelAttributes.borderMode, temp_tensor[0], temp_tensor[1]);

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto OnError;
        }
        vxmONERROR(vxnneShaderOperation_Initialize(&adapterLayer->adapter_convert_sh_operation,
            &adapterLayer->base,
            VXNNE_OPERATOR_TENSOR_COPY,
            batchCount0,
            shaderExecutable));

        vxmONERROR(vxnneOperation_AddReference(&adapterLayer->adapter_convert_sh_operation.base, (vx_reference)temp_tensor[0], VXNNE_OPERATION_REFENRENCE_INPUT));
        vxmONERROR(vxnneOperation_AddReference(&adapterLayer->adapter_convert_sh_operation.base, (vx_reference)temp_tensor[1], VXNNE_OPERATION_REFENRENCE_OUTPUT));

        vxmONERROR(vxnneLayer_SetOperation(
            &adapterLayer->base,
            &adapterLayer->adapter_convert_sh_operation.base,
            convOPIdx));
    }


    if (src != VX_NULL) adapterLayer->base.temp_tensors[temp_tensor_idx++] = src;
    if (dst != VX_NULL) adapterLayer->base.temp_tensors[temp_tensor_idx++] = dst;

    adapterLayer->base.num_temp_tensors = temp_tensor_idx;

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, _num, reg_param);

    return status;
}

VX_PRIVATE_API vx_status vxoNNadapterLayer_SH_EVIS_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    status = vxoNNadapterLayer_SH_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_true_e);

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}
VX_PRIVATE_API vx_bool vxoNNadapterLayer_SH_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{

    vx_tensor  inputs = (vx_tensor)parameters[0];
    vx_tensor  outputs = (vx_tensor)parameters[2];
    vx_enum    inputFormat = TENSOR_DATA_TYPE(inputs);
    vx_enum    outputFormat = TENSOR_DATA_TYPE(outputs);
    vx_bool    enable_dataFormat = vx_false_e;

    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    enable_dataFormat = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) ||
        (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32) ||
        (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8));

    support = support && enable_dataFormat;

    if (support)
    {
        SETBIT(reg_param->flag, ((enable_dataFormat == vx_true_e) ? 1 : 0), 0);
    }

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNadapterLayer_SH_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    status = vxoNNadapterLayer_SH_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_false_e);

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNadapterLayer_TP_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{

    vx_tensor  inputs = (vx_tensor)parameters[0];
    vx_scalar  type_s = (vx_scalar)parameters[1];
    vx_tensor  outputs = (vx_tensor)parameters[2];
    vx_enum    type = type_s->value->e;

    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_TP, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    support = support && vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_TP_TRANSPOSE);

    support = support && vxnneIsTPSupportFormat(node->base.context, inputs, VX_NULL, outputs);
    support = support && (type == VX_ADAPTER_CWHN_TO_WHCN || type == VX_ADAPTER_WHCN_TO_CWHN);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNadapterLayer_TP_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vxnne_adapter_layer  adapterLayer = (vxnne_adapter_layer)ops_layer;

    vx_tensor  inputs = (vx_tensor)parameters[0];
    vx_scalar  type_s = (vx_scalar)parameters[1];
    vx_tensor  outputs = (vx_tensor)parameters[2];
    vx_tensor  src = NULL;
    vx_enum    type = type_s->value->e;
    vx_uint32  perm_array[4] = { 0, 1, 2, 3 };
    vx_uint32  temp_tensor_idx = 0;

    vx_op_param_s conv = { 0 };
    vx_uint32 dnum = 4;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    if (type == VX_ADAPTER_CWHN_TO_WHCN)
    {
        vx_uint32 sizes[4] = { TENSOR_VIEW_SIZE_INDEX(inputs, 3),
            TENSOR_VIEW_SIZE_INDEX(inputs, 2),
            TENSOR_VIEW_SIZE_INDEX(inputs, 1),
            TENSOR_VIEW_SIZE_INDEX(inputs, 0) };

        src = vxoTensor_ReshapeTensor(inputs, (vx_int32*)sizes, dnum);

        perm_array[0] = 1;
        perm_array[1] = 2;
        perm_array[2] = 0;
        perm_array[3] = 3;
    }
    else /* type == VX_ADAPTER_WHCN_TO_CWHN */
    {
        perm_array[0] = 2;
        perm_array[1] = 0;
        perm_array[2] = 1;
        perm_array[3] = 3;
    }

    vxmONERROR(vxnneOperation_Initialize(&adapterLayer->adapter_tp_operation.base,
        &adapterLayer->base,
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
    conv.other_ref = src == VX_NULL ? (vx_reference)inputs : (vx_reference)src;
    conv.data_buff = gcvNULL;
    conv.tp_value = (vx_tp_value_cmd)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
    conv.tp_value->u32[0] = dnum;
    conv.tp_value->p8[0] = (vx_uint8_ptr)vxAllocateAndZeroMemory(sizeof(vx_uint32) * dnum);
    vxMemCopy(conv.tp_value->p8[0], perm_array, sizeof(vx_uint32) * dnum);

    vxMemCopy(&adapterLayer->adapter_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));

    vxmONERROR(vxnneLayer_SetOperation(
        &adapterLayer->base,
        &adapterLayer->adapter_tp_operation.base,
        0));

    adapterLayer->adapter_tp_operation.input = inputs;
    adapterLayer->adapter_tp_operation.output = outputs;

    vxmONERROR(vxnneOperation_AddReference(&adapterLayer->adapter_tp_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&adapterLayer->adapter_tp_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    if (src != VX_NULL) adapterLayer->base.temp_tensors[temp_tensor_idx++] = src;

    adapterLayer->base.num_temp_tensors = temp_tensor_idx;
OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_status vxoNNLayer_GetAdapterOperations(vxnne_layer ops_layer, vx_uint32_ptr max_num_operations, vxnne_operation **operations)
{
    vx_status  status = VX_SUCCESS;
    vxnne_adapter_layer  adapterLayer = (vxnne_adapter_layer)ops_layer;

    *max_num_operations = gcmCOUNTOF(adapterLayer->operations);

    *operations = adapterLayer->operations;

    return status;
}
#endif

VX_PRIVATE_API vx_status VX_CALLBACK vxoAdapter_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status                      = VX_SUCCESS;
#if REGISTER_FRAME

    vxnne_layer_imp_s registeradapterLayers[] = {/* Please DON'T adjust the order, it's importent */
        { "adapterLayer NN", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "adapterLayer TP", vxoNNadapterLayer_TP_Support, vxoNNadapterLayer_TP_Initialize, vxoNNCommon_Deinitialize },
        { "adapterLayer SH EVIS", vxoNNadapterLayer_SH_EVIS_Support, vxoNNadapterLayer_SH_EVIS_Initialize, VX_NULL },
        { "adapterLayer SH F32", vxoNNadapterLayer_SH_Support, vxoNNadapterLayer_SH_Initialize, VX_NULL },
        { "adapterLayer SW support", vxoNNCommon_Support, vxoNNadapterLayer_SW_Initialize, VX_NULL },
    };

    REGISTER_LAYERS(registeradapterLayers, vxnne_adapter_layer_s, "AdapterLayer", vxoNNLayer_GetAdapterOperations);

OnError:
#else

    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_scalar  type_s                     = (vx_scalar)parameters[1];
    vx_tensor  outputs                    = (vx_tensor)parameters[2];
    vx_enum    inputFormat                = TENSOR_DATA_TYPE(inputs);
    vx_enum    outputFormat               = TENSOR_DATA_TYPE(outputs);
    vx_tensor  src                        = NULL;
    vx_tensor  dst                        = NULL;
    vx_tensor  temp_tensor[2]             = {NULL};
    vx_enum    type                       = type_s->value->e;
    vx_uint32  perm_array[4]              = {0, 1, 2, 3};
    vx_uint32  dnum                       = 0;
    vx_bool    shExe_flag                 = vx_true_e;
    vx_bool    enable_dataFormat          = vx_false_e;
    vx_bool    enable_dataConvert         = vx_false_e;
    vx_uint32  dims                       = TENSOR_VIEW_DIM_NUM(inputs);
    vx_uint32  width                      = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
    vx_uint32  height                     = dims > 1 ? TENSOR_VIEW_SIZE_INDEX(inputs, 1) : 1;
    vx_uint32  depth                      = dims > 2 ? TENSOR_VIEW_SIZE_INDEX(inputs, 2) : 1;
    vx_uint32  batch                      = dims > 3 ? TENSOR_VIEW_SIZE_INDEX(inputs, 3) : 1;
    vx_uint32  batchCount                 = 1;/*TENSOR_SIZE_INDEX(inputs, 3);*/
    vx_uint32  temp_tensor_idx            = 0;

    vxnne_adapter_layer  adapterLayer      = VX_NULL;
    vx_context           context           = vxGetContext((vx_reference)node);

    vxInfo("%s[%d] batchCount = %d", __FUNCTION__, __LINE__, batchCount);

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }
    gcoOS_Allocate(gcvNULL, sizeof(vxnne_adapter_layer_s), (gctPOINTER*)&adapterLayer);
    if (!adapterLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(adapterLayer, sizeof(vxnne_adapter_layer_s));

    vxnneLayer_Initialize(&adapterLayer->base,
                          "AdapterLayer",
                          node,
                          vxmOPERATION_COUNT(adapterLayer),
                          adapterLayer->operations,
                          VX_NULL);

    if(node->base.context->evisNoInst.supportEVIS)
    {
        enable_dataFormat  = (vx_bool)((inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
                                    || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT32)
                                    || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT16));

        if ((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT32)
         || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT16))
        {
            if (type == VX_ADAPTER_CWHN_TO_WHCN)
            {
                batch   = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
                depth   = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
                height  = TENSOR_VIEW_SIZE_INDEX(inputs, 2);
                width   = TENSOR_VIEW_SIZE_INDEX(inputs, 3);
            }
            else if (type == VX_ADAPTER_WHCN_TO_CWHN)
            {
                batch  = TENSOR_SIZE_INDEX(outputs, 0);
                depth  = TENSOR_SIZE_INDEX(outputs, 1);
                height = TENSOR_SIZE_INDEX(outputs, 2);
                width  = TENSOR_SIZE_INDEX(outputs, 3);
            }

            if (width * height * depth * batch < IMG_MAX_WIDTH)
            {
                enable_dataConvert = vx_true_e;
            }
            else if ((width * height * depth < IMG_MAX_WIDTH) && (width * height * depth % INPUT_SIZE_ALIGN_4 == 0))
            {
                enable_dataConvert = vx_true_e;
            }
            else if ((width * height < IMG_MAX_WIDTH) && (width * height % INPUT_SIZE_ALIGN_4 == 0))
            {
                enable_dataConvert = vx_true_e;
            }
            else if (width % INPUT_SIZE_ALIGN_4 == 0)
            {
                enable_dataConvert = vx_true_e;
            }
            else
            {
                enable_dataFormat = vx_false_e;
            }
        }
    }
    else
    {
        enable_dataFormat = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) ||
                                    (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32)     ||
                                    (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8));
    }

    if (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_TP_TRANSPOSE) &&
        vxnneIsTPSupportFormat(context, inputs, VX_NULL, outputs) &&
        (type == VX_ADAPTER_CWHN_TO_WHCN || type == VX_ADAPTER_WHCN_TO_CWHN))
    {
        vx_op_param_s conv = {0};
        vx_uint32 dnum = 4;

        if (type == VX_ADAPTER_CWHN_TO_WHCN)
        {
            vx_uint32 sizes[4] = {TENSOR_VIEW_SIZE_INDEX(inputs, 3),
                                  TENSOR_VIEW_SIZE_INDEX(inputs, 2),
                                  TENSOR_VIEW_SIZE_INDEX(inputs, 1),
                                  TENSOR_VIEW_SIZE_INDEX(inputs, 0)};

            src = vxoTensor_ReshapeTensor(inputs, (vx_int32*)sizes, dnum);

            perm_array[0] = 1;
            perm_array[1] = 2;
            perm_array[2] = 0;
            perm_array[3] = 3;
        }
        else /* type == VX_ADAPTER_WHCN_TO_CWHN */
        {
            perm_array[0] = 2;
            perm_array[1] = 0;
            perm_array[2] = 1;
            perm_array[3] = 3;
        }

        status = vxnneOperation_Initialize(&adapterLayer->adapter_tp_operation.base,
                                           &adapterLayer->base,
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
        conv.other_ref = src == VX_NULL ? (vx_reference)inputs : (vx_reference)src;
        conv.data_buff = gcvNULL;
        conv.tp_value = (vx_tp_value_cmd)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
        conv.tp_value->u32[0] = dnum;
        conv.tp_value->p8[0] = (vx_uint8_ptr)vxAllocateAndZeroMemory(sizeof(vx_uint32) * dnum);
        vxMemCopy(conv.tp_value->p8[0], perm_array, sizeof(vx_uint32) * dnum);

        vxMemCopy(&adapterLayer->adapter_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));

        vxnneLayer_SetOperation(
            &adapterLayer->base,
            &adapterLayer->adapter_tp_operation.base,
            0);

        adapterLayer->adapter_tp_operation.input  = inputs;
        adapterLayer->adapter_tp_operation.output = outputs;

        vxnneOperation_AddReference(&adapterLayer->adapter_tp_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&adapterLayer->adapter_tp_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }
    else if ((vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)) && (enable_dataFormat || enable_dataConvert) )
    {
        vx_uint32_ptr pPerm         = VX_NULL;
        vx_uint32     num           = 0;
        vx_int32      sizes[]       = {1, 1, 1, 1};
        vx_uint32     convOPIdx     = 0;
        vx_uint32     transOPIdx    = 0;
        vx_uint32     batchCount0   = 1;
        vx_uint32     batchCount1   = 1;

        if (enable_dataConvert)
        {
            if (width * height * depth * batch < IMG_MAX_WIDTH)
            {
                sizes[0]        = width * height * depth * batch;
                sizes[1]        = 1;

                batchCount0 = 1;
            }
            else if ((width * height * depth < IMG_MAX_WIDTH) && batch < IMG_MAX_WIDTH && (width * height * depth % INPUT_SIZE_ALIGN_4 == 0))
            {
                sizes[0]        = width * height * depth;
                sizes[1]        = batch;

                batchCount0 = 1;
            }
            else if ((width * height < IMG_MAX_WIDTH) && (depth * batch < IMG_MAX_WIDTH) && (width * height % INPUT_SIZE_ALIGN_4 == 0))
            {
                sizes[0]        = width * height;
                sizes[1]        = depth * batch;

                batchCount0 = 1;
            }
            else if ((width * height < IMG_MAX_WIDTH) && depth  < IMG_MAX_WIDTH && (width * height % INPUT_SIZE_ALIGN_4 == 0))
            {
                sizes[0]        = width * height;
                sizes[1]        = depth;
                sizes[2]        = 1;
                sizes[3]        = batch;

                batchCount0 = batch;
            }
            else
            {
                sizes[0]        = width;
                sizes[1]        = height;
                sizes[2]        = depth;
                sizes[3]        = batch;

                batchCount0 = batch;
            }

            if (type == VX_ADAPTER_F16_TO_F32 || type == VX_ADAPTER_F32_TO_F16)
            {
                temp_tensor[0] = vxoTensor_ReshapeTensor(inputs, sizes, dims);
                temp_tensor[1] = vxoTensor_ReshapeTensor(outputs, sizes, dims);
            }
            else if (type == VX_ADAPTER_CWHN_TO_WHCN)
            {
                vx_tensor_create_params_t tensor_create_params;

                gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
                tensor_create_params.num_of_dims = TENSOR_DIM_NUM(inputs);
                tensor_create_params.sizes = (vx_uint32 *)sizes;
                tensor_create_params.data_format = VX_TYPE_FLOAT16;
                tensor_create_params.quant_format = TENSOR_QUANT_TYPE(inputs);

                temp_tensor[0] = vxoTensor_ReshapeTensor(inputs, sizes, dims);
                temp_tensor[1] = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);

                convOPIdx  = 0;
                transOPIdx = 1;
            }
            else if (type == VX_ADAPTER_WHCN_TO_CWHN)
            {
                vx_tensor_create_params_t tensor_create_params;

                gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
                tensor_create_params.num_of_dims = TENSOR_DIM_NUM(inputs);
                tensor_create_params.sizes = (vx_uint32 *)sizes;
                tensor_create_params.data_format = VX_TYPE_FLOAT16;
                tensor_create_params.quant_format = TENSOR_QUANT_TYPE(inputs);

                temp_tensor[0] = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);
                temp_tensor[1] = vxoTensor_ReshapeTensor(outputs, sizes, dims);

                convOPIdx  = 1;
                transOPIdx = 0;
            }

            adapterLayer->base.temp_tensors[temp_tensor_idx ++] = temp_tensor[0];
            adapterLayer->base.temp_tensors[temp_tensor_idx ++] = temp_tensor[1];

            enable_dataConvert = vx_true_e;
        }
        else
        {
            temp_tensor[0] = outputs;
            temp_tensor[1] = inputs;
        }

        if (type == VX_ADAPTER_CWHN_TO_WHCN)
        {
            vx_uint32 input_batch   = TENSOR_VIEW_SIZE_INDEX(inputs, 0);  /* N */
            vx_uint32 input_height  = TENSOR_VIEW_SIZE_INDEX(inputs, 1); /* H */
            vx_uint32 input_width   = TENSOR_VIEW_SIZE_INDEX(inputs, 2);  /* W */
            vx_uint32 input_depth   = TENSOR_VIEW_SIZE_INDEX(inputs, 3);  /* C */
            vx_uint32 output_width  = TENSOR_SIZE_INDEX(outputs, 0);  /* W */
            vx_uint32 output_height = TENSOR_SIZE_INDEX(outputs, 1); /* H */
            vx_uint32 output_depth  = TENSOR_SIZE_INDEX(outputs, 2);  /* C */
            vx_uint32 output_batch  = TENSOR_SIZE_INDEX(outputs, 3);  /* N */
            vx_uint32 dims          = TENSOR_VIEW_DIM_NUM(inputs);

            sizes[0]                = input_depth;
            sizes[1]                = input_width;
            sizes[2]                = input_height;
            sizes[3]                = input_batch;
            src                     = vxoTensor_ReshapeTensor(temp_tensor[1], (vx_int32*)sizes, dims);

            sizes[0]                = output_width;
            sizes[1]                = output_height;
            sizes[2]                = output_depth;
            sizes[3]                = output_batch;
            dst                     = vxoTensor_ReshapeTensor(outputs, (vx_int32*)sizes, dims);

            perm_array[0]           = 1;
            perm_array[1]           = 2;
            perm_array[2]           = 0;
            perm_array[3]           = 3;

            dnum                    = 3;

            batchCount1             = dims > 3 ? output_batch : 1;
        }
        else if (type == VX_ADAPTER_WHCN_TO_CWHN)
        {
            vx_uint32 input_width   = TENSOR_SIZE_INDEX(inputs, 0);  /* W */
            vx_uint32 input_height  = TENSOR_SIZE_INDEX(inputs, 1); /* H */
            vx_uint32 input_depth   = TENSOR_SIZE_INDEX(inputs, 2);  /* C */
            vx_uint32 input_batch   = TENSOR_SIZE_INDEX(inputs, 3);  /* N */
            vx_uint32 output_batch  = TENSOR_SIZE_INDEX(outputs, 0);  /* N */
            vx_uint32 output_height = TENSOR_SIZE_INDEX(outputs, 1); /* H */
            vx_uint32 output_width  = TENSOR_SIZE_INDEX(outputs, 2);  /* W */
            vx_uint32 output_depth  = TENSOR_SIZE_INDEX(outputs, 3);  /* C */
            vx_uint32 dims          = TENSOR_VIEW_DIM_NUM(inputs);

            sizes[0]                = input_width;
            sizes[1]                = input_height;
            sizes[2]                = input_depth;
            sizes[3]                = input_batch;
            src                     = vxoTensor_ReshapeTensor(inputs, (vx_int32*)sizes, dims);

            sizes[0]                = output_depth;
            sizes[1]                = output_width;
            sizes[2]                = output_height;
            sizes[3]                = output_batch;
            dst                     = vxoTensor_ReshapeTensor(temp_tensor[0], (vx_int32*)sizes, dims);

            perm_array[0]           = 2;
            perm_array[1]           = 0;
            perm_array[2]           = 1;
            perm_array[3]           = 3;

            dnum                    = 3;

            batchCount1             = dims > 3 ? input_batch : 1;
        }

        if (type == VX_ADAPTER_CWHN_TO_WHCN || type == VX_ADAPTER_WHCN_TO_CWHN)
        {
            pPerm = (vx_uint32_ptr)perm_array;
            num   = dnum;

            shExe_flag    = (vx_bool)((enable_dataFormat && pPerm[0] == 2 && pPerm[1] == 0 && pPerm[2] == 1  && num == 3)
                ||(enable_dataFormat && pPerm[0] == 2 && pPerm[1] == 1 && pPerm[2] == 0  && num == 3)
                ||(enable_dataFormat && pPerm[0] == 1 && pPerm[1] == 2 && pPerm[2] == 0  && num == 3)
                ||(enable_dataFormat && pPerm[0] == 0 && pPerm[1] == 2 && pPerm[2] == 1  && num == 3)
                ||(enable_dataFormat && pPerm[0] == 1 && pPerm[1] == 0 && num <= 3 && num >= 2));

            if(shExe_flag && dnum > 1)
            {
                vxnne_shader_executable shaderExecutable = VX_NULL;

                if(node->base.context->evisNoInst.supportEVIS)
                {
                    if (src && dst)
                        shaderExecutable = vxnneTensorTransposeShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_TRANSPOSE, &node->kernelAttributes.borderMode, src, pPerm, num, dst);
                }
                else
                {
                    if (src && dst)
                        shaderExecutable = vxnneGPUTensorTransposeShaderExecutable(node->base.context, VXNNE_KERNEL_GPU_TENSOR_TRANSPOSE, &node->kernelAttributes.borderMode, src, pPerm, num, dst);
                }

                if (!shaderExecutable)
                {
                    status = VX_FAILURE;
                    goto exit;
                }
                status = vxnneShaderOperation_Initialize(&adapterLayer->adapter_sh_operation,
                    &adapterLayer->base,
                    VXNNE_OPERATOR_TENSOR_TRANS,
                    batchCount1,
                    shaderExecutable);

                if (status != VX_SUCCESS)
                    goto exit;

                vxnneOperation_AddReference(&adapterLayer->adapter_sh_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&adapterLayer->adapter_sh_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                vxnneLayer_SetOperation(
                    &adapterLayer->base,
                    &adapterLayer->adapter_sh_operation.base,
                    transOPIdx);
            }
        }

        if (enable_dataConvert)
        {
            vxnne_shader_executable shaderExecutable = VX_NULL;

            if (temp_tensor[0] && temp_tensor[1])
                shaderExecutable = vxnneTensorCopyShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_COPY, &node->kernelAttributes.borderMode, temp_tensor[0], temp_tensor[1]);

            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto exit;
            }
            status = vxnneShaderOperation_Initialize(&adapterLayer->adapter_convert_sh_operation,
                &adapterLayer->base,
                VXNNE_OPERATOR_TENSOR_COPY,
                batchCount0,
                shaderExecutable);

            if (status != VX_SUCCESS)
                goto exit;

            vxnneOperation_AddReference(&adapterLayer->adapter_convert_sh_operation.base, (vx_reference)temp_tensor[0], VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&adapterLayer->adapter_convert_sh_operation.base, (vx_reference)temp_tensor[1], VXNNE_OPERATION_REFENRENCE_OUTPUT);

            vxnneLayer_SetOperation(
                &adapterLayer->base,
                &adapterLayer->adapter_convert_sh_operation.base,
                convOPIdx);
        }
    }
    else
    {
        vxnneOperation_Initialize(&adapterLayer->adapter_sw_operation.base,
            &adapterLayer->base,
            VXNNE_OPERATION_TARGET_SW,
            VXNNE_OPERATOR_ADAPTER,
            vxnneExecuteSWAdapter,
            VX_NULL,
            batchCount,
            0);

        vxnneLayer_SetOperation(
            &adapterLayer->base,
            &adapterLayer->adapter_sw_operation.base,
            0);

        adapterLayer->adapter_sw_operation.inputs           = inputs;
        adapterLayer->adapter_sw_operation.type             = type_s;
        adapterLayer->adapter_sw_operation.outputs          = outputs;

        vxnneOperation_AddReference(&adapterLayer->adapter_sw_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&adapterLayer->adapter_sw_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }

    if (src != VX_NULL) adapterLayer->base.temp_tensors[temp_tensor_idx ++] = src;
    if (dst != VX_NULL) adapterLayer->base.temp_tensors[temp_tensor_idx ++] = dst;

    adapterLayer->base.num_temp_tensors = temp_tensor_idx;

    node->layer = &adapterLayer->base;

    return status;

exit:
    if (adapterLayer) {
        gcoOS_Free(NULL, (gctPOINTER)adapterLayer);
    }
#endif
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoAdapter_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = NULL;
    }

    return VX_SUCCESS;
}

