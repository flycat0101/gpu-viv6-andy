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
#include <layers/gc_vx_layer_roi_pool.h>

extern vx_tensor _AllocateTPLUTorListBuffer(vx_context context, vx_node node, vx_uint32 size, vx_enum type);

VX_PRIVATE_API vx_tensor vxnneAllocateTPROIListBuffer(vx_context context, vx_node node, vx_uint32 size, vx_enum type)
{
    return _AllocateTPLUTorListBuffer(context, node, size, type);
}

VX_PRIVATE_API void vxnneInitROITensorFromBuffer(vx_tensor tensor, void* buffer, vx_uint32 element_size)
{
    vx_uint8_ptr data_ptr;

    vxoTensor_GetTensorViewMemory(tensor, (gctPOINTER *)&data_ptr, VX_NULL);

    vxMemCopy(data_ptr, buffer, element_size);
}

/**************************************************************************************
 *                     ROI POOL
 *************************************************************************************/
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

    vx_type_e in_data_format    = (vx_type_e)TENSOR_DATA_TYPE(input_data);
    vx_type_e in_roi_format     = (vx_type_e)TENSOR_DATA_TYPE(input_roi);
    vx_type_e out_format        = (vx_type_e)TENSOR_DATA_TYPE(output);
    vx_int8 in_data_fp          = TENSOR_POS(input_data);
    vx_int8 in_roi_fp           = TENSOR_POS(input_roi);
    vx_int8 out_fp              = TENSOR_POS(output);
    vx_enum in_roi_rMode        = TENSOR_ROUNDING_MODE(input_roi);
    vx_enum out_rMode           = TENSOR_ROUNDING_MODE(output);

    vx_int32 in_data_items      = vxnneGetTypeSize(in_data_format);
    vx_int32 in_roi_items       = vxnneGetTypeSize(in_roi_format);
    vx_int32 out_items          = vxnneGetTypeSize(out_format);

    vx_uint8_ptr input_data_ptr,rois_data_ptr,output_data_ptr;
    vx_int32 n, c, ph, pw, h, w;

    vx_bool enable_relu = vx_false_e;

    if (roipoolOperation->relu != VX_NULL)
        enable_relu = roipoolOperation->relu->value->b;

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
        /* map the roi coordinates to the feature map */
        roi_start_w = (vx_int32)vxnneRound((vx_float32)vxnneGetDataExt(in_roi_format, TENSOR_QUANT_TYPE(input_roi), offset, rois_data_ptr, in_roi_fp, TENSOR_TF_ZEROPOINT(input_roi), TENSOR_TF_SCALE(input_roi)) * spatial_scale, in_roi_rMode);
        roi_start_h = (vx_int32)vxnneRound((vx_float32)vxnneGetDataExt(in_roi_format, TENSOR_QUANT_TYPE(input_roi), offset + 1, rois_data_ptr, in_roi_fp, TENSOR_TF_ZEROPOINT(input_roi), TENSOR_TF_SCALE(input_roi)) * spatial_scale, in_roi_rMode);
        roi_end_w = (vx_int32)vxnneRound((vx_float32)vxnneGetDataExt(in_roi_format, TENSOR_QUANT_TYPE(input_roi), offset + 2, rois_data_ptr, in_roi_fp, TENSOR_TF_ZEROPOINT(input_roi), TENSOR_TF_SCALE(input_roi)) * spatial_scale, in_roi_rMode);
        roi_end_h = (vx_int32)vxnneRound((vx_float32)vxnneGetDataExt(in_roi_format, TENSOR_QUANT_TYPE(input_roi), offset + 3, rois_data_ptr, in_roi_fp, TENSOR_TF_ZEROPOINT(input_roi), TENSOR_TF_SCALE(input_roi)) * spatial_scale, in_roi_rMode);

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
                                vx_float32 batch_data_v = 0.0f;

                                batch_data_v = (vx_float32)vxnneGetDataExt(in_data_format, TENSOR_QUANT_TYPE(input_data), index, batch_data, in_data_fp, TENSOR_TF_ZEROPOINT(input_data), TENSOR_TF_SCALE(input_data));

                                if (batch_data_v > output_data_v)
                                    output_data_v = batch_data_v;
                            }
                        }
                    }

                    if (enable_relu)
                    {
                        if (output_data_v < 0)
                            output_data_v = 0;
                    }

                    /* Save the max value to the output */
                    vxnneSaveDataExt(out_format, TENSOR_QUANT_TYPE(output), pool_index, output_data_v, output_data_ptr, out_fp, TENSOR_TF_ZEROPOINT(output), TENSOR_TF_SCALE(output), out_rMode);
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
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}
#if REGISTER_FRAME
VX_PRIVATE_API vx_status vxoROIPoolLayer_SW_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vxnne_tensor_roipool_layer  roipoolLayer = (vxnne_tensor_roipool_layer)ops_layer;

    vx_tensor  input_data                 = (vx_tensor)parameters[0];
    vx_tensor  input_rois                 = (vx_tensor)parameters[1];
    vx_scalar  pool_types                 = (vx_scalar)parameters[2];
    vx_scalar  spatial_scales             = (vx_scalar)parameters[3];
    vx_scalar  pooled_heights             = (vx_scalar)parameters[4];
    vx_scalar  pooled_widths              = (vx_scalar)parameters[5];
    vx_tensor  outputs                    = (vx_tensor)parameters[6];
    vx_scalar  relu                       = (vx_scalar)parameters[7];

    vx_uint32  batchCount                 = TENSOR_SIZE_INDEX(input_data, 3);

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxnneOperation_Initialize(&roipoolLayer->tensorROIPoolSW.base,
        &roipoolLayer->base,
        VXNNE_OPERATION_TARGET_SW,
        VXNNE_OPERATOR_ROIPOOL,
        vxnneExecuteSWROIPooling,
        VX_NULL,
        batchCount,
        0));

    vxmONERROR(vxnneLayer_SetOperation(
             &roipoolLayer->base,
             &roipoolLayer->tensorROIPoolSW.base,
             0));

    roipoolLayer->tensorROIPoolSW.input_data      = input_data;
    roipoolLayer->tensorROIPoolSW.input_rois      = input_rois;
    roipoolLayer->tensorROIPoolSW.pool_type       = pool_types;
    roipoolLayer->tensorROIPoolSW.pooled_height   = pooled_heights;
    roipoolLayer->tensorROIPoolSW.pooled_width    = pooled_widths;
    roipoolLayer->tensorROIPoolSW.spatial_scale   = spatial_scales;
    roipoolLayer->tensorROIPoolSW.output          = outputs;
    roipoolLayer->tensorROIPoolSW.relu            = relu;

    vxmONERROR(vxnneOperation_AddReference(&roipoolLayer->tensorROIPoolSW.base, (vx_reference)input_data, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&roipoolLayer->tensorROIPoolSW.base, (vx_reference)input_rois, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&roipoolLayer->tensorROIPoolSW.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoROIPoolLayer_SH_EVIS_Support_Ext(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_tensor  input_data                 = (vx_tensor)parameters[0];
    vx_tensor  input_rois                 = (vx_tensor)parameters[1];

    vx_scalar  pooled_heights             = (vx_scalar)parameters[4];
    vx_scalar  pooled_widths              = (vx_scalar)parameters[5];
    vx_tensor  outputs                    = (vx_tensor)parameters[6];

    vx_uint32  pool_width                 = pooled_widths->value->u32;
    vx_uint32  pool_height                = pooled_heights->value->u32;

    vx_uint32  width                      = TENSOR_VIEW_SIZE_INDEX(input_data, 0);
    vx_uint32  height                     = TENSOR_VIEW_SIZE_INDEX(input_data, 1);
    vx_uint32  roi_stride                 = TENSOR_VIEW_SIZE_INDEX(input_rois, 2);

    vx_enum    inputFormat                = TENSOR_DATA_TYPE(input_data);
    vx_enum    roisFormat                 = TENSOR_DATA_TYPE(input_rois);
    vx_enum    outputFormat               = TENSOR_DATA_TYPE(outputs);

    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vx_bool    shExe_flag0  = (vx_bool)(width == 20 && height == 16 && roi_stride == 5 && pool_width == 6 && pool_height == 6 && inputFormat == VX_TYPE_FLOAT16 && roisFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16);
    vx_bool    shExe_flag1  = (vx_bool)(width == 51 && height == 39 && roi_stride == 5 && pool_width == 6 && pool_height == 6 && inputFormat == VX_TYPE_FLOAT16 && roisFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16);
    vx_bool    shExe_flag2  = (vx_bool)(width == 51 && height == 39 && roi_stride == 5 && pool_width == 6 && pool_height == 6 && inputFormat == VX_TYPE_INT8 && roisFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT8);
    vx_bool    shExe_flag3  = (vx_bool)(width == 20 && height == 16 && roi_stride == 5 && pool_width == 6 && pool_height == 6 && inputFormat == VX_TYPE_INT8 && roisFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT8);
    vx_bool    shExe_flag4  = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && roisFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) /*||
                                                (inputFormat == VX_TYPE_INT16 && roisFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT16)||
                                                (inputFormat == VX_TYPE_UINT8 && roisFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_UINT8)*/);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support =  support && (shExe_flag2 || shExe_flag3 || shExe_flag4);

    if (support)
    {
        SETBIT(reg_param->flag, shExe_flag0, 0);
        SETBIT(reg_param->flag, shExe_flag1, 1);
        SETBIT(reg_param->flag, shExe_flag2, 2);
        SETBIT(reg_param->flag, shExe_flag3, 3);
        SETBIT(reg_param->flag, shExe_flag4, 4);
    }

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_bool vxoROIPoolLayer_SH_EVIS_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && node->base.context->evisNoInst.supportEVIS;

    if (!support)return support;

    support = support && vxoROIPoolLayer_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_true_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoROIPoolLayer_SH_EVIS_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vx_tensor  input_data                 = (vx_tensor)parameters[0];
    vx_tensor  input_rois                 = (vx_tensor)parameters[1];
    vx_scalar  spatial_scales             = (vx_scalar)parameters[3];
    vx_scalar  pooled_heights             = (vx_scalar)parameters[4];
    vx_scalar  pooled_widths              = (vx_scalar)parameters[5];
    vx_tensor  outputs                    = (vx_tensor)parameters[6];
    vx_scalar  relu                       = (vx_scalar)parameters[7];

    vx_float32 spatial_scale              = spatial_scales->value->f32;
    vx_uint32  pool_width                 = pooled_widths->value->u32;
    vx_uint32  pool_height                = pooled_heights->value->u32;

    vx_uint32  width                      = TENSOR_VIEW_SIZE_INDEX(input_data, 0);
    vx_uint32  height                     = TENSOR_VIEW_SIZE_INDEX(input_data, 1);
    vx_uint32  depth                      = TENSOR_VIEW_SIZE_INDEX(input_data, 2);
    vx_uint32  roi_stride                 = TENSOR_VIEW_SIZE_INDEX(input_rois, 2);
    vx_uint32  rois_num                   = TENSOR_VIEW_SIZE_INDEX(input_rois, 3);

    vx_uint32  batchCount                 = TENSOR_SIZE_INDEX(input_data, 3);

    vxnne_tensor_roipool_layer  roipoolLayer = (vxnne_tensor_roipool_layer)ops_layer;

    vx_bool    shExe_flag0  = GETBIT(reg_param->flag, 0);
    vx_bool    shExe_flag1  = GETBIT(reg_param->flag, 1);
    vx_bool    shExe_flag2  = GETBIT(reg_param->flag, 2);
    vx_bool    shExe_flag3  = GETBIT(reg_param->flag, 3);
    vx_bool    shExe_flag4  = GETBIT(reg_param->flag, 4);

    vx_tensor_create_params_t  tensor_create_params;

    vxnne_shader_executable shaderExecutable = VX_NULL;
    vx_bool enable_relu = relu ? relu->value->b : vx_false_e;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    if (shExe_flag0 || shExe_flag1 || shExe_flag2 || shExe_flag3)
    {
        vx_uint32 imgNum                = width == 51 ? 7 : 4;
        vx_uint32 dims                  = 3;
        vx_uint32 tmp_sizes0[3]         = {width, height, depth * imgNum };
        vx_uint32 tmp_sizes1[3]         = {84, rois_num, 1 };
        vx_uint32 outputs_dims          = outputs->dimCount;
        vx_tensor outputs_reshp         = NULL;
        vx_tensor vertMaxPoolTensor, preTreatedRectTensor;

        gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
        tensor_create_params.num_of_dims = dims;
        tensor_create_params.sizes = tmp_sizes0;
        tensor_create_params.data_format = TENSOR_DATA_TYPE(outputs);
        tensor_create_params.quant_format = TENSOR_QUANT_TYPE(outputs);
        if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
        {
            tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(outputs);
        }
        else
        {
            tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(outputs);
            tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(outputs);
        }

        vertMaxPoolTensor = vxoTensor_CreateTensor(ops_layer->node->base.context, ops_layer->node->graph, &tensor_create_params, vx_true_e);
        if (vertMaxPoolTensor == VX_NULL)
        {
            vxError("vxoTensor_CreateTensor fail at function %s, line %d", __FUNCTION__, __LINE__);
            status = VX_ERROR_NO_MEMORY;
            goto OnError;
        }

        tensor_create_params.data_format = TENSOR_DATA_TYPE(input_rois);
        tensor_create_params.quant_format = TENSOR_QUANT_TYPE(input_rois);
        if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
        {
            tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(input_rois);
        }
        else
        {
            tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(input_rois);
            tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(input_rois);
        }
        tensor_create_params.sizes = tmp_sizes1;
        preTreatedRectTensor = vxoTensor_CreateTensor(ops_layer->node->base.context, ops_layer->node->graph, &tensor_create_params, vx_true_e);
        if (preTreatedRectTensor == VX_NULL)
        {
            vxError("vxoTensor_CreateTensor fail at function %s, line %d", __FUNCTION__, __LINE__);
            status = VX_ERROR_NO_MEMORY;
            goto OnError;
        }

        //operation1:vertMaxPool
        shaderExecutable = vxnneVertMaxPoolShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_VERTMAXPOOL, &ops_layer->node->kernelAttributes.borderMode, input_data, pool_width, pool_height, enable_relu, vertMaxPoolTensor);

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto OnError;
        }
        vxmONERROR(vxnneShaderOperation_Initialize(&roipoolLayer->vertmaxpool_operation.vertmaxpool_SHoperation,
            &roipoolLayer->base,
            VXNNE_OPERATOR_VERTMAXPOOL,
            batchCount,
            shaderExecutable));

        vxmONERROR(vxnneOperation_AddReference(&roipoolLayer->vertmaxpool_operation.vertmaxpool_SHoperation.base, (vx_reference)input_data, VXNNE_OPERATION_REFENRENCE_INPUT));
        vxmONERROR(vxnneOperation_AddReference(&roipoolLayer->vertmaxpool_operation.vertmaxpool_SHoperation.base, (vx_reference)vertMaxPoolTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT));

        //operation2:preTreatedRect
        shaderExecutable = vxnnePreTreatedRectShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_PRETREATEDRECT, &ops_layer->node->kernelAttributes.borderMode, input_rois, roi_stride, rois_num, width, height, spatial_scale, preTreatedRectTensor);


        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto OnError;
        }
        vxmONERROR(vxnneShaderOperation_Initialize(&roipoolLayer->pretreatedrect_operation.pretreatedrect_SHoperation,
            &roipoolLayer->base,
            VXNNE_OPERATOR_PRETREATEDRECT,
            batchCount,
            shaderExecutable));

        vxmONERROR(vxnneOperation_AddReference(&roipoolLayer->pretreatedrect_operation.pretreatedrect_SHoperation.base, (vx_reference)input_rois, VXNNE_OPERATION_REFENRENCE_INPUT));
        vxmONERROR(vxnneOperation_AddReference(&roipoolLayer->pretreatedrect_operation.pretreatedrect_SHoperation.base, (vx_reference)preTreatedRectTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT));

        //operation3:horzMaxPool
        if(outputs_dims == 4)
        {
            vx_int32 new_size[3] = {pool_width * pool_height, depth, rois_num};
            outputs_dims = 3;
            outputs_reshp = vxoTensor_ReshapeTensor(outputs, new_size, outputs_dims);
            TENSOR_POS(outputs_reshp) = TENSOR_POS(outputs);
        }

        TENSOR_POS(vertMaxPoolTensor) = TENSOR_POS(input_data);
        if(outputs_reshp)
        {
            shaderExecutable = vxnneHorzMaxPoolShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_HORZMAXPOOL, &ops_layer->node->kernelAttributes.borderMode, vertMaxPoolTensor, preTreatedRectTensor, outputs_reshp);
            vxmONERROR(vxoTensor_ReleaseTensor(&outputs_reshp));
        }
        else
        {
            shaderExecutable = vxnneHorzMaxPoolShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_HORZMAXPOOL, &ops_layer->node->kernelAttributes.borderMode, vertMaxPoolTensor, preTreatedRectTensor, outputs);
        }

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto OnError;
        }
        vxmONERROR(vxnneShaderOperation_Initialize(&roipoolLayer->horzmaxpool_operation.horzmaxpool_SHoperation,
            &roipoolLayer->base,
            VXNNE_OPERATOR_HORZMAXPOOL,
            batchCount,
            shaderExecutable));

        vxmONERROR(vxnneOperation_AddReference(&roipoolLayer->horzmaxpool_operation.horzmaxpool_SHoperation.base, (vx_reference)vertMaxPoolTensor, VXNNE_OPERATION_REFENRENCE_INPUT));
        vxmONERROR(vxnneOperation_AddReference(&roipoolLayer->horzmaxpool_operation.horzmaxpool_SHoperation.base, (vx_reference)preTreatedRectTensor, VXNNE_OPERATION_REFENRENCE_INPUT));
        vxmONERROR(vxnneOperation_AddReference(&roipoolLayer->horzmaxpool_operation.horzmaxpool_SHoperation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

        vxmONERROR(vxnneLayer_SetOperation(
            &roipoolLayer->base,
            &roipoolLayer->vertmaxpool_operation.vertmaxpool_SHoperation.base,
            0));
        vxmONERROR(vxnneLayer_SetOperation(
            &roipoolLayer->base,
            &roipoolLayer->pretreatedrect_operation.pretreatedrect_SHoperation.base,
            1));
        vxmONERROR(vxnneLayer_SetOperation(
            &roipoolLayer->base,
            &roipoolLayer->horzmaxpool_operation.horzmaxpool_SHoperation.base,
            2));

        roipoolLayer->base.num_temp_tensors                  = 2;
        roipoolLayer->base.temp_tensors[0] = vertMaxPoolTensor;
        roipoolLayer->base.temp_tensors[1] = preTreatedRectTensor;
    }
    else if (shExe_flag4)
    {
        shaderExecutable = vxnneROIPoolShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_ROIPOOL, &ops_layer->node->kernelAttributes.borderMode, input_data, input_rois, pool_width, pool_height, spatial_scale, enable_relu, outputs);

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto OnError;
        }
        vxmONERROR(vxnneShaderOperation_Initialize(&roipoolLayer->tensorROIPoolSH,
            &roipoolLayer->base,
            VXNNE_OPERATOR_ROIPOOL,
            batchCount,
            shaderExecutable));

        vxmONERROR(vxnneOperation_AddReference(&roipoolLayer->tensorROIPoolSH.base, (vx_reference)input_data, VXNNE_OPERATION_REFENRENCE_INPUT));
        vxmONERROR(vxnneOperation_AddReference(&roipoolLayer->tensorROIPoolSH.base, (vx_reference)input_rois, VXNNE_OPERATION_REFENRENCE_INPUT));
        vxmONERROR(vxnneOperation_AddReference(&roipoolLayer->tensorROIPoolSH.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

        vxmONERROR(vxnneLayer_SetOperation(
                 &roipoolLayer->base,
                 &roipoolLayer->tensorROIPoolSH.base,
                 0));
    }

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

OnError:
    return status;
}

VX_PRIVATE_API vx_bool vxoROIPoolLayer_TP_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_tensor  input_data                 = (vx_tensor)parameters[0];
    vx_tensor  input_rois                 = (vx_tensor)parameters[1];
    vx_tensor  outputs                    = (vx_tensor)parameters[6];

    vx_enum    roisFormat                 = TENSOR_DATA_TYPE(input_rois);

    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_TP, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    support = support && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_TP_ROI_POOLING) &&
                            vxnneIsTPSupportFormat(node->base.context, input_data, VX_NULL, outputs) &&
                            (roisFormat == VX_TYPE_FLOAT16 || roisFormat == VX_TYPE_BFLOAT16));

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoROIPoolLayer_TP_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vxnne_tensor_roipool_layer  roipoolLayer = (vxnne_tensor_roipool_layer)ops_layer;

    vx_tensor  input_data                 = (vx_tensor)parameters[0];
    vx_tensor  input_rois                 = (vx_tensor)parameters[1];

    vx_scalar  spatial_scales             = (vx_scalar)parameters[3];
    vx_scalar  pooled_heights             = (vx_scalar)parameters[4];
    vx_scalar  pooled_widths              = (vx_scalar)parameters[5];
    vx_tensor  outputs                    = (vx_tensor)parameters[6];
    vx_scalar  relu                       = (vx_scalar)parameters[7];

    vx_context context                    = vxGetContext((vx_reference)ops_layer->node);

    vx_float32 spatial_scale              = spatial_scales->value->f32;
    vx_uint32  pool_width                 = pooled_widths->value->u32;
    vx_uint32  pool_height                = pooled_heights->value->u32;
    vx_uint32  roi_stride                 = TENSOR_VIEW_SIZE_INDEX(input_rois, 2);
    vx_uint32  rois_num                   = TENSOR_VIEW_SIZE_INDEX(input_rois, 3);
    vx_uint32  batchCount                 = TENSOR_SIZE_INDEX(input_data, 3);

    vxnne_shader_executable shaderExecutable = VX_NULL;
    vx_uint32 size, maxpool, poolx, pooly, poolz;
    vx_op_param_s conv = {0};
    vx_tensor tmpTensor = VX_NULL;
    vx_tensor list = VX_NULL;
    vx_tensor split_end = VX_NULL;
    vx_tensor_create_params_t tensor_create_params;
    vx_uint32 core = context->nnConfig.fixedFeature.tpCoreCount;
    vx_bool mult = context->options.enableMultiTP && core > 1;
    vx_uint32 slice = !mult ? 1 : gcmMIN(TENSOR_VIEW_SIZE_INDEX(outputs, 3), core);
    vx_uint32 roi_size = TENSOR_VIEW_SIZE_INDEX(outputs, 3);
    vx_uint32 split_size_array[TP_TENSOR_COUNT] = {0};
    vx_uint32 split_offset_array[TP_TENSOR_COUNT] = {0};
    vx_uint32 splitEnds[TP_TENSOR_COUNT] = {0};
    vx_uint32 i = 0;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    calculateSplitSize(roi_size, slice, split_size_array, split_offset_array);

    splitEnds[0] = split_size_array[0] - 1;
    for (i = 1; i < slice; i++)
    {
        splitEnds[i] = splitEnds[i - 1] + split_size_array[i];
    }

    vxmONERROR(vxnneOperation_Initialize(&roipoolLayer->roipool_tp_operation[0].base,
                                        &roipoolLayer->base,
                                        VXNNE_OPERATION_TARGET_TP,
                                        VXNNE_OPERATOR_ROIPOOL,
                                        VX_NULL,
                                        vxnneOperation_TP_Deinitialize,
                                        batchCount,
                                        0));

    vxmONERROR(vxnneOperation_Initialize(&roipoolLayer->roipool_tp_operation[1].base,
                                        &roipoolLayer->base,
                                        VXNNE_OPERATION_TARGET_TP,
                                        VXNNE_OPERATOR_ROIPOOL,
                                        VX_NULL,
                                        vxnneOperation_TP_Deinitialize,
                                        batchCount,
                                        0));

    /* Prepare ROI intermediate output buffer. */
    maxpool = (TENSOR_VIEW_SIZE_INDEX(input_data, 0) + pool_width - 1) / pool_width;
    poolx = 1 << (vx_uint32) ceil(log(TENSOR_VIEW_SIZE_INDEX(input_data, 0)) / log(2));
    pooly = 1 << (vx_uint32) ceil(log(TENSOR_VIEW_SIZE_INDEX(input_data, 1)) / log(2));
    poolz = 1 << (vx_uint32) ceil(log(TENSOR_VIEW_SIZE_INDEX(input_data, 2)) / log(2));
    size = poolx * pooly * poolz * maxpool * TENSOR_DATA_SIZE(input_data);

    gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
    tensor_create_params.num_of_dims = 1;
    tensor_create_params.sizes = &size;
    tensor_create_params.data_format = TENSOR_DATA_TYPE(input_data);
    tensor_create_params.quant_format = TENSOR_QUANT_TYPE(input_data);
    if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
    {
        tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(input_data);
    }
    else
    {
        tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(input_data);
        tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(input_data);
    }

    tmpTensor = vxoTensor_CreateTensor(ops_layer->node->base.context, ops_layer->node->graph, &tensor_create_params, vx_true_e);
    if (tmpTensor == VX_NULL)
    {
        vxError("vxoTensor_CreateTensor fail at function %s, line %d", __FUNCTION__, __LINE__);
        status = VX_ERROR_NO_MEMORY;
        goto OnError;
    }

    conv.pad_x_left = 0;
    conv.pad_y_top = 0;
    conv.pool_size_x = 0;
    conv.pool_size_y = 0;
    conv.pool_stride = 1;
    conv.enable_relu = vx_false_e;
    conv.conv_rounding_type = 0;
    conv.pad_mode = VX_PAD_CONSTANT;
    conv.pad_const = 0;
    conv.tpType = TP_ROI_POOLING_STEP_1;
    conv.other_ref = (vx_reference)input_rois;
    conv.data_buff = gcvNULL;
    conv.tp_value = (vx_tp_value_cmd_s*)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
    conv.tp_value->u32[0] = pool_width;
    conv.tp_value->u32[1] = pool_height;
    conv.tp_value->f32[0] = spatial_scale;
    conv.tp_value->u32[2] = maxpool;
    conv.tp_value->u32[3] = poolx;
    conv.tp_value->u32[4] = pooly;
    conv.tp_value->u32[5] = poolz;
    conv.tp_value->u32[6] = TENSOR_VIEW_SIZE_INDEX(outputs, 3);
    conv.tp_value->e32[0] = 0;

    vxMemCopy(&roipoolLayer->roipool_tp_operation[0].base.parameter, &conv, sizeof(vx_op_param_s));

    vxmONERROR(vxnneLayer_SetOperation(
        &roipoolLayer->base,
        &roipoolLayer->roipool_tp_operation[0].base,
        0));

    roipoolLayer->roipool_tp_operation[0].input  = input_data;
    roipoolLayer->roipool_tp_operation[0].output = tmpTensor;
    vxmONERROR(vxnneOperation_AddReference(&roipoolLayer->roipool_tp_operation[0].base, (vx_reference)input_data, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&roipoolLayer->roipool_tp_operation[0].base, (vx_reference)tmpTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    /* Prepare ROI list. */
    num = TENSOR_VIEW_SIZE_INDEX(outputs, 3) * sizeof(vx_tp_roi_pool) / sizeof(vx_uint32) * 2;
    list = vxnneAllocateTPROIListBuffer(context, ops_layer->node, num, VX_TYPE_UINT16);
    if (list == VX_NULL)
    {
        status = VX_ERROR_NO_MEMORY;
        goto OnError;
    }
    /* Prepare Split end list. */
    split_end = vxnneAllocateTPROIListBuffer(context, ops_layer->node, slice, VX_TYPE_UINT32);
    if (split_end == VX_NULL)
    {
        status = VX_ERROR_NO_MEMORY;
        goto OnError;
    }

    vxnneInitROITensorFromBuffer(split_end, splitEnds, slice * sizeof(vx_uint32));

    shaderExecutable = vxnneROIRect2ROIListShaderExecutable(context, VXNNE_KERNEL_ROIRECT2ROILIST, &ops_layer->node->kernelAttributes.borderMode, input_rois, roi_stride, rois_num, pool_width, pool_height, spatial_scale, slice, split_end, list);
    if (!shaderExecutable)
    {
        status = VX_FAILURE;
        goto OnError;
    }

    vxmONERROR(vxnneShaderOperation_Initialize(&roipoolLayer->tensorROIPoolSH,
        &roipoolLayer->base,
        VXNNE_OPERATOR_PRETREATEDRECT,
        batchCount,
        shaderExecutable));

    vxmONERROR(vxnneOperation_AddReference(&roipoolLayer->tensorROIPoolSH.base, (vx_reference)input_rois, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&roipoolLayer->tensorROIPoolSH.base, (vx_reference)split_end, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&roipoolLayer->tensorROIPoolSH.base, (vx_reference)list, VXNNE_OPERATION_REFENRENCE_OUTPUT));
    vxmONERROR(vxnneLayer_SetOperation(
        &roipoolLayer->base,
        &roipoolLayer->tensorROIPoolSH.base,
        1));
    conv.tpType = TP_ROI_POOLING_STEP_2;
    conv.other_ref = (vx_reference)input_data;
    conv.data_buff = list;
    if (relu != VX_NULL)
        conv.enable_relu = relu->value->b;
    else
        conv.enable_relu = vx_false_e;
    conv.tp_value = (vx_tp_value_cmd_s*)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
    conv.tp_value->u32[0] = pool_width;
    conv.tp_value->u32[1] = pool_height;
    conv.tp_value->f32[0] = spatial_scale;
    conv.tp_value->u32[2] = maxpool;
    conv.tp_value->u32[3] = poolx;
    conv.tp_value->u32[4] = pooly;
    conv.tp_value->u32[5] = poolz;
    conv.tp_value->u32[6] = TENSOR_VIEW_SIZE_INDEX(outputs, 3);
    conv.tp_value->e32[0] = 1;

    vxMemCopy(&roipoolLayer->roipool_tp_operation[1].base.parameter, &conv, sizeof(vx_op_param_s));

    vxmONERROR(vxnneLayer_SetOperation(
        &roipoolLayer->base,
        &roipoolLayer->roipool_tp_operation[1].base,
        2));

    roipoolLayer->roipool_tp_operation[1].input  = tmpTensor;
    roipoolLayer->roipool_tp_operation[1].output = outputs;

    vxmONERROR(vxnneOperation_AddReference(&roipoolLayer->roipool_tp_operation[1].base, (vx_reference)tmpTensor, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&roipoolLayer->roipool_tp_operation[1].base, (vx_reference)list, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&roipoolLayer->roipool_tp_operation[1].base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    roipoolLayer->base.num_temp_tensors = 2;
    roipoolLayer->base.temp_tensors[0] = tmpTensor;
    roipoolLayer->base.temp_tensors[1] = split_end;

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_status vxoNNLayer_GetOperations(vxnne_layer ops_layer, vx_uint32_ptr max_num_operations, vxnne_operation **operations)
{
    vx_status  status = VX_SUCCESS;
    vxnne_tensor_roipool_layer  roipoolLayer = (vxnne_tensor_roipool_layer)ops_layer;

    *max_num_operations = gcmCOUNTOF(roipoolLayer->operations);

    *operations = roipoolLayer->operations;

    return status;
}

VX_PRIVATE_API vx_status vxnneROIPoolLayer_Initializer(vx_node    node, char* name, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vxnne_layer_imp_s registerROIPoolLayers[] = {/* Please DON'T adjust the order, it's importent */
        { "ROIPool NN", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "ROIPool TP", vxoROIPoolLayer_TP_Support, vxoROIPoolLayer_TP_Initialize, VX_NULL },
        { "ROIPool SH EVIS", vxoROIPoolLayer_SH_EVIS_Support, vxoROIPoolLayer_SH_EVIS_Initialize, VX_NULL },
        { "ROIPool SH F32", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "ROIPool SW", vxoNNCommon_Support, vxoROIPoolLayer_SW_Initialize, VX_NULL },
    };

    REGISTER_LAYERS(registerROIPoolLayers, vxnne_tensor_roipool_layer_s, name, vxoNNLayer_GetOperations);

OnError:
    return status;
}
#else

VX_PRIVATE_API vx_status vxnneROIPoolLayer_Initializer(
    vx_node    node,
    char*      name,
    vx_tensor  input_data,
    vx_tensor  input_rois,
    vx_scalar  pool_types,
    vx_scalar  spatial_scales,
    vx_scalar  pooled_heights,
    vx_scalar  pooled_widths,
    vx_tensor  outputs,
    vx_scalar  relu
    )
{
    vx_status  status                     = VX_SUCCESS;
    vx_context context                    = vxGetContext((vx_reference)node);

    vx_float32 spatial_scale              = spatial_scales->value->f32;
    vx_uint32  pool_width                 = pooled_widths->value->u32;
    vx_uint32  pool_height                = pooled_heights->value->u32;
    vx_bool    shExe_flag                 = vx_true_e;
    vx_uint32  width                      = TENSOR_VIEW_SIZE_INDEX(input_data, 0);
    vx_uint32  height                     = TENSOR_VIEW_SIZE_INDEX(input_data, 1);
    vx_uint32  depth                      = TENSOR_VIEW_SIZE_INDEX(input_data, 2);
    vx_uint32  roi_stride                 = TENSOR_VIEW_SIZE_INDEX(input_rois, 2);
    vx_uint32  rois_num                   = TENSOR_VIEW_SIZE_INDEX(input_rois, 3);
    vx_enum    inputFormat                = TENSOR_DATA_TYPE(input_data);
    vx_enum    roisFormat                 = TENSOR_DATA_TYPE(input_rois);
    vx_enum    outputFormat               = TENSOR_DATA_TYPE(outputs);
    vx_uint32  batchCount                 = TENSOR_SIZE_INDEX(input_data, 3);

    vx_tensor_create_params_t  tensor_create_params;
    vxnne_tensor_roipool_layer roipoolLayer=VX_NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    {
        gcoOS_Allocate(gcvNULL, sizeof(vxnne_tensor_roipool_layer_s), (gctPOINTER*)&roipoolLayer);
        if (!roipoolLayer)
        {
            status = VX_ERROR_NO_MEMORY;
            vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            goto exit;
        }


        gcoOS_ZeroMemory(roipoolLayer, sizeof(vxnne_tensor_roipool_layer_s));

        vxnneLayer_Initialize(&roipoolLayer->base,
            name,
            node,
            vxmOPERATION_COUNT(roipoolLayer),
            roipoolLayer->operations,
            VX_NULL);

        if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_ROI_POOLING) &&
            vxnneIsTPSupportFormat(context, input_data, VX_NULL, outputs) &&
            (roisFormat == VX_TYPE_FLOAT16 || roisFormat == VX_TYPE_BFLOAT16))
        {
            vxnne_shader_executable shaderExecutable = VX_NULL;
            vx_uint32 num, size, maxpool, poolx, pooly, poolz;
            vx_op_param_s conv = {0};
            vx_tensor tmpTensor = VX_NULL;
            vx_tensor list = VX_NULL;
            vx_tensor split_end = VX_NULL;
            vx_tensor_create_params_t tensor_create_params;
            vx_uint32 core = context->nnConfig.fixedFeature.tpCoreCount;
            vx_bool mult = context->options.enableMultiTP && core > 1;
            vx_uint32 slice = !mult ? 1 : gcmMIN(TENSOR_VIEW_SIZE_INDEX(outputs, 3), core);
            vx_uint32 roi_size = TENSOR_VIEW_SIZE_INDEX(outputs, 3);
            vx_uint32 split_size_array[TP_TENSOR_COUNT] = {0};
            vx_uint32 split_offset_array[TP_TENSOR_COUNT] = {0};
            vx_uint32 splitEnds[TP_TENSOR_COUNT] = {0};
            vx_uint32 i = 0;

            calculateSplitSize(roi_size, slice, split_size_array, split_offset_array);

            splitEnds[0] = split_size_array[0] - 1;
            for (i = 1; i < slice; i++)
            {
                splitEnds[i] = splitEnds[i - 1] + split_size_array[i];
            }

            status = vxnneOperation_Initialize(&roipoolLayer->roipool_tp_operation[0].base,
                                                &roipoolLayer->base,
                                                VXNNE_OPERATION_TARGET_TP,
                                                VXNNE_OPERATOR_ROIPOOL,
                                                VX_NULL,
                                                vxnneOperation_TP_Deinitialize,
                                                batchCount,
                                                0);
            if (status != VX_SUCCESS) goto exit;

            status = vxnneOperation_Initialize(&roipoolLayer->roipool_tp_operation[1].base,
                                                &roipoolLayer->base,
                                                VXNNE_OPERATION_TARGET_TP,
                                                VXNNE_OPERATOR_ROIPOOL,
                                                VX_NULL,
                                                vxnneOperation_TP_Deinitialize,
                                                batchCount,
                                                0);
            if (status != VX_SUCCESS) goto exit;

            /* Prepare ROI intermediate output buffer. */
            maxpool = (TENSOR_VIEW_SIZE_INDEX(input_data, 0) + pool_width - 1) / pool_width;
            poolx = 1 << (vx_uint32) ceil(log(TENSOR_VIEW_SIZE_INDEX(input_data, 0)) / log(2));
            pooly = 1 << (vx_uint32) ceil(log(TENSOR_VIEW_SIZE_INDEX(input_data, 1)) / log(2));
            poolz = 1 << (vx_uint32) ceil(log(TENSOR_VIEW_SIZE_INDEX(input_data, 2)) / log(2));
            size = poolx * pooly * poolz * maxpool * TENSOR_DATA_SIZE(input_data);

            gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
            tensor_create_params.num_of_dims = 1;
            tensor_create_params.sizes = &size;
            tensor_create_params.data_format = TENSOR_DATA_TYPE(input_data);
            tensor_create_params.quant_format = TENSOR_QUANT_TYPE(input_data);
            if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
            {
                tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(input_data);
            }
            else
            {
                tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(input_data);
                tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(input_data);
            }

            tmpTensor = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);
            if (tmpTensor == VX_NULL)
            {
                vxError("vxoTensor_CreateTensor fail at function %s, line %d", __FUNCTION__, __LINE__);
                status = VX_ERROR_NO_MEMORY;
                goto exit;
            }

            conv.pad_x_left = 0;
            conv.pad_y_top = 0;
            conv.pool_size_x = 0;
            conv.pool_size_y = 0;
            conv.pool_stride = 1;
            conv.enable_relu = vx_false_e;
            conv.conv_rounding_type = 0;
            conv.pad_mode = VX_PAD_CONSTANT;
            conv.pad_const = 0;
            conv.tpType = TP_ROI_POOLING_STEP_1;
            conv.other_ref = (vx_reference)input_rois;
            conv.data_buff = gcvNULL;
            conv.tp_value = (vx_tp_value_cmd_s*)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
            conv.tp_value->u32[0] = pool_width;
            conv.tp_value->u32[1] = pool_height;
            conv.tp_value->f32[0] = spatial_scale;
            conv.tp_value->u32[2] = maxpool;
            conv.tp_value->u32[3] = poolx;
            conv.tp_value->u32[4] = pooly;
            conv.tp_value->u32[5] = poolz;
            conv.tp_value->u32[6] = TENSOR_VIEW_SIZE_INDEX(outputs, 3);
            conv.tp_value->e32[0] = 0;

            vxMemCopy(&roipoolLayer->roipool_tp_operation[0].base.parameter, &conv, sizeof(vx_op_param_s));

            vxnneLayer_SetOperation(
                &roipoolLayer->base,
                &roipoolLayer->roipool_tp_operation[0].base,
                0);

            roipoolLayer->roipool_tp_operation[0].input  = input_data;
            roipoolLayer->roipool_tp_operation[0].output = tmpTensor;
            vxnneOperation_AddReference(&roipoolLayer->roipool_tp_operation[0].base, (vx_reference)input_data, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&roipoolLayer->roipool_tp_operation[0].base, (vx_reference)tmpTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            /* Prepare ROI list. */
            num = TENSOR_VIEW_SIZE_INDEX(outputs, 3) * sizeof(vx_tp_roi_pool) / sizeof(vx_uint32) * 2;
            list = vxnneAllocateTPROIListBuffer(context, node, num, VX_TYPE_UINT16);
            if (list == VX_NULL)
            {
                status = VX_ERROR_NO_MEMORY;
                goto exit;
            }
            /* Prepare Split end list. */
            split_end = vxnneAllocateTPROIListBuffer(context, node, slice, VX_TYPE_UINT32);
            if (split_end == VX_NULL)
            {
                status = VX_ERROR_NO_MEMORY;
                goto exit;
            }

            vxnneInitROITensorFromBuffer(split_end, splitEnds, slice * sizeof(vx_uint32));

            shaderExecutable = vxnneROIRect2ROIListShaderExecutable(node->base.context, VXNNE_KERNEL_ROIRECT2ROILIST, &node->kernelAttributes.borderMode, input_rois, roi_stride, rois_num, pool_width, pool_height, spatial_scale, slice, split_end, list);
            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto exit;
            }

            status = vxnneShaderOperation_Initialize(&roipoolLayer->tensorROIPoolSH,
                &roipoolLayer->base,
                VXNNE_OPERATOR_PRETREATEDRECT,
                batchCount,
                shaderExecutable);

            vxnneOperation_AddReference(&roipoolLayer->tensorROIPoolSH.base, (vx_reference)input_rois, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&roipoolLayer->tensorROIPoolSH.base, (vx_reference)split_end, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&roipoolLayer->tensorROIPoolSH.base, (vx_reference)list, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            vxnneLayer_SetOperation(
                &roipoolLayer->base,
                &roipoolLayer->tensorROIPoolSH.base,
                1);
            conv.tpType = TP_ROI_POOLING_STEP_2;
            conv.other_ref = (vx_reference)input_data;
            conv.data_buff = list;
            if (relu != VX_NULL)
                conv.enable_relu = relu->value->b;
            else
                conv.enable_relu = vx_false_e;
            conv.tp_value = (vx_tp_value_cmd_s*)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
            conv.tp_value->u32[0] = pool_width;
            conv.tp_value->u32[1] = pool_height;
            conv.tp_value->f32[0] = spatial_scale;
            conv.tp_value->u32[2] = maxpool;
            conv.tp_value->u32[3] = poolx;
            conv.tp_value->u32[4] = pooly;
            conv.tp_value->u32[5] = poolz;
            conv.tp_value->u32[6] = TENSOR_VIEW_SIZE_INDEX(outputs, 3);
            conv.tp_value->e32[0] = 1;

            vxMemCopy(&roipoolLayer->roipool_tp_operation[1].base.parameter, &conv, sizeof(vx_op_param_s));

            vxnneLayer_SetOperation(
                &roipoolLayer->base,
                &roipoolLayer->roipool_tp_operation[1].base,
                2);

            roipoolLayer->roipool_tp_operation[1].input  = tmpTensor;
            roipoolLayer->roipool_tp_operation[1].output = outputs;

            vxnneOperation_AddReference(&roipoolLayer->roipool_tp_operation[1].base, (vx_reference)tmpTensor, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&roipoolLayer->roipool_tp_operation[1].base, (vx_reference)list, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&roipoolLayer->roipool_tp_operation[1].base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            roipoolLayer->base.num_temp_tensors = 2;
            roipoolLayer->base.temp_tensors[0] = tmpTensor;
            roipoolLayer->base.temp_tensors[1] = split_end;
            node->layer = &roipoolLayer->base;
        }
        else
        {
            vx_bool    shExe_flag0  = (vx_bool)(width == 20 && height == 16 && roi_stride == 5 && pool_width == 6 && pool_height == 6 && inputFormat == VX_TYPE_FLOAT16 && roisFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16);
            vx_bool    shExe_flag1  = (vx_bool)(width == 51 && height == 39 && roi_stride == 5 && pool_width == 6 && pool_height == 6 && inputFormat == VX_TYPE_FLOAT16 && roisFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16);
            vx_bool    shExe_flag2  = (vx_bool)(width == 51 && height == 39 && roi_stride == 5 && pool_width == 6 && pool_height == 6 && inputFormat == VX_TYPE_INT8 && roisFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT8);
            vx_bool    shExe_flag3  = (vx_bool)(width == 20 && height == 16 && roi_stride == 5 && pool_width == 6 && pool_height == 6 && inputFormat == VX_TYPE_INT8 && roisFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT8);
            vx_bool    shExe_flag4  = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && roisFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) /*||
                                                (inputFormat == VX_TYPE_INT16 && roisFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT16)||
                                                (inputFormat == VX_TYPE_UINT8 && roisFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_UINT8)*/);

            shExe_flag =  shExe_flag2 || shExe_flag3 || shExe_flag4;

            if (shExe_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
            {
                vxnne_shader_executable shaderExecutable = VX_NULL;
                vx_bool enable_relu = relu ? relu->value->b : vx_false_e;

                if (shExe_flag0 || shExe_flag1 || shExe_flag2 || shExe_flag3)
                {
                    vx_uint32 imgNum                = width == 51 ? 7 : 4;
                    vx_uint32 dims                  = 3;
                    vx_uint32 tmp_sizes0[3]         = {width, height, depth * imgNum };
                    vx_uint32 tmp_sizes1[3]         = {84, rois_num, 1 };
                    vx_uint32 outputs_dims          = outputs->dimCount;
                    vx_tensor outputs_reshp         = NULL;
                    vx_tensor vertMaxPoolTensor, preTreatedRectTensor;

                    gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
                    tensor_create_params.num_of_dims = dims;
                    tensor_create_params.sizes = tmp_sizes0;
                    tensor_create_params.data_format = TENSOR_DATA_TYPE(outputs);
                    tensor_create_params.quant_format = TENSOR_QUANT_TYPE(outputs);
                    if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
                    {
                        tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(outputs);
                    }
                    else
                    {
                        tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(outputs);
                        tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(outputs);
                    }

                    vertMaxPoolTensor = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);
                    if (vertMaxPoolTensor == VX_NULL)
                    {
                        vxError("vxoTensor_CreateTensor fail at function %s, line %d", __FUNCTION__, __LINE__);
                        status = VX_ERROR_NO_MEMORY;
                        goto exit;
                    }

                    tensor_create_params.data_format = TENSOR_DATA_TYPE(input_rois);
                    tensor_create_params.quant_format = TENSOR_QUANT_TYPE(input_rois);
                    if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
                    {
                        tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(input_rois);
                    }
                    else
                    {
                        tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(input_rois);
                        tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(input_rois);
                    }
                    tensor_create_params.sizes = tmp_sizes1;
                    preTreatedRectTensor = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);
                    if (preTreatedRectTensor == VX_NULL)
                    {
                        vxError("vxoTensor_CreateTensor fail at function %s, line %d", __FUNCTION__, __LINE__);
                        status = VX_ERROR_NO_MEMORY;
                        goto exit;
                    }

                    //operation1:vertMaxPool
                    shaderExecutable = vxnneVertMaxPoolShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_VERTMAXPOOL, &node->kernelAttributes.borderMode, input_data, pool_width, pool_height, enable_relu, vertMaxPoolTensor);

                    if (!shaderExecutable)
                    {
                        status = VX_FAILURE;
                        goto exit;
                    }
                    status = vxnneShaderOperation_Initialize(&roipoolLayer->vertmaxpool_operation.vertmaxpool_SHoperation,
                        &roipoolLayer->base,
                        VXNNE_OPERATOR_VERTMAXPOOL,
                        batchCount,
                        shaderExecutable);

                    if (status != VX_SUCCESS)
                        goto exit;

                    vxnneOperation_AddReference(&roipoolLayer->vertmaxpool_operation.vertmaxpool_SHoperation.base, (vx_reference)input_data, VXNNE_OPERATION_REFENRENCE_INPUT);
                    vxnneOperation_AddReference(&roipoolLayer->vertmaxpool_operation.vertmaxpool_SHoperation.base, (vx_reference)vertMaxPoolTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);

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
                        batchCount,
                        shaderExecutable);

                    if (status != VX_SUCCESS)
                        goto exit;

                    vxnneOperation_AddReference(&roipoolLayer->pretreatedrect_operation.pretreatedrect_SHoperation.base, (vx_reference)input_rois, VXNNE_OPERATION_REFENRENCE_INPUT);
                    vxnneOperation_AddReference(&roipoolLayer->pretreatedrect_operation.pretreatedrect_SHoperation.base, (vx_reference)preTreatedRectTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                    //operation3:horzMaxPool
                    if(outputs_dims == 4)
                    {
                        vx_int32 new_size[3] = {pool_width * pool_height, depth, rois_num};
                        outputs_dims = 3;
                        outputs_reshp = vxoTensor_ReshapeTensor(outputs, new_size, outputs_dims);
                        TENSOR_POS(outputs_reshp) = TENSOR_POS(outputs);
                    }

                    TENSOR_POS(vertMaxPoolTensor) = TENSOR_POS(input_data);
                    if(outputs_reshp)
                    {
                        shaderExecutable = vxnneHorzMaxPoolShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_HORZMAXPOOL, &node->kernelAttributes.borderMode, vertMaxPoolTensor, preTreatedRectTensor, outputs_reshp);
                        vxoTensor_ReleaseTensor(&outputs_reshp);
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
                        batchCount,
                        shaderExecutable);

                    if (status != VX_SUCCESS)
                        goto exit;

                    vxnneOperation_AddReference(&roipoolLayer->horzmaxpool_operation.horzmaxpool_SHoperation.base, (vx_reference)vertMaxPoolTensor, VXNNE_OPERATION_REFENRENCE_INPUT);
                    vxnneOperation_AddReference(&roipoolLayer->horzmaxpool_operation.horzmaxpool_SHoperation.base, (vx_reference)preTreatedRectTensor, VXNNE_OPERATION_REFENRENCE_INPUT);
                    vxnneOperation_AddReference(&roipoolLayer->horzmaxpool_operation.horzmaxpool_SHoperation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                    vxnneLayer_SetOperation(
                        &roipoolLayer->base,
                        &roipoolLayer->vertmaxpool_operation.vertmaxpool_SHoperation.base,
                        0);
                    vxnneLayer_SetOperation(
                        &roipoolLayer->base,
                        &roipoolLayer->pretreatedrect_operation.pretreatedrect_SHoperation.base,
                        1);
                    vxnneLayer_SetOperation(
                        &roipoolLayer->base,
                        &roipoolLayer->horzmaxpool_operation.horzmaxpool_SHoperation.base,
                        2);

                    roipoolLayer->base.num_temp_tensors                  = 2;
                    roipoolLayer->base.temp_tensors[0] = vertMaxPoolTensor;
                    roipoolLayer->base.temp_tensors[1] = preTreatedRectTensor;
                }
                else if (shExe_flag4)
                {
                    shaderExecutable = vxnneROIPoolShaderExecutable(node->base.context, VXNNE_KERNEL_ROIPOOL, &node->kernelAttributes.borderMode, input_data, input_rois, pool_width, pool_height, spatial_scale, enable_relu, outputs);

                    if (!shaderExecutable)
                    {
                        status = VX_FAILURE;
                        goto exit;
                    }
                    status = vxnneShaderOperation_Initialize(&roipoolLayer->tensorROIPoolSH,
                        &roipoolLayer->base,
                        VXNNE_OPERATOR_ROIPOOL,
                        batchCount,
                        shaderExecutable);

                    if (status != VX_SUCCESS)
                        goto exit;

                    vxnneOperation_AddReference(&roipoolLayer->tensorROIPoolSH.base, (vx_reference)input_data, VXNNE_OPERATION_REFENRENCE_INPUT);
                    vxnneOperation_AddReference(&roipoolLayer->tensorROIPoolSH.base, (vx_reference)input_rois, VXNNE_OPERATION_REFENRENCE_INPUT);
                    vxnneOperation_AddReference(&roipoolLayer->tensorROIPoolSH.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                    vxnneLayer_SetOperation(
                             &roipoolLayer->base,
                             &roipoolLayer->tensorROIPoolSH.base,
                             0);
                }

                node->layer = &roipoolLayer->base;
            }
            else
            {
                vxnneOperation_Initialize(&roipoolLayer->tensorROIPoolSW.base,
                    &roipoolLayer->base,
                    VXNNE_OPERATION_TARGET_SW,
                    VXNNE_OPERATOR_ROIPOOL,
                    vxnneExecuteSWROIPooling,
                    VX_NULL,
                    batchCount,
                    0);

                vxnneLayer_SetOperation(
                         &roipoolLayer->base,
                         &roipoolLayer->tensorROIPoolSW.base,
                         0);

                roipoolLayer->tensorROIPoolSW.input_data      = input_data;
                roipoolLayer->tensorROIPoolSW.input_rois      = input_rois;
                roipoolLayer->tensorROIPoolSW.pool_type       = pool_types;
                roipoolLayer->tensorROIPoolSW.pooled_height   = pooled_heights;
                roipoolLayer->tensorROIPoolSW.pooled_width    = pooled_widths;
                roipoolLayer->tensorROIPoolSW.spatial_scale   = spatial_scales;
                roipoolLayer->tensorROIPoolSW.output          = outputs;
                roipoolLayer->tensorROIPoolSW.relu            = relu;

                vxnneOperation_AddReference(&roipoolLayer->tensorROIPoolSW.base, (vx_reference)input_data, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&roipoolLayer->tensorROIPoolSW.base, (vx_reference)input_rois, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&roipoolLayer->tensorROIPoolSW.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
                node->layer = &roipoolLayer->base;
            }
        }
    }
    return status;

exit:
    if(roipoolLayer) gcoOS_Free(NULL, (gctPOINTER)roipoolLayer);
    return status;
}
#endif

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
#if REGISTER_FRAME
    vx_reference params[] = {
        parameters[0], parameters[1], parameters[2], parameters[3], parameters[4],
        parameters[5], parameters[6], VX_NULL,
    };

    return vxnneROIPoolLayer_Initializer(node, "ROIPoolLayer", params, gcmCOUNTOF(params));
#else
    vx_tensor  input_data                 = (vx_tensor)parameters[0];
    vx_tensor  input_rois                 = (vx_tensor)parameters[1];
    vx_scalar  pool_types                 = (vx_scalar)parameters[2];
    vx_scalar  spatial_scales             = (vx_scalar)parameters[3];
    vx_scalar  pooled_heights             = (vx_scalar)parameters[4];
    vx_scalar  pooled_widths              = (vx_scalar)parameters[5];
    vx_tensor  outputs                    = (vx_tensor)parameters[6];

    return vxnneROIPoolLayer_Initializer(
        node,
        "ROIPoolLayer",
        input_data,
        input_rois,
        pool_types,
        spatial_scales,
        pooled_heights,
        pooled_widths,
        outputs,
        VX_NULL
        );
#endif


}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_NNROIPoolReluLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolReluLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolReluLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolReluLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
#if REGISTER_FRAME
    return vxnneROIPoolLayer_Initializer(node, "ROIPoolLayer", parameters, num);
#else
    vx_tensor  input_data                 = (vx_tensor)parameters[0];
    vx_tensor  input_rois                 = (vx_tensor)parameters[1];
    vx_scalar  pool_types                 = (vx_scalar)parameters[2];
    vx_scalar  spatial_scales             = (vx_scalar)parameters[3];
    vx_scalar  pooled_heights             = (vx_scalar)parameters[4];
    vx_scalar  pooled_widths              = (vx_scalar)parameters[5];
    vx_tensor  outputs                    = (vx_tensor)parameters[6];
    vx_scalar  relu                       = (vx_scalar)parameters[7];

    return vxnneROIPoolLayer_Initializer(
        node,
        "ROIPoolReluLayer",
        input_data,
        input_rois,
        pool_types,
        spatial_scales,
        pooled_heights,
        pooled_widths,
        outputs,
        relu
        );
#endif
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolReluLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

