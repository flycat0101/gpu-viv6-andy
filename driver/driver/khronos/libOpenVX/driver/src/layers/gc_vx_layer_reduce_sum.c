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
#include <layers/gc_vx_layer_reduce_sum.h>

extern vx_status vxnneExecuteSWTensorTranspose(struct _vxnne_operation_s *operation);

vx_status vxnneExecuteSWTensorReduceSum(struct _vxnne_operation_s *operation)
{
    vxnne_tensor_reduce_sum_sw_operation           reduceOperation   = (vxnne_tensor_reduce_sum_sw_operation)operation;

    vx_tensor src  = (vx_tensor)reduceOperation->src;
    vx_tensor dst = (vx_tensor)reduceOperation->dst;
    vx_scalar rDim = (vx_scalar)reduceOperation->reduceDim;
    vx_scalar keep = (vx_scalar)reduceOperation->keepDim;

    gctPOINTER srcLogical = VX_NULL;
    gctPOINTER dstLogical = VX_NULL;

    vx_uint32 srcCount;
    vx_uint32 axis = 1024;
    vx_uint32 i;
    vx_bool keepDim = keep->value->b;
    vx_int8 srcFp = TENSOR_POS(src);
    vx_int8 dstFp = TENSOR_POS(dst);
    vx_enum   dstRoundingMode = TENSOR_ROUNDING_MODE(dst);
    vx_uint32 srcSizes[VX_CONTEXT_TENSOR_MAX_DIMENSION];

    if (src->isViewed)
    {
        for (i = 0; i < src->viewRegion.dimCount; i++)
        {
            srcSizes[i] = src->viewRegion.viewEnds[i] - src->viewRegion.viewStarts[i];
        }

    }
    else
    {
        for (i = 0; i < src->viewRegion.dimCount; i++)
        {
            srcSizes[i] = src->dims[i];
        }
    }

    if (rDim)
        axis = rDim->value->u32;

    if (keepDim)
    {
        vxmASSERT(src->viewRegion.dimCount == dst->viewRegion.dimCount);
    }
    else if (rDim && src->viewRegion.dimCount > 1)
    {
        vxmASSERT(src->viewRegion.dimCount == dst->viewRegion.dimCount + 1);
    }

    vxoTensor_GetTensorViewMemory(src, &srcLogical, VX_NULL);
    vxoTensor_GetTensorViewMemory(dst, &dstLogical, VX_NULL);

    if (!rDim)
    {
        /* When axis is NULL, if don't keep dimension, output is 1D tensor, otherwise output is 1x(dim count-2)x1 tensor*/
        vx_float32 sum = 0;
        vxoTensor_GetTensorElementCount(src, &srcCount);
        for (i = 0; i < srcCount; i++)
        {
            vx_float32 src0 = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(src), TENSOR_QUANT_TYPE(src), i, (vx_uint8_ptr)srcLogical, srcFp, TENSOR_TF_ZEROPOINT(src), TENSOR_TF_SCALE(src));
            sum += src0;

        }
        vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(dst), TENSOR_QUANT_TYPE(dst), 0, sum, (vx_uint8_ptr)dstLogical, dstFp, TENSOR_TF_ZEROPOINT(dst), TENSOR_TF_SCALE(dst), dstRoundingMode);
    }
    else if (axis == 0)
    {
        /* Reduce dimension[0]*/
        vx_uint32 srcOffset, dstOffset;
        vx_uint32 x, y, z, w;
        vx_float32 sum = 0;
        vx_float32 src0;

        switch (src->viewRegion.dimCount)
        {
        case 4:
            {
                for (w = 0; w < srcSizes[3]; w++)
                {
                    for (z = 0; z < srcSizes[2]; z++)
                    {
                        for (y = 0; y < srcSizes[1]; y++)
                        {
                            for (x = 0; x < srcSizes[0]; x++)
                            {
                                srcOffset = x + srcSizes[0] * y + srcSizes[0] * srcSizes[1] * z + srcSizes[0] * srcSizes[1] * srcSizes[2] * w;
                                src0 = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(src), TENSOR_QUANT_TYPE(src), srcOffset, (vx_uint8_ptr)srcLogical, srcFp, TENSOR_TF_ZEROPOINT(src), TENSOR_TF_SCALE(src));
                                sum += src0;
                            }
                            dstOffset = y + srcSizes[1] * z + srcSizes[1] * srcSizes[2] * w;
                            vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(dst), TENSOR_QUANT_TYPE(dst), dstOffset, sum, (vx_uint8_ptr)dstLogical, dstFp, TENSOR_TF_ZEROPOINT(dst), TENSOR_TF_SCALE(dst), dstRoundingMode);
                            sum = 0;
                        }

                    }
                }
            }
            break;

        case 3:
            {
                for (z = 0; z < srcSizes[2]; z++)
                {
                    for (y = 0; y < srcSizes[1]; y++)
                    {
                        for (x = 0; x < srcSizes[0]; x++)
                        {
                            srcOffset = x + srcSizes[0] * y + srcSizes[0] * srcSizes[1] * z;
                            src0 = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(src), TENSOR_QUANT_TYPE(src), srcOffset, (vx_uint8_ptr)srcLogical, srcFp, TENSOR_TF_ZEROPOINT(src), TENSOR_TF_SCALE(src));
                            sum += src0;
                        }
                        dstOffset = y + srcSizes[1] * z;
                        vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(dst), TENSOR_QUANT_TYPE(dst), dstOffset, sum, (vx_uint8_ptr)dstLogical, dstFp, TENSOR_TF_ZEROPOINT(dst), TENSOR_TF_SCALE(dst), dstRoundingMode);
                        sum = 0;
                    }
                }
            }
            break;

        case 2:
            {
                for (y = 0; y < srcSizes[1]; y++)
                {
                    for (x = 0; x < srcSizes[0]; x++)
                    {
                        srcOffset = x + srcSizes[0] * y;
                        src0 = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(src), TENSOR_QUANT_TYPE(src), srcOffset, (vx_uint8_ptr)srcLogical, srcFp, TENSOR_TF_ZEROPOINT(src), TENSOR_TF_SCALE(src));
                        sum += src0;
                    }
                    vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(dst), TENSOR_QUANT_TYPE(dst), y, sum, (vx_uint8_ptr)dstLogical, dstFp, TENSOR_TF_ZEROPOINT(dst), TENSOR_TF_SCALE(dst), dstRoundingMode);
                    sum = 0;
                }
            }
            break;

        case 1:
            {
                for (x = 0; x < srcSizes[0]; x++)
                {
                    src0 = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(src), TENSOR_QUANT_TYPE(src), x, (vx_uint8_ptr)srcLogical, srcFp, TENSOR_TF_ZEROPOINT(src), TENSOR_TF_SCALE(src));
                    sum += src0;
                }
                vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(dst), TENSOR_QUANT_TYPE(dst), 0, sum, (vx_uint8_ptr)dstLogical, dstFp, TENSOR_TF_ZEROPOINT(dst), TENSOR_TF_SCALE(dst), dstRoundingMode);
            }
            break;

        default:
            vxError("Invalid input dimention count");
            return VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else if (axis == 1)
    {
        /* Reduce dimension[1], dimention count should lager than 1*/
        vx_uint32 srcOffset, dstOffset;
        vx_uint32 x, y, z, w;
        vx_float32 sum = 0;
        vx_float32 src0;

        switch (src->viewRegion.dimCount)
        {
        case 4:
            {
                for (w = 0; w < srcSizes[3]; w++)
                {
                    for (z = 0; z < srcSizes[2]; z++)
                    {
                        for (x = 0; x < srcSizes[0]; x++)
                        {
                            for (y = 0; y < srcSizes[1]; y++)
                            {
                                srcOffset = x + srcSizes[0] * y + srcSizes[0] * srcSizes[1] * z + srcSizes[0] * srcSizes[1] * srcSizes[2] * w;
                                src0 = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(src), TENSOR_QUANT_TYPE(src), srcOffset, (vx_uint8_ptr)srcLogical, srcFp, TENSOR_TF_ZEROPOINT(src), TENSOR_TF_SCALE(src));
                                sum += src0;
                            }
                            dstOffset = x + srcSizes[0] * z + srcSizes[0] * srcSizes[2] * w;
                            vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(dst), TENSOR_QUANT_TYPE(dst), dstOffset, sum, (vx_uint8_ptr)dstLogical, dstFp, TENSOR_TF_ZEROPOINT(dst), TENSOR_TF_SCALE(dst), dstRoundingMode);
                            sum = 0;
                        }

                    }
                }
            }
            break;

        case 3:
            {
                for (z = 0; z < srcSizes[2]; z++)
                {
                    for (x = 0; x < srcSizes[0]; x++)
                    {
                        for (y = 0; y < srcSizes[1]; y++)
                        {
                            srcOffset = x + srcSizes[0] * y + srcSizes[0] * srcSizes[1] * z;
                            src0 = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(src), TENSOR_QUANT_TYPE(src), srcOffset, (vx_uint8_ptr)srcLogical, srcFp, TENSOR_TF_ZEROPOINT(src), TENSOR_TF_SCALE(src));
                            sum += src0;
                        }
                        dstOffset = x + srcSizes[0] * z;
                        vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(dst), TENSOR_QUANT_TYPE(dst), dstOffset, sum, (vx_uint8_ptr)dstLogical, dstFp, TENSOR_TF_ZEROPOINT(dst), TENSOR_TF_SCALE(dst), dstRoundingMode);
                        sum = 0;
                    }
                }
            }
            break;

        case 2:
            {
                for (x = 0; x < srcSizes[0]; x++)
                {
                    for (y = 0; y < srcSizes[1]; y++)
                    {
                        srcOffset = x + srcSizes[0] * y;
                        src0 = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(src), TENSOR_QUANT_TYPE(src), srcOffset, (vx_uint8_ptr)srcLogical, srcFp, TENSOR_TF_ZEROPOINT(src), TENSOR_TF_SCALE(src));
                        sum += src0;
                    }
                    vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(dst), TENSOR_QUANT_TYPE(dst), x, sum, (vx_uint8_ptr)dstLogical, dstFp, TENSOR_TF_ZEROPOINT(dst), TENSOR_TF_SCALE(dst), dstRoundingMode);
                    sum = 0;
                }
            }
            break;

        default:
            vxError("Invalid input dimention count");
            return VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else if (axis == 2)
    {
        /* Reduce dimension[2], dimention count should lager than 2*/
        vx_uint32 srcOffset, dstOffset;
        vx_uint32 x, y, z, w;
        vx_float32 sum = 0;
        vx_float32 src0;

        switch (src->viewRegion.dimCount)
        {
        case 4:
            {
                for (w = 0; w < srcSizes[3]; w++)
                {
                    for (y = 0; y < srcSizes[1]; y++)
                    {
                        for (x = 0; x < srcSizes[0]; x++)
                        {
                            for (z = 0; z < srcSizes[2]; z++)
                            {
                                srcOffset = x + srcSizes[0] * y + srcSizes[0] * srcSizes[1] * z + srcSizes[0] * srcSizes[1] * srcSizes[2] * w;
                                src0 = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(src), TENSOR_QUANT_TYPE(src), srcOffset, (vx_uint8_ptr)srcLogical, srcFp, TENSOR_TF_ZEROPOINT(src), TENSOR_TF_SCALE(src));
                                sum += src0;
                            }
                            dstOffset = x + srcSizes[0] * y + srcSizes[0] * srcSizes[1] * w;
                            vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(dst), TENSOR_QUANT_TYPE(dst), dstOffset, sum, (vx_uint8_ptr)dstLogical, dstFp, TENSOR_TF_ZEROPOINT(dst), TENSOR_TF_SCALE(dst), dstRoundingMode);
                            sum = 0;
                        }

                    }
                }
            }
            break;

        case 3:
            {
                for (y = 0; y < srcSizes[1]; y++)
                {
                    for (x = 0; x < srcSizes[0]; x++)
                    {
                        for (z = 0; z < srcSizes[2]; z++)
                        {
                            srcOffset = x + srcSizes[0] * y + srcSizes[0] * srcSizes[1] * z;
                            src0 = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(src), TENSOR_QUANT_TYPE(src), srcOffset, (vx_uint8_ptr)srcLogical, srcFp, TENSOR_TF_ZEROPOINT(src), TENSOR_TF_SCALE(src));
                            sum += src0;
                        }
                        dstOffset = x + srcSizes[0] * y;
                        vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(dst), TENSOR_QUANT_TYPE(dst), dstOffset, sum, (vx_uint8_ptr)dstLogical, dstFp, TENSOR_TF_ZEROPOINT(dst), TENSOR_TF_SCALE(dst), dstRoundingMode);
                        sum = 0;
                    }
                }
            }
            break;

        default:
            vxError("Invalid input dimention count");
            return VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else if (axis == 3)
    {
        /* Reduce dimension[3], dimention count should lager than 3*/
        vx_uint32 srcOffset, dstOffset;
        vx_uint32 x, y, z, w;
        vx_float32 sum = 0;
        vx_float32 src0;

        switch (src->viewRegion.dimCount)
        {
        case 4:
            {
                for (z = 0; z < srcSizes[2]; z++)
                {
                    for (y = 0; y < srcSizes[1]; y++)
                    {
                        for (x = 0; x < srcSizes[0]; x++)
                        {
                            for (w = 0; w < srcSizes[3]; w++)
                            {
                                srcOffset = x + srcSizes[0] * y + srcSizes[0] * srcSizes[1] * z + srcSizes[0] * srcSizes[1] * srcSizes[2] * w;
                                src0 = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(src), TENSOR_QUANT_TYPE(src), srcOffset, (vx_uint8_ptr)srcLogical, srcFp, TENSOR_TF_ZEROPOINT(src), TENSOR_TF_SCALE(src));
                                sum += src0;
                            }
                            dstOffset = x + srcSizes[0] * y + srcSizes[0] * srcSizes[1] * z;
                            vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(dst), TENSOR_QUANT_TYPE(dst), dstOffset, sum, (vx_uint8_ptr)dstLogical, dstFp, TENSOR_TF_ZEROPOINT(dst), TENSOR_TF_SCALE(dst), dstRoundingMode);
                            sum = 0;
                        }

                    }
                }
            }
            break;

        default:
            vxError("Invalid input dimention count");
            return VX_ERROR_INVALID_PARAMETERS;
        }
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternelKernel_NNTensorReduceSum(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorReduceSum_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorReduceSum_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}
#if REGISTER_FRAME

VX_PRIVATE_API vx_status vxoNNTensorReduceSum_SW_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_tensor  src                     = (vx_tensor)parameters[0];
    vx_tensor  dst                     = (vx_tensor)parameters[1];
    vx_scalar  reduceDim               = (vx_scalar)parameters[2];
    vx_scalar  keepDim                 = (vx_scalar)parameters[3];
    vx_uint32  batchCount              = TENSOR_SIZE_INDEX(src, 3);

    vx_status status = VX_SUCCESS;
    vxnne_tensor_reduce_sum  reduceNode = (vxnne_tensor_reduce_sum)ops_layer;

    vxmONERROR(vxnneOperation_Initialize(&reduceNode->tensor_reduce_sum_operation.base,
        &reduceNode->base,
        VXNNE_OPERATION_TARGET_SW,
        VXNNE_OPERATOR_TENSOR_REDUCE_SUM,
        vxnneExecuteSWTensorReduceSum,
        VX_NULL,
        batchCount,
        0));

    vxmONERROR(vxnneLayer_SetOperation(
        &reduceNode->base,
        &reduceNode->tensor_reduce_sum_operation.base,
        0));

    reduceNode->tensor_reduce_sum_operation.src           = src;
    reduceNode->tensor_reduce_sum_operation.dst           = dst;
    reduceNode->tensor_reduce_sum_operation.reduceDim     = reduceDim;
    reduceNode->tensor_reduce_sum_operation.keepDim       = keepDim;

    vxmONERROR(vxnneOperation_AddReference(&reduceNode->tensor_reduce_sum_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&reduceNode->tensor_reduce_sum_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT));
    if (reduceDim)
        vxmONERROR(vxnneOperation_AddReference(&reduceNode->tensor_reduce_sum_operation.base, (vx_reference)reduceDim, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&reduceNode->tensor_reduce_sum_operation.base, (vx_reference)keepDim, VXNNE_OPERATION_REFENRENCE_INPUT));
OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNTensorReduceSum_SH_EVIS_Support_Ext(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_tensor  src                     = (vx_tensor)parameters[0];
    vx_tensor  dst                     = (vx_tensor)parameters[1];
    vx_scalar  reduceDim               = (vx_scalar)parameters[2];

    vx_uint32  axis                    = 1024;
    vx_enum    inputFormat             = TENSOR_DATA_TYPE(src);
    vx_enum    outputFormat            = TENSOR_DATA_TYPE(dst);


    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    if (reduceDim)
    {
        axis = reduceDim->value->u32;
        if (axis >= (vx_uint32)TENSOR_DIM_NUM(src))
        {
            vxError("Invalid input dimention %d the axis value must be in the range [0, %d) function %s line %d", axis, TENSOR_DIM_NUM(src), __FUNCTION__, __LINE__);
            support = vx_false_e;
        }
    }
    else
    {
        vxError("input params reduceDim is NULL function %s line %d", __FUNCTION__, __LINE__);
        support = vx_false_e;
    }

    if (!support)return support;

    if(evis)
    {
        support = (vx_bool)(((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
            || (inputFormat == VX_TYPE_BFLOAT16 && outputFormat == VX_TYPE_BFLOAT16)
            || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT16)
            || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT8)
            || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_UINT8)
            || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16)
            || (inputFormat == VX_TYPE_INT8  && outputFormat == VX_TYPE_INT8)
            || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_FLOAT16)
            || (inputFormat == VX_TYPE_INT8  && outputFormat == VX_TYPE_FLOAT16)
            || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
            || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT16))
            && (reduceDim != NULL)
            && (axis <= 3));
    }
    else
    {
        support = (vx_bool)(((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
            || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32)
            || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8))
            && (reduceDim != NULL)
            && (axis <= 3));
    }

    if (support)
        reg_param->flag = axis;

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_bool vxoNNTensorReduceSum_SH_EVIS_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && node->base.context->evisNoInst.supportEVIS;

    if (!support)return support;

    support = support && vxoNNTensorReduceSum_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_true_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNTensorReduceSum_SH_EVIS_Initialize_Ext(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_tensor  src                     = (vx_tensor)parameters[0];
    vx_tensor  dst                     = (vx_tensor)parameters[1];
    vx_uint32  batchCount              = TENSOR_SIZE_INDEX(src, 3);

    vx_uint32  axis                    = reg_param->flag;
    vx_uint32  tmpTensorIndex          = 0;
    vx_uint32  operationIdx            = 0;

    vx_status status = VX_SUCCESS;

    vxnne_tensor_reduce_sum  reduceNode = (vxnne_tensor_reduce_sum)ops_layer;

    vx_tensor_create_params_t tensor_create_params;
    vx_uint32 dims          = TENSOR_DIM_NUM(src);
    vx_uint32 width         = TENSOR_VIEW_SIZE_INDEX(src, 0);
    vx_uint32 height        = dims > 1 ? TENSOR_VIEW_SIZE_INDEX(src, 1) : 1;
    vx_uint32 depth         = dims > 2 ? TENSOR_VIEW_SIZE_INDEX(src, 2) : 1;
    vx_uint32 batch         = dims > 3 ? TENSOR_VIEW_SIZE_INDEX(src, 3) : 1;
    vx_uint32 sizes[4]      = {width, height, depth, batch};
    vx_uint32 new_sizes[4]  = {width, height, depth, batch};
    vx_uint32 perm_array[4] = {0, 1, 2, 3};
    vx_tensor transTensor   = NULL;
    vx_uint32 i             = 0;
    vx_bool   enable_trans  = vx_false_e;
    vx_bool   enable_axis   = vx_true_e;
    vx_bool   is_trans_sw   = vx_false_e;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    if (axis == 3)
    {
        enable_trans = vx_true_e;

        if (axis == 3)
        {
            axis = 0;
            perm_array[0] = 3;
            perm_array[1] = 0;
            perm_array[2] = 1;
            perm_array[3] = 2;
            is_trans_sw   = vx_true_e;
        }
    }
    else
    {
        transTensor = src;
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
        tensor_create_params.data_format = TENSOR_DATA_TYPE(src);
        tensor_create_params.quant_format = TENSOR_QUANT_TYPE(src);
        if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
        {
            tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(src);
        }
        else
        {
            tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(src);
            tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(src);
        }

        transTensor = vxoTensor_CreateTensor(ops_layer->node->base.context, ops_layer->node->graph, &tensor_create_params, vx_true_e);

        reduceNode->base.temp_tensors[tmpTensorIndex++] = transTensor;

        if (vxoContext_IsFeatureAvailable(ops_layer->node->base.context, VX_NN_FEATURE_TP_TRANSPOSE) &&
            vxnneIsTPSupportFormat(ops_layer->node->base.context, src, VX_NULL, transTensor))
        {
            vx_op_param_s conv = {0};
            vx_uint32 dnum = 4;

            vxmONERROR(vxnneOperation_Initialize(&reduceNode->tensor_reduce_sum_trans_tp_operation.base,
                &reduceNode->base,
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
            conv.other_ref = (vx_reference)src;
            conv.data_buff = gcvNULL;
            conv.tp_value = (vx_tp_value_cmd)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
            conv.tp_value->u32[0] = dnum;
            conv.tp_value->p8[0] = (vx_uint8_ptr)vxAllocateAndZeroMemory(sizeof(vx_uint32) * dnum);
            vxMemCopy(conv.tp_value->p8[0], perm_array, sizeof(vx_uint32) * dnum);

            vxMemCopy(&reduceNode->tensor_reduce_sum_trans_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));

            vxmONERROR(vxnneLayer_SetOperation(
                &reduceNode->base,
                &reduceNode->tensor_reduce_sum_trans_tp_operation.base,
                operationIdx++));

            reduceNode->tensor_reduce_sum_trans_tp_operation.input  = src;
            reduceNode->tensor_reduce_sum_trans_tp_operation.output = transTensor;

            vxmONERROR(vxnneOperation_AddReference(&reduceNode->tensor_reduce_sum_trans_tp_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT));
            vxmONERROR(vxnneOperation_AddReference(&reduceNode->tensor_reduce_sum_trans_tp_operation.base, (vx_reference)transTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT));
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
                    elementCnt *= TENSOR_VIEW_SIZE_INDEX(src, i);
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
                    shaderExecutable = vxnneTensorTransposeShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_TRANSPOSE, &ops_layer->node->kernelAttributes.borderMode, src, perm_array, dims, transTensor);
                }
                else
                {
                    shaderExecutable = vxnneGPUTensorTransposeShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_GPU_TENSOR_TRANSPOSE, &ops_layer->node->kernelAttributes.borderMode, src, perm_array, dims, transTensor);
                }

                if (!shaderExecutable)
                {
                    status = VX_FAILURE;
                    goto OnError;
                }
                vxmONERROR(vxnneShaderOperation_Initialize(&reduceNode->tensor_reduce_sum_trans_sh_operation,
                    &reduceNode->base,
                    VXNNE_OPERATOR_TENSOR_TRANS,
                    batchCount,
                    shaderExecutable));

                vxmONERROR(vxnneOperation_AddReference(&reduceNode->tensor_reduce_sum_trans_sh_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT));
                vxmONERROR(vxnneOperation_AddReference(&reduceNode->tensor_reduce_sum_trans_sh_operation.base, (vx_reference)transTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT));

                vxmONERROR(vxnneLayer_SetOperation(
                    &reduceNode->base,
                    &reduceNode->tensor_reduce_sum_trans_sh_operation.base,
                    operationIdx++));
            }
            else
            {
                vx_array perm = vxCreateArray(ops_layer->node->base.context, VX_TYPE_UINT32, pnum);
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

                pnum_s = vxCreateScalar(ops_layer->node->base.context, VX_TYPE_UINT32, &pnum);

                vxmONERROR(vxnneOperation_Initialize(&reduceNode->tensor_trans_sw_operation.base,
                                          &reduceNode->base,
                                          VXNNE_OPERATION_TARGET_SW,
                                          VXNNE_OPERATOR_TENSOR_TRANS,
                                          vxnneExecuteSWTensorTranspose,
                                          VX_NULL,
                                          batchCount,
                                          0));

                vxmONERROR(vxnneLayer_SetOperation(
                    &reduceNode->base,
                    &reduceNode->tensor_trans_sw_operation.base,
                    operationIdx++));

                reduceNode->tensor_trans_sw_operation.input   = src;
                reduceNode->tensor_trans_sw_operation.perm    = perm;
                reduceNode->tensor_trans_sw_operation.pnum    = pnum_s;
                reduceNode->tensor_trans_sw_operation.output  = transTensor;

                vxmONERROR(vxnneOperation_AddReference(&reduceNode->tensor_trans_sw_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT));
                vxmONERROR(vxnneOperation_AddReference(&reduceNode->tensor_trans_sw_operation.base, (vx_reference)transTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT));
            }
        }
    }

    if (enable_axis)
    {
        vxnne_shader_executable shaderExecutable = NULL;
        vx_float32              axis_coef        = 1.0f;
        vx_uint32               batch            = new_sizes[3];

        if(evis)
        {
            shaderExecutable = vxnneGetTensorMeanAxisShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_MEAN_AXIS, &ops_layer->node->kernelAttributes.borderMode, axis_coef, transTensor, dst, axis);
        }
        else
        {
            shaderExecutable = vxnneGetGPUTensorMeanAxisShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_GPU_TENSOR_MEAN_AXIS, &ops_layer->node->kernelAttributes.borderMode, axis_coef, transTensor, dst, axis);
        }

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto OnError;
        }

        vxmONERROR(vxnneShaderOperation_Initialize(&reduceNode->tensor_reduce_sum_sh_operation,
            &reduceNode->base,
            VXNNE_OPERATOR_TENSOR_MEAN,
            batch,
            shaderExecutable));

        vxmONERROR(vxnneOperation_AddReference(&reduceNode->tensor_reduce_sum_sh_operation.base, (vx_reference)transTensor, VXNNE_OPERATION_REFENRENCE_INPUT));
        vxmONERROR(vxnneOperation_AddReference(&reduceNode->tensor_reduce_sum_sh_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT));

        vxmONERROR(vxnneLayer_SetOperation(
            &reduceNode->base,
            &reduceNode->tensor_reduce_sum_sh_operation.base,
            operationIdx++));
    }

    reduceNode->base.num_temp_tensors = tmpTensorIndex;

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}
VX_PRIVATE_API vx_status vxoNNTensorReduceSum_SH_EVIS_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxoNNTensorReduceSum_SH_EVIS_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_true_e));
OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status vxoNNTensorReduceSum_SH_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxoNNTensorReduceSum_SH_EVIS_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_false_e));
OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_bool vxoNNTensorReduceSum_SH_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    support = support && vxoNNTensorReduceSum_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_false_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNLayer_GetOperations(vxnne_layer ops_layer, vx_uint32_ptr max_num_operations, vxnne_operation **operations)
{
    vx_status  status = VX_SUCCESS;
    vxnne_tensor_reduce_sum  reduceNode = (vxnne_tensor_reduce_sum)ops_layer;

    *max_num_operations = gcmCOUNTOF(reduceNode->operations);

    *operations = reduceNode->operations;

    return status;
}

#endif

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorReduceSum_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status  status                     = VX_SUCCESS;
#if REGISTER_FRAME
    vxnne_layer_imp_s registerTensorReduceSums[] = {/* Please DON'T adjust the order, it's importent */
        { "Reduce Sum NN", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "Reduce Sum TP", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "Reduce Sum SH EVIS", vxoNNTensorReduceSum_SH_EVIS_Support, vxoNNTensorReduceSum_SH_EVIS_Initialize, VX_NULL },
        { "Reduce Sum SH F32", vxoNNTensorReduceSum_SH_Support, vxoNNTensorReduceSum_SH_Initialize, VX_NULL },
        { "Reduce Sum SW ", vxoNNCommon_Support, vxoNNTensorReduceSum_SW_Initialize, VX_NULL },
    };

    REGISTER_LAYERS(registerTensorReduceSums, vxnne_tensor_reduce_sum_s, "TensorReduceSum", vxoNNLayer_GetOperations);

OnError:
#else

    vx_tensor  src                     = (vx_tensor)parameters[0];
    vx_tensor  dst                     = (vx_tensor)parameters[1];
    vx_scalar  reduceDim               = (vx_scalar)parameters[2];
    vx_scalar  keepDim                 = (vx_scalar)parameters[3];
    vx_uint32  batchCount              = TENSOR_SIZE_INDEX(src, 3);

    vx_uint32  axis                    = 1024;
    vx_bool    shExe_flag              = vx_false_e;
    vx_enum    inputFormat             = TENSOR_DATA_TYPE(src);
    vx_enum    outputFormat            = TENSOR_DATA_TYPE(dst);
    vx_uint32  tmpTensorIndex          = 0;
    vx_uint32  operationIdx            = 0;

    vx_context context                 = vxGetContext((vx_reference)node);

    vxnne_tensor_reduce_sum  reduceNode = VX_NULL;
    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_tensor_reduce_sum_s), (gctPOINTER*)&reduceNode);
    if (!reduceNode)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(reduceNode, sizeof(vxnne_tensor_reduce_sum_s));

    vxnneLayer_Initialize(&reduceNode->base,
        "TensorReduceSum",
        node,
        vxmOPERATION_COUNT(reduceNode),
        reduceNode->operations,
        VX_NULL);

    if (reduceDim)
    {
        axis = reduceDim->value->u32;
        if (axis >= (vx_uint32)TENSOR_DIM_NUM(src))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            vxError("Invalid input dimention %d the axis value must be in the range [0, %d) function %s line %d", axis, TENSOR_DIM_NUM(src), __FUNCTION__, __LINE__);
            goto exit;
        }
    }
    else
    {
        vxError("input params reduceDim is NULL function %s line %d", __FUNCTION__, __LINE__);
        status = VX_ERROR_INVALID_PARAMETERS;
        goto exit;
    }

    if(context->evisNoInst.supportEVIS)
    {
    shExe_flag = (vx_bool)(((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
        || (inputFormat == VX_TYPE_BFLOAT16 && outputFormat == VX_TYPE_BFLOAT16)
        || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT16)
        || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT8)
        || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_UINT8)
        || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16)
        || (inputFormat == VX_TYPE_INT8  && outputFormat == VX_TYPE_INT8)
        || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_FLOAT16)
        || (inputFormat == VX_TYPE_INT8  && outputFormat == VX_TYPE_FLOAT16)
        || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
        || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT16))
        && (reduceDim != NULL)
        && (axis <= 3));
    }
    else
    {
        shExe_flag = (vx_bool)(((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
            || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32)
            || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8))
            && (reduceDim != NULL)
            && (axis <= 3));
    }

    if(shExe_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
    {
        vx_tensor_create_params_t tensor_create_params;
        vx_uint32 dims          = TENSOR_DIM_NUM(src);
        vx_uint32 width         = TENSOR_VIEW_SIZE_INDEX(src, 0);
        vx_uint32 height        = dims > 1 ? TENSOR_VIEW_SIZE_INDEX(src, 1) : 1;
        vx_uint32 depth         = dims > 2 ? TENSOR_VIEW_SIZE_INDEX(src, 2) : 1;
        vx_uint32 batch         = dims > 3 ? TENSOR_VIEW_SIZE_INDEX(src, 3) : 1;
        vx_uint32 sizes[4]      = {width, height, depth, batch};
        vx_uint32 new_sizes[4]  = {width, height, depth, batch};
        vx_uint32 perm_array[4] = {0, 1, 2, 3};
        vx_tensor transTensor   = NULL;
        vx_uint32 i             = 0;
        vx_bool   enable_trans  = vx_false_e;
        vx_bool   enable_axis   = vx_true_e;
        vx_bool   is_trans_sw   = vx_false_e;
        if (axis == 3)
        {
            enable_trans = vx_true_e;

            if (axis == 3)
            {
                axis = 0;
                perm_array[0] = 3;
                perm_array[1] = 0;
                perm_array[2] = 1;
                perm_array[3] = 2;
                is_trans_sw   = vx_true_e;
            }
        }
        else
        {
            transTensor = src;
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
            tensor_create_params.data_format = TENSOR_DATA_TYPE(src);
            tensor_create_params.quant_format = TENSOR_QUANT_TYPE(src);
            if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
            {
                tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(src);
            }
            else
            {
                tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(src);
                tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(src);
            }

            transTensor = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);

            reduceNode->base.temp_tensors[tmpTensorIndex++] = transTensor;

            if (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_TP_TRANSPOSE) &&
                vxnneIsTPSupportFormat(context, src, VX_NULL, transTensor))
            {
                vx_op_param_s conv = {0};
                vx_uint32 dnum = 4;

                status = vxnneOperation_Initialize(&reduceNode->tensor_reduce_sum_trans_tp_operation.base,
                    &reduceNode->base,
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
                conv.other_ref = (vx_reference)src;
                conv.data_buff = gcvNULL;
                conv.tp_value = (vx_tp_value_cmd)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
                conv.tp_value->u32[0] = dnum;
                conv.tp_value->p8[0] = (vx_uint8_ptr)vxAllocateAndZeroMemory(sizeof(vx_uint32) * dnum);
                vxMemCopy(conv.tp_value->p8[0], perm_array, sizeof(vx_uint32) * dnum);

                vxMemCopy(&reduceNode->tensor_reduce_sum_trans_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));

                vxnneLayer_SetOperation(
                    &reduceNode->base,
                    &reduceNode->tensor_reduce_sum_trans_tp_operation.base,
                    operationIdx++);

                reduceNode->tensor_reduce_sum_trans_tp_operation.input  = src;
                reduceNode->tensor_reduce_sum_trans_tp_operation.output = transTensor;

                vxnneOperation_AddReference(&reduceNode->tensor_reduce_sum_trans_tp_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&reduceNode->tensor_reduce_sum_trans_tp_operation.base, (vx_reference)transTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);
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
                        elementCnt *= TENSOR_VIEW_SIZE_INDEX(src, i);
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
                        shaderExecutable = vxnneTensorTransposeShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_TRANSPOSE, &node->kernelAttributes.borderMode, src, perm_array, dims, transTensor);
                    }
                    else
                    {
                        shaderExecutable = vxnneGPUTensorTransposeShaderExecutable(node->base.context, VXNNE_KERNEL_GPU_TENSOR_TRANSPOSE, &node->kernelAttributes.borderMode, src, perm_array, dims, transTensor);
                    }

                    if (!shaderExecutable)
                    {
                        status = VX_FAILURE;
                        goto exit;
                    }
                    status = vxnneShaderOperation_Initialize(&reduceNode->tensor_reduce_sum_trans_sh_operation,
                        &reduceNode->base,
                        VXNNE_OPERATOR_TENSOR_TRANS,
                        batchCount,
                        shaderExecutable);

                    if (status != VX_SUCCESS)
                        goto exit;

                    vxnneOperation_AddReference(&reduceNode->tensor_reduce_sum_trans_sh_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT);
                    vxnneOperation_AddReference(&reduceNode->tensor_reduce_sum_trans_sh_operation.base, (vx_reference)transTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                    vxnneLayer_SetOperation(
                        &reduceNode->base,
                        &reduceNode->tensor_reduce_sum_trans_sh_operation.base,
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

                    vxnneOperation_Initialize(&reduceNode->tensor_trans_sw_operation.base,
                                              &reduceNode->base,
                                              VXNNE_OPERATION_TARGET_SW,
                                              VXNNE_OPERATOR_TENSOR_TRANS,
                                              vxnneExecuteSWTensorTranspose,
                                              VX_NULL,
                                              batchCount,
                                              0);

                    vxnneLayer_SetOperation(
                        &reduceNode->base,
                        &reduceNode->tensor_trans_sw_operation.base,
                        operationIdx++);

                    reduceNode->tensor_trans_sw_operation.input   = src;
                    reduceNode->tensor_trans_sw_operation.perm    = perm;
                    reduceNode->tensor_trans_sw_operation.pnum    = pnum_s;
                    reduceNode->tensor_trans_sw_operation.output  = transTensor;

                    vxnneOperation_AddReference(&reduceNode->tensor_trans_sw_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT);
                    vxnneOperation_AddReference(&reduceNode->tensor_trans_sw_operation.base, (vx_reference)transTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);
                }
            }
        }

        if (enable_axis)
        {
            vxnne_shader_executable shaderExecutable = NULL;
            vx_float32              axis_coef        = 1.0f;
            vx_uint32               batch            = new_sizes[3];

            if(node->base.context->evisNoInst.supportEVIS)
            {
                shaderExecutable = vxnneGetTensorMeanAxisShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_MEAN_AXIS, &node->kernelAttributes.borderMode, axis_coef, transTensor, dst, axis);
            }
            else
            {
                shaderExecutable = vxnneGetGPUTensorMeanAxisShaderExecutable(node->base.context, VXNNE_KERNEL_GPU_TENSOR_MEAN_AXIS, &node->kernelAttributes.borderMode, axis_coef, transTensor, dst, axis);
            }

            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto exit;
            }

            status = vxnneShaderOperation_Initialize(&reduceNode->tensor_reduce_sum_sh_operation,
                &reduceNode->base,
                VXNNE_OPERATOR_TENSOR_MEAN,
                batch,
                shaderExecutable);

            if (status != VX_SUCCESS)
            {
                goto exit;
            }

            vxnneOperation_AddReference(&reduceNode->tensor_reduce_sum_sh_operation.base, (vx_reference)transTensor, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&reduceNode->tensor_reduce_sum_sh_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            vxnneLayer_SetOperation(
                &reduceNode->base,
                &reduceNode->tensor_reduce_sum_sh_operation.base,
                operationIdx++);
        }

        reduceNode->base.num_temp_tensors = tmpTensorIndex;
    }
    else
    {
        vxnneOperation_Initialize(&reduceNode->tensor_reduce_sum_operation.base,
            &reduceNode->base,
            VXNNE_OPERATION_TARGET_SW,
            VXNNE_OPERATOR_TENSOR_REDUCE_SUM,
            vxnneExecuteSWTensorReduceSum,
            VX_NULL,
            batchCount,
            0);

        vxnneLayer_SetOperation(
            &reduceNode->base,
            &reduceNode->tensor_reduce_sum_operation.base,
            0);

        reduceNode->tensor_reduce_sum_operation.src           = src;
        reduceNode->tensor_reduce_sum_operation.dst           = dst;
        reduceNode->tensor_reduce_sum_operation.reduceDim     = reduceDim;
        reduceNode->tensor_reduce_sum_operation.keepDim       = keepDim;

        vxnneOperation_AddReference(&reduceNode->tensor_reduce_sum_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&reduceNode->tensor_reduce_sum_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        if (reduceDim)
            vxnneOperation_AddReference(&reduceNode->tensor_reduce_sum_operation.base, (vx_reference)reduceDim, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&reduceNode->tensor_reduce_sum_operation.base, (vx_reference)keepDim, VXNNE_OPERATION_REFENRENCE_INPUT);
    }

    node->layer = &reduceNode->base;

    return status;

exit:
    if (reduceNode)
    {
        if (reduceNode->tensor_reduce_sum_trans_tp_operation.base.parameter.tp_value)
        {
            if (reduceNode->tensor_reduce_sum_trans_tp_operation.base.parameter.tp_value->p8[0])
            {
                gcoOS_Free(NULL, (gctPOINTER)reduceNode->tensor_reduce_sum_trans_tp_operation.base.parameter.tp_value->p8[0]);
            }
            gcoOS_Free(NULL, (gctPOINTER)reduceNode->tensor_reduce_sum_trans_tp_operation.base.parameter.tp_value);
        }
        gcoOS_Free(NULL, (gctPOINTER)reduceNode);
    }
#endif
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorReduceSum_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

