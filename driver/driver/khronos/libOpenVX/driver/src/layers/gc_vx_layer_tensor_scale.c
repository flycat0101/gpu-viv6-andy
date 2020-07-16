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
#include <layers/gc_vx_layer_tensor_scale.h>

/***************************************************************************************************************************
 *                                                 Tensor Scale
 ***************************************************************************************************************************/
static vx_status _ExecuteSWScale(
    vx_enum   type,
    vx_uint8_ptr input_ptr,
    vx_uint8_ptr output_ptr,
    vx_uint32 input_width,
    vx_uint32 input_height,
    vx_uint32 input_depth,
    vx_uint32 output_width,
    vx_uint32 output_height,
    vx_uint32 output_depth,
    vx_uint32 output_batch,
    vx_uint32 input_width_orig,
    vx_uint32 output_width_orig,
    vx_type_e input_format,
    vx_type_e output_format,
    vx_int8 in_fixpoint,
    vx_int8 out_fixpoint,
    vx_uint32 in_tf_format,
    vx_uint32 out_tf_format,
    vx_uint32 in_tf_zp,
    vx_uint32 out_tf_zp,
    vx_float32 in_tf_scale,
    vx_float32 out_tf_scale,
    vx_enum out_rounding_mode
    )
{
    vx_status status = VX_SUCCESS;
    vx_float32 width_scale = (input_width * 1.0f) / output_width;
    vx_float32 height_scale = (input_height * 1.0f) / output_height;

    vx_uint32 b = 0, d = 0, w = 0, h = 0;

    vx_float32 data00 = .0f, data01 = .0f, data10 = .0f, data11 = .0f, interpolation = .0f;

    if (type == VX_INTERPOLATION_BILINEAR)
    {
        for (b = 0; b < output_batch; b ++)
        {
            for (d = 0; d < output_depth; d ++)
            {
                vx_int32 input_base = b * input_depth * input_width_orig * input_height + d * input_width_orig * input_height;
                vx_int32 output_base = b * output_depth * output_width_orig * output_height + d * output_width_orig * output_height;

                for (h = 0; h < output_height; h ++)
                {
                    vx_float32 input_h = h * height_scale;
                    vx_uint32 h0 = (vx_int32)input_h;
                    vx_uint32 h1 = gcmMIN(h0 + 1, input_height - 1);

                    for (w = 0; w < output_width; w ++)
                    {
                        vx_float32 input_w = w * width_scale;
                        vx_int32 w0 = (vx_int32)input_w;
                        vx_int32 w1 = gcmMIN(w0 + 1, (vx_int32)(input_width - 1));

                        data00 = vxnneGetDataExt((vx_type_e)input_format, in_tf_format, input_base + h0 * input_width_orig + w0, input_ptr, in_fixpoint, in_tf_zp, in_tf_scale);
                        data01 = vxnneGetDataExt((vx_type_e)input_format, in_tf_format, input_base + h0 * input_width_orig + w1, input_ptr, in_fixpoint, in_tf_zp, in_tf_scale);
                        data10 = vxnneGetDataExt((vx_type_e)input_format, in_tf_format, input_base + h1 * input_width_orig + w0, input_ptr, in_fixpoint, in_tf_zp, in_tf_scale);
                        data11 = vxnneGetDataExt((vx_type_e)input_format, in_tf_format, input_base + h1 * input_width_orig + w1, input_ptr, in_fixpoint, in_tf_zp, in_tf_scale);

                        interpolation = data00 * (1 - (input_h - h0)) * (1 - (input_w - w0)) +
                                        data10 * (input_h - h0) * (1 - (input_w - w0)) +
                                        data01 * (1 - (input_h - h0)) * (input_w - w0) +
                                        data11 * (input_h - h0) * (input_w - w0);

                        status |= vxnneSaveDataExt((vx_type_e)output_format, out_tf_format, output_base + h * output_width_orig + w, interpolation, output_ptr, out_fixpoint, out_tf_zp, out_tf_scale, out_rounding_mode);
                    }
                }
            }
        }
    }
    else if (type == VX_INTERPOLATION_NEAREST_NEIGHBOR)
    {
        for (d = 0; d < output_depth; d ++)
        {
            for (h = 0; h < output_height; h ++)
            {
                vx_uint32 in_y = gcmMIN((vx_uint32)floorf(h * height_scale), input_height - 1);

                for (w = 0; w < output_width; w ++)
                {
                    vx_uint32   in_x        = gcmMIN((vx_uint32)floorf(w * width_scale), input_width - 1);
                    vx_int32    in_index    = in_x + in_y * input_width_orig + d * input_width_orig * input_height;
                    vx_int32    out_index   = w + h * output_width_orig + d * output_width_orig * output_height;
                    vx_float32  data;

                    data = vxnneGetDataExt((vx_type_e)input_format, in_tf_format, in_index, input_ptr, in_fixpoint, in_tf_zp, in_tf_scale);

                    status |= vxnneSaveDataExt((vx_type_e)output_format, out_tf_format, out_index, data, output_ptr, out_fixpoint, out_tf_zp, out_tf_scale, out_rounding_mode);
                }
            }
        }
    }

    return status;
}

vx_status vxnneExecuteSWTensorScale(struct _vxnne_operation_s *operation)
{
    vxnne_tensor_scale_operation scaleOperation = (vxnne_tensor_scale_operation)operation;

    vx_tensor input  = scaleOperation->inputs;
    vx_tensor output = scaleOperation->outputs;
    vx_scalar types  = scaleOperation->type;

    vx_type_e input_format  = (vx_type_e)TENSOR_DATA_TYPE(input);
    vx_type_e output_format = (vx_type_e)TENSOR_DATA_TYPE(output);

    vx_uint8_ptr input_ptr;
    vx_uint8_ptr output_ptr;

    vx_enum in_tf_format    = TENSOR_QUANT_TYPE(input);
    vx_enum out_tf_format   = TENSOR_QUANT_TYPE(output);

    vx_uint32 in_tf_zp      = TENSOR_TF_ZEROPOINT(input);
    vx_uint32 out_tf_zp     = TENSOR_TF_ZEROPOINT(output);

    vx_float32 in_tf_scale  = TENSOR_TF_SCALE(input);
    vx_float32 out_tf_scale = TENSOR_TF_SCALE(output);

    vx_int8 in_fixpoint    = TENSOR_POS(input);
    vx_int8 out_fixpoint   = TENSOR_POS(output);

    vx_enum out_rounding_mode = TENSOR_ROUNDING_MODE(output);

    vx_enum type = types->value->e;

    vx_uint32 input_width = TENSOR_SIZE_INDEX(input, 0);  /* W */
    vx_uint32 input_height = TENSOR_SIZE_INDEX(input, 1); /* H */
    vx_uint32 input_depth = TENSOR_SIZE_INDEX(input, 2);  /* C */
    /*vx_uint32 input_batch = TENSOR_SIZE_INDEX(input, 3);   N */

    vx_uint32 output_width = TENSOR_SIZE_INDEX(output, 0);  /* W */
    vx_uint32 output_height = TENSOR_SIZE_INDEX(output, 1); /* H */
    vx_uint32 output_depth = TENSOR_SIZE_INDEX(output, 2);  /* C */
    vx_uint32 output_batch = TENSOR_SIZE_INDEX(output, 3);  /* N */

    vxoTensor_GetTensorViewMemory(input, (gctPOINTER*)&input_ptr, VX_NULL);
    vxoTensor_GetTensorViewMemory(output, (gctPOINTER*)&output_ptr, VX_NULL);

    return _ExecuteSWScale(
            type,
            input_ptr,
            output_ptr,
            input_width,
            input_height,
            input_depth,
            output_width,
            output_height,
            output_depth,
            output_batch,
            input_width,
            output_width,
            input_format,
            output_format,
            in_fixpoint,
            out_fixpoint,
            in_tf_format,
            out_tf_format,
            in_tf_zp,
            out_tf_zp,
            in_tf_scale,
            out_tf_scale,
            out_rounding_mode);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorScale(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoTensorScale_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxoTensorScale_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}
#if REGISTER_FRAME
VX_PRIVATE_API vx_status vxoTensorScale_SW_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vxnne_tensor_scale_layer  tensor_scaleLayer = (vxnne_tensor_scale_layer)ops_layer;
    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_scalar  type_s                     = (vx_scalar)parameters[1];
    vx_tensor  outputs                    = (vx_tensor)parameters[2];
    vx_uint32  batchCount                 = TENSOR_SIZE_INDEX(inputs, 3);

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);
    vxmONERROR(vxnneOperation_Initialize(&tensor_scaleLayer->tensor_scale_sw_operation.base,
        &tensor_scaleLayer->base,
        VXNNE_OPERATION_TARGET_SW,
        VXNNE_OPERATOR_TENSOR_SCALE,
        vxnneExecuteSWTensorScale,
        VX_NULL,
        batchCount,
        0));

    vxmONERROR(vxnneLayer_SetOperation(
        &tensor_scaleLayer->base,
        &tensor_scaleLayer->tensor_scale_sw_operation.base,
        0));

    tensor_scaleLayer->tensor_scale_sw_operation.inputs           = inputs;
    tensor_scaleLayer->tensor_scale_sw_operation.type             = type_s;
    tensor_scaleLayer->tensor_scale_sw_operation.outputs          = outputs;

    vxmONERROR(vxnneOperation_AddReference(&tensor_scaleLayer->tensor_scale_sw_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&tensor_scaleLayer->tensor_scale_sw_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoTensorScale_SH_EVIS_Support_Ext(vx_node node, const vx_reference parameters[], vx_uint32 _num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_scalar  type_s                     = (vx_scalar)parameters[1];
    vx_tensor  outputs                    = (vx_tensor)parameters[2];
    vx_bool    useShadeExe                = vx_false_e;
    vx_bool    enable_format              = vx_false_e;
    vx_bool    enable_nearest_format      = vx_false_e;
    vx_bool    enable_tmp_format          = vx_false_e;
    vx_bool    enable_nearest_neighbor    = vx_false_e;
    vx_bool    enable_nearest_scaleVal    = vx_false_e;
    vx_enum    srcFormat                  = TENSOR_DATA_TYPE(inputs);
    vx_enum    dstFormat                  = TENSOR_DATA_TYPE(outputs);
    vx_enum    type                       = type_s->value->e;
    vx_uint32  in_width                   = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
    vx_uint32  in_height                  = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
    vx_uint32  out_width                  = TENSOR_VIEW_SIZE_INDEX(outputs, 0);
    vx_uint32  out_height                 = TENSOR_VIEW_SIZE_INDEX(outputs, 1);
    vx_float32 width_scale                = (vx_float32)in_width / out_width;
    vx_float32 height_scale               = (vx_float32)in_height / out_height;
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, _num, reg_param);

    if(evis)
    {
        enable_format           = (vx_bool)((srcFormat == VX_TYPE_FLOAT16 && dstFormat == VX_TYPE_FLOAT16)
                                         ||(srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_FLOAT16)
                                         ||(srcFormat == VX_TYPE_FLOAT16 && dstFormat == VX_TYPE_UINT8)
                                         ||(srcFormat == VX_TYPE_INT8 && dstFormat == VX_TYPE_INT8)
                                         ||(srcFormat == VX_TYPE_INT16 && dstFormat == VX_TYPE_INT16)
                                         ||(srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_UINT8)
                                         ||(srcFormat == VX_TYPE_BFLOAT16 && dstFormat == VX_TYPE_BFLOAT16));
        enable_nearest_format   = (vx_bool)(!checkOutputTensorDoAlu(inputs, outputs)
                                        || (srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_UINT8)
                                        || (srcFormat == VX_TYPE_FLOAT16 && dstFormat == VX_TYPE_UINT8)
                                        || (srcFormat == VX_TYPE_FLOAT16 && dstFormat == VX_TYPE_INT8));

        enable_tmp_format       = (vx_bool)((srcFormat == VX_TYPE_FLOAT16 && dstFormat == VX_TYPE_FLOAT16)
                                         || (srcFormat == VX_TYPE_INT16 && dstFormat == VX_TYPE_INT16)
                                         || (srcFormat == VX_TYPE_INT8 && dstFormat == VX_TYPE_INT8)
                                         || (srcFormat == VX_TYPE_BFLOAT16 && dstFormat == VX_TYPE_BFLOAT16)
                                         || (srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_UINT8));
    }
    else
    {
        enable_format           = (vx_bool)((srcFormat == VX_TYPE_FLOAT16 && dstFormat == VX_TYPE_FLOAT16)
                                         ||(srcFormat == VX_TYPE_FLOAT32 && dstFormat == VX_TYPE_FLOAT32)
                                         ||(srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_FLOAT16)
                                         ||(srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_FLOAT32)
                                         ||(srcFormat == VX_TYPE_FLOAT16 && dstFormat == VX_TYPE_UINT8)
                                         ||(srcFormat == VX_TYPE_FLOAT32 && dstFormat == VX_TYPE_UINT8)
                                         ||(srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_UINT8));
        enable_nearest_format   = (vx_bool)(!checkOutputTensorDoAlu(inputs, outputs)
                                        || (srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_UINT8)
                                        || (srcFormat == VX_TYPE_FLOAT16 && dstFormat == VX_TYPE_UINT8)
                                        || (srcFormat == VX_TYPE_FLOAT32 && dstFormat == VX_TYPE_UINT8));

        enable_tmp_format       = (vx_bool)((srcFormat == VX_TYPE_FLOAT16 && dstFormat == VX_TYPE_FLOAT16)
                                         || (srcFormat == VX_TYPE_FLOAT32 && dstFormat == VX_TYPE_FLOAT32)
                                         || (srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_UINT8));
    }

    enable_nearest_scaleVal = (vx_bool) ((width_scale == 2.0f && height_scale == 2.0f && in_width * in_height < IMG_MAX_WIDTH) || (width_scale == 0.5f && height_scale == 0.5f));
    enable_nearest_neighbor = (vx_bool) (((enable_nearest_format  && enable_nearest_scaleVal) || enable_tmp_format) && type == VX_INTERPOLATION_NEAREST_NEIGHBOR);

    useShadeExe     =  (vx_bool)((enable_format && type == VX_INTERPOLATION_BILINEAR) || enable_nearest_neighbor);

    support = useShadeExe && support;

    vxoLayer_VerificationFoot(node, parameters, _num, reg_param, &support);
    return support;
}

VX_PRIVATE_API vx_status vxoTensorScale_SH_Initialize_Ext(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 _num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_status status = VX_SUCCESS;
    vxnne_tensor_scale_layer  tensor_scaleLayer = (vxnne_tensor_scale_layer)ops_layer;
    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_scalar  type_s                     = (vx_scalar)parameters[1];
    vx_tensor  outputs                    = (vx_tensor)parameters[2];
    vx_enum    type                       = type_s->value->e;

    vx_uint32  batchCount                 = TENSOR_SIZE_INDEX(inputs, 3);
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxoLayer_InitializeHead(ops_layer, parameters, _num, reg_param);

    if (type == VX_INTERPOLATION_BILINEAR)
    {
        if(evis)
        {
            shaderExecutable = vxnneGetTensorScaleShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_SCALE, &ops_layer->node->kernelAttributes.borderMode, inputs, type, outputs);
        }
        else
        {
            shaderExecutable = vxnneGetGPUTensorScaleShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_GPU_TENSOR_SCALE, &ops_layer->node->kernelAttributes.borderMode, inputs, type, outputs);
        }
    }
    else
    {
        if(evis)
        {
            shaderExecutable = vxnneGetResizeNearestNeighborShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_RESIZE_NEAREST_NEIGHBOR, &ops_layer->node->kernelAttributes.borderMode, inputs, type, outputs);
        }
        else
        {
            shaderExecutable = vxnneGetGPUResizeNearestNeighborShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_GPU_RESIZE_NEAREST_NEIGHBOR, &ops_layer->node->kernelAttributes.borderMode, inputs, type, outputs);
        }

    }

    if (!shaderExecutable)
    {
        status = VX_FAILURE;
        goto OnError;
    }

    vxmONERROR(vxnneShaderOperation_Initialize(&tensor_scaleLayer->tensor_scale_sh_operation,
        &tensor_scaleLayer->base,
        VXNNE_OPERATOR_TENSOR_SCALE,
        batchCount,
        shaderExecutable));

    vxmONERROR(vxnneOperation_AddReference(&tensor_scaleLayer->tensor_scale_sh_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&tensor_scaleLayer->tensor_scale_sh_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    vxmONERROR(vxnneLayer_SetOperation(
        &tensor_scaleLayer->base,
        &tensor_scaleLayer->tensor_scale_sh_operation.base,
        0));
OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, _num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoTensorScale_SH_EVIS_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && node->base.context->evisNoInst.supportEVIS;

    if (!support)return support;

    support = support && vxoTensorScale_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_true_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoTensorScale_SH_EVIS_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    status = vxoTensorScale_SH_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_true_e);

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoTensorScale_SH_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    support = support && vxoTensorScale_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_false_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoTensorScale_SH_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    status = vxoTensorScale_SH_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_false_e);

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_status vxoNNLayer_GetOperations(vxnne_layer ops_layer, vx_uint32_ptr max_num_operations, vxnne_operation **operations)
{
    vx_status  status = VX_SUCCESS;

    vxnne_tensor_scale_layer  tensor_scaleLayer = (vxnne_tensor_scale_layer)ops_layer;

    *max_num_operations = gcmCOUNTOF(tensor_scaleLayer->operations);

    *operations = tensor_scaleLayer->operations;

    return status;
}

#endif

VX_PRIVATE_API vx_status VX_CALLBACK vxoTensorScale_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{

    vx_status status = VX_SUCCESS;
#if REGISTER_FRAME
    vxnne_layer_imp_s registerTensorScale[] = {/* Please DON'T adjust the order, it's importent */
        { "TensorScale NN", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "TensorScale TP", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "TensorScale SH EVIS", vxoTensorScale_SH_EVIS_Support, vxoTensorScale_SH_EVIS_Initialize, VX_NULL },
        { "TensorScale SH F32", vxoTensorScale_SH_Support, vxoTensorScale_SH_Initialize, VX_NULL },
        { "TensorScale SW ", vxoNNCommon_Support, vxoTensorScale_SW_Initialize, VX_NULL },
    };

    REGISTER_LAYERS(registerTensorScale, vxnne_tensor_scale_layer_s, "TensorScale", vxoNNLayer_GetOperations);

OnError:
#else
    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_scalar  type_s                     = (vx_scalar)parameters[1];
    vx_tensor  outputs                    = (vx_tensor)parameters[2];
    vx_bool    useShadeExe                = vx_false_e;
    vx_bool    enable_format              = vx_false_e;
    vx_bool    enable_nearest_format      = vx_false_e;
    vx_bool    enable_tmp_format          = vx_false_e;
    vx_bool    enable_nearest_neighbor    = vx_false_e;
    vx_bool    enable_nearest_scaleVal    = vx_false_e;
    vx_enum    srcFormat                  = TENSOR_DATA_TYPE(inputs);
    vx_enum    dstFormat                  = TENSOR_DATA_TYPE(outputs);
    vx_enum    type                       = type_s->value->e;
    vx_uint32  in_width                   = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
    vx_uint32  in_height                  = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
    vx_uint32  out_width                  = TENSOR_VIEW_SIZE_INDEX(outputs, 0);
    vx_uint32  out_height                 = TENSOR_VIEW_SIZE_INDEX(outputs, 1);
    vx_float32 width_scale                = (vx_float32)in_width / out_width;
    vx_float32 height_scale               = (vx_float32)in_height / out_height;

    vx_uint32  batchCount                 = TENSOR_SIZE_INDEX(inputs, 3);

    vxnne_tensor_scale_layer  tensor_scaleLayer     = VX_NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }
    gcoOS_Allocate(gcvNULL, sizeof(vxnne_tensor_scale_layer_s), (gctPOINTER*)&tensor_scaleLayer);
    if (!tensor_scaleLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("Out of Memory at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(tensor_scaleLayer, sizeof(vxnne_tensor_scale_layer_s));

    vxnneLayer_Initialize(&tensor_scaleLayer->base,
                          "TensorScale",
                          node,
                          vxmOPERATION_COUNT(tensor_scaleLayer),
                          tensor_scaleLayer->operations,
                          VX_NULL);

    if(node->base.context->evisNoInst.supportEVIS)
    {
        enable_format           = (vx_bool)((srcFormat == VX_TYPE_FLOAT16 && dstFormat == VX_TYPE_FLOAT16)
                                         ||(srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_FLOAT16)
                                         ||(srcFormat == VX_TYPE_FLOAT16 && dstFormat == VX_TYPE_UINT8)
                                         ||(srcFormat == VX_TYPE_INT8 && dstFormat == VX_TYPE_INT8)
                                         ||(srcFormat == VX_TYPE_INT16 && dstFormat == VX_TYPE_INT16)
                                         ||(srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_UINT8)
                                         ||(srcFormat == VX_TYPE_BFLOAT16 && dstFormat == VX_TYPE_BFLOAT16));
        enable_nearest_format   = (vx_bool)(!checkOutputTensorDoAlu(inputs, outputs)
                                        || (srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_UINT8)
                                        || (srcFormat == VX_TYPE_FLOAT16 && dstFormat == VX_TYPE_UINT8)
                                        || (srcFormat == VX_TYPE_FLOAT16 && dstFormat == VX_TYPE_INT8));

        enable_tmp_format       = (vx_bool)((srcFormat == VX_TYPE_FLOAT16 && dstFormat == VX_TYPE_FLOAT16)
                                         || (srcFormat == VX_TYPE_INT16 && dstFormat == VX_TYPE_INT16)
                                         || (srcFormat == VX_TYPE_INT8 && dstFormat == VX_TYPE_INT8)
                                         || (srcFormat == VX_TYPE_BFLOAT16 && dstFormat == VX_TYPE_BFLOAT16)
                                         || (srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_UINT8));
    }
    else
    {
        enable_format           = (vx_bool)((srcFormat == VX_TYPE_FLOAT16 && dstFormat == VX_TYPE_FLOAT16)
                                         ||(srcFormat == VX_TYPE_FLOAT32 && dstFormat == VX_TYPE_FLOAT32)
                                         ||(srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_FLOAT16)
                                         ||(srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_FLOAT32)
                                         ||(srcFormat == VX_TYPE_FLOAT16 && dstFormat == VX_TYPE_UINT8)
                                         ||(srcFormat == VX_TYPE_FLOAT32 && dstFormat == VX_TYPE_UINT8)
                                         ||(srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_UINT8));
        enable_nearest_format   = (vx_bool)(!checkOutputTensorDoAlu(inputs, outputs)
                                        || (srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_UINT8)
                                        || (srcFormat == VX_TYPE_FLOAT16 && dstFormat == VX_TYPE_UINT8)
                                        || (srcFormat == VX_TYPE_FLOAT32 && dstFormat == VX_TYPE_UINT8));

        enable_tmp_format       = (vx_bool)((srcFormat == VX_TYPE_FLOAT16 && dstFormat == VX_TYPE_FLOAT16)
                                         || (srcFormat == VX_TYPE_FLOAT32 && dstFormat == VX_TYPE_FLOAT32)
                                         || (srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_UINT8));
    }

    enable_nearest_scaleVal = (vx_bool) ((width_scale == 2.0f && height_scale == 2.0f && in_width * in_height < IMG_MAX_WIDTH) || (width_scale == 0.5f && height_scale == 0.5f));
    enable_nearest_neighbor = (vx_bool) (((enable_nearest_format  && enable_nearest_scaleVal) || enable_tmp_format) && type == VX_INTERPOLATION_NEAREST_NEIGHBOR);

    useShadeExe     =  (vx_bool)((enable_format && type == VX_INTERPOLATION_BILINEAR) || enable_nearest_neighbor);

    if(useShadeExe && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
    {
        vxnne_shader_executable shaderExecutable = VX_NULL;

        if (type == VX_INTERPOLATION_BILINEAR)
        {
            if(node->base.context->evisNoInst.supportEVIS)
            {
                shaderExecutable = vxnneGetTensorScaleShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_SCALE, &node->kernelAttributes.borderMode, inputs, type, outputs);
            }
            else
            {
                shaderExecutable = vxnneGetGPUTensorScaleShaderExecutable(node->base.context, VXNNE_KERNEL_GPU_TENSOR_SCALE, &node->kernelAttributes.borderMode, inputs, type, outputs);
            }
        }
        else
        {
            if(node->base.context->evisNoInst.supportEVIS)
            {
                shaderExecutable = vxnneGetResizeNearestNeighborShaderExecutable(node->base.context, VXNNE_KERNEL_RESIZE_NEAREST_NEIGHBOR, &node->kernelAttributes.borderMode, inputs, type, outputs);
            }
            else
            {
                shaderExecutable = vxnneGetGPUResizeNearestNeighborShaderExecutable(node->base.context, VXNNE_KERNEL_GPU_RESIZE_NEAREST_NEIGHBOR, &node->kernelAttributes.borderMode, inputs, type, outputs);
            }

        }

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&tensor_scaleLayer->tensor_scale_sh_operation,
            &tensor_scaleLayer->base,
            VXNNE_OPERATOR_TENSOR_SCALE,
            batchCount,
            shaderExecutable);

        if (status != VX_SUCCESS) {
            goto exit;
        }

        vxnneOperation_AddReference(&tensor_scaleLayer->tensor_scale_sh_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&tensor_scaleLayer->tensor_scale_sh_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        vxnneLayer_SetOperation(
            &tensor_scaleLayer->base,
            &tensor_scaleLayer->tensor_scale_sh_operation.base,
            0);
    }
    else
    {
        vxnneOperation_Initialize(&tensor_scaleLayer->tensor_scale_sw_operation.base,
            &tensor_scaleLayer->base,
            VXNNE_OPERATION_TARGET_SW,
            VXNNE_OPERATOR_TENSOR_SCALE,
            vxnneExecuteSWTensorScale,
            VX_NULL,
            batchCount,
            0);

        vxnneLayer_SetOperation(
            &tensor_scaleLayer->base,
            &tensor_scaleLayer->tensor_scale_sw_operation.base,
            0);

        tensor_scaleLayer->tensor_scale_sw_operation.inputs           = inputs;
        tensor_scaleLayer->tensor_scale_sw_operation.type             = type_s;
        tensor_scaleLayer->tensor_scale_sw_operation.outputs          = outputs;

        vxnneOperation_AddReference(&tensor_scaleLayer->tensor_scale_sw_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&tensor_scaleLayer->tensor_scale_sw_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }

    node->layer = &tensor_scaleLayer->base;


    return status;
exit:
    if (tensor_scaleLayer) gcoOS_Free(gcvNULL, (gctPOINTER)tensor_scaleLayer);
#endif
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoTensorScale_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

/***************************************************************************************************************************
 *                                                 Tensor YUV2RGB Scale
 ***************************************************************************************************************************/
vx_status vxnneComputeYUV2RGBInputParameter(
    vx_uint32 outputSize,
    vx_uint32 scale,
    vx_uint32 inputStart,
    vx_uint32 * splitNum,
    vx_uint32 * outputStarts,
    vx_uint32 * outputSizes,
    vx_uint32 * inputStarts,
    vx_uint32 * inputSizes,
    vx_uint16 * inputInitErrors,
    vx_uint16 * inputInitIntErrors
    )
{
    vx_uint32 num, i, offset, inputSize;

    inputSize = (vx_uint32)((vx_float32)(outputSize * scale) / (1 << 15) + 0.5f);
    offset = gcmMAX(0, ((vx_int32)scale >> 1) - (1 << 14));

    num = gcmMIN(gcmMIN(inputSize, outputSize), *splitNum);

    calculateSplitSize(outputSize, num, outputSizes, outputStarts);

    for (i = 0; i < num; i++)
    {
        inputStarts[i] = (vx_uint16)inputStart + (vx_uint16)((offset & 0xFFFF8000) >> 15);
        if (inputStarts[i] & 0x1)
        {
            inputStarts[i]--;
            inputInitIntErrors[i] = 0x1;
        }
        else
        {
            inputInitIntErrors[i] = 0x0;
        }
        if (i > 0)
        {
            inputSizes[i-1] = gcmMAX(1, inputStarts[i] - inputStarts[i-1]);
        }
        inputInitErrors[i] = (vx_uint16)(offset & 0x7FFF);
        offset += scale * outputSizes[i];
    }

    inputSizes[i-1] = inputStart + inputSize - inputStarts[i-1];

    *splitNum = num;

    return VX_SUCCESS;
}

vx_status vxnneExecuteSWYUV2RGBScale(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_yuv2rgb_scale_operation scaleOperation = (vxnne_yuv2rgb_scale_operation)operation;

    vx_image image = scaleOperation->inputs;
    vx_tensor output = scaleOperation->outputs;
    vx_float32 r_mean = scaleOperation->r_mean->value->f32;
    vx_float32 g_mean = scaleOperation->g_mean->value->f32;
    vx_float32 b_mean = scaleOperation->b_mean->value->f32;
    vx_float32 rgb_scale = scaleOperation->rgb_scale->value->f32;
    vx_bool y_only = scaleOperation->y_only->value->b;
    vx_bool output_rgb = scaleOperation->output_rgb->value->b;

    vx_rectangle_t rect = scaleOperation->rect;

    vx_type_e output_format = (vx_type_e)TENSOR_DATA_TYPE(output);
    vx_type_e input_format = VX_TYPE_UINT8;

    vx_uint32 input_widths[3], input_heights[3];
    vx_uint32 output_width  = TENSOR_SIZE_INDEX(output, 0);  /* W */
    vx_uint32 output_height = TENSOR_SIZE_INDEX(output, 1);  /* H */

    vx_uint8_ptr input_bases[3], output_base;

    vx_enum out_tf_format     = TENSOR_QUANT_TYPE(output);
    vx_uint32 out_tf_zp       = TENSOR_TF_ZEROPOINT(output);
    vx_float32 out_tf_scale   = TENSOR_TF_SCALE(output);
    vx_int8 out_fixpoint      = TENSOR_POS(output);
    vx_enum out_rounding_mode = TENSOR_ROUNDING_MODE(output);

    vx_float32 width_scale, height_scale;

    vx_uint32 i;

    gcmASSERT(image->format == VX_DF_IMAGE_IYUV);
    gcmASSERT(TENSOR_SIZE_INDEX(output, 2) == 3);

    input_widths[0] = rect.end_x - rect.start_x;
    input_widths[1] = input_widths[0] / image->scales[1][VX_DIM_X];
    input_widths[2] = input_widths[0] / image->scales[2][VX_DIM_X];

    input_heights[0] = rect.end_y - rect.start_y;
    input_heights[1] = input_heights[0] / image->scales[1][VX_DIM_Y];
    input_heights[2] = input_heights[0] / image->scales[2][VX_DIM_Y];

    for (i = 0; i < 3; i++)
    {
        vx_uint32 offset = vxComputePlaneOffset(image, rect.start_x, rect.start_y, i);
        input_bases[i] = image->memory.logicals[i] + offset;
    }

    width_scale = (input_widths[0] * 1.0f) / output_width;
    height_scale = (input_heights[0] * 1.0f) / output_height;

    vxoTensor_GetTensorViewMemory(output, (vx_ptr_ptr)&output_base, VX_NULL);

    {
        vx_uint32 h, w;
        vx_float32 y, u, v, r = .0f, g = .0f, b = .0f;
        vx_float32 data00 = .0f, data01 = .0f, data10 = .0f, data11 = .0f;

        vx_int32 yy, uu, vv;
        vx_int32 post_shift = 8;
        const vx_int32 CST_CY  =  298;
        const vx_int32 CST_CUB =  517;
        const vx_int32 CST_CUG = -100;
        const vx_int32 CST_CVG = -208;
        const vx_int32 CST_CVR =  409;
        vx_int32 c0 =        (vx_int32)(0.5f                              + CST_CY  * rgb_scale);
        vx_int32 c1 =        (vx_int32)(0.5f                              + CST_CVR * rgb_scale);
        vx_int32 c2 = (-1) * (vx_int32)(0.5f                              + CST_CVG * rgb_scale);
        vx_int32 c3 = (-1) * (vx_int32)(0.5f                              + CST_CUG * rgb_scale);
        vx_int32 c4 =        (vx_int32)(0.5f                              + CST_CUB * rgb_scale);
        vx_int32 c5 =        (vx_int32)(0.5f - (56992 + r_mean * (1 << post_shift)) * rgb_scale + (1 << (post_shift - 1)));
        vx_int32 c6 =        (vx_int32)(0.5f + (34784 - g_mean * (1 << post_shift)) * rgb_scale + (1 << (post_shift - 1)));
        vx_int32 c7 =        (vx_int32)(0.5f - (70816 + b_mean * (1 << post_shift)) * rgb_scale + (1 << (post_shift - 1)));

        for (h = 0; h < output_height; h++)
        {
            vx_float32 y_input_h = h * height_scale;
            vx_uint32 yh0 = (vx_int32)y_input_h;
            vx_uint32 yh1 = gcmMIN(yh0 + 1, input_heights[0] - 1);

            for (w = 0; w < output_width; w++)
            {
                vx_float32 y_input_w = w * width_scale;
                vx_int32 yw0 = (vx_int32)y_input_w;
                vx_int32 yw1 = gcmMIN(yw0 + 1, (vx_int32)(input_widths[0] - 1));

                data00 = vxnneGetData((vx_type_e)input_format, yh0 * input_widths[0] + yw0, input_bases[0], 0);
                data01 = vxnneGetData((vx_type_e)input_format, yh0 * input_widths[0] + yw1, input_bases[0], 0);
                data10 = vxnneGetData((vx_type_e)input_format, yh1 * input_widths[0] + yw0, input_bases[0], 0);
                data11 = vxnneGetData((vx_type_e)input_format, yh1 * input_widths[0] + yw1, input_bases[0], 0);

                y = data00 * (1 - (y_input_h - yh0)) * (1 - (y_input_w - yw0)) +
                    data10 * (y_input_h - yh0)       * (1 - (y_input_w - yw0)) +
                    data01 * (1 - (y_input_h - yh0)) * (y_input_w - yw0) +
                    data11 * (y_input_h - yh0)       * (y_input_w - yw0);

                if (!y_only)
                {
                    vx_int32 uw0, uw1, uh0, uh1, vw0, vw1, vh0, vh1;

                    uw0 = yw0 / image->scales[1][VX_DIM_X];
                    uw1 = gcmMIN(uw0 + 1, (vx_int32)(input_widths[1] - 1));
                    uh0 = yh0 / image->scales[1][VX_DIM_Y];
                    uh1 = gcmMIN(uh0 + 1, (vx_int32)(input_heights[1] - 1));

                    data00 = vxnneGetData((vx_type_e)input_format, uh0 * input_widths[1] + uw0, input_bases[1], 0);
                    data01 = vxnneGetData((vx_type_e)input_format, uh0 * input_widths[1] + uw1, input_bases[1], 0);
                    data10 = vxnneGetData((vx_type_e)input_format, uh1 * input_widths[1] + uw0, input_bases[1], 0);
                    data11 = vxnneGetData((vx_type_e)input_format, uh1 * input_widths[1] + uw1, input_bases[1], 0);
                    u = data00 * (1 - (y_input_h - yh0)) * (1 - (y_input_w - yw0)) +
                        data10 * (y_input_h - yh0)       * (1 - (y_input_w - yw0)) +
                        data01 * (1 - (y_input_h - yh0)) * (y_input_w - yw0) +
                        data11 * (y_input_h - yh0)       * (y_input_w - yw0);

                    vw0 = yw0 / image->scales[2][VX_DIM_X];
                    vw1 = gcmMIN(vw0 + 1, (vx_int32)(input_widths[2] - 1));
                    vh0 = yh0 / image->scales[2][VX_DIM_Y];
                    vh1 = gcmMIN(vh0 + 1, (vx_int32)(input_heights[2] - 1));

                    data00 = vxnneGetData((vx_type_e)input_format, vh0 * input_widths[2] + vw0, input_bases[2], 0);
                    data01 = vxnneGetData((vx_type_e)input_format, vh0 * input_widths[2] + vw1, input_bases[2], 0);
                    data10 = vxnneGetData((vx_type_e)input_format, vh1 * input_widths[2] + vw0, input_bases[2], 0);
                    data11 = vxnneGetData((vx_type_e)input_format, vh1 * input_widths[2] + vw1, input_bases[2], 0);
                    v = data00 * (1 - (y_input_h - yh0)) * (1 - (y_input_w - yw0)) +
                        data10 * (y_input_h - yh0)       * (1 - (y_input_w - yw0)) +
                        data01 * (1 - (y_input_h - yh0)) * (y_input_w - yw0) +
                        data11 * (y_input_h - yh0)       * (y_input_w - yw0);

                    {
                        /* hardware conversion, similiar to digital BT601 */
                        yy = (vx_int32)y;
                        uu = (vx_int32)u;
                        vv = (vx_int32)v;
                        r  = (vx_float32)((yy * c0 + vv * c1           + c5) >> post_shift);
                        g  = (vx_float32)((yy * c0 - vv * c2 - uu * c3 + c6) >> post_shift);
                        b  = (vx_float32)((yy * c0           + uu * c4 + c7) >> post_shift);
                    }

                    if (output_rgb)
                    {
                        status |= vxnneSaveDataExt((vx_type_e)output_format, out_tf_format, 0 * output_width * output_height + h * output_width + w, b, output_base, out_fixpoint, out_tf_zp, out_tf_scale, out_rounding_mode);
                        status |= vxnneSaveDataExt((vx_type_e)output_format, out_tf_format, 1 * output_width * output_height + h * output_width + w, g, output_base, out_fixpoint, out_tf_zp, out_tf_scale, out_rounding_mode);
                        status |= vxnneSaveDataExt((vx_type_e)output_format, out_tf_format, 2 * output_width * output_height + h * output_width + w, r, output_base, out_fixpoint, out_tf_zp, out_tf_scale, out_rounding_mode);
                    }
                    else /* ouput bgr */
                    {
                        status |= vxnneSaveDataExt((vx_type_e)output_format, out_tf_format, 2 * output_width * output_height + h * output_width + w, b, output_base, out_fixpoint, out_tf_zp, out_tf_scale, out_rounding_mode);
                        status |= vxnneSaveDataExt((vx_type_e)output_format, out_tf_format, 1 * output_width * output_height + h * output_width + w, g, output_base, out_fixpoint, out_tf_zp, out_tf_scale, out_rounding_mode);
                        status |= vxnneSaveDataExt((vx_type_e)output_format, out_tf_format, 0 * output_width * output_height + h * output_width + w, r, output_base, out_fixpoint, out_tf_zp, out_tf_scale, out_rounding_mode);
                    }
                }
                else
                {
                    r = (y - r_mean) * rgb_scale;

                    status |= vxnneSaveDataExt((vx_type_e)output_format, out_tf_format, 0 * output_width * output_height + h * output_width + w, r, output_base, out_fixpoint, out_tf_zp, out_tf_scale, out_rounding_mode);
                }
            }
        }
    }

    return status;
}

vx_status vxnneExecuteSCYUV2RGBScale(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_yuv2rgb_scale_operation scaleOperation = (vxnne_yuv2rgb_scale_operation)operation;
    vx_graph graph = operation->layer->node->graph;
    gctUINT8 *stateBuffer = VX_NULL;
    vx_nn_cmd_info_u info;

    vx_image image = scaleOperation->inputs;
    vx_tensor output = scaleOperation->outputs;

    vx_float32 r_mean = ((vx_scalar)operation->layer->node->paramTable[2])->value->f32;
    vx_float32 g_mean = ((vx_scalar)operation->layer->node->paramTable[3])->value->f32;
    vx_float32 b_mean = ((vx_scalar)operation->layer->node->paramTable[4])->value->f32;
    vx_float32 rgb_scale = ((vx_scalar)operation->layer->node->paramTable[5])->value->f32;
    vx_bool y_only = ((vx_scalar)operation->layer->node->paramTable[6])->value->b;
    vx_bool output_rgb = ((vx_scalar)operation->layer->node->paramTable[7])->value->b;

    vx_rectangle_t rect = scaleOperation->rect;
    vx_uint32 scale_x = scaleOperation->x_scale;
    vx_uint32 scale_y = scaleOperation->y_scale;
    vx_uint16 x_init_err = scaleOperation->x_init_error;
    vx_uint16 y_init_err = scaleOperation->y_init_error;
    vx_uint16 x_init_int_err = scaleOperation->x_init_int_error;
    vx_uint16 y_init_int_err = scaleOperation->y_init_int_error;

    vx_uint32 input_address_y, input_address_u, input_address_v;
    vx_uint32 output_address, output_address_r, output_address_g, output_address_b;

    vx_uint32 input_hstride_y = image->memory.strides[0][VX_DIM_Y];

    vx_uint32 output_width  = TENSOR_VIEW_SIZE_INDEX(output, 0);
    vx_uint32 output_height = scaleOperation->output_y_end - scaleOperation->output_y_start;
    vx_uint32 output_stride = TENSOR_STRIDE_INDEX(output, 1);

    vx_uint32 output_bits_size = TENSOR_DATA_SIZE(output) * 8;

    vx_type_e output_format = (vx_type_e)TENSOR_DATA_TYPE(output);
    vx_int8 out_fixpoint      = TENSOR_POS(output);
    vx_enum out_tf_format     = TENSOR_QUANT_TYPE(output);
    vx_uint32 out_tf_zp       = TENSOR_TF_ZEROPOINT(output);
    vx_float32 out_tf_scale   = TENSOR_TF_SCALE(output);

    vx_uint32 input_rect_width, input_rect_height, input_width, input_height, offset;

    vx_uint8 post_shift;
    vx_int32 c0 = 0, c1 = 0, c2 = 0, c3 = 0, c4 = 0, c5 = 0, c6 = 0, c7 = 0;
    vx_float32 fc0 = .0f, fc1 = .0f, fc2 = .0f, fc3 = .0f, fc4 = .0f, fc5 = .0f, fc6 = .0f, fc7 = .0f, fmin, fmax, fr, fg, fb, fp = 1.0f;
    vx_int16 min_r_clamp, max_r_clamp, min_g_clamp, max_g_clamp, min_b_clamp, max_b_clamp;

    gcmASSERT(output_format == VX_TYPE_INT8 || output_format == VX_TYPE_INT16 || output_format == VX_TYPE_UINT8);

    if (output_format == VX_TYPE_UINT8 && out_tf_format != VX_QUANT_AFFINE_SCALE)
    {
        out_tf_scale = 1.0f;
        out_tf_zp = 0;
    }

    if (y_only)
    {
        if (output_format == VX_TYPE_UINT8)
        {
            fc0 = rgb_scale / out_tf_scale;
            fc5 = 0.5f - r_mean * rgb_scale / out_tf_scale + out_tf_zp;
        }
        else /* output_format == VX_TYPE_INT8 || output_format == VX_TYPE_INT16 */
        {
            fp = out_fixpoint > 0 ? (vx_float32)(1 << out_fixpoint) : (1.0f / (vx_float32)(1 << -out_fixpoint));

            fc0 = rgb_scale * fp;
            fc5 = 0.5f - r_mean * rgb_scale * fp;
        }
    }
    else
    {
        const vx_float32 CST_CY  =  298.0f / 256;
        const vx_float32 CST_CUB =  517.0f / 256;
        const vx_float32 CST_CUG = -100.0f / 256;
        const vx_float32 CST_CVG = -208.0f / 256;
        const vx_float32 CST_CVR =  409.0f / 256;
        const vx_float32 CST_R   = 56992.0f / 256;
        const vx_float32 CST_G   = 34784.0f / 256;
        const vx_float32 CST_B   = 70816.0f / 256;

        if (output_format == VX_TYPE_UINT8)
        {
            fc0 =            CST_CY * rgb_scale / out_tf_scale;
            fc1 =           CST_CVR * rgb_scale / out_tf_scale;
            fc2 =           CST_CVG * rgb_scale / out_tf_scale;
            fc3 =           CST_CUG * rgb_scale / out_tf_scale;
            fc4 =           CST_CUB * rgb_scale / out_tf_scale;
            fc5 = 0.5f - (CST_R + r_mean) * rgb_scale / out_tf_scale + out_tf_zp;
            fc6 = 0.5f + (CST_G - g_mean) * rgb_scale / out_tf_scale + out_tf_zp;
            fc7 = 0.5f - (CST_B + b_mean) * rgb_scale / out_tf_scale + out_tf_zp;
        }
        else /* output_format == VX_TYPE_INT8 || output_format == VX_TYPE_INT16 */
        {
            fp = out_fixpoint > 0 ? (vx_float32)(1 << out_fixpoint) : (1.0f / (vx_float32)(1 << -out_fixpoint));

            fc0 =            CST_CY * rgb_scale * fp;
            fc1 =           CST_CVR * rgb_scale * fp;
            fc2 =           CST_CVG * rgb_scale * fp;
            fc3 =           CST_CUG * rgb_scale * fp;
            fc4 =           CST_CUB * rgb_scale * fp;
            fc5 = 0.5f - (CST_R + r_mean) * rgb_scale * fp;
            fc6 = 0.5f + (CST_G - g_mean) * rgb_scale * fp;
            fc7 = 0.5f - (CST_B + b_mean) * rgb_scale * fp;
        }
    }

    fr = (vx_float32)fabs(fc0 * 255 + fc1 * 127 + fc5);
    fg = (vx_float32)fabs(fc0 * 255 + fc3 * 127 + fc2 * 127 + fc6);
    fb = (vx_float32)fabs(fc0 * 255 + fc4 * 127 + fc7);

    fmin = gcmMIN(fr, gcmMIN(fg, fb));

    if (fmin <= 0x07)
      post_shift = 13;
    else if (fmin <= 0x0F)
      post_shift = 12;
    else if (fmin <= 0x1F)
      post_shift = 11;
    else if (fmin <= 0x3F)
      post_shift = 10;
    else if (fmin <= 0x7F)
      post_shift = 9;
    else /* if (fmin <= 0xFF) */
      post_shift = 8;

    fmax = gcmMAX(fc0, gcmMAX(fc1, gcmMAX((fc2 * -1.0f), gcmMAX((fc3 * -1.0f), fc4))));
    if (fmax * (1 << post_shift) >= 1023)
    {
        /* c0 - c4 register is 10 bit */
        post_shift = (vx_uint8)(log(1023.0f / fmax) / log(2.0f));
    }

    c0 = (vx_int32)(fc0 * (1 << post_shift) + 0.5f);
    c1 = (vx_int32)(fc1 * (1 << post_shift) + 0.5f);
    c2 = (vx_int32)-(fc2 * (1 << post_shift) + 0.5f);
    c3 = (vx_int32)-(fc3 * (1 << post_shift) + 0.5f);
    c4 = (vx_int32)(fc4 * (1 << post_shift) + 0.5f);
    c5 = (vx_int32)(fc5 * (1 << post_shift) + 0.5f);
    c6 = (vx_int32)(fc6 * (1 << post_shift) + 0.5f);
    c7 = (vx_int32)(fc7 * (1 << post_shift) + 0.5f);

    gcmASSERT(image->format == VX_DF_IMAGE_IYUV);
    gcmASSERT(output_bits_size == 16 || output_bits_size == 8);

    input_rect_width  = rect.end_x - rect.start_x;
    input_rect_height = rect.end_y - rect.start_y;

    input_width  = image->region.start_x > image->region.end_x ? image->memory.dims[0][VX_DIM_X] : image->region.end_x - image->region.start_x;
    input_height = image->region.start_y > image->region.end_y ? image->memory.dims[0][VX_DIM_Y] : image->region.end_y - image->region.start_y;

    gcmASSERT(input_rect_width <= input_width);
    gcmASSERT(input_rect_height <= input_height);

    input_address_y = image->memory.physicals[0] + vxComputePlaneOffset(image, rect.start_x, rect.start_y, 0);
    input_address_u = image->memory.physicals[1] + vxComputePlaneOffset(image, rect.start_x, rect.start_y, 1);
    input_address_v = image->memory.physicals[2] + vxComputePlaneOffset(image, rect.start_x, rect.start_y, 2);

    vxoTensor_GetTensorViewMemory(output, VX_NULL, &output_address);
    offset = scaleOperation->output_y_start * TENSOR_STRIDE_INDEX(output, 1);
    output_address_r = output_address + offset;
    output_address_g = output_address + offset + TENSOR_STRIDE_INDEX(output, 2) * 1;
    output_address_b = output_address + offset + TENSOR_STRIDE_INDEX(output, 2) * 2;

    {
        vx_float32 min = 0;
        vx_float32 max = 255;
        vx_float32 min_r = (min - r_mean) * rgb_scale;
        vx_float32 max_r = (max - r_mean) * rgb_scale;
        vx_float32 min_g = (min - g_mean) * rgb_scale;
        vx_float32 max_g = (max - g_mean) * rgb_scale;
        vx_float32 min_b = (min - b_mean) * rgb_scale;
        vx_float32 max_b = (max - b_mean) * rgb_scale;

        if (output_format == VX_TYPE_UINT8)
        {
            min_r_clamp = (vx_int16)gcmMAX(0, vxnneRound(min_r / out_tf_scale + out_tf_zp, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING));
            max_r_clamp = (vx_int16)gcmMIN(255, vxnneRound(max_r / out_tf_scale + out_tf_zp, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING));
            min_g_clamp = (vx_int16)gcmMAX(0, vxnneRound(min_g / out_tf_scale + out_tf_zp, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING));
            max_g_clamp = (vx_int16)gcmMIN(255, vxnneRound(max_g / out_tf_scale + out_tf_zp, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING));
            min_b_clamp = (vx_int16)gcmMAX(0, vxnneRound(min_b / out_tf_scale + out_tf_zp, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING));
            max_b_clamp = (vx_int16)gcmMIN(255, vxnneRound(max_b / out_tf_scale + out_tf_zp, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING));
        }
        else
        {
            if (output_format == VX_TYPE_INT8)
            {
                min_r_clamp = (vx_int16)gcmMAX(-128,vxnneRound(min_r * fp, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING));
                max_r_clamp = (vx_int16)gcmMIN(127,vxnneRound(max_r * fp, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING));
                min_g_clamp = (vx_int16)gcmMAX(-128,vxnneRound(min_g * fp, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING));
                max_g_clamp = (vx_int16)gcmMIN(127,vxnneRound(max_g * fp, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING));
                min_b_clamp = (vx_int16)gcmMAX(-128,vxnneRound(min_b * fp, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING));
                max_b_clamp = (vx_int16)gcmMIN(127,vxnneRound(max_b * fp, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING));
            }
            else /* INT16 */
            {
                min_r_clamp = (vx_int16)gcmMAX(-32768,vxnneRound(min_r * fp, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING));
                max_r_clamp = (vx_int16)gcmMIN(32767,vxnneRound(max_r * fp, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING));
                min_g_clamp = (vx_int16)gcmMAX(-32768,vxnneRound(min_g * fp, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING));
                max_g_clamp = (vx_int16)gcmMIN(32767,vxnneRound(max_g * fp, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING));
                min_b_clamp = (vx_int16)gcmMAX(-32768,vxnneRound(min_b * fp, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING));
                max_b_clamp = (vx_int16)gcmMIN(32767,vxnneRound(max_b * fp, VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING));
            }
        }
    }

    /* send command */
    memset(&info, 0, sizeof(vx_nn_cmd_info_u));

    info.vx_yuv2rgb_scaler_cmd_info.inImageBaseY     = input_address_y;
    info.vx_yuv2rgb_scaler_cmd_info.inImageBaseU     = input_address_u;
    info.vx_yuv2rgb_scaler_cmd_info.inImageBaseV     = input_address_v;

    info.vx_yuv2rgb_scaler_cmd_info.inRectX          = (vx_uint16)rect.start_x;
    info.vx_yuv2rgb_scaler_cmd_info.inRectY          = (vx_uint16)rect.start_y;

    info.vx_yuv2rgb_scaler_cmd_info.inRectWidth      = (vx_uint16)input_rect_width;
    info.vx_yuv2rgb_scaler_cmd_info.inRectHeight     = (vx_uint16)input_rect_height;

    info.vx_yuv2rgb_scaler_cmd_info.inImageWidth     = (vx_uint16)input_width;
    info.vx_yuv2rgb_scaler_cmd_info.inImageHeight    = (vx_uint16)input_height;

    info.vx_yuv2rgb_scaler_cmd_info.inImageStrideY   = (vx_uint16)input_hstride_y;

    if (output_rgb)
    {
        info.vx_yuv2rgb_scaler_cmd_info.outImageBaseR    = output_address_r;
        info.vx_yuv2rgb_scaler_cmd_info.outImageBaseG    = output_address_g;
        info.vx_yuv2rgb_scaler_cmd_info.outImageBaseB    = output_address_b;
    }
    else /* output bgr */
    {
        info.vx_yuv2rgb_scaler_cmd_info.outImageBaseR    = !y_only ? output_address_b : output_address_r;
        info.vx_yuv2rgb_scaler_cmd_info.outImageBaseG    = output_address_g;
        info.vx_yuv2rgb_scaler_cmd_info.outImageBaseB    = !y_only ? output_address_r : output_address_b;
    }

    info.vx_yuv2rgb_scaler_cmd_info.outImageWidth    = (vx_uint16)output_width;
    info.vx_yuv2rgb_scaler_cmd_info.outImageHeight   = (vx_uint16)output_height;

    info.vx_yuv2rgb_scaler_cmd_info.outImageStride   = (vx_uint16)output_stride;
    info.vx_yuv2rgb_scaler_cmd_info.outImageBitsSize = (vx_uint16)output_bits_size;

    info.vx_yuv2rgb_scaler_cmd_info.scaleX           = scale_x;
    info.vx_yuv2rgb_scaler_cmd_info.scaleY           = scale_y;

    info.vx_yuv2rgb_scaler_cmd_info.inImageInitErrX     = x_init_err;
    info.vx_yuv2rgb_scaler_cmd_info.inImageInitErrY     = y_init_err;
    info.vx_yuv2rgb_scaler_cmd_info.inImageInitIntErrX  = x_init_int_err;
    info.vx_yuv2rgb_scaler_cmd_info.inImageInitIntErrY  = y_init_int_err;

    info.vx_yuv2rgb_scaler_cmd_info.yOnly            = y_only ? 1 : 0;
    info.vx_yuv2rgb_scaler_cmd_info.outSigned        = TENSOR_DATA_TYPE(output) == VX_TYPE_INT8 || TENSOR_DATA_TYPE(output) == VX_TYPE_INT16 ? 1 : 0;
    info.vx_yuv2rgb_scaler_cmd_info.postShift        = post_shift;

    info.vx_yuv2rgb_scaler_cmd_info.c0               = (vx_uint16)c0;
    info.vx_yuv2rgb_scaler_cmd_info.c1               = (vx_uint16)c1;
    info.vx_yuv2rgb_scaler_cmd_info.c2               = (vx_uint16)c2;
    info.vx_yuv2rgb_scaler_cmd_info.c3               = (vx_uint16)c3;
    info.vx_yuv2rgb_scaler_cmd_info.c4               = (vx_uint16)c4;
    info.vx_yuv2rgb_scaler_cmd_info.c5               = c5;
    info.vx_yuv2rgb_scaler_cmd_info.c6               = c6;
    info.vx_yuv2rgb_scaler_cmd_info.c7               = c7;

    info.vx_yuv2rgb_scaler_cmd_info.minRClamp        = min_r_clamp;
    info.vx_yuv2rgb_scaler_cmd_info.maxRClamp        = max_r_clamp;
    info.vx_yuv2rgb_scaler_cmd_info.minGClamp        = min_g_clamp;
    info.vx_yuv2rgb_scaler_cmd_info.maxGClamp        = max_g_clamp;
    info.vx_yuv2rgb_scaler_cmd_info.minBClamp        = min_b_clamp;
    info.vx_yuv2rgb_scaler_cmd_info.maxBClamp        = max_b_clamp;

    /*Per HW suggestion, default set 32*/
    {
        gctSTRING envctrl = gcvNULL;
        if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "SCALER_OUTSTANDING_REQUEST", &envctrl)) && envctrl)
        {
            info.vx_yuv2rgb_scaler_cmd_info.outRequestCount = atoi(envctrl);
        }
        else
        {
            info.vx_yuv2rgb_scaler_cmd_info.outRequestCount  = 32;
        }
        vxInfo("YUV2RGB scaler outstanding request is %d\n", info.vx_yuv2rgb_scaler_cmd_info.outRequestCount);
    }

    if (graph->binarySave)
    {
        vxmONERROR(gcoOS_Allocate(gcvNULL, VX_MAX_SC_OPERATION_STATE_SIZE, (gctPOINTER *)&stateBuffer));
        status = gcfVX_CaptureState(stateBuffer,
                                    VX_MAX_SC_OPERATION_STATE_SIZE,
                                    gcvNULL,
                                    gcvTRUE, gcvFALSE);
        if (status != VX_SUCCESS)
        {
            vxError("fail to capture scale states\n");
            vxmONERROR(VX_FAILURE);
        }
    }

    status = gcoVX_ProgrammYUV2RGBScale((void*)&info, operation->gpuId, operation->mGpuSync);

    if (graph->binarySave)
    {
        vx_node node = operation->layer->node;
        gctUINT32 actualSize = 0;

        status = gcfVX_CaptureState(gcvNULL, 0, &actualSize, gcvFALSE, gcvFALSE);
        if (actualSize <= 0)
        {
            vxError("error: fail to save layer name : %s to binary in SC operation\n", node->layer->name);
            vxmONERROR(VX_FAILURE);
        }

        vxmONERROR(vxoBinaryGraph_SaveScalerOperation(operation, stateBuffer, actualSize));
    }

OnError:
    if (stateBuffer != VX_NULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, stateBuffer));
    }

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNYUV2RGBScale(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    status = VX_SUCCESS;

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoYUV2RGBScale_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxoYUV2RGBScale_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}
#if REGISTER_FRAME
VX_PRIVATE_API vx_status vxoYUV2RGBScale_SW_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vx_image   image                      = (vx_image)parameters[0];
    vx_array   rects                      = (vx_array)parameters[1];
    vx_scalar  r_mean                     = (vx_scalar)parameters[2];
    vx_scalar  g_mean                     = (vx_scalar)parameters[3];
    vx_scalar  b_mean                     = (vx_scalar)parameters[4];
    vx_scalar  rgb_scale                  = (vx_scalar)parameters[5];
    vx_scalar  y_only                     = (vx_scalar)parameters[6];
    vx_scalar  output_rgb                 = (vx_scalar)parameters[7];
    vx_tensor  outputs                    = (vx_tensor)parameters[8];

    vx_rectangle_t rect;

    vx_uint32 input_rect_width, input_rect_height, input_width, input_height, output_width, output_height, scale_x, scale_y;

    vxnne_yuv2rgb_scale_layer  yuv2rgb_scaleLayer = (vxnne_yuv2rgb_scale_layer)ops_layer;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);
    rect.start_x = *((vx_uint32_ptr)rects->memory.logicals[0] + 0);
    rect.start_y = *((vx_uint32_ptr)rects->memory.logicals[0] + 1);
    rect.end_x   = *((vx_uint32_ptr)rects->memory.logicals[0] + 2);
    rect.end_y   = *((vx_uint32_ptr)rects->memory.logicals[0] + 3);

    if (!rect.end_x || rect.start_x >= rect.end_x)
    {
        rect.start_x = 0;
        rect.end_x = image->memory.dims[0][VX_DIM_X];
    }
    if (!rect.end_y || rect.start_y >= rect.end_y)
    {
        rect.start_y = 0;
        rect.end_y = image->memory.dims[0][VX_DIM_Y];
    }
    if (rect.end_x > (vx_uint32)image->memory.dims[0][VX_DIM_X]) rect.end_x = image->memory.dims[0][VX_DIM_X];
    if (rect.end_y > (vx_uint32)image->memory.dims[0][VX_DIM_Y]) rect.end_y = image->memory.dims[0][VX_DIM_Y];
    if (rect.start_x > rect.end_x) rect.start_x = 0;
    if (rect.start_y > rect.end_y) rect.start_y = 0;

    input_rect_width  = rect.end_x - rect.start_x;
    input_rect_height = rect.end_y - rect.start_y;

    input_width  = image->region.start_x > image->region.end_x ? image->memory.dims[0][VX_DIM_X] : image->region.end_x - image->region.start_x;
    input_height = image->region.start_y > image->region.end_y ? image->memory.dims[0][VX_DIM_Y] : image->region.end_y - image->region.start_y;

    if((input_rect_width <= input_width && input_rect_height <= input_height) == 0)
        vxmASSERT(0);

    output_width  = TENSOR_SIZE_INDEX(outputs, 0);
    output_height = TENSOR_SIZE_INDEX(outputs, 1);

    scale_x = (input_rect_width << 15) / output_width;
    scale_y = (input_rect_height << 15) / output_height;


    vxmONERROR(vxnneOperation_Initialize(
        &yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.base,
        &yuv2rgb_scaleLayer->base,
        VXNNE_OPERATION_TARGET_SW,
        VXNNE_OPERATOR_YUV2RGB_SCALE,
        vxnneExecuteSWYUV2RGBScale,
        VX_NULL,
        1,
        0));

    vxmONERROR(vxnneLayer_SetOperation(
        &yuv2rgb_scaleLayer->base,
        &yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.base,
        0));

    yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.inputs     = image;
    yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.r_mean     = r_mean;
    yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.g_mean     = g_mean;
    yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.b_mean     = b_mean;
    yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.rgb_scale  = rgb_scale;
    yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.y_only     = y_only;
    yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.output_rgb = output_rgb;
    yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.rect       = rect;
    yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.x_scale    = scale_x;
    yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.y_scale    = scale_y;
    yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.outputs    = outputs;

    vxmONERROR(vxnneOperation_AddReference(&yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.base, (vx_reference)image, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoYUV2RGBScale_NN_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_image   image                      = (vx_image)parameters[0];
    vx_array   rects                      = (vx_array)parameters[1];

    vx_rectangle_t rect;
    vx_uint32 input_rect_width, /*input_rect_height,*/ input_width, input_height;

    vx_bool support = vx_true_e;

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    rect.start_x = *((vx_uint32_ptr)rects->memory.logicals[0] + 0);
    rect.start_y = *((vx_uint32_ptr)rects->memory.logicals[0] + 1);
    rect.end_x   = *((vx_uint32_ptr)rects->memory.logicals[0] + 2);
    rect.end_y   = *((vx_uint32_ptr)rects->memory.logicals[0] + 3);

    if (!rect.end_x || rect.start_x >= rect.end_x)
    {
        rect.start_x = 0;
        rect.end_x = image->memory.dims[0][VX_DIM_X];
    }
    if (!rect.end_y || rect.start_y >= rect.end_y)
    {
        rect.start_y = 0;
        rect.end_y = image->memory.dims[0][VX_DIM_Y];
    }
    if (rect.end_x > (vx_uint32)image->memory.dims[0][VX_DIM_X]) rect.end_x = image->memory.dims[0][VX_DIM_X];
    if (rect.end_y > (vx_uint32)image->memory.dims[0][VX_DIM_Y]) rect.end_y = image->memory.dims[0][VX_DIM_Y];
    if (rect.start_x > rect.end_x) rect.start_x = 0;
    if (rect.start_y > rect.end_y) rect.start_y = 0;

    input_rect_width  = rect.end_x - rect.start_x;
    /*input_rect_height = rect.end_y - rect.start_y;*/

    input_width  = image->region.start_x > image->region.end_x ? image->memory.dims[0][VX_DIM_X] : image->region.end_x - image->region.start_x;
    input_height = image->region.start_y > image->region.end_y ? image->memory.dims[0][VX_DIM_Y] : image->region.end_y - image->region.start_y;

    support = support && vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SCALER);
    support = support && input_width <= 4096 && input_height <= 4096;
    support = support && (input_rect_width <= 1920 || vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SCALER_4K));

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoYUV2RGBScale_NN_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vx_image   image                      = (vx_image)parameters[0];
    vx_array   rects                      = (vx_array)parameters[1];
    vx_scalar  r_mean                     = (vx_scalar)parameters[2];
    vx_scalar  g_mean                     = (vx_scalar)parameters[3];
    vx_scalar  b_mean                     = (vx_scalar)parameters[4];
    vx_scalar  rgb_scale                  = (vx_scalar)parameters[5];
    vx_scalar  y_only                     = (vx_scalar)parameters[6];
    vx_scalar  output_rgb                 = (vx_scalar)parameters[7];
    vx_tensor  outputs                    = (vx_tensor)parameters[8];

    vx_rectangle_t rect;
    vx_uint32 input_rect_width, input_rect_height, input_width, input_height, output_width, output_height, scale_x, scale_y;

    vxnne_yuv2rgb_scale_layer  yuv2rgb_scaleLayer = (vxnne_yuv2rgb_scale_layer)ops_layer;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);
    rect.start_x = *((vx_uint32_ptr)rects->memory.logicals[0] + 0);
    rect.start_y = *((vx_uint32_ptr)rects->memory.logicals[0] + 1);
    rect.end_x   = *((vx_uint32_ptr)rects->memory.logicals[0] + 2);
    rect.end_y   = *((vx_uint32_ptr)rects->memory.logicals[0] + 3);

    if (!rect.end_x || rect.start_x >= rect.end_x)
    {
        rect.start_x = 0;
        rect.end_x = image->memory.dims[0][VX_DIM_X];
    }
    if (!rect.end_y || rect.start_y >= rect.end_y)
    {
        rect.start_y = 0;
        rect.end_y = image->memory.dims[0][VX_DIM_Y];
    }
    if (rect.end_x > (vx_uint32)image->memory.dims[0][VX_DIM_X]) rect.end_x = image->memory.dims[0][VX_DIM_X];
    if (rect.end_y > (vx_uint32)image->memory.dims[0][VX_DIM_Y]) rect.end_y = image->memory.dims[0][VX_DIM_Y];
    if (rect.start_x > rect.end_x) rect.start_x = 0;
    if (rect.start_y > rect.end_y) rect.start_y = 0;

    input_rect_width  = rect.end_x - rect.start_x;
    input_rect_height = rect.end_y - rect.start_y;

    input_width  = image->region.start_x > image->region.end_x ? image->memory.dims[0][VX_DIM_X] : image->region.end_x - image->region.start_x;
    input_height = image->region.start_y > image->region.end_y ? image->memory.dims[0][VX_DIM_Y] : image->region.end_y - image->region.start_y;

    if((input_rect_width <= input_width && input_rect_height <= input_height) == 0)
        vxmASSERT(0);

    output_width  = TENSOR_SIZE_INDEX(outputs, 0);
    output_height = TENSOR_SIZE_INDEX(outputs, 1);

    scale_x = (input_rect_width << 15) / output_width;
    scale_y = (input_rect_height << 15) / output_height;


    vxmONERROR(vxnneOperation_Initialize(
        &yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.base,
        &yuv2rgb_scaleLayer->base,
        VXNNE_OPERATION_TARGET_SC,
        VXNNE_OPERATOR_YUV2RGB_SCALE,
        vxnneExecuteSCYUV2RGBScale,
        VX_NULL,
        1,
        0));

    vxmONERROR(vxnneLayer_SetOperation(
        &yuv2rgb_scaleLayer->base,
        &yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.base,
        0));

    yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.inputs     = image;
    yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.r_mean     = r_mean;
    yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.g_mean     = g_mean;
    yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.b_mean     = b_mean;
    yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.rgb_scale  = rgb_scale;
    yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.y_only     = y_only;
    yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.output_rgb = output_rgb;
    yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.rect       = rect;
    yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.x_scale    = scale_x;
    yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.y_scale    = scale_y;
    yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.outputs    = outputs;

    vxmONERROR(vxnneOperation_AddReference(&yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.base, (vx_reference)image, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_status vxoNNLayer_GetOperations1(vxnne_layer ops_layer, vx_uint32_ptr max_num_operations, vxnne_operation **operations)
{
    vx_status  status = VX_SUCCESS;

    vxnne_yuv2rgb_scale_layer  yuv2rgb_scaleLayer = (vxnne_yuv2rgb_scale_layer)ops_layer;

    *max_num_operations = gcmCOUNTOF(yuv2rgb_scaleLayer->operations);

    *operations = yuv2rgb_scaleLayer->operations;

    return status;
}
#endif

VX_PRIVATE_API vx_status VX_CALLBACK vxoYUV2RGBScale_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
#if REGISTER_FRAME
    vxnne_layer_imp_s registerYUV2RGBScale[] = {/* Please DON'T adjust the order, it's importent */
        { "YUV2RGBScale NN", vxoYUV2RGBScale_NN_Support, vxoYUV2RGBScale_NN_Initialize, VX_NULL },
        { "YUV2RGBScale TP", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "YUV2RGBScale SH EVIS", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "YUV2RGBScale SH F32", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "YUV2RGBScale SW ", vxoNNCommon_Support, vxoYUV2RGBScale_SW_Initialize, VX_NULL },
    };

    REGISTER_LAYERS(registerYUV2RGBScale, vxnne_yuv2rgb_scale_layer_s, "YUV2RGBScale", vxoNNLayer_GetOperations1);

OnError:
#else
    vx_image   image                      = (vx_image)parameters[0];
    vx_array   rects                      = (vx_array)parameters[1];
    vx_scalar  r_mean                     = (vx_scalar)parameters[2];
    vx_scalar  g_mean                     = (vx_scalar)parameters[3];
    vx_scalar  b_mean                     = (vx_scalar)parameters[4];
    vx_scalar  rgb_scale                  = (vx_scalar)parameters[5];
    vx_scalar  y_only                     = (vx_scalar)parameters[6];
    vx_scalar  output_rgb                 = (vx_scalar)parameters[7];
    vx_tensor  outputs                    = (vx_tensor)parameters[8];

    vx_rectangle_t rect;
    vx_uint32 input_rect_width, input_rect_height, input_width, input_height, output_width, output_height, scale_x, scale_y;

    vxnne_yuv2rgb_scale_layer  yuv2rgb_scaleLayer = VX_NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }
    gcoOS_Allocate(gcvNULL, sizeof(vxnne_yuv2rgb_scale_layer_s), (gctPOINTER*)&yuv2rgb_scaleLayer);
    if (!yuv2rgb_scaleLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("Out of Memory at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(yuv2rgb_scaleLayer, sizeof(vxnne_yuv2rgb_scale_layer_s));

    vxnneLayer_Initialize(&yuv2rgb_scaleLayer->base,
                          "YUV2RGBScale",
                          node,
                          vxmOPERATION_COUNT(yuv2rgb_scaleLayer),
                          yuv2rgb_scaleLayer->operations,
                          VX_NULL);

    rect.start_x = *((vx_uint32_ptr)rects->memory.logicals[0] + 0);
    rect.start_y = *((vx_uint32_ptr)rects->memory.logicals[0] + 1);
    rect.end_x   = *((vx_uint32_ptr)rects->memory.logicals[0] + 2);
    rect.end_y   = *((vx_uint32_ptr)rects->memory.logicals[0] + 3);

    if (!rect.end_x || rect.start_x >= rect.end_x)
    {
        rect.start_x = 0;
        rect.end_x = image->memory.dims[0][VX_DIM_X];
    }
    if (!rect.end_y || rect.start_y >= rect.end_y)
    {
        rect.start_y = 0;
        rect.end_y = image->memory.dims[0][VX_DIM_Y];
    }
    if (rect.end_x > (vx_uint32)image->memory.dims[0][VX_DIM_X]) rect.end_x = image->memory.dims[0][VX_DIM_X];
    if (rect.end_y > (vx_uint32)image->memory.dims[0][VX_DIM_Y]) rect.end_y = image->memory.dims[0][VX_DIM_Y];
    if (rect.start_x > rect.end_x) rect.start_x = 0;
    if (rect.start_y > rect.end_y) rect.start_y = 0;

    input_rect_width  = rect.end_x - rect.start_x;
    input_rect_height = rect.end_y - rect.start_y;

    input_width  = image->region.start_x > image->region.end_x ? image->memory.dims[0][VX_DIM_X] : image->region.end_x - image->region.start_x;
    input_height = image->region.start_y > image->region.end_y ? image->memory.dims[0][VX_DIM_Y] : image->region.end_y - image->region.start_y;

    vxmASSERT(input_rect_width <= input_width && input_rect_height <= input_height);

    output_width  = TENSOR_SIZE_INDEX(outputs, 0);
    output_height = TENSOR_SIZE_INDEX(outputs, 1);

    scale_x = (input_rect_width << 15) / output_width;
    scale_y = (input_rect_height << 15) / output_height;

    if (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SCALER) &&
        input_width <= 4096 && input_height <= 4096 &&
        (input_rect_width <= 1920 || vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SCALER_4K)))
    {
        vxnneOperation_Initialize(
            &yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.base,
            &yuv2rgb_scaleLayer->base,
            VXNNE_OPERATION_TARGET_SC,
            VXNNE_OPERATOR_YUV2RGB_SCALE,
            vxnneExecuteSCYUV2RGBScale,
            VX_NULL,
            1,
            0);

        vxnneLayer_SetOperation(
            &yuv2rgb_scaleLayer->base,
            &yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.base,
            0);

        yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.inputs     = image;
        yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.r_mean     = r_mean;
        yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.g_mean     = g_mean;
        yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.b_mean     = b_mean;
        yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.rgb_scale  = rgb_scale;
        yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.y_only     = y_only;
        yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.output_rgb = output_rgb;
        yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.rect       = rect;
        yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.x_scale    = scale_x;
        yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.y_scale    = scale_y;
        yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.outputs    = outputs;

        vxnneOperation_AddReference(&yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.base, (vx_reference)image, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&yuv2rgb_scaleLayer->yuv2rgb_scale_sc_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }
    else
    {
        vxnneOperation_Initialize(
            &yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.base,
            &yuv2rgb_scaleLayer->base,
            VXNNE_OPERATION_TARGET_SW,
            VXNNE_OPERATOR_YUV2RGB_SCALE,
            vxnneExecuteSWYUV2RGBScale,
            VX_NULL,
            1,
            0);

        vxnneLayer_SetOperation(
            &yuv2rgb_scaleLayer->base,
            &yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.base,
            0);

        yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.inputs     = image;
        yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.r_mean     = r_mean;
        yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.g_mean     = g_mean;
        yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.b_mean     = b_mean;
        yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.rgb_scale  = rgb_scale;
        yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.y_only     = y_only;
        yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.output_rgb = output_rgb;
        yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.rect       = rect;
        yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.x_scale    = scale_x;
        yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.y_scale    = scale_y;
        yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.outputs    = outputs;

        vxnneOperation_AddReference(&yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.base, (vx_reference)image, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&yuv2rgb_scaleLayer->yuv2rgb_scale_sw_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }

    node->layer = &yuv2rgb_scaleLayer->base;
#endif

    return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoYUV2RGBScale_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}


