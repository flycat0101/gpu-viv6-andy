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
#include <layers/gc_vx_layer_rpn.h>


VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNRPNLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNRPNLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNRPNLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}



/*RPN - CPU*/
vx_status vxnneExecuteSWRPN(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_tensor_rpn_operation rpnOperation = (vxnne_tensor_rpn_operation)operation;

    /*
        width: W, height: H, anchor number: k

        input  score:    1x(k*2)xWxH
        input  bbox:     1x(k*4)xWxH
        input  anchor:   kx4x1x1
        input  img_info: 1x4x1x1
        output roi:      300x5x1x1
        output score:    300x1x1x1
    */
    vx_tensor score         = rpnOperation->score;
    vx_tensor bbox          = rpnOperation->bbox;
    vx_tensor anchor        = rpnOperation->anchors;
    vx_tensor img_info      = rpnOperation->img_info;
    vx_tensor roi_output    = rpnOperation->roi_output;
    vx_tensor score_output  = rpnOperation->score_output;

    vx_uint32 feat_stride   = rpnOperation->feature_stride->value->u32;
    vx_uint32 min_size      = rpnOperation->min_size->value->u32;
    vx_uint32 pre_nms_topn  = rpnOperation->pre_nms_topn->value->u32;
    vx_uint32 post_nms_topn = rpnOperation->post_nms_topn->value->u32;
    vx_float32 nms_thresh   = rpnOperation->nms_thresh->value->f32;

    vx_uint32 score_channel = TENSOR_VIEW_SIZE_INDEX(score, 2);
    vx_uint32 score_height  = TENSOR_VIEW_SIZE_INDEX(score, 1);
    vx_uint32 score_width   = TENSOR_VIEW_SIZE_INDEX(score, 0);
    vx_uint32 score_count   = score_width * score_height * score_channel/2; /*14 x 14 x (2x42) -> 17901 x 2 */

    vx_uint32 bbox_height   = TENSOR_VIEW_SIZE_INDEX(bbox, 1);
    vx_uint32 bbox_width    = TENSOR_VIEW_SIZE_INDEX(bbox, 0);

    vx_uint32 anchor_count  = TENSOR_VIEW_SIZE_INDEX(anchor, 3); /* anchor batch = anchor number */

    vx_type_e in_score_format   = (vx_type_e)TENSOR_DATA_TYPE(score);
    vx_type_e in_bbox_format    = (vx_type_e)TENSOR_DATA_TYPE(bbox);
    vx_type_e in_anchor_format  = (vx_type_e)TENSOR_DATA_TYPE(anchor);
    vx_type_e in_img_format     = (vx_type_e)TENSOR_DATA_TYPE(img_info);
    vx_type_e out_roi_format    = (vx_type_e)TENSOR_DATA_TYPE(roi_output);
    vx_int8 in_score_fp         = TENSOR_POS(score);
    vx_int8 in_bbox_fp          = TENSOR_POS(bbox);
    vx_int8 in_anchor_fp        = TENSOR_POS(anchor);
    vx_int8 in_img_fp           = TENSOR_POS(img_info);
    vx_int8 out_roi_fp          = TENSOR_POS(roi_output);
    vx_enum in_score_quant_format   = TENSOR_QUANT_TYPE(score);
    vx_enum in_bbox_quant_format    = TENSOR_QUANT_TYPE(bbox);
    vx_enum in_anchor_quant_format  = TENSOR_QUANT_TYPE(anchor);
    vx_enum in_img_quant_format     = TENSOR_QUANT_TYPE(img_info);
    vx_enum out_roi_quant_format    = TENSOR_QUANT_TYPE(roi_output);
    vx_int32 in_score_zp            = TENSOR_TF_ZEROPOINT(score);
    vx_int32 in_bbox_zp             = TENSOR_TF_ZEROPOINT(bbox);
    vx_int32 in_anchor_zp           = TENSOR_TF_ZEROPOINT(anchor);
    vx_int32 in_img_zp              = TENSOR_TF_ZEROPOINT(img_info);
    vx_int32 out_roi_zp             = TENSOR_TF_ZEROPOINT(roi_output);
    vx_float32 in_score_scale       = TENSOR_TF_SCALE(score);
    vx_float32 in_bbox_scale        = TENSOR_TF_SCALE(bbox);
    vx_float32 in_anchor_scale      = TENSOR_TF_SCALE(anchor);
    vx_float32 in_img_scale         = TENSOR_TF_SCALE(img_info);
    vx_float32 out_roi_scale        = TENSOR_TF_SCALE(roi_output);
    vx_enum out_roi_rMode           = TENSOR_ROUNDING_MODE(roi_output);

    vx_uint32 proposal_width, proposal_height, proposal_area, proposal_count;
    vx_float32 img_W,img_H,img_scale_H,img_scale_W;
    vx_float32 min_box_H,min_box_W;
    vx_uint8_ptr score_data,bbox_data,img_data,anchor_data,out_roi_data;
    vx_float32_ptr score_buffer = NULL, foreground_score = NULL;
    vx_float32_ptr proposals    = NULL, p_proposals = NULL;
    vx_uint32_ptr roi_indices   = NULL;
    vx_uint32 i = 0, w = 0, h = 0, k = 0;
    vx_uint32 real_roi = 0;

    vx_type_e out_score_format = 0;
    vx_int8 out_score_fp = 0;
    vx_enum out_score_quant_format = 0;
    vx_int32 out_score_zp = 0;
    vx_float32 out_score_scale = 0;
    vx_enum out_score_rMode = 0;
    vx_uint8_ptr out_score_data = NULL;

    vx_bool input_stage,output_stage;

    vxoBinaryGraph_SaveSWOperation(operation);

    if(score_height != bbox_height || score_width != bbox_width)
    {
        vxError("parameter error: score_H[%u] != bbox_H[%u] || score_W[%u] != bbox_W[%u]\n",
            score_height, bbox_height, score_width, bbox_width);
        return VX_FAILURE;
    }

    proposal_width  = score_width;
    proposal_height = score_height;
    proposal_area   = proposal_width * proposal_height;
    proposal_count  = proposal_area * anchor_count;

    score_buffer    = (vx_float32_ptr)vxAllocateAndZeroMemory((score_count * 2) * sizeof(vx_float32)); /* foreground + background */
    proposals       = (vx_float32_ptr)vxAllocateAndZeroMemory((proposal_count * 5) * sizeof(vx_float32)); /* 5: score,x1,y1,x2,y2 */
    roi_indices     = (vx_uint32_ptr)vxAllocateAndZeroMemory(post_nms_topn * sizeof(vx_uint32));

    if (score_buffer == NULL || proposals == NULL || roi_indices == NULL)
    {
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        if(score_buffer) vxFree(score_buffer);
        if(proposals) vxFree(proposals);
        if(roi_indices) vxFree(roi_indices);
        return VX_ERROR_NO_MEMORY;
    }

    input_stage = vx_true_e;
    output_stage = vx_true_e;
    vxnneGetTensorMemeory(score, (vx_ptr_ptr)&score_data, input_stage, vx_false_e);
    vxnneGetTensorMemeory(bbox, (vx_ptr_ptr)&bbox_data, input_stage, vx_false_e);
    vxnneGetTensorMemeory(img_info, (vx_ptr_ptr)&img_data, input_stage, vx_false_e);
    vxnneGetTensorMemeory(anchor, (vx_ptr_ptr)&anchor_data, input_stage, vx_false_e);

    vxnneGetTensorMemeory(roi_output, (vx_ptr_ptr)&out_roi_data, output_stage, vx_true_e);

    if(score_output)
    {
        out_score_format        = (vx_type_e)TENSOR_DATA_TYPE(score_output);
        out_score_fp            = TENSOR_POS(score_output);
        out_score_quant_format  = TENSOR_QUANT_TYPE(score_output);
        out_score_zp            = TENSOR_TF_ZEROPOINT(score_output);
        out_score_scale         = TENSOR_TF_SCALE(score_output);
        out_score_rMode         = TENSOR_ROUNDING_MODE(score_output);
        vxnneGetTensorMemeory(score_output, (vx_ptr_ptr)&out_score_data, output_stage, vx_true_e);
    }

    img_W       = (vx_float32)vxnneGetDataExt(in_img_format, in_img_quant_format, 0, (vx_uint8_ptr)img_data, in_img_fp, in_img_zp, in_img_scale);
    img_H       = (vx_float32)vxnneGetDataExt(in_img_format, in_img_quant_format, 1, (vx_uint8_ptr)img_data, in_img_fp, in_img_zp, in_img_scale);
    img_scale_W = (vx_float32)vxnneGetDataExt(in_img_format, in_img_quant_format, 2, (vx_uint8_ptr)img_data, in_img_fp, in_img_zp, in_img_scale);
    img_scale_H = (vx_float32)vxnneGetDataExt(in_img_format, in_img_quant_format, 3, (vx_uint8_ptr)img_data, in_img_fp, in_img_zp, in_img_scale);
    min_box_W   = min_size * img_scale_W;
    min_box_H   = min_size * img_scale_H;

    /*
        1. prepare the score softmax
          1.1 resharp score data
          1.2 softmax the score data
          1.3 resharp score data back
    */
    for (i = 0; i < score_count; i++)
    {
        vx_float32 score0 = (vx_float32)vxnneGetDataExt(in_score_format, in_score_quant_format, i, (vx_uint8_ptr)score_data, in_score_fp, in_score_zp, in_score_scale);
        vx_float32 score1 = (vx_float32)vxnneGetDataExt(in_score_format, in_score_quant_format, i + score_count, (vx_uint8_ptr)score_data, in_score_fp, in_score_zp, in_score_scale);
        vx_float32 sum = 0.0f, max = gcmMAX(score0, score1);

        score0 -= max;
        score1 -= max;

        score0 = expf(score0);
        score1 = expf(score1);
        sum = score0 + score1;

        /*
            score_buffer:
                0 ~ score_count:                Background scores
                score_count ~ score_count*2:    Foreground scores
        */
        score_buffer[i] = score0 / sum;
        score_buffer[i + score_count] = score1 / sum;
    }
    foreground_score = score_buffer + score_count;

    /*
        2. fill proposal
          2.1 bbox regression
          2.2 filter out too small boxes
          2.3 fill score and bbox into proposal buffer
    */
    p_proposals = proposals;
    for(h=0; h<proposal_height; h++)
    {
        for(w=0; w<proposal_width; w++)
        {
            vx_uint32 x = w * feat_stride;
            vx_uint32 y = h * feat_stride;
            vx_uint32 offset    = h * proposal_width + w;
            vx_uint8_ptr p_box  = bbox_data + offset * vxnneGetTypeSize(in_bbox_format);
            vx_float32 *p_score = foreground_score + offset;

            for(k=0; k<anchor_count; k++)
            {
                vx_float32 dx = vxnneGetDataExt(in_bbox_format, in_bbox_quant_format, (k*4+0)*proposal_area, (vx_uint8_ptr)p_box, in_bbox_fp, in_bbox_zp, in_bbox_scale);
                vx_float32 dy = vxnneGetDataExt(in_bbox_format, in_bbox_quant_format, (k*4+1)*proposal_area, (vx_uint8_ptr)p_box, in_bbox_fp, in_bbox_zp, in_bbox_scale);
                vx_float32 d_log_w = vxnneGetDataExt(in_bbox_format, in_bbox_quant_format, (k*4+2)*proposal_area, (vx_uint8_ptr)p_box, in_bbox_fp, in_bbox_zp, in_bbox_scale);
                vx_float32 d_log_h = vxnneGetDataExt(in_bbox_format, in_bbox_quant_format, (k*4+3)*proposal_area, (vx_uint8_ptr)p_box, in_bbox_fp, in_bbox_zp, in_bbox_scale);

                p_proposals[0] = x + vxnneGetDataExt(in_anchor_format, in_anchor_quant_format, (k*4+0), (vx_uint8_ptr)anchor_data, in_anchor_fp, in_anchor_zp, in_anchor_scale);
                p_proposals[1] = y + vxnneGetDataExt(in_anchor_format, in_anchor_quant_format, (k*4+1), (vx_uint8_ptr)anchor_data, in_anchor_fp, in_anchor_zp, in_anchor_scale);
                p_proposals[2] = x + vxnneGetDataExt(in_anchor_format, in_anchor_quant_format, (k*4+2), (vx_uint8_ptr)anchor_data, in_anchor_fp, in_anchor_zp, in_anchor_scale);
                p_proposals[3] = y + vxnneGetDataExt(in_anchor_format, in_anchor_quant_format, (k*4+3), (vx_uint8_ptr)anchor_data, in_anchor_fp, in_anchor_zp, in_anchor_scale);

                p_proposals[4] = vx_nn_rpn_transform_box(
                                    p_proposals,
                                    dx, dy,
                                    d_log_w, d_log_h,
                                    img_W, img_H,
                                    min_box_W, min_box_H
                                  ) * p_score[k * proposal_area];
                p_proposals += 5;
            }
        }
    }

    /* 3. Sort the proposal buffer */
    vx_nn_rpn_qsort_box(proposals, 0, proposal_count-1, pre_nms_topn);

    /* 4. NMS */
    vx_nn_rpn_nms_cpu(pre_nms_topn, proposals, roi_indices, &real_roi, 0, nms_thresh, post_nms_topn);

    /* 5. Retrieve the rois, output proposal buffer to roi_output & score_output */
    for(i = 0; i < real_roi; i++)
    {
        vx_float32 item_index = 0.0f; /* item_index = input score batch, but we only supported single batch */
        p_proposals = proposals + roi_indices[i] * 5;

        /* Copy proposals coordinate(x1, y1, x2, y2) to roi output tensor */
        vxnneSaveDataExt(out_roi_format, out_roi_quant_format, (i * 5 + 0), item_index, (vx_uint8_ptr)out_roi_data, out_roi_fp, out_roi_zp, out_roi_scale, out_roi_rMode);
        vxnneSaveDataExt(out_roi_format, out_roi_quant_format, (i * 5 + 1), p_proposals[0], (vx_uint8_ptr)out_roi_data, out_roi_fp, out_roi_zp, out_roi_scale, out_roi_rMode);
        vxnneSaveDataExt(out_roi_format, out_roi_quant_format, (i * 5 + 2), p_proposals[1], (vx_uint8_ptr)out_roi_data, out_roi_fp, out_roi_zp, out_roi_scale, out_roi_rMode);
        vxnneSaveDataExt(out_roi_format, out_roi_quant_format, (i * 5 + 3), p_proposals[2], (vx_uint8_ptr)out_roi_data, out_roi_fp, out_roi_zp, out_roi_scale, out_roi_rMode);
        vxnneSaveDataExt(out_roi_format, out_roi_quant_format, (i * 5 + 4), p_proposals[3], (vx_uint8_ptr)out_roi_data, out_roi_fp, out_roi_zp, out_roi_scale, out_roi_rMode);

        /* Copy proposals score to score output tensor */
        if(score_output)
        {
            vxnneSaveDataExt(out_score_format, out_score_quant_format, i, p_proposals[4], (vx_uint8_ptr)out_score_data, out_score_fp, out_score_zp, out_score_scale, out_score_rMode);
        }
    }

    if(score_buffer) vxFree(score_buffer);
    if(proposals) vxFree(proposals);
    if(roi_indices) vxFree(roi_indices);

    if (input_stage)
    {
        vxFree(score_data);
        vxFree(bbox_data);
        vxFree(img_data);
        vxFree(anchor_data);
    }

    if (output_stage)
    {
        vx_uint32 roi_output_size = 0;
        vx_ptr roi_output_logical = VX_NULL;
        vxoTensor_GetTensorSize(roi_output, &roi_output_size);
        vxoTensor_GetTensorViewMemory(roi_output, &roi_output_logical, VX_NULL);
        gcoOS_MemCopy(roi_output_logical, out_roi_data, roi_output_size);

        vxFree(out_roi_data);
    }

    if(score_output && output_stage == vx_true_e)
    {
        vx_uint32 score_output_size = 0;
        vx_ptr score_output_logical = VX_NULL;
        vxoTensor_GetTensorSize(score_output, &score_output_size);
        vxoTensor_GetTensorViewMemory(score_output, &score_output_logical, VX_NULL);
        gcoOS_MemCopy(score_output_logical, out_score_data, score_output_size);

        vxFree(out_score_data);
    }

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNRPNLayer_Initializer_cpu(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status  status                     = VX_SUCCESS;

    vx_tensor  score                      = (vx_tensor)parameters[0];
    vx_tensor  bbox                       = (vx_tensor)parameters[1];
    vx_tensor  anchors                    = (vx_tensor)parameters[2];
    vx_tensor  img_info                   = (vx_tensor)parameters[3];
    vx_scalar  feature_stride             = (vx_scalar)parameters[4];
    vx_scalar  min_size                   = (vx_scalar)parameters[5];
    vx_scalar  pre_nms_topn               = (vx_scalar)parameters[6];
    vx_scalar  post_nms_topn              = (vx_scalar)parameters[7];
    vx_scalar  nms_thresh                 = (vx_scalar)parameters[8];
    vx_tensor  roi_output                 = (vx_tensor)parameters[9];
    vx_tensor  score_output               = (vx_tensor)parameters[10];
    vx_uint32  batchCount                 = TENSOR_SIZE_INDEX(score, 3);

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    {
        vxnne_tensor_rpn_layer rpnLayer;

        gcoOS_Allocate(gcvNULL, sizeof(vxnne_tensor_rpn_layer_s), (gctPOINTER*)&rpnLayer);
        if (!rpnLayer)
        {
            status = VX_ERROR_NO_MEMORY;
            vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            goto exit;
        }

        gcoOS_ZeroMemory(rpnLayer, sizeof(vxnne_tensor_rpn_layer_s));

        vxnneLayer_Initialize(&rpnLayer->base,
            "RpnLayer",
            node,
            vxmOPERATION_COUNT(rpnLayer),
            rpnLayer->operations,
            VX_NULL);

        vxnneOperation_Initialize(&rpnLayer->tensorRpnSW.base,
            &rpnLayer->base,
            VXNNE_OPERATION_TARGET_SW,
            VXNNE_OPERATOR_RPN,
            vxnneExecuteSWRPN,
            VX_NULL,
            batchCount,
            0);

        rpnLayer->tensorRpnSW.score           = score;
        rpnLayer->tensorRpnSW.bbox            = bbox;
        rpnLayer->tensorRpnSW.anchors         = anchors;
        rpnLayer->tensorRpnSW.img_info        = img_info;
        rpnLayer->tensorRpnSW.feature_stride  = feature_stride;
        rpnLayer->tensorRpnSW.min_size        = min_size;
        rpnLayer->tensorRpnSW.pre_nms_topn    = pre_nms_topn;
        rpnLayer->tensorRpnSW.post_nms_topn   = post_nms_topn;
        rpnLayer->tensorRpnSW.nms_thresh      = nms_thresh;
        rpnLayer->tensorRpnSW.roi_output      = roi_output;
        rpnLayer->tensorRpnSW.score_output    = score_output;

        vxnneOperation_AddReference(&rpnLayer->tensorRpnSW.base, (vx_reference)score, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&rpnLayer->tensorRpnSW.base, (vx_reference)bbox, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&rpnLayer->tensorRpnSW.base, (vx_reference)anchors, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&rpnLayer->tensorRpnSW.base, (vx_reference)img_info, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&rpnLayer->tensorRpnSW.base, (vx_reference)roi_output, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        vxnneOperation_AddReference(&rpnLayer->tensorRpnSW.base, (vx_reference)score_output, VXNNE_OPERATION_REFENRENCE_OUTPUT);


        vxnneLayer_SetOperation(
            &rpnLayer->base,
            &rpnLayer->tensorRpnSW.base,
            0);
        //rpnLayer->operations[0] = (vxnne_operation)&rpnLayer->tensorRpnSW;
        node->layer = &rpnLayer->base;
    }

exit:

    return status;
}
/*RPN - SHD*/
vx_status vxnneExecuteSWRPN_Softmax(struct _vxnne_operation_s *operation)
{
    vx_status  status = VX_SUCCESS;
    vxnne_tensor_rpn_softmax_operation rpnSoftmaxOperation = (vxnne_tensor_rpn_softmax_operation)operation;

    vx_tensor input     = rpnSoftmaxOperation->input;
    vx_tensor output    = rpnSoftmaxOperation->output;

    vx_uint32 channel   = TENSOR_VIEW_SIZE_INDEX(input, 2);
    vx_uint32 height    = TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_uint32 width     = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32 count     = width * height * channel / 2;

    vx_type_e in_format     = (vx_type_e)TENSOR_DATA_TYPE(input);
    vx_type_e out_format    = (vx_type_e)TENSOR_DATA_TYPE(output);
    vx_int8 in_fp           = TENSOR_POS(input);
    vx_int8 out_fp          = TENSOR_POS(output);

    vx_enum in_quant_format = TENSOR_QUANT_TYPE(input);
    vx_enum out_quant_format = TENSOR_QUANT_TYPE(output);
    vx_int32 in_zp = TENSOR_TF_ZEROPOINT(input);
    vx_int32 out_zp = TENSOR_TF_ZEROPOINT(output);
    vx_float32 in_scale = TENSOR_TF_SCALE(input);
    vx_float32 out_scale = TENSOR_TF_SCALE(output);

    vx_bool input_stage = rpnSoftmaxOperation->input_stage;
    vx_bool output_stage = rpnSoftmaxOperation->output_stage;

    vx_uint32 i;
    vx_uint8_ptr input_data,output_data;

    vxnneGetTensorMemeory(input, (vx_ptr_ptr)&input_data, input_stage, vx_false_e);
    vxnneGetTensorMemeory(output, (vx_ptr_ptr)&output_data, output_stage, vx_true_e);

    for(i = 0; i < count; i++)
    {
        vx_float32 value0,value1;
        vx_float32 score0 = (vx_float32)vxnneGetDataExt(in_format, in_quant_format, i, (vx_uint8_ptr)input_data, in_fp, in_zp, in_scale);
        vx_float32 score1 = (vx_float32)vxnneGetDataExt(in_format, in_quant_format, i + count, (vx_uint8_ptr)input_data, in_fp, in_zp, in_scale);
        vx_float32 sum = 0.0f, max = gcmMAX(score0, score1);

        score0 -= max;
        score1 -= max;

        score0 = expf(score0);
        score1 = expf(score1);
        sum = score0 + score1;

        value0 = score0 / sum;
        value1 = score1 / sum;
        vxnneSaveDataExt(out_format, out_quant_format, i, value0, (vx_uint8_ptr)output_data, out_fp, out_zp, out_scale, TENSOR_ROUNDING_MODE(output));
        vxnneSaveDataExt(out_format, out_quant_format, (i + count), value1, (vx_uint8_ptr)output_data, out_fp, out_zp, out_scale, TENSOR_ROUNDING_MODE(output));
    }

    if(input_stage)
    {
        vxFree(input_data);
    }
    if(output_stage)
    {
        vx_uint32 output_size = 0;
        vx_ptr output_logical = VX_NULL;
        vxoTensor_GetTensorSize(output, &output_size);
        vxoTensor_GetTensorViewMemory(output, &output_logical, VX_NULL);
        gcoOS_MemCopy(output_logical, output_data, output_size);

        vxFree(output_data);
    }

    return status;
}

vx_status vxnneExecuteSWRPN_Regression(struct _vxnne_operation_s *operation)
{
    vx_status  status = VX_SUCCESS;
    vxnne_tensor_rpn_regression_operation rpnRegOperation = (vxnne_tensor_rpn_regression_operation)operation;

    vx_tensor score         = rpnRegOperation->score_buffer;
    vx_tensor bbox          = rpnRegOperation->bbox;
    vx_tensor anchor        = rpnRegOperation->anchors;
    vx_tensor img_info      = rpnRegOperation->img_info;
    vx_tensor output        = rpnRegOperation->output;

    vx_uint32 feat_stride   = rpnRegOperation->feature_stride->value->u32;
    vx_uint32 min_size      = rpnRegOperation->min_size->value->u32;

    vx_bool input_stage     = rpnRegOperation->input_stage;
    vx_bool output_stage    = rpnRegOperation->output_stage;

    vx_type_e in_score_format   = (vx_type_e)TENSOR_DATA_TYPE(score);
    vx_type_e in_bbox_format    = (vx_type_e)TENSOR_DATA_TYPE(bbox);
    vx_type_e in_anchor_format  = (vx_type_e)TENSOR_DATA_TYPE(anchor);
    vx_type_e in_img_format     = (vx_type_e)TENSOR_DATA_TYPE(img_info);
    vx_type_e output_format     = (vx_type_e)TENSOR_DATA_TYPE(output);

    vx_enum in_score_quant_format = TENSOR_QUANT_TYPE(score);
    vx_enum in_bbox_quant_format = TENSOR_QUANT_TYPE(bbox);
    vx_enum in_anchor_quant_format = TENSOR_QUANT_TYPE(anchor);
    vx_enum in_img_quant_format = TENSOR_QUANT_TYPE(img_info);
    vx_enum output_quant_format = TENSOR_QUANT_TYPE(output);

    vx_int8 in_bbox_fp          = TENSOR_POS(bbox);
    vx_int8 in_anchor_fp        = TENSOR_POS(anchor);
    vx_int8 in_img_fp           = TENSOR_POS(img_info);
    vx_int8 in_score_fp         = TENSOR_POS(score);
    vx_int8 output_fp           = TENSOR_POS(output);

    vx_int32 in_bbox_zp         = TENSOR_TF_ZEROPOINT(bbox);
    vx_int32 in_anchor_zp       = TENSOR_TF_ZEROPOINT(anchor);
    vx_int32 in_img_zp          = TENSOR_TF_ZEROPOINT(img_info);
    vx_int32 in_score_zp        = TENSOR_TF_ZEROPOINT(score);
    vx_int32 output_zp          = TENSOR_TF_ZEROPOINT(output);

    vx_float32 in_bbox_scale    = TENSOR_TF_SCALE(bbox);
    vx_float32 in_anchor_scale  = TENSOR_TF_SCALE(anchor);
    vx_float32 in_img_scale     = TENSOR_TF_SCALE(img_info);
    vx_float32 in_score_scale   = TENSOR_TF_SCALE(score);
    vx_float32 output_scale     = TENSOR_TF_SCALE(output);

    vx_uint32 score_channel = TENSOR_VIEW_SIZE_INDEX(score, 2);
    vx_uint32 score_height  = TENSOR_VIEW_SIZE_INDEX(score, 1);
    vx_uint32 score_width   = TENSOR_VIEW_SIZE_INDEX(score, 0);
    vx_uint32 score_count   = score_width * score_height * score_channel/2;

    vx_uint32 proposal_width    = score_width;
    vx_uint32 proposal_height   = score_height;
    vx_uint32 anchor_count      = TENSOR_VIEW_SIZE_INDEX(anchor, 3); /* anchor batch = anchor number */

    vx_uint32 proposal_area     = proposal_width * proposal_height;

    vx_uint8_ptr bbox_data = VX_NULL, img_data = VX_NULL, anchor_data = VX_NULL, score_data = VX_NULL,out_data = VX_NULL;
    vx_float32_ptr /*foreground_score,*/ proposals, p_proposals;

    vx_float32 img_W,img_H,img_scale_H,img_scale_W,min_box_H,min_box_W;
    vx_uint32 w, h, k;
    vx_float32 tmp_output[5];

    vxmONERROR(vxnneGetTensorMemeory(score, (vx_ptr_ptr)&score_data, input_stage, vx_false_e));
    vxmONERROR(vxnneGetTensorMemeory(bbox, (vx_ptr_ptr)&bbox_data, input_stage, vx_false_e));
    vxmONERROR(vxnneGetTensorMemeory(img_info, (vx_ptr_ptr)&img_data, input_stage, vx_false_e));
    vxmONERROR(vxnneGetTensorMemeory(anchor, (vx_ptr_ptr)&anchor_data, input_stage, vx_false_e));
    vxmONERROR(vxnneGetTensorMemeory(output, (vx_ptr_ptr)&out_data, output_stage, vx_true_e));

    proposals        = (vx_float32_ptr)out_data;

    img_W       = (vx_float32)vxnneGetDataExt(in_img_format, in_img_quant_format, 0, (vx_uint8_ptr)img_data, in_img_fp, in_img_zp, in_img_scale);
    img_H       = (vx_float32)vxnneGetDataExt(in_img_format, in_img_quant_format, 1, (vx_uint8_ptr)img_data, in_img_fp, in_img_zp, in_img_scale);
    img_scale_W = (vx_float32)vxnneGetDataExt(in_img_format, in_img_quant_format, 2, (vx_uint8_ptr)img_data, in_img_fp, in_img_zp, in_img_scale);
    img_scale_H = (vx_float32)vxnneGetDataExt(in_img_format, in_img_quant_format, 3, (vx_uint8_ptr)img_data, in_img_fp, in_img_zp, in_img_scale);
    min_box_W   = min_size * img_scale_W;
    min_box_H   = min_size * img_scale_H;

    p_proposals = proposals;
    for(h=0; h<proposal_height; h++)
    {
        for(w=0; w<proposal_width; w++)
        {
            vx_uint32 x = w * feat_stride;
            vx_uint32 y = h * feat_stride;
            vx_uint32 offset    = h * proposal_width + w;
            vx_uint32 offset_ouptput    = h * proposal_width * anchor_count + w * anchor_count;
            vx_uint8_ptr p_box  = bbox_data + offset * vxnneGetTypeSize(in_bbox_format);
            //vx_float32 *p_score = foreground_score + offset;
            vx_uint8_ptr p_score  = score_data + (offset +  score_count)* vxnneGetTypeSize(in_score_format);

            for(k=0; k<anchor_count; k++)
            {
                vx_float32 dx = vxnneGetDataExt(in_bbox_format, in_bbox_quant_format, (k*4+0)*proposal_area, (vx_uint8_ptr)p_box, in_bbox_fp, in_bbox_zp, in_bbox_scale);
                vx_float32 dy = vxnneGetDataExt(in_bbox_format, in_bbox_quant_format, (k*4+1)*proposal_area, (vx_uint8_ptr)p_box, in_bbox_fp, in_bbox_zp, in_bbox_scale);
                vx_float32 d_log_w = vxnneGetDataExt(in_bbox_format, in_bbox_quant_format, (k*4+2)*proposal_area, (vx_uint8_ptr)p_box, in_bbox_fp, in_bbox_zp, in_bbox_scale);
                vx_float32 d_log_h = vxnneGetDataExt(in_bbox_format, in_bbox_quant_format, (k*4+3)*proposal_area, (vx_uint8_ptr)p_box, in_bbox_fp, in_bbox_zp, in_bbox_scale);

                /* proposals = {x1, y1, x2, y2, score} */
                vx_float32 cur_score = vxnneGetDataExt(in_score_format, in_score_quant_format, k*proposal_area, (vx_uint8_ptr)p_score, in_score_fp, in_score_zp, in_score_scale);

                tmp_output[0] = x + vxnneGetDataExt(in_anchor_format, in_anchor_quant_format, (k*4+0), (vx_uint8_ptr)anchor_data, in_anchor_fp, in_anchor_zp, in_anchor_scale);
                tmp_output[1] = y + vxnneGetDataExt(in_anchor_format, in_anchor_quant_format, (k*4+1), (vx_uint8_ptr)anchor_data, in_anchor_fp, in_anchor_zp, in_anchor_scale);
                tmp_output[2] = x + vxnneGetDataExt(in_anchor_format, in_anchor_quant_format, (k*4+2), (vx_uint8_ptr)anchor_data, in_anchor_fp, in_anchor_zp, in_anchor_scale);
                tmp_output[3] = y + vxnneGetDataExt(in_anchor_format, in_anchor_quant_format, (k*4+3), (vx_uint8_ptr)anchor_data, in_anchor_fp, in_anchor_zp, in_anchor_scale);

                tmp_output[4] = vx_nn_rpn_transform_box(
                                    tmp_output,
                                    dx, dy,
                                    d_log_w, d_log_h,
                                    img_W, img_H,
                                    min_box_W, min_box_H
                                  ) * cur_score;

                vxnneSaveDataExt(output_format, output_quant_format, (offset_ouptput + k) * 5 + 0, tmp_output[0], (vx_uint8_ptr)out_data, output_fp, output_zp, output_scale, TENSOR_ROUNDING_MODE(output));
                vxnneSaveDataExt(output_format, output_quant_format, (offset_ouptput + k) * 5 + 1, tmp_output[1], (vx_uint8_ptr)out_data, output_fp, output_zp, output_scale, TENSOR_ROUNDING_MODE(output));
                vxnneSaveDataExt(output_format, output_quant_format, (offset_ouptput + k) * 5 + 2, tmp_output[2], (vx_uint8_ptr)out_data, output_fp, output_zp, output_scale, TENSOR_ROUNDING_MODE(output));
                vxnneSaveDataExt(output_format, output_quant_format, (offset_ouptput + k) * 5 + 3, tmp_output[3], (vx_uint8_ptr)out_data, output_fp, output_zp, output_scale, TENSOR_ROUNDING_MODE(output));
                vxnneSaveDataExt(output_format, output_quant_format, (offset_ouptput + k) * 5 + 4, tmp_output[4], (vx_uint8_ptr)out_data, output_fp, output_zp, output_scale, TENSOR_ROUNDING_MODE(output));
            }
        }
    }

OnError:
    if (input_stage)
    {
        vxFree(score_data);
        vxFree(bbox_data);
        vxFree(img_data);
        vxFree(anchor_data);
    }

    if (output_stage)
    {
        vx_uint32 output_size = 0;
        vx_ptr output_logical = VX_NULL;
        vxoTensor_GetTensorSize(output, &output_size);
        vxoTensor_GetTensorViewMemory(output, &output_logical, VX_NULL);
        gcoOS_MemCopy(output_logical, out_data, output_size);

        vxFree(out_data);
    }

    return status;
}

vx_status vxnneExecuteSWRPN_Sort(struct _vxnne_operation_s *operation)
{
    vx_status  status = VX_SUCCESS;
    vxnne_tensor_rpn_sort_operation rpnSortOperation = (vxnne_tensor_rpn_sort_operation)operation;
    //vx_uint32 w;

    vx_tensor proposals     = rpnSortOperation->proposal;
    vx_uint32 pre_nms_topn  = rpnSortOperation->pre_nms_topn->value->u32;

    vx_bool output_stage    = rpnSortOperation->output_stage;

    //vx_uint32 proposal_count = TENSOR_VIEW_SIZE_INDEX(proposals, 3);
    vx_uint32 proposal_count =  TENSOR_VIEW_SIZE_INDEX(proposals, 0) * TENSOR_VIEW_SIZE_INDEX(proposals, 1)*
                                TENSOR_VIEW_SIZE_INDEX(proposals, 2) * TENSOR_VIEW_SIZE_INDEX(proposals, 3)/5;
    vx_type_e proposals_data_format   = (vx_type_e)TENSOR_DATA_TYPE(proposals);
    vx_uint8_ptr proposals_data = NULL;
    vxnneGetTensorMemeory(proposals, (vx_ptr_ptr)&proposals_data, output_stage, vx_false_e);

    if (proposals_data_format == VX_TYPE_FLOAT32){
        vx_float32_ptr proposals_ptr = (vx_float32_ptr)proposals_data;
        vx_nn_rpn_qsort_box(proposals_ptr, 0, proposal_count-1, pre_nms_topn);
    }
    else if (proposals_data_format == VX_TYPE_FLOAT16)
    {
        vx_int16_ptr proposals_ptr = (vx_int16_ptr)proposals_data;

        // xzq test

        vx_nn_rpn_qsort_box_fp16(proposals_ptr, 0, proposal_count-1, pre_nms_topn);
    }
    else
    {
        // only surpported F32 and F16 data type
        // todo...
        status = VX_ERROR_INVALID_FORMAT;
        vxError("Not support format %d", proposals_data_format);
    }

    if(output_stage)
    {
        vx_uint32 output_size = 0;
        vx_ptr output_logical = VX_NULL;
        vxoTensor_GetTensorSize(proposals, &output_size);
        vxoTensor_GetTensorViewMemory(proposals, &output_logical, VX_NULL);
        gcoOS_MemCopy(output_logical, proposals_data, output_size);

        vxFree(proposals_data);
    }

    return status;
}

vx_status vxnneExecuteSWRPN_NMS(struct _vxnne_operation_s *operation)
{
    vx_status  status = VX_SUCCESS;
    vxnne_tensor_rpn_nms_operation rpnNmsOperation = (vxnne_tensor_rpn_nms_operation)operation;

    vx_tensor proposal      = rpnNmsOperation->proposal;
    vx_tensor roi_indices   = rpnNmsOperation->roi_indices;
    vx_scalar real_roi_t    = rpnNmsOperation->real_roi_t;
    vx_uint32 pre_nms_topn  = rpnNmsOperation->pre_nms_topn->value->u32;
    vx_uint32 post_nms_topn = rpnNmsOperation->post_nms_topn->value->u32;
    vx_float32 nms_thresh   = rpnNmsOperation->nms_thresh->value->f32;
    vx_bool output_stage    = rpnNmsOperation->output_stage;

    vx_uint32 roi_count         = TENSOR_VIEW_SIZE_INDEX(roi_indices, 0);
    vx_type_e roi_ind_format    = (vx_type_e)TENSOR_DATA_TYPE(roi_indices);
    vx_int8 roi_ind_fp          = TENSOR_POS(roi_indices);
    vx_enum roi_ind_rMode       = TENSOR_ROUNDING_MODE(roi_indices);

    vx_uint8_ptr proposals_data = NULL, roi_indices_data = NULL;

    vx_uint32_ptr roi_indices_ptr = NULL;
    vx_uint32 i,real_roi = 0;
    vx_type_e proposal_data_format   = (vx_type_e)TENSOR_DATA_TYPE(proposal);
    vx_uint32 proposal_count =  TENSOR_VIEW_SIZE_INDEX(proposal, 0) * TENSOR_VIEW_SIZE_INDEX(proposal, 1)*
                                TENSOR_VIEW_SIZE_INDEX(proposal, 2) * TENSOR_VIEW_SIZE_INDEX(proposal, 3)/5;

    roi_indices_ptr = (vx_uint32_ptr)vxAllocateAndZeroMemory(roi_count * sizeof(vx_uint32));

    gcoOS_MemFill(roi_indices_ptr, 0, roi_count * sizeof(vx_uint32));
    vxnneGetTensorMemeory(proposal, (vx_ptr_ptr)&proposals_data, output_stage, vx_false_e);
    vxnneGetTensorMemeory(roi_indices, (vx_ptr_ptr)&roi_indices_data, output_stage, vx_true_e);

    if(pre_nms_topn>proposal_count)
        pre_nms_topn = proposal_count;

    if (proposal_data_format == VX_TYPE_FLOAT32){
        vx_float32_ptr proposals_ptr = (vx_float32_ptr)proposals_data;
        vx_nn_rpn_nms_cpu(pre_nms_topn, proposals_ptr, roi_indices_ptr, &real_roi, 0, nms_thresh, post_nms_topn);
    }
    else if (proposal_data_format == VX_TYPE_FLOAT16){
        vx_int16_ptr proposals_ptr = (vx_int16_ptr)proposals_data;
        vx_nn_rpn_nms_cpu_f16(pre_nms_topn, proposals_ptr, roi_indices_ptr, &real_roi, 0, nms_thresh, post_nms_topn);
    }
    else{
        // only surpported F32 and F16 data type
        // todo...
        status = VX_ERROR_INVALID_FORMAT;
        vxError("Not support format %d", proposal_data_format);
    }
    real_roi_t->value->u32 = real_roi;

    for(i = 0; i < roi_count; i++)
    {
        vxnneSaveDataExt(roi_ind_format, TENSOR_QUANT_TYPE(roi_indices), i, roi_indices_ptr[i], roi_indices_data, roi_ind_fp, TENSOR_TF_ZEROPOINT(roi_indices), TENSOR_TF_SCALE(roi_indices), roi_ind_rMode);
    }

    if(roi_indices_ptr)
        vxFree(roi_indices_ptr);

    if(output_stage)
    {
        vx_uint32 output_size = 0;
        vx_ptr output_logical = VX_NULL;

        vxoTensor_GetTensorSize(proposal, &output_size);
        vxoTensor_GetTensorViewMemory(proposal, &output_logical, VX_NULL);
        gcoOS_MemCopy(output_logical, proposals_data, output_size);

        vxoTensor_GetTensorSize(roi_indices, &output_size);
        vxoTensor_GetTensorViewMemory(roi_indices, &output_logical, VX_NULL);
        gcoOS_MemCopy(output_logical, roi_indices_data, output_size);

        vxFree(proposals_data);
        vxFree(roi_indices_data);
    }

    return status;
}

vx_status vxnneExecuteSWRPN_Retrieve(struct _vxnne_operation_s *operation)
{
    vx_status  status = VX_SUCCESS;
    vxnne_tensor_rpn_retrieve_operation rpnRetOperation = (vxnne_tensor_rpn_retrieve_operation)operation;

    vx_tensor proposal      = rpnRetOperation->proposal;
    vx_tensor roi_indices   = rpnRetOperation->roi_indices;
    vx_scalar real_roi_t    = rpnRetOperation->real_roi_t;
    vx_tensor roi_output    = rpnRetOperation->roi_output;
    vx_tensor score_output  = rpnRetOperation->score_output;

    vx_bool input_stage     = rpnRetOperation->input_stage;
    vx_bool output_stage    = rpnRetOperation->output_stage;

    vx_type_e roi_out_format   = (vx_type_e)TENSOR_DATA_TYPE(roi_output);
    vx_int8 roi_out_fp         = TENSOR_POS(roi_output);
    vx_enum roi_out_rMode      = TENSOR_ROUNDING_MODE(roi_output);
    vx_enum roi_out_quant_format = TENSOR_QUANT_TYPE(roi_output);
    vx_int32 roi_out_zp = TENSOR_TF_ZEROPOINT(roi_output);
    vx_float32 roi_out_scale = TENSOR_TF_SCALE(roi_output);

    vx_uint8_ptr proposal_data = NULL, roi_indices_data = NULL;
    vx_uint8_ptr roi_output_data = NULL;
    vx_float32_ptr roi_indices_ptr = NULL;
    vx_uint32 i,real_roi = 0;

    vx_type_e out_score_format = 0;
    vx_int8 out_score_fp = 0;
    vx_uint8_ptr out_score_data = NULL;
    vx_enum out_score_rMode = 0;
    vx_enum out_score_quant_format = 0;
    vx_int32 out_score_zp = 0;
    vx_float32 out_score_scale = 1.0f;

    vx_type_e proposal_data_format   = (vx_type_e)TENSOR_DATA_TYPE(proposal);
    vxnneGetTensorMemeory(proposal, (vx_ptr_ptr)&proposal_data, input_stage, vx_false_e);
    vxnneGetTensorMemeory(roi_indices, (vx_ptr_ptr)&roi_indices_data, input_stage, vx_false_e);
    vxnneGetTensorMemeory(roi_output, (vx_ptr_ptr)&roi_output_data, output_stage, vx_true_e);

    roi_indices_ptr = (vx_float32_ptr)roi_indices_data;

    if(score_output)
    {
        out_score_format    = (vx_type_e)TENSOR_DATA_TYPE(score_output);
        out_score_fp        = TENSOR_POS(score_output);
        out_score_rMode     = TENSOR_ROUNDING_MODE(score_output);
        vxnneGetTensorMemeory(score_output, (vx_ptr_ptr)&out_score_data, output_stage, vx_true_e);

        out_score_quant_format = TENSOR_QUANT_TYPE(score_output);
        out_score_zp = TENSOR_TF_ZEROPOINT(score_output);
        out_score_scale = TENSOR_TF_SCALE(score_output);
    }


    real_roi = real_roi_t->value->u32;

    for(i = 0; i < real_roi; i++)
    {
        vx_float32 output[5];
        vx_float32 item_index = 0.0f; /* item_index = input score batch, but we only supported single batch */
        vx_float32 findex = roi_indices_ptr[i];
        vx_uint32 index = (vx_uint32)findex;
        if (proposal_data_format == VX_TYPE_FLOAT32){
            vx_float32_ptr proposal_ptr = NULL, p_proposal_ptr = NULL;
            proposal_ptr = (vx_float32_ptr)proposal_data;
            p_proposal_ptr = proposal_ptr + index * 5;

            output[0]= p_proposal_ptr[0];
            output[1]= p_proposal_ptr[1];
            output[2]= p_proposal_ptr[2];
            output[3]= p_proposal_ptr[3];
            output[4]= p_proposal_ptr[4];
        }
        else if (proposal_data_format == VX_TYPE_FLOAT16){
            vx_int16_ptr proposal_ptr = NULL, p_proposal_ptr = NULL;
            proposal_ptr = (vx_int16_ptr)proposal_data;
            p_proposal_ptr = proposal_ptr + index * 5;

            output[0]= Fp16toFp32(p_proposal_ptr[0]);
            output[1]= Fp16toFp32(p_proposal_ptr[1]);
            output[2]= Fp16toFp32(p_proposal_ptr[2]);
            output[3]= Fp16toFp32(p_proposal_ptr[3]);
            output[4]= Fp16toFp32(p_proposal_ptr[4]);
        }
        else{
            // only surpported F32 and F16 data type
            // todo...
            status = VX_ERROR_INVALID_FORMAT;
            vxError("Not support format %d", proposal_data_format);
        }


        /* Copy proposals coordinate(x1, y1, x2, y2) to roi output tensor */
        vxnneSaveDataExt(roi_out_format, roi_out_quant_format, (i * 5 + 0), item_index, (vx_uint8_ptr)roi_output_data, roi_out_fp, roi_out_zp, roi_out_scale, roi_out_rMode);
        vxnneSaveDataExt(roi_out_format, roi_out_quant_format, (i * 5 + 1), output[0], (vx_uint8_ptr)roi_output_data, roi_out_fp, roi_out_zp, roi_out_scale, roi_out_rMode);
        vxnneSaveDataExt(roi_out_format, roi_out_quant_format, (i * 5 + 2), output[1], (vx_uint8_ptr)roi_output_data, roi_out_fp, roi_out_zp, roi_out_scale, roi_out_rMode);
        vxnneSaveDataExt(roi_out_format, roi_out_quant_format, (i * 5 + 3), output[2], (vx_uint8_ptr)roi_output_data, roi_out_fp, roi_out_zp, roi_out_scale, roi_out_rMode);
        vxnneSaveDataExt(roi_out_format, roi_out_quant_format, (i * 5 + 4), output[3], (vx_uint8_ptr)roi_output_data, roi_out_fp, roi_out_zp, roi_out_scale, roi_out_rMode);

        /* Copy proposals score to score output tensor */
        if(score_output)
        {
            vxnneSaveDataExt(out_score_format, out_score_quant_format, i, output[4], (vx_uint8_ptr)out_score_data, out_score_fp, out_score_zp, out_score_scale, out_score_rMode);
        }

    }

    if (input_stage)
    {
        vxFree(proposal_data);
        vxFree(roi_indices_data);
    }

    if (output_stage)
    {
        vx_uint32 roi_output_size = 0;
        vx_ptr roi_output_logical = VX_NULL;
        vxoTensor_GetTensorSize(roi_output, &roi_output_size);
        vxoTensor_GetTensorViewMemory(roi_output, &roi_output_logical, VX_NULL);
        gcoOS_MemCopy(roi_output_logical, roi_output_data, roi_output_size);

        vxFree(roi_output_data);
    }

    if(score_output && output_stage == vx_true_e)
    {
        vx_uint32 score_output_size = 0;
        vx_ptr score_output_logical = VX_NULL;
        vxoTensor_GetTensorSize(score_output, &score_output_size);
        vxoTensor_GetTensorViewMemory(score_output, &score_output_logical, VX_NULL);
        gcoOS_MemCopy(score_output_logical, out_score_data, score_output_size);
    }

    if (out_score_data)
    {
        vxFree(out_score_data);
        out_score_data = VX_NULL;
    }
    return status;
}

vx_status vxnneExecuteSWRPN_SortNMS(struct _vxnne_operation_s *operation)
{
    vx_status  status = VX_SUCCESS;
    vxnne_tensor_rpn_sort_nms_operation rpnSortNmsOperation = (vxnne_tensor_rpn_sort_nms_operation)operation;

    vx_tensor proposal      = rpnSortNmsOperation->proposal;
    vx_tensor roi_output   = rpnSortNmsOperation->roi_output;
    vx_tensor score_output  = rpnSortNmsOperation->score_output;
    vx_uint32 pre_nms_topn  = rpnSortNmsOperation->pre_nms_topn->value->u32;
    vx_uint32 post_nms_topn = rpnSortNmsOperation->post_nms_topn->value->u32;
    vx_float32 nms_thresh   = rpnSortNmsOperation->nms_thresh->value->f32;
    vx_bool output_stage    = rpnSortNmsOperation->output_stage;

    vx_int8 roi_output_fp          = TENSOR_POS(roi_output);
    vx_enum roi_output_rMode       = TENSOR_ROUNDING_MODE(roi_output);

    vx_uint8_ptr proposals_data = NULL, roi_output_data = NULL,score_output_data = NULL;

    vx_uint32 i,real_roi = 0;
    vx_type_e proposal_data_format   = (vx_type_e)TENSOR_DATA_TYPE(proposal);
    vx_uint32 proposal_count =  TENSOR_VIEW_SIZE_INDEX(proposal, 0) * TENSOR_VIEW_SIZE_INDEX(proposal, 1)*
                                TENSOR_VIEW_SIZE_INDEX(proposal, 2) * TENSOR_VIEW_SIZE_INDEX(proposal, 3)/5;


    vx_float32_ptr proposals_ptr = NULL;

    vxnneGetTensorMemeory(proposal, (vx_ptr_ptr)&proposals_data, output_stage, vx_false_e);
    vxnneGetTensorMemeory(roi_output, (vx_ptr_ptr)&roi_output_data, output_stage, vx_false_e);
    vxnneGetTensorMemeory(score_output,(vx_ptr_ptr)&score_output_data, output_stage, vx_false_e);


    if(pre_nms_topn>proposal_count)
        pre_nms_topn = proposal_count;

    if (proposal_data_format == VX_TYPE_FLOAT32){
        proposals_ptr = (vx_float32_ptr)proposals_data;
        vx_nn_rpn_qsort_box(proposals_ptr, 0, proposal_count-1, pre_nms_topn);
        vx_nn_rpn_nms_cpu(pre_nms_topn, proposals_ptr, NULL, &real_roi, 0, nms_thresh, post_nms_topn);
    }
    else if (proposal_data_format == VX_TYPE_FLOAT16){
        proposals_ptr = (vx_float32_ptr)proposals_data;
        proposal_count = proposal_count/2;
        vx_nn_rpn_qsort_box(proposals_ptr, 0, proposal_count-1, pre_nms_topn);
        vx_nn_rpn_nms_cpu(pre_nms_topn, proposals_ptr, NULL, &real_roi, 0, nms_thresh, post_nms_topn);

    }
    else{
        // only surpported F32 and F16 data type
        // todo...
        status = VX_ERROR_INVALID_FORMAT;
        vxError("Not support format %d", proposal_data_format);
    }

    for(i = 0; i < real_roi; i++)
    {
        vx_int32 index = 5*i;
        vx_float32 * ptr = proposals_ptr+index;
        vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(roi_output), TENSOR_QUANT_TYPE(roi_output), index+0, 0, (vx_uint8_ptr)roi_output_data, roi_output_fp, TENSOR_TF_ZEROPOINT(roi_output), TENSOR_TF_SCALE(roi_output), roi_output_rMode);
        vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(roi_output), TENSOR_QUANT_TYPE(roi_output), index+1, ptr[0], (vx_uint8_ptr)roi_output_data, roi_output_fp, TENSOR_TF_ZEROPOINT(roi_output), TENSOR_TF_SCALE(roi_output), roi_output_rMode);
        vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(roi_output), TENSOR_QUANT_TYPE(roi_output), index+2, ptr[1], (vx_uint8_ptr)roi_output_data, roi_output_fp, TENSOR_TF_ZEROPOINT(roi_output), TENSOR_TF_SCALE(roi_output), roi_output_rMode);
        vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(roi_output), TENSOR_QUANT_TYPE(roi_output), index+3, ptr[2], (vx_uint8_ptr)roi_output_data, roi_output_fp, TENSOR_TF_ZEROPOINT(roi_output), TENSOR_TF_SCALE(roi_output), roi_output_rMode);
        vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(roi_output), TENSOR_QUANT_TYPE(roi_output), index+4, ptr[3], (vx_uint8_ptr)roi_output_data, roi_output_fp, TENSOR_TF_ZEROPOINT(roi_output), TENSOR_TF_SCALE(roi_output), roi_output_rMode);

        if(score_output)
        {
            vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(score_output),
                TENSOR_QUANT_TYPE(score_output), i, ptr[4], (vx_uint8_ptr)score_output_data, TENSOR_POS(score_output),
                TENSOR_TF_ZEROPOINT(score_output), TENSOR_TF_SCALE(score_output), TENSOR_ROUNDING_MODE(score_output));
        }
    }

    if(output_stage)
    {
        vx_uint32 output_size = 0;
        vx_ptr output_logical = VX_NULL;

        vxoTensor_GetTensorSize(proposal, &output_size);
        vxoTensor_GetTensorViewMemory(proposal, &output_logical, VX_NULL);
        gcoOS_MemCopy(output_logical, proposals_data, output_size);

        vxoTensor_GetTensorSize(roi_output, &output_size);
        vxoTensor_GetTensorViewMemory(roi_output, &output_logical, VX_NULL);
        gcoOS_MemCopy(output_logical, roi_output_data, output_size);

        if(score_output)
        {
            vxoTensor_GetTensorSize(score_output, &output_size);
            vxoTensor_GetTensorViewMemory(score_output, &output_logical, VX_NULL);
            gcoOS_MemCopy(output_logical, score_output_data, output_size);
        }
        vxFree(proposals_data);
        vxFree(roi_output_data);
    }

    if (score_output_data)
    {
        vxFree(score_output_data);
        score_output_data = VX_NULL;
    }
    return status;
}
/*SHADER+CPU*/
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNRPNLayer_Initializer_shd_cpu(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status  status                     = VX_SUCCESS;

    vx_tensor  score                      = (vx_tensor)parameters[0];
    vx_tensor  bbox                       = (vx_tensor)parameters[1];
    vx_tensor  anchors                    = (vx_tensor)parameters[2];
    vx_tensor  img_info                   = (vx_tensor)parameters[3];
    vx_scalar  feature_stride             = (vx_scalar)parameters[4];
    vx_scalar  min_size                   = (vx_scalar)parameters[5];
    vx_scalar  pre_nms_topn               = (vx_scalar)parameters[6];
    vx_scalar  post_nms_topn              = (vx_scalar)parameters[7];
    vx_scalar  nms_thresh                 = (vx_scalar)parameters[8];
    vx_tensor  roi_output                 = (vx_tensor)parameters[9];
    vx_tensor  score_output               = (vx_tensor)parameters[10];

    vxnne_tensor_rpn_layer rpnLayer;
    vx_uint32 dims,sizes[4] = {0,0,0,0};
    vx_tensor socreBufferTensor = VX_NULL;
    vx_tensor proposalTensor = VX_NULL;


    vx_enum tmpBuf_format   = VX_TYPE_FLOAT32;
    vx_bool input_stage     = vx_true_e;
    vx_bool output_stage    = vx_true_e;

    vx_enum score_format = TENSOR_DATA_TYPE(score);
    vx_enum bbox_format  = TENSOR_DATA_TYPE(bbox);
    vx_enum anchors_format = TENSOR_DATA_TYPE(anchors);

    vx_bool enable_condition0 = (vx_bool)((score_format   == VX_TYPE_FLOAT16)
                                        && (bbox_format    == VX_TYPE_FLOAT16)
                                        && (anchors_format == VX_TYPE_FLOAT32));

    vx_bool enable_condition1 = (vx_bool)((score_format   == VX_TYPE_INT16)
                                        && (bbox_format    == VX_TYPE_INT16)
                                        && (anchors_format == VX_TYPE_FLOAT32));

    vx_bool enable_condition2 = (vx_bool)((score_format   == VX_TYPE_INT8)
                                        && (bbox_format    == VX_TYPE_INT8)
                                        && (anchors_format == VX_TYPE_FLOAT32));

    vx_bool enable_condition3 = (vx_bool)((score_format   == VX_TYPE_UINT8)
                                        && (bbox_format    == VX_TYPE_UINT8)
                                        && (anchors_format == VX_TYPE_FLOAT32));


    vx_bool enable_gpu_soft_max   = (vx_bool)(enable_condition0 || enable_condition1 || enable_condition2 || enable_condition3);
    vx_bool enable_gpu_regression = enable_gpu_soft_max;

    vx_uint32 batchCount = TENSOR_SIZE_INDEX(score, 3);
    vx_tensor_create_params_t tensor_create_params;
    vxnne_operation softmax_op, regression_op, sort_nms_op;

    if(enable_gpu_soft_max)
        tmpBuf_format = VX_TYPE_FLOAT16;
    else
        tmpBuf_format = VX_TYPE_FLOAT32;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    {
        rpnLayer = VX_NULL;
        gcoOS_Allocate(gcvNULL, sizeof(vxnne_tensor_rpn_layer_s), (gctPOINTER*)&rpnLayer);
        if (!rpnLayer)
        {
            status = VX_ERROR_NO_MEMORY;
            vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            goto exit;
        }
        gcoOS_ZeroMemory(rpnLayer, sizeof(vxnne_tensor_rpn_layer_s));
        vxnneLayer_Initialize(&rpnLayer->base,
                                "RpnLayer",
                                node,
                                vxmOPERATION_COUNT(rpnLayer),
                                rpnLayer->operations,
                                VX_NULL);

        /* -----------RPN Softmax------------ */
        /* create a temp tensor to store the scores. */
        dims        = TENSOR_DIM_NUM(score);
        sizes[0]    = TENSOR_SIZE_INDEX(score, 0);
        sizes[1]    = TENSOR_SIZE_INDEX(score, 1);
        sizes[2]    = TENSOR_SIZE_INDEX(score, 2);
        sizes[3]    = TENSOR_SIZE_INDEX(score, 3);

        gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
        tensor_create_params.num_of_dims = dims;
        tensor_create_params.sizes = sizes;
        tensor_create_params.data_format = tmpBuf_format;
        tensor_create_params.quant_format = VX_QUANT_DYNAMIC_FIXED_POINT;

        socreBufferTensor = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);
        if (socreBufferTensor == VX_NULL)
        {
            vxError("vxoTensor_CreateTensor fail at function %s, line %d", __FUNCTION__, __LINE__);
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }

        // 1. gpu softmax process
        if (enable_gpu_soft_max && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
        {
            vxnne_shader_executable shaderExecutable = VX_NULL;
            // reshap tensor objs from dim(4) to dim(3)
            vx_tensor rs_score = NULL, rs_socreBufferTensor = NULL;
            vx_bool rs_score_flag               = (vx_bool)(score->dimCount             == 4);
            vx_bool rs_socreBufferTensor_flag   = (vx_bool)(socreBufferTensor->dimCount == 4);
            if (rs_score_flag){
                vx_int32 new_size[6] = {score->dims[0], score->dims[1],
                                        score->dims[2] * score->dims[3], 1, 1, 1};
                rs_score              = vxoTensor_ReshapeTensor(score, new_size, 3);
            }
            if (rs_socreBufferTensor_flag){
                vx_int32 new_size[6] = {socreBufferTensor->dims[0], socreBufferTensor->dims[1],
                                        socreBufferTensor->dims[2] * socreBufferTensor->dims[3], 1, 1, 1};
                rs_socreBufferTensor = vxoTensor_ReshapeTensor(socreBufferTensor, new_size, 3);
            }
            // --end reshape
            shaderExecutable =
                vxnneRPNSoftMaxShaderExecutable(node->base.context, VXNNE_KERNEL_RPN_SOFTMAX, &node->kernelAttributes.borderMode,
                                                rs_score_flag               ? rs_score              : score,
                                                rs_socreBufferTensor_flag   ? rs_socreBufferTensor  : socreBufferTensor);

            if(rs_score_flag && (NULL != rs_score)){
                vxoTensor_ReleaseTensor(&rs_score);
                rs_score = NULL;
            }
            if(rs_socreBufferTensor_flag && (NULL != rs_socreBufferTensor)){
                vxoTensor_ReleaseTensor(&rs_socreBufferTensor);
                rs_socreBufferTensor = NULL;
            }

            if (!shaderExecutable){
                status = VX_FAILURE;
                goto exit;
            }
            status = vxnneShaderOperation_Initialize(&rpnLayer->tensorRpnSoftmaxSH, &rpnLayer->base,
                                                     VXNNE_OPERATOR_RPN_SOFTMAX, batchCount, shaderExecutable);
            if (status != VX_SUCCESS) goto exit;

            vxnneOperation_AddReference(&rpnLayer->tensorRpnSoftmaxSH.base, (vx_reference)score, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnSoftmaxSH.base, (vx_reference)socreBufferTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }
        else
        {
            vxnneOperation_Initialize(&rpnLayer->tensorRpnSoftmaxSW.base,
                                        &rpnLayer->base,
                                        VXNNE_OPERATION_TARGET_SW,
                                        VXNNE_OPERATOR_RPN_SOFTMAX,
                                        vxnneExecuteSWRPN_Softmax,
                                        VX_NULL,
                                        batchCount,
                                        0);

            rpnLayer->tensorRpnSoftmaxSW.input          = score;
            rpnLayer->tensorRpnSoftmaxSW.output         = socreBufferTensor;

            rpnLayer->tensorRpnSoftmaxSW.input_stage    = input_stage;
            rpnLayer->tensorRpnSoftmaxSW.output_stage   = output_stage;

            vxnneOperation_AddReference(&rpnLayer->tensorRpnSoftmaxSW.base, (vx_reference)score, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnSoftmaxSW.base, (vx_reference)socreBufferTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }

        /* -----------RPN bbox regression------------- */
        /*
            create a temp tensor to store the proposal buffer
            proposal buffer: N*5*1*1
                N: score_width*score_height*anchor_number
                5: 2 sets of coordinates + scores --- (x1, y1, x2, y2, score)
        */
        dims = 3;
 //     sizes[0]    = TENSOR_SIZE_INDEX(score, 0)* 5;
        sizes[0]    = TENSOR_SIZE_INDEX(score, 0)* 5*2;
        sizes[1]    = TENSOR_SIZE_INDEX(score, 1);
        sizes[2]    = TENSOR_SIZE_INDEX(anchors, 3);

        gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
        tensor_create_params.num_of_dims = dims;
        tensor_create_params.sizes = sizes;
        tensor_create_params.data_format = tmpBuf_format;
        tensor_create_params.quant_format = VX_QUANT_DYNAMIC_FIXED_POINT;

        proposalTensor = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);
        if (proposalTensor == VX_NULL)
        {
            vxError("vxoTensor_CreateTensor fail at function %s, line %d", __FUNCTION__, __LINE__);
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }
        if (enable_gpu_regression && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
        {
            vxnne_shader_executable shaderExecutable = VX_NULL;
            vx_type_e in_img_format     = (vx_type_e)TENSOR_DATA_TYPE(img_info);
            vx_int8 in_img_fp           = TENSOR_POS(img_info);
            vx_uint32 min_size_v    = min_size->value->u32;
            vx_uint32 i;
            vx_scalar *pScalarObj = NULL;
            vx_float32 img_W, img_H, min_box_W, min_box_H;

            vx_uint8_ptr img_data;
            vx_float32 img_scale_H,img_scale_W;

            vx_tensor rs_socreBufferTensor = NULL, rs_bbox = NULL, rs_anchors = NULL;
            vx_bool rs_anchors_flag             = (vx_bool)(anchors->dimCount == 4);
            vx_bool rs_box_flag                 = (vx_bool)(bbox->dimCount == 4);
            vx_bool rs_socreBufferTensor_flag   = (vx_bool)(socreBufferTensor->dimCount == 4);

            // reshap tensor objs from dim(4) to dim(3)
            if (rs_socreBufferTensor_flag){
                vx_int32 new_size[6] = {socreBufferTensor->dims[0], socreBufferTensor->dims[1], socreBufferTensor->dims[2] * socreBufferTensor->dims[3], 1, 1, 1};
                rs_socreBufferTensor = vxoTensor_ReshapeTensor(socreBufferTensor, new_size, 3);
            }
            if (rs_box_flag){
                vx_int32 new_size[6] = {bbox->dims[0], bbox->dims[1], bbox->dims[2] * bbox->dims[3], 1, 1, 1};
                rs_bbox = vxoTensor_ReshapeTensor(bbox, new_size, 3);
            }
            if (rs_anchors_flag){

                vx_int32 new_size[6] = {anchors->dims[0]*anchors->dims[1]*anchors->dims[2] * anchors->dims[3], 1, 1, 1, 1, 1};
                rs_anchors = vxoTensor_ReshapeTensor(anchors, new_size, 3);
            }
            // get img_info as input paramerters
            vxnneGetTensorMemeory(img_info, (vx_ptr_ptr)&img_data, vx_false_e, vx_false_e);
            img_W   = (vx_float32)vxnneGetData(in_img_format, 0, (vx_uint8_ptr)img_data, in_img_fp);
            img_H   = (vx_float32)vxnneGetData(in_img_format, 1, (vx_uint8_ptr)img_data, in_img_fp);
            img_scale_W         = (vx_float32)vxnneGetData(in_img_format, 2, (vx_uint8_ptr)img_data, in_img_fp);
            img_scale_H         = (vx_float32)vxnneGetData(in_img_format, 3, (vx_uint8_ptr)img_data, in_img_fp);
            min_box_W   = min_size_v * img_scale_W;
            min_box_H   = min_size_v * img_scale_H;

            pScalarObj = (vx_scalar *)calloc(4, sizeof(vx_scalar));
            pScalarObj[0] = vxCreateScalar(node->base.context, VX_TYPE_FLOAT32, &img_W);
            pScalarObj[1] = vxCreateScalar(node->base.context, VX_TYPE_FLOAT32, &img_H);
            pScalarObj[2] = vxCreateScalar(node->base.context, VX_TYPE_FLOAT32, &min_box_W);
            pScalarObj[3] = vxCreateScalar(node->base.context, VX_TYPE_FLOAT32, &min_box_H);

            shaderExecutable =
                vxnneRPNRegressionShaderExecutable(node->base.context, VXNNE_KERNEL_RPN_REGRESSION, &node->kernelAttributes.borderMode,
                                                rs_socreBufferTensor_flag   ? rs_socreBufferTensor  : socreBufferTensor,
                                                rs_box_flag ? rs_bbox : bbox,
                                                rs_anchors_flag ? rs_anchors : anchors,
                                                proposalTensor,
                                                feature_stride, pScalarObj[0],pScalarObj[1],pScalarObj[2],pScalarObj[3]);

            if(rs_socreBufferTensor_flag && (NULL != rs_socreBufferTensor)){
                vxoTensor_ReleaseTensor(&rs_socreBufferTensor);
                rs_socreBufferTensor = NULL;
            }
            if(rs_box_flag && (NULL != rs_bbox)){
                vxoTensor_ReleaseTensor(&rs_bbox);
                rs_bbox = NULL;
            }
            if(rs_anchors_flag && (NULL != rs_anchors)){
                vxoTensor_ReleaseTensor(&rs_anchors);
                rs_anchors = NULL;
            }

            if (NULL != pScalarObj){
                for(i=0; i< 4; i++){
                    if(NULL != (pScalarObj)[i])
                        vxReleaseScalar(&((pScalarObj)[i]));
                }
                vxFree(pScalarObj);
            }

            if (!shaderExecutable){
                status = VX_FAILURE;
                goto exit;
            }
            status = vxnneShaderOperation_Initialize(
                &rpnLayer->tensorRpnRegressionSH,
                &rpnLayer->base,
                VXNNE_OPERATOR_RPN_REGRESSION,
                batchCount,
                shaderExecutable);

            if (status != VX_SUCCESS) goto exit;

            vxnneOperation_AddReference(&rpnLayer->tensorRpnRegressionSH.base, (vx_reference)socreBufferTensor, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnRegressionSH.base, (vx_reference)bbox, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnRegressionSH.base, (vx_reference)anchors, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnRegressionSH.base, (vx_reference)img_info, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnRegressionSH.base, (vx_reference)proposalTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }
        else
        {
            vxnneOperation_Initialize(&rpnLayer->tensorRpnRegressionSW.base,
                &rpnLayer->base,
                VXNNE_OPERATION_TARGET_SW,
                VXNNE_OPERATOR_RPN_REGRESSION,
                vxnneExecuteSWRPN_Regression,
                VX_NULL,
                batchCount,
                0);

            rpnLayer->tensorRpnRegressionSW.feature_stride  = feature_stride;
            rpnLayer->tensorRpnRegressionSW.min_size        = min_size;
            rpnLayer->tensorRpnRegressionSW.score_buffer    = socreBufferTensor;
            rpnLayer->tensorRpnRegressionSW.bbox            = bbox;
            rpnLayer->tensorRpnRegressionSW.anchors         = anchors;
            rpnLayer->tensorRpnRegressionSW.img_info        = img_info;
            rpnLayer->tensorRpnRegressionSW.output          = proposalTensor;

            vxnneOperation_AddReference(&rpnLayer->tensorRpnRegressionSW.base, (vx_reference)socreBufferTensor, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnRegressionSW.base, (vx_reference)bbox, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnRegressionSW.base, (vx_reference)anchors, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnRegressionSW.base, (vx_reference)img_info, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnRegressionSW.base, (vx_reference)proposalTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            rpnLayer->tensorRpnRegressionSW.input_stage     = input_stage;
            rpnLayer->tensorRpnRegressionSW.output_stage    = output_stage;
        }

// sort and nms
        {
            vxnneOperation_Initialize(&rpnLayer->tensorRpnSortNmsSW.base,
                                        &rpnLayer->base,
                                        VXNNE_OPERATION_TARGET_SW,
                                        VXNNE_OPERATOR_RPN_SORT_NMS,
                                        vxnneExecuteSWRPN_SortNMS,
                                        VX_NULL,
                                        batchCount,
                                        0);

            rpnLayer->tensorRpnSortNmsSW.pre_nms_topn   = pre_nms_topn;
            rpnLayer->tensorRpnSortNmsSW.post_nms_topn  = post_nms_topn;
            rpnLayer->tensorRpnSortNmsSW.nms_thresh     = nms_thresh;
            rpnLayer->tensorRpnSortNmsSW.proposal       = proposalTensor;
            rpnLayer->tensorRpnSortNmsSW.roi_output    = roi_output;
            rpnLayer->tensorRpnSortNmsSW.score_output     = score_output;

            vxnneOperation_AddReference(&rpnLayer->tensorRpnSortNmsSW.base, (vx_reference)proposalTensor, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnSortNmsSW.base, (vx_reference)roi_output, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnSortNmsSW.base, (vx_reference)score_output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            rpnLayer->tensorRpnSortNmsSW.input_stage     = input_stage;
            rpnLayer->tensorRpnSortNmsSW.output_stage    = output_stage;
        }


        rpnLayer->base.num_temp_tensors     = 2;
        rpnLayer->base.temp_tensors[0]      = socreBufferTensor;
        rpnLayer->base.temp_tensors[1]      = proposalTensor;

        if (enable_gpu_soft_max&& (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
            softmax_op         = (vxnne_operation)&rpnLayer->tensorRpnSoftmaxSH.base;
        else
            softmax_op         = (vxnne_operation)&rpnLayer->tensorRpnSoftmaxSW.base;

        if (enable_gpu_regression&& (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
            regression_op       = (vxnne_operation)&rpnLayer->tensorRpnRegressionSH.base;
        else
            regression_op       = (vxnne_operation)&rpnLayer->tensorRpnRegressionSW.base;

         sort_nms_op         = (vxnne_operation)&rpnLayer->tensorRpnSortNmsSW.base;

        vxnneLayer_SetOperation(
            &rpnLayer->base,
            softmax_op,
            0);
        vxnneLayer_SetOperation(
            &rpnLayer->base,
            regression_op,
            1);

        vxnneLayer_SetOperation(
            &rpnLayer->base,
            sort_nms_op,
            2);


        node->layer = &rpnLayer->base;
        return VX_SUCCESS;
    }

exit:
    if (rpnLayer)
    {
        gcoOS_Free(gcvNULL, rpnLayer);
        rpnLayer = VX_NULL;
    }
    return status;
}

/*RPN - ALL SHADER*/
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNRPNLayer_Initializer_shd(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status  status                     = VX_SUCCESS;

    vx_tensor  score                      = (vx_tensor)parameters[0];
    vx_tensor  bbox                       = (vx_tensor)parameters[1];
    vx_tensor  anchors                    = (vx_tensor)parameters[2];
    vx_tensor  img_info                   = (vx_tensor)parameters[3];
    vx_scalar  feature_stride             = (vx_scalar)parameters[4];
    vx_scalar  min_size                   = (vx_scalar)parameters[5];
    vx_scalar  pre_nms_topn               = (vx_scalar)parameters[6];
    vx_scalar  post_nms_topn              = (vx_scalar)parameters[7];
    vx_scalar  nms_thresh                 = (vx_scalar)parameters[8];
    vx_tensor  roi_output                 = (vx_tensor)parameters[9];
    vx_tensor  score_output               = (vx_tensor)parameters[10];

    vxnne_tensor_rpn_layer rpnLayer;
    vx_uint32 dims,sizes[4] = {0,0,0,0};
    vx_tensor socreBufferTensor = VX_NULL;
    vx_tensor proposalTensor = VX_NULL;
    vx_tensor roiIndicesTensor = VX_NULL;
    vx_scalar realRoiScalar = VX_NULL;
    vx_uint32 realRoi = 0;

    vx_enum temp_format     = VX_TYPE_FLOAT32; /* To avoid the loss of accuracy */
    vx_enum tmpBuf_format   = VX_TYPE_FLOAT16;
    vx_bool input_stage     = vx_true_e;
    vx_bool output_stage    = vx_true_e;

    vx_enum score_format = TENSOR_DATA_TYPE(score);
    vx_enum bbox_format  = TENSOR_DATA_TYPE(bbox);
    vx_enum anchors_format = TENSOR_DATA_TYPE(anchors);

    vx_bool enable_condition0 = (vx_bool)((score_format   == VX_TYPE_FLOAT16)
                                        && (bbox_format    == VX_TYPE_FLOAT16)
                                        && (anchors_format == VX_TYPE_FLOAT32));

    vx_bool enable_condition1 = (vx_bool)((score_format   == VX_TYPE_INT8)
                                        && (bbox_format    == VX_TYPE_INT8)
                                        && (anchors_format == VX_TYPE_FLOAT32));

    vx_bool enable_condition2 = (vx_bool)((score_format   == VX_TYPE_UINT8)
                                        && (bbox_format    == VX_TYPE_UINT8)
                                        && (anchors_format == VX_TYPE_FLOAT32));

    vx_bool enable_condition3 = (vx_bool)((score_format   == VX_TYPE_INT16)
                                        && (bbox_format    == VX_TYPE_INT16)
                                        && (anchors_format == VX_TYPE_FLOAT32));

    vx_bool enable_gpu_soft_max   = (vx_bool)(enable_condition0 || enable_condition1 || enable_condition2 || enable_condition3);
    vx_bool enable_gpu_regression = enable_gpu_soft_max  ;
    vx_bool enable_gpu_nms = vx_false_e;
    vx_bool enable_gpu_retrive = vx_false_e;
    vx_bool enable_gpu_sort = vx_false_e;
    vx_uint32 batchCount = TENSOR_SIZE_INDEX(score, 3);
    vx_tensor_create_params_t tensor_create_params;
    vxnne_operation softmax_op, regression_op, sort_op, nms_op, retrieve_op;


    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    {
        rpnLayer = VX_NULL;
        gcoOS_Allocate(gcvNULL, sizeof(vxnne_tensor_rpn_layer_s), (gctPOINTER*)&rpnLayer);
        if (!rpnLayer)
        {
            status = VX_ERROR_NO_MEMORY;
            vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            goto exit;
        }
        gcoOS_ZeroMemory(rpnLayer, sizeof(vxnne_tensor_rpn_layer_s));
        vxnneLayer_Initialize(&rpnLayer->base,
                                "RpnLayer",
                                node,
                                vxmOPERATION_COUNT(rpnLayer),
                                rpnLayer->operations,
                                VX_NULL);

        /* -----------RPN Softmax------------ */
        /* create a temp tensor to store the scores. */
        dims        = TENSOR_DIM_NUM(score);
        sizes[0]    = TENSOR_SIZE_INDEX(score, 0);
        sizes[1]    = TENSOR_SIZE_INDEX(score, 1);
        sizes[2]    = TENSOR_SIZE_INDEX(score, 2);
        sizes[3]    = TENSOR_SIZE_INDEX(score, 3);

        gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
        tensor_create_params.num_of_dims = dims;
        tensor_create_params.sizes = sizes;
        tensor_create_params.data_format = tmpBuf_format;
        tensor_create_params.quant_format = VX_QUANT_DYNAMIC_FIXED_POINT;

        socreBufferTensor = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);
        if (socreBufferTensor == VX_NULL)
        {
            vxError("vxoTensor_CreateTensor fail at function %s, line %d", __FUNCTION__, __LINE__);
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }

        // 1. gpu softmax process
        if (enable_gpu_soft_max && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
        {
            vxnne_shader_executable shaderExecutable = VX_NULL;
            // reshap tensor objs from dim(4) to dim(3)
            vx_tensor rs_score = NULL, rs_socreBufferTensor = NULL;
            vx_bool rs_score_flag               = (vx_bool)(score->dimCount             == 4);
            vx_bool rs_socreBufferTensor_flag   = (vx_bool)(socreBufferTensor->dimCount == 4);
            if (rs_score_flag){
                vx_int32 new_size[6] = {score->dims[0], score->dims[1],
                                        score->dims[2] * score->dims[3], 1, 1, 1};
                rs_score              = vxoTensor_ReshapeTensor(score, new_size, 3);
            }
            if (rs_socreBufferTensor_flag){
                vx_int32 new_size[6] = {socreBufferTensor->dims[0], socreBufferTensor->dims[1],
                                        socreBufferTensor->dims[2] * socreBufferTensor->dims[3], 1, 1, 1};
                rs_socreBufferTensor = vxoTensor_ReshapeTensor(socreBufferTensor, new_size, 3);
            }
            // --end reshape
            shaderExecutable =
                vxnneRPNSoftMaxShaderExecutable(node->base.context, VXNNE_KERNEL_RPN_SOFTMAX, &node->kernelAttributes.borderMode,
                                                rs_score_flag               ? rs_score              : score,
                                                rs_socreBufferTensor_flag   ? rs_socreBufferTensor  : socreBufferTensor);

            if(rs_score_flag && (NULL != rs_score)){
                vxoTensor_ReleaseTensor(&rs_score);
                rs_score = NULL;
            }
            if(rs_socreBufferTensor_flag && (NULL != rs_socreBufferTensor)){
                vxoTensor_ReleaseTensor(&rs_socreBufferTensor);
                rs_socreBufferTensor = NULL;
            }

            if (!shaderExecutable){
                status = VX_FAILURE;
                goto exit;
            }
            status = vxnneShaderOperation_Initialize(&rpnLayer->tensorRpnSoftmaxSH, &rpnLayer->base,
                                                     VXNNE_OPERATOR_RPN_SOFTMAX, batchCount, shaderExecutable);
            if (status != VX_SUCCESS) goto exit;

            vxnneOperation_AddReference(&rpnLayer->tensorRpnSoftmaxSH.base, (vx_reference)score, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnSoftmaxSH.base, (vx_reference)socreBufferTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }
        else
        {
            vxnneOperation_Initialize(&rpnLayer->tensorRpnSoftmaxSW.base,
                                        &rpnLayer->base,
                                        VXNNE_OPERATION_TARGET_SW,
                                        VXNNE_OPERATOR_RPN_SOFTMAX,
                                        vxnneExecuteSWRPN_Softmax,
                                        VX_NULL,
                                        batchCount,
                                        0);

            rpnLayer->tensorRpnSoftmaxSW.input          = score;
            rpnLayer->tensorRpnSoftmaxSW.output         = socreBufferTensor;

            rpnLayer->tensorRpnSoftmaxSW.input_stage    = input_stage;
            rpnLayer->tensorRpnSoftmaxSW.output_stage   = output_stage;

            vxnneOperation_AddReference(&rpnLayer->tensorRpnSoftmaxSW.base, (vx_reference)score, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnSoftmaxSW.base, (vx_reference)socreBufferTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }

        /* -----------RPN bbox regression------------- */
        /*
            create a temp tensor to store the proposal buffer
            proposal buffer: N*5*1*1
                N: score_width*score_height*anchor_number
                5: 2 sets of coordinates + scores --- (x1, y1, x2, y2, score)
        */
        dims = 3;
        sizes[0]    = TENSOR_SIZE_INDEX(score, 0)* 5;
        sizes[1]    = TENSOR_SIZE_INDEX(score, 1);
        sizes[2]    = TENSOR_SIZE_INDEX(anchors, 3);

        gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
        tensor_create_params.num_of_dims = dims;
        tensor_create_params.sizes = sizes;
        tensor_create_params.data_format = tmpBuf_format;
        tensor_create_params.quant_format = VX_QUANT_DYNAMIC_FIXED_POINT;

        proposalTensor = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);
        if (proposalTensor == VX_NULL)
        {
            vxError("vxoTensor_CreateTensor fail at function %s, line %d", __FUNCTION__, __LINE__);
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }
        if (enable_gpu_regression && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
        {
            vxnne_shader_executable shaderExecutable = VX_NULL;
            vx_type_e in_img_format     = (vx_type_e)TENSOR_DATA_TYPE(img_info);
            vx_int8 in_img_fp           = TENSOR_POS(img_info);
            vx_uint32 min_size_v    = min_size->value->u32;
            vx_uint32 i;
            vx_scalar *pScalarObj = NULL;
            vx_float32 img_W, img_H, min_box_W, min_box_H;

            vx_uint8_ptr img_data;
            vx_float32 img_scale_H,img_scale_W;

            vx_tensor rs_socreBufferTensor = NULL, rs_bbox = NULL, rs_anchors = NULL;
            vx_bool rs_anchors_flag             = (vx_bool)(anchors->dimCount == 4);
            vx_bool rs_box_flag                 = (vx_bool)(bbox->dimCount == 4);
            vx_bool rs_socreBufferTensor_flag   = (vx_bool)(socreBufferTensor->dimCount == 4);

            // reshap tensor objs from dim(4) to dim(3)
            if (rs_socreBufferTensor_flag){
                vx_int32 new_size[6] = {socreBufferTensor->dims[0], socreBufferTensor->dims[1], socreBufferTensor->dims[2] * socreBufferTensor->dims[3], 1, 1, 1};
                rs_socreBufferTensor = vxoTensor_ReshapeTensor(socreBufferTensor, new_size, 3);
            }
            if (rs_box_flag){
                vx_int32 new_size[6] = {bbox->dims[0], bbox->dims[1], bbox->dims[2] * bbox->dims[3], 1, 1, 1};
                rs_bbox = vxoTensor_ReshapeTensor(bbox, new_size, 3);
            }
            if (rs_anchors_flag){

                vx_int32 new_size[6] = {anchors->dims[0]*anchors->dims[1]*anchors->dims[2] * anchors->dims[3], 1, 1, 1, 1, 1};
                rs_anchors = vxoTensor_ReshapeTensor(anchors, new_size, 3);

            }
            // get img_info as input paramerters
            vxnneGetTensorMemeory(img_info, (vx_ptr_ptr)&img_data, vx_false_e, vx_false_e);
            img_W   = (vx_float32)vxnneGetData(in_img_format, 0, (vx_uint8_ptr)img_data, in_img_fp);
            img_H   = (vx_float32)vxnneGetData(in_img_format, 1, (vx_uint8_ptr)img_data, in_img_fp);
            img_scale_W         = (vx_float32)vxnneGetData(in_img_format, 2, (vx_uint8_ptr)img_data, in_img_fp);
            img_scale_H         = (vx_float32)vxnneGetData(in_img_format, 3, (vx_uint8_ptr)img_data, in_img_fp);
            min_box_W   = min_size_v * img_scale_W;
            min_box_H   = min_size_v * img_scale_H;

            pScalarObj = (vx_scalar *)calloc(4, sizeof(vx_scalar));
            pScalarObj[0] = vxCreateScalar(node->base.context, VX_TYPE_FLOAT32, &img_W);
            pScalarObj[1] = vxCreateScalar(node->base.context, VX_TYPE_FLOAT32, &img_H);
            pScalarObj[2] = vxCreateScalar(node->base.context, VX_TYPE_FLOAT32, &min_box_W);
            pScalarObj[3] = vxCreateScalar(node->base.context, VX_TYPE_FLOAT32, &min_box_H);

            shaderExecutable =
                vxnneRPNRegressionShaderExecutable(node->base.context, VXNNE_KERNEL_RPN_REGRESSION, &node->kernelAttributes.borderMode,
                                                rs_socreBufferTensor_flag   ? rs_socreBufferTensor  : socreBufferTensor,
                                                rs_box_flag ? rs_bbox : bbox,
                                                rs_anchors_flag ? rs_anchors : anchors,
                                                proposalTensor,
                                                feature_stride, pScalarObj[0],pScalarObj[1],pScalarObj[2],pScalarObj[3]);

            if(rs_socreBufferTensor_flag && (NULL != rs_socreBufferTensor)){
                vxoTensor_ReleaseTensor(&rs_socreBufferTensor);
                rs_socreBufferTensor = NULL;
            }
            if(rs_box_flag && (NULL != rs_bbox)){
                vxoTensor_ReleaseTensor(&rs_bbox);
                rs_bbox = NULL;
            }
            if(rs_anchors_flag && (NULL != rs_anchors)){
                vxoTensor_ReleaseTensor(&rs_anchors);
                rs_anchors = NULL;
            }

            if (NULL != pScalarObj){
                for(i=0; i< 4; i++){
                    if(NULL != (pScalarObj)[i])
                        vxReleaseScalar(&((pScalarObj)[i]));
                }
                vxFree(pScalarObj);
            }

            if (!shaderExecutable){
                status = VX_FAILURE;
                goto exit;
            }
            status = vxnneShaderOperation_Initialize(
                &rpnLayer->tensorRpnRegressionSH,
                &rpnLayer->base,
                VXNNE_OPERATOR_RPN_REGRESSION,
                batchCount,
                shaderExecutable);

            if (status != VX_SUCCESS) goto exit;

            vxnneOperation_AddReference(&rpnLayer->tensorRpnRegressionSH.base, (vx_reference)socreBufferTensor, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnRegressionSH.base, (vx_reference)bbox, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnRegressionSH.base, (vx_reference)anchors, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnRegressionSH.base, (vx_reference)img_info, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnRegressionSH.base, (vx_reference)proposalTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }
        else
        {
            vxnneOperation_Initialize(&rpnLayer->tensorRpnRegressionSW.base,
                &rpnLayer->base,
                VXNNE_OPERATION_TARGET_SW,
                VXNNE_OPERATOR_RPN_REGRESSION,
                vxnneExecuteSWRPN_Regression,
                VX_NULL,
                batchCount,
                0);

            rpnLayer->tensorRpnRegressionSW.feature_stride  = feature_stride;
            rpnLayer->tensorRpnRegressionSW.min_size        = min_size;
            rpnLayer->tensorRpnRegressionSW.score_buffer    = socreBufferTensor;
            rpnLayer->tensorRpnRegressionSW.bbox            = bbox;
            rpnLayer->tensorRpnRegressionSW.anchors         = anchors;
            rpnLayer->tensorRpnRegressionSW.img_info        = img_info;
            rpnLayer->tensorRpnRegressionSW.output          = proposalTensor;

            vxnneOperation_AddReference(&rpnLayer->tensorRpnRegressionSW.base, (vx_reference)socreBufferTensor, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnRegressionSW.base, (vx_reference)bbox, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnRegressionSW.base, (vx_reference)anchors, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnRegressionSW.base, (vx_reference)img_info, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnRegressionSW.base, (vx_reference)proposalTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            rpnLayer->tensorRpnRegressionSW.input_stage     = input_stage;
            rpnLayer->tensorRpnRegressionSW.output_stage    = output_stage;
        }

        /* ------------RPN Sort--------------------- */
        if (enable_gpu_sort == vx_false_e)
        {
            enable_gpu_sort = vx_true_e;
            if (TENSOR_DATA_TYPE(proposalTensor) != VX_TYPE_FLOAT16)
                enable_gpu_sort = vx_false_e;
        }

        if (enable_gpu_sort && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
        {
            vxnne_shader_executable shaderExecutable = VX_NULL;
            vx_tensor rs_proposalTensor = NULL;
            if (proposalTensor != NULL)
            {
                vx_int32 len = proposalTensor->dims[0] * proposalTensor->dims[1] * proposalTensor->dims[2] * proposalTensor->dims[3];
                vx_int32 new_size[6] = {5, len/5, 1, 1, 1, 1};
                rs_proposalTensor = vxoTensor_ReshapeTensor(proposalTensor, new_size, 3);
            }
            shaderExecutable =  vxnneRPNSortShaderExecutable(
                                node->base.context,
                                VXNNE_KERNEL_RPN_SORT,
                                &node->kernelAttributes.borderMode,
                                rs_proposalTensor);

            if(NULL != rs_proposalTensor){
                vxoTensor_ReleaseTensor(&rs_proposalTensor);
                rs_proposalTensor = NULL;
            }

            if (!shaderExecutable){
                status = VX_FAILURE;
                goto exit;
            }
            status = vxnneShaderOperation_Initialize(
                &rpnLayer->tensorRpnSortSH,
                &rpnLayer->base,
                VXNNE_OPERATOR_RPN_SORT,
                batchCount,
                shaderExecutable);

            if (status != VX_SUCCESS) goto exit;

            vxnneOperation_AddReference(&rpnLayer->tensorRpnSortSH.base, (vx_reference)proposalTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }
        else
        {
            vxnneOperation_Initialize(&rpnLayer->tensorRpnSortSW.base,
                                        &rpnLayer->base,
                                        VXNNE_OPERATION_TARGET_SW,
                                        VXNNE_OPERATOR_RPN_SORT,
                                        vxnneExecuteSWRPN_Sort,
                                        VX_NULL,
                                        batchCount,
                                        0);

            rpnLayer->tensorRpnSortSW.pre_nms_topn  = pre_nms_topn;
            rpnLayer->tensorRpnSortSW.proposal      = proposalTensor;

            vxnneOperation_AddReference(&rpnLayer->tensorRpnSortSW.base, (vx_reference)proposalTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            rpnLayer->tensorRpnSortSW.input_stage     = input_stage;
            rpnLayer->tensorRpnSortSW.output_stage    = output_stage;
        }

        /* ------------RPN NMS--------------------- */
        /* create a temp tensor to store the index of the nms's proposal */
        dims        = 3;
        sizes[0]    = post_nms_topn->value->u32;
        sizes[1]    = 1;
        sizes[2]    = 1;
        sizes[3]    = 1;

        gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
        tensor_create_params.num_of_dims = dims;
        tensor_create_params.sizes = sizes;
        tensor_create_params.data_format = temp_format;
        tensor_create_params.quant_format = VX_QUANT_DYNAMIC_FIXED_POINT;

        roiIndicesTensor = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);
        if (roiIndicesTensor == VX_NULL)
        {
            vxError("vxoTensor_CreateTensor fail at function %s, line %d", __FUNCTION__, __LINE__);
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }

        /* create a temp tensor to store the real proposal number. because maybe real_roi < post_nms_topn */
        realRoiScalar = vxCreateScalar(node->base.context,VX_TYPE_UINT32,&realRoi);
        if (realRoiScalar==NULL)
        {
            vxError("vxCreateScalar fail at function %s, line %d", __FUNCTION__, __LINE__);
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }

        if (enable_gpu_nms == vx_false_e)
        {
            enable_gpu_nms = vx_true_e;
            if (TENSOR_DATA_TYPE(proposalTensor) != VX_TYPE_FLOAT16)
                enable_gpu_nms = vx_false_e;
            if (TENSOR_DATA_TYPE(roiIndicesTensor) != VX_TYPE_FLOAT32)
                enable_gpu_nms = vx_false_e;
        }
        if(enable_gpu_nms && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
        {
            vxnne_shader_executable shaderExecutable = VX_NULL;
            vx_tensor rs_proposalTensor = NULL;
            if (proposalTensor != NULL)
            {
                vx_int32 len = proposalTensor->dims[0] * proposalTensor->dims[1] * proposalTensor->dims[2] * proposalTensor->dims[3];
                vx_int32 new_size[6] = {5, len/5, 1, 1, 1, 1};
                rs_proposalTensor = vxoTensor_ReshapeTensor(proposalTensor, new_size, 3);
            }

            shaderExecutable =  vxnneRPNNmsShaderExecutable(
                                node->base.context,
                                VXNNE_KERNEL_RPN_NMS,
                                &node->kernelAttributes.borderMode,
                                 rs_proposalTensor,
                                 roiIndicesTensor,
                                 realRoiScalar,
                                 pre_nms_topn,
                                 post_nms_topn,
                                 nms_thresh);
            if(rs_proposalTensor)
                vxoTensor_ReleaseTensor(&rs_proposalTensor);

            if (!shaderExecutable){
                status = VX_FAILURE;
                goto exit;
            }
            status = vxnneShaderOperation_Initialize(&rpnLayer->tensorRpnNmsSH,
                                                      &rpnLayer->base,
                                                     VXNNE_OPERATOR_RPN_NMS,
                                                     batchCount,
                                                     shaderExecutable);
            if (status != VX_SUCCESS) goto exit;

            vxnneOperation_AddReference(&rpnLayer->tensorRpnNmsSH.base, (vx_reference)proposalTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnNmsSH.base, (vx_reference)roiIndicesTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }
        else
        {
            vxnneOperation_Initialize(&rpnLayer->tensorRpnNmsSW.base,
                                        &rpnLayer->base,
                                        VXNNE_OPERATION_TARGET_SW,
                                        VXNNE_OPERATOR_RPN_NMS,
                                        vxnneExecuteSWRPN_NMS,
                                        VX_NULL,
                                        batchCount,
                                        0);

            rpnLayer->tensorRpnNmsSW.pre_nms_topn   = pre_nms_topn;
            rpnLayer->tensorRpnNmsSW.post_nms_topn  = post_nms_topn;
            rpnLayer->tensorRpnNmsSW.nms_thresh     = nms_thresh;
            rpnLayer->tensorRpnNmsSW.proposal       = proposalTensor;
            rpnLayer->tensorRpnNmsSW.roi_indices    = roiIndicesTensor;
            rpnLayer->tensorRpnNmsSW.real_roi_t     = realRoiScalar;

            vxnneOperation_AddReference(&rpnLayer->tensorRpnNmsSW.base, (vx_reference)proposalTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnNmsSW.base, (vx_reference)roiIndicesTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            rpnLayer->tensorRpnNmsSW.input_stage     = input_stage;
            rpnLayer->tensorRpnNmsSW.output_stage    = output_stage;
        }

        /* ------------RPN Retrieve--------------------- */
        if (enable_gpu_retrive == vx_false_e)
        {
            enable_gpu_retrive = vx_true_e;
            if (TENSOR_DATA_TYPE(proposalTensor) != VX_TYPE_FLOAT16)
                enable_gpu_retrive = vx_false_e;
            if (TENSOR_DATA_TYPE(roiIndicesTensor) != VX_TYPE_FLOAT32)
                enable_gpu_retrive = vx_false_e;
            if (TENSOR_DATA_TYPE(roi_output) != VX_TYPE_FLOAT16)
                enable_gpu_retrive = vx_false_e;
            if (score_output != NULL)
                if(TENSOR_DATA_TYPE(score_output) != VX_TYPE_FLOAT16)
                    enable_gpu_retrive = vx_false_e;
        }
        if (enable_gpu_retrive && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
        {
            vxnne_shader_executable shaderExecutable = VX_NULL;
            vx_tensor rs_proposalTensor = NULL, rs_roi_output = NULL, rs_score_output = NULL;
            if (proposalTensor != NULL)
            {
                vx_int32 len = proposalTensor->dims[0] * proposalTensor->dims[1] * proposalTensor->dims[2] * proposalTensor->dims[3];
                vx_int32 new_size[6] = {5, len/5, 1, 1, 1, 1};
                rs_proposalTensor = vxoTensor_ReshapeTensor(proposalTensor, new_size, 3);
            }
            if (roi_output != NULL)
            {
                vx_int32 len = roi_output->dims[0] * roi_output->dims[1] * roi_output->dims[2] * roi_output->dims[3];
                vx_int32 new_size[6] = {5, len/5, 1, 1, 1, 1};
                rs_roi_output = vxoTensor_ReshapeTensor(roi_output, new_size, 3);
            }
            if (score_output != NULL)
            {
                vx_int32 len = score_output->dims[0] * score_output->dims[1] * score_output->dims[2] * score_output->dims[3];
                vx_int32 new_size[6] = {len, 1, 1, 1, 1, 1};
                rs_score_output = vxoTensor_ReshapeTensor(score_output, new_size, 3);
            }
            shaderExecutable =  vxnneRPNRetrieveShaderExecutable(
                                node->base.context,
                                VXNNE_KERNEL_RPN_RETRIEVE,
                                &node->kernelAttributes.borderMode,
                                rs_proposalTensor,
                                roiIndicesTensor,
                                realRoiScalar,
                                rs_roi_output,
                                rs_score_output);

            if(NULL != rs_proposalTensor){
                vxoTensor_ReleaseTensor(&rs_proposalTensor);
                rs_proposalTensor = NULL;
            }
            if(NULL != rs_roi_output){
                vxoTensor_ReleaseTensor(&rs_roi_output);
                rs_roi_output = NULL;
            }
            if(NULL != rs_score_output){
                vxoTensor_ReleaseTensor(&rs_score_output);
                rs_score_output = NULL;
            }

            if (!shaderExecutable){
                status = VX_FAILURE;
                goto exit;
            }
            status = vxnneShaderOperation_Initialize(&rpnLayer->tensorRpnRetrieveSH, &rpnLayer->base,
                VXNNE_OPERATOR_RPN_RETRIEVE, batchCount, shaderExecutable);
            if (status != VX_SUCCESS) goto exit;

            vxnneOperation_AddReference(&rpnLayer->tensorRpnRetrieveSH.base, (vx_reference)proposalTensor, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnRetrieveSH.base, (vx_reference)roiIndicesTensor, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnRetrieveSH.base, (vx_reference)roi_output, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnRetrieveSH.base, (vx_reference)score_output, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }
        else
        {
            vxnneOperation_Initialize(&rpnLayer->tensorRpnRetrieveSW.base,
                                        &rpnLayer->base,
                                        VXNNE_OPERATION_TARGET_SW,
                                        VXNNE_OPERATOR_RPN_RETRIEVE,
                                        vxnneExecuteSWRPN_Retrieve,
                                        VX_NULL,
                                        batchCount,
                                        0);

            rpnLayer->tensorRpnRetrieveSW.proposal       = proposalTensor;
            rpnLayer->tensorRpnRetrieveSW.roi_indices    = roiIndicesTensor;
            rpnLayer->tensorRpnRetrieveSW.real_roi_t     = realRoiScalar;
            rpnLayer->tensorRpnRetrieveSW.roi_output     = roi_output;
            rpnLayer->tensorRpnRetrieveSW.score_output   = score_output;

            vxnneOperation_AddReference(&rpnLayer->tensorRpnRetrieveSW.base, (vx_reference)proposalTensor, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnRetrieveSW.base, (vx_reference)roiIndicesTensor, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnRetrieveSW.base, (vx_reference)roi_output, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            vxnneOperation_AddReference(&rpnLayer->tensorRpnRetrieveSW.base, (vx_reference)score_output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            rpnLayer->tensorRpnRetrieveSW.input_stage    = input_stage;
            rpnLayer->tensorRpnRetrieveSW.output_stage   = output_stage;
        }
        /* --------------------------------- */

        rpnLayer->base.num_temp_tensors     = 3;
        rpnLayer->base.temp_tensors[0]      = socreBufferTensor;
        rpnLayer->base.temp_tensors[1]      = proposalTensor;
        rpnLayer->base.temp_tensors[2]      = roiIndicesTensor;


        if (enable_gpu_soft_max&& (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
            softmax_op         = (vxnne_operation)&rpnLayer->tensorRpnSoftmaxSH.base;
        else
            softmax_op         = (vxnne_operation)&rpnLayer->tensorRpnSoftmaxSW.base;

        if (enable_gpu_regression&& (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
            regression_op       = (vxnne_operation)&rpnLayer->tensorRpnRegressionSH.base;
        else
            regression_op       = (vxnne_operation)&rpnLayer->tensorRpnRegressionSW.base;

        if (enable_gpu_sort&& (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
            sort_op         = (vxnne_operation)&rpnLayer->tensorRpnSortSH.base;
        else
            sort_op         = (vxnne_operation)&rpnLayer->tensorRpnSortSW.base;

        if (enable_gpu_nms&& (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
            nms_op         = (vxnne_operation)&rpnLayer->tensorRpnNmsSH.base;
        else
            nms_op         = (vxnne_operation)&rpnLayer->tensorRpnNmsSW.base;


        if (enable_gpu_retrive&& (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
            retrieve_op         = (vxnne_operation)&rpnLayer->tensorRpnRetrieveSH.base;
        else
            retrieve_op         = (vxnne_operation)&rpnLayer->tensorRpnRetrieveSW.base;

        vxnneLayer_SetOperation(
            &rpnLayer->base,
            softmax_op,
            0);
        vxnneLayer_SetOperation(
            &rpnLayer->base,
            regression_op,
            1);
        vxnneLayer_SetOperation(
            &rpnLayer->base,
            sort_op,
            2);
        vxnneLayer_SetOperation(
            &rpnLayer->base,
            nms_op,
            3);
       vxnneLayer_SetOperation(
            &rpnLayer->base,
            retrieve_op,
            4);

        node->layer = &rpnLayer->base;
    }

    return VX_SUCCESS;

exit:
    if (rpnLayer)
    {
        gcoOS_Free(VX_NULL, rpnLayer);
    }

    return status;
}


VX_PRIVATE_API vx_status VX_CALLBACK vxoNNRPNLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status;
    vx_int32 rpn_type = 0;
    gctSTRING envctrl = gcvNULL;
    vx_tensor  score                      = (vx_tensor)parameters[0];
    vx_tensor  bbox                       = (vx_tensor)parameters[1];
    vx_tensor  anchors                    = (vx_tensor)parameters[2];

    vx_enum score_format = TENSOR_DATA_TYPE(score);
    vx_enum bbox_format  = TENSOR_DATA_TYPE(bbox);
    vx_enum anchors_format = TENSOR_DATA_TYPE(anchors);

    vx_bool enable_condition0 = (vx_bool)((score_format   == VX_TYPE_FLOAT16)
                                        && (bbox_format    == VX_TYPE_FLOAT16)
                                        && (anchors_format == VX_TYPE_FLOAT32));

    vx_bool enable_condition1 = (vx_bool)((score_format   == VX_TYPE_INT16)
                                        && (bbox_format    == VX_TYPE_INT16)
                                        && (anchors_format == VX_TYPE_FLOAT32));

    vx_bool enable_condition2 = (vx_bool)((score_format   == VX_TYPE_INT8)
                                        && (bbox_format    == VX_TYPE_INT8)
                                        && (anchors_format == VX_TYPE_FLOAT32));

    vx_bool enable_condition3 = (vx_bool)((score_format   == VX_TYPE_UINT8)
                                        && (bbox_format    == VX_TYPE_UINT8)
                                        && (anchors_format == VX_TYPE_FLOAT32));




    /*USE_RPN_MODE-0: SHD+CPU 1: ALL CPU 2: ALL SHD*/
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "USE_RPN_MODE", &envctrl)) && envctrl)
    {
        rpn_type = atoi(envctrl);
    }
    if(!(enable_condition0 | enable_condition1 | enable_condition2 | enable_condition3))
        rpn_type = 1;
    if(rpn_type == 0)
        status = vxoNNRPNLayer_Initializer_shd_cpu(node,parameters,num);
    else if(rpn_type == 1)
        status = vxoNNRPNLayer_Initializer_cpu(node,parameters,num);
    else
        status = vxoNNRPNLayer_Initializer_shd(node,parameters,num);
    return status;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNRPNLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

