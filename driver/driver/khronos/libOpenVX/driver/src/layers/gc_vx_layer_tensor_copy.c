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
#include <layers/gc_vx_layer_tensor_copy.h>

/* Greatest Common Divisor*/
extern vx_bool vxoGetDataDivisors(vx_uint32 input_value, vx_uint32 *divisors, vx_uint32 gcd);

vx_status vxnneExecuteSWTensorCopy(struct _vxnne_operation_s *operation)
{
    vxnne_tensor_copy_sw_operation           copyOperation   = (vxnne_tensor_copy_sw_operation)operation;

    vx_tensor src  = (vx_tensor)copyOperation->src;
    vx_tensor dst = (vx_tensor)copyOperation->dst;
    gctPOINTER srcLogical = VX_NULL;
    gctPOINTER dstLogical = VX_NULL;

    vx_uint32 dstSize, srcSize, copySize;
    vx_uint32 dstCount, srcCount, copyCount;
    vx_int8 srcFp = TENSOR_POS(src);
    vx_int8 dstFp = TENSOR_POS(dst);

    vxoTensor_GetTensorViewMemory(src, &srcLogical, VX_NULL);
    vxoTensor_GetTensorViewMemory(dst, &dstLogical, VX_NULL);

    if (TENSOR_DATA_TYPE(src) == TENSOR_DATA_TYPE(dst) && srcFp == dstFp)
    {
        vxoTensor_GetTensorSize(src,&srcSize);
        vxoTensor_GetTensorSize(dst,&dstSize);

        copySize = gcmMIN(srcSize, dstSize);
        memcpy(dstLogical, srcLogical, copySize);
    }
    else
    {
        vx_uint32 i;
        vx_float32 src0 = 0;

        if (src->isViewed)
        {
            srcCount = 1;

            for (i = 0; i < TENSOR_VIEW_DIM_NUM(src); i++)
            {
                srcCount *= TENSOR_VIEW_SIZE_INDEX(src, i);
            }
        }
        else
            vxoTensor_GetTensorElementCount(src, &srcCount);

        if (dst->isViewed)
        {
            dstCount = 1;

            for (i = 0; i < TENSOR_VIEW_DIM_NUM(dst); i++)
            {
                dstCount *= TENSOR_VIEW_SIZE_INDEX(dst, i);
            }
        }
        else
            vxoTensor_GetTensorElementCount(dst, &dstCount);

        copyCount = gcmMIN(srcCount, dstCount);

        for (i = 0; i < copyCount; i++)
        {
            src0 = VX_GET_DATA_FROM_TENSOR(src, i);

            VX_SAVE_DATA_TO_TENSOR(dst, src0, i);

        }
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorCopy(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorCopy_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorCopy_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

#define INPUT_SIZE_ALIGN_4  (4)
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorCopy_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status  status                     = VX_SUCCESS;

    vx_tensor  src                     = (vx_tensor)parameters[0];
    vx_tensor  dst                     = (vx_tensor)parameters[1];
    vx_uint32  batchCount              = TENSOR_VIEW_DIM_NUM(src) > 3 ? TENSOR_SIZE_INDEX(src, TENSOR_VIEW_DIM_NUM(src) - 1) : 1;
    vx_enum    inputFormat             = TENSOR_DATA_TYPE(src);
    vx_enum    outputFormat            = TENSOR_DATA_TYPE(dst);
    vx_bool    shExe_flag              = vx_false_e;
    vx_bool    enable_dataConv2F32     = vx_false_e;
    vx_uint32  reshpTensor_Sizes[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {1};
    vx_uint32  reshpTensor_Dims           = 2;
    vxnne_tensor_copy  copyNode        = VX_NULL;
    vx_context context                 = vxGetContext((vx_reference)node);

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    {
        vx_uint32 src_elementCount = 0;
        vx_uint32 dst_elementCount = 0;

        status = vxoTensor_GetTensorElementCount(src, &src_elementCount);
        status = vxoTensor_GetTensorElementCount(dst, &dst_elementCount);

        if(context->evisNoInst.supportEVIS)
        {
            if (src_elementCount < IMG_MAX_WIDTH)
            {
                reshpTensor_Sizes[0]   = src_elementCount;
                reshpTensor_Sizes[1]   = 1;
                reshpTensor_Dims = 2;

                if (inputFormat != VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32)
                    enable_dataConv2F32 = vx_true_e;
            }
            else
            {
                vx_uint32 gcd = outputFormat == VX_TYPE_FLOAT32 ? 4 : 1;
                vx_uint32 divisors = 1;
                vx_uint32 element_count = src_elementCount;

                vxoGetDataDivisors(element_count, &divisors, gcd);
                reshpTensor_Sizes[0] = divisors;
                element_count = element_count / divisors;
                vxoGetDataDivisors(element_count, &divisors, 1);
                reshpTensor_Sizes[1] = divisors;
                element_count = element_count / divisors;
                reshpTensor_Sizes[2] = element_count;
                reshpTensor_Dims = 3;

                if (inputFormat != VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32
                    && (reshpTensor_Sizes[0] % INPUT_SIZE_ALIGN_4 == 0))
                    enable_dataConv2F32 = vx_true_e;
            }

            shExe_flag = (vx_bool)(((inputFormat == VX_TYPE_FLOAT16 && outputFormat != VX_TYPE_FLOAT32)
                                 || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16)
                                 || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8)
                                 || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_FLOAT16)
                                 || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT16)
                                 || (inputFormat == VX_TYPE_INT32 && outputFormat == VX_TYPE_INT16)
                                 || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT32)
                                 || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
                                 || (inputFormat == VX_TYPE_FLOAT32 && outputFormat != VX_TYPE_FLOAT32)
                                 || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT16)
                                 || (inputFormat == VX_TYPE_BFLOAT16 && outputFormat == VX_TYPE_BFLOAT16)
                                 || enable_dataConv2F32)
                                 && src_elementCount == dst_elementCount);
        }
        else
        {
            vxoElementOptimization_GetTensorShape(src, reshpTensor_Sizes, &reshpTensor_Dims);

            shExe_flag = (vx_bool)(((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
                                 || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32)
                                 || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT16)
                                 || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT32)
                                 || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_UINT8)
                                 || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_UINT8)
                                 || (inputFormat == VX_TYPE_INT32 && outputFormat == VX_TYPE_INT32)
                                 || (inputFormat == VX_TYPE_INT32 && outputFormat == VX_TYPE_INT16)
                                 || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT32)
                                 || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
                                 || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT32)
                                 || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT16))
                                 && src_elementCount == dst_elementCount);
        }

        gcoOS_Allocate(gcvNULL, sizeof(vxnne_tensor_copy_s), (gctPOINTER*)&copyNode);
        if (!copyNode)
        {
            status = VX_ERROR_NO_MEMORY;
            vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            goto exit;
        }

        gcoOS_ZeroMemory(copyNode, sizeof(vxnne_tensor_copy_s));

        vxnneLayer_Initialize(&copyNode->base,
                              "TensorCopy",
                              node,
                              vxmOPERATION_COUNT(copyNode),
                              copyNode->operations,
                              VX_NULL);

        if (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_TP) &&
            (TENSOR_VIEW_SIZE_INDEX(dst, 0) * TENSOR_VIEW_SIZE_INDEX(dst, 1) * TENSOR_VIEW_SIZE_INDEX(dst, 2) > 1) &&
            vxnneIsTPSupportFormat(context, src, VX_NULL, dst))
        {
            vx_op_param_s conv = {0};

            status = vxnneOperation_Initialize(&copyNode->tensor_copy_tp_operation.base,
                &copyNode->base,
                VXNNE_OPERATION_TARGET_TP,
                VXNNE_OPERATOR_TENSOR_COPY,
                VX_NULL,
                 vxnneOperation_TP_Deinitialize,
                batchCount,
                0);

            if (status != VX_SUCCESS) goto exit;

            memset(&conv, 0, sizeof(vx_op_param_s));

            conv.enable_relu = vx_false_e;
            conv.pool_stride = 1;
            conv.tpType = TP_TENSOR_COPY;

            memcpy(&copyNode->tensor_copy_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));

            vxnneOperation_AddReference(&copyNode->tensor_copy_tp_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&copyNode->tensor_copy_tp_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            copyNode->tensor_copy_tp_operation.input = src;
            copyNode->tensor_copy_tp_operation.output = dst;

            vxnneLayer_SetOperation(
                &copyNode->base,
                &copyNode->tensor_copy_tp_operation.base,
                0);
        }
        else
        {
            if (shExe_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)) )
            {
                vxnne_shader_executable shaderExecutable = VX_NULL;
                vx_tensor input      = NULL;
                vx_tensor output     = NULL;

                input     = vxoTensor_ReshapeTensor(src, (vx_int32*)reshpTensor_Sizes, reshpTensor_Dims);
                output     = vxoTensor_ReshapeTensor(dst, (vx_int32*)reshpTensor_Sizes, reshpTensor_Dims);

                copyNode->base.temp_tensors[0] = input;
                copyNode->base.temp_tensors[1] = output;
                copyNode->base.num_temp_tensors = 2;

                if(node->base.context->evisNoInst.supportEVIS)
                {
                    if (input && output)
                        shaderExecutable = vxnneTensorCopyShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_COPY, &node->kernelAttributes.borderMode, input, output);
                }
                else
                {
                    if (input && output)
                        shaderExecutable = vxnneGPUTensorCopyShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_COPY, &node->kernelAttributes.borderMode, input, output);
                }

                if (!shaderExecutable)
                {
                    status = VX_FAILURE;
                    goto exit;
                }
                status = vxnneShaderOperation_Initialize(&copyNode->tensor_copy_sh_operation,
                    &copyNode->base,
                    VXNNE_OPERATOR_CONVERT_FORMAT,
                    1, /*batchCount is 1 after reshape tensor object*/
                    shaderExecutable);

                if (status != VX_SUCCESS)
                    goto exit;

                vxnneOperation_AddReference(&copyNode->tensor_copy_sh_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&copyNode->tensor_copy_sh_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);
                vxnneLayer_SetOperation(&copyNode->base, &copyNode->tensor_copy_sh_operation.base, 0);
            }
            else
            {
                vxnneOperation_Initialize(&copyNode->tensor_copy_operation.base,
                    &copyNode->base,
                    VXNNE_OPERATION_TARGET_SW,
                    VXNNE_OPERATOR_TENSOR_COPY,
                    vxnneExecuteSWTensorCopy,
                    VX_NULL,
                    batchCount,
                    0);
                vxnneLayer_SetOperation(&copyNode->base, &copyNode->tensor_copy_operation.base, 0);

                copyNode->tensor_copy_operation.src           = src;
                copyNode->tensor_copy_operation.dst           = dst;

                vxnneOperation_AddReference(&copyNode->tensor_copy_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&copyNode->tensor_copy_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            }
        }

        node->layer = &copyNode->base;
    }

    return status;

exit:
    if(copyNode) gcoOS_Free(NULL, (gctPOINTER)copyNode);
    return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorCopy_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

