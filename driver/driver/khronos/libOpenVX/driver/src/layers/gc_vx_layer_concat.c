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
#include <layers/gc_vx_layer_concat.h>

vx_status vxnneExecuteSWConcat2(struct _vxnne_operation_s *operation)
{
    vxnne_concat2_sw_operation           concatOperation   = (vxnne_concat2_sw_operation)operation;

    vx_tensor input0  = (vx_tensor)concatOperation->inputs0;
    vx_tensor input1  = (vx_tensor)concatOperation->inputs1;
    vx_tensor output = (vx_tensor)concatOperation->outputs;
    vx_type_e   input0_format = (vx_type_e)TENSOR_DATA_TYPE(input0);
    vx_type_e   input1_format = (vx_type_e)TENSOR_DATA_TYPE(input1);
    vx_type_e   output_format = (vx_type_e)TENSOR_DATA_TYPE(output);
    vx_int8   src0FixPointPos  = TENSOR_POS(input0);
    vx_int8   src1FixPointPos  = TENSOR_POS(input1);
    vx_int8   dstFixPointPos  = TENSOR_POS(output);
    vx_enum   dstRoundingMode = TENSOR_ROUNDING_MODE(output);
    vx_uint32  m = 0;
    vx_uint32  index = 0;

    vx_uint8_ptr pInput0Buf = NULL;
    vx_uint8_ptr pInput1Buf = NULL;
    vx_uint8_ptr pOutputBuf = NULL;

    vx_uint32 input0Size = 0, input1Size = 0;

    vxoTensor_GetTensorViewMemory(input0, (gctPOINTER *)&pInput0Buf, VX_NULL);
    vxoTensor_GetTensorViewMemory(input1, (gctPOINTER *)&pInput1Buf, VX_NULL);
    vxoTensor_GetTensorViewMemory(output, (gctPOINTER *)&pOutputBuf, VX_NULL);

    if (input0->isViewed)
    {
        vxmASSERT(vxoTensor_GetTensorSize(input0, &input0Size) == VX_SUCCESS);
    }
    else
    {
        input0Size = (vx_uint32)vxoMemory_ComputeSize(&input0->tensorBuffer->memory, 0);
    }

    if (input1->isViewed)
    {
        vxmASSERT(vxoTensor_GetTensorSize(input1, &input1Size) == VX_SUCCESS);
    }
    else
    {
        input1Size = (vx_uint32)vxoMemory_ComputeSize(&input1->tensorBuffer->memory, 0);
    }

    if((input0_format == VX_TYPE_FLOAT16 && input1_format == VX_TYPE_FLOAT16 && output_format == VX_TYPE_FLOAT16)
    ||(input0_format == VX_TYPE_INT8 && input1_format == VX_TYPE_INT8 && output_format == VX_TYPE_INT8 && src0FixPointPos == src1FixPointPos && src0FixPointPos == dstFixPointPos))
    {
        vxMemCopy(pOutputBuf, pInput0Buf, input0Size);
        vxMemCopy(&pOutputBuf[input0Size], pInput1Buf, input1Size);
    }
    else
    {
        input0Size = input0Size / TENSOR_DATA_SIZE(input0);
        input1Size = input1Size / TENSOR_DATA_SIZE(input1);
        for (m = 0; m < input0Size; m ++, index ++)
        {
            vx_float32 src0 = vxnneGetDataExt(input0_format, TENSOR_QUANT_TYPE(input0), m, (vx_uint8_ptr)pInput0Buf, src0FixPointPos, TENSOR_TF_ZEROPOINT(input0), TENSOR_TF_SCALE(input0));
            vxnneSaveDataExt(output_format, TENSOR_QUANT_TYPE(output), index, src0, (vx_uint8_ptr)pOutputBuf, dstFixPointPos, TENSOR_TF_ZEROPOINT(output), TENSOR_TF_SCALE(output), dstRoundingMode);
        }

        for (m = 0; m < input1Size; m ++, index ++)
        {
            vx_float32 src1 = vxnneGetDataExt(input1_format, TENSOR_QUANT_TYPE(input1), m, (vx_uint8_ptr)pInput1Buf, src1FixPointPos, TENSOR_TF_ZEROPOINT(input1), TENSOR_TF_SCALE(input1));
            vxnneSaveDataExt(output_format, TENSOR_QUANT_TYPE(output), index, src1, (vx_uint8_ptr)pOutputBuf, dstFixPointPos, TENSOR_TF_ZEROPOINT(output), TENSOR_TF_SCALE(output), dstRoundingMode);
        }
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNConcat2Layer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcat2Layer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcat2Layer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}
#if REGISTER_FRAME

VX_PRIVATE_API vx_status vxoNNConcat2Layer_SW_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vxnne_concat2_layer  concatLayer = (vxnne_concat2_layer)ops_layer;

    vx_tensor  inputs[2]               = { (vx_tensor)parameters[0], (vx_tensor)parameters[1]};
    vx_tensor  output_s                = (vx_tensor)parameters[2];
    vx_uint32  batchCount              = TENSOR_SIZE_INDEX(inputs[0], 3);

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxnneOperation_Initialize(&concatLayer->concat2_operation.base,
        &concatLayer->base,
        VXNNE_OPERATION_TARGET_SW,
        VXNNE_OPERATOR_CONCAT2,
        vxnneExecuteSWConcat2,
        VX_NULL,
        batchCount,
        0));

    vxmONERROR(vxnneLayer_SetOperation(&concatLayer->base, &concatLayer->concat2_operation.base, 0));
    concatLayer->concat2_operation.inputs0           = inputs[0];
    concatLayer->concat2_operation.inputs1           = inputs[1];
    concatLayer->concat2_operation.outputs           = output_s;

    vxmONERROR(vxnneOperation_AddReference(&concatLayer->concat2_operation.base, (vx_reference)inputs[0], VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&concatLayer->concat2_operation.base, (vx_reference)inputs[1], VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&concatLayer->concat2_operation.base, (vx_reference)output_s, VXNNE_OPERATION_REFENRENCE_OUTPUT));
OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNConcat2Layer_SH_EVIS_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_tensor  inputs[2]               = { (vx_tensor)parameters[0], (vx_tensor)parameters[1]};
    vx_tensor  output_s                = (vx_tensor)parameters[2];

    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && node->base.context->evisNoInst.supportEVIS;

    if (!support)return support;

    support = support && (vx_bool)(TENSOR_DATA_TYPE(output_s) != VX_TYPE_FLOAT32);
    support = support && (TENSOR_DATA_TYPE(inputs[0]) != VX_TYPE_FLOAT32);
    support = support && (TENSOR_DATA_TYPE(inputs[1]) != VX_TYPE_FLOAT32);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNConcat2Layer_SH_EVIS_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vxnne_concat2_layer  concatLayer = (vxnne_concat2_layer)ops_layer;

    vx_tensor  inputs[2]               = { (vx_tensor)parameters[0], (vx_tensor)parameters[1]};
    vx_tensor  output_s                = (vx_tensor)parameters[2];
    vx_uint32  batchCount              = TENSOR_SIZE_INDEX(inputs[0], 3);
    vx_uint32  opIdx                   = 0;
    vx_uint32  operationIndex          = 0;
    vx_uint32  tmpTensorIndex          = 0;

    vxnne_shader_executable shaderExecutable = VX_NULL;
    vx_uint32 i          = 0;
    vx_uint32 w         = TENSOR_VIEW_SIZE_INDEX(output_s, 0);
    vx_uint32 h         = TENSOR_VIEW_SIZE_INDEX(output_s, 1);
    vx_uint32 c         = TENSOR_VIEW_SIZE_INDEX(output_s, 2);
    vx_uint32 n         = TENSOR_VIEW_SIZE_INDEX(output_s, 3);
    vx_uint32 sizes[4]  = {w, h, c, n};
    vx_uint32 start[4]  = {0, 0, 0, 0};
    vx_uint32 end[4]    = {0, 0, 0, 0};

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    if (1)
    {
        vx_uint32 dimCount = TENSOR_DIM_NUM(inputs[0]);
        vx_uint32 axis = dimCount - 1;

        for (i = 0; i < axis; i ++)
        {
            end[i] = sizes[i];
        }
        for (i = 0; i < 2; i ++)
        {
            vx_tensor input = (vx_tensor)inputs[i];
            vx_tensor_view  tensor_view  = NULL;
            vx_tensor       subtensor    = NULL;
            vx_uint32  reshpTensor_Sizes[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {1};
            vx_uint32  reshpTensor_Dims           = 2;
            vx_tensor input_rs      = NULL;
            vx_tensor output_rs     = NULL;

            vxoElementOptimization_GetTensorShape(input, reshpTensor_Sizes, &reshpTensor_Dims);

            batchCount     = TENSOR_VIEW_DIM_NUM(input) > 3 ? TENSOR_VIEW_SIZE_INDEX(input, 3) : 1;

            start[axis]    = end[axis];
            end[axis]      += TENSOR_VIEW_SIZE_INDEX(input, axis);

            tensor_view = vxCreateTensorView(ops_layer->node->base.context, start, end, (vx_uint8)dimCount);
            if (tensor_view == NULL)
            {
                vxError("vxCreateTensorView failure! at line %d\n", __LINE__);
                vxmONERROR(VX_ERROR_NO_MEMORY);
            }

            subtensor           = vxoTensor_CreateTensorFromView(output_s, tensor_view);

            input_rs  = vxoTensor_ReshapeTensor(input, (vx_int32*)reshpTensor_Sizes, reshpTensor_Dims);
            output_rs = vxoTensor_ReshapeTensor(subtensor, (vx_int32*)reshpTensor_Sizes, reshpTensor_Dims);
            concatLayer->base.temp_tensors[tmpTensorIndex++] = input_rs;
            concatLayer->base.temp_tensors[tmpTensorIndex++] = output_rs;

            if(ops_layer->node->base.context->evisNoInst.supportEVIS)/*Need confirm?*/
            {
                shaderExecutable    = vxnneTensorCopyShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_COPY, &ops_layer->node->kernelAttributes.borderMode, input_rs, output_rs);
            }
            else
            {
                shaderExecutable    = vxnneGPUTensorCopyShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_COPY, &ops_layer->node->kernelAttributes.borderMode, input_rs, output_rs);
            }

            if (!shaderExecutable)
            {
                vxmONERROR(VX_FAILURE);
            }

            vxmONERROR(vxnneShaderOperation_Initialize(&concatLayer->concat_sh_unit_operation[operationIndex],
                &concatLayer->base,
                VXNNE_OPERATOR_TENSOR_COPY,
                batchCount,
                shaderExecutable));

            vxmONERROR(vxnneOperation_AddReference(&concatLayer->concat_sh_unit_operation[operationIndex].base, (vx_reference)input_rs, VXNNE_OPERATION_REFENRENCE_INPUT));
            vxmONERROR(vxnneOperation_AddReference(&concatLayer->concat_sh_unit_operation[operationIndex].base, (vx_reference)output_rs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

            vxmONERROR(vxnneLayer_SetOperation(
                &concatLayer->base,
                &concatLayer->concat_sh_unit_operation[operationIndex].base,
                opIdx++));
            operationIndex ++;

            concatLayer->base.temp_tensors[tmpTensorIndex++] = subtensor;
            if (tensor_view) vxmONERROR(vxReleaseTensorView(&tensor_view));
        }

    }

    concatLayer->base.num_temp_tensors = tmpTensorIndex;
OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoNNLayer_GetOperations2(vxnne_layer ops_layer, vx_uint32_ptr max_num_operations, vxnne_operation **operations)
{
    vx_status  status = VX_SUCCESS;
    vxnne_concat2_layer  concatLayer = (vxnne_concat2_layer)ops_layer;

    *max_num_operations = gcmCOUNTOF(concatLayer->operations);

    *operations = concatLayer->operations;

    return status;
}
#endif


VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcat2Layer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status  status                  = VX_SUCCESS;
#if REGISTER_FRAME

    vxnne_layer_imp_s registerConcat2Layers[] = {/* Please DON'T adjust the order, it's importent */
        { "Concat2Layer NN", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "Concat2Layer TP", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "Concat2Layer SH EVIS", vxoNNConcat2Layer_SH_EVIS_Support, vxoNNConcat2Layer_SH_EVIS_Initialize, VX_NULL },
        { "Concat2Layer SH F32", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "Concat2Layer SW", vxoNNCommon_Support, vxoNNConcat2Layer_SW_Initialize, VX_NULL },
    };

    REGISTER_LAYERS(registerConcat2Layers, vxnne_concat2_layer_s, "ConcatLayer", vxoNNLayer_GetOperations2);

OnError:
#else
    vx_tensor  inputs[2]               = { (vx_tensor)parameters[0], (vx_tensor)parameters[1]};
    vx_tensor  output_s                = (vx_tensor)parameters[2];
    vx_uint32  batchCount              = TENSOR_SIZE_INDEX(inputs[0], 3);
    vx_bool    enable_SHExe            = vx_false_e;
    vx_uint32  opIdx                   = 0;
    vx_uint32  operationIndex          = 0;
    vx_uint32  tmpTensorIndex          = 0;
    vxnne_concat2_layer  concatLayer = VX_NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    if(node->base.context->evisNoInst.supportEVIS)
    {
        enable_SHExe = (vx_bool)(TENSOR_DATA_TYPE(output_s) != VX_TYPE_FLOAT32);

        enable_SHExe = (vx_bool)(TENSOR_DATA_TYPE(inputs[0]) != VX_TYPE_FLOAT32 && enable_SHExe);
        enable_SHExe = (vx_bool)(TENSOR_DATA_TYPE(inputs[1]) != VX_TYPE_FLOAT32 && enable_SHExe);
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_concat2_layer_s), (gctPOINTER*)&concatLayer);
    if (!concatLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto OnError;
    }

    gcoOS_ZeroMemory(concatLayer, sizeof(vxnne_concat2_layer_s));

    vxnneLayer_Initialize(&concatLayer->base,
        "ConcatLayer",
        node,
        vxmOPERATION_COUNT(concatLayer),
        concatLayer->operations,
        VX_NULL);

    if (enable_SHExe)
    {
        vxnne_shader_executable shaderExecutable = VX_NULL;
        vx_uint32 i          = 0;
        vx_uint32 w         = TENSOR_VIEW_SIZE_INDEX(output_s, 0);
        vx_uint32 h         = TENSOR_VIEW_SIZE_INDEX(output_s, 1);
        vx_uint32 c         = TENSOR_VIEW_SIZE_INDEX(output_s, 2);
        vx_uint32 n         = TENSOR_VIEW_SIZE_INDEX(output_s, 3);
        vx_uint32 sizes[4]  = {w, h, c, n};
        vx_uint32 start[4]  = {0, 0, 0, 0};
        vx_uint32 end[4]    = {0, 0, 0, 0};

        if (1)
        {
            vx_uint32 dimCount = TENSOR_DIM_NUM(inputs[0]);
            vx_uint32 axis = dimCount - 1;

            for (i = 0; i < axis; i ++)
            {
                end[i] = sizes[i];
            }
            for (i = 0; i < 2; i ++)
            {
                vx_tensor input = (vx_tensor)inputs[i];
                vx_tensor_view  tensor_view  = NULL;
                vx_tensor       subtensor    = NULL;
                vx_uint32  reshpTensor_Sizes[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {1};
                vx_uint32  reshpTensor_Dims           = 2;
                vx_tensor input_rs      = NULL;
                vx_tensor output_rs     = NULL;

                vxoElementOptimization_GetTensorShape(input, reshpTensor_Sizes, &reshpTensor_Dims);

                batchCount     = TENSOR_VIEW_DIM_NUM(input) > 3 ? TENSOR_VIEW_SIZE_INDEX(input, 3) : 1;

                start[axis]    = end[axis];
                end[axis]      += TENSOR_VIEW_SIZE_INDEX(input, axis);

                tensor_view = vxCreateTensorView(node->base.context, start, end, (vx_uint8)dimCount);
                if (tensor_view == NULL)
                {
                    vxError("vxCreateTensorView failure! at line %d\n", __LINE__);
                    vxmONERROR(VX_ERROR_NO_MEMORY);
                }

                subtensor           = vxoTensor_CreateTensorFromView(output_s, tensor_view);

                input_rs  = vxoTensor_ReshapeTensor(input, (vx_int32*)reshpTensor_Sizes, reshpTensor_Dims);
                output_rs = vxoTensor_ReshapeTensor(subtensor, (vx_int32*)reshpTensor_Sizes, reshpTensor_Dims);
                concatLayer->base.temp_tensors[tmpTensorIndex++] = input_rs;
                concatLayer->base.temp_tensors[tmpTensorIndex++] = output_rs;

                if(node->base.context->evisNoInst.supportEVIS)
                {
                    shaderExecutable    = vxnneTensorCopyShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_COPY, &node->kernelAttributes.borderMode, input_rs, output_rs);
                }
                else
                {
                    shaderExecutable    = vxnneGPUTensorCopyShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_COPY, &node->kernelAttributes.borderMode, input_rs, output_rs);
                }

                if (!shaderExecutable)
                {
                    vxmONERROR(VX_FAILURE);
                }

                vxmONERROR(vxnneShaderOperation_Initialize(&concatLayer->concat_sh_unit_operation[operationIndex],
                    &concatLayer->base,
                    VXNNE_OPERATOR_TENSOR_COPY,
                    batchCount,
                    shaderExecutable));

                vxnneOperation_AddReference(&concatLayer->concat_sh_unit_operation[operationIndex].base, (vx_reference)input_rs, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&concatLayer->concat_sh_unit_operation[operationIndex].base, (vx_reference)output_rs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                vxnneLayer_SetOperation(
                    &concatLayer->base,
                    &concatLayer->concat_sh_unit_operation[operationIndex].base,
                    opIdx++);
                operationIndex ++;

                concatLayer->base.temp_tensors[tmpTensorIndex++] = subtensor;
                if (tensor_view) vxReleaseTensorView(&tensor_view);
            }

        }

        concatLayer->base.num_temp_tensors = tmpTensorIndex;
    }
    else
    {
        vxnneOperation_Initialize(&concatLayer->concat2_operation.base,
            &concatLayer->base,
            VXNNE_OPERATION_TARGET_SW,
            VXNNE_OPERATOR_CONCAT2,
            vxnneExecuteSWConcat2,
            VX_NULL,
            batchCount,
            0);

        vxnneLayer_SetOperation(&concatLayer->base, &concatLayer->concat2_operation.base, 0);
        concatLayer->concat2_operation.inputs0           = inputs[0];
        concatLayer->concat2_operation.inputs1           = inputs[1];
        concatLayer->concat2_operation.outputs           = output_s;

        vxnneOperation_AddReference(&concatLayer->concat2_operation.base, (vx_reference)inputs[0], VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&concatLayer->concat2_operation.base, (vx_reference)inputs[1], VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&concatLayer->concat2_operation.base, (vx_reference)output_s, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }

    node->layer = &concatLayer->base;
    return status;
OnError:
    if(concatLayer)
        gcoOS_Free(gcvNULL, (gctPOINTER)concatLayer);
#endif

    return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcat2Layer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_bool _Is_concat_on_highest_dimension(
    vx_tensor tensor,
    vx_uint32 axis
    )
{
    vx_uint32 dim       = TENSOR_DIM_NUM(tensor);
    vx_bool   result    = vx_true_e;
    vx_uint32 i         = 0;

    if(axis == dim - 1)
    {
        return result;
    }

    for (i = axis + 1; i < dim; i++)
    {
        if (TENSOR_VIEW_SIZE_INDEX(tensor, i) != 1)
            return vx_false_e;
    }

    return result;
}

vx_status vxnneExecuteSWConcatIndefinite(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_concatIndefinite_sw_operation concatOp = (vxnne_concatIndefinite_sw_operation)operation;

    vx_object_array inputArray = concatOp->inputs;
    vx_tensor output = concatOp->outputs;
    vx_uint32 axis = concatOp->axis->value->n32;
    vx_uint32 i, offset = 0, dim_count = TENSOR_DIM_NUM(output);
    vx_uint8_ptr pOutputBuf = NULL;

    vxoTensor_GetTensorViewMemory(output, (gctPOINTER *)&pOutputBuf, VX_NULL);

    vxmASSERT(inputArray->itemType == VX_TYPE_TENSOR && inputArray->itemCount > 0);

    for (i = 0; i < inputArray->itemCount; i++)
    {
        vx_tensor input = (vx_tensor)inputArray->itemsTable[i];

        vx_uint8_ptr pInputBuf = NULL;
        vx_uint32 inputSize = 0;
        vx_uint32 input_slice = 1, count = 1, output_slice = 1;
        vx_uint32 m = 0, n = 0;

        if (input->isViewed)
        {
            status = vxoTensor_GetTensorSize(input, &inputSize) == VX_SUCCESS;
        }
        else
        {
            inputSize = (vx_uint32)vxoMemory_ComputeSize(&input->tensorBuffer->memory, 0);
        }

        vxoTensor_GetTensorViewMemory(input, (gctPOINTER *)&pInputBuf, VX_NULL);
        gcmASSERT(axis < dim_count);

        for (m = 0; m <= axis; m++)
        {
            input_slice *= TENSOR_SIZE_INDEX(input, m);
            output_slice *= TENSOR_SIZE_INDEX(output, m);
        }

        for (m = axis + 1; m < dim_count; m++)
        {
            count *= TENSOR_SIZE_INDEX(input, m);
        }

        /*
         *     30 * 212 * 50 * 1          60 * 212 * 50 * 1                          90 * 212 * 50 * 1
         *          _________             ____________                               ________________
         *         /        /|           /           /|                             /               /|
         *  0)    /________/ |    +     /___________/ |          =>                /_______________/ |
         *        |        | /          |           | /                            |               | /
         *        |________|/           |___________|/                             |_______________|/
         *
         *      ----------------------------------------------------------------------------------------------------
         *     212 * 30 * 50 * 10                                212 * 90 * 50 * 10
         *          _________                                    _________
         *         /        /|                                  /        /|
         *  1)    /________/ |               =>                /________/ |
         *        |        | /                                 |        | |
         *        |________|/                                  |        | |
         *                                                     |        | |
         *             +                                       |        | |
         *     212 * 60 * 50 * 10                              |        | |
         *          ________                                   |        | /
         *         /       /|                                  |________|/
         *        /_______/ |
         *        |       | |
         *        |       | |
         *        |       | /
         *        |__ ____|/
         *
         *      ----------------------------------------------------------------------------------------------------
         *     212 * 50 * 30 * 1                             212 * 50 * 90 * 1
         *          _________                                 _________
         *         /        /|                               /        /|
         *  2)    /________/ |          =>                  /        / |
         *        |        | /                             /        /  |
         *        |________|/                             /        /  /
         *                                               /________/  /
         *              +                                |        | /
         *      212 * 50 * 60 * 1                        |________|/
         *           _________
         *          /        /|
         *         /        / |
         *        /________/  |
         *        |        | /
         *        |________|/
         */
        for (m = 0; m < count; m++)
        {
            if (_IsSameType(input, output))
            {
                memcpy(pOutputBuf + (output_slice * m + offset) * TENSOR_DATA_SIZE(input), pInputBuf + input_slice * m * TENSOR_DATA_SIZE(input), input_slice * TENSOR_DATA_SIZE(input));
            }
            else
            {
                for (n = 0; n < input_slice; n++)
                {
                    vx_float32 src0 = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(input), TENSOR_QUANT_TYPE(input), input_slice * m + n, pInputBuf,
                        TENSOR_POS(input), TENSOR_TF_ZEROPOINT(input), TENSOR_TF_SCALE(input));

                    status = vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(output), TENSOR_QUANT_TYPE(output), output_slice * m + n + offset, src0, pOutputBuf,
                        TENSOR_POS(output), TENSOR_TF_ZEROPOINT(output), TENSOR_TF_SCALE(output), TENSOR_ROUNDING_MODE(output));
                }
            }

        }

        offset += input_slice;
    }

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNConcatIndefiniteLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcatIndefiniteLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcatIndefiniteLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

vx_status vxnneConcatIndefiniteLayer_Deinitialize(struct _vxnne_layer_s* layer)
{
    vx_uint32 i;
    vxnne_concatIndefinite_layer concatNLayer = (vxnne_concatIndefinite_layer)layer;

    for (i = 0; i < VX_MAX_TEMP_TENSORS; i++)
    {
        if (layer->temp_tensors[i] != VX_NULL)
        {
            vxoTensor_ReleaseTensor(&layer->temp_tensors[i]);
        }
    }

    for (i = 0; i < layer->num_operations; i++)
    {
        if (layer->operations[i]->deinitialize != VX_NULL)
        {
            layer->operations[i]->deinitialize(layer->operations[i]);
        }
    }

    if(concatNLayer->operations2)
    {
        gcoOS_Free(NULL, concatNLayer->operations2);
        concatNLayer->operations2 = VX_NULL;
    }

    if(concatNLayer->concat_sh_unit_operation)
    {
        gcoOS_Free(NULL, concatNLayer->concat_sh_unit_operation);
        concatNLayer->concat_sh_unit_operation = VX_NULL;
    }

    if (concatNLayer->tp_operation)
    {
        gcoOS_Free(NULL, concatNLayer->tp_operation);
        concatNLayer->tp_operation = VX_NULL;
    }

    return VX_SUCCESS;
}
#if REGISTER_FRAME

VX_PRIVATE_API vx_status vxoNNConcatIndefiniteLayer_SW_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vxnne_concatIndefinite_layer  concatNLayer = (vxnne_concatIndefinite_layer)ops_layer;

    vx_object_array  input_s    = (vx_object_array)parameters[0];
    vx_scalar  axis_s           = (vx_scalar)parameters[1];
    vx_tensor  output_s         = (vx_tensor)parameters[2];

    vx_uint32  i                = 0;
    vx_uint32  dimCount         = TENSOR_VIEW_DIM_NUM(output_s);
    vx_uint32  itemCount        = (vx_uint32)input_s->itemCount;
    vx_uint32  batchCount       = dimCount > 3 ? TENSOR_SIZE_INDEX(output_s, 3) : 1;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxnneOperation_Initialize(&concatNLayer->concatIndefinite_operation.base,
        &concatNLayer->base,
        VXNNE_OPERATION_TARGET_SW,
        VXNNE_OPERATOR_CONCATINDEFINITE,
        vxnneExecuteSWConcatIndefinite,
        VX_NULL,
        batchCount,
        0));

    vxmONERROR(vxnneLayer_SetOperation(
        &concatNLayer->base,
        &concatNLayer->concatIndefinite_operation.base,
        0));

    concatNLayer->concatIndefinite_operation.inputs          = input_s;
    concatNLayer->concatIndefinite_operation.axis            = axis_s;
    concatNLayer->concatIndefinite_operation.outputs         = output_s;

    for (i = 0; i < itemCount; i++)
    {
        vx_tensor input = (vx_tensor)input_s->itemsTable[i];
        vxmONERROR(vxnneOperation_AddReference(&concatNLayer->concatIndefinite_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT));
    }
    vxmONERROR(vxnneOperation_AddReference(&concatNLayer->concatIndefinite_operation.base, (vx_reference)output_s, VXNNE_OPERATION_REFENRENCE_OUTPUT));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNConcatIndefiniteLayer_SH_EVIS_Support_Ext(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_object_array  input_s    = (vx_object_array)parameters[0];
    vx_scalar  axis_s           = (vx_scalar)parameters[1];
    vx_tensor  output_s         = (vx_tensor)parameters[2];

    vx_uint32  axis             = (vx_uint32)axis_s->value->n32;
    vx_uint32  i                = 0;
    vx_uint32  dimCount         = TENSOR_VIEW_DIM_NUM(output_s);
    vx_uint32  itemCount        = (vx_uint32)input_s->itemCount;
    vx_uint32  batchCount       = dimCount > 3 ? TENSOR_SIZE_INDEX(output_s, 3) : 1;

    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    if(evis)
    {
        support = support && (TENSOR_DATA_TYPE(output_s) != VX_TYPE_FLOAT32);
        for (i = 0; i < itemCount; i++)
        {
            vx_tensor input = (vx_tensor)input_s->itemsTable[i];

            support = (vx_bool)(TENSOR_DATA_TYPE(input) != VX_TYPE_FLOAT32 && support);
        }
        support = support && (_Is_concat_on_highest_dimension(output_s, axis) || (axis < 3 && batchCount == 1));
    }
    else
    {
        support = support && (_Is_concat_on_highest_dimension(output_s, axis)
                                || ((dimCount - 1) > axis && dimCount < 4)
                                || (axis < 3 && batchCount == 1));
    }

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_bool vxoNNConcatIndefiniteLayer_SH_EVIS_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && node->base.context->evisNoInst.supportEVIS;

    if (!support)return support;


    support = support && vxoNNConcatIndefiniteLayer_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_true_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNConcatIndefiniteLayer_SH_EVIS_Initialize_Ext(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_status status = VX_SUCCESS;

    vxnne_concatIndefinite_layer  concatNLayer = (vxnne_concatIndefinite_layer)ops_layer;

    vx_object_array  input_s    = (vx_object_array)parameters[0];
    vx_scalar  axis_s           = (vx_scalar)parameters[1];
    vx_tensor  output_s         = (vx_tensor)parameters[2];

    vx_uint32  axis             = (vx_uint32)axis_s->value->n32;
    vx_uint32  i                = 0;
    vx_uint32  dimCount         = TENSOR_VIEW_DIM_NUM(output_s);
    vx_uint32  itemCount        = (vx_uint32)input_s->itemCount;
    vx_uint32  batchCount       = dimCount > 3 ? TENSOR_SIZE_INDEX(output_s, 3) : 1;
    vx_uint32  operationCount   = itemCount + 1;
    vx_uint32  opIdx            = 0;
    vx_uint32  operationIndex   = 0;
    vx_uint32  tmpTensorIndex   = 0;

    vxnne_shader_executable shaderExecutable = VX_NULL;
    vx_uint32 w         = TENSOR_VIEW_SIZE_INDEX(output_s, 0);
    vx_uint32 h         = TENSOR_VIEW_SIZE_INDEX(output_s, 1);
    vx_uint32 c         = TENSOR_VIEW_SIZE_INDEX(output_s, 2);
    vx_uint32 n         = TENSOR_VIEW_SIZE_INDEX(output_s, 3);
    vx_uint32 sizes[4]  = {w, h, c, n};
    vx_uint32 start[4]  = {0, 0, 0, 0};
    vx_uint32 end[4]    = {0, 0, 0, 0};

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_shader_operation_s) * operationCount, (gctPOINTER*)&concatNLayer->concat_sh_unit_operation);
    if (!concatNLayer->concat_sh_unit_operation)
    {
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }
    else
        gcoOS_ZeroMemory(concatNLayer->concat_sh_unit_operation, sizeof(vxnne_shader_operation_s) * operationCount);

    if (_Is_concat_on_highest_dimension(output_s, axis))
    {
        for (i = 0; i < axis; i ++)
        {
            end[i] = sizes[i];
        }

        for (i = 0; i < itemCount; i ++)
        {
            vx_tensor input = (vx_tensor)input_s->itemsTable[i];
            vx_tensor_view  tensor_view  = NULL;
            vx_tensor       subtensor    = NULL;
            vx_uint32  reshpTensor_Sizes[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {1};
            vx_uint32  reshpTensor_Dims           = 2;
            vx_tensor input_rs      = NULL;
            vx_tensor output_rs     = NULL;

            vxoElementOptimization_GetTensorShape(input, reshpTensor_Sizes, &reshpTensor_Dims);

            batchCount     = TENSOR_VIEW_DIM_NUM(input) > 3 ? TENSOR_VIEW_SIZE_INDEX(input, 3) : 1;

            start[axis]    = end[axis];
            end[axis]      += TENSOR_VIEW_SIZE_INDEX(input, axis);

            tensor_view = vxCreateTensorView(ops_layer->node->base.context, start, end, (vx_uint8)dimCount);
            if (tensor_view == NULL)
            {
                vxError("vxCreateTensorView failure! at line %d\n", __LINE__);
                vxmONERROR(VX_ERROR_NO_MEMORY);
            }

            subtensor           = vxoTensor_CreateTensorFromView(output_s, tensor_view);

            input_rs  = vxoTensor_ReshapeTensor(input, (vx_int32*)reshpTensor_Sizes, reshpTensor_Dims);
            output_rs = vxoTensor_ReshapeTensor(subtensor, (vx_int32*)reshpTensor_Sizes, reshpTensor_Dims);
            concatNLayer->base.temp_tensors[tmpTensorIndex++] = input_rs;
            concatNLayer->base.temp_tensors[tmpTensorIndex++] = output_rs;

            if(evis)
            {
                shaderExecutable    = vxnneTensorCopyShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_COPY, &ops_layer->node->kernelAttributes.borderMode, input_rs, output_rs);
            }
            else
            {
                shaderExecutable    = vxnneGPUTensorCopyShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_COPY, &ops_layer->node->kernelAttributes.borderMode, input_rs, output_rs);
            }

            if (!shaderExecutable)
            {
                vxmONERROR(VX_FAILURE);
            }

            vxmONERROR(vxnneShaderOperation_Initialize(&concatNLayer->concat_sh_unit_operation[operationIndex],
                &concatNLayer->base,
                VXNNE_OPERATOR_TENSOR_COPY,
                batchCount,
                shaderExecutable));

            vxmONERROR(vxnneOperation_AddReference(&concatNLayer->concat_sh_unit_operation[operationIndex].base, (vx_reference)input_rs, VXNNE_OPERATION_REFENRENCE_INPUT));
            vxmONERROR(vxnneOperation_AddReference(&concatNLayer->concat_sh_unit_operation[operationIndex].base, (vx_reference)output_rs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

            vxmONERROR(vxnneLayer_SetOperation(
                &concatNLayer->base,
                &concatNLayer->concat_sh_unit_operation[operationIndex].base,
                opIdx++));
            operationIndex ++;

            concatNLayer->base.temp_tensors[tmpTensorIndex++] = subtensor;
            if (tensor_view) vxmONERROR(vxReleaseTensorView(&tensor_view));
        }

    }
    else if ((dimCount - 1) > axis)
    {
        vx_tensor_create_params_t tensor_create_params;
        vx_tensor output_tmp                = NULL;
        vx_uint32 new_sizes[4]              = {w, h, c, n};
        vx_uint32 dims_idx[4]               = {0, 1, 2, 3};
        vx_uint32 perm_array[4]             = {0, 1, 2, 3};

        dimCount = dimCount < 4 ? dimCount : dimCount - 1;

        perm_array[axis]            = dims_idx[dimCount - 1];
        perm_array[dimCount - 1]    = dims_idx[axis];
        new_sizes[axis]             = sizes[dimCount - 1];
        new_sizes[dimCount - 1]     = sizes[axis];

        gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
        tensor_create_params.num_of_dims = dimCount;
        tensor_create_params.sizes = new_sizes;
        tensor_create_params.data_format = TENSOR_DATA_TYPE(output_s);
        tensor_create_params.quant_format = TENSOR_QUANT_TYPE(output_s);
        if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
        {
            tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(output_s);
        }
        else
        {
            tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(output_s);
            tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(output_s);
        }
        output_tmp = vxoTensor_CreateTensor(ops_layer->node->base.context, ops_layer->node->graph, &tensor_create_params, vx_true_e);

        concatNLayer->base.temp_tensors[tmpTensorIndex++] = output_tmp;

        for (i = 0; i < dimCount - 1; i ++)
        {
            end[i] = new_sizes[i];
        }

        for (i = 0; i < itemCount; i ++)
        {
            vx_tensor input = (vx_tensor)input_s->itemsTable[i];
            vx_tensor_view  tensor_view  = NULL;
            vx_tensor       subtensor    = NULL;

            batchCount                   = 1;

            start[dimCount - 1]    = end[dimCount - 1];
            end[dimCount - 1]      += TENSOR_VIEW_SIZE_INDEX(input, axis);

            tensor_view = vxCreateTensorView(ops_layer->node->base.context, start, end, (vx_uint8)dimCount);
            if (tensor_view == NULL)
            {
                vxError("vxCreateTensorView failure! at line %d\n", __LINE__);
                vxmONERROR(VX_FAILURE);
            }

            subtensor           = vxoTensor_CreateTensorFromView(output_tmp, tensor_view);

            /* operation i: permute input concat dimension to the highest dimension */
            if(evis)
            {
               shaderExecutable = vxnneTensorTransposeShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_TRANSPOSE, &ops_layer->node->kernelAttributes.borderMode, input, perm_array, 3, subtensor);
            }
            else
            {
                shaderExecutable = vxnneGPUTensorTransposeShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_TRANSPOSE, &ops_layer->node->kernelAttributes.borderMode, input, perm_array, 3, subtensor);
            }

            if (!shaderExecutable)
            {
                vxmONERROR(VX_FAILURE);
            }

            vxmONERROR(vxnneShaderOperation_Initialize(&concatNLayer->concat_sh_unit_operation[operationIndex],
                &concatNLayer->base,
                VXNNE_OPERATOR_TENSOR_TRANS,
                batchCount,
                shaderExecutable));

            vxmONERROR(vxnneOperation_AddReference(&concatNLayer->concat_sh_unit_operation[operationIndex].base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT));
            vxmONERROR(vxnneOperation_AddReference(&concatNLayer->concat_sh_unit_operation[operationIndex].base, (vx_reference)subtensor, VXNNE_OPERATION_REFENRENCE_OUTPUT));

            vxmONERROR(vxnneLayer_SetOperation(
                &concatNLayer->base,
                &concatNLayer->concat_sh_unit_operation[operationIndex].base,
                opIdx++));
            operationIndex ++;

            concatNLayer->base.temp_tensors[tmpTensorIndex++] = subtensor;
            if (tensor_view) vxmONERROR(vxReleaseTensorView(&tensor_view));
        }

        /* operation : permute to get final result */
        if(evis)
        {
           shaderExecutable = vxnneTensorTransposeShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_TRANSPOSE, &ops_layer->node->kernelAttributes.borderMode, output_tmp, perm_array, 3, output_s);
        }
        else
        {
           shaderExecutable = vxnneGPUTensorTransposeShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_TRANSPOSE, &ops_layer->node->kernelAttributes.borderMode, output_tmp, perm_array, 3, output_s);
        }

        if (!shaderExecutable)
        {
            vxmONERROR(VX_FAILURE);
        }

        vxmONERROR(vxnneShaderOperation_Initialize(&concatNLayer->concat_sh_unit_operation[operationIndex],
            &concatNLayer->base,
            VXNNE_OPERATOR_TENSOR_TRANS,
            batchCount,
            shaderExecutable));

        vxmONERROR(vxnneOperation_AddReference(&concatNLayer->concat_sh_unit_operation[operationIndex].base, (vx_reference)output_tmp, VXNNE_OPERATION_REFENRENCE_INPUT));
        vxmONERROR(vxnneOperation_AddReference(&concatNLayer->concat_sh_unit_operation[operationIndex].base, (vx_reference)output_s, VXNNE_OPERATION_REFENRENCE_OUTPUT));

        vxmONERROR(vxnneLayer_SetOperation(
            &concatNLayer->base,
            &concatNLayer->concat_sh_unit_operation[operationIndex].base,
            opIdx++));
        operationIndex ++;
    }
    concatNLayer->base.num_temp_tensors = tmpTensorIndex;

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}
VX_PRIVATE_API vx_status vxoNNConcatIndefiniteLayer_SH_EVIS_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxoNNConcatIndefiniteLayer_SH_EVIS_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_true_e));
OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status vxoNNConcatIndefiniteLayer_SH_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxoNNConcatIndefiniteLayer_SH_EVIS_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_false_e));
OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_bool vxoNNConcatIndefiniteLayer_SH_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);


    support = support && vxoNNConcatIndefiniteLayer_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_false_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_bool vxoNNConcatIndefiniteLayer_TP_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_object_array  input_s    = (vx_object_array)parameters[0];
    vx_tensor  output_s         = (vx_tensor)parameters[2];
    vx_uint32  i                = 0;
    vx_uint32  itemCount        = (vx_uint32)input_s->itemCount;

    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_TP, VX_TYPE_INVALID, VX_NULL);

    if (!support)
        return support;

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    for (i = 0; i < itemCount; i++)
    {
        vx_tensor input = (vx_tensor)input_s->itemsTable[i];

        if (!vxnneIsTPSupportFormat(node->base.context, input, VX_NULL, output_s))
        {
            support = vx_false_e;
            break;
        }
    }
    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNConcatIndefiniteLayer_TP_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vxnne_concatIndefinite_layer  concatNLayer = (vxnne_concatIndefinite_layer)ops_layer;

    vx_object_array  input_s    = (vx_object_array)parameters[0];
    vx_scalar  axis_s           = (vx_scalar)parameters[1];
    vx_tensor  output_s         = (vx_tensor)parameters[2];

    vx_uint32  axis             = (vx_uint32)axis_s->value->n32;
    vx_uint32  i                = 0;
    vx_uint32  dimCount         = TENSOR_VIEW_DIM_NUM(output_s);
    vx_uint32  itemCount        = (vx_uint32)input_s->itemCount;
    vx_uint32  batchCount       = dimCount > 3 ? TENSOR_SIZE_INDEX(output_s, 3) : 1;
    vx_uint32  operationIndex   = 0;
    vx_uint32  tmpTensorIndex   = 0;

    vx_tensor input = VX_NULL, output = VX_NULL;
    vx_tensor_view tensor_view = VX_NULL;
    vx_uint32 start[4] = {0, 0, 0, 0}, end[4] = {0, 0, 0, 0};
    vx_uint32 op_count = itemCount;
    vx_op_param param = VX_NULL;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_tp_operation_s) * op_count, (gctPOINTER*)&concatNLayer->tp_operation);
    gcoOS_ZeroMemory(concatNLayer->tp_operation, sizeof(vxnne_tp_operation_s) * op_count);

    for (i = 0; i < dimCount; i++)
    {
        if (i != axis)
        {
            end[i] = TENSOR_VIEW_SIZE_INDEX(output_s, i);
        }
    }

    end[axis] = start[axis];

    for (i = 0; i < itemCount; i++)
    {
        input = (vx_tensor)input_s->itemsTable[i];
        batchCount = TENSOR_VIEW_DIM_NUM(input) > 3 ? TENSOR_VIEW_SIZE_INDEX(input, 3) : 1;

        end[axis] = start[axis] + TENSOR_VIEW_SIZE_INDEX(input, axis);

        tensor_view = vxCreateTensorView(ops_layer->node->base.context, start, end, (vx_uint8)dimCount);
        if (!tensor_view)
        {
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }

        output = vxoTensor_CreateTensorFromView(output_s, tensor_view);

        vxmONERROR(vxnneOperation_Initialize(&concatNLayer->tp_operation[i].base,
                                             &concatNLayer->base,
                                             VXNNE_OPERATION_TARGET_TP,
                                             VXNNE_OPERATOR_CONCATINDEFINITE,
                                             VX_NULL,
                                             VX_NULL,
                                             batchCount,
                                             0));

        param = &concatNLayer->tp_operation[i].base.parameter;
        param->tpType = TP_TENSOR_COPY4CONCAT;

        concatNLayer->tp_operation[i].input = input;
        concatNLayer->tp_operation[i].output = output;

        vxmONERROR(vxnneLayer_SetOperation(&concatNLayer->base,
                                           &concatNLayer->tp_operation[i].base,
                                           operationIndex++));

        vxmONERROR(vxnneOperation_AddReference(&concatNLayer->tp_operation[i].base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT));
        vxmONERROR(vxnneOperation_AddReference(&concatNLayer->tp_operation[i].base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT));

        concatNLayer->base.temp_tensors[tmpTensorIndex++] = output;

        vxReleaseTensorView(&tensor_view);

        /* Update the start point of view. */
        start[axis] = end[axis];

    }

    concatNLayer->base.num_temp_tensors = tmpTensorIndex;

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_status vxoNNLayer_GetOperations(vxnne_layer ops_layer, vx_uint32_ptr max_num_operations, vxnne_operation **operations)
{
    vx_status  status = VX_SUCCESS;
    vx_object_array  input_s    = (vx_object_array)ops_layer->node->paramTable[0];
    vx_uint32  itemCount        = (vx_uint32)input_s->itemCount;

    *max_num_operations = itemCount + 1;

    vxmONERROR(gcoOS_Allocate(gcvNULL, sizeof(vxnne_operation) * (itemCount + 1), (gctPOINTER*)operations));

OnError:
    return status;
}
#endif


VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcatIndefiniteLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
#if REGISTER_FRAME

    vxnne_layer_imp_s registerConcatIndefiniteLayers[] = {/* Please DON'T adjust the order, it's importent */
        { "Concat NN", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "Concat TP", vxoNNConcatIndefiniteLayer_TP_Support, vxoNNConcatIndefiniteLayer_TP_Initialize, vxnneConcatIndefiniteLayer_Deinitialize },
        { "Concat SH EVIS", vxoNNConcatIndefiniteLayer_SH_EVIS_Support, vxoNNConcatIndefiniteLayer_SH_EVIS_Initialize, vxnneConcatIndefiniteLayer_Deinitialize },
        { "Concat SH F32", vxoNNConcatIndefiniteLayer_SH_Support, vxoNNConcatIndefiniteLayer_SH_Initialize, vxnneConcatIndefiniteLayer_Deinitialize },
        { "Concat SW", vxoNNCommon_Support, vxoNNConcatIndefiniteLayer_SW_Initialize, vxnneConcatIndefiniteLayer_Deinitialize },
    };

    REGISTER_LAYERS(registerConcatIndefiniteLayers, vxnne_concatIndefinite_layer_s, "ConcatLayer", vxoNNLayer_GetOperations);

OnError:
#else
    vx_context context          = vxGetContext((vx_reference)node);

    vx_object_array  input_s    = (vx_object_array)parameters[0];
    vx_scalar  axis_s           = (vx_scalar)parameters[1];
    vx_tensor  output_s         = (vx_tensor)parameters[2];

    vx_bool    enable_SHExe     = vx_false_e;
    vx_uint32  axis             = (vx_uint32)axis_s->value->n32;
    vx_uint32  i                = 0;
    vx_uint32  dimCount         = TENSOR_VIEW_DIM_NUM(output_s);
    vx_uint32  itemCount        = (vx_uint32)input_s->itemCount;
    vx_uint32  batchCount       = dimCount > 3 ? TENSOR_SIZE_INDEX(output_s, 3) : 1;
    vx_uint32  operationCount   = 1;
    vx_uint32  opIdx            = 0;
    vx_uint32  operationIndex   = 0;
    vx_uint32  tmpTensorIndex   = 0;
    vx_bool    isTpSupportFormat = vx_true_e;

    vxnne_concatIndefinite_layer  concatNLayer = VX_NULL;
    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    if(node->base.context->evisNoInst.supportEVIS)
    {
        enable_SHExe = (vx_bool)(TENSOR_DATA_TYPE(output_s) != VX_TYPE_FLOAT32);
        for (i = 0; i < itemCount; i++)
        {
            vx_tensor input = (vx_tensor)input_s->itemsTable[i];

            enable_SHExe = (vx_bool)(TENSOR_DATA_TYPE(input) != VX_TYPE_FLOAT32 && enable_SHExe);
        }
        enable_SHExe = enable_SHExe && (_Is_concat_on_highest_dimension(output_s, axis) || (axis < 3 && batchCount == 1));
    }
    else
    {
        enable_SHExe = (vx_bool)(_Is_concat_on_highest_dimension(output_s, axis)
                                || ((dimCount - 1) > axis && dimCount < 4)
                                || (axis < 3 && batchCount == 1));
    }

    for (i = 0; i < itemCount; i++)
    {
        vx_tensor input = (vx_tensor)input_s->itemsTable[i];

        if (!vxnneIsTPSupportFormat(context, input, VX_NULL, output_s))
        {
            isTpSupportFormat = vx_false_e;
            break;
        }
    }

    enable_SHExe = enable_SHExe && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER));

    if (enable_SHExe)
        operationCount = itemCount + 1;

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_concatIndefinite_layer_s), (gctPOINTER*)&concatNLayer);
    if (!concatNLayer)
    {
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }

    gcoOS_ZeroMemory(concatNLayer, sizeof(vxnne_concatIndefinite_layer_s));

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_operation) * (itemCount + 1), (gctPOINTER*)&concatNLayer->operations2);

    gcoOS_ZeroMemory(concatNLayer->operations2, sizeof(vxnne_operation) * (itemCount + 1));

    vxmONERROR(vxnneLayer_Initialize(&concatNLayer->base,
                                     "ConcatLayer",
                                     node,
                                     itemCount + 1,
                                     concatNLayer->operations2,
                                     vxnneConcatIndefiniteLayer_Deinitialize));

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP) &&
        isTpSupportFormat)
    {
        vx_tensor input = VX_NULL, output = VX_NULL;
        vx_tensor_view tensor_view = VX_NULL;
        vx_uint32 start[4] = {0, 0, 0, 0}, end[4] = {0, 0, 0, 0};
        vx_uint32 op_count = itemCount;
        vx_op_param param = VX_NULL;

        gcoOS_Allocate(gcvNULL, sizeof(vxnne_tp_operation_s) * op_count, (gctPOINTER*)&concatNLayer->tp_operation);
        gcoOS_ZeroMemory(concatNLayer->tp_operation, sizeof(vxnne_tp_operation_s) * op_count);

        for (i = 0; i < dimCount; i++)
        {
            if (i != axis)
            {
                end[i] = TENSOR_VIEW_SIZE_INDEX(output_s, i);
            }
        }

        end[axis] = start[axis];

        for (i = 0; i < itemCount; i++)
        {
            input = (vx_tensor)input_s->itemsTable[i];
            batchCount = TENSOR_VIEW_DIM_NUM(input) > 3 ? TENSOR_VIEW_SIZE_INDEX(input, 3) : 1;

            end[axis] = start[axis] + TENSOR_VIEW_SIZE_INDEX(input, axis);

            tensor_view = vxCreateTensorView(context, start, end, (vx_uint8)dimCount);
            if (!tensor_view)
            {
                vxmONERROR(VX_ERROR_NO_MEMORY);
            }

            output = vxoTensor_CreateTensorFromView(output_s, tensor_view);

            vxmONERROR(vxnneOperation_Initialize(&concatNLayer->tp_operation[i].base,
                                                 &concatNLayer->base,
                                                 VXNNE_OPERATION_TARGET_TP,
                                                 VXNNE_OPERATOR_CONCATINDEFINITE,
                                                 VX_NULL,
                                                 VX_NULL,
                                                 batchCount,
                                                 0));

            param = &concatNLayer->tp_operation[i].base.parameter;
            param->tpType = TP_TENSOR_COPY4CONCAT;

            concatNLayer->tp_operation[i].input = input;
            concatNLayer->tp_operation[i].output = output;

            vxmONERROR(vxnneLayer_SetOperation(&concatNLayer->base,
                                               &concatNLayer->tp_operation[i].base,
                                               operationIndex++));

            vxnneOperation_AddReference(&concatNLayer->tp_operation[i].base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&concatNLayer->tp_operation[i].base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            concatNLayer->base.temp_tensors[tmpTensorIndex++] = output;

            vxReleaseTensorView(&tensor_view);

            /* Update the start point of view. */
            start[axis] = end[axis];
        }
    }
    else if (enable_SHExe)
    {
        vxnne_shader_executable shaderExecutable = VX_NULL;
        vx_uint32 i          = 0;
        vx_uint32 w         = TENSOR_VIEW_SIZE_INDEX(output_s, 0);
        vx_uint32 h         = TENSOR_VIEW_SIZE_INDEX(output_s, 1);
        vx_uint32 c         = TENSOR_VIEW_SIZE_INDEX(output_s, 2);
        vx_uint32 n         = TENSOR_VIEW_SIZE_INDEX(output_s, 3);
        vx_uint32 sizes[4]  = {w, h, c, n};
        vx_uint32 start[4]  = {0, 0, 0, 0};
        vx_uint32 end[4]    = {0, 0, 0, 0};

        gcoOS_Allocate(gcvNULL, sizeof(vxnne_shader_operation_s) * operationCount, (gctPOINTER*)&concatNLayer->concat_sh_unit_operation);
        if (!concatNLayer->concat_sh_unit_operation)
        {
            vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }
        else
            gcoOS_ZeroMemory(concatNLayer->concat_sh_unit_operation, sizeof(vxnne_shader_operation_s) * operationCount);

        if (_Is_concat_on_highest_dimension(output_s, axis))
        {
            for (i = 0; i < axis; i ++)
            {
                end[i] = sizes[i];
            }

            for (i = 0; i < itemCount; i ++)
            {
                vx_tensor input = (vx_tensor)input_s->itemsTable[i];
                vx_tensor_view  tensor_view  = NULL;
                vx_tensor       subtensor    = NULL;
                vx_uint32  reshpTensor_Sizes[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {1};
                vx_uint32  reshpTensor_Dims           = 2;
                vx_tensor input_rs      = NULL;
                vx_tensor output_rs     = NULL;

                vxoElementOptimization_GetTensorShape(input, reshpTensor_Sizes, &reshpTensor_Dims);

                batchCount     = TENSOR_VIEW_DIM_NUM(input) > 3 ? TENSOR_VIEW_SIZE_INDEX(input, 3) : 1;

                start[axis]    = end[axis];
                end[axis]      += TENSOR_VIEW_SIZE_INDEX(input, axis);

                tensor_view = vxCreateTensorView(node->base.context, start, end, (vx_uint8)dimCount);
                if (tensor_view == NULL)
                {
                    vxError("vxCreateTensorView failure! at line %d\n", __LINE__);
                    vxmONERROR(VX_ERROR_NO_MEMORY);
                }

                subtensor           = vxoTensor_CreateTensorFromView(output_s, tensor_view);

                input_rs  = vxoTensor_ReshapeTensor(input, (vx_int32*)reshpTensor_Sizes, reshpTensor_Dims);
                output_rs = vxoTensor_ReshapeTensor(subtensor, (vx_int32*)reshpTensor_Sizes, reshpTensor_Dims);
                concatNLayer->base.temp_tensors[tmpTensorIndex++] = input_rs;
                concatNLayer->base.temp_tensors[tmpTensorIndex++] = output_rs;

                if(node->base.context->evisNoInst.supportEVIS)
                {
                    shaderExecutable    = vxnneTensorCopyShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_COPY, &node->kernelAttributes.borderMode, input_rs, output_rs);
                }
                else
                {
                    shaderExecutable    = vxnneGPUTensorCopyShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_COPY, &node->kernelAttributes.borderMode, input_rs, output_rs);
                }

                if (!shaderExecutable)
                {
                    vxmONERROR(VX_FAILURE);
                }

                vxmONERROR(vxnneShaderOperation_Initialize(&concatNLayer->concat_sh_unit_operation[operationIndex],
                    &concatNLayer->base,
                    VXNNE_OPERATOR_TENSOR_COPY,
                    batchCount,
                    shaderExecutable));

                vxnneOperation_AddReference(&concatNLayer->concat_sh_unit_operation[operationIndex].base, (vx_reference)input_rs, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&concatNLayer->concat_sh_unit_operation[operationIndex].base, (vx_reference)output_rs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                vxnneLayer_SetOperation(
                    &concatNLayer->base,
                    &concatNLayer->concat_sh_unit_operation[operationIndex].base,
                    opIdx++);
                operationIndex ++;

                concatNLayer->base.temp_tensors[tmpTensorIndex++] = subtensor;
                if (tensor_view) vxReleaseTensorView(&tensor_view);
            }

        }
        else if ((dimCount - 1) > axis)
        {
            vx_tensor_create_params_t tensor_create_params;
            vx_tensor output_tmp                = NULL;
            vx_uint32 new_sizes[4]              = {w, h, c, n};
            vx_uint32 dims_idx[4]               = {0, 1, 2, 3};
            vx_uint32 perm_array[4]             = {0, 1, 2, 3};

            dimCount = dimCount < 4 ? dimCount : dimCount - 1;

            perm_array[axis]            = dims_idx[dimCount - 1];
            perm_array[dimCount - 1]    = dims_idx[axis];
            new_sizes[axis]             = sizes[dimCount - 1];
            new_sizes[dimCount - 1]     = sizes[axis];

            gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
            tensor_create_params.num_of_dims = dimCount;
            tensor_create_params.sizes = new_sizes;
            tensor_create_params.data_format = TENSOR_DATA_TYPE(output_s);
            tensor_create_params.quant_format = TENSOR_QUANT_TYPE(output_s);
            if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
            {
                tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(output_s);
            }
            else
            {
                tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(output_s);
                tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(output_s);
            }
            output_tmp = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);

            concatNLayer->base.temp_tensors[tmpTensorIndex++] = output_tmp;

            for (i = 0; i < dimCount - 1; i ++)
            {
                end[i] = new_sizes[i];
            }

            for (i = 0; i < itemCount; i ++)
            {
                vx_tensor input = (vx_tensor)input_s->itemsTable[i];
                vx_tensor_view  tensor_view  = NULL;
                vx_tensor       subtensor    = NULL;

                batchCount                   = 1;

                start[dimCount - 1]    = end[dimCount - 1];
                end[dimCount - 1]      += TENSOR_VIEW_SIZE_INDEX(input, axis);

                tensor_view = vxCreateTensorView(node->base.context, start, end, (vx_uint8)dimCount);
                if (tensor_view == NULL)
                {
                    vxError("vxCreateTensorView failure! at line %d\n", __LINE__);
                    vxmONERROR(VX_FAILURE);
                }

                subtensor           = vxoTensor_CreateTensorFromView(output_tmp, tensor_view);

                /* operation i: permute input concat dimension to the highest dimension */
                if(node->base.context->evisNoInst.supportEVIS)
                {
                   shaderExecutable = vxnneTensorTransposeShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_TRANSPOSE, &node->kernelAttributes.borderMode, input, perm_array, 3, subtensor);
                }
                else
                {
                    shaderExecutable = vxnneGPUTensorTransposeShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_TRANSPOSE, &node->kernelAttributes.borderMode, input, perm_array, 3, subtensor);
                }

                if (!shaderExecutable)
                {
                    vxmONERROR(VX_FAILURE);
                }

                vxmONERROR(vxnneShaderOperation_Initialize(&concatNLayer->concat_sh_unit_operation[operationIndex],
                    &concatNLayer->base,
                    VXNNE_OPERATOR_TENSOR_TRANS,
                    batchCount,
                    shaderExecutable));

                vxnneOperation_AddReference(&concatNLayer->concat_sh_unit_operation[operationIndex].base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&concatNLayer->concat_sh_unit_operation[operationIndex].base, (vx_reference)subtensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                vxnneLayer_SetOperation(
                    &concatNLayer->base,
                    &concatNLayer->concat_sh_unit_operation[operationIndex].base,
                    opIdx++);
                operationIndex ++;

                concatNLayer->base.temp_tensors[tmpTensorIndex++] = subtensor;
                if (tensor_view) vxReleaseTensorView(&tensor_view);
            }

            /* operation : permute to get final result */
            if(node->base.context->evisNoInst.supportEVIS)
            {
               shaderExecutable = vxnneTensorTransposeShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_TRANSPOSE, &node->kernelAttributes.borderMode, output_tmp, perm_array, 3, output_s);
            }
            else
            {
               shaderExecutable = vxnneGPUTensorTransposeShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_TRANSPOSE, &node->kernelAttributes.borderMode, output_tmp, perm_array, 3, output_s);
            }

            if (!shaderExecutable)
            {
                vxmONERROR(VX_FAILURE);
            }

            vxmONERROR(vxnneShaderOperation_Initialize(&concatNLayer->concat_sh_unit_operation[operationIndex],
                &concatNLayer->base,
                VXNNE_OPERATOR_TENSOR_TRANS,
                batchCount,
                shaderExecutable));

            vxnneOperation_AddReference(&concatNLayer->concat_sh_unit_operation[operationIndex].base, (vx_reference)output_tmp, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&concatNLayer->concat_sh_unit_operation[operationIndex].base, (vx_reference)output_s, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            vxnneLayer_SetOperation(
                &concatNLayer->base,
                &concatNLayer->concat_sh_unit_operation[operationIndex].base,
                opIdx++);
            operationIndex ++;
        }
        concatNLayer->base.num_temp_tensors = tmpTensorIndex;
    }
    else
    {
        vxnneOperation_Initialize(&concatNLayer->concatIndefinite_operation.base,
            &concatNLayer->base,
            VXNNE_OPERATION_TARGET_SW,
            VXNNE_OPERATOR_CONCATINDEFINITE,
            vxnneExecuteSWConcatIndefinite,
            VX_NULL,
            batchCount,
            0);

        vxnneLayer_SetOperation(
            &concatNLayer->base,
            &concatNLayer->concatIndefinite_operation.base,
            0);

        concatNLayer->concatIndefinite_operation.inputs          = input_s;
        concatNLayer->concatIndefinite_operation.axis            = axis_s;
        concatNLayer->concatIndefinite_operation.outputs         = output_s;

        for (i = 0; i < itemCount; i++)
        {
            vx_tensor input = (vx_tensor)input_s->itemsTable[i];
            vxnneOperation_AddReference(&concatNLayer->concatIndefinite_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
        }
        vxnneOperation_AddReference(&concatNLayer->concatIndefinite_operation.base, (vx_reference)output_s, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }

    node->layer = &concatNLayer->base;

    return status;

OnError:
    if(concatNLayer)
    {
        if (concatNLayer->tp_operation)
        {
            gcoOS_Free(gcvNULL, concatNLayer->tp_operation);
            concatNLayer->tp_operation = VX_NULL;
        }

        if(concatNLayer->operations2)
        {
            gcoOS_Free(gcvNULL, concatNLayer->operations2);
        }
        if(concatNLayer->concat_sh_unit_operation)
        {
            gcoOS_Free(gcvNULL, concatNLayer->concat_sh_unit_operation);
        }

        gcoOS_Free(gcvNULL, (gctPOINTER)concatNLayer);
    }
#endif

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcatIndefiniteLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = NULL;
    }

    return VX_SUCCESS;
}

