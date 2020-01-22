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
#include <layers/gc_vx_layer_tensor_stride_slice.h>


/***************************************************************************************************************************
*                                                 TENSOR STRIDE SLICE
***************************************************************************************************************************/

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorStrideSlice(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorStrideSlice_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorStrideSlice_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_int32 vxoNNTensorStrideSlice_GetAxisValue(vx_int32 value, vx_uint32 dimension_size)
{
    vx_int32 axis_vlaue = 0;
    if (value < 0)
        axis_vlaue = value + dimension_size;
    else
        axis_vlaue = value;
    return axis_vlaue;
}

VX_PRIVATE_API vx_int32 vxoNNTensorStrideSlice_MaskStartValue(vx_int32 stride, vx_uint32 dimension_size)
{
    vx_int32 start_vlaue = 0;
    if (stride > 0)
        start_vlaue = 0;
    else
        start_vlaue = dimension_size - 1;
    return start_vlaue;
}

VX_PRIVATE_API vx_int32 vxoNNTensorStrideSlice_MaskStopValue(vx_int32 stride, vx_uint32 dimension_size)
{
    vx_int32 stop_vlaue = 0;
    if (stride > 0)
        stop_vlaue = dimension_size;
    else
        stop_vlaue = -1;
    return stop_vlaue;
}

VX_PRIVATE_API vx_int32 vxoNNTensorStrideSlice_ClampStop(vx_int32 stride, vx_int32 stop, vx_uint32 dimension_size)
{
    vx_int32 stop_vlaue = 0;
    if (stride > 0)
    {
        stop_vlaue = gcmCLAMP(stop, 0, (vx_int32)dimension_size);
    }
    else
    {
        stop_vlaue = gcmCLAMP(stop, -1, (vx_int32)dimension_size - 1);
    }
    return stop_vlaue;
}

VX_PRIVATE_API vx_status vxoNNTensorStrideSlice_getStartStopStride(vx_tensor input, vx_tensor begin_dims, vx_tensor end_dims, vx_tensor stride_dims, vx_scalar begin_mask, vx_scalar end_mask, vx_scalar shrink_axis_mask, vx_int32 start[4], vx_int32 stop[4], vx_int32 stride[4])
{
    vx_uint8_ptr begin_dims_base = VX_NULL, end_dims_base = VX_NULL, stride_dims_base = VX_NULL;

    vx_uint32 i = 0;
    vx_int32 int32_value = 0;
    vx_uint32 srcIdx = 0;

    vxoTensor_GetTensorViewMemory(begin_dims, (gctPOINTER*)&begin_dims_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(end_dims, (gctPOINTER*)&end_dims_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(stride_dims, (gctPOINTER*)&stride_dims_base, VX_NULL);

    for (i = 0; i < 4; i ++)
    {
        start[i]    = 0;
        stop[i]     = 1;
        stride[i]   = 1;
    }

    for (i = 0; i < TENSOR_VIEW_SIZE_INDEX(stride_dims, 0); ++i)
    {
        int32_value = (vx_int32)vxnneGetDataExt(TENSOR_DATA_TYPE(stride_dims), TENSOR_QUANT_TYPE(stride_dims), srcIdx++, stride_dims_base,
            TENSOR_POS(stride_dims), TENSOR_TF_ZEROPOINT(stride_dims), TENSOR_TF_SCALE(stride_dims));

        stride[i] = int32_value;
    }

    srcIdx = 0;
    for (i = 0; i < TENSOR_VIEW_SIZE_INDEX(begin_dims, 0); ++i)
    {
        int32_value = (vx_int32)vxnneGetDataExt(TENSOR_DATA_TYPE(begin_dims), TENSOR_QUANT_TYPE(begin_dims), srcIdx++, begin_dims_base,
            TENSOR_POS(begin_dims), TENSOR_TF_ZEROPOINT(begin_dims), TENSOR_TF_SCALE(begin_dims));

        start[i] = vxoNNTensorStrideSlice_GetAxisValue(int32_value, TENSOR_VIEW_SIZE_INDEX(input, i));
    }

    srcIdx = 0;
    for (i = 0; i < TENSOR_VIEW_SIZE_INDEX(end_dims, 0); ++i)
    {
        int32_value = (vx_int32)vxnneGetDataExt(TENSOR_DATA_TYPE(end_dims), TENSOR_QUANT_TYPE(end_dims), srcIdx++, end_dims_base,
            TENSOR_POS(end_dims), TENSOR_TF_ZEROPOINT(end_dims), TENSOR_TF_SCALE(end_dims));

        stop[i] = vxoNNTensorStrideSlice_GetAxisValue(int32_value, TENSOR_VIEW_SIZE_INDEX(input, i));
    }

    /*if the ith bit of mask is set, the start or stop will be the fullest possible range in that dimension.*/
    for (i = 0; i < 4; i ++)
    {
        if (begin_mask->value->n32 & (1 << i))
        {
            start[i] = vxoNNTensorStrideSlice_MaskStartValue(stride[i], TENSOR_VIEW_SIZE_INDEX(input, i));
        }

        start[i] = gcmCLAMP(start[i], 0, (vx_int32)(TENSOR_VIEW_SIZE_INDEX(input, i) - 1));

        if (shrink_axis_mask->value->n32 & (1 << i))
        {
            stop[i] = start[i] + 1;
        }

        if (end_mask->value->n32 & (1 << i))
        {
            stop[i] = vxoNNTensorStrideSlice_MaskStopValue(stride[i], TENSOR_VIEW_SIZE_INDEX(input, i));
        }

        stop[i] = vxoNNTensorStrideSlice_ClampStop(stride[i], stop[i], TENSOR_VIEW_SIZE_INDEX(input, i));
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoNNTensorStrideSlice_getReverseAxis(vx_int32 start[4], vx_int32 stop[4], vx_int32 stride[4], vx_uint32 reverseAxis[4], vx_uint32 *numOfAxis, vx_uint32 *tensor_size)
{
    vx_uint32 i   = 0;
    vx_uint32 idx = 0;

    for (i = 0; i < 4; i ++)
    {
        reverseAxis[idx] = 0xcdcdcdcd;

        if (stride[i] < 0 && gcmABS(stop[i] - start[i]) > 1)
        {
            vx_uint32 start_axis    = tensor_size[i] - start[i] - 1;
            vx_uint32 stop_axis     = tensor_size[i] - stop[i] - 2;

            stride[i]   = abs(stride[i]);
            start[i]    = start_axis;
            stop[i]     = stop_axis;

            reverseAxis[idx] = i;
            idx++;
        }
    }

    *numOfAxis = idx;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxnneExecuteSWTensorStrideSlice(struct _vxnne_operation_s *operation)
{
    vxnne_tensor_stride_slice_operation transOperation = (vxnne_tensor_stride_slice_operation)operation;
    vx_tensor input = (vx_tensor)transOperation->input;
    vx_tensor begin_dims = (vx_tensor)transOperation->begin_dims;
    vx_tensor end_dims = (vx_tensor)transOperation->end_dims;
    vx_tensor stride_dims = (vx_tensor)transOperation->strides;
    vx_scalar begin_mask = (vx_scalar)transOperation->begin_mask;
    vx_scalar end_mask = (vx_scalar)transOperation->end_mask;
    vx_scalar shrink_axis_mask = (vx_scalar)transOperation->shrink_axis_mask;
    vx_tensor output = (vx_tensor)transOperation->output;
    vx_uint8_ptr input_base = VX_NULL, output_base = VX_NULL, begin_dims_base = VX_NULL, end_dims_base = VX_NULL, stride_dims_base = VX_NULL;

    vx_uint32 i = 0;
    vx_int32 int32_value = 0;
    vx_float32 float32_value = 0;
    vx_uint32 srcIdx = 0, dstIdx = 0;

    vx_int32 start_w = 0, start_h = 0, start_c = 0, start_n = 0;
    vx_int32 stop_w = 1, stop_h = 1, stop_c = 1, stop_n = 1;
    vx_int32 stride_w = 1, stride_h = 1, stride_c = 1, stride_n = 1;
    vx_int32 in_w = 0, in_h = 0, in_c = 0, in_n = 0;

    vx_uint32 shrink_axis_w = shrink_axis_mask->value->n32 & (1 << 0);
    vx_uint32 shrink_axis_h = shrink_axis_mask->value->n32 & (1 << 1);
    vx_uint32 shrink_axis_c = shrink_axis_mask->value->n32 & (1 << 2);
    vx_uint32 shrink_axis_n = shrink_axis_mask->value->n32 & (1 << 3);

    vxoTensor_GetTensorViewMemory(begin_dims, (gctPOINTER*)&begin_dims_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(end_dims, (gctPOINTER*)&end_dims_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(stride_dims, (gctPOINTER*)&stride_dims_base, VX_NULL);

    for (i = 0; i < stride_dims->dims[0]; ++i)
    {
        int32_value = (vx_int32)vxnneGetDataExt(TENSOR_DATA_TYPE(stride_dims), TENSOR_QUANT_TYPE(stride_dims),
            srcIdx++, stride_dims_base, TENSOR_POS(stride_dims), TENSOR_TF_ZEROPOINT(stride_dims), TENSOR_TF_SCALE(stride_dims));
        if (i == 0)
            stride_w = int32_value;
        if (i == 1)
            stride_h = int32_value;
        if (i == 2)
            stride_c = int32_value;
        if (i == 3)
            stride_n = int32_value;
    }

    srcIdx = 0;
    for (i = 0; i < begin_dims->dims[0]; ++i)
    {
        int32_value = (vx_int32)vxnneGetDataExt(TENSOR_DATA_TYPE(begin_dims), TENSOR_QUANT_TYPE(begin_dims),
            srcIdx++, begin_dims_base, TENSOR_POS(begin_dims), TENSOR_TF_ZEROPOINT(begin_dims), TENSOR_TF_SCALE(begin_dims));
        if (i == 0)
        {
            start_w = vxoNNTensorStrideSlice_GetAxisValue(int32_value, input->dims[0]);
        }

        if (i == 1)
        {
            start_h = vxoNNTensorStrideSlice_GetAxisValue(int32_value, input->dims[1]);
        }

        if (i == 2)
        {
            start_c = vxoNNTensorStrideSlice_GetAxisValue(int32_value, input->dims[2]);
        }

        if (i == 3)
        {
            start_n = vxoNNTensorStrideSlice_GetAxisValue(int32_value, input->dims[3]);
        }
    }

    srcIdx = 0;
    for (i = 0; i < end_dims->dims[0]; ++i)
    {
        int32_value = (vx_int32)vxnneGetDataExt(TENSOR_DATA_TYPE(end_dims), TENSOR_QUANT_TYPE(end_dims),
            srcIdx++, end_dims_base, TENSOR_POS(end_dims), TENSOR_TF_ZEROPOINT(end_dims), TENSOR_TF_SCALE(end_dims));
        if (i == 0)
        {
            stop_w = vxoNNTensorStrideSlice_GetAxisValue(int32_value, input->dims[0]);
        }

        if (i == 1)
        {
            stop_h = vxoNNTensorStrideSlice_GetAxisValue(int32_value, input->dims[1]);
        }

        if (i == 2)
        {
            stop_c = vxoNNTensorStrideSlice_GetAxisValue(int32_value, input->dims[2]);
        }

        if (i == 3)
        {
            stop_n = vxoNNTensorStrideSlice_GetAxisValue(int32_value, input->dims[3]);
        }
    }

    /*if the ith bit of mask is set, the start or stop will be the fullest possible range in that dimension.*/
    if (begin_mask->value->n32 & 1 << 0)
    {
        start_w = vxoNNTensorStrideSlice_MaskStartValue(stride_w, input->dims[0]);
    }

    if (begin_mask->value->n32 & 1 << 1)
    {
        start_h = vxoNNTensorStrideSlice_MaskStartValue(stride_h, input->dims[1]);
    }

    if (begin_mask->value->n32 & 1 << 2)
    {
        start_c = vxoNNTensorStrideSlice_MaskStartValue(stride_c, input->dims[2]);
    }

    if (begin_mask->value->n32 & 1 << 3)
    {
        start_n = vxoNNTensorStrideSlice_MaskStartValue(stride_n, input->dims[3]);
    }

    start_w = gcmCLAMP(start_w, 0, (vx_int32)(input->dims[0] - 1));
    start_h = gcmCLAMP(start_h, 0, (vx_int32)(input->dims[1] - 1));
    start_c = gcmCLAMP(start_c, 0, (vx_int32)(input->dims[2] - 1));
    start_n = gcmCLAMP(start_n, 0, (vx_int32)(input->dims[3] - 1));

    if (shrink_axis_w)
    {
        stop_w = start_w + 1;
    }

    if (shrink_axis_h)
    {
        stop_h = start_h + 1;
    }

    if (shrink_axis_c)
    {
        stop_c = start_c + 1;
    }

    if (shrink_axis_n)
    {
        stop_n = start_n + 1;
    }

    if (end_mask->value->n32 & (1 << 0))
    {
        stop_w = vxoNNTensorStrideSlice_MaskStopValue(stride_w, input->dims[0]);
    }

    if (end_mask->value->n32 & (1 << 1))
    {
        stop_h = vxoNNTensorStrideSlice_MaskStopValue(stride_h, input->dims[1]);
    }

    if (end_mask->value->n32 & (1 << 2))
    {
        stop_c = vxoNNTensorStrideSlice_MaskStopValue(stride_c, input->dims[2]);
    }

    if (end_mask->value->n32 & (1 << 3))
    {
        stop_n = vxoNNTensorStrideSlice_MaskStopValue(stride_n, input->dims[3]);
    }

    stop_w = vxoNNTensorStrideSlice_ClampStop(stride_w, stop_w, input->dims[0]);
    stop_h = vxoNNTensorStrideSlice_ClampStop(stride_h, stop_h, input->dims[1]);
    stop_c = vxoNNTensorStrideSlice_ClampStop(stride_c, stop_c, input->dims[2]);
    stop_n = vxoNNTensorStrideSlice_ClampStop(stride_n, stop_n, input->dims[3]);

    vxoTensor_GetTensorViewMemory(input, (gctPOINTER*)&input_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(output, (gctPOINTER*)&output_base, VX_NULL);

    for (in_n = start_n; !((stride_n > 0) ? (in_n >= stop_n) : (in_n <= stop_n)); in_n += stride_n)
    {
        for (in_c = start_c; !((stride_c > 0) ? (in_c >= stop_c) : (in_c <= stop_c)); in_c += stride_c)
        {
            for (in_h = start_h; !((stride_h > 0) ? (in_h >= stop_h) : (in_h <= stop_h)); in_h += stride_h)
            {
                for (in_w = start_w; !((stride_w > 0) ? (in_w >= stop_w) : (in_w <= stop_w)); in_w += stride_w)
                {
                    srcIdx = ((in_n * input->dims[2] + in_c) * input->dims[1] + in_h) * input->dims[0] + in_w;
                    float32_value = vxnneGetDataExt(TENSOR_DATA_TYPE(input), TENSOR_QUANT_TYPE(input),
                        srcIdx, input_base, TENSOR_POS(input), TENSOR_TF_ZEROPOINT(input), TENSOR_TF_SCALE(input));
                    vxnneSaveDataExt(TENSOR_DATA_TYPE(output), TENSOR_QUANT_TYPE(output), dstIdx++, float32_value, output_base,
                        TENSOR_POS(output), TENSOR_TF_ZEROPOINT(output), TENSOR_TF_SCALE(output), TENSOR_ROUNDING_MODE(output));
                }
            }
        }
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_bool vxnneExecuteSWTSS_FullPositiveSeq(vx_tensor dims, vx_int32 mask)
{
    vx_uint32 i = 0;

    if (mask != 0)
        return vx_false_e;
    for (i = 0; i < TENSOR_SIZE_INDEX(dims, 0); i++)
    {
        if ((vx_int32)VX_GET_DATA_FROM_TENSOR(dims, i) < 0)
            return vx_false_e;
    }

    return vx_true_e;
}

#define STRIDED_SLICE_CHECK_TP_SUPPORT \
        vxnneExecuteSWTSS_FullPositiveSeq(begin_dims, begin_mask->value->n32) && /* only support positive sequence currently*/ \
        vxnneExecuteSWTSS_FullPositiveSeq(end_dims, end_mask->value->n32) && \
        vxnneExecuteSWTSS_FullPositiveSeq(stride_dims, shrink_axis_mask->value->n32)
#if REGISTER_FRAME
VX_PRIVATE_API vx_status vxoNNTensorStrideSlice_SW_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vxnne_tensor_stride_slice_layer  tensor_stride_slice_layer = (vxnne_tensor_stride_slice_layer)ops_layer;

    vx_tensor input                 = (vx_tensor)parameters[0];
    vx_tensor begin_dims            = (vx_tensor)parameters[1];
    vx_tensor end_dims              = (vx_tensor)parameters[2];
    vx_tensor stride_dims           = (vx_tensor)parameters[3];
    vx_scalar begin_mask            = (vx_scalar)parameters[4];
    vx_scalar end_mask              = (vx_scalar)parameters[5];
    vx_scalar shrink_axis_mask      = (vx_scalar)parameters[6];
    vx_tensor output                = (vx_tensor)parameters[7];
    vx_uint32 batchCount            = 1;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxnneOperation_Initialize(&tensor_stride_slice_layer->tensor_stride_slice_sw_operation.base,
        &tensor_stride_slice_layer->base,
        VXNNE_OPERATION_TARGET_SW,
        VXNNE_OPERATOR_TENSOR_STRIDE_SLICE,
        vxnneExecuteSWTensorStrideSlice,
        VX_NULL,
        batchCount,
        0));

    vxmONERROR(vxnneLayer_SetOperation(
        &tensor_stride_slice_layer->base,
        &tensor_stride_slice_layer->tensor_stride_slice_sw_operation.base,
        0));

    tensor_stride_slice_layer->tensor_stride_slice_sw_operation.input       = input;
    tensor_stride_slice_layer->tensor_stride_slice_sw_operation.begin_dims  = begin_dims;
    tensor_stride_slice_layer->tensor_stride_slice_sw_operation.end_dims    = end_dims;
    tensor_stride_slice_layer->tensor_stride_slice_sw_operation.strides     = stride_dims;
    tensor_stride_slice_layer->tensor_stride_slice_sw_operation.begin_mask  = begin_mask;
    tensor_stride_slice_layer->tensor_stride_slice_sw_operation.end_mask    = end_mask;
    tensor_stride_slice_layer->tensor_stride_slice_sw_operation.shrink_axis_mask = shrink_axis_mask;
    tensor_stride_slice_layer->tensor_stride_slice_sw_operation.output      = output;

    vxmONERROR(vxnneOperation_AddReference(&tensor_stride_slice_layer->tensor_stride_slice_sw_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&tensor_stride_slice_layer->tensor_stride_slice_sw_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNTensorStrideSlice_SH_EVIS_Support_Ext(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_tensor input                 = (vx_tensor)parameters[0];
    vx_tensor begin_dims            = (vx_tensor)parameters[1];
    vx_tensor end_dims              = (vx_tensor)parameters[2];
    vx_tensor stride_dims           = (vx_tensor)parameters[3];
    vx_scalar begin_mask            = (vx_scalar)parameters[4];
    vx_scalar end_mask              = (vx_scalar)parameters[5];
    vx_scalar shrink_axis_mask      = (vx_scalar)parameters[6];
    vx_tensor output                = (vx_tensor)parameters[7];

    vx_int32  start[4]              = {0};
    vx_int32  stop[4]               = {1};
    vx_int32  stride[4]             = {1};

    vx_enum   inputFormat           = TENSOR_DATA_TYPE(input);
    vx_enum   outputFormat          = TENSOR_DATA_TYPE(output);
    vx_uint32 batch                 = TENSOR_VIEW_DIM_NUM(input) > 3 ? TENSOR_VIEW_SIZE_INDEX(input, 3) : 1;
    vx_bool   enable_sh_crop        = vx_false_e;

    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    vxoNNTensorStrideSlice_getStartStopStride(input, begin_dims, end_dims, stride_dims, begin_mask, end_mask, shrink_axis_mask, start, stop, stride);

    enable_sh_crop = (vx_bool)((inputFormat != VX_TYPE_FLOAT32 && outputFormat != VX_TYPE_FLOAT32) && gcoMATH_Absolute((vx_float32)stride[0]) == gcoMATH_Absolute((vx_float32)stride[1]) && gcoMATH_Absolute((vx_float32)stride[0]) == gcoMATH_Absolute((vx_float32)stride[2]) && gcoMATH_Absolute((vx_float32)stride[0]) == 1 && batch == 1);


    support = support && (((_IsSameType(input, output) && batch == 1) || enable_sh_crop) && (TENSOR_VIEW_SIZE_INDEX(input, 0) < IMG_MAX_WIDTH && TENSOR_VIEW_SIZE_INDEX(input, 1) < IMG_MAX_WIDTH)) ;

    if (evis && ((inputFormat == VX_TYPE_FLOAT32) || (outputFormat == VX_TYPE_FLOAT32)))
        support = vx_false_e;

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_bool vxoNNTensorStrideSlice_SH_EVIS_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && node->base.context->evisNoInst.supportEVIS;

    if (!support)return support;

    support = support && vxoNNTensorStrideSlice_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_true_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNTensorStrideSlice_SH_EVIS_Initialize_Ext(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_status status = VX_SUCCESS;

    vx_tensor input                 = (vx_tensor)parameters[0];
    vx_tensor begin_dims            = (vx_tensor)parameters[1];
    vx_tensor end_dims              = (vx_tensor)parameters[2];
    vx_tensor stride_dims           = (vx_tensor)parameters[3];
    vx_scalar begin_mask            = (vx_scalar)parameters[4];
    vx_scalar end_mask              = (vx_scalar)parameters[5];
    vx_scalar shrink_axis_mask      = (vx_scalar)parameters[6];
    vx_tensor output                = (vx_tensor)parameters[7];
    vx_uint32 batchCount            = 1;
    vx_int32  start[4]              = {0};
    vx_int32  stop[4]               = {1};
    vx_int32  stride[4]             = {1};
    vx_uint32 reverseAxis[4]        = {0xcdcd};
    vx_uint32 numOfAxis             = 0;
    vx_uint32 opIdx                 = 0;
    vx_uint32 input_size[4]         = {0};
    vx_uint32 i                     = 0;
    vx_enum   inputFormat           = TENSOR_DATA_TYPE(input);
    vx_enum   outputFormat          = TENSOR_DATA_TYPE(output);
    vx_uint32 batch                 = TENSOR_VIEW_DIM_NUM(input) > 3 ? TENSOR_VIEW_SIZE_INDEX(input, 3) : 1;
    vx_bool   enable_sh_crop        = vx_false_e;
    vx_bool   enable_sh_reverse     = vx_false_e;

    vxnne_tensor_stride_slice_layer  tensor_stride_slice_layer = (vxnne_tensor_stride_slice_layer)ops_layer;

    vxnne_shader_executable shaderExecutable = VX_NULL;
    vx_tensor tmpTensor = NULL;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    for (i = 0; i < TENSOR_DIM_NUM(input); i++)
    {
        input_size[i] = TENSOR_VIEW_SIZE_INDEX(input, i);
    }

    vxmONERROR(vxoNNTensorStrideSlice_getStartStopStride(input, begin_dims, end_dims, stride_dims, begin_mask, end_mask, shrink_axis_mask, start, stop, stride));

    enable_sh_crop = (vx_bool)((inputFormat != VX_TYPE_FLOAT32 && outputFormat != VX_TYPE_FLOAT32) && gcoMATH_Absolute((vx_float32)stride[0]) == gcoMATH_Absolute((vx_float32)stride[1]) && gcoMATH_Absolute((vx_float32)stride[0]) == gcoMATH_Absolute((vx_float32)stride[2]) && gcoMATH_Absolute((vx_float32)stride[0]) == 1 && batch == 1);


    vxmONERROR(vxoNNTensorStrideSlice_getReverseAxis(start, stop, stride, reverseAxis, &numOfAxis, input_size));

    enable_sh_reverse    = (vx_bool)(numOfAxis > 0 && numOfAxis < 4);

    if (enable_sh_reverse)
    {
        vx_uint32 sizes[4] = {1};
        vx_uint32 dims     = TENSOR_DIM_NUM(output);
        vx_uint32 idx      = 0;
        vx_tensor_create_params_t tensor_create_params;
        vx_context context = vxGetContext((vx_reference)ops_layer->node);

        for (idx = 0; idx < dims; idx++)
        {
            sizes[idx] = TENSOR_VIEW_SIZE_INDEX(input, idx);
        }

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

        tmpTensor = vxoTensor_CreateTensor(ops_layer->node->base.context, ops_layer->node->graph, &tensor_create_params, vx_true_e);

        tensor_stride_slice_layer->base.temp_tensors[0]  = tmpTensor;
        tensor_stride_slice_layer->base.num_temp_tensors = 1;


        if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_REVERSE) &&
            vxnneIsTPSupportFormat(context, input, VX_NULL, tmpTensor))
        {
            vx_op_param_s conv = {0};
            vx_int32 axis[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};

            for (i = 0; i < numOfAxis; i++)
            {
                axis[i] = reverseAxis[i];
            }

            vxmONERROR(vxnneOperation_Initialize(&tensor_stride_slice_layer->tensor_reverse_tp_operation.base,
                &tensor_stride_slice_layer->base,
                VXNNE_OPERATION_TARGET_TP,
                VXNNE_OPERATOR_TENSOR_REVERSE,
                VX_NULL,
                vxnneOperation_TP_Deinitialize,
                batchCount,
                0));

            vxmONERROR(vxnneLayer_SetOperation(&tensor_stride_slice_layer->base, &tensor_stride_slice_layer->tensor_reverse_tp_operation.base, opIdx++));
            tensor_stride_slice_layer->tensor_reverse_tp_operation.input  = input;
            tensor_stride_slice_layer->tensor_reverse_tp_operation.output = tmpTensor;

            vxmONERROR(vxnneOperation_AddReference(&tensor_stride_slice_layer->tensor_reverse_tp_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT));
            vxmONERROR(vxnneOperation_AddReference(&tensor_stride_slice_layer->tensor_reverse_tp_operation.base, (vx_reference)tmpTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT));

            conv.pad_x_left = 0;
            conv.pad_y_top = 0;
            conv.pool_size_x = 0;
            conv.pool_size_y = 0;
            conv.pool_stride = 1;
            conv.enable_relu = vx_false_e;
            conv.conv_rounding_type = 0;
            conv.pad_mode = VX_PAD_CONSTANT;
            conv.pad_const = 0;
            conv.tpType = TP_REVERSE;
            conv.data_buff = gcvNULL;
            conv.other_ref = (vx_reference)input;
            conv.tp_value = (vx_tp_value_cmd)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
            conv.tp_value->u32[0] = numOfAxis;
            conv.tp_value->p8[0] = (vx_uint8_ptr)vxAllocateAndZeroMemory(sizeof(axis));
            vxMemCopy(conv.tp_value->p8[0], axis, sizeof(axis));

            vxMemCopy(&tensor_stride_slice_layer->tensor_reverse_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));
        }
        else
        {
            if(evis)
            {
                shaderExecutable = vxnneGetReverseShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_REVERSE, &ops_layer->node->kernelAttributes.borderMode,
                    input, tmpTensor, numOfAxis, reverseAxis);
            }
            else
            {
                shaderExecutable = vxnneGetGPUReverseShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_GPU_TENSOR_REVERSE, &ops_layer->node->kernelAttributes.borderMode,
                    input, tmpTensor, numOfAxis, reverseAxis);
            }

            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto OnError;
            }
            vxmONERROR(vxnneShaderOperation_Initialize(&tensor_stride_slice_layer->tensor_reverse_sh_operation,
                &tensor_stride_slice_layer->base,
                VXNNE_OPERATOR_TENSOR_STRIDE_SLICE,
                batchCount,
                shaderExecutable));

            vxmONERROR(vxnneOperation_AddReference(&tensor_stride_slice_layer->tensor_reverse_sh_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT));
            vxmONERROR(vxnneOperation_AddReference(&tensor_stride_slice_layer->tensor_reverse_sh_operation.base, (vx_reference)tmpTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT));

            vxmONERROR(vxnneLayer_SetOperation(
                &tensor_stride_slice_layer->base,
                &tensor_stride_slice_layer->tensor_reverse_sh_operation.base,
                opIdx++));
        }
    }
    else
    {
        tmpTensor = input;
    }

    if (enable_sh_crop)
    {
        if(evis)
        {
            shaderExecutable = vxnneGetTensorCropShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_CROP, &ops_layer->node->kernelAttributes.borderMode, start, stop, tmpTensor, output);
        }
        else
        {
            shaderExecutable = vxnneGetGPUTensorCropShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_GPU_TENSOR_CROP, &ops_layer->node->kernelAttributes.borderMode, start, stop, tmpTensor, output);
        }

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto OnError;
        }
        vxmONERROR(vxnneShaderOperation_Initialize(&tensor_stride_slice_layer->tensor_crop_sh_operation,
            &tensor_stride_slice_layer->base,
            VXNNE_OPERATOR_TENSOR_STRIDE_SLICE,
            batchCount,
            shaderExecutable));

        vxmONERROR(vxnneOperation_AddReference(&tensor_stride_slice_layer->tensor_crop_sh_operation.base, (vx_reference)tmpTensor, VXNNE_OPERATION_REFENRENCE_INPUT));
        vxmONERROR(vxnneOperation_AddReference(&tensor_stride_slice_layer->tensor_crop_sh_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT));

        vxmONERROR(vxnneLayer_SetOperation(
            &tensor_stride_slice_layer->base,
            &tensor_stride_slice_layer->tensor_crop_sh_operation.base,
            opIdx++));
    }
    else
    {
        if(evis)
        {
            shaderExecutable = vxnneGetTensorStridedSliceShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_STRIDE_SLICE, &ops_layer->node->kernelAttributes.borderMode, start, stop, stride, tmpTensor, output);
        }
        else
        {
            shaderExecutable = vxnneGetGPUTensorStridedSliceShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_GPU_TENSOR_STRIDE_SLICE, &ops_layer->node->kernelAttributes.borderMode, start, stop, stride, tmpTensor, output);
        }

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto OnError;
        }
        vxmONERROR(vxnneShaderOperation_Initialize(&tensor_stride_slice_layer->tensor_stride_slice_sh_operation,
            &tensor_stride_slice_layer->base,
            VXNNE_OPERATOR_TENSOR_STRIDE_SLICE,
            batchCount,
            shaderExecutable));

        vxmONERROR(vxnneOperation_AddReference(&tensor_stride_slice_layer->tensor_stride_slice_sh_operation.base, (vx_reference)tmpTensor, VXNNE_OPERATION_REFENRENCE_INPUT));
        vxmONERROR(vxnneOperation_AddReference(&tensor_stride_slice_layer->tensor_stride_slice_sh_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT));

        vxmONERROR(vxnneLayer_SetOperation(
            &tensor_stride_slice_layer->base,
            &tensor_stride_slice_layer->tensor_stride_slice_sh_operation.base,
            opIdx++));
    }

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

OnError:
    return status;
}

VX_PRIVATE_API vx_status vxoNNTensorStrideSlice_SH_EVIS_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxoNNTensorStrideSlice_SH_EVIS_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_true_e));

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

OnError:
    return status;
}
VX_PRIVATE_API vx_bool vxoNNTensorStrideSlice_SH_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && vxoNNTensorStrideSlice_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_false_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNTensorStrideSlice_SH_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxoNNTensorStrideSlice_SH_EVIS_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_false_e));

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);
OnError:
    return status;
}

VX_PRIVATE_API vx_bool vxoNNTensorStrideSlice_TP_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_tensor input                 = (vx_tensor)parameters[0];
    vx_tensor begin_dims            = (vx_tensor)parameters[1];
    vx_tensor end_dims              = (vx_tensor)parameters[2];
    vx_tensor stride_dims           = (vx_tensor)parameters[3];
    vx_scalar begin_mask            = (vx_scalar)parameters[4];
    vx_scalar end_mask              = (vx_scalar)parameters[5];
    vx_scalar shrink_axis_mask      = (vx_scalar)parameters[6];
    vx_tensor output                = (vx_tensor)parameters[7];

    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_TP, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support  && (vxnneIsTPSupportFormat(node->base.context, input, VX_NULL, output) && STRIDED_SLICE_CHECK_TP_SUPPORT);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNTensorStrideSlice_TP_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vxnne_tensor_stride_slice_layer  tensor_stride_slice_layer = (vxnne_tensor_stride_slice_layer)ops_layer;

    vx_tensor input                 = (vx_tensor)parameters[0];
    vx_tensor begin_dims            = (vx_tensor)parameters[1];
    vx_tensor end_dims              = (vx_tensor)parameters[2];
    vx_tensor stride_dims           = (vx_tensor)parameters[3];

    vx_tensor output                = (vx_tensor)parameters[7];

    vx_uint32 batch                 = TENSOR_VIEW_DIM_NUM(input) > 3 ? TENSOR_VIEW_SIZE_INDEX(input, 3) : 1;

    vx_op_param_s conv = { 0 };

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    conv.pad_x_left = 0;
    conv.pad_y_top = 0;
    conv.pool_size_x = 0;
    conv.pool_size_y = 0;
    conv.pool_stride = 1;
    conv.enable_relu = vx_false_e;
    conv.pad_mode = VX_PAD_CONSTANT;
    conv.pad_const = 0;

    vxmONERROR(vxnneOperation_Initialize(&tensor_stride_slice_layer->tensor_stride_slice_tp_operation.base,
        &tensor_stride_slice_layer->base,
        VXNNE_OPERATION_TARGET_TP,
        VXNNE_OPERATOR_TENSOR_STRIDE_SLICE,
        VX_NULL,
        vxnneOperation_TP_Deinitialize,
        batch,
        0));

    vxmONERROR(vxnneLayer_SetOperation(
        &tensor_stride_slice_layer->base,
        &tensor_stride_slice_layer->tensor_stride_slice_tp_operation.base,
        0));

    tensor_stride_slice_layer->tensor_stride_slice_tp_operation.input = input;
    tensor_stride_slice_layer->tensor_stride_slice_tp_operation.output = output;

    vxmONERROR(vxnneOperation_AddReference(&tensor_stride_slice_layer->tensor_stride_slice_tp_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&tensor_stride_slice_layer->tensor_stride_slice_tp_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    conv.tpType = TP_TENSOR_STRIDED_SLICE;
    conv.other_ref = (vx_reference)input;
    conv.data_buff = gcvNULL;

    conv.tp_value = (vx_tp_value_cmd)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
    /*
     * u32[0]: begin_dims[0]
     * u32[1]: begin_dims[1]
     * u32[2]: end_dims[0]
     * u32[3]: end_dims[1]
     * u32[4]: stride_dims[0]
     * u32[5]: stride_dims[1]
     */
    conv.tp_value->u32[0] = (vx_uint32)VX_GET_DATA_FROM_TENSOR(begin_dims, 0);
    conv.tp_value->u32[1] = TENSOR_VIEW_SIZE_INDEX(begin_dims, 0) > 1 ? (vx_uint32)VX_GET_DATA_FROM_TENSOR(begin_dims, 1) : 0;
    conv.tp_value->u32[2] = (vx_uint32)VX_GET_DATA_FROM_TENSOR(end_dims, 0);
    conv.tp_value->u32[3] = TENSOR_VIEW_SIZE_INDEX(end_dims, 0) > 1 ? (vx_uint32)VX_GET_DATA_FROM_TENSOR(end_dims, 1) : 1;
    conv.tp_value->u32[4] = (vx_uint32)VX_GET_DATA_FROM_TENSOR(stride_dims, 0); /* stride x*/
    conv.tp_value->u32[5] = TENSOR_VIEW_SIZE_INDEX(stride_dims, 0) > 1 ? (vx_uint32)VX_GET_DATA_FROM_TENSOR(stride_dims, 1) : 1; /* stride y*/

    memcpy(&tensor_stride_slice_layer->tensor_stride_slice_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_status vxoNNLayer_GetOperations(vxnne_layer ops_layer, vx_uint32_ptr max_num_operations, vxnne_operation **operations)
{
    vx_status  status = VX_SUCCESS;
    vxnne_tensor_stride_slice_layer  tensor_stride_slice_layer = (vxnne_tensor_stride_slice_layer)ops_layer;

    *max_num_operations = gcmCOUNTOF(tensor_stride_slice_layer->operations);

    *operations = tensor_stride_slice_layer->operations;

    return status;
}
#endif

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorStrideSlice_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
#if REGISTER_FRAME
    vxnne_layer_imp_s registerTensorStrideSliceLayers[] = {/* Please DON'T adjust the order, it's importent */
        { "RPNLAYER NN", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "RPNLAYER TP", vxoNNTensorStrideSlice_TP_Support, vxoNNTensorStrideSlice_TP_Initialize, VX_NULL },
        { "RPNLAYER SH EVIS", vxoNNTensorStrideSlice_SH_EVIS_Support, vxoNNTensorStrideSlice_SH_EVIS_Initialize, VX_NULL },
        { "RPNLAYER SH F32", vxoNNTensorStrideSlice_SH_Support, vxoNNTensorStrideSlice_SH_Initialize, VX_NULL },
        { "RPNLAYER SW", vxoNNCommon_Support, vxoNNTensorStrideSlice_SW_Initialize, VX_NULL },
    };

    REGISTER_LAYERS(registerTensorStrideSliceLayers, vxnne_tensor_stride_slice_layer_s, "TensorStrideSlice", vxoNNLayer_GetOperations);

OnError:
#else

    vx_tensor input                 = (vx_tensor)parameters[0];
    vx_tensor begin_dims            = (vx_tensor)parameters[1];
    vx_tensor end_dims              = (vx_tensor)parameters[2];
    vx_tensor stride_dims           = (vx_tensor)parameters[3];
    vx_scalar begin_mask            = (vx_scalar)parameters[4];
    vx_scalar end_mask              = (vx_scalar)parameters[5];
    vx_scalar shrink_axis_mask      = (vx_scalar)parameters[6];
    vx_tensor output                = (vx_tensor)parameters[7];
    vx_uint32 batchCount            = 1;
    vx_int32  start[4]              = {0};
    vx_int32  stop[4]               = {1};
    vx_int32  stride[4]             = {1};
    vx_uint32 reverseAxis[4]        = {0xcdcd};
    vx_uint32 numOfAxis             = 0;
    vx_uint32 opIdx                 = 0;
    vx_uint32 input_size[4]         = {0};
    vx_uint32 i                     = 0;
    vx_enum   inputFormat           = TENSOR_DATA_TYPE(input);
    vx_enum   outputFormat          = TENSOR_DATA_TYPE(output);
    vx_uint32 batch                 = TENSOR_VIEW_DIM_NUM(input) > 3 ? TENSOR_VIEW_SIZE_INDEX(input, 3) : 1;
    vx_bool   enable_sh_crop        = vx_false_e;
    vx_bool   enable_sh_reverse     = vx_false_e;
    vx_bool   shExe_flag            = vx_false_e;

    vxnne_tensor_stride_slice_layer tensor_stride_slice_layer = VX_NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_tensor_stride_slice_layer_s), (gctPOINTER*)&tensor_stride_slice_layer);
    if (!tensor_stride_slice_layer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(tensor_stride_slice_layer, sizeof(vxnne_tensor_stride_slice_layer_s));

    vxnneLayer_Initialize(&tensor_stride_slice_layer->base,
        "TensorStrideSlice",
        node,
        vxmOPERATION_COUNT(tensor_stride_slice_layer),
        tensor_stride_slice_layer->operations,
        VX_NULL);

    for (i = 0; i < TENSOR_DIM_NUM(input); i++)
    {
        input_size[i] = TENSOR_VIEW_SIZE_INDEX(input, i);
    }

    vxoNNTensorStrideSlice_getStartStopStride(input, begin_dims, end_dims, stride_dims, begin_mask, end_mask, shrink_axis_mask, start, stop, stride);

    enable_sh_crop = (vx_bool)((inputFormat != VX_TYPE_FLOAT32 && outputFormat != VX_TYPE_FLOAT32) && gcoMATH_Absolute((vx_float32)stride[0]) == gcoMATH_Absolute((vx_float32)stride[1]) && gcoMATH_Absolute((vx_float32)stride[0]) == gcoMATH_Absolute((vx_float32)stride[2]) && gcoMATH_Absolute((vx_float32)stride[0]) == 1 && batch == 1);


    vxoNNTensorStrideSlice_getReverseAxis(start, stop, stride, reverseAxis, &numOfAxis, input_size);

    enable_sh_reverse    = (vx_bool)(numOfAxis > 0 && numOfAxis < 4);

    shExe_flag = (vx_bool)(((_IsSameType(input, output) && batch == 1) || enable_sh_crop) && (TENSOR_VIEW_SIZE_INDEX(input, 0) < IMG_MAX_WIDTH && TENSOR_VIEW_SIZE_INDEX(input, 1) < IMG_MAX_WIDTH)) ;

    if (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_TP) &&
        vxnneIsTPSupportFormat(node->base.context, input, VX_NULL, output) && STRIDED_SLICE_CHECK_TP_SUPPORT
        )
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

        vxnneOperation_Initialize(&tensor_stride_slice_layer->tensor_stride_slice_tp_operation.base,
            &tensor_stride_slice_layer->base,
            VXNNE_OPERATION_TARGET_TP,
            VXNNE_OPERATOR_TENSOR_STRIDE_SLICE,
            VX_NULL,
            vxnneOperation_TP_Deinitialize,
            batch,
            0);

        vxnneLayer_SetOperation(
            &tensor_stride_slice_layer->base,
            &tensor_stride_slice_layer->tensor_stride_slice_tp_operation.base,
            0);

        tensor_stride_slice_layer->tensor_stride_slice_tp_operation.input = input;
        tensor_stride_slice_layer->tensor_stride_slice_tp_operation.output = output;

        vxnneOperation_AddReference(&tensor_stride_slice_layer->tensor_stride_slice_tp_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&tensor_stride_slice_layer->tensor_stride_slice_tp_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        conv.tpType = TP_TENSOR_STRIDED_SLICE;
        conv.other_ref = (vx_reference)input;
        conv.data_buff = gcvNULL;

        conv.tp_value = (vx_tp_value_cmd)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
        /*
         * u32[0]: begin_dims[0]
         * u32[1]: begin_dims[1]
         * u32[2]: end_dims[0]
         * u32[3]: end_dims[1]
         * u32[4]: stride_dims[0]
         * u32[5]: stride_dims[1]
         */
        conv.tp_value->u32[0] = (vx_uint32)VX_GET_DATA_FROM_TENSOR(begin_dims, 0);
        conv.tp_value->u32[1] = TENSOR_VIEW_SIZE_INDEX(begin_dims, 0) > 1 ? (vx_uint32)VX_GET_DATA_FROM_TENSOR(begin_dims, 1) : 0;
        conv.tp_value->u32[2] = (vx_uint32)VX_GET_DATA_FROM_TENSOR(end_dims, 0);
        conv.tp_value->u32[3] = TENSOR_VIEW_SIZE_INDEX(end_dims, 0) > 1 ? (vx_uint32)VX_GET_DATA_FROM_TENSOR(end_dims, 1) : 1;
        conv.tp_value->u32[4] = (vx_uint32)VX_GET_DATA_FROM_TENSOR(stride_dims, 0); /* stride x*/
        conv.tp_value->u32[5] = TENSOR_VIEW_SIZE_INDEX(stride_dims, 0) > 1 ? (vx_uint32)VX_GET_DATA_FROM_TENSOR(stride_dims, 1) : 1; /* stride y*/

        memcpy(&tensor_stride_slice_layer->tensor_stride_slice_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));
    }
    else if(shExe_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
    {
        vxnne_shader_executable shaderExecutable = VX_NULL;
        vx_tensor tmpTensor = NULL;

        if (enable_sh_reverse)
        {
            vx_uint32 sizes[4] = {1};
            vx_uint32 dims     = TENSOR_DIM_NUM(output);
            vx_uint32 idx      = 0;
            vx_tensor_create_params_t tensor_create_params;
            vx_context context = vxGetContext((vx_reference)node);

            for (idx = 0; idx < dims; idx++)
            {
                sizes[idx] = TENSOR_VIEW_SIZE_INDEX(input, idx);
            }

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

            tmpTensor = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);

            tensor_stride_slice_layer->base.temp_tensors[0]  = tmpTensor;
            tensor_stride_slice_layer->base.num_temp_tensors = 1;


            if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_REVERSE) &&
                vxnneIsTPSupportFormat(context, input, VX_NULL, tmpTensor))
            {
                vx_op_param_s conv = {0};
                vx_int32 axis[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};

                for (i = 0; i < numOfAxis; i++)
                {
                    axis[i] = reverseAxis[i];
                }

                status = vxnneOperation_Initialize(&tensor_stride_slice_layer->tensor_reverse_tp_operation.base,
                    &tensor_stride_slice_layer->base,
                    VXNNE_OPERATION_TARGET_TP,
                    VXNNE_OPERATOR_TENSOR_REVERSE,
                    VX_NULL,
                    vxnneOperation_TP_Deinitialize,
                    batchCount,
                    0);
                if (status != VX_SUCCESS) goto exit;
                vxnneLayer_SetOperation(&tensor_stride_slice_layer->base, &tensor_stride_slice_layer->tensor_reverse_tp_operation.base, opIdx++);
                tensor_stride_slice_layer->tensor_reverse_tp_operation.input  = input;
                tensor_stride_slice_layer->tensor_reverse_tp_operation.output = tmpTensor;

                vxnneOperation_AddReference(&tensor_stride_slice_layer->tensor_reverse_tp_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&tensor_stride_slice_layer->tensor_reverse_tp_operation.base, (vx_reference)tmpTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                conv.pad_x_left = 0;
                conv.pad_y_top = 0;
                conv.pool_size_x = 0;
                conv.pool_size_y = 0;
                conv.pool_stride = 1;
                conv.enable_relu = vx_false_e;
                conv.conv_rounding_type = 0;
                conv.pad_mode = VX_PAD_CONSTANT;
                conv.pad_const = 0;
                conv.tpType = TP_REVERSE;
                conv.data_buff = gcvNULL;
                conv.other_ref = (vx_reference)input;
                conv.tp_value = (vx_tp_value_cmd)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
                conv.tp_value->u32[0] = numOfAxis;
                conv.tp_value->p8[0] = (vx_uint8_ptr)vxAllocateAndZeroMemory(sizeof(axis));
                vxMemCopy(conv.tp_value->p8[0], axis, sizeof(axis));

                vxMemCopy(&tensor_stride_slice_layer->tensor_reverse_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));
            }
            else
            {
                if(node->base.context->evisNoInst.supportEVIS)
                {
                    shaderExecutable = vxnneGetReverseShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_REVERSE, &node->kernelAttributes.borderMode,
                        input, tmpTensor, numOfAxis, reverseAxis);
                }
                else
                {
                    shaderExecutable = vxnneGetGPUReverseShaderExecutable(node->base.context, VXNNE_KERNEL_GPU_TENSOR_REVERSE, &node->kernelAttributes.borderMode,
                        input, tmpTensor, numOfAxis, reverseAxis);
                }

                if (!shaderExecutable)
                {
                    status = VX_FAILURE;
                    goto exit;
                }
                status = vxnneShaderOperation_Initialize(&tensor_stride_slice_layer->tensor_reverse_sh_operation,
                    &tensor_stride_slice_layer->base,
                    VXNNE_OPERATOR_TENSOR_STRIDE_SLICE,
                    batchCount,
                    shaderExecutable);

                if (status != VX_SUCCESS)
                    goto exit;

                vxnneOperation_AddReference(&tensor_stride_slice_layer->tensor_reverse_sh_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&tensor_stride_slice_layer->tensor_reverse_sh_operation.base, (vx_reference)tmpTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                vxnneLayer_SetOperation(
                    &tensor_stride_slice_layer->base,
                    &tensor_stride_slice_layer->tensor_reverse_sh_operation.base,
                    opIdx++);
            }
        }
        else
        {
            tmpTensor = input;
        }

        if (enable_sh_crop)
        {
            if(node->base.context->evisNoInst.supportEVIS)
            {
                shaderExecutable = vxnneGetTensorCropShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_CROP, &node->kernelAttributes.borderMode, start, stop, tmpTensor, output);
            }
            else
            {
                shaderExecutable = vxnneGetGPUTensorCropShaderExecutable(node->base.context, VXNNE_KERNEL_GPU_TENSOR_CROP, &node->kernelAttributes.borderMode, start, stop, tmpTensor, output);
            }

            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto exit;
            }
            status = vxnneShaderOperation_Initialize(&tensor_stride_slice_layer->tensor_crop_sh_operation,
                &tensor_stride_slice_layer->base,
                VXNNE_OPERATOR_TENSOR_STRIDE_SLICE,
                batchCount,
                shaderExecutable);

            if (status != VX_SUCCESS)
                goto exit;

            vxnneOperation_AddReference(&tensor_stride_slice_layer->tensor_crop_sh_operation.base, (vx_reference)tmpTensor, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&tensor_stride_slice_layer->tensor_crop_sh_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            vxnneLayer_SetOperation(
                &tensor_stride_slice_layer->base,
                &tensor_stride_slice_layer->tensor_crop_sh_operation.base,
                opIdx++);
        }
        else
        {
            if(node->base.context->evisNoInst.supportEVIS)
            {
                shaderExecutable = vxnneGetTensorStridedSliceShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_STRIDE_SLICE, &node->kernelAttributes.borderMode, start, stop, stride, tmpTensor, output);
            }
            else
            {
                shaderExecutable = vxnneGetGPUTensorStridedSliceShaderExecutable(node->base.context, VXNNE_KERNEL_GPU_TENSOR_STRIDE_SLICE, &node->kernelAttributes.borderMode, start, stop, stride, tmpTensor, output);
            }

            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto exit;
            }
            status = vxnneShaderOperation_Initialize(&tensor_stride_slice_layer->tensor_stride_slice_sh_operation,
                &tensor_stride_slice_layer->base,
                VXNNE_OPERATOR_TENSOR_STRIDE_SLICE,
                batchCount,
                shaderExecutable);

            if (status != VX_SUCCESS)
                goto exit;

            vxnneOperation_AddReference(&tensor_stride_slice_layer->tensor_stride_slice_sh_operation.base, (vx_reference)tmpTensor, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&tensor_stride_slice_layer->tensor_stride_slice_sh_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            vxnneLayer_SetOperation(
                &tensor_stride_slice_layer->base,
                &tensor_stride_slice_layer->tensor_stride_slice_sh_operation.base,
                opIdx++);
        }
    }
    else
    {
        vxnneOperation_Initialize(&tensor_stride_slice_layer->tensor_stride_slice_sw_operation.base,
            &tensor_stride_slice_layer->base,
            VXNNE_OPERATION_TARGET_SW,
            VXNNE_OPERATOR_TENSOR_STRIDE_SLICE,
            vxnneExecuteSWTensorStrideSlice,
            VX_NULL,
            batchCount,
            0);

        vxnneLayer_SetOperation(
            &tensor_stride_slice_layer->base,
            &tensor_stride_slice_layer->tensor_stride_slice_sw_operation.base,
            0);

        tensor_stride_slice_layer->tensor_stride_slice_sw_operation.input       = input;
        tensor_stride_slice_layer->tensor_stride_slice_sw_operation.begin_dims  = begin_dims;
        tensor_stride_slice_layer->tensor_stride_slice_sw_operation.end_dims    = end_dims;
        tensor_stride_slice_layer->tensor_stride_slice_sw_operation.strides     = stride_dims;
        tensor_stride_slice_layer->tensor_stride_slice_sw_operation.begin_mask  = begin_mask;
        tensor_stride_slice_layer->tensor_stride_slice_sw_operation.end_mask    = end_mask;
        tensor_stride_slice_layer->tensor_stride_slice_sw_operation.shrink_axis_mask = shrink_axis_mask;
        tensor_stride_slice_layer->tensor_stride_slice_sw_operation.output      = output;

        vxnneOperation_AddReference(&tensor_stride_slice_layer->tensor_stride_slice_sw_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&tensor_stride_slice_layer->tensor_stride_slice_sw_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

    }

    node->layer = &tensor_stride_slice_layer->base;
    return status;

exit:
    if (tensor_stride_slice_layer)
    {
        if (tensor_stride_slice_layer->tensor_reverse_tp_operation.base.parameter.tp_value)
        {
            if (tensor_stride_slice_layer->tensor_reverse_tp_operation.base.parameter.tp_value->p8[0])
            {
                gcoOS_Free(NULL, (gctPOINTER)tensor_stride_slice_layer->tensor_reverse_tp_operation.base.parameter.tp_value->p8[0]);
            }
            gcoOS_Free(NULL, (gctPOINTER)tensor_stride_slice_layer->tensor_reverse_tp_operation.base.parameter.tp_value);
        }
        gcoOS_Free(NULL, (gctPOINTER)tensor_stride_slice_layer);
    }
#endif
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorStrideSlice_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

