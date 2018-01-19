/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <VX/vx.h>
#ifdef WIN32
#include <dirent_win.h>
#else
#include <dirent.h>
#endif

#include <gc_vx_common.h>
#include <gc_vx_internal_node_api.h>
#include <gc_vx_nn_util.h>
#include <ops/gc_vx_nn_extension_concat.h>

vx_status vxnneExecuteSWConcatIndefinite(struct _vxnne_operation_s *operation)
{
    vx_tensor *input,output;
    vx_uint32 num,axis;

    vx_uint32 i,j;
    vx_uint32 inputSize,outputSize,totalSize,outElementSize;
    vx_uint32 inputCount,outputCount,totalCount;
    vx_enum inType,outType;
    vx_uint8 inFpPos,outFpPos;
    gctPOINTER inLogical = VX_NULL;
    gctPOINTER outLogical = VX_NULL;

    vx_float32 inputValue;
    vx_uint32 index;

    vxnne_concatIndefinite_sw_operation concatOperation = (vxnne_concatIndefinite_sw_operation)operation;

    input    = (vx_tensor *)concatOperation->inputs;
    output   = (vx_tensor)concatOperation->outputs;
    num      = (vx_uint32)concatOperation->num;
    axis     = (vx_uint32)concatOperation->axis;

    vxoTensor_GetTensorViewMemory(output, &outLogical, VX_NULL);
    vxoTensor_GetTensorElementCount(output, &outputCount);
    vxoTensor_GetTensorSize(output, &outputSize);
    outType         = output->tensorBuffer->dataFormat;
    outFpPos        = output->tensorBuffer->fixedPointPos;
    outElementSize  = output->tensorBuffer->elementSize;

    /* check output element count and size */
    totalCount = 0;
    totalSize = 0;
    for(i=0; i<num; i++)
    {
        vxoTensor_GetTensorElementCount(input[i], &inputCount);
        totalCount += inputCount;
    }

    if(totalCount != outputCount)
    {
        printf("count error, total input count %u != output count %u\n", totalCount, outputCount);
        return VX_FAILURE;
    }

    totalSize = totalCount * outElementSize;
    if(totalSize != outputSize)
    {
        printf("size error, total input size %u != output size %u\n", totalSize, outputSize);
        return VX_FAILURE;
    }

    /* concat input tensor to output tensor */
    index = 0;
    for(i=0; i<num; i++)
    {
        vxoTensor_GetTensorViewMemory(input[i], &inLogical, VX_NULL);
        vxoTensor_GetTensorElementCount(input[i], &inputCount);
        vxoTensor_GetTensorSize(input[i], &inputSize);
        inType      = input[i]->tensorBuffer->dataFormat;
        inFpPos     = input[i]->tensorBuffer->fixedPointPos;

        if(inType == outType)
        {
            if(inType == VX_TYPE_INT8 && outType == VX_TYPE_INT8)
            {
                if(outFpPos == inFpPos)
                {
                    vxMemCopy((vx_uint8_ptr)outLogical+(index*outElementSize), (vx_uint8_ptr)inLogical, inputSize);
                    index += inputCount;
                }
                else
                {
                    for(j=0; j<inputCount; j++,index++)
                    {
                        /* convert input data to fp32 */
                        inputValue = vxnneGetData((vx_type_e)inType, j, (vx_uint8_ptr)inLogical, inFpPos);

                        /* convert fp32 to output data by output format */
                        vxnneSaveData((vx_type_e)outType, index, inputValue, outLogical, outFpPos, output->tensorBuffer->roundingMode);
                    }
                }
            }
            else
            {
                vxMemCopy((vx_uint8_ptr)outLogical+(index*outElementSize), (vx_uint8_ptr)inLogical, inputSize);
                index += inputCount;
            }
        }
        else
        {
            for(j=0; j<inputCount; j++,index++)
            {
                /* convert input data to fp32 */
                inputValue = vxnneGetData((vx_type_e)inType, j, (vx_uint8_ptr)inLogical, inFpPos);

                /* convert fp32 to output data by output format */
                vxnneSaveData((vx_type_e)outType, index, inputValue, outLogical, outFpPos, output->tensorBuffer->roundingMode);
            }
        }
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNConcatIndefiniteLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcatIndefiniteLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcatIndefiniteLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcatIndefiniteLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    vx_scalar  input_s  = (vx_scalar)parameters[0];
    vx_scalar  num_s    = (vx_scalar)parameters[1];
    vx_scalar  axis_s   = (vx_scalar)parameters[2];
    vx_tensor  outputs  = (vx_tensor)parameters[3];

    vx_uint32 input_n   = input_s->value->u32;
    vx_uint32 number    = num_s->value->u32;
    vx_uint32 axis      = axis_s->value->u32;

    vx_tensor *inputs   = (vx_tensor *)gcmINT2PTR(input_n);

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    {
        vxnne_concatIndefinite_layer  concatNLayer;

        gcoOS_Allocate(gcvNULL, sizeof(vxnne_concatIndefinite_layer_s), (gctPOINTER*)&concatNLayer);
        if (!concatNLayer)
        {
            status = VX_ERROR_NO_MEMORY;
            gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            goto exit;
        }

        gcoOS_ZeroMemory(concatNLayer, sizeof(vxnne_concatIndefinite_layer_s));

        vxnneLayer_Initialize(&concatNLayer->base,
                              "ConcatLayer",
                              node,
                              concatNLayer->operations,
                              VX_NULL);

        vxnneOperation_Initialize(&concatNLayer->concatIndefinite_operation.base,
                                  &concatNLayer->base,
                                  VXNNE_OPERATION_TARGET_SW,
                                  VXNNE_OPERATOR_CONCATINDEFINITE,
                                  vxnneExecuteSWConcatIndefinite,
                                  VX_NULL);

        concatNLayer->base.num_operations    = 1;
        concatNLayer->operations[0] = (vxnne_operation)&concatNLayer->concatIndefinite_operation;

        concatNLayer->concatIndefinite_operation.inputs          = inputs;
        concatNLayer->concatIndefinite_operation.num             = number;
        concatNLayer->concatIndefinite_operation.axis            = axis;
        concatNLayer->concatIndefinite_operation.outputs         = outputs;

        node->layer = &concatNLayer->base;
    }


exit:
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcatIndefiniteLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

