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
#include <layers/gc_vx_layer_l2norm.h>

/***************************************************************************************************************************
 *                                                 L2Normalize
 ***************************************************************************************************************************/

vx_status vxnneExecuteSWL2Normalize(struct _vxnne_operation_s *operation)
{
    vxnne_l2normalize_operation l2normalizeOperation   = (vxnne_l2normalize_operation)operation;

    vx_tensor  inputs           = (vx_tensor)l2normalizeOperation->inputs;
    vx_tensor  outputs          = (vx_tensor)l2normalizeOperation->outputs;

    vx_uint32  dims             = TENSOR_VIEW_DIM_NUM(inputs);
    vx_uint32  input_width      = TENSOR_SIZE_INDEX(inputs, 0);
    vx_uint32  input_height     = TENSOR_SIZE_INDEX(inputs, 1);
    vx_uint32  input_depth      = TENSOR_SIZE_INDEX(inputs, 2);
    vx_type_e  inputFormat      = (vx_type_e)TENSOR_DATA_TYPE(inputs);
    vx_type_e  outputFormat     = (vx_type_e)TENSOR_DATA_TYPE(outputs);
    vx_int8   inputFixedPoint   = TENSOR_POS(inputs);
    vx_int8   outputFixedPoint  = TENSOR_POS(outputs);

    vx_uint32  i,j,c;
    vx_float32 epsilon = (vx_float32)1e-12;
    vx_float32 rsqrt = 0.0f;
    vx_float32 data = 0.0f;
    gctPOINTER inputBase;
    gctPOINTER outputBase;

    vx_status status = VX_SUCCESS;

    switch(dims)
    {
    case 1:
        input_depth   = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
        input_width   = 1;
        input_height  = 1;
        break;
    case 2:
        input_width   = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
        input_depth   = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
        input_height  = 1;
        break;
    case 3:
    case 4:
        input_width   = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
        input_height  = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
        input_depth   = TENSOR_VIEW_SIZE_INDEX(inputs, 2);
        break;
    default:
        vxError("Input tensor error dimension[%u]\n", dims);
    }

    vxoTensor_GetTensorBatchArrayViewMemory(inputs, operation->currBatchIndex, (gctPOINTER *)&inputBase, VX_NULL);
    vxoTensor_GetTensorBatchArrayViewMemory(outputs, operation->currBatchIndex, (gctPOINTER *)&outputBase, VX_NULL);

    if ((inputFormat != VX_TYPE_FLOAT16 && inputFormat != VX_TYPE_FLOAT32 && inputFormat != VX_TYPE_INT8 && inputFormat != VX_TYPE_INT16 && inputFormat != VX_TYPE_UINT8)
        || (outputFormat != VX_TYPE_FLOAT16 && outputFormat != VX_TYPE_FLOAT32 && outputFormat != VX_TYPE_INT8 && outputFormat != VX_TYPE_INT16 && outputFormat != VX_TYPE_UINT8))
    {
        vxError("input or outputs format is not support");
        status = VX_ERROR_NOT_SUPPORTED;
        return status;
    }

    for(j = 0; j < input_height; ++j)
    {
        for(i = 0; i < input_width; ++i)
        {
            vx_float32 sum = 0.0f;
            for (c = 0; c < input_depth; c++)
            {
                vx_int32 in_index = i + input_width * (j + input_height * c);
                data = vxnneGetDataExt(inputFormat, TENSOR_QUANT_TYPE(inputs), in_index, (vx_uint8_ptr)inputBase, inputFixedPoint, TENSOR_TF_ZEROPOINT(inputs), TENSOR_TF_SCALE(inputs));
                sum += data * data;
            }
            rsqrt = gcoMATH_ReciprocalSquareRoot(gcoMATH_MAX(sum,epsilon));
            for (c = 0; c < input_depth; c++)
            {
                vx_int32 index = i + input_width * (j + input_height * c);
                data = vxnneGetDataExt(inputFormat, TENSOR_QUANT_TYPE(inputs), index, (vx_uint8_ptr)inputBase, inputFixedPoint, TENSOR_TF_ZEROPOINT(inputs), TENSOR_TF_SCALE(inputs));
                data *= rsqrt;
                vxnneSaveDataExt(outputFormat, TENSOR_QUANT_TYPE(outputs), index, data, (vx_uint8_ptr)outputBase, outputFixedPoint, TENSOR_TF_ZEROPOINT(outputs), TENSOR_TF_SCALE(outputs), TENSOR_ROUNDING_MODE(outputs));
            }
        }
    }

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNL2NormalizeLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNL2NormalizeLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNL2NormalizeLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}
#if REGISTER_FRAME

VX_PRIVATE_API vx_status vxoNNL2NormalizeLayer_SW_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vxnne_l2normalize_layer l2normalizeLayer = (vxnne_l2normalize_layer)ops_layer;
    vx_tensor  input                      = (vx_tensor)parameters[0];
    vx_tensor  output                     = (vx_tensor)parameters[1];
    vx_uint32  dims                       = TENSOR_VIEW_DIM_NUM(input);
    vx_uint32  width                      = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32  height                     = (dims > 1) ? TENSOR_VIEW_SIZE_INDEX(input, 1) : 1;
    vx_uint32  depth                      = (dims > 2) ? TENSOR_VIEW_SIZE_INDEX(input, 2) : 1;
    vx_uint32  batch                      = (dims > 3) ? TENSOR_VIEW_SIZE_INDEX(input, 3) : 1;

    switch(dims)
    {
    case 1:
        depth   = TENSOR_VIEW_SIZE_INDEX(input, 0);
        batch   = 1;
        width   = 1;
        height  = 1;
        dims    = 3;
        break;
    case 2:
        depth   = TENSOR_VIEW_SIZE_INDEX(input, 0);
        batch   = TENSOR_VIEW_SIZE_INDEX(input, 1);
        width   = 1;
        height  = 1;
        dims    = 4;
        break;
    case 3:
        width   = TENSOR_VIEW_SIZE_INDEX(input, 0);
        height  = TENSOR_VIEW_SIZE_INDEX(input, 1);
        depth   = TENSOR_VIEW_SIZE_INDEX(input, 2);
        batch   = 1;
        dims    = 3;
        break;
    case 4:
        width   = TENSOR_VIEW_SIZE_INDEX(input, 0);
        height  = TENSOR_VIEW_SIZE_INDEX(input, 1);
        depth   = TENSOR_VIEW_SIZE_INDEX(input, 2);
        batch   = TENSOR_VIEW_SIZE_INDEX(input, 3);
        break;
    default:
        vxError("Input tensor error dimension[%u]\n", dims);
    }

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);
    vxmONERROR(vxnneOperation_Initialize(&l2normalizeLayer->l2normalize_sw_operation.base,
                                &l2normalizeLayer->base,
                                VXNNE_OPERATION_TARGET_SW,
                                VXNNE_OPERATOR_L2NORMALIZE,
                                vxnneExecuteSWL2Normalize,
                                VX_NULL,
                                batch,
                                0));

    vxmONERROR(vxnneLayer_SetOperation(
        &l2normalizeLayer->base,
        &l2normalizeLayer->l2normalize_sw_operation.base,
        0));

    l2normalizeLayer->l2normalize_sw_operation.inputs           = input;
    l2normalizeLayer->l2normalize_sw_operation.outputs          = output;

    vxmONERROR(vxnneOperation_AddReference(&l2normalizeLayer->l2normalize_sw_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&l2normalizeLayer->l2normalize_sw_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNL2NormalizeLayer_SH_EVIS_Support_Ext(vx_node node, const vx_reference parameters[], vx_uint32 _num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_tensor  input                      = (vx_tensor)parameters[0];
    vx_tensor  output                     = (vx_tensor)parameters[1];
    vx_bool    enableShader;
    vx_type_e  inputFormat                = (vx_type_e)TENSOR_DATA_TYPE(input);
    vx_type_e  outputFormat               = (vx_type_e)TENSOR_DATA_TYPE(output);
    vx_uint32  dims                       = TENSOR_VIEW_DIM_NUM(input);
    vx_uint32  width                      = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32  height                     = (dims > 1) ? TENSOR_VIEW_SIZE_INDEX(input, 1) : 1;
    vx_uint32  depth                      = (dims > 2) ? TENSOR_VIEW_SIZE_INDEX(input, 2) : 1;
    vx_uint32  batch                      = (dims > 3) ? TENSOR_VIEW_SIZE_INDEX(input, 3) : 1;
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, _num, reg_param);
    switch(dims)
    {
    case 1:
        depth   = TENSOR_VIEW_SIZE_INDEX(input, 0);
        batch   = 1;
        width   = 1;
        height  = 1;
        dims    = 3;
        break;
    case 2:
        depth   = TENSOR_VIEW_SIZE_INDEX(input, 0);
        batch   = TENSOR_VIEW_SIZE_INDEX(input, 1);
        width   = 1;
        height  = 1;
        dims    = 4;
        break;
    case 3:
        width   = TENSOR_VIEW_SIZE_INDEX(input, 0);
        height  = TENSOR_VIEW_SIZE_INDEX(input, 1);
        depth   = TENSOR_VIEW_SIZE_INDEX(input, 2);
        batch   = 1;
        dims    = 3;
        break;
    case 4:
        width   = TENSOR_VIEW_SIZE_INDEX(input, 0);
        height  = TENSOR_VIEW_SIZE_INDEX(input, 1);
        depth   = TENSOR_VIEW_SIZE_INDEX(input, 2);
        batch   = TENSOR_VIEW_SIZE_INDEX(input, 3);
        break;
    default:
        vxError("Input tensor error dimension[%u]\n", dims);
    }

    if(evis)
    {
        enableShader = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
                          || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT8)
                          || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT16)
                          || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_UINT8)
                          || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8)
                          || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_FLOAT16)
                          || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16)
                          || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_FLOAT16)
                          || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
                          || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT16)
                          || (inputFormat == VX_TYPE_BFLOAT16 && outputFormat == VX_TYPE_BFLOAT16));
        enableShader  = enableShader && (width * height < IMG_MAX_WIDTH);
    }
    else
        enableShader = vx_true_e;

    support = support && enableShader;

    vxoLayer_VerificationFoot(node, parameters, _num, reg_param, &support);
    return support;
}

VX_PRIVATE_API vx_status vxoNNL2NormalizeLayer_SH_Initialize_Ext(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 _num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_status status = VX_SUCCESS;

    vx_tensor  input                      = (vx_tensor)parameters[0];
    vx_tensor  output                     = (vx_tensor)parameters[1];
    vx_uint32  dims                       = TENSOR_VIEW_DIM_NUM(input);
    vx_uint32  width                      = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32  height                     = (dims > 1) ? TENSOR_VIEW_SIZE_INDEX(input, 1) : 1;
    vx_uint32  depth                      = (dims > 2) ? TENSOR_VIEW_SIZE_INDEX(input, 2) : 1;
    vx_uint32  batch                      = (dims > 3) ? TENSOR_VIEW_SIZE_INDEX(input, 3) : 1;
    vx_uint32  sizes[4]                   = {0, 0, 0, 0};
    vx_tensor  src                        = NULL;
    vx_tensor  dst                        = NULL;
    vxnne_l2normalize_layer l2normalizeLayer = (vxnne_l2normalize_layer)ops_layer;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vx_uint32 dim                  = 4;
    vx_uint32 tmp_sizes[4]         = {1, 1, 1, 1};
    vx_tensor_create_params_t      tensor_create_params;
    vx_tensor sumTmp = VX_NULL;

    switch(dims)
    {
    case 1:
        depth   = TENSOR_VIEW_SIZE_INDEX(input, 0);
        batch   = 1;
        width   = 1;
        height  = 1;
        dims    = 3;
        break;
    case 2:
        depth   = TENSOR_VIEW_SIZE_INDEX(input, 0);
        batch   = TENSOR_VIEW_SIZE_INDEX(input, 1);
        width   = 1;
        height  = 1;
        dims    = 4;
        break;
    case 3:
        width   = TENSOR_VIEW_SIZE_INDEX(input, 0);
        height  = TENSOR_VIEW_SIZE_INDEX(input, 1);
        depth   = TENSOR_VIEW_SIZE_INDEX(input, 2);
        batch   = 1;
        dims    = 3;
        break;
    case 4:
        width   = TENSOR_VIEW_SIZE_INDEX(input, 0);
        height  = TENSOR_VIEW_SIZE_INDEX(input, 1);
        depth   = TENSOR_VIEW_SIZE_INDEX(input, 2);
        batch   = TENSOR_VIEW_SIZE_INDEX(input, 3);
        break;
    default:
        vxError("Input tensor error dimension[%u]\n", dims);
    }

    sizes[0] = width;
    sizes[1] = height;
    sizes[2] = depth;
    sizes[3] = batch;

    src      = vxoTensor_ReshapeTensor(input, (vx_int32*)sizes, dims);
    dst      = vxoTensor_ReshapeTensor(output, (vx_int32*)sizes, dims);
    l2normalizeLayer->base.temp_tensors[0] = src;
    l2normalizeLayer->base.temp_tensors[1] = dst;
    l2normalizeLayer->base.num_temp_tensors = 2;
    vxoLayer_InitializeHead(ops_layer, parameters, _num, reg_param);

    if(evis)
    {
        tmp_sizes[0] = gcmALIGN(width * height, 16);
        tmp_sizes[3] = batch;
    }
    else
    {
        tmp_sizes[0] = width;
        tmp_sizes[1] = height;
        tmp_sizes[3] = batch;
    }

    gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
    tensor_create_params.num_of_dims = dim;
    tensor_create_params.sizes = tmp_sizes;
    tensor_create_params.data_format = VX_TYPE_FLOAT32;
    tensor_create_params.quant_format = 0;

    sumTmp = vxoTensor_CreateTensor(ops_layer->node->base.context, ops_layer->node->graph, &tensor_create_params, vx_true_e);
    if (sumTmp == VX_NULL)
    {
        vxError("vxoTensor_CreateTensor fail at function %s, line %d", __FUNCTION__, __LINE__);
        status = VX_ERROR_NO_MEMORY;
        goto OnError;
    }
    l2normalizeLayer->base.temp_tensors[2] = sumTmp;
    l2normalizeLayer->base.num_temp_tensors = 3;
    //node 1
    if(evis)
    {
        shaderExecutable = vxnneL2NormSumSqrtShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_L2NORM_SUMSQRT, &ops_layer->node->kernelAttributes.borderMode, src, sumTmp);
    }
    else
    {
        shaderExecutable = vxnneGPUL2NormSumSqrtShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_GPU_L2NORM_SUMSQRT, &ops_layer->node->kernelAttributes.borderMode, src, sumTmp);
    }

    if (!shaderExecutable)
    {
        status = VX_FAILURE;
        goto OnError;
    }
    vxmONERROR(vxnneShaderOperation_Initialize(&l2normalizeLayer->l2normalize_SumSqrt_sh_operation,
                                    &l2normalizeLayer->base,
                                    VXNNE_OPERATOR_L2NORMALIZE_SUMSQRT,
                                    batch,
                                    shaderExecutable));

    vxmONERROR(vxnneOperation_AddReference(&l2normalizeLayer->l2normalize_SumSqrt_sh_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&l2normalizeLayer->l2normalize_SumSqrt_sh_operation.base, (vx_reference)sumTmp, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    //node 2
    if(evis)
    {
        shaderExecutable = vxnneL2NormSumScaleShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_L2NORM_SUMSCALE, &ops_layer->node->kernelAttributes.borderMode, src, sumTmp, dst);
    }
    else
    {
        shaderExecutable = vxnneGPUL2NormSumScaleShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_GPU_L2NORM_SUMSCALE, &ops_layer->node->kernelAttributes.borderMode, src, sumTmp, dst);
    }

    if (!shaderExecutable)
    {
        status = VX_FAILURE;
        goto OnError;
    }
    vxmONERROR(vxnneShaderOperation_Initialize(&l2normalizeLayer->l2normalize_sumScale_sh_operation,
                                    &l2normalizeLayer->base,
                                    VXNNE_OPERATOR_L2NORMALIZE_SUMSCALE,
                                    batch,
                                    shaderExecutable));

    vxmONERROR(vxnneOperation_AddReference(&l2normalizeLayer->l2normalize_sumScale_sh_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&l2normalizeLayer->l2normalize_sumScale_sh_operation.base, (vx_reference)sumTmp, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&l2normalizeLayer->l2normalize_sumScale_sh_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    vxmONERROR(vxnneLayer_SetOperation(
        &l2normalizeLayer->base,
        &l2normalizeLayer->l2normalize_SumSqrt_sh_operation.base,
        0));
    vxmONERROR(vxnneLayer_SetOperation(
        &l2normalizeLayer->base,
        &l2normalizeLayer->l2normalize_sumScale_sh_operation.base,
        1));

OnError:

    vxoLayer_InitializeFoot(ops_layer, parameters, _num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNL2NormalizeLayer_SH_EVIS_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && node->base.context->evisNoInst.supportEVIS;

    if (!support)return support;

    support = support && vxoNNL2NormalizeLayer_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_true_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNL2NormalizeLayer_SH_EVIS_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    status = vxoNNL2NormalizeLayer_SH_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_true_e);

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNL2NormalizeLayer_SH_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    support = support && vxoNNL2NormalizeLayer_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_false_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNL2NormalizeLayer_SH_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    status = vxoNNL2NormalizeLayer_SH_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_false_e);

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_status vxoNNLayer_GetOperations(vxnne_layer ops_layer, vx_uint32_ptr max_num_operations, vxnne_operation **operations)
{
    vx_status  status = VX_SUCCESS;

    vxnne_l2normalize_layer l2normalizeLayer = (vxnne_l2normalize_layer)ops_layer;

    *max_num_operations = gcmCOUNTOF(l2normalizeLayer->operations);

    *operations = l2normalizeLayer->operations;

    return status;
}

#endif

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNL2NormalizeLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
#if REGISTER_FRAME
    vxnne_layer_imp_s registerL2NormalizeLayer[] = {/* Please DON'T adjust the order, it's importent */
        { "L2NormalizeLayer NN", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "L2NormalizeLayer TP", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "L2NormalizeLayer SH EVIS", vxoNNL2NormalizeLayer_SH_EVIS_Support, vxoNNL2NormalizeLayer_SH_EVIS_Initialize, VX_NULL },
        { "L2NormalizeLayer SH F32", vxoNNL2NormalizeLayer_SH_Support, vxoNNL2NormalizeLayer_SH_Initialize, VX_NULL },
        { "L2NormalizeLayer SW ", vxoNNCommon_Support, vxoNNL2NormalizeLayer_SW_Initialize, VX_NULL },
    };

    REGISTER_LAYERS(registerL2NormalizeLayer, vxnne_l2normalize_layer_s, "L2NormalizeLayer", vxoNNLayer_GetOperations);

OnError:
#else

    vx_tensor  input                      = (vx_tensor)parameters[0];
    vx_tensor  output                     = (vx_tensor)parameters[1];
    vx_bool    enableShader;
    vx_type_e  inputFormat                = (vx_type_e)TENSOR_DATA_TYPE(input);
    vx_type_e  outputFormat               = (vx_type_e)TENSOR_DATA_TYPE(output);
    vx_uint32  dims                       = TENSOR_VIEW_DIM_NUM(input);
    vx_uint32  width                      = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32  height                     = (dims > 1) ? TENSOR_VIEW_SIZE_INDEX(input, 1) : 1;
    vx_uint32  depth                      = (dims > 2) ? TENSOR_VIEW_SIZE_INDEX(input, 2) : 1;
    vx_uint32  batch                      = (dims > 3) ? TENSOR_VIEW_SIZE_INDEX(input, 3) : 1;
    vx_uint32  sizes[4]                   = {0, 0, 0, 0};
    vx_tensor  src                        = NULL;
    vx_tensor  dst                        = NULL;

    vxnne_l2normalize_layer l2normalizeLayer = VX_NULL;
    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_l2normalize_layer_s), (gctPOINTER*)&l2normalizeLayer);
    if (!l2normalizeLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    switch(dims)
    {
    case 1:
        depth   = TENSOR_VIEW_SIZE_INDEX(input, 0);
        batch   = 1;
        width   = 1;
        height  = 1;
        dims    = 3;
        break;
    case 2:
        depth   = TENSOR_VIEW_SIZE_INDEX(input, 0);
        batch   = TENSOR_VIEW_SIZE_INDEX(input, 1);
        width   = 1;
        height  = 1;
        dims    = 4;
        break;
    case 3:
        width   = TENSOR_VIEW_SIZE_INDEX(input, 0);
        height  = TENSOR_VIEW_SIZE_INDEX(input, 1);
        depth   = TENSOR_VIEW_SIZE_INDEX(input, 2);
        batch   = 1;
        dims    = 3;
        break;
    case 4:
        width   = TENSOR_VIEW_SIZE_INDEX(input, 0);
        height  = TENSOR_VIEW_SIZE_INDEX(input, 1);
        depth   = TENSOR_VIEW_SIZE_INDEX(input, 2);
        batch   = TENSOR_VIEW_SIZE_INDEX(input, 3);
        break;
    default:
        vxError("Input tensor error dimension[%u]\n", dims);
    }

    sizes[0] = width;
    sizes[1] = height;
    sizes[2] = depth;
    sizes[3] = batch;

    src      = vxoTensor_ReshapeTensor(input, (vx_int32*)sizes, dims);
    dst      = vxoTensor_ReshapeTensor(output, (vx_int32*)sizes, dims);

    gcoOS_ZeroMemory(l2normalizeLayer, sizeof(vxnne_l2normalize_layer_s));

    vxnneLayer_Initialize(&l2normalizeLayer->base,
                          "L2NormalizeLayer",
                          node,
                          vxmOPERATION_COUNT(l2normalizeLayer),
                          l2normalizeLayer->operations,
                          VX_NULL);

    if(node->base.context->evisNoInst.supportEVIS)
    {
        enableShader = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
                          || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT8)
                          || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT16)
                          || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_UINT8)
                          || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8)
                          || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_FLOAT16)
                          || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16)
                          || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_FLOAT16)
                          || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
                          || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT16)
                          || (inputFormat == VX_TYPE_BFLOAT16 && outputFormat == VX_TYPE_BFLOAT16));
        enableShader  = enableShader && (width * height < IMG_MAX_WIDTH);
    }
    else
        enableShader = vx_true_e;

    if (enableShader && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
    {
        vxnne_shader_executable shaderExecutable = VX_NULL;
        vx_uint32 dim                  = 4;
        vx_uint32 tmp_sizes[4]         = {1, 1, 1, 1};
        vx_tensor_create_params_t      tensor_create_params;
        vx_tensor sumTmp = VX_NULL;

        if(node->base.context->evisNoInst.supportEVIS)
        {
            tmp_sizes[0] = gcmALIGN(width * height, 16);
            tmp_sizes[3] = batch;
        }
        else
        {
            tmp_sizes[0] = width;
            tmp_sizes[1] = height;
            tmp_sizes[3] = batch;
        }

        gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
        tensor_create_params.num_of_dims = dim;
        tensor_create_params.sizes = tmp_sizes;
        tensor_create_params.data_format = VX_TYPE_FLOAT32;
        tensor_create_params.quant_format = 0;

        sumTmp = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);
        if (sumTmp == VX_NULL)
        {
            vxError("vxoTensor_CreateTensor fail at function %s, line %d", __FUNCTION__, __LINE__);
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }

        //node 1
        if(node->base.context->evisNoInst.supportEVIS)
        {
            shaderExecutable = vxnneL2NormSumSqrtShaderExecutable(node->base.context, VXNNE_KERNEL_L2NORM_SUMSQRT, &node->kernelAttributes.borderMode, src, sumTmp);
        }
        else
        {
            shaderExecutable = vxnneGPUL2NormSumSqrtShaderExecutable(node->base.context, VXNNE_KERNEL_GPU_L2NORM_SUMSQRT, &node->kernelAttributes.borderMode, src, sumTmp);
        }

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }
        status = vxnneShaderOperation_Initialize(&l2normalizeLayer->l2normalize_SumSqrt_sh_operation,
                                        &l2normalizeLayer->base,
                                        VXNNE_OPERATOR_L2NORMALIZE_SUMSQRT,
                                        batch,
                                        shaderExecutable);
        if (status != VX_SUCCESS)
            goto exit;

        vxnneOperation_AddReference(&l2normalizeLayer->l2normalize_SumSqrt_sh_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&l2normalizeLayer->l2normalize_SumSqrt_sh_operation.base, (vx_reference)sumTmp, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        //node 2
        if(node->base.context->evisNoInst.supportEVIS)
        {
            shaderExecutable = vxnneL2NormSumScaleShaderExecutable(node->base.context, VXNNE_KERNEL_L2NORM_SUMSCALE, &node->kernelAttributes.borderMode, src, sumTmp, dst);
        }
        else
        {
            shaderExecutable = vxnneGPUL2NormSumScaleShaderExecutable(node->base.context, VXNNE_KERNEL_GPU_L2NORM_SUMSCALE, &node->kernelAttributes.borderMode, src, sumTmp, dst);
        }

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }
        status = vxnneShaderOperation_Initialize(&l2normalizeLayer->l2normalize_sumScale_sh_operation,
                                        &l2normalizeLayer->base,
                                        VXNNE_OPERATOR_L2NORMALIZE_SUMSCALE,
                                        batch,
                                        shaderExecutable);
        if (status != VX_SUCCESS)
            goto exit;

        vxnneOperation_AddReference(&l2normalizeLayer->l2normalize_sumScale_sh_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&l2normalizeLayer->l2normalize_sumScale_sh_operation.base, (vx_reference)sumTmp, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&l2normalizeLayer->l2normalize_sumScale_sh_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        vxnneLayer_SetOperation(
            &l2normalizeLayer->base,
            &l2normalizeLayer->l2normalize_SumSqrt_sh_operation.base,
            0);
        vxnneLayer_SetOperation(
            &l2normalizeLayer->base,
            &l2normalizeLayer->l2normalize_sumScale_sh_operation.base,
            1);

        l2normalizeLayer->base.temp_tensors[0] = sumTmp;
        l2normalizeLayer->base.temp_tensors[1] = src;
        l2normalizeLayer->base.temp_tensors[2] = dst;
        l2normalizeLayer->base.num_temp_tensors = 3;

        node->layer = &l2normalizeLayer->base;
    }
    else
    {
        vxnneOperation_Initialize(&l2normalizeLayer->l2normalize_sw_operation.base,
                                  &l2normalizeLayer->base,
                                  VXNNE_OPERATION_TARGET_SW,
                                  VXNNE_OPERATOR_L2NORMALIZE,
                                  vxnneExecuteSWL2Normalize,
                                  VX_NULL,
                                  batch,
                                  0);

        vxnneLayer_SetOperation(
            &l2normalizeLayer->base,
            &l2normalizeLayer->l2normalize_sw_operation.base,
            0);

        l2normalizeLayer->l2normalize_sw_operation.inputs           = input;
        l2normalizeLayer->l2normalize_sw_operation.outputs          = output;

        vxnneOperation_AddReference(&l2normalizeLayer->l2normalize_sw_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&l2normalizeLayer->l2normalize_sw_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        node->layer = &l2normalizeLayer->base;
    }

    return status;

exit:
    if (src) vxoTensor_ReleaseTensor(&src);
    if (dst) vxoTensor_ReleaseTensor(&dst);
    if (l2normalizeLayer)
        gcoOS_Free(NULL, (gctPOINTER)l2normalizeLayer);
#endif

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNL2NormalizeLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

