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
#include <layers/gc_vx_layer_activation.h>

extern vx_tensor vxnneAllocateTPLUTBuffer(vx_context context, vx_node node);

vx_status vxnneExecuteSWActivation(struct _vxnne_operation_s *operation)
{
    vxnne_activation_sw_operation           activationOperation   = (vxnne_activation_sw_operation)operation;

    vx_tensor inputs  = (vx_tensor)activationOperation->inputs;
    vx_scalar func = (vx_scalar)activationOperation->func;
    vx_scalar a  = (vx_scalar)activationOperation->a;
    vx_scalar b = (vx_scalar)activationOperation->b;
    vx_tensor outputs = (vx_tensor)activationOperation->outputs;

    vx_enum   func_v = func->value->e;

    vx_uint32 elementCount = 0;
    vx_uint32 i;
    vx_float32 value = 0.0f, result = 0.0f;
    gctPOINTER inputBase;
    gctPOINTER outputBase;
    vx_enum   a_type     = vxoScalar_GetDataType(a);
    vx_enum   b_type     = vxoScalar_GetDataType(b);

    vx_float32 a_v = a_type == VX_TYPE_FLOAT32 ? a->value->f32 : (vx_float32)a->value->n32;
    vx_float32 b_v = b_type == VX_TYPE_FLOAT32 ? b->value->f32 : (vx_float32)b->value->n32;


    vx_status status = VX_SUCCESS;

    elementCount = (vx_uint32)vxoMemory_ComputeElementCount(&inputs->tensorBuffer->memory, 0);
    vxoTensor_GetTensorViewMemory(inputs, &inputBase, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, &outputBase, VX_NULL);

    for (i = 0; i < elementCount; i++)
    {
        value = VX_GET_DATA_FROM_TENSOR(inputs, i);

        switch (func_v)
        {
        case VX_NN_ACTIVATION_LOGISTIC:
            {
                result = 1.0f / (1 + gcoMATH_Exp(value * (-1)));
            }
            break;

        case VX_NN_ACTIVATION_HYPERBOLIC_TAN:
            {
                vx_float32 av = (vxoScalar_GetDataType(a) == VX_TYPE_FLOAT32) ? a->value->f32 : (vx_float32)a->value->n32;
                vx_float32 bv = (vxoScalar_GetDataType(b) == VX_TYPE_FLOAT32) ? b->value->f32 : (vx_float32)b->value->n32;
                result = av * gcoMATH_TangentH(bv * value);
            }
            break;

        case VX_NN_ACTIVATION_RELU:
            {
                result = gcoMATH_MAX(0.0f, value);
            }
            break;

        case VX_NN_ACTIVATION_BRELU:
            {
                result = gcoMATH_MIN(a_v, gcoMATH_MAX(0.0f, value));
            }
            break;

        case VX_NN_ACTIVATION_SOFTRELU:
            {
                result = gcoMATH_Log(1 + gcoMATH_Exp(value));
            }
            break;

        case VX_NN_ACTIVATION_ABS:
            {
                result = gcoMATH_Absolute(value);
            }
            break;

        case VX_NN_ACTIVATION_SQUARE:
            {
                result = gcoMATH_Power(value, 2);
            }
            break;

        case VX_NN_ACTIVATION_SQRT:
            {
                result = gcoMATH_SquareRoot(value);
            }
            break;

        case VX_NN_ACTIVATION_LINEAR:
            {
                result = a_v * value + b_v;
            }
            break;

        case VX_NN_ACTIVATION_LEAKYRELU:
            {
                result = (value > 0.0f) ? value : 0.1f * value;
            }
            break;

        case VX_NN_ACTIVATION_RELU6:
            {
                result = gcoMATH_MIN(gcoMATH_MAX(value, 0), 6);
            }
            break;

        case VX_NN_ACTIVATION_RELU1:
            {
                result = gcoMATH_MIN(gcoMATH_MAX(value, -1), 1);
            }
            break;

        case VX_NN_ACTIVATION_RSQRT:
            {
                result = gcoMATH_ReciprocalSquareRoot(value);
            }
            break;

        default:
            vxError("this activation func not support");
            status = VX_ERROR_NOT_SUPPORTED;
            return status;
        }

        VX_SAVE_DATA_TO_TENSOR(outputs, result, i);
    }

    return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNActivationLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNActivationLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNActivationLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}
#if REGISTER_FRAME


VX_PRIVATE_API vx_status vxoNNActivationLayer_SW_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vx_tensor  inputs = (vx_tensor)parameters[0];
    vx_scalar  func_s = (vx_scalar)parameters[1];
    vx_scalar  a_s = (vx_scalar)parameters[2];
    vx_scalar  b_s = (vx_scalar)parameters[3];
    vx_tensor  outputs = (vx_tensor)parameters[4];
    vx_uint32 batchCount = (TENSOR_DIM_NUM(inputs) > 3) ? TENSOR_SIZE_INDEX(inputs, 3) : 1;
    vxnne_activation_layer  activationLayer = (vxnne_activation_layer)ops_layer;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxnneOperation_Initialize(&activationLayer->activation_operation.base,
        &activationLayer->base,
        VXNNE_OPERATION_TARGET_SW,
        VXNNE_OPERATOR_ACTIVATION,
        vxnneExecuteSWActivation,
        VX_NULL,
        batchCount,
        0));

    vxmONERROR(vxnneLayer_SetOperation(
        &activationLayer->base,
        &activationLayer->activation_operation.base,
        0));

    activationLayer->activation_operation.inputs = inputs;
    activationLayer->activation_operation.func = func_s;
    activationLayer->activation_operation.a = a_s;
    activationLayer->activation_operation.b = b_s;
    activationLayer->activation_operation.outputs = outputs;

    vxmONERROR(vxnneOperation_AddReference(&activationLayer->activation_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&activationLayer->activation_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNActivationLayer_SH_EVIS_Support_Ext(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_scalar  func_s                     = (vx_scalar)parameters[1];
    vx_tensor  outputs                    = (vx_tensor)parameters[2];
    vx_enum   inputFormat                 = TENSOR_DATA_TYPE(inputs);
    vx_enum   outputFormat                = TENSOR_DATA_TYPE(outputs);
    vx_enum   func_v                      = func_s->value->e;
    vx_bool   support_dataType[4]         = {vx_false_e, vx_false_e, vx_false_e};
    vx_bool   enable_tf_quantize          = vx_false_e;
    vx_bool   enable_tensorABS_SHExe      = vx_false_e;
    vx_bool   enable_tensorTR_SHExe       = vx_false_e;
    vx_bool   enable_Leaky_SHExe          = vx_false_e;
    vxoLayer_VerificationHead(node, parameters, num, reg_param);
    reg_param->flag = 0;
    if (!support)return support;

    if(evis)
    {
        support_dataType[0] = (vx_bool)(((inputFormat == VX_TYPE_INT8 || inputFormat == VX_TYPE_FLOAT16) && (outputFormat == VX_TYPE_INT8 || outputFormat == VX_TYPE_FLOAT16)) || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16));
        support_dataType[1] = (vx_bool)((inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8) || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16));
        support_dataType[2] = (vx_bool)((inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8));

        support_dataType[3] = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
                                        || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT8)
                                        || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT16)
                                        || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_UINT8)
                                        || (inputFormat == VX_TYPE_BFLOAT16 && outputFormat == VX_TYPE_BFLOAT16)
                                        || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8)
                                        || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_FLOAT16)
                                        || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16)
                                        || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_FLOAT16)
                                        || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
                                        || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT16));

        enable_tensorABS_SHExe = (vx_bool)(support_dataType[3] && func_v == VX_NN_ACTIVATION_ABS);

        enable_Leaky_SHExe = (vx_bool)(((inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
                                      || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT16)
                                      || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8)
                                      || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_FLOAT16)
                                      || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16)
                                      || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_FLOAT16)
                                      || (inputFormat == VX_TYPE_FLOAT16 && outputFormat != VX_TYPE_FLOAT32))
                                     && func_v == VX_NN_ACTIVATION_LEAKYRELU);
    }
    else
    {
        support_dataType[0] = (vx_bool)((inputFormat == VX_TYPE_FLOAT32 || inputFormat == VX_TYPE_FLOAT16) && (outputFormat == VX_TYPE_FLOAT32 || outputFormat == VX_TYPE_FLOAT16));
        support_dataType[1] = (vx_bool)((inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32) || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16));
        support_dataType[2] = (vx_bool)((inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8));

        support_dataType[3] = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 || inputFormat == VX_TYPE_FLOAT32 || inputFormat == VX_TYPE_UINT8)
                                  && (outputFormat == VX_TYPE_FLOAT16 || outputFormat == VX_TYPE_FLOAT32 || outputFormat == VX_TYPE_UINT8));

        enable_Leaky_SHExe  = (vx_bool)(support_dataType[3] && func_v == VX_NN_ACTIVATION_LEAKYRELU);
    }

    enable_tensorTR_SHExe  = (vx_bool)(support_dataType[3] &&
                                       (func_v == VX_NN_ACTIVATION_SQRT  ||
                                        func_v == VX_NN_ACTIVATION_RSQRT ||
                                        /*func_v == VX_NN_ACTIVATION_EXP ||*/
                                        /*func_v == VX_NN_ACTIVATION_LOG ||*/
                                        /*func_v == VX_NN_ACTIVATION_SIN ||*/
                                        func_v == VX_NN_ACTIVATION_SOFTRELU ||
                                        func_v == VX_NN_ACTIVATION_LOGISTIC ||
                                        func_v == VX_NN_ACTIVATION_SQUARE ||
                                        func_v == VX_NN_ACTIVATION_HYPERBOLIC_TAN));

    enable_tf_quantize = (vx_bool)((func_v == VX_NN_ACTIVATION_RELU && support_dataType[2]) ||
                                   (func_v == VX_NN_ACTIVATION_RELU1 && support_dataType[2]) ||
                                   (func_v == VX_NN_ACTIVATION_BRELU && support_dataType[2]) ||
                                   (func_v == VX_NN_ACTIVATION_RELU6 && support_dataType[2]));

    support = support && ((func_v == VX_NN_ACTIVATION_RELU && support_dataType[0]) ||
                           (func_v == VX_NN_ACTIVATION_RELU1 && support_dataType[1]) ||
                           (func_v == VX_NN_ACTIVATION_RELU6 && support_dataType[1]) ||
                           (func_v == VX_NN_ACTIVATION_LOGISTIC && (support_dataType[1] || support_dataType[2])) ||
                           (func_v == VX_NN_ACTIVATION_HYPERBOLIC_TAN && (support_dataType[1] || support_dataType[2])) ||
                           (func_v == VX_NN_ACTIVATION_RSQRT && (support_dataType[1] || support_dataType[2])) ||
                           (func_v == VX_NN_ACTIVATION_SQRT && (support_dataType[1] || support_dataType[2])) ||
                           (func_v == VX_NN_ACTIVATION_ABS && (support_dataType[1] || support_dataType[2])) ||
                           enable_tensorABS_SHExe ||
                           enable_tensorTR_SHExe  ||
                           enable_Leaky_SHExe ||
                           enable_tf_quantize);

    if (support)
    {
        SETBIT(reg_param->flag, (enable_tf_quantize == vx_true_e)?1:0, 0);
        SETBIT(reg_param->flag, (enable_tensorABS_SHExe == vx_true_e)?1:0, 1);
        SETBIT(reg_param->flag, (enable_tensorTR_SHExe == vx_true_e)?1:0, 2);
        SETBIT(reg_param->flag, (enable_Leaky_SHExe == vx_true_e)?1:0, 3);
    }

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_bool vxoNNActivationLayer_SH_EVIS_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && node->base.context->evisNoInst.supportEVIS;

    if (!support)return support;


    support = support && vxoNNActivationLayer_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_true_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNActivationLayer_SH_EVIS_Initialize_Ext(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_status status = VX_SUCCESS;

    vx_context context                    = vxGetContext((vx_reference)ops_layer->node);

    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_scalar  func_s                     = (vx_scalar)parameters[1];
    vx_scalar  a_s                        = (vx_scalar)parameters[2];
    vx_scalar  b_s                        = (vx_scalar)parameters[3];
    vx_tensor  outputs                    = (vx_tensor)parameters[4];
    vx_uint32 batchCount                  = (TENSOR_DIM_NUM(inputs) > 3) ? TENSOR_SIZE_INDEX(inputs, 3) : 1;
    vx_enum   func_v                      = func_s->value->e;
    vx_bool   enable_tf_quantize          = (vx_bool)GETBIT(reg_param->flag, 0);
    vx_bool   enable_tensorABS_SHExe      = (vx_bool)GETBIT(reg_param->flag, 1);
    vx_bool   enable_tensorTR_SHExe       = (vx_bool)GETBIT(reg_param->flag, 2);
    vx_bool   enable_Leaky_SHExe          = (vx_bool)GETBIT(reg_param->flag, 3);
    vxnne_activation_layer  activationLayer = (vxnne_activation_layer)ops_layer;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vx_uint32  reshpTensor_Sizes[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {1};
    vx_uint32  reshpTensor_Dims = 2;
    vx_tensor input      = NULL;
    vx_tensor output     = NULL;
    vx_enum   a_type     = vxoScalar_GetDataType(a_s);
    vx_enum   b_type     = vxoScalar_GetDataType(b_s);

    vx_float32 minVal = a_type == VX_TYPE_FLOAT32 ? a_s->value->f32 : (vx_float32)a_s->value->n32;
    vx_float32 maxVal = b_type == VX_TYPE_FLOAT32 ? b_s->value->f32 : (vx_float32)b_s->value->n32;

    vx_float32 val = 0.1f;
    vx_scalar negative_slopes = (enable_Leaky_SHExe == vx_true_e)?vxCreateScalar(context, VX_TYPE_FLOAT32, &val):VX_NULL;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxoElementOptimization_GetTensorShape(inputs, reshpTensor_Sizes, &reshpTensor_Dims);

    if (!enable_Leaky_SHExe)
    {
        input     = vxoTensor_ReshapeTensor(inputs, (vx_int32*)reshpTensor_Sizes, reshpTensor_Dims);
        output     = vxoTensor_ReshapeTensor(outputs, (vx_int32*)reshpTensor_Sizes, reshpTensor_Dims);

        activationLayer->base.temp_tensors[0] = input;
        activationLayer->base.temp_tensors[1] = output;
        activationLayer->base.num_temp_tensors = 2;

        if (func_v == VX_NN_ACTIVATION_RELU1)
        {
            minVal = -1;
            maxVal = 1;
        }
        else if (func_v == VX_NN_ACTIVATION_RELU6)
        {
            minVal = 0;
            maxVal = 6;
        }
        else if (func_v == VX_NN_ACTIVATION_RELU)
        {
            minVal = 0;
            maxVal = 32767;
        }
        else if (func_v == VX_NN_ACTIVATION_BRELU)
        {
            maxVal = minVal;
            minVal = 0;
        }

        batchCount = reshpTensor_Dims > 3 ? reshpTensor_Sizes[3] : 1;
    }
    else
    {
        input = inputs;
        output = outputs;
    }

    if(evis)
    {
        if (enable_tensorABS_SHExe)
            shaderExecutable = vxnneGetTensorAbsShaderExecutable(context, VXNNE_KERNEL_TENSOR_ABS, &ops_layer->node->kernelAttributes.borderMode, input, output);
        else if (enable_tensorTR_SHExe)
            shaderExecutable = vxnneGetTensorTRShaderExecutable(context, VXNNE_KERNEL_TENSOR_TRANSCENDENTAL, &ops_layer->node->kernelAttributes.borderMode, input, minVal, maxVal, func_v, output);
        else if (enable_tf_quantize)
            shaderExecutable = vxnneGetActivation_UInt8ShaderExecutable(context, VXNNE_KERNEL_ACTIVATION_UINT8, &ops_layer->node->kernelAttributes.borderMode, func_v, input, minVal, maxVal, output);
        else if (enable_Leaky_SHExe)
            shaderExecutable = vxnneGetLeakyReluShaderExecutable(context, VXNNE_KERNEL_NN_LEAKY, &ops_layer->node->kernelAttributes.borderMode, inputs, negative_slopes, output);
        else
            shaderExecutable = vxnneGetActivationShaderExecutable(context, VXNNE_KERNEL_ACTIVATION, &ops_layer->node->kernelAttributes.borderMode, func_v, input, minVal, maxVal, output);
    }
    else
    {
        if (enable_tensorTR_SHExe)
            shaderExecutable = vxnneGetGPUTensorTRShaderExecutable(context, VXNNE_KERNEL_TENSOR_TRANSCENDENTAL, &ops_layer->node->kernelAttributes.borderMode, input, minVal, maxVal, func_v, output);
        else if (enable_Leaky_SHExe)
            shaderExecutable = vxnneGetGPULeakyReluShaderExecutable(context, VXNNE_KERNEL_NN_LEAKY, &ops_layer->node->kernelAttributes.borderMode, inputs, negative_slopes, outputs);
        else
            shaderExecutable = vxnneGetGPUActivationShaderExecutable(context, VXNNE_KERNEL_ACTIVATION, &ops_layer->node->kernelAttributes.borderMode, func_v, input, minVal, maxVal, output);
    }

    if (!shaderExecutable)
    {
        status = VX_FAILURE;
        goto OnError;
    }

    vxmONERROR(vxnneShaderOperation_Initialize(&activationLayer->activation_SHoperation,
        &activationLayer->base,
        VXNNE_OPERATOR_ACTIVATION,
        batchCount,
        shaderExecutable));

    vxmONERROR(vxnneOperation_AddReference(&activationLayer->activation_SHoperation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&activationLayer->activation_SHoperation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    vxmONERROR(vxnneLayer_SetOperation(
        &activationLayer->base,
        &activationLayer->activation_SHoperation.base,
        0));

    if (enable_Leaky_SHExe == vx_true_e)
        vxmONERROR(vxReleaseScalar(&negative_slopes));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status vxoNNActivationLayer_SH_EVIS_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxoNNActivationLayer_SH_EVIS_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_true_e));
OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status vxoNNActivationLayer_SH_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxoNNActivationLayer_SH_EVIS_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_false_e));
OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_bool vxoNNActivationLayer_SH_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);


    support = support && vxoNNActivationLayer_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_false_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_bool vxoNNActivationLayer_TP_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_scalar  func_s                     = (vx_scalar)parameters[1];
    vx_tensor  outputs                    = (vx_tensor)parameters[4];
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_TP, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    support =  support && (vxnneIsTPSupportFormat(node->base.context, inputs, VX_NULL, outputs) &&
                        TENSOR_VIEW_SIZE_INDEX(inputs, 3) == TENSOR_VIEW_SIZE_INDEX(outputs, 3) &&
                        (TENSOR_VIEW_SIZE_INDEX(outputs, 0) * TENSOR_VIEW_SIZE_INDEX(outputs, 1) * TENSOR_VIEW_SIZE_INDEX(outputs, 2) > 1) &&
                        (func_s->value->e == VX_NN_ACTIVATION_LOGISTIC ||
                         func_s->value->e == VX_NN_ACTIVATION_RELU  ||
                         func_s->value->e == VX_NN_ACTIVATION_RELU1 ||
                         func_s->value->e == VX_NN_ACTIVATION_RELU6 ||
                         func_s->value->e == VX_NN_ACTIVATION_LEAKYRELU ||
                         func_s->value->e == VX_NN_ACTIVATION_HYPERBOLIC_TAN));

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNActivationLayer_TP_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vxnne_activation_layer  activationLayer = (vxnne_activation_layer)ops_layer;
    vx_context context = vxGetContext((vx_reference)ops_layer->node);
    vx_tensor  inputs = (vx_tensor)parameters[0];
    vx_scalar  func_s = (vx_scalar)parameters[1];
    vx_scalar  a_s = (vx_scalar)parameters[2];
    vx_scalar  b_s = (vx_scalar)parameters[3];
    vx_tensor  outputs = (vx_tensor)parameters[4];
    vx_uint32 batchCount = (TENSOR_DIM_NUM(inputs) > 3) ? TENSOR_SIZE_INDEX(inputs, 3) : 1;
    vx_op_param_s conv = { 0 };

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxnneOperation_Initialize(&activationLayer->activation_tp_operation.base,
        &activationLayer->base,
        VXNNE_OPERATION_TARGET_TP,
        VXNNE_OPERATOR_ACTIVATION,
        VX_NULL,
        vxnneOperation_TP_Deinitialize,
        batchCount,
        0));

    conv.data_buff = vxnneAllocateTPLUTBuffer(context, ops_layer->node);
    if (conv.data_buff == VX_NULL)
    {
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
    conv.tpType = TP_ACTIVATION;
    conv.other_ref = gcvNULL;
    conv.tp_value = (vx_tp_value_cmd_s*)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
    conv.tp_value->e32[0] = func_s->value->e;
    conv.tp_value->f32[0] = (vxoScalar_GetDataType(a_s) == VX_TYPE_FLOAT32) ? a_s->value->f32 : (vx_float32)a_s->value->n32;
    conv.tp_value->f32[1] = (vxoScalar_GetDataType(b_s) == VX_TYPE_FLOAT32) ? b_s->value->f32 : (vx_float32)b_s->value->n32;

    vxMemCopy(&activationLayer->activation_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));

    vxmONERROR(vxnneLayer_SetOperation(
        &activationLayer->base,
        &activationLayer->activation_tp_operation.base,
        0));

    activationLayer->activation_tp_operation.input = inputs;
    activationLayer->activation_tp_operation.output = outputs;

    vxmONERROR(vxnneOperation_AddReference(&activationLayer->activation_tp_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&activationLayer->activation_tp_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_status vxoNNLayer_GetActivationOperations(vxnne_layer ops_layer, vx_uint32_ptr max_num_operations, vxnne_operation **operations)
{
    vx_status  status = VX_SUCCESS;
    vxnne_activation_layer  activationLayer = (vxnne_activation_layer)ops_layer;

    *max_num_operations = gcmCOUNTOF(activationLayer->operations);

    *operations = activationLayer->operations;

    return status;
}
#endif


VX_PRIVATE_API vx_status VX_CALLBACK vxoNNActivationLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status  status                     = VX_SUCCESS;
#if REGISTER_FRAME

    vxnne_layer_imp_s registerActivationLayers[] = {/* Please DON'T adjust the order, it's importent */
        { "ActivationLayer NN", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "ActivationLayer TP", vxoNNActivationLayer_TP_Support, vxoNNActivationLayer_TP_Initialize, vxoNNCommon_Deinitialize },
        { "ActivationLayer SH EVIS", vxoNNActivationLayer_SH_EVIS_Support, vxoNNActivationLayer_SH_EVIS_Initialize, VX_NULL },
        { "ActivationLayer SH F32", vxoNNActivationLayer_SH_Support, vxoNNActivationLayer_SH_Initialize, VX_NULL },
        { "ActivationLayer SW support", vxoNNCommon_Support, vxoNNActivationLayer_SW_Initialize, VX_NULL },
    };

    REGISTER_LAYERS(registerActivationLayers, vxnne_activation_layer_s, "ActivationLayer", vxoNNLayer_GetActivationOperations);

OnError:
#else
    vx_context context                    = vxGetContext((vx_reference)node);

    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_scalar  func_s                     = (vx_scalar)parameters[1];
    vx_scalar  a_s                        = (vx_scalar)parameters[2];
    vx_scalar  b_s                        = (vx_scalar)parameters[3];
    vx_tensor  outputs                    = (vx_tensor)parameters[4];
    vx_enum   inputFormat                 = TENSOR_DATA_TYPE(inputs);
    vx_enum   outputFormat                = TENSOR_DATA_TYPE(outputs);
    vx_uint32 batchCount                  = (TENSOR_DIM_NUM(inputs) > 3) ? TENSOR_SIZE_INDEX(inputs, 3) : 1;
    vx_enum   func_v                      = func_s->value->e;
    vx_bool   support_dataType[4]         = {vx_false_e, vx_false_e, vx_false_e};
    vx_bool   enable_tf_quantize          = vx_false_e;
    vx_bool   shExe_flag                  = vx_false_e;
    vx_bool   enable_tensorABS_SHExe      = vx_false_e;
    vx_bool   enable_tensorTR_SHExe       = vx_false_e;
    vx_bool   enable_Leaky_SHExe          = vx_false_e;
    vxnne_activation_layer  activationLayer = gcvNULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_activation_layer_s), (gctPOINTER*)&activationLayer);
    if (!activationLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(activationLayer, sizeof(vxnne_activation_layer_s));

    vxnneLayer_Initialize(&activationLayer->base,
                          "ActivationLayer",
                          node,
                          vxmOPERATION_COUNT(activationLayer),
                          activationLayer->operations,
                          vxnneLayer_Deinitialize);

    if(context->evisNoInst.supportEVIS)
    {
        support_dataType[0] = (vx_bool)(((inputFormat == VX_TYPE_INT8 || inputFormat == VX_TYPE_FLOAT16) && (outputFormat == VX_TYPE_INT8 || outputFormat == VX_TYPE_FLOAT16)) || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16));
        support_dataType[1] = (vx_bool)((inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8) || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16));
        support_dataType[2] = (vx_bool)((inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8));

        support_dataType[3] = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
                                        || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT8)
                                        || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT16)
                                        || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_UINT8)
                                        || (inputFormat == VX_TYPE_BFLOAT16 && outputFormat == VX_TYPE_BFLOAT16)
                                        || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8)
                                        || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_FLOAT16)
                                        || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16)
                                        || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_FLOAT16)
                                        || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
                                        || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT16));

        enable_tensorABS_SHExe = (vx_bool)(support_dataType[3] && func_v == VX_NN_ACTIVATION_ABS);

        enable_Leaky_SHExe = (vx_bool)(((inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
                                      || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT16)
                                      || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8)
                                      || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_FLOAT16)
                                      || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16)
                                      || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_FLOAT16)
                                      || (inputFormat == VX_TYPE_FLOAT16 && outputFormat != VX_TYPE_FLOAT32))
                                     && func_v == VX_NN_ACTIVATION_LEAKYRELU);
    }
    else
    {
        support_dataType[0] = (vx_bool)((inputFormat == VX_TYPE_FLOAT32 || inputFormat == VX_TYPE_FLOAT16) && (outputFormat == VX_TYPE_FLOAT32 || outputFormat == VX_TYPE_FLOAT16));
        support_dataType[1] = (vx_bool)((inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32) || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16));
        support_dataType[2] = (vx_bool)((inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8));

        support_dataType[3] = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 || inputFormat == VX_TYPE_FLOAT32 || inputFormat == VX_TYPE_UINT8)
                                  && (outputFormat == VX_TYPE_FLOAT16 || outputFormat == VX_TYPE_FLOAT32 || outputFormat == VX_TYPE_UINT8));

        enable_Leaky_SHExe  = (vx_bool)(support_dataType[3] && func_v == VX_NN_ACTIVATION_LEAKYRELU);
    }

    enable_tensorTR_SHExe  = (vx_bool)(support_dataType[3] &&
                                       (func_v == VX_NN_ACTIVATION_SQRT  ||
                                        func_v == VX_NN_ACTIVATION_RSQRT ||
                                        /*func_v == VX_NN_ACTIVATION_EXP ||*/
                                        /*func_v == VX_NN_ACTIVATION_LOG ||*/
                                        /*func_v == VX_NN_ACTIVATION_SIN ||*/
                                        func_v == VX_NN_ACTIVATION_SOFTRELU ||
                                        func_v == VX_NN_ACTIVATION_LOGISTIC ||
                                        func_v == VX_NN_ACTIVATION_SQUARE ||
                                        func_v == VX_NN_ACTIVATION_HYPERBOLIC_TAN));

    enable_tf_quantize = (vx_bool)((func_v == VX_NN_ACTIVATION_RELU && support_dataType[2]) ||
                                   (func_v == VX_NN_ACTIVATION_RELU1 && support_dataType[2]) ||
                                   (func_v == VX_NN_ACTIVATION_BRELU && support_dataType[2]) ||
                                   (func_v == VX_NN_ACTIVATION_RELU6 && support_dataType[2]));

    shExe_flag = (vx_bool)((func_v == VX_NN_ACTIVATION_RELU && support_dataType[0]) ||
                           (func_v == VX_NN_ACTIVATION_RELU1 && support_dataType[1]) ||
                           (func_v == VX_NN_ACTIVATION_RELU6 && support_dataType[1]) ||
                           (func_v == VX_NN_ACTIVATION_LOGISTIC && (support_dataType[1] || support_dataType[2])) ||
                           (func_v == VX_NN_ACTIVATION_HYPERBOLIC_TAN && (support_dataType[1] || support_dataType[2])) ||
                           (func_v == VX_NN_ACTIVATION_RSQRT && (support_dataType[1] || support_dataType[2])) ||
                           (func_v == VX_NN_ACTIVATION_SQRT && (support_dataType[1] || support_dataType[2])) ||
                           (func_v == VX_NN_ACTIVATION_ABS && (support_dataType[1] || support_dataType[2])) ||
                           enable_tensorABS_SHExe ||
                           enable_tensorTR_SHExe  ||
                           enable_tf_quantize);

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_ACTIVATION) &&
        vxnneIsTPSupportFormat(context, inputs, VX_NULL, outputs) &&
        TENSOR_VIEW_SIZE_INDEX(inputs, 3) == TENSOR_VIEW_SIZE_INDEX(outputs, 3) &&
        (TENSOR_VIEW_SIZE_INDEX(outputs, 0) * TENSOR_VIEW_SIZE_INDEX(outputs, 1) * TENSOR_VIEW_SIZE_INDEX(outputs, 2) > 1) &&
        (func_s->value->e == VX_NN_ACTIVATION_LOGISTIC ||
         func_s->value->e == VX_NN_ACTIVATION_RELU  ||
         func_s->value->e == VX_NN_ACTIVATION_RELU1 ||
         func_s->value->e == VX_NN_ACTIVATION_RELU6 ||
         func_s->value->e == VX_NN_ACTIVATION_LEAKYRELU ||
         func_s->value->e == VX_NN_ACTIVATION_HYPERBOLIC_TAN))
    {
        vx_op_param_s conv = {0};

        status = vxnneOperation_Initialize(&activationLayer->activation_tp_operation.base,
                                           &activationLayer->base,
                                           VXNNE_OPERATION_TARGET_TP,
                                           VXNNE_OPERATOR_ACTIVATION,
                                           VX_NULL,
                                           vxnneOperation_TP_Deinitialize,
                                           batchCount,
                                           0);
        if (status != VX_SUCCESS) goto exit;

        conv.data_buff = vxnneAllocateTPLUTBuffer(context, node);
        if (conv.data_buff == VX_NULL)
        {
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
        conv.tpType = TP_ACTIVATION;
        conv.other_ref = gcvNULL;
        conv.tp_value = (vx_tp_value_cmd_s*)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
        conv.tp_value->e32[0] = func_s->value->e;
        conv.tp_value->f32[0] = (vxoScalar_GetDataType(a_s) == VX_TYPE_FLOAT32) ? a_s->value->f32 : (vx_float32)a_s->value->n32;
        conv.tp_value->f32[1] = (vxoScalar_GetDataType(b_s) == VX_TYPE_FLOAT32) ? b_s->value->f32 : (vx_float32)b_s->value->n32;

        vxMemCopy(&activationLayer->activation_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));

        vxnneLayer_SetOperation(
            &activationLayer->base,
            &activationLayer->activation_tp_operation.base,
            0);

        activationLayer->activation_tp_operation.input  = inputs;
        activationLayer->activation_tp_operation.output = outputs;

        vxnneOperation_AddReference(&activationLayer->activation_tp_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&activationLayer->activation_tp_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }
    else if(shExe_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
    {
        vxnne_shader_executable shaderExecutable = VX_NULL;
        vx_uint32  reshpTensor_Sizes[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {1};
        vx_uint32  reshpTensor_Dims = 2;
        vx_tensor input      = NULL;
        vx_tensor output     = NULL;
        vx_enum   a_type     = vxoScalar_GetDataType(a_s);
        vx_enum   b_type     = vxoScalar_GetDataType(b_s);

        vx_float32 minVal = a_type == VX_TYPE_FLOAT32 ? a_s->value->f32 : (vx_float32)a_s->value->n32;
        vx_float32 maxVal = b_type == VX_TYPE_FLOAT32 ? b_s->value->f32 : (vx_float32)b_s->value->n32;

        vxoElementOptimization_GetTensorShape(inputs, reshpTensor_Sizes, &reshpTensor_Dims);

        input     = vxoTensor_ReshapeTensor(inputs, (vx_int32*)reshpTensor_Sizes, reshpTensor_Dims);
        output     = vxoTensor_ReshapeTensor(outputs, (vx_int32*)reshpTensor_Sizes, reshpTensor_Dims);

        activationLayer->base.temp_tensors[0] = input;
        activationLayer->base.temp_tensors[1] = output;
        activationLayer->base.num_temp_tensors = 2;

        if (func_v == VX_NN_ACTIVATION_RELU1)
        {
            minVal = -1;
            maxVal = 1;
        }
        else if (func_v == VX_NN_ACTIVATION_RELU6)
        {
            minVal = 0;
            maxVal = 6;
        }
        else if (func_v == VX_NN_ACTIVATION_RELU)
        {
            minVal = 0;
            maxVal = 32767;
        }
        else if (func_v == VX_NN_ACTIVATION_BRELU)
        {
            maxVal = minVal;
            minVal = 0;
        }

        batchCount = reshpTensor_Dims > 3 ? reshpTensor_Sizes[3] : 1;

        if(node->base.context->evisNoInst.supportEVIS)
        {
            if (enable_tensorABS_SHExe)
                shaderExecutable = vxnneGetTensorAbsShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_ABS, &node->kernelAttributes.borderMode, input, output);
            else if (enable_tensorTR_SHExe)
                shaderExecutable = vxnneGetTensorTRShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_TRANSCENDENTAL, &node->kernelAttributes.borderMode, input, minVal, maxVal, func_v, output);
            else if (enable_tf_quantize)
                shaderExecutable = vxnneGetActivation_UInt8ShaderExecutable(node->base.context, VXNNE_KERNEL_ACTIVATION_UINT8, &node->kernelAttributes.borderMode, func_v, input, minVal, maxVal, output);
            else
                shaderExecutable = vxnneGetActivationShaderExecutable(node->base.context, VXNNE_KERNEL_ACTIVATION, &node->kernelAttributes.borderMode, func_v, input, minVal, maxVal, output);
        }
        else
        {
            if (enable_tensorTR_SHExe)
                shaderExecutable = vxnneGetGPUTensorTRShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_TRANSCENDENTAL, &node->kernelAttributes.borderMode, input, minVal, maxVal, func_v, output);
            else
                shaderExecutable = vxnneGetGPUActivationShaderExecutable(node->base.context, VXNNE_KERNEL_ACTIVATION, &node->kernelAttributes.borderMode, func_v, input, minVal, maxVal, output);
        }

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&activationLayer->activation_SHoperation,
            &activationLayer->base,
            VXNNE_OPERATOR_ACTIVATION,
            batchCount,
            shaderExecutable);

        if (status != VX_SUCCESS) goto exit;

        vxnneOperation_AddReference(&activationLayer->activation_SHoperation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&activationLayer->activation_SHoperation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        vxnneLayer_SetOperation(
            &activationLayer->base,
            &activationLayer->activation_SHoperation.base,
            0);
    }
    else if(enable_Leaky_SHExe && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
    {
        vxnne_shader_executable shaderExecutable = VX_NULL;
        vx_float32 val = 0.1f;
        vx_scalar negative_slopes = vxCreateScalar(context, VX_TYPE_FLOAT32, &val);

        if (negative_slopes != NULL)
        {
            if(context->evisNoInst.supportEVIS)
            {
                shaderExecutable = vxnneGetLeakyReluShaderExecutable(node->base.context, VXNNE_KERNEL_NN_LEAKY, &node->kernelAttributes.borderMode, inputs, negative_slopes, outputs);
            }
            else
            {
                shaderExecutable = vxnneGetGPULeakyReluShaderExecutable(node->base.context, VXNNE_KERNEL_NN_LEAKY, &node->kernelAttributes.borderMode, inputs, negative_slopes, outputs);
            }
        }
        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&activationLayer->activation_SHoperation,
                                        &activationLayer->base,
                                        VXNNE_OPERATOR_ACTIVATION,
                                        batchCount,
                                        shaderExecutable);

        if (status != VX_SUCCESS) goto exit;

        vxnneOperation_AddReference(&activationLayer->activation_SHoperation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&activationLayer->activation_SHoperation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        vxnneLayer_SetOperation(
            &activationLayer->base,
            &activationLayer->activation_SHoperation.base,
            0);

        if (negative_slopes) vxReleaseScalar(&negative_slopes);
    }
    else
    {
        status = vxnneOperation_Initialize(&activationLayer->activation_operation.base,
                                           &activationLayer->base,
                                           VXNNE_OPERATION_TARGET_SW,
                                           VXNNE_OPERATOR_ACTIVATION,
                                           vxnneExecuteSWActivation,
                                           VX_NULL,
                                           batchCount,
                                           0);
        if (status != VX_SUCCESS) goto exit;

        vxnneLayer_SetOperation(
            &activationLayer->base,
            &activationLayer->activation_operation.base,
            0);

        activationLayer->activation_operation.inputs           = inputs;
        activationLayer->activation_operation.func             = func_s;
        activationLayer->activation_operation.a                = a_s;
        activationLayer->activation_operation.b                = b_s;
        activationLayer->activation_operation.outputs          = outputs;

        vxnneOperation_AddReference(&activationLayer->activation_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&activationLayer->activation_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }

    node->layer = &activationLayer->base;
    return status;

exit:
    if (activationLayer) gcoOS_Free(gcvNULL, (gctPOINTER)activationLayer);
#endif
    return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNActivationLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

/***************************************************************************************************************************
 *                                                       Leaky Relu
 ***************************************************************************************************************************/

vx_status vxnneExecuteSWLeakyRelu(struct _vxnne_operation_s *operation)
{
    vxnne_activation_sw_operation activationOperation   = (vxnne_activation_sw_operation)operation;

    vx_tensor   inputs            = (vx_tensor)activationOperation->inputs;
    vx_scalar   negative_slopes   = (vx_scalar)activationOperation->a;
    vx_tensor   outputs           = (vx_tensor)activationOperation->outputs;
    vx_float32  negative_slope_v  = negative_slopes->value->f32;
    vx_uint32 elementCount = 0;
    vx_uint32 i;
    vx_float32 result = 0.0f;
    gctPOINTER inputBase;
    gctPOINTER outputBase;
    vx_type_e inputFormat  = (vx_type_e)TENSOR_DATA_TYPE(inputs);
    vx_type_e outputFormat = (vx_type_e)TENSOR_DATA_TYPE(outputs);

    vx_status status = VX_SUCCESS;

    elementCount = (vx_uint32)vxoMemory_ComputeElementCount(&inputs->tensorBuffer->memory, 0);
    vxoTensor_GetTensorViewMemory(inputs, &inputBase, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, &outputBase, VX_NULL);

    for (i = 0; i < elementCount; i++)
    {
        vx_float32 data = 0;

        data = vxnneGetDataExt(inputFormat, TENSOR_QUANT_TYPE(inputs), i, (vx_uint8_ptr)inputBase, TENSOR_POS(inputs), TENSOR_TF_ZEROPOINT(inputs), TENSOR_TF_SCALE(inputs));

        result = (data > 0.0f) ? data : negative_slope_v * data;

        vxnneSaveDataExt(outputFormat, TENSOR_QUANT_TYPE(outputs), i, result, (vx_uint8_ptr)outputBase, TENSOR_POS(outputs), TENSOR_TF_ZEROPOINT(outputs), TENSOR_TF_SCALE(outputs), TENSOR_ROUNDING_MODE(outputs));
    }
    return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNLeakyReluLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNLeakyReluLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNLeakyReluLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}
#if REGISTER_FRAME

VX_PRIVATE_API vx_status vxoNNLeakyReluLayer_SW_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vx_tensor inputs = (vx_tensor)parameters[0];
    vx_scalar negative_slopes = (vx_scalar)parameters[1];
    vx_tensor outputs = (vx_tensor)parameters[2];
    vx_uint32 batchCount = TENSOR_SIZE_INDEX(inputs, 3);

    vxnne_activation_layer  activationLayer = (vxnne_activation_layer)ops_layer;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxnneOperation_Initialize(&activationLayer->activation_operation.base,
        &activationLayer->base,
        VXNNE_OPERATION_TARGET_SW,
        VXNNE_OPERATOR_ACTIVATION,
        vxnneExecuteSWLeakyRelu,
        VX_NULL,
        batchCount,
        0));

    vxmONERROR(vxnneLayer_SetOperation(
        &activationLayer->base,
        &activationLayer->activation_operation.base,
        0));

    activationLayer->activation_operation.inputs = inputs;
    activationLayer->activation_operation.func = VX_NULL;
    activationLayer->activation_operation.a = negative_slopes;
    activationLayer->activation_operation.b = VX_NULL;
    activationLayer->activation_operation.outputs = outputs;

    vxmONERROR(vxnneOperation_AddReference(&activationLayer->activation_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&activationLayer->activation_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNLeakyReluLayer_SH_EVIS_Support_Ext(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vx_tensor inputs = (vx_tensor)parameters[0];
    vx_tensor outputs = (vx_tensor)parameters[2];
    vx_enum   srcFormat = TENSOR_DATA_TYPE(inputs);
    vx_enum   dstFormat = TENSOR_DATA_TYPE(outputs);
    vx_bool   shExe_flag = vx_false_e;

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    if (evis)
    {
        shExe_flag = (vx_bool)((srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_UINT8)
            || (srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_FLOAT16)
            || (srcFormat == VX_TYPE_INT8 && dstFormat == VX_TYPE_INT8)
            || (srcFormat == VX_TYPE_INT8 && dstFormat == VX_TYPE_FLOAT16)
            || (srcFormat == VX_TYPE_INT16 && dstFormat == VX_TYPE_INT16)
            || (srcFormat == VX_TYPE_INT16 && dstFormat == VX_TYPE_FLOAT16)
            || (srcFormat == VX_TYPE_FLOAT16 && dstFormat != VX_TYPE_FLOAT32));
    }
    else
    {
        shExe_flag  = (vx_bool)((srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_UINT8)
            || (srcFormat == VX_TYPE_FLOAT16 && dstFormat == VX_TYPE_FLOAT16)
            || (srcFormat == VX_TYPE_FLOAT32 && dstFormat == VX_TYPE_FLOAT32)
            || (srcFormat == VX_TYPE_FLOAT32 && dstFormat == VX_TYPE_FLOAT16)
            || (srcFormat == VX_TYPE_FLOAT16 && dstFormat == VX_TYPE_FLOAT32));
    }

    support = support && shExe_flag;

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_bool vxoNNLeakyReluLayer_SH_EVIS_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && node->base.context->evisNoInst.supportEVIS;

    if (!support)return support;

    support = support && vxoNNActivationLayer_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_true_e);;

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNLeakyReluLayer_SH_EVIS_Initialize_Ext(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_status status = VX_SUCCESS;

    vx_tensor inputs = (vx_tensor)parameters[0];
    vx_scalar negative_slopes = (vx_scalar)parameters[1];
    vx_tensor outputs = (vx_tensor)parameters[2];
    vx_uint32 batchCount = TENSOR_SIZE_INDEX(inputs, 3);

    vxnne_activation_layer  activationLayer = (vxnne_activation_layer)ops_layer;

    vxnne_shader_executable shaderExecutable = VX_NULL;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    if(evis)
        shaderExecutable = vxnneGetLeakyReluShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_NN_LEAKY, &ops_layer->node->kernelAttributes.borderMode, inputs, negative_slopes, outputs);
    else
        shaderExecutable = vxnneGetGPULeakyReluShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_NN_LEAKY, &ops_layer->node->kernelAttributes.borderMode, inputs, negative_slopes, outputs);

    if (!shaderExecutable)
    {
        status = VX_FAILURE;
        goto OnError;
    }

    vxmONERROR(vxnneShaderOperation_Initialize(&activationLayer->activation_SHoperation,
        &activationLayer->base,
        VXNNE_OPERATOR_ACTIVATION,
        batchCount,
        shaderExecutable));

    vxmONERROR(vxnneOperation_AddReference(&activationLayer->activation_SHoperation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&activationLayer->activation_SHoperation.base, (vx_reference)negative_slopes, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&activationLayer->activation_SHoperation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    vxmONERROR(vxnneLayer_SetOperation(
        &activationLayer->base,
        &activationLayer->activation_SHoperation.base,
        0));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_status vxoNNLeakyReluLayer_SH_EVIS_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxoNNLeakyReluLayer_SH_EVIS_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_true_e));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNLeakyReluLayer_SH_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && vxoNNActivationLayer_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_false_e);;

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNLeakyReluLayer_SH_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxoNNLeakyReluLayer_SH_EVIS_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_false_e));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNLeakyReluLayer_TP_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_tensor inputs = (vx_tensor)parameters[0];
    vx_tensor outputs = (vx_tensor)parameters[2];

    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_TP, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    support = support && vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_TP_ACTIVATION);
    support = support && vxnneIsTPSupportFormat(node->base.context, inputs, VX_NULL, outputs);
    support = support && ((TENSOR_VIEW_SIZE_INDEX(outputs, 0) * TENSOR_VIEW_SIZE_INDEX(outputs, 1) * TENSOR_VIEW_SIZE_INDEX(outputs, 2) > 1));

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNLeakyReluLayer_TP_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vx_tensor inputs = (vx_tensor)parameters[0];
    vx_scalar negative_slopes = (vx_scalar)parameters[1];
    vx_tensor outputs = (vx_tensor)parameters[2];
    vx_uint32 batchCount = TENSOR_SIZE_INDEX(inputs, 3);
    vxnne_activation_layer  activationLayer = (vxnne_activation_layer)ops_layer;

    vx_op_param_s conv = { 0 };

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxnneOperation_Initialize(&activationLayer->activation_tp_operation.base,
        &activationLayer->base,
        VXNNE_OPERATION_TARGET_TP,
        VXNNE_OPERATOR_ACTIVATION,
        VX_NULL,
        vxnneOperation_TP_Deinitialize,
        batchCount,
        0));

    conv.data_buff = vxnneAllocateTPLUTBuffer(ops_layer->node->base.context, ops_layer->node);
    if (conv.data_buff == VX_NULL)
    {
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
    conv.tpType = TP_ACTIVATION;
    conv.other_ref = gcvNULL;
    conv.tp_value = (vx_tp_value_cmd_s*)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
    conv.tp_value->e32[0] = VX_NN_ACTIVATION_LEAKYRELU;
    conv.tp_value->f32[0] = negative_slopes->value->f32;

    vxMemCopy(&activationLayer->activation_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));

    vxmONERROR(vxnneLayer_SetOperation(
        &activationLayer->base,
        &activationLayer->activation_tp_operation.base,
        0));

    activationLayer->activation_tp_operation.input = inputs;
    activationLayer->activation_tp_operation.output = outputs;

    vxmONERROR(vxnneOperation_AddReference(&activationLayer->activation_tp_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&activationLayer->activation_tp_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_status vxoNNLayer_GetLeakyReluOperations(vxnne_layer ops_layer, vx_uint32_ptr max_num_operations, vxnne_operation **operations)
{
    vx_status  status = VX_SUCCESS;
    vxnne_activation_layer  activationLayer = (vxnne_activation_layer)ops_layer;

    *max_num_operations = gcmCOUNTOF(activationLayer->operations);

    *operations = activationLayer->operations;

    return status;
}
#endif

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNLeakyReluLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status                     = VX_SUCCESS;
#if REGISTER_FRAME

    vxnne_layer_imp_s registerLeakyRleuLayers[] = {/* Please DON'T adjust the order, it's importent */
        { "LeakyReluLayer NN", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "LeakyReluLayer TP", vxoNNLeakyReluLayer_TP_Support, vxoNNLeakyReluLayer_TP_Initialize, vxoNNCommon_Deinitialize },
        { "LeakyReluLayer SH EVIS", vxoNNLeakyReluLayer_SH_EVIS_Support, vxoNNLeakyReluLayer_SH_EVIS_Initialize,VX_NULL },
        { "LeakyReluLayer SH F32", vxoNNLeakyReluLayer_SH_Support, vxoNNLeakyReluLayer_SH_Initialize, VX_NULL },
        { "LeakyReluLayer SW support", vxoNNCommon_Support, vxoNNLeakyReluLayer_SW_Initialize, VX_NULL },
    };

    REGISTER_LAYERS(registerLeakyRleuLayers, vxnne_activation_layer_s, "LeakyReluLayer", vxoNNLayer_GetLeakyReluOperations);

OnError:
#else
    vx_context context                   = vxGetContext((vx_reference)node);

    vx_tensor inputs                     = (vx_tensor)parameters[0];
    vx_scalar negative_slopes            = (vx_scalar)parameters[1];
    vx_tensor outputs                    = (vx_tensor)parameters[2];
    vx_enum   srcFormat                  = TENSOR_DATA_TYPE(inputs);
    vx_enum   dstFormat                  = TENSOR_DATA_TYPE(outputs);
    vx_uint32 batchCount                 = TENSOR_SIZE_INDEX(inputs, 3);
    vx_bool   shExe_flag                 = vx_false_e;

    vxnne_activation_layer  activationLayer = VX_NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_activation_layer_s), (gctPOINTER*)&activationLayer);
    if (!activationLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(activationLayer, sizeof(vxnne_activation_layer_s));

    vxnneLayer_Initialize(&activationLayer->base,
                          "LeakyReluLayer",
                          node,
                          vxmOPERATION_COUNT(activationLayer),
                          activationLayer->operations,
                          vxnneLayer_Deinitialize);

    if(context->evisNoInst.supportEVIS)
    {
        shExe_flag  = (vx_bool)((srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_UINT8)
                              || (srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_FLOAT16)
                              || (srcFormat == VX_TYPE_INT8 && dstFormat == VX_TYPE_INT8)
                              || (srcFormat == VX_TYPE_INT8 && dstFormat == VX_TYPE_FLOAT16)
                              || (srcFormat == VX_TYPE_INT16 && dstFormat == VX_TYPE_INT16)
                              || (srcFormat == VX_TYPE_INT16 && dstFormat == VX_TYPE_FLOAT16)
                              || (srcFormat == VX_TYPE_FLOAT16 && dstFormat != VX_TYPE_FLOAT32));
    }
    else
    {
        shExe_flag  = (vx_bool)((srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_UINT8)
                              || (srcFormat == VX_TYPE_FLOAT16 && dstFormat == VX_TYPE_FLOAT16)
                              || (srcFormat == VX_TYPE_FLOAT32 && dstFormat == VX_TYPE_FLOAT32)
                              || (srcFormat == VX_TYPE_FLOAT32 && dstFormat == VX_TYPE_FLOAT16)
                              || (srcFormat == VX_TYPE_FLOAT16 && dstFormat == VX_TYPE_FLOAT32));
    }

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_ACTIVATION) &&
        vxnneIsTPSupportFormat(context, inputs, VX_NULL, outputs) &&
        (TENSOR_VIEW_SIZE_INDEX(outputs, 0) * TENSOR_VIEW_SIZE_INDEX(outputs, 1) * TENSOR_VIEW_SIZE_INDEX(outputs, 2) > 1))
    {
        vx_op_param_s conv = {0};

        status = vxnneOperation_Initialize(&activationLayer->activation_tp_operation.base,
                                           &activationLayer->base,
                                           VXNNE_OPERATION_TARGET_TP,
                                           VXNNE_OPERATOR_ACTIVATION,
                                           VX_NULL,
                                           vxnneOperation_TP_Deinitialize,
                                           batchCount,
                                           0);
        if (status != VX_SUCCESS) goto exit;

        conv.data_buff = vxnneAllocateTPLUTBuffer(context, node);
        if (conv.data_buff == VX_NULL)
        {
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
        conv.tpType = TP_ACTIVATION;
        conv.other_ref = gcvNULL;
        conv.tp_value = (vx_tp_value_cmd_s*)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
        conv.tp_value->e32[0] = VX_NN_ACTIVATION_LEAKYRELU;
        conv.tp_value->f32[0] = negative_slopes->value->f32;

        vxMemCopy(&activationLayer->activation_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));

        vxnneLayer_SetOperation(
            &activationLayer->base,
            &activationLayer->activation_tp_operation.base,
            0);

        activationLayer->activation_tp_operation.input  = inputs;
        activationLayer->activation_tp_operation.output = outputs;

        vxnneOperation_AddReference(&activationLayer->activation_tp_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&activationLayer->activation_tp_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }
    else if(shExe_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
    {
        vxnne_shader_executable shaderExecutable = VX_NULL;

        if(context->evisNoInst.supportEVIS)
        {
            shaderExecutable = vxnneGetLeakyReluShaderExecutable(node->base.context, VXNNE_KERNEL_NN_LEAKY, &node->kernelAttributes.borderMode, inputs, negative_slopes, outputs);
        }
        else
        {
            shaderExecutable = vxnneGetGPULeakyReluShaderExecutable(node->base.context, VXNNE_KERNEL_NN_LEAKY, &node->kernelAttributes.borderMode, inputs, negative_slopes, outputs);
        }

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&activationLayer->activation_SHoperation,
                                        &activationLayer->base,
                                        VXNNE_OPERATOR_ACTIVATION,
                                        batchCount,
                                        shaderExecutable);

        if (status != VX_SUCCESS) goto exit;

        vxnneOperation_AddReference(&activationLayer->activation_SHoperation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&activationLayer->activation_SHoperation.base, (vx_reference)negative_slopes, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&activationLayer->activation_SHoperation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        vxnneLayer_SetOperation(
            &activationLayer->base,
            &activationLayer->activation_SHoperation.base,
            0);
    }
    else
    {
        status = vxnneOperation_Initialize(&activationLayer->activation_operation.base,
                                           &activationLayer->base,
                                           VXNNE_OPERATION_TARGET_SW,
                                           VXNNE_OPERATOR_ACTIVATION,
                                           vxnneExecuteSWLeakyRelu,
                                           VX_NULL,
                                           batchCount,
                                           0);
        if (status != VX_SUCCESS) goto exit;

        vxnneLayer_SetOperation(
            &activationLayer->base,
            &activationLayer->activation_operation.base,
            0);

        activationLayer->activation_operation.inputs           = inputs;
        activationLayer->activation_operation.func             = VX_NULL;
        activationLayer->activation_operation.a                = negative_slopes;
        activationLayer->activation_operation.b                = VX_NULL;
        activationLayer->activation_operation.outputs          = outputs;

        vxnneOperation_AddReference(&activationLayer->activation_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&activationLayer->activation_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }

    node->layer = &activationLayer->base;
    return status;

exit:
    if (activationLayer) gcoOS_Free(gcvNULL, (gctPOINTER)activationLayer);
#endif
    return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNLeakyReluLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

/***************************************************************************************************************************
 *                                                       PRelu
 ***************************************************************************************************************************/

vx_status vxnneExecuteSWPRelu(struct _vxnne_operation_s *operation)
{
    vxnne_prelu_sw_operation preluOperation   = (vxnne_prelu_sw_operation)operation;

    vx_tensor   inputs            = (vx_tensor)preluOperation->inputs;
    vx_tensor   alpha             = (vx_tensor)preluOperation->alpha;
    vx_tensor   outputs           = (vx_tensor)preluOperation->outputs;
    vx_int32    inputZP           = TENSOR_TF_ZEROPOINT(inputs);
    vx_int8     inputFP           = TENSOR_POS(inputs);
    vx_float32  inputScale        = TENSOR_TF_SCALE(inputs);
    vx_int32    alphaZP           = TENSOR_TF_ZEROPOINT(alpha);
    vx_int8     alphaFP           = TENSOR_POS(alpha);
    vx_float32  alphaScale        = TENSOR_TF_SCALE(alpha);
    vx_int32    outputZP          = TENSOR_TF_ZEROPOINT(outputs);
    vx_float32  outputScale       = TENSOR_TF_SCALE(outputs);
    vx_int8     outputFP          = TENSOR_POS(outputs);

    vx_uint32   width             = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
    vx_uint32   height            = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
    vx_uint32   channel           = TENSOR_VIEW_SIZE_INDEX(inputs, 2);
    vx_uint32   spatial           = width * height;

    vx_uint32   i, c;
    vx_float32  result            = 0.0f;
    gctPOINTER  inputBase;
    gctPOINTER  alphaBase;
    gctPOINTER  outputBase;
    vx_type_e   inputFormat       = (vx_type_e)TENSOR_DATA_TYPE(inputs);
    vx_enum     inputQuantFormat  = TENSOR_QUANT_TYPE(inputs);
    vx_type_e   alphaFormat       = (vx_type_e)TENSOR_DATA_TYPE(alpha);
    vx_enum     alphaQuantFormat  = TENSOR_QUANT_TYPE(alpha);
    vx_type_e   outputFormat      = (vx_type_e)TENSOR_DATA_TYPE(outputs);
    vx_enum     outputQuantFormat = TENSOR_QUANT_TYPE(outputs);
    vx_enum     outputRound       = TENSOR_ROUNDING_MODE(outputs);

    vx_status   status            = VX_SUCCESS;

    vxoTensor_GetTensorViewMemory(inputs, &inputBase, VX_NULL);
    vxoTensor_GetTensorViewMemory(alpha, &alphaBase, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, &outputBase, VX_NULL);

    for (c = 0; c < channel; c++)
    {
        vx_float32 aV = vxnneGetDataExt(alphaFormat, alphaQuantFormat, c, (vx_uint8_ptr)alphaBase, alphaFP, alphaZP, alphaScale);
        for (i = 0; i < spatial; i++)
        {
            vx_uint32 index = c * spatial + i;
            vx_float32 in = vxnneGetDataExt(inputFormat, inputQuantFormat, index, (vx_uint8_ptr)inputBase, inputFP, inputZP, inputScale);

            result = (in < 0) ? in * aV : in;

            status = vxnneSaveDataExt(outputFormat, outputQuantFormat, index, result, (vx_uint8_ptr)outputBase, outputFP, outputZP, outputScale, outputRound);
        }
    }

    return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNPReluLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPReluLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPReluLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}
#if REGISTER_FRAME

VX_PRIVATE_API vx_status vxoNNPReluLayer_SW_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vxnne_prelu_layer  pReluLayer = (vxnne_prelu_layer)ops_layer;
    vx_tensor inputs = (vx_tensor)parameters[0];
    vx_tensor alpha = (vx_tensor)parameters[1];
    vx_tensor outputs = (vx_tensor)parameters[2];
    vx_uint32 batchCount = TENSOR_SIZE_INDEX(inputs, 3);

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxnneOperation_Initialize(&pReluLayer->prelu_operation.base,
        &pReluLayer->base,
        VXNNE_OPERATION_TARGET_SW,
        VXNNE_OPERATOR_PRELU,
        vxnneExecuteSWPRelu,
        VX_NULL,
        batchCount,
        0));

    vxmONERROR(vxnneLayer_SetOperation(
        &pReluLayer->base,
        &pReluLayer->prelu_operation.base,
        0));

    pReluLayer->prelu_operation.inputs = inputs;
    pReluLayer->prelu_operation.alpha = alpha;
    pReluLayer->prelu_operation.outputs = outputs;

    vxmONERROR(vxnneOperation_AddReference(&pReluLayer->prelu_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&pReluLayer->prelu_operation.base, (vx_reference)alpha, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&pReluLayer->prelu_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNPReluLayer_SH_EVIS_Support_Ext(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_tensor inputs = (vx_tensor)parameters[0];
    vx_tensor alpha = (vx_tensor)parameters[1];
    vx_enum   srcFormat = TENSOR_DATA_TYPE(inputs);
    vx_enum   alphaFormat = TENSOR_DATA_TYPE(alpha);
    vx_enum   dstFormat = TENSOR_DATA_TYPE(inputs);
    vx_bool   shExe_flag = vx_false_e;

    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    if(evis)
    {
        shExe_flag  = (vx_bool)(((srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_UINT8)
                          || (srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_FLOAT16)
                          || (srcFormat == VX_TYPE_INT8 && dstFormat == VX_TYPE_INT8)
                          || (srcFormat == VX_TYPE_INT8 && dstFormat == VX_TYPE_FLOAT16)
                          || (srcFormat == VX_TYPE_INT16 && dstFormat == VX_TYPE_INT16)
                          || (srcFormat == VX_TYPE_INT16 && dstFormat == VX_TYPE_FLOAT16)
                          || (srcFormat == VX_TYPE_BFLOAT16 && dstFormat == VX_TYPE_BFLOAT16)
                          || (srcFormat == VX_TYPE_FLOAT16 && dstFormat != VX_TYPE_FLOAT32))
                         && alphaFormat == VX_TYPE_FLOAT16);
        shExe_flag  = (vx_bool)(shExe_flag ||
                               (srcFormat == VX_TYPE_BFLOAT16 && dstFormat == VX_TYPE_BFLOAT16
                               && alphaFormat == VX_TYPE_BFLOAT16));
    }
    else
    {
        shExe_flag  = (vx_bool)(((srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_UINT8)
                          || (srcFormat == VX_TYPE_FLOAT16 && dstFormat == VX_TYPE_FLOAT16)
                          || (srcFormat == VX_TYPE_FLOAT32 && dstFormat == VX_TYPE_FLOAT32))
                         && (alphaFormat == VX_TYPE_FLOAT16 || alphaFormat == VX_TYPE_FLOAT32));
    }

    support = support && shExe_flag;

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNPReluLayer_SH_EVIS_Initialize_Ext(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_status status = VX_SUCCESS;
    vxnne_prelu_layer  pReluLayer = (vxnne_prelu_layer)ops_layer;

    vx_tensor inputs = (vx_tensor)parameters[0];
    vx_tensor alpha = (vx_tensor)parameters[1];
    vx_tensor outputs = (vx_tensor)parameters[2];
    vx_uint32 batchCount = TENSOR_SIZE_INDEX(inputs, 3);
    vxnne_shader_executable shaderExecutable = VX_NULL;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    if(evis)
        shaderExecutable = vxnneGetPReluShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_PRELU, &ops_layer->node->kernelAttributes.borderMode, inputs, alpha, outputs);
    else
        shaderExecutable = vxnneGetGPUPReluShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_PRELU, &ops_layer->node->kernelAttributes.borderMode, inputs, alpha, outputs);

    if (!shaderExecutable)
    {
        status = VX_FAILURE;
        goto OnError;
    }

    vxmONERROR(vxnneShaderOperation_Initialize(&pReluLayer->prelu_sh_operation,
        &pReluLayer->base,
        VXNNE_OPERATOR_PRELU,
        batchCount,
        shaderExecutable));

    vxmONERROR(vxnneOperation_AddReference(&pReluLayer->prelu_sh_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&pReluLayer->prelu_sh_operation.base, (vx_reference)alpha, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&pReluLayer->prelu_sh_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    vxmONERROR(vxnneLayer_SetOperation(
        &pReluLayer->base,
        &pReluLayer->prelu_sh_operation.base,
        0));

    if (batchCount > 1)
    {
        vxmONERROR(vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NO_BATCH_BIT));
    }
OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNPReluLayer_SH_EVIS_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && node->base.context->evisNoInst.supportEVIS;

    if (!support)return support;

    support = support && vxoNNPReluLayer_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_true_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNPReluLayer_SH_EVIS_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxoNNPReluLayer_SH_EVIS_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_true_e));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNPReluLayer_SH_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && vxoNNPReluLayer_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_false_e);;

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNPReluLayer_SH_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxoNNPReluLayer_SH_EVIS_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_false_e));

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNPReluLayer_TP_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_tensor inputs = (vx_tensor)parameters[0];
    vx_tensor outputs = (vx_tensor)parameters[2];

    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_TP, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    support = support && vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_TP_ACTIVATION);
    support = support && vxnneIsTPSupportFormat(node->base.context, inputs, VX_NULL, outputs);
    support = support && (TENSOR_VIEW_SIZE_INDEX(outputs, 0) * TENSOR_VIEW_SIZE_INDEX(outputs, 1) /** TENSOR_VIEW_SIZE_INDEX(outputs, 2) */> 1);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNPReluLayer_TP_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vxnne_prelu_layer  pReluLayer = (vxnne_prelu_layer)ops_layer;
    vx_context context = vxGetContext((vx_reference)ops_layer->node);
    vx_tensor inputs = (vx_tensor)parameters[0];
    vx_tensor alpha = (vx_tensor)parameters[1];
    vx_tensor outputs = (vx_tensor)parameters[2];
    vx_uint32 batchCount = TENSOR_SIZE_INDEX(inputs, 3);

    vx_uint32 op_index = 0;

    vx_int32    alphaZP = TENSOR_TF_ZEROPOINT(alpha);
    vx_int8     alphaFP = TENSOR_POS(alpha);
    vx_float32  alphaScale = TENSOR_TF_SCALE(alpha);
    vx_uint32   width = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
    vx_uint32   height = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
    vx_uint32   channel = TENSOR_VIEW_SIZE_INDEX(inputs, 2);
    vx_uint32   dimsionCount = TENSOR_DIM_NUM(inputs);

    vx_uint32   c;
    gctPOINTER  alphaBase;

    vx_type_e   alphaFormat = (vx_type_e)TENSOR_DATA_TYPE(alpha);
    vx_enum     alphaQuantFormat = TENSOR_QUANT_TYPE(alpha);

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    assert(dimsionCount < 4 || (dimsionCount == 4 && (TENSOR_VIEW_SIZE_INDEX(inputs, 3) == 1)));

    vxoTensor_GetTensorViewMemory(alpha, &alphaBase, VX_NULL);

    for (c = 0; c < channel; c++) /*Use leakyRelu do Prelu */
    {
        vx_op_param_s conv = { 0 };
        vx_float32 aV = vxnneGetDataExt(alphaFormat, alphaQuantFormat, c, (vx_uint8_ptr)alphaBase, alphaFP, alphaZP, alphaScale);

        vx_size view_start[4] = { 0,0,c,0 };
        vx_size view_end[4] = { width, height, c + 1,0 };
        vx_tensor t_in, t_out;

        t_in = vxCreateTensorFromView(inputs, dimsionCount, view_start, view_end);
        CHECK_NULL(t_in);
        t_out = vxCreateTensorFromView(outputs, dimsionCount, view_start, view_end);
        CHECK_NULL(t_out);

        vxmONERROR(vxnneOperation_Initialize(&pReluLayer->activation_tp_operation[c].base,
            &pReluLayer->base,
            VXNNE_OPERATION_TARGET_TP,
            VXNNE_OPERATOR_ACTIVATION,
            VX_NULL,
            vxnneOperation_TP_Deinitialize,
            batchCount,
            0));

        conv.data_buff = vxnneAllocateTPLUTBuffer(context, ops_layer->node);
        if (conv.data_buff == VX_NULL)
        {
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
        conv.tpType = TP_ACTIVATION;
        conv.other_ref = gcvNULL;
        conv.tp_value = (vx_tp_value_cmd_s*)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
        conv.tp_value->e32[0] = VX_NN_ACTIVATION_LEAKYRELU;
        conv.tp_value->f32[0] = aV;

        vxMemCopy(&pReluLayer->activation_tp_operation[c].base.parameter, &conv, sizeof(vx_op_param_s));

        vxmONERROR(vxnneLayer_SetOperation(
            &pReluLayer->base,
            &pReluLayer->activation_tp_operation[c].base,
            op_index++));

        pReluLayer->activation_tp_operation[c].input = t_in;
        pReluLayer->activation_tp_operation[c].output = t_out;

        vxmONERROR(vxnneOperation_AddReference(&pReluLayer->activation_tp_operation[c].base, (vx_reference)t_in, VXNNE_OPERATION_REFENRENCE_INPUT));
        vxmONERROR(vxnneOperation_AddReference(&pReluLayer->activation_tp_operation[c].base, (vx_reference)t_out, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    }
OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_status vxoNNLayer_GetPReluOperations(vxnne_layer ops_layer, vx_uint32_ptr max_num_operations, vxnne_operation **operations)
{
    vx_status  status = VX_SUCCESS;
    vxnne_prelu_layer  preluLayer = (vxnne_prelu_layer)ops_layer;

    *max_num_operations = gcmCOUNTOF(preluLayer->operations);

    *operations = preluLayer->operations;

    return status;
}
#endif


VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPReluLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status                     = VX_SUCCESS;
#if REGISTER_FRAME

    vxnne_layer_imp_s registerPReluLayers[] = {/* Please DON'T adjust the order, it's importent */
        { "PReluLayer NN", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "PReluLayer TP", vxoNNPReluLayer_TP_Support, vxoNNPReluLayer_TP_Initialize, VX_NULL },
        { "PReluLayer SH EVIS", vxoNNPReluLayer_SH_EVIS_Support, vxoNNPReluLayer_SH_EVIS_Initialize, VX_NULL },
        { "PReluLayer SH F32", vxoNNPReluLayer_SH_Support, vxoNNPReluLayer_SH_Initialize, VX_NULL },
        { "PReluLayer SW support", vxoNNCommon_Support, vxoNNPReluLayer_SW_Initialize, VX_NULL },
    };

    REGISTER_LAYERS(registerPReluLayers, vxnne_prelu_layer_s, "PReluLayer", vxoNNLayer_GetPReluOperations);

OnError:
#else
    vx_context context                   = vxGetContext((vx_reference)node);
    vx_tensor inputs                     = (vx_tensor)parameters[0];
    vx_tensor alpha                      = (vx_tensor)parameters[1];
    vx_tensor outputs                    = (vx_tensor)parameters[2];
    vx_uint32 batchCount                 = TENSOR_SIZE_INDEX(inputs, 3);
    vx_enum   srcFormat                  = TENSOR_DATA_TYPE(inputs);
    vx_enum   alphaFormat                = TENSOR_DATA_TYPE(alpha);
    vx_enum   dstFormat                  = TENSOR_DATA_TYPE(inputs);
    vx_bool   shExe_flag                 = vx_false_e;
    vx_uint32 op_index = 0;
    vxnne_prelu_layer  pReluLayer = VX_NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_prelu_layer_s), (gctPOINTER*)&pReluLayer);
    if (!pReluLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(pReluLayer, sizeof(vxnne_prelu_layer_s));

    vxnneLayer_Initialize(&pReluLayer->base,
                          "PReluLayer",
                          node,
                          vxmOPERATION_COUNT(pReluLayer),
                          pReluLayer->operations,
                          vxnneLayer_Deinitialize);

    if(node->base.context->evisNoInst.supportEVIS)
    {
        shExe_flag  = (vx_bool)(((srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_UINT8)
                          || (srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_FLOAT16)
                          || (srcFormat == VX_TYPE_INT8 && dstFormat == VX_TYPE_INT8)
                          || (srcFormat == VX_TYPE_INT8 && dstFormat == VX_TYPE_FLOAT16)
                          || (srcFormat == VX_TYPE_INT16 && dstFormat == VX_TYPE_INT16)
                          || (srcFormat == VX_TYPE_INT16 && dstFormat == VX_TYPE_FLOAT16)
                          || (srcFormat == VX_TYPE_BFLOAT16 && dstFormat == VX_TYPE_BFLOAT16)
                          || (srcFormat == VX_TYPE_FLOAT16 && dstFormat != VX_TYPE_FLOAT32))
                         && alphaFormat == VX_TYPE_FLOAT16);
        shExe_flag  = (vx_bool)(shExe_flag ||
                               (srcFormat == VX_TYPE_BFLOAT16 && dstFormat == VX_TYPE_BFLOAT16
                               && alphaFormat == VX_TYPE_BFLOAT16));
    }
    else
    {
        shExe_flag  = (vx_bool)(((srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_UINT8)
                          || (srcFormat == VX_TYPE_FLOAT16 && dstFormat == VX_TYPE_FLOAT16)
                          || (srcFormat == VX_TYPE_FLOAT32 && dstFormat == VX_TYPE_FLOAT32))
                         && (alphaFormat == VX_TYPE_FLOAT16 || alphaFormat == VX_TYPE_FLOAT32));
    }

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_ACTIVATION) &&
            vxnneIsTPSupportFormat(context, inputs, VX_NULL, outputs) &&
            (TENSOR_VIEW_SIZE_INDEX(outputs, 0) * TENSOR_VIEW_SIZE_INDEX(outputs, 1) /** TENSOR_VIEW_SIZE_INDEX(outputs, 2)*/ > 1) &&
            TENSOR_VIEW_SIZE_INDEX(inputs, 2) < PRELU_CHANNEL_MAX)
        {

        vx_int32    alphaZP           = TENSOR_TF_ZEROPOINT(alpha);
        vx_int8     alphaFP           = TENSOR_POS(alpha);
        vx_float32  alphaScale        = TENSOR_TF_SCALE(alpha);
        vx_uint32   width             = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
        vx_uint32   height            = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
        vx_uint32   channel           = TENSOR_VIEW_SIZE_INDEX(inputs, 2);
        vx_uint32   dimsionCount      = TENSOR_DIM_NUM(inputs);

        vx_uint32   c;
        gctPOINTER  alphaBase;

        vx_type_e   alphaFormat       = (vx_type_e)TENSOR_DATA_TYPE(alpha);
        vx_enum     alphaQuantFormat  = TENSOR_QUANT_TYPE(alpha);
        assert(dimsionCount < 4 || (dimsionCount == 4 && (TENSOR_VIEW_SIZE_INDEX(inputs, 3) == 1)));

        vxoTensor_GetTensorViewMemory(alpha, &alphaBase, VX_NULL);

        for (c = 0; c < channel; c++) /*Use leakyRelu do Prelu */
        {
            vx_op_param_s conv = {0};
            vx_float32 aV = vxnneGetDataExt(alphaFormat, alphaQuantFormat, c, (vx_uint8_ptr)alphaBase, alphaFP, alphaZP, alphaScale);

            vx_size view_start[4] = {0,0,c,0};
            vx_size view_end[4] = {width, height, c+1,0};
            vx_tensor t_in, t_out;

            t_in = vxCreateTensorFromView(inputs, dimsionCount, view_start, view_end);
            CHECK_NULL(t_in);
            t_out= vxCreateTensorFromView(outputs, dimsionCount, view_start, view_end);
            CHECK_NULL(t_out);

            status = vxnneOperation_Initialize(&pReluLayer->activation_tp_operation[c].base,
                                    &pReluLayer->base,
                                    VXNNE_OPERATION_TARGET_TP,
                                    VXNNE_OPERATOR_ACTIVATION,
                                    VX_NULL,
                                    vxnneOperation_TP_Deinitialize,
                                    batchCount,
                                    0);

            if (status != VX_SUCCESS) goto exit;

            conv.data_buff = vxnneAllocateTPLUTBuffer(context, node);
            if (conv.data_buff == VX_NULL)
            {
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
            conv.tpType = TP_ACTIVATION;
            conv.other_ref = gcvNULL;
            conv.tp_value = (vx_tp_value_cmd_s*)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
            conv.tp_value->e32[0] = VX_NN_ACTIVATION_LEAKYRELU;
            conv.tp_value->f32[0] = aV;

            vxMemCopy(&pReluLayer->activation_tp_operation[c].base.parameter, &conv, sizeof(vx_op_param_s));

            vxnneLayer_SetOperation(
                &pReluLayer->base,
                &pReluLayer->activation_tp_operation[c].base,
                op_index++);

            pReluLayer->activation_tp_operation[c].input  = t_in;
            pReluLayer->activation_tp_operation[c].output = t_out;

            vxnneOperation_AddReference(&pReluLayer->activation_tp_operation[c].base, (vx_reference)t_in, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&pReluLayer->activation_tp_operation[c].base, (vx_reference)t_out, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        }
    }
    else if(shExe_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
    {
        vxnne_shader_executable shaderExecutable = VX_NULL;

        if(node->base.context->evisNoInst.supportEVIS)
            shaderExecutable = vxnneGetPReluShaderExecutable(node->base.context, VXNNE_KERNEL_PRELU, &node->kernelAttributes.borderMode, inputs, alpha, outputs);
        else
            shaderExecutable = vxnneGetGPUPReluShaderExecutable(node->base.context, VXNNE_KERNEL_PRELU, &node->kernelAttributes.borderMode, inputs, alpha, outputs);

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&pReluLayer->prelu_sh_operation,
                                        &pReluLayer->base,
                                        VXNNE_OPERATOR_PRELU,
                                        batchCount,
                                        shaderExecutable);

        if (status != VX_SUCCESS) goto exit;

        vxnneOperation_AddReference(&pReluLayer->prelu_sh_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&pReluLayer->prelu_sh_operation.base, (vx_reference)alpha, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&pReluLayer->prelu_sh_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        vxnneLayer_SetOperation(
            &pReluLayer->base,
            &pReluLayer->prelu_sh_operation.base,
            0);

        if (batchCount > 1)
        {
            vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NO_BATCH_BIT);
        }
    }
    else
    {
        status = vxnneOperation_Initialize(&pReluLayer->prelu_operation.base,
                                           &pReluLayer->base,
                                           VXNNE_OPERATION_TARGET_SW,
                                           VXNNE_OPERATOR_PRELU,
                                           vxnneExecuteSWPRelu,
                                           VX_NULL,
                                           batchCount,
                                           0);
        if (status != VX_SUCCESS) goto exit;

        vxnneLayer_SetOperation(
            &pReluLayer->base,
            &pReluLayer->prelu_operation.base,
            0);

        pReluLayer->prelu_operation.inputs           = inputs;
        pReluLayer->prelu_operation.alpha            = alpha;
        pReluLayer->prelu_operation.outputs          = outputs;

        vxnneOperation_AddReference(&pReluLayer->prelu_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&pReluLayer->prelu_operation.base, (vx_reference)alpha, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&pReluLayer->prelu_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }

    node->layer = &pReluLayer->base;
    return status;

exit:
    if (pReluLayer) gcoOS_Free(gcvNULL, (gctPOINTER)pReluLayer);
#endif

    return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPReluLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

vx_float32 vxnneActivation(vx_enum func_v, vx_float32 a_v, vx_float32 b_v, vx_float32 value)
{
    vx_float64 result = value;

    switch (func_v)
    {
    case VX_NN_ACTIVATION_LOGISTIC:
        {
            result = 1.0f / (1 + gcoMATH_Exp(value * (-1)));
        }
        break;

    case VX_NN_ACTIVATION_HYPERBOLIC_TAN:
        {
            result = a_v * gcoMATH_TangentH(b_v * value);
        }
        break;

    case VX_NN_ACTIVATION_RELU:
        {
            result = gcoMATH_MAX(0.0f, value);
        }
        break;

    case VX_NN_ACTIVATION_BRELU:
        {
            result = gcoMATH_MIN(a_v, gcoMATH_MAX(0.0f, value));
        }
        break;

    case VX_NN_ACTIVATION_SOFTRELU:
        {
            result = gcoMATH_Log(1 + gcoMATH_Exp(value));
        }
        break;

    case VX_NN_ACTIVATION_ABS:
        {
            result = gcoMATH_Absolute(value);
        }
        break;

    case VX_NN_ACTIVATION_SQUARE:
        {
            result = gcoMATH_Power(value, 2);
        }
        break;

    case VX_NN_ACTIVATION_SQRT:
        {
            result = gcoMATH_SquareRoot(value);
        }
        break;

    case VX_NN_ACTIVATION_LINEAR:
        {
            result = a_v * value + b_v;
        }
        break;

    case VX_NN_ACTIVATION_LEAKYRELU:
        {
            result = (value > 0.0f) ? value : 0.1f * value;
        }
        break;

    case VX_NN_ACTIVATION_RELU6:
        {
            result = gcoMATH_MIN(gcoMATH_MAX(value, 0), 6);
        }
        break;

    case VX_NN_ACTIVATION_RELU1:
        {
            result = gcoMATH_MIN(gcoMATH_MAX(value, -1), 1);
        }
        break;

    case VX_NN_ACTIVATION_RSQRT:
        {
            result = gcoMATH_ReciprocalSquareRoot(value);
        }
        break;

    default:
        vxError("this activation func not support");
        break;
    }
    return (vx_float32)result;
}

