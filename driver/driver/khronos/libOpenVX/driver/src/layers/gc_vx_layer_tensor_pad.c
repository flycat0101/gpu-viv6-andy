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
#include <layers/gc_vx_layer_tensor_pad.h>


vx_status vxnneExecuteSWTensorPad(struct _vxnne_operation_s *operation)
{
    vxnne_tensor_pad_sw_operation           padOperation   = (vxnne_tensor_pad_sw_operation)operation;

    vx_tensor src  = (vx_tensor)padOperation->src;
    vx_tensor dst = (vx_tensor)padOperation->dst;

    vx_scalar padLeft = (vx_scalar)padOperation->padLeft;
    vx_scalar padRight = (vx_scalar)padOperation->padRight;
    vx_scalar padTop = (vx_scalar)padOperation->padTop;
    vx_scalar padBottom = (vx_scalar)padOperation->padBottom;
    vx_scalar padMode = (vx_scalar)padOperation->padMode;
    vx_scalar padConst = (vx_scalar)padOperation->padConst;

    gctPOINTER srcLogical = VX_NULL;
    gctPOINTER dstLogical = VX_NULL;

    vx_int8 srcFp = TENSOR_POS(src);
    vx_int8 dstFp = TENSOR_POS(dst);
    vx_enum  dstRoundingMode = TENSOR_ROUNDING_MODE(dst);

    vx_uint32 dstSize;

    vx_uint32 left = padLeft->value->u32;
    vx_uint32 right = padRight->value->u32;
    vx_uint32 top = padTop->value->u32;
    vx_uint32 bottom = padBottom->value->u32;

    vx_uint32 inWidth, inHeight;
    vx_uint32 outWidth, outHeight, ofm, batch;
    vx_uint32 x, y, z, w;

    vx_enum mode = padMode->value->e;
    vx_uint32 constant = padConst->value->u32;

    if (src->isViewed)
    {
        inWidth = src->viewRegion.viewEnds[0] - src->viewRegion.viewStarts[0];
        inHeight = src->viewRegion.viewEnds[1] - src->viewRegion.viewStarts[1];
    }
    else
    {
        inWidth = TENSOR_SIZE_INDEX(src, 0);
        inHeight = TENSOR_SIZE_INDEX(src, 1);
    }

    if (dst->isViewed)
    {
        outWidth = dst->viewRegion.viewEnds[0] - dst->viewRegion.viewStarts[0];
        outHeight = dst->viewRegion.viewEnds[1] - dst->viewRegion.viewStarts[1];
        ofm = dst->viewRegion.viewEnds[2] - dst->viewRegion.viewStarts[2];
        batch = dst->viewRegion.viewEnds[3] - dst->viewRegion.viewStarts[3];
    }
    else
    {
        outWidth = TENSOR_SIZE_INDEX(dst, 0);
        outHeight = TENSOR_SIZE_INDEX(dst, 1);
        ofm = TENSOR_SIZE_INDEX(dst, 2);
        batch = TENSOR_SIZE_INDEX(dst, 3);
    }

    if (inHeight + top + bottom != outHeight ||
        inWidth + left + right != outWidth)
    {
        vxmASSERT(gcvFALSE);
    }

    vxoTensor_GetTensorViewMemory(src, &srcLogical, VX_NULL);
    vxoTensor_GetTensorViewMemory(dst, &dstLogical, VX_NULL);

    vxoTensor_GetTensorSize(dst,&dstSize);

    switch (mode)
    {
    case VX_PAD_CONSTANT:
        {
            if (TENSOR_DATA_TYPE(dst) == VX_TYPE_INT8 || TENSOR_DATA_TYPE(dst) == VX_TYPE_UINT8)
            {
                /*Set const value to dst*/
                memset(dstLogical,
                    constant,
                    dstSize);

                /* Copy non-padding part*/
                for (w = 0; w < batch; w++)
                {
                    for (z = 0; z < ofm; z++)
                    {
                        for (y = top; y < outHeight - bottom; y++)
                        {
                            for (x = left; x < outWidth - right; x++)
                            {
                                vx_float32 src0;
                                vx_uint32 srcOffset = (x - left) + inWidth * (y - top) + inWidth * inHeight * z + inWidth * inHeight * ofm * w;
                                vx_uint32 dstOffset = x + outWidth * y + outWidth * outHeight * z + outWidth * outHeight * ofm * w;

                                src0 = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(src), TENSOR_QUANT_TYPE(src), srcOffset, (vx_uint8_ptr)srcLogical, srcFp, TENSOR_TF_ZEROPOINT(src), TENSOR_TF_SCALE(src));
                                vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(dst), TENSOR_QUANT_TYPE(dst), dstOffset, src0, (vx_uint8_ptr)dstLogical, dstFp, TENSOR_TF_ZEROPOINT(dst), TENSOR_TF_SCALE(dst), dstRoundingMode);
                            }
                        }
                    }
                }
            }
            else
            {
                for (w = 0; w < batch; w++)
                {
                    for (z = 0; z < ofm; z++)
                    {
                        for (y = 0; y < outHeight; y++)
                        {
                            for (x = 0; x < outWidth; x++)
                            {
                                vx_float32 src0;
                                vx_uint32 srcOffset;
                                vx_uint32 dstOffset = x + outWidth * y + outWidth * outHeight * z + outWidth * outHeight * ofm * w;

                                if (x < left
                                    || x >= outWidth - right
                                    || y < top
                                    || y >= outHeight - bottom)
                                {
                                    src0 = (vx_float32)constant;
                                }
                                else
                                {
                                    srcOffset = (x - left) + inWidth * (y - top) + inWidth * inHeight * z + inWidth * inHeight * ofm * w;
                                    src0 = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(src), TENSOR_QUANT_TYPE(src), srcOffset, (vx_uint8_ptr)srcLogical, srcFp, TENSOR_TF_ZEROPOINT(src), TENSOR_TF_SCALE(src));
                                }
                                vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(dst), TENSOR_QUANT_TYPE(dst), dstOffset, src0, (vx_uint8_ptr)dstLogical, dstFp, TENSOR_TF_ZEROPOINT(dst), TENSOR_TF_SCALE(dst), dstRoundingMode);
                            }
                        }
                    }
                }
            }
        }
        break;

    case VX_PAD_REPLICATE:
        {
            for (w = 0; w < batch; w++)
            {
                for (z = 0; z < ofm; z++)
                {
                    for (y = 0; y < outHeight; y++)
                    {
                        for (x = 0; x < outWidth; x++)
                        {
                            vx_float32 src0;
                            vx_uint32 srcX, srcY;
                            vx_uint32 srcOffset;
                            vx_uint32 dstOffset = x + outWidth * y + outWidth * outHeight * z + outWidth * outHeight * ofm * w;

                            if (x < left)
                                srcX = 0;
                            else if (x >= outWidth - right)
                                srcX = inWidth - 1;
                            else
                                srcX = x - left;

                            if (y < top)
                                srcY = 0;
                            else if (y >= outHeight - bottom)
                                srcY = inHeight - 1;
                            else
                                srcY = y - top;

                            srcOffset = srcX + inWidth * srcY + inWidth * inHeight * z + inWidth * inHeight * ofm * w;
                            src0 = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(src), TENSOR_QUANT_TYPE(src), srcOffset, (vx_uint8_ptr)srcLogical, srcFp, TENSOR_TF_ZEROPOINT(src), TENSOR_TF_SCALE(src));
                            vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(dst), TENSOR_QUANT_TYPE(dst), dstOffset, src0, (vx_uint8_ptr)dstLogical, dstFp, TENSOR_TF_ZEROPOINT(dst), TENSOR_TF_SCALE(dst), dstRoundingMode);
                        }
                    }
                }
            }
        }
        break;

    case VX_PAD_MIRROR_SYMMETRIC:
        {
            for (w = 0; w < batch; w++)
            {
                for (z = 0; z < ofm; z++)
                {
                    for (y = 0; y < outHeight; y++)
                    {
                        for (x = 0; x < outWidth; x++)
                        {
                            vx_float32 src0;
                            vx_uint32 srcX, srcY;
                            vx_uint32 srcOffset;
                            vx_uint32 dstOffset = x + outWidth * y + outWidth * outHeight * z + outWidth * outHeight * ofm * w;

                            if (x < left)
                                srcX = left - x - 1;
                            else if (x >= outWidth - right)
                                srcX = inWidth - (x - inWidth - left + 1);
                            else
                                srcX = x - left;

                            if (y < top)
                                srcY = top - y - 1;
                            else if (y >= outHeight - bottom)
                                srcY = inHeight - (y - inHeight - top + 1);
                            else
                                srcY = y - top;

                            srcOffset = srcX + inWidth * srcY + inWidth * inHeight * z + inWidth * inHeight * ofm * w;
                            src0 = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(src), TENSOR_QUANT_TYPE(src), srcOffset, (vx_uint8_ptr)srcLogical, srcFp, TENSOR_TF_ZEROPOINT(src), TENSOR_TF_SCALE(src));
                            vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(dst), TENSOR_QUANT_TYPE(dst), dstOffset, src0, (vx_uint8_ptr)dstLogical, dstFp, TENSOR_TF_ZEROPOINT(dst), TENSOR_TF_SCALE(dst), dstRoundingMode);
                        }
                    }
                }
            }
        }
        break;

    case VX_PAD_MIRROR_REFLECT:
        {
            for (w = 0; w < batch; w++)
            {
                for (z = 0; z < ofm; z++)
                {
                    for (y = 0; y < outHeight; y++)
                    {
                        for (x = 0; x < outWidth; x++)
                        {
                            vx_float32 src0;
                            vx_uint32 srcX, srcY;
                            vx_uint32 srcOffset;
                            vx_uint32 dstOffset = x + outWidth * y + outWidth * outHeight * z + outWidth * outHeight * ofm * w;

                            if (x < left)
                                srcX = left - x;
                            else if (x >= outWidth - right)
                                srcX = inWidth - (x - inWidth - left + 2);
                            else
                                srcX = x - left;

                            if (y < top)
                                srcY = top - y;
                            else if (y >= outHeight - bottom)
                                srcY = inHeight - (y - inHeight - top + 2);
                            else
                                srcY = y - top;

                            srcOffset = srcX + inWidth * srcY + inWidth * inHeight * z + inWidth * inHeight * ofm * w;
                            src0 = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(src), TENSOR_QUANT_TYPE(src), srcOffset, (vx_uint8_ptr)srcLogical, srcFp, TENSOR_TF_ZEROPOINT(src), TENSOR_TF_SCALE(src));
                            vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(dst), TENSOR_QUANT_TYPE(dst), dstOffset, src0, (vx_uint8_ptr)dstLogical, dstFp, TENSOR_TF_ZEROPOINT(dst), TENSOR_TF_SCALE(dst), dstRoundingMode);
                        }
                    }
                }
            }
        }
        break;

    default:
        vxError("Invalid pad mode");
        return VX_ERROR_INVALID_PARAMETERS;
    }

    return VX_SUCCESS;
}

vx_status vxnnePAD(vx_int32 pad, vx_int32 slice, vx_uint8_ptr* dst_ptr, vx_int32 item_size, vx_enum mode, vx_int32 constant, vx_uint8_ptr src_ptr)
{

    if (pad > 0)
    {
        vx_int32 s = 0;
        for (s = 0; s < slice * pad; s++)
        {
            switch (mode)
            {
            case VX_PAD_CONSTANT:
                memset(*dst_ptr, constant, item_size);
                break;
            case VX_PAD_REPLICATE:
                break;
            case VX_PAD_MIRROR_SYMMETRIC:
                break;
            case VX_PAD_MIRROR_REFLECT:
                break;
            }

            *dst_ptr += item_size;
        }

    }
    return VX_SUCCESS;

}

vx_status vxnneExecuteSWTensorPad2(struct _vxnne_operation_s *operation)
{
    vxnne_tensor_pad_sw_operation           padOperation = (vxnne_tensor_pad_sw_operation)operation;

    vx_tensor src = (vx_tensor)padOperation->src;
    vx_tensor dst = (vx_tensor)padOperation->dst;

    vx_tensor pad_dims = (vx_tensor)padOperation->pad_dims;
    vx_scalar padMode = (vx_scalar)padOperation->padMode;
    vx_scalar padConst = (vx_scalar)padOperation->padConst;
    vx_uint8_ptr src_base = VX_NULL, dst_base = VX_NULL, dst_ptr = VX_NULL, src_ptr = VX_NULL;
    vx_uint32 constant = padConst != VX_NULL?padConst->value->u32:0;
    vx_enum mode = padMode->value->e;
    vx_int32 left_n_padding = 0, left_c_padding = 0, left_h_padding = 0, left_w_padding = 0;
    vx_int32 right_n_padding = 0, right_c_padding = 0, right_h_padding = 0, right_w_padding = 0;
    vx_int32 inWidth = 0, inHeight = 0, inDepth = 0, inBatch = 0;
    vx_int32 outWidth = 0, outHeight = 0, outDepth = 0, outBatch = 0;
    vx_int32 item_size = vxnneGetTypeSize(TENSOR_DATA_TYPE(src));
    vx_int32 b = 0, c = 0, w = 0, h = 0;
    vx_int32_ptr pad_base = VX_NULL;

    vxoTensor_GetTensorViewMemory(pad_dims, (gctPOINTER*)&pad_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(src, (gctPOINTER*)&src_base, VX_NULL);
    vxoTensor_GetTensorViewMemory(dst, (gctPOINTER*)&dst_base, VX_NULL);

    /*
     *     w      h       c       n
     *  0   1   2   3   4   5   6   7
     * w_s w_e h_s h_e c_s c_e n_s n_e
     */
    left_n_padding = pad_base[6];
    left_c_padding = pad_base[4];
    left_h_padding = pad_base[2];
    left_w_padding = pad_base[0];

    right_n_padding = pad_base[7];
    right_c_padding = pad_base[5];
    right_h_padding = pad_base[3];
    right_w_padding = pad_base[1];

    if (src->isViewed)
    {
        inWidth  = src->viewRegion.viewEnds[0] - src->viewRegion.viewStarts[0];
        inHeight = src->viewRegion.viewEnds[1] - src->viewRegion.viewStarts[1];
        inDepth  = src->viewRegion.viewEnds[2] - src->viewRegion.viewStarts[2];
        inBatch  = src->viewRegion.viewEnds[3] - src->viewRegion.viewStarts[3];
    }
    else
    {
        inWidth  = TENSOR_SIZE_INDEX(src, 0);
        inHeight = TENSOR_SIZE_INDEX(src, 1);
        inDepth  = TENSOR_SIZE_INDEX(src, 2);
        inBatch  = TENSOR_SIZE_INDEX(src, 3);
    }

    if (dst->isViewed)
    {
        outWidth  = dst->viewRegion.viewEnds[0] - dst->viewRegion.viewStarts[0];
        outHeight = dst->viewRegion.viewEnds[1] - dst->viewRegion.viewStarts[1];
        outDepth  = dst->viewRegion.viewEnds[2] - dst->viewRegion.viewStarts[2];
        outBatch  = dst->viewRegion.viewEnds[3] - dst->viewRegion.viewStarts[3];
    }
    else
    {
        outWidth = TENSOR_SIZE_INDEX(dst, 0);
        outHeight = TENSOR_SIZE_INDEX(dst, 1);
        outDepth = TENSOR_SIZE_INDEX(dst, 2);
        outBatch = TENSOR_SIZE_INDEX(dst, 3);
    }

    gcmASSERT(inHeight + left_h_padding + right_h_padding == outHeight);
    gcmASSERT(inWidth  + left_w_padding + right_w_padding == outWidth);
    gcmASSERT(inDepth  + left_c_padding + right_c_padding == outDepth);
    gcmASSERT(inBatch  + left_n_padding + right_n_padding == outBatch);

    dst_ptr = dst_base;
    src_ptr = src_base;

    vxnnePAD(left_n_padding, outDepth * outWidth * outHeight, &dst_ptr, item_size, mode, constant, VX_NULL);

    for (b = 0; b < inBatch; b ++)
    {

        vxnnePAD(left_c_padding, outWidth * outHeight, &dst_ptr, item_size, mode, constant, VX_NULL);

        for (c = 0; c < inDepth; c++)
        {
            vxnnePAD(left_h_padding, outWidth, &dst_ptr, item_size, mode, constant, VX_NULL);

            for (h = 0; h < inHeight; h++)
            {
                vxnnePAD(left_w_padding, 1, &dst_ptr, item_size, mode, constant, VX_NULL);

                for (w = 0; w < inWidth; w++)
                {
                    memcpy(dst_ptr, src_ptr, item_size);
                    src_ptr += item_size;

                    dst_ptr += item_size;
                }

                vxnnePAD(right_w_padding, 1, &dst_ptr, item_size, mode, constant, VX_NULL);
            }
            vxnnePAD(right_h_padding, outWidth, &dst_ptr, item_size, mode, constant, VX_NULL);
        }
        vxnnePAD(right_c_padding, outWidth * outHeight, &dst_ptr, item_size, mode, constant, VX_NULL);
    }

    vxnnePAD(right_n_padding, outDepth * outWidth * outHeight, &dst_ptr, item_size, mode, constant, VX_NULL);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternelKernel_NNTensorPad(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorPad_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorPad_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

#if REGISTER_FRAME
VX_PRIVATE_API vx_status vxoNNTensorPad_SW_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vx_tensor  src                     = (vx_tensor)parameters[0];
    vx_tensor  dst                     = (vx_tensor)parameters[1];
    vx_scalar  padLeft                 = (vx_scalar)parameters[2];
    vx_scalar  padRight                = (vx_scalar)parameters[3];
    vx_scalar  padTop                  = (vx_scalar)parameters[4];
    vx_scalar  padBottom               = (vx_scalar)parameters[5];
    vx_scalar  padMode                 = (vx_scalar)parameters[6];
    vx_scalar  padConst                = (vx_scalar)parameters[7];
    vx_uint32  batchCount              = TENSOR_SIZE_INDEX(src, 3);
    vxnne_tensor_pad  padNode = (vxnne_tensor_pad)ops_layer;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);
    vxmONERROR(vxnneOperation_Initialize(&padNode->tensor_pad_operation.base,
                                &padNode->base,
                                VXNNE_OPERATION_TARGET_SW,
                                VXNNE_OPERATOR_TENSOR_PAD,
                                vxnneExecuteSWTensorPad,
                                VX_NULL,
                                batchCount,
                                0));

    vxnneLayer_SetOperation(
        &padNode->base,
        &padNode->tensor_pad_operation.base,
        0);

    padNode->tensor_pad_operation.src           = src;
    padNode->tensor_pad_operation.dst           = dst;
    padNode->tensor_pad_operation.padLeft       = padLeft;
    padNode->tensor_pad_operation.padRight      = padRight;
    padNode->tensor_pad_operation.padTop        = padTop;
    padNode->tensor_pad_operation.padBottom     = padBottom;
    padNode->tensor_pad_operation.padMode       = padMode;
    padNode->tensor_pad_operation.padConst      = padConst;

    vxmONERROR(vxnneOperation_AddReference(&padNode->tensor_pad_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&padNode->tensor_pad_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT));
    vxmONERROR(vxnneOperation_AddReference(&padNode->tensor_pad_operation.base, (vx_reference)padLeft, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&padNode->tensor_pad_operation.base, (vx_reference)padRight, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&padNode->tensor_pad_operation.base, (vx_reference)padTop, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&padNode->tensor_pad_operation.base, (vx_reference)padBottom, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&padNode->tensor_pad_operation.base, (vx_reference)padMode, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&padNode->tensor_pad_operation.base, (vx_reference)padConst, VXNNE_OPERATION_REFENRENCE_INPUT));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNTensorPad_SH_EVIS_Support_Ext(vx_node node, const vx_reference parameters[], vx_uint32 _num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_tensor  src                     = (vx_tensor)parameters[0];
    vx_tensor  dst                     = (vx_tensor)parameters[1];
    vx_scalar  padMode                 = (vx_scalar)parameters[6];

    vx_enum    inputFormat             = TENSOR_DATA_TYPE(src);
    vx_int8    inputFixPointPos        = TENSOR_POS(src);
    vx_int32   inputZeroPoint          = TENSOR_TF_ZEROPOINT(src);
    vx_float32 inputScale              = TENSOR_TF_SCALE(src);
    vx_uint32  inputElementSize        = TENSOR_DATA_SIZE(src);
    vx_enum    outputFormat            = TENSOR_DATA_TYPE(dst);
    vx_int8    outputFixPointPos       = TENSOR_POS(dst);
    vx_int32   outputZeroPoint         = TENSOR_TF_ZEROPOINT(dst);
    vx_float32 outputScale             = TENSOR_TF_SCALE(dst);
    vx_enum    padModeVal              = padMode->value->e;
    vx_bool    dataFormatFlag          = vx_false_e;
    vx_bool    tensorPadSHFlag         = vx_false_e;
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, _num, reg_param);

    if(evis)
    {
        dataFormatFlag = (vx_bool)((inputFormat == outputFormat) && (inputElementSize & 3) && (inputFixPointPos == outputFixPointPos)
            && (inputZeroPoint == outputZeroPoint) && (inputScale == outputScale));
    }
    else
    {
        dataFormatFlag = (vx_bool)(((inputFormat == outputFormat) && (inputFormat == VX_TYPE_FLOAT32 || inputFormat == VX_TYPE_FLOAT16)) ||
                                    ((inputFormat == VX_TYPE_UINT8) && (inputZeroPoint == outputZeroPoint) && (inputScale == outputScale)));
    }

    tensorPadSHFlag = (vx_bool)(dataFormatFlag && (padModeVal == VX_PAD_CONSTANT || padModeVal == VX_PAD_REPLICATE));

    support = tensorPadSHFlag && support;

    vxoLayer_VerificationFoot(node, parameters, _num, reg_param, &support);
    return support;
}

VX_PRIVATE_API vx_status vxoNNTensorPad_SH_Initialize_Ext(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 _num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_status status = VX_SUCCESS;
    vx_tensor  src                     = (vx_tensor)parameters[0];
    vx_tensor  dst                     = (vx_tensor)parameters[1];
    vx_scalar  padLeft                 = (vx_scalar)parameters[2];
    vx_scalar  padRight                = (vx_scalar)parameters[3];
    vx_scalar  padTop                  = (vx_scalar)parameters[4];
    vx_scalar  padBottom               = (vx_scalar)parameters[5];
    vx_scalar  padMode                 = (vx_scalar)parameters[6];
    vx_scalar  padConst                = (vx_scalar)parameters[7];
    vx_uint32  batchCount              = TENSOR_SIZE_INDEX(src, 3);
    vxnne_tensor_pad  padNode = (vxnne_tensor_pad)ops_layer;    vxnne_shader_executable shaderExecutable = NULL;
    vxoLayer_InitializeHead(ops_layer, parameters, _num, reg_param);

    if(evis)
    {
        shaderExecutable = vxnneGetTensorPadShaderExecutable(ops_layer->node->base.context,
                                                            VXNNE_KERNEL_TENSOR_PAD,
                                                            &ops_layer->node->kernelAttributes.borderMode,
                                                            src,
                                                            padLeft,
                                                            padRight,
                                                            padTop,
                                                            padBottom,
                                                            padMode,
                                                            padConst,
                                                            dst);
    }
    else
    {
        shaderExecutable = vxnneGetGPUTensorPadShaderExecutable(ops_layer->node->base.context,
                                                                VXNNE_KERNEL_GPU_TENSOR_PAD,
                                                                &ops_layer->node->kernelAttributes.borderMode,
                                                                src,
                                                                padLeft,
                                                                padRight,
                                                                padTop,
                                                                padBottom,
                                                                padMode,
                                                                padConst,
                                                                dst);
    }

    if (!shaderExecutable)
    {
        status = VX_FAILURE;
        goto OnError;
    }

    vxmONERROR(vxnneShaderOperation_Initialize(&padNode->tensor_pad_sh_operation,
                                                &padNode->base,
                                                VXNNE_OPERATOR_TENSOR_PAD,
                                                batchCount,
                                                shaderExecutable));

    vxmONERROR(vxnneLayer_SetOperation(
            &padNode->base,
            &padNode->tensor_pad_sh_operation.base,
            0));

    vxmONERROR(vxnneOperation_AddReference(&padNode->tensor_pad_sh_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&padNode->tensor_pad_sh_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, _num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNTensorPad_SH_EVIS_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && node->base.context->evisNoInst.supportEVIS;

    if (!support)return support;

    support = support && vxoNNTensorPad_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_true_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNTensorPad_SH_EVIS_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    status = vxoNNTensorPad_SH_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_true_e);

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNTensorPad_SH_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    support = support && vxoNNTensorPad_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_false_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNTensorPad_SH_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    status = vxoNNTensorPad_SH_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_false_e);

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNTensorPad_TP_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_tensor  src                     = (vx_tensor)parameters[0];
    vx_tensor  dst                     = (vx_tensor)parameters[1];
    vx_context context                 = vxGetContext((vx_reference)node);
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_TP, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    support = support && vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_TP);
    support = support && vxnneIsTPSupportFormat(context, src, VX_NULL, dst);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNTensorPad_TP_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vx_tensor  src                     = (vx_tensor)parameters[0];
    vx_tensor  dst                     = (vx_tensor)parameters[1];
    vx_scalar  padLeft                 = (vx_scalar)parameters[2];
    vx_scalar  padRight                = (vx_scalar)parameters[3];
    vx_scalar  padTop                  = (vx_scalar)parameters[4];
    vx_scalar  padBottom               = (vx_scalar)parameters[5];
    vx_scalar  padMode                 = (vx_scalar)parameters[6];
    vx_uint32  batchCount              = TENSOR_SIZE_INDEX(src, 3);
    vx_enum    padModeVal              = padMode->value->e;


    vxnne_tensor_pad  padNode = (vxnne_tensor_pad)ops_layer;
    vx_op_param_s conv = {0};
    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxnneOperation_Initialize(&padNode->tensor_pad_tp_operation.base,
        &padNode->base,
        VXNNE_OPERATION_TARGET_TP,
        VXNNE_OPERATOR_TENSOR_PAD,
        VX_NULL,
        VX_NULL,
        batchCount,
        0));

    memset(&conv, 0, sizeof(vx_op_param_s));

    conv.pad_x_left = padLeft->value->n32;
    conv.pad_y_top = padTop->value->n32;
    conv.pad_x_right = padRight->value->n32;
    conv.pad_y_bottom = padBottom->value->n32;
    conv.pad_mode = padModeVal;
    conv.pad_const = TENSOR_PAD_ZERO_VALUE(src);
    conv.pool_size_x = conv.pool_size_y = 0;
    conv.pool_stride = 1;
    conv.enable_relu = vx_false_e;
    conv.pool_stride = 1;
    conv.tpType = TP_TENSOR_PAD;

    memcpy(&padNode->tensor_pad_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));

    vxmONERROR(vxnneOperation_AddReference(&padNode->tensor_pad_tp_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&padNode->tensor_pad_tp_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    padNode->tensor_pad_tp_operation.input = src;
    padNode->tensor_pad_tp_operation.output = dst;

    vxmONERROR(vxnneLayer_SetOperation(
        &padNode->base,
        &padNode->tensor_pad_tp_operation.base,
        0));
OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_status vxoNNLayer_GetOperations(vxnne_layer ops_layer, vx_uint32_ptr max_num_operations, vxnne_operation **operations)
{
    vx_status  status = VX_SUCCESS;

    vxnne_tensor_pad  padNode = (vxnne_tensor_pad)ops_layer;

    *max_num_operations = gcmCOUNTOF(padNode->operations);

    *operations = padNode->operations;

    return status;
}

#endif
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorPad_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status  status                     = VX_SUCCESS;
#if REGISTER_FRAME
    vxnne_layer_imp_s registerTensorPad[] = {/* Please DON'T adjust the order, it's importent */
        { "TensorPad NN", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "TensorPad TP", vxoNNTensorPad_TP_Support, vxoNNTensorPad_TP_Initialize, VX_NULL },
        { "TensorPad SH EVIS", vxoNNTensorPad_SH_EVIS_Support, vxoNNTensorPad_SH_EVIS_Initialize, VX_NULL },
        { "TensorPad SH F32", vxoNNTensorPad_SH_Support, vxoNNTensorPad_SH_Initialize, VX_NULL },
        { "TensorPad SW ", vxoNNCommon_Support, vxoNNTensorPad_SW_Initialize, VX_NULL },
    };

    REGISTER_LAYERS(registerTensorPad, vxnne_tensor_pad_s, "TensorPadOperation", vxoNNLayer_GetOperations);

OnError:
#else
    vx_tensor  src                     = (vx_tensor)parameters[0];
    vx_tensor  dst                     = (vx_tensor)parameters[1];
    vx_scalar  padLeft                 = (vx_scalar)parameters[2];
    vx_scalar  padRight                = (vx_scalar)parameters[3];
    vx_scalar  padTop                  = (vx_scalar)parameters[4];
    vx_scalar  padBottom               = (vx_scalar)parameters[5];
    vx_scalar  padMode                 = (vx_scalar)parameters[6];
    vx_scalar  padConst                = (vx_scalar)parameters[7];
    vx_uint32  batchCount              = TENSOR_SIZE_INDEX(src, 3);

    vx_enum    inputFormat             = TENSOR_DATA_TYPE(src);
    vx_int8    inputFixPointPos        = TENSOR_POS(src);
    vx_int32   inputZeroPoint          = TENSOR_TF_ZEROPOINT(src);
    vx_float32 inputScale              = TENSOR_TF_SCALE(src);
    vx_uint32  inputElementSize        = TENSOR_DATA_SIZE(src);
    vx_enum    outputFormat            = TENSOR_DATA_TYPE(dst);
    vx_int8    outputFixPointPos       = TENSOR_POS(dst);
    vx_int32   outputZeroPoint         = TENSOR_TF_ZEROPOINT(dst);
    vx_float32 outputScale             = TENSOR_TF_SCALE(dst);
    vx_enum    padModeVal              = padMode->value->e;
    vx_bool    dataFormatFlag          = vx_false_e;
    vx_bool    tensorPadSHFlag         = vx_false_e;
    vx_context context                 = vxGetContext((vx_reference)node);

    vxnne_tensor_pad  padNode = VX_NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }
    gcoOS_Allocate(gcvNULL, sizeof(vxnne_tensor_pad_s), (gctPOINTER*)&padNode);
    if (!padNode)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(padNode, sizeof(vxnne_tensor_pad_s));

    vxnneLayer_Initialize(&padNode->base,
                          "TensorPadOperation",
                          node,
                          vxmOPERATION_COUNT(padNode),
                          padNode->operations,
                          VX_NULL);

    if(node->base.context->evisNoInst.supportEVIS)
    {
        dataFormatFlag = (vx_bool)((inputFormat == outputFormat) && (inputElementSize & 3) && (inputFixPointPos == outputFixPointPos)
            && (inputZeroPoint == outputZeroPoint) && (inputScale == outputScale));
    }
    else
    {
        dataFormatFlag = (vx_bool)(((inputFormat == outputFormat) && (inputFormat == VX_TYPE_FLOAT32 || inputFormat == VX_TYPE_FLOAT16)) ||
                                    ((inputFormat == VX_TYPE_UINT8) && (inputZeroPoint == outputZeroPoint) && (inputScale == outputScale)));
    }

    tensorPadSHFlag = (vx_bool)(dataFormatFlag && (padModeVal == VX_PAD_CONSTANT || padModeVal == VX_PAD_REPLICATE));

    if (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_TP) &&
        vxnneIsTPSupportFormat(context, src, VX_NULL, dst))
    {
        vx_op_param_s conv = {0};

        status = vxnneOperation_Initialize(&padNode->tensor_pad_tp_operation.base,
            &padNode->base,
            VXNNE_OPERATION_TARGET_TP,
            VXNNE_OPERATOR_TENSOR_PAD,
            VX_NULL,
            VX_NULL,
            batchCount,
            0);

        if (status != VX_SUCCESS) goto exit;

        memset(&conv, 0, sizeof(vx_op_param_s));

        conv.pad_x_left = padLeft->value->n32;
        conv.pad_y_top = padTop->value->n32;
        conv.pad_x_right = padRight->value->n32;
        conv.pad_y_bottom = padBottom->value->n32;
        conv.pad_mode = padModeVal;
        conv.pad_const = TENSOR_PAD_ZERO_VALUE(src);
        conv.pool_size_x = conv.pool_size_y = 0;
        conv.pool_stride = 1;
        conv.enable_relu = vx_false_e;
        conv.pool_stride = 1;
        conv.tpType = TP_TENSOR_PAD;

        memcpy(&padNode->tensor_pad_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));

        vxnneOperation_AddReference(&padNode->tensor_pad_tp_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&padNode->tensor_pad_tp_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        padNode->tensor_pad_tp_operation.input = src;
        padNode->tensor_pad_tp_operation.output = dst;

        vxnneLayer_SetOperation(
            &padNode->base,
            &padNode->tensor_pad_tp_operation.base,
            0);
    }
    else if(tensorPadSHFlag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
    {
        vxnne_shader_executable shaderExecutable = NULL;

        if(node->base.context->evisNoInst.supportEVIS)
        {
            shaderExecutable = vxnneGetTensorPadShaderExecutable(node->base.context,
                                                             VXNNE_KERNEL_TENSOR_PAD,
                                                             &node->kernelAttributes.borderMode,
                                                             src,
                                                             padLeft,
                                                             padRight,
                                                             padTop,
                                                             padBottom,
                                                             padMode,
                                                             padConst,
                                                             dst);
        }
        else
        {
            shaderExecutable = vxnneGetGPUTensorPadShaderExecutable(node->base.context,
                                                                 VXNNE_KERNEL_GPU_TENSOR_PAD,
                                                                 &node->kernelAttributes.borderMode,
                                                                 src,
                                                                 padLeft,
                                                                 padRight,
                                                                 padTop,
                                                                 padBottom,
                                                                 padMode,
                                                                 padConst,
                                                                 dst);
        }

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&padNode->tensor_pad_sh_operation,
                                                 &padNode->base,
                                                 VXNNE_OPERATOR_TENSOR_PAD,
                                                 batchCount,
                                                 shaderExecutable);
        if (status != VX_SUCCESS) goto exit;

        vxnneLayer_SetOperation(
                &padNode->base,
                &padNode->tensor_pad_sh_operation.base,
                0);

        vxnneOperation_AddReference(&padNode->tensor_pad_sh_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&padNode->tensor_pad_sh_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }
    else
    {
        vxnneOperation_Initialize(&padNode->tensor_pad_operation.base,
                                  &padNode->base,
                                  VXNNE_OPERATION_TARGET_SW,
                                  VXNNE_OPERATOR_TENSOR_PAD,
                                  vxnneExecuteSWTensorPad,
                                  VX_NULL,
                                  batchCount,
                                  0);

        vxnneLayer_SetOperation(
            &padNode->base,
            &padNode->tensor_pad_operation.base,
            0);

        padNode->tensor_pad_operation.src           = src;
        padNode->tensor_pad_operation.dst           = dst;
        padNode->tensor_pad_operation.padLeft       = padLeft;
        padNode->tensor_pad_operation.padRight      = padRight;
        padNode->tensor_pad_operation.padTop        = padTop;
        padNode->tensor_pad_operation.padBottom     = padBottom;
        padNode->tensor_pad_operation.padMode       = padMode;
        padNode->tensor_pad_operation.padConst      = padConst;

        vxnneOperation_AddReference(&padNode->tensor_pad_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&padNode->tensor_pad_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        vxnneOperation_AddReference(&padNode->tensor_pad_operation.base, (vx_reference)padLeft, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&padNode->tensor_pad_operation.base, (vx_reference)padRight, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&padNode->tensor_pad_operation.base, (vx_reference)padTop, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&padNode->tensor_pad_operation.base, (vx_reference)padBottom, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&padNode->tensor_pad_operation.base, (vx_reference)padMode, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&padNode->tensor_pad_operation.base, (vx_reference)padConst, VXNNE_OPERATION_REFENRENCE_INPUT);
    }

    node->layer = &padNode->base;
    return status;

exit:
    if (padNode != NULL) gcoOS_Free(NULL, padNode);
#endif
    return status;
}

#if REGISTER_FRAME
VX_PRIVATE_API vx_status vxoNNTensorPad2_SW_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vx_tensor  src = (vx_tensor)parameters[0];
    vx_tensor  dst = (vx_tensor)parameters[1];
    vx_tensor  pad_dims = (vx_tensor)parameters[2];
    vx_scalar  padMode = (vx_scalar)parameters[3];
    vx_scalar  padConst = (vx_scalar)parameters[4];
    vx_uint32  batchCount = 1;
    vx_int32_ptr pad_base = VX_NULL;

    vxnne_tensor_pad  padNode = (vxnne_tensor_pad)ops_layer;
    vxoTensor_GetTensorViewMemory(pad_dims, (gctPOINTER*)&pad_base, VX_NULL);
    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);
    vxmONERROR(vxnneOperation_Initialize(&padNode->tensor_pad_operation.base,
        &padNode->base,
        VXNNE_OPERATION_TARGET_SW,
        VXNNE_OPERATOR_TENSOR_PAD,
        vxnneExecuteSWTensorPad2,
        VX_NULL,
        batchCount,
        0));

    vxmONERROR(vxnneLayer_SetOperation(
        &padNode->base,
        &padNode->tensor_pad_operation.base,
        0));

    padNode->tensor_pad_operation.src       = src;
    padNode->tensor_pad_operation.dst       = dst;
    padNode->tensor_pad_operation.pad_dims  = pad_dims;
    padNode->tensor_pad_operation.padMode   = padMode;
    padNode->tensor_pad_operation.padConst  = padConst;

    vxmONERROR(vxnneOperation_AddReference(&padNode->tensor_pad_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&padNode->tensor_pad_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT));
    vxmONERROR(vxnneOperation_AddReference(&padNode->tensor_pad_operation.base, (vx_reference)pad_dims, VXNNE_OPERATION_REFENRENCE_INPUT));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNTensorPad2_SH_EVIS_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_tensor  src = (vx_tensor)parameters[0];
    vx_tensor  dst = (vx_tensor)parameters[1];
    vx_tensor  pad_dims = (vx_tensor)parameters[2];
    vx_scalar  padMode = (vx_scalar)parameters[3];
    vx_int32 inWidth = 0, inHeight = 0, inDepth = 0, inBatch = 0;
    vx_int32 outWidth = 0, outHeight = 0, outDepth = 0, outBatch = 0;
    vx_bool shader_flag = vx_false_e;
    vx_bool dataFormatFlag = vx_false_e;
    vx_bool fp2bfp16Flag = vx_false_e;
    vx_bool pad_flag = vx_false_e;
    vx_bool whc_flag = vx_false_e;
    vx_int32_ptr pad_base = VX_NULL;
    vx_enum pad_mode = padMode->value->e;

    vx_enum    inputFormat             = TENSOR_DATA_TYPE(src);
    vx_int8    inputFixPointPos        = TENSOR_POS(src);
    vx_int32   inputZeroPoint          = TENSOR_TF_ZEROPOINT(src);
    vx_float32 inputScale              = TENSOR_TF_SCALE(src);
    vx_uint32  inputElementSize        = TENSOR_DATA_SIZE(src);
    vx_enum    outputFormat            = TENSOR_DATA_TYPE(dst);
    vx_int8    outputFixPointPos       = TENSOR_POS(dst);
    vx_int32   outputZeroPoint         = TENSOR_TF_ZEROPOINT(dst);
    vx_float32 outputScale             = TENSOR_TF_SCALE(dst);

    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);
    support = support && node->base.context->evisNoInst.supportEVIS;

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    outWidth  = TENSOR_VIEW_SIZE_INDEX(dst, 0);
    outHeight = TENSOR_VIEW_SIZE_INDEX(dst, 1);
    outDepth  = TENSOR_VIEW_SIZE_INDEX(dst, 2);
    outBatch  = TENSOR_VIEW_SIZE_INDEX(dst, 3);
    inWidth   = TENSOR_VIEW_SIZE_INDEX(src, 0);
    inHeight  = TENSOR_VIEW_SIZE_INDEX(src, 1);
    inDepth   = TENSOR_VIEW_SIZE_INDEX(src, 2);
    inBatch   = TENSOR_VIEW_SIZE_INDEX(src, 3);
    vxoTensor_GetTensorViewMemory(pad_dims, (gctPOINTER*)&pad_base, VX_NULL);

    dataFormatFlag = (vx_bool)((inputFormat == outputFormat) && (inputElementSize & 3) && (inputFixPointPos == outputFixPointPos)
            && (inputZeroPoint == outputZeroPoint) && (inputScale == outputScale));

    fp2bfp16Flag = (vx_bool)(inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_BFLOAT16
                                && (pad_mode == VX_PAD_MIRROR_REFLECT || pad_mode == VX_PAD_MIRROR_SYMMETRIC));

    if(outDepth == inDepth && outBatch == inBatch)
    {
        pad_flag = vx_true_e;
        shader_flag = vx_true_e;
    }
    else if(outWidth == inWidth && outHeight == inHeight && pad_mode == VX_PAD_CONSTANT
        && ((outDepth != inDepth && outBatch == inBatch) || (outDepth == inDepth && outBatch != inBatch)))
    {
        shader_flag = vx_true_e;

        if(outBatch > 1)
        {
            if(outDepth != inDepth)
            {
                if(outWidth * outHeight < 65536
                    || outDepth * outBatch < 65536)
                    shader_flag = vx_true_e;
                else
                    shader_flag = vx_false_e;
            }
            else
            {
                if(outWidth * outHeight < 65536
                    || outHeight * outDepth < 65536
                    || outDepth * outBatch < 65536)
                    shader_flag = vx_true_e;
                else
                    shader_flag = vx_false_e;
            }
        }
    }
    else if(pad_mode == VX_PAD_CONSTANT && outBatch < 2)
    {
        shader_flag = vx_true_e;
        whc_flag = vx_true_e;
    }
    else if(pad_mode == VX_PAD_MIRROR_SYMMETRIC
        || pad_mode == VX_PAD_MIRROR_REFLECT)
    {
        shader_flag = vx_true_e;
    }

    support = support && shader_flag && (dataFormatFlag || fp2bfp16Flag);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_bool vxoNNTensorPad2_GPU_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_tensor  src = (vx_tensor)parameters[0];
    vx_tensor  dst = (vx_tensor)parameters[1];
    vx_scalar  padMode = (vx_scalar)parameters[3];
    vx_bool shader_flag = vx_false_e;
    vx_bool dataFormatFlag  = vx_false_e;
    vx_bool dataFormatFlag2 = vx_false_e;
    vx_enum pad_mode = padMode->value->e;

    vx_enum    inputFormat       = TENSOR_DATA_TYPE(src);
    vx_enum    outputFormat      = TENSOR_DATA_TYPE(dst);
    vx_int32   inputZeroPoint    = TENSOR_TF_ZEROPOINT(src);
    vx_float32 inputScale        = TENSOR_TF_SCALE(src);
    vx_int32   outputZeroPoint   = TENSOR_TF_ZEROPOINT(dst);
    vx_float32 outputScale       = TENSOR_TF_SCALE(dst);
    vx_uint32  indims            = TENSOR_DIM_NUM(src);
    vx_uint32  outdims           = TENSOR_DIM_NUM(dst);
    vx_int32   outDepth          = outdims > 2 ? TENSOR_VIEW_SIZE_INDEX(dst, 2) : 1;
    vx_int32   outBatch          = outdims > 3 ? TENSOR_VIEW_SIZE_INDEX(dst, 3) : 1;
    vx_int32   inDepth           = indims  > 2 ? TENSOR_VIEW_SIZE_INDEX(src, 2) : 1;
    vx_int32   inBatch           = indims  > 3 ? TENSOR_VIEW_SIZE_INDEX(src, 3) : 1;

    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    dataFormatFlag = (vx_bool)((pad_mode == VX_PAD_CONSTANT) && (inputFormat == outputFormat) &&
                            (inputFormat == VX_TYPE_FLOAT32 || inputFormat == VX_TYPE_UINT8 || inputFormat == VX_TYPE_FLOAT16));


    dataFormatFlag2 = (vx_bool)((inputFormat == outputFormat) && ((inputFormat == VX_TYPE_FLOAT32 || inputFormat == VX_TYPE_FLOAT16) ||
                                ((inputFormat == VX_TYPE_UINT8) && (inputZeroPoint == outputZeroPoint) && (inputScale == outputScale))));

    dataFormatFlag2 = dataFormatFlag2 && (vx_bool)(pad_mode == VX_PAD_REPLICATE && (outDepth == inDepth) && (outBatch == inBatch));

    if(dataFormatFlag || dataFormatFlag2)
    {
        shader_flag = vx_true_e;
    }

    support = support && shader_flag;

    if (support)
    {
        SETBIT(reg_param->flag, (dataFormatFlag == vx_true_e)?1:0, 0);
        SETBIT(reg_param->flag, (dataFormatFlag2 == vx_true_e)?1:0, 1);
    }

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNTensorPad2_SH_EVIS_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vx_tensor  src = (vx_tensor)parameters[0];
    vx_tensor  dst = (vx_tensor)parameters[1];
    vx_tensor  pad_dims = (vx_tensor)parameters[2];
    vx_scalar  padMode = (vx_scalar)parameters[3];
    vx_scalar  padConst = (vx_scalar)parameters[4];
    vx_uint32  batchCount = TENSOR_SIZE_INDEX(src, 3);

    vx_int32 inWidth = 0, inHeight = 0, inDepth = 0, inBatch = 0;
    vx_int32 outWidth = 0, outHeight = 0, outDepth = 0, outBatch = 0;
    vx_bool shader_flag = vx_false_e;
    vx_bool pad_flag = vx_false_e;
    vx_bool whc_flag = vx_false_e;
    vx_int32_ptr pad_base = VX_NULL;
    vx_enum pad_mode = padMode->value->e;
    vxnne_tensor_pad  padNode = (vxnne_tensor_pad)ops_layer;

    vxnne_shader_executable shaderExecutable;
    outWidth  = TENSOR_VIEW_SIZE_INDEX(dst, 0);
    outHeight = TENSOR_VIEW_SIZE_INDEX(dst, 1);
    outDepth = TENSOR_VIEW_SIZE_INDEX(dst, 2);
    outBatch = TENSOR_VIEW_SIZE_INDEX(dst, 3);
    inWidth  = TENSOR_VIEW_SIZE_INDEX(src, 0);
    inHeight = TENSOR_VIEW_SIZE_INDEX(src, 1);
    inDepth = TENSOR_VIEW_SIZE_INDEX(src, 2);
    inBatch = TENSOR_VIEW_SIZE_INDEX(src, 3);
    vxoTensor_GetTensorViewMemory(pad_dims, (gctPOINTER*)&pad_base, VX_NULL);

    if(outDepth == inDepth && outBatch == inBatch)
    {
        pad_flag = vx_true_e;
        shader_flag = vx_true_e;
    }
    else if(outWidth == inWidth && outHeight == inHeight && pad_mode == VX_PAD_CONSTANT
        && ((outDepth != inDepth && outBatch == inBatch) || (outDepth == inDepth && outBatch != inBatch)))
    {
        shader_flag = vx_true_e;

        if(outBatch > 1)
        {
            if(outDepth != inDepth)
            {
                if(outWidth * outHeight < 65536
                    || outDepth * outBatch < 65536)
                    shader_flag = vx_true_e;
                else
                    shader_flag = vx_false_e;
            }
            else
            {
                if(outWidth * outHeight < 65536
                    || outHeight * outDepth < 65536
                    || outDepth * outBatch < 65536)
                    shader_flag = vx_true_e;
                else
                    shader_flag = vx_false_e;
            }
        }
    }
    else if(pad_mode == VX_PAD_CONSTANT && outBatch < 2)
    {
        shader_flag = vx_true_e;
        whc_flag = vx_true_e;
    }
    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    if(pad_mode == VX_PAD_MIRROR_SYMMETRIC)
    {
        shaderExecutable = vxnneGetTensorPadSymShaderExecutable(ops_layer->node->base.context,
            VXNNE_KERNEL_TENSOR_PAD,
            &ops_layer->node->kernelAttributes.borderMode,
            src,
            dst,
            pad_base);
    }
    else if(pad_mode == VX_PAD_MIRROR_REFLECT)
    {
        shaderExecutable = vxnneGetTensorPadRefShaderExecutable(ops_layer->node->base.context,
            VXNNE_KERNEL_TENSOR_PAD,
            &ops_layer->node->kernelAttributes.borderMode,
            src,
            dst,
            pad_base);
    }
    else if(pad_flag)
    {
        vx_scalar padLeft = vxCreateScalar(ops_layer->node->base.context, VX_TYPE_FLOAT32, &pad_base[0]);
        vx_scalar padRight = vxCreateScalar(ops_layer->node->base.context, VX_TYPE_FLOAT32, &pad_base[1]);
        vx_scalar padTop = vxCreateScalar(ops_layer->node->base.context, VX_TYPE_FLOAT32, &pad_base[2]);
        vx_scalar padBottom = vxCreateScalar(ops_layer->node->base.context, VX_TYPE_FLOAT32, &pad_base[3]);

        shaderExecutable = vxnneGetTensorPadShaderExecutable(ops_layer->node->base.context,
            VXNNE_KERNEL_TENSOR_PAD,
            &ops_layer->node->kernelAttributes.borderMode,
            src,
            padLeft,
            padRight,
            padTop,
            padBottom,
            padMode,
            padConst,
            dst);

        vxReleaseScalar(&padLeft);
        vxReleaseScalar(&padRight);
        vxReleaseScalar(&padTop);
        vxReleaseScalar(&padBottom);
    }
    else if(outWidth * outHeight < 65536 && outDepth != inDepth && !whc_flag)
    {
        vx_uint32  reshpTensor_Sizes[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {1};
        vx_uint32  reshpTensor_Dims           = 3;
        vx_uint32 leftPad  = 0, topPad = 0, rightPad = 0, bottomPad = 0;
        vx_tensor input      = NULL;
        vx_tensor output     = NULL;
        vx_scalar padLeft, padRight, padTop, padBottom;

        reshpTensor_Sizes[0] = outWidth * outHeight;
        reshpTensor_Sizes[1] = outDepth;
        reshpTensor_Sizes[2] = outBatch == 0 ? 1: outBatch;
        output     = vxoTensor_ReshapeTensor(dst, (vx_int32*)reshpTensor_Sizes, reshpTensor_Dims);

        reshpTensor_Sizes[0] = inWidth * inHeight;
        reshpTensor_Sizes[1] = inDepth;
        reshpTensor_Sizes[2] = inBatch == 0 ? 1: inBatch;
        input     = vxoTensor_ReshapeTensor(src, (vx_int32*)reshpTensor_Sizes, reshpTensor_Dims);

        topPad = pad_base[4];
        bottomPad = pad_base[5];

        padLeft = vxCreateScalar(ops_layer->node->base.context, VX_TYPE_FLOAT32, &leftPad);
        padRight = vxCreateScalar(ops_layer->node->base.context, VX_TYPE_FLOAT32, &rightPad);
        padTop = vxCreateScalar(ops_layer->node->base.context, VX_TYPE_FLOAT32, &topPad);
        padBottom = vxCreateScalar(ops_layer->node->base.context, VX_TYPE_FLOAT32, &bottomPad);

        shaderExecutable = vxnneGetTensorPadShaderExecutable(ops_layer->node->base.context,
            VXNNE_KERNEL_TENSOR_PAD,
            &ops_layer->node->kernelAttributes.borderMode,
            input,
            padLeft,
            padRight,
            padTop,
            padBottom,
            padMode,
            padConst,
            output);

        vxReleaseScalar(&padLeft);
        vxReleaseScalar(&padRight);
        vxReleaseScalar(&padTop);
        vxReleaseScalar(&padBottom);

        vxoTensor_ReleaseTensor(&input);
        vxoTensor_ReleaseTensor(&output);
    }
    else if(outWidth * outHeight * inDepth < 65536 && inBatch != outBatch && !whc_flag)
    {
        vx_uint32  reshpTensor_Sizes[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {1};
        vx_uint32  reshpTensor_Dims           = 3;
        vx_uint32 leftPad  = 0, topPad = 0, rightPad = 0, bottomPad = 0;
        vx_tensor input      = NULL;
        vx_tensor output     = NULL;
        vx_scalar padLeft, padRight, padTop, padBottom;

        reshpTensor_Sizes[0] = outWidth * outHeight * outDepth;
        reshpTensor_Sizes[1] = outBatch == 0 ? 1: outBatch;
        reshpTensor_Sizes[2] = 1;
        output     = vxoTensor_ReshapeTensor(dst, (vx_int32*)reshpTensor_Sizes, reshpTensor_Dims);

        reshpTensor_Sizes[0] = inWidth * inHeight * inDepth;
        reshpTensor_Sizes[1] = inBatch == 0 ? 1: inBatch;
        reshpTensor_Sizes[2] = 1;
        input     = vxoTensor_ReshapeTensor(src, (vx_int32*)reshpTensor_Sizes, reshpTensor_Dims);

        topPad = pad_base[6];
        bottomPad = pad_base[7];

        padLeft = vxCreateScalar(ops_layer->node->base.context, VX_TYPE_FLOAT32, &leftPad);
        padRight = vxCreateScalar(ops_layer->node->base.context, VX_TYPE_FLOAT32, &rightPad);
        padTop = vxCreateScalar(ops_layer->node->base.context, VX_TYPE_FLOAT32, &topPad);
        padBottom = vxCreateScalar(ops_layer->node->base.context, VX_TYPE_FLOAT32, &bottomPad);

        shaderExecutable = vxnneGetTensorPadShaderExecutable(ops_layer->node->base.context,
            VXNNE_KERNEL_TENSOR_PAD,
            &ops_layer->node->kernelAttributes.borderMode,
            input,
            padLeft,
            padRight,
            padTop,
            padBottom,
            padMode,
            padConst,
            output);

        vxReleaseScalar(&padLeft);
        vxReleaseScalar(&padRight);
        vxReleaseScalar(&padTop);
        vxReleaseScalar(&padBottom);

        vxoTensor_ReleaseTensor(&input);
        vxoTensor_ReleaseTensor(&output);
    }
    else
    {
        shaderExecutable = vxnneGetTensorPad2ShaderExecutable(ops_layer->node->base.context,
            VXNNE_KERNEL_TENSOR_PAD2,
            &ops_layer->node->kernelAttributes.borderMode,
            src,
            padConst,
            dst,
            pad_base);
    }

    if (!shaderExecutable)
    {
        status = VX_FAILURE;
        goto OnError;
    }

    vxmONERROR(vxnneShaderOperation_Initialize(&padNode->tensor_pad_sh_operation,
        &padNode->base,
        VXNNE_OPERATOR_TENSOR_PAD,
        batchCount,
        shaderExecutable));

    vxmONERROR(vxnneOperation_AddReference(&padNode->tensor_pad_sh_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&padNode->tensor_pad_sh_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    vxmONERROR(vxnneLayer_SetOperation(
        &padNode->base,
        &padNode->tensor_pad_sh_operation.base,
        0));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_status vxoNNTensorPad2_GPU_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vx_tensor  src = (vx_tensor)parameters[0];
    vx_tensor  dst = (vx_tensor)parameters[1];
    vx_tensor  pad_dims = (vx_tensor)parameters[2];
    vx_scalar  padMode = (vx_scalar)parameters[3];
    vx_scalar  padConst = (vx_scalar)parameters[4];
    vx_uint32  batchCount = TENSOR_SIZE_INDEX(src, 3);

    vx_int32_ptr pad_base = VX_NULL;
    vxnne_tensor_pad  padNode = (vxnne_tensor_pad)ops_layer;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vx_bool dataFormatFlag    = GETBIT(reg_param->flag, 0);
    vx_bool dataFormatFlag2   = GETBIT(reg_param->flag, 1);




    vxoTensor_GetTensorViewMemory(pad_dims, (gctPOINTER*)&pad_base, VX_NULL);

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    if (dataFormatFlag)
    {
        shaderExecutable = vxnneGetGPUTensorPad2ShaderExecutable(ops_layer->node->base.context,
                VXNNE_KERNEL_GPU_TENSOR_PAD2,
                &ops_layer->node->kernelAttributes.borderMode,
                src,
                padConst,
                dst,
                pad_base);
    }
    else if (dataFormatFlag2)
    {
        vx_int32 left_h_padding = 0, left_w_padding = 0;
        vx_int32  right_h_padding = 0, right_w_padding = 0;
        vx_scalar padLeft                        = VX_NULL;
        vx_scalar padTop                         = VX_NULL;
        vx_scalar padRight                       = VX_NULL;
        vx_scalar padBottom                        = VX_NULL;

        left_h_padding  = pad_base[2];
        left_w_padding  = pad_base[0];
        right_h_padding = pad_base[3];
        right_w_padding = pad_base[1];
        padLeft         = vxCreateScalar(ops_layer->node->base.context, VX_TYPE_INT32, &left_w_padding);
        padTop          = vxCreateScalar(ops_layer->node->base.context, VX_TYPE_INT32, &left_h_padding);
        padRight        = vxCreateScalar(ops_layer->node->base.context, VX_TYPE_INT32, &right_w_padding);
        padBottom       = vxCreateScalar(ops_layer->node->base.context, VX_TYPE_INT32, &right_h_padding);
        shaderExecutable = vxnneGetGPUTensorPadShaderExecutable(ops_layer->node->base.context,
                                                                VXNNE_KERNEL_GPU_TENSOR_PAD,
                                                                &ops_layer->node->kernelAttributes.borderMode,
                                                                src,
                                                                padLeft,
                                                                padRight,
                                                                padTop,
                                                                padBottom,
                                                                padMode,
                                                                padConst,
                                                                dst);
        if(padLeft) vxReleaseScalar(&padLeft);
        if(padTop) vxReleaseScalar(&padTop);
        if(padBottom) vxReleaseScalar(&padBottom);
        if(padRight) vxReleaseScalar(&padRight);
    }

    if (!shaderExecutable)
    {
        status = VX_FAILURE;
        goto OnError;
    }

    vxmONERROR(vxnneShaderOperation_Initialize(&padNode->tensor_pad_sh_operation,
        &padNode->base,
        VXNNE_OPERATOR_TENSOR_PAD,
        batchCount,
        shaderExecutable));

    vxmONERROR(vxnneOperation_AddReference(&padNode->tensor_pad_sh_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&padNode->tensor_pad_sh_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    vxmONERROR(vxnneLayer_SetOperation(
        &padNode->base,
        &padNode->tensor_pad_sh_operation.base,
        0));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNTensorPad2_TP_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_tensor  src = (vx_tensor)parameters[0];
    vx_tensor  dst = (vx_tensor)parameters[1];
    vx_tensor  pad_dims = (vx_tensor)parameters[2];
    vx_scalar  padMode = (vx_scalar)parameters[3];

    vx_int32_ptr pad_base = VX_NULL;
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_TP, VX_TYPE_INVALID, VX_NULL);
    vxoTensor_GetTensorViewMemory(pad_dims, (gctPOINTER *)&pad_base, VX_NULL);

    if((padMode->value->e != VX_PAD_CONSTANT) && ((pad_base[4] != 0) || (pad_base[5] != 0)))
    {
        return vx_false_e;
    }
    else if ((padMode->value->e == VX_PAD_CONSTANT) && (pad_base[5] > 3))
    {
        return vx_false_e;
    }

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    support = support && vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_TP);
    support = support && vxnneIsTPSupportFormat(node->base.context, src, VX_NULL, dst);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNTensorPad2_TP_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vx_tensor  src = (vx_tensor)parameters[0];
    vx_tensor  dst = (vx_tensor)parameters[1];
    vx_tensor  pad_dims = (vx_tensor)parameters[2];
    vx_scalar  padMode = (vx_scalar)parameters[3];
    vx_uint32  batchCount = TENSOR_SIZE_INDEX(src, 3);
    vxnne_tensor_pad  padNode = (vxnne_tensor_pad)ops_layer;

    vx_int32 index = 0;
    vx_op_param_s conv = { 0 };
    vx_int32_ptr pad_base = VX_NULL;
    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);
    vxoTensor_GetTensorViewMemory(pad_dims, (gctPOINTER *)&pad_base, VX_NULL);

    vxmONERROR(vxnneOperation_Initialize(&padNode->tensor_pad_tp_operation.base,
        &padNode->base,
        VXNNE_OPERATION_TARGET_TP,
        VXNNE_OPERATOR_TENSOR_PAD,
        VX_NULL,
        VX_NULL,
        batchCount,
        0));

    /*pad width and height*/
    memset(&conv, 0, sizeof(vx_op_param_s));

    conv.pad_x_left = pad_base[0];
    conv.pad_y_top = pad_base[2];
    conv.pad_z_front = pad_base[4];
    conv.pad_x_right = pad_base[1];
    conv.pad_y_bottom = pad_base[3];
    conv.pad_z_back = pad_base[5];
    conv.pad_mode = padMode->value->e;
    conv.pad_const = 0;
    conv.pool_size_x = conv.pool_size_y = 0;
    conv.pool_stride = 1;
    conv.enable_relu = vx_false_e;
    conv.pool_stride = 1;
    conv.tpType = TP_TENSOR_PAD;

    memcpy(&padNode->tensor_pad_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));

    vxmONERROR(vxnneOperation_AddReference(&padNode->tensor_pad_tp_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&padNode->tensor_pad_tp_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    padNode->tensor_pad_tp_operation.input = src;
    padNode->tensor_pad_tp_operation.output = dst;

    vxmONERROR(vxnneLayer_SetOperation(
        &padNode->base,
        &padNode->tensor_pad_tp_operation.base,
        index++));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_status vxoNNLayer_GetOperations2(vxnne_layer ops_layer, vx_uint32_ptr max_num_operations, vxnne_operation **operations)
{
    vx_status  status = VX_SUCCESS;

    vxnne_tensor_pad  padNode = (vxnne_tensor_pad)ops_layer;

    *max_num_operations = gcmCOUNTOF(padNode->operations);

    *operations = padNode->operations;

    return status;
}
#endif
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorPad2_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status  status = VX_SUCCESS;
#if REGISTER_FRAME
    vxnne_layer_imp_s registerTensorPad2[] = {/* Please DON'T adjust the order, it's importent */
        { "TensorPad2 NN", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "TensorPad2 TP", vxoNNTensorPad2_TP_Support, vxoNNTensorPad2_TP_Initialize, VX_NULL },
        { "TensorPad2 SH EVIS", vxoNNTensorPad2_SH_EVIS_Support, vxoNNTensorPad2_SH_EVIS_Initialize, VX_NULL },
        { "TensorPad2 SH F32", vxoNNTensorPad2_GPU_Support, vxoNNTensorPad2_GPU_Initialize, VX_NULL },
        { "TensorPad2 SW ", vxoNNCommon_Support, vxoNNTensorPad2_SW_Initialize, VX_NULL },
    };

    REGISTER_LAYERS(registerTensorPad2, vxnne_tensor_pad_s, "TensorPadOperation2", vxoNNLayer_GetOperations2);

OnError:
#else
    vx_tensor  src = (vx_tensor)parameters[0];
    vx_tensor  dst = (vx_tensor)parameters[1];
    vx_tensor  pad_dims = (vx_tensor)parameters[2];
    vx_scalar  padMode = (vx_scalar)parameters[3];
    vx_scalar  padConst = (vx_scalar)parameters[4];
    vx_uint32  batchCount = TENSOR_SIZE_INDEX(src, 3);

    vxnne_tensor_pad  padNode = VX_NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }
    gcoOS_Allocate(gcvNULL, sizeof(vxnne_tensor_pad_s), (gctPOINTER*)&padNode);
    if (!padNode)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(padNode, sizeof(vxnne_tensor_pad_s));

    vxnneLayer_Initialize(&padNode->base,
        "TensorPadOperation2",
        node,
        vxmOPERATION_COUNT(padNode),
        padNode->operations,
        VX_NULL);
    if (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_TP) &&
        vxnneIsTPSupportFormat(node->base.context, src, VX_NULL, dst))
    {
        vx_int32 index = 0;
        vx_op_param_s conv = { 0 };
        vx_int32_ptr pad_base = VX_NULL;
        vxoTensor_GetTensorViewMemory(pad_dims, (gctPOINTER *)&pad_base, VX_NULL);

        status = vxnneOperation_Initialize(&padNode->tensor_pad_tp_operation.base,
            &padNode->base,
            VXNNE_OPERATION_TARGET_TP,
            VXNNE_OPERATOR_TENSOR_PAD,
            VX_NULL,
            VX_NULL,
            batchCount,
            0);

        if (status != VX_SUCCESS) goto exit;
        /*pad width and height*/
        memset(&conv, 0, sizeof(vx_op_param_s));

        conv.pad_x_left = pad_base[0];
        conv.pad_y_top = pad_base[2];
        conv.pad_x_right = pad_base[1];
        conv.pad_y_bottom = pad_base[3];
        conv.pad_mode = padMode->value->e;
        conv.pad_const = TENSOR_PAD_ZERO_VALUE(src);
        conv.pool_size_x = conv.pool_size_y = 0;
        conv.pool_stride = 1;
        conv.enable_relu = vx_false_e;
        conv.pool_stride = 1;
        conv.tpType = TP_TENSOR_PAD;

        memcpy(&padNode->tensor_pad_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));

        vxnneOperation_AddReference(&padNode->tensor_pad_tp_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&padNode->tensor_pad_tp_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        padNode->tensor_pad_tp_operation.input = src;
        padNode->tensor_pad_tp_operation.output = dst;

        vxnneLayer_SetOperation(
            &padNode->base,
            &padNode->tensor_pad_tp_operation.base,
            index++);

        /* pad depth and batch*/

    }
    else
    {
        vx_int32 inWidth = 0, inHeight = 0, inDepth = 0, inBatch = 0;
        vx_int32 outWidth = 0, outHeight = 0, outDepth = 0, outBatch = 0;
        vx_bool isFp32Flag = vx_false_e;
        vx_bool dataFormatFlag = vx_false_e;
        vx_bool shader_flag = vx_false_e;
        vx_bool pad_flag = vx_false_e;
        vx_bool whc_flag = vx_false_e;
        vx_int32_ptr pad_base = VX_NULL;
        vx_enum pad_mode = padMode->value->e;

        vx_enum    inputFormat             = TENSOR_DATA_TYPE(src);
        vx_int8    inputFixPointPos        = TENSOR_POS(src);
        vx_int32   inputZeroPoint          = TENSOR_TF_ZEROPOINT(src);
        vx_float32 inputScale              = TENSOR_TF_SCALE(src);
        vx_uint32  inputElementSize        = TENSOR_DATA_SIZE(src);
        vx_enum    outputFormat            = TENSOR_DATA_TYPE(dst);
        vx_int8    outputFixPointPos       = TENSOR_POS(dst);
        vx_int32   outputZeroPoint         = TENSOR_TF_ZEROPOINT(dst);
        vx_float32 outputScale             = TENSOR_TF_SCALE(dst);

        dataFormatFlag = (vx_bool)((inputFormat == outputFormat) && (inputElementSize & 3) && (inputFixPointPos == outputFixPointPos)
            && (inputZeroPoint == outputZeroPoint) && (inputScale == outputScale));
        isFp32Flag = (vx_bool)((inputFormat == outputFormat) && (inputFormat == VX_TYPE_FLOAT32));

        outWidth  = TENSOR_VIEW_SIZE_INDEX(dst, 0);
        outHeight = TENSOR_VIEW_SIZE_INDEX(dst, 1);
        outDepth = TENSOR_VIEW_SIZE_INDEX(dst, 2);
        outBatch = TENSOR_VIEW_SIZE_INDEX(dst, 3);
        inWidth  = TENSOR_VIEW_SIZE_INDEX(src, 0);
        inHeight = TENSOR_VIEW_SIZE_INDEX(src, 1);
        inDepth = TENSOR_VIEW_SIZE_INDEX(src, 2);
        inBatch = TENSOR_VIEW_SIZE_INDEX(src, 3);
        vxoTensor_GetTensorViewMemory(pad_dims, (gctPOINTER*)&pad_base, VX_NULL);

        if(outDepth == inDepth && outBatch == inBatch)
        {
            pad_flag = vx_true_e;
            shader_flag = vx_true_e;
        }
        else if(outWidth == inWidth && outHeight == inHeight && pad_mode == VX_PAD_CONSTANT
            && ((outDepth != inDepth && outBatch == inBatch) || (outDepth == inDepth && outBatch != inBatch)))
        {
            shader_flag = vx_true_e;

            if(outBatch > 1)
            {
                if(outDepth != inDepth)
                {
                    if(outWidth * outHeight < 65536
                        || outDepth * outBatch < 65536)
                        shader_flag = vx_true_e;
                    else
                        shader_flag = vx_false_e;
                }
                else
                {
                    if(outWidth * outHeight < 65536
                        || outHeight * outDepth < 65536
                        || outDepth * outBatch < 65536)
                        shader_flag = vx_true_e;
                    else
                        shader_flag = vx_false_e;
                }
            }
        }
        else if(pad_mode == VX_PAD_CONSTANT && outBatch < 2)
        {
            shader_flag = vx_true_e;
            whc_flag = vx_true_e;
        }

        if(!dataFormatFlag && !isFp32Flag)
        {
            shader_flag = vx_false_e;
        }

        if (shader_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
        {
            vxnne_shader_executable shaderExecutable;

            if(isFp32Flag)
            {
                shaderExecutable = vxnneGetGPUTensorPad2ShaderExecutable(node->base.context,
                    VXNNE_KERNEL_GPU_TENSOR_PAD2,
                    &node->kernelAttributes.borderMode,
                    src,
                    padConst,
                    dst,
                    pad_base);
            }
            else if(pad_flag)
            {
                vx_scalar padLeft = vxCreateScalar(node->base.context, VX_TYPE_FLOAT32, &pad_base[0]);
                vx_scalar padRight = vxCreateScalar(node->base.context, VX_TYPE_FLOAT32, &pad_base[1]);
                vx_scalar padTop = vxCreateScalar(node->base.context, VX_TYPE_FLOAT32, &pad_base[2]);
                vx_scalar padBottom = vxCreateScalar(node->base.context, VX_TYPE_FLOAT32, &pad_base[3]);

                shaderExecutable = vxnneGetTensorPadShaderExecutable(node->base.context,
                    VXNNE_KERNEL_TENSOR_PAD,
                    &node->kernelAttributes.borderMode,
                    src,
                    padLeft,
                    padRight,
                    padTop,
                    padBottom,
                    padMode,
                    padConst,
                    dst);

                vxReleaseScalar(&padLeft);
                vxReleaseScalar(&padRight);
                vxReleaseScalar(&padTop);
                vxReleaseScalar(&padBottom);
            }
            else if(outWidth * outHeight < 65536 && outDepth != inDepth && !whc_flag)
            {
                vx_uint32  reshpTensor_Sizes[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {1};
                vx_uint32  reshpTensor_Dims           = 3;
                vx_uint32 leftPad  = 0, topPad = 0, rightPad = 0, bottomPad = 0;
                vx_tensor input      = NULL;
                vx_tensor output     = NULL;
                vx_scalar padLeft, padRight, padTop, padBottom;

                reshpTensor_Sizes[0] = outWidth * outHeight;
                reshpTensor_Sizes[1] = outDepth;
                reshpTensor_Sizes[2] = outBatch == 0 ? 1: outBatch;
                output     = vxoTensor_ReshapeTensor(dst, (vx_int32*)reshpTensor_Sizes, reshpTensor_Dims);

                reshpTensor_Sizes[0] = inWidth * inHeight;
                reshpTensor_Sizes[1] = inDepth;
                reshpTensor_Sizes[2] = inBatch == 0 ? 1: inBatch;
                input     = vxoTensor_ReshapeTensor(src, (vx_int32*)reshpTensor_Sizes, reshpTensor_Dims);

                topPad = pad_base[4];
                bottomPad = pad_base[5];

                padLeft = vxCreateScalar(node->base.context, VX_TYPE_FLOAT32, &leftPad);
                padRight = vxCreateScalar(node->base.context, VX_TYPE_FLOAT32, &rightPad);
                padTop = vxCreateScalar(node->base.context, VX_TYPE_FLOAT32, &topPad);
                padBottom = vxCreateScalar(node->base.context, VX_TYPE_FLOAT32, &bottomPad);

                shaderExecutable = vxnneGetTensorPadShaderExecutable(node->base.context,
                    VXNNE_KERNEL_TENSOR_PAD,
                    &node->kernelAttributes.borderMode,
                    input,
                    padLeft,
                    padRight,
                    padTop,
                    padBottom,
                    padMode,
                    padConst,
                    output);

                vxReleaseScalar(&padLeft);
                vxReleaseScalar(&padRight);
                vxReleaseScalar(&padTop);
                vxReleaseScalar(&padBottom);

                vxoTensor_ReleaseTensor(&input);
                vxoTensor_ReleaseTensor(&output);
            }
            else if(outWidth * outHeight * inDepth < 65536 && inBatch != outBatch && !whc_flag)
            {
                vx_uint32  reshpTensor_Sizes[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {1};
                vx_uint32  reshpTensor_Dims           = 3;
                vx_uint32 leftPad  = 0, topPad = 0, rightPad = 0, bottomPad = 0;
                vx_tensor input      = NULL;
                vx_tensor output     = NULL;
                vx_scalar padLeft, padRight, padTop, padBottom;

                reshpTensor_Sizes[0] = outWidth * outHeight * outDepth;
                reshpTensor_Sizes[1] = outBatch == 0 ? 1: outBatch;
                reshpTensor_Sizes[2] = 1;
                output     = vxoTensor_ReshapeTensor(dst, (vx_int32*)reshpTensor_Sizes, reshpTensor_Dims);

                reshpTensor_Sizes[0] = inWidth * inHeight * inDepth;
                reshpTensor_Sizes[1] = inBatch == 0 ? 1: inBatch;
                reshpTensor_Sizes[2] = 1;
                input     = vxoTensor_ReshapeTensor(src, (vx_int32*)reshpTensor_Sizes, reshpTensor_Dims);

                topPad = pad_base[6];
                bottomPad = pad_base[7];

                padLeft = vxCreateScalar(node->base.context, VX_TYPE_FLOAT32, &leftPad);
                padRight = vxCreateScalar(node->base.context, VX_TYPE_FLOAT32, &rightPad);
                padTop = vxCreateScalar(node->base.context, VX_TYPE_FLOAT32, &topPad);
                padBottom = vxCreateScalar(node->base.context, VX_TYPE_FLOAT32, &bottomPad);

                shaderExecutable = vxnneGetTensorPadShaderExecutable(node->base.context,
                    VXNNE_KERNEL_TENSOR_PAD,
                    &node->kernelAttributes.borderMode,
                    input,
                    padLeft,
                    padRight,
                    padTop,
                    padBottom,
                    padMode,
                    padConst,
                    output);

                vxReleaseScalar(&padLeft);
                vxReleaseScalar(&padRight);
                vxReleaseScalar(&padTop);
                vxReleaseScalar(&padBottom);

                vxoTensor_ReleaseTensor(&input);
                vxoTensor_ReleaseTensor(&output);
            }
            else
            {
                shaderExecutable = vxnneGetTensorPad2ShaderExecutable(node->base.context,
                    VXNNE_KERNEL_TENSOR_PAD2,
                    &node->kernelAttributes.borderMode,
                    src,
                    padConst,
                    dst,
                    pad_base);
            }

            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto exit;
            }

            status = vxnneShaderOperation_Initialize(&padNode->tensor_pad_sh_operation,
                &padNode->base,
                VXNNE_OPERATOR_TENSOR_PAD,
                batchCount,
                shaderExecutable);
            if (status != VX_SUCCESS)
                goto exit;

            vxnneOperation_AddReference(&padNode->tensor_pad_sh_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&padNode->tensor_pad_sh_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            vxnneLayer_SetOperation(
                &padNode->base,
                &padNode->tensor_pad_sh_operation.base,
                0);
        }
        else
        {
            vxnneOperation_Initialize(&padNode->tensor_pad_operation.base,
                &padNode->base,
                VXNNE_OPERATION_TARGET_SW,
                VXNNE_OPERATOR_TENSOR_PAD,
                vxnneExecuteSWTensorPad2,
                VX_NULL,
                batchCount,
                0);

            vxnneLayer_SetOperation(
                &padNode->base,
                &padNode->tensor_pad_operation.base,
                0);

            padNode->tensor_pad_operation.src       = src;
            padNode->tensor_pad_operation.dst       = dst;
            padNode->tensor_pad_operation.pad_dims  = pad_dims;
            padNode->tensor_pad_operation.padMode   = padMode;
            padNode->tensor_pad_operation.padConst  = padConst;

            vxnneOperation_AddReference(&padNode->tensor_pad_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&padNode->tensor_pad_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            vxnneOperation_AddReference(&padNode->tensor_pad_operation.base, (vx_reference)pad_dims, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&padNode->tensor_pad_operation.base, (vx_reference)padMode, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&padNode->tensor_pad_operation.base, (vx_reference)padConst, VXNNE_OPERATION_REFENRENCE_INPUT);
        }
    }

    node->layer = &padNode->base;
    return status;

exit:
    if (padNode != NULL) gcoOS_Free(NULL, padNode);
#endif
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorPad_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}


