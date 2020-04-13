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
#include <gc_vx_nn_util.h>
#include <gc_vx_nn_wb.h>
#include <gc_vx_nn_encoder.h>
#include <gc_vx_nn_util.h>
#if defined(__linux__) || defined(__ANDROID__) || defined(__QNX__) || defined(__APPLE__) || defined(__CYGWIN__)
#include <utime.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#elif defined(_WIN32) || defined(UNDER_CE)
#include <dirent_win.h>
#include <windows.h>
#include <sys/utime.h>
#endif
#ifndef ORI_NNARCHPERF
#include "archModelInterface.h"
#endif

#define ENABLE_SAVE_OFFSET_IN_NBG    1

#define SH_COMMAND_ALIGN_SIZE        256
#define MEMORY_ALIGN_SIZE            64
#define ONCE_FILL_SIZE               1024
#define BINARY_FILE_NAME_MAX_SIZE    256
#define LOAD_WHOLE_BINARY_TO_BUFFER  0
#define _GC_OBJ_ZONE            gcdZONE_VX_BINARY
#define TP_MAX_XYSIZE (0x1<<16)
#define MULTI_TP_RESHUFFLE_SIZE 100

#define WRITE_NBG_STATUS_CHECK()    \
    if (status != VX_SUCCESS)   \
    {   \
        vxError("%s[%d]:failed to write data\n", __FUNCTION__, __LINE__);   \
        vxmONERROR(VX_ERROR_NO_MEMORY);  \
    }

extern vx_status vxnneOperation_AddReference(
    vxnne_operation_s*            operation,
    vx_reference                  reference,
    vxnne_operation_reference_e   refType
    );

VX_PRIVATE_API vx_status vxoBinaryGraph_GetNBGSize(
    vx_graph graph,
    vx_size *size
    );

VX_PRIVATE_API vx_status vxoBinaryGraph_SaveErrorHandle(
    vx_graph graph,
    vx_status status
    );

VX_PRIVATE_API vx_type_e vxoBinaryGraph_ConvertToOVXDataFormat(
    vx_uint32 format,
    vx_uint32 type
    )
{
    vx_enum ret = VX_TYPE_UINT8;
    switch (format)
    {
        case VX_BINARY_DATA_FORMAT_FP16:
            ret = VX_TYPE_FLOAT16;
            break;
        case VX_BINARY_DATA_FORMAT_FP32:
            if (type == VX_BINARY_BUFFER_TYPE_IMAGE)
            {
                ret = VX_DF_IMAGE_F32;
            }
            else
            {
                ret = VX_TYPE_FLOAT32;
            }
            break;
        case VX_BINARY_DATA_FORMAT_UINT32:
            ret = VX_DF_IMAGE_U32;
            break;
        case VX_BINARY_DATA_FORMAT_INT32:
            ret = VX_DF_IMAGE_S32;
            break;
        case VX_BINARY_DATA_FORMAT_UINT8:
            if (type == VX_BINARY_BUFFER_TYPE_IMAGE)
            {
                ret = VX_DF_IMAGE_U8;
            }
            else
            {
                ret = VX_TYPE_UINT8;
            }
            break;
        case VX_BINARY_DATA_FORMAT_INT8:
            ret = VX_TYPE_INT8;
            break;
        case VX_BINARY_DATA_FORMAT_UINT16:
            if (type == VX_BINARY_BUFFER_TYPE_IMAGE)
            {
                ret = VX_DF_IMAGE_U16;
            }
            else
            {
                ret = VX_TYPE_UINT16;
            }
            break;
        case VX_BINARY_DATA_FORMAT_INT16:
            if (type == VX_BINARY_BUFFER_TYPE_IMAGE)
            {
                ret = VX_DF_IMAGE_S16;
            }
            else
            {
                ret = VX_TYPE_INT16;
            }
            break;
        case VX_BINARY_DATA_FORMAT_CHAR:
            ret = VX_TYPE_CHAR;
            break;
        case VX_BINARY_DATA_FORMAT_FP64:
            ret = VX_TYPE_FLOAT64;
            break;
        case VX_BINARY_DATA_FORMAT_INT64:
            ret = VX_TYPE_INT64;
            break;
        case VX_BINARY_DATA_FORMAT_UINT64:
            ret = VX_TYPE_UINT64;
            break;
        default:
            ret = VX_TYPE_UINT8;
            break;
    }

    return ret;
}

/****************SW Operation Implement Start**********************/

static vx_status do_RPNLayer(
    vx_binary_segment_s *segment
    )
{
    vx_status status = VX_SUCCESS;
    vx_binary_nn_layer_RPN_s *rpnLayer = (vx_binary_nn_layer_RPN_s*)segment->base;

    vx_uint32 feat_stride   = *((vx_uint32*)rpnLayer->feature_stride.buffer);
    vx_uint32 min_size      = *((vx_uint32*)rpnLayer->min_size.buffer);
    vx_uint32 pre_nms_topn  = *((vx_uint32*)rpnLayer->pre_nms_topn.buffer);
    vx_uint32 post_nms_topn = *((vx_uint32*)rpnLayer->post_nms_topn.buffer);
    vx_float32 nms_thresh   = *((vx_float32*)rpnLayer->nms_thresh.buffer);

    vx_uint32 score_channel = rpnLayer->score.dims[2];
    vx_uint32 score_height  = rpnLayer->score.dims[1];
    vx_uint32 score_width   = rpnLayer->score.dims[0];
    vx_uint32 score_count   = score_width * score_height * score_channel/2; /*14 x 14 x (2x42) -> 17901 x 2 */

    vx_uint32 bbox_height   = rpnLayer->bbox.dims[1];
    vx_uint32 bbox_width    = rpnLayer->bbox.dims[0];

    vx_uint32 anchor_count  = rpnLayer->anchor.dims[3]; /* anchor batch = anchor number */

    vx_type_e in_score_format   = vxoBinaryGraph_ConvertToOVXDataFormat(rpnLayer->score.dataFormat, rpnLayer->score.dataType);
    vx_type_e in_bbox_format    = vxoBinaryGraph_ConvertToOVXDataFormat(rpnLayer->bbox.dataFormat, rpnLayer->bbox.dataType);
    vx_type_e in_anchor_format  = vxoBinaryGraph_ConvertToOVXDataFormat(rpnLayer->anchor.dataFormat, rpnLayer->anchor.dataType);
    vx_type_e in_img_format     = vxoBinaryGraph_ConvertToOVXDataFormat(rpnLayer->img_info.dataFormat, rpnLayer->img_info.dataType);
    vx_type_e out_roi_format    = vxoBinaryGraph_ConvertToOVXDataFormat(rpnLayer->roi_output.dataFormat, rpnLayer->roi_output.dataType);
    vx_int8 in_score_fp         = (vx_int8)rpnLayer->score.fixPointZeroPoint;
    vx_int8 in_bbox_fp          = (vx_int8)rpnLayer->bbox.fixPointZeroPoint;
    vx_int8 in_anchor_fp        = (vx_int8)rpnLayer->anchor.fixPointZeroPoint;
    vx_int8 in_img_fp           = (vx_int8)rpnLayer->img_info.fixPointZeroPoint;
    vx_int8 out_roi_fp          = (vx_int8)rpnLayer->roi_output.fixPointZeroPoint;
    vx_enum in_score_quant_format   = rpnLayer->score.quantFormat;
    vx_enum in_bbox_quant_format    = rpnLayer->bbox.quantFormat;
    vx_enum in_anchor_quant_format  = rpnLayer->anchor.quantFormat;
    vx_enum in_img_quant_format     = rpnLayer->img_info.quantFormat;
    vx_enum out_roi_quant_format    = rpnLayer->roi_output.quantFormat;
    vx_int32 in_score_zp            = rpnLayer->score.fixPointZeroPoint;
    vx_int32 in_bbox_zp             = rpnLayer->bbox.fixPointZeroPoint;
    vx_int32 in_anchor_zp           = rpnLayer->anchor.fixPointZeroPoint;
    vx_int32 in_img_zp              = rpnLayer->img_info.fixPointZeroPoint;
    vx_int32 out_roi_zp             = rpnLayer->roi_output.fixPointZeroPoint;
    vx_float32 in_score_scale       = rpnLayer->score.tfScale;
    vx_float32 in_bbox_scale        = rpnLayer->bbox.tfScale;
    vx_float32 in_anchor_scale      = rpnLayer->anchor.tfScale;
    vx_float32 in_img_scale         = rpnLayer->img_info.tfScale;
    vx_float32 out_roi_scale        = rpnLayer->roi_output.tfScale;
    vx_enum out_roi_rMode           = VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING;

    vx_uint32 proposal_width, proposal_height, proposal_area, proposal_count;
    vx_float32 img_W,img_H,img_scale_H,img_scale_W;
    vx_float32 min_box_H,min_box_W;
    vx_uint8_ptr score_data,bbox_data,img_data,anchor_data,out_roi_data;
    vx_float32_ptr score_buffer = NULL, foreground_score = NULL;
    vx_float32_ptr proposals    = NULL, p_proposals = NULL, pro_base = NULL;
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

    score_data = rpnLayer->score.buffer;
    bbox_data = rpnLayer->bbox.buffer;
    img_data = rpnLayer->img_info.buffer;
    anchor_data = rpnLayer->anchor.buffer;
    out_roi_data = rpnLayer->roi_output.buffer;

    out_score_format = vxoBinaryGraph_ConvertToOVXDataFormat(rpnLayer->score_output.dataFormat, rpnLayer->score_output.dataType);
    out_score_fp = (vx_int8)rpnLayer->score_output.fixPointZeroPoint;
    out_score_quant_format = rpnLayer->score_output.quantFormat;
    out_score_zp = rpnLayer->score_output.fixPointZeroPoint;
    out_score_scale = rpnLayer->score_output.tfScale;
    out_score_rMode = VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING;
    out_score_data = rpnLayer->score_output.buffer;


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
    pro_base = proposals;
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
        vxnneSaveDataExt((vx_type_e)out_roi_format, out_roi_quant_format, (i * 5 + 0), item_index, (vx_uint8_ptr)out_roi_data, out_roi_fp, out_roi_zp, out_roi_scale, out_roi_rMode);
        vxnneSaveDataExt((vx_type_e)out_roi_format, out_roi_quant_format, (i * 5 + 1), p_proposals[0], (vx_uint8_ptr)out_roi_data, out_roi_fp, out_roi_zp, out_roi_scale, out_roi_rMode);
        vxnneSaveDataExt((vx_type_e)out_roi_format, out_roi_quant_format, (i * 5 + 2), p_proposals[1], (vx_uint8_ptr)out_roi_data, out_roi_fp, out_roi_zp, out_roi_scale, out_roi_rMode);
        vxnneSaveDataExt((vx_type_e)out_roi_format, out_roi_quant_format, (i * 5 + 3), p_proposals[2], (vx_uint8_ptr)out_roi_data, out_roi_fp, out_roi_zp, out_roi_scale, out_roi_rMode);
        vxnneSaveDataExt((vx_type_e)out_roi_format, out_roi_quant_format, (i * 5 + 4), p_proposals[3], (vx_uint8_ptr)out_roi_data, out_roi_fp, out_roi_zp, out_roi_scale, out_roi_rMode);

        /* Copy proposals score to score output tensor */
        vxnneSaveDataExt(out_score_format, out_score_quant_format, i, p_proposals[4], (vx_uint8_ptr)out_score_data, out_score_fp, out_score_zp, out_score_scale, out_score_rMode);
    }

    if(score_buffer) vxFree(score_buffer);
    if(pro_base) vxFree(pro_base);
    if(roi_indices) vxFree(roi_indices);

    return status;
}

/****************SW Operation Implement End**********************/

VX_PRIVATE_API vx_status vxoBinaryGraph_PaserSWLayerParameter(
    vx_node node,
    vx_binary_loader_s *binLoad,
    vx_binary_layer_parameter_s *layerParam,
    vx_binary_layer_buffer layerBuffer
    )
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i = 0;
    vx_reference *parameters = node->paramTable;
    vx_graph graph = node->graph;

    switch (layerParam->sourceType)
    {
        case VX_BINARY_SOURCE_INPUT:
        {
            gctPOINTER tensorLogical  = gcvNULL;
            vx_uint32 tensorPhysical  = 0;
            vx_reference ref = VX_NULL;
            vx_int32 inIndex = layerParam->index;
            if (graph->inputCount && graph->inputs)
            {
                ref = graph->inputs[layerParam->index];
            }
            else
                ref = parameters[inIndex];
            if (ref->type == VX_TYPE_TENSOR)
            {
                vx_tensor tensor = (vx_tensor)ref;
                if (!tensor->tensorBuffer->memory.allocated)
                {
                    vxmONERROR(vxoTensor_AllocateMemory(tensor));
                }
                vxoTensor_GetTensorBatchArrayViewMemory(tensor, 0, &tensorLogical, &tensorPhysical);
                layerBuffer->buffer = (vx_uint8_ptr)tensorLogical;
            }
        }
        break;

        case VX_BINARY_SOURCE_OUTPUT:
        {
            gctPOINTER tensorLogical  = gcvNULL;
            vx_uint32 tensorPhysical  = 0;
            vx_reference ref = VX_NULL;
            vx_uint32 outIndex = layerParam->index + binLoad->fixed.header.inputCount;
            if (graph->outputCount && graph->outputs)
            {
                ref = graph->outputs[layerParam->index];
             }
            else
                ref = parameters[outIndex];
            if (ref->type == VX_TYPE_TENSOR)
            {
                vx_tensor tensor = (vx_tensor)ref;
                if (!tensor->tensorBuffer->memory.allocated)
                {
                    vxmONERROR(vxoTensor_AllocateMemory(tensor));
                }
                vxoTensor_GetTensorBatchArrayViewMemory(tensor, 0, &tensorLogical, &tensorPhysical);
                layerBuffer->buffer = (vx_uint8_ptr)tensorLogical;
            }
        }
        break;

        case VX_BINARY_SOURCE_MEMORY_POOL:
        {
            layerBuffer->buffer = node->binLoadMem->pool.logical + layerParam->addressOffset;
        }
        break;

        case VX_BINARY_SOURCE_MISC_DYNAMIC_GENERIC:
        case VX_BINARY_SOURCE_MISC_DYNAMIC_INPUT:
        case VX_BINARY_SOURCE_MISC_DYNAMIC_OUTPUT:
        {
            vx_int32 index = layerParam->index;
            layerBuffer->buffer = binLoad->LCD.logical + binLoad->LCDT[index].offset + layerParam->addressOffset;
        }
        break;

        default:
        {
            vxError("%s[%d]: not support this source type: %d\n", __FUNCTION__, __LINE__, layerParam->sourceType);
        }
        break;
    }

    layerBuffer->dimCount = layerParam->dimCount;
    layerBuffer->dataFormat = layerParam->dataFormat;
    layerBuffer->dataType = layerParam->dataType;
    layerBuffer->quantFormat = layerParam->quantFormat;
    layerBuffer->fixPointZeroPoint = layerParam->fixPointZeroPoint;
    layerBuffer->tfScale = layerParam->tfScale;
    for (i = 0; i < layerParam->dimCount; i++)
    {
        layerBuffer->dims[i] = layerParam->dims[i];
    }

OnError:
    return status;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_patchSWRPN(
    vx_node node,
    vx_binary_sw_operation_info_s *swOpData,
    vx_binary_operation_info_s *operation,
    vx_binary_loader_s *binLoad,
    vx_binary_segment_s *segment
    )
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i = 0;
    vx_binary_layer_parameter_s *layerParam = VX_NULL;
    vx_binary_nn_layer_RPN_s *rpnLayer = VX_NULL;

    rpnLayer = (vx_binary_nn_layer_RPN_s*)vxAllocateAndZeroMemory(sizeof(vx_binary_nn_layer_RPN_s));

    for (i = 0; i < operation->counterOfPatches; i++)
    {
        layerParam = &binLoad->layerParam[operation->indexOfFirstPatch + i];

        if (gcoOS_StrCmp(layerParam->paramName, "score") == 0)
        {
            vxoBinaryGraph_PaserSWLayerParameter(node, binLoad, layerParam, &rpnLayer->score);
        }
        else if (gcoOS_StrCmp(layerParam->paramName, "bbox") == 0)
        {
            vxoBinaryGraph_PaserSWLayerParameter(node, binLoad, layerParam, &rpnLayer->bbox);
        }
        else if (gcoOS_StrCmp(layerParam->paramName, "anchor") == 0)
        {
            vxoBinaryGraph_PaserSWLayerParameter(node, binLoad, layerParam, &rpnLayer->anchor);
        }
        else if (gcoOS_StrCmp(layerParam->paramName, "img_info") == 0)
        {
            vxoBinaryGraph_PaserSWLayerParameter(node, binLoad, layerParam, &rpnLayer->img_info);
        }
        else if (gcoOS_StrCmp(layerParam->paramName, "roi_output") == 0)
        {
            vxoBinaryGraph_PaserSWLayerParameter(node, binLoad, layerParam, &rpnLayer->roi_output);
        }
        else if (gcoOS_StrCmp(layerParam->paramName, "score_output") == 0)
        {
            vxoBinaryGraph_PaserSWLayerParameter(node, binLoad, layerParam, &rpnLayer->score_output);
        }
        else if (gcoOS_StrCmp(layerParam->paramName, "feature_stride") == 0)
        {
            vxoBinaryGraph_PaserSWLayerParameter(node, binLoad, layerParam, &rpnLayer->feature_stride);
        }
        else if (gcoOS_StrCmp(layerParam->paramName, "min_size") == 0)
        {
            vxoBinaryGraph_PaserSWLayerParameter(node, binLoad, layerParam, &rpnLayer->min_size);
        }
        else if (gcoOS_StrCmp(layerParam->paramName, "pre_nms_topn") == 0)
        {
            vxoBinaryGraph_PaserSWLayerParameter(node, binLoad, layerParam, &rpnLayer->pre_nms_topn);
        }
        else if (gcoOS_StrCmp(layerParam->paramName, "post_nms_topn") == 0)
        {
            vxoBinaryGraph_PaserSWLayerParameter(node, binLoad, layerParam, &rpnLayer->post_nms_topn);
        }
        else if (gcoOS_StrCmp(layerParam->paramName, "nms_thresh") == 0)
        {
            vxoBinaryGraph_PaserSWLayerParameter(node, binLoad, layerParam, &rpnLayer->nms_thresh);
        }
        else
        {
            vxError("%s[%d]: can't match layer parameter\n", __FUNCTION__, __LINE__);
        }
    }

    rpnLayer->base.segmentType = swOpData->swOperationType;
    segment->base = &rpnLayer->base;

    return status;
}

/****************SW Operation Implement End**********************/

static void openReader(
    vx_binary_reader_s *reader,
    void *data,
    vx_uint32 size
    )
{
    reader->currentData = reader->data = (vx_char *)data;
    reader->size = size;
    reader->offset = 0;
}

static void closeReader(
    vx_binary_reader_s *reader
    )
{
    reader->currentData = reader->data = VX_NULL;
    reader->size = reader->offset = 0;
}

/*
 * @brief
 * Rewind the current position to the beginning.
 */
static void readerRewind(
    vx_binary_reader_s *reader
    )
{
    reader->currentData = reader->data;
    reader->offset = 0;
}

/*
 * @brief
 * Move the read pointer forward by Step.
 */
static vx_status readerForward(
    vx_binary_reader_s *reader,
    vx_uint32 step
    )
{
    vx_status status = VX_SUCCESS;
    gcmHEADER_ARG("reader=%p, step=0x%x", reader, step);
    if (reader->offset + step >= reader->size)
    {
        vxError("%s[%d]: reader->offset + step < reader->size\n", __FUNCTION__, __LINE__);
        status = VX_ERROR_INVALID_VALUE;
    }
    else
    {
        reader->offset += step;
        reader->currentData = reader->data + reader->offset;
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

/*
 * @brief
 * Move the read pointer to a new location.
 */
static vx_status readerLocate(
    vx_binary_reader_s *reader,
    vx_uint32 location
    )
{
    vx_status status = VX_SUCCESS;
    gcmHEADER_ARG("reader=%p, location=0x%x", reader, location);

    if (location >= reader->size)
    {
        vxError("%s[%d]: location: %d size: %d\n", __FUNCTION__, __LINE__,
            location, reader->size);
        status = VX_ERROR_INVALID_VALUE;
    }
    else
    {
        reader->offset = location;
        reader->currentData = reader->data + reader->offset;
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

/*********** Read different types of data. ***********/
static vx_int8 readByte(
    vx_binary_reader_s *reader,
    vx_uint32 autoMove
    )
{
    vx_int8  data = *((vx_int8 *)reader->currentData);
    gcmHEADER_ARG("reader=%p, autoMove=0x%x", reader, autoMove);
    if (autoMove != 0)
    {
        reader->offset += sizeof(vx_int8);
        reader->currentData = reader->data + reader->offset;
    }
    gcmFOOTER_ARG("%p", data);
    return data;
}

static vx_uint32 readuInt(
    vx_binary_reader_s *reader,
    vx_uint32 autoMove
    )
{
    vx_uint32  data = *((vx_uint32 *)reader->currentData);
    gcmHEADER_ARG("reader=%p, autoMove=0x%x", reader, autoMove);
    if (autoMove != 0)
    {
        reader->offset += sizeof(vx_uint32);
        reader->currentData = reader->data + reader->offset;
    }
    gcmFOOTER_ARG("%p", data);
    return data;
}

static vx_status readData(
    vx_binary_reader_s *reader,
    void *memory,
    vx_uint32 size
    )
{
    vx_status status = VX_SUCCESS;

    gcmHEADER_ARG("reader=%p, memory=%p, size=0x%x", reader, memory, size);

    if ((reader->offset + size <= reader->size) && (memory != NULL))
    {
        vxMemCopy((vx_ptr)memory, (vx_const_ptr)reader->currentData, (vx_size)size);
    }
    else
    {
        vxError("%s[%d]: reader->offset + size > reader->size\n", __FUNCTION__, __LINE__);
        status = VX_ERROR_INVALID_VALUE;
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

static vx_status readBinHeader(
    vx_binary_reader_s *reader,
    vx_binary_header_s *header
    )
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i = 0;

    static const char magic[4] = {'V', 'P', 'M', 'N'};
    gcmHEADER_ARG("reader=%p, header=%p", reader, header);
    readerRewind(reader);

    /* read magic*/
    for (i = 0; i < sizeof(magic); i++)
    {
        header->magic[i] = readByte(reader, 1);
        if (header->magic[i] != magic[i])
        {
            vxError("%s[%d]: binary magic not match\n", __FUNCTION__, __LINE__);
            vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
        }
    }

    header->version = readuInt(reader, 1);
    header->target = readuInt(reader, 1);

    if (header->target != reader->binLoad->context->pid)
    {
        vxError("%s[%d]: binary target: 0x%x, actually target: 0x%x", __FUNCTION__, __LINE__,
            header->target, reader->binLoad->context->pid);
        vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
    }

    vxmONERROR(readData(reader, header->networkName, sizeof(header->networkName)));
    vxmONERROR(readerForward(reader, sizeof(header->networkName)));

    header->layerCount = readuInt(reader, 1);
    header->operationCount = readuInt(reader, 1);
    header->inputCount = readuInt(reader, 1);
    header->outputCount = readuInt(reader, 1);

    /* NBG support feature database in version 1.3 or higher */
    if (header->version >= 0x00010003)
    {
        vxmONERROR(readData(reader, &header->featureDB, sizeof(vx_binary_feature_database_s)));
        vxmONERROR(readerForward(reader, sizeof(vx_binary_feature_database_s)));
    }

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

static vx_status readBinPool(
    vx_binary_reader_s *reader,
    vx_binary_memory_pool_info_s *pool
    )
{
    gcmHEADER_ARG("reader=%p, pool=%p", reader, pool);
    pool->alignedSize = readuInt(reader, 1);
    pool->alignement = readuInt(reader, 1);
    pool->memoryPoolBase = readuInt(reader, 1);

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

static vx_status readAxiSram(
    vx_binary_reader_s *reader,
    vx_binary_axi_sram_info_s *axiTable
    )
{
    vx_status status = VX_SUCCESS;
    gcmHEADER_ARG("reader=%p, axiTable=%p", reader, axiTable);

    axiTable->sramBase = readuInt(reader, 1);
    axiTable->sramSize = readuInt(reader, 1);

    gcmFOOTER_ARG("%d", status);
    return status;
}

static vx_status readVipSram(
    vx_binary_reader_s *reader,
    vx_binary_vip_sram_info_s *vipTable
    )
{
    vx_status status = VX_SUCCESS;
    gcmHEADER_ARG("reader=%p, vipTable=%p", reader, vipTable);

    vipTable->sramBase = readuInt(reader, 1);
    vipTable->sramSize = readuInt(reader, 1);

    gcmFOOTER_ARG("%d", status);
    return status;
}

vx_status readBinEntry(
    vx_binary_reader_s *reader,
    vx_binary_entry_s *Entry
    )
{
    Entry->offset = readuInt(reader, 1);
    Entry->size = readuInt(reader, 1);

    return VX_SUCCESS;
}

static vx_status readBinFixed(
    vx_binary_reader_s *reader,
    vx_binary_loader_s *binaryLoad
    )
{
    vx_status status = VX_SUCCESS;
    vxmONERROR(readBinHeader(reader, &binaryLoad->fixed.header));

    readBinPool(reader, &binaryLoad->fixed.poolTable);
    vxmONERROR(readAxiSram(reader, &binaryLoad->fixed.axiSramTable));

    if (binaryLoad->fixed.header.version >= 0x00010008)
    {
        vxmONERROR(readVipSram(reader, &binaryLoad->fixed.vipSramTable));
    }

    readBinEntry(reader, &binaryLoad->fixed.inputTable);
    readBinEntry(reader, &binaryLoad->fixed.outputTable);
    readBinEntry(reader, &binaryLoad->fixed.layerTable);
    readBinEntry(reader, &binaryLoad->fixed.opeartionTable);
    readBinEntry(reader, &binaryLoad->fixed.LCDTable);
    readBinEntry(reader, &binaryLoad->fixed.LCD);
    readBinEntry(reader, &binaryLoad->fixed.nnOpDataTable);
    readBinEntry(reader, &binaryLoad->fixed.tpOpDataTable);
    readBinEntry(reader, &binaryLoad->fixed.shOpDataTable);
    readBinEntry(reader, &binaryLoad->fixed.patchDataTable);
    readBinEntry(reader, &binaryLoad->fixed.layerParamTable);
    readBinEntry(reader, &binaryLoad->fixed.swOpDataTable);

OnError:
    return status;
}

static vx_uint32 getSHCmdSize(
    vx_binary_loader_s *binaryLoad
    )
{
    vx_uint32 size = 0;
    vx_uint32 i = 0;

    for (i = 0; i < binaryLoad->nShOps; i++)
    {
        size += gcmALIGN(binaryLoad->LCDT[binaryLoad->shOpsData[i].instructionCmdLCDTIndex].size,
                    SH_COMMAND_ALIGN_SIZE);
    }

    return size;
}

static vx_uint32 getStateSize(
    vx_binary_loader_s *binaryLoad
    )
{
    vx_uint32 size = 0;
    vx_uint32 i = 0;
    gcmHEADER_ARG("binaryLoad=%p", binaryLoad);

    for (i = 0; i < binaryLoad->nOperations; i++)
    {
        /* bypass INIT & SW operation */
        if ((binaryLoad->operations[i].operationType != VX_BINARY_OPERATION_TYPE_INIT) &&
            (binaryLoad->operations[i].operationType != VX_BINARY_OPERATION_TYPE_SW))
        {
            size += binaryLoad->LCDT[binaryLoad->operations[i].stateLCDTIndex].size;
        }
    }
    gcmFOOTER_ARG("0x%x", size);
    return size;
}

static vx_status readBinDynamic(
    vx_binary_reader_s *reader,
    vx_binary_loader_s *binLoad,
    vx_uint32 from,
    gctPOINTER lcdBuffer
    )
{
    vx_status status = VX_SUCCESS;

    gcmHEADER_ARG("reader=%p, binLoad=%p", reader, binLoad);

    /* Input data entries*/
    if (binLoad->fixed.inputTable.size > 0)
    {
        vxmONERROR(readerLocate(reader, binLoad->fixed.inputTable.offset));
        binLoad->inputs = (vx_binary_input_output_info_s *)(reader->currentData);

        if (binLoad->fixed.header.version >= 0x00010004)
        {
            binLoad->nInputs = binLoad->fixed.inputTable.size / sizeof(vx_binary_input_output_info_s);
        }
        else
        {
            binLoad->nInputs = binLoad->fixed.inputTable.size / (sizeof(vx_binary_input_output_info_s) - sizeof(vx_char) * VX_MAX_IO_NAME_LEGTH);
        }
    }

    /* Output data entries*/
    if (binLoad->fixed.outputTable.size > 0)
    {
        vxmONERROR(readerLocate(reader, binLoad->fixed.outputTable.offset));
        binLoad->outputs = (vx_binary_input_output_info_s *)(reader->currentData);

        if (binLoad->fixed.header.version >= 0x00010004)
        {
            binLoad->nOutputs = binLoad->fixed.outputTable.size / sizeof(vx_binary_input_output_info_s);
        }
        else
        {
            binLoad->nOutputs = binLoad->fixed.outputTable.size / (sizeof(vx_binary_input_output_info_s) - sizeof(vx_char) * VX_MAX_IO_NAME_LEGTH);
        }
    }

    /* Layer data entries*/
    if (binLoad->fixed.layerTable.size > 0)
    {
        vxmONERROR(readerLocate(reader, binLoad->fixed.layerTable.offset));
        binLoad->layersInfo = (vx_binary_layers_info_s *)(reader->currentData);

        if (binLoad->fixed.header.version >= 0x00010008)
        {
            binLoad->nLayersInfo = binLoad->fixed.layerTable.size / sizeof(vx_binary_layers_info_s);
        }
        else
        {
            binLoad->nLayersInfo = binLoad->fixed.layerTable.size / (sizeof(vx_binary_layers_info_s) - sizeof(vx_uint32));
        }
    }

    /* Operation data entries*/
    if (binLoad->fixed.opeartionTable.size > 0)
    {
        vxmONERROR(readerLocate(reader, binLoad->fixed.opeartionTable.offset));
        binLoad->operations = (vx_binary_operation_info_s *)(reader->currentData);
        binLoad->nOperations = binLoad->fixed.opeartionTable.size / sizeof(vx_binary_operation_info_s);
    }

    /* LCDT data entries*/
    if (binLoad->fixed.LCDTable.size > 0)
    {
        vxmONERROR(readerLocate(reader, binLoad->fixed.LCDTable.offset));
        binLoad->LCDT = (vx_binary_entry_s *)(reader->currentData);
        binLoad->nLCDT = binLoad->fixed.LCDTable.size / sizeof(vx_binary_entry_s);
    }

    /* NN op data entries*/
    if (binLoad->fixed.nnOpDataTable.size > 0)
    {
        vxmONERROR(readerLocate(reader, binLoad->fixed.nnOpDataTable.offset));
        binLoad->nnOpsData = (vx_binary_nn_operation_info_s *)(reader->currentData);
        binLoad->nNnOps = binLoad->fixed.nnOpDataTable.size / sizeof(vx_binary_nn_operation_info_s);
    }

    /* TP op data entries*/
    if (binLoad->fixed.tpOpDataTable.size > 0)
    {
        vxmONERROR(readerLocate(reader, binLoad->fixed.tpOpDataTable.offset));
        binLoad->tpOpsData = (vx_binary_tp_operation_info_s *)(reader->currentData);
        binLoad->nTpOps = binLoad->fixed.tpOpDataTable.size / sizeof(vx_binary_tp_operation_info_s);
    }

    /* SH op data entries*/
    if (binLoad->fixed.shOpDataTable.size > 0)
    {
        vxmONERROR(readerLocate(reader, binLoad->fixed.shOpDataTable.offset));
        binLoad->shOpsData = (vx_binary_sh_operation_info_s *)(reader->currentData);
        binLoad->nShOps = binLoad->fixed.shOpDataTable.size / sizeof(vx_binary_sh_operation_info_s);
    }

    /* Patch data entries*/
    if (binLoad->fixed.patchDataTable.size > 0)
    {
        vxmONERROR(readerLocate(reader, binLoad->fixed.patchDataTable.offset));
        binLoad->patchData = (vx_binary_patch_info_s *)(reader->currentData);

        if (binLoad->fixed.header.version >= 0x00010006)
        {
            binLoad->nPdEntries = binLoad->fixed.patchDataTable.size / sizeof(vx_binary_patch_info_s);
        }
        else
        {
            binLoad->nPdEntries = binLoad->fixed.patchDataTable.size / (sizeof(vx_binary_patch_info_s) - sizeof(vx_uint32));
        }
    }

    /* layer parameter data entries*/
    if (binLoad->fixed.layerParamTable.size > 0)
    {
        vxmONERROR(readerLocate(reader, binLoad->fixed.layerParamTable.offset));
        binLoad->layerParam = (vx_binary_layer_parameter_s *)(reader->currentData);
        binLoad->nlayerParam = binLoad->fixed.layerParamTable.size / sizeof(vx_binary_layer_parameter_s);
    }

    /* SW op data entries*/
    if (binLoad->fixed.swOpDataTable.size > 0)
    {
        vxmONERROR(readerLocate(reader, binLoad->fixed.swOpDataTable.offset));
        binLoad->swOpsData = (vx_binary_sw_operation_info_s *)(reader->currentData);
        binLoad->nSwOps = binLoad->fixed.swOpDataTable.size / sizeof(vx_binary_sw_operation_info_s);
    }

    /* load LCD */
    if (binLoad->fixed.LCD.size > 0)
    {
        /*allocate all LCD alignment buffer(sates + ks + lut + shcmd)*/
        if (gcmIS_ERROR(gcoVX_AllocateMemory(binLoad->fixed.LCD.size, (gctPOINTER*)&binLoad->LCD.logical,
                                            &binLoad->LCD.physical, &binLoad->LCD.nodePtr)))
        {
            vxError("%s[%d]: fail to allocate memory for lcd\n", __FUNCTION__, __LINE__);
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }

        if (from == VX_BINARY_DYN_FROM_WHOLE_MEMORY)
        {
            /*copy LCD data to GPU memory*/
            vxmONERROR(readerLocate(reader, binLoad->fixed.LCD.offset));
            vxmONERROR(readData(reader, binLoad->LCD.logical, binLoad->fixed.LCD.size));
        }
        else if (from == VX_BINARY_DYN_FROM_FILE)
        {
            /*read LCD data from binary file*/
            gctSIZE_T readSize = 0;
            gcoOS_Seek(gcvNULL, binLoad->dFile, binLoad->fixed.LCD.offset, gcvFILE_SEEK_SET);
            gcoOS_Read(gcvNULL, binLoad->dFile, binLoad->fixed.LCD.size, binLoad->LCD.logical, &readSize);
            if ((vx_uint32)readSize != binLoad->fixed.LCD.size)
            {
                vxError("%s[%d]: fail to read lcd data from file: readSize: %d, lcdSize: %d\n",
                    __FUNCTION__, __LINE__, (vx_uint32)readSize, binLoad->fixed.LCD.size);
                vxmONERROR(VX_FAILURE);
            }
        }
        else if (from == VX_BINARY_DYN_FROM_HANDLE)
        {
            vxMemCopy((vx_ptr)binLoad->LCD.logical, lcdBuffer, binLoad->fixed.LCD.size);
        }
    }
    else
    {
        vxError("%s[%d]: lcd size if 0, error\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_FAILURE);
    }

    if (binLoad->nOperations < 1)
    {
        vxError("%s[%d]: error, the number of operation %d\n", __FUNCTION__,
                __LINE__, binLoad->nOperations);
        vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
    }

    if ((binLoad->fixed.header.inputCount != binLoad->nInputs) ||
        (binLoad->fixed.header.outputCount != binLoad->nOutputs))
    {
        vxError("%s[%d]: header input %d, output %d\n", __FUNCTION__, __LINE__,
                binLoad->nInputs, binLoad->nOutputs);
        vxmONERROR(VX_FAILURE);
    }
    gcmFOOTER_ARG("%d", status);
    return status;

OnError:
    if (binLoad->LCD.nodePtr != VX_NULL)
    {
        gcmVERIFY_OK(gcoVX_FreeMemory(binLoad->LCD.nodePtr));
        binLoad->LCD.nodePtr = VX_NULL;
    }
    vxError("fail in read Binary Dynamic\n");
    gcmFOOTER_ARG("%d", status);
    return status;
}

#if LOAD_WHOLE_BINARY_TO_BUFFER
VX_PRIVATE_API gctUINT getFileSize(
    gctFILE df
    )
{
    gctUINT length = 0;
    gcmHEADER_ARG("df=%p", df);

    gcoOS_Seek(gcvNULL, df, 0, gcvFILE_SEEK_END);
    gcoOS_GetPos(gcvNULL, df, &length);

    gcmFOOTER_ARG("0x%x", length);
    return length;
}
#endif

VX_PRIVATE_API vx_uint32 geLCDOffset(
    vx_binary_loader_s *binLoad,
    vx_uint32 version
    )
{
    vx_uint32 LCDEntryOffsetPos = 0;


    LCDEntryOffsetPos = sizeof(vx_binary_header_s) + sizeof(vx_binary_memory_pool_info_s)
                            + sizeof(vx_binary_axi_sram_info_s)
                            + sizeof(vx_binary_vip_sram_info_s)
                            + sizeof(vx_binary_entry_s)/*inputTable*/
                            + sizeof(vx_binary_entry_s)/*outputTable*/
                            + sizeof(vx_binary_entry_s)/*layerTable*/
                            + sizeof(vx_binary_entry_s)/*opeartionTable*/
                            + sizeof(vx_binary_entry_s); /*LCDTable*/

    if (version < 0x00010003)
    {
        LCDEntryOffsetPos -= sizeof(vx_binary_feature_database_s);
    }

    if (version < 0x00010008)
    {
        LCDEntryOffsetPos -= sizeof(vx_binary_vip_sram_info_s);
    }

    return LCDEntryOffsetPos;
}

VX_PRIVATE_API vx_status getLCDEntryInfo(
    gctPOINTER binHandle,
    vx_binary_entry_s *lcdInfo
    )
{
    vx_status status = VX_SUCCESS;
    vx_uint32 version = 0;
    vx_uint32 LCDEntryOffsetPos = 0;

    version =  *((vx_uint32*)((vx_uint8_ptr)binHandle + 4));
    vxInfo("binary graph format version, 0x%x\n", version);

    LCDEntryOffsetPos = geLCDOffset(binHandle, version);

    lcdInfo->offset = *((vx_uint32*)((vx_uint8_ptr)binHandle + LCDEntryOffsetPos));
    lcdInfo->size = *((vx_uint32*)((vx_uint8_ptr)binHandle + LCDEntryOffsetPos + 4));

    return status;
}

VX_PRIVATE_API gctSIZE_T loadBinaryEntry(
    vx_binary_loader_s *binLoad
    )
{
    vx_uint32 LCDOffsetPos = 0;
    vx_uint32 array[2] = {0};
    vx_uint32 LCDEntryOffsetPos = 0;
    gctSIZE_T readSize = 0;
    vx_status status = VX_SUCCESS;
    vx_uint32 version[2] = {0};
    gcmHEADER_ARG("binLoad=%p", binLoad);

    gcoOS_Seek(gcvNULL, binLoad->dFile, 4, gcvFILE_SEEK_SET);
    gcoOS_Read(gcvNULL, binLoad->dFile, sizeof(vx_uint32), version, &readSize);

    LCDEntryOffsetPos = geLCDOffset(binLoad, version[0]);
    readSize = 0;

    gcoOS_Seek(gcvNULL, binLoad->dFile, LCDEntryOffsetPos, gcvFILE_SEEK_SET);
    gcoOS_Read(gcvNULL, binLoad->dFile, sizeof(vx_uint32), array, &readSize);
    LCDOffsetPos = array[0];
    if ((LCDOffsetPos <= LCDEntryOffsetPos) || ((vx_uint32)readSize != sizeof(vx_uint32)))
    {
        vxError("%s[%d]: fail to read lcdt offset, cdt: %d, lcd: %d\n", __FUNCTION__, __LINE__, LCDEntryOffsetPos, LCDOffsetPos);
        vxmONERROR(VX_FAILURE);
    }

    binLoad->binaryBuffer = vxAllocateAndZeroMemory((vx_size)LCDOffsetPos);
    if (VX_NULL == binLoad->binaryBuffer)
    {
        vxError("%s[%d]: fail to allocate memory for binary buffer\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_FAILURE);
    }

    gcoOS_Seek(gcvNULL, binLoad->dFile, 0, gcvFILE_SEEK_SET);
    gcoOS_Read(gcvNULL, binLoad->dFile, LCDOffsetPos, binLoad->binaryBuffer, &readSize);
    if (LCDOffsetPos != (vx_uint32)readSize)
    {
        vxError("%s[%d]: fail to read entry data, readsize: %d, entrySize: %d\n",
                 __FUNCTION__, __LINE__, (vx_uint32)readSize, LCDOffsetPos);
        vxmONERROR(VX_FAILURE);
    }

    gcmFOOTER_ARG("0x%x", readSize);
    return readSize;

OnError:
    gcmFOOTER_NO();
    return 0;
}

#if LOAD_WHOLE_BINARY_TO_BUFFER
VX_PRIVATE_API gctSIZE_T loadBinaryWhole(
    vx_binary_loader_s *binLoad
    )
{
    gctUINT length = 0;
    gctSIZE_T readSize = 0;

    gcmHEADER_ARG(" binLoad=%p", binLoad);

    vxmASSERT(binLoad->binaryBuffer != NULL);

    gcoOS_Seek(gcvNULL, binLoad->dFile, 0, gcvFILE_SEEK_END);
    gcoOS_GetPos(gcvNULL, binLoad->dFile, &length);
    gcoOS_Seek(gcvNULL, binLoad->dFile, 0, gcvFILE_SEEK_SET);
    gcoOS_Read(gcvNULL, binLoad->dFile, length, binLoad->binaryBuffer, &readSize);

    gcmFOOTER_ARG("0x%x", readSize);
    return readSize;
}
#endif

/*
aculater layer count in import binary graph mode.
this is for nn layer dump or sw operation
*/
VX_PRIVATE_API vx_uint32 vxoBinaryGraph_GetLayerCount(
    vx_binary_loader_s *binLoad
    )
{
    vx_uint32 layerCount = 0;
    vx_uint32 swOPCount = 0;
    vx_uint32 i = 0;
    vx_binary_operation_info_s *operation = VX_NULL;

    if ((binLoad->context->options.enableNNLayerDump) && (binLoad->nOperations > 0))
    {
        layerCount = binLoad->nOperations;/* a operation is layer for nn layer dump feature */
    }
    else
    {
        for (i = 0; i < binLoad->nOperations; i++)
        {
            operation = &binLoad->operations[i];
            if (VX_BINARY_OPERATION_TYPE_SW == operation->operationType)
            {
                swOPCount++;
            }
        }
        layerCount = swOPCount * 2 + 1;
    }

    return layerCount;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_DumpFile(
    vx_char *layerName,
    vx_uint32 layerId,
    vx_uint8_ptr *dumpLogical,
    vx_uint32 *dumpSize,
    vx_uint32 dumpCount
    )
{
    vx_status status = VX_SUCCESS;
    vx_char fileName[255] = {'\0'};
    vx_uint32 index = 0;
    vx_uint32 offset = 0;
    gctFILE fpLayer = VX_NULL;
    gcmHEADER_ARG("layerName=%s, layerId=0x%x, dumpLogical=%p, dumpSize=%p, dumpCount=0x%x", layerName, layerId, dumpLogical, dumpSize, dumpCount);

    gcoOS_MemFill(fileName, 0, gcmSIZEOF(fileName));

    vxInfo("***********dump layer : %2d***************\n", layerId);

    for (index = 0; index < dumpCount; index++)
    {
        gcoOS_PrintStrSafe(fileName, gcmSIZEOF(fileName), &offset, "%d_%s_%d_%d.bin", layerId, layerName, index, dumpSize[index]);

        gcmVERIFY_OK(gcoOS_Open(gcvNULL, fileName, gcvFILE_CREATE, (gctFILE*)(&fpLayer)));

        gcoOS_Seek(gcvNULL, fpLayer, 0, gcvFILE_SEEK_SET);
        gcoOS_Write(gcvNULL, fpLayer, dumpSize[index], dumpLogical[index]);

        gcoOS_Flush(gcvNULL, fpLayer);
        gcmVERIFY_OK(gcoOS_Close(gcvNULL, fpLayer));
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vx_bool vxoBinaryGraph_IsSWOperationInBinary(
    vx_binary_loader_s *binLoad
    )
{
    vx_uint32 i = 0;
    vx_bool isSW = vx_false_e;
    vx_binary_segment_s *segment = VX_NULL;

    for (i = 0; i < binLoad->segmentsCount; i++)
    {
        segment = &binLoad->segments[i];
        if ((segment != VX_NULL) && (segment->isSWSegment == vx_true_e))
        {
            isSW = vx_true_e;
            break;
        }
    }

    return isSW;
}

VX_INTERNAL_API void* vxoBinaryGraph_GetLayerInfoPtrByIndex(
    vx_binary_loader_s *binLoad,
    vx_binary_layers_info_s *layerPtr,
    vx_int32 index
    )
{
    void *ptr = VX_NULL;
    vx_uint32 size = 0;

    size = sizeof(vx_binary_layers_info_s) - sizeof(vx_uint32);

    if (binLoad->fixed.header.version >= 0x00010008)
    {
        ptr = (void*)(layerPtr + index);
    }
    else
    {
        ptr = (void*)((vx_char *)layerPtr + index * size);
    }

    return ptr;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_NNLayerDump(
    vx_node node,
    vx_binary_loader_s *binLoad
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    vx_uint32 index = 0, i = 0, j = 0;
    vx_uint8_ptr layerStatesBuffer = VX_NULL;
    gctPOINTER layerStatesBufferBase = VX_NULL;
    vx_uint8_ptr nodeStatesBuff = (vx_uint8_ptr)node->binLoadMem->statesBuff;
    vx_binary_operation_info_s *operation = VX_NULL;
    vx_binary_segment_s *segment = VX_NULL;
    vx_binary_layers_info_s *layer = VX_NULL;

    gcmHEADER_ARG("node=%p, binLoad=%p", node, binLoad);

    layerStatesBuffer = (vx_uint8_ptr)vxAllocateAndZeroMemory(sizeof(vx_uint8) * node->binLoadMem->statesSize);
    layerStatesBufferBase = (gctPOINTER)layerStatesBuffer;

    for (index = 0; index < binLoad->fixed.header.layerCount; index++)
    {
        vx_uint8_ptr dumpLogical[VX_MAX_OPERTAION_INPUTS_OUTPUTS] = {VX_NULL};
        vx_uint32 dumpSize[VX_MAX_OPERTAION_INPUTS_OUTPUTS] = {0};
        vx_uint32 layerStatesSize = 0;
        vx_uint32 dumpCount = 0;
        vx_uint32 layerLastOpindex = 0;

        layer = (vx_binary_layers_info_s*)vxoBinaryGraph_GetLayerInfoPtrByIndex(binLoad, binLoad->layersInfo, (vx_int32)index);
        layerStatesBuffer = (vx_uint8_ptr)layerStatesBufferBase;

        /* get this layer command */
        for (i = 0; i < binLoad->nOperations; i++)
        {
            operation = &binLoad->operations[i];
            if ((operation->operationType == VX_BINARY_OPERATION_TYPE_INIT) ||
                (operation->operationType == VX_BINARY_OPERATION_TYPE_NONE) ||
                (operation->operationType == VX_BINARY_OPERATION_TYPE_SW))
            {
                continue;
            }

            if (operation->layerId == index)
            {
                segment = &binLoad->segments[i];
                vxMemCopy((gctPOINTER)layerStatesBuffer, (vx_ptr)(nodeStatesBuff + segment->statesStartPos), segment->statesSize);
                layerStatesBuffer += segment->statesSize;
                layerStatesSize += segment->statesSize;

                layerLastOpindex = i;
            }
        }

        /* get this layer outputs */
        segment = &binLoad->segments[layerLastOpindex];
        for (i = 0; i < segment->outputCount; i++)
        {
            for (j = 0; j < dumpCount; j++)
            {
                if (dumpLogical[j] == segment->outputlogical[i])
                {
                    break;
                }
            }

            if (j == dumpCount)
            {
                dumpLogical[dumpCount] = segment->outputlogical[i];
                dumpSize[dumpCount] = segment->outputSize[i];
                dumpCount++;
            }
        }

        /* replay this layer command to hardware */
        if (layerStatesSize > 0)
        {
            gcmONERROR(gcoVX_Replay((gctPOINTER)layerStatesBufferBase, layerStatesSize));
            gcfVX_Flush(gcvTRUE);

            vxoBinaryGraph_DumpFile(layer->layerName, index, dumpLogical, dumpSize, dumpCount);
        }
        else
        {
            vxError("%s[%d]: states size is 0\n", __FUNCTION__, __LINE__);
        }
    }

OnError:
    vxFree(layerStatesBufferBase);
    gcmFOOTER_NO();
    return gcmIS_SUCCESS(status) ? VX_SUCCESS : VX_FAILURE;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_SubmitCommand(
    gctPOINTER statesBuff,
    vx_uint32 statesSize,
    vx_binary_loader_s *binLoad,
    vx_uint32 startOPIndex,
    vx_uint32 endOPIndex
    )
{
    vx_status status = VX_SUCCESS;
    vx_uint32 maxSize = gcmALIGN_BASE(VX_GRAPH_COMMAND_BUFFER_SIZE, 64);

    gcmHEADER_ARG("statesBuff=%p, binLoad=%p, statesSize=%d", statesBuff, binLoad, statesSize);
    if (maxSize == VX_GRAPH_COMMAND_BUFFER_SIZE)
    {
        maxSize = maxSize - 0x800;
    }

    if (statesSize > maxSize)
    {
        vx_uint32  size = 0, replayedSize = 0, i = 0;
        vx_uint32  replaySize = statesSize;
        vx_uint8_ptr buffer = (vx_uint8_ptr)statesBuff;
        vx_binary_operation_info_s *operation = VX_NULL;

        for (i = startOPIndex; i < endOPIndex; i++)
        {
            operation = &binLoad->operations[i];
            if ((operation->operationType == VX_BINARY_OPERATION_TYPE_INIT) ||
                (operation->operationType == VX_BINARY_OPERATION_TYPE_NONE) ||
                (operation->operationType == VX_BINARY_OPERATION_TYPE_SW) ||
                (operation->operationType == VX_BINARY_OPERATION_TYPE_END))
            {
                continue;
            }

            size += binLoad->LCDT[operation->stateLCDTIndex].size;
            if (size > maxSize)
            {
                replaySize = size - binLoad->LCDT[operation->stateLCDTIndex].size;
                gcmONERROR(gcoVX_Replay((gctPOINTER)(buffer + replayedSize), replaySize));
                replayedSize += replaySize;
                size = binLoad->LCDT[operation->stateLCDTIndex].size;
            }
        }
        gcmONERROR(gcoVX_Replay((gctPOINTER)(buffer + replayedSize), size));
    }
    else
    {
        gcmONERROR(gcoVX_Replay(statesBuff, statesSize));
    }

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_RunOpearation(
    vx_node node,
    vx_binary_loader_s *binLoad
    )
{
    vx_status status = VX_SUCCESS;
    vx_uint32 segmentIndex = 0;
    vx_binary_segment_s *segment = VX_NULL;
    vx_uint8_ptr statesBuff = (vx_uint8_ptr)node->binLoadMem->statesBuff;

    gcmHEADER_ARG("node=%p, binLoad=%p", node, binLoad);
    for (segmentIndex = 0; segmentIndex < binLoad->segmentsCount; segmentIndex++)
    {
        segment = &binLoad->segments[segmentIndex];

        if (vx_true_e == segment->isSWSegment)
        {
            gcfVX_Flush(gcvTRUE);

            switch (segment->base->segmentType)
            {
                case VX_BINARY_SW_OPERATION_RPN:
                {
                    do_RPNLayer(segment);
                }
                break;

                default:
                    break;
            }
        }
        else if (segment->statesSize > 0)
        {
            vx_uint8_ptr buffer = statesBuff + segment->statesStartPos;
            vxmONERROR(vxoBinaryGraph_SubmitCommand(buffer, segment->statesSize, binLoad,
                              segment->startOperationIndex, segment->endOperationIndex));
        }
        else
        {
            vxError("%s[%d]: can't run operation\n", __FUNCTION__, __LINE__);
        }
    }

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status vxoBinaryGraph_Run(
    vx_node node,
    vx_binary_loader_s *binLoad
    )
{
    vx_status status = VX_SUCCESS;
    vx_bool isSW = vxoBinaryGraph_IsSWOperationInBinary(binLoad);

    gcmHEADER_ARG("node=%p, binLoad=%p", node, binLoad);
    if (node->base.context->options.enableNNLayerDump)
    {
        vxoBinaryGraph_NNLayerDump(node, binLoad);
    }
    else if (vx_true_e == isSW)
    {
        vxmONERROR(vxoBinaryGraph_RunOpearation(node, binLoad));
    }
    else
    {
        vxmONERROR(vxoBinaryGraph_SubmitCommand(node->binLoadMem->statesBuff,
                    node->binLoadMem->statesSize, binLoad, 0, binLoad->nOperations));
    }

OnError:
    gcmFOOTER_NO();
    return status;
}

VX_INTERNAL_API vx_bool vxoBinaryGraph_HasBinaryInGraph(
    vx_graph graph
)
{
    vx_uint32 index = 0;
    for (index = 0; index < graph->nodeCount; index++)
    {
        vx_node node = graph->nodeTable[index];
        if (node->kernel->enumeration == VX_KERNEL_IMPORT_FROM_FILE)
        {
            return vx_true_e;
        }
    }

    return vx_false_e;
}

/* this is for generating binary graph
   vxoNode_SetParameter() to change graph input buffer.
   generate binary graph using the lastest input buffer.
*/
VX_INTERNAL_API vx_status vxoBinaryGraph_UpdataIOPhsicalTable(
    vx_node node,
    vx_uint32 index
)
{
    vx_status status = VX_SUCCESS;
    vx_kernel kernel = VX_NULL;
    vx_reference paramRef = VX_NULL;
    vx_binary_save_s *binarySave = VX_NULL;

    gcmHEADER_ARG("node=%p index=%d", node, index);

    if (node == VX_NULL)
    {
        vxError("%s[%d]: node is NULL\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_FAILURE);
    }

    kernel = node->kernel;
    paramRef = node->paramTable[index];
    binarySave = node->graph->binarySave;

    if (binarySave == VX_NULL)
    {
        goto OnError;
    }

    if (paramRef == VX_NULL)
    {
        vxError("%s[%d]: parameter index: %d is NULL\n", __FUNCTION__, __LINE__, index);
        vxmONERROR(VX_FAILURE);
    }

    if (kernel->signature.isStaticTable[index] == vx_true_e)
    {
        goto OnError;
    }

    if (node->kernel->signature.directionTable[index] == VX_INPUT)
    {
        vx_uint32 physical = 0;
        if (paramRef->type == VX_TYPE_TENSOR)
        {
            vx_tensor tensor = (vx_tensor)paramRef;

            if ((VX_TENSOR_LIFE_TIME_DYNAMIC != TENSOR_DATA_LIFETIME(tensor)) ||
                  (vxoMemory_GetType(&tensor->tensorBuffer->memory) == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR) ||
                  (node->kernel->signature.stateTable[index] == VX_PARAMETER_STATE_OPTIONAL))
            {
                goto OnError;
            }

            physical = TENSOR_PHYSICAL_ADDR(tensor);
        }
        else if (paramRef->type == VX_TYPE_IMAGE)
        {
            vx_image image = (vx_image)paramRef;
            if ((vxoMemory_GetType(&image->memory) == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR) ||
                (node->kernel->signature.stateTable[index] == VX_PARAMETER_STATE_OPTIONAL))
            {
                goto OnError;
            }
            physical = image->memory.physicals[0];
        }
        else if (paramRef->type == VX_TYPE_SCALAR)
        {
            if (node->kernel->signature.stateTable[index] != VX_PARAMETER_STATE_OPTIONAL)
            {
                vx_scalar scalar = (vx_scalar)paramRef;
                physical = scalar ->physical;
            }
        }

        if (physical != 0)
        {
            vx_uint32 i = 0, j = 0, k = 0;
            for (i = 0; i < binarySave->inputNodeCount; i++)
            {
                if (node == binarySave->inputNode[i].node)
                {
                    for (j = 0; j < binarySave->inputNode[i].count; j++)
                    {
                        if (index == binarySave->inputNode[i].paramIndex[j])
                        {
                            vx_uint32 inputTablePhysical = binarySave->inputNode[i].inputPhysical[j];
                            for (k  = 0; k < binarySave->inputParamCount; k++)
                            {
                                if (binarySave->inputPhysicalEntry[k] == inputTablePhysical)
                                {
                                    binarySave->inputPhysicalEntry[k] = physical;/* update input physical table */
                                    status = VX_SUCCESS;
                                    goto OnError;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

/*
    import binary graph support multiple input/output buffer
*/
VX_INTERNAL_API vx_status vxoBinaryGraph_SetParameter(
    vx_node node,
    vx_uint32 index
    )
{
    vx_status status = VX_SUCCESS;
    vx_kernel kernel = node->kernel;
    vx_binary_loader_s *binLoad = (vx_binary_loader_s*)kernel->base.reserved;
    vx_uint32 inputNum = binLoad->fixed.header.inputCount;
    vx_uint32 outputNum = binLoad->fixed.header.outputCount;
    vx_uint32 outIndex = index - inputNum;
    vx_uint32 i = 0;
    gcmHEADER_ARG("node=%p index=%d", node, index);

    if (binLoad->status != VX_BINARY_LOAD_STATUS_INIT)
    {
        gcmFOOTER_ARG("%d", status);
        return VX_SUCCESS;
    }

    if (index >= (inputNum + outputNum))
    {
        vxError("%s[%d]: fail to set Parameter index >= inputNum + outputNum: index%d\n",
            __FUNCTION__, __LINE__, index);
        vxmONERROR(VX_FAILURE);
    }

    if ((VX_TYPE_TENSOR == node->paramTable[index]->type) ||
        (VX_TYPE_SCALAR == node->paramTable[index]->type))
    {
        vx_uint32 physical = 0;
        if (VX_TYPE_TENSOR == node->paramTable[index]->type)
        {
            vx_tensor tensor = (vx_tensor)node->paramTable[index];
            if (!tensor->tensorBuffer->memory.allocated)
            {
                vxmONERROR(vxoTensor_AllocateMemory(tensor));
            }
            physical = TENSOR_PHYSICAL_ADDR(tensor);
        }
        else if (VX_TYPE_SCALAR == node->paramTable[index]->type)
        {
            vx_scalar scalar = (vx_scalar)node->paramTable[index];
            physical = scalar->physical;
        }

        if (index < inputNum)
        {
            vx_binary_io_patch_info_s *inputPatch = &binLoad->inputPatch[index];
            for (i = 0; i < inputPatch->number; i++)
            {
                *(inputPatch->references[i]) = inputPatch->offsets[i] + physical;
            }
        }
        else
        {
            vx_binary_io_patch_info_s *outputPatch = &binLoad->outputPatch[outIndex];
            for (i = 0; i < outputPatch->number; i++)
            {
                *(outputPatch->references[i]) = outputPatch->offsets[i] + physical;
            }
        }
    }
    else if (VX_TYPE_IMAGE == node->paramTable[index]->type)
    {
        vx_image image = (vx_image)node->paramTable[index];
        vx_uint32 physical = 0;

        if (index < inputNum)
        {
            vx_binary_io_patch_info_s *inputPatch = &binLoad->inputPatch[index];
            for (i = 0; i < inputPatch->number; i++)
            {
                if (1 == image->planeCount)
                {
                    physical = image->memory.physicals[0];
                }
                else
                {/* scaler operation, IYUV is mutli plane image */
                    physical = image->memory.physicals[i];
                }
                *(inputPatch->references[i]) = inputPatch->offsets[i] + physical;
            }
        }
        else
        {
            vx_binary_io_patch_info_s *outputPatch = &binLoad->outputPatch[outIndex];
            for (i = 0; i < outputPatch->number; i++)
            {
                if (1 == image->planeCount)
                {
                    physical = image->memory.physicals[0];
                }
                else
                {/* scaler operation, IYUV is mutli plane image */
                    physical = image->memory.physicals[i];
                }
                *(outputPatch->references[i]) = outputPatch->offsets[i] + physical;
            }
        }
    }

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API void* vxoBinaryGraph_GetInputOutputPtrByIndex(
    vx_binary_loader_s *binLoad,
    vx_binary_input_output_info_s *ioPtr,
    vx_int32 index
    )
{
    void *ptrInOut = VX_NULL;
    vx_uint32 size = 0;

    size = sizeof(vx_binary_input_output_info_s) - sizeof(vx_char) * VX_MAX_IO_NAME_LEGTH;

    if (binLoad->fixed.header.version >= 0x00010004)
    {
        ptrInOut = (void*)(ioPtr + index);
    }
    else
    {
        ptrInOut = (void*)((vx_char *)ioPtr + index * size);
    }

    return ptrInOut;
}

VX_INTERNAL_API void* vxoBinaryGraph_GetPatchPtrByIndex(
    vx_binary_loader_s *binLoad,
    vx_binary_patch_info_s *patchPtr,
    vx_int32 index
    )
{
    void *ptr = VX_NULL;
    vx_uint32 size = 0;

    size = sizeof(vx_binary_patch_info_s) - sizeof(vx_uint32);

    if (binLoad->fixed.header.version >= 0x00010006)
    {
        ptr = (void*)(patchPtr + index);
    }
    else
    {
        ptr = (void*)((vx_char *)patchPtr + index * size);
    }

    return ptr;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_GetNetworkNameAndRank(
    vx_binary_save_s *binarySave
    )
{
    vx_char networkName[VX_MAX_NAME_LEGTH] = "dummy_network_name";
    vx_char *env = VX_NULL;

    gcoOS_GetEnv(gcvNULL, "VIV_VX_NBG_NAME", &env);
    if (env != VX_NULL)
    {
        gcoOS_StrCopySafe(networkName, VX_MAX_NAME_LEGTH, env);
    }

    env = VX_NULL;
    gcoOS_GetEnv(gcvNULL, "VIV_VX_NBG_INPUT_RANK", &env);
    if (env != VX_NULL)
    {
        gcoOS_StrCatSafe(networkName, VX_MAX_NAME_LEGTH, "_");
        gcoOS_StrCatSafe(networkName, VX_MAX_NAME_LEGTH, env);
    }

    gcoOS_StrCopySafe(binarySave->headerInfo.networkName, VX_MAX_NAME_LEGTH, networkName);
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_GetFeatureDB(
    vx_graph graph,
    vx_binary_save_s *binarySave
    )
{
    vx_status status = VX_SUCCESS;
    vx_binary_header_s *headerInfo = &binarySave->headerInfo;
    gctUINT32 gpuCount = 1;

    headerInfo->featureDB.hi_reorder_fix = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_HI_REORDER_FIX);
    headerInfo->featureDB.ocb_counter = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_OCB_COUNTER);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_MULTI_PIXELPIPES))
    {
        headerInfo->featureDB.num_pixel_pipes = 2;
    }
    else
    {
        headerInfo->featureDB.num_pixel_pipes = 1;
    }

    gcmONERROR(gcoVX_QueryCoreCount(graph->deviceID, &gpuCount));
    headerInfo->featureDB.core_count = (vx_uint8)gpuCount;
    if (gpuCount > 255)
    {
        vxError("not support gpuCount: %d\n", gpuCount);
        vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
    }

OnError:
    return status;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_patchNN(
    vx_node node,
    vx_binary_nn_operation_info_s *NNData,
    vx_uint8 *commandBuf,
    vx_uint8 *statesBuf,
    vx_binary_operation_info_s *operation,
    vx_binary_loader_s *binLoad,
    vx_uint32 *stateSize,
    vx_binary_segment_s *segment
    )
{
    vx_status status = VX_SUCCESS;
    vx_uint32 cmdPhysical = 0xffff;
    vx_int32 ioIndex = -1;
    vx_uint32 i = 0;
    vx_uint32 j = 0;
    vx_uint32 *triggerWord = VX_NULL;
    vx_uint32 *memAddr = VX_NULL;
    vx_uint8 *NNStates = VX_NULL;
    vx_uint8 *NNCommand = VX_NULL;
    vx_binary_entry_s *lcdtDate = VX_NULL;
    vx_binary_patch_info_s *nnPatchData = VX_NULL;
    vx_kernel kernel = node->kernel;
    vx_reference *parameters = node->paramTable;
    gctPOINTER tensorLogical  = gcvNULL;
    vx_uint32 tensorPhysical  = 0;
    vx_tensor tensor = VX_NULL;
    vx_graph graph = node->graph;

    gcmHEADER_ARG("node=%p, NNData=%p, commandBuf=%p, statesBuf=%p, operation=%p, binLoad=%p, stateSize=%p",
        node, NNData, commandBuf, statesBuf, operation, binLoad, stateSize);

    NNCommand = commandBuf;
    NNStates = statesBuf;
    /*get GPU memory addr*/
    cmdPhysical = node->binLoadMem->nntpCmdBuff.physical +
                  gcmPTR2INT32(commandBuf - (vx_uint8 *)node->binLoadMem->nntpCmdBuff.logical);

    /* copy command data*/
    vxMemCopy((vx_ptr)NNCommand, (vx_const_ptr)NNData->nnCmd, sizeof(NNData->nnCmd));

    /* copy states*/
    lcdtDate = &binLoad->LCDT[operation->stateLCDTIndex];
    vxMemCopy(NNStates, (vx_uint8 *)binLoad->LCD.logical + lcdtDate->offset, lcdtDate->size);

    /* get states size*/
    if (stateSize != VX_NULL)
    {
        *stateSize = lcdtDate->size;
    }
     /* Patch addresses*/
    for (i = 0; i < operation->counterOfPatches; i++)
    {
        ioIndex = -1;
        nnPatchData = (vx_binary_patch_info_s*)vxoBinaryGraph_GetPatchPtrByIndex(binLoad, binLoad->patchData,
                                                                                 operation->indexOfFirstPatch + i);

        if (nnPatchData->type == VX_BINARY_PATCH_TYPE_STATE)
        {
            vxmASSERT(nnPatchData->sourceType == VX_BINARY_SOURCE_COMMAND);
            NNStates += nnPatchData->offset;/* 3*seizeof(uint32)*/
            triggerWord = (vx_uint32 *)NNStates;
            *triggerWord = ((*triggerWord) & 0x3f) | (cmdPhysical & ~0x3f);
        }
        else if (nnPatchData->type == VX_BINARY_PATCH_TYPE_COMMAND)
        {
            ioIndex = nnPatchData->index;
            switch(nnPatchData->sourceType)
            {
                case VX_BINARY_SOURCE_INPUT:
                {
                    /*Patch input. */
                    if (ioIndex >= 0)
                    {
                        vx_reference ref = VX_NULL;
                        if (graph->inputCount && graph->inputs)
                        {
                            ref = graph->inputs[ioIndex];
                        }
                        else
                        {
                            ref = parameters[ioIndex];
                        }

                        if (ref->type == VX_TYPE_TENSOR)
                        {
                            vx_int32 *dims = VX_NULL;
                            vx_uint32 dimCount = 0;
                            tensor = (vx_tensor)ref;
                            dims = TENSOR_ORIG_SIZES(tensor);
                            dimCount = TENSOR_ORIG_DIM_NUM(tensor);
                            for (j = 0; j < dimCount; j++)
                            {
                                vx_binary_input_output_info_s *ptrInOut = (vx_binary_input_output_info_s *)vxoBinaryGraph_GetInputOutputPtrByIndex(binLoad, binLoad->inputs, ioIndex);
                                if (dims[j] != (vx_int32)ptrInOut->dims[j])
                                    break;
                            }
                            if ((j == dimCount) && (kernel->signature.directionTable[ioIndex] == VX_INPUT))
                            {
                                vx_uint32 number = binLoad->inputPatch[ioIndex].number;
                                memAddr = (vx_uint32 *)(NNCommand + nnPatchData->offset);
                                binLoad->inputPatch[ioIndex].offsets[number] = *memAddr - nnPatchData->originalBaseAddress;
                                binLoad->inputPatch[ioIndex].references[number] = memAddr;
                                binLoad->inputPatch[ioIndex].number++;

                                if (!tensor->tensorBuffer->memory.allocated)
                                {
                                    vxmONERROR(vxoTensor_AllocateMemory(tensor));
                                }

                                vxoTensor_GetTensorBatchArrayViewMemory(tensor, 0, &tensorLogical, &tensorPhysical);
                                *memAddr = (*memAddr - nnPatchData->originalBaseAddress) + tensorPhysical;
                                tensorPhysical = 0;
                            }
                            else
                            {
                                vxError("nn patch input failed, please check your input format\n");
                                vxmONERROR(VX_ERROR_INVALID_FORMAT);
                            }
                        }
                        else
                        {
                            vxError("nn patch input failed, doesn't support this format : %d\n", ref->type);
                            vxmONERROR(VX_ERROR_INVALID_FORMAT);
                        }
                    }
                    else
                    {
                        vxmASSERT(0);
                        vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
                    }
                 }
                 break;

                case VX_BINARY_SOURCE_OUTPUT:
                {
                    /* Patch output. */
                    if (ioIndex >= 0)
                    {
                        vx_uint32 outIndex = ioIndex + binLoad->fixed.header.inputCount;
                        vx_reference ref = VX_NULL;
                        if (graph->outputCount && graph->outputs)
                        {
                            ref = graph->outputs[ioIndex];
                        }
                        else
                        {
                            ref = parameters[outIndex];
                        }

                        if (ref->type == VX_TYPE_TENSOR)
                        {
                            vx_int32 *dims = VX_NULL;
                            vx_uint32 dimCount = 0;
                            tensor = (vx_tensor)ref; /* get output tensor*/
                            dims = TENSOR_ORIG_SIZES(tensor);
                            dimCount = TENSOR_ORIG_DIM_NUM(tensor);
                            for (j = 0; j < dimCount; j++)
                            {
                                vx_binary_input_output_info_s *ptrInOut = (vx_binary_input_output_info_s *)vxoBinaryGraph_GetInputOutputPtrByIndex(binLoad, binLoad->outputs, ioIndex);
                                if (dims[j] != (vx_int32)ptrInOut->dims[j])
                                    break;
                            }
                            if ((j == dimCount) && (kernel->signature.directionTable[outIndex] == VX_OUTPUT))
                            {
                                vx_uint32 number = binLoad->outputPatch[ioIndex].number;

                                memAddr = (vx_uint32 *)(NNCommand + nnPatchData->offset);
                                binLoad->outputPatch[ioIndex].offsets[number] = *memAddr - nnPatchData->originalBaseAddress;
                                binLoad->outputPatch[ioIndex].references[number] = memAddr;
                                binLoad->outputPatch[ioIndex].number++;

                                if (!tensor->tensorBuffer->memory.allocated)
                                {
                                    vxmONERROR(vxoTensor_AllocateMemory(tensor));
                                }
                                vxoTensor_GetTensorBatchArrayViewMemory(tensor, 0, &tensorLogical, &tensorPhysical);
                                *memAddr = (*memAddr - nnPatchData->originalBaseAddress) + tensorPhysical;
                                tensorPhysical = 0;
                            }
                            else
                            {
                                vxError("nn patch output failed, please check your output format\n");
                                vxmONERROR(VX_ERROR_INVALID_FORMAT);
                            }
                        }
                        else
                        {
                            vxError("nn patch output failed, doesn't support this format : %d\n", ref->type);
                            vxmONERROR(VX_ERROR_INVALID_FORMAT);
                        }

                        if ((binLoad->context->options.enableNNLayerDump) && (segment != VX_NULL))
                        {
                            vx_size tensorSize = 0;
                            vx_uint32 outputCount = segment->outputCount;
                            vxoTensor_GetTensorWholeSize(tensor, &tensorSize);
                            segment->outputlogical[outputCount] = (vx_uint8_ptr)tensorLogical;
                            segment->outputSize[outputCount] = (vx_uint32)tensorSize;
                            segment->outputCount++;
                        }
                    }
                    else
                    {
                        vxError("%s[%d]: error: patch NN output, ioindex is  %d\n", __FUNCTION__, __LINE__, ioIndex);
                        vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
                    }
                }
                break;

                case VX_BINARY_SOURCE_MEMORY_POOL:/*patch input or output*/
                {
                    memAddr = (vx_uint32 *)(NNCommand + nnPatchData->offset);
                    if (nnPatchData->transformation == VX_BINARY_PATCH_TRANSFORMATION_RIGHT_SHIFT_6)
                    {
                        if (nnPatchData->originalBaseAddress != 0)
                        {
                            *memAddr <<= 6;
                        }
                    }

                    *memAddr = (*memAddr - nnPatchData->originalBaseAddress) + node->binLoadMem->pool.physical;
                    if (nnPatchData->transformation == VX_BINARY_PATCH_TRANSFORMATION_RIGHT_SHIFT_6)
                    {
                        *memAddr >>= 6;
                    }

                    if (binLoad->context->options.enableNNLayerDump)
                    {
                        vxError("%s[%d]: cann't support data type %d, please set ENV NN_LAYER_DUMP=1 and re-generate binary graph\n",
                                __FUNCTION__, __LINE__, nnPatchData->sourceType);
                    }
                }
                break;

                case VX_BINARY_SOURCE_AXI_SRAM:
                {
                    vx_uint32 srcAddr = 0;
                    vx_uint32 dstAddr = 0;
                    memAddr = (vx_uint32 *)(NNCommand + nnPatchData->offset);
                    srcAddr = *memAddr;
                    if (nnPatchData->transformation == VX_BINARY_PATCH_TRANSFORMATION_RIGHT_SHIFT_6)
                    {
                        if (nnPatchData->originalBaseAddress != 0)
                        {
                            srcAddr <<= 6;
                        }
                    }
                    dstAddr = (srcAddr - nnPatchData->originalBaseAddress) + binLoad->context->axiSRAM[graph->deviceID].physical;
                    if (nnPatchData->transformation == VX_BINARY_PATCH_TRANSFORMATION_RIGHT_SHIFT_6)
                    {
                        dstAddr = ((*memAddr) & 0xfc000000) | (dstAddr >> 6);
                    }
                    *memAddr = dstAddr;

                    if (binLoad->context->options.enableNNLayerDump)
                    {
                        vxError("%s[%d]: cann't support data type %d, please set ENV NN_LAYER_DUMP=1 and re-generate binary graph\n",
                                __FUNCTION__, __LINE__, nnPatchData->sourceType);
                    }
                }
                break;

                case VX_BINARY_SOURCE_VIP_SRAM:
                {
                    vx_uint32 srcAddr = 0;
                    vx_uint32 dstAddr = 0;
                    vx_context context = binLoad->context;
                    memAddr = (vx_uint32 *)(NNCommand + nnPatchData->offset);
                    srcAddr = *memAddr;
                    if (nnPatchData->transformation == VX_BINARY_PATCH_TRANSFORMATION_RIGHT_SHIFT_6)
                    {
                        if (nnPatchData->originalBaseAddress != 0)
                        {
                            srcAddr <<= 6;
                        }
                    }
                    dstAddr = (srcAddr - nnPatchData->originalBaseAddress) + binLoad->context->vipSRAM.physical - VX_VIP_SRAM_IMAGE_STREAM_SIZE;
                    if (nnPatchData->transformation == VX_BINARY_PATCH_TRANSFORMATION_RIGHT_SHIFT_6)
                    {
                        dstAddr = ((*memAddr) & 0xfc000000) | (dstAddr >> 6);
                    }
                    *memAddr = dstAddr;

                    if (binLoad->context->options.enableNNLayerDump)
                    {
                        vxError("%s[%d]: cann't support data type %d, please set ENV NN_LAYER_DUMP=1 and re-generate binary graph\n",
                                __FUNCTION__, __LINE__, nnPatchData->sourceType);
                    }
                }
                break;
                case VX_BINARY_SOURCE_MISC_DYNAMIC_GENERIC:
                {
                    vx_uint32 phyAddr = 0;
                    vx_uint32 dstAddr = 0;
                    memAddr  = (vx_uint32 *)(NNCommand + nnPatchData->offset);
                    phyAddr  = *memAddr << 6;
                    dstAddr = binLoad->LCD.physical + binLoad->LCDT[nnPatchData->index].offset
                              + (phyAddr - nnPatchData->originalBaseAddress);
                    /*Kernel base address is 0~25 only*/
                    dstAddr >>=  6;
                    *memAddr =  ((*memAddr) & 0xfc000000) | dstAddr;
                    gcmDUMP(gcvNULL, "#[info: NN KS data]");
                    gcmDUMP_BUFFER(gcvNULL, gcvDUMP_BUFFER_MEMORY, binLoad->LCD.physical,
                                    binLoad->LCD.logical, binLoad->LCDT[nnPatchData->index].offset,
                                    gcmALIGN(binLoad->LCDT[nnPatchData->index].size, 64));
                }
                break;

                case VX_BINARY_SOURCE_MISC_DYNAMIC_INPUT:
                {
                    memAddr  = (vx_uint32 *)(NNCommand + nnPatchData->offset);
                    *memAddr = binLoad->LCD.physical + binLoad->LCDT[nnPatchData->index].offset
                               + (*memAddr - nnPatchData->originalBaseAddress);
                }
                break;

                case VX_BINARY_SOURCE_MISC_DYNAMIC_OUTPUT:
                {
                    memAddr  = (vx_uint32 *)(NNCommand + nnPatchData->offset);
                    *memAddr = binLoad->LCD.physical + binLoad->LCDT[nnPatchData->index].offset
                               + (*memAddr - nnPatchData->originalBaseAddress);

                    if ((binLoad->context->options.enableNNLayerDump) && (segment != VX_NULL))
                    {
                        vx_uint32 outputCount = segment->outputCount;
                        vx_uint32 outputSize = binLoad->LCDT[nnPatchData->index].size;
                        vx_uint8_ptr outputlogical = binLoad->LCD.logical + binLoad->LCDT[nnPatchData->index].offset;/* base address to dump */

                        segment->outputlogical[outputCount] = outputlogical;
                        segment->outputSize[outputCount] = outputSize;
                        segment->outputCount++;
                    }
                }
                break;

                default:
                    vxError("not implement this sourceType: %d\n", nnPatchData->sourceType);
                break;
            }
        }
        else
        {
            vxError("NN patch type: %d\n", nnPatchData->type);
            vxmASSERT(0);
        }
    }

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_patchTP(
    vx_node node,
    vx_binary_tp_operation_info_s *TPData,
    vx_uint8 *commandBuf,
    vx_uint8 *statesBuf,
    vx_binary_operation_info_s *operation,
    vx_binary_loader_s *binLoad,
    vx_uint32 *stateSize,
    vx_binary_segment_s *segment
    )
{
    vx_binary_patch_info_s *tpPatchData = VX_NULL;
    vx_uint8 *TPCommand = VX_NULL;
    vx_uint32 *triggerWord = VX_NULL;
    vx_uint32 *memAddr = VX_NULL;
    vx_uint8 *TPStates = VX_NULL;
    vx_binary_entry_s *lcdtDate = VX_NULL;
    vx_status status = VX_SUCCESS;
    vx_uint32 cmdPhysical = 0xffff;
    vx_int32 ioIndex = -1;
    vx_uint32 i = 0;
    vx_uint32 j = 0;
    vx_kernel kernel = node->kernel;
    vx_reference *parameters = node->paramTable;
    gctPOINTER tensorLogical  = gcvNULL;
    vx_uint32 tensorPhysical  = 0;
    vx_tensor tensor = VX_NULL;
    vx_graph graph = node->graph;
    gcmHEADER_ARG("node=%p, TPData=%p, commandBuf=%p, statesBuf=%p, operation=%p, binLoad=%p, stateSize=%p",
        node, TPData, commandBuf, statesBuf, operation, binLoad, stateSize);

    TPCommand = commandBuf;
    TPStates = statesBuf;
    cmdPhysical = node->binLoadMem->nntpCmdBuff.physical +
                    gcmPTR2INT32(commandBuf - (vx_uint8 *)node->binLoadMem->nntpCmdBuff.logical);

    /* copy TP command data*/
    vxMemCopy(TPCommand, TPData->tpCmd, sizeof(TPData->tpCmd));

    /* copy TP states. */
    lcdtDate = &binLoad->LCDT[operation->stateLCDTIndex];
    vxMemCopy(TPStates, (vx_uint8 *)binLoad->LCD.logical + lcdtDate->offset, lcdtDate->size);

    if (stateSize != VX_NULL)
    {
        *stateSize = lcdtDate->size;
    }

    for (i = 0; i < operation->counterOfPatches; i++)
    {
        ioIndex = -1;
        tpPatchData = (vx_binary_patch_info_s*)vxoBinaryGraph_GetPatchPtrByIndex(binLoad, binLoad->patchData,
                                                                                 operation->indexOfFirstPatch + i);

        if (tpPatchData->type == VX_BINARY_PATCH_TYPE_STATE)
        {
            vxmASSERT(tpPatchData->sourceType == VX_BINARY_SOURCE_COMMAND);
            TPStates  += tpPatchData->offset;/*offset in cmd  is 1*sizeof(uint32) for TP*/
            triggerWord = (vx_uint32 *)TPStates;
            *triggerWord = ((*triggerWord) & 0x3f) | (cmdPhysical & ~0x3f);
        }
        else if (tpPatchData->type == VX_BINARY_PATCH_TYPE_COMMAND)
        {
            ioIndex = tpPatchData->index;
            switch (tpPatchData->sourceType)
            {
                case VX_BINARY_SOURCE_INPUT:
                {
                    /* patch input*/
                    if (ioIndex >= 0)
                    {
                        vx_reference ref = VX_NULL;
                        if (graph->inputCount && graph->inputs)
                        {
                            ref = graph->inputs[ioIndex];
                        }
                        else
                        {
                            ref = parameters[ioIndex];
                        }

                        if (ref->type == VX_TYPE_TENSOR)
                        {
                            vx_int32 *dims = VX_NULL;
                            vx_uint32 dimCount = 0;
                            tensor = (vx_tensor)ref;
                            dims = TENSOR_ORIG_SIZES(tensor);
                            dimCount = TENSOR_ORIG_DIM_NUM(tensor);
                            for (j = 0; j < dimCount; j++)
                            {
                                vx_binary_input_output_info_s *ptrInOut = (vx_binary_input_output_info_s *)vxoBinaryGraph_GetInputOutputPtrByIndex(binLoad, binLoad->inputs, ioIndex);
                                if (dims[j] != (vx_int32)ptrInOut->dims[j])
                                    break;
                            }
                            if ((j == dimCount) && (kernel->signature.directionTable[ioIndex] == VX_INPUT))
                            {
                                vx_uint32 number = binLoad->inputPatch[ioIndex].number;

                                memAddr = (vx_uint32 *)(TPCommand + tpPatchData->offset);
                                binLoad->inputPatch[ioIndex].offsets[number] = *memAddr - tpPatchData->originalBaseAddress;
                                binLoad->inputPatch[ioIndex].references[number] = memAddr;
                                binLoad->inputPatch[ioIndex].number++;

                                if (!tensor->tensorBuffer->memory.allocated)
                                {
                                    vxmONERROR(vxoTensor_AllocateMemory(tensor));
                                }

                                vxoTensor_GetTensorBatchArrayViewMemory(tensor, 0, &tensorLogical, &tensorPhysical);
                                *memAddr = (*memAddr - tpPatchData->originalBaseAddress) + tensorPhysical;
                                tensorPhysical = 0;
                            }
                            else
                            {
                                vxError("tp patch input failed, please check your input format\n");
                                vxmONERROR(VX_ERROR_INVALID_FORMAT);
                            }
                        }
                        else
                        {
                            vxError("tp patch input failed, doesn't support this format\n");
                            vxmONERROR(VX_ERROR_INVALID_FORMAT);
                        }
                    }
                    else
                    {
                        vxError("%s[%d]: error: patch TP input, ioindex is  %d\n", __FUNCTION__, __LINE__, ioIndex);
                        vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
                    }
                }
                break;

                case VX_BINARY_SOURCE_OUTPUT:
                {
                    /*patch output*/
                    if (ioIndex >= 0)
                    {
                        vx_uint32   outIndex = ioIndex + binLoad->fixed.header.inputCount;
                        vx_reference ref = VX_NULL;
                        if (graph->outputCount && graph->outputs)
                        {
                            ref = graph->outputs[ioIndex];
                        }
                        else
                        {
                            ref = parameters[outIndex];
                        }

                        if (ref->type == VX_TYPE_TENSOR)
                        {
                            vx_int32 *dims = VX_NULL;
                            vx_uint32 dimCount = 0;
                            tensor = (vx_tensor)ref; /* get output tensor*/
                            dims = TENSOR_ORIG_SIZES(tensor);
                            dimCount = TENSOR_ORIG_DIM_NUM(tensor);
                            for (j = 0; j < dimCount; j++)
                            {
                                vx_binary_input_output_info_s *ptrInOut = (vx_binary_input_output_info_s *)vxoBinaryGraph_GetInputOutputPtrByIndex(binLoad, binLoad->outputs, ioIndex);
                                if (dims[j] != (vx_int32)ptrInOut->dims[j])
                                    break;
                            }
                            if ((j == dimCount) && (kernel->signature.directionTable[outIndex] == VX_OUTPUT))
                            {
                                vx_uint32 number = binLoad->outputPatch[ioIndex].number;

                                memAddr = (vx_uint32 *)(TPCommand + tpPatchData->offset);
                                binLoad->outputPatch[ioIndex].offsets[number] = *memAddr - tpPatchData->originalBaseAddress;
                                binLoad->outputPatch[ioIndex].references[number] = memAddr;
                                binLoad->outputPatch[ioIndex].number++;

                                if (!tensor->tensorBuffer->memory.allocated)
                                {
                                    vxmONERROR(vxoTensor_AllocateMemory(tensor));
                                }
                                vxoTensor_GetTensorBatchArrayViewMemory(tensor, 0, &tensorLogical, &tensorPhysical);
                                *memAddr = (*memAddr - tpPatchData->originalBaseAddress) + tensorPhysical;
                                tensorPhysical = 0;
                            }
                            else
                            {
                                vxError("tp patch output failed, please check your output format\n");
                                vxmONERROR(VX_ERROR_INVALID_FORMAT);
                            }
                        }
                        else
                        {
                            vxError("tp patch output failed, doesn't support this format : %d\n", ref->type);
                            vxmONERROR(VX_ERROR_INVALID_FORMAT);
                        }

                        if ((binLoad->context->options.enableNNLayerDump) && (segment != VX_NULL))
                        {
                            vx_size tensorSize = 0;
                            vx_uint32 outputCount = segment->outputCount;
                            vxoTensor_GetTensorWholeSize(tensor, &tensorSize);
                            segment->outputlogical[outputCount] = (vx_uint8_ptr)tensorLogical;
                            segment->outputSize[outputCount] = (vx_uint32)tensorSize;
                            segment->outputCount++;
                        }
                    }
                    else
                    {
                        vxError("%s[%d]: error: patch NN output, ioindex is  %d\n", __FUNCTION__, __LINE__, ioIndex);
                        vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
                    }
                }
                break;

                case VX_BINARY_SOURCE_MEMORY_POOL:
                {
                    memAddr = (vx_uint32 *)(TPCommand + tpPatchData->offset);
                    if (tpPatchData->transformation== VX_BINARY_PATCH_TRANSFORMATION_RIGHT_SHIFT_6)
                    {
                        if (tpPatchData->originalBaseAddress != 0)
                        {
                            *memAddr <<= 6;
                        }
                    }
                    *memAddr = (*memAddr - tpPatchData->originalBaseAddress) + node->binLoadMem->pool.physical;
                    if (tpPatchData->transformation == VX_BINARY_PATCH_TRANSFORMATION_RIGHT_SHIFT_6)
                    {
                        *memAddr >>= 6;
                    }

                    if (binLoad->context->options.enableNNLayerDump)
                    {
                        vxError("%s[%d]: cann't support data type %d, please set ENV NN_LAYER_DUMP=1 and re-generate binary graph\n",
                                __FUNCTION__, __LINE__, tpPatchData->sourceType);
                    }
                }
                break;

                case VX_BINARY_SOURCE_AXI_SRAM:
                {
                    vx_uint32 srcAddr = 0;
                    vx_uint32 dstAddr = 0;
                    memAddr = (vx_uint32 *)(TPCommand + tpPatchData->offset);
                    srcAddr = *memAddr;
                    if (tpPatchData->transformation== VX_BINARY_PATCH_TRANSFORMATION_RIGHT_SHIFT_6)
                    {
                        if (tpPatchData->originalBaseAddress != 0)
                        {
                            srcAddr <<= 6;
                        }
                    }
                    dstAddr = (srcAddr - tpPatchData->originalBaseAddress) + binLoad->context->axiSRAM[graph->deviceID].physical;
                    if (tpPatchData->transformation == VX_BINARY_PATCH_TRANSFORMATION_RIGHT_SHIFT_6)
                    {
                        dstAddr = ((*memAddr) & 0xfc000000) | (dstAddr >> 6);
                    }
                    *memAddr = dstAddr;

                    if (binLoad->context->options.enableNNLayerDump)
                    {
                        vxError("%s[%d]: cann't support data type %d, please set ENV NN_LAYER_DUMP=1 and re-generate binary graph\n",
                                __FUNCTION__, __LINE__, tpPatchData->sourceType);
                    }
                }
                break;

                case VX_BINARY_SOURCE_VIP_SRAM:
                {
                    vx_uint32 srcAddr = 0;
                    vx_uint32 dstAddr = 0;
                    vx_context context = binLoad->context;
                    memAddr = (vx_uint32 *)(TPCommand + tpPatchData->offset);
                    srcAddr = *memAddr;
                    if (tpPatchData->transformation== VX_BINARY_PATCH_TRANSFORMATION_RIGHT_SHIFT_6)
                    {
                        if (tpPatchData->originalBaseAddress != 0)
                        {
                            srcAddr <<= 6;
                        }
                    }
                    dstAddr = (srcAddr - tpPatchData->originalBaseAddress) + binLoad->context->vipSRAM.physical - VX_VIP_SRAM_IMAGE_STREAM_SIZE;
                    if (tpPatchData->transformation == VX_BINARY_PATCH_TRANSFORMATION_RIGHT_SHIFT_6)
                    {
                        dstAddr = ((*memAddr) & 0xfc000000) | (dstAddr >> 6);
                    }
                    *memAddr = dstAddr;

                    if (binLoad->context->options.enableNNLayerDump)
                    {
                        vxError("%s[%d]: cann't support data type %d, please set ENV NN_LAYER_DUMP=1 and re-generate binary graph\n",
                                __FUNCTION__, __LINE__, tpPatchData->sourceType);
                    }
                }
                break;

                case VX_BINARY_SOURCE_MISC_DYNAMIC_GENERIC:
                {
                    memAddr = (vx_uint32 *)(TPCommand + tpPatchData->offset);
                    *memAddr = binLoad->LCD.physical + binLoad->LCDT[tpPatchData->index].offset
                               + (*memAddr - tpPatchData->originalBaseAddress);
                    gcmDUMP(gcvNULL, "#[info: TP KS data]");
                    gcmDUMP_BUFFER(gcvNULL, gcvDUMP_BUFFER_MEMORY, binLoad->LCD.physical,
                                binLoad->LCD.logical, binLoad->LCDT[tpPatchData->index].offset,
                                gcmALIGN(binLoad->LCDT[tpPatchData->index].size, 64));
                }
                break;

                case VX_BINARY_SOURCE_MISC_DYNAMIC_INPUT:
                {
                    memAddr = (vx_uint32 *)(TPCommand + tpPatchData->offset);
                    *memAddr = binLoad->LCD.physical + binLoad->LCDT[tpPatchData->index].offset
                               + (*memAddr - tpPatchData->originalBaseAddress);
                }
                break;

                case VX_BINARY_SOURCE_MISC_DYNAMIC_OUTPUT:
                {
                    memAddr = (vx_uint32 *)(TPCommand + tpPatchData->offset);
                    *memAddr = binLoad->LCD.physical + binLoad->LCDT[tpPatchData->index].offset
                               + (*memAddr - tpPatchData->originalBaseAddress);

                    if ((binLoad->context->options.enableNNLayerDump) && (segment != VX_NULL))
                    {
                        vx_uint32 outputCount = segment->outputCount;
                        vx_uint32 outputSize = binLoad->LCDT[tpPatchData->index].size;
                        vx_uint8_ptr outputlogical = binLoad->LCD.logical + binLoad->LCDT[tpPatchData->index].offset;/* base address to dump*/

                        segment->outputlogical[outputCount] = outputlogical;
                        segment->outputSize[outputCount] = outputSize;
                        segment->outputCount++;
                    }
                }
                break;

                default:
                    vxError("not implement this sourceType: %d\n", tpPatchData->sourceType);
                break;
            }
        }
        else
        {
            vxError("TP patch type: %d\n", tpPatchData->type);
            vxmASSERT(0);
        }
    }

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_patchSH(
    vx_node node,
    vx_binary_sh_operation_info_s *SHDate,
    vx_uint8 *SHCommand,
    vx_uint8 *SHStates,
    vx_binary_operation_info_s *operation,
    vx_binary_loader_s *binLoad,
    vx_uint32 *cmdBufSize,
    vx_uint32 *stateSize,
    vx_binary_segment_s *segment
    )
{
    vx_binary_patch_info_s *shPatchData = VX_NULL;
    vx_binary_entry_s *lcdtDate = VX_NULL;
    vx_uint32 *memAddr = VX_NULL;
    vx_status status = VX_SUCCESS;
    vx_int32 ioIndex = 0;
    vx_int32 lcdtIndex = 0;
    vx_uint32 i = 0;
    vx_uint32 j = 0;
    vx_kernel kernel = node->kernel;
    vx_reference *parameters = node->paramTable;
    gctPOINTER tensorLogical  = gcvNULL;
    vx_uint32 tensorPhysical  = 0;
    vx_tensor tensor = VX_NULL;
    vx_graph graph = node->graph;

    gcmHEADER_ARG("node=%p, SHDate=%p, SHCommand=%p, SHStates=%p, operation=%p, binLoad=%p, cmdBufSize=%p, stateSize=%p",
        node, SHDate, SHCommand, SHStates, operation, binLoad, cmdBufSize, stateSize);

    /* copy SH states*/
    lcdtDate = &binLoad->LCDT[operation->stateLCDTIndex];
    vxMemCopy((vx_ptr)SHStates, (vx_const_ptr)((vx_uint8 *)binLoad->LCD.logical+ lcdtDate->offset),
              (vx_size)lcdtDate->size);
    if (stateSize != VX_NULL)
    {
        *stateSize = lcdtDate->size;
    }
    /* copy SH instruction CMD*/
    lcdtDate = &binLoad->LCDT[SHDate->instructionCmdLCDTIndex];
    vxMemCopy((vx_ptr)SHCommand, (vx_const_ptr)((vx_uint8 *)binLoad->LCD.logical+ lcdtDate->offset),
              (vx_size)lcdtDate->size);
    if (cmdBufSize != VX_NULL)
    {
        *cmdBufSize = lcdtDate->size;
    }

    /* patch SH states */
    for (i = 0; i < operation->counterOfPatches; i++)
    {
        shPatchData = (vx_binary_patch_info_s*)vxoBinaryGraph_GetPatchPtrByIndex(binLoad, binLoad->patchData,
                                                                                 operation->indexOfFirstPatch + i);

        ioIndex = shPatchData->index;
        if (shPatchData->type == VX_BINARY_PATCH_TYPE_STATE)
        {
            switch (shPatchData->sourceType)
            {
            case VX_BINARY_SOURCE_COMMAND:/* Patch the command address to this states offset. */
            {
                vx_uint32 *pStateAddr = (vx_uint32*)(SHStates + shPatchData->offset);
                vx_uint32 cmdPhysical = node->binLoadMem->shCmdBuff.physical +
                                        gcmPTR2INT32(SHCommand - node->binLoadMem->shCmdBuff.logical);
                if (shPatchData->transformation== VX_BINARY_PATCH_TRANSFORMATION_RIGHT_SHIFT_6)
                {
                    cmdPhysical >>= 6;
                }
                *pStateAddr = cmdPhysical;
            }
            break;

            case VX_BINARY_SOURCE_INPUT:
            {
                if (ioIndex >= 0)
                {
                    vx_uint32 number = binLoad->inputPatch[ioIndex].number;
                    vx_reference ref = VX_NULL;
                    if (graph->inputCount && graph->inputs)
                    {
                        ref = graph->inputs[ioIndex];
                    }
                    else
                    {
                        ref = parameters[ioIndex];
                    }

                    memAddr = (vx_uint32 *) (SHStates + shPatchData->offset);
                    binLoad->inputPatch[ioIndex].offsets[number] = *memAddr - shPatchData->originalBaseAddress;
                    binLoad->inputPatch[ioIndex].references[number] = memAddr;
                    binLoad->inputPatch[ioIndex].number++;

                    if (ref->type == VX_TYPE_SCALAR)
                    {
                        vx_scalar scalar =  (vx_scalar)ref;
                        vx_binary_input_output_info_s *ptrInOut = (vx_binary_input_output_info_s *)vxoBinaryGraph_GetInputOutputPtrByIndex(binLoad, binLoad->inputs, ioIndex);
                        if ((kernel->signature.directionTable[ioIndex] == VX_INPUT) &&
                            (vxoScalar_GetTypeSize(scalar) == ptrInOut->dims[0]))
                        {
                            *memAddr = (*memAddr - shPatchData->originalBaseAddress) + scalar->physical;
                        }
                        else
                        {
                            vxError("sh patch input failed, please check your input scalar data format\n");
                            vxmONERROR(VX_ERROR_INVALID_FORMAT);
                        }
                    }
                    else if (ref->type == VX_TYPE_TENSOR)
                    {
                        vx_int32 *dims = VX_NULL;
                        vx_uint32 dimCount = 0;
                        tensor = (vx_tensor)ref;
                        dims = TENSOR_ORIG_SIZES(tensor);
                        dimCount = TENSOR_ORIG_DIM_NUM(tensor);
                        for (j = 0; j < dimCount; j++)
                        {
                            vx_binary_input_output_info_s *ptrInOut = (vx_binary_input_output_info_s *)vxoBinaryGraph_GetInputOutputPtrByIndex(binLoad, binLoad->inputs, ioIndex);
                            if (dims[j] != (vx_int32)ptrInOut->dims[j])
                                break;
                        }
                        if ((j == dimCount) && (kernel->signature.directionTable[ioIndex] == VX_INPUT))
                        {
                            if (!tensor->tensorBuffer->memory.allocated)
                            {
                                vxmONERROR(vxoTensor_AllocateMemory(tensor));
                            }
                            vxoTensor_GetTensorBatchArrayViewMemory(tensor, 0, &tensorLogical, &tensorPhysical);
                            *memAddr = (*memAddr - shPatchData->originalBaseAddress) + tensorPhysical;
                            tensorPhysical = 0;
                        }
                        else
                        {
                            vxError("SH patch input failed, please check your input format\n");
                            vxmONERROR(VX_ERROR_INVALID_FORMAT);
                        }
                    }
                    else if (ref->type == VX_TYPE_IMAGE)
                    {
                        vx_binary_input_output_info_s *ptrInOut = VX_NULL;
                        vx_image image = (vx_image)ref;
                        gcoVX_Kernel_Context kernelContext; /* not useful, just fulfill the interface */
                        gcsVX_IMAGE_INFO imageInfo;
                        INITIALIZE_STRUCT(imageInfo);

                        gcoOS_MemFill((gctPOINTER*)(&kernelContext), 0, sizeof(gcoVX_Kernel_Context));
                        gcfVX_GetImageInfo(&kernelContext, image, &imageInfo, 1);

                        ptrInOut = (vx_binary_input_output_info_s *)vxoBinaryGraph_GetInputOutputPtrByIndex(binLoad, binLoad->inputs, ioIndex);
                        if ((kernel->signature.directionTable[ioIndex] == VX_INPUT) &&
                            (imageInfo.width == ptrInOut->dims[0]) &&
                            (imageInfo.height == ptrInOut->dims[1]) &&
                            (image->memory.allocated))
                        {
                            *memAddr = (*memAddr - shPatchData->originalBaseAddress) + image->memory.physicals[0];
                        }
                        else
                        {
                            vxError("sh patch input failed, please check your input image data format\n");
                            vxmONERROR(VX_ERROR_INVALID_FORMAT);
                        }
                    }
                    else if (ref->type == VX_TYPE_ARRAY)
                    {
                        vx_array array = (vx_array)ref;
                        if (kernel->signature.directionTable[ioIndex] == VX_INPUT)
                        {
                            *memAddr = (*memAddr - shPatchData->originalBaseAddress) + array->memory.physicals[0];
                        }
                        else
                        {
                            vxError("sh patch input failed, please check your input array data format\n");
                            vxmONERROR(VX_ERROR_INVALID_FORMAT);
                        }
                    }
                    else
                    {
                        vxError("sh patch input failed, doesn't support this format\n");
                        vxmONERROR(VX_ERROR_INVALID_FORMAT);
                    }
                }
                else
                {
                    vxError("%s[%d]: error: patch shader input, ioindex is  %d\n", __FUNCTION__, __LINE__, ioIndex);
                    vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
                }
            }
            break;

            case VX_BINARY_SOURCE_OUTPUT:
            {
                if (ioIndex >= 0)
                {
                    vx_uint32 number = binLoad->outputPatch[ioIndex].number;
                    vx_uint32 outIndex = ioIndex + binLoad->fixed.header.inputCount;
                    vx_reference ref = VX_NULL;
                    if (graph->outputCount && graph->outputs)
                    {
                        ref = graph->outputs[ioIndex];
                    }
                    else
                    {
                        ref = parameters[outIndex];
                    }
                    memAddr = (vx_uint32 *) (SHStates + shPatchData->offset);
                    binLoad->outputPatch[ioIndex].offsets[number] = *memAddr - shPatchData->originalBaseAddress;
                    binLoad->outputPatch[ioIndex].references[number] = memAddr;
                    binLoad->outputPatch[ioIndex].number++;

                    if (ref->type == VX_TYPE_TENSOR)
                    {
                        vx_int32 *dims = VX_NULL;
                        vx_uint32 dimCount = 0;
                        tensor = (vx_tensor)ref; /* get output tensor*/
                        dims = TENSOR_ORIG_SIZES(tensor);
                        dimCount = TENSOR_ORIG_DIM_NUM(tensor);

                        for (j = 0; j < dimCount; j++)
                        {
                            vx_binary_input_output_info_s *ptrInOut = (vx_binary_input_output_info_s *)vxoBinaryGraph_GetInputOutputPtrByIndex(binLoad, binLoad->outputs, ioIndex);
                            if (dims[j] != (vx_int32)ptrInOut->dims[j])
                                break;
                        }
                        if ((j == dimCount) && (kernel->signature.directionTable[outIndex] == VX_OUTPUT))
                        {
                            if (!tensor->tensorBuffer->memory.allocated)
                            {
                                vxmONERROR(vxoTensor_AllocateMemory(tensor));
                            }
                            vxoTensor_GetTensorBatchArrayViewMemory(tensor, 0, &tensorLogical, &tensorPhysical);
                            *memAddr = (*memAddr - shPatchData->originalBaseAddress) + tensorPhysical;
                            tensorPhysical = 0;
                        }
                        else
                        {
                            vxError("sh patch output failed, please check your output format\n");
                            vxmONERROR(VX_ERROR_INVALID_FORMAT);
                        }
                    }
                    else if (ref->type == VX_TYPE_IMAGE)
                    {
                        vx_binary_input_output_info_s *ptrInOut = VX_NULL;
                        vx_image image = (vx_image)ref;
                        gcoVX_Kernel_Context kernelContext; /* not useful, just fulfill the interface */
                        gcsVX_IMAGE_INFO imageInfo;
                        INITIALIZE_STRUCT(imageInfo);

                        gcoOS_MemFill((gctPOINTER*)(&kernelContext), 0, sizeof(gcoVX_Kernel_Context));
                        gcfVX_GetImageInfo(&kernelContext, image, &imageInfo, 1);

                        ptrInOut = (vx_binary_input_output_info_s *)vxoBinaryGraph_GetInputOutputPtrByIndex(binLoad, binLoad->outputs, ioIndex);
                        if ((kernel->signature.directionTable[outIndex] == VX_OUTPUT) &&
                            (imageInfo.width == ptrInOut->dims[0]) &&
                            (imageInfo.height == ptrInOut->dims[1]) &&
                            (image->memory.allocated))
                        {
                            *memAddr = (*memAddr - shPatchData->originalBaseAddress) + image->memory.physicals[0];
                        }
                        else
                        {
                            vxError("sh patch output failed, please check your output image data format\n");
                            vxmONERROR(VX_ERROR_INVALID_FORMAT);
                        }
                    }
                    else if (ref->type == VX_TYPE_ARRAY)
                    {
                        vx_array array = (vx_array)ref;
                        if (kernel->signature.directionTable[outIndex] == VX_OUTPUT)
                        {
                            *memAddr = (*memAddr - shPatchData->originalBaseAddress) + array->memory.physicals[0];
                        }
                        else
                        {
                            vxError("sh patch output failed, please check your output array data format\n");
                            vxmONERROR(VX_ERROR_INVALID_FORMAT);
                        }
                    }
                    else if (ref->type == VX_TYPE_SCALAR)
                    {
                        vx_scalar scalar =  (vx_scalar)ref;
                        vx_uint32 size = vxoScalar_GetTypeSize(scalar);
                        vx_binary_input_output_info_s *ptrInOut = (vx_binary_input_output_info_s *)vxoBinaryGraph_GetInputOutputPtrByIndex(binLoad, binLoad->outputs, ioIndex);
                        vx_uint32 dim = (vx_uint32)ptrInOut->dims[0];
                        if ((kernel->signature.directionTable[outIndex] == VX_OUTPUT) &&
                            (size == dim))
                        {
                            *memAddr = (*memAddr - shPatchData->originalBaseAddress) + scalar->physical;
                        }
                        else
                        {
                            vxError("sh patch output failed, please check your output scalar data format\n");
                            vxmONERROR(VX_ERROR_INVALID_FORMAT);
                        }
                    }
                    else
                    {
                        vxError("sh patch output failed, doesn't support this format : %d\n", ref->type);
                        vxmONERROR(VX_ERROR_INVALID_FORMAT);
                    }

                    if ((binLoad->context->options.enableNNLayerDump) && (segment != VX_NULL))
                    {
                        vx_size tensorSize = 0;
                        vx_uint32 outputCount = segment->outputCount;
                        vxoTensor_GetTensorWholeSize(tensor, &tensorSize);
                        segment->outputlogical[outputCount] = (vx_uint8_ptr)tensorLogical;
                        segment->outputSize[outputCount] = (vx_uint32)tensorSize;
                        segment->outputCount++;
                    }
                }
                else
                {
                    vxError("%s[%d]: error: patch shader output, ioindex is  %d\n", __FUNCTION__, __LINE__, ioIndex);
                    vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
                }
            }
            break;

            case VX_BINARY_SOURCE_MEMORY_POOL:
            {
                memAddr = (vx_uint32 *)(SHStates + shPatchData->offset);
                if (shPatchData->transformation== VX_BINARY_PATCH_TRANSFORMATION_RIGHT_SHIFT_6)
                {
                    if (shPatchData->originalBaseAddress != 0)
                    {
                        *memAddr <<= 6;
                    }
                }
                *memAddr = node->binLoadMem->pool.physical + (*memAddr - shPatchData->originalBaseAddress);
                if (shPatchData->transformation == VX_BINARY_PATCH_TRANSFORMATION_RIGHT_SHIFT_6)
                {
                    *memAddr >>= 6;
                }

                if (binLoad->context->options.enableNNLayerDump)
                {
                    vxError("%s[%d]: cann't support data type %d, please set ENV NN_LAYER_DUMP=1 and re-generate binary graph\n",
                            __FUNCTION__, __LINE__, shPatchData->sourceType);
                }
            }
            break;

            case VX_BINARY_SOURCE_AXI_SRAM:
            {
                memAddr = (vx_uint32 *)(SHStates + shPatchData->offset);
                if (shPatchData->transformation== VX_BINARY_PATCH_TRANSFORMATION_RIGHT_SHIFT_6)
                {
                    if (shPatchData->originalBaseAddress != 0)
                    {
                        *memAddr <<= 6;
                    }
                }
                *memAddr =  (*memAddr - shPatchData->originalBaseAddress) + binLoad->context->axiSRAM[graph->deviceID].physical;
                if (shPatchData->transformation == VX_BINARY_PATCH_TRANSFORMATION_RIGHT_SHIFT_6)
                {
                    *memAddr >>= 6;
                }

                if (binLoad->context->options.enableNNLayerDump)
                {
                    vxError("%s[%d]: cann't support data type %d, please set ENV NN_LAYER_DUMP=1 and re-generate binary graph\n",
                            __FUNCTION__, __LINE__, shPatchData->sourceType);
                }
            }
            break;

            case VX_BINARY_SOURCE_VIP_SRAM:
            {
                vx_context context = binLoad->context;
                memAddr = (vx_uint32 *)(SHStates + shPatchData->offset);
                if (shPatchData->transformation== VX_BINARY_PATCH_TRANSFORMATION_RIGHT_SHIFT_6)
                {
                    if (shPatchData->originalBaseAddress != 0)
                    {
                        *memAddr <<= 6;
                    }
                }
                *memAddr =  (*memAddr - shPatchData->originalBaseAddress) + binLoad->context->vipSRAM.physical - VX_VIP_SRAM_IMAGE_STREAM_SIZE;
                if (shPatchData->transformation == VX_BINARY_PATCH_TRANSFORMATION_RIGHT_SHIFT_6)
                {
                    *memAddr >>= 6;
                }

                if (binLoad->context->options.enableNNLayerDump)
                {
                    vxError("%s[%d]: cann't support data type %d, please set ENV NN_LAYER_DUMP=1 and re-generate binary graph\n",
                            __FUNCTION__, __LINE__, shPatchData->sourceType);
                }
            }
            break;

            case VX_BINARY_SOURCE_MISC_DYNAMIC_GENERIC:
            {
                lcdtIndex = shPatchData->index;
                memAddr = (vx_uint32 *)(SHStates + shPatchData->offset);
                *memAddr = binLoad->LCD.physical + binLoad->LCDT[lcdtIndex].offset + (*memAddr - shPatchData->originalBaseAddress);
                gcmDUMP(gcvNULL, "#[info: SH misc dynamic]");
                gcmDUMP_BUFFER(gcvNULL, gcvDUMP_BUFFER_MEMORY, binLoad->LCD.physical,
                                binLoad->LCD.logical, binLoad->LCDT[lcdtIndex].offset,
                                (binLoad->LCDT[lcdtIndex].size, 64));
            }
            break;

            case VX_BINARY_SOURCE_MISC_DYNAMIC_INPUT:
            {
                lcdtIndex = shPatchData->index;
                memAddr = (vx_uint32 *)(SHStates + shPatchData->offset);
                *memAddr = binLoad->LCD.physical + binLoad->LCDT[lcdtIndex].offset + (*memAddr - shPatchData->originalBaseAddress);
            }
            break;

            case VX_BINARY_SOURCE_MISC_DYNAMIC_OUTPUT:
            {
                lcdtIndex = shPatchData->index;
                memAddr = (vx_uint32 *)(SHStates + shPatchData->offset);
                *memAddr = binLoad->LCD.physical + binLoad->LCDT[lcdtIndex].offset + (*memAddr - shPatchData->originalBaseAddress);

                if ((binLoad->context->options.enableNNLayerDump) && (segment != VX_NULL))
                {
                    vx_uint32 outputCount = segment->outputCount;
                    vx_uint32 outputSize = binLoad->LCDT[shPatchData->index].size;
                    vx_uint8_ptr outputlogical = binLoad->LCD.logical + binLoad->LCDT[shPatchData->index].offset; /* base address to dump */

                    segment->outputlogical[outputCount] = outputlogical;
                    segment->outputSize[outputCount] = outputSize;
                    segment->outputCount++;
                }
            }
            break;

            default:
                vxError("shader not implement this sourceType: %d\n", shPatchData->sourceType);
            break;
          }
        }
        else
        {
            vxError("%s[%d]:  note support this shader patch type: %d\n", __FUNCTION__, __LINE__, shPatchData->type);
            vxmONERROR(VX_FAILURE);
        }
    }

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_patchSC(
    vx_node node,
    vx_uint8 *SCStates,
    vx_binary_operation_info_s *operation,
    vx_binary_loader_s *binLoad,
    vx_uint32 *stateSize,
    vx_binary_segment_s *segment
    )
{
    vx_status status = VX_SUCCESS;
    vx_binary_entry_s *lcdtDate = VX_NULL;
    vx_binary_patch_info_s *scPatchData = VX_NULL;
    vx_reference *parameters = node->paramTable;
    vx_kernel kernel = node->kernel;
    vx_tensor tensor = VX_NULL;
    vx_uint32 i = 0;
    vx_uint32 j = 0;
    vx_int32 ioIndex = 0;
    vx_int32 lcdtIndex = 0;
    vx_uint32 *memAddr = VX_NULL;
    vx_uint32 inputImagePlane = 0;
    vx_graph graph = node->graph;
    gcmHEADER_ARG("node=%p, SCStates=%p, operation=%p, binLoad=%p, stateSize=%p", node, SCStates, operation, binLoad, stateSize);

    /* copy SC states*/
    lcdtDate = &binLoad->LCDT[operation->stateLCDTIndex];
    vxMemCopy((vx_ptr)SCStates, (vx_const_ptr)((vx_uint8 *)binLoad->LCD.logical+ lcdtDate->offset),
              (vx_size)lcdtDate->size);

    if (stateSize != VX_NULL)
    {
        *stateSize = lcdtDate->size;
    }

    /* patch SC states */
    for (i = 0; i < operation->counterOfPatches; i++)
    {
        scPatchData = (vx_binary_patch_info_s*)vxoBinaryGraph_GetPatchPtrByIndex(binLoad, binLoad->patchData,
                                                                                 operation->indexOfFirstPatch + i);

        ioIndex = scPatchData->index;
        if (scPatchData->type == VX_BINARY_PATCH_TYPE_STATE)
        {
            switch (scPatchData->sourceType)
            {
                case VX_BINARY_SOURCE_INPUT:
                {
                    if (ioIndex >= 0)
                    {
                        vx_reference ref = VX_NULL;
                        vx_uint32 number = binLoad->inputPatch[ioIndex].number;
                        if (graph->inputCount && graph->inputs)
                        {
                            ref = graph->inputs[ioIndex];
                        }
                        else
                        {
                            ref = parameters[ioIndex];
                        }

                        memAddr = (vx_uint32 *) (SCStates + scPatchData->offset);
                        binLoad->inputPatch[ioIndex].offsets[number] = *memAddr - scPatchData->originalBaseAddress;
                        binLoad->inputPatch[ioIndex].references[number] = memAddr;
                        binLoad->inputPatch[ioIndex].number++;

                        if (ref->type == VX_TYPE_IMAGE)
                        {
                            vx_binary_input_output_info_s *ptrInOut = VX_NULL;
                            vx_image image = (vx_image)ref;
                            gcoVX_Kernel_Context kernelContext; /* not useful, just fulfill the interface */
                            gcsVX_IMAGE_INFO imageInfo;
                            INITIALIZE_STRUCT(imageInfo);

                            gcfVX_GetImageInfo(&kernelContext, image, &imageInfo, 1);

                            ptrInOut = (vx_binary_input_output_info_s *)vxoBinaryGraph_GetInputOutputPtrByIndex(binLoad, binLoad->inputs, ioIndex);
                            if ((kernel->signature.directionTable[ioIndex] == VX_INPUT) &&
                                (imageInfo.width == ptrInOut->dims[0]) &&
                                (imageInfo.height == ptrInOut->dims[1]) &&
                                (image->memory.allocated))
                            {
                                vx_uint32 planePhysicalBase = image->memory.physicals[inputImagePlane];
                                *memAddr = (*memAddr - scPatchData->originalBaseAddress) + planePhysicalBase;
                            }
                            else
                            {
                                vxError("%s[%d]: error: scaler patch input image\n", __FUNCTION__, __LINE__);
                                vxmONERROR(VX_FAILURE);
                            }
                            inputImagePlane++;
                            if (inputImagePlane > image->planeCount)
                            {
                                vxError("%s[%d]: error: scaler inputImagePlane >= image->planeCount\n", __FUNCTION__, __LINE__);
                                vxmONERROR(VX_FAILURE);
                            }
                        }
                        else
                        {
                            vxError("%s[%d]: error: patch scaler input type: %d\n", __FUNCTION__, __LINE__, ref->type);
                            vxmONERROR(VX_FAILURE);
                        }
                    }
                    else
                    {
                        vxError("%s[%d]: error: patch scaler input, ioindex is %d\n", __FUNCTION__, __LINE__, ioIndex);
                        vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
                    }
                }
                break;

                case VX_BINARY_SOURCE_OUTPUT:
                {
                    if (ioIndex >= 0)
                    {
                        vx_uint32 outIndex = ioIndex + binLoad->fixed.header.inputCount;
                        vx_uint8_ptr tensorLogical = VX_NULL;
                        vx_uint32 number = binLoad->outputPatch[ioIndex].number;
                        vx_reference ref = VX_NULL;
                        if (graph->outputCount && graph->outputs)
                        {
                            ref = graph->outputs[ioIndex];
                        }
                        else
                        {
                            ref = parameters[outIndex];
                        }

                        if (ref->type == VX_TYPE_TENSOR)
                        {
                            vx_int32 *dims = VX_NULL;
                            vx_uint32 dimCount = 0;
                            vx_uint32 outputPhyddr = 0;

                            tensor = (vx_tensor)ref; /* get output tensor*/
                            dims = TENSOR_ORIG_SIZES(tensor);
                            dimCount = TENSOR_ORIG_DIM_NUM(tensor);

                            memAddr = (vx_uint32 *) (SCStates + scPatchData->offset);
                            binLoad->outputPatch[ioIndex].offsets[number] = *memAddr - scPatchData->originalBaseAddress;
                            binLoad->outputPatch[ioIndex].references[number] = memAddr;
                            binLoad->outputPatch[ioIndex].number++;

                            for (j = 0; j < dimCount; j++)
                            {
                                vx_binary_input_output_info_s *ptrInOut = (vx_binary_input_output_info_s *)vxoBinaryGraph_GetInputOutputPtrByIndex(binLoad, binLoad->outputs, ioIndex);
                                if (dims[j] != (vx_int32)ptrInOut->dims[j])
                                    break;
                            }
                            if ((j == dimCount) && (kernel->signature.directionTable[outIndex] == VX_OUTPUT))
                            {
                                if (!tensor->tensorBuffer->memory.allocated)
                                {
                                    vxmONERROR(vxoTensor_AllocateMemory(tensor));
                                }
                                outputPhyddr = TENSOR_PHYSICAL_ADDR(tensor);
                                tensorLogical = TENSOR_LOGICAL_ADDR(tensor);
                                *memAddr = (*memAddr - scPatchData->originalBaseAddress) + outputPhyddr;
                            }
                            else
                            {
                                vxError("binary graph nn patch output failed, please check your output format\n");
                                vxmONERROR(VX_ERROR_INVALID_FORMAT);
                            }
                        }
                        else
                        {
                            vxError("%s[%d]: not support this type: %d as patch scaler output \n", __FUNCTION__, __LINE__, ref->type);
                            vxmONERROR(VX_FAILURE);
                        }

                        if ((binLoad->context->options.enableNNLayerDump) && (segment != VX_NULL))
                        {
                            vx_size tensorSize = 0;
                            vx_uint32 outputCount = segment->outputCount;
                            vxoTensor_GetTensorWholeSize(tensor, &tensorSize);
                            segment->outputlogical[outputCount] = (vx_uint8_ptr)tensorLogical;
                            segment->outputSize[outputCount] = (vx_uint32)tensorSize;
                            segment->outputCount++;
                        }
                    }
                    else
                    {
                        vxError("%s[%d]: error: patch scaler output, ioindex is  %d\n", __FUNCTION__, __LINE__, ioIndex);
                        vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
                    }
                }
                break;

                case VX_BINARY_SOURCE_MISC_DYNAMIC_INPUT:
                {
                    lcdtIndex = scPatchData->index;
                    memAddr = (vx_uint32 *)(SCStates + scPatchData->offset);
                    *memAddr = binLoad->LCD.physical + binLoad->LCDT[lcdtIndex].offset + (*memAddr - scPatchData->originalBaseAddress);
                }
                break;

                case VX_BINARY_SOURCE_MISC_DYNAMIC_OUTPUT:
                {
                    lcdtIndex = scPatchData->index;
                    memAddr = (vx_uint32 *)(SCStates + scPatchData->offset);
                    *memAddr = binLoad->LCD.physical + binLoad->LCDT[lcdtIndex].offset + (*memAddr - scPatchData->originalBaseAddress);

                    if ((binLoad->context->options.enableNNLayerDump) && (segment != VX_NULL))
                    {
                        vx_uint32 outputCount = segment->outputCount;
                        vx_uint32 outputSize = binLoad->LCDT[scPatchData->index].size;
                        vx_uint8_ptr outputlogical = binLoad->LCD.logical + binLoad->LCDT[scPatchData->index].offset; /* base address to dump */

                        segment->outputlogical[outputCount] = outputlogical;
                        segment->outputSize[outputCount] = outputSize;
                        segment->outputCount++;
                    }
                }
                break;

                case VX_BINARY_SOURCE_MEMORY_POOL:
                {
                    memAddr = (vx_uint32 *)(SCStates + scPatchData->offset);
                    *memAddr = node->binLoadMem->pool.physical + (*memAddr - scPatchData->originalBaseAddress);
                    if (binLoad->context->options.enableNNLayerDump)
                    {
                        vxError("%s[%d]: cann't support data type %d, please set ENV NN_LAYER_DUMP=1 and re-generate binary graph\n",
                                __FUNCTION__, __LINE__, scPatchData->sourceType);
                    }
                }
                break;

                case VX_BINARY_SOURCE_AXI_SRAM:
                {
                    memAddr = (vx_uint32 *)(SCStates + scPatchData->offset);
                    *memAddr =  (*memAddr - scPatchData->originalBaseAddress) + binLoad->context->axiSRAM[graph->deviceID].physical;
                    if (binLoad->context->options.enableNNLayerDump)
                    {
                        vxError("%s[%d]: cann't support data type %d, please set ENV NN_LAYER_DUMP=1 and re-generate binary graph\n",
                                __FUNCTION__, __LINE__, scPatchData->sourceType);
                    }
                }
                break;

                case VX_BINARY_SOURCE_VIP_SRAM:
                {
                    vx_context context = binLoad->context;
                    memAddr = (vx_uint32 *)(SCStates + scPatchData->offset);
                    *memAddr =  (*memAddr - scPatchData->originalBaseAddress) + binLoad->context->vipSRAM.physical - VX_VIP_SRAM_IMAGE_STREAM_SIZE;
                    if (binLoad->context->options.enableNNLayerDump)
                    {
                        vxError("%s[%d]: cann't support data type %d, please set ENV NN_LAYER_DUMP=1 and re-generate binary graph\n",
                                __FUNCTION__, __LINE__, scPatchData->sourceType);
                    }
                }
                break;

                default:
                    vxError("%s[%d]: scaler not implement this sourceType: %d\n", __FUNCTION__, __LINE__, scPatchData->sourceType);
                break;
            }
        }
        else
        {
            vxError("%s[%d]:  note support this scaler patch type: %d\n", __FUNCTION__, __LINE__, scPatchData->type);
            vxmONERROR(VX_FAILURE);
        }
    }

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}


VX_PRIVATE_API vx_status vxoBinaryGraph_patchSW(
    vx_node node,
    vx_binary_sw_operation_info_s *swOpData,
    vx_binary_operation_info_s *operation,
    vx_binary_loader_s *binLoad,
    vx_binary_segment_s *segment
    )
{
    vx_status status = VX_SUCCESS;

    switch (swOpData->swOperationType)
    {
        case VX_BINARY_SW_OPERATION_RPN:
        {
            vxmONERROR(vxoBinaryGraph_patchSWRPN(node, swOpData, operation, binLoad, segment));
        }
        break;

        default:
        {
            vxError("%s[%d]: not support this sw operation : %d\n", __FUNCTION__, __LINE__, swOpData->swOperationType);
        }
        break;
    }

OnError:
    return status;
}

VX_PRIVATE_API vx_status binaryGenerateStatesBuffer(
    vx_node node,
    vx_binary_loader_s *binLoad
    )
{
    vx_binary_operation_info_s *operation = VX_NULL;
    vx_binary_nn_operation_info_s *nnOPData = VX_NULL;
    vx_binary_tp_operation_info_s *tpOPData = VX_NULL;
    vx_binary_sh_operation_info_s *shOPData = VX_NULL;
    vx_binary_sw_operation_info_s *swOpData = VX_NULL;
    vx_binary_segment_s *segment = VX_NULL;
    vx_uint8 *nntpCommands = VX_NULL;
    vx_uint8 *shCommands = VX_NULL;
    vx_uint8 *states = VX_NULL;
    vx_status status = VX_SUCCESS;
    vx_uint32 cmdbufSize = 0;
    vx_uint32 opStateSize = 0;
    vx_uint32 i = 0;
    vx_uint32 layerIndex = 0;
    vx_uint32 totalStatesSize = 0;
    vx_uint32 preLayerId = 0;
    vx_uint32 endStatesPos = 0;
    vx_uint32 preSegmentStatesPos = 0;
    vx_uint32 sumSegmentStatesSize = 0;
    vx_uint32 startOPIndex = 0;
    gcmHEADER_ARG("node=%p, binLoad=%p", node, binLoad);

    nntpCommands = (vx_uint8 *)node->binLoadMem->nntpCmdBuff.logical;
    shCommands = (vx_uint8 *)node->binLoadMem->shCmdBuff.logical;
    states = (vx_uint8 *)node->binLoadMem->statesBuff;

    for (i = 0; i < binLoad->nOperations; i++)
    {
        operation = &binLoad->operations[i];
        opStateSize = 0;

        if (binLoad->context->options.enableNNLayerDump)
        {
            segment = &binLoad->segments[layerIndex];
            vxmASSERT(layerIndex < binLoad->segmentsCount);
            layerIndex++;
        }

        switch (operation->operationType)
        {
            case VX_BINARY_OPERATION_TYPE_NN:
            {
                nnOPData = &binLoad->nnOpsData[operation->operationIndex];
                vxmONERROR(vxoBinaryGraph_patchNN(node, nnOPData, nntpCommands,
                                          states, operation, binLoad,
                                          &opStateSize, segment));
                gcmDUMP(gcvNULL, "#[info: NN command in the binary]");
                gcmDUMP_BUFFER(gcvNULL, gcvDUMP_BUFFER_MEMORY, node->binLoadMem->nntpCmdBuff.physical,
                                node->binLoadMem->nntpCmdBuff.logical,
                                gcmPTR2INT32(nntpCommands - node->binLoadMem->nntpCmdBuff.logical),
                                NNE_COMMAND_SIZE);
                states += opStateSize;
                nntpCommands += NNE_COMMAND_SIZE;
            }
            break;

            case VX_BINARY_OPERATION_TYPE_TP:
            {
                tpOPData = &binLoad->tpOpsData[operation->operationIndex];
                vxmONERROR(vxoBinaryGraph_patchTP(node, tpOPData, nntpCommands,
                                          states, operation, binLoad,
                                          &opStateSize, segment));
                gcmDUMP(gcvNULL, "#[info: TP command in the binary]");
                gcmDUMP_BUFFER(gcvNULL, gcvDUMP_BUFFER_MEMORY, node->binLoadMem->nntpCmdBuff.physical,
                                node->binLoadMem->nntpCmdBuff.logical,
                                gcmPTR2INT32(nntpCommands - node->binLoadMem->nntpCmdBuff.logical),
                                TP_COMMAND_SIZE);
                states += opStateSize;
                nntpCommands += TP_COMMAND_SIZE;
            }
            break;

            case VX_BINARY_OPERATION_TYPE_SH:
            {
                shOPData = &binLoad->shOpsData[operation->operationIndex];
                vxmONERROR(vxoBinaryGraph_patchSH(node, shOPData, shCommands,
                                          states, operation, binLoad,
                                          &cmdbufSize, &opStateSize, segment));
                gcmDUMP(gcvNULL, "#[info: SH instructions in the binary]");
                gcmDUMP_BUFFER(gcvNULL, gcvDUMP_BUFFER_MEMORY, node->binLoadMem->shCmdBuff.physical,
                                node->binLoadMem->shCmdBuff.logical,
                                gcmPTR2INT32(shCommands - node->binLoadMem->shCmdBuff.logical),
                                cmdbufSize);
                states += opStateSize;
                shCommands += gcmALIGN(cmdbufSize, SH_COMMAND_ALIGN_SIZE);
            }
            break;

            case VX_BINARY_OPERATION_TYPE_SC:
            {
                vxmONERROR(vxoBinaryGraph_patchSC(node, states, operation, binLoad, &opStateSize, segment));
                states += opStateSize;
            }
            break;

            case VX_BINARY_OPERATION_TYPE_INIT:
            {
                gcmDUMP(gcvNULL, "#[info: INIT command in the binary]");
                gcmDUMP_BUFFER(gcvNULL, gcvDUMP_BUFFER_MEMORY, binLoad->LCD.physical,
                            binLoad->LCD.logical, binLoad->LCDT[operation->stateLCDTIndex].offset,
                            gcmALIGN(binLoad->LCDT[operation->stateLCDTIndex].size, 64));
                /* ignore INIT operation */
            }
            break;

            case VX_BINARY_OPERATION_TYPE_SW:
            {
                if (!binLoad->context->options.enableNNLayerDump)
                {
                    /*1. gpu segment */
                    segment = &binLoad->segments[layerIndex];
                    segment->statesStartPos = preSegmentStatesPos;
                    segment->statesSize = sumSegmentStatesSize;
                    segment->layerId = preLayerId;
                    segment->isSWSegment = vx_false_e;
                    segment->startOperationIndex = startOPIndex;
                    segment->endOperationIndex = i + 1;

                    startOPIndex = i;
                    endStatesPos = segment->statesStartPos + segment->statesSize;
                    preSegmentStatesPos += sumSegmentStatesSize;
                    sumSegmentStatesSize = 0;
                    opStateSize = 0;
                    layerIndex++;

                    /*2. cpu segment */
                    swOpData = &binLoad->swOpsData[operation->operationIndex];
                    vxmASSERT(layerIndex < binLoad->segmentsCount);
                    segment = &binLoad->segments[layerIndex];
                    segment->isSWSegment = vx_true_e;
                    segment->layerId = operation->layerId;
                    segment->startOperationIndex = i;
                    segment->endOperationIndex = i;

                    layerIndex++;

                    vxmONERROR(vxoBinaryGraph_patchSW(node, swOpData, operation, binLoad, segment));
                }
            }
            break;

            case VX_BINARY_OPERATION_TYPE_END:
            {
                vx_binary_entry_s *lcdtDate = VX_NULL;
                lcdtDate = &binLoad->LCDT[operation->stateLCDTIndex];
                vxMemCopy((vx_ptr)states, (vx_const_ptr)((vx_uint8 *)binLoad->LCD.logical+ lcdtDate->offset),
                          (vx_size)lcdtDate->size);
                states += lcdtDate->size;
                gcmDUMP(gcvNULL, "#[info: END command in the binary]");
                gcmDUMP_BUFFER(gcvNULL, gcvDUMP_BUFFER_MEMORY, binLoad->LCD.physical,
                            binLoad->LCD.logical, binLoad->LCDT[operation->stateLCDTIndex].offset,
                            gcmALIGN(binLoad->LCDT[operation->stateLCDTIndex].size, 64));
                /* ignore END operation */
            }
            break;

            default:
            {
                vxError("%s[%d]: binary graph not implement operation type: %d\n", __FUNCTION__, __LINE__, operation->operationType);
            }
            break;
        }

        if (binLoad->context->options.enableNNLayerDump)
        {
            vx_uint32 statesStartPos = gcmPTR2INT32(states - (vx_uint8 *)node->binLoadMem->statesBuff) - opStateSize;
            segment->statesStartPos = statesStartPos;
            segment->statesSize = opStateSize;
            segment->layerId = operation->layerId;
        }
        else
        {
            totalStatesSize += opStateSize;
            sumSegmentStatesSize += opStateSize;
            preLayerId = operation->layerId;
        }
    }

    if ((endStatesPos != 0) && (totalStatesSize > endStatesPos))
    {
        /* for gpu segment with SW operation in binary */
        segment = &binLoad->segments[layerIndex];
        segment->statesStartPos = endStatesPos ;
        segment->statesSize = totalStatesSize - endStatesPos;
        segment->layerId = preLayerId;
        segment->isSWSegment = vx_false_e;
        segment->startOperationIndex = startOPIndex;
        segment->endOperationIndex = i + 1;

        vxmASSERT(sumSegmentStatesSize == (totalStatesSize - endStatesPos));
    }

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vx_status vxnneImportKernelOperation_Execute(
    vxnne_operation operation
    )
{
    vx_node node = operation->layer->node;
    vx_status status = VX_SUCCESS;

    gcmHEADER_ARG("operation=%p", operation);

    if (node->kernel->function)
    {
        vxmONERROR(node->kernel->function(node,
                                          node->paramTable,
                                          node->kernel->signature.paramCount));
    }

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status vxoBinaryGraph_WrapNBGKernel(
    vx_node node,
    vx_binary_loader_s *binLoad
    )
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i = 0;
    vx_uint32 operationIndex = 0;
    vxnne_nbg_layer NBGLayer = VX_NULL;

    gcmHEADER_ARG("node=%p, binLoad=%p", node, binLoad);
    if ((node == VX_NULL) || (binLoad == VX_NULL))
    {
        vxError("node or binLoad is NULL, in deinitialize memory\n");
        vxmONERROR(VX_ERROR_NOT_ALLOCATED);
    }

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    NBGLayer = (vxnne_nbg_layer)vxAllocateAndZeroMemory(gcmSIZEOF(vxnne_nbg_layer_s));
    if (NBGLayer == VX_NULL)
    {
        vxError("fail to allocate memory for import kernel layer\n");
        vxmONERROR(VX_ERROR_INVALID_VALUE);
    }

    vxnneLayer_Initialize(&NBGLayer->base,
                          "network_binary_graph",
                          node,
                          gcmCOUNTOF(NBGLayer->operations),
                          NBGLayer->operations,
                          VX_NULL);

    vxmONERROR(vxnneOperation_Initialize(&NBGLayer->NBGOperation.base,
                                         &NBGLayer->base,
                                         VXNNE_OPERATION_TARGET_SW,
                                         VXNNE_OPERATOR_NBG,
                                         vxnneImportKernelOperation_Execute,
                                         VX_NULL,
                                         1,
                                         0));

     for (i = 0; i < node->kernel->signature.paramCount; i++)
     {
         if (node->kernel->signature.directionTable[i] == VX_INPUT &&
             (node->kernel->signature.isStaticTable[i] == vx_false_e ||
              (node->kernel->signature.dataTypeTable[i] == VX_TYPE_TENSOR &&
               TENSOR_DATA_LIFETIME((vx_tensor)(node->paramTable[i])) == VX_TENSOR_LIFE_TIME_DYNAMIC)))
         {
             vxnneOperation_AddReference(&NBGLayer->NBGOperation.base,
                                         node->paramTable[i], VXNNE_OPERATION_REFENRENCE_INPUT);
         }
         else if (node->kernel->signature.directionTable[i] == VX_OUTPUT)
         {
             vxnneOperation_AddReference(&NBGLayer->NBGOperation.base,
                                        node->paramTable[i], VXNNE_OPERATION_REFENRENCE_OUTPUT);
         }
     }

     vxnneLayer_SetOperation(&NBGLayer->base,
                             &NBGLayer->NBGOperation.base,
                             operationIndex++);

     node->layer = &NBGLayer->base;
     gcmFOOTER_ARG("%d", status);

     return VX_SUCCESS;

OnError:
    vxError("fail to initial memory in generate states buffer\n");
    if (NBGLayer)
    {
        gcoOS_Free(gcvNULL, NBGLayer);
        NBGLayer = VX_NULL;
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status vxoBinaryGraph_GenerateStatesBuffer(
    vx_node node,
    vx_binary_loader_s *binLoad
    )
{
    vx_status status = VX_SUCCESS;
    vx_uint32 nntpCmdsSize = 0;
    vx_uint32 stateSize = 0;
    vx_uint32 shCmdsSize = 0;
    vx_graph graph = VX_NULL;
    vx_context context = VX_NULL;
    gcmHEADER_ARG("node=%p, binLoad=%p", node, binLoad);

    vxmASSERT(node != VX_NULL);
    vxmASSERT(node->binLoadMem != VX_NULL);
    if ((node == VX_NULL) || (binLoad == VX_NULL) || (node->binLoadMem == VX_NULL))
    {
        vxError("node/binary_loader/binLoadMem is NULL, in deinitialize memory\n");
        gcmFOOTER_ARG("%d", VX_ERROR_NOT_ALLOCATED);
        return VX_ERROR_NOT_ALLOCATED;
    }

    if (binLoad->binaryBuffer == VX_NULL)
    {
        vxError("%s[%d]: error: binary buffer is NULL\n", __FUNCTION__, __LINE__);
        gcmFOOTER_ARG("%d", VX_ERROR_NOT_ALLOCATED);
        return VX_ERROR_NOT_ALLOCATED;
    }

    graph = node->graph;
    context = node->base.context;
    /* check AXI-SRAM size */
    if (binLoad->fixed.axiSramTable.sramSize != context->axiSRAM[graph->deviceID].size)
    {
        vxError("%s[%d]: binary sramSize: 0x%x, context size: 0x%x\n",
            __FUNCTION__, __LINE__, binLoad->fixed.axiSramTable.sramSize,
            context->axiSRAM[graph->deviceID].size);

        if (binLoad->fixed.axiSramTable.sramSize > context->axiSRAM[graph->deviceID].size)
        {
            vxError("%s[%d]: binary sram more than context sram\n", __FUNCTION__, __LINE__);
            vxmONERROR(VX_ERROR_INVALID_VALUE);
        }
    }

    /* check core count */
    if (binLoad->fixed.header.version >= 0x00010003)
    {
        gctUINT32 gpuCount = 1;
        gcmONERROR(gcoVX_QueryCoreCount(graph->deviceID, &gpuCount));
        if ((binLoad->fixed.header.featureDB.core_count !=0 ) &&
            (binLoad->fixed.header.featureDB.core_count != (vx_uint8)gpuCount))
        {
            vxError("%s[%d]: binary core count: %d, context core count: %d\n",
                    __FUNCTION__, __LINE__, binLoad->fixed.header.featureDB.core_count, gpuCount);
            vxmONERROR(VX_ERROR_INVALID_VALUE);
        }
    }

    /* 1. Create memory pool*/
    if (binLoad->fixed.poolTable.alignedSize > 0)
    {
        if (gcmIS_ERROR(gcoVX_AllocateMemory((gctUINT32)binLoad->fixed.poolTable.alignedSize,
                            (gctPOINTER*)&node->binLoadMem->pool.logical,
                            &node->binLoadMem->pool.physical,
                            &node->binLoadMem->pool.nodePtr)))
        {
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }
    }

    /* 2. Create states/command buffer*/
    shCmdsSize = getSHCmdSize(binLoad);
    stateSize = getStateSize(binLoad);
    nntpCmdsSize = NNE_COMMAND_SIZE * binLoad->nNnOps + TP_COMMAND_SIZE * binLoad->nTpOps;

    if (stateSize > 0)
    {
        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof(vx_uint8) * stateSize,
                                        (gctPOINTER*)&node->binLoadMem->statesBuff)))
        {
            vxError("%s[%d]: fail to allocate memory for states buffer\n", __FUNCTION__, __LINE__);
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }
        gcoOS_MemFill((gctPOINTER)node->binLoadMem->statesBuff, 0, sizeof(vx_uint8) * stateSize);
    }
    if (nntpCmdsSize > 0)
    {
        if (gcmIS_ERROR(gcoVX_AllocateMemory(nntpCmdsSize,
                                        (gctPOINTER*)&node->binLoadMem->nntpCmdBuff.logical,
                                        &node->binLoadMem->nntpCmdBuff.physical,
                                        &node->binLoadMem->nntpCmdBuff.nodePtr)))
        {
            vxError("%s[%d]: fail to allocate memory for nn/tp command buffer\n", __FUNCTION__, __LINE__);
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }
        gcoOS_MemFill((gctPOINTER)node->binLoadMem->nntpCmdBuff.logical, 0, nntpCmdsSize);
    }
    if (shCmdsSize > 0)
    {
        if (gcmIS_ERROR(gcoVX_AllocateMemoryEx(&shCmdsSize, gcvSURF_ICACHE, gcvPOOL_DEFAULT, SH_COMMAND_ALIGN_SIZE,
                                        &node->binLoadMem->shCmdBuff.physical,
                                        (gctPOINTER*)&node->binLoadMem->shCmdBuff.logical,
                                        &node->binLoadMem->shCmdBuff.nodePtr)))
        {
            vxError("%s[%d]: fail to allocate memory for sh command buffer\n", __FUNCTION__, __LINE__);
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }
        gcoOS_MemFill((gctPOINTER)node->binLoadMem->shCmdBuff.logical, 0, shCmdsSize);
    }

    /* 3. generate states buffer */
    vxmONERROR(binaryGenerateStatesBuffer(node, binLoad));
    node->binLoadMem->statesSize = stateSize;

    /* 4. free binaryBuffer which allocate on system.
          This buffer will be used in vxoImportKernelFromFile() when nn layer dump mode or
          stateSize > (gcmALIGN_BASE(VX_GRAPH_COMMAND_BUFFER_SIZE, 64) - 0x800)*/
    if ((0 == binLoad->context->options.enableNNLayerDump) && (binLoad->binaryBuffer != VX_NULL) &&
        (stateSize <= (gcmALIGN_BASE(VX_GRAPH_COMMAND_BUFFER_SIZE, 64) - 0x800)))
    {
        gcoOS_Free(gcvNULL, (gctPOINTER)binLoad->binaryBuffer);
        binLoad->binaryBuffer = VX_NULL;
    }
    binLoad->status = VX_BINARY_LOAD_STATUS_INIT;

    gcmFOOTER_ARG("%d", status);
    return status;
OnError:
    vxError("fail to initial memory in generate states buffer\n");
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status vxoBinaryGraph_ReleaseStatesBuffer(
    vx_node node
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("node=%p", node);

    vxmASSERT(node != VX_NULL);
    if (node == VX_NULL)
    {
        vxError("node is NULL, in deinitialize memory\n");
        gcmFOOTER_ARG("%d", VX_ERROR_NOT_ALLOCATED);
        return VX_ERROR_NOT_ALLOCATED;
    }

    if(node->binLoadMem == VX_NULL)
    {
        gcmFOOTER_ARG("%d", VX_SUCCESS);
        return VX_SUCCESS;
    }

    if (node->binLoadMem->statesBuff != VX_NULL)
    {
        gcmONERROR(gcoOS_Free(gcvNULL, (gctPOINTER)node->binLoadMem->statesBuff));
        node->binLoadMem->statesBuff = VX_NULL;
    }

    if (node->binLoadMem->nntpCmdBuff.nodePtr != VX_NULL)
    {
        gcmONERROR(gcoVX_FreeMemory(node->binLoadMem->nntpCmdBuff.nodePtr));
        node->binLoadMem->nntpCmdBuff.nodePtr = VX_NULL;
    }

    if (node->binLoadMem->shCmdBuff.nodePtr != VX_NULL)
    {
        gcmONERROR(gcoVX_FreeMemoryEx(node->binLoadMem->shCmdBuff.nodePtr, gcvSURF_ICACHE));
        node->binLoadMem->shCmdBuff.nodePtr = VX_NULL;
    }

    if (node->binLoadMem->pool.nodePtr!= VX_NULL)
    {
        gcmONERROR(gcoVX_FreeMemory(node->binLoadMem->pool.nodePtr));
        node->binLoadMem->pool.nodePtr= VX_NULL;
    }

    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

OnError:
    gcmFOOTER_NO();
    return (gcmIS_ERROR(status) ? VX_ERROR_NO_MEMORY : VX_SUCCESS);
}

VX_PRIVATE_API vx_status vxoBinaryGraph_InitBinaryLoad(
    vx_context context,
    vx_binary_loader_s **binaryLoad
    )
{
    vx_status status = VX_SUCCESS;
    vx_binary_loader_s *binLoad = *binaryLoad;
    vx_uint32 i = 0;

    gcmHEADER_ARG("context=%p, binLoad=%p", context, binLoad);

    /* Preprocess graph input/output table data. The table entries record where to modify the addresses. */
   if (binLoad->fixed.header.inputCount > 0)
   {
       binLoad->inputPatch = (vx_binary_io_patch_info_s*)vxAllocateAndZeroMemory(binLoad->nInputs * sizeof(vx_binary_io_patch_info_s));
   }

   if (binLoad->fixed.header.outputCount > 0)
   {
       binLoad->outputPatch = (vx_binary_io_patch_info_s*)vxAllocateAndZeroMemory(binLoad->nOutputs * sizeof(vx_binary_io_patch_info_s));
   }

   /* find out network inputs/outputs count */
   for (i = 0; i < binLoad->nOperations; i++)
   {
       vx_uint32 j = 0;
       vx_int32 ioIndex = -1;
       vx_binary_patch_info_s *patchData = VX_NULL;
       vx_binary_operation_info_s *operation = &binLoad->operations[i];

       for (j = 0; j < operation->counterOfPatches; j++)
       {
           patchData = (vx_binary_patch_info_s*)vxoBinaryGraph_GetPatchPtrByIndex(binLoad, binLoad->patchData,
                                                                                  operation->indexOfFirstPatch + j);

           ioIndex = patchData->index;
           if (ioIndex >= 0)
           {
               if (VX_BINARY_SOURCE_INPUT == patchData->sourceType)
               {
                   binLoad->inputPatch[ioIndex].count++;
               }
               else if (VX_BINARY_SOURCE_OUTPUT == patchData->sourceType)
               {
                   binLoad->outputPatch[ioIndex].count++;
               }
           }
       }
   }

   /* allocte memory for inputs/outputs */
   for (i = 0; i < binLoad->fixed.header.inputCount; i++)
   {
       if (binLoad->inputPatch[i].count > 0)
       {
           binLoad->inputPatch[i].references = (vx_uint32**)vxAllocateAndZeroMemory(binLoad->inputPatch[i].count * sizeof(vx_uint32*));
           binLoad->inputPatch[i].offsets = (vx_uint32*)vxAllocateAndZeroMemory(binLoad->inputPatch[i].count * sizeof(vx_uint32));
       }
   }

   for (i = 0; i < binLoad->fixed.header.outputCount; i++)
   {
       if (binLoad->outputPatch[i].count > 0)
       {
           binLoad->outputPatch[i].references = (vx_uint32**)vxAllocateAndZeroMemory(binLoad->outputPatch[i].count * sizeof(vx_uint32*));
           binLoad->outputPatch[i].offsets = (vx_uint32*)vxAllocateAndZeroMemory(binLoad->outputPatch[i].count * sizeof(vx_uint32));
       }
   }

   /* allocate memory for NN layer dump or SW operation */
   binLoad->segmentsCount = vxoBinaryGraph_GetLayerCount(binLoad);
   if (binLoad->segmentsCount > 0)
   {
       binLoad->segments = (vx_binary_segment_s*)vxAllocateAndZeroMemory(binLoad->segmentsCount * sizeof(vx_binary_segment_s));
   }

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_LoadFromFile(
    vx_context context,
    vx_binary_loader_s **binaryLoad,
    gctCONST_STRING fileName
    )
{
    vx_binary_reader_s reader;
    vx_binary_loader_s *binLoad = VX_NULL;
    vx_status status = VX_SUCCESS;
    vx_uint32 readSize = 0;
    vx_uint32 from = VX_BINARY_DYN_FROM_FILE;
#if LOAD_WHOLE_BINARY_TO_BUFFER
    gctUINT fileSize = 0;
#endif
    gcmHEADER_ARG("context=%p, binLoad=%p, fileName=%s", context, binaryLoad, fileName);

    vxmASSERT(context != VX_NULL);
    vxmASSERT(fileName != NULL);
    if ((context == VX_NULL) || (fileName == VX_NULL))
    {
        vxError("load binary network context/fileName/network is NULL\n");
        gcmFOOTER_ARG("%d", VX_ERROR_NOT_ALLOCATED);
        return VX_ERROR_NOT_ALLOCATED;
    }

    binLoad = (vx_binary_loader_s*)vxAllocateAndZeroMemory(sizeof(vx_binary_loader_s));
    if (VX_NULL == binLoad)
    {
        vxError("%s[%d]: fail to allocate memory for binary load\n", __FUNCTION__, __LINE__);
        *binaryLoad = VX_NULL;
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }

    *binaryLoad = binLoad;
    binLoad->context = context;

    if (gcmIS_ERROR(gcoOS_Open(gcvNULL, fileName, gcvFILE_READ, &binLoad->dFile)))
    {
        vxError("open network binary failed");
        vxmONERROR(VX_FAILURE);
    }

#if LOAD_WHOLE_BINARY_TO_BUFFER
    fileSize = getFileSize(binLoad->dFile);
    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, fileSize, &binLoad->binaryBuffer)))
    {
        vxError("%s[%d]: fail to allocate memory for binary buffer\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }
    readSize = (vx_uint32)loadBinaryWhole(binLoad);
    from = VX_BINARY_DYN_FROM_WHOLE_MEMORY;
#else
    readSize = (vx_uint32)loadBinaryEntry(binLoad);
    from =  VX_BINARY_DYN_FROM_FILE;
#endif
    if (readSize <= 0)
    {
        vxError("%s[%d]: fail to load Binary File\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_INVALID_VALUE);
    }

    openReader(&reader, binLoad->binaryBuffer, readSize);
    reader.binLoad = binLoad;

    vxmONERROR(readBinFixed(&reader, binLoad));
    vxmONERROR(readBinDynamic(&reader, binLoad, from, VX_NULL));

    closeReader(&reader);

    vxmONERROR(vxoBinaryGraph_InitBinaryLoad(context, &binLoad));

    if (binLoad->dFile != VX_NULL)
    {
        gcoOS_Close(gcvNULL, binLoad->dFile);
        binLoad->dFile = VX_NULL;
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;

OnError:
    if (binLoad != NULL)
    {
        vxoBinaryGraph_ReleaseNBG(binLoad);
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_LoadFromPointer(
    vx_context context,
    vx_binary_loader_s **binaryLoad,
    const char *pointer
    )
{
    vx_binary_loader_s *binLoad = VX_NULL;
    vx_status status = VX_SUCCESS;
    vx_binary_reader_s reader;
    char *binHandle = (char*)pointer;
    vx_binary_entry_s lcdInfo;

    gcmHEADER_ARG("context=%p, binLoad=%p, pointer=%s", context, binaryLoad, pointer);

    binLoad = (vx_binary_loader_s*)vxAllocateAndZeroMemory(sizeof(vx_binary_loader_s));
    if (VX_NULL == binLoad)
    {
        vxError("%s[%d]: fail to allocate memory for binary load\n", __FUNCTION__, __LINE__);
        *binaryLoad = VX_NULL;
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }
    *binaryLoad = binLoad;
    binLoad->context = context;

    status = getLCDEntryInfo(binHandle, &lcdInfo);
    if (status != VX_SUCCESS)
    {
        vxError("fail to get lcd entry info\n");
        vxmONERROR(VX_FAILURE);
    }
    binLoad->binaryBuffer = vxAllocateAndZeroMemory((vx_size)lcdInfo.offset);
    if (VX_NULL == binLoad->binaryBuffer)
    {
        vxError("%s[%d]: fail to allocate memory for binary buffer\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_FAILURE);
    }

    vxMemCopy((vx_ptr)binLoad->binaryBuffer, binHandle, lcdInfo.offset);

    openReader(&reader, binLoad->binaryBuffer, lcdInfo.offset);
    reader.binLoad = binLoad;

    vxmONERROR(readBinFixed(&reader, binLoad));

    if (binLoad->fixed.LCD.size != lcdInfo.size)
    {
        vxError("%s[%d]: fixed lcd size 0x%x, 0x%x\n", __FUNCTION__, __LINE__, binLoad->fixed.LCD.size, lcdInfo.size);
        vxmONERROR(VX_FAILURE);
    }

    vxmONERROR(readBinDynamic(&reader, binLoad, VX_BINARY_DYN_FROM_HANDLE,
                              ((vx_uint8_ptr)binHandle + lcdInfo.offset)));

    closeReader(&reader);

    vxmONERROR(vxoBinaryGraph_InitBinaryLoad(context, &binLoad));

    binLoad->dFile = VX_NULL;
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;

OnError:
    vxError("fail to load binary from pointer to create graph\n");

    if (binLoad != NULL)
    {
        vxoBinaryGraph_ReleaseNBG(binLoad);
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status vxoBinaryGraph_LoadNBG(
    vx_context context,
    vx_binary_loader_s **binaryLoad,
    const vx_char *type,
    const vx_char *url
    )
{
    vx_status status = VX_SUCCESS;
    gcmHEADER_ARG("context=%p, binaryLoad=%p, type=%p, url=%p", context, binaryLoad, type, url);

    if ((context == VX_NULL) || (binaryLoad == VX_NULL) || (type == VX_NULL) || (url == VX_NULL))
    {
        vxError("load binary network context/binaryLoad/type/url is NULL\n");
        vxmONERROR(VX_ERROR_NOT_ALLOCATED);
    }

    if (gcoOS_StrCmp(type, VX_VIVANTE_IMPORT_KERNEL_FROM_FILE) == gcvSTATUS_OK)
    {
        vxmONERROR(vxoBinaryGraph_LoadFromFile(context, binaryLoad, url));
    }
    else if (gcoOS_StrCmp(type, VX_VIVANTE_IMPORT_KERNEL_FROM_POINTER) == gcvSTATUS_OK)
    {
        vxmONERROR(vxoBinaryGraph_LoadFromPointer(context, binaryLoad, url));
    }
    else
    {
        vxError("not support this type load NBG to create graph\n");
    }

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status vxoBinaryGraph_ReleaseNBG(
    vx_binary_loader_s *binLoad
    )
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i = 0;
    gcmHEADER_ARG("binLoad=%p", binLoad);

    vxmASSERT(binLoad != VX_NULL);
    if (binLoad == VX_NULL)
    {
        vxError("network is NULL, in release binary\n");
        gcmFOOTER_ARG("%d", VX_ERROR_NOT_ALLOCATED);
        return VX_ERROR_NOT_ALLOCATED;
    }

    /* free SW operation segment object */
    if (binLoad != VX_NULL)
    {
        for (i= 0 ; i < binLoad->segmentsCount; i++)
        {
            vx_binary_segment_s *segment = &binLoad->segments[i];
            if ((segment != VX_NULL) && (segment->isSWSegment == vx_true_e) && (segment->base != VX_NULL))
            {
                switch (segment->base->segmentType)
                {
                    case VX_BINARY_SW_OPERATION_RPN:
                    {
                        vx_binary_nn_layer_RPN_s *rpnLayer = (vx_binary_nn_layer_RPN_s*)segment->base;
                        gcoOS_Free(gcvNULL, (gctPOINTER)rpnLayer);
                    }
                    break;

                    default:
                    {
                        vxError("%s[%d]: not release this sw operation : %d\n",
                            __FUNCTION__, __LINE__, segment->base->segmentType);
                    }
                    break;
                }
            }
        }
    }

    for (i = 0; i < binLoad->fixed.header.inputCount; i++)
    {
         if ((binLoad->inputPatch != VX_NULL) && (binLoad->inputPatch[i].references != VX_NULL))
         {
             vxFree((vx_ptr)binLoad->inputPatch[i].references);
             binLoad->inputPatch[i].references = VX_NULL;
             vxFree((vx_ptr)binLoad->inputPatch[i].offsets);
             binLoad->inputPatch[i].offsets = VX_NULL;
         }
    }

    if (binLoad->inputPatch != VX_NULL)
    {
        vxFree((vx_ptr)binLoad->inputPatch);
        binLoad->inputPatch = VX_NULL;
    }

    for (i = 0; i < binLoad->fixed.header.outputCount; i++)
    {
         if ((binLoad->outputPatch != VX_NULL) && (binLoad->outputPatch[i].references != VX_NULL))
         {
             vxFree((vx_ptr)binLoad->outputPatch[i].references);
             binLoad->outputPatch[i].references = VX_NULL;
             vxFree((vx_ptr)binLoad->outputPatch[i].offsets);
             binLoad->outputPatch[i].offsets = VX_NULL;
         }
    }

    if (binLoad->outputPatch != VX_NULL)
    {
        vxFree((vx_ptr)binLoad->outputPatch);
        binLoad->outputPatch = VX_NULL;
    }

    if ((binLoad->segmentsCount > 0) && (binLoad->segments != VX_NULL))
    {
        vxFree((vx_ptr)binLoad->segments);
        binLoad->segments = VX_NULL;
    }

    if (binLoad->binaryBuffer != VX_NULL)
    {
        gcmOS_SAFE_FREE(gcvNULL,binLoad->binaryBuffer);
    }

    if (binLoad->LCD.nodePtr!= VX_NULL)
    {
        gcmVERIFY_OK(gcoVX_FreeMemory(binLoad->LCD.nodePtr));
        binLoad->LCD.nodePtr = VX_NULL;
    }

    if (binLoad->dFile != VX_NULL)
    {
        gcoOS_Close(gcvNULL, binLoad->dFile);
        binLoad->dFile = VX_NULL;
    }

    if (binLoad != NULL)
    {
        gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER*)binLoad));
        binLoad = VX_NULL;
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vx_uint32 vxoBinaryGraph_GetReferencePhyAddress(
    vx_reference ref
    )
{
    vx_uint32 physicalAddress = 0xFFFFFFFF;

    if (ref->type == VX_TYPE_TENSOR)
    {
        vx_tensor tensor = (vx_tensor)ref;
        physicalAddress = TENSOR_PHYSICAL_ADDR(tensor);
    }
    else if (ref->type == VX_TYPE_IMAGE)
    {
        vx_image image = (vx_image)ref;
       physicalAddress = image->memory.physicals[0];/*Y channel*/
    }
    else if (ref->type == VX_TYPE_ARRAY)
    {
        vx_array array = (vx_array)ref;
        physicalAddress = array->memory.physicals[0];
    }
    else if (ref->type == VX_TYPE_SCALAR)
    {
        vx_scalar scalar = (vx_scalar)ref;
        physicalAddress = scalar->physical;
    }
    else
    {
        vxError("%s[%d]: can't support this data type: %d \n",
            __FUNCTION__, __LINE__, ref->type);
    }

    return physicalAddress;
}

VX_PRIVATE_API vx_int32 vxoBinaryGraph_GetLCDTIndexForTempTensor(
    vx_graph graph,
    vx_uint32 tempPhysical,
    vx_uint32 *size
    )
{
    vx_uint32 i;
    for (i = 0; i < graph->binarySave->numberOfTempTensorInfo; i++)
    {
        if (tempPhysical == graph->binarySave->tempTensorsPhysical[i].tensorPhysical)
        {
            *size = graph->binarySave->tempTensorsPhysical[i].size;
            return (vx_int32)graph->binarySave->tempTensorsPhysical[i].LCDTIndex;
        }
    }

    return -1;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_Write(
    vx_binary_save binarySave,
    vx_uint32 offset,
    vx_uint32 size,
    void *data
    )
{
    vx_status status = VX_SUCCESS;

    if ((size == 0) || (data == VX_NULL))
    {
        vxError("%s[%d]: data is null or size is 0\n", __FUNCTION__, __LINE__);
        return VX_FAILURE;
    }

    if (1 == binarySave->generateNBGToMemory)
    {   /* write NBG to memory */
        vx_uint8_ptr buffer = (vx_uint8_ptr)binarySave->NBGBuffer + offset;
        if ((offset + size) > binarySave->NBGInMemorySize)
        {
            vxError("%s[%d]:generate NBG in memory, out of buffer boundary\n", __FUNCTION__, __LINE__);
            vxmASSERT(0);
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }

        vxMemCopy((vx_ptr)buffer, data, size);
        if ((offset + size) > *(binarySave->NBGSize))
        {
            *(binarySave->NBGSize) = offset + size;
        }
    }
    else
    {   /* write NBG file */
        gcoOS_Seek(gcvNULL, binarySave->binarySaveFile, offset, gcvFILE_SEEK_SET);
        gcoOS_Write(gcvNULL, binarySave->binarySaveFile, size, data);
        gcoOS_Flush(gcvNULL, binarySave->binarySaveFile);
        if ((offset + size) > binarySave->NBGFileSize)
        {
            binarySave->NBGFileSize = offset + size;
        }
    }

OnError:
    return status;
}

VX_PRIVATE_API vx_uint32 vxoBinaryGraph_SavePatchEntry(
    vx_graph graph,
    void *patchEntry
    )
{
    vx_uint32 patchIndex;
    vx_status status = VX_SUCCESS;
    vx_binary_save binarySave = graph->binarySave;

    status = vxoBinaryGraph_Write(binarySave, binarySave->currPatchOffset,
                                 sizeof(vx_binary_patch_info_s), patchEntry);
    if (status != VX_SUCCESS)
    {
        vxError("%s[%d]:failed to write data\n", __FUNCTION__, __LINE__);
        return 0xFFFFFFFF;
    }

    graph->binarySave->currPatchOffset += sizeof(vx_binary_patch_info_s);
    patchIndex = graph->binarySave->currPatchIndex++;

    return patchIndex;
}

VX_PRIVATE_API vx_enum vxoBinaryGraph_ConvertToBinaryOperationTarget(
    vx_int32 target
    )
{
    vx_enum operationTarget = VX_BINARY_OPERATION_TYPE_NONE;

    switch (target)
    {
        case VXNNE_OPERATION_TARGET_NONE:
            operationTarget = VX_BINARY_OPERATION_TYPE_NONE;
            break;
        case VXNNE_OPERATION_TARGET_SH:
            operationTarget = VX_BINARY_OPERATION_TYPE_SH;
            break;
        case VXNNE_OPERATION_TARGET_NN:
            operationTarget = VX_BINARY_OPERATION_TYPE_NN;
            break;
        case VXNNE_OPERATION_TARGET_TP:
            operationTarget = VX_BINARY_OPERATION_TYPE_TP;
            break;
        case VXNNE_OPERATION_TARGET_SW:
            operationTarget = VX_BINARY_OPERATION_TYPE_SW;
            break;
        case VXNNE_OPERATION_TARGET_SC:
            operationTarget = VX_BINARY_OPERATION_TYPE_SC;
            break;
        default:
            operationTarget = VX_BINARY_OPERATION_TYPE_NONE;
            break;
    }
    return operationTarget;
}

VX_INTERNAL_API vx_enum vxoBinaryGraph_ConvertToOVXDataType(
    vx_enum dataType
    )
{
    vx_enum ret = VX_BINARY_BUFFER_TYPE_MAX;
    switch (dataType)
    {
        case VX_BINARY_BUFFER_TYPE_TENSOR:
            ret = VX_TYPE_TENSOR;
            break;
        case VX_BINARY_BUFFER_TYPE_IMAGE:
            ret = VX_TYPE_IMAGE;
            break;
        case VX_BINARY_BUFFER_TYPE_ARRAY:
            ret = VX_TYPE_ARRAY;
            break;
        case VX_BINARY_BUFFER_TYPE_SCALAR:
            ret = VX_TYPE_SCALAR;
            break;
        default:
            ret = VX_BINARY_BUFFER_TYPE_MAX;
            break;
    }

    return ret;
}

VX_PRIVATE_API vx_enum vxoBinaryGraph_ConvertToBinaryBufferFormat(
    vx_uint32 format
    )
{
    vx_enum ret = VX_BINARY_BUFFER_FORMAT_UINT8;
    switch (format)
    {
        case VX_TYPE_FLOAT16:
            ret = VX_BINARY_BUFFER_FORMAT_FP16;
            break;
        case VX_TYPE_FLOAT32:
        case VX_DF_IMAGE_U32:
        case VX_DF_IMAGE_S32:
        case VX_DF_IMAGE_F32:
            ret = VX_BINARY_BUFFER_FORMAT_FP32;
            break;
        case VX_TYPE_UINT8:
        case VX_DF_IMAGE_U8:
            ret = VX_BINARY_BUFFER_FORMAT_UINT8;
            break;
        case VX_TYPE_INT8:
            ret = VX_BINARY_BUFFER_FORMAT_INT8;
            break;
        case VX_TYPE_UINT16:
        case VX_DF_IMAGE_U16:
            ret = VX_BINARY_BUFFER_FORMAT_UINT16;
            break;
        case VX_TYPE_INT16:
        case VX_DF_IMAGE_S16:
            ret = VX_BINARY_BUFFER_FORMAT_INT16;
            break;
        default:
            ret = VX_BINARY_BUFFER_FORMAT_UINT8;
            break;
    }

    return ret;
}

VX_PRIVATE_API vx_enum vxoBinaryGraph_ConvertToBinaryDataFormat(
    vx_uint32 format
    )
{
    vx_enum ret = VX_BINARY_DATA_FORMAT_UINT8;
    switch (format)
    {
        case VX_TYPE_FLOAT16:
            ret = VX_BINARY_DATA_FORMAT_FP16;
            break;
        case VX_TYPE_FLOAT32:
            ret = VX_BINARY_DATA_FORMAT_FP32;
            break;
        case VX_DF_IMAGE_U32:
            ret = VX_BINARY_DATA_FORMAT_UINT32;
            break;
        case VX_DF_IMAGE_S32:
            ret = VX_BINARY_DATA_FORMAT_INT32;
            break;
        case VX_DF_IMAGE_F32:
            ret = VX_BINARY_DATA_FORMAT_FP32;
            break;
        case VX_TYPE_UINT8:
            ret = VX_BINARY_DATA_FORMAT_UINT8;
            break;
        case VX_DF_IMAGE_U8:
            ret = VX_BINARY_DATA_FORMAT_UINT8;
            break;
        case VX_TYPE_INT8:
            ret = VX_BINARY_DATA_FORMAT_INT8;
            break;
        case VX_TYPE_UINT16:
        case VX_DF_IMAGE_U16:
            ret = VX_BINARY_DATA_FORMAT_UINT16;
            break;
        case VX_TYPE_INT16:
        case VX_DF_IMAGE_S16:
            ret = VX_BINARY_DATA_FORMAT_INT16;
            break;
        case VX_TYPE_CHAR:
            ret = VX_BINARY_DATA_FORMAT_CHAR;
            break;
        case VX_TYPE_FLOAT64:
            ret = VX_BINARY_DATA_FORMAT_FP64;
            break;
        case VX_TYPE_INT64:
            ret = VX_BINARY_DATA_FORMAT_INT64;
            break;
        case VX_TYPE_UINT64:
            ret = VX_BINARY_DATA_FORMAT_UINT64;
            break;
        default:
            ret = VX_BINARY_DATA_FORMAT_UINT8;
            break;
    }

    return ret;
}

VX_PRIVATE_API vx_enum vxoBinaryGraph_GetMiscDynamicSourceType(
    vxnne_operation_s *operation,
    vx_uint32 physical
    )
{
    vx_reference opParam = VX_NULL;
    vx_uint32  opPhy = 0;
    vx_uint32  i = 0;
    gcmHEADER_ARG("operation=%p, physical=0x%x", operation, physical);

    /* search from input parameter*/
    for (i = 0; i < operation->inputsNum; i++)
    {
        opParam = operation->inputs[i];
        if (VX_TYPE_TENSOR == opParam->type)
        {
            vx_tensor opTensor = (vx_tensor)opParam;
            opPhy = TENSOR_PHYSICAL_ADDR(opTensor);
        }
        else if (VX_TYPE_ARRAY == opParam->type)
        {
            vx_array opArray = (vx_array)opParam;
            opPhy = opArray->memory.physicals[0];
        }
        else if (VX_TYPE_IMAGE == opParam->type)
        {
            vx_image opImage = (vx_image)opParam;
            opPhy = opImage->memory.physicals[0];
        }

        if (opPhy == physical)
        {
           gcmFOOTER_ARG("%d", VX_BINARY_SOURCE_MISC_DYNAMIC_INPUT);
           return VX_BINARY_SOURCE_MISC_DYNAMIC_INPUT;
        }
    }

    /* search from output parameter*/
    for (i = 0; i < operation->outputsNum; i++)
    {
        opParam = operation->outputs[i];
        if (VX_TYPE_TENSOR == opParam->type)
        {
            vx_tensor opTensor = (vx_tensor)opParam;
            opPhy = TENSOR_PHYSICAL_ADDR(opTensor);
        }
        else if (VX_TYPE_ARRAY == opParam->type)
        {
            vx_array opArray = (vx_array)opParam;
            opPhy = opArray->memory.physicals[0];
        }
        else if (VX_TYPE_IMAGE == opParam->type)
        {
            vx_image opImage = (vx_image)opParam;
            opPhy = opImage->memory.physicals[0];
        }

        if (opPhy == physical)
        {
            gcmFOOTER_ARG("%d", VX_BINARY_SOURCE_MISC_DYNAMIC_OUTPUT);
            return VX_BINARY_SOURCE_MISC_DYNAMIC_OUTPUT;
        }
    }
    gcmFOOTER_ARG("%d", VX_BINARY_SOURCE_MISC_DYNAMIC_GENERIC);
    return VX_BINARY_SOURCE_MISC_DYNAMIC_GENERIC;
}


VX_PRIVATE_API vx_int32 vxoBinaryGraph_GetIndexOfInputOutputEntry(
    vx_uint32 *inputOutputPhysicalEntry,
    vx_uint32 physical
    )
{
    vx_uint32 index = 0;

    vx_uint32 inputOutputPhysical = inputOutputPhysicalEntry[index];
    gcmHEADER_ARG("inputOutputPhysicalEntry=%p, physical=0x%x", inputOutputPhysicalEntry, physical);

    while (inputOutputPhysical != 0)
    {
        if (physical == inputOutputPhysical)
        {
            gcmFOOTER_ARG("%d", index);
            return index;
        }

        inputOutputPhysical = inputOutputPhysicalEntry[++index];
    }
    gcmFOOTER_ARG("%d", -1);
    return -1;
}

/* search pattern index from buffer
   return:  the number of the pattern in buffer
   offset:  all offsets in buffer be finded
*/
VX_INTERNAL_API vx_int32 vxoBinaryGraph_SearchPattern(
    gctUINT32_PTR buffer,
    gctUINT32 sizeInUint,
    gctUINT32 pattern,
    vx_uint32 *offset,
    vx_bool multiple
    )
{
    vx_uint32 index = 0;
    vx_uint32 cnt = 0;

    gcmHEADER_ARG("buffer=%p, sizeInUint=0x%x, pattern=0x%x", buffer, sizeInUint, pattern);

    if (VX_NULL == buffer)
    {
        vxError("%s[%d]: error, buffer is NULL\n", __FUNCTION__, __LINE__);
        return 0;
    }

    if (vx_false_e == multiple)
    {
        offset[0] = 0xFFFF;
        for (index = 0; index < sizeInUint; index++)
        {
            if (buffer[index] == pattern)
            {
                offset[0] = (vx_uint32)(index * gcmSIZEOF(gctUINT32));
                gcmFOOTER_ARG("%d", 1);
                return 1;
            }
        }
    }
    else
    {
        for (index = 0; index < sizeInUint; index++)
        {
            offset[cnt] = 0xFFFF;
            if (buffer[index] == pattern)
            {
                offset[cnt++] = (vx_uint32)(index * gcmSIZEOF(gctUINT32));
            }
        }
    }
    gcmFOOTER_ARG("%d", cnt);
    return cnt;
}

VX_PRIVATE_API vx_uint32 vxoBinaryGraph_SearchPatternRightShift6(
    gctUINT32_PTR buffer,
    gctUINT32  sizeInUint,
    gctUINT32  pattern
    )
{
    vx_uint32 index = 0;
    vx_uint32 offset = 0;
    gcmHEADER_ARG("buffer=%p, sizeInUint=0x%x, pattern=0x%x", buffer, sizeInUint, pattern);

    for (index = 0; index < sizeInUint; index++)
    {
        if ((buffer[index] >> 6) == (pattern >> 6))
        {
            offset = (vx_uint32)(index * gcmSIZEOF(gctUINT32));
            break;
        }
    }
    gcmFOOTER_ARG("0x%x", offset);
    return offset;
}

VX_PRIVATE_API vx_uint32 vxoBinaryGraph_SaveLoadingConfigData(
    vx_graph  graph,
    vx_uint8 *source,
    vx_uint32 bytes,
    vx_uint32 alignSize
    )
{
#define WRITE_NBG_CHECK_LCD()   \
    if (status != VX_SUCCESS)   \
    {   \
        loadingDataTableIndex = 0xFFFFFFFF; \
        vxmONERROR(VX_ERROR_NO_MEMORY); \
    }

    vx_uint8  zero[ONCE_FILL_SIZE] = {0};
    vx_uint32 i = 0;
    vx_status status = VX_SUCCESS;
    vx_uint32 loadingDataTableIndex;
    vx_uint32 alignedBytes = 0;
    vx_binary_loadingdata_table_info_s loadingDataTable;
    vx_binary_save_s *binarySave = graph->binarySave;
    gcmHEADER_ARG("graph=%p, source=%p, bytes=0x%x, alignSize=0x%x", graph, source, bytes, alignSize);
    if (bytes <= 0)
    {
        vxError("%s[%d]: save bytes is 0, fail to save binary\n", __FUNCTION__, __LINE__);
        vxmASSERT(0);
        gcmFOOTER_ARG("0x%x", 0xFFFFFFFF);
        return 0xFFFFFFFF;
    }

    if (alignSize > 0)
    {
        alignedBytes = gcmALIGN(bytes, alignSize);
    }
    else
    {
        alignedBytes = bytes;
    }

    /* save loading data */
    if (source != VX_NULL)
    {
        status = vxoBinaryGraph_Write(binarySave, binarySave->currLoadingDataOffset, bytes, source);
        WRITE_NBG_CHECK_LCD();

        if (alignedBytes > bytes)
        {
            vx_uint32 offset = binarySave->currLoadingDataOffset + bytes;
            /* fill zero to file for alignment data*/
            for (i = 0; i < (alignedBytes - bytes) / ONCE_FILL_SIZE; i++)
            {
                status = vxoBinaryGraph_Write(binarySave, offset, ONCE_FILL_SIZE, zero);
                WRITE_NBG_CHECK_LCD();
                offset += ONCE_FILL_SIZE;
            }
            if (((alignedBytes - bytes) % ONCE_FILL_SIZE) != 0)
            {
                status = vxoBinaryGraph_Write(binarySave, offset, (alignedBytes - bytes) % ONCE_FILL_SIZE, zero);
                WRITE_NBG_CHECK_LCD();
            }
        }
    }
    else
    {
        /* fill zero to file for tp/nn source is NULL*/
        if (alignedBytes <= ONCE_FILL_SIZE)
        {
            status = vxoBinaryGraph_Write(binarySave, binarySave->currLoadingDataOffset, alignedBytes, zero);
            WRITE_NBG_CHECK_LCD();
        }
        else
        {
            vx_uint32 offset = binarySave->currLoadingDataOffset;
            for (i = 0; i < alignedBytes / ONCE_FILL_SIZE; i++)
            {
                status = vxoBinaryGraph_Write(binarySave, offset, ONCE_FILL_SIZE, zero);
                WRITE_NBG_CHECK_LCD();
                offset += ONCE_FILL_SIZE;
            }
            if ((alignedBytes  % ONCE_FILL_SIZE) != 0)
            {
                status = vxoBinaryGraph_Write(binarySave, offset, alignedBytes % ONCE_FILL_SIZE, zero);
                WRITE_NBG_CHECK_LCD();
            }
        }
    }

    /* save loading table */
    loadingDataTable.loadingDataOffset = binarySave->currLoadingDataOffset - binarySave->loadingDataStartOffset;
    loadingDataTable.loadingDataSize = bytes;

    status = vxoBinaryGraph_Write(binarySave, binarySave->currLoadingDataTableOffset,
                                  sizeof(vx_binary_loadingdata_table_info_s), &loadingDataTable);
    WRITE_NBG_CHECK_LCD();

    binarySave->currLoadingDataTableOffset += sizeof(vx_binary_loadingdata_table_info_s);
    loadingDataTableIndex = binarySave->currLoadingDataTableIndex++;
    binarySave->currLoadingDataOffset += alignedBytes;
    binarySave->loadingDataTotalBytes += alignedBytes;

 OnError:

    status = vxoBinaryGraph_SaveErrorHandle(graph, status);
    if (status != VX_SUCCESS)
    {
        loadingDataTableIndex = 0xFFFFFFFF;
    }

    gcmFOOTER_ARG("%d", loadingDataTableIndex);
    return loadingDataTableIndex;
}

VX_PRIVATE_API vx_bool vxoBinaryGraph_ScaleIsNetworkInput(
    vx_graph graph,
    vx_node node,
    vxnne_operation operation
    )
{
    vx_node *nodeTable = graph->nodeTable;
    vx_bool isInput = vx_false_e;
    vx_uint32 i = 0;

    gcmHEADER_ARG("graph=%p, node=%p, operation=%p", graph, node, operation);

    /* bypass if user set input via API */
    if (graph->inputCount && graph->inputs)
    {
        gcmFOOTER_ARG("%d", isInput);
        return vx_true_e;
    }

    if (operation->target != VXNNE_OPERATION_TARGET_SH)
    {
        gcmFOOTER_ARG("%d", isInput);
        return vx_true_e;
    }

    if ((VXNNE_OPERATOR_TENSOR_MUL == operation->operatorType) ||
        (VXNNE_OPERATOR_TENSOR_DIV == operation->operatorType))
    {
        for (i = 0; i < graph->headNodeCount; i++)
        {
            if (node == nodeTable[graph->headNodeIndexTable[i]])
            {
                isInput = vx_true_e;
            }
        }
    }
    else
    {
        isInput = vx_true_e;
    }

    gcmFOOTER_ARG("%d", isInput);
    return isInput;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_ChangeAddressToOffset(
    vx_uint8 *buffer,
    vx_uint32 posOffset,
    vx_uint32 baseAddress,
    vx_uint32 shift6
    )
{
    vx_status status = VX_SUCCESS;
    vx_uint32 offset = 0;
    vx_uint32 data = 0;

    vx_uint32 *pos = (vx_uint32*)(buffer + posOffset);

    if (VX_BINARY_PATCH_TRANSFORMATION_RIGHT_SHIFT_6 == shift6)
    {
        data = *pos << 6;
    }
    else if (10 == shift6)
    {/* for NN patch command buffer */
        data = ((*pos) & 0x3f);
        *pos = data;
        return status;
    }
    else if (11 == shift6)
    {/* for NN KS */
        data = ((*pos) & 0xfc000000);
        *pos = data;
        return status;
    }
    else
    {
        data = *pos;
    }

    if (data < baseAddress)
    {
        vxError("%s[%d]: pos: 0x%x, base address: 0x%x\n", __FUNCTION__, __LINE__, data, baseAddress);
        return VX_FAILURE;
    }

    offset = data - baseAddress;

    *pos = offset;

    return status;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_SaveShaderPatchTable(
    vx_node node,
    vxnne_operation operation,
    vx_uint32 basePhysical,
    vx_uint8_ptr baseLogical,
    vx_uint32 wholeSize,
    vx_uint32 batchPhysical,
    gctUINT8_PTR stateBuffer,
    gctUINT stateSize,
    vx_enum allocType,
    vx_uint32 *patchCount,
    vx_uint32 *patchedParamPhy,
    vx_uint32 *patchParamCnt,
    vx_uint32 paramIndex,
    vx_type_e type
    )
{
    vx_int32 entryIndex = -1;
    vx_int32 LCDTindex = -1;
    vx_status status = VX_SUCCESS;
    vx_enum   sourceType = VX_BINARY_SOURCE_MEMORY_POOL;
    vx_uint32 multiNum = 0, i = 0;
    vx_uint32 offsetArray[VX_MAX_SHADER_PARAMETERS] = {0};
    vx_binary_patch_info_s patchInfo;
    vx_binary_save_s *binarySave = node->graph->binarySave;
    vx_context context = node->base.context;
    static vx_uint32 dynamicIndex = 0;
    static vx_uint32 isScaleShader = 0;
    gcmHEADER_ARG("node=%p, operation=%p, basePhysical=0x%x, baseLogical=0x%x, wholeSize=0x%x, batchPhysical=0x%x",
        node, operation, basePhysical, baseLogical, wholeSize, batchPhysical);

    gcoOS_ZeroMemory(&patchInfo, sizeof(vx_binary_patch_info_s));

    if (0 == paramIndex)
    {
        dynamicIndex = 0; /* initialize dynamicIndex for each shader operation */
        if (gcoOS_StrStr(node->kernel->name, "ScaletoTensor", VX_NULL))
        {
            isScaleShader = 1;
        }
        else
        {
            isScaleShader = 0;
        }
    }

    if (allocType == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR)
    {
        sourceType = VX_BINARY_SOURCE_MEMORY_POOL;
    }
    else if (allocType & VXNNE_MEM_POOL_TYPE_AXI_SRAM)
    {
        sourceType = VX_BINARY_SOURCE_AXI_SRAM;
    }
    else if (allocType & VXNNE_MEM_POOL_TYPE_VIP_SRAM)
    {
        sourceType = VX_BINARY_SOURCE_VIP_SRAM;
    }
    else if (allocType == VXNNE_MEM_POOL_TYPE_ORIG_DDR)
    {
        entryIndex = vxoBinaryGraph_GetIndexOfInputOutputEntry(binarySave->inputPhysicalEntry, basePhysical);
        if ((type == VX_TYPE_SCALAR) && (entryIndex != -1))
        {
            vx_bool isInput = vxoBinaryGraph_ScaleIsNetworkInput(node->graph, node, operation);
            if (isInput == vx_false_e)
            {
                entryIndex = -1;
            }
        }

        if (-1 == entryIndex)
        {
            entryIndex = vxoBinaryGraph_GetIndexOfInputOutputEntry(binarySave->outputPhysicalEntry, basePhysical);
            if (-1 == entryIndex)
            {
                vx_uint32 lcdSize = 0;

                LCDTindex = vxoBinaryGraph_GetLCDTIndexForTempTensor(node->graph, basePhysical, &lcdSize);
                if (-1 == LCDTindex)
                {
                    LCDTindex = (vx_int32)vxoBinaryGraph_SaveLoadingConfigData(node->graph, baseLogical, wholeSize, MEMORY_ALIGN_SIZE);
                    binarySave->tempTensorsPhysical[binarySave->numberOfTempTensorInfo].tensorPhysical = basePhysical;
                    binarySave->tempTensorsPhysical[binarySave->numberOfTempTensorInfo].LCDTIndex = LCDTindex;
                    binarySave->tempTensorsPhysical[binarySave->numberOfTempTensorInfo].size  = wholeSize;
                    binarySave->numberOfTempTensorInfo++;
                    if (LCDTindex > 0xFFFE)
                    {
                        vxmONERROR(VX_ERROR_NO_MEMORY);
                    }
                }
                else
                {
                    if (wholeSize > lcdSize)
                    {
                        vxError("error: fail to save shader's tensor to lcd, LCDTindex: %d, lcdSize: %d, tensorSize: %d\n",
                                LCDTindex, lcdSize, wholeSize);
                        vxmASSERT(0);
                    }
                }
                sourceType = vxoBinaryGraph_GetMiscDynamicSourceType(operation, basePhysical);
            }
            else
            {
                sourceType = VX_BINARY_SOURCE_OUTPUT;
                vxInfo("target sh parameter: %d, layer %s is output of the binary graph, address: 0x%x, entryIndex: %d\n",
                            paramIndex, node->layer->name, basePhysical, entryIndex);
            }
        }
        else
        {
            sourceType = VX_BINARY_SOURCE_INPUT;
            vxInfo("target sh parameter: %d, layer %s is input of the binary graph, address: 0x%x, entryIndex: %d\n",
                        paramIndex, node->layer->name, basePhysical, entryIndex);
        }
    }
    else
    {
        vxError("%s[%d]: not support this output type\n", __FUNCTION__, __LINE__);
        vxmASSERT(0);
        vxmONERROR(VX_ERROR_NOT_SUPPORTED);
    }

    patchInfo.type = VX_BINARY_PATCH_TYPE_STATE;
    if (allocType == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR)
    {
        patchInfo.originalBaseAddress = node->graph->memoryPool->physical;
    }
    else if (allocType & VXNNE_MEM_POOL_TYPE_AXI_SRAM)
    {
        patchInfo.originalBaseAddress = node->base.context->axiSRAM[node->graph->deviceID].physical;
    }
    else if (allocType & VXNNE_MEM_POOL_TYPE_VIP_SRAM)
    {
        patchInfo.originalBaseAddress = node->base.context->vipSRAM.physical - VX_VIP_SRAM_IMAGE_STREAM_SIZE;
    }
    else
    {
        patchInfo.originalBaseAddress = basePhysical;
    }
    patchInfo.sourceType = sourceType;
    switch (sourceType)
    {
        case VX_BINARY_SOURCE_MEMORY_POOL:
        case VX_BINARY_SOURCE_AXI_SRAM:
        case VX_BINARY_SOURCE_VIP_SRAM:
            patchInfo.index = -1;
            break;
        case VX_BINARY_SOURCE_INPUT:
        {
            if ((binarySave->supportDynInputShape) && isScaleShader)
            {
                patchInfo.name = VX_BINARY_PATCH_NAME_SCALE_INPUT;
            }
            patchInfo.index = (vx_uint32)entryIndex;
            break;
        }
        case VX_BINARY_SOURCE_OUTPUT:
        {
            patchInfo.index = (vx_uint32)entryIndex;
            break;
        }
        case VX_BINARY_SOURCE_MISC_DYNAMIC_GENERIC:
        {
            if ((binarySave->supportDynInputShape) && isScaleShader)
            {
                if (0 == dynamicIndex)
                {
                   patchInfo.name =  VX_BINARY_PATCH_NAME_SCALE_RATIO_X;
                }
                else if (1 == dynamicIndex)
                {
                    patchInfo.name =  VX_BINARY_PATCH_NAME_SCALE_RATIO_Y;
                }
                else if (2 == dynamicIndex)
                {
                    patchInfo.name =  VX_BINARY_PATCH_NAME_SCALE_OFFSET_X;
                }
                else if (3 == dynamicIndex)
                {
                    patchInfo.name =  VX_BINARY_PATCH_NAME_SCALE_OFFSET_Y;
                }
                dynamicIndex++;
            }
            patchInfo.index = (vx_uint32)LCDTindex;
            break;
        }
        case VX_BINARY_SOURCE_MISC_DYNAMIC_INPUT:
        case VX_BINARY_SOURCE_MISC_DYNAMIC_OUTPUT:
            patchInfo.index = (vx_uint32)LCDTindex;
            break;
        default:
            vxmASSERT(0);
            break;
    }
    patchInfo.transformation = VX_BINARY_PATCH_TRANSFORMATION_ORIGINAL;

    for (i = 0; i < *patchParamCnt; i++)
    {
        if (patchedParamPhy[i] == batchPhysical)
        {
            break;
        }
    }
    if (i < *patchParamCnt)
    {
        gcmFOOTER_ARG("%d", VX_SUCCESS);
        return VX_SUCCESS;/* the parameter has been patched*/
    }

    /* get offset address in states buffer*/
    if ((multiNum = vxoBinaryGraph_SearchPattern((gctUINT32 *)stateBuffer,
                                        stateSize/gcmSIZEOF(gctUINT32),
                                        batchPhysical, offsetArray, vx_true_e)) > 0)
    {
        if (multiNum > 0)
        {
            patchedParamPhy[*patchParamCnt] = batchPhysical;
            (*patchParamCnt)++;
            if (VX_BINARY_SOURCE_INPUT == patchInfo.sourceType)
            {
                binarySave->inputInPatchedBinOffset[binarySave->inputInPatchedNum] = binarySave->currPatchOffset;
                binarySave->inputInPatchedPhysical[binarySave->inputInPatchedNum] = basePhysical;
                binarySave->inputInPatchedIndex[binarySave->inputInPatchedNum] = patchInfo.index;
                binarySave->inputInPatchedNum++;
            }
        }
        for (i = 0; i < multiNum; i++)
        {
            vx_uint32 ret = 0;
            patchInfo.offset = offsetArray[i];
            #if (ENABLE_SAVE_OFFSET_IN_NBG == 1)
            vxmONERROR(vxoBinaryGraph_ChangeAddressToOffset(stateBuffer, offsetArray[i],
                                                             patchInfo.originalBaseAddress,
                                                             patchInfo.transformation));
            patchInfo.originalBaseAddress = 0;
            #endif
            ret = vxoBinaryGraph_SavePatchEntry(node->graph, (void *)&patchInfo);
            if (ret == 0xFFFFFFFF)
            {
                vxmONERROR(VX_ERROR_NO_MEMORY);
            }
            (*patchCount)++;
        }
    }
    else
    {   /* fix batch prelu fail issue */
        if ((multiNum = vxoBinaryGraph_SearchPattern((gctUINT32 *)stateBuffer,
                                            stateSize/gcmSIZEOF(gctUINT32),
                                            basePhysical, offsetArray, vx_true_e)) > 0)
        {
            if (multiNum > 0)
            {
                patchedParamPhy[*patchParamCnt] = basePhysical;
                (*patchParamCnt)++;
                if (VX_BINARY_SOURCE_INPUT == patchInfo.sourceType)
                {
                    binarySave->inputInPatchedBinOffset[binarySave->inputInPatchedNum] = binarySave->currPatchOffset;
                    binarySave->inputInPatchedPhysical[binarySave->inputInPatchedNum] = basePhysical;
                    binarySave->inputInPatchedIndex[binarySave->inputInPatchedNum] = patchInfo.index;
                    binarySave->inputInPatchedNum++;
                }
            }
            for (i = 0; i < multiNum; i++)
            {
                vx_uint32 ret = 0;
                patchInfo.offset = offsetArray[i];
                #if (ENABLE_SAVE_OFFSET_IN_NBG == 1)
                vxmONERROR(vxoBinaryGraph_ChangeAddressToOffset(stateBuffer, offsetArray[i],
                                                                 patchInfo.originalBaseAddress,
                                                                 patchInfo.transformation));
                patchInfo.originalBaseAddress = 0;
                #endif
                ret = vxoBinaryGraph_SavePatchEntry(node->graph, (void *)&patchInfo);
                if (ret == 0xFFFFFFFF)
                {
                    vxmONERROR(VX_ERROR_NO_MEMORY);
                }
                (*patchCount)++;
            }
        }
    }

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vx_uint32 vxoBinaryGraph_CalculateNNSliceCount(
    vx_context context,
    vxnne_operation_command operationCommand
)
{
    vx_uint32 commandCount = 1;
    vx_uint32 fitN = 0;
    vxnne_convolution_relu_pooling_operation convOperation = (vxnne_convolution_relu_pooling_operation)operationCommand->operation;
    vxnne_tiling_rect input = &operationCommand->inputTile;
    vx_uint32 inputDepth = WB_KERNEL_Z(convOperation->weights_biases);
    vx_uint32 inputWidth = input->width;
    vx_uint32 inputHeight = input->height;
    vx_uint32 xcount = 1;
    vx_uint32 ycount = 1;

    if (convOperation->do_zdp_opt &&
        context->options.do1xnAfterSwtiling)
    {
        calcFitZdp3N(context, input->width, input->height, &fitN, 1, operationCommand->operation->parameter.pool_size_x);
        if (fitN == 0)
        {
            vxError("%s[%d]: fixN is zero\n", __FUNCTION__, __LINE__);
            fitN = 1;
        }
        inputWidth = input->width * input->height / fitN;
        inputHeight = fitN;
    }
    else if (convOperation->do_1xN &&
             context->options.do1xnAfterSwtiling)
    {
        fitN = calcFit1xN(context, inputDepth, input->width, input->height);
        inputWidth = input->width * input->height;
        inputHeight = fitN;
    }

    xcount = gcmALIGN_NP2_SAFE(inputWidth, NN_IMAGE_XSIZE_MAX) / NN_IMAGE_XSIZE_MAX;
    ycount = gcmALIGN_NP2_SAFE(inputHeight, NN_IMAGE_YSIZE_MAX) / NN_IMAGE_YSIZE_MAX;
    commandCount = xcount * ycount;

    return commandCount;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_RefineInputOutput(
    vx_graph graph,
    vx_uint32 *inputCount,
    vx_uint32 *outputCount
    )
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i = 0;
    vx_uint32 j = 0;
    vx_uint32 inCount = *inputCount;
    vx_uint32 outCount = *outputCount;
    vx_reference ref = VX_NULL;
    vx_binary_save_s *binarySave = graph->binarySave;
    vx_uint32 useUserSet = 1;
    gcmHEADER_ARG("graph=%p", graph);

    if (graph->inputCount && graph->inputs)
    {
        /* sanity check, just for debugging info */
        for (i = 0; i < graph->inputCount; i++)
        {
            for (j = 0; j < inCount; j++)
            {
                if (graph->inputs[i] == binarySave->inputEntry[j])
                {
                    break;
                }
            }

            if (vxoBinaryGraph_GetReferencePhyAddress(graph->inputs[i]) == 0)
            {
                /* doesn't allocate mmeory for this reference object, let bypass */
                useUserSet = 0;
                break;
            }
        }

        if (1 == useUserSet)
        {
            *inputCount = graph->inputCount;

            for (i = 0; i < graph->inputCount; i++)
            {
                ref = graph->inputs[i];
                binarySave->inputEntry[i] = ref;
                binarySave->inputPhysicalEntry[i] = vxoBinaryGraph_GetReferencePhyAddress(ref);
            }

            /* clean other data*/
            for (i = graph->inputCount; i < VX_MAX_NN_INOUT_PARAM_COUNT; i++)
            {
                binarySave->inputPhysicalEntry[i] = 0;
            }
        }
        else
        {
            vxError("%s[%d]: bypass use user set inputs. useUserSet: %d\n", __FUNCTION__, __LINE__, useUserSet);
        }
    }

    useUserSet = 1;

    if (graph->outputCount && graph->outputs)
    {
        /* sanity check, just for debugging info */
        for (i = 0; i < graph->outputCount; i++)
        {
            for (j = 0; j < outCount; j++)
            {
                if (graph->outputs[i] == binarySave->outputEntry[j])
                {
                    break;
                }
            }

            if (vxoBinaryGraph_GetReferencePhyAddress(graph->outputs[i]) == 0)
            {
                /* doesn't allocate mmeory for this reference object, let bypass */
                useUserSet = 0;
                break;
            }
        }

        if (1 == useUserSet)
        {
            *outputCount = graph->outputCount;

            for (i = 0; i < graph->outputCount; i++)
            {
                ref = graph->outputs[i];
                binarySave->outputEntry[i] = ref;
                binarySave->outputPhysicalEntry[i] = vxoBinaryGraph_GetReferencePhyAddress(ref);
            }
            /* clean other data*/
            for (i = graph->outputCount; i < VX_MAX_NN_INOUT_PARAM_COUNT; i++)
            {
                binarySave->outputPhysicalEntry[i] = 0;
            }
        }
        else
        {
            vxError("%s[%d]: bypass use user set outputs. useUserSet: %d\n", __FUNCTION__, __LINE__, useUserSet);

        }
    }

    vxInfo(" input table address: ");
    for (i = 0; i < graph->inputCount; i++)
    {
        vxInfo("0x%x ", binarySave->inputPhysicalEntry[i]);
    }
    vxInfo("\n output table address: ");
    for (i = 0; i < graph->outputCount; i++)
    {
        vxInfo("0x%x ", binarySave->outputPhysicalEntry[i]);
    }
    vxInfo("\n");

    if ((0 == *outputCount) || (0 == *inputCount))
    {
        vxError("%s[%d]: failed input or output count is 0\n", __FUNCTION__, __LINE__);
        status = VX_FAILURE;
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_GetUidFromOutputTensor(
    vx_tensor tensor
    )
{
    vx_int32 uid = -1;
    char uidName[64] = {'\0'};

    if (strlen(((vx_reference)tensor)->name) > 4)
    {
        char *p;
        gcoOS_StrCopySafe(uidName, 64, &((vx_reference)tensor)->name[4]);
        p = uidName;
        while (p)
        {
            if (*p == '_')
            {
                *p = '\0';
                uid = atoi(uidName);
                break;
            }
            p++;
        }
    }

    return uid;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_Initialize(
    vx_graph graph,
    vx_char *fileName
    )
{
    vx_binary_save binarySave = VX_NULL;
    vx_status status = VX_SUCCESS;
    vx_char *networkBinaryPath = VX_NULL;
    vx_char NBNameFromGraph[256];
    vx_uint32 useGraphName = 0;
    vx_char *env = VX_NULL;

    gcmHEADER_ARG("graph=%p, fileName=%s", graph, fileName);

    if (graph == VX_NULL)
    {
        vxError("%s[%d]: graph is NULL\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_INVALID_VALUE);
    }

    if (gcoOS_StrCmp(graph->base.name, "\0") != gcvSTATUS_OK)
    {
        /* specify NBG name by vx_graph's reference */
        gctSTRING special = VX_NULL;

        gcoOS_StrCopySafe(NBNameFromGraph, BINARY_FILE_NAME_MAX_SIZE, graph->base.name);
        gcoOS_StrFindReverse(graph->base.name, '.', &special);
        if (special == VX_NULL)
        {
            gcoOS_StrCatSafe(NBNameFromGraph, BINARY_FILE_NAME_MAX_SIZE, ".nb");
        }
        useGraphName = 1;
        vxInfo("graph name : %s\n", NBNameFromGraph);
    }
    else
    {
        gcoOS_MemFill((gctPOINTER)NBNameFromGraph, 0, 256);
        useGraphName = 0;
    }

    /* get save binary file path */
    gcoOS_GetEnv(gcvNULL, "VIV_VX_SAVE_NETWORK_BINARY_PATH", &networkBinaryPath);
    /* sanity check for networkBinaryPath */
    if (networkBinaryPath != VX_NULL)
    {
        gctSTRING special = VX_NULL;
        gcoOS_StrFindReverse(networkBinaryPath, '.', &special);
        if (special == VX_NULL)
        {
            networkBinaryPath = VX_NULL;
            vxError("please export VIV_VX_SAVE_NETWORK_BINARY_PATH= has . suffix filename path\n");
            vxError("genereate network binary graph default filename\n");
        }
        else
        {
            vxInfo("env specify nbg name : %s\n", networkBinaryPath);
        }
    }

    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof(vx_binary_save_s), (gctPOINTER*)&graph->binarySave)))
    {
        vxError("fail to allocate memory for binarySave\n");
        vxmONERROR(VX_FAILURE);
    }
    else
    {
        gcoOS_MemFill((gctPOINTER)graph->binarySave, 0, sizeof(vx_binary_save_s));
    }
    binarySave = graph->binarySave;

    if (fileName == VX_NULL)
    {
        /* save binary graph for VIPlite*/
        char dumpFile[BINARY_FILE_NAME_MAX_SIZE];
        vx_uint32 offset = 0;

        if (1 == useGraphName)
        {
            gcoOS_StrCopySafe(dumpFile, BINARY_FILE_NAME_MAX_SIZE, NBNameFromGraph);
        }
        else if (networkBinaryPath != VX_NULL)
        {
            gcoOS_StrCopySafe(dumpFile, BINARY_FILE_NAME_MAX_SIZE, networkBinaryPath);
        }
        else
        {
            if (graph->graphID == 0)
            {
                gcmVERIFY_OK(gcoOS_PrintStrSafe(
                                                dumpFile,
                                                gcmSIZEOF(dumpFile),
                                                &offset,
                                                "network_binary_pid-%d_tid-%d.nb",
                                                gcmALL_TO_UINT32(gcoOS_GetCurrentProcessID()),
                                                gcmALL_TO_UINT32(gcoOS_GetCurrentThreadID())
                                                ));
            }
            else
            {
                gcmVERIFY_OK(gcoOS_PrintStrSafe(
                                                dumpFile,
                                                gcmSIZEOF(dumpFile),
                                                &offset,
                                                "network_binary_pid-%d_tid-%d_%d.nb",
                                                gcmALL_TO_UINT32(gcoOS_GetCurrentProcessID()),
                                                gcmALL_TO_UINT32(gcoOS_GetCurrentThreadID()),
                                                graph->graphID
                                                ));
            }
        }

        /* Open tga file for write. */
        gcmVERIFY_OK(gcoOS_Open(gcvNULL, dumpFile, gcvFILE_CREATE, (gctFILE*)(&binarySave->binarySaveFile)));
        gcoOS_StrCopySafe(binarySave->binaryFileName, BINARY_FILE_NAME_MAX_SIZE, dumpFile);
        vxInfo("Save binary graph for VIPLite. \n");
    }
    else
    {
        gcmVERIFY_OK(gcoOS_Open(gcvNULL, fileName, gcvFILE_CREATE, (gctFILE*)(&binarySave->binarySaveFile)));
        gcoOS_StrCopySafe(binarySave->binaryFileName, BINARY_FILE_NAME_MAX_SIZE, fileName);
        vxInfo("Save binary graph for unified drvier. \n");
    }

    gcoOS_GetEnv(gcvNULL, "VIV_VX_NBG_SUPPORT_DYNAMIC_INPUT_SHAPE", &env);
    if (env)
    {
        if (atoi(env))
        {
            binarySave->supportDynInputShape = vx_true_e;
        }
        else
        {
            binarySave->supportDynInputShape = vx_false_e;
        }
    }

    if (binarySave->binarySaveFile == NULL)
    {
        vxError("%s[%d]: binary Save File is NULL\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_FAILURE);
    }
    vxInfo("network binary graph file has been opened\n");

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}


/*  get the graph input and output
    inputNum: the number of this graph input
    outputNum: the number ot this graph output
*/
VX_PRIVATE_API vx_status vxoBinaryGraph_InputsOutputs(
    vx_graph graph,
    vx_reference *inputTable,
    vx_uint32 *inputNum,
    vx_reference *outputTable,
    vx_uint32 *outputNum
    )
{
    vx_uint32 nodeIndex = 0, inNum = 0, outNum = 0;
    vx_status status = VX_SUCCESS;
    gcmHEADER_ARG("graph=%p, inputTable=%p, inputNum=%p, outputTable=%p, outputNum=%p", graph, inputTable, inputNum, outputTable, outputNum);

    *inputNum = 0;
    *outputNum = 0;
    for (nodeIndex = 0; nodeIndex < graph->nodeCount; nodeIndex++)
    {
        vx_bool breakFlag = vx_false_e;
        /* graph->nodeTable[nodeIndex]: user create nodeTable order to collect graph's inputs*/
        vx_node node = graph->nodeTable[nodeIndex];
        vx_uint32 nodeIndex2 = 0, paramIndex2 = 0, paramIndex = 0;

        for (paramIndex = 0; paramIndex < node->kernel->signature.paramCount; paramIndex++)
        {
            vx_reference paramRef = node->paramTable[paramIndex];

            breakFlag = vx_false_e;
            if (paramRef == VX_NULL) continue;
            if (node->kernel->signature.directionTable[paramIndex] != VX_INPUT) continue;
            if (node->kernel->signature.isStaticTable[paramIndex] == vx_true_e) continue;
            if (paramRef->type == VX_TYPE_TENSOR)
            {
                if ((VX_TENSOR_LIFE_TIME_DYNAMIC != TENSOR_DATA_LIFETIME((vx_tensor)paramRef)) ||
                   (vxoMemory_GetType(&((vx_tensor)paramRef)->tensorBuffer->memory) == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR) ||
                   (node->kernel->signature.stateTable[paramIndex] == VX_PARAMETER_STATE_OPTIONAL))
                {
                    continue;
                }
            }
            else if (paramRef->type == VX_TYPE_SCALAR)
            {
                if (node->kernel->signature.stateTable[paramIndex] != VX_PARAMETER_STATE_OPTIONAL)
                {
                    /* scalar is graph input*/
                    inputTable[inNum++] = paramRef;
                }
                continue;
            }
            else if (paramRef->type == VX_TYPE_IMAGE)
            {
                vx_image image = (vx_image)paramRef;
                if ((vxoMemory_GetType(&image->memory) == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR) ||
                   (node->kernel->signature.stateTable[paramIndex] == VX_PARAMETER_STATE_OPTIONAL))
                {
                    continue;
                }
            }
            else if (paramRef->type == VX_TYPE_ARRAY)
            {
                vx_array array = (vx_array)paramRef;
                if ((vxoMemory_GetType(&array->memory) == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR) ||
                   (node->kernel->signature.stateTable[paramIndex] == VX_PARAMETER_STATE_OPTIONAL))
                {
                    continue;
                }
            }
            else
            {
               continue;
            }

            for (nodeIndex2 = vxoGraph_GetNextNodeIndex(graph, nodeIndex);
                nodeIndex2 != nodeIndex;
                nodeIndex2 = vxoGraph_GetNextNodeIndex(graph, nodeIndex2))
            {
                vx_node node2 = graph->nodeTable[nodeIndex2];

                for (paramIndex2 = 0; paramIndex2 < node2->kernel->signature.paramCount; paramIndex2++)
                {
                    vx_reference paramRef2 = node2->paramTable[paramIndex2];

                    if (paramRef2 == VX_NULL) continue;

                    if (node2->kernel->signature.directionTable[paramIndex2] == VX_INPUT) continue;

                    if (vxoReference_HasWriteDependency(paramRef, paramRef2))
                    {
                        breakFlag = vx_true_e;
                        break;
                    }
                }
                if (breakFlag) break;
            }

            if (nodeIndex2 == nodeIndex)
            {
                /* the input is graph's input */
                if ((paramRef->type == VX_TYPE_TENSOR) || (paramRef->type == VX_TYPE_IMAGE) ||
                    (paramRef->type == VX_TYPE_ARRAY))
                {
                    inputTable[inNum++] = paramRef;
                }
                else
                {
                    vxError("error: not support the type as graph input: %d\n", (vx_int32)paramRef->type);
                    vxmONERROR(VX_FAILURE);
                }
            }
        }
    }

    /* collect outputs of a network */
    for (nodeIndex = 0; nodeIndex < graph->nodeCount; nodeIndex++)
    {
        vx_bool breakFlag = vx_false_e;
        vx_node node = graph->nodeTable[nodeIndex];
        vx_uint32 nodeIndex2 = 0, paramIndex2 = 0, paramIndex = 0;

        for (paramIndex = 0; paramIndex < node->kernel->signature.paramCount; paramIndex++)
        {
            vx_reference paramRef = node->paramTable[paramIndex];

            breakFlag = vx_false_e;
            if (paramRef == VX_NULL) continue;
            if (node->kernel->signature.directionTable[paramIndex] != VX_OUTPUT) continue;

            if (paramRef->type == VX_TYPE_TENSOR)
            {
                if ((VX_TENSOR_LIFE_TIME_DYNAMIC != TENSOR_DATA_LIFETIME((vx_tensor)paramRef)) ||
                    (vxoMemory_GetType(&((vx_tensor)paramRef)->tensorBuffer->memory) == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR) ||
                    (node->kernel->signature.stateTable[paramIndex] == VX_PARAMETER_STATE_OPTIONAL))
                {
                    continue;
                }
            }
            else if (paramRef->type == VX_TYPE_IMAGE)
            {
                vx_image image = (vx_image)paramRef;
                if ((vxoMemory_GetType(&image->memory) == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR) ||
                   (node->kernel->signature.stateTable[paramIndex] == VX_PARAMETER_STATE_OPTIONAL))
                {
                    continue;
                }
            }
            else if (paramRef->type == VX_TYPE_ARRAY)
            {
                vx_array array = (vx_array)paramRef;
                if ((vxoMemory_GetType(&array->memory) == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR) ||
                   (node->kernel->signature.stateTable[paramIndex] == VX_PARAMETER_STATE_OPTIONAL))
                {
                    continue;
                }
            }
            else if (paramRef->type == VX_TYPE_SCALAR)
            {
                if (node->kernel->signature.stateTable[paramIndex] != VX_PARAMETER_STATE_OPTIONAL)
                {
                    /* scalar is graph output*/
                    outputTable[outNum++] = paramRef;
                }
                continue;
            }
            else
            {
               continue;
            }

            for (nodeIndex2 = vxoGraph_GetNextNodeIndex(graph, nodeIndex);
                nodeIndex2 != nodeIndex;
                nodeIndex2 = vxoGraph_GetNextNodeIndex(graph, nodeIndex2))
            {
                vx_node node2 = graph->nodeTable[nodeIndex2];

                for (paramIndex2 = 0; paramIndex2 < node2->kernel->signature.paramCount; paramIndex2++)
                {
                    vx_reference paramRef2 = node2->paramTable[paramIndex2];

                    if (paramRef2 == VX_NULL) continue;
                    if (node2->kernel->signature.directionTable[paramIndex2] == VX_OUTPUT) continue;
                    if (vxoReference_HasWriteDependency(paramRef, paramRef2))
                    {
                        breakFlag = vx_true_e;
                        break;
                    }
                }
                if (breakFlag) break;
            }
            if (nodeIndex2 == nodeIndex)
            {
                if ((paramRef->type == VX_TYPE_TENSOR) || (paramRef->type == VX_TYPE_IMAGE) ||
                    (paramRef->type == VX_TYPE_ARRAY))
                {
                    outputTable[outNum++] = paramRef;
                }
                else
                {
                    vxError("error: not support the type as graph output: %d\n", (vx_int32)paramRef->type);
                    vxmONERROR(VX_FAILURE);
                }
            }
        }
    }

    if ((inNum >= VX_MAX_NN_INOUT_PARAM_COUNT) || (outNum >= VX_MAX_NN_INOUT_PARAM_COUNT))
    {
        vxError("failed input or output bigger than MAX value..\n");
        vxmONERROR(VX_FAILURE);
    }

    *inputNum = inNum;
    *outputNum = outNum;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_unInitial(
    vx_graph graph
    )
{
    vx_binary_save binarySave = graph->binarySave;
    gcmHEADER_ARG("graph=%p", graph);

    if (binarySave == VX_NULL)
    {
        gcmFOOTER_ARG("%d", VX_SUCCESS);
        return VX_SUCCESS;
    }

    if (binarySave->patchNNTPCmdPhysical != VX_NULL)
    {
        gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)binarySave->patchNNTPCmdPhysical));
        binarySave->patchNNTPCmdPhysical = VX_NULL;
    }
    if (binarySave->patchNNTPCmdOffset != VX_NULL)
    {
        gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)binarySave->patchNNTPCmdOffset));
        binarySave->patchNNTPCmdOffset = VX_NULL;
    }
    if (binarySave->operationCmdPhysical != VX_NULL)
    {
        gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)binarySave->operationCmdPhysical));
        binarySave->operationCmdPhysical = VX_NULL;
    }
    if (binarySave->operationOffset != VX_NULL)
    {
        gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)binarySave->operationOffset));
        binarySave->operationOffset = VX_NULL;
    }

    if (binarySave->NNTPDataOffset != VX_NULL)
    {
        vxFree((vx_ptr)binarySave->NNTPDataOffset);
        binarySave->NNTPDataOffset = VX_NULL;
    }

    if (binarySave->NNTPDataCmdPhysical != VX_NULL)
    {
        vxFree((vx_ptr)binarySave->NNTPDataCmdPhysical);
        binarySave->NNTPDataCmdPhysical = VX_NULL;
    }

    if  ((0 == graph->binarySave->generateNBGToMemory) &&
        (0 != graph->base.context->options.enableSaveBinary))
    {
        vxInfo("Actual NBG size : %d bytes\n", binarySave->NBGFileSize);
    }

    if (binarySave->binarySaveFile != VX_NULL)
    {
        gcoOS_Flush(gcvNULL, binarySave->binarySaveFile);
        gcmVERIFY_OK(gcoOS_Close(gcvNULL, binarySave->binarySaveFile));
        binarySave->binarySaveFile = VX_NULL;
        vxInfo("network binary graph file has been closed, NBG name: %s\n", binarySave->binaryFileName);
    }

    if (graph->binarySave->inputInfo != VX_NULL)
    {
        vxFree(graph->binarySave->inputInfo);
        graph->binarySave->inputInfo = VX_NULL;
    }

    if (graph->binarySave != VX_NULL)
    {
        gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)graph->binarySave));
        graph->binarySave = VX_NULL;
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_SaveErrorHandle(
    vx_graph graph,
    vx_status status
    )
{
    vx_status re_status = status;
    vx_binary_save_s *binarySave = graph->binarySave;

    if ((graph->base.context->options.enableSaveBinary == 0) && (status != VX_SUCCESS) && (binarySave != VX_NULL))
    {
        /* error in cache binary graph mode, original path run network */
        re_status = VX_SUCCESS;
        if (gcoOS_StrCmp(binarySave->binaryFileName, "\0") != gcvSTATUS_OK)
        {
            gcoOS_Remove(gcvNULL, binarySave->binaryFileName);
        }
        vxoBinaryGraph_unInitial(graph);
    }
    else if ((graph->base.context->options.enableSaveBinary == 1) && (status != VX_SUCCESS) && (binarySave != VX_NULL))
    {
        re_status = VX_FAILURE;
        vxError("%s[%d]: failed to generate NBG file, remove it\n", __FUNCTION__, __LINE__);
        if (gcoOS_StrCmp(binarySave->binaryFileName, "\0") != gcvSTATUS_OK)
        {
            gcoOS_Remove(gcvNULL, binarySave->binaryFileName);
        }
    }

    return re_status;
}

VX_PRIVATE_API vx_bool vxoBinaryGraph_isSupportSWOperation(
    vxnne_operation operation
    )
{
    vx_bool isSupport = vx_false_e;

    if (operation->layer->num_operations != 1)
    {
        return isSupport;
    }

    if (VXNNE_OPERATOR_RPN == operation->operatorType)
    {
        isSupport = vx_true_e;
    }

    return isSupport;
}

VX_PRIVATE_API vx_uint32 vxoBinaryGraph_SaveLayerParamTable(
    vx_graph graph,
    void *layerParam
    )
{
    vx_uint32 index = 0;
    vx_status status = VX_SUCCESS;
    vx_binary_save binarySave = graph->binarySave;

    status = vxoBinaryGraph_Write(binarySave, binarySave->currLayerParamOffset,
                                  sizeof(vx_binary_layer_parameter_s), layerParam);
    if (status != VX_SUCCESS)
    {
        vxError("%s[%d]:failed to write data\n", __FUNCTION__, __LINE__);
        return 0xFFFFFFFF;
    }

    graph->binarySave->currLayerParamOffset += sizeof(vx_binary_layer_parameter_s);
    index = graph->binarySave->currLayerParamIndex++;

    return index;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_SaveSWOperationInfo(
    vx_graph graph,
    vx_uint32 operationType
)
{
    vx_binary_sw_operation_info_s swOperationInfo;
    vx_binary_save binarySave = graph->binarySave;
    vx_status status = VX_SUCCESS;

    swOperationInfo.swOperationType = operationType;

    status = vxoBinaryGraph_Write(binarySave, binarySave->currSWOperationOffset,
                         sizeof(vx_binary_sw_operation_info_s), &swOperationInfo);
    WRITE_NBG_STATUS_CHECK();

    binarySave->currSWOperationOffset += sizeof(vx_binary_sw_operation_info_s);

OnError:
    return status;
}

VX_PRIVATE_API vx_uint32 vxoBinaryGraph_SaveOperationTableForSW(
    vx_graph graph,
    vxnne_operation operation,
    vx_uint32 firstIndex,
    vx_uint32 paramCount
    )
{
    vx_binary_operation_info_s operationInfo;
    vx_binary_save binarySave = graph->binarySave;
    vx_node node = operation->layer->node;
    vx_uint32 index = 0, layerId = 0;
    vx_status status = VX_SUCCESS;

    INITIALIZE_STRUCT(operationInfo);
    /* find layer id */
    for (index = 0; index < graph->nodeCount; index++)
    {
        vx_node nodeTmp = graph->nodeTable[graph->allNodeIndexTable[index]];
        if (node == nodeTmp)
        {
            layerId = index;
            break;
        }
    }
    if (index == graph->nodeCount)
    {
        vxError("%s[%d]: fail to get layer ID in save operation table\n", __FUNCTION__, __LINE__);
    }
    operationInfo.indexOfFirstPatch = firstIndex;
    operationInfo.counterOfPatches = paramCount;
    operationInfo.layerId = layerId;
    operationInfo.operationType = VX_BINARY_OPERATION_TYPE_SW;
    operationInfo.operationIndex = binarySave->currSWOperationIndex++;
    operationInfo.stateLCDTIndex = 0xFFFF;

    /* find operation location */
    for (index = 0; index < binarySave->operationCount; index++)
    {
        if ((binarySave->operationCmdPhysical[index] == 0) || \
            (binarySave->swOperationOffset >= binarySave->operationOffset[index]))
        {
            continue;
        }
        else if (gcmPTR_TO_UINT64(operation) == binarySave->operationCmdPhysical[index])
        {
            break;
        }
    }

    if (index >= binarySave->operationCount)
    {
        vxError("error: fail to save operation, index: %d, opCount: %d\n",
            index, binarySave->operationCount);
        vxmONERROR(VX_ERROR_INVALID_VALUE);
    }

    status = vxoBinaryGraph_Write(binarySave, binarySave->operationOffset[index],
                                    sizeof(vx_binary_operation_info_s), &operationInfo);
    WRITE_NBG_STATUS_CHECK();

    binarySave->swOperationOffset = binarySave->operationOffset[index];

OnError:
    return status;
}

VX_PRIVATE_API vx_uint32 vxoBinaryGraph_GetSourceType(
    vx_graph graph,
    vxnne_operation operation,
    vx_enum allocType,
    vx_uint32 wholeSize,
    vx_uint32 basePhysical,
    vx_uint8_ptr baseLogical,
    vx_int32 *index
)
{
    vx_binary_save binarySave = graph->binarySave;
    vx_uint32 sourceType = 0;
    *index = -1;

    if (allocType == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR)
    {
        sourceType = VX_BINARY_SOURCE_MEMORY_POOL;
    }
    else if (allocType == VXNNE_MEM_POOL_TYPE_ORIG_DDR)
    {
        vx_int32 entryIndex = -1, LCDTindex = -1;

        entryIndex = vxoBinaryGraph_GetIndexOfInputOutputEntry(binarySave->inputPhysicalEntry, basePhysical);
        if (-1 == entryIndex)
        {
            entryIndex = vxoBinaryGraph_GetIndexOfInputOutputEntry(binarySave->outputPhysicalEntry, basePhysical);
            if (-1 == entryIndex)
            {
                vx_uint32 lcdSize = 0;

                LCDTindex = vxoBinaryGraph_GetLCDTIndexForTempTensor(graph, basePhysical, &lcdSize);
                if (-1 == LCDTindex)
                {
                    LCDTindex = (vx_int32)vxoBinaryGraph_SaveLoadingConfigData(graph, baseLogical, wholeSize, MEMORY_ALIGN_SIZE);
                    binarySave->tempTensorsPhysical[binarySave->numberOfTempTensorInfo].tensorPhysical = basePhysical;
                    binarySave->tempTensorsPhysical[binarySave->numberOfTempTensorInfo].LCDTIndex = LCDTindex;
                    binarySave->tempTensorsPhysical[binarySave->numberOfTempTensorInfo].size  = wholeSize;
                    binarySave->numberOfTempTensorInfo++;
                    vxmASSERT(binarySave->numberOfTempTensorInfo < VX_MAX_TEMP_TENSORS);
                    if (LCDTindex > 0xFFFE)
                    {
                        vxError("save data error for NBG\n");
                        return 0xFFFFFFFF;
                    }
                }
                else
                {
                    if (wholeSize > lcdSize)
                    {
                        vxError("error: fail to save tensor to lcd, LCDTindex: %d, lcdSize: %d, tensorSize: %d\n",
                                LCDTindex, lcdSize, wholeSize);
                        vxmASSERT(0);
                    }
                }
                sourceType = vxoBinaryGraph_GetMiscDynamicSourceType(operation, basePhysical);
                *index = LCDTindex;
            }
            else
            {
                sourceType  = VX_BINARY_SOURCE_OUTPUT;
                *index = entryIndex;
            }
        }
        else
        {
            sourceType  = VX_BINARY_SOURCE_INPUT;
            *index = entryIndex;
        }
    }

    if (VX_BINARY_SOURCE_INPUT == sourceType)
    {
        binarySave->inputInPatchedBinOffset[binarySave->inputInPatchedNum] = 0xFFFF;
        binarySave->inputInPatchedPhysical[binarySave->inputInPatchedNum] = basePhysical;
        binarySave->inputInPatchedIndex[binarySave->inputInPatchedNum] = 0xFFFF;
        binarySave->inputInPatchedNum++;
    }

    return sourceType;
}

VX_PRIVATE_API vx_uint32 vxoBinaryGraph_SaveTensorToLayerParamTable(
    vx_graph graph,
    vxnne_operation operation,
    vx_tensor tensor,
    vx_char *paramName
    )
{
    vx_binary_layer_parameter_s layerParam;
    vx_size tensorSize = 0;
    vx_status status = VX_SUCCESS;
    vx_uint32 wholeSize = 0, index = 0;
    gctPOINTER viewLogical;
    vx_uint32 viewPhysical;
    vx_uint8_ptr baseLogical = TENSOR_LOGICAL_ADDR(tensor);
    vx_uint32 basePhysical = TENSOR_PHYSICAL_ADDR(tensor);
    vx_enum allocType = vxoMemory_GetType(&tensor->tensorBuffer->memory);
    vxoTensor_GetTensorViewMemory(tensor, &viewLogical, &viewPhysical);

    vxoTensor_GetTensorWholeSize(tensor, &tensorSize);
    wholeSize = (vx_uint32)tensorSize;

    INITIALIZE_STRUCT(layerParam);

    vxmONERROR(vxQueryTensor(tensor, VX_TENSOR_DIMS, layerParam.dims,
                             sizeof(layerParam.dims)));
    vxmONERROR(vxQueryTensor(tensor, VX_TENSOR_NUMBER_OF_DIMS, &layerParam.dimCount,
                             sizeof(layerParam.dimCount)));

    vxMemCopy((vx_ptr)layerParam.paramName, (vx_const_ptr)paramName, sizeof(layerParam.paramName));
    layerParam.dataFormat = vxoBinaryGraph_ConvertToBinaryDataFormat(TENSOR_DATA_TYPE(tensor));
    layerParam.dataType = VX_BINARY_BUFFER_TYPE_TENSOR;
    layerParam.quantFormat = TENSOR_QUANT_TYPE(tensor);
    if (VX_QUANT_DYNAMIC_FIXED_POINT == TENSOR_QUANT_TYPE(tensor))
    {
        layerParam.fixPointZeroPoint = TENSOR_POS(tensor);
    }
    else
    {
        layerParam.tfScale = TENSOR_TF_SCALE(tensor);
        layerParam.fixPointZeroPoint = TENSOR_TF_ZEROPOINT(tensor);
    }

    layerParam.sourceType = vxoBinaryGraph_GetSourceType(graph, operation, allocType,
                                                wholeSize, basePhysical, baseLogical, &layerParam.index);

    if (layerParam.sourceType == VX_BINARY_SOURCE_MEMORY_POOL)
    {
        layerParam.addressOffset = viewPhysical - graph->memoryPool->physical;
    }
    else
    {
        layerParam.addressOffset = viewPhysical - basePhysical;
    }

    index = vxoBinaryGraph_SaveLayerParamTable(graph, &layerParam);

OnError:
    return index;
}

VX_PRIVATE_API vx_uint32 vxoBinaryGraph_SaveScalarToLayerParamTable(
    vx_graph graph,
    vxnne_operation operation,
    vx_scalar scalar,
    vx_char *paramName
)
{
    vx_binary_layer_parameter_s layerParam;
    vx_uint32 index = 0;
    vx_uint32 physical = scalar->physical;
    vx_uint32 size = vxoScalar_GetTypeSize(scalar);

    vxMemCopy((vx_ptr)layerParam.paramName, (vx_const_ptr)paramName, sizeof(layerParam.paramName));
    layerParam.dataFormat = vxoBinaryGraph_ConvertToBinaryDataFormat(scalar->dataType);
    layerParam.dataType = VX_BINARY_BUFFER_TYPE_SCALAR;
    layerParam.quantFormat = 0;
    layerParam.fixPointZeroPoint = 0;
    layerParam.tfScale = 0;
    layerParam.addressOffset = 0;
    layerParam.dimCount = 1;
    layerParam.dims[0] = size;
    layerParam.sourceType = vxoBinaryGraph_GetSourceType(graph, operation, VXNNE_MEM_POOL_TYPE_ORIG_DDR,
                                                        size, physical, (vx_uint8_ptr)scalar->value, &layerParam.index);

    index = vxoBinaryGraph_SaveLayerParamTable(graph, &layerParam);

    return index;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_SaveSWRPN(
    vxnne_operation operation
)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 indexOfFirst = 0, paramCount = 0;
    vx_graph graph = operation->layer->node->graph;
    vxnne_tensor_rpn_operation rpnOperation = (vxnne_tensor_rpn_operation)operation;
    vx_tensor score = rpnOperation->score;
    vx_tensor bbox = rpnOperation->bbox;
    vx_tensor anchor = rpnOperation->anchors;
    vx_tensor img_info = rpnOperation->img_info;
    vx_tensor roi_output = rpnOperation->roi_output;
    vx_tensor score_output = rpnOperation->score_output;

    vx_scalar feature_stride = rpnOperation->feature_stride;
    vx_scalar min_size = rpnOperation->min_size;
    vx_scalar pre_nms_topn = rpnOperation->pre_nms_topn;
    vx_scalar post_nms_topn = rpnOperation->post_nms_topn;
    vx_scalar nms_thresh = rpnOperation->nms_thresh;
    gcmHEADER_ARG("operation=%p", operation);

    indexOfFirst = vxoBinaryGraph_SaveTensorToLayerParamTable(graph, operation, score, "score");
    paramCount++;

    vxoBinaryGraph_SaveTensorToLayerParamTable(graph, operation, bbox, "bbox");
    paramCount++;

    vxoBinaryGraph_SaveTensorToLayerParamTable(graph, operation, anchor, "anchor");
    paramCount++;

    vxoBinaryGraph_SaveTensorToLayerParamTable(graph, operation, img_info, "img_info");
    paramCount++;

    vxoBinaryGraph_SaveTensorToLayerParamTable(graph, operation, roi_output, "roi_output");
    paramCount++;

    vxoBinaryGraph_SaveTensorToLayerParamTable(graph, operation, score_output, "score_output");
    paramCount++;

    vxoBinaryGraph_SaveScalarToLayerParamTable(graph, operation, feature_stride, "feature_stride");
    paramCount++;

    vxoBinaryGraph_SaveScalarToLayerParamTable(graph, operation, min_size, "min_size");
    paramCount++;

    vxoBinaryGraph_SaveScalarToLayerParamTable(graph, operation, pre_nms_topn, "pre_nms_topn");
    paramCount++;

    vxoBinaryGraph_SaveScalarToLayerParamTable(graph, operation, post_nms_topn, "post_nms_topn");
    paramCount++;

    vxoBinaryGraph_SaveScalarToLayerParamTable(graph, operation, nms_thresh, "nms_thresh");
    paramCount++;

    vxoBinaryGraph_SaveOperationTableForSW(graph, operation, indexOfFirst, paramCount);

    /*5. save SW operation data */
    vxoBinaryGraph_SaveSWOperationInfo(graph, VX_BINARY_SW_OPERATION_RPN);

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status vxoBinaryGraph_SaveSWOperation(
    vxnne_operation operation
)
{
    vx_context context = operation->layer->node->base.context;
    vx_graph graph = operation->layer->node->graph;
    vx_status status = VX_SUCCESS;
    gcmHEADER_ARG("operation=%p", operation);

    if (0 == context->options.enableSaveBinary)
    {
        gcmFOOTER_ARG("%d", VX_SUCCESS);
        return VX_SUCCESS;
    }

    switch (operation->operatorType)
    {
        case VXNNE_OPERATOR_RPN:
            vxmONERROR(vxoBinaryGraph_SaveSWRPN(operation));
            break;

        default:
            vxError("not implement this SW layer in binary graph. operator type: %d\n", operation->operatorType);
            break;
    }

    gcmFOOTER_ARG("%d", status);
    return status;

OnError:

    status = vxoBinaryGraph_SaveErrorHandle(graph, status);

    vxError("%s[%d]: fail to save SW layer\n", __FUNCTION__, __LINE__);
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_SaveInitialOperation(
    vx_graph graph
    )
{
    vx_binary_operation_info_s operationInfo;
    vx_context context = graph->base.context;
    vx_uint32 deviceID = graph->deviceID;
    vx_binary_save_s *binarySave = graph->binarySave;
    vx_ptr initBuffer = VX_NULL;
    vx_status status = VX_SUCCESS;
    vx_uint32 first = 0;
    vx_uint32 stateLCDTIndex = 0;

    gcmHEADER_ARG("graph=%p", graph);

    if (binarySave == VX_NULL)
    {
        vxError("%s[%d]: binary save is NULL\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_INVALID_VALUE);
    }

    if (context->binaryGraphInitBuffer == VX_NULL)
    {
        vxError("%s[%d]: BinaryGraphInitBuffer is NULL\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_FAILURE);
    }

    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof(vx_uint8) * context->binaryGraphInitSize[deviceID],
                                        (gctPOINTER*)&initBuffer)))
    {
        vxError("fail to allocate memory shade states buffer\n");
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }
    else
    {
        gcoOS_MemCopy(initBuffer, context->binaryGraphInitBuffer[deviceID], context->binaryGraphInitSize[deviceID]);
    }

    if ((context->binaryGraphInitBuffer[deviceID] != VX_NULL) && (context->binaryGraphInitSize[deviceID] > 0))
    {
        vx_uint32 check = 0;
        vx_uint32 indexOfFirstPatch = 0, patchCount = 0;
        vx_uint32 indexTemp = 0;
        vx_uint32 offsetArray[20] = {0};
        vx_int32 ret = 0;
        vx_binary_patch_info_s patchInfo;

        if (context->axiSRAM[deviceID].size > 0)
        {
            /*1. patch start/end remap address for AXI-SRAM */
            vx_uint32 axiSRAMPhysical = context->axiSRAM[deviceID].physical;
            vx_uint32 axiSRAMPhysicalEnd = context->axiSRAM[deviceID].physical + context->axiSRAM[deviceID].size;

            ret = vxoBinaryGraph_SearchPattern((gctUINT32 *)initBuffer,
                                          context->binaryGraphInitSize[deviceID]/gcmSIZEOF(gctUINT32),
                                          axiSRAMPhysical, offsetArray, vx_true_e);
            if (ret == 1)
            {   /* The AXI-SRAM need to be patched */
                /*1.1 patch start remap address */
                gcoOS_ZeroMemory(&patchInfo, sizeof(vx_binary_patch_info_s));
                patchInfo.offset = offsetArray[0];
                patchInfo.type                = VX_BINARY_PATCH_TYPE_COMMAND;
                patchInfo.sourceType          = VX_BINARY_SOURCE_AXI_SRAM;
                patchInfo.index               = -1;
                patchInfo.originalBaseAddress = axiSRAMPhysical;
                patchInfo.transformation = VX_BINARY_PATCH_TRANSFORMATION_ORIGINAL;
                #if (ENABLE_SAVE_OFFSET_IN_NBG == 1)
                vxmONERROR(vxoBinaryGraph_ChangeAddressToOffset(initBuffer, offsetArray[0],
                                                                patchInfo.originalBaseAddress,
                                                                patchInfo.transformation));
                patchInfo.originalBaseAddress = 0;
                #endif
                indexTemp = vxoBinaryGraph_SavePatchEntry(graph, (void *)&patchInfo);
                if (indexTemp == 0xFFFFFFFF)
                {
                    vxmONERROR(VX_ERROR_NO_MEMORY);
                }
                patchCount++;
                if (first == 0)
                {
                    indexOfFirstPatch = indexTemp;
                    first = 1;
                }

                /*1.2 patch end remap address */
                ret = vxoBinaryGraph_SearchPattern((gctUINT32 *)initBuffer,
                                      context->binaryGraphInitSize[deviceID]/gcmSIZEOF(gctUINT32),
                                      axiSRAMPhysicalEnd, offsetArray, vx_true_e);
                if (ret == 1)
                {
                    gcoOS_ZeroMemory(&patchInfo, sizeof(vx_binary_patch_info_s));
                    patchInfo.offset = offsetArray[0];
                    patchInfo.type                = VX_BINARY_PATCH_TYPE_COMMAND;
                    patchInfo.sourceType          = VX_BINARY_SOURCE_AXI_SRAM;
                    patchInfo.index               = -1;
                    patchInfo.originalBaseAddress = axiSRAMPhysical;
                    patchInfo.transformation      = VX_BINARY_PATCH_TRANSFORMATION_ORIGINAL;
                    #if (ENABLE_SAVE_OFFSET_IN_NBG == 1)
                    vxmONERROR(vxoBinaryGraph_ChangeAddressToOffset(initBuffer, offsetArray[0],
                                                                    patchInfo.originalBaseAddress,
                                                                    patchInfo.transformation));
                    patchInfo.originalBaseAddress = 0;
                    #endif
                    check = vxoBinaryGraph_SavePatchEntry(graph, (void *)&patchInfo);
                    if (check == 0xFFFFFFFF)
                    {
                        vxmONERROR(VX_ERROR_NO_MEMORY);
                    }
                    patchCount++;
                }
                else
                {
                    vx_uint32 offset[20] = {0};
                    ret = vxoBinaryGraph_SearchPattern((gctUINT32 *)initBuffer,
                                          context->binaryGraphInitSize[deviceID]/gcmSIZEOF(gctUINT32),
                                          0x08010e50, offset, vx_true_e);
                    if (ret == 1)
                    {
                        gcoOS_ZeroMemory(&patchInfo, sizeof(vx_binary_patch_info_s));
                        patchInfo.offset = offset[0] + sizeof(vx_uint32);
                        patchInfo.type                = VX_BINARY_PATCH_TYPE_COMMAND;
                        patchInfo.sourceType          = VX_BINARY_SOURCE_AXI_SRAM;
                        patchInfo.index               = -1;
                        patchInfo.originalBaseAddress = axiSRAMPhysical;
                        patchInfo.transformation      = VX_BINARY_PATCH_TRANSFORMATION_ORIGINAL;
                        #if (ENABLE_SAVE_OFFSET_IN_NBG == 1)
                        vxmONERROR(vxoBinaryGraph_ChangeAddressToOffset(initBuffer, patchInfo.offset,
                                                                         patchInfo.originalBaseAddress,
                                                                         patchInfo.transformation));
                        patchInfo.originalBaseAddress = 0;
                        #endif
                        check = vxoBinaryGraph_SavePatchEntry(graph, (void *)&patchInfo);
                        if (check == 0xFFFFFFFF)
                        {
                            vxmONERROR(VX_ERROR_NO_MEMORY);
                        }
                        patchCount++;
                    }
                    else
                    {
                        vxError("fail to search AXI-SRAM address in init command buffer\n");
                        vxmONERROR(VX_ERROR_INVALID_VALUE);
                    }
                }
            }
            else
            {
                vx_uint32 offset[20] = {0};
                vx_uint32 pathLocal = 0;
                ret = vxoBinaryGraph_SearchPattern((gctUINT32 *)initBuffer,
                                          context->binaryGraphInitSize[deviceID]/gcmSIZEOF(gctUINT32),
                                          0x08010e4f, offset, vx_true_e);
                if (ret == 1)
                {
                    pathLocal = offset[0] + sizeof(vx_uint32);
                    /*1.1 patch start remap address */
                    gcoOS_ZeroMemory(&patchInfo, sizeof(vx_binary_patch_info_s));
                    patchInfo.offset = pathLocal;
                    patchInfo.type                = VX_BINARY_PATCH_TYPE_COMMAND;
                    patchInfo.sourceType          = VX_BINARY_SOURCE_AXI_SRAM;
                    patchInfo.index               = -1;
                    patchInfo.originalBaseAddress = axiSRAMPhysical;
                    patchInfo.transformation      = VX_BINARY_PATCH_TRANSFORMATION_ORIGINAL;
                    #if (ENABLE_SAVE_OFFSET_IN_NBG == 1)
                    vxmONERROR(vxoBinaryGraph_ChangeAddressToOffset(initBuffer, patchInfo.offset,
                                                                     patchInfo.originalBaseAddress,
                                                                     patchInfo.transformation));
                    patchInfo.originalBaseAddress = 0;
                    #endif
                    indexTemp = vxoBinaryGraph_SavePatchEntry(graph, (void *)&patchInfo);
                    if (indexTemp == 0xFFFFFFFF)
                    {
                        vxmONERROR(VX_ERROR_NO_MEMORY);
                    }
                    patchCount++;
                    if (first == 0)
                    {
                        indexOfFirstPatch = indexTemp;
                        first = 1;
                    }

                    /*1.2 patch end remap address */
                    ret = vxoBinaryGraph_SearchPattern((gctUINT32 *)initBuffer,
                                          context->binaryGraphInitSize[deviceID]/gcmSIZEOF(gctUINT32),
                                          axiSRAMPhysicalEnd, offsetArray, vx_true_e);
                    if (ret == 1)
                    {
                        gcoOS_ZeroMemory(&patchInfo, sizeof(vx_binary_patch_info_s));
                        patchInfo.offset = offsetArray[0];
                        patchInfo.type                = VX_BINARY_PATCH_TYPE_COMMAND;
                        patchInfo.sourceType          = VX_BINARY_SOURCE_AXI_SRAM;
                        patchInfo.index               = -1;
                        patchInfo.originalBaseAddress = axiSRAMPhysical;
                        patchInfo.transformation      = VX_BINARY_PATCH_TRANSFORMATION_ORIGINAL;
                        #if (ENABLE_SAVE_OFFSET_IN_NBG == 1)
                        vxmONERROR(vxoBinaryGraph_ChangeAddressToOffset(initBuffer, patchInfo.offset,
                                                                         patchInfo.originalBaseAddress,
                                                                         patchInfo.transformation));
                        patchInfo.originalBaseAddress = 0;
                        #endif
                        check = vxoBinaryGraph_SavePatchEntry(graph, (void *)&patchInfo);
                        if (check == 0xFFFFFFFF)
                        {
                            vxmONERROR(VX_ERROR_NO_MEMORY);
                        }
                        patchCount++;
                    }
                    else
                    {
                        vxError("fail to search AXI-SRAM address in init command buffer\n");
                        vxmONERROR(VX_ERROR_INVALID_VALUE);
                    }
                }
                else
                {
                    vxError("fail to search AXI-SRAM address in init command buffer\n");
                    vxmONERROR(VX_ERROR_INVALID_VALUE);
                }
            }
        }

        if ((context->vipSRAM.size > 0) && (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SWTILING_PHASE3)))
        {
            /*1. patch start remap address for VIP-SRAM */
            vx_uint32 vipSRAMPhysical = context->vipSRAM.physical - VX_VIP_SRAM_IMAGE_STREAM_SIZE;

            ret = vxoBinaryGraph_SearchPattern((gctUINT32 *)initBuffer,
                                                context->binaryGraphInitSize[deviceID]/gcmSIZEOF(gctUINT32),
                                                vipSRAMPhysical, offsetArray, vx_true_e);
            if (ret == 1)
            {   /* The VIP-SRAM need to be patched */
                /*1.1 patch start remap address */
                gcoOS_ZeroMemory(&patchInfo, sizeof(vx_binary_patch_info_s));
                patchInfo.offset = offsetArray[0];
                patchInfo.type                = VX_BINARY_PATCH_TYPE_COMMAND;
                patchInfo.sourceType          = VX_BINARY_SOURCE_VIP_SRAM;
                patchInfo.index               = -1;
                patchInfo.originalBaseAddress = vipSRAMPhysical;
                patchInfo.transformation      = VX_BINARY_PATCH_TRANSFORMATION_ORIGINAL;
                #if (ENABLE_SAVE_OFFSET_IN_NBG == 1)
                vxmONERROR(vxoBinaryGraph_ChangeAddressToOffset(initBuffer, patchInfo.offset,
                                                                 patchInfo.originalBaseAddress,
                                                                 patchInfo.transformation));
                patchInfo.originalBaseAddress = 0;
                #endif
                indexTemp = vxoBinaryGraph_SavePatchEntry(graph, (void *)&patchInfo);
                if (indexTemp == 0xFFFFFFFF)
                {
                    vxmONERROR(VX_ERROR_NO_MEMORY);
                }
                patchCount++;
                if (first == 0)
                {
                    indexOfFirstPatch = indexTemp;
                    first = 1;
                }
            }
            else
            {
                vx_uint32 offset[20] = {0};
                vxError("fail to search VIP-SRAM map address in init command buffer\n");
                ret = vxoBinaryGraph_SearchPattern((gctUINT32 *)initBuffer,
                                                context->binaryGraphInitSize[deviceID]/gcmSIZEOF(gctUINT32),
                                                0x08010e4e, offset, vx_true_e);
                if (ret == 1)
                {
                    /*1.1 patch start remap address */
                    gcoOS_ZeroMemory(&patchInfo, sizeof(vx_binary_patch_info_s));
                    patchInfo.offset = offset[0] + sizeof(vx_uint32);
                    patchInfo.type                = VX_BINARY_PATCH_TYPE_COMMAND;
                    patchInfo.sourceType          = VX_BINARY_SOURCE_VIP_SRAM;
                    patchInfo.index               = -1;
                    patchInfo.originalBaseAddress = vipSRAMPhysical;
                    patchInfo.transformation      = VX_BINARY_PATCH_TRANSFORMATION_ORIGINAL;
                    #if (ENABLE_SAVE_OFFSET_IN_NBG == 1)
                    vxmONERROR(vxoBinaryGraph_ChangeAddressToOffset(initBuffer, patchInfo.offset,
                                                                     patchInfo.originalBaseAddress,
                                                                     patchInfo.transformation));
                    patchInfo.originalBaseAddress = 0;
                    #endif
                    indexTemp = vxoBinaryGraph_SavePatchEntry(graph, (void *)&patchInfo);
                    if (indexTemp == 0xFFFFFFFF)
                    {
                        vxmONERROR(VX_ERROR_NO_MEMORY);
                    }
                    patchCount++;
                    if (first == 0)
                    {
                        indexOfFirstPatch = indexTemp;
                        first = 1;
                    }
                }
                else
                {
                    vxError("fail to search VIP-SRAM address in init command buffer\n");
                    vxmONERROR(VX_ERROR_INVALID_VALUE);
                }
            }
        }

        /*2. save INIT command to LCD*/
        stateLCDTIndex = vxoBinaryGraph_SaveLoadingConfigData(graph,
                                                              (vx_uint8_ptr)initBuffer,
                                                              context->binaryGraphInitSize[deviceID],
                                                              MEMORY_ALIGN_SIZE);
        if (stateLCDTIndex == 0xFFFFFFFF)
        {
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }

        /*3. save operation info*/
        operationInfo.indexOfFirstPatch = indexOfFirstPatch;
        operationInfo.counterOfPatches = patchCount;
        operationInfo.layerId = 0xFFFF;
        operationInfo.operationType = VX_BINARY_OPERATION_TYPE_INIT;
        operationInfo.operationIndex = 0xFFFF;
        operationInfo.stateLCDTIndex = stateLCDTIndex;

        status= vxoBinaryGraph_Write(binarySave, binarySave->currOperationOffset,
                                     sizeof(vx_binary_operation_info_s), &operationInfo);
        WRITE_NBG_STATUS_CHECK();

        binarySave->currOperationOffset += sizeof(vx_binary_operation_info_s);

        binarySave->savedOperationCount++;
    }
    else
    {
        vxError("%s[%d]: not save initialization command init size: %d\n", __FUNCTION__, __LINE__,
                context->binaryGraphInitSize[deviceID] );
    }

OnError:
    if (initBuffer != VX_NULL)
    {
        gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)initBuffer));
        initBuffer = VX_NULL;
    }

    status = vxoBinaryGraph_SaveErrorHandle(graph, status);

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status vxoBinaryGraph_SaveEndOperation(
    vx_graph graph,
    vx_uint8_ptr stateBuffer,
    vx_uint32 stateSize
    )
{
    vx_binary_operation_info_s operationInfo;
    vx_binary_save_s *binarySave = graph->binarySave;
    vx_status status = VX_SUCCESS;
    vx_uint32 stateLCDTIndex = 0;

    gcmHEADER_ARG("graph=%p", graph);

    if ((binarySave == VX_NULL) || (stateSize == 0) || (stateBuffer == VX_NULL))
    {
        vxError("%s[%d]: binary save or stateBuffer is NULL\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_INVALID_VALUE);
    }

    if (stateSize > VX_MAX_INITIALIZE_COMMAND_SIZE)
    {
        vxError("%s[%d]: failed, states size is bigget than %d > %d\n", __FUNCTION__, __LINE__,
                    stateSize, VX_MAX_INITIALIZE_COMMAND_SIZE);
        vxmONERROR(VX_ERROR_INVALID_VALUE);
    }

    /*2. save END commands to LCD*/
    stateLCDTIndex = vxoBinaryGraph_SaveLoadingConfigData(graph,
                                                          stateBuffer,
                                                          stateSize,
                                                          MEMORY_ALIGN_SIZE);
    if (stateLCDTIndex == 0xFFFFFFFF)
    {
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }

    /*3. save operation info*/
    operationInfo.indexOfFirstPatch = 0xFFFF;
    operationInfo.counterOfPatches = 0;
    operationInfo.layerId = 0xFFFF;
    operationInfo.operationType = VX_BINARY_OPERATION_TYPE_END;
    operationInfo.operationIndex = 0xFFFF;
    operationInfo.stateLCDTIndex = stateLCDTIndex;

    status = vxoBinaryGraph_Write(binarySave, binarySave->lastOperation0ffset,
                                  sizeof(vx_binary_operation_info_s), &operationInfo);
    WRITE_NBG_STATUS_CHECK();

    binarySave->lastOperation0ffset += sizeof(vx_binary_operation_info_s);

    binarySave->savedOperationCount++;

OnError:
    status = vxoBinaryGraph_SaveErrorHandle(graph, status);

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status vxoBinaryGraph_SaveScalerOperation(
    vxnne_operation operation,
    gctUINT8_PTR stateLogical,
    gctUINT32 stateSize
    )
{
    vx_status status = VX_SUCCESS;
    vx_binary_patch_info_s patchInfo;
    vx_binary_operation_info_s operationInfo;
    vx_graph graph = operation->layer->node->graph;
    vx_node node = operation->layer->node;
    vxnne_yuv2rgb_scale_operation scaleOperation = (vxnne_yuv2rgb_scale_operation)operation;
    vx_image inputImage = scaleOperation->inputs;
    vx_tensor output = scaleOperation->outputs;
    vx_binary_save_s *binarySave = graph->binarySave;
    vx_uint32 stateLCDTIndex = 0;
    vx_uint32 plane = 0, index = 0;
    vx_int32 ret = 0;
    vx_uint32 firstPatchIndex = 0, patchCount = 0, patchIndex = 0;
    vx_uint32 offsetArray[VX_MAX_SHADER_PARAMETERS] = {0};
    vx_int32 entryIndex = -1;
    vx_int32 LCDTindex = -1;
    vx_enum sourceType = VX_BINARY_SOURCE_MAX;
    vx_uint32 layerId = 0;
    vx_context context = node->base.context;
    vx_uint8 *stateBuffer = VX_NULL;

    gcmHEADER_ARG("operation=%p, stateLogical=%p, stateSize=0x%x", operation, stateLogical, stateSize);

    if (VX_NULL == binarySave)
    {
        vxError("%s[%d]: binary save is NULL\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_INVALID_VALUE);
    }

    if (VX_NULL == stateLogical)
    {
        vxError("%s[%d]: state Buffer is NULL\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_INVALID_VALUE);
    }

    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof(vx_uint8) * stateSize,
                                        (gctPOINTER*)&stateBuffer)))
    {
        vxError("fail to allocate memory shade states buffer\n");
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }
    else
    {
        gcoOS_MemCopy(stateBuffer, stateLogical, stateSize);
    }

    /*1. save input image to patch table */
    for (plane = 0; plane < inputImage->planeCount; plane++)
    {
        vx_enum allocType = vxoMemory_GetType(&inputImage->memory);
        vx_rectangle_t rect = scaleOperation->rect;
        vx_uint32 planePhysicalBase = inputImage->memory.physicals[plane];
        vx_uint32 planePhysicalRect = inputImage->memory.physicals[plane] +
                                    vxComputePlaneOffset(inputImage, rect.start_x, rect.start_y, plane);
        vx_uint8_ptr planeLogicaBase = inputImage->memory.logicals[plane];
        vx_uint32 planeSize = (vx_uint32)inputImage->memory.sizes[plane];

        INITIALIZE_STRUCT(patchInfo);

        if (allocType == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR)
        {
            sourceType = VX_BINARY_SOURCE_MEMORY_POOL;
        }
        else if (allocType & VXNNE_MEM_POOL_TYPE_AXI_SRAM)
        {
            sourceType = VX_BINARY_SOURCE_AXI_SRAM;
        }
        else if (allocType & VXNNE_MEM_POOL_TYPE_VIP_SRAM)
        {
            sourceType = VX_BINARY_SOURCE_VIP_SRAM;
        }
        else if (allocType == VXNNE_MEM_POOL_TYPE_ORIG_DDR)
        {
            entryIndex = vxoBinaryGraph_GetIndexOfInputOutputEntry(binarySave->inputPhysicalEntry,
                                                                  inputImage->memory.physicals[0]);/* y channel address in inputPhysicalEntry*/
            if (-1 == entryIndex)
            {
                vx_uint32 lcdSize = 0;

                LCDTindex = vxoBinaryGraph_GetLCDTIndexForTempTensor(graph, planePhysicalBase, &lcdSize);
                if (-1 == LCDTindex)
                {
                    LCDTindex = (vx_int32)vxoBinaryGraph_SaveLoadingConfigData(graph, planeLogicaBase, planeSize, MEMORY_ALIGN_SIZE);
                    binarySave->tempTensorsPhysical[binarySave->numberOfTempTensorInfo].tensorPhysical = planePhysicalBase;
                    binarySave->tempTensorsPhysical[binarySave->numberOfTempTensorInfo].LCDTIndex = LCDTindex;
                    binarySave->tempTensorsPhysical[binarySave->numberOfTempTensorInfo].size  = planeSize;
                    binarySave->numberOfTempTensorInfo++;
                    if (LCDTindex > 0xFFFE)
                    {
                        vxmONERROR(VX_ERROR_NO_MEMORY);
                    }
                }
                else
                {
                    if (planeSize > lcdSize)
                    {
                        vxError("error: fail to save scaler's tensor to lcd, LCDTindex: %d, lcdSize: %d, tensorSize: %d\n",
                                LCDTindex, lcdSize, planeSize);
                    }
                }
                sourceType = VX_BINARY_SOURCE_MISC_DYNAMIC_INPUT;
            }
            else
            {
                sourceType = VX_BINARY_SOURCE_INPUT;
                vxInfo("target scaler, layer %s is input of the binary graph\n", node->layer->name);
            }
        }
        else
        {
            vxError("%s[%d]: not support this alloc type: %d\n", __FUNCTION__, __LINE__, allocType);
            vxmONERROR(VX_ERROR_INVALID_VALUE);
        }

        patchInfo.type = VX_BINARY_PATCH_TYPE_STATE;
        if (allocType == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR)
        {
            patchInfo.originalBaseAddress = graph->memoryPool->physical;
        }
        else if (allocType & VXNNE_MEM_POOL_TYPE_AXI_SRAM)
        {
            patchInfo.originalBaseAddress = graph->base.context->axiSRAM[graph->deviceID].physical;
        }
        else if (allocType & VXNNE_MEM_POOL_TYPE_VIP_SRAM)
        {
            patchInfo.originalBaseAddress = node->base.context->vipSRAM.physical - VX_VIP_SRAM_IMAGE_STREAM_SIZE;
        }
        else
        {
            patchInfo.originalBaseAddress = planePhysicalBase;
        }
        patchInfo.sourceType = sourceType;
        switch (sourceType)
        {
            case VX_BINARY_SOURCE_MEMORY_POOL:
            case VX_BINARY_SOURCE_AXI_SRAM:
            case VX_BINARY_SOURCE_VIP_SRAM:
                patchInfo.index = -1;
                break;
            case VX_BINARY_SOURCE_INPUT:
            case VX_BINARY_SOURCE_OUTPUT:
                patchInfo.index = (vx_uint32)entryIndex;
                break;
            case VX_BINARY_SOURCE_MISC_DYNAMIC_INPUT:
                patchInfo.index = (vx_uint32)LCDTindex;
                break;
            default:
                vxError("%s[%d]: source type: %d\n", __FUNCTION__, __LINE__, sourceType);
                break;
        }
        patchInfo.transformation = VX_BINARY_PATCH_TRANSFORMATION_ORIGINAL;

        if ((VX_BINARY_SOURCE_INPUT == patchInfo.sourceType) && (0 == plane))
        {
            binarySave->inputInPatchedBinOffset[binarySave->inputInPatchedNum] = binarySave->currPatchOffset;
            binarySave->inputInPatchedPhysical[binarySave->inputInPatchedNum] = inputImage->memory.physicals[0];
            binarySave->inputInPatchedIndex[binarySave->inputInPatchedNum] = patchInfo.index;
            binarySave->inputInPatchedNum++;
        }

        ret = vxoBinaryGraph_SearchPattern((gctUINT32 *)stateBuffer,
                                      stateSize/gcmSIZEOF(gctUINT32),
                                      planePhysicalRect, offsetArray, vx_false_e);
        if (ret != 1)
        {
            vxError("fail to search image address in states buffer\n");
            vxmONERROR(VX_ERROR_INVALID_VALUE);
        }
        patchInfo.offset = offsetArray[0];
        #if (ENABLE_SAVE_OFFSET_IN_NBG == 1)
        vxmONERROR(vxoBinaryGraph_ChangeAddressToOffset(stateBuffer, patchInfo.offset,
                                                         patchInfo.originalBaseAddress,
                                                         patchInfo.transformation));
        patchInfo.originalBaseAddress = 0;
        #endif

        patchIndex = vxoBinaryGraph_SavePatchEntry(graph, (void *)&patchInfo);
        patchCount++;
        if (0 == plane)
        {
            firstPatchIndex = patchIndex;
        }
    }

    /*2. save output RGB to patch table */
    {
        vx_enum allocType = vxoMemory_GetType(&output->tensorBuffer->memory);
        vx_uint32 physicalAddrBase = TENSOR_PHYSICAL_ADDR(output);
        vx_uint8_ptr logicalAddr = TENSOR_LOGICAL_ADDR(output);
        vx_size tensorSize = 0;
        vx_uint32 wholeSize = 0;
        vx_uint32 outputPhyddr = 0, outputPhyddrR = 0, outputPhyddrG = 0, outputPhyddrB = 0;
        vx_uint32 offset = 0;

        vxoTensor_GetTensorWholeSize(output, &tensorSize);
        wholeSize = (vx_uint32)tensorSize;

        if (TENSOR_DIM_NUM(output) > 3)
        {
            vx_uint32 outputSize = 0;
            outputSize = TENSOR_VIEW_SIZE_INDEX(output, 3);
            if (outputSize != 1)
            {
                vxError("%s[%d]: not support output dim: %d in scaler output, outputSize: %d\n", __FUNCTION__, __LINE__, TENSOR_DIM_NUM(output), outputSize);
                vxmONERROR(VX_FAILURE);
            }
        }

        INITIALIZE_STRUCT(patchInfo);

        if (allocType == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR)
        {
            sourceType = VX_BINARY_SOURCE_MEMORY_POOL;
        }
        else if (allocType & VXNNE_MEM_POOL_TYPE_AXI_SRAM)
        {
            sourceType = VX_BINARY_SOURCE_AXI_SRAM;
        }
        else if (allocType & VXNNE_MEM_POOL_TYPE_VIP_SRAM)
        {
            sourceType = VX_BINARY_SOURCE_VIP_SRAM;
        }
        else if (allocType == VXNNE_MEM_POOL_TYPE_ORIG_DDR)
        {
            entryIndex = vxoBinaryGraph_GetIndexOfInputOutputEntry(binarySave->outputPhysicalEntry,
                                                                   physicalAddrBase);
            if (-1 == entryIndex)
            {
                vx_uint32 lcdSize = 0;

                LCDTindex = vxoBinaryGraph_GetLCDTIndexForTempTensor(graph, physicalAddrBase, &lcdSize);
                if (-1 == LCDTindex)
                {
                    LCDTindex = (vx_int32)vxoBinaryGraph_SaveLoadingConfigData(graph, logicalAddr, wholeSize, MEMORY_ALIGN_SIZE);
                    binarySave->tempTensorsPhysical[binarySave->numberOfTempTensorInfo].tensorPhysical = physicalAddrBase;
                    binarySave->tempTensorsPhysical[binarySave->numberOfTempTensorInfo].LCDTIndex = LCDTindex;
                    binarySave->tempTensorsPhysical[binarySave->numberOfTempTensorInfo].size  = wholeSize;
                    binarySave->numberOfTempTensorInfo++;
                    if (LCDTindex > 0xFFFE)
                    {
                        vxmONERROR(VX_ERROR_NO_MEMORY);
                    }
                }
                else
                {
                    if (wholeSize > lcdSize)
                    {
                        vxError("error: fail to save scaler's tensor to lcd, LCDTindex: %d, lcdSize: %d, tensorSize: %d\n",
                                LCDTindex, lcdSize, wholeSize);
                    }
                }
                sourceType = VX_BINARY_SOURCE_MISC_DYNAMIC_OUTPUT;
            }
            else
            {
                sourceType = VX_BINARY_SOURCE_OUTPUT;
            }
        }

        patchInfo.type = VX_BINARY_PATCH_TYPE_STATE;
        if (allocType == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR)
        {
            patchInfo.originalBaseAddress = graph->memoryPool->physical;
        }
        else if (allocType & VXNNE_MEM_POOL_TYPE_AXI_SRAM)
        {
            patchInfo.originalBaseAddress = graph->base.context->axiSRAM[graph->deviceID].physical;
        }
        else if (allocType & VXNNE_MEM_POOL_TYPE_VIP_SRAM)
        {
            patchInfo.originalBaseAddress = node->base.context->vipSRAM.physical - VX_VIP_SRAM_IMAGE_STREAM_SIZE;
        }
        else
        {
            patchInfo.originalBaseAddress = physicalAddrBase;
        }
        patchInfo.sourceType = sourceType;
        switch (sourceType)
        {
            case VX_BINARY_SOURCE_MEMORY_POOL:
            case VX_BINARY_SOURCE_AXI_SRAM:
            case VX_BINARY_SOURCE_VIP_SRAM:
                patchInfo.index = -1;
                break;
            case VX_BINARY_SOURCE_OUTPUT:
                patchInfo.index = (vx_uint32)entryIndex;
                break;
            case VX_BINARY_SOURCE_MISC_DYNAMIC_OUTPUT:
                patchInfo.index = (vx_uint32)LCDTindex;
                break;
            default:
                vxError("%s[%d]: not support source type: %d\n", __FUNCTION__, __LINE__, sourceType);
                break;
        }
        patchInfo.transformation = VX_BINARY_PATCH_TRANSFORMATION_ORIGINAL;

        vxoTensor_GetTensorViewMemory(output, VX_NULL, &outputPhyddr);
        offset = scaleOperation->output_y_start * TENSOR_STRIDE_INDEX(output, 1);
        outputPhyddrR = outputPhyddr + offset;
        outputPhyddrG = outputPhyddr + offset + TENSOR_STRIDE_INDEX(output, 2) * 1;
        outputPhyddrB = outputPhyddr + offset + TENSOR_STRIDE_INDEX(output, 2) * 2;

        /* R */
        ret = vxoBinaryGraph_SearchPattern((gctUINT32 *)stateBuffer,
                                      stateSize/gcmSIZEOF(gctUINT32),
                                      outputPhyddrR, offsetArray, vx_false_e);
        if (ret != 1)
        {
            vxError("fail to search output tensor R address in states buffer\n");
            vxmONERROR(VX_ERROR_INVALID_VALUE);
        }
        patchInfo.offset = offsetArray[0];
        #if (ENABLE_SAVE_OFFSET_IN_NBG == 1)
        vxmONERROR(vxoBinaryGraph_ChangeAddressToOffset(stateBuffer, patchInfo.offset,
                                                         patchInfo.originalBaseAddress,
                                                         patchInfo.transformation));
        patchInfo.originalBaseAddress = 0;
        #endif
        patchIndex = vxoBinaryGraph_SavePatchEntry(graph, (void *)&patchInfo);
        patchCount++;
        if (patchIndex == 0xFFFFFFFF)
        {
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }

        /* G */
        ret = vxoBinaryGraph_SearchPattern((gctUINT32 *)stateBuffer,
                                      stateSize/gcmSIZEOF(gctUINT32),
                                      outputPhyddrG, offsetArray, vx_false_e);
        if (ret != 1)
        {
            vxError("fail to search output tensor G address in states buffer\n");
            vxmONERROR(VX_ERROR_INVALID_VALUE);
        }
        patchInfo.offset = offsetArray[0];
        #if (ENABLE_SAVE_OFFSET_IN_NBG == 1)
        vxmONERROR(vxoBinaryGraph_ChangeAddressToOffset(stateBuffer, patchInfo.offset,
                                                         patchInfo.originalBaseAddress,
                                                         patchInfo.transformation));
        patchInfo.originalBaseAddress = 0;
        #endif
        patchIndex = vxoBinaryGraph_SavePatchEntry(graph, (void *)&patchInfo);
        patchCount++;
        if (patchIndex == 0xFFFFFFFF)
        {
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }

        /* B */
        ret = vxoBinaryGraph_SearchPattern((gctUINT32 *)stateBuffer,
                                      stateSize/gcmSIZEOF(gctUINT32),
                                      outputPhyddrB, offsetArray, vx_false_e);
        if (ret != 1)
        {
            vxError("fail to search output tensor B address in states buffer\n");
            vxmONERROR(VX_ERROR_INVALID_VALUE);
        }
        patchInfo.offset = offsetArray[0];
        #if (ENABLE_SAVE_OFFSET_IN_NBG == 1)
        vxmONERROR(vxoBinaryGraph_ChangeAddressToOffset(stateBuffer, patchInfo.offset,
                                                         patchInfo.originalBaseAddress,
                                                         patchInfo.transformation));
        patchInfo.originalBaseAddress = 0;
        #endif
        patchIndex = vxoBinaryGraph_SavePatchEntry(graph, (void *)&patchInfo);
        patchCount++;
        if (patchIndex == 0xFFFFFFFF)
        {
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }
    }

    /*3. save operation info */
    INITIALIZE_STRUCT(operationInfo);
    for (index = 0; index < graph->nodeCount; index++)
    {
        vx_node nodeTmp = graph->nodeTable[graph->allNodeIndexTable[index]];
        if (node == nodeTmp)
        {
            layerId = index;
            break;
        }
    }
    if (index == graph->nodeCount)
    {
        vxError("%s[%d]: fail to get layer ID in save scalaer operation\n", __FUNCTION__, __LINE__);
    }

    /*4. save states to LCD */
    stateLCDTIndex = vxoBinaryGraph_SaveLoadingConfigData(graph,
                                                          stateBuffer,
                                                          (vx_uint32)stateSize,
                                                          MEMORY_ALIGN_SIZE);
    if (stateLCDTIndex == 0xFFFFFFFF)
    {
        vxError("%s[%d]: fail to save scaler states to LCD\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }

    operationInfo.indexOfFirstPatch = firstPatchIndex;
    operationInfo.counterOfPatches = patchCount;
    operationInfo.layerId = layerId;
    operationInfo.operationType = VX_BINARY_OPERATION_TYPE_SC;
    operationInfo.operationIndex = 0xFFFF;
    operationInfo.stateLCDTIndex = stateLCDTIndex;

    for (index = 0; index < binarySave->operationCount; index++)
    {
        vx_uint64 cmdAddr = binarySave->operationCmdPhysical[index];
        if ((cmdAddr == 0) || (binarySave->scOperationOffset >= binarySave->operationOffset[index]))
        {
            continue;
        }
        else if (0xa5babaed == cmdAddr) /* 0xa5babaed is magic data */
        {
            break;
        }
    }

    if (index >= binarySave->operationCount)
    {
        vxError("error: fail to save scaler operation, index: %d, opCount: %d\n",
            index, binarySave->operationCount);
        vxmONERROR(VX_ERROR_INVALID_VALUE);
    }

    status = vxoBinaryGraph_Write(binarySave, binarySave->operationOffset[index],
                                  sizeof(vx_binary_operation_info_s), &operationInfo);
    WRITE_NBG_STATUS_CHECK();

    binarySave->scOperationOffset = binarySave->operationOffset[index];

    binarySave->savedOperationCount++;
OnError:
    if (stateBuffer != VX_NULL)
    {
        gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)stateBuffer));
        stateBuffer = VX_NULL;
    }

    status = vxoBinaryGraph_SaveErrorHandle(graph, status);

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status vxoBinaryGraph_SaveShaderOperation(
    vx_node node,
    vxnne_operation operation,
    vx_shader kernelShader,
    vx_reference *shParameter,
    vx_uint32 shParamNum,
    gctUINT8_PTR stateLogical,
    gctUINT32 stateSize,
    vx_uint32 batchIndex
    )
{
    vx_binary_operation_info_s operationInfo;
    vx_binary_sh_operation_info_s shOperationInfo;
    vx_uint32 patchCount = 0;
    vx_uint32 stateLCDTIndex = 0;
    gcsSURF_NODE_PTR instMemNode = VX_NULL;
    gcsSURF_NODE_PTR sharedMemNode = VX_NULL;
    vx_uint32 instructionLCDTIndex = 0;
    vx_uint32 firstPatchIndex = 0;
    gcsHINT_PTR hints = kernelShader->states.programState.hints;
    vx_binary_save_s *binarySave = node->graph->binarySave;
    vx_graph graph = node->graph;
    vx_uint32 index = 0;
    vx_uint32 offsetArray[VX_MAX_SHADER_PARAMETERS] = {0};
    vx_uint32 patchedParamPhy[VX_MAX_SHADER_PARAMETERS] = {0};
    vx_status status = VX_SUCCESS;
    vx_uint32 patchParamNum = 0;
    vx_uint32 layerId = 0;
    vx_uint8 *stateBuffer = VX_NULL;

    gcmHEADER_ARG("node=%p, operation=%p, kernelShader=%p, shParameter=%p, shParamNum=0x%x, stateLogical=0x%x, stateSize=0x%x, batchIndex=0x%x",
        node, operation, kernelShader, shParameter, shParamNum, stateLogical, stateSize, batchIndex);
    if (VX_NULL == binarySave)
    {
        vxError("%s[%d]: binary save is NULL\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_INVALID_VALUE);
    }

    if (VX_NULL == stateLogical)
    {
        vxError("%s[%d]: state Buffer is NULL\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_INVALID_VALUE);
    }

    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof(vx_uint8) * stateSize,
                                        (gctPOINTER*)&stateBuffer)))
    {
        vxError("fail to allocate memory shade states buffer\n");
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }
    else
    {
        gcoOS_MemCopy(stateBuffer, stateLogical, stateSize);
    }

    /* remove draw ID command in shader states buffer */
    #if (ENABLE_SAVE_OFFSET_IN_NBG == 1)
    {
    vx_uint32 *tmpBuffer = (vx_uint32*)stateBuffer;
    vx_uint32 size = stateSize / sizeof(vx_uint32);
    for (index = 0; index < size; index++)
    {
        if (tmpBuffer[index] == 0x08010E27)
        {
            tmpBuffer[index + 1] = 0x00;
        }
    }
    }
    #endif

    /*1. save shader's instruction to LCD */
    instMemNode = (gcsSURF_NODE_PTR)hints->shaderVidNodes.instVidmemNode[gceSGSK_FRAGMENT_SHADER];
    if (VX_NULL != instMemNode)
    {
        gctUINT32 address;
        vx_int32 ret = -1;
        vx_binary_patch_info_s patchInfo;
        vx_uint32 SHinstrSize = hints->fsInstCount * sizeof(gctUINT) * 4;
        vx_binary_save_s *binarySave = graph->binarySave;
        vx_uint32 alignSize = 0;
        vx_uint32 diffSize = 0;

        /* SH instr should be aligned to 256 */
        alignSize = gcmALIGN_SAFE(binarySave->currLoadingDataOffset, SH_COMMAND_ALIGN_SIZE);
        if (alignSize > binarySave->currLoadingDataOffset)
        {
            diffSize = alignSize - binarySave->currLoadingDataOffset;
        }

        if (diffSize > 0)
        {
            /* fill zero to LCD for alignSize */
            vxoBinaryGraph_SaveLoadingConfigData(node->graph, VX_NULL, diffSize, 0);
        }

        if (SHinstrSize > (vx_uint32)instMemNode->size)
        {
            SHinstrSize = (vx_uint32)instMemNode->size;
            vxError("%s[%d]: SHinstrSize: 0x%x is large than instMemNode->size: 0x%x\n", __FUNCTION__, __LINE__,
                     SHinstrSize, (vx_uint32)instMemNode->size);
        }

        gcmGETHARDWAREADDRESSP(instMemNode,address);

        instructionLCDTIndex = vxoBinaryGraph_SaveLoadingConfigData(node->graph,
                                                    instMemNode->logical,
                                                    SHinstrSize, SH_COMMAND_ALIGN_SIZE);
        if (instructionLCDTIndex == 0xFFFFFFFF)
        {
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }

        gcoOS_ZeroMemory(&patchInfo, sizeof(vx_binary_patch_info_s));
        patchInfo.type = VX_BINARY_PATCH_TYPE_STATE;
        patchInfo.sourceType = VX_BINARY_SOURCE_COMMAND;

        ret = vxoBinaryGraph_SearchPattern((gctUINT32 *)stateBuffer,
                                            stateSize/gcmSIZEOF(gctUINT32),
                                            address, offsetArray, vx_false_e);
        vxmASSERT(1 == ret);
        if (ret != 1)
        {
            vxError("fail to search shader instruction address in command buffer\n");
            vxmONERROR(VX_ERROR_INVALID_VALUE);
        }

        patchInfo.offset = offsetArray[0];
#if ENABLE_SAVE_OFFSET_IN_NBG
        patchInfo.originalBaseAddress = 0;
        vxmONERROR(vxoBinaryGraph_ChangeAddressToOffset(stateBuffer, offsetArray[0], address, patchInfo.transformation));
#else
        patchInfo.originalBaseAddress = address;
#endif
        patchInfo.transformation = VX_BINARY_PATCH_TRANSFORMATION_ORIGINAL;
        patchInfo.index = -1; /* meaningless for this type of patch */
        firstPatchIndex = vxoBinaryGraph_SavePatchEntry(node->graph, (void *)&patchInfo);
        patchCount++;
        if (firstPatchIndex == 0xFFFFFFFF)
        {
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }
    }
    else
    {
        vxError("error: shader operation instruction memory can't be null");
        vxmONERROR(VX_ERROR_INVALID_VALUE);
    }

    /*2. save shader's share memory to LCD */
    sharedMemNode = (gcsSURF_NODE_PTR)hints->shaderVidNodes.sharedMemVidMemNode;
    if (VX_NULL != sharedMemNode)
    {
        gctUINT32 address;

        gcmGETHARDWAREADDRESSP(sharedMemNode, address);

        vxmONERROR(vxoBinaryGraph_SaveShaderPatchTable(node, operation, (vx_uint32)address,
                                            (vx_uint8_ptr)sharedMemNode->logical,
                                            (vx_uint32)sharedMemNode->size, (vx_uint32)address,
                                            stateBuffer, stateSize, VXNNE_MEM_POOL_TYPE_ORIG_DDR,
                                            &patchCount, patchedParamPhy, &patchParamNum, 0xFF, 0x00));
    }

    vxmASSERT(!hints->shaderVidNodes.crSpillVidmemNode[gceSGSK_FRAGMENT_SHADER] &&
              !hints->shaderVidNodes.gprSpillVidmemNode[gceSGSK_FRAGMENT_SHADER]);

    /*3. save parameters of shader kernel to patch table */
    for (index = 0; index < shParamNum; index++)
    {
        vx_reference parameter = shParameter[index];

        if (vx_true_e == vxoReference_IsValidAndSpecific(parameter, VX_TYPE_IMAGE))
        {
            gcoVX_Kernel_Context kernelContext; /* not useful, just fulfill the interface */
            gcsVX_IMAGE_INFO imageInfo;
            vx_image     image = (vx_image)parameter;
            vx_uint32    physical = image->memory.physicals[0];
            vx_uint8_ptr logical = image->memory.logicals[0];
            INITIALIZE_STRUCT(imageInfo);

            gcoOS_MemFill((gctPOINTER*)(&kernelContext), 0, sizeof(gcoVX_Kernel_Context));
            gcfVX_GetImageInfo(&kernelContext, (vx_image)parameter, &imageInfo, 1);
            vxmASSERT(image->memory.planeCount == 1);
            if (image->memory.planeCount != 1)
            {
                vxError("just to support planeCount==1 image\n");
                vxmONERROR(VX_FAILURE);
            }

            vxmONERROR(vxoBinaryGraph_SaveShaderPatchTable(node, operation, physical, logical,
                                                (vx_uint32)imageInfo.bytes, physical,
                                                stateBuffer, stateSize, vxoMemory_GetType(&image->memory),
                                                &patchCount, patchedParamPhy, &patchParamNum, index, VX_TYPE_IMAGE));
        }
        else if (vx_true_e == vxoReference_IsValidAndSpecific(parameter, VX_TYPE_TENSOR))
        {
            gcsVX_IMAGE_INFO imageInfo;
            vx_tensor    tensor = (vx_tensor)parameter;
            vx_uint32    physical = TENSOR_PHYSICAL_ADDR(tensor);
            vx_uint8_ptr logical = TENSOR_LOGICAL_ADDR(tensor);
            vx_enum      allocType = vxoMemory_GetType(&tensor->tensorBuffer->memory);
            vx_size      tensorSize = 0;
            vx_uint32    wholeSize = 0;
            gceSTATUS    retStatus = gcvSTATUS_OK;

            vxoTensor_GetTensorWholeSize(tensor, &tensorSize);
            retStatus = gcfVX_GetImageInfoFromTensor(VX_BORDER_UNDEFINED, tensor, batchIndex, &imageInfo);
            if (retStatus != gcvSTATUS_OK)
            {
                vxError("fail to get image infor from tensor..\n");
                vxmONERROR(VX_FAILURE);
            }

            wholeSize = (vx_uint32)tensorSize;

            if (allocType == VXNNE_MEM_POOL_TYPE_ORIG_DDR)
            {
                /* only do alignment in C channel, W/H channel TBD */
                /* the global size of shader should be aligned to local size, such as avgpool shader.The shader local size is 1x1x8,
                  8x8x21 output memory will be aligned to 8x8x24 as output.
                  The output parameter will be patched to LCD when disable virtual buffer
                  the output memory of shader may overflow if allocate 8x8x21 memory in binary loader program.
                  So, we must allocate at least 8x8x24. */
                vx_kernel_execution_parameters_t *shaderParam = VX_NULL;
                vx_uint32 globalWSize = 0;
                if (vxoBinaryGraph_GetMiscDynamicSourceType(operation, physical) == VX_BINARY_SOURCE_MISC_DYNAMIC_OUTPUT)
                {
                    if (operation->operatorType == VXNNE_OPERATOR_USER_VXC)
                    {
                        shaderParam = &operation->layer->node->kernelAttributes.shaderParameter;
                    }
                    else
                    {
                        vxnne_shader_operation  shaderOperation = (vxnne_shader_operation)operation;
                        shaderParam = &shaderOperation->shaderExecutable->shaderParam;
                    }
                    if ((shaderParam->workDim == VX_MAX_WORK_ITEM_DIMENSIONS) &&
                        shaderParam->workDim == TENSOR_ORIG_DIM_NUM(tensor))
                    {
                        globalWSize = (vx_uint32)(shaderParam->globalWorkSize[2]);
                        if (globalWSize <= (vx_uint32)tensor->tensorBuffer->memory.dims[0][2])
                        {
                            globalWSize = gcmALIGN(tensor->tensorBuffer->memory.dims[0][2], (vx_uint32)shaderParam->localWorkSize[2]);
                        }
                        if (globalWSize > (vx_uint32)tensor->tensorBuffer->memory.dims[0][2])
                        {
                            wholeSize = (vx_uint32)tensor->tensorBuffer->memory.dims[0][0] *
                                        (vx_uint32)tensor->tensorBuffer->memory.dims[0][1] *
                                        globalWSize * TENSOR_DATA_SIZE(tensor);
                        }
                    }
                }
            }

            vxmONERROR(vxoBinaryGraph_SaveShaderPatchTable(node, operation, physical, logical,
                                                wholeSize, imageInfo.physicals[0],
                                                stateBuffer, stateSize, allocType,
                                                &patchCount, patchedParamPhy, &patchParamNum, index, VX_TYPE_TENSOR));
        }
        else if (vx_true_e == vxoReference_IsValidAndSpecific(parameter, VX_TYPE_ARRAY))
        {
            vx_array     array = (vx_array)parameter;
            vx_uint32    physical = array->memory.physicals[0];
            vx_uint8_ptr logical = array->memory.logicals[0];
            vx_enum      allocType = vxoMemory_GetType(&array->memory);

            vxmONERROR(vxoBinaryGraph_SaveShaderPatchTable(node, operation, physical, logical,
                                                (vx_uint32)array->memory.sizes[0], physical,
                                                stateBuffer, stateSize, allocType,
                                                &patchCount, patchedParamPhy, &patchParamNum, index, VX_TYPE_ARRAY));
        }
        else if (vx_true_e == vxoReference_IsValidAndSpecific(parameter, VX_TYPE_SCALAR))
        {
            vx_scalar scalar = (vx_scalar)parameter;
            vx_uint32 physical = scalar->physical;
            vx_uint32 scalarSize = vxoScalar_GetTypeSize(scalar);
            vx_uint8_ptr logical = (vx_uint8_ptr)(scalar->value);
            vxmONERROR(vxoBinaryGraph_SaveShaderPatchTable(node, operation, physical, logical,
                                                scalarSize, physical,
                                                stateBuffer, stateSize, VXNNE_MEM_POOL_TYPE_ORIG_DDR,
                                                &patchCount, patchedParamPhy, &patchParamNum, index, VX_TYPE_SCALAR));
        }
        else
        {
            if (operation->operatorType != VXNNE_OPERATOR_USER_VXC)
            {
                /* all parameter of internal shader kernel need to be patched*/
                vxError("error: generate binary not support the source type in shader, type: %d\n",
                          vxoReference_GetType(parameter));
                vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
            }
            else
            {
                vxError("VXC shader, The parameter need to be patched?\n");
            }
        }
    }

    /*4. save uniform args to patch table */
    for (index = 0; index < kernelShader->numArgs; index++)
    {
        vx_argument  arg = &kernelShader->args[index];

        if (arg->uniform == gcvNULL) continue;
        if ((isUniformConstantAddressSpace(arg->uniform)) && (GetUniformPhysical(arg->uniform) != -1))
        {
            vx_mem_alloc_info memAllocInfo = (vx_mem_alloc_info) arg->data;
            vx_uint8_ptr logical = VX_NULL;
            vx_uint32  physical = 0;
            if (memAllocInfo->node != VX_NULL)
            {
                physical = memAllocInfo->physical;
                logical = (vx_uint8_ptr)memAllocInfo->logical;
                vxmASSERT((physical != 0) && (logical != VX_NULL));

                vxmONERROR(vxoBinaryGraph_SaveShaderPatchTable(node, operation, physical, logical,
                                                    memAllocInfo->allocatedSize, physical,
                                                    stateBuffer, stateSize, vx_false_e,
                                                    &patchCount, patchedParamPhy, &patchParamNum, 0xFF, 0x00));
            }
            else
            {
                vxError("constant memory info is NULL in save shader opration function\n");
            }
        }
        else if (isUniformLocalAddressSpace(arg->uniform))
        {
            vxError("shader uniform local address space need to be patched ?\n");
        }
        else if (isUniformPrivateAddressSpace(arg->uniform))
        {
            vxError("shader uniform private address space need to be patched ?\n");
        }
        else if (isUniformStorageBlockAddress(arg->uniform))
        {
            vxError("shader uniform StorageBlock address space need to be patched ?\n");
        }
        else if (isUniformTempRegSpillAddress(arg->uniform))
        {
            vxError("shader uniform TempRegSpill address space need to be patched ?\n");
        }
        else if (isUniformUBOAddress(arg->uniform))
        {
            vxError("shader uniform UBO address space need to be patched ?\n");
        }
    }

    /*5. save SH operation data */
    shOperationInfo.instructionCmdLCDTIndex = instructionLCDTIndex;

    status = vxoBinaryGraph_Write(binarySave, binarySave->currSHOperationOffset,
                                  sizeof(vx_binary_sh_operation_info_s), &shOperationInfo);
    WRITE_NBG_STATUS_CHECK();

    binarySave->currSHOperationOffset += sizeof(vx_binary_sh_operation_info_s);

    /*6. save operation info */
    for (index = 0; index < graph->nodeCount; index++)
    {
        vx_node nodeTmp = graph->nodeTable[graph->allNodeIndexTable[index]];
        if (node == nodeTmp)
        {
            layerId = index;
            break;
        }
    }
    if (index == graph->nodeCount)
    {
        vxError("%s[%d]: fail to get layer ID in save shader operation\n", __FUNCTION__, __LINE__);
    }

    stateLCDTIndex = vxoBinaryGraph_SaveLoadingConfigData(node->graph,
                                                          stateBuffer,
                                                          (vx_uint32)stateSize,
                                                          MEMORY_ALIGN_SIZE);
    if (stateLCDTIndex == 0xFFFFFFFF)
    {
        vxError("%s[%d]: fail to save shader states\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }

    operationInfo.indexOfFirstPatch = firstPatchIndex;
    operationInfo.counterOfPatches = patchCount;
    operationInfo.layerId = layerId;
    operationInfo.operationType = VX_BINARY_OPERATION_TYPE_SH;
    operationInfo.operationIndex = binarySave->currSHOperationIndex++;
    operationInfo.stateLCDTIndex = stateLCDTIndex;

    for (index = 0; index < binarySave->operationCount; index++)
    {
        if ((binarySave->operationCmdPhysical[index] == 0) || \
            (binarySave->currOperationOffset >= binarySave->operationOffset[index]))
        {
            continue;
        }
        else if (gcmPTR_TO_UINT64(operation) == binarySave->operationCmdPhysical[index])
        {
            break;
        }
    }

    vxmASSERT((index >= 0) && (index < binarySave->operationCount));
    if (index >= binarySave->operationCount)
    {
        vxError("error: fail to save shader operation, index: %d, opCount: %d\n",
            index, binarySave->operationCount);
        vxmONERROR(VX_ERROR_INVALID_VALUE);
    }

    status = vxoBinaryGraph_Write(binarySave, binarySave->operationOffset[index],
                                  sizeof(vx_binary_operation_info_s), &operationInfo);
    WRITE_NBG_STATUS_CHECK();

    binarySave->currOperationOffset = binarySave->operationOffset[index];

    /* this is for save END operation */
    if (binarySave->lastOperation0ffset < (binarySave->currOperationOffset + sizeof(vx_binary_operation_info_s)))
    {
        binarySave->lastOperation0ffset = binarySave->currOperationOffset + sizeof(vx_binary_operation_info_s);
    }

    binarySave->savedOperationCount++;

OnError:
    if (stateBuffer != VX_NULL)
    {
        gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)stateBuffer));
        stateBuffer = VX_NULL;
    }

    status = vxoBinaryGraph_SaveErrorHandle(graph, status);

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status vxoBinaryGraph_SaveTPNNOperation(
        vx_node node,
        vx_uint8_ptr cmdLogical,
        vx_uint32 cmdPhysicalAddress,
        vx_uint32 cmdSize,
        vx_binary_operation_target_e cmdType,
        vx_uint8_ptr ksDataLogical,
        vx_uint32  ksDataPhysical,
        vx_uint32 ksDataSize,
        vxnne_tensor_info input,
        vxnne_tensor_info output,
        gctUINT32 inputPhyAddr,
        gctUINT32 outputPhyAddr
        )
{
    vx_uint32 indexOfFirstPatch = 0;
    vx_uint32 currOperationIndex = 0;

    vx_binary_operation_info_s operationInfo;
    vx_binary_save_s *binarySave = node->graph->binarySave;
    vx_binary_nn_operation_info_s nnOperationInfo;
    vx_binary_tp_operation_info_s tpOperationInfo;
    vx_binary_patch_info_s patchInfo;
    vx_graph graph = node->graph;
    vx_uint32 patchCount = 0;
    vx_uint32 patchIndex;
    vx_uint32 offsetArray[100] = {0};
    vx_int32 ret = 0;
    vx_uint32 index = 0, layerId = 0;
    vx_status status = VX_SUCCESS;
    vx_context context = node->base.context;
    vx_uint8 *cmdLogicalAddress = VX_NULL;
    gcmHEADER_ARG("node=%p, cmdLogical=0x%x, cmdPhysicalAddress=0x%x, cmdSize=0x%x, cmdType=0x%x",
        node, cmdLogical, cmdLogical, cmdSize, cmdType);

    if (binarySave == VX_NULL)
    {
        vxError("%s[%d]: binary save is NULL\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_INVALID_VALUE);
    }

    if (VX_NULL == cmdLogical)
    {
        vxError("%s[%d]: cmdLogical is NULL\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_INVALID_VALUE);
    }

    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof(vx_uint8) * cmdSize,
                                        (gctPOINTER*)&cmdLogicalAddress)))
    {
        vxError("fail to allocate memory command buffer\n");
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }
    else
    {
        gcoOS_MemCopy(cmdLogicalAddress, cmdLogical, cmdSize);
    }

    /*1. save nn/tp instruction to patch info*/
    binarySave->patchNNTPCmdPhysical[binarySave->patchNNTPCmdCount] = cmdPhysicalAddress;
    binarySave->patchNNTPCmdOffset[binarySave->patchNNTPCmdCount] = binarySave->currPatchOffset;
    binarySave->patchNNTPCmdCount++;
    gcoOS_ZeroMemory(&patchInfo, sizeof(vx_binary_patch_info_s));
    patchInfo.type = VX_BINARY_PATCH_TYPE_STATE;
    patchInfo.offset = 0; /* re-write later*/
    patchInfo.sourceType = VX_BINARY_SOURCE_COMMAND;
    patchInfo.index = 0;
    patchInfo.transformation = VX_BINARY_PATCH_TRANSFORMATION_RIGHT_SHIFT_6;
#if (ENABLE_SAVE_OFFSET_IN_NBG == 1)
    patchInfo.originalBaseAddress = 0;
#else
    patchInfo.originalBaseAddress = cmdPhysicalAddress;
#endif
    patchIndex = vxoBinaryGraph_SavePatchEntry(node->graph, (void *)&patchInfo);
    indexOfFirstPatch = patchIndex;
    patchCount++;
    if (patchIndex == 0xFFFFFFFF)
    {
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }

    /*2. save input & output to patch info */
    {
        vx_enum inputSourceType;
        vx_enum outputSourceType;
        vx_int32 entryIndexIn = -1;
        vx_int32 entryIndexOut = -1;
        vx_int32 LCDTindex = 0;

        /* 2.1 start to search input */
        entryIndexIn = vxoBinaryGraph_GetIndexOfInputOutputEntry(binarySave->inputPhysicalEntry, input->memoryPhysicalBase);
        entryIndexOut = vxoBinaryGraph_GetIndexOfInputOutputEntry(binarySave->outputPhysicalEntry, input->memoryPhysicalBase);

        if (node->graph->memoryPool && (input->memoryPhysicalBase == node->graph->memoryPool->physical))
        {
            inputSourceType = VX_BINARY_SOURCE_MEMORY_POOL;
        }
        else if (input->memoryPhysicalBase == node->graph->base.context->axiSRAM[node->graph->deviceID].physical)
        {
            inputSourceType = VX_BINARY_SOURCE_AXI_SRAM;
            vxmASSERT(input->sRAM);
        }
        else if (input->memoryPhysicalBase == node->graph->base.context->vipSRAM.physical)
        {
            inputSourceType = VX_BINARY_SOURCE_VIP_SRAM;
            vxmASSERT(input->sRAM);
        }
        else if ((-1 != entryIndexIn) || (-1 != entryIndexOut))
        {
            if (-1 != entryIndexIn)
            {
                inputSourceType = VX_BINARY_SOURCE_INPUT;
                if (cmdType == VX_BINARY_OPERATION_TYPE_NN)
                {
                    vxInfo("target nn, layer %s is input of the binary graph, address: 0x%x\n", node->layer->name, input->memoryPhysicalBase);
                }
                else if (cmdType == VX_BINARY_OPERATION_TYPE_TP)
                {
                    vxInfo("target tp, layer %s is input of the binary graph, address: 0x%x\n", node->layer->name, input->memoryPhysicalBase);
                }
            }
            else
            {
                /* this is for the output of network as a operation input */
                inputSourceType = VX_BINARY_SOURCE_OUTPUT;
                if (cmdType == VX_BINARY_OPERATION_TYPE_NN)
                {
                    vxInfo("target nn, layer %s is input -> grpah output of the binary graph, address: 0x%x\n", node->layer->name, input->memoryPhysicalBase);
                }
                else if (cmdType == VX_BINARY_OPERATION_TYPE_TP)
                {
                    vxInfo("target tp, layer %s is input -> grpah output of the binary graph, address: 0x%x\n", node->layer->name, input->memoryPhysicalBase);
                }
            }
        }
        else
        {
            vx_uint32 lcdSize = 0;
            inputSourceType = VX_BINARY_SOURCE_MISC_DYNAMIC_INPUT;
            vxmASSERT(input->memoryPhysicalBase);
            LCDTindex   = vxoBinaryGraph_GetLCDTIndexForTempTensor(node->graph, input->memoryPhysicalBase, &lcdSize);
            if (-1 == LCDTindex)
            {
                LCDTindex = (vx_int32)vxoBinaryGraph_SaveLoadingConfigData(node->graph, input->memoryLogicalBase,
                                                                           input->memorySize, MEMORY_ALIGN_SIZE);
                binarySave->tempTensorsPhysical[binarySave->numberOfTempTensorInfo].tensorPhysical = input->memoryPhysicalBase;
                binarySave->tempTensorsPhysical[binarySave->numberOfTempTensorInfo].LCDTIndex = LCDTindex;
                binarySave->tempTensorsPhysical[binarySave->numberOfTempTensorInfo].size = input->memorySize;
                binarySave->numberOfTempTensorInfo++;
                if (LCDTindex > 0xFFFE)
                {
                    vxmONERROR(VX_ERROR_NO_MEMORY);
                }
            }
            else
            {
                if (input->memorySize > lcdSize)
                {
                    vxError("error: fail to save input source to lcd, LCDTindex: %d, lcdSize: %d, inputSize: %d\n",
                            LCDTindex, lcdSize, input->memorySize);
                    vxmONERROR(VX_FAILURE);
                }
            }
        }

        /*2.2 save patch input info*/
        gcoOS_ZeroMemory(&patchInfo, sizeof(vx_binary_patch_info_s));
        ret = vxoBinaryGraph_SearchPattern((gctUINT32 *)cmdLogicalAddress,
                                      cmdSize / gcmSIZEOF(gctUINT32),
                                      inputPhyAddr,
                                      offsetArray, vx_true_e);
        if (ret > 1)
        {
            vx_int32 count = 0;
            /* muti inputPhyAddr data in cmdLogicalAddress */
            if (cmdType == VX_BINARY_OPERATION_TYPE_NN)
            {
                for (count = 0; count < ret; count++)
                {
                    if (offsetArray[count] == 24)
                    {
                        offsetArray[0] = 24;/* 24 means input address offset of NN */
                        break;
                    }
                }
                if (count >= ret)
                {
                    vxError("%s[%d]: error search input for NN\n", __FUNCTION__, __LINE__);
                }
            }
            else if (cmdType == VX_BINARY_OPERATION_TYPE_TP)
            {
                for (count = 0; count < ret; count++)
                {
                    if (offsetArray[count] == 40)
                    {
                        offsetArray[0] = 40;/* 40 means input address offset of TP */
                        break;
                    }
                }
                if (count >= ret)
                {
                    vxError("%s[%d]: error search input for TP\n", __FUNCTION__, __LINE__);
                }
            }
        }

        patchInfo.offset              = offsetArray[0];
        patchInfo.type                = VX_BINARY_PATCH_TYPE_COMMAND;
        patchInfo.sourceType          = inputSourceType;
        if (inputSourceType == VX_BINARY_SOURCE_INPUT)
        {
            patchInfo.index = entryIndexIn;
        }
        else if (inputSourceType == VX_BINARY_SOURCE_OUTPUT)
        {
            patchInfo.index = entryIndexOut;
        }
        else if (inputSourceType == VX_BINARY_SOURCE_MISC_DYNAMIC_INPUT)
        {
            patchInfo.index = LCDTindex;
        }
        else
        {
            patchInfo.index = -1;
        }

        patchInfo.originalBaseAddress = (inputSourceType == VX_BINARY_SOURCE_VIP_SRAM) ?
                                        (input->memoryPhysicalBase - VX_VIP_SRAM_IMAGE_STREAM_SIZE) : input->memoryPhysicalBase;
        patchInfo.transformation      = VX_BINARY_PATCH_TRANSFORMATION_ORIGINAL;

        if (VX_BINARY_SOURCE_INPUT == patchInfo.sourceType)
        {
            binarySave->inputInPatchedBinOffset[binarySave->inputInPatchedNum] = binarySave->currPatchOffset;
            binarySave->inputInPatchedPhysical[binarySave->inputInPatchedNum] = input->memoryPhysicalBase;
            binarySave->inputInPatchedIndex[binarySave->inputInPatchedNum] = patchInfo.index;
            binarySave->inputInPatchedNum++;
        }
#if (ENABLE_SAVE_OFFSET_IN_NBG == 1)
        vxmONERROR(vxoBinaryGraph_ChangeAddressToOffset(cmdLogicalAddress, patchInfo.offset,
                                                        patchInfo.originalBaseAddress,
                                                        patchInfo.transformation));
        patchInfo.originalBaseAddress = 0;
#endif
        patchIndex = vxoBinaryGraph_SavePatchEntry(node->graph, (void *)&patchInfo);
        patchCount++;
        if (patchIndex == 0xFFFFFFFF)
        {
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }

        /*2.3 input image circular buffer end address as a patch item*/
        if (((VX_BINARY_SOURCE_AXI_SRAM == inputSourceType) ||
            (VX_BINARY_SOURCE_VIP_SRAM == inputSourceType)) &&
            (input->physical.circularBufEndAddrPlus1 != 0xFFFFFFFF) &&
            (input->physical.circularBufEndAddrPlus1 != 0) &&
            (input->physical.circularBufEndAddrPlus1 != 0xCDCDCDCD))
        {
            gcoOS_ZeroMemory(&patchInfo, sizeof(vx_binary_patch_info_s));
            ret = vxoBinaryGraph_SearchPattern((gctUINT32 *)cmdLogicalAddress,
                                           cmdSize / gcmSIZEOF(gctUINT32),
                                           input->physical.circularBufEndAddrPlus1 >> 6,
                                           offsetArray, vx_true_e);
            if (ret > 1)
            {
                vx_int32 count = 0;
                if (cmdType == VX_BINARY_OPERATION_TYPE_NN)
                {
                    for (count = 0; count < ret; count++)
                    {
                        if (offsetArray[count] == 21 * 4)
                        {
                            offsetArray[0] = 21 * 4;/* 21 * 4 means circularBufEndAddrPlus1 address offset of NN instr */
                            break;
                        }
                    }
                    if (count >= ret)
                    {
                        vxError("%s[%d]: error search input for NN\n", __FUNCTION__, __LINE__);
                    }
                }
                else if (cmdType == VX_BINARY_OPERATION_TYPE_TP)
                {
                    for (count = 0; count < ret; count++)
                    {
                        if (offsetArray[count] == 26 * 4)
                        {
                            offsetArray[0] = 26 * 4;/* 26 * 4 means circularBufEndAddrPlus1 offset of TP instr */
                            break;
                        }
                    }
                    if (count >= ret)
                    {
                        vxError("%s[%d]: error search input for TP\n", __FUNCTION__, __LINE__);
                    }
                }
            }

            patchInfo.offset = offsetArray[0];
            patchInfo.type                = VX_BINARY_PATCH_TYPE_COMMAND;
            patchInfo.sourceType          = inputSourceType;
            patchInfo.index               = -1;
            patchInfo.originalBaseAddress = (inputSourceType == VX_BINARY_SOURCE_VIP_SRAM) ?
                                            (input->memoryPhysicalBase - VX_VIP_SRAM_IMAGE_STREAM_SIZE) : input->memoryPhysicalBase;
            patchInfo.transformation      = VX_BINARY_PATCH_TRANSFORMATION_RIGHT_SHIFT_6;
#if (ENABLE_SAVE_OFFSET_IN_NBG == 1)
            vxmONERROR(vxoBinaryGraph_ChangeAddressToOffset(cmdLogicalAddress, patchInfo.offset,
                                                            patchInfo.originalBaseAddress,
                                                            patchInfo.transformation));
            patchInfo.originalBaseAddress = 0;
#endif
            patchIndex = vxoBinaryGraph_SavePatchEntry(node->graph, (void *)&patchInfo);
            if (patchIndex == 0xFFFFFFFF)
            {
                vxmONERROR(VX_ERROR_NO_MEMORY);
            }
            patchCount++;
            /* debug check point*/
            if (input->circleBufferSize == 0)
            {
                vxError("input inImageCircularBufSize is zero, please check\n");
                vxmONERROR(VX_FAILURE);
            }
        }

        /*2.4 start to search output */
        entryIndexIn = vxoBinaryGraph_GetIndexOfInputOutputEntry(binarySave->inputPhysicalEntry,
                                                          output->memoryPhysicalBase);
        entryIndexOut = vxoBinaryGraph_GetIndexOfInputOutputEntry(binarySave->outputPhysicalEntry,
                                                          output->memoryPhysicalBase);

        if (node->graph->memoryPool && (output->memoryPhysicalBase == node->graph->memoryPool->physical))
        {
            outputSourceType = VX_BINARY_SOURCE_MEMORY_POOL;
        }
        else if (output->memoryPhysicalBase == node->graph->base.context->axiSRAM[node->graph->deviceID].physical)
        {
            outputSourceType = VX_BINARY_SOURCE_AXI_SRAM;
            vxmASSERT(output->sRAM);
        }
        else if (output->memoryPhysicalBase == node->graph->base.context->vipSRAM.physical)
        {
            outputSourceType = VX_BINARY_SOURCE_VIP_SRAM;
            vxmASSERT(output->sRAM);
        }
        else if ((-1 != entryIndexOut) || (-1 != entryIndexIn))
        {
            if (-1 != entryIndexOut)
            {
                outputSourceType = VX_BINARY_SOURCE_OUTPUT;
                if (cmdType == VX_BINARY_OPERATION_TYPE_NN)
                {
                    vxInfo("target nn, layer %s is ouput of the binary graph, address: 0x%x\n", node->layer->name, output->memoryPhysicalBase);
                }
                else if (cmdType == VX_BINARY_OPERATION_TYPE_TP)
                {
                    vxInfo("target tp, layer %s is ouput of the binary graph, address: 0x%x\n", node->layer->name, output->memoryPhysicalBase);
                }
            }
            else
            {   /* maybe, no such special case in network */
                outputSourceType = VX_BINARY_SOURCE_INPUT;
                if (cmdType == VX_BINARY_OPERATION_TYPE_NN)
                {
                    vxInfo("target nn, layer %s is ouput -> graph input of the binary graph, address: 0x%x\n", node->layer->name, output->memoryPhysicalBase);
                }
                else if (cmdType == VX_BINARY_OPERATION_TYPE_TP)
                {
                    vxInfo("target tp, layer %s is ouput -> graph input of the binary graph, address: 0x%x\n", node->layer->name, output->memoryPhysicalBase);
                }
            }
        }
        else
        {
            vx_uint32 lcdSize = 0;
            outputSourceType = VX_BINARY_SOURCE_MISC_DYNAMIC_OUTPUT;
            LCDTindex   = vxoBinaryGraph_GetLCDTIndexForTempTensor(node->graph, output->memoryPhysicalBase, &lcdSize);
            if (-1 == LCDTindex)
            {
                LCDTindex = (vx_int32)vxoBinaryGraph_SaveLoadingConfigData(node->graph, VX_NULL, output->memorySize, MEMORY_ALIGN_SIZE);
                binarySave->tempTensorsPhysical[binarySave->numberOfTempTensorInfo].tensorPhysical = output->memoryPhysicalBase;
                binarySave->tempTensorsPhysical[binarySave->numberOfTempTensorInfo].LCDTIndex = LCDTindex;
                binarySave->tempTensorsPhysical[binarySave->numberOfTempTensorInfo].size = output->memorySize;
                binarySave->numberOfTempTensorInfo++;
                if (LCDTindex > 0xFFFE)
                {
                    vxmONERROR(VX_ERROR_NO_MEMORY);
                }
            }
            else
            {
                if (output->memorySize > lcdSize)
                {
                    vxError("error: fail to save output source to lcd, LCDTindex: %d, lcdSize: %d, outputSize: %d\n",
                            LCDTindex, lcdSize, output->memorySize);
                    vxmONERROR(VX_FAILURE);
                }
            }
        }

        /*2.5 save patch output info */
        gcoOS_ZeroMemory(&patchInfo, sizeof(vx_binary_patch_info_s));
        ret = vxoBinaryGraph_SearchPattern((gctUINT32 *)cmdLogicalAddress,
                                     cmdSize / gcmSIZEOF(gctUINT32),
                                     outputPhyAddr,
                                     offsetArray, vx_true_e);
        if (ret > 1)
        {
            vx_int32 count = 0;
            if (cmdType == VX_BINARY_OPERATION_TYPE_NN)
            {
                for (count = 0; count < ret; count++)
                {
                    if (offsetArray[count] == 7 * 4)
                    {
                        offsetArray[0] = 7 * 4;/* 7 * 4 means output address offset of NN instr*/
                        break;
                    }
                }
                if (count >= ret)
                {
                    vxError("%s[%d]: error search output for NN\n", __FUNCTION__, __LINE__);
                }
            }
            else if (cmdType == VX_BINARY_OPERATION_TYPE_TP)
            {
                for (count = 0; count < ret; count++)
                {
                    if (offsetArray[count] == 13 * 4)
                    {
                        offsetArray[0] = 13 * 4;/* 13 * 4 means output offset of TP instr*/
                        break;
                    }
                }
                if (count >= ret)
                {
                    vxError("%s[%d]: error search output for TP\n", __FUNCTION__, __LINE__);
                }
            }
        }

        patchInfo.offset              = offsetArray[0];
        patchInfo.type                = VX_BINARY_PATCH_TYPE_COMMAND;
        patchInfo.sourceType          = outputSourceType;

        if (outputSourceType == VX_BINARY_SOURCE_OUTPUT)
        {
            patchInfo.index = entryIndexOut;
        }
        else if (outputSourceType == VX_BINARY_SOURCE_INPUT)
        {
            patchInfo.index = entryIndexIn;
        }
        else if (outputSourceType == VX_BINARY_SOURCE_MISC_DYNAMIC_OUTPUT)
        {
            patchInfo.index = LCDTindex;
        }
        else
        {
            patchInfo.index = -1;
        }

        patchInfo.originalBaseAddress = (outputSourceType == VX_BINARY_SOURCE_VIP_SRAM) ?
                                        (output->memoryPhysicalBase - VX_VIP_SRAM_IMAGE_STREAM_SIZE) : output->memoryPhysicalBase;
        patchInfo.transformation      = VX_BINARY_PATCH_TRANSFORMATION_ORIGINAL;
#if (ENABLE_SAVE_OFFSET_IN_NBG == 1)
        vxmONERROR(vxoBinaryGraph_ChangeAddressToOffset(cmdLogicalAddress, patchInfo.offset,
                                                        patchInfo.originalBaseAddress,
                                                        patchInfo.transformation));
        patchInfo.originalBaseAddress = 0;
#endif
        patchIndex = vxoBinaryGraph_SavePatchEntry(node->graph, (void *)&patchInfo);
        patchCount++;
        if (patchIndex == 0xFFFFFFFF)
        {
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }

        /*2.6 output image circular buffer end address as a patch item*/
        if (((VX_BINARY_SOURCE_AXI_SRAM == outputSourceType) ||
            (VX_BINARY_SOURCE_VIP_SRAM == outputSourceType)) &&
            (output->physical.circularBufEndAddrPlus1 != 0xFFFFFFFF) &&
            (output->physical.circularBufEndAddrPlus1 != 0) &&
            (output->physical.circularBufEndAddrPlus1 != 0xCDCDCDCD))
        {
            gcoOS_ZeroMemory(&patchInfo, sizeof(vx_binary_patch_info_s));
            ret = vxoBinaryGraph_SearchPattern((gctUINT32 *)cmdLogicalAddress,
                                         cmdSize / gcmSIZEOF(gctUINT32),
                                         output->physical.circularBufEndAddrPlus1 >> 6,
                                         offsetArray, vx_true_e);
            if (ret > 1)
            {
                vx_int32 count = 0;
                if (cmdType == VX_BINARY_OPERATION_TYPE_NN)
                {
                    for (count = 0; count < ret; count++)
                    {
                        if (offsetArray[count] == 19 * 4)
                        {
                            offsetArray[0] = 19 * 4;/* 19 * 4 means circularBufEndAddrPlus1 address offset of NN instr*/
                            break;
                        }
                    }
                    if (count >= ret)
                    {
                        vxError("%s[%d]: error circularBufEndAddrPlus1 input for NN\n", __FUNCTION__, __LINE__);
                    }
                }
                else if (cmdType == VX_BINARY_OPERATION_TYPE_TP)
                {
                    for (count = 0; count < ret; count++)
                    {
                        if (offsetArray[count] == 28 * 4)
                        {
                            offsetArray[0] = 28 * 4;/* 28 * 4 means circularBufEndAddrPlus1 offset of TP instr*/
                            break;
                        }
                    }
                    if (count >= ret)
                    {
                        vxError("%s[%d]: error search circularBufEndAddrPlus1 for TP\n", __FUNCTION__, __LINE__);
                    }
                }
            }

            patchInfo.offset = offsetArray[0];
            patchInfo.type                = VX_BINARY_PATCH_TYPE_COMMAND;
            patchInfo.sourceType          = outputSourceType;
            patchInfo.index               = -1;
            patchInfo.originalBaseAddress = (outputSourceType == VX_BINARY_SOURCE_VIP_SRAM) ?
                                            (output->memoryPhysicalBase - VX_VIP_SRAM_IMAGE_STREAM_SIZE) : output->memoryPhysicalBase;
            patchInfo.transformation      = VX_BINARY_PATCH_TRANSFORMATION_RIGHT_SHIFT_6;
#if (ENABLE_SAVE_OFFSET_IN_NBG == 1)
            vxmONERROR(vxoBinaryGraph_ChangeAddressToOffset(cmdLogicalAddress, patchInfo.offset,
                                                            patchInfo.originalBaseAddress,
                                                            patchInfo.transformation));
            patchInfo.originalBaseAddress = 0;
#endif
            patchIndex = vxoBinaryGraph_SavePatchEntry(node->graph, (void *)&patchInfo);
            patchCount++;
            if (patchIndex == 0xFFFFFFFF)
            {
                vxmONERROR(VX_ERROR_NO_MEMORY);
            }
            /* debug check point*/
            if (output->circleBufferSize == 0)
            {
                vxError("output outImageCircularBufSize is zero, please check\n");
                vxmONERROR(VX_FAILURE);
            }
        }
    }

    /*3. save kernel stream to LCD and patch infor */
    if ((ksDataLogical != NULL) && (ksDataSize != 0))
    {
        vx_uint32 lcdSize = 0;
        vx_int32 ksLCDTIndex = 0;
        ksLCDTIndex = vxoBinaryGraph_GetLCDTIndexForTempTensor(node->graph, ksDataPhysical, &lcdSize);
        if (-1 == ksLCDTIndex)
        {
            /*if (binarySave->generateNBGToMemory)
            {
                vxInfo("save NBG KS physcial: 0x%08xm size: 0x%08x \n", ksDataPhysical, ksDataSize);
            }*/

            ksLCDTIndex = vxoBinaryGraph_SaveLoadingConfigData(node->graph,
                                                                ksDataLogical,
                                                                ksDataSize,
                                                                MEMORY_ALIGN_SIZE);
            binarySave->tempTensorsPhysical[binarySave->numberOfTempTensorInfo].tensorPhysical = ksDataPhysical;
            binarySave->tempTensorsPhysical[binarySave->numberOfTempTensorInfo].LCDTIndex = (vx_uint32)ksLCDTIndex;
            binarySave->tempTensorsPhysical[binarySave->numberOfTempTensorInfo].size = ksDataSize;
            binarySave->numberOfTempTensorInfo++;
            if (ksLCDTIndex > 0xFFFE)
            {
                vxmONERROR(VX_ERROR_NO_MEMORY);
            }
        }

        gcoOS_ZeroMemory(&patchInfo, sizeof(vx_binary_patch_info_s));
        if (cmdType == VX_BINARY_OPERATION_TYPE_NN)
        {
            ret = vxoBinaryGraph_SearchPattern((gctUINT32 *)cmdLogicalAddress,
                                          cmdSize / gcmSIZEOF(gctUINT32),
                                          ((gctUINT32)ksDataPhysical) >> 6,
                                          offsetArray, vx_true_e);
            if (1 == ret)
            {
                patchInfo.offset = offsetArray[0];
            }
            else
            {
                /* position of kernel stream address in nn instruction */
                vx_uint32 KSPhysicalPos = 5;
                vx_uint32 KSPhysicalAddr = *((vx_uint32*)cmdLogicalAddress + KSPhysicalPos);
                if (KSPhysicalAddr != 0 )
                {
                    patchInfo.offset = KSPhysicalPos * sizeof(vx_uint32);
                }
                else
                {
                    vxError("%s[%d]: KSPhysicalAddr = 0\n", __FUNCTION__, __LINE__);
                    vxmONERROR(VX_FAILURE);
                }
            }
            patchInfo.transformation = VX_BINARY_PATCH_TRANSFORMATION_RIGHT_SHIFT_6;
        }
        else if (cmdType == VX_BINARY_OPERATION_TYPE_TP)
        {
            ret = vxoBinaryGraph_SearchPattern((gctUINT32 *)cmdLogicalAddress,
                                           cmdSize / gcmSIZEOF(gctUINT32),
                                           (gctUINT32)ksDataPhysical,
                                           offsetArray, vx_true_e);
            if (1 == ret)
            {
                patchInfo.offset = offsetArray[0];
            }
            else
            {
                /* can't search KS pyhsical address in instruction if ksDataPhysical is KS's base address,
                   then ksDataPhysical + offset in instrcution(cmdLogicalAddress)*/
                /* position of tp roi's lut address in tp instruction */
                vx_uint32 inTileListPos = 7;
                /* position of other tp(except for roi)'s lut physical address in tp instruction */
                vx_uint32 aluLoadPos = 11;
                vx_uint32 inTileListPhyAddr = *((vx_uint32*)cmdLogicalAddress + inTileListPos);
                vx_uint32 lutPhyAddr = *((vx_uint32*)cmdLogicalAddress + aluLoadPos);
                if (inTileListPhyAddr != 0)
                {
                    patchInfo.offset = inTileListPos * sizeof(vx_uint32);
                }
                else if (lutPhyAddr != 0)
                {
                    patchInfo.offset = aluLoadPos * sizeof(vx_uint32);
                }
                else
                {
                    vxError("%s[%d]: lutPhyAddr = 0\n", __FUNCTION__, __LINE__);
                    vxmONERROR(VX_FAILURE);
                }
            }
            patchInfo.transformation = VX_BINARY_PATCH_TRANSFORMATION_ORIGINAL;
        }
        else
        {
            vxError("%s[%d]: error, not support this type\n", __FUNCTION__, __LINE__);
            vxmONERROR(VX_FAILURE);
        }
        vxmASSERT(patchInfo.offset > 0);

        patchInfo.type = VX_BINARY_PATCH_TYPE_COMMAND;
        patchInfo.sourceType = VX_BINARY_SOURCE_MISC_DYNAMIC_GENERIC;
        patchInfo.index = ksLCDTIndex;
        patchInfo.originalBaseAddress = ksDataPhysical;
#if (ENABLE_SAVE_OFFSET_IN_NBG == 1)
        if (patchInfo.transformation == VX_BINARY_PATCH_TRANSFORMATION_RIGHT_SHIFT_6)
        {
            vxmONERROR(vxoBinaryGraph_ChangeAddressToOffset(cmdLogicalAddress, patchInfo.offset,
                                                            patchInfo.originalBaseAddress,
                                                            11));
        }
        else
        {
            vxmONERROR(vxoBinaryGraph_ChangeAddressToOffset(cmdLogicalAddress, patchInfo.offset,
                                                            patchInfo.originalBaseAddress,
                                                            patchInfo.transformation));
        }

        patchInfo.originalBaseAddress = 0;
#endif

        patchIndex = vxoBinaryGraph_SavePatchEntry(node->graph, (void *)&patchInfo);
        patchCount++;
        if (patchIndex == 0xFFFFFFFF)
        {
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }
    }

    /*4. save nn/tp instruction */
    if (cmdType == VX_BINARY_OPERATION_TYPE_NN)
    {
        if (binarySave->nnOpNum > binarySave->nnOperationCount)
        {
            vxError("%s[%d]: nn count is bigger than %d > %d\n", __FUNCTION__, __LINE__, binarySave->nnOpNum, binarySave->nnOperationCount);
            vxmONERROR(VX_FAILURE);
        }

        binarySave->NNTPDataCmdPhysical[binarySave->NNTPDataCount] = cmdPhysicalAddress;
        binarySave->NNTPDataOffset[binarySave->NNTPDataCount] = binarySave->currNNOperationOffset;
        binarySave->NNTPDataCount++;

        gcoOS_MemCopy(nnOperationInfo.nnCmd, cmdLogicalAddress, cmdSize);

        status = vxoBinaryGraph_Write(binarySave, binarySave->currNNOperationOffset,
                                      sizeof(vx_binary_nn_operation_info_s), &nnOperationInfo);
        WRITE_NBG_STATUS_CHECK();

        binarySave->currNNOperationOffset += sizeof(vx_binary_nn_operation_info_s);
        currOperationIndex = binarySave->currNNOperationIndex;
        binarySave->currNNOperationIndex++;

        binarySave->nnOpNum++;
    }
    else if (cmdType == VX_BINARY_OPERATION_TYPE_TP)
    {
        if (binarySave->tpOpNum > binarySave->tpOperationCount)
        {
            vxError("%s[%d]: tp count is bigger than %d > %d\n", __FUNCTION__, __LINE__, binarySave->tpOpNum, binarySave->tpOperationCount);
            vxmONERROR(VX_FAILURE);
        }

        binarySave->NNTPDataCmdPhysical[binarySave->NNTPDataCount] = cmdPhysicalAddress;
        binarySave->NNTPDataOffset[binarySave->NNTPDataCount] = binarySave->currTPOperationOffset;
        binarySave->NNTPDataCount++;

        gcoOS_MemCopy(&tpOperationInfo.tpCmd, cmdLogicalAddress, cmdSize);

        status = vxoBinaryGraph_Write(binarySave, binarySave->currTPOperationOffset,
                                      sizeof(vx_binary_tp_operation_info_s), &tpOperationInfo);
        WRITE_NBG_STATUS_CHECK();

        binarySave->currTPOperationOffset += sizeof(vx_binary_tp_operation_info_s);
        currOperationIndex = binarySave->currTPOperationIndex;
        binarySave->currTPOperationIndex++;

        binarySave->tpOpNum++;
    }
    else
    {
        vxError("%s[%d]: error, not support this type\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_FAILURE);
    }

    /*5. save operation */
    for (index = 0; index < graph->nodeCount; index++)
    {
        vx_node nodeTmp = graph->nodeTable[graph->allNodeIndexTable[index]];
        if (node == nodeTmp)
        {
            layerId = index;
            break;
        }
    }
    if (index == graph->nodeCount)
    {
        vxError("%s[%d]: fail to get layer ID in save shader operation\n", __FUNCTION__, __LINE__);
    }

    operationInfo.operationType = cmdType;
    operationInfo.operationIndex = currOperationIndex;
    operationInfo.layerId = layerId;
    operationInfo.stateLCDTIndex = 0; /* re-write later*/
    operationInfo.indexOfFirstPatch = indexOfFirstPatch;
    operationInfo.counterOfPatches = patchCount;

    status = vxoBinaryGraph_Write(binarySave, binarySave->currOperationOffset,
                                  sizeof(vx_binary_operation_info_s), &operationInfo);
    WRITE_NBG_STATUS_CHECK();

    binarySave->operationCmdPhysical[binarySave->currOperationIndex] = (vx_uint64)cmdPhysicalAddress;
    binarySave->operationOffset[binarySave->currOperationIndex] = binarySave->currOperationOffset;
    binarySave->currOperationIndex++;
    binarySave->currOperationOffset += sizeof(vx_binary_operation_info_s);

    binarySave->savedOperationCount++;

OnError:
    if (cmdLogicalAddress != VX_NULL)
    {
        gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)cmdLogicalAddress));
        cmdLogicalAddress = VX_NULL;
    }

    status = vxoBinaryGraph_SaveErrorHandle(graph, status);

    gcmFOOTER_ARG("%d", status);

    return status;
}

/* re-write nn/tp command buffer in offset bytes location */
VX_INTERNAL_API vx_status vxoBinaryGraph_ReSaveNNTPCommand(
    vxnne_operation operation,
    vx_uint32 cmdPhysical,
    vx_uint32 offset,
    vx_uint32 value
    )
{
    vx_status status = VX_SUCCESS;
    vx_node node = operation->layer->node;
    vx_binary_save_s *binarySave = node->graph->binarySave;
    vx_uint32 i = 0;
    vx_uint32 index = 0;
    vx_uint32 location = 0;
    gcmHEADER_ARG("operation=%p, offset=0x%x, value=0x%x", operation, offset, value);

    if (!binarySave)
    {
        gcmFOOTER_ARG("%d", status);
        return status;
    }

    for (i = 0; i < binarySave->NNTPDataCount; i++)
    {
        if (binarySave->NNTPDataCmdPhysical[i] == 0)
        {
            continue;
        }
        else if (cmdPhysical == binarySave->NNTPDataCmdPhysical[i])
        {
            index = i;
            break;
        }
    }
    if (index >= binarySave->NNTPDataCount)
    {
        vxError("%s[%d]: can't search this operation in NNTPDataCmdPhysical\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_INVALID_VALUE);
    }

    location = binarySave->NNTPDataOffset[index] + offset;

    status = vxoBinaryGraph_Write(binarySave, location, sizeof(vx_uint32), &value);
    WRITE_NBG_STATUS_CHECK();

OnError:

    status = vxoBinaryGraph_SaveErrorHandle(node->graph, status);

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status vxoBinaryGraph_SaveNNTPStates(
    vx_node node,
    vx_uint32 cmdPhysical,
    gctUINT8 *stateLogical,
    vx_uint32 stateSize
    )
{
    vx_uint32 stateLCDTIndex = 0;
    vx_int32  operationIndex = -1;
    vx_uint32 cmdIndex = 0;
    vx_uint32 cmdOffsetInStates = 0;
    vx_uint32 offset = 0;
    vx_uint32 index = 0;
    vx_status status = VX_SUCCESS;
    vx_uint64 cmdAddr = (vx_uint64)cmdPhysical;
    vx_uint64 opCmdPhy = 0xFFFF;
    vx_binary_save_s *binarySave = node->graph->binarySave;
    vx_uint8 *stateBuffer = VX_NULL;
    gcmHEADER_ARG("node=%p, cmdPhysical=0x%x, stateLogical=%p, stateSize=0x%x", node, cmdPhysical, stateLogical, stateSize);

    if ((stateSize < sizeof(gctUINT32)) || (binarySave == VX_NULL))
    {
        vxError("%s[%d]: statSize < sizeof(int32) or binary save is NULL\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_INVALID_VALUE);
    }

    if (VX_NULL == stateLogical)
    {
        vxError("%s[%d]: stateLogical is NULL\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_INVALID_VALUE);
    }

    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof(vx_uint8) * stateSize,
                                        (gctPOINTER*)&stateBuffer)))
    {
        vxError("fail to allocate memory states buffer\n");
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }
    else
    {
        gcoOS_MemCopy(stateBuffer, stateLogical, stateSize);
    }

    cmdOffsetInStates = vxoBinaryGraph_SearchPatternRightShift6((gctUINT32 *)stateBuffer,
                                                    stateSize / gcmSIZEOF(gctUINT32),
                                                    cmdPhysical);
    if (cmdOffsetInStates < sizeof(gctUINT32))
    {
        vxError("%s[%d]: cmdOffsetInStates < sizeof(int32) or binary save is NULL\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_INVALID_VALUE);
    }

#if (ENABLE_SAVE_OFFSET_IN_NBG == 1)
    vxmONERROR(vxoBinaryGraph_ChangeAddressToOffset(stateBuffer, cmdOffsetInStates, cmdPhysical, 10));
#endif

    /* 1. re-write state lcd index to operation table*/
    stateLCDTIndex = vxoBinaryGraph_SaveLoadingConfigData(node->graph,
                                                          stateBuffer,
                                                          stateSize,
                                                          MEMORY_ALIGN_SIZE);
    if (stateLCDTIndex == 0xFFFFFFFF)
    {
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }

    for (index = 0; index < binarySave->operationCount; index++)
    {
        opCmdPhy = binarySave->operationCmdPhysical[index];
        if (opCmdPhy == 0)
        {
            continue;
        }
        else if (cmdAddr == opCmdPhy)
        {
            operationIndex = index;
            break;
        }
    }
    if (operationIndex < 0)
    {
        vxError("%s[%d]: operationIndex is %d\n", __FUNCTION__, __LINE__, operationIndex);
        vxmONERROR(VX_ERROR_INVALID_VALUE);
    }

    /* 3 means: sizeof(Operation_type) + sizeof(Operation_index) + sizeof(Layer_id) */
    offset = binarySave->operationOffset[operationIndex] + 3 * sizeof(vx_uint32);
    status = vxoBinaryGraph_Write(binarySave, offset, sizeof(vx_uint32), &stateLCDTIndex);
    WRITE_NBG_STATUS_CHECK();

    /* 2. re-write nn/tp's command buffer offset to patch information table */
    for (index = 0; index < binarySave->patchNNTPCmdCount; index++)
    {
        if (node->graph->binarySave->patchNNTPCmdPhysical[index] == 0)
        {
            continue;
        }
        else if (cmdPhysical == binarySave->patchNNTPCmdPhysical[index])
        {
            cmdIndex = index;
            break;
        }
    }
    if (index >= binarySave->patchNNTPCmdCount)
    {
        vxError("%s[%d]: index > patchNNTPCmdCount\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_INVALID_VALUE);
    }

    /* 1 means: sizeof(type) in vx_binary_patch_info_s */
    offset = binarySave->patchNNTPCmdOffset[cmdIndex] + 1 * sizeof(vx_uint32);
    status= vxoBinaryGraph_Write(binarySave, offset, sizeof(vx_uint32), &cmdOffsetInStates);
    WRITE_NBG_STATUS_CHECK();

OnError:
    if (stateBuffer != VX_NULL)
    {
        gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)stateBuffer));
        stateBuffer = VX_NULL;
    }

    status = vxoBinaryGraph_SaveErrorHandle(node->graph, status);

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status vxoBinaryGraph_ReSaveInputAndPatchTable(
    vx_graph graph
    )
{
    vx_binary_save_s *binarySave = graph->binarySave;
    vx_status status = VX_SUCCESS;
    vx_uint32 i = 0, j = 0, k = 0;
    vx_uint32 inputEntryStartOffset = 0;
    vx_uint32 patchOffset = 0;
    vx_uint32 entracnceInputOffset = 0;
    vx_uint32 inputSize = 0;
    vx_uint32 loopNum = 0;
    vx_uint32 inputCount = 0;
    vx_bool reWrite = vx_false_e;
    gcmHEADER_ARG("graph=%p", graph);

    if (binarySave == VX_NULL)
    {
        vxError("%s[%d]: binary save is NULL\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_INVALID_VALUE);
    }

    /* re-write operation count */
     if (binarySave->savedOperationCount < binarySave->operationCount)
     {
         binarySave->headerInfo.operationCount = binarySave->savedOperationCount;
         status= vxoBinaryGraph_Write(binarySave, 0, sizeof(vx_binary_header_s), &binarySave->headerInfo);
         WRITE_NBG_STATUS_CHECK();

         binarySave->entrancesInfo.operationsEntr.operationsBytes = binarySave->savedOperationCount * sizeof(vx_binary_operation_info_s);
     }
     else if (binarySave->savedOperationCount > binarySave->operationCount)
     {
         vxError("%s[%d]: saved operation count: %d, operation count: %d\n",
             __FUNCTION__, __LINE__, binarySave->savedOperationCount, binarySave->operationCount);
         vxmONERROR(VX_ERROR_INVALID_VALUE);
     }

    /* re-write loading data entrance of LCD bytes */
     binarySave->entrancesInfo.loadingConfigDataEntr.loadingConfigDataBytes = binarySave->loadingDataTotalBytes;
     status = vxoBinaryGraph_Write(binarySave, binarySave->entryTablePos, sizeof(vx_binary_entrance_info_s), &binarySave->entrancesInfo);
     WRITE_NBG_STATUS_CHECK();

    /* should re-write input table and patch table if they size don't equal */
    inputEntryStartOffset = sizeof(vx_binary_header_s) +
                            sizeof(vx_binary_memory_pool_info_s) +
                            sizeof(vx_binary_axi_sram_info_s) +
                            sizeof(vx_binary_vip_sram_info_s) +
                            sizeof(vx_binary_entrance_info_s);

    for (i = 0; i < binarySave->inputParamCount; i++)
    {
        if (loopNum >= binarySave->inputParamCount) break;
        for (j = 0; j < binarySave->inputInPatchedNum; j++)
        {
            if (binarySave->inputPhysicalEntry[i] == binarySave->inputInPatchedPhysical[j])
            {
                break;
            }
        }
        if (j >=  binarySave->inputInPatchedNum)
        {
            /* remove the input from input table info */
            for (k = i; k < binarySave->inputParamCount; k++)
            {
                gcoOS_MemCopy((gctPOINTER)(&binarySave->inputInfo[k]), (gctPOINTER)(&binarySave->inputInfo[k + 1]),
                                sizeof(vx_binary_input_output_info_s));
                binarySave->inputPhysicalEntry[k] = binarySave->inputPhysicalEntry[k + 1];
                reWrite = vx_true_e;
            }

            if (i > 0)
            {
                i = i - 1;
            }
        }
        loopNum++;
    }

    /* r-write input table. bypass this re-write if user set inputTable by vxIdentifyGraphInputsAndOutputs() */
    if (reWrite && (graph->inputCount == 0))
    {
        /* re-write whole input entry table */
        status = vxoBinaryGraph_Write(binarySave, inputEntryStartOffset,
                                     sizeof(vx_binary_input_output_info_s) * binarySave->inputParamCount,
                                     (gctPOINTER)(binarySave->inputInfo));
        WRITE_NBG_STATUS_CHECK();

        /* re-write index value for patch teble info*/
        for (i = 0; i < binarySave->inputInPatchedNum; i++)
        {
            for (j = 0; j < binarySave->inputInPatchedNum; j++)
            {
                if (binarySave->inputInPatchedPhysical[i] == binarySave->inputPhysicalEntry[j])
                {
                    if (binarySave->inputInPatchedIndex[i] != j)
                    {
                        /* 3 means: sizeof(type) + sizeof(offset) + sizeof(sourceType) */
                        patchOffset = binarySave->inputInPatchedBinOffset[i] + 3 * sizeof(vx_uint32);
                        status = vxoBinaryGraph_Write(binarySave, patchOffset, sizeof(vx_uint32), (gctPOINTER)(&j));
                        WRITE_NBG_STATUS_CHECK();
                    }
                }
            }
        }

        /* find same input tensor by diffence operations patched*/
        inputCount = binarySave->inputInPatchedNum;
        for (i = 0; i < inputCount - 1; i++)
        {
            if (0 != binarySave->inputInPatchedPhysical[i])
            {
                for (j = 0; j < i; j++)
                {
                    if (binarySave->inputInPatchedPhysical[i] == binarySave->inputInPatchedPhysical[j])
                    {
                        j = inputCount; /* Alread processed, continue with next. */
                        break;
                    }
                }

                if (j == inputCount)
                {
                    continue;
                }

                for (j = i + 1; j < inputCount; j++)
                {
                    if (binarySave->inputInPatchedPhysical[i] == binarySave->inputInPatchedPhysical[j])
                    {
                        binarySave->inputInPatchedNum--;
                    }
                }
            }
        }

        if (binarySave->inputParamCount != binarySave->inputInPatchedNum)
        {
            /* re-write input number of header info*/
            status = vxoBinaryGraph_Write(binarySave, sizeof(vx_binary_header_s) - 2 * sizeof(vx_uint32),
                                         sizeof(vx_uint32), (gctPOINTER)(&binarySave->inputInPatchedNum));
            WRITE_NBG_STATUS_CHECK();

            /* re-write input size of entrance infor */
            entracnceInputOffset = sizeof(vx_binary_header_s) +
                                   sizeof(vx_binary_memory_pool_info_s) +
                                   sizeof(vx_binary_axi_sram_info_s) +
                                   sizeof(vx_binary_vip_sram_info_s) +
                                   sizeof(vx_uint32);
            inputSize = sizeof(vx_binary_input_output_info_s) * binarySave->inputInPatchedNum;
            status= vxoBinaryGraph_Write(binarySave, entracnceInputOffset,
                                         sizeof(vx_uint32), (gctPOINTER)(&inputSize));
            WRITE_NBG_STATUS_CHECK();
        }
    }

OnError:

    status = vxoBinaryGraph_SaveErrorHandle(graph, status);

    /* save binary graph done, destructor all resource */
    vxoBinaryGraph_unInitial(graph);
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status vxoBinaryGraph_StoreOperationPtr(
    vxnne_operation operation
    )
{
    vx_node node = operation->layer->node;
    vx_binary_save binarySave = node->graph->binarySave;

    if (operation->target == VXNNE_OPERATION_TARGET_SC)
    {
        if (node->base.context->options.enableMultiVIPCombined)
        {
            gctUINT32 gpuCount = 1;
            gctUINT32 i = 0;
            gcoVX_QueryCoreCount(node->graph->deviceID, &gpuCount);
            for (i = 0; i < gpuCount; i++)
            {
                binarySave->operationCmdPhysical[binarySave->currOperationIndex] = 0xa5babaed;/* magic data for SC operation */
                binarySave->operationOffset[binarySave->currOperationIndex] = binarySave->currOperationOffset;
                binarySave->currOperationIndex++;
                binarySave->currOperationOffset += sizeof(vx_binary_operation_info_s);
            }
        }
        else
        {
            binarySave->operationCmdPhysical[binarySave->currOperationIndex] = 0xa5babaed;/* magic data for SC operation */
            binarySave->operationOffset[binarySave->currOperationIndex] = binarySave->currOperationOffset;
            binarySave->currOperationIndex++;
            binarySave->currOperationOffset += sizeof(vx_binary_operation_info_s);
        }
    }
    else
    {
        binarySave->operationCmdPhysical[binarySave->currOperationIndex] = gcmPTR_TO_UINT64(operation);
        binarySave->operationOffset[binarySave->currOperationIndex] = binarySave->currOperationOffset;
        binarySave->currOperationIndex++;
        binarySave->currOperationOffset += sizeof(vx_binary_operation_info_s);
    }

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoBinaryGraph_GetGraphInputOutput(
    vx_graph graph
    )
{
    vx_status status = VX_SUCCESS;
    vx_binary_save_s *binarySave = VX_NULL;

    gcmHEADER_ARG("graph=%p", graph);

    if ((0 == graph->base.context->options.enableSaveBinary) ||
        (1 == graph->base.context->options.enableCacheBinaryGraph))
    {   /* bypass this function if in cache binary graph */
        gcmFOOTER_ARG("%d", VX_SUCCESS);
        return VX_SUCCESS;
    }

    if (vx_true_e == vxoBinaryGraph_HasBinaryInGraph(graph))
    {
        /* bypass the network if it import from binary graph */
        gcmFOOTER_ARG("%d", VX_SUCCESS);
        return VX_SUCCESS;
    }

    if (graph->binarySave == VX_NULL)
    {
        vxmONERROR(vxoBinaryGraph_Initialize(graph, VX_NULL));
    }

    binarySave = graph->binarySave;

    if (binarySave == VX_NULL)
    {
        vxError("error: binarySave is NULL in Graph SavebinarySave");
        vxmASSERT(0);
        vxmONERROR(VX_ERROR_INVALID_VALUE);
    }

    vxmONERROR(vxoBinaryGraph_InputsOutputs(graph, binarySave->inputTableRef, &binarySave->inputTableRefCount,
                                            binarySave->outputTableRef, &binarySave->outputTableRefCount));

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status vxoBinaryGraph_UpdateInputOutputPhysicalTable(
    vx_context context,
    vx_uint32 oldPhysical,
    vx_uint32 newPhysical
    )
{
    vx_status status = VX_SUCCESS;
    vx_graph graph = VX_NULL;
    vx_binary_save_s *binarySave = VX_NULL;
    vx_reference_item current;
    vx_uint32 i = 0, j = 0;
    vx_uint32 updated = 0;

    gcmHEADER_ARG("context=%p, oldPhysical=0x%x, newPhysical=0x%x", context, oldPhysical, newPhysical);

    current = context->refListHead;
    while (current != VX_NULL)
    {
        vx_reference ref = current->ref;

        if (ref != VX_NULL)
        {
            if (ref->type == VX_TYPE_GRAPH)
            {
                graph = (vx_graph)ref;
                binarySave = graph->binarySave;
                if (binarySave != VX_NULL)
                {
                    for (i = 0 ; i < graph->inputCount; i++)
                    {
                        if (binarySave->inputPhysicalEntry[i] == oldPhysical)
                        {
                            binarySave->inputPhysicalEntry[i] = newPhysical;
                            for (j = 0; j < graph->outputCount; j++)
                            {
                                if (binarySave->outputPhysicalEntry[j] == newPhysical)
                                {   /* is swap input/output */
                                    binarySave->outputPhysicalEntry[j] = oldPhysical;
                                }
                            }
                            updated = 1;
                        }
                    }

                    if (updated == 0)
                    {
                        for (i = 0; i < graph->outputCount; i++)
                        {
                            if (binarySave->outputPhysicalEntry[i] == oldPhysical)
                            {
                                for (j = 0; j < graph->inputCount; j++)
                                {
                                    if (binarySave->inputPhysicalEntry[j] == newPhysical)
                                    {   /* is swap input/output */
                                        binarySave->inputPhysicalEntry[j] = oldPhysical;
                                    }
                                }
                                binarySave->outputPhysicalEntry[i] = newPhysical;
                            }
                        }
                    }
                }
            }
        }
        current = current->next;
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

/*
save binary graph entrance table.
such as Header, entrance, input data, output data, layers.
*/
VX_INTERNAL_API vx_status vxoBinaryGraph_SaveBinaryEntrance(
    vx_graph graph
    )
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i = 0, j = 0, k = 0;
    vx_int32 numI = 0, numJ = 0;
    vx_uint32 inputCount = 0, outputCount = 0;
    vx_size tensorSize = 0;
    vx_binary_memory_pool_info_s memoryPoolInfo;
    vx_binary_axi_sram_info_s axiSramInfo;
    vx_binary_input_output_info_s *inputInfoArray = VX_NULL;
    vx_binary_input_output_info_s outputInfo;
    vx_binary_save_s *binarySave = graph->binarySave;
    vx_binary_layers_info_s layersInfo;
    vx_uint32 currPos = 0, operationPos = 0;
    vx_context context = graph->base.context;
    vx_uint32 nodeIndex = 0;
    vxnne_execution_layer   executionLayer = (vxnne_execution_layer)graph->layer;
    vx_kernel_execution_parameters_t **shaderParam = VX_NULL;
    vx_uint32 *shOperationID = VX_NULL;
    vx_uint32 shOperationIDCnt = 0;
    vx_uint8  *fillZero = VX_NULL;

    gcmHEADER_ARG("graph=%p", graph);

    if ((0 == graph->base.context->options.enableSaveBinary) &&
        (binarySave == VX_NULL))
    {
        gcmFOOTER_ARG("%d", VX_SUCCESS);
        return VX_SUCCESS;
    }

    if (vx_true_e == vxoBinaryGraph_HasBinaryInGraph(graph))
    {
        /* bypass the network if it import from binary graph */
        gcmFOOTER_ARG("%d", VX_SUCCESS);
        return VX_SUCCESS;
    }

    /* calculate size for online genereate NBG testing */
    if ((0 == graph->binarySave->generateNBGToMemory) &&
        (0 != graph->base.context->options.enableSaveBinary) &&
        (0 == context->options.enableCacheBinaryGraph))
    {
        vxoBinaryGraph_GetNBGSize(graph, VX_NULL);
    }

    graph->binarySave->NBGFileSize = 0;

    /* create NBG file and allocate memory for binarySave */
    if (binarySave == VX_NULL)
    {
        vxmONERROR(vxoBinaryGraph_Initialize(graph, VX_NULL));
        binarySave = graph->binarySave;
        if (binarySave == VX_NULL)
        {
            vxError("error: binarySave is NULL in Graph SavebinarySave");
            vxmASSERT(0);
            vxmONERROR(VX_ERROR_INVALID_VALUE);
        }
    }

    /* allocate memory from heap */
    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof(vx_uint8) * ONCE_FILL_SIZE, (gctPOINTER*)&fillZero)))
    {
        vxError("fail to allocate memory for fill Zero\n");
        vxmASSERT(0);
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }
    gcoOS_MemFill((gctPOINTER*)fillZero, 0, sizeof(vx_uint8) * ONCE_FILL_SIZE);

    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof(vx_uint32) * VX_MAX_OPERATION_COUNT, (gctPOINTER*)&shOperationID)))
    {
        vxError("fail to allocate memory for shader Operation ID\n");
        vxmASSERT(0);
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }
    gcoOS_MemFill((gctPOINTER*)shOperationID, 0, sizeof(vx_uint32) * VX_MAX_OPERATION_COUNT);

    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof(shaderParam) * VX_MAX_OPERATION_COUNT, (gctPOINTER*)&shaderParam)))
    {
        vxError("fail to allocate memory for shader Operation ID\n");
        vxmASSERT(0);
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }
    gcoOS_MemFill((gctPOINTER*)(shaderParam), 0, sizeof(*shaderParam) * VX_MAX_OPERATION_COUNT);

    /* collect inputs of this network */
    for (nodeIndex = 0; nodeIndex < graph->nodeCount; nodeIndex++)
    {
        vx_bool breakFlag = vx_false_e;
        /* graph->nodeTable[nodeIndex]: use user create nodeTable order to collect grah's inputs*/
        vx_node node = graph->nodeTable[nodeIndex];
        vx_uint32 nodeIndex2 = 0, paramIndex2 = 0, paramIndex = 0;
        vx_uint32 i = 0;

        for (paramIndex = 0; paramIndex < node->kernel->signature.paramCount; paramIndex++)
        {
            vx_reference paramRef = node->paramTable[paramIndex];

            breakFlag = vx_false_e;
            if (paramRef == VX_NULL) continue;
            if (node->kernel->signature.directionTable[paramIndex] != VX_INPUT) continue;
            if (node->kernel->signature.isStaticTable[paramIndex] == vx_true_e) continue;
            /* graph input type is vx_tensor, only support this type */
            if (paramRef->type == VX_TYPE_TENSOR)
            {
                if ((VX_TENSOR_LIFE_TIME_DYNAMIC != TENSOR_DATA_LIFETIME((vx_tensor)paramRef)) ||
                   (vxoMemory_GetType(&((vx_tensor)paramRef)->tensorBuffer->memory) == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR) ||
                   (node->kernel->signature.stateTable[paramIndex] == VX_PARAMETER_STATE_OPTIONAL))
                {
                    continue;
                }
            }
            else if (paramRef->type == VX_TYPE_SCALAR)
            {
                if (node->kernel->signature.stateTable[paramIndex] != VX_PARAMETER_STATE_OPTIONAL)
                {
                    /* scalar is graph input*/
                    vx_scalar scalar = (vx_scalar)paramRef;
                    vx_uint32 index = 0;
                    if (0 != graph->base.context->options.enableSaveBinary)
                    {
                        for (i = 0; i < binarySave->inputTableRefCount; i++)
                        {
                            vx_reference inputRef = binarySave->inputTableRef[i];
                            if (paramRef == inputRef)
                            {
                                binarySave->inputEntry[i] = paramRef;
                                index = i;
                                binarySave->inputParamCount++;
                                break;
                            }
                        }
                    }
                    else
                    {   /* cache binary graph */
                        index = binarySave->inputParamCount;
                        binarySave->inputEntry[index] = paramRef;
                        binarySave->inputParamCount++;
                    }

                    if ((i >= binarySave->inputTableRefCount) && (0 == graph->inputCount))
                    {
                        vxError("generate binary graph input not match for scale....\n");
                    }

                    binarySave->inputPhysicalEntry[index] = scalar->physical;

                    /* for updating binary graph input/output table if user set parameter again */
                    for (i = 0; i < binarySave->inputNodeCount; i++)
                    {
                        if (node == binarySave->inputNode[i].node)
                        {
                            break;
                        }
                    }
                    if (i == binarySave->inputNodeCount)
                    {
                        vx_uint32 count = binarySave->inputNode[i].count;
                        binarySave->inputNode[i].node = node;
                        binarySave->inputNode[i].inputPhysical[count] = scalar->physical;
                        binarySave->inputNode[i].paramIndex[count] = paramIndex;
                        binarySave->inputNode[i].count++;
                        binarySave->inputNodeCount++;
                        if (binarySave->inputNodeCount >= VX_MAX_INPUT_NODE_NUM)
                        {
                            vxError("%s[%d]: input node count is bigger than %d > %d\n", __FUNCTION__, __LINE__,
                                     binarySave->inputNodeCount, VX_MAX_INPUT_NODE_NUM);
                            vxmONERROR(VX_FAILURE);
                        }
                    }
                    else
                    {
                        vx_uint32 count = binarySave->inputNode[i].count;
                        binarySave->inputNode[i].inputPhysical[count] = scalar->physical;
                        binarySave->inputNode[i].paramIndex[count] = paramIndex;
                        binarySave->inputNode[i].count++;
                    }
                }
                continue;
            }
            else if (paramRef->type == VX_TYPE_IMAGE)
            {
                vx_image image = (vx_image)paramRef;
                if ((vxoMemory_GetType(&image->memory) == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR) ||
                   (node->kernel->signature.stateTable[paramIndex] == VX_PARAMETER_STATE_OPTIONAL))
                {
                    continue;
                }
            }
            else if (paramRef->type == VX_TYPE_ARRAY)
            {
                vx_array array = (vx_array)paramRef;
                if ((vxoMemory_GetType(&array->memory) == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR) ||
                   (node->kernel->signature.stateTable[paramIndex] == VX_PARAMETER_STATE_OPTIONAL))
                {
                    continue;
                }
            }
            else
            {
               continue;
            }

            for (nodeIndex2 = vxoGraph_GetNextNodeIndex(graph, nodeIndex);
                nodeIndex2 != nodeIndex;
                nodeIndex2 = vxoGraph_GetNextNodeIndex(graph, nodeIndex2))
            {
                vx_node node2 = graph->nodeTable[nodeIndex2];

                for (paramIndex2 = 0; paramIndex2 < node2->kernel->signature.paramCount; paramIndex2++)
                {
                    vx_reference paramRef2 = node2->paramTable[paramIndex2];

                    if (paramRef2 == VX_NULL) continue;

                    if (node2->kernel->signature.directionTable[paramIndex2] == VX_INPUT) continue;

                    if (vxoReference_HasWriteDependency(paramRef, paramRef2))
                    {
                        breakFlag = vx_true_e;
                        break;
                    }
                }
                if (breakFlag) break;
            }

            if (nodeIndex2 == nodeIndex)
            {
                vx_uint32 physical = 0;
                vx_uint32 index = 0;
                /* the input is graph's input */
                if (0 != graph->base.context->options.enableSaveBinary)
                {
                    for (i = 0; i < binarySave->inputTableRefCount; i++)
                    {
                        vx_reference inputRef = binarySave->inputTableRef[i];
                        if (paramRef == inputRef)
                        {
                            binarySave->inputEntry[i] = paramRef;
                            index = i;
                            binarySave->inputParamCount++;
                            break;
                        }
                        else if (paramRef->type == VX_TYPE_TENSOR)
                        {/* this is for batch FC to CONV, output tensor will be reshaped */
                            vx_tensor tensor = (vx_tensor)paramRef;
                            vx_tensor tensorTable = (vx_tensor)inputRef;
                            if (tensor->reshape == tensorTable)
                            {
                                binarySave->inputEntry[i] = paramRef;
                                index = i;
                                binarySave->inputParamCount++;
                                break;
                            }
                        }
                    }
                }
                else
                {
                    /* cache binary graph */
                    index = binarySave->inputParamCount;
                    binarySave->inputEntry[index] = paramRef;
                    binarySave->inputParamCount++;
                }

                if (paramRef->type == VX_TYPE_TENSOR)
                {
                    vx_tensor tenosr = (vx_tensor)paramRef;
                    binarySave->inputPhysicalEntry[index] = TENSOR_PHYSICAL_ADDR(tenosr);
                    physical = TENSOR_PHYSICAL_ADDR(tenosr);
                }
                else if (paramRef->type == VX_TYPE_IMAGE)
                {
                    vx_image image = (vx_image)paramRef;
                    binarySave->inputPhysicalEntry[index] = image->memory.physicals[0];/*Y channel*/
                    physical = image->memory.physicals[0];
                }
                else if (paramRef->type == VX_TYPE_ARRAY)
                {
                    vx_array array = (vx_array)paramRef;
                    binarySave->inputPhysicalEntry[index] = array->memory.physicals[0];
                    physical = array->memory.physicals[0];
                }
                else
                {
                    vxError("error: not support the type as graph input: %d\n", (vx_int32)paramRef->type);
                    vxmONERROR(VX_FAILURE);
                }

                /* for updating binary graph input/output table if user set parameter again */
                if (physical !=0)
                {
                    for (i = 0; i < binarySave->inputNodeCount; i++)
                    {
                        if (node == binarySave->inputNode[i].node)
                        {
                            break;
                        }
                    }
                    if (i == binarySave->inputNodeCount)
                    {
                        vx_uint32 count = binarySave->inputNode[i].count;
                        binarySave->inputNode[i].node = node;
                        binarySave->inputNode[i].inputPhysical[count] = physical;
                        binarySave->inputNode[i].paramIndex[count] = paramIndex;
                        binarySave->inputNode[i].count++;
                        binarySave->inputNodeCount++;
                        if (binarySave->inputNodeCount >= VX_MAX_INPUT_NODE_NUM)
                        {
                            vxError("%s[%d]: input node count is bigger than %d > %d\n", __FUNCTION__, __LINE__,
                                     binarySave->inputNodeCount, VX_MAX_INPUT_NODE_NUM);
                            vxmONERROR(VX_FAILURE);
                        }
                    }
                    else
                    {
                        vx_uint32 count = binarySave->inputNode[i].count;
                        binarySave->inputNode[i].inputPhysical[count] = physical;
                        binarySave->inputNode[i].paramIndex[count] = paramIndex;
                        binarySave->inputNode[i].count++;
                    }
                }
            }
        }
    }

    /* collect outputs of this network */
    for (nodeIndex = 0; nodeIndex < graph->nodeCount; nodeIndex++)
    {
        vx_bool breakFlag = vx_false_e;
        vx_node node = graph->nodeTable[nodeIndex];
        vx_uint32 nodeIndex2 = 0, paramIndex2 = 0, paramIndex = 0;

        for (paramIndex = 0; paramIndex < node->kernel->signature.paramCount; paramIndex++)
        {
            vx_reference paramRef = node->paramTable[paramIndex];

            breakFlag = vx_false_e;
            if (paramRef == VX_NULL) continue;
            if (node->kernel->signature.directionTable[paramIndex] != VX_OUTPUT) continue;
            if (node->kernel->signature.isStaticTable[paramIndex] == vx_true_e) continue;

            if (paramRef->type == VX_TYPE_TENSOR)
            {
                if ((VX_TENSOR_LIFE_TIME_DYNAMIC != TENSOR_DATA_LIFETIME((vx_tensor)paramRef)) ||
                    (vxoMemory_GetType(&((vx_tensor)paramRef)->tensorBuffer->memory) == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR) ||
                    (node->kernel->signature.stateTable[paramIndex] == VX_PARAMETER_STATE_OPTIONAL))
                {
                    continue;
                }
            }
            else if (paramRef->type == VX_TYPE_IMAGE)
            {
                vx_image image = (vx_image)paramRef;
                if ((vxoMemory_GetType(&image->memory) == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR) ||
                   (node->kernel->signature.stateTable[paramIndex] == VX_PARAMETER_STATE_OPTIONAL))
                {
                    continue;
                }
            }
            else if (paramRef->type == VX_TYPE_ARRAY)
            {
                vx_array array = (vx_array)paramRef;
                if ((vxoMemory_GetType(&array->memory) == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR) ||
                   (node->kernel->signature.stateTable[paramIndex] == VX_PARAMETER_STATE_OPTIONAL))
                {
                    continue;
                }
            }
            else if (paramRef->type == VX_TYPE_SCALAR)
            {
                if (node->kernel->signature.stateTable[paramIndex] != VX_PARAMETER_STATE_OPTIONAL)
                {
                    /* scalar is graph input*/
                    vx_scalar scalar = (vx_scalar)paramRef;
                    vx_uint32 index = 0;
                    if (0 != graph->base.context->options.enableSaveBinary)
                    {
                        for (i = 0; i < binarySave->outputTableRefCount; i++)
                        {
                            vx_reference outputRef = binarySave->outputTableRef[i];
                            if (paramRef == outputRef)
                            {
                                binarySave->outputEntry[i] = paramRef;
                                index = i;
                                binarySave->outputParamCount++;
                                break;
                            }
                        }
                    }
                    else
                    {   /* cache binary graph */
                        index = binarySave->outputParamCount;
                        binarySave->outputEntry[index] = paramRef;
                        binarySave->outputParamCount++;
                    }

                    if ((i >= binarySave->outputTableRefCount) && (0 == graph->outputCount))
                    {
                        vxError("generate binary graph output not match for scale....\n");
                    }

                    binarySave->outputPhysicalEntry[index] = scalar->physical;
                }
                continue;
            }
            else
            {
               continue;
            }

            for (nodeIndex2 = vxoGraph_GetNextNodeIndex(graph, nodeIndex);
                nodeIndex2 != nodeIndex;
                nodeIndex2 = vxoGraph_GetNextNodeIndex(graph, nodeIndex2))
            {
                vx_node node2 = graph->nodeTable[nodeIndex2];

                for (paramIndex2 = 0; paramIndex2 < node2->kernel->signature.paramCount; paramIndex2++)
                {
                    vx_reference paramRef2 = node2->paramTable[paramIndex2];

                    if (paramRef2 == VX_NULL) continue;
                    if (node2->kernel->signature.directionTable[paramIndex2] == VX_OUTPUT) continue;
                    if (vxoReference_HasWriteDependency(paramRef, paramRef2))
                    {
                        breakFlag = vx_true_e;
                        break;
                    }
                }
                if (breakFlag) break;
            }
            if (nodeIndex2 == nodeIndex)
            {
                vx_uint32 index = 0;
                if (0 != graph->base.context->options.enableSaveBinary)
                {
                    for (i = 0; i < binarySave->outputTableRefCount; i++)
                    {
                        /* sort output table, as chang node table index by graph transfer feature */
                        vx_reference outputRef = binarySave->outputTableRef[i];
                        if (paramRef == outputRef)
                        {
                            binarySave->outputEntry[i] = paramRef;
                            index = i;
                            binarySave->outputParamCount++;
                            break;
                        }
                        else if (paramRef->type == VX_TYPE_TENSOR)
                        {   /* this is for batch FC to CONV, output tensor will be reshaped */
                            vx_tensor tensor = (vx_tensor)paramRef;
                            vx_tensor tensorTable = (vx_tensor)outputRef;
                            if (tensor->reshape == tensorTable)
                            {
                                binarySave->outputEntry[i] = paramRef;
                                index = i;
                                binarySave->outputParamCount++;
                                break;
                            }
                        }
                    }

                    if ((i >= binarySave->outputTableRefCount) && (0 == graph->outputCount))
                    {
                        vxError("generate binary graph output not match....\n");
                    }
                }
                else
                {
                    /* cache binary graph */
                    index = binarySave->outputParamCount;
                    binarySave->outputEntry[index] = paramRef;
                    binarySave->outputParamCount++;
                }

                if (paramRef->type == VX_TYPE_TENSOR)
                {
                    vx_tensor tensor = (vx_tensor)paramRef;
                    binarySave->outputPhysicalEntry[index] = TENSOR_PHYSICAL_ADDR(tensor);
                }
                else if (paramRef->type == VX_TYPE_IMAGE)
                {
                    vx_image image = (vx_image)paramRef;
                    binarySave->outputPhysicalEntry[index] = image->memory.physicals[0];
                }
                else if (paramRef->type == VX_TYPE_ARRAY)
                {
                    vx_array array = (vx_array)paramRef;
                    binarySave->outputPhysicalEntry[index] = array->memory.physicals[0];
                }
                else
                {
                    vxError("error: not support the type as graph output: %d\n", (vx_int32)paramRef->type);
                    vxmONERROR(VX_FAILURE);
                }
            }
        }
    }

    /* find out same input source/tensor */
    inputCount = binarySave->inputParamCount;
    for (i = 0; i < inputCount - 1; i++)
    {
        if (graph->inputCount && graph->inputs)
        {
            break;
        }
        if (0 != binarySave->inputPhysicalEntry[i])
        {
            for (j = 0; j < i; j++)
            {
                if (binarySave->inputPhysicalEntry[i] == binarySave->inputPhysicalEntry[j])
                {
                    j = inputCount; /* Alread processed, continue with next. */
                    break;
                }
            }

            if (j == inputCount)
            {
                continue;
            }

            for (j = i + 1; j < inputCount; j++)
            {
                if (binarySave->inputPhysicalEntry[i] == binarySave->inputPhysicalEntry[j])
                {
                    binarySave->inputParamCount--;
                }
            }
        }
    }

    /* find out same output source/tensor */
    outputCount = binarySave->outputParamCount;
    for (i = 0; i < outputCount - 1; i++)
    {
        if (graph->outputCount && graph->outputs)
        {
            break;
        }
        if (0 != binarySave->outputPhysicalEntry[i])
        {
            for (j = 0; j < i; j++)
            {
                if (binarySave->outputPhysicalEntry[i] == binarySave->outputPhysicalEntry[j])
                {
                    j = outputCount;    /* Alread processed, continue with next. */
                    break;
                }
            }

            if (j == outputCount)
            {
                continue;
            }

            for (j = i + 1; j < outputCount; j++)
            {
                if (binarySave->outputPhysicalEntry[i] == binarySave->outputPhysicalEntry[j])
                {
                    binarySave->outputParamCount--;
                }
            }
        }
    }

    /* collect all patch count, op count, lcdt count from operations */
    for (i = 0; i < executionLayer->opIndicesNum; i++)
    {
        vxnne_operation_command operationCommand = VX_NULL;
        vx_op_param parameter = VX_NULL;
        vxnne_operation operation = VX_NULL;
        vx_tensor input = VX_NULL;
        vx_tensor output = VX_NULL;
        vx_enum operationTarget = VX_BINARY_OPERATION_TYPE_NONE;

        operationCommand = &executionLayer->opIndices[i];
        parameter = &operationCommand->parameter;
        operation = executionLayer->operations[executionLayer->opIndices[i].operationID];
        operationTarget = vxoBinaryGraph_ConvertToBinaryOperationTarget(operation->target);

        if ((operationTarget == VX_BINARY_OPERATION_TYPE_NN) || (operationTarget == VX_BINARY_OPERATION_TYPE_TP))
        {
            if (operationTarget == VX_BINARY_OPERATION_TYPE_NN)
            {
                vxnne_convolution_relu_pooling_operation nnConvOperation = (vxnne_convolution_relu_pooling_operation)operation;
                vx_uint32 commandCount = 1;
                commandCount = vxoBinaryGraph_CalculateNNSliceCount(context, operationCommand);
                if (commandCount < 1)
                {
                    vxError("%s[%d]: fail to calculate nn slice count, count: %d\n",
                        __FUNCTION__, __LINE__, commandCount);
                    vxmONERROR(VX_ERROR_INVALID_VALUE);
                }

                /* lcd: hw state, ks */
                binarySave->loadingDataCount += 2 * commandCount;

                binarySave->nnOperationCount += commandCount;

                /* patch hw instr, input, output, ks */
                binarySave->patchCount += 4 * commandCount;

                /* patch image circular buffer end address */
                if ((operationCommand->inputTile.circularBufEndAddrPlus1 != 0xFFFFFFFF) &&
                    (operationCommand->inputTile.circularBufEndAddrPlus1 != 0) &&
                    (operationCommand->inputTile.circularBufEndAddrPlus1 != 0xCDCDCDCD))
                {
                    binarySave->patchCount += commandCount;
                }
                if ((operationCommand->outputTile.circularBufEndAddrPlus1 != 0xFFFFFFFF) &&
                    (operationCommand->outputTile.circularBufEndAddrPlus1 != 0) &&
                    (operationCommand->outputTile.circularBufEndAddrPlus1 != 0xCDCDCDCD))
                {
                    binarySave->patchCount += commandCount;
                }

                input = nnConvOperation->inputs;
                output = nnConvOperation->outputs;
            }
            else
            {
                vxnne_tp_operation tpOperation = (vxnne_tp_operation)operation;
                vx_enum tpType = parameter->tpType;
                vx_uint32 core = tpType != TP_SINGLE_FC ? context->nnConfig.fixedFeature.tpCoreCount :
                                                          context->nnConfig.fixedFeature.tpCoreCount + context->nnConfig.fixedFeature.tpliteCoreCount;
                vx_uint32 opNum = core;
                vx_bool mult = context->options.enableMultiTP && core > 1;

                if (opNum < context->nnConfig.fixedFeature.tpCoreCount + context->nnConfig.fixedFeature.tpliteCoreCount)
                {
                    opNum = context->nnConfig.fixedFeature.tpCoreCount + context->nnConfig.fixedFeature.tpliteCoreCount;
                }

                if (operation->parameter.tpType == TP_SINGLE_FC)
                {
                    vx_tensor otherRef = (vx_tensor)operation->parameter.other_ref;
                    vx_tp_value_cmd value = operation->parameter.tp_value;
                    if (!value->e32[0])
                    {
                        vxmASSERT(otherRef != VX_NULL);
                        opNum = WB_TOTAL_SLICE_NUM((vx_weights_biases_parameter)otherRef);
                    }
                }
                else if (operation->parameter.tpType == TP_TRANSPOSE)
                {
                    vx_tensor otherRef = (vx_tensor)operation->parameter.other_ref;
                    vx_tp_value_cmd value = operation->parameter.tp_value;
                    vx_uint32 i, x, y, dnum = value->u32[0], tsize = 1;
                    vx_uint32 dims[VX_CONTEXT_TENSOR_MAX_DIMENSION], strides[VX_CONTEXT_TENSOR_MAX_DIMENSION];

                    vxmASSERT(otherRef != VX_NULL);
                    vxoTensor_GetTensorDimStride(otherRef, &dnum, dims, strides);
                    vxmASSERT(dims[0] < TP_MAX_XYSIZE && dims[1] < TP_MAX_XYSIZE);
                    for (i = 0; i < TENSOR_DIM_NUM(otherRef); i++)
                    {
                        tsize *= dims[i];
                    }
                    x = dims[0];
                    y = tsize / dims[0];
                    for (i = 1; i < dnum; i++)
                    {
                        if (x >= TP_MAX_XYSIZE || y < TP_MAX_XYSIZE)
                        {
                            break;
                        }
                        else
                        {
                            x *= dims[i];
                            y /= dims[i];
                        }
                    }
                    if (x < TP_MAX_XYSIZE && y < TP_MAX_XYSIZE)
                    {
                        opNum = 1;
                    }
                    else
                    {
                        for (i = dnum - 1; (vx_int32)i >= 0; i--)
                        {
                            if (dims[i] > 1) break;
                        }

                        /* TP X/Y size has max size limitation, must split */
                        y = tsize / dims[0];
                        opNum = gcmALIGN_NP2(y, TP_MAX_XYSIZE-1) / (TP_MAX_XYSIZE-1);
                    }
                    if (opNum == 1 && (dnum == 3 || (dnum == 4 && dims[3] == 1)))
                    {
                        vx_uint32_ptr perm = (vx_uint32*)value->p8[0];
                        if (perm[0] == 1 && perm[1] == 0) /* y, x, z */
                        {
                            opNum = x * y > 128 && mult ? gcmMIN(dims[2], core) : 1;
                        }
                        else if (perm[0] == 1 && perm[1] == 2) /* y, z, x */ /* use single TP to reduce bandwidth */
                        {
                            opNum = context->hwChipInfo.customerID == 0xAE && x * y > 128 && mult ? gcmMIN(dims[2], core) : 1;
                        }
                        else if (context->hwChipInfo.customerID == 0xAE)
                        {
                            if (perm[0] == 2 && perm[1] == 0) /* z, x, y */
                            {
                                opNum = mult ? gcmMIN(dims[1], core) : 1;
                            }
                            else if (perm[0] == 2 && perm[1] == 1) /* z, y, x */
                            {
                                opNum = mult ? gcmMIN(dims[1], core) : 1;
                            }
                            else if (perm[0] == 0 && perm[1] == 2) /* x, z, y */
                            {
                                opNum = mult ? gcmMIN(dims[1], core) : 1;
                            }
                        }
                    }
                }

                if (opNum < core)
                {
                    opNum = core;
                }

                binarySave->tpOperationCount += opNum;

                /* add state patch count 3: instr, input, output */
                binarySave->patchCount += 3 * opNum;

                /* lcd: hw state */
                binarySave->loadingDataCount += opNum;

                /* Increase the loading data counter for LUT or weights_bias. */
                if (tpOperation->base.parameter.data_buff != VX_NULL)
                {
                    /* lcd: lut */
                    binarySave->loadingDataCount += opNum;
                    binarySave->patchCount += opNum;
                }

                if ((tpOperation->base.operatorType != VXNNE_OPERATOR_RESHUFFLE) && tpOperation->weights_biases)
                {
                    /* lcd: ks */
                    binarySave->loadingDataCount += opNum;
                    binarySave->patchCount += opNum;
                }

                /* patch image circular buffer end address */
                if ((operationCommand->inputTile.circularBufEndAddrPlus1 != 0xFFFFFFFF) &&
                    (operationCommand->inputTile.circularBufEndAddrPlus1 != 0) &&
                    (operationCommand->inputTile.circularBufEndAddrPlus1 != 0xCDCDCDCD))
                {
                    binarySave->patchCount += opNum;
                }
                if ((operationCommand->outputTile.circularBufEndAddrPlus1 != 0xFFFFFFFF) &&
                    (operationCommand->outputTile.circularBufEndAddrPlus1 != 0) &&
                    (operationCommand->outputTile.circularBufEndAddrPlus1 != 0xCDCDCDCD))
                {
                    binarySave->patchCount += opNum;
                }

                input = tpOperation->input;
                output = tpOperation->output;
            }

            vxoTensor_GetTensorWholeSize(input, &tensorSize);
            operationCommand->inputTile.memorySize = (vx_uint32)tensorSize;

            if (vxoMemory_GetType(&input->tensorBuffer->memory) & VXNNE_MEM_POOL_TYPE_AXI_SRAM)
            {
                operationCommand->inputTile.memoryPhysicalBase = context->axiSRAM[graph->deviceID].physical;
            }
            else if (vxoMemory_GetType(&input->tensorBuffer->memory) & VXNNE_MEM_POOL_TYPE_VIP_SRAM)
            {
                operationCommand->inputTile.memoryPhysicalBase = context->vipSRAM.physical;
            }
            else if (vxoMemory_GetType(&input->tensorBuffer->memory) == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR)
            {
                operationCommand->inputTile.memoryPhysicalBase = graph->memoryPool->physical;
            }
            else if (vxoMemory_GetType(&input->tensorBuffer->memory) == VXNNE_MEM_POOL_TYPE_ORIG_DDR)
            {
                vx_int32 entryIndex = 0;
                vx_uint32 globalWSize = 0;

                operationCommand->inputTile.memoryPhysicalBase = TENSOR_PHYSICAL_ADDR(input);
                entryIndex = vxoBinaryGraph_GetIndexOfInputOutputEntry(binarySave->inputPhysicalEntry,
                                                        operationCommand->inputTile.memoryPhysicalBase);

                /* when disable virtual buffer, all internel tensor are not from memory pool, not in inputoutputEntry.
                   if they can't be in SRAM, put it into LCD */
                if (-1 == entryIndex)
                {
                    vxnne_operation shOperation = VX_NULL;
                    vx_tensor shOutput = VX_NULL;

                    operationCommand->inputTile.memoryLogicalBase = TENSOR_LOGICAL_ADDR(input);
                    binarySave->loadingDataCount += 1;
                    binarySave->patchCount += 1;

                    /* check the input whether output by shader */
                    for (j = 0; j < shOperationIDCnt; j++)
                    {
                        shOperation = executionLayer->operations[shOperationID[j]];
                        for (k = 0; k < shOperation->outputsNum; k++)
                        {
                            if (shOperation->outputs[k]->type != VX_TYPE_TENSOR) continue;
                            shOutput = (vx_tensor)shOperation->outputs[k];
                            if (TENSOR_PHYSICAL_ADDR(input) == TENSOR_PHYSICAL_ADDR(shOutput))
                            {
                                /* only do alignment in C channel, W/H channel TBD */
                                /* the global size of shader should be aligned to local size,
                                  such as avgpool shader.The shader local size is 1x1x8,
                                  8x8x21 output memory will be aligned to 8x8x24 as output.
                                  The output parameter will be patched to LCD when disable virtual buffer
                                  the output memory of shader may overflow if allocate 8x8x21 memory in binary loader program.
                                  So, we must allocate at least 8x8x24. */
                                if ((shaderParam[j]->workDim == VX_MAX_WORK_ITEM_DIMENSIONS) &&
                                    shaderParam[j]->workDim == TENSOR_ORIG_DIM_NUM(input))
                                {
                                    globalWSize = (vx_uint32)(shaderParam[j]->globalWorkSize[2]);
                                    if (globalWSize <= (vx_uint32)input->tensorBuffer->memory.dims[0][2])
                                    {
                                        globalWSize = gcmALIGN(input->tensorBuffer->memory.dims[0][2],
                                                            (vx_uint32)shaderParam[j]->localWorkSize[2]);
                                    }
                                    if (globalWSize > (vx_uint32)input->tensorBuffer->memory.dims[0][2])
                                    {
                                        operationCommand->inputTile.memorySize = (vx_uint32)input->tensorBuffer->memory.dims[0][0] *
                                                                         (vx_uint32)input->tensorBuffer->memory.dims[0][1] *
                                                                         globalWSize * TENSOR_DATA_SIZE(input);
                                    }
                                    j = shOperationIDCnt;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                vxError("%s[%d]: not support this input type\n", __FUNCTION__, __LINE__);
                vxmASSERT(0);
                vxmONERROR(VX_ERROR_NOT_SUPPORTED);
            }

            /* get whole tensor size for batch mode */
            tensorSize = 0;
            vxoTensor_GetTensorWholeSize(output, &tensorSize);
            operationCommand->outputTile.memorySize = (vx_uint32)tensorSize;

            if (vxoMemory_GetType(&output->tensorBuffer->memory) & VXNNE_MEM_POOL_TYPE_AXI_SRAM)
            {
                operationCommand->outputTile.memoryPhysicalBase = context->axiSRAM[graph->deviceID].physical;
            }
            else if (vxoMemory_GetType(&output->tensorBuffer->memory) & VXNNE_MEM_POOL_TYPE_VIP_SRAM)
            {
                operationCommand->outputTile.memoryPhysicalBase = context->vipSRAM.physical;
            }
            else if (vxoMemory_GetType(&output->tensorBuffer->memory) == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR)
            {
                operationCommand->outputTile.memoryPhysicalBase = graph->memoryPool->physical;
            }
            else if (vxoMemory_GetType(&output->tensorBuffer->memory) == VXNNE_MEM_POOL_TYPE_ORIG_DDR)
            {
                vx_int32 entryIndex = 0;

                operationCommand->outputTile.memoryPhysicalBase = TENSOR_PHYSICAL_ADDR(output);
                entryIndex = vxoBinaryGraph_GetIndexOfInputOutputEntry(binarySave->outputPhysicalEntry,
                                                        operationCommand->outputTile.memoryPhysicalBase);

                /* when disable virtual buffer, all internel tensor are not from memory pool, not in inputoutputEntry.
                   if they can't be in SRAM, put it into LCD */
                if (-1 == entryIndex)
                {
                    binarySave->loadingDataCount += 1;
                    binarySave->patchCount += 1;
                    operationCommand->outputTile.memoryLogicalBase = TENSOR_LOGICAL_ADDR(output);
                }
            }
            else
            {
                vxError("%s[%d]: not support this output type\n", __FUNCTION__, __LINE__);
                vxmASSERT(0);
                vxmONERROR(VX_ERROR_NOT_SUPPORTED);
            }
        }
        else if (operationTarget == VX_BINARY_OPERATION_TYPE_SH)
        {
            binarySave->shOperationCount++;

            /* all parameters of shader executable.
             * In fact, we should NOT count real input/output and non-tensor paramters of a shader executable,
             * as input/output are not in LCDT and non-tensor parameters(all scalar today) are programmed to
             * constant registers. However, to reduce the effort to acurately collect each parameter's attribute
             * we just include all parameters here.
             */
            if (operation->operatorType == VXNNE_OPERATOR_USER_VXC)
            {
                vx_node   node = operation->layer->node;
                gctUINT   currentShaderID = 0;
                vx_shader kernelShader = VX_NULL;
                gcsHINT_PTR hints = VX_NULL;
                gcsSURF_NODE_PTR sharedMemNode = VX_NULL;

                vxmONERROR(vxoProgramKernel_GetCurrentShaderID(node, &currentShaderID));
                kernelShader = node->kernel->kernelShader[currentShaderID];
                hints = kernelShader->states.programState.hints;
                /* the share memory of shader need to be patched? */
                sharedMemNode = (gcsSURF_NODE_PTR)hints->shaderVidNodes.sharedMemVidMemNode;
                if (VX_NULL != sharedMemNode)
                {
                    binarySave->loadingDataCount ++;
                    binarySave->patchCount ++;
                }
                /* 2 is HW state + instruction memory */
                binarySave->loadingDataCount += 2 + node->kernel->signature.paramCount;
                /* 1 is patch intruction memory in HW stats */
                /* all parameters, some of non-tensor parameter will not be stored */
                binarySave->patchCount += 1 + node->kernel->signature.paramCount;
            }
            else
            {
                vxnne_shader_operation  shaderOperation = (vxnne_shader_operation)operation;
                gcsHINT_PTR hints = shaderOperation->shaderExecutable->kernelShader->states.programState.hints;
                /* the share memory of shader need to be patched? */
                gcsSURF_NODE_PTR sharedMemNode = (gcsSURF_NODE_PTR)hints->shaderVidNodes.sharedMemVidMemNode;
                if (VX_NULL != sharedMemNode)
                {
                    binarySave->loadingDataCount ++;
                    binarySave->patchCount ++;
                }

                binarySave->loadingDataCount += 2 + shaderOperation->shaderExecutable->paramNum;
                /* 1 is patch intruction memory in HW stats */
                binarySave->patchCount += 1 + shaderOperation->shaderExecutable->paramNum;
                vxmASSERT(operation->operatorType != VXNNE_OPERATOR_USER_CPU);
            }
            /* get all shader operations id for patching TP/NN input parameters,
            their input parameter need to align to shader's localsize[2] if this parameter output by shader*/
            for (k = 0; k < shOperationIDCnt; k++)
            {
                if (executionLayer->opIndices[i].operationID == shOperationID[k])
                    break;
            }
            if (k >= shOperationIDCnt)
            {
                shOperationID[shOperationIDCnt] = executionLayer->opIndices[i].operationID;
                if (operation->operatorType == VXNNE_OPERATOR_USER_VXC)
                {
                    shaderParam[shOperationIDCnt] = &operation->layer->node->kernelAttributes.shaderParameter;
                }
                else
                {
                    vxnne_shader_operation  shaderOperation = (vxnne_shader_operation)operation;
                    shaderParam[shOperationIDCnt] = &shaderOperation->shaderExecutable->shaderParam;
                }
                shOperationIDCnt++;
            }
        }
        else if (operationTarget == VX_BINARY_OPERATION_TYPE_SC)
        {
            vxnne_yuv2rgb_scale_operation scaleOperation = (vxnne_yuv2rgb_scale_operation)operation;
            vx_image image = scaleOperation->inputs;
            vx_tensor output = scaleOperation->outputs;
            gctUINT32 gpuCount = 1;

            if (graph->base.context->options.enableMultiVIPCombined)
            {
                gcoVX_QueryCoreCount(graph->deviceID, &gpuCount);
            }

            binarySave->scOperationCount += (vx_uint32)gpuCount;

            /* lcd: hw states */
            binarySave->loadingDataCount += (vx_uint32)gpuCount;

            /* patch input(vx_image), output(r g b is 3 channel) */
            binarySave->patchCount += (image->planeCount + 3) * (vx_uint32)gpuCount;

            if (vxoMemory_GetType(&output->tensorBuffer->memory) == VXNNE_MEM_POOL_TYPE_ORIG_DDR)
            {
                vx_int32 entryIndex;
                entryIndex = vxoBinaryGraph_GetIndexOfInputOutputEntry(binarySave->outputPhysicalEntry,
                                                                        TENSOR_PHYSICAL_ADDR(output));
                /* when disable virtual buffer, all internel tensor are not from memory pool, not in inputoutputEntry.
                   if they can't be in SRAM, put it into LCD */
                if (-1 == entryIndex)
                {
                    binarySave->loadingDataCount += (vx_uint32)gpuCount;;
                }
            }

            if (vxoMemory_GetType(&image->memory) == VXNNE_MEM_POOL_TYPE_ORIG_DDR)
            {
                vx_int32 entryIndex;
                entryIndex = vxoBinaryGraph_GetIndexOfInputOutputEntry(binarySave->outputPhysicalEntry,
                                                                        image->memory.physicals[0]);
                /* when disable virtual buffer, all internel tensor are not from memory pool, not in inputoutputEntry.
                   if they can't be in SRAM, put it into LCD */
                if (-1 == entryIndex)
                {
                    binarySave->loadingDataCount += image->planeCount * (vx_uint32)gpuCount;;
                }
            }
            vxmASSERT(image->planeCount);
        }
        else if (operationTarget == VX_BINARY_OPERATION_TYPE_SW)
        {
            vx_node node = operation->layer->node;
            vx_bool isSupport = vxoBinaryGraph_isSupportSWOperation(operation);
            if (!isSupport)
            {
                vxError("fail to generate binary, because not support this software operation\n");
                vxError("layer name: %s\n", operation->layer->name);
                vxError("operation name: %s\n", operation->layer->node->kernel->name);
                vxmASSERT(0);
                vxmONERROR(VX_ERROR_NOT_SUPPORTED);
            }

            binarySave->swOperationCount++;

            binarySave->loadingDataCount += node->kernel->signature.paramCount;

            binarySave->layerParamCount += node->kernel->signature.paramCount;
        }
        else
        {
            /* For now, we don't support SW operations in binary graph */
            vxError("fail to generate binary, not support this operation target\n");
            vxmASSERT(0);
            vxmONERROR(VX_ERROR_NOT_SUPPORTED);
        }
    }

    if (context->binaryGraphInitBuffer != VX_NULL)
    {
        binarySave->initOperationCount = 1; /* A binary graph only one INIT operation */
        binarySave->loadingDataCount += binarySave->initOperationCount;
        binarySave->patchCount += 2; /* patch start/end remap addres for AXI-SRAM or VIP-SRAM */
    }

    binarySave->patchCount += 10; /* patch initialize commands */

    binarySave->endOperationCount = 1;
    binarySave->loadingDataCount += binarySave->endOperationCount;

    binarySave->operationCount = binarySave->nnOperationCount
                                + binarySave->tpOperationCount
                                + binarySave->shOperationCount
                                + binarySave->scOperationCount
                                + binarySave->initOperationCount
                                + binarySave->swOperationCount
                                + binarySave->endOperationCount;

    if (0 == binarySave->operationCount)
    {
        vxError("error: Operation Count is zero, can't generate network binary\n");
        vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
    }

    /* binarySave->operationCount value may change, so first release if the pointer is NOT NULL*/
    if (binarySave->operationCmdPhysical != VX_NULL)
    {
        gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)binarySave->operationCmdPhysical));
    }
    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof(vx_uint64) * binarySave->operationCount,
                      (gctPOINTER*)&binarySave->operationCmdPhysical)))
    {
        vxError("fail to allocate memory for operationCmdPhysical\n");
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }

    if (binarySave->operationOffset != VX_NULL)
    {
        gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)binarySave->operationOffset));
    }
    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof(vx_uint32) * binarySave->operationCount,
                          (gctPOINTER*)&binarySave->operationOffset)))
    {
        vxError("fail to allocate memory for operationOffset\n");
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }

    if ((binarySave->tpOperationCount + binarySave->nnOperationCount) > 0)
    {
        if (binarySave->patchNNTPCmdPhysical != VX_NULL)
        {
            gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)binarySave->patchNNTPCmdPhysical));
        }
        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL,
                        sizeof(vx_uint32) * (binarySave->tpOperationCount + binarySave->nnOperationCount),
                        (gctPOINTER*)&binarySave->patchNNTPCmdPhysical)))
        {
            vxError("fail to allocate memory for patchNNTPCmdPhysical\n");
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }

        if (binarySave->patchNNTPCmdOffset != VX_NULL)
        {
            gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)binarySave->patchNNTPCmdOffset));
        }
        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL,
                        sizeof(vx_uint32) * (binarySave->tpOperationCount + binarySave->nnOperationCount),
                        (gctPOINTER*)&binarySave->patchNNTPCmdOffset)))
        {
            vxError("fail to allocate memory for patchNNTPCmdOffset\n");
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }

        if (binarySave->NNTPDataCmdPhysical != VX_NULL)
        {
            vxFree((vx_ptr)binarySave->NNTPDataCmdPhysical);
        }
        binarySave->NNTPDataCmdPhysical = (vx_uint32*)vxAllocateAndZeroMemory((binarySave->tpOperationCount + binarySave->nnOperationCount) * sizeof(vx_uint32) );
        if (binarySave->NNTPDataCmdPhysical == VX_NULL)
        {
            vxError("fail to allocate memory for NNTPDataCmdPhysical\n");
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }

        if (binarySave->NNTPDataOffset != VX_NULL)
        {
            vxFree((vx_ptr)binarySave->NNTPDataOffset);
        }
        binarySave->NNTPDataOffset = (vx_uint32*)vxAllocateAndZeroMemory((binarySave->tpOperationCount + binarySave->nnOperationCount) * sizeof(vx_uint32));
        if (binarySave->NNTPDataOffset == VX_NULL)
        {
            vxError("fail to allocate memory for NNTPDataOffset\n");
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }
    }

    gcoOS_MemFill(&memoryPoolInfo, 0, sizeof(vx_binary_memory_pool_info_s));
    if (graph->memoryPool != NULL)
    {
        memoryPoolInfo.alignedSize    = (vx_uint32)graph->memoryPool->size;
        memoryPoolInfo.alignement     = 64;
        #if (ENABLE_SAVE_OFFSET_IN_NBG == 1)
        memoryPoolInfo.memoryPoolBase = 0;
        #else
        memoryPoolInfo.memoryPoolBase = graph->memoryPool->physical;
        #endif
    }

    gcoOS_MemFill(&axiSramInfo, 0, sizeof(vx_binary_axi_sram_info_s));
    if (context->axiSRAM[graph->deviceID].physical != 0)
    {
        #if (ENABLE_SAVE_OFFSET_IN_NBG == 1)
        axiSramInfo.sramBase = 0;
        #else
        axiSramInfo.sramBase = context->axiSRAM[graph->deviceID].physical;
        #endif
        axiSramInfo.sramSize = context->axiSRAM[graph->deviceID].size;
    }

    /* save network binary header - re-write later */
    gcoOS_MemFill(&binarySave->headerInfo, 0, sizeof(vx_binary_header_s));
    status = vxoBinaryGraph_Write(binarySave, 0, sizeof(vx_binary_header_s), &binarySave->headerInfo);
    WRITE_NBG_STATUS_CHECK();
    currPos = sizeof(vx_binary_header_s);

    /* save network binary memorypool */
    status = vxoBinaryGraph_Write(binarySave, currPos, sizeof(vx_binary_memory_pool_info_s), &memoryPoolInfo);
    WRITE_NBG_STATUS_CHECK();
    currPos += sizeof(vx_binary_memory_pool_info_s);

    /* save network binary axiSram */
    status = vxoBinaryGraph_Write(binarySave, currPos, sizeof(vx_binary_axi_sram_info_s), &axiSramInfo);
    WRITE_NBG_STATUS_CHECK();
    currPos += sizeof(vx_binary_axi_sram_info_s);

    /* save vip sram info */
    {
    vx_binary_vip_sram_info_s vipSramInfo;
    gcoOS_MemFill(&vipSramInfo, 0, sizeof(vx_binary_vip_sram_info_s));
    if (context->vipSRAM.size != 0)
    {
    #if (ENABLE_SAVE_OFFSET_IN_NBG == 1)
        vipSramInfo.sramBase = 0;
    #else
        vipSramInfo.sramBase = context->vipSRAM.physical;
    #endif
        vipSramInfo.sramSize = context->vipSRAM.size + VX_VIP_SRAM_IMAGE_STREAM_SIZE;
    }
    status = vxoBinaryGraph_Write(binarySave, currPos, sizeof(vx_binary_vip_sram_info_s), &vipSramInfo);
    WRITE_NBG_STATUS_CHECK();
    currPos += sizeof(vx_binary_vip_sram_info_s);
    }

    /* save network binary entranceInfo - re-write later */
    binarySave->entryTablePos = currPos;
    gcoOS_MemFill(&binarySave->entrancesInfo, 0, sizeof(vx_binary_entrance_info_s));
    status = vxoBinaryGraph_Write(binarySave, currPos, sizeof(vx_binary_entrance_info_s), &binarySave->entrancesInfo);
    WRITE_NBG_STATUS_CHECK();
    currPos += sizeof(vx_binary_entrance_info_s);

    /* refine input/output of graph if user call api vxIdentifyGraphInputsAndOutputs() */
    vxmONERROR(vxoBinaryGraph_RefineInputOutput(graph, &inputCount, &outputCount));

    /* allocate memory for inputInfo */
    graph->binarySave->inputInfo = (vx_binary_input_output_info_s*)vxAllocateAndZeroMemory(inputCount * sizeof(vx_binary_input_output_info_s));
    if (graph->binarySave->inputInfo == VX_NULL)
    {
        vxError("%s[%d]: failed to allocate memory for inputInfo\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }

    /* save network input info */
    inputInfoArray = graph->binarySave->inputInfo;
    for (i = 0; i < inputCount; i++)
    {
        if (0 != binarySave->inputPhysicalEntry[i])
        {
            vx_binary_input_output_info_s *inputInfo = &inputInfoArray[binarySave->headerInfo.inputCount];
            vx_reference ref = binarySave->inputEntry[i];
            if (ref->type == VX_TYPE_TENSOR)
            {
                vx_tensor tensor = (vx_tensor)ref;
                vx_int32 *dims = TENSOR_ORIG_SIZES(tensor);
                inputInfo->dimCount = TENSOR_ORIG_DIM_NUM(tensor);
                inputInfo->dataType = VX_BINARY_BUFFER_TYPE_TENSOR;
                for (j = 0; j < inputInfo->dimCount; j++)
                {
                    inputInfo->dims[j]   = (vx_uint32)dims[j];
                }
                for (j = inputInfo->dimCount; j < NN_MAX_DIMENSION; j++)
                {
                    inputInfo->dims[j] = 1;
                }
                vxmASSERT(NN_MAX_DIMENSION >= inputInfo->dimCount);
                for (k = i + 1; k < inputCount; k++)
                {
                    if ((binarySave->inputPhysicalEntry[i] == binarySave->inputPhysicalEntry[k]) &&
                        (graph->inputCount == 0))
                    {
                        /* 1. a tensor used by multiple nodes as input resource.
                           2. use vxReshapeTensor to reshape a tensor, and then the tensor be used to node input.
                           3. the tensor is a tensorView */
                        binarySave->inputPhysicalEntry[k] = 0;
                    }
                }

                inputInfo->dataFormat    = vxoBinaryGraph_ConvertToBinaryBufferFormat(TENSOR_DATA_TYPE(tensor));
                inputInfo->quantFormat   = TENSOR_QUANT_TYPE(tensor);
                inputInfo->fixedPointPos = TENSOR_POS(tensor);
                inputInfo->tfScale       = TENSOR_TF_SCALE(tensor);
                inputInfo->tfZeroPoint   = TENSOR_TF_ZEROPOINT(tensor);
            }
            else if (ref->type == VX_TYPE_SCALAR)
            {
                /* the buffer size of scalar equal to dims[0] * dims[1] * dims[2] * dims[3] * sizeof(int8)
                   set all scalar data format to int8, application used dims to calculator buffer size */
                vx_scalar scalar = (vx_scalar)ref;
                inputInfo->dimCount      = 1;
                inputInfo->dataFormat    = vxoBinaryGraph_ConvertToBinaryBufferFormat(VX_TYPE_INT8);
                inputInfo->dataType      = VX_BINARY_BUFFER_TYPE_SCALAR;
                inputInfo->quantFormat   = VX_BINARY_BUFFER_QUANT_FORMAT_NONE;
                inputInfo->dims[0]       = vxoScalar_GetTypeSize(scalar);
                inputInfo->dims[1]       = 1;
                inputInfo->dims[2]       = 1;
                inputInfo->dims[3]       = 1;
                inputInfo->fixedPointPos = 0;
                inputInfo->tfScale       = 0;
                inputInfo->tfZeroPoint   = 0;
            }
            else if (ref->type == VX_TYPE_ARRAY)
            {
                vx_array array = (vx_array)ref;
                inputInfo->dimCount      = 1;
                inputInfo->dataFormat    = vxoBinaryGraph_ConvertToBinaryBufferFormat((vx_uint32)array->itemType);
                inputInfo->dataType      = VX_BINARY_BUFFER_TYPE_ARRAY;
                inputInfo->quantFormat   = VX_BINARY_BUFFER_QUANT_FORMAT_NONE;
                inputInfo->dims[0]       = (vx_uint32)array->capacity;
                inputInfo->dims[1]       = 1;
                inputInfo->dims[2]       = 1;
                inputInfo->dims[3]       = 1;
                inputInfo->fixedPointPos = 0;
                inputInfo->tfScale       = 0;
                inputInfo->tfZeroPoint   = 0;
            }
            else if (ref->type == VX_TYPE_IMAGE)
            {
                gcoVX_Kernel_Context kernelContext; /* not useful, just fulfill the interface */
                gcsVX_IMAGE_INFO imageInfo;
                vx_df_image format;
                vx_image image = (vx_image)ref;
                INITIALIZE_STRUCT(imageInfo);

                gcoOS_MemFill((gctPOINTER*)(&kernelContext), 0, sizeof(gcoVX_Kernel_Context));
                gcfVX_GetImageInfo(&kernelContext, image, &imageInfo, 1);
                vxQueryImage(image, VX_IMAGE_FORMAT, &format, sizeof(format));
                inputInfo->dataType = VX_BINARY_BUFFER_TYPE_IMAGE;
                if (1 == image->planeCount)
                {
                    inputInfo->dimCount      = 2; /* width, height*/
                    inputInfo->dataFormat    = vxoBinaryGraph_ConvertToBinaryBufferFormat((vx_uint32)format);
                    inputInfo->dims[0]       = imageInfo.width;
                    inputInfo->dims[1]       = imageInfo.height;
                    inputInfo->dims[2]       = 1;
                    inputInfo->dims[3]       = 1;
                    inputInfo->fixedPointPos = 0;
                    inputInfo->tfScale       = 0;
                    inputInfo->tfZeroPoint   = 0;
                    inputInfo->quantFormat   = VX_BINARY_BUFFER_QUANT_FORMAT_NONE;
                }
                else if (3 == image->planeCount)
                {
                    if (VX_DF_IMAGE_IYUV == format)
                    {
                        inputInfo->dimCount      = 3;
                        inputInfo->dataFormat    = vxoBinaryGraph_ConvertToBinaryBufferFormat((vx_uint32)format); /* uint8 */
                        inputInfo->dims[0]       = imageInfo.width;
                        inputInfo->dims[1]       = imageInfo.height;
                        inputInfo->dims[2]       = 2;
                        inputInfo->dims[3]       = 1;
                        inputInfo->fixedPointPos = 0;
                        inputInfo->tfScale       = 0;
                        inputInfo->tfZeroPoint   = 0;
                        inputInfo->quantFormat   = VX_BINARY_BUFFER_QUANT_FORMAT_NONE;
                    }
                    else if (VX_DF_IMAGE_YUV4 == format)
                    {
                        inputInfo->dimCount      = 3;
                        inputInfo->dataFormat    = vxoBinaryGraph_ConvertToBinaryBufferFormat((vx_uint32)format); /* uint8 */;
                        inputInfo->dims[0]       = imageInfo.width;
                        inputInfo->dims[1]       = imageInfo.height;
                        inputInfo->dims[2]       = 3;
                        inputInfo->dims[3]       = 1;
                        inputInfo->fixedPointPos = 0;
                        inputInfo->tfScale       = 0;
                        inputInfo->tfZeroPoint   = 0;
                        inputInfo->quantFormat   = VX_BINARY_BUFFER_QUANT_FORMAT_NONE;
                    }
                    else
                    {
                        vxError("%s[%d]: input not support this image format type : %d\n", __FUNCTION__, __LINE__, format);
                        continue;
                    }
                }
                else
                {
                    vxError("error: input not support this image format: %d\n", (vx_uint32)format);
                    continue;
                }
            }
            else
            {
                vxError("error: not support the type as graph input: %d\n", (vx_int32)ref->type);
                vxmONERROR(VX_FAILURE);
            }
            if (gcoOS_StrCmp(ref->name, "\0") != gcvSTATUS_OK)
            {
                gcoOS_StrCopySafe(inputInfo->name, VX_MAX_IO_NAME_LEGTH, ref->name);
            }
            else
            {
                vx_char input_name[256];
                sprintf(input_name, "input[%d]", i);
                gcoOS_StrCopySafe(inputInfo->name, VX_MAX_IO_NAME_LEGTH, input_name);
            }

            status = vxoBinaryGraph_Write(binarySave, currPos, sizeof(vx_binary_input_output_info_s), inputInfo);
            WRITE_NBG_STATUS_CHECK();
            currPos += sizeof(vx_binary_input_output_info_s);

            binarySave->headerInfo.inputCount ++;
        }
    }

    k = 0;
    for (i = 0; i < inputCount; i++)
    {
        if (k >= inputCount) break;
        if (0 == binarySave->inputPhysicalEntry[i])
        {
           /* clean up useless input pyhsical entry */
           for (j = i; j < inputCount; j++)
           {
               binarySave->inputPhysicalEntry[j] = binarySave->inputPhysicalEntry[j + 1];
               binarySave->inputEntry[j] = binarySave->inputEntry[j + 1];
           }
           i -= 1;
        }
        k++;
    }

    /* delete the same outputs from back to front, mark useless outputPhysicalEntry is 0
       this is for concat layer to be last layer. concat layer use tensorview to implement */
    for (numI = outputCount - 1; numI >= 0; numI--)
    {
        if (binarySave->outputPhysicalEntry[numI] != 0)
        {
            for (numJ = numI - 1; numJ >= 0; numJ--)
            {
                if (binarySave->outputPhysicalEntry[numI] == binarySave->outputPhysicalEntry[numJ])
                {
                    binarySave->outputPhysicalEntry[numJ] = 0;
                }
            }
        }
    }

    /* clean up useless output pyhsical entry */
    k = 0;
    for (i = 0; i < outputCount; i++)
    {
       if (k >= outputCount)  break;
       if (0 == binarySave->outputPhysicalEntry[i])
       {
           for (j = i; j < outputCount; j++)
           {
               binarySave->outputPhysicalEntry[j] = binarySave->outputPhysicalEntry[j + 1];
               binarySave->outputEntry[j] = binarySave->outputEntry[j + 1];
           }
           i -= 1;
       }
       k++;
    }

    /* save network output info */
    for (i = 0; i < outputCount; i++)
    {
        if (0 != binarySave->outputPhysicalEntry[i])
        {
            vx_reference ref = binarySave->outputEntry[i];
            INITIALIZE_STRUCT(outputInfo);
            if (ref->type == VX_TYPE_TENSOR)
            {
                vx_tensor tensor = (vx_tensor)ref;
                vx_int32 *dims = TENSOR_ORIG_SIZES(tensor);
                outputInfo.dimCount = TENSOR_ORIG_DIM_NUM(tensor);
                for (j = 0; j < outputInfo.dimCount; j++)
                {
                   outputInfo.dims[j]   = dims[j];
                }
                for (j = outputInfo.dimCount; j < NN_MAX_DIMENSION; j++)
                {
                    outputInfo.dims[j] = 1;
                }

                vxmASSERT(NN_MAX_DIMENSION >= outputInfo.dimCount);
                for (k = i + 1; k < outputCount; k++)
                {
                    if ((binarySave->outputPhysicalEntry[i] == binarySave->outputPhysicalEntry[k]) &&
                        (graph->outputCount != 0))
                    {
                        /* 1. a tensor used by multiple nodes as output resource.
                           2. use vxReshapeTensor to reshape a tensor, and then the tensor be used to node output.
                           3. the tensor is a tensorView */
                        binarySave->outputPhysicalEntry[k] = 0;
                    }
                }

                outputInfo.dataFormat    = vxoBinaryGraph_ConvertToBinaryBufferFormat(TENSOR_DATA_TYPE(tensor));
                outputInfo.dataType      = VX_BINARY_BUFFER_TYPE_TENSOR;
                outputInfo.quantFormat   = TENSOR_QUANT_TYPE(tensor);
                outputInfo.fixedPointPos = TENSOR_POS(tensor);
                outputInfo.tfScale       = TENSOR_TF_SCALE(tensor);
                outputInfo.tfZeroPoint   = TENSOR_TF_ZEROPOINT(tensor);
            }
            else if (ref->type == VX_TYPE_ARRAY)
            {
                vx_array array = (vx_array)ref;
                outputInfo.dimCount      = 1;
                outputInfo.dataFormat    = vxoBinaryGraph_ConvertToBinaryBufferFormat((vx_uint32)array->itemType);
                outputInfo.dataType      = VX_BINARY_BUFFER_TYPE_ARRAY;
                outputInfo.quantFormat   = VX_BINARY_BUFFER_QUANT_FORMAT_NONE;
                outputInfo.dims[0]       = (vx_uint32)array->capacity;
                outputInfo.dims[1]       = 1;
                outputInfo.dims[2]       = 1;
                outputInfo.dims[3]       = 1;
                outputInfo.fixedPointPos = 0;
                outputInfo.tfScale       = 0;
                outputInfo.tfZeroPoint   = 0;
            }
            else if (ref->type == VX_TYPE_SCALAR)
            {
                /* the buffer size of scalar equal to dims[0] * dims[1] * dims[2] * dims[3] * sizeof(int8)
                   set all scalar data format to int8, application used dims to calculator buffer size */
                vx_scalar scalar = (vx_scalar)ref;
                outputInfo.dimCount      = 1;
                outputInfo.dataFormat    = vxoBinaryGraph_ConvertToBinaryBufferFormat(VX_TYPE_INT8);
                outputInfo.dataType      = VX_BINARY_BUFFER_TYPE_SCALAR;
                outputInfo.quantFormat   = VX_BINARY_BUFFER_QUANT_FORMAT_NONE;
                outputInfo.dims[0]       = vxoScalar_GetTypeSize(scalar);
                outputInfo.dims[1]       = 1;
                outputInfo.dims[2]       = 1;
                outputInfo.dims[3]       = 1;
                outputInfo.fixedPointPos = 0;
                outputInfo.tfScale       = 0;
                outputInfo.tfZeroPoint   = 0;
            }
            else if (ref->type == VX_TYPE_IMAGE)
            {
                gcoVX_Kernel_Context kernelContext; /* not useful, just fulfill the interface */
                gcsVX_IMAGE_INFO imageInfo;
                vx_df_image format;
                vx_image image = (vx_image)ref;
                INITIALIZE_STRUCT(imageInfo);

                gcoOS_MemFill((gctPOINTER*)(&kernelContext), 0, sizeof(gcoVX_Kernel_Context));
                gcfVX_GetImageInfo(&kernelContext, image, &imageInfo, 1);
                vxQueryImage(image, VX_IMAGE_FORMAT, &format, sizeof(format));
                outputInfo.dataType      = VX_BINARY_BUFFER_TYPE_IMAGE;
                if (1 == image->planeCount)
                {
                    outputInfo.dimCount      = 2; /* width, height*/
                    outputInfo.dataFormat    = vxoBinaryGraph_ConvertToBinaryBufferFormat((vx_uint32)format);
                    outputInfo.dims[0]       = imageInfo.width;
                    outputInfo.dims[1]       = imageInfo.height;
                    outputInfo.dims[2]       = 1;
                    outputInfo.dims[3]       = 1;
                    outputInfo.fixedPointPos = 0;
                    outputInfo.tfScale       = 0;
                    outputInfo.tfZeroPoint   = 0;
                    outputInfo.quantFormat   = VX_BINARY_BUFFER_QUANT_FORMAT_NONE;
                }
                else if (3 == image->planeCount)
                {
                    if (VX_DF_IMAGE_IYUV == format)
                    {
                        outputInfo.dimCount      = 3;
                        outputInfo.dataFormat    = vxoBinaryGraph_ConvertToBinaryBufferFormat((vx_uint32)format);/* uint8 */
                        outputInfo.dims[0]       = imageInfo.width;
                        outputInfo.dims[1]       = imageInfo.height;
                        outputInfo.dims[2]       = 2;
                        outputInfo.dims[3]       = 1;
                        outputInfo.fixedPointPos = 0;
                        outputInfo.tfScale       = 0;
                        outputInfo.tfZeroPoint   = 0;
                        outputInfo.quantFormat   = VX_BINARY_BUFFER_QUANT_FORMAT_NONE;
                    }
                    else if (VX_DF_IMAGE_YUV4 == format)
                    {
                        outputInfo.dimCount      = 3;
                        outputInfo.dataFormat    = vxoBinaryGraph_ConvertToBinaryBufferFormat((vx_uint32)format);/* uint8 */
                        outputInfo.dims[0]       = imageInfo.width;
                        outputInfo.dims[1]       = imageInfo.height;
                        outputInfo.dims[2]       = 3;
                        outputInfo.dims[3]       = 1;
                        outputInfo.fixedPointPos = 0;
                        outputInfo.tfScale       = 0;
                        outputInfo.tfZeroPoint   = 0;
                        outputInfo.quantFormat   = VX_BINARY_BUFFER_QUANT_FORMAT_NONE;
                    }
                    else
                    {
                        vxError("%s[%d]: output not support this image format type : %d\n", __FUNCTION__, __LINE__, format);
                        continue;
                    }
                }
                else
                {
                    vxError("error: output not support this image format: %d\n", (vx_uint32)format);
                    continue;
                }
            }
            else
            {
               vxError("error: not support the type as graph output: %d\n", (vx_int32)ref->type);
               vxmONERROR(VX_FAILURE);
            }
            if (gcoOS_StrCmp(ref->name, "\0") != gcvSTATUS_OK)
            {
                gcoOS_StrCopySafe(outputInfo.name, VX_MAX_IO_NAME_LEGTH, ref->name);
            }
            else
            {
                vx_char output_name[256];
                sprintf(output_name, "output[%d]", i);
                gcoOS_StrCopySafe(outputInfo.name, VX_MAX_IO_NAME_LEGTH, output_name);
            }

            status = vxoBinaryGraph_Write(binarySave, currPos, sizeof(vx_binary_input_output_info_s), &outputInfo);
            WRITE_NBG_STATUS_CHECK();
            currPos += sizeof(vx_binary_input_output_info_s);

            binarySave->headerInfo.outputCount ++;
        }
    }

    binarySave->inputParamCount = binarySave->headerInfo.inputCount;
    binarySave->outputParamCount = binarySave->headerInfo.outputCount;

    vxmASSERT(binarySave->headerInfo.inputCount > 0);
    vxmASSERT(binarySave->headerInfo.outputCount > 0);
    if ((binarySave->headerInfo.outputCount <= 0) || (binarySave->headerInfo.inputCount <= 0))
    {
        vxError("error: fail to generate binary, input/output error\n");
        vxmONERROR(VX_FAILURE);
    }

    /* save network layers info */
    for (i = 0; i < graph->nodeCount; i++)
    {
        vx_node node = graph->nodeTable[graph->allNodeIndexTable[i]];
        vx_kernel kernel = node->kernel;
        vx_uint32 k = 0;

        gcoOS_ZeroMemory(&layersInfo, sizeof(vx_binary_layers_info_s));
        /* The ID is execution order of the nodes */
        layersInfo.layerId = i;
        gcoOS_StrCopySafe(layersInfo.layerName, VX_MAX_LAYER_NAME_LENGTH, node->layer->name);

        layersInfo.operationCount = node->layer->num_operations;

        /* get UID */
        layersInfo.uid = 0xFFFFFFFF;
        for (k = 0; k < VX_MAX_PARAMETERS; k++)
        {
            vx_reference ref = node->paramTable[k];
            if ((ref != VX_NULL) && (ref->type != VX_TYPE_SCALAR) && (ref->type != VX_TYPE_VENDOR_OBJECT_START))
            {
                if (kernel->signature.directionTable[k] != VX_INPUT)
                {
                    if (gcoOS_StrStr(ref->name, "out", VX_NULL))
                    {
                        vx_int32 uid = vxoBinaryGraph_GetUidFromOutputTensor((vx_tensor)ref);
                        if (uid >= 0)
                        {
                            layersInfo.uid = uid;
                            break;
                        }
                    }
                }
            }
        }

        status = vxoBinaryGraph_Write(binarySave, currPos, sizeof(vx_binary_layers_info_s), &layersInfo);
        WRITE_NBG_STATUS_CHECK();
        currPos += sizeof(vx_binary_layers_info_s);
    }

    operationPos = currPos;

    binarySave->currOperationOffset = currPos;

    currPos += binarySave->operationCount * sizeof(vx_binary_operation_info_s);
    binarySave->currNNOperationOffset = currPos;

    currPos += binarySave->nnOperationCount * sizeof(vx_binary_nn_operation_info_s);
    binarySave->currTPOperationOffset = currPos;

    currPos += binarySave->tpOperationCount * sizeof(vx_binary_tp_operation_info_s);
    binarySave->currSHOperationOffset = currPos;

    currPos += binarySave->shOperationCount * sizeof(vx_binary_sh_operation_info_s);
    binarySave->currPatchOffset = currPos;

    currPos += binarySave->patchCount * sizeof(vx_binary_patch_info_s);
    binarySave->currLayerParamOffset = currPos;

    currPos += binarySave->layerParamCount * sizeof(vx_binary_layer_parameter_s);
    binarySave->currSWOperationOffset = currPos;

    currPos += binarySave->swOperationCount * sizeof(vx_binary_sw_operation_info_s);
    binarySave->currLoadingDataTableOffset = currPos;

    currPos += binarySave->loadingDataCount * sizeof(vx_binary_loadingdata_table_info_s);

    /* LCD offset should be aligned to 256 */
    if (1 == binarySave->generateNBGToMemory)
    {
        vx_uint8_ptr pos =  (vx_uint8_ptr)binarySave->NBGBuffer + currPos;
        vx_uint64 address = gcmPTR_TO_UINT64(pos);
        vx_uint64 alignAddress = gcmALIGN_SAFE(address, SH_COMMAND_ALIGN_SIZE);
        vx_uint32 diff = (vx_uint32)(alignAddress - address);
        currPos += diff;
    }

    currPos = gcmALIGN(currPos, SH_COMMAND_ALIGN_SIZE); /* for DDR-less project alignment */

    binarySave->currLoadingDataOffset = binarySave->loadingDataStartOffset = currPos;

    /* save network binary header info*/
    binarySave->headerInfo.magic[0]= 'V';
    binarySave->headerInfo.magic[1]= 'P';
    binarySave->headerInfo.magic[2]= 'M';
    binarySave->headerInfo.magic[3]= 'N';
    binarySave->headerInfo.version = 0x00010008;
    binarySave->headerInfo.target = context->pid;
    binarySave->headerInfo.layerCount     = graph->nodeCount;
    binarySave->headerInfo.operationCount = binarySave->operationCount;
    vxoBinaryGraph_GetNetworkNameAndRank(binarySave);
    vxoBinaryGraph_GetFeatureDB(graph, binarySave);

    status = vxoBinaryGraph_Write(binarySave, 0, sizeof(vx_binary_header_s), &binarySave->headerInfo);
    WRITE_NBG_STATUS_CHECK();

    /* save network binary entrance info */
    binarySave->entrancesInfo.inputEntr.inputInfoOffset = sizeof(vx_binary_header_s) + sizeof(vx_binary_memory_pool_info_s)
                                                            + sizeof(vx_binary_axi_sram_info_s) + sizeof(vx_binary_vip_sram_info_s)
                                                            + sizeof(vx_binary_entrance_info_s);
    binarySave->entrancesInfo.inputEntr.inputInfoBytes = binarySave->headerInfo.inputCount * sizeof(vx_binary_input_output_info_s);

    binarySave->entrancesInfo.outputEntr.outputInfoOffset = binarySave->entrancesInfo.inputEntr.inputInfoOffset
                                                            + binarySave->entrancesInfo.inputEntr.inputInfoBytes;
    binarySave->entrancesInfo.outputEntr.outputInfoBytes = binarySave->headerInfo.outputCount * sizeof(vx_binary_input_output_info_s);

    binarySave->entrancesInfo.layersEntr.layersInfoOffset = binarySave->entrancesInfo.outputEntr.outputInfoOffset
                                                            + binarySave->entrancesInfo.outputEntr.outputInfoBytes;
    binarySave->entrancesInfo.layersEntr.layersInfoBytes = graph->nodeCount * sizeof(vx_binary_layers_info_s);

    binarySave->entrancesInfo.operationsEntr.operationsOffset = binarySave->entrancesInfo.layersEntr.layersInfoOffset
                                                                + binarySave->entrancesInfo.layersEntr.layersInfoBytes;
    binarySave->entrancesInfo.operationsEntr.operationsBytes = binarySave->operationCount * sizeof(vx_binary_operation_info_s);

    binarySave->entrancesInfo.nnOperationsEntr.nnOperationDataOffset = binarySave->entrancesInfo.operationsEntr.operationsOffset
                                                                        + binarySave->entrancesInfo.operationsEntr.operationsBytes;
    binarySave->entrancesInfo.nnOperationsEntr.nnOperationDataBytes  = binarySave->nnOperationCount * sizeof(vx_binary_nn_operation_info_s);

    binarySave->entrancesInfo.tpOperationsEntr.tpOperationDataOffset = binarySave->entrancesInfo.nnOperationsEntr.nnOperationDataOffset
                                                                        + binarySave->entrancesInfo.nnOperationsEntr.nnOperationDataBytes;
    binarySave->entrancesInfo.tpOperationsEntr.tpOperationDataBytes  = binarySave->tpOperationCount * sizeof(vx_binary_tp_operation_info_s);

    binarySave->entrancesInfo.shOperationsEntr.shaderOperationDataOffset = binarySave->entrancesInfo.tpOperationsEntr.tpOperationDataOffset
                                                                            + binarySave->entrancesInfo.tpOperationsEntr.tpOperationDataBytes;
    binarySave->entrancesInfo.shOperationsEntr.shaderOperationDataBytes  = binarySave->shOperationCount * sizeof(vx_binary_sh_operation_info_s);

    binarySave->entrancesInfo.patchsEntr.patchDataOffset          = binarySave->entrancesInfo.shOperationsEntr.shaderOperationDataOffset
                                                                    + binarySave->entrancesInfo.shOperationsEntr.shaderOperationDataBytes;
    binarySave->entrancesInfo.patchsEntr.patchDataBytes           = binarySave->patchCount * sizeof(vx_binary_patch_info_s);

    binarySave->entrancesInfo.layerParamEntr.layerParamOffset = binarySave->entrancesInfo.patchsEntr.patchDataOffset
                                                                + binarySave->entrancesInfo.patchsEntr.patchDataBytes;
    binarySave->entrancesInfo.layerParamEntr.layerParamBytes = binarySave->layerParamCount * sizeof(vx_binary_layer_parameter_s);

    binarySave->entrancesInfo.swOperationsEntr.swOperationDataOffset = binarySave->entrancesInfo.layerParamEntr.layerParamOffset
                                                                        + binarySave->entrancesInfo.layerParamEntr.layerParamBytes;
    binarySave->entrancesInfo.swOperationsEntr.swOperationDataBytes = binarySave->swOperationCount * sizeof(vx_binary_sw_operation_info_s);

    binarySave->entrancesInfo.loadingConfigDataTableEntr.loadingConfigDataTableOffset = binarySave->entrancesInfo.swOperationsEntr.swOperationDataOffset
                                                                                        + binarySave->entrancesInfo.swOperationsEntr.swOperationDataBytes;
    binarySave->entrancesInfo.loadingConfigDataTableEntr.loadingConfigDataTableBytes  = binarySave->loadingDataCount * sizeof(vx_binary_loadingdata_table_info_s);

    binarySave->entrancesInfo.loadingConfigDataEntr.loadingConfigDataOffset = gcmALIGN((binarySave->entrancesInfo.loadingConfigDataTableEntr.loadingConfigDataTableOffset
                                                                            + binarySave->entrancesInfo.loadingConfigDataTableEntr.loadingConfigDataTableBytes),
                                                                            SH_COMMAND_ALIGN_SIZE); /* for DDR-less project alignment */
    binarySave->entrancesInfo.loadingConfigDataEntr.loadingConfigDataBytes  = 0; /* rewrite later */

    binarySave->entrancesInfo.loadingConfigDataEntr.loadingConfigDataOffset =  binarySave->currLoadingDataOffset;

    status = vxoBinaryGraph_Write(binarySave, binarySave->entryTablePos, sizeof(vx_binary_entrance_info_s), &binarySave->entrancesInfo);
    WRITE_NBG_STATUS_CHECK();

    /* fseek can't move file pointer to offset if offset is larger than the binary size*/
    /* so, fill zero to initial binary*/
    if ((currPos -  operationPos) > ONCE_FILL_SIZE)
    {
        vx_uint32 offset = operationPos;
        for (i = 0; i < (currPos -  operationPos) / ONCE_FILL_SIZE; i++)
        {
            status = vxoBinaryGraph_Write(binarySave, offset, ONCE_FILL_SIZE, fillZero);
            WRITE_NBG_STATUS_CHECK();
            offset += ONCE_FILL_SIZE;
        }
        if (((currPos -  operationPos) % ONCE_FILL_SIZE) != 0)
        {
            status = vxoBinaryGraph_Write(binarySave, offset, (currPos -  operationPos) % ONCE_FILL_SIZE, fillZero);
            WRITE_NBG_STATUS_CHECK();
        }
    }
    else
    {
        status = vxoBinaryGraph_Write(binarySave, operationPos, currPos -  operationPos, fillZero);
        WRITE_NBG_STATUS_CHECK();
    }

    /* save initialization command */
    if ((context->binaryGraphInitBuffer != VX_NULL) && (context->binaryGraphInitSize != VX_NULL))
    {
        vxmONERROR(vxoBinaryGraph_SaveInitialOperation(graph));
    }

OnError:
    if (fillZero != VX_NULL)
    {
        gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)fillZero));
        fillZero = VX_NULL;
    }
    if (shOperationID != VX_NULL)
    {
        gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)shOperationID));
        shOperationID = VX_NULL;
    }
    if (shaderParam != VX_NULL)
    {
        gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)shaderParam));
        shaderParam = VX_NULL;
    }

    status = vxoBinaryGraph_SaveErrorHandle(graph, status);

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vx_bool vxoBinaryGraph_isCONV(
    vx_node node
    )
{
    vx_enum enumType = node->kernel->enumeration;
    vx_bool isCONV = vx_false_e;

    switch (enumType)
    {
        case VX_KERNEL_NN_CONVOLUTION_RELU_POOLING_LAYER2:
        case VX_KERNEL_NN_CONVOLUTION_RELU_POOLING_LAYER:
        case VX_KERNEL_NN_CONVOLUTION_RELU_LAYER:
        case VX_KERNEL_CONVOLUTION_LAYER:
            isCONV = vx_true_e;
            break;
        default:
            isCONV = vx_false_e;
            break;
    }
    return isCONV;
}

VX_PRIVATE_API vx_bool vxoBinaryGraph_isFC(
    vx_node node
    )
{
    vx_enum enumType = node->kernel->enumeration;
    vx_bool isFC = vx_false_e;

    switch (enumType)
    {
        case VX_KERNEL_NN_FULLY_CONNECTED_RELU_LAYER:
        case VX_KERNEL_NN_FULLY_CONNECTED_LAYER:
        case VX_KERNEL_FULLY_CONNECTED_LAYER:
            isFC = vx_true_e;
            break;
        default:
            isFC = vx_false_e;
            break;
    }
    return isFC;
}

/*
find the index of nodeTable.
return node index:
 1. the last node with weight/bias parameter if this graph doesn't branch.
 2. there are weight/bias parameter before the first barnch node.
*/
VX_PRIVATE_API vx_status vxoBinaryGraph_FindNodeIndexForWeight(
    vx_graph graph,
    vx_int32 *index
    )
{
    vx_int32 nodeIndex = 0;
    vx_int32 i = 0;
    vx_bool isBranchGraph = vx_false_e;
    vx_int32 nodeCount = graph->nodeCount;
    vx_node *nodeTable = graph->nodeTable;
    vx_node node = VX_NULL;
    gcmHEADER_ARG("graph=%p, index=%p", graph, index);

    *index = -1;
    for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        node = nodeTable[graph->allNodeIndexTable[nodeIndex]];
        if (node->numChildren > 1)
        {
            isBranchGraph = vx_true_e;
            break;
        }
    }

    if (!isBranchGraph)
    {
        /* the graph doesn't branch */
        for (nodeIndex = nodeCount - 1; nodeIndex >= 0; nodeIndex--)
        {
            node = nodeTable[graph->allNodeIndexTable[nodeIndex]];
            /* find convolution weight/bias first */
            if (vxoBinaryGraph_isCONV(node))
            {
                *index = nodeIndex;
                break;
            }
        }
        /* use FC's weight/bias to match if this graph doesn't CONV node*/
        if (nodeIndex < 0)
        {
            for (nodeIndex = nodeCount - 1; nodeIndex >= 0; nodeIndex--)
            {
                node = nodeTable[graph->allNodeIndexTable[nodeIndex]];
                if (vxoBinaryGraph_isFC(node))
                {
                    *index = nodeIndex;
                    break;
                }
            }
        }
        if (nodeIndex < 0)
        {
            /* the graph doesn't CONV and FC */
            *index = -1;
            vxInfo("the graph has branch. doesn't CONV and FC, right or not?\n");
        }
    }
    else
    {
        /* The graph has branchs, nodeTable[graph->allNodeIndexTable[nodeIndex]] is branch node*/
        vx_int32 FcNodeIndex = -1;
        for (i = nodeIndex; i >= 0; i--)
        {
            node = nodeTable[graph->allNodeIndexTable[i]];
            if (vxoBinaryGraph_isCONV(node))
            {
                *index = i;
                break;
            }

            if (vxoBinaryGraph_isFC(node))
            {
                FcNodeIndex = i;
            }
        }

        if ((i < 0) && (-1 != FcNodeIndex))
        {
            /* no CONV, FC is in front of branch*/
            *index = FcNodeIndex;
            i = FcNodeIndex;
        }

        if (i < 0)
        {   /* no CONV/FC in front of branch  */
            for (i = nodeIndex + 1; i < nodeCount; i++)
            {
                node = nodeTable[graph->allNodeIndexTable[i]];
                if (vxoBinaryGraph_isCONV(node))
                {
                    *index = i;
                    break;
                }

                if (vxoBinaryGraph_isFC(node))
                {
                    FcNodeIndex = i;
                }
            }
            if ((i == nodeCount) && (-1 != FcNodeIndex))
            {
                /* no CONV, FC is behind the branch*/
                *index = FcNodeIndex;
                i = 0;
            }

            if (i == nodeCount)
            {
                /* the graph doesn't CONV and FC */
                *index = -1;
                vxInfo("the graph no branch. doesn't CONV and FC, right or not?\n");
            }
        }
    }
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

 VX_PRIVATE_API vx_tensor vxoBinaryGraph_GetWeightTensorFromNode(
    vx_node node
    )
 {
    vx_enum enumType = node->kernel->enumeration;
    vx_tensor weight = VX_NULL;
    gcmHEADER_ARG("node=%p", node);

    switch (enumType)
    {
        case VX_KERNEL_NN_CONVOLUTION_RELU_POOLING_LAYER2:
        case VX_KERNEL_NN_CONVOLUTION_RELU_POOLING_LAYER:
        case VX_KERNEL_NN_CONVOLUTION_RELU_LAYER:
        case VX_KERNEL_NN_FULLY_CONNECTED_RELU_LAYER:
        {
            vx_weights_biases_parameter wb = VX_NULL;
            vx_uint32 i = 0;

            for (i = 0; i < node->kernel->signature.paramCount; i++)
            {
                if (node->paramTable[i]->type == VX_TYPE_WEIGHTS_BIASES_PARAMETER)
                {
                    wb = (vx_weights_biases_parameter)node->paramTable[i];
                    break;
                }
            }
            if (wb != VX_NULL)
            {
                weight = WB_WEIGHT_TENSOR(wb);
            }
            break;
        }

        case VX_KERNEL_CONVOLUTION_LAYER:
        case VX_KERNEL_FULLY_CONNECTED_LAYER:
        case VX_KERNEL_NN_FULLY_CONNECTED_LAYER:
        {
            weight = (vx_tensor)node->paramTable[1];
            break;
        }

        default:
            break;
    }
    gcmFOOTER_ARG("%p", weight);
    return weight;
 }

VX_PRIVATE_API vx_status vxoBinaryGraph_GetNodeParametersValue(
    vx_node node,
    vx_uint32_ptr *toplogyInfo
    )
{
    vx_weights_biases_parameter wb = VX_NULL;
    vx_enum nodeType = node->kernel->enumeration;
    vx_uint32 i = 0;
    vx_uint32_ptr buffer = *toplogyInfo;

    gcmHEADER_ARG("node=%p, toplogyInfo=%p", node, toplogyInfo);

    for (i = 0; i < node->numParameters; i++)
    {
        if (node->paramTable[i]->type == VX_TYPE_WEIGHTS_BIASES_PARAMETER)
        {
            wb = (vx_weights_biases_parameter)node->paramTable[i];
            break;
        }
    }

    switch(nodeType)
    {
        case VX_KERNEL_CONVOLUTION_LAYER:
        {
            /* kernel size */
            if (wb != VX_NULL)
            {
                *buffer = WB_KERNEL_X(wb); buffer++;
            }
            /* pad */
            *buffer = SCALAR_VALUE(node->paramTable[3], u32); buffer++;
            /* stride*/
            *buffer = SCALAR_VALUE(node->paramTable[11], u32); buffer++;
            /* dilationXScalar */
            *buffer = SCALAR_VALUE(node->paramTable[9], u32); buffer++;
            /* input w */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[0], 0); buffer++;
            /* input h */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[0], 1);
            break;
        }
        case VX_KERNEL_NN_CONVOLUTION_RELU_LAYER:
        {
            /* kernel size */
            if (wb != VX_NULL)
            {
                *buffer = WB_KERNEL_X(wb); buffer++;
            }
            /* pad */
            *buffer = SCALAR_VALUE(node->paramTable[2], u32); buffer++;
            /* enable relu */
            *buffer = SCALAR_VALUE(node->paramTable[8], u32); buffer++;
            /* input w */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[0], 0); buffer++;
            /* input h */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[0], 1); buffer++;
            /* output w */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[node->numParameters - 1], 0);
            break;
        }
        case VX_KERNEL_NN_CONVOLUTION_RELU_POOLING_LAYER:
        {
            /* kernel size */
            if (wb != VX_NULL)
            {
                *buffer = WB_KERNEL_X(wb); buffer++;
            }
            /* pad */
            *buffer = SCALAR_VALUE(node->paramTable[3], u32); buffer++;
            /* stride*/
            *buffer = SCALAR_VALUE(node->paramTable[8], u32);; buffer++;
            /* enable relu */
            *buffer = SCALAR_VALUE(node->paramTable[10], u32); buffer++;
            /* input w */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[0], 0); buffer++;
            /* input h */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[0], 1);
            break;
        }
        case VX_KERNEL_NN_CONVOLUTION_RELU_POOLING_LAYER2:
        {
            /* kernel size */
            if (wb != VX_NULL)
            {
                *buffer = WB_KERNEL_X(wb); buffer++;
            }
            /* pad */
            *buffer = SCALAR_VALUE(node->paramTable[4], u32); buffer++;
            /* enable relu*/
            *buffer = SCALAR_VALUE(node->paramTable[12], u32); buffer++;
            /* pool size */
            *buffer = SCALAR_VALUE(node->paramTable[14], u32); buffer++;
            /* input w */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[0], 0); buffer++;
            /* input h */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[0], 1);
            break;
        }
        case VX_KERNEL_NN_FULLY_CONNECTED_LAYER:
        {
            /* input w */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[0], 0); buffer++;
            /* input h */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[0], 1); buffer++;
            /* pad*/
            *buffer = SCALAR_VALUE(node->paramTable[3], u32); buffer++;
            /* output w */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[node->numParameters - 1], 0); buffer++;
            /* output h */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[node->numParameters - 1], 1);
            break;
        }
        case VX_KERNEL_NN_FULLY_CONNECTED_RELU_LAYER:
        {
            /* input w */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[0], 0); buffer++;
            /* input h */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[0], 1); buffer++;
            /* pad*/
            *buffer = SCALAR_VALUE(node->paramTable[2], u32); buffer++;
            /* enable relu */
            *buffer = SCALAR_VALUE(node->paramTable[7], u32); buffer++;
            /* output w */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[node->numParameters - 1], 0); buffer++;
            /* output h */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[node->numParameters - 1], 1);
            break;
        }
        case VX_KERNEL_POOLING_LAYER:
        {
            /* input w */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[0], 0); buffer++;
            /* input h */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[0], 1); buffer++;
            /* pool type */
            *buffer = SCALAR_VALUE(node->paramTable[1], u32); buffer++;
            /* pool size */
            *buffer = SCALAR_VALUE(node->paramTable[2], u32); buffer++;
            /* output w */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[node->numParameters - 1], 0); buffer++;
            /* output h */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[node->numParameters - 1], 1);
            break;
        }
        case VX_KERNEL_NN_POOLING_LAYER2:
        {
            /* input w */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[0], 0); buffer++;
            /* input h */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[0], 1); buffer++;
            /* pool type */
            *buffer = SCALAR_VALUE(node->paramTable[1], u32); buffer++;
            /* pool size */
            *buffer = SCALAR_VALUE(node->paramTable[2], u32); buffer++;
            /* output w */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[node->numParameters - 1], 0); buffer++;
            /* output h */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[node->numParameters - 1], 1);
            break;
        }
        case VX_KERNEL_ACTIVATION_LAYER:
        {
            /* input w */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[0], 0); buffer++;
            /* input h */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[0], 1); buffer++;
            /* input c */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[0], 3); buffer++;
            /* type */
            *buffer = SCALAR_VALUE(node->paramTable[1], u32); buffer++;
            /* output w */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[node->numParameters - 1], 0); buffer++;
            /* output h */
            *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[node->numParameters - 1], 1);
            break;
        }
        default:
        {
            if (node->paramTable[0]->type == VX_TYPE_TENSOR)
            {
                /* input w */
                *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[0], 0); buffer++;
                /* input h */
                *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[0], 1); buffer++;
                /* input c */
                *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[0], 2); buffer++;
                /* input n */
                *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[0], 3);
                if (node->paramTable[node->numParameters - 1]->type == VX_TYPE_TENSOR)
                {
                    buffer++;
                    /* output w */
                    *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[node->numParameters - 1], 0); buffer++;
                    /* output h */
                    *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[node->numParameters - 1], 1);
                }
            }
            else
            {
                if (node->paramTable[node->numParameters - 1]->type == VX_TYPE_TENSOR)
                {
                    /* output w */
                    *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[node->numParameters - 1], 0); buffer++;
                    /* output h */
                    *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[node->numParameters - 1], 1); buffer++;
                    /* output c */
                    *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[node->numParameters - 1], 2); buffer++;
                    /* output n */
                    *buffer = TENSOR_VIEW_SIZE_INDEX((vx_tensor)node->paramTable[node->numParameters - 1], 3);
                }
            }
            break;
        }
    }
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

/* remove unused binary file
   remove the binary file that hasn't been used for the longest time
   if the number of binary file is greater than BINARY_MAX_CACHE_NUMBER
*/
VX_PRIVATE_API vx_status vxoBinaryGraph_RemoveUnusedFile()
{
    #define BINARY_MAX_CACHE_NUMBER     5
    #define BINARY_MAX_CACHE_MAX_SIZE   1024 * 512 /* 512M */
    vx_status status = VX_SUCCESS;
    vx_char path[BINARY_FILE_NAME_MAX_SIZE] = {0};
    vx_char fileName[BINARY_FILE_NAME_MAX_SIZE] = {0};
    vx_char oldFileName[BINARY_FILE_NAME_MAX_SIZE] = {0};
    vx_char *env = VX_NULL;
    DIR *dir = VX_NULL;
    struct dirent *dirEnt = VX_NULL;
    struct stat statBuffer;
    vx_uint32 fileCount = 0;
    vx_uint64 accTime = 0;
    vx_uint64 changeTime = 0;
    vx_uint32 totalSize = 0, maxCacheSizeKB = BINARY_MAX_CACHE_MAX_SIZE;
    vx_uint32 maxCacheCount = BINARY_MAX_CACHE_NUMBER;
    vx_bool removeDone = vx_false_e;
    gcmHEADER();

    gcoOS_GetEnv(gcvNULL, "VIV_VX_CACHE_BINARY_GRAPH_MAX_SIZEKB", &env);
    if (env != VX_NULL)
    {
        maxCacheSizeKB = atoi(env);
    }

    env = VX_NULL;
    gcoOS_GetEnv(gcvNULL, "VIV_VX_CACHE_BINARY_GRAPH_MAX_COUNT", &env);
    if (env != VX_NULL)
    {
        maxCacheCount = atoi(env);
    }

    env = VX_NULL;
    gcoOS_GetEnv(gcvNULL, "VIV_VX_CACHE_BINARY_GRAPH_DIR", &env);
    if (VX_NULL == env)
    {
        gcoOS_StrCopySafe(path, BINARY_FILE_NAME_MAX_SIZE, "./");
    }
    else
    {
        gcoOS_StrCopySafe(path, BINARY_FILE_NAME_MAX_SIZE, env);
    }

    while (removeDone != vx_true_e)
    {
        fileCount = 0;
        totalSize = 0;
        accTime = 0;
        changeTime = 0;
        dir = opendir(path);
        if (dir == VX_NULL)
        {
            vxError("%s[%d]: fail to opendir %s\n", __FUNCTION__, __LINE__, path);
            vxmONERROR(VX_FAILURE);
        }

        while ((dirEnt = readdir(dir)) != VX_NULL)
        {
            #if (!defined(__QNX__))
            if (dirEnt->d_type == DT_REG)
            {
            #endif
                gctSTRING special = VX_NULL;
                gceSTATUS retStatus;

                gcoOS_StrCopySafe(fileName, BINARY_FILE_NAME_MAX_SIZE, path);
                gcoOS_StrCatSafe(fileName, BINARY_FILE_NAME_MAX_SIZE, "/");
                gcoOS_StrCatSafe(fileName, BINARY_FILE_NAME_MAX_SIZE, dirEnt->d_name);

                gcoOS_StrFindReverse(fileName, '.', &special);
                if (special == VX_NULL)
                {
                    continue;
                }
                retStatus = gcoOS_StrCmp(special, ".nb");
                if (gcvSTATUS_OK == retStatus)
                {
                    fileCount++;
                    INITIALIZE_STRUCT(statBuffer);
                    gcoOS_Stat(gcvNULL, fileName, &statBuffer);
                    totalSize += (vx_uint32)statBuffer.st_size;
                    if (((vx_uint64)statBuffer.st_atime < accTime) || (0 == accTime))
                    {
                        accTime = (vx_uint64)statBuffer.st_atime;
                        changeTime =  (vx_uint64)statBuffer.st_ctime;
                        gcoOS_StrCopySafe(oldFileName, BINARY_FILE_NAME_MAX_SIZE, fileName);
                    }
                    else if (((vx_uint64)statBuffer.st_atime == accTime) && ((vx_uint64)statBuffer.st_ctime < changeTime))
                    {
                        accTime = (vx_uint64)statBuffer.st_atime;
                        changeTime =  (vx_uint64)statBuffer.st_ctime;
                        gcoOS_StrCopySafe(oldFileName, BINARY_FILE_NAME_MAX_SIZE, fileName);
                    }
                }
            #if (!defined(__QNX__))
            }
            #endif
        }

        if ((fileCount >= maxCacheCount) || ((totalSize) / 1024 > maxCacheSizeKB))
        {
            /* remove the binary graph from cache directory */
            gcoOS_Remove(gcvNULL, oldFileName);
            vxInfo("remove binary graph file: %s, fileCount: %d, totalSize: %d\n", oldFileName, fileCount, totalSize);
        }
        else
        {
            removeDone = vx_true_e;
        }

        closedir(dir);
        dir = VX_NULL;
    }

    gcmFOOTER_ARG("%d", status);
    return status;

OnError:
    if (dir != VX_NULL)
    {
        closedir(dir);
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vx_uint8_ptr vxoBinaryGraph_GetToplogyParameters(
    vx_graph graph,
    vx_uint32 *infoSize
    )
{
    #define BYTES_PER_NODE     7
    /* describe graph toplogy information with BYTES_PER_NODE * sizeof(vx_uint32) bytes pre node:
    1. kernel enumeration(1) -- must*
    2. kernel size(1)  -- option    3. pad(1) -- option
    4. stride(1) -- option          5. poolSize(1) -- option
    6. input_w/h/c --option         7. output_w/h/c -- option */
    vx_uint32 size = graph->nodeCount * sizeof(vx_uint32) * BYTES_PER_NODE + sizeof(vx_uint32);
    /* sizeof(vx_uint32) is for target id */
    vx_uint32_ptr toplogyInfo = (vx_uint32_ptr)vxAllocate((vx_size)size);
    vx_uint32_ptr base = toplogyInfo;
    vx_node *nodeTable = graph->nodeTable;
    vx_int32 nodeCount = graph->nodeCount;
    vx_node node = VX_NULL;
    vx_int32 nodeIndex = 0;
    gcmHEADER_ARG("graph=%p, infoSize=%p", graph, infoSize);

    if (toplogyInfo == VX_NULL)
    {
        *infoSize = 0;
        gcmFOOTER_ARG("%p", toplogyInfo);
        return (vx_uint8_ptr)toplogyInfo;
    }

    /* add hardware target id to KEY */
    *toplogyInfo = graph->base.context->pid;
    toplogyInfo++;

    for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        node = nodeTable[graph->allNodeIndexTable[nodeIndex]];
        *toplogyInfo = (vx_uint32)node->kernel->enumeration; /* kernel enumeration */
        toplogyInfo++;

        vxoBinaryGraph_GetNodeParametersValue(node, &toplogyInfo);
        toplogyInfo += BYTES_PER_NODE - 1;
    }
    *infoSize = size;
    gcmFOOTER_ARG("%p", base);
    return (vx_uint8_ptr)base;
}

VX_PRIVATE_API vx_bool vxoBinaryGraph_SearchInSystem(
    vx_char *binaryFile
)
{
    vx_bool findFile = vx_false_e;
    gctFILE *pF = VX_NULL;
    vx_char *env = VX_NULL;
    vx_char temp[BINARY_FILE_NAME_MAX_SIZE] ={'0'};
    gcmHEADER_ARG("binaryFile=%p", binaryFile);

    if (binaryFile == VX_NULL)
    {
        vxError("vxoBinaryGraph_SearchInSystem binaryFile is NULL\n");
        gcmFOOTER_ARG("%d", findFile);
        return vx_false_e;
    }

    gcoOS_StrCopySafe(temp, BINARY_FILE_NAME_MAX_SIZE, binaryFile);
    gcoOS_GetEnv(gcvNULL, "VIV_VX_CACHE_BINARY_GRAPH_DIR", &env);
    if (env)
    {
        gcoOS_StrCopySafe(binaryFile, BINARY_FILE_NAME_MAX_SIZE, env);
        gcoOS_StrCatSafe(binaryFile, BINARY_FILE_NAME_MAX_SIZE, "/");
        gcoOS_StrCatSafe(binaryFile, BINARY_FILE_NAME_MAX_SIZE, temp);
    }

    gcoOS_Open(gcvNULL, binaryFile, gcvFILE_READ, (gctFILE *)&pF);
    if (pF != VX_NULL)
    {
        findFile = vx_true_e;
        gcoOS_Close(gcvNULL, pF);

        /* update file timestamp*/
        utime(binaryFile, VX_NULL);
    }
    gcmFOOTER_ARG("%d", findFile);
    return findFile;
}

VX_PRIVATE_API vx_bool vxoBinaryGraph_RemoveOneFile(
    vx_char *binaryFile
)
{
    gctFILE *pF = VX_NULL;
    gcmHEADER_ARG("binaryFile=%p", binaryFile);
    gcoOS_Open(gcvNULL, binaryFile, gcvFILE_READ, (gctFILE *)&pF);
    if (pF != VX_NULL)
    {
        gcoOS_Close(gcvNULL, pF);
        gcoOS_Remove(gcvNULL, binaryFile);
    }
    gcmFOOTER_NO();
    return vx_true_e;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_SetInputOutput(
    vx_graph graph,
    vx_binary_loader_s *binaryLoad,
    vx_node node
    )
{
    vx_status status = VX_SUCCESS;
    vx_reference *inputTable = VX_NULL;
    vx_reference *outputTable = VX_NULL;
    vx_uint32 *ioPhysical = VX_NULL;
    vx_uint32 inputNum = 0, outputNum = 0;
    vx_uint32 i = 0, j = 0, k = 0;
    vx_uint32 ioNum = 0;

    gcmHEADER_ARG("graph=%p", graph);

    if ((graph->inputCount == 0) || (graph->outputCount == 0))
    {
        inputTable = (vx_reference*)vxAllocateAndZeroMemory(VX_MAX_NN_INOUT_PARAM_COUNT * sizeof(vx_reference) );
        if (inputTable == VX_NULL)
        {
            vxError("%s[%d]: failed to allocate memory for inputTable\n", __FUNCTION__, __LINE__);
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }
        outputTable = (vx_reference*)vxAllocateAndZeroMemory(VX_MAX_NN_INOUT_PARAM_COUNT * sizeof(vx_reference) );
        if (outputTable == VX_NULL)
        {
            vxError("%s[%d]: failed to allocate memory for outputTable\n", __FUNCTION__, __LINE__);
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }
        ioPhysical = (vx_uint32*)vxAllocateAndZeroMemory(VX_MAX_NN_INOUT_PARAM_COUNT * sizeof(vx_uint32) );
        if (ioPhysical == VX_NULL)
        {
            vxError("%s[%d]: failed to allocate memory for ioPhysical\n", __FUNCTION__, __LINE__);
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }

        vxmONERROR(vxoBinaryGraph_InputsOutputs(graph, inputTable, &inputNum, outputTable, &outputNum));
        if ((inputNum == 0) || (outputNum == 0))
        {
            vxError("fail to get input/output table inputNum: %d, outputNum: %d \n", inputNum, outputNum);
            vxmONERROR(VX_FAILURE);
        }
    }

    /* set input for node */
    if (graph->inputCount && graph->inputs)
    {
        for (i = 0; i < graph->inputCount; i++)
        {
            vxSetParameterByIndex(node, i, graph->inputs[i]);
            node->numParameters++;
        }

        if (graph->inputCount != binaryLoad->fixed.header.inputCount)
        {
            vxError("%s[%d]: binary graph no output error, %d\n", __FUNCTION__, __LINE__, graph->inputCount);
            vxmONERROR(VX_FAILURE);
        }
    }
    else
    {
        for (i = 0; i < binaryLoad->fixed.header.inputCount; i++)
        {
            vx_binary_input_output_info_s *ptrInOut = (vx_binary_input_output_info_s *)vxoBinaryGraph_GetInputOutputPtrByIndex(binaryLoad, binaryLoad->inputs, i);
            vx_binary_input_output_info_s *input = &ptrInOut[i];

            for (j = 0; j < inputNum; j++)
            {
                vx_bool dupFlag = vx_false_e;
                if (inputTable[j] == VX_NULL)
                {
                    vxError("%s[%d]: failed, inputTable[%d] is NULL\n", __FUNCTION__, __LINE__, j);
                    vxmONERROR(VX_FAILURE);
                }
                if ((input->dataType == VX_BINARY_BUFFER_TYPE_TENSOR) && (inputTable[j]->type == VX_TYPE_TENSOR))
                {
                    vx_tensor tensor = (vx_tensor)inputTable[j];
                    vx_int32 *dims = TENSOR_ORIG_SIZES(tensor);
                    vx_uint32 dimCount = TENSOR_ORIG_DIM_NUM(tensor);
                    for (k = 0; k < ioNum; k++)
                    {
                        if ((TENSOR_PHYSICAL_ADDR(tensor) == ioPhysical[k]) && (TENSOR_PHYSICAL_ADDR(tensor)))
                        {
                            dupFlag = vx_true_e;
                            break;
                        }
                    }
                    if ((input->dataFormat == vxoBinaryGraph_ConvertToBinaryBufferFormat(TENSOR_DATA_TYPE(tensor))) &&
                        (input->dimCount == dimCount) && (input->quantFormat == TENSOR_QUANT_TYPE(tensor)) &&
                        ((vx_int32)input->dims[0] == dims[0]) && ((vx_int32)input->dims[1] == dims[1]) &&
                        ((vx_int32)input->dims[2] == dims[2]) && ((vx_int32)input->dims[3] == dims[3]) &&
                        (dupFlag == vx_false_e))
                    {
                        vxSetParameterByIndex(node, i, inputTable[j]);
                        node->numParameters++;
                        ioPhysical[ioNum++] = TENSOR_PHYSICAL_ADDR(tensor);
                        break;
                    }
                }
                else if ((input->dataType == VX_BINARY_BUFFER_TYPE_SCALAR) && (inputTable[j]->type == VX_TYPE_SCALAR))
                {
                    vx_scalar scalar = (vx_scalar)inputTable[j];
                    for (k = 0; k < ioNum; k++)
                    {
                        if (scalar->physical == ioPhysical[k])
                        {
                            dupFlag = vx_true_e;
                            break;
                        }
                    }
                    if ((input->dataFormat == vxoBinaryGraph_ConvertToBinaryBufferFormat(VX_TYPE_INT8)) &&
                        (input->dims[0] == vxoScalar_GetTypeSize(scalar)) &&
                        (input->dims[1] == 1) &&
                        (dupFlag == vx_false_e))
                    {
                        vxSetParameterByIndex(node, i, inputTable[j]);
                        node->numParameters++;
                        ioPhysical[ioNum++] = scalar->physical;
                        break;
                    }
                }
                else if ((input->dataType == VX_BINARY_BUFFER_TYPE_IMAGE) && (inputTable[j]->type == VX_TYPE_IMAGE))
                {
                    vx_image image = (vx_image)inputTable[j];
                    gcoVX_Kernel_Context kernelContext; /* not useful, just fulfill the interface */
                    gcsVX_IMAGE_INFO imageInfo;
                    vx_df_image format;

                    INITIALIZE_STRUCT(imageInfo);
                    gcoOS_MemFill((gctPOINTER*)(&kernelContext), 0, sizeof(gcoVX_Kernel_Context));
                    gcfVX_GetImageInfo(&kernelContext, image, &imageInfo, 1);
                    vxQueryImage(image, VX_IMAGE_FORMAT, &format, sizeof(format));

                    for (k = 0; k < ioNum; k++)
                    {
                        if (image->memory.physicals[0] == ioPhysical[k])
                        {
                            dupFlag = vx_true_e;
                            break;
                        }
                    }

                    if (1 == image->planeCount)
                    {
                        if ((input->dimCount == 2) && (input->dataFormat ==  vxoBinaryGraph_ConvertToBinaryBufferFormat((vx_uint32)format)) &&
                            (input->dims[0] == imageInfo.width) && (input->dims[1] == imageInfo.height) && (input->dims[2] == 1) &&
                            (input->quantFormat  == VX_BINARY_BUFFER_QUANT_FORMAT_NONE) &&
                            (dupFlag == vx_false_e))
                        {
                            vxSetParameterByIndex(node, i, inputTable[j]);
                            node->numParameters++;
                            ioPhysical[ioNum++] = image->memory.physicals[0];
                            break;
                        }
                    }
                    else if (3 == image->planeCount)
                    {
                        if ((input->dimCount == 3) && (input->dataFormat ==  vxoBinaryGraph_ConvertToBinaryBufferFormat((vx_uint32)format)) &&
                            (input->dims[0] == imageInfo.width) && (input->dims[1] == imageInfo.height) &&
                            (input->quantFormat  == VX_BINARY_BUFFER_QUANT_FORMAT_NONE) &&
                            (dupFlag == vx_false_e))
                        {
                            vxSetParameterByIndex(node, i, inputTable[j]);
                            node->numParameters++;
                            ioPhysical[ioNum++] = image->memory.physicals[0];
                            break;
                        }
                    }
                }
            }
        }

        if (node->numParameters != binaryLoad->fixed.header.inputCount)
        {
            vxError("%s[%d]: error: binary graph no input numParameters: %d\n",
                    __FUNCTION__, __LINE__, node->numParameters);
            vxmONERROR(VX_FAILURE);
        }
    }

    for (i = 0; i < ioNum; i++)
    {
        ioPhysical[i] = 0; /* clear input physical address */
    }
    ioNum = 0;

    /* set output for node */
    if (graph->outputCount && graph->outputs)
    {
        for (i = 0; i < graph->outputCount; i++)
        {
            vxSetParameterByIndex(node, binaryLoad->fixed.header.inputCount + i, graph->outputs[i]);
            node->numParameters++;
        }

        if (graph->outputCount != binaryLoad->fixed.header.outputCount)
        {
            vxError("%s[%d]: binary graph no output error, %d\n", __FUNCTION__, __LINE__, graph->outputCount);
            vxmONERROR(VX_FAILURE);
        }
    }
    else
    {
        for (i = 0; i < binaryLoad->fixed.header.outputCount; i++)
        {
            vx_binary_input_output_info_s * ptrInOut = (vx_binary_input_output_info_s *)vxoBinaryGraph_GetInputOutputPtrByIndex(binaryLoad, binaryLoad->outputs, i);
            vx_binary_input_output_info_s *output = &ptrInOut[i];
            for (j = 0; j < outputNum; j++)
            {
                vx_bool dupFlag = vx_false_e;
                if (outputTable[j] == VX_NULL)
                {
                    vxError("%s[%d]: failed, outputTable[%d] is NULL\n", __FUNCTION__, __LINE__, j);
                    vxmONERROR(VX_FAILURE);
                }
                if ((output->dataType == VX_BINARY_BUFFER_TYPE_TENSOR) && (outputTable[j]->type == VX_TYPE_TENSOR))
                {
                    vx_tensor tensor = (vx_tensor)outputTable[j];
                    vx_int32 *dims = TENSOR_ORIG_SIZES(tensor);
                    vx_uint32 dimCount = TENSOR_ORIG_DIM_NUM(tensor);
                    for (k = 0; k < ioNum; k++)
                    {
                        if ((TENSOR_PHYSICAL_ADDR(tensor) == ioPhysical[k]) && (TENSOR_PHYSICAL_ADDR(tensor)))
                        {
                            dupFlag = vx_true_e;
                            break;
                        }
                    }
                    if ((output->dataFormat == vxoBinaryGraph_ConvertToBinaryBufferFormat(TENSOR_DATA_TYPE(tensor))) &&
                        (output->dimCount == dimCount) && (output->quantFormat == TENSOR_QUANT_TYPE(tensor)) &&
                        ((vx_int32)output->dims[0] == dims[0]) && ((vx_int32)output->dims[1] == dims[1]) &&
                        ((vx_int32)output->dims[2] == dims[2]) && ((vx_int32)output->dims[3] == dims[3]) &&
                        (dupFlag == vx_false_e))
                    {
                        vxSetParameterByIndex(node, binaryLoad->fixed.header.inputCount + i, outputTable[j]);
                        node->numParameters++;
                        ioPhysical[ioNum++] = TENSOR_PHYSICAL_ADDR(tensor);
                        break;
                    }
                }
                else if ((output->dataType == VX_BINARY_BUFFER_TYPE_IMAGE) && (inputTable[j]->type == VX_TYPE_IMAGE))
                {
                    vx_image image = (vx_image)inputTable[j];
                    gcoVX_Kernel_Context kernelContext; /* not useful, just fulfill the interface */
                    gcsVX_IMAGE_INFO imageInfo;
                    vx_df_image format;

                    INITIALIZE_STRUCT(imageInfo);
                    gcoOS_MemFill((gctPOINTER*)(&kernelContext), 0, sizeof(gcoVX_Kernel_Context));
                    gcfVX_GetImageInfo(&kernelContext, image, &imageInfo, 1);
                    vxQueryImage(image, VX_IMAGE_FORMAT, &format, sizeof(format));

                    for (k = 0; k < ioNum; k++)
                    {
                        if (image->memory.physicals[0] == ioPhysical[k])
                        {
                            dupFlag = vx_true_e;
                            break;
                        }
                    }
                    if (1 == image->planeCount)
                    {
                        if ((output->dimCount == 2) && (output->dataFormat ==  vxoBinaryGraph_ConvertToBinaryBufferFormat((vx_uint32)format)) &&
                            (output->dims[0] == imageInfo.width) && (output->dims[1] == imageInfo.height) &&
                            (output->quantFormat  == VX_BINARY_BUFFER_QUANT_FORMAT_NONE) &&
                            (dupFlag == vx_false_e))
                        {
                            vxSetParameterByIndex(node, binaryLoad->fixed.header.inputCount + i, outputTable[j]);
                            node->numParameters++;
                            ioPhysical[ioNum++] = image->memory.physicals[0];
                            break;
                        }
                    }
                    else if (3 == image->planeCount)
                    {
                        if ((output->dimCount == 3) && (output->dataFormat ==  vxoBinaryGraph_ConvertToBinaryBufferFormat((vx_uint32)format)) &&
                            (output->dims[0] == imageInfo.width) && (output->dims[1] == imageInfo.height) &&
                            (output->quantFormat  == VX_BINARY_BUFFER_QUANT_FORMAT_NONE) &&
                            (dupFlag == vx_false_e))
                        {
                            vxSetParameterByIndex(node, binaryLoad->fixed.header.inputCount + i, outputTable[j]);
                            node->numParameters++;
                            ioPhysical[ioNum++] = image->memory.physicals[0];
                            break;
                        }
                    }
                }
                else
                {
                    vxError("binary graph cann't support this data type as output, %d\n", inputTable[j]->type);
                    vxmONERROR(VX_FAILURE);
                }
            }
        }
    }

    if (node->numParameters != (binaryLoad->fixed.header.inputCount + binaryLoad->fixed.header.outputCount))
    {
        vxError("%s[%d]: binary graph no output error, %d\n", __FUNCTION__, __LINE__, node->numParameters);
        vxmONERROR(VX_FAILURE);
    }

OnError:
    if (inputTable != VX_NULL)
    {
        vxFree(inputTable);
    }
    if (outputTable != VX_NULL)
    {
        vxFree(outputTable);
    }
    if (ioPhysical != VX_NULL)
    {
        vxFree(ioPhysical);
    }

    gcmFOOTER_ARG("status=%d", status);
    return status;
}

VX_INTERNAL_API void vxoBinaryGraph_ReleaseCache(
    vx_graph graph
    )
{
    vx_uint32 nodeIndex = 0;
    vx_binary_loader_s *binaryLoad = NULL;

    gcmHEADER_ARG("graph=%p", graph);

    if (graph == VX_NULL)
    {
        gcmFOOTER_NO();
        return;
    }

    if ((graph->base.context->options.enableSaveBinary) ||
        (graph->base.context->options.enableNNLayerDump) ||
        (0 == graph->base.context->options.enableCacheBinaryGraph))
    {
        gcmFOOTER_NO();
        return;
    }

    for (nodeIndex = 0; nodeIndex < graph->nodeCount; nodeIndex++)
    {
        vx_node node = graph->nodeTable[nodeIndex];
        vx_kernel kernel = node->kernel;
         binaryLoad = (vx_binary_loader_s*)kernel->base.reserved;
        if ((kernel->enumeration == VX_KERNEL_IMPORT_FROM_FILE) &&
            (binaryLoad != VX_NULL))
        {
            vxoBinaryGraph_ReleaseNBG(binaryLoad);
        }
    }

    gcmFOOTER_NO();

    return;
}

/* cache binary graph to VIV_VX_CACHE_BINARY_GRAPH_DIR
   or import binary graph from VIV_VX_CACHE_BINARY_GRAPH_DIR
*/
VX_INTERNAL_API void vxoBinaryGraph_CacheOrImport(
    vx_graph graph
    )
{
    #define KEY_LENGTH_BYTE      16
    vx_node *nodeTable = graph->nodeTable;
    vx_context context =graph->base.context;
    vx_tensor weight = VX_NULL;
    vx_int32 nodeIndex = 0;
    vx_status status = VX_SUCCESS;
    vx_uint8_ptr weightLogical = VX_NULL, toplogyInfo = VX_NULL;
    vx_uint32 weightSize = 0, toplogyInfoSize = 0, i =0;
    vx_char binaryName[BINARY_FILE_NAME_MAX_SIZE] = {'0'};
    vx_char *name = binaryName;
    vx_char *base = name;
    vx_kernel kernel = VX_NULL;
    vx_node node = VX_NULL;
    vx_binary_loader_s *binaryLoad = NULL;
    vx_uint32 nodeCount = graph->nodeCount;
    gcsHASH_MD5CTX md5Ctx;
    vx_uint8 md5digest[KEY_LENGTH_BYTE];
    gctSTRING envctrl = gcvNULL;
    gceSTATUS mcfeEnabled = (gcvSTATUS_TRUE == gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_MCFE));
    gcmHEADER_ARG("graph=%p", graph);

    /* get env for nn api path */
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_CACHE_GRAPH_BINARY", &envctrl)) && envctrl)
    {
        context->options.enableCacheBinaryGraph = atoi(envctrl);
    }

    if ((graph->base.context->options.enableSaveBinary) ||
        (graph->base.context->options.enableNNLayerDump) ||
        (graph->base.context->options.enableCNNPerf) ||
         /* NBG doesn't support MCFE hardware */
         mcfeEnabled ||
        (0 == graph->base.context->options.enableCacheBinaryGraph))
    {
        gcmFOOTER_NO();
        return;
    }

    if (vx_true_e == vxoBinaryGraph_HasBinaryInGraph(graph))
    {
        /* bypass the network if it import from binary graph */
        gcmFOOTER_NO();
        return;
    }

    INITIALIZE_STRUCT(md5Ctx);

    /* get graph toplogy and nodes parameter info */
    toplogyInfo = vxoBinaryGraph_GetToplogyParameters(graph, &toplogyInfoSize);
    if ((toplogyInfoSize <= 0) || (toplogyInfo == VX_NULL))
    {
        vxError("%s[%d]: fail to get toplogy parameters information\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_FAILURE);
    }

    vxmONERROR(vxoBinaryGraph_FindNodeIndexForWeight(graph, &nodeIndex));
    if (nodeIndex >= 0)
    {
        vx_size size = 0;
        vx_node weightNode = graph->nodeTable[graph->allNodeIndexTable[nodeIndex]];
        weight = vxoBinaryGraph_GetWeightTensorFromNode(weightNode);
        vxmONERROR_NULLPTR(weight);

        weightLogical = TENSOR_LOGICAL_ADDR(weight);
        vxoTensor_GetTensorWholeSize(weight, &size);
        weightSize = (vx_uint32)size;
        if ((weightSize <= 0) || (weightLogical == VX_NULL))
        {
            /* no weight data in weight_bias_parameter, create from vData? */
            vxError("%s[%d]: no weight data in weight_bias_parameter\n", __FUNCTION__, __LINE__);
            vxmONERROR(VX_FAILURE);
        }
    }
    else
    {
        vxInfo("can't find weight in this network\n");
    }

    /* generate a key */
    gcsHASH_MD5Init(&md5Ctx);
    gcsHASH_MD5Update(&md5Ctx, toplogyInfo, toplogyInfoSize);
    if (weightLogical != VX_NULL) {
        gcsHASH_MD5Update(&md5Ctx, weightLogical, weightSize);
    }
    gcsHASH_MD5Final(&md5Ctx, md5digest);

    for (i = 0; i < KEY_LENGTH_BYTE; i++)
    {
        sprintf(name, "%2.2x", md5digest[i]);
        name += 2;
    }
    gcoOS_StrCatSafe(base, KEY_LENGTH_BYTE * 2 + 4, ".nb");

    /* search this binary name in ENV  VIV_VX_CACHE_BINARY_GRAPH_DIR */
    if (vxoBinaryGraph_SearchInSystem(binaryName))
    {
        vxInfo("loading binary graph to run...\n");
        vxInfo("binary path: %s\n", binaryName);
    }
    else
    {
        /* binary doesn't exist, generate binary graph */
        vxInfo("generate binary graph: %s\n", binaryName);
        goto SaveBin;
    }

    /* load binary graph file and create vx_kernel */
    kernel = vxImportKernelFromURL(context, VX_VIVANTE_IMPORT_KERNEL_FROM_FILE, binaryName);
    vxmONERROR_NULLPTR(kernel);
    binaryLoad = (vx_binary_loader_s*)kernel->base.reserved;

    /* create vx_node for the binary graph kernel */
    node = vxCreateGenericNode(graph, kernel);
    vxmONERROR_NULLPTR(node);

    vxmONERROR(vxoBinaryGraph_SetInputOutput(graph, binaryLoad, node));

    for (i = 0; i < nodeCount; i++)
    {
        vxoNode_RemoveFromGraph(&nodeTable[0]);
    }

    /* sanity check */
    if ((vx_false_e == vxoBinaryGraph_HasBinaryInGraph(graph)) || (graph->nodeCount > 1))
    {
        vxError("%s[%d]: remove node fail: %d\n", __FUNCTION__, __LINE__, graph->nodeCount);
        vxmONERROR(VX_FAILURE);
    }

    if (toplogyInfo != VX_NULL)
    {
        vxFree(toplogyInfo);
        toplogyInfo = VX_NULL;
    }
    gcmFOOTER_NO();
    return;

SaveBin:
    vxoBinaryGraph_RemoveUnusedFile();

    if (gcoOS_StrCmp(binaryName, "\0") == gcvSTATUS_OK)
    {
        vxError("%s[%d]: bianry name is NULL\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_FAILURE);
    }

    /* open binaryName file for saving binary */
    if ((vxoBinaryGraph_Initialize(graph, binaryName)) != VX_SUCCESS)
    {
        vxError("%s[%d]: fail to initial binary graph\n", __FUNCTION__, __LINE__);
    }

    if (toplogyInfo != VX_NULL)
    {
         vxFree(toplogyInfo);
         toplogyInfo = VX_NULL;
    }
    gcmFOOTER_NO();
    return;

OnError:
    if (gcoOS_StrCmp(binaryName, "\0") != gcvSTATUS_OK)
    {
        /* fail to load binary, remove it */
        vxoBinaryGraph_RemoveOneFile(binaryName);
    }

    if (kernel != NULL)
    {
        vxReleaseKernel(&kernel);
        kernel = NULL;
    }

    if (node != VX_NULL)
    {
        vxoNode_RemoveFromGraph(&node);
    }

    if (toplogyInfo != VX_NULL)
    {
         vxFree(toplogyInfo);
         toplogyInfo = VX_NULL;
    }
    gcmFOOTER_NO();
    return;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_GetSHParamSize(
    vx_graph graph,
    vx_reference *shParams,
    vx_uint32 paramNum,
    vx_uint32 **lcdTensorPhy,
    vx_uint32 *lcdTensorsPhyIndex,
    vx_uint32 *size
    )
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i = 0, j =0;
    vx_uint32 totalSize = 0;
    vx_enum allocType = VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR;
    vx_uint32 *tensorPhy = *lcdTensorPhy;
    vx_uint32 index = *lcdTensorsPhyIndex;

    for (i = 0; i < paramNum; i++)
    {
        vx_reference ref = shParams[i];

        /* is network's input */
        for (j = 0; j < graph->inputCount; j++)
        {
            if (ref == graph->inputs[j])
            {
                continue;
            }
        }
        if (j != graph->inputCount) continue;

        /* is network's output */
        for (j = 0; j < graph->outputCount; j++)
        {
            if (ref == graph->outputs[j])
            {
                continue;
            }
        }
        if (j != graph->inputCount) continue;

        if (ref->type == VX_TYPE_IMAGE)
        {
            vx_image image = (vx_image)ref;
            allocType = vxoMemory_GetType(&image->memory);
            if (allocType == VXNNE_MEM_POOL_TYPE_ORIG_DDR)
            {
                gcsVX_IMAGE_INFO imageInfo;
                gcoVX_Kernel_Context kernelContext; /* not useful, just fulfill the interface */
                INITIALIZE_STRUCT(imageInfo);

                gcoOS_MemFill((gctPOINTER*)(&kernelContext), 0, sizeof(gcoVX_Kernel_Context));
                gcfVX_GetImageInfo(&kernelContext, image, &imageInfo, 1);
                totalSize += gcmALIGN((vx_uint32)imageInfo.bytes, 64);
            }
        }
        else if (ref->type == VX_TYPE_ARRAY)
        {
            vx_array array = (vx_array)ref;
            allocType = vxoMemory_GetType(&array->memory);
            if (allocType == VXNNE_MEM_POOL_TYPE_ORIG_DDR)
            {
                totalSize += gcmALIGN((vx_uint32)array->memory.sizes[0], 64);
            }
        }
        else if (ref->type == VX_TYPE_SCALAR)
        {
            totalSize += 64;/* alignement to 64 bytes */
        }
        else if (ref->type == VX_TYPE_TENSOR)
        {
            vx_tensor tensor = (vx_tensor)ref;
            vx_enum allocType = vxoMemory_GetType(&tensor->tensorBuffer->memory);

            if (allocType == VXNNE_MEM_POOL_TYPE_ORIG_DDR)
            {
                vx_uint32 physical = TENSOR_PHYSICAL_ADDR(tensor);
                vx_uint32 i = 0;

                for (i = 0; i < index; i++)
                {
                    if (physical == tensorPhy[i])
                    {
                        break;
                    }
                }

                if (i == index)
                {
                    vx_size size_tensor = 0;
                    vx_uint32 size_temp = 0;
                    vxoTensor_GetTensorWholeSize(tensor, &size_tensor);
                    size_temp = gcmALIGN(size_temp, 64);

                    totalSize += size_temp;

                    tensorPhy[index] = physical;
                    index++;
                    *lcdTensorsPhyIndex = index;
                }
            }
        }
    }

    *size = totalSize;

    return status;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_GetShaderStatesSize(
    vx_graph graph,
    vx_shader kernelShader,
    vx_uint32 *size
    )
{
    vx_status status = VX_SUCCESS;
    vx_argument args = VX_NULL;
    vx_uint32 numArgs = 0;
    vx_uint32 i = 0;
    vx_uint32 uniformNum = 0;
    vx_uint32 statesSize = 0;
    gctUINT32 gpuCount = 1;

    if ((kernelShader == VX_NULL) || (graph == VX_NULL) || (size == VX_NULL))
    {
        vxError("%s[%d]: kernelShader/graph/size is NULL\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_INVALID_VALUE);
    }

    args = kernelShader->args;
    numArgs = (vx_uint32)kernelShader->numArgs;

    for (i = 0; i < numArgs; i++)
    {
        if (args[i].uniform && !isUniformInactive(args[i].uniform))
        {
            uniformNum += 2;
        }
    }

    statesSize = uniformNum * 24; /* 6 * sizeof(UINT32) */

    statesSize += kernelShader->states.programState.stateBufferSize;

    statesSize += 120; /* gcoHARDWARE_FlushShaders + gcoHARDWARE_FlushPrefetchInst */

    gcmONERROR(gcoVX_QueryCoreCount(graph->deviceID, &gpuCount));

    statesSize += gpuCount * 120; /* for gcoHARDWARE_InvokeThreadWalkerCL() for(i = 0; i < usedGPUCount; i++) */

    statesSize += 4 * 25;

    statesSize = gcmALIGN_NP2_SAFE(statesSize, 64);

    *size = statesSize;

OnError:
    return status;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_GetSectionsSize(
    vx_graph graph,
    vx_uint32 *sectionSize,
    vx_uint32 *retLcdSize
    )
{
    #define NN_STATES_SIZE          0X40 /* alignment to 64bytes */
    #define TP_STATES_SIZE          0X40
    #define SC_STATES_SIZE          0X40
    vx_status status = VX_SUCCESS;
    vx_context context = graph->base.context;
    vxnne_execution_layer executionLayer = graph->layer;
    vx_uint32 i = 0;
    vxnne_operation_command operationCommand = VX_NULL;
    vxnne_operation operation = VX_NULL;
    vx_node node = VX_NULL;
    vx_uint32 KernelStreamSize = 0;
    vx_uint32 operationCount = 0;
    vx_uint32 nnCount = 0;
    vx_uint32 tpCount = 0;
    vx_uint32 shCount = 0;
    vx_uint32 swCount = 0;
    vx_uint32 scCount = 0;
    vx_uint32 layerParamCount = 0;
    vx_uint32 patchCount = 0;
    vx_uint32 lcdtCount = 0;
    vx_uint32 operationSize = 0;
    vx_uint32 lcdtSize = 0;
    vx_uint32 nnSize = 0;
    vx_uint32 tpSize = 0;
    vx_uint32 shSize = 0;
    vx_uint32 swSize = 0;
    vx_uint32 layerParamSize = 0;
    vx_uint32 patchSize = 0;
    vx_uint32 totalSize = 0;
    vx_uint32 lcdSize = 0;
    vx_uint32 statesSize = 0;
    vx_uint32 shIntrSize = 0;
    vx_uint32 shShareMemSize = 0;
    vx_uint32 shParaSize = 0;
    vx_uint32 *lcdTensorsPhysical = VX_NULL;
    vx_uint32 *lcdKernelDataPhysical = VX_NULL;
    vx_uint32 lcdTensorsPhyIndex = 0;
    vx_uint32 lcdKernelDataPhyIndex = 0;
    vx_uint32 lcdTensorSize = 0;
    vx_uint32 shaderStatesSize = 0;
    vx_uint32 lcdKernelDataNum = 0;

    for (i = 0; i < executionLayer->opIndicesNum; i++)
    {
        operation = executionLayer->operations[executionLayer->opIndices[i].operationID];
        operationCommand = &executionLayer->opIndices[i];
        if (operation->target == VXNNE_OPERATION_TARGET_NN)
        {
            lcdKernelDataNum++;
        }
        else if (operation->target == VXNNE_OPERATION_TARGET_TP)
        {
            vx_uint32 j = 0;
            vx_op_param parameter = &operationCommand->operation->parameter;
            if (parameter->data_buff != VX_NULL)
            {
                lcdKernelDataNum++;
            }
            else if (parameter->tpType == TP_SINGLE_FC && parameter->other_ref != VX_NULL)
            {
                vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter)parameter->other_ref;

                for (j = 0; j < WB_TOTAL_SLICE_NUM(weights_biases); j++)
                {
                    lcdKernelDataNum++;
                }
             }
        }
    }

    lcdTensorsPhysical = (vx_uint32*)vxAllocateAndZeroMemory(sizeof(vx_uint32) * VX_MAX_TEMP_TENSORS);
    lcdKernelDataPhysical = (vx_uint32*)vxAllocateAndZeroMemory(sizeof(vx_uint32) * (lcdKernelDataNum + 50));

    patchCount += 10;
    operationCount = 4; /* initialize and end */
    patchCount += 4; /* SRAM */
    lcdtCount = operationCount;
    statesSize += 128;
    statesSize += 0X40;

    for (i = 0; i < executionLayer->opIndicesNum; i++)
    {
        vx_tensor input = VX_NULL;
        vx_tensor output = VX_NULL;

        operation = executionLayer->operations[executionLayer->opIndices[i].operationID];
        operationCommand = &executionLayer->opIndices[i];
        node = operation->layer->node;

        if (operation->target == VXNNE_OPERATION_TARGET_NN)
        {
            vx_weights_biases_parameter weights_biases = operationCommand->cmdInfo.wb;
            vxnne_convolution_relu_pooling_operation nnConvOperation = (vxnne_convolution_relu_pooling_operation)operation;
            vx_uint32 ksDataSize = 0;
            vx_uint32 tempCount = 1;
            vx_uint32 ksDataPhysical = 0;
            vx_uint32 cnt = 0;

            tempCount = vxoBinaryGraph_CalculateNNSliceCount(context, operationCommand);
            if (tempCount > context->nnConfig.fixedFeature.nnCoreCount)
            {
                tempCount = context->nnConfig.fixedFeature.nnCoreCount;
            }

            vxmASSERT(weights_biases != VX_NULL);

            ksDataPhysical = (vx_uint32)WB_MEM_PHYSICAL_ADDR_INDEX(weights_biases, 0);
            ksDataSize = (vx_uint32)WB_MEM_SIZE_INDEX(weights_biases, 0);
            ksDataSize = gcmALIGN_NP2_SAFE(ksDataSize, 64);
            if (0 == ksDataSize)
            {
                vxError("fail to get NBG size, NN kernel stream size is 0\n");
                vxmONERROR(VX_ERROR_INVALID_VALUE);
            }

            for (cnt = 0; cnt < lcdKernelDataPhyIndex; cnt++)
            {
                if (lcdKernelDataPhysical[cnt] == ksDataPhysical)
                {
                    break;
                }
            }

            if (cnt == lcdKernelDataPhyIndex)
            {
                /*vxInfo("get NBG size nn KS physcial: 0x%08xm size: 0x%08x \n", ksDataPhysical, ksDataSize);*/
                KernelStreamSize += ksDataSize;
                lcdKernelDataPhysical[lcdKernelDataPhyIndex] = ksDataPhysical;
                lcdKernelDataPhyIndex++;
            }

            operationCount += tempCount;
            nnCount += tempCount;
            lcdtCount += tempCount * 3;

            input = nnConvOperation->inputs;
            output = nnConvOperation->outputs;

            patchCount += tempCount * 5;
            if ((operationCommand->inputTile.circularBufEndAddrPlus1 != 0xFFFFFFFF) &&
                (operationCommand->inputTile.circularBufEndAddrPlus1 != 0) &&
                (operationCommand->inputTile.circularBufEndAddrPlus1 != 0xCDCDCDCD))
            {
                patchCount += tempCount;
            }
            if ((operationCommand->outputTile.circularBufEndAddrPlus1 != 0xFFFFFFFF) &&
                (operationCommand->outputTile.circularBufEndAddrPlus1 != 0) &&
                (operationCommand->outputTile.circularBufEndAddrPlus1 != 0xCDCDCDCD))
            {
                patchCount += tempCount;
            }
        }
        else if (operation->target == VXNNE_OPERATION_TARGET_TP)
        {
            vx_op_param parameter = &operationCommand->operation->parameter;
            vx_uint32 ksDataSize = 0;
            vx_uint32 tempCount = context->nnConfig.fixedFeature.tpCoreCount + context->nnConfig.fixedFeature.tpliteCoreCount;
            vxnne_tp_operation tpOperation = (vxnne_tp_operation)operation;
            vx_uint32 ksDataPhysical = 0;
            vx_uint32 cnt = 0;

            if (operation->parameter.tpType == TP_SINGLE_FC)
            {
                vx_tensor otherRef = (vx_tensor)operation->parameter.other_ref;
                vx_tp_value_cmd value = operation->parameter.tp_value;
                if (!value->e32[0])
                {
                    vxmASSERT(otherRef != VX_NULL);
                    tempCount = WB_TOTAL_SLICE_NUM((vx_weights_biases_parameter)otherRef);
                }
                else
                {
                    tempCount = 1;
                }
            }

            operationCount += tempCount;
            tpCount += tempCount;
            lcdtCount += tempCount;
            patchCount += tempCount * 4;

            if (tpOperation->base.parameter.data_buff != VX_NULL)
            {
                lcdtCount += tempCount;
                patchCount += tempCount;
            }

            if ((tpOperation->base.operatorType != VXNNE_OPERATOR_RESHUFFLE) && tpOperation->weights_biases)
            {
                /* lcd: ks */
                lcdtCount += tempCount;
                patchCount += tempCount;
            }

            /* patch image circular buffer end address */
            if ((operationCommand->inputTile.circularBufEndAddrPlus1 != 0xFFFFFFFF) &&
                (operationCommand->inputTile.circularBufEndAddrPlus1 != 0) &&
                (operationCommand->inputTile.circularBufEndAddrPlus1 != 0xCDCDCDCD))
            {
                patchCount += tempCount;
            }
            if ((operationCommand->outputTile.circularBufEndAddrPlus1 != 0xFFFFFFFF) &&
                (operationCommand->outputTile.circularBufEndAddrPlus1 != 0) &&
                (operationCommand->outputTile.circularBufEndAddrPlus1 != 0xCDCDCDCD))
            {
                patchCount += tempCount;
            }

            if (parameter->data_buff != VX_NULL)
            {
                vx_size size_tensor = 0;
                vxoTensor_GetTensorWholeSize(parameter->data_buff, &size_tensor);
                ksDataSize = (vx_uint32)size_tensor;
                ksDataSize = (vx_uint32)((vx_float32)ksDataSize);

                vxoTensor_GetTensorBatchArrayViewMemory(parameter->data_buff, 0, VX_NULL, &ksDataPhysical);

                for (cnt = 0; cnt < lcdKernelDataPhyIndex; cnt++)
                {
                    if (lcdKernelDataPhysical[cnt] == ksDataPhysical)
                    {
                        break;
                    }
                }

                if (cnt == lcdKernelDataPhyIndex)
                {
                    ksDataSize = gcmALIGN_NP2_SAFE(ksDataSize, 64);
                    /*vxInfo("get NBG size TP KS physcial: 0x%08xm size: 0x%08x \n", ksDataPhysical, ksDataSize);*/
                    KernelStreamSize += ksDataSize;
                    lcdKernelDataPhysical[lcdKernelDataPhyIndex] = ksDataPhysical;
                    lcdKernelDataPhyIndex++;
                }
            }
            else if (parameter->tpType == TP_SINGLE_FC && parameter->other_ref != VX_NULL)
            {
                vx_float32 temp = 0;
                vx_uint32 j = 0;
                vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter)parameter->other_ref;

                if (WB_TOTAL_SLICE_NUM(weights_biases) == 0)
                {
                    vxError("fail to get NBG size, tp weights_biases sclice num is 0\n");
                    vxmONERROR(VX_ERROR_INVALID_VALUE);
                }

                for (j = 0; j < WB_TOTAL_SLICE_NUM(weights_biases); j++)
                {
                    ksDataSize = (vx_uint32)WB_MEM_SIZE_INDEX(weights_biases, j);
                    temp = (vx_float32)ksDataSize * 1.004f; /* compress */
                    ksDataSize = (vx_uint32)temp;
                    ksDataPhysical = (vx_uint32)WB_MEM_PHYSICAL_ADDR_INDEX(weights_biases, j);

                    ksDataSize = gcmALIGN_NP2_SAFE(ksDataSize, 64);

                    for (cnt = 0; cnt < lcdKernelDataPhyIndex; cnt++)
                    {
                        if (lcdKernelDataPhysical[cnt] == ksDataPhysical)
                        {
                            break;
                        }
                    }

                    if (cnt == lcdKernelDataPhyIndex)
                    {
                        /*vxInfo("get NBG size TP KS physcial: 0x%08xm size: 0x%08x \n", ksDataPhysical, ksDataSize);*/
                        KernelStreamSize += ksDataSize;
                        lcdKernelDataPhysical[lcdKernelDataPhyIndex] = ksDataPhysical;
                        lcdKernelDataPhyIndex++;
                    }
                }

                if (ksDataSize == 0)
                {
                    vxError("fail to get NBG size, tp kernel stream size is 0\n");
                    vxmONERROR(VX_ERROR_INVALID_VALUE);
                }
            }

            input = tpOperation->input;
            output = tpOperation->output;
        }
        else if (operation->target == VXNNE_OPERATION_TARGET_SH)
        {
            vxnne_shader_operation shaderOperation  = (vxnne_shader_operation)operation;
            gcsSURF_NODE_PTR instMemNode = VX_NULL;
            gcsSURF_NODE_PTR sharedMemNode = VX_NULL;
            vx_shader kernelShader = VX_NULL;
            gcsHINT_PTR hints = VX_NULL;
            vx_reference *shParams = VX_NULL;
            vx_uint32 tempSize = 0;
            vx_uint32 statesSize = 0;

            operationCount += 1;
            shCount += 1;

            if (operation->operatorType == VXNNE_OPERATOR_USER_VXC)
            {
                gctUINT currentShaderID = 0;

                vxmONERROR(vxoProgramKernel_GetCurrentShaderID(node, &currentShaderID));
                kernelShader = node->kernel->kernelShader[currentShaderID];

                shParams = shaderOperation->shaderExecutable->params;

                lcdtCount += 2 + node->kernel->signature.paramCount;
                patchCount += 1 + node->kernel->signature.paramCount;
            }
            else
            {
                kernelShader = shaderOperation->shaderExecutable->kernelShader;

                shParams = shaderOperation->shaderExecutable->param;

                lcdtCount += 2 + shaderOperation->shaderExecutable->paramNum;
                patchCount += 1 + shaderOperation->shaderExecutable->paramNum;
            }

            hints = kernelShader->states.programState.hints;
            instMemNode = (gcsSURF_NODE_PTR)hints->shaderVidNodes.instVidmemNode[gceSGSK_FRAGMENT_SHADER];
            if (VX_NULL != instMemNode)
            {
                vx_uint32 SHinstrSize = hints->fsInstCount * sizeof(gctUINT) * 4;
                if (SHinstrSize > (vx_uint32)instMemNode->size)
                {
                    SHinstrSize = (vx_uint32)instMemNode->size;
                    vxError("%s[%d]: SHinstrSize: 0x%x is large than instMemNode->size: 0x%x\n", __FUNCTION__, __LINE__,
                             SHinstrSize, (vx_uint32)instMemNode->size);
                }
                shIntrSize += gcmALIGN_NP2_SAFE(SHinstrSize, 256);
            }

            sharedMemNode = (gcsSURF_NODE_PTR)hints->shaderVidNodes.sharedMemVidMemNode;
            if (VX_NULL != sharedMemNode)
            {
                shShareMemSize += gcmALIGN_NP2_SAFE((vx_uint32)sharedMemNode->size, 64);
                lcdtCount++;
                patchCount++;
            }

            vxmONERROR(vxoBinaryGraph_GetSHParamSize(graph, shParams, shaderOperation->shaderExecutable->paramNum,
                                                     &lcdTensorsPhysical, &lcdTensorsPhyIndex, &tempSize));
            shParaSize += tempSize;

            vxmONERROR(vxoBinaryGraph_GetShaderStatesSize(graph, kernelShader, &statesSize));
            shaderStatesSize += statesSize;
        }
        else if (operation->target == VXNNE_OPERATION_TARGET_SC)
        {
            operationCount += 1;
            scCount += 1;
            patchCount += 3;
            lcdtCount += 6;
        }
        else if (operation->target == VXNNE_OPERATION_TARGET_SW)
        {
            operationCount += 1;
            swCount += 1;
            layerParamCount += node->kernel->signature.paramCount;
            lcdtCount += node->kernel->signature.paramCount;
        }

        if (input != VX_NULL)
        {
            vx_size tensorSize = 0;
            vx_uint32 physical = TENSOR_PHYSICAL_ADDR(input);
            vx_uint32 i = 0;
            vx_uint32 num = 0;
            vx_uint32 flag = 0;

            for (num = 0; num < graph->inputCount; num++)
            {
                if ((vx_reference)input == graph->inputs[num])
                {
                    flag = 1;/* is graph's input*/
                }
            }

            if ((vxoMemory_GetType(&input->tensorBuffer->memory) == VXNNE_MEM_POOL_TYPE_ORIG_DDR) && (0 == flag))
            {
                for (i = 0; i < lcdTensorsPhyIndex; i++)
                {
                    if (physical == lcdTensorsPhysical[i])
                    {
                        break;
                    }
                }

                if (i == lcdTensorsPhyIndex)
                {
                    vxoTensor_GetTensorWholeSize(input, &tensorSize);
                    lcdTensorsPhysical[lcdTensorsPhyIndex] = physical;
                    lcdTensorsPhyIndex++;
                    lcdTensorSize += (vx_uint32)tensorSize;
                    lcdtCount += 1;
                    patchCount += 1;
                }
            }
        }

        if (output != VX_NULL)
        {
            vx_size tensorSize = 0;
            vx_uint32 physical = TENSOR_PHYSICAL_ADDR(output);
            vx_uint32 i = 0;
            vx_uint32 num = 0;
            vx_uint32 flag = 0;

            for (num = 0; num < graph->outputCount; num++)
            {
                if ((vx_reference)output == graph->outputs[num])
                {
                    flag = 1;/* is graph's input*/
                }
            }

            if ((vxoMemory_GetType(&output->tensorBuffer->memory) == VXNNE_MEM_POOL_TYPE_ORIG_DDR) && (0 == flag))
            {
                for (i = 0; i < lcdTensorsPhyIndex; i++)
                {
                    if (physical == lcdTensorsPhysical[i])
                    {
                        break;
                    }
                }

                if (i == lcdTensorsPhyIndex)
                {
                    vxoTensor_GetTensorWholeSize(output, &tensorSize);
                    lcdTensorsPhysical[lcdTensorsPhyIndex] = physical;
                    lcdTensorsPhyIndex++;
                    lcdTensorSize += (vx_uint32)tensorSize;
                    lcdtCount += 1;
                    patchCount += 1;
                }
            }
        }
    }

    if (graph->nodeCount == 1)
    {
        lcdTensorSize = 0;
    }

    operationSize = sizeof(vx_binary_operation_info_s) * operationCount;
    lcdtSize = sizeof(vx_binary_loadingdata_table_info_s) * lcdtCount;
    nnSize = NNE_COMMAND_SIZE * nnCount;
    tpSize = TP_COMMAND_SIZE * tpCount;
    shSize = sizeof(vx_uint32) * shCount;
    patchSize = sizeof(vx_binary_patch_info_s) * patchCount;
    swSize = sizeof(vx_uint32) * swCount;
    layerParamSize = sizeof(vx_binary_layer_parameter_s) * layerParamCount;

    statesSize = nnCount * NN_STATES_SIZE + tpCount * TP_STATES_SIZE + scCount * SC_STATES_SIZE + shaderStatesSize;
    lcdSize = KernelStreamSize + statesSize + shShareMemSize + shIntrSize + shParaSize + lcdTensorSize;

    lcdSize += SH_COMMAND_ALIGN_SIZE;

    vxInfo("KernelStreamSize: 0x%x, statesSize: 0x%x, shShareMemSize: 0x%x, shIntrSize: 0x%x, shParaSize: 0x%x, lcdTensorSize: 0x%x, shaderStatesSize: 0x%x\n",
            KernelStreamSize, statesSize, shShareMemSize, shIntrSize, shParaSize, lcdTensorSize, shaderStatesSize);

    totalSize = operationSize + lcdtSize + nnSize + tpSize + shSize + patchSize
                + swSize + layerParamSize;

    *sectionSize = totalSize;
    *retLcdSize = lcdSize;

    vxInfo("NBG: operationSize: 0x%x, nnSize: 0x%x, tpSize: 0x%x, shSize: 0x%x, swSize: 0x%x, layerParamSize: 0x%x, lcdtSize: 0x%x, patchSize: 0x%x, lcdSize 0x%x\n",
            operationSize, nnSize, tpSize, shSize, swSize, layerParamSize, lcdtSize, patchSize, lcdSize);

OnError:
    if (lcdTensorsPhysical != VX_NULL)
    {
        vxFree(lcdTensorsPhysical);
        lcdTensorsPhysical = VX_NULL;
    }

    if (lcdKernelDataPhysical != VX_NULL)
    {
        vxFree(lcdKernelDataPhysical);
        lcdKernelDataPhysical = VX_NULL;
    }

    return status;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_VerifyGraph(
    vx_graph graph
    )
{
    vx_status status = VX_SUCCESS;
    gcoHARDWARE savedHardware = gcvNULL;
    gctUINT32 savedCoreIndex = 0;
    gceHARDWARE_TYPE savedHardwareType = gcvHARDWARE_INVALID;
    gctBOOL switched = gcvFALSE;
    vx_bool first = ((graph->verified == vx_false_e) && (graph->reverify == vx_false_e)) ? vx_true_e : vx_false_e;

    if (graph->verified)
    {
        vxError("%s[%d]: graph has been verified\n", __FUNCTION__, __LINE__);
        return VX_SUCCESS;
    }

    vxAcquireMutex(graph->base.lock);

    if(graph->parentGraph == gcvNULL)
    {
        gcmONERROR(gcoVX_SwitchContext(graph->deviceID, &savedHardware, &savedHardwareType, &savedCoreIndex));
        switched = gcvTRUE;
    }

    vxmONERROR(vxoGraph_UserKernelPreprocess(graph, first));

    vxmONERROR(vxoGraph_VerifyAllNodeParameters(graph));

    vxmONERROR(vxoGraph_VerifyAllNodeWriteDependencies(graph));

    vxmONERROR(vxoGraph_RetrieveTopology(graph));

    if (graph->base.context->options.enableGraphAdapter)
    {
        vxmONERROR(vxoGraph_Adapter(graph));
    }

    vxmONERROR(vxoGraph_AllocateAllMemoryObjects(graph));

    vxmONERROR(vxoGraph_DetectAllHeadNodes(graph));

    vxmONERROR(vxoGraph_DetectAllTailNodes(graph));

    vxmONERROR(vxoBinaryGraph_GetGraphInputOutput(graph));

    vxmONERROR(vxoGraphOptimization(graph));

    vxmONERROR(vxoGraph_DetectCycle(graph));;

    vxmONERROR(vxoGraph_DetectUnvisitedNodes(graph));

    vxmONERROR(vxoGraph_VerifyAllNodesByTarget(graph));

    vxoGraph_GenerateAllNodeIndexTable(graph);

    vxmONERROR(vxoGraph_InitializeAllNodeKernels(graph));

    vxmONERROR(vxoGraph_CaculateCostFactors(graph));

    vxoGraph_GenerateOperationTable(graph);

    if (graph->layer != NULL)
    {
        vxoGraph_GenerateOpParentChild(graph);

        if (vxoContext_IsFeatureAvailable(graph->base.context, VX_NN_TP_PARALLEL))
        {
            vxoGraphParallel_AnalyzeOperationsBefore(graph);
        }

        vxmONERROR(vxoGraph_VerifyTiling(graph));

        vxmONERROR(vxoGraph_VerifyVirtualBuffer(graph));
    }

    if(switched)
    {
        gcoVX_RestoreContext(savedHardware,savedHardwareType,savedCoreIndex);
    }

    graph->reverify = vx_false_e;
    graph->verified = vx_true_e;
    graph->status   = VX_GRAPH_STATE_VERIFIED;

    return status;

OnError:
    vxReleaseMutex(graph->base.lock);

    graph->reverify = vx_false_e;
    graph->verified = vx_false_e;
    graph->status   = VX_GRAPH_STATE_UNVERIFIED;

    return status;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_GeneratNBG(
    vx_graph graph
    )
{
    vx_status status = VX_SUCCESS;
    gcoHARDWARE savedHardware = gcvNULL;
    gctUINT32 savedCoreIndex = 0;
    gceHARDWARE_TYPE savedHardwareType = gcvHARDWARE_INVALID;
    gctBOOL switched = gcvFALSE;

    vxAcquireMutex(graph->base.lock);

    if(graph->parentGraph == gcvNULL)
    {
        gcmONERROR(gcoVX_SwitchContext(graph->deviceID, &savedHardware, &savedHardwareType, &savedCoreIndex));
        switched = gcvTRUE;
    }

    vxmONERROR(vxoBinaryGraph_SaveBinaryEntrance(graph));

    vxmONERROR(vxnneExecutionLayer_GenerateCommands(graph->base.context, &graph->layer->base));

    if (vxoContext_IsFeatureAvailable(graph->base.context, VX_NN_TP_PARALLEL))
    {
        vxoGraphParallel_AnalyzeOperationsAfter(graph);
    }

    vxoGraph_VerifyOperationSync(graph);

    if(switched)
    {
        gcoVX_RestoreContext(savedHardware,savedHardwareType,savedCoreIndex);
    }

OnError:
    vxReleaseMutex(graph->base.lock);

    return status;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_GetNBGSize(
    vx_graph graph,
    vx_size *size
    )
{
    vx_status status = VX_SUCCESS;
    vx_uint32 nbEntranceSize = 0;
    vx_uint32 nbIOSize = 0;
    vx_uint32 layeSize = 0;
    vx_uint32 sectionsSize = 0;
    vx_uint32 lcdSize = 0;
    vx_uint32 headerSize = 0;
    vx_uint32 inputCount = 0;
    vx_uint32 outputCount = 0;
    vx_uint32 totalSize = 0;
    vx_uint32 inputOutputSize = 0;
    vx_reference *inputTableRef = VX_NULL;
    vx_reference *outputTableRef = VX_NULL;

    inputTableRef = (vx_reference*)vxAllocateAndZeroMemory(VX_MAX_NN_INOUT_PARAM_COUNT * sizeof(vx_reference) );
    if (inputTableRef == VX_NULL)
    {
        vxError("%s[%d]: failed to allocate memory for inputTableRef\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }

    outputTableRef = (vx_reference*)vxAllocateAndZeroMemory(VX_MAX_NN_INOUT_PARAM_COUNT * sizeof(vx_reference) );
    if (outputTableRef == VX_NULL)
    {
        vxError("%s[%d]: failed to allocate memory for outputTableRef\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }

    /* 1. enrtance size */
    nbEntranceSize = sizeof(vx_binary_header_s) + sizeof(vx_binary_memory_pool_info_s)
                     + sizeof(vx_binary_axi_sram_info_s) + sizeof(vx_binary_vip_sram_info_s)
                     + sizeof(vx_binary_entrance_info_s);

    /* 2. input and output table size */
    if ((graph->inputCount == 0) || (graph->outputCount == 0))
    {
        vxmONERROR(vxoBinaryGraph_InputsOutputs(graph, inputTableRef, &inputCount,
                                                outputTableRef, &outputCount));
    }

    if (graph->inputCount && graph->inputs)
    {
        nbIOSize += sizeof(vx_binary_input_output_info_s) * graph->inputCount;
    }
    else if (inputCount > 0)
    {
        nbIOSize += sizeof(vx_binary_input_output_info_s) * inputCount;
    }
    else
    {
        vxError("%s[%d]: failed to get input count %d\n", __FUNCTION__, __LINE__, inputCount);
        vxmONERROR(VX_FAILURE);
    }

    if (graph->outputCount && graph->outputs)
    {
        nbIOSize += sizeof(vx_binary_input_output_info_s) * graph->outputCount;
    }
    else if (outputCount > 0)
    {
        nbIOSize += sizeof(vx_binary_input_output_info_s) * outputCount;
    }
    else
    {
        vxError("%s[%d]: failed to get output count %d\n", __FUNCTION__, __LINE__, outputCount);
        vxmONERROR(VX_FAILURE);
    }

    /* 3. layer size */
    layeSize = sizeof(vx_binary_layers_info_s) * graph->nodeCount;

    /* 4. other sections size */
    vxmONERROR(vxoBinaryGraph_GetSectionsSize(graph, &sectionsSize, &lcdSize));

    if (0 != graph->inputCount)
    {
        vx_uint32 i = 0;
        vx_size tensorSize = 0;
        for (i = 0; i < graph->inputCount; i++)
        {
            vx_reference ref = graph->inputs[i];
            tensorSize = 0;
            if (ref->type == VX_TYPE_TENSOR)
            {
                vx_tensor tensor = (vx_tensor)ref;
                vxoTensor_GetTensorWholeSize(tensor, &tensorSize);
                inputOutputSize += (vx_uint32)tensorSize;
            }
        }
    }
    else
    {
        vx_uint32 i = 0;
        vx_size tensorSize = 0;
        for (i = 0; i < inputCount; i++)
        {
            vx_reference ref = inputTableRef[i];
            tensorSize = 0;
            if (ref->type == VX_TYPE_TENSOR)
            {
                vx_tensor tensor = (vx_tensor)ref;
                vxoTensor_GetTensorWholeSize(tensor, &tensorSize);
                inputOutputSize += (vx_uint32)tensorSize;
            }
        }
    }

    if (0 != graph->outputCount)
    {
        vx_uint32 i = 0;
        vx_size tensorSize = 0;
        for (i = 0; i < graph->outputCount; i++)
        {
            vx_reference ref = graph->outputs[i];
            tensorSize = 0;
            if (ref->type == VX_TYPE_TENSOR)
            {
                vx_tensor tensor = (vx_tensor)ref;
                vxoTensor_GetTensorWholeSize(tensor, &tensorSize);
                inputOutputSize += (vx_uint32)tensorSize;
            }
        }
    }
    else
    {
        vx_uint32 i = 0;
        vx_size tensorSize = 0;
        for (i = 0; i < outputCount; i++)
        {
            vx_reference ref = outputTableRef[i];
            tensorSize = 0;
            if (ref->type == VX_TYPE_TENSOR)
            {
                vx_tensor tensor = (vx_tensor)ref;
                vxoTensor_GetTensorWholeSize(tensor, &tensorSize);
                inputOutputSize += (vx_uint32)tensorSize;
            }
        }
    }

    if (graph->nodeCount == 1)
    {
        inputOutputSize = 0;
    }

    headerSize = nbEntranceSize + layeSize + sectionsSize + nbIOSize;

    vxInfo("NBG: entranceSize: 0x%x, nbIOSize: 0x%x, layeSize: 0x%x, sectionsSize: 0x%x, inputoutput size: 0x%x\n",
            nbEntranceSize, nbIOSize, layeSize, sectionsSize, inputOutputSize);
    vxInfo("NBG: lcdSize: 0x%x, headerSize : 0x%x\n", lcdSize, headerSize);

    totalSize = headerSize + lcdSize;

    totalSize += SH_COMMAND_ALIGN_SIZE; /* alignment PPU command size */

    if (size != VX_NULL)
    {
        *size = (vx_size)totalSize;
    }

    graph->binarySave->NBGInMemorySize = totalSize;

    vxInfo("Calculate NBG size : %d bytes\n", (vx_uint32)(totalSize));
OnError:
    if (inputTableRef != VX_NULL)
    {
        vxFree(inputTableRef);
    }
    if (outputTableRef != VX_NULL)
    {
        vxFree(outputTableRef);
    }

    return status;
}

VX_PRIVATE_API vx_status vxoBinaryGraph_GenerateNBG(
    vx_graph graph,
    vx_ptr buffer,
    vx_size *size
    )
{
    vx_status status = VX_SUCCESS;
    vx_context context =graph->base.context;
    vx_uint32 nbgSize = 0;
    vx_uint32 enableDumpNBG = 0;
    gctSTRING envctrl = gcvNULL;

    /* enable save binary */
    context->options.enableSaveBinary = 1;
    graph->binarySave->generateNBGToMemory = 1;
    graph->binarySave->NBGBuffer = buffer;
    graph->binarySave->NBGSize = &nbgSize;

    vxmONERROR(vxoBinaryGraph_GeneratNBG(graph));

    vxmONERROR(vxProcessGraph(graph));

    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_DUMP_NBG", &envctrl)) && envctrl)
    {
        enableDumpNBG = atoi(envctrl);
    }

    if (enableDumpNBG == 1)
    {
        FILE *binarySaveFile;
        gcmVERIFY_OK(gcoOS_Open(gcvNULL, "network_binary_graph.nb", gcvFILE_CREATE, (gctFILE*)(&binarySaveFile)));
        gcoOS_Seek(gcvNULL, binarySaveFile, 0, gcvFILE_SEEK_SET);
        gcoOS_Write(gcvNULL, binarySaveFile, nbgSize, buffer);
        gcoOS_Flush(gcvNULL, binarySaveFile);
        gcmVERIFY_OK(gcoOS_Close(gcvNULL, binarySaveFile));
    }

    vxInfo("Generate NBG in memory Actual NBG size : %d bytes\n", nbgSize);

    if (size != VX_NULL)
    {
        *size = (vx_size)nbgSize;
    }

OnError:
    context->options.enableSaveBinary = 0;
    vxoBinaryGraph_unInitial(graph);

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxGenerateNBG(vx_graph graph, void *buffer, vx_size *size)
{
    vx_status status = VX_SUCCESS;
    vx_context context = VX_NULL;
    gcmHEADER_ARG("graph=%p, buffer=%p, size=%p", graph, buffer, size);
    gcmDUMP_API("$VX vxGenerateNBG: graph=%p, buffer=%p, size=%p", graph, buffer, size);

    if (graph == VX_NULL)
    {
        vxError("%s[%d]: graph is NULL\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_FAILURE);
    }

    context = graph->base.context;

    if (vx_true_e == vxoBinaryGraph_HasBinaryInGraph(graph))
    {
        /* bypass the network if it import from binary graph */
        vxError("%s[%d]: has binary in graph, can't genereate NBG\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_NOT_SUPPORTED);
    }

    if (context->options.enableCacheBinaryGraph)
    {
        vxError("%s[%d]: not support this feature when cache binary graph enabled\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_NOT_SUPPORTED);
    }

    if (0 == graph->nodeCount)
    {
        vxError("%s[%d]: nodeCount is %d\n", __FUNCTION__, __LINE__, graph->nodeCount);
        vxmONERROR(VX_FAILURE);
    }

    if (((buffer == VX_NULL) && (size != VX_NULL)) || (graph->verified == vx_false_e))
    {
        /* 1. allocate memory for binarySave */
        graph->binarySave = (vx_binary_save)vxAllocateAndZeroMemory(sizeof(vx_binary_save_s));
        if (VX_NULL == graph->binarySave)
        {
            vxError("%s[%d]: fail to allocate memory for binary save\n", __FUNCTION__, __LINE__);
            vxmONERROR(VX_FAILURE);
        }

        /* 2. verify graph */
        vxmONERROR(vxoBinaryGraph_VerifyGraph(graph));

        /* 3. calculate NBG size */
        vxmONERROR(vxoBinaryGraph_GetNBGSize(graph, size));
    }
    else if (buffer != VX_NULL)
    {
        vxmONERROR(vxoBinaryGraph_GenerateNBG(graph, buffer, size));
    }
    else
    {
        vxError("%s[%d]: fail to generate NBG buffer: %p, size: %p\n", __FUNCTION__, __LINE__, buffer, size);
    }

    gcmFOOTER_ARG("%d", status);
OnError:
    return status;
}

