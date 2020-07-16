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
#include <layers/gc_vx_layer_batch_norm.h>

/***************************************************************************************************************************
 *                                                       Batch Normalization
 ***************************************************************************************************************************/

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNBatchNormalizationLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNBatchNormalizationLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNBatchNormalizationLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

vx_status vxnneExecuteSWBatchNormPreProcess(vx_tensor means, vx_tensor variances, vx_tensor gamma, vx_tensor beta, vx_float32 eps, vx_float32 inputScale, vx_int32 input_ZP, vx_float32 outputScale, vx_float32 output_ZP, vx_tensor weights, vx_tensor biases)
{
    vx_status       status          = VX_SUCCESS;
    vx_uint8_ptr    weightsLogic    = NULL;
    vx_uint8_ptr    biasesLogic     = NULL;
    vx_uint8_ptr    meanLogic       = NULL;
    vx_uint8_ptr    varianceLogic   = NULL;
    vx_uint8_ptr    gammaLogic      = NULL;
    vx_uint8_ptr    betaLogic       = NULL;
    vx_float32_ptr  weightsF32Ptr   = NULL;
    vx_float32_ptr  biasesF32Ptr    = NULL;
    vx_type_e       meanFormat      = (vx_type_e)(TENSOR_DATA_TYPE(means));
    vx_type_e       varianceFormat  = (vx_type_e)(TENSOR_DATA_TYPE(variances));
    vx_type_e       gammaFormat     = (vx_type_e)(TENSOR_DATA_TYPE(gamma));
    vx_type_e       betaFormat      = (vx_type_e)(TENSOR_DATA_TYPE(beta));
    vx_uint32       elementCount    = 1;
    vx_uint32       i               = 0;
    vx_float32      meanf           = 0;
    vx_float32      variancef       = 0;
    vx_float32      gammaf          = 0;
    vx_float32      betaf           = 0;
    vx_float32      weightf         = 0;
    vx_float32      biasf           = 0;

    vxoTensor_GetTensorElementCount(means, &elementCount);

    vxoTensor_GetTensorViewMemory(weights, (gctPOINTER*)&weightsLogic, VX_NULL);
    vxoTensor_GetTensorViewMemory(biases, (gctPOINTER*)&biasesLogic, VX_NULL);
    vxoTensor_GetTensorViewMemory(means, (gctPOINTER*)&meanLogic, VX_NULL);
    vxoTensor_GetTensorViewMemory(variances, (gctPOINTER*)&varianceLogic, VX_NULL);
    vxoTensor_GetTensorViewMemory(gamma, (gctPOINTER*)&gammaLogic, VX_NULL);
    vxoTensor_GetTensorViewMemory(beta, (gctPOINTER*)&betaLogic, VX_NULL);

    weightsF32Ptr = (vx_float32_ptr)weightsLogic;
    biasesF32Ptr  = (vx_float32_ptr)biasesLogic;
    for (i = 0; i < elementCount; i ++)
    {
        meanf     = vxnneGetDataExt(meanFormat, TENSOR_QUANT_TYPE(means), i, meanLogic, TENSOR_POS(means), TENSOR_TF_ZEROPOINT(means), TENSOR_TF_SCALE(means));
        variancef = vxnneGetDataExt(varianceFormat, TENSOR_QUANT_TYPE(variances), i, varianceLogic, TENSOR_POS(variances), TENSOR_TF_ZEROPOINT(variances), TENSOR_TF_SCALE(variances));
        gammaf    = vxnneGetDataExt(gammaFormat, TENSOR_QUANT_TYPE(gamma), i, gammaLogic, TENSOR_POS(gamma), TENSOR_TF_ZEROPOINT(gamma), TENSOR_TF_SCALE(gamma));
        betaf     = vxnneGetDataExt(betaFormat, TENSOR_QUANT_TYPE(beta), i, betaLogic, TENSOR_POS(beta), TENSOR_TF_ZEROPOINT(beta), TENSOR_TF_SCALE(beta));

        weightf      = gammaf / sqrtf(variancef + eps);
        biasf        = betaf - meanf * weightf;

        weightf      = weightf * inputScale * outputScale;
        biasf        = biasf * outputScale + output_ZP - input_ZP * weightf;

        weightsF32Ptr[i] = weightf;
        biasesF32Ptr[i]  = biasf;
    }

    return status;
}

vx_status vxnneExecuteSWBatchNormalization(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;

    vx_tensor input,output;
    vx_tensor means,variances,gamma,beta;
    vx_scalar epss;
    vx_uint8_ptr inputLogic, outputLogic;
    vx_uint8_ptr meanLogic, varianceLogic, gammaLogic, betaLogic;

    vx_uint32  width, height, channel, batch;
    vx_uint32  c, i, spatial, b;
    vx_float32 meanf, variancef, gammaf, betaf;
    vx_float32 inputf, outputf;
    vx_type_e  inFormat, outFormat, meanFormat, varianceFormat, gammaFormat, betaFormat;
    vx_float32 eps;
    vx_float32 normalize;
    vx_uint32  index;

    vxnne_batchnorm_sw_operation activationOperation = (vxnne_batchnorm_sw_operation)operation;
    means       = (vx_tensor)activationOperation->mean;
    variances   = (vx_tensor)activationOperation->variance;
    gamma       = (vx_tensor)activationOperation->gamma;
    beta        = (vx_tensor)activationOperation->beta;

    input       = (vx_tensor)activationOperation->input;
    output      = (vx_tensor)activationOperation->output;

    epss        = (vx_scalar)activationOperation->eps;
    eps         = epss->value->f32;

    width       = TENSOR_VIEW_SIZE_INDEX(input, 0);
    height      = TENSOR_VIEW_SIZE_INDEX(input, 1);
    channel     = TENSOR_VIEW_SIZE_INDEX(input, 2);
    batch       = TENSOR_VIEW_SIZE_INDEX(input, 3);
    spatial     = width * height;

    inFormat        = (vx_type_e)(TENSOR_DATA_TYPE(input));
    outFormat       = (vx_type_e)(TENSOR_DATA_TYPE(output));
    meanFormat      = (vx_type_e)(TENSOR_DATA_TYPE(means));
    varianceFormat  = (vx_type_e)(TENSOR_DATA_TYPE(variances));
    gammaFormat     = (vx_type_e)(TENSOR_DATA_TYPE(gamma));
    betaFormat      = (vx_type_e)(TENSOR_DATA_TYPE(beta));

    vxoTensor_GetTensorViewMemory(input, (gctPOINTER*)&inputLogic, VX_NULL);
    vxoTensor_GetTensorViewMemory(output, (gctPOINTER*)&outputLogic, VX_NULL);
    vxoTensor_GetTensorViewMemory(means, (gctPOINTER*)&meanLogic, VX_NULL);
    vxoTensor_GetTensorViewMemory(variances, (gctPOINTER*)&varianceLogic, VX_NULL);
    vxoTensor_GetTensorViewMemory(gamma, (gctPOINTER*)&gammaLogic, VX_NULL);
    vxoTensor_GetTensorViewMemory(beta, (gctPOINTER*)&betaLogic, VX_NULL);

    for(b = 0; b < batch; ++b)
    {
        for(c = 0; c < channel; c ++)
        {
            meanf     = vxnneGetDataExt(meanFormat, TENSOR_QUANT_TYPE(means), c, meanLogic, TENSOR_POS(means), TENSOR_TF_ZEROPOINT(means), TENSOR_TF_SCALE(means));
            variancef = vxnneGetDataExt(varianceFormat, TENSOR_QUANT_TYPE(variances), c, varianceLogic, TENSOR_POS(variances), TENSOR_TF_ZEROPOINT(variances), TENSOR_TF_SCALE(variances));
            gammaf    = vxnneGetDataExt(gammaFormat, TENSOR_QUANT_TYPE(gamma), c, gammaLogic, TENSOR_POS(gamma), TENSOR_TF_ZEROPOINT(gamma), TENSOR_TF_SCALE(gamma));
            betaf     = vxnneGetDataExt(betaFormat, TENSOR_QUANT_TYPE(beta), c, betaLogic, TENSOR_POS(beta), TENSOR_TF_ZEROPOINT(beta), TENSOR_TF_SCALE(beta));
            for(i = 0; i < spatial; i ++)
            {
                index       = b * channel * spatial + c * spatial + i;
                inputf  = vxnneGetDataExt(inFormat, TENSOR_QUANT_TYPE(input), index, inputLogic, TENSOR_POS(input), TENSOR_TF_ZEROPOINT(input), TENSOR_TF_SCALE(input));
                /* Compute Normalize */
                normalize   = (inputf - meanf)/sqrtf(variancef + eps);
                /* Scale and Shift */
                outputf     = gammaf * normalize + betaf;
                vxnneSaveDataExt(outFormat, TENSOR_QUANT_TYPE(output), index, outputf, outputLogic, TENSOR_POS(output), TENSOR_TF_ZEROPOINT(output), TENSOR_TF_SCALE(output), TENSOR_ROUNDING_MODE(output));
            }
        }
    }
    return status;
}


#if REGISTER_FRAME
VX_PRIVATE_API vx_status vxoNNBatchNormalizationLayer_SW_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vx_scalar  epss                       = (vx_scalar)parameters[0];
    vx_tensor  means                      = (vx_tensor)parameters[1];
    vx_tensor  variances                  = (vx_tensor)parameters[2];
    vx_tensor  gamma                      = (vx_tensor)parameters[3];
    vx_tensor  beta                       = (vx_tensor)parameters[4];
    vx_tensor  input                      = (vx_tensor)parameters[5];
    vx_tensor  output                     = (vx_tensor)parameters[6];

    vxnne_batchnorm_layer  batchnormLayer = (vxnne_batchnorm_layer)ops_layer;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxnneOperation_Initialize(&batchnormLayer->batchnorm_operation.base,
                                       &batchnormLayer->base,
                                       VXNNE_OPERATION_TARGET_SW,
                                       VXNNE_OPERATOR_BATCHNORM,
                                       vxnneExecuteSWBatchNormalization,
                                       VX_NULL,
                                       1/*batchCount*/,
                                       0));

    vxmONERROR(vxnneLayer_SetOperation(
        &batchnormLayer->base,
        &batchnormLayer->batchnorm_operation.base,
        0));

    batchnormLayer->batchnorm_operation.eps              = epss;
    batchnormLayer->batchnorm_operation.mean             = means;
    batchnormLayer->batchnorm_operation.variance         = variances;
    batchnormLayer->batchnorm_operation.gamma            = gamma;
    batchnormLayer->batchnorm_operation.beta             = beta;
    batchnormLayer->batchnorm_operation.input            = input;
    batchnormLayer->batchnorm_operation.output           = output;

    vxmONERROR(vxnneOperation_AddReference(&batchnormLayer->batchnorm_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&batchnormLayer->batchnorm_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT));
OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNBatchNormalizationLayer_SH_EVIS_Support_Ext(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_tensor  input                      = (vx_tensor)parameters[5];
    vx_tensor  output                     = (vx_tensor)parameters[6];
    vx_enum    inputFormat                = TENSOR_DATA_TYPE(input);
    vx_enum    outputFormat               = TENSOR_DATA_TYPE(output);

    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    if(evis)
    {
        support = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
                          || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT8)
                          || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT16)
                          || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_UINT8)
                          || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8)
                          || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_FLOAT16)
                          || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16)
                          || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_FLOAT16)
                          || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
                          || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT16)
                          || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_BFLOAT16)
                          || (inputFormat == VX_TYPE_BFLOAT16 && outputFormat == VX_TYPE_BFLOAT16));
    }
    else
    {
        support = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
                          || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT32)
                          || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32)
                          || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT16)
                          || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8));
    }

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_bool vxoNNBatchNormalizationLayer_SH_EVIS_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && node->base.context->evisNoInst.supportEVIS;

    if (!support)return support;

    support = support && vxoNNBatchNormalizationLayer_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_true_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNBatchNormalizationLayer_SH_EVIS_Initialize_Ext(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_status status = VX_SUCCESS;

    vx_scalar  epss                       = (vx_scalar)parameters[0];
    vx_tensor  means                      = (vx_tensor)parameters[1];
    vx_tensor  variances                  = (vx_tensor)parameters[2];
    vx_tensor  gamma                      = (vx_tensor)parameters[3];
    vx_tensor  beta                       = (vx_tensor)parameters[4];
    vx_tensor  input                      = (vx_tensor)parameters[5];
    vx_tensor  output                     = (vx_tensor)parameters[6];

    vx_tensor weights                     = NULL;
    vx_tensor biases                      = NULL;
    vx_tensor_create_params_t tensor_create_params;
    vx_uint32 batchCount                  = TENSOR_SIZE_INDEX(input, 3);

    vxnne_batchnorm_layer  batchnormLayer = (vxnne_batchnorm_layer)ops_layer;

    vxnne_shader_executable shaderExecutable = VX_NULL;
    vx_uint32  sizes[]          = {1, 1, 1, 1};
    vx_uint32  dims             = 2;
    vx_float32 inputScale       = 1.0f;
    vx_int32   input_ZP         = 0;
    vx_float32 outputScale      = 1.0f;
    vx_float32 output_ZP        = 0.0f;
    vx_int8    srcFixPointPos   = TENSOR_POS(input);
    vx_int8    dstFixPointPos   = TENSOR_POS(output);
    vx_uint32  axis             = 2;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    if (TENSOR_DIM_NUM(input) < 3)
        axis = 0;

    sizes[0] = evis ? TENSOR_VIEW_SIZE_INDEX(input, axis) * 2 : TENSOR_VIEW_SIZE_INDEX(input, axis);
    sizes[1] = 1;

    gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
    tensor_create_params.num_of_dims = dims;
    tensor_create_params.sizes = sizes;
    tensor_create_params.data_format = evis ? VX_TYPE_INT16 : VX_TYPE_FLOAT32;
    tensor_create_params.quant_format = VX_QUANT_NONE;
    tensor_create_params.quant_data.dfp.fixed_point_pos = 0;

    weights = vxoTensor_CreateTensor(ops_layer->node->base.context, ops_layer->node->graph, &tensor_create_params, vx_false_e);
    if (vxoTensor_AllocateMemory(weights) != VX_SUCCESS)
    {
        vxError("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
        status = VX_ERROR_NO_MEMORY;
        goto OnError;
    }

    biases = vxoTensor_CreateTensor(ops_layer->node->base.context, ops_layer->node->graph, &tensor_create_params, vx_false_e);
    if (vxoTensor_AllocateMemory(biases) != VX_SUCCESS)
    {
        vxError("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
        status = VX_ERROR_NO_MEMORY;
        goto OnError;
    }

    if (TENSOR_QUANT_TYPE(input) == VX_QUANT_DYNAMIC_FIXED_POINT)
    {
        if (srcFixPointPos >= 0)
        {
            inputScale = 1.0f / (vx_float32) (1 << srcFixPointPos);
        }
        else if (srcFixPointPos < 0)
        {
            inputScale = (vx_float32) (1 << -srcFixPointPos);
        }
    }
    else if (TENSOR_QUANT_TYPE(input) == VX_QUANT_AFFINE_SCALE)
    {
        input_ZP   = evis ? 0 : TENSOR_TF_ZEROPOINT(input);
        inputScale = TENSOR_TF_SCALE(input);
    }

    if (TENSOR_QUANT_TYPE(output) == VX_QUANT_DYNAMIC_FIXED_POINT)
    {
        if (dstFixPointPos >= 0)
        {
            outputScale = (vx_float32) (1 << dstFixPointPos);
        }
        else if (dstFixPointPos < 0)
        {
            outputScale = 1.0f / (vx_float32) (1 << -dstFixPointPos);
        }
    }
    else if (TENSOR_QUANT_TYPE(output) == VX_QUANT_AFFINE_SCALE)
    {
        outputScale = 1.0f / TENSOR_TF_SCALE(output);
        output_ZP   = (vx_float32)TENSOR_TF_ZEROPOINT(output);
        if (evis == vx_false_e)
            output_ZP += 0.5f;
    }

    vxmONERROR(vxnneExecuteSWBatchNormPreProcess(means, variances, gamma, beta, epss->value->f32, inputScale, input_ZP, outputScale, output_ZP, weights, biases));
    if(evis)
    {
        shaderExecutable = vxnneGetBatchNormShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_BATCHNORM, &ops_layer->node->kernelAttributes.borderMode, axis, input, weights, biases, output);
    }
    else
    {
        shaderExecutable = vxnneGetGPUBatchNormShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_GPU_BATCHNORM, &ops_layer->node->kernelAttributes.borderMode, axis, input, weights, biases, output);
    }

    if (!shaderExecutable)
    {
        status = VX_FAILURE;
        goto OnError;
    }

    vxmONERROR(vxnneShaderOperation_Initialize(&batchnormLayer->batchnorm_sh_operation,
                                    &batchnormLayer->base,
                                    VXNNE_OPERATOR_BATCHNORM,
                                    batchCount,
                                    shaderExecutable));

    vxmONERROR(vxnneLayer_SetOperation(
        &batchnormLayer->base,
        &batchnormLayer->batchnorm_sh_operation.base,
        0));

    vxmONERROR(vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 2, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NO_BATCH_BIT));
    vxmONERROR(vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 3, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NO_BATCH_BIT));

    vxmONERROR(vxnneOperation_AddReference(&batchnormLayer->batchnorm_sh_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&batchnormLayer->batchnorm_sh_operation.base, (vx_reference)weights, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&batchnormLayer->batchnorm_sh_operation.base, (vx_reference)biases, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&batchnormLayer->batchnorm_sh_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    batchnormLayer->base.num_temp_tensors = 2;
    batchnormLayer->base.temp_tensors[0] = weights;
    batchnormLayer->base.temp_tensors[1] = biases;

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

OnError:
    return status;
}

VX_PRIVATE_API vx_status vxoNNBatchNormalizationLayer_SH_EVIS_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxoNNBatchNormalizationLayer_SH_EVIS_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_true_e));

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

OnError:
    return status;
}
VX_PRIVATE_API vx_bool vxoNNBatchNormalizationLayer_SH_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && vxoNNBatchNormalizationLayer_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_false_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNBatchNormalizationLayer_SH_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxoNNBatchNormalizationLayer_SH_EVIS_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_false_e));

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);
OnError:
    return status;
}

VX_PRIVATE_API vx_status vxoNNLayer_GetOperations(vxnne_layer ops_layer, vx_uint32_ptr max_num_operations, vxnne_operation **operations)
{
    vx_status  status = VX_SUCCESS;
    vxnne_batchnorm_layer  Pooling = (vxnne_batchnorm_layer)ops_layer;

    *max_num_operations = gcmCOUNTOF(Pooling->operations);

    *operations = Pooling->operations;

    return status;
}
#endif

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNBatchNormalizationLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status  status                     = VX_SUCCESS;
#if REGISTER_FRAME
    vxnne_layer_imp_s registerBNLayers[] = {/* Please DON'T adjust the order, it's importent */
        { "Concat2Layer NN", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "Concat2Layer TP", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "Concat2Layer SH EVIS", vxoNNBatchNormalizationLayer_SH_EVIS_Support, vxoNNBatchNormalizationLayer_SH_EVIS_Initialize, VX_NULL },
        { "Concat2Layer SH F32", vxoNNBatchNormalizationLayer_SH_Support, vxoNNBatchNormalizationLayer_SH_Initialize, VX_NULL },
        { "Concat2Layer SW", vxoNNCommon_Support, vxoNNBatchNormalizationLayer_SW_Initialize, VX_NULL },
    };

    REGISTER_LAYERS(registerBNLayers, vxnne_concat2_layer_s, "BatchNormalizationLayer", vxoNNLayer_GetOperations);

OnError:
#else
    vx_context context                    = vxGetContext((vx_reference)node);

    vx_scalar  epss                       = (vx_scalar)parameters[0];
    vx_tensor  means                      = (vx_tensor)parameters[1];
    vx_tensor  variances                  = (vx_tensor)parameters[2];
    vx_tensor  gamma                      = (vx_tensor)parameters[3];
    vx_tensor  beta                       = (vx_tensor)parameters[4];
    vx_tensor  input                      = (vx_tensor)parameters[5];
    vx_tensor  output                     = (vx_tensor)parameters[6];
    vx_enum    inputFormat                = TENSOR_DATA_TYPE(input);
    vx_enum    outputFormat               = TENSOR_DATA_TYPE(output);
    vx_bool   shExe_flag                  = vx_false_e;
    vx_tensor weights                     = NULL;
    vx_tensor biases                      = NULL;
    vx_tensor_create_params_t tensor_create_params;
    vx_uint32 batchCount                  = TENSOR_SIZE_INDEX(input, 3);

    vxnne_batchnorm_layer  batchnormLayer = NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_batchnorm_layer_s), (gctPOINTER*)&batchnormLayer);
    if (!batchnormLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(batchnormLayer, sizeof(vxnne_batchnorm_layer_s));

    vxnneLayer_Initialize(&batchnormLayer->base,
                          "BatchNormalizationLayer",
                          node,
                          vxmOPERATION_COUNT(batchnormLayer),
                          batchnormLayer->operations,
                          vxnneLayer_Deinitialize);

    if(context->evisNoInst.supportEVIS)
    {
        shExe_flag = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
                          || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT8)
                          || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT16)
                          || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_UINT8)
                          || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8)
                          || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_FLOAT16)
                          || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16)
                          || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_FLOAT16)
                          || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
                          || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT16)
                          || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_BFLOAT16)
                          || (inputFormat == VX_TYPE_BFLOAT16 && outputFormat == VX_TYPE_BFLOAT16));
    }
    else
    {
        shExe_flag = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
                          || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT32)
                          || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32)
                          || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT16)
                          || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8));
    }

    if (shExe_flag && vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER))
    {
        vxnne_shader_executable shaderExecutable = VX_NULL;
        vx_uint32  sizes[]          = {1, 1, 1, 1};
        vx_uint32  dims             = 2;
        vx_float32 inputScale       = 1.0f;
        vx_int32   input_ZP         = 0;
        vx_float32 outputScale      = 1.0f;
        vx_float32 output_ZP        = 0.0f;
        vx_int8    srcFixPointPos   = TENSOR_POS(input);
        vx_int8    dstFixPointPos   = TENSOR_POS(output);
        vx_uint32  axis             = 2;

        if (TENSOR_DIM_NUM(input) < 3)
            axis = 0;

        sizes[0] = context->evisNoInst.supportEVIS ? TENSOR_VIEW_SIZE_INDEX(input, axis) * 2 : TENSOR_VIEW_SIZE_INDEX(input, axis);
        sizes[1] = 1;

        gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
        tensor_create_params.num_of_dims = dims;
        tensor_create_params.sizes = sizes;
        tensor_create_params.data_format = context->evisNoInst.supportEVIS ? VX_TYPE_INT16 : VX_TYPE_FLOAT32;
        tensor_create_params.quant_format = VX_QUANT_NONE;
        tensor_create_params.quant_data.dfp.fixed_point_pos = 0;

        weights = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_false_e);
        if (vxoTensor_AllocateMemory(weights) != VX_SUCCESS)
        {
            vxError("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }

        biases = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_false_e);
        if (vxoTensor_AllocateMemory(biases) != VX_SUCCESS)
        {
            vxError("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }

        if (TENSOR_QUANT_TYPE(input) == VX_QUANT_DYNAMIC_FIXED_POINT)
        {
            if (srcFixPointPos >= 0)
            {
                inputScale = 1.0f / (vx_float32) (1 << srcFixPointPos);
            }
            else if (srcFixPointPos < 0)
            {
                inputScale = (vx_float32) (1 << -srcFixPointPos);
            }
        }
        else if (TENSOR_QUANT_TYPE(input) == VX_QUANT_AFFINE_SCALE)
        {
            input_ZP   = context->evisNoInst.supportEVIS ? 0 : TENSOR_TF_ZEROPOINT(input);
            inputScale = TENSOR_TF_SCALE(input);
        }

        if (TENSOR_QUANT_TYPE(output) == VX_QUANT_DYNAMIC_FIXED_POINT)
        {
            if (dstFixPointPos >= 0)
            {
                outputScale = (vx_float32) (1 << dstFixPointPos);
            }
            else if (dstFixPointPos < 0)
            {
                outputScale = 1.0f / (vx_float32) (1 << -dstFixPointPos);
            }
        }
        else if (TENSOR_QUANT_TYPE(output) == VX_QUANT_AFFINE_SCALE)
        {
            outputScale = 1.0f / TENSOR_TF_SCALE(output);
            output_ZP   = (vx_float32)TENSOR_TF_ZEROPOINT(output);
            if (context->evisNoInst.supportEVIS == vx_false_e)
                output_ZP += 0.5f;
        }

        vxnneExecuteSWBatchNormPreProcess(means, variances, gamma, beta, epss->value->f32, inputScale, input_ZP, outputScale, output_ZP, weights, biases);
        if(context->evisNoInst.supportEVIS)
        {
            shaderExecutable = vxnneGetBatchNormShaderExecutable(node->base.context, VXNNE_KERNEL_BATCHNORM, &node->kernelAttributes.borderMode, axis, input, weights, biases, output);
        }
        else
        {
            shaderExecutable = vxnneGetGPUBatchNormShaderExecutable(node->base.context, VXNNE_KERNEL_GPU_BATCHNORM, &node->kernelAttributes.borderMode, axis, input, weights, biases, output);
        }

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&batchnormLayer->batchnorm_sh_operation,
                                        &batchnormLayer->base,
                                        VXNNE_OPERATOR_BATCHNORM,
                                        batchCount,
                                        shaderExecutable);

        if (status != VX_SUCCESS) goto exit;

        vxnneLayer_SetOperation(
            &batchnormLayer->base,
            &batchnormLayer->batchnorm_sh_operation.base,
            0);

        vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 2, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NO_BATCH_BIT);
        vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 3, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NO_BATCH_BIT);

        vxnneOperation_AddReference(&batchnormLayer->batchnorm_sh_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&batchnormLayer->batchnorm_sh_operation.base, (vx_reference)weights, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&batchnormLayer->batchnorm_sh_operation.base, (vx_reference)biases, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&batchnormLayer->batchnorm_sh_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        batchnormLayer->base.num_temp_tensors = 2;
        batchnormLayer->base.temp_tensors[0] = weights;
        batchnormLayer->base.temp_tensors[1] = biases;
    }
    else
    {
        status = vxnneOperation_Initialize(&batchnormLayer->batchnorm_operation.base,
                                           &batchnormLayer->base,
                                           VXNNE_OPERATION_TARGET_SW,
                                           VXNNE_OPERATOR_BATCHNORM,
                                           vxnneExecuteSWBatchNormalization,
                                           VX_NULL,
                                           1/*batchCount*/,
                                           0);
        if (status != VX_SUCCESS) goto exit;

        vxnneLayer_SetOperation(
            &batchnormLayer->base,
            &batchnormLayer->batchnorm_operation.base,
            0);

        batchnormLayer->batchnorm_operation.eps              = epss;
        batchnormLayer->batchnorm_operation.mean             = means;
        batchnormLayer->batchnorm_operation.variance         = variances;
        batchnormLayer->batchnorm_operation.gamma            = gamma;
        batchnormLayer->batchnorm_operation.beta             = beta;
        batchnormLayer->batchnorm_operation.input            = input;
        batchnormLayer->batchnorm_operation.output           = output;

        vxnneOperation_AddReference(&batchnormLayer->batchnorm_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&batchnormLayer->batchnorm_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }

    node->layer = &batchnormLayer->base;
    return status;

exit:
    if (batchnormLayer) gcoOS_Free(gcvNULL, (gctPOINTER)batchnormLayer);
#endif
    return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNBatchNormalizationLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

